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
#include "PkPhases.h"
#include "PkGlobalData.h"
#include "PkParallelIntersectorUtil.h"
#include "PkMiscUtil.h"
#include "PkStats.h"

namespace PkPhase
{

// Intersect across all loci to determine feasible sibling sets
void lociIntersection()
{
	PkLogf( "Beginning phase loci intersection...\n" );

	// Cache reference to sibling set partitions
	const PkArray( PkSiblingSetBitSetArray )& encodedSiblingSetPartitions = PkGlobalData::getEncodedSiblingSetPartitions();
	
	// Store number of sibling set partitions
	const PkUInt numPartitions = encodedSiblingSetPartitions.size();
	PkAssert( numPartitions >= 1 );

	// Create indirect array of partitions for sorting
	PkArray( const PkSiblingSetBitSetArray* ) sortedPartitions( numPartitions );
	for ( PkUInt itrPartition=0; itrPartition<numPartitions; ++itrPartition )
	{
		sortedPartitions[ itrPartition ] = &(PkGlobalData::getEncodedSiblingSetPartitions()[itrPartition]);
	}

	// Sort partitions from largest to smallest to avoid multiplicative overhead
	std::sort( sortedPartitions.begin(), sortedPartitions.end(), PkBitSetBridgeUtils::IndirectLociSortingPredicate<PkSiblingSetBitSetArray>() );

	// Initialize global sibling sets to first partition
	PkGlobalData::getGlobalSiblingSets() = *sortedPartitions[ 0 ];
	
	// Intersect remaining partitions with global set to determine final global output set
	for ( PkUInt itrPartition=1; itrPartition<numPartitions; ++itrPartition )
	{
		// Log intersection round attributes
		PkLogf( "Intersection round %u of %u.\n\tIntersecting %u sets with %u sets\n"
			, itrPartition
			, numPartitions-1
			, PkGlobalData::getGlobalSiblingSets().size()
			, sortedPartitions[ itrPartition ]->size()
			);

		Pk::IntersectSiblingSets( PkGlobalData::getGlobalSiblingSets(), *sortedPartitions[ itrPartition ] );
	}

	PkLogf( "Loci intersection phase finished\n" );
}

} // end of PkPhase namespace
