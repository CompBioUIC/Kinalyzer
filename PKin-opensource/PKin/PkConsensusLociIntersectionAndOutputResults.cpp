/**
 * @author Alan Perez-Rathke 
 *
 * @date June 26, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkGlobalData.h"
#include "PkParallelIntersectorUtil.h"
#include "PkMiscUtil.h"
#include "PkPhases.h"
#include "PkStats.h"
#include "PkConsensus.h"

// Flip this switch to allow redundant intersections (for profiling purposes)
#define Pk_ENABLE_NAIVE_CONSENSUS_INTERSECTION 0

// The max threshold [0,1] for disparity between left and right intersections trees
// If disparity is greater than this value, then left intersections tree will not be computed recursively
#define Pk_CONSENSUS_COMPUTE_LEFT_INTERSECTIONS_TREE_THRESH 0.3

namespace
{
	// Associates a set of intersections with a mask denoting which loci have been intersected to produce this set
	struct PkIntersectionsInfo
	{
		PkBitSet mask;
		PkSiblingSetBitSetArray intersections;
	};

	// Prototypes
	static void ConsensusLociIntersectionAndOutputResultsQuadratic( const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo* pParentIntersectionsInfo );
	static void ConsensusLociIntersectionAndOutputResultsHybrid( const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo& parentIntersectionsInfo );

	// @return pivot index for loci partitioning
	static inline PkUInt getPivotIndex( const PkUInt numElements )
	{
		// Split count in half, give remainder to left half
		return ((numElements>>1)/*divide by 2*/+(numElements&0x1)/*mod 2*/);
	}

	// @return the summation of partition sizes
	static inline PkUInt accumulateNumIntersections( const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions )
	{
		PkUInt sum = 0;
		PkAssert( lociMask.size() == lociPartitions.size() );
		for ( PkBitSet::size_type itrLoci = lociMask.find_first(); itrLoci != PkBitSet::npos; itrLoci = lociMask.find_next( itrLoci ) )
		{
			PkAssert( lociPartitions[ itrLoci ] );
			sum += lociPartitions[ itrLoci ]->size();
		}
		return sum;
	}

	// @return true if recursive consensus loci intersection should recurse down the left child tree, false otherwise
	static PkBool shouldRecurseDownLeftTree( const PkBitSet& leftLociMask, const PkBitSet& rightLociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions )
	{
		// Determine number of sibling sets in left half
		const PkUInt leftSum = accumulateNumIntersections( leftLociMask, lociPartitions );

		// Determine number of sibling sets in right half
		const PkUInt rightSum = accumulateNumIntersections( rightLociMask, lociPartitions );
		
		// For now, assuming input was pre-sorted in descending order and therefore leftSum > rightSum
		PkAssert( leftSum >= rightSum );

		// Assume no divide by 0!
		PkAssert( leftSum > 0 );
		const double disparity = (double)( leftSum - rightSum ) / (double) leftSum;

		// See if disparity is within threshhold
		return disparity < Pk_CONSENSUS_COMPUTE_LEFT_INTERSECTIONS_TREE_THRESH;
	}

	// Will compute the child intersections for the parameter loci mask and append them to the parameter parent intersection
	// In addition, will cache intermediate intersections if the cache pointer is non-null
	static void	computeChildIntersections( PkIntersectionsInfo& outChildIntersectionsInfo, PkArray( PkIntersectionsInfo )* pOutIntermediateIntersectionsInfosCache, const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo* pParentIntersectionsInfo )
	{
		PkLogf( "\tAppending intersections for: %s\n", PkStringOf( lociMask ).c_str() );
		
		// Verify assumptions:
		// Make sure there's no overlap between parent intersections and the child intersections we're computing
		PkAssert( lociMask.size() == lociPartitions.size() );
		PkAssert( (NULL == pParentIntersectionsInfo) || (lociMask.size() == pParentIntersectionsInfo->mask.size()) );
		PkAssert( (NULL == pParentIntersectionsInfo) || !(lociMask & pParentIntersectionsInfo->mask).any() );

		// A mask for keeping track of intermediate intersections
		PkBitSet intermediateIntersectionsMask( lociPartitions.size() );
		if ( pParentIntersectionsInfo )
		{
			intermediateIntersectionsMask = pParentIntersectionsInfo->mask;
		}

		// Some branching to handle case where intersections array is empty (initially it is)
		PkBitSet::size_type itrLoci = lociMask.find_first();
		if ( (NULL == pParentIntersectionsInfo) || pParentIntersectionsInfo->intersections.empty() )
		{
			PkAssert( itrLoci != PkBitSet::npos );
			outChildIntersectionsInfo.intersections = *lociPartitions[ itrLoci  ];
			intermediateIntersectionsMask.set( itrLoci, true );
			itrLoci = lociMask.find_next( itrLoci );
		}
		else
		{
			outChildIntersectionsInfo.intersections = pParentIntersectionsInfo->intersections;
		}

		// Intersect across the loci
		while ( itrLoci != PkBitSet::npos )
		{
			// Update intermediate intersections mask
			intermediateIntersectionsMask.set( itrLoci, true );

			// Defer to utility function to perform intersection
			Pk::IntersectSiblingSets( outChildIntersectionsInfo.intersections, *lociPartitions[ itrLoci ] );

			// Compute next loci index
			const PkBitSet::size_type nextItrLoci = lociMask.find_next( itrLoci );
			
			// Check to see if we should cache intermediate intersections
			if ( pOutIntermediateIntersectionsInfosCache && ( nextItrLoci != PkBitSet::npos ) )
			{
				pOutIntermediateIntersectionsInfosCache->push_back( PkIntersectionsInfo() );
				pOutIntermediateIntersectionsInfosCache->back().mask = intermediateIntersectionsMask;
				pOutIntermediateIntersectionsInfosCache->back().intersections = outChildIntersectionsInfo.intersections;
			}

			// Advance loci iterator
			itrLoci = nextItrLoci;
		}

		// Set the mask for final child intersections array
		outChildIntersectionsInfo.mask = intermediateIntersectionsMask;
	}

	// Performs common computations and returns true if base case was reached
	// If not base case, then right and left loci masks are calculated
	static PkBool consensusLociIntersectionBaseCase( PkBitSet& outLeftLociMask, PkBitSet& outRightLociMask, const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo& parentIntersectionsInfo )
	{
		// Get initial loci count
		const PkUInt totalLociCount = lociMask.count();
		PkAssert( totalLociCount >= 1 );

		// Base case: lociMask has a count of 1
		if ( totalLociCount == 1 )
		{
			PkAssert( parentIntersectionsInfo.mask.count() == (parentIntersectionsInfo.mask.size()-1) );
			PkAssert( lociMask.find_first() != PkBitSet::npos );
			// Output gms and non-gms results file
			PkString outputFileName;
			PkConsensus::getIntersectionOutputFileName( outputFileName, lociMask.find_first() );
			PkPhase::outputResults( outputFileName.c_str(), parentIntersectionsInfo.intersections );
			// Signify base case was reached
			return true;
		}

		// Split loci count in half, give remainder to leftmost branches
		const PkUInt leftLociCount = getPivotIndex( totalLociCount );
		
		// Determine left loci mask
		outLeftLociMask.resize( lociMask.size() );
		PkAssert( outLeftLociMask.any() == false );
		PkBitSet::size_type itrBit = lociMask.find_first();
		for ( PkUInt itrLoci=0; itrLoci<leftLociCount; ++itrLoci )
		{
			PkAssert( PkBitSet::npos != itrBit );
			outLeftLociMask.set( itrBit );
			itrBit = lociMask.find_next( itrBit );
		}

		// Determine right loci mask
		outRightLociMask = lociMask - outLeftLociMask;

		// Log left and right children
		PkLogf( "\tLeft child is: %s with count %u\n", PkStringOf( outLeftLociMask ).c_str(), leftLociCount );
		PkLogf( "\tRight child is: %s\n", PkStringOf( outRightLociMask ).c_str() );	

		// Signify base case wasn't reached
		return false;
	}

	// Utility function for recursing down a child loci tree
	static void consensusLociIntersectionRecurseTree( PkArray( PkIntersectionsInfo )* pOutIntermediateIntersectionsInfosCache, const PkBitSet& recurseMask, const PkBitSet& intersectsMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo& parentIntersectionsInfo )
	{
		// Compute intersections for child tree
		PkIntersectionsInfo childIntersectionsInfo;
		computeChildIntersections(
			  childIntersectionsInfo
			, pOutIntermediateIntersectionsInfosCache
			, intersectsMask
			, lociPartitions
			, &parentIntersectionsInfo
			);

		// Recurse down child tree
		ConsensusLociIntersectionAndOutputResultsHybrid( recurseMask, lociPartitions, childIntersectionsInfo );
	}

	// Utility function for calling a quadratic intersection routine within the hybrid algorithm
	static void consensusLociIntersectionCallQuadraticBody( PkBitSet::size_type& outItrLoci, PkBitSet& lociMaskForQuadraticIntersection, const PkBitSet& leftLociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo* pParentIntersectionsInfo )
	{
		PkAssert( leftLociMask.size() == lociPartitions.size() );
		PkAssert( lociMaskForQuadraticIntersection.size() == lociPartitions.size() );
		PkAssert( false == lociMaskForQuadraticIntersection.any() );
		lociMaskForQuadraticIntersection.set( outItrLoci, true );
		ConsensusLociIntersectionAndOutputResultsQuadratic(
			  lociMaskForQuadraticIntersection
			, lociPartitions
			, pParentIntersectionsInfo
			);
		lociMaskForQuadraticIntersection.set( outItrLoci, false );
		outItrLoci = leftLociMask.find_next( outItrLoci );
	}

	// A quadratic ("naive") algorithm for computing drop-out loci
	static void ConsensusLociIntersectionAndOutputResultsQuadratic( const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo* pParentIntersectionsInfo )
	{
		// Verify assumptions
		PkAssert( lociMask.size() == lociPartitions.size() );
		PkAssert( (NULL == pParentIntersectionsInfo) || (lociMask.size() == pParentIntersectionsInfo->mask.size()) );
		PkAssert( (NULL == pParentIntersectionsInfo) || !(lociMask & pParentIntersectionsInfo->mask).any() );

		// Create a mask for dropping out loci when computing child intersections
		PkBitSet childIntersectionsLociMask( lociPartitions.size() );
		childIntersectionsLociMask.set();

		// Don't compute any intersections we've already done
		if ( pParentIntersectionsInfo )
		{
			childIntersectionsLociMask -= pParentIntersectionsInfo->mask;
		}

		// Iterate over parameter loci mask and compute intersections for each locus dropped out
		for ( PkBitSet::size_type itrLocusToDrop = lociMask.find_first(); itrLocusToDrop != PkBitSet::npos; itrLocusToDrop = lociMask.find_next( itrLocusToDrop ) )
		{
			PkLogf( "Naive consensus intersection for locus idx %u\n", itrLocusToDrop ); 
			
			// Drop out current locus
			PkAssert( childIntersectionsLociMask[ itrLocusToDrop ] );
			childIntersectionsLociMask.set( itrLocusToDrop, false );

			// Compute all intersections with a single locus dropped out
			PkIntersectionsInfo droppedLociIntersectionsInfo;
			computeChildIntersections(
				  droppedLociIntersectionsInfo
				, NULL
				, childIntersectionsLociMask
				, lociPartitions
				, pParentIntersectionsInfo
				);

			// Verify that we intersected every locus except for the one we dropped out
			PkAssert( droppedLociIntersectionsInfo.mask.count() == (lociPartitions.size()-1) );

			// Output results of dropped locus intersections
			PkString outputFileName;
			PkConsensus::getIntersectionOutputFileName( outputFileName, itrLocusToDrop );
			PkPhase::outputResults( outputFileName.c_str(), droppedLociIntersectionsInfo.intersections );
			
			// Reset intersections mask
			childIntersectionsLociMask.set( itrLocusToDrop, true );
			PkAssert( childIntersectionsLociMask[ itrLocusToDrop ] );
		}
	}

	// Will use logarthmic tree recursion splitting for right half
	// However, will only conditionally recurse down left half, if conditional fails, then will use quadratic method
	static void ConsensusLociIntersectionAndOutputResultsHybrid( const PkBitSet& lociMask, const PkArray( const PkSiblingSetBitSetArray* )& lociPartitions, const PkIntersectionsInfo& parentIntersectionsInfo )
	{
		PkLogf( "Entering hybrid consensus intersection for loci: %s\n", PkStringOf( lociMask ).c_str() );

		// See if we reach base case and partition loci around a pivot
		PkBitSet leftLociMask, rightLociMask;
		if ( consensusLociIntersectionBaseCase( leftLociMask, rightLociMask, lociMask, lociPartitions, parentIntersectionsInfo ) )
		{
			// Base case was reached
			return;
		}

		// Determine if we will be recursing down the left tree
		const PkBool bShouldRecurseDownLeftTree = shouldRecurseDownLeftTree( leftLociMask, rightLociMask, lociPartitions );

		// A cache to store the intermediate intersections computed for the right child tree 
		// These intersections can be re-used for the quadratic intersection routine
		// Intermediates should only be cached if we're *not* recursing down the left tree
		PkArray( PkIntersectionsInfo ) intermediateIntersectionsInfoCache;

		// Recurse down right child
		consensusLociIntersectionRecurseTree(
			  // Cache intermediate intersections for re-use if we're not recursing down the left child tree
			  bShouldRecurseDownLeftTree ? NULL : &intermediateIntersectionsInfoCache
			, rightLociMask
			, leftLociMask
			, lociPartitions
			, parentIntersectionsInfo
			);
		
		// Check to see if hueristic said we should recurse left child as well
		if ( bShouldRecurseDownLeftTree )
		{
			// Recurse down left child
			consensusLociIntersectionRecurseTree( NULL, leftLociMask, rightLociMask, lociPartitions, parentIntersectionsInfo );	
		}
		else
		{
			// Disparity between left and right child tree is too large
			// Use a quadratic method for computing the left child while re-using cached intersections from initial right child call
			// Verify that we have the proper number of cached intersections
			PkAssert(
				   // The +1 is because the intermediate intersections for the first loci are actually the parent intersections
				   ( leftLociMask.count() == intermediateIntersectionsInfoCache.size() + 1 ) 
				   // Of note, if the parent intersections are empty, we are the leftmost tree and, therefore,
				   // we are missing both the cache of the first loci with itself and parent intersections for the first loci
				|| ( parentIntersectionsInfo.intersections.empty() && ( leftLociMask.count() == intermediateIntersectionsInfoCache.size() + 2 ) )
				);
			
			// Verify assumption that we have at least one loci to work with
			PkAssert( leftLociMask.count() >= 1 );

			// Verify assumption that empty parent intersections imply we're the leftmost tree
			PkAssert(
				   ( !parentIntersectionsInfo.intersections.empty() )
				|| ( parentIntersectionsInfo.intersections.empty() && leftLociMask[ 0 ] )
				);
			
			// Allocate a bit mask to feed to the quadratic intersection routine
			PkBitSet lociMaskForQuadraticIntersection( lociPartitions.size() );
			PkAssert( false == lociMaskForQuadraticIntersection.any() );

			// Iterator for left child tree loci
			PkBitSet::size_type itrLoci = leftLociMask.find_first();
			
			// Special case, for the first loci, use the parent intersections info
			if ( itrLoci != PkBitSet::npos )
			{
				consensusLociIntersectionCallQuadraticBody( 
					  itrLoci
					, lociMaskForQuadraticIntersection
					, leftLociMask
					, lociPartitions
					, &parentIntersectionsInfo
					);
			}

			// Special case, for second loci and leftmost tree, we also don't have an intermediate intersection cached
			if ( parentIntersectionsInfo.intersections.empty() )
			{
				consensusLociIntersectionCallQuadraticBody( 
					  itrLoci
					, lociMaskForQuadraticIntersection
					, leftLociMask
					, lociPartitions
					, NULL
					);
			}

			// For the remaining loci, use the intermediates cache
			PkUInt itrIntermediates = 0;
			while( itrLoci != PkBitSet::npos )
			{
				consensusLociIntersectionCallQuadraticBody( 
					  itrLoci
					, lociMaskForQuadraticIntersection
					, leftLociMask
					, lociPartitions
					, &intermediateIntersectionsInfoCache[ itrIntermediates++ ]
					);
			}

			// Verify we used all our cached intermediates, no less, no more
			PkAssert( intermediateIntersectionsInfoCache.size() == itrIntermediates );
		}
	}

} // end of unnamed namespace

