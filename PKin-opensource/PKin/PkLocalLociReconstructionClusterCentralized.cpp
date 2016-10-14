/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
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
#include "PkLocalLociReconstructionClusterCentralized.h"
#include "PkBitSetUtils.h"

// Destructor which frees any internal dynamic memory
PkLocalLociReconstructionClusterCentralized::~PkLocalLociReconstructionClusterCentralized()
{
	// Release our dynamically allocated job blockers
	for ( PkInt i=this->getJobBlockers().size()-1; i>=0; --i )
	{
		delete this->getJobBlockers()[ i ];
		this->getJobBlockers()[ i ] = NULL;
	}
	this->getJobBlockers().clear();
}

// Queues jobs to reconstruct possible sibling groups
void PkLocalLociReconstructionClusterCentralized::queue( 
  const PkLociArray(PkInt)& lociIndices
, const PkUInt numExeUnits 
)
{
	// Validate parameters
	PkAssert( lociIndices.size() > 0 );
	PkAssert( numExeUnits > 0 );

	// Cache global population size and reference to population
	const PkArray(PkIndividual)& population = PkGlobalData::getPopulation(); 
	const PkInt populationSize = (PkInt)population.size();
	PkAssert( populationSize > 0 );

	// Seed the command buffer with singleton sets
	for ( PkInt i=0; i<populationSize; ++i )
	{
		// Create a singleton group for current individual
		PkSiblingGroup singletonGroup(
			  population[ i ]
			, i
			, lociIndices
			, populationSize 
			);

		// Append singleton group to command buffer
		this->getCommandBuffer().pushCommand( singletonGroup );
	}

	// Validate that we're working with empty arrays, else unexpected stuff may occur
	PkAssert( this->getReconstructionJobs().size() == 0 && "Expecting empty jobs array!" );
	PkAssert( this->getJobBlockers().size() == 0 && "Expecting empty job blockers array!" );
	
	// Cache a reference to our global thread pool
	PkThreadPool& threadPool = PkGlobalData::getThreadPool();
	
	// Presize our job and job blocker arrays
	this->getReconstructionJobs().resize( numExeUnits );
	this->getJobBlockers().resize( numExeUnits, NULL );

	// Create a sibling reconstructor and job blocker for every execution unit and kick them off
	for ( PkUInt i=0; i<numExeUnits; ++i )
	{
		// Initialize the reconstruction job with the loci and command buffer to work with
		this->getReconstructionJobs()[ i ].init( lociIndices, this->getCommandBuffer() );
		
		// Allocate a job blocker so we can wait on the reconstruction job
		this->getJobBlockers()[ i ] = new PkBlockableRunnable( this->getReconstructionJobs()[ i ] );
		PkAssert( this->getJobBlockers()[ i ] );
		
		// Kick-off parallel possible sibling reconstruction
		threadPool.queueJob( *(this->getJobBlockers()[ i ]) );
	}
}

// Will block calling thread until all execution units of the cluster finish
// The results are copied into the parameter array
void PkLocalLociReconstructionClusterCentralized::blockUntilFinished( PkSiblingSetBitSetArray& outEncodedSiblingSets )
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
