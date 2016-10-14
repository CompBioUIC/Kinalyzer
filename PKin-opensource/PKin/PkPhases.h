/**
 * @author Alan Perez-Rathke 
 *
 * @date March 15, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPhases_h
#define PkPhases_h

#include "PkBuild.h"
#include "PkTypes.h"

// Enumerated local loci reconstruction algorithms
enum EPkLocalLociReconstructionAlgorithm
{
	// The min value of algorithms available
	  EPkReconAlg_MinVal = 0
	// All group construction uses a single, centralized command buffer
	, EPkReconAlg_Centralized = EPkReconAlg_MinVal
	// All group construction happens on its own 'island' - no inter-thread communication
	, EPkReconAlg_Island
	// All group construction uses multiple, de-centralized command buffers which can steal work from each other
	, EPkReconAlg_WorkStealing
	// The number of algorithms available
	, EPkReconAlg_NumAlgorithms
};

// The main application organized into sequential phases
namespace PkPhase
{
	// Reconstruct possible sibling groups locally at each locus
	extern void localLociReconstruction();

	// Intersect across all loci to determine feasible sibling sets
	extern void lociIntersection();

	// Writes output to data file
	extern void outputResults( const char* strOutFileName, const PkSiblingSetBitSetArray& outputSets );
}

#endif // PkIndividual_h
