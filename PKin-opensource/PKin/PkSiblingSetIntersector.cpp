/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkSiblingSetIntersector.h"
#include "PkStats.h"

// Switches to toggle different local subset culling policies.
// If all are turned off, the default is a null policy which does no culling
#define Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY_EXTERNAL 0
#define Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY_INTERNAL 1

// Compile time check to make sure at most a single policy is enabled
#if (Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY_EXTERNAL && Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY_INTERNAL)
	#error "Multiple local subset culling policies enabled.  Please enable a single policy or none at all."
#endif

// A policy that culls subsets outside of intersection inner loop
#if Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY_EXTERNAL

// Specific includes for this policy
#include "PkGlobalData.h"
#include "PkSubsetOracle.h"
#include "PkSubsetFrequencyMapper.h"

class PkLocalSubsetCullingPolicyExternal
{
public:

	// Simply appends the bitset for post loop SIMD culling
	inline static void processIntersection( const PkBitSet& bitSet, PkSiblingSetBitSetArray& outBitSets )
	{
		outBitSets.push_back( bitSet );
	}

	// Uses the oracle mapping method to cull subsets
	inline static void onIntersectionLoopFinished( PkSiblingSetBitSetArray& outBitSets )
	{
		// Store number of output sets
		const PkUInt numOutBitSets = outBitSets.size();

		// Short circuit if we're empty!
		if ( numOutBitSets <= 0 )
		{
			return;
		}

		// Initialize oracle
		PkSubsetOracle oracle( PkGlobalData::getPopulation().size(), numOutBitSets );

		// Frequency map each set
		PkUInt itrSets=0;
		for ( ; itrSets < numOutBitSets; ++itrSets )
		{
			oracle.computeFrequencyMapFor( outBitSets[ itrSets ], itrSets );
		}

		// Copy the sets for this round as we're going to need to clobber the outBitSets array
		const PkSiblingSetBitSetArray workingSets( outBitSets );
		outBitSets.clear();

		// Attempt to reserve a heuristic amount of memory - assuming ~25% of internal sets get culled 
		outBitSets.reserve( ( workingSets.size() * 3 ) / 4 );

		// Output set if it's not a subset
		for ( itrSets = 0; itrSets < numOutBitSets; ++itrSets )
		{
			if ( !oracle.isSubset( workingSets[ itrSets ], itrSets, workingSets ) )
			{
				outBitSets.push_back( workingSets[ itrSets ] );
			}
		}
	}

private:

	// Disallow instantiation and assignment as best as possible
	PkLocalSubsetCullingPolicyExternal();
	PkLocalSubsetCullingPolicyExternal( const PkLocalSubsetCullingPolicyExternal& );
	PkLocalSubsetCullingPolicyExternal& operator=( const PkLocalSubsetCullingPolicyExternal& );
};

// A typedef to abstract which policy we actually use
typedef PkLocalSubsetCullingPolicyExternal PkLocalSubsetCullingPolicy;

#elif Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY_INTERNAL

// Specific includes for this policy
#include "PkBitSetUtils.h"

// An inner loop culling policy	
class PkLocalSubsetCullingPolicyInternal
{
public:

	// Culls subset within intersection loop
	inline static void processIntersection( const PkBitSet& bitSet, PkSiblingSetBitSetArray& outBitSets )
	{
		PkBitSetUtils::conditionalAppendAndCullSubSets( bitSet, outBitSets );
	}

	// Does nothing
	inline static void onIntersectionLoopFinished( PkSiblingSetBitSetArray& outBitSets )
	{
	}

private:

	// Disallow instantiation and assignment as best as possible
	PkLocalSubsetCullingPolicyInternal();
	PkLocalSubsetCullingPolicyInternal( const PkLocalSubsetCullingPolicyInternal& );
	PkLocalSubsetCullingPolicyInternal& operator=( const PkLocalSubsetCullingPolicyInternal& );
};

// A typedef to abstract which policy we actually use
typedef PkLocalSubsetCullingPolicyInternal PkLocalSubsetCullingPolicy;

// Don't enable any subset culling policy
#else

class PkLocalSubsetCullingPolicyNull
{
public:

