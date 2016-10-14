/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkLocalLociReconstructionClusterInterface_h
#define PkLocalLociReconstructionClusterInterface_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLociArray.h"

// Interface for a collection of execution units to locally reconstruct
// sibling groups based on a set of loci
class PkLocalLociReconstructionClusterInterface
{
public:

	// Virtual destructor for proper routing to child classes
	virtual ~PkLocalLociReconstructionClusterInterface() {}

	// Queues jobs to reconstruct possible sibling groups
	virtual void queue( 
		  const PkLociArray(PkInt)& lociIndices
		, const PkUInt numExeUnits 
		) = 0;

	// Will block calling thread until all execution units of the cluster finish
	// The results are copied into the parameter array
	virtual void blockUntilFinished( PkSiblingSetBitSetArray& outEncodedSiblingSets ) = 0;
};

#endif // PkLocalLociReconstructionClusterInterface_h
