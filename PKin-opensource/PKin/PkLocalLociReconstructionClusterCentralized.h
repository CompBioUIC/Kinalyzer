/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkLocalLociReconstructionClusterCentralized_h
#define PkLocalLociReconstructionClusterCentralized_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLocalLociReconstructionClusterInterface.h"
#include "PkSiblingGroupCommandBufferCentralized.h"
#include "PkLocalSiblingGroupReconstructor.h"

// Reconstructs possible local sibling relationships using a centralized command buffer
class PkLocalLociReconstructionClusterCentralized : public PkLocalLociReconstructionClusterInterface
{
public:

	// Our default constructor
	PkLocalLociReconstructionClusterCentralized() {}

	// Queues jobs to reconstruct possible sibling groups
	virtual void queue( 
		  const PkLociArray(PkInt)& lociIndices
		, const PkUInt numExeUnits 
		);

	// Will block calling thread until all execution units of the cluster finish
	// The results are copied into the parameter array
	virtual void blockUntilFinished( PkSiblingSetBitSetArray& outEncodedSiblingSets );

	// Destructor which frees any internal dynamic memory
	virtual ~PkLocalLociReconstructionClusterCentralized();

private:

	// Disallow copy and assignment as our destructor has nasty effects
	PkLocalLociReconstructionClusterCentralized( const PkLocalLociReconstructionClusterCentralized& );
	PkLocalLociReconstructionClusterCentralized& operator=( const PkLocalLociReconstructionClusterCentralized& );

	// @return reference to our command buffer
	inline PkSiblingGroupCommandBufferCentralized& getCommandBuffer()
	{
		return m_CommandBuffer;
	}

	// @return reference to our collection of reconstruction jobs
	inline PkArray( PkLocalSiblingGroupReconstructor ) & getReconstructionJobs()
	{
		return m_ReconstructionJobs;
	}

	// @return reference to our collection of job blockers
	inline PkArray( class PkBlockableRunnable* )& getJobBlockers()
	{
		return m_JobBlockers;
	}

	// Our single, centralized command buffer
	PkSiblingGroupCommandBufferCentralized m_CommandBuffer;

	// Our jobs which reconstruct sibling relationships on ideally seperate execution units
	PkArray( PkLocalSiblingGroupReconstructor ) m_ReconstructionJobs;

	// Our blockers which tell us when all of our jobs have finished
	PkArray( class PkBlockableRunnable* ) m_JobBlockers;
};

#endif // PkLocalLociReconstructionClusterCentralized_h
