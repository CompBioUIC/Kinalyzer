/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkRunnable_h
#define PkRunnable_h

#include "PkBuild.h"
#include "PkTypes.h"

// Interface for a job that runs on a thread
class PkRunnable
{
public:

	// Constructor which defaults to unmanaged
	PkRunnable() : m_bIsManaged( false ) {}

	// Constructor, sets whether runnable allocation is managed by thread pool system or not
	explicit PkRunnable( const PkBool bIsManaged ) : m_bIsManaged( bIsManaged ) {}

	// Virtual destructor to allow proper routing of destructor for child classes
	virtual ~PkRunnable() {}

	// Overload this to specify what work the job should do
	virtual PkBool doWork( const PkUInt threadId ) = 0;

	// Overload this to handle early termination of a job. 
	// Child classes are responsible for making this thread safe
	// as doWork() and abort() are likely to be called on different threads
	virtual void abort() {};

	// @return true if runnable is to be auto-deleted by thread pool system, false otherwise
	inline PkBool isManaged() const { return m_bIsManaged; }

private:

	// When true, runnable is auto-deleted by thread pool system
	PkBool m_bIsManaged;
};

#endif // PkRunnable_h