namespace PkConsensus
{

// Obtains the file name for the solution with parameter dropped locus
void getIntersectionOutputFileName( PkString& outFileName, const PkUInt droppedLocus )
{
	outFileName = PkString(PkGlobalData::getOutputFileName()) + PkString("_droppedLocus") + PkStringOf( droppedLocus );
}

// Generates each single allele drop out solution.
// Assumes that a full loci decomposition was used prior
// Intersect across all loci to determine feasible sibling sets
// Computes several solutions using consensus method
void lociIntersectionAndOutputResults()
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_ConsensusLociIntersectionAndOutputResultsTime );

	// Create a mask to tell which loci are not in the intersection
	const PkUInt numLoci = PkGlobalData::getEncodedSiblingSetPartitions().size();
	PkBitSet lociMask( numLoci );
	// Flip all bits to 1
	lociMask.set();
	
	// Initial intersections array is empty
	PkSiblingSetBitSetArray intersections;

	// Create indirect array of loci partitions for sorting
	PkArray( const PkSiblingSetBitSetArray* ) lociPartitions( numLoci );
	for ( PkUInt itrLocus=0; itrLocus<numLoci; ++itrLocus )
	{
		lociPartitions[ itrLocus ] = &(PkGlobalData::getEncodedSiblingSetPartitions()[itrLocus]);
	}

	// Sort partitions from largest to smallest to avoid multiplicative overhead
	std::sort( lociPartitions.begin(), lociPartitions.end(), PkBitSetBridgeUtils::IndirectLociSortingPredicate<PkSiblingSetBitSetArray>() );

	// If naive consensus intersection is enabled, do all the redundant work we've worked so hard to avoid!
#if Pk_ENABLE_NAIVE_CONSENSUS_INTERSECTION
	ConsensusLociIntersectionAndOutputResultsQuadratic( lociMask, lociPartitions, NULL );
#else
	PkIntersectionsInfo parentIntersectionsInfo;
	parentIntersectionsInfo.mask.resize( lociPartitions.size() );
	PkAssert( false == parentIntersectionsInfo.mask.any() );
	ConsensusLociIntersectionAndOutputResultsHybrid( lociMask, lociPartitions, parentIntersectionsInfo );
#endif
}

} // end of PkConsensus namespace
