/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroupCommandBufferInterface_h
#define PkSiblingGroupCommandBufferInterface_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLociArray.h"

// An interface for a sibling group command buffer
class PkSiblingGroupCommandBufferInterface
{
public:

	// Our virtual destructor for proper routing of child destructors
	virtual ~PkSiblingGroupCommandBufferInterface() {}

	// Per-job initialization can be overloaded here
	virtual void localInit( const PkLociArray( PkInt )& lociIndices ) {}

	// Obtains a command from the command buffer
	// @return true if command exists, false otherwise
	virtual PkBool getNextCommand( class PkSiblingGroup& outCommand ) = 0;

	// Pushes a command onto the command buffer
	virtual void pushCommand( class PkSiblingGroup& inCommand ) = 0;

	// Meant to be called when a command has finished processing
	virtual void finalizeCommand( const class PkSiblingGroup& inCommand ) {};
};

#endif // PkSiblingGroupCommandBufferInterface_h
