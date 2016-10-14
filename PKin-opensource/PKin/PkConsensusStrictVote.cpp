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
#include "PkConsensusSetCoverParser.h"
#include "PkGlobalData.h"
#include "PkRunnable.h"
#include "PkBlockableRunnable.h"
#include "PkStats.h"

namespace
{
	// Parse set cover files for each locus
	void loadSetCovers( PkArray( PkBitSetArray )& outSetCovers  )
	{
		PkAssert( outSetCovers.size() == PkGlobalData::getNumLoci() );
		for ( PkUInt itrLocus=0; itrLocus<outSetCovers.size(); ++itrLocus )
		{
			// Determine base intersection file name
			PkString intersectionBaseFileName;
			PkConsensus::getIntersectionOutputFileName( intersectionBaseFileName, itrLocus );

			// Determine set cover file name
			PkString setCoverFileName;
			PkConsensus::getWriteSolutionScriptOutputFileName( setCoverFileName, intersectionBaseFileName );

			// Parse set cover
			PkVerify( 1 == PkConsensusSetCoverParser::parseFile( 
				  setCoverFileName.c_str()
				, PkGlobalData::getPopulation().size()
				, outSetCovers[ itrLocus ] 
				) );
		}
	}
} // end of unnamed namespace

// Determines which pairs are allowable within a locus set cover
class PkLocalLocusSetCoverVoter : public PkRunnable
{
public:

	// Default constructor
	PkLocalLocusSetCoverVoter() : m_pSetCover( NULL ), m_PopulationSize( 0 ) {}

	// Initializes voter job
	void init( const PkBitSetArray& setCover, const PkUInt populationSize )
	{
		// Validate assumptions
		PkAssert( NULL == m_pSetCover );
		PkAssert( m_Votes.size() == 0 );
		PkAssert( m_IndividualFrequencies.size() == 0 );

		// Initialize set cover pointer
		m_pSetCover = &setCover;

		// Store population size
		m_PopulationSize = populationSize;
	}

	// Compute pairs that satisfy local set cover strict voting criteria
	virtual PkBool doWork( const PkUInt threadId )
	{
		// Allocate and zero our 2-D voting table
		m_Votes.resize( m_PopulationSize, PkBitSet( m_PopulationSize, 0 ) );

		// Allocate and zero frequencies list
		m_IndividualFrequencies.resize( m_PopulationSize, 0 );

		// Iterate over sets
		const PkUInt numSets = this->getSetCover().size();
		for ( PkUInt itrSets=0; itrSets<numSets; ++itrSets )
		{
			// Iterate over individuals in set
			const PkBitSet& siblingSet = this->getSetCover()[ itrSets ];
			PkAssert( ( siblingSet.size() == m_PopulationSize ) && "sibling set does not match population size" );
			for( PkBitSet::size_type itrIndividualA=siblingSet.find_first(); itrIndividualA!=PkBitSet::npos; itrIndividualA=siblingSet.find_next( itrIndividualA ) )
			{
				// Validate range for individual A iterator
				PkAssert( itrIndividualA != PkBitSet::npos );
				PkAssert( itrIndividualA >= 0 );
				PkAssert( itrIndividualA < m_PopulationSize );
		
				// Update frequency information for individual A
				if ( (++m_IndividualFrequencies[ itrIndividualA ]) != 1 )
				{
					// Don't bother checking pairs as they are now invalidated no matter what
					continue;
				}

				// Update vote status for each pair
				for ( PkBitSet::size_type itrIndividualB=siblingSet.find_next( itrIndividualA ); itrIndividualB!=PkBitSet::npos; itrIndividualB=siblingSet.find_next( itrIndividualB ) )
				{
					// Validate range for individual B iterator
					PkAssert( itrIndividualB != PkBitSet::npos );
					PkAssert( itrIndividualB >= 0 );
					PkAssert( (itrIndividualB < m_PopulationSize) && "individual iterator exceeds population size" );

					// Update vote status
					m_Votes[ itrIndividualA ].set( itrIndividualB, true );
				}
			}
		}

		// Redact any votes were an individual occurs in more than one set
		for ( PkUInt itrIndividualA=0; itrIndividualA<m_PopulationSize; ++itrIndividualA )
		{
			PkAssert( m_IndividualFrequencies[ itrIndividualA ] >= 1 );
			if ( m_IndividualFrequencies[ itrIndividualA ] > 1 )
			{
				for ( PkUInt itrIndividualB=0; itrIndividualB<m_PopulationSize; ++itrIndividualB )
				{
					m_Votes[ itrIndividualA ].set( itrIndividualB, false );
					m_Votes[ itrIndividualB ].set( itrIndividualA, false );
				}
			}

			// Make sure individual is flagged
			m_Votes[ itrIndividualA ].set( itrIndividualA, true );
		}

		return true;
	}

