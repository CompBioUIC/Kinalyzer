/** 
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkEventBoost_h
#define PkEventBoost_h

#include "PkBuild.h"

#if Pk_ENABLE_BOOST_THREADS

#include "PkTypes.h"
#include <boost/thread.hpp>

#include "PkScopeLock.h"

/*
* Boost does not have events, the closest we have is a condition variable
*/
class PkEvent
{
public:

	// Our default constructor
	PkEvent() : m_CappedSpinCounter( 0 ) {}

	// Waits for the event to be signaled
	void block()
	{
		// The constructor of unique_lock will acquire lock of parameter mutex
		boost::unique_lock<boost::mutex> uniqueLock( m_Mutex );
		// Wait on our condition variable
		m_CappedSpinCounter = std::min<PkInt>( MAX_EVENT_SPIN_COUNT, m_CappedSpinCounter+1 );
		while ( m_CappedSpinCounter > 0 )
		{
			// wait() will internally unlock the unique lock parameter
			m_ConditionVariable.wait( uniqueLock );
		}
		// boost::unique_lock will release the mutex in its destructor if owns_lock() is true

		// HACK: block on a semaphore to gaurantee that notify() is finished!
		Pk_SCOPE_LOCK( m_EnsureNotifyIsAtomic );
	}

	// Signals a thread waiting on an event. Since events are auto-reset,
	// the event returns immediately to the non-signaled waitable state
	void notify()
	{
		// HACK: block on a semaphore to gaurantee that notify() is finished!
		Pk_SCOPE_LOCK( m_EnsureNotifyIsAtomic );

		// Wake up anyone waiting on us
		{
			boost::lock_guard<boost::mutex> lock(m_Mutex);
			m_CappedSpinCounter = std::max<PkInt>( MIN_EVENT_SPIN_COUNT, m_CappedSpinCounter-1 );
			// boost::lock_guard will release the mutex in its destructor
		}
		// Note: this function call is not atomic!!
		// Actual crash occurred when blocker was notified and then subsequently
		// destroyed this event data, assuming nothing was using it.  Later, this
		// thread resumes and notify_one() attempts to finish and seg faults. To
		// prevent this, added a scope lock m_EnsureNotifyIsAtomic to make sure
		// notify_one is finished before blockers can resume.
		m_ConditionVariable.notify_one();	
	}

private:

	// The range of allowed spin count values
	enum { MIN_EVENT_SPIN_COUNT = -1, MAX_EVENT_SPIN_COUNT = 1 };

	// Explicitly disallow copy and assignment consistent with other platforms
	PkEvent( const PkEvent& );
	PkEvent& operator=( const PkEvent& );

	// A capped spin counter between [MIN_EVENT_SPIN_COUNT, MAX_EVENT_SPIN_COUNT] 
	// to simulate Windows event semantics
	PkInt m_CappedSpinCounter;
	// Our boost condition variable
	boost::condition_variable m_ConditionVariable;
	// Our boost mutex which must be locked when passed to the condition variable
	boost::mutex m_Mutex;

	// Critical section used to ensure that notify() operations are atomic
	PkCriticalSection m_EnsureNotifyIsAtomic;
};

#endif // Pk_ENABLE_BOOST_THREADS

#endif // PkEventBoost_h
