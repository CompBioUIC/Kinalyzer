/**
 * @author Alan Perez-Rathke 
 *
 * @date August 4, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSubsetCullingParallelPolicy_h
#define PkSubsetCullingParallelPolicy_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkSubsetOracle.h"
#include "PkSubsetFrequencyMapper.h"
#include "PkSubsetCuller.h"
#include "PkGlobalData.h"
#include "PkStatsDetailedSubsetCullingStatTracker.h"

class PkSubsetCullingParallelPolicy
{
public:

	static void OnIntersectionJobFinish( const PkSiblingSetBitSetArray& jobOutputSets, PkSiblingSetBitSetArray& outputSets )
	{
		PkBitSetBridgeUtils::push_back_sibling_sets( jobOutputSets, outputSets );
	}

	static void OnIntersectionLoopFinish( PkSiblingSetBitSetArray& outputSets )
	{
		// Store number of output sets
		const PkInt numOutputSets = (PkInt)outputSets.size();

		// The max number of output sets to assign to a single job
		// For now, make resolution a function of the number of threads
		// Though, a smaller granularity would potentially give better load balancing
		const PkInt numLogicalThreads = (PkInt)PkGlobalData::getThreadPool().numThreads();
		const PkInt setResolution = std::max<PkInt>( 1, numOutputSets / numLogicalThreads );

		// Determine number of jobs to create
		const PkInt numJobs = ( numOutputSets / setResolution ) + ( ( numOutputSets % setResolution ) > 0 );

		//**********************************
		// TODO: FACTOR INTO A FUNCTION
		//**********************************

		// Initialize and queue jobs for frequency mapping
		PkInt itrSet = 0;
		PkInt itrJob = 0;

		// Initialize oracle
		PkSubsetOracle oracle( PkGlobalData::getPopulation().size(), numOutputSets );

		// Create jobs to perform frequency mapping
		PkArray( PkSubsetFrequencyMapper ) frequencyMapperJobs( numJobs );
		PkArray( PkBlockableRunnable* ) frequencyMapperBlockers( numJobs );

		while ( itrSet < numOutputSets )
		{
			// Determine range to assign to current job
			const PkInt numSetsToAssign = std::min<PkInt>( numOutputSets - itrSet, setResolution );
			const PkInt endItr = itrSet + numSetsToAssign;

			// Initialize current job
			frequencyMapperJobs[ itrJob ].init( oracle, outputSets, itrSet, endItr );
			frequencyMapperBlockers[ itrJob ] = new PkBlockableRunnable( frequencyMapperJobs[ itrJob ] );
			PkAssert( frequencyMapperBlockers[ itrJob ] );

			// Kick off job
			PkVerify( PkGlobalData::getThreadPool().queueJob( *frequencyMapperBlockers[ itrJob ] ) );
			PkLogf( "\t\tKicked off frequency mapper job %d of %d for set indices %d to %d\n"
				, itrJob+1
				, numJobs
				, itrSet
				, endItr-1
				);

			// Update iterators
			itrSet = endItr;
			++itrJob;
		}

		// Assert on loop postconditions
		PkAssert( itrSet == numOutputSets );
		PkAssert( itrJob == numJobs );
		PkAssert( numJobs == (PkInt)frequencyMapperJobs.size() );
		PkAssert( frequencyMapperJobs.size() == frequencyMapperBlockers.size() );

		// Wait until frequency remapper jobs finish (simulate a barrier)
		{
			Pk_SCOPED_STAT_TIMER( ePkSTAT_GlobalSubsetFrequencyMappingTime );
			for ( itrJob=0; itrJob<numJobs; ++itrJob )
			{
				// Block on current job
				frequencyMapperBlockers[ itrJob ]->blockUntilFinished();

				// Free job blocker memory
				delete frequencyMapperBlockers[ itrJob ];
				frequencyMapperBlockers[ itrJob ] = NULL;

				// Log that we finished an intersection job
				PkLogf( "\t\tFrequency mapper job %d of %d finished.\n"
				, itrJob+1
				, numJobs
				);
			}
		} // end of barrier


		//**********************************
		// TODO: FACTOR INTO A FUNCTION
		//**********************************

		// Copy the sets for this round as we're going to need to clobber the outputSets array
		const PkSiblingSetBitSetArray workingSets( outputSets );
		outputSets.clear();
		PkBitSetBridgeUtils::reinit( outputSets, PkGlobalData::getPopulation().size() );

		// The subsets mask for marking subsets
		PkBitSet subsetsMask( numOutputSets );
		PkAssert( subsetsMask.size() == numOutputSets );
		subsetsMask.set(); // set all bits to 1 for 'not a subset'
		PkAssert( subsetsMask.any() == true );

		// Create jobs to perform subset culling
		PkArray( PkSubsetCuller ) cullerJobs( numJobs );
		PkArray( PkBlockableRunnable* ) cullerBlockers( numJobs );

		// Initialize and queue jobs for frequency mapping
		itrSet = 0;
		itrJob = 0;
		while ( itrSet < numOutputSets )
		{
			// Determine range to assign to current job
			const PkInt numSetsToAssign = std::min<PkInt>( numOutputSets - itrSet, setResolution );
			const PkInt endItr = itrSet + numSetsToAssign;

			// Initialize current job
			cullerJobs[ itrJob ].init( 
				  subsetsMask
				, oracle
				, workingSets
				, itrSet
				, endItr 
				);
			cullerBlockers[ itrJob ] = new PkBlockableRunnable( cullerJobs[ itrJob ] );
			PkAssert( cullerBlockers[ itrJob ] );

			// Kick off job
			PkVerify( PkGlobalData::getThreadPool().queueJob( *cullerBlockers[ itrJob ] ) );
			PkLogf( "\t\tKicked off subset culler job %d of %d for set indices %d to %d\n"
				, itrJob+1
				, numJobs
				, itrSet
				, endItr-1
				);

			// Update iterators
			itrSet = endItr;
			++itrJob;
		}
	
		// Assert on loop postconditions
		PkAssert( itrSet == numOutputSets );
		PkAssert( itrJob == numJobs );
		PkAssert( numJobs == (PkInt)cullerJobs.size() );
		PkAssert( cullerJobs.size() == cullerBlockers.size() );

		// Wait until culling jobs finish (simulate a barrier)
		{
			Pk_SCOPED_STAT_TIMER( ePkSTAT_GlobalSubsetCullingTime );

			for ( itrJob=0; itrJob<numJobs; ++itrJob )
			{
				// Block on current job
				cullerBlockers[ itrJob ]->blockUntilFinished();

				// Free job blocker memory
				delete cullerBlockers[ itrJob ];
				cullerBlockers[ itrJob ] = NULL;

				//  Copy sets that are not masked as subsets
				for 
				(
					  // Start iteration at first valid bit
					  PkBitSet::size_type itrMask = subsetsMask.test( cullerJobs[ itrJob ].getBeginIndex() ) 
						? cullerJobs[ itrJob ].getBeginIndex() : subsetsMask.find_next( cullerJobs[ itrJob ].getBeginIndex() )
					; itrMask<(PkUInt)cullerJobs[itrJob].getEndIndex()
					; itrMask = subsetsMask.find_next( itrMask ) 
				)
				{
					PkAssert( itrMask != PkBitSet::npos );
					outputSets.push_back( workingSets[ itrMask ] );
				}

				// Log that we finished an intersection job
				PkLogf( "\t\tSubset culler job %d of %d finished.\n"
					, itrJob+1
					, numJobs
					);
			}
		} // end of barrier

		// Log number of resulting sets
		PkLogf( "\t\tGlobal subset culling finished with %u sets (down from %u => %f percent culled)\n", outputSets.size(), workingSets.size(), (100.0 * ( workingSets.size() - outputSets.size() )) / std::max<double>( 1.0, workingSets.size() ) ); 

		// Record any stats on the subset culling process
		Pk_STATS_UPDATE_FOR_SUBSET_CULLING( workingSets, subsetsMask );
	}
};

#endif // PkSubsetCullingParallelPolicy_h
