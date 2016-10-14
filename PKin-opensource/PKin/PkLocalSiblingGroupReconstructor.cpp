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
#include "PkLocalSiblingGroupReconstructor.h"
#include "PkSiblingGroup.h"
#include "PkSiblingGroupCommandBufferInterface.h"
#include "PkGlobalData.h"

// Default constructor so that we can work with containers easier
PkLocalSiblingGroupReconstructor::PkLocalSiblingGroupReconstructor()
: PkRunnable( false /* bIsManaged */ )
, m_pCommandBuffer( NULL )
{}

// Constructor which initializes lociIndices and the command buffer to work with
PkLocalSiblingGroupReconstructor::PkLocalSiblingGroupReconstructor(
  const PkLociArray( PkInt )& lociIndices
, PkSiblingGroupCommandBufferInterface& commandBuffer
)
: PkRunnable( false /* bIsManaged*/ )
, m_LociIndices( lociIndices )
, m_pCommandBuffer( &commandBuffer )
{}

// Initializes lociIndices and the command buffer to work with
void PkLocalSiblingGroupReconstructor::init(
  const PkLociArray( PkInt )& lociIndices
, PkSiblingGroupCommandBufferInterface& commandBuffer
)
{
	m_LociIndices = lociIndices;
	m_pCommandBuffer = &commandBuffer;
}

// Reads commands from the command buffer until no work is available.  A command
// should just be an incomplete sibling group. Can generate new commands by 
// pushing groups that have different allele mappings.  The popped commands
// (groups) are completed by adding higher order individuals and then subsets are culled
PkBool PkLocalSiblingGroupReconstructor::doWork( const PkUInt threadId )
{
	// Cache reference to our command buffer (implementation may change to pointer in future)
	PkSiblingGroupCommandBufferInterface& commandBuffer = this->getCommandBuffer();

	// Do any per runnable initialization
	commandBuffer.localInit( this->getLociIndices() );

	// Cache global population size and reference to population
	const PkArray(PkIndividual)& population = PkGlobalData::getPopulation(); 
	const PkInt populationSize = (PkInt)population.size();

	// Loop until we run out of commands
	PkSiblingGroup currentSiblingGroup;
	while ( commandBuffer.getNextCommand( currentSiblingGroup ) )
	{
		// Attempt to add individuals to the sibling group
		for ( PkInt itrPop=0; itrPop<populationSize; ++itrPop )
		{
			// Skip processing this individual if group already has it
			if ( currentSiblingGroup.hasIndividual( itrPop ) )
			{
				continue;
			}

			// Make a copy of the original sibling group in case the possible parent set changes
			// Really don't like all these small dynamic allocations that we're causing :(
			PkSiblingGroup tempSiblingGroup = currentSiblingGroup;

			// Attempt to assign individual to sibling group
			const EPkSiblingGroupAssignmentReturnCode rc = tempSiblingGroup.conditionalAssignIndividual(
				  population[ itrPop ]
				, itrPop
				, this->getLociIndices()
				);

			// Determine course of action based on return code
			switch( rc )
			{
			// The assignment was a success, new alleles were mapped
			case ESGARC_Success_Modified:
				// Add the new group for further processing
				commandBuffer.pushCommand( tempSiblingGroup );
				break;
			
			// The assignment was a success, no new alleles were mapped
			case ESGARC_Success_NoChange:
				// Discard the temporary and force assign the individual
				currentSiblingGroup.forceAssignIndividual( itrPop );
				break;

			default:
				// The assignment failed through violation of 4-allele
				PkAssert( ESGARC_Fail == rc );
				break;
			}
		} // end iteration for addition of higher ordered individuals

		// ***************************************************************
		// Output the assigned individuals
		{
			commandBuffer.finalizeCommand( currentSiblingGroup );
			const PkSiblingGroup& constRef = currentSiblingGroup;
			this->getOutputEncodedSiblingSets().push_back(
				constRef.getAssignedIndividuals() 
				);
		}
	} // end iteration over command buffer

	// Signify we finished
	return true;
}
