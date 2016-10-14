/**
 * @author Alan Perez-Rathke 
 *
 * @date April 2, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkGlobalData.h"
#include "PkThreadPool.h"
#include "PkBlockableRunnable.h"
#include "PkLocalLociReconstructionClusterWorkStealing.h"
#include "PkBitSetUtils.h"

// Queues jobs to reconstruct possible sibling groups
void PkLocalLociReconstructionClusterWorkStealing::queue
( 
  const PkLociArray(PkInt)& lociIndices
, const PkUInt numExeUnits 
)
{
	// Validate parameters
	PkAssert( lociIndices.size() > 0 );
	PkAssert( numExeUnits > 0 );

	// Create a command buffer for each execution unit
	PkAssert( this->getCommandBuffers().size() == 0 );
	this->getCommandBuffers().resize( numExeUnits );

	// Create a sibling group reconstructor job for each command buffer
	PkAssert( this->getReconstructionJobs().size() == 0 );
	this->getReconstructionJobs().resize( this->getCommandBuffers().size() );

	// Reserve a job blocker for each sibling group reconstructor job
	PkAssert( this->getJobBlockers().size() == 0 );
	this->getJobBlockers().resize( this->getReconstructionJobs().size() );

	// Cache a reference to our global thread pool
	PkThreadPool& threadPool = PkGlobalData::getThreadPool();

	// Cache global population size
	const PkUInt populationSize = PkGlobalData::getPopulation().size();
	PkAssert( populationSize > 0 );

	// Compute how many individuals to assign to each job
	const PkUInt numIndividualsToAssign = populationSize / numExeUnits;
	PkUInt numIndividualsRemainder = populationSize % numExeUnits;

	// Initialize and kick off all the command buffers and jobs
	PkAssert( this->getCommandBuffers().size() == this->getReconstructionJobs().size() );
	PkAssert( this->getCommandBuffers().size() == this->getJobBlockers().size() );
	const PkUInt numJobs = this->getReconstructionJobs().size();
	for ( PkUInt i=0; i<numJobs; ++i )
	{
		// Allocate a new command buffer
		this->getCommandBuffers()[ i ] = new PkSiblingGroupCommandBufferWorkStealing();
		PkAssert( this->getCommandBuffers()[ i ] );

		// Assign a range of individuals for local initialization to each command buffer
		const PkUInt idxStartIndividual = i * numIndividualsToAssign;
		const PkUInt actualNumIndividualsToAssign = numIndividualsToAssign + (numIndividualsRemainder >= 1);
		numIndividualsRemainder = numIndividualsRemainder > 0 ? numIndividualsRemainder - 1 : 0;
		this->getCommandBuffers()[ i ]->setLocalInitPopulationRange(
			  idxStartIndividual
			, actualNumIndividualsToAssign
			);

		// Store global command buffers array for work stealing
		this->getCommandBuffers()[ i ]->setGlobalCommandBuffers( this->getCommandBuffers() );

		// Initialize a reconstruction job
		this->getReconstructionJobs()[ i ].init( lociIndices, *(this->getCommandBuffers()[ i ]) );

		// Create a job blocker wrapper
		this->getJobBlockers()[ i ] = new PkBlockableRunnable( this->getReconstructionJobs()[ i ] );
		PkAssert( this->getJobBlockers()[ i ] );

		// Kick-off parallel possible sibling reconstruction
		threadPool.queueJob( *(this->getJobBlockers()[ i ]) );
	}
}

// Will block calling thread until all execution units of the cluster finish
// The results are copied into the parameter array
void PkLocalLociReconstructionClusterWorkStealing::blockUntilFinished( PkSiblingSetBitSetArray& outEncodedSiblingSets )
{
	PkAssert( this->getJobBlockers().size() == this->getReconstructionJobs().size() );
	for ( PkInt i=this->getJobBlockers().size()-1; i>=0; --i )
	{
		// Wait for job to finish
		PkAssert( this->getJobBlockers()[ i ] );
		this->getJobBlockers()[ i ]->blockUntilFinished();

		// Copy the results before blocking on the next job
		const PkLocalSiblingGroupReconstructor& reconstructor = this->getReconstructionJobs()[ i ];
		for ( PkInt iSet=((PkInt)reconstructor.getOutputEncodedSiblingSets().size())-1; iSet>=0; --iSet )
		{
			// Trim any subsets
			PkBitSetUtils::conditionalAppendAndCullSubSets( reconstructor.getOutputEncodedSiblingSets()[iSet], outEncodedSiblingSets );
		}
	}
}

// Destructor which frees any internal dynamic memory
PkLocalLociReconstructionClusterWorkStealing::~PkLocalLociReconstructionClusterWorkStealing()
{
	// Release our dynamically allocated job blockers and command buffers
	PkAssert( this->getCommandBuffers().size() == this->getJobBlockers().size() );
	for ( PkInt i=this->getJobBlockers().size()-1; i>=0; --i )
	{
		// Release our job blockers
		delete this->getJobBlockers()[ i ];
		this->getJobBlockers()[ i ] = NULL;

		// Release our command buffers
		delete this->getCommandBuffers()[ i ];
		this->getCommandBuffers()[ i ] = NULL;
	}
	
	// Reset our dynamic arrays
	this->getJobBlockers().clear();
	this->getCommandBuffers().clear();
}
