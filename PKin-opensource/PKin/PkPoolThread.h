/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPoolThread_h
#define PkPoolThread_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkPlatformThreadPolicy.h"
#include "PkEvent.h"
#include "PkCriticalSection.h"

class PkPoolThread
{
public:
	
	// Constructor which creates a new thread with the parameter stack size
	// and managed by the parameter pool
	PkPoolThread( class PkThreadPool& owningPool, const PkUInt stackSizeBytes, const PkUInt threadId );

	// Destructor, will kill the thread
	~PkPoolThread();

	// Assigns a job to this thread
	void doWork( class PkRunnable& job );

	// The thread entry point, analagous to process main
	void threadMain();

private:

	// Disallow default constructor, copy constructor, and assignment operator
	PkPoolThread();
	PkPoolThread( const PkPoolThread& );
	PkPoolThread& operator=( const PkPoolThread& );

	// @return true if thread should exit, false otherwise
	inline PkBool shouldExit() const { return m_bShouldExit; }

	// The owning thread pool
	class PkThreadPool& m_OwningThreadPool;

	// The job this thread is currently doing, NULL otherwise
	// This pointer is set and read from different threads, hence its volatility
	class PkRunnable* volatile m_pJob;

	// Whether or not our internal job pointer is managed, this must
	// be copied from the job in order to allow it to be deleted by external
	// processes upon finishing.  Furthermore, this value is set and read
	// from different threads - hence, the volatility
	volatile PkBool m_bJobIsManaged;

	// If true, the thread is being killed and should not do any more work
	volatile PkBool m_bShouldExit;

	// Event to wake up thread because a job is available
	PkEvent m_WorkAvailableEvent;

	// A critical section to synchronize access to the thread job
	PkCriticalSection m_JobSynch;

	// A unique identifier for this thread
	PkUInt m_ThreadId;

	// Platform-specific data members
	struct PkPlatformThreadPolicy::tPlatformData m_PlatformData;
};

#endif // PkPoolThread_h
