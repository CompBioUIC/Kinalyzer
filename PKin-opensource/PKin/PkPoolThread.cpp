/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkPoolThread.h"
#include "PkRunnable.h"
#include "PkEvent.h"
#include "PkCriticalSection.h"
#include "PkScopeLock.h"
#include "PkThreadPool.h"
#include "PkPlatformThreadPolicy.h"

// Constructor which creates a new thread with the parameter stack size
// and managed by the parameter pool
PkPoolThread::PkPoolThread( PkThreadPool& owningPool, const PkUInt stackSizeBytes, const PkUInt threadId )
: m_OwningThreadPool( owningPool )
, m_pJob( NULL )
, m_bShouldExit( false )
, m_bJobIsManaged( false )
, m_ThreadId( threadId )
{
	// Create the platform thread
	PkPlatformThreadPolicy::create( stackSizeBytes, *this, m_PlatformData );
}

// Destructor, will kill the thread
PkPoolThread::~PkPoolThread()
{
	// Signal that we are exiting
	m_bShouldExit = true;

	// Synchronize access to job pointer to avoid it being NULL'ed
	// from another thread before we use it
	{
		Pk_SCOPE_LOCK( m_JobSynch );

		// Abort any outstanding jobs, only safe to call if job is managed
		if ( m_pJob && m_bJobIsManaged )
		{
			// This should be safe to call even if work has already completed
			m_pJob->abort();
		}
	}

	// Wake up the thread so that it finishes any outstanding processing
	m_WorkAvailableEvent.notify();

	// Kill the platform thread
	PkPlatformThreadPolicy::kill( m_PlatformData ); 
	
	// Free job memory if it's managed
	if 
	( 
	   m_pJob 
	&& m_bJobIsManaged
	)
	{
		delete m_pJob;
	}
	m_pJob = NULL;
}

// The thread entry point, analagous to process main
void PkPoolThread::threadMain()
{
	while ( !this->shouldExit() )
	{
		// Wait for a signal that work is available (or we are exiting)
		m_WorkAvailableEvent.block();

		// Check if a job is available
		if ( m_pJob != NULL )
		{
			// Job found, perform it
			m_pJob->doWork( m_ThreadId );
			
			// Synchronize access to job pointer modifications in case 
			// we're actually aborting it from a different thread
			// Notice how we're *not* locking during doWork() because we
			// want to be able to abort() during doWork().
			{
				Pk_SCOPE_LOCK( m_JobSynch );

				// If the job is managed, we are the owners and need to destroy it
				if ( m_bJobIsManaged )
				{
					delete m_pJob;
				}
			
				// Reset job to NULL to signal we have no work to do
				m_pJob = NULL;
			}

			// Signal thread pool that we're available for another job
			m_OwningThreadPool.onJobFinished( *this );
		}
	}
}

// Assigns new job to this thread
void PkPoolThread::doWork( PkRunnable& job )
{
	PkAssert( NULL == m_pJob );

	// Don't assign this job if we're exiting
	if ( !this->shouldExit() )
	{
		// Synchronize access to job in case we're actually aborting it from a different thread
		{
			Pk_SCOPE_LOCK( m_JobSynch ); 

			// Copy the manage flag, necessary to gaurantee the ability for other threads
			// to delete the job from underneath once pJob->doWork() completes
			m_bJobIsManaged = job.isManaged();

			// Assign a new job to this thread
			m_pJob = &job;
		}
	
		// Wake up thread so it can start doing its job
		m_WorkAvailableEvent.notify();
	}
}
