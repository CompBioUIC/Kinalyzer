/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkLocalLociReconstructionClusterWorkStealing_h
#define PkLocalLociReconstructionClusterWorkStealing_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLocalLociReconstructionClusterInterface.h"
#include "PkLocalSiblingGroupReconstructor.h"
#include "PkSiblingGroupCommandBufferWorkStealing.h"

// Reconstructs possible local sibling relationships using a centralized command buffer
class PkLocalLociReconstructionClusterWorkStealing : public PkLocalLociReconstructionClusterInterface
{
public:

	// Our default constructor
	PkLocalLociReconstructionClusterWorkStealing() {}

	// Queues jobs to reconstruct possible sibling groups
	virtual void queue( 
		  const PkLociArray(PkInt)& lociIndices
		, const PkUInt numExeUnits 
		);

	// Will block calling thread until all execution units of the cluster finish
	// The results are copied into the parameter array
	virtual void blockUntilFinished( PkSiblingSetBitSetArray& outEncodedSiblingSets );

	// Destructor which frees any internal dynamic memory
	virtual ~PkLocalLociReconstructionClusterWorkStealing();

private:	

	// Disallow copy and assignment as our destructor has nasty effects
	PkLocalLociReconstructionClusterWorkStealing( const PkLocalLociReconstructionClusterWorkStealing& );
	PkLocalLociReconstructionClusterWorkStealing& operator=( const PkLocalLociReconstructionClusterWorkStealing& );

	typedef PkArray( PkSiblingGroupCommandBufferWorkStealing* ) tSiblingGroupCommandBufferArray;

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

#endif // PkLocalLociReconstructionClusterWorkStealing_h