	// Return reference to the valid pairs within a set cover
	const PkBitSetArray& getVotes() const
	{
		return m_Votes;
	}

private:

	inline const PkBitSetArray& getSetCover() const
	{
		PkAssert( NULL != m_pSetCover );
		return *m_pSetCover;
	}

	// The set cover to determine votes for
	const PkBitSetArray* m_pSetCover;

	// The all-pairs table of votes
	PkBitSetArray m_Votes;

	// A list of frequencies of each individual
	PkArray( PkInt ) m_IndividualFrequencies;

	// The number of individuals in the population
	PkUInt m_PopulationSize;
};

namespace PkConsensus
{

// Performs a strict consensus vote for all resulting set covers
// We want to find all pairs that are:
// 1. Common across all loci set covers,
// 2. Only occur in a single set within a locus set cover
// 3. Neither of the individuals in the pair occur in more than one set within a locus set cover
void strictVote( PkBitSetArray& outGroups )
{
	PkLogf( "Performing strict consensus vote.\n" );
	Pk_SCOPED_STAT_TIMER( ePkSTAT_ConsensusStrictVoteTime );

	// Store number of loci and population size
	const PkUInt numLoci = PkGlobalData::getNumLoci();
	PkAssert( numLoci > 0 );
	const PkUInt populationSize = PkGlobalData::getPopulation().size();
	PkAssert( populationSize > 0 );

	// Load all set covers
	PkArray( PkBitSetArray ) setCovers( numLoci );
	loadSetCovers( setCovers );

	// Set up local locus set cover voter jobs
	PkArray( PkLocalLocusSetCoverVoter ) localLocusSetCoverVoterJobs( numLoci );
	
	// TODO: Factor into a function
	// Vote locally at each locus set cover
	{
		PkArray( PkBlockableRunnable* ) jobBlockers( numLoci );
		for ( PkUInt itrSetCover=0; itrSetCover<numLoci; ++itrSetCover )
		{
			// Initialize local locus set cover voter jobs
			localLocusSetCoverVoterJobs[ itrSetCover ].init( setCovers[ itrSetCover ], populationSize ); 
			
			// Allocate a job blocker for each voter job
			jobBlockers[ itrSetCover ] = new PkBlockableRunnable( localLocusSetCoverVoterJobs[ itrSetCover ] );
			PkAssert( jobBlockers[ itrSetCover ] );

			// Queue the job
			PkLogf( "\tKicking off strict vote job (%x) for loci %d of %d\n", &localLocusSetCoverVoterJobs[ itrSetCover ], itrSetCover+1, numLoci ); 
			PkVerify( PkGlobalData::getThreadPool().queueJob( *jobBlockers[ itrSetCover ] ) );
		}

		// Wait for jobs to finish
		for ( PkUInt itrSetCover=0; itrSetCover<numLoci; ++itrSetCover )
		{
			jobBlockers[ itrSetCover ]->blockUntilFinished();
			delete jobBlockers[ itrSetCover ];
		}
	}

	// Merge vote results (serial for now)
	PkBitSetArray globalLociConsensusTable( localLocusSetCoverVoterJobs[ 0 ].getVotes() );
	for ( PkUInt itrSetCover=1; itrSetCover<numLoci; ++itrSetCover )
	{
		for ( PkUInt itrIndividual=0; itrIndividual<populationSize; ++itrIndividual )
		{
			// Intersect rows to determine which pairs meet the strict consensus
			globalLociConsensusTable[ itrIndividual ] &= 
				localLocusSetCoverVoterJobs[ itrSetCover ].getVotes()[ itrIndividual ];
		}
	}

	// A mask for creating disjoint sets
	PkBitSet populationMask( populationSize, true );
	populationMask.set(); // make sure all bits are true

	// Output bit sets that pass the strict consensus
	PkBitSet::size_type itrIndividual = 0;
	while ( ( itrIndividual = populationMask.find_first() ) != PkBitSet::npos )
	{
		PkAssert( itrIndividual >= 0 );
		PkAssert( itrIndividual < globalLociConsensusTable.size() );
		PkAssert( ( globalLociConsensusTable[ itrIndividual ] & populationMask ) == globalLociConsensusTable[ itrIndividual ] );
		outGroups.push_back( globalLociConsensusTable[ itrIndividual ] );
		populationMask -= outGroups.back();
	}
}

} // end of PkConsensus namespace
