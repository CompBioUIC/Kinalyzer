/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkThreadPool_h
#define PkThreadPool_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkCriticalSection.h"

/**
* A simple thread pool, not meant to be inherited from
*/
class PkThreadPool
{
public:

	// Default constructor
	PkThreadPool();

	// A constructor which simply forwards to initialize()
	PkThreadPool( const PkUInt numThreads, const PkUInt stackSizeBytes );

	// Non-virtual destructor! Not meant to be inherited from! simply forwards to destroy()
	~PkThreadPool();

	// @return true if job successfully queued for running, false otherwise
	PkBool queueJob( class PkRunnable& job );

	// Callback for pool threads that finish their jobs.
	// They either get a new job immediately, or are queued to wait for jobs.
	void onJobFinished( class PkPoolThread& poolThread );

	// Initializes the pool to a parameter number of threads
	// Each thread will have a stack size as determined by parameter
	void initialize( const PkUInt numThreads, const PkUInt stackSizeBytes );

	// An optional hook for forcing clean-up of a thread pool
	void destroy();

	// @return number of threads assigned to thread pool
	inline PkUInt numThreads() const { return m_AllThreads.size(); }

private:

	// Disallow assignment and copy
	PkThreadPool( const PkThreadPool& );
	PkThreadPool& operator=( const PkThreadPool& );

	// @return true if initialized, false otherwise
	inline PkBool isInitialized() const { return m_bIsInitialized; }

	// A queue of jobs that needs to be run
	PkArray(class PkRunnable*) m_JobQueue;
	
	// The internal pool of threads which are available for assigning tasks to
	PkArray(class PkPoolThread*) m_AvailableThreads;

	// All threads that belong to pool both active and inactive
	PkArray(class PkPoolThread*) m_AllThreads;

	// A critical section for protecting access to the jobs queue
	PkCriticalSection m_JobSynch;

	// A flag for letting everyone know that this pool is initialized
	volatile PkBool m_bIsInitialized;
};

#endif // PkThreadPool_h

