/**
 * @author Alan Perez-Rathke 
 *
 * @date March 09, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkThreadPool.h"
#include "PkAssert.h"
#include "PkCriticalSection.h"
#include "PkPoolThread.h"
#include "PkScopeLock.h"
#include "PkRunnable.h"

// Default constructor
PkThreadPool::PkThreadPool()
: m_bIsInitialized( false )
{
}

// A constructor which simply forwards to initialize()
PkThreadPool::PkThreadPool( const PkUInt numThreads, const PkUInt stackSizeBytes )
: m_bIsInitialized( false )
{
	this->initialize( numThreads, stackSizeBytes );
}

// Non-virtual destructor! Not meant to be inherited from! simply forwards to destroy()
PkThreadPool::~PkThreadPool()
{
	this->destroy();
}

// Initializes the pool to a parameter number of threads
// Each thread will have a stack size as determined by parameter
void PkThreadPool::initialize( const PkUInt numThreads, const PkUInt stackSizeBytes )
{
	// Verify we're not already initialized
	PkAssert( 0 == m_JobQueue.size() );
	PkAssert( 0 == m_AvailableThreads.size() );
	PkAssert( 0 == m_AllThreads.size() );
	PkAssert( !this->isInitialized() );

	// Create threads
	m_AvailableThreads.reserve( numThreads );
	m_AllThreads.reserve( numThreads );
	for ( PkUInt i=0; i<numThreads; ++i )
	{
		m_AllThreads.push_back( new PkPoolThread( *this, stackSizeBytes, i ) );
		PkAssert( m_AllThreads.back() );
	}
	m_AvailableThreads = m_AllThreads;

	// Flag that we've been initialized
	m_bIsInitialized = true;
}

void PkThreadPool::destroy()
{
	// Soft fail if we've already been destroyed
	if ( !this->isInitialized() )
	{
		return;
	}

	// First, flag de-initialization and empty any tasks that haven't started yet
	{
		Pk_SCOPE_LOCK( m_JobSynch );
		// Flag that we're no longer initialized
		m_bIsInitialized = false;
		// Empty any queued tasks
		for ( PkInt i=((PkInt)m_JobQueue.size())-1; i>=0; --i )
		{
			PkAssert( NULL != m_JobQueue[ i ] );
			// Abort the job
			m_JobQueue[ i ]->abort();
			// Determine if job is managed, if so, then we are responsible for deleting it
			if ( m_JobQueue[ i ]->isManaged() )
			{
				delete m_JobQueue[ i ];
			}
		}
		m_JobQueue.clear();
	}

	// Now, terminate all threads.  Thread pool manages its threads so it's responsible for deleting them
	for ( PkInt i=((PkInt)m_AllThreads.size())-1; i>=0; --i )
	{
		PkAssert( NULL != m_AllThreads[i] );
		// Destructor is assumed to be safe to call on a running thread.
		// The thread implementation should be able to kill itself
		delete m_AllThreads[ i ];
	}
	m_AllThreads.clear();
	
	// Since all threads have been destroyed by this point, it's safe to access
	// m_AvailableThreads because nothing else should be accessing it
	m_AvailableThreads.clear();
}

// @return true if job successfully queued for running, false otherwise
PkBool PkThreadPool::queueJob( PkRunnable& job )
{
	// Synchronize access to available threads and job queue containers
	Pk_SCOPE_LOCK( m_JobSynch );

	// Terminate early if thread pool is not available
	if ( !this->isInitialized() )
	{
		return false;
	}

	// See if any threads are free to start running the job immediately 
	if ( !m_AvailableThreads.empty() )
	{
		PkAssert( NULL != m_AvailableThreads.back() );
		PkPoolThread& poolThread = *m_AvailableThreads.back();
		m_AvailableThreads.pop_back();
		poolThread.doWork( job );
	}
	else
	{
		// No free threads, add to the job queue
		m_JobQueue.push_back( &job ); 
	}

	// Job was successfully queued
	return true;
}

// Callback for pool threads that finish their jobs.
// They either get a new job immediately, or are queued to wait for jobs.
void PkThreadPool::onJobFinished( PkPoolThread& poolThread )
{
	// Synchronize access to available threads and job queue containers
	Pk_SCOPE_LOCK( m_JobSynch );

	// Check to see if any work is available right away
	if ( !m_JobQueue.empty() )
	{
		// Prevent starvation by running oldest job in the queue
		PkAssert( NULL != m_JobQueue.front() );
		PkRunnable& job = *m_JobQueue.front();
		m_JobQueue.erase( m_JobQueue.begin() );
		poolThread.doWork( job );
	}
	else
	{
		// No work available at the moment, queue the thread
		m_AvailableThreads.push_back( &poolThread );
	}
}
