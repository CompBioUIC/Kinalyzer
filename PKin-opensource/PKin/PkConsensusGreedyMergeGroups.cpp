/**
 * @author Alan Perez-Rathke 
 *
 * @date July 3, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkConsensus.h"
#include "PkConsensusCosts.h"
#include "PkGlobalData.h"
#include "PkConsensusSiblingGroup.h"
#include "PkStats.h"

// HACK: http://www.codeguru.com/forum/showthread.php?t=369447
// Windows defines min/max macros which conflict with any other min/max tokens
#if WINDOWS
#	undef max
#	undef min
#endif
#include <limits>

#include <math.h>

// Our initial minimum cost for determing which groups to merge
// Note: may be replaced by initializing with cost to merge group 0 with group 1
#define Pk_CONSENSUS_INITIAL_MIN_COST std::numeric_limits<double>::max()

namespace
{

// Utility function to populate a set of sibling groups from a bitset representation
// This is necessary in order to reconstruct the allele mappings and compute merge costs
void createSiblingGroupsFromBitSets
( 
  PkArray( PkConsensusSiblingGroup )& outSiblingGroups
, const PkBitSetArray& bitSetGroups
, const PkLociArray( PkInt )& lociIndices
)
{
	// Pre-allocate some memory for sibling groups
	outSiblingGroups.resize( bitSetGroups.size() );

	// Iterate over each bitset and convert a full-fledged sibling group data structure
	for ( PkInt itrBitSets=(PkInt)bitSetGroups.size()-1; itrBitSets>=0; --itrBitSets )
	{
		outSiblingGroups[ itrBitSets ].initFromBitSet(
			  bitSetGroups[ itrBitSets ]
			, lociIndices
			);
	}
}

// @return true if getMergeCost() reward and punishment loops should continue
inline PkBool shouldContinueCalculatingMergeCost
(
  const PkBitSet::size_type idxIndividual
, const double cost
, const double MaxErrorRateCost
)
{
	return ( idxIndividual != PkBitSet::npos )
		&& ( cost <= PkConsensus::getCost( PkConsensus::EC_MAX_GROUP_EDIT ) )
		&& ( cost <= MaxErrorRateCost );
}

// Attempt to assign all individuals in B to A
// If this is possible, return as a reward, else return as a punishment	
double getMergeCost
(
  const PkConsensusSiblingGroup& grpA
, const PkConsensusSiblingGroup& grpB
, const PkLociArray( PkInt )& lociIndices
)
{
	// Store reference to population
	const PkArray( PkIndividual )& population( PkGlobalData::getPopulation() );

	// Create a scratch copy of the first group
	PkConsensusSiblingGroup grpTemp( grpA );

	// The final output cost
	double cost = 0.0;

	// Initialize our individual iterator
	PkBitSet::size_type idxIndividual = grpB.getAssignedIndividuals().find_first();

	// Store max error rate cost
	const double MaxErrorRateCost = ceil( PkConsensus::getCost( PkConsensus::EC_MAX_ERROR_RATE ) * ((double)(grpA.getNumAssignedIndividuals()+grpB.getNumAssignedIndividuals())) );

	// Reward loop
	while ( shouldContinueCalculatingMergeCost( idxIndividual, cost, MaxErrorRateCost ) )
	{
		// Assert on bounds checks
		PkAssert( idxIndividual != PkBitSet::npos );
		PkAssert( idxIndividual >= 0 );
		PkAssert( idxIndividual < population.size() );

		// Attempt to assign individual, if possible, then reward, else, kick out of loop
		if ( ESGARC_Fail == grpTemp.conditionalAssignIndividual(
				  population[ idxIndividual ]
				, (PkInt)idxIndividual
				, lociIndices
				) )
		{
			// Reset scratch group
			grpTemp = grpA;
			// Reset cost, wipe all rewards
			cost = 0.0;
			// Exit reward loop
			break;
		}

		// Apply reward for successfully assigning this individual to the group
		cost += grpTemp.calcAssignmentReward( population[ idxIndividual ] );

		// Advance to next individual
		idxIndividual = grpB.getAssignedIndividuals().find_next( idxIndividual );
	}

	// Punishment loop
	while ( shouldContinueCalculatingMergeCost( idxIndividual, cost, MaxErrorRateCost ) )
	{
		// Assert on bounds checks
		PkAssert( idxIndividual != PkBitSet::npos );
		PkAssert( idxIndividual >= 0 );
		PkAssert( idxIndividual < population.size() );

		// Apply punishment for assigning this individual to the group
		// Using integer truncation, hence the cast - load hit store :( 
		const double tempCost = (PkInt)(grpTemp.calcAssignmentPunishment( population[ idxIndividual ] ));
		cost += ( tempCost <= PkConsensus::getCost( PkConsensus::EC_MAX_EDIT ) ) ?
			tempCost : PkConsensus::getCost( PkConsensus::EC_INFINITY );

		// Advance to next individual
		idxIndividual = grpB.getAssignedIndividuals().find_next( idxIndividual );
	}

	return cost;
}

// Utility function to find the lowest cost groups to merge
void getLowestCostGroupsToMerge
(
  double& outMinCost
, PkInt& outMinIdxA
, PkInt& outMinIdxB
, const PkArray( PkConsensusSiblingGroup )& siblingGroups
, const PkLociArray( PkInt )& lociIndices
)
{
	// Initialize our output costs and indices
	outMinCost = Pk_CONSENSUS_INITIAL_MIN_COST;
	outMinIdxA = Pk_INVALID_INDEX;
	outMinIdxB = Pk_INVALID_INDEX;

	// Find lowest cost merge by iterating over upper tri
	const PkInt numSiblingGroups = (PkInt)siblingGroups.size();
	for ( PkInt itrGroupA=0; itrGroupA<numSiblingGroups; ++itrGroupA )
	{
		for ( PkInt itrGroupB=itrGroupA+1; itrGroupB<numSiblingGroups; ++itrGroupB )
		{
			// Compute cost of merging first group with second group
			const PkConsensusSiblingGroup& groupA( siblingGroups[ itrGroupA ] );
			const PkConsensusSiblingGroup& groupB( siblingGroups[ itrGroupB ] );
			const double mergeCostAToB = getMergeCost( groupA, groupB, lociIndices );
			const double mergeCostBToA = getMergeCost( groupB, groupA, lociIndices );
			
			// See if merging A to B is the cheapest so far
			if ( mergeCostAToB < outMinCost )
			{
				outMinCost = mergeCostAToB + groupA.getEditCost();
				outMinIdxA = itrGroupA;
				outMinIdxB = itrGroupB;
			}

			// See if merging B to A is the cheapest so far
			if ( mergeCostBToA < outMinCost )
			{
				outMinCost = mergeCostBToA + groupB.getEditCost();
				outMinIdxA = itrGroupA;
				outMinIdxB = itrGroupB;
			}
		}
	}
}

// @return true if we are finished merging! false otherwise
inline PkBool shouldTerminateMergingLoop
(
  const double minCost
, const PkInt minIdxA
, const PkInt minIdxB
, const PkArray( PkConsensusSiblingGroup )& siblingGroups
)
{
	return ( minIdxA == Pk_INVALID_INDEX )
		|| ( minIdxB == Pk_INVALID_INDEX )
		|| ( minCost > PkConsensus::getCost( PkConsensus::EC_MAX_GROUP_EDIT ) )
		|| ( minCost > ceil( PkConsensus::getCost( PkConsensus::EC_MAX_ERROR_RATE ) * ((double)(siblingGroups[minIdxA].getNumAssignedIndividuals()+siblingGroups[minIdxB].getNumAssignedIndividuals())) ) );
}

} // end of unnamed namespace

namespace PkConsensus
{

// Greedily merges groups
void greedyMergeGroups( PkArray( PkConsensusSiblingGroup )& outSiblingGroups, const PkBitSetArray& groupsToMerge )
{
	PkLogf( "Greedily merging %u groups.\n", groupsToMerge.size() );
	Pk_SCOPED_STAT_TIMER( ePkSTAT_ConsensusGreedyMergeGroupsTime );

	// Initialize our loci indices array
	PkLociArray( PkInt ) lociIndices( PkGlobalData::getNumLoci() );
	for ( PkInt itrLocus=(PkInt)lociIndices.size()-1; itrLocus>=0; --itrLocus )
	{
		lociIndices[ itrLocus ] = itrLocus;
	}

	// Convert bitsets to actual sibgroups to restore loci information
	PkLogf( "\tConverting bit sets to consensus sibling group data structures\n" );
	createSiblingGroupsFromBitSets( outSiblingGroups, groupsToMerge, lociIndices );

	// Keep performing lowest cost merge until either:
	// - no more merges left
	// - no merge is within the cost thresholds allowed
	while ( true )
	{
		// Determine the two lowest cost groups to merge
		double minCost = Pk_CONSENSUS_INITIAL_MIN_COST;
		PkInt minIdxA = Pk_INVALID_INDEX, minIdxB = Pk_INVALID_INDEX;
		getLowestCostGroupsToMerge( minCost, minIdxA, minIdxB, outSiblingGroups, lociIndices );
		PkAssert( (minIdxA >= 0 && minIdxA < (PkInt)outSiblingGroups.size()) || (minIdxA == Pk_INVALID_INDEX) );
		PkAssert( (minIdxB >= 0 && minIdxB < (PkInt)outSiblingGroups.size()) || (minIdxB == Pk_INVALID_INDEX) );
		
		// Check our loop termination condition!
		if ( shouldTerminateMergingLoop( minCost, minIdxA, minIdxB, outSiblingGroups ) )
		{
			// We finished!  No more merging.
			return;
		}
		
		// The cost of merge is acceptable, let us merge the two groups
		PkLogf( "\tMerging group %d with group %d with cost %f\n", minIdxA, minIdxB, minCost );

		// Update our edit cost
		outSiblingGroups[ minIdxA ].setEditCost(
			std::max<double>( minCost, outSiblingGroups[ minIdxA ].getEditCost() )
			);

		// Add individuals from second group to first group
		if ( minCost < outSiblingGroups[ minIdxA ].getEditCost() )
		{
			const PkConsensusSiblingGroup& constRefB( outSiblingGroups[ minIdxB ] );
			// Attempt to update the group's allele mappings and parent sets
			outSiblingGroups[ minIdxA ].conditionalAssignIndividualsFromBitSet(
				  constRefB.getAssignedIndividuals()
				, lociIndices
				);
		}
		else
		{
			// Just union the assignments
			outSiblingGroups[ minIdxA ].forceAssignIndividualsFrom(
				  outSiblingGroups[ minIdxB ]
				);
		}

		// Now, remove (unordered) the second group
		outSiblingGroups[ minIdxB ] = outSiblingGroups.back();
		outSiblingGroups.pop_back();
	}
}

} // end of PkConsensus namespace
