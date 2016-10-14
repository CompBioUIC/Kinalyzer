/**
 * @author Alan Perez-Rathke 
 *
 * @date July 12th, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkConsensusSiblingGroup_h
#define PkConsensusSiblingGroup_h

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkConsensusCosts.h"
#include "PkGlobalData.h"
#include "PkSiblingGroup.h"
#include "PkPossibleParentsMap.h"

// A sibling group class with an additional field for storing edit cost
class PkConsensusSiblingGroup : public PkSiblingGroup
{
private:

	// An empty intersection predicate used for calculating punishment
	struct tEmptyIntersectionPredicate
	{
		inline PkBool shouldContinueLoop( const PkInt itrLocus, const PkInt NumLoci, const double cost ) const
		{
			return ((itrLocus<NumLoci) && (cost<=PkConsensus::getCost( PkConsensus::EC_MAX_EDIT )));
		}
		inline PkBool shouldPenalize( const PkParentSet intersection, const PkParentSet groupParentSet_unused ) const
		{
			return Pk_IS_EMPTY_PARENT_SET( intersection );
		}
	};

	// An equality intersection predicate used for calculating reward
	struct tEqualityIntersectionPredicate
	{
		inline PkBool shouldContinueLoop( const PkInt itrLocus, const PkInt NumLoci, const double cost ) const
		{
			return itrLocus < NumLoci;
		}
		inline PkBool shouldPenalize( const PkParentSet intersection, const PkParentSet groupParentSet ) const
		{
			return (intersection == groupParentSet);
		}
	};
	
	// A predicated punishment utility function
	template < typename tPredicatePolicy >
	double calcAssignmentPunishment( const PkIndividual& individual, const tPredicatePolicy policy ) const
	{
		double cost = 0.0;
		const PkInt NumLoci = PkGlobalData::getNumLoci();
		for ( PkInt itrLocus=0; policy.shouldContinueLoop( itrLocus, NumLoci, cost ); ++itrLocus )
		{
			const tAlleleMap& alleleMap = this->getMappedAlleles()[ itrLocus ];

			// See if first allele is mapped
			const tAlleleMap::const_iterator xMapIter =
				std::find( alleleMap.begin(), alleleMap.end(), individual[ itrLocus ].first );

			// See if second allele is mapped
			const tAlleleMap::const_iterator yMapIter =
				std::find( alleleMap.begin(), alleleMap.end(), individual[ itrLocus ].second );

			// If both alleles are unmapped, determine if homo or hetero mistype
			if ( xMapIter == alleleMap.end() && yMapIter == alleleMap.end() )
			{
				// A homo mistype occurs if both alleleles are the same
				if ( individual[ itrLocus ].first == individual[ itrLocus ].second )
				{
					cost += PkConsensus::getCost( PkConsensus::EC_HOMO_MISTYPE_ERROR );
				}
				// A hetero mistype occurs otherwise (when they are different)
				else
				{
					cost += 2.0 * PkConsensus::getCost( PkConsensus::EC_HETERO_MISTYPE_ERROR );
				}
			}
			// Check to see if only a single allele was unmapped
			else if ( xMapIter == alleleMap.end() || yMapIter == alleleMap.end() )
			{
				cost += PkConsensus::getCost( PkConsensus::EC_HETERO_MISTYPE_ERROR );
			}
			// Check to see if we have a null parent set
			else
			{
				const PkInt xMap = xMapIter - alleleMap.begin();
				const PkInt yMap = yMapIter - alleleMap.begin();
				PkParentSet parentSetIntersection = PkPossibleParentsMap::map( xMap, yMap );
				parentSetIntersection &= this->getParentSets()[ itrLocus ];
				if ( policy.shouldPenalize( parentSetIntersection, this->getParentSets()[ itrLocus ] ) )
				{
					cost += PkConsensus::getCost( PkConsensus::EC_POSSIBILITY_MISFIT );
				}
			}
		}

		return cost;
	}

public:

	// Default constructor, initializes edit cost
	PkConsensusSiblingGroup() : m_EditCost( 0.0 ) {}

	// Read-only accessor for edit cost data member
	inline double getEditCost() const 
	{ 
		return m_EditCost; 
	}

	// Mutator for edit cost data member
	inline void setEditCost( const double inEditCost ) 
	{ 
		m_EditCost = inEditCost; 
	}

	// @return punishment value for assigning individual to this group
	double calcAssignmentPunishment( const PkIndividual& individual ) const
	{
		return this->calcAssignmentPunishment( individual, tEmptyIntersectionPredicate() );
	}

	// @return reward value for assigning individual to this group
	double calcAssignmentReward( const PkIndividual& individual ) const
	{
		const double NumLoci = (double)PkGlobalData::getNumLoci();
		double cost = -( 2.0 * PkConsensus::getCost( PkConsensus::EC_HETERO_MISTYPE_ERROR ) + PkConsensus::getCost(  PkConsensus::EC_POSSIBILITY_MISFIT ) ) * (double)NumLoci;
		cost += this->calcAssignmentPunishment( individual, tEqualityIntersectionPredicate() );
		return cost;
	}

	// Initializes sibling group from a bitSet
	void initFromBitSet( const PkBitSet& bitSet, const PkLociArray( PkInt )& lociIndices )
	{
		// Store reference to population array
		const PkArray( PkIndividual )& population( PkGlobalData::getPopulation() );
		// Cache population size
		const PkUInt populationSize = population.size();

		// Assert bitset contains at least a single true bit
		PkAssert( bitSet.any() );

		// Find first individual index from first true bit		
		PkBitSet::size_type idxIndividual = bitSet.find_first();
		// Verify bit index is valid
		PkAssert( idxIndividual != PkBitSet::npos );
		PkAssert( idxIndividual >= 0 );
		PkAssert( idxIndividual < populationSize );

		// Initialize sibling group from first true bit
		this->init(
			  population[ idxIndividual ] // individual
			, (PkInt)idxIndividual          // idxIndividual
			, lociIndices                 // lociIndices
			, populationSize              // popSize
			);

		// Assign the remaining individuals to update allele mappings
		while ( ( idxIndividual = bitSet.find_next( idxIndividual ) ) != PkBitSet::npos )
		{
			// Assert bit index is valid
			PkAssert( idxIndividual != PkBitSet::npos );
			PkAssert( idxIndividual >= 0 );
			PkAssert( idxIndividual < populationSize );

			// Assign individual to current output group
			PkVerify( ESGARC_Fail != this->conditionalAssignIndividual(
				  population[ idxIndividual ] // individual
				, (PkInt)idxIndividual          // idxIndividual
				, lociIndices                 // lociIndices
				) );	
		}
	}

	// Assigns individuals from a bit set to a sibling group, may not always work
	void conditionalAssignIndividualsFromBitSet( const PkBitSet& bitSet, const PkLociArray( PkInt )& lociIndices )
	{
		// Store reference to population array
		const PkArray( PkIndividual )& population( PkGlobalData::getPopulation() );

		// Find first individual index from first true bit		
		PkBitSet::size_type idxIndividual = bitSet.find_first();

		while ( idxIndividual != PkBitSet::npos )
		{
			PkAssert( idxIndividual != PkBitSet::npos );
			PkAssert( idxIndividual >= 0 );
			PkAssert( idxIndividual < population.size() );
			// Assign individual to current output group
			PkVerify( ESGARC_Fail != this->conditionalAssignIndividual(
				  population[ idxIndividual ] // individual
				, (PkInt)idxIndividual          // idxIndividual
				, lociIndices                 // lociIndices
				) );	
			idxIndividual = bitSet.find_next( idxIndividual );
		}
	}

private:

	// Data member for storing the current edit cost
	double m_EditCost;
};

#endif // PkConsensusSiblingGroup_h