	// Simply appends, does not cull
	inline static void processIntersection( const PkBitSet& bitSet, PkSiblingSetBitSetArray& outBitSets )
	{
		outBitSets.push_back( bitSet );
	}

	// Does nothing
	inline static void onIntersectionLoopFinished( PkSiblingSetBitSetArray& outBitSets )
	{
	}

private:

	// Disallow instantiation and assignment as best as possible
	PkLocalSubsetCullingPolicyNull();
	PkLocalSubsetCullingPolicyNull( const PkLocalSubsetCullingPolicyNull& );
	PkLocalSubsetCullingPolicyNull& operator=( const PkLocalSubsetCullingPolicyNull& );
};

// A typedef to abstract which policy we actually use
typedef PkLocalSubsetCullingPolicyNull PkLocalSubsetCullingPolicy;

#endif // Pk_ENABLE_LOCAL_SUBSET_CULLING_POLICY 

// Initializes job
void PkSiblingSetIntersector::init( const PkUInt idxStart, const PkUInt idxEnd, const PkSiblingSetBitSetArray& firstSets, const PkSiblingSetBitSetArray& secondSets )
{
	PkAssert( idxEnd > idxStart );
	PkAssert( idxStart >= 0 );
	PkAssert( idxStart < firstSets.size() );
	PkAssert( idxEnd >= 0 );
	PkAssert( idxEnd <= firstSets.size() );
	
	m_idxStart = idxStart;
	m_idxEnd = idxEnd;
	m_pFirstSets = &firstSets;
	m_pSecondSets = &secondSets;

	PkBitSetBridgeUtils::reinit( m_OutputSets, firstSets.size() > 0 ? firstSets[0].size() : 0 );
}

// Intersects parameter data sets across each other
PkBool PkSiblingSetIntersector::doWork( const PkUInt threadId )
{
	// Track total time of intersection work done at this thread
	Pk_SCOPED_STAT_THREADED_TIMER( ePkSTAT_ThreadedIntersectionTotalTime, threadId );

	// Store size of second set
	const PkUInt numSecond = this->getSecondSets().size();

	// Track total number of intersections done at this thread
	Pk_INC_STAT_THREADED_COUNTER_BY( ePkSTAT_ThreadedIntersectionsCount, threadId, ((m_idxEnd-m_idxStart)*numSecond) );

	// Store computed thread-based offset to reduce memory contention 
	const PkUInt idxOffset = this->getThreadOffset();

	// Iterate over parameter range of first set
	for ( PkUInt idxFirst=m_idxStart; idxFirst<m_idxEnd; ++idxFirst )
	{
		// Store reference to current first bitset
		PkBitTraits<PkSiblingSetBitSetArray>::tBitSetConstHandle firstSet = this->getFirstSets()[ idxFirst ];

		// Intersect current first set with every set in second set
		PkUInt numIters = 0;
		// Avoid memory contention by starting intersections at different offsets based on thread id
		for ( PkUInt idxSecond=idxOffset; numIters++<numSecond; idxSecond=(idxSecond+1)%numSecond )
		{
			// Perform intersection
			PkAssert( firstSet.size() == this->getSecondSets()[ idxSecond ].size() );
			const PkBitSet intersectionSet( firstSet & this->getSecondSets()[ idxSecond ] );

			// If not an empty intersection, add it to output sets if no overlaps
			if ( intersectionSet.any() )
			{
				// Defer to our policy for inner loop processing of potential subsets
				Pk_SCOPED_STAT_THREADED_TIMER( ePkSTAT_ThreadedLocalSubsetPolicyProcessIntersectionTime, threadId );
				PkLocalSubsetCullingPolicy::processIntersection( intersectionSet, this->getOutputSets() );
			}
		}
	}

	// Defer to our policy for external to loop processing of potential subsets
	{
		Pk_SCOPED_STAT_THREADED_TIMER( ePkSTAT_ThreadedLocalSubsetPolicyOnIntersectionLoopFinishedTime, threadId );
		PkLocalSubsetCullingPolicy::onIntersectionLoopFinished( this->getOutputSets() );
	}

	// Signify we finished the job
	return true;
}

// Avoid memory contention by starting intersections at different offsets based on thread id
PkUInt PkSiblingSetIntersector::getThreadOffset() const
{
	// @todo: get this part working
	return 0;
}
