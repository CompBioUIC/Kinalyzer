/**
 * @author Alan Perez-Rathke 
 *
 * @date March 12, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkBlockableRunnable_h
#define PkBlockableRunnable_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkEvent.h"
#include "PkRunnable.h"

// A wrapper runnable that a thread may block on until job is finished
// Sort of a 'decorator pattern' I think...
class PkBlockableRunnable : public PkRunnable
{
public:

	// Constructor, wraps a job with an event that we can block on
	// We are not managed as someone else is probably blocking on us
	PkBlockableRunnable( PkRunnable& jobToBlockOn ) 
	: PkRunnable( false /* bIsManaged */ )
	, m_bIsJobToBlockOnManaged( jobToBlockOn.isManaged() )
	, m_JobToBlockOn( jobToBlockOn )
	{}

	// Our destructor
	virtual ~PkBlockableRunnable()
	{
		// Clean up job if it's managed
		if ( m_bIsJobToBlockOnManaged )
		{
			delete &m_JobToBlockOn;
		}
	}

	// Overload this to specify what work the job should do
	virtual PkBool doWork( const PkUInt threadId )
	{
		// Run the job that everyone is waiting for!
		const PkBool retVal = m_JobToBlockOn.doWork( threadId );
		// Job finished, wake up whomever was blocking on us
		m_FinishedEvent.notify();
		// Note: managed jobs are cleaned in the destructor
		// Return result of job
		return retVal;
	}

	// Overload this to handle early termination of a job. 
	// Child classes are responsible for making this thread safe
	// as doWork() and abort() are likely to be called on different threads
	virtual void abort() 
	{
		// Forward to our internal job
		if ( m_bIsJobToBlockOnManaged )
		{
			m_JobToBlockOn.abort();
		}
	};

	// Causes calling thread to block until internal job has finished its work
	void blockUntilFinished()
	{
		m_FinishedEvent.block();
	}

private:

	// Forbid copy and assignment as our destructor can have nasty side effects
	PkBlockableRunnable( const PkBlockableRunnable& );
	PkBlockableRunnable& operator=( const PkBlockableRunnable& );

	// Copy of whether or not internal job is managed
	PkBool m_bIsJobToBlockOnManaged;

	// An event which is triggered when the internal runnable finishes
	PkEvent m_FinishedEvent;

	// The actual job that we want to run and block on until finished
	PkRunnable& m_JobToBlockOn;
};

#endif // PkBlockableRunnable_h
