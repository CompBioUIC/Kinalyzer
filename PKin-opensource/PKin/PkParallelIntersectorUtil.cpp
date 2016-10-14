/**
 * @author Alan Perez-Rathke 
 *
 * @date March 17, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkGlobalData.h"
#include "PkBlockableRunnable.h"
#include "PkSiblingSetIntersector.h"
#include "PkBitSetUtils.h"
#include "PkStats.h"
#include "PkMiscUtil.h"
#include <algorithm>
#include <numeric>

// A switch to determine if CUDA should be used for intersections
#define Pk_ENABLE_CUDA_INTERSECTION 1

// A switch to enable parallel global subset culling, else serial global subset culling will be used
#define Pk_ENABLE_PARALLEL_SUBSET_CULLING 1

#if Pk_ENABLE_PARALLEL_SUBSET_CULLING
	#include "PkSubsetCullingParallelPolicy.h"
	typedef PkSubsetCullingParallelPolicy PkSubsetCullingPolicy;
#else
	#include "PkSubsetCullingSerialPolicy.h"
	typedef PkSubsetCullingSerialPolicy PkSubsetCullingPolicy;
#endif

// For dynamic balancing, will auto-balance so that jobs have no more than this many base sets to intersect
#define Pk_DYNAMIC_BALANCE_MAX_SET_RESOLUTION 1000

// For dynamic balancing, the min number of jobs per software thread
#define Pk_DYNAMIC_BALANCE_MIN_JOBS_PER_THREAD 3

namespace
{
	inline PkInt computeSetResolutionForLogicalThreadCount( const PkInt numFirstSets, const PkInt numLogicalThreads )
	{
		PkAssert( numLogicalThreads > 0 );
		return std::max<PkInt>( 1, numFirstSets / numLogicalThreads );
	}
	// @return number of sets to assign to each job
	PkInt computeSetResolution( const PkInt numFirstSets )
	{
		if ( PkGlobalData::getNumLogicalThreadsLociIntersection() > 0 )
		{
			return computeSetResolutionForLogicalThreadCount( numFirstSets, PkGlobalData::getNumLogicalThreadsLociIntersection() );
		}
		else
		{
			const PkInt numSoftwareThreads = (PkInt)PkGlobalData::getThreadPool().numThreads();
			const PkInt threadMultiplier = std::max<PkInt>( Pk_DYNAMIC_BALANCE_MIN_JOBS_PER_THREAD, numFirstSets / ( Pk_DYNAMIC_BALANCE_MAX_SET_RESOLUTION * numSoftwareThreads ) );
			return computeSetResolutionForLogicalThreadCount( numFirstSets, threadMultiplier * numSoftwareThreads );
		}
	}
}

namespace Pk
{

#if Pk_ENABLE_CUDA_INTERSECTION

void IntersectSiblingSets( PkSiblingSetBitSetArray& outputSets, const PkSiblingSetBitSetArray& partitionSets )
{
	// Track total intersection overhead
	Pk_SCOPED_STAT_TIMER( ePkSTAT_IntersectSiblingSetsTotalTime );

	// Track number of calls to this function
	Pk_INC_STAT_COUNTER( ePkSTAT_IntersectSiblingSetsCallCount );

	extern void CUDAIntersectSiblingSets(
		  PkSiblingSetBitSetArray& outputSets
		, const PkSiblingSetBitSetArray& partitionSets
		);

	CUDAIntersectSiblingSets( outputSets, partitionSets );

	// Update subset culling policy after intersections finished
	{
		Pk_SCOPED_STAT_TIMER( ePkSTAT_GlobalSubsetPolicyOnIntersectionLoopFinishTime );
		PkSubsetCullingPolicy::OnIntersectionLoopFinish( outputSets );
	}
}

#else

// Intersects each set of outputSets with each set of partitionSets and append to outputSets
void IntersectSiblingSets( PkSiblingSetBitSetArray& outputSets, const PkSiblingSetBitSetArray& partitionSets )
{
	// Track total intersection overhead
	Pk_SCOPED_STAT_TIMER( ePkSTAT_IntersectSiblingSetsTotalTime );

	// Track number of calls to this function
	Pk_INC_STAT_COUNTER( ePkSTAT_IntersectSiblingSetsCallCount );

	// Copy the sets for this round as we're going to need to clobber the outputSets array
	const PkSiblingSetBitSetArray workingSets( outputSets );
	outputSets.clear();
	PkBitSetBridgeUtils::reinit( outputSets, PkGlobalData::getPopulation().size() );

	// Determine first set, we want it to be larger to have better balancing with the current scheme
	const PkBool workingSetsIsBiggerThanPartitionSets = workingSets.size() > partitionSets.size();
	const PkSiblingSetBitSetArray& firstSet = workingSetsIsBiggerThanPartitionSets ? workingSets : partitionSets;
	// Second set should be the smaller of the two sets for better load balancing
	const PkSiblingSetBitSetArray& secondSet = workingSetsIsBiggerThanPartitionSets ? partitionSets : workingSets;

	// Determine major sets size
	const PkInt numFirstSets = (PkInt)firstSet.size();

	// The max number of output sets to assign to a single job
	// For now, make resolution a function of the number of threads
	// Though, a smaller granularity would potentially give better load balancing
	const PkInt setResolution = computeSetResolution( numFirstSets );
	PkAssert( setResolution > 0 );

	// Determine number of jobs to create
	const PkInt numJobs = ( numFirstSets / setResolution ) + ( ( numFirstSets % setResolution ) > 0 );

	// Create jobs to perform intersections in parallel
	PkArray( PkSiblingSetIntersector ) intersectorJobs( numJobs );
	PkArray( PkBlockableRunnable* ) jobBlockers( numJobs );

	// Initialize and queue jobs
	PkInt itrSet = 0;
	PkInt itrJob = 0;
	while ( itrSet < numFirstSets )
	{
		// Determine range to assign to current job
		const PkInt numSetsToAssign = std::min<PkInt>( numFirstSets - itrSet, setResolution );
		const PkInt endItr = itrSet + numSetsToAssign;

		// Initialize current job
		intersectorJobs[ itrJob ].init( itrSet, endItr, firstSet, secondSet );
		jobBlockers[ itrJob ] = new PkBlockableRunnable( intersectorJobs[ itrJob ] );
		PkAssert( jobBlockers[ itrJob ] );

		// Kick off job
		PkVerify( PkGlobalData::getThreadPool().queueJob( *jobBlockers[ itrJob ] ) );
		PkLogf( "\t\tKicked off intersection job %d of %d for set indices %d to %d\n"
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
	PkAssert( itrSet == numFirstSets );
	PkAssert( itrJob == numJobs );
	PkAssert( numJobs == (PkInt)intersectorJobs.size() );
	PkAssert( intersectorJobs.size() == jobBlockers.size() );
	
	for ( itrJob=0; itrJob<numJobs; ++itrJob )
	{
		// Wait until jobs finish (simulate a barrier)
		{
			Pk_SCOPED_STAT_TIMER( ePkSTAT_IntersectSiblingSetsBlockTime );
			// Block on current job
			jobBlockers[ itrJob ]->blockUntilFinished();
		}

		// Free job blocker memory
		delete jobBlockers[ itrJob ];
		jobBlockers[ itrJob ] = NULL;
			
		// Store a reference to the intersection job output sets
		const PkSiblingSetBitSetArray& jobOutputSets =
			((const PkSiblingSetIntersector&)intersectorJobs[ itrJob ]).getOutputSets();

		// Log that we finished an intersection job
		PkLogf( "\t\tIntersection job %d of %d finished with %u unculled sets\n"
			, itrJob+1
			, numJobs
			, jobOutputSets.size()
			);
			
		// Update subset culling policy after job finished
		{
			Pk_SCOPED_STAT_TIMER( ePkSTAT_GlobalSubsetPolicyOnIntersectionJobFinishTime );
			PkSubsetCullingPolicy::OnIntersectionJobFinish( jobOutputSets, outputSets );
		}
	}

	// Update subset culling policy after intersections finished
	{
		Pk_SCOPED_STAT_TIMER( ePkSTAT_GlobalSubsetPolicyOnIntersectionLoopFinishTime );
		PkSubsetCullingPolicy::OnIntersectionLoopFinish( outputSets );
	}
}

#endif

} // end of Pk namespace
