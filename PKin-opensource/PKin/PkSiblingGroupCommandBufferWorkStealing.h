/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroupCommandBufferWorkStealing_h
#define PkSiblingGroupCommandBufferWorkStealing_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkSiblingGroupCommandBufferInterface.h"
#include "PkCriticalSection.h"
#include "PkScopeLock.h"
#include "PkSiblingGroup.h"
#include "PkRand.h"
#include "PkGlobalData.h"
#include "PkSiblingGroupCommandBufferIsland.h"

// A simple implementation of work stealing
class PkSiblingGroupCommandBufferWorkStealing : public PkSiblingGroupCommandBufferInterface
{
public:

	// Default constructor
	PkSiblingGroupCommandBufferWorkStealing() 
	: m_pGlobalCommandBuffers( NULL )
	{}

	// Initializes a pointer to the parameter command buffers
	// This memory is not managed!  The parameter is expected to live as long as this command buffer itself
	inline void setGlobalCommandBuffers( PkArray( PkSiblingGroupCommandBufferWorkStealing* )& globalCommandBuffers )
	{
		m_pGlobalCommandBuffers = &globalCommandBuffers;
	}

	// Obtains a command from the command buffer
	// @return true if command exists, false otherwise
	virtual PkBool getNextCommand( PkSiblingGroup& outCommand )
	{
		// Steal work from ourself before stealing work from anyone else
		PkInt bCommandExists = this->stealCommand( outCommand );

		// We have no work! Try stealing work from a random command buffer.
		if ( !bCommandExists )
		{
			// Cache number of global buffers we're working with 
			const PkInt numGlobalCommandBuffers = this->getGlobalCommandBuffers().size();

			// Cap the number of attempts as function of the number of global command buffers
			// Note: subtracting 1 from count ensures that we don't infinite loop if we're
			// the only command buffer actually allocated
			const PkInt maxAttempts = std::max<PkInt>( 0, numGlobalCommandBuffers-1 ) * 4 / 3;

			// Keep track of the number of attempts
			PkInt numAttempts = 0;

			// Iterate over external command buffers and attempt to steal work
			while ( numAttempts++ < maxAttempts )
			{
				PkSiblingGroupCommandBufferWorkStealing* pGlobalCommandBuffer = NULL;
				// Generate a random command buffer index
				PkInt idxGlobalCommandBuffer = -1;
				do
				{
					idxGlobalCommandBuffer = PkRand( 0, numGlobalCommandBuffers );
					// Assert on bounds check 
					PkAssert( idxGlobalCommandBuffer >= 0 );
					PkAssert( idxGlobalCommandBuffer < numGlobalCommandBuffers );
					pGlobalCommandBuffer = this->getGlobalCommandBuffers()[ idxGlobalCommandBuffer ];
				}
				// Ensure that we don't generate our own index
				while ( this == pGlobalCommandBuffer );
				
				// Steal work from random global command buffer
				PkAssert( NULL != pGlobalCommandBuffer );
				if ( bCommandExists = pGlobalCommandBuffer->stealCommand( outCommand ) )
				{
					// Work found, break out of loop
					break;
				}
			}
		}

		// Return whether or not a command exists
		return bCommandExists;
	}

	// Pushes a command onto the command buffer
	virtual void pushCommand( PkSiblingGroup& inCommand )
	{
		// Lock our internal command buffer
		Pk_SCOPE_LOCK( m_InternalCommandBufferSynch );

		// Redirect to our island command buffer implementation
		m_IslandCommandBuffer.pushCommand( inCommand );
	}

	// Meant to be called when a command has finished processing
	// In this case, serves the purpose of merging any added individuals
	virtual void finalizeCommand( const PkSiblingGroup& inCommand )
	{
		// Lock our internal command buffer
		Pk_SCOPE_LOCK( m_InternalCommandBufferSynch );

		// Redirect to our island command buffer implementation
		m_IslandCommandBuffer.finalizeCommand( inCommand );
	}
	
	// Per-job initialization can be overloaded here
	virtual void localInit( const PkLociArray( PkInt )& lociIndices )
	{
		// Lock our internal command buffer
		Pk_SCOPE_LOCK( m_InternalCommandBufferSynch );

		// Initialize our internal island command buffer
		m_IslandCommandBuffer.localInit( lociIndices );
	}

	// Sets range of population to create commands for during local initialization
	inline void setLocalInitPopulationRange(
			  const PkUInt idxStartIndividual
			, const PkUInt numIndividualsToAssign
			)
	{
		m_IslandCommandBuffer.setLocalInitPopulationRange( idxStartIndividual, numIndividualsToAssign );
	}

private:

	// Steals a command from the internal command buffer if available
	// @return true if command was stolen, false otherwise
	inline PkBool stealCommand( PkSiblingGroup& outCommand )
	{
		// Lock our internal command buffer
		Pk_SCOPE_LOCK( m_InternalCommandBufferSynch );

		// Attempt to obtain a command from our island command buffer
		return m_IslandCommandBuffer.getNextCommand( outCommand );
	}

	// @return reference to global command buffers array
	inline PkArray( PkSiblingGroupCommandBufferWorkStealing* )& getGlobalCommandBuffers()
	{
		PkAssert( NULL != m_pGlobalCommandBuffers );
		return *m_pGlobalCommandBuffers;
	}

	// A pointer to our global command buffers
	PkArray( PkSiblingGroupCommandBufferWorkStealing* )* m_pGlobalCommandBuffers;

	// A critical section for protecting access to the internal command buffer
	PkCriticalSection m_InternalCommandBufferSynch;

	// We are implemented in terms of a command buffer island
	PkSiblingGroupCommandBufferIsland m_IslandCommandBuffer;
};

#endif // PkSiblingGroupCommandBufferWorkStealing_h
