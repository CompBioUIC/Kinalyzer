/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroupCommandBufferCentralized_h
#define PkSiblingGroupCommandBufferCentralized_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkSiblingGroupCommandBufferInterface.h"
#include "PkCriticalSection.h"
#include "PkScopeLock.h"
#include "PkSiblingGroupCommandBufferIsland.h"
#include "PkSiblingGroup.h"

// A simple multi-threaded work queue, can act as a single, centralized command buffer
class PkSiblingGroupCommandBufferCentralized : public PkSiblingGroupCommandBufferInterface
{
public:

	// Obtains a command from the command buffer
	// @return true if command exists, false otherwise
	virtual PkBool getNextCommand( PkSiblingGroup& outCommand )
	{
		// Lock our internal command buffer
		Pk_SCOPE_LOCK( m_InternalCommandBufferSynch );

		// See if we can get a command from internal island command buffer
		return m_IslandCommandBuffer.getNextCommand( outCommand );
	}

	// Pushes a command onto the command buffer
	virtual void pushCommand( PkSiblingGroup& inCommand )
	{
		// Lock our internal command buffer
		Pk_SCOPE_LOCK( m_InternalCommandBufferSynch );

		// Push command into internal island command buffer
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

private:

	// A critical section for protecting access to the internal command buffer
	PkCriticalSection m_InternalCommandBufferSynch;

	// We are implemented in terms of an island command buffer
	PkSiblingGroupCommandBufferIsland m_IslandCommandBuffer;
};

#endif // PkSiblingGroupCommandBufferCentralized_h
