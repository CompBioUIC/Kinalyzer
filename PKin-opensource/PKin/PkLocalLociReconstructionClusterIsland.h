/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkLocalLociReconstructionClusterIsland_h
#define PkLocalLociReconstructionClusterIsland_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLocalLociReconstructionClusterInterface.h"
#include "PkLocalSiblingGroupReconstructor.h"
#include "PkSiblingGroupCommandBufferIsland.h"

// Reconstructs possible sibling relationships using no inter-thread communication
class PkLocalLociReconstructionClusterIsland : public PkLocalLociReconstructionClusterInterface
{
public:

	// Our default constructor
	PkLocalLociReconstructionClusterIsland() {}

	// Queues jobs to reconstruct possible sibling groups
	virtual void queue( 
		  const PkLociArray(PkInt)& lociIndices
		, const PkUInt numExeUnits 
		);

	// Will block calling thread until all execution units of the cluster finish
	// The results are copied into the parameter array
	virtual void blockUntilFinished( PkSiblingSetBitSetArray& outEncodedSiblingSets );

	// Destructor which frees any internal dynamic memory
	virtual ~PkLocalLociReconstructionClusterIsland();

private:
	
	// Disallow copy and assignment as our destructor has nasty effects
	PkLocalLociReconstructionClusterIsland( const PkLocalLociReconstructionClusterIsland& );
	PkLocalLociReconstructionClusterIsland& operator=( const PkLocalLociReconstructionClusterIsland& );

	typedef PkArray( PkSiblingGroupCommandBufferIsland ) tSiblingGroupCommandBufferArray;

	// @return reference to our command buffer
	inline tSiblingGroupCommandBufferArray& getCommandBuffers()
	{
		return m_CommandBuffers;
	}

	// @return reference to our collection of reconstruction jobs
	inline PkArray( PkLocalSiblingGroupReconstructor )& getReconstructionJobs()
	{
		return m_ReconstructionJobs;
	}

	// @return reference to our collection of job blockers
	inline PkArray( class PkBlockableRunnable* )& getJobBlockers()
	{
		return m_JobBlockers;
	}

	// Our single, centralized command buffer
	tSiblingGroupCommandBufferArray m_CommandBuffers;

	// Our jobs which reconstruct sibling relationships on ideally seperate execution units
	PkArray( PkLocalSiblingGroupReconstructor ) m_ReconstructionJobs;

	// Our blockers which tell us when all of our jobs have finished
	PkArray( class PkBlockableRunnable* ) m_JobBlockers;
};

#endif // PkLocalLociReconstructionClusterIsland_h
