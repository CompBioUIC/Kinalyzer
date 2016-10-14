/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkPhases.h"
#include "PkGlobalData.h"
#include "PkTypes.h"
#include "PkLocalLociReconstructionClusterCentralized.h"
#include "PkLocalLociReconstructionClusterIsland.h"
#include "PkLocalLociReconstructionClusterWorkStealing.h"
#include "PkStats.h"

namespace
{
	// Simple utility function that allocates a pre-sized array of cluster interfaces to a concrete type 
	template < typename tClusterPolicy >
	void allocateClustersArray( PkArray( PkLocalLociReconstructionClusterInterface* )& outClusters )
	{
		for ( PkInt iCluster=(PkInt)outClusters.size()-1; iCluster>=0; --iCluster )
		{
			PkAssert( NULL == outClusters[ iCluster ] );
			outClusters[ iCluster ] = new tClusterPolicy();
			PkAssert( outClusters[ iCluster ] );
		}
	}
}

namespace PkPhase
{

// Reconstruct possible sibling groups locally at each locus
void localLociReconstruction()
{
	PkLogf( "Beginning phase local loci reconstruction...\n" );
	Pk_SCOPED_STAT_TIMER( ePkSTAT_PhaseLocalLociReconstructionTime );

	// Store number of loci clusters demanded from user input
	const PkInt numLociClusters = PkGlobalData::getNumLociClusters();
	PkAssert( numLociClusters > 0 );

	// Declare a pre-sized clusters array
	PkArray( PkLocalLociReconstructionClusterInterface* ) clusters( numLociClusters, (PkLocalLociReconstructionClusterInterface*)NULL );

	// Allocate each cluster with user-specified concrete type
	switch( PkGlobalData::getLocalLociReconstructionAlgorithm() )
	{
	case EPkReconAlg_Centralized:
		// All group construction uses a single, centralized command buffer
		allocateClustersArray< PkLocalLociReconstructionClusterCentralized >( clusters );
		break;
	case EPkReconAlg_Island:
		// All group construction happens on its own 'island' - no inter-thread communication
		allocateClustersArray< PkLocalLociReconstructionClusterIsland >( clusters );
		break;
	case EPkReconAlg_WorkStealing:
		// All group construction uses multiple, de-centralized command buffers which can steal work from each other
		allocateClustersArray< PkLocalLociReconstructionClusterWorkStealing >( clusters );
		break;
	default:
		PkAssert( false && "Unrecognized reconstruction algorithm!" );
		break;
	};

	// Determine number of loci to assign to each cluster
	const PkInt numLoci = PkGlobalData::getNumLoci();
	PkAssert( numLoci > 0 );
	const PkInt numLociPerCluster = numLoci / numLociClusters;
	PkAssert( numLociPerCluster > 0 );
	PkInt numLociPerClusterRemainder = numLoci % numLociClusters;

	// Determine number of execution units to assign to each cluster
	const PkInt numThreads = PkGlobalData::getNumLogicalThreadsLocalLociReconstruction();
	const PkInt numExeUnitsPerCluster =  numThreads / numLociClusters;
	PkAssert( numExeUnitsPerCluster > 0 );
	PkInt numExeUnitsPerClusterRemainder = numThreads % numLociClusters;

	// Keeps track of loci indices which we have assigned
	PkInt currentLociIndex = numLoci - 1;
	PkLociArray( PkInt ) currentLociIndices;
	currentLociIndices.reserve( numLociPerCluster + ( numLociPerClusterRemainder > 0 ) );

	// Assign work to each cluster
	for ( PkInt iCluster=(PkInt)clusters.size()-1; iCluster>=0; --iCluster )
	{
		// Generate the loci index array
		currentLociIndices.resize( numLociPerCluster + ( numLociPerClusterRemainder-- > 0 ) );
		for ( PkInt iLocus=(PkInt)currentLociIndices.size()-1; iLocus>=0; --iLocus )
		{
			PkAssert( currentLociIndex < numLoci );
			PkAssert( currentLociIndex >= 0 );
			currentLociIndices[ iLocus ] = currentLociIndex--;
		}

		// Compute the actual number of execution units to assign to this cluster
		const PkInt actualNumExeUnits = numExeUnitsPerCluster + ( numExeUnitsPerClusterRemainder-- > 0 );

		// Queue the work
		PkAssert( clusters[ iCluster ] );
		clusters[ iCluster ]->queue( currentLociIndices, actualNumExeUnits );
	}

	// Assert all loci were assigned
	PkAssert( currentLociIndex == -1 );

	// Allocate slots to ouput encoded sibling groups
	PkAssert( 0 == PkGlobalData::getEncodedSiblingSetPartitions().size() );
	PkGlobalData::getEncodedSiblingSetPartitions().resize( numLociClusters );

	// Simulate a barrier.  Block until all clusters finish.
	for ( PkInt iCluster=(PkInt)clusters.size()-1; iCluster>=0; --iCluster )
	{
		PkAssert( clusters[ iCluster ] );
		PkBitSetBridgeUtils::reinit(
			  PkGlobalData::getEncodedSiblingSetPartitions()[ iCluster ] /* store */
			, PkGlobalData::getPopulation().size() /* num_bits */
			);
		clusters[ iCluster ]->blockUntilFinished( PkGlobalData::getEncodedSiblingSetPartitions()[ iCluster ] );	
	}

	// Clean up our dynamically allocated cluster interfaces
	// Note: this is done in a separate loop as implementations such as work stealing
	// may still be querying finished clusters for jobs
	for ( PkInt iCluster=(PkInt)clusters.size()-1; iCluster>=0; --iCluster )
	{
		PkAssert( clusters[ iCluster ] );
		delete clusters[ iCluster ];
		clusters[ iCluster ] = NULL;
	}
	clusters.clear();

	PkLogf( "Local loci reconstruction phase finished\n" );
}

} // end of PkPhase namespace
