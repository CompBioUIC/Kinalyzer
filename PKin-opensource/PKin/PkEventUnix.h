/** 

 * @author Alan Perez-Rathke 

 *

 * @date March 10, 2010

 *

 * Department of Computer Science

 * University of Illinois at Chicago 

 */



#ifndef PkEventUnix_h

#define PkEventUnix_h



#include "PkBuild.h"



#if Pk_ENABLE_UNIX_THREADS



#include "PkAssert.h"

#include "PkTypes.h"

#include <pthread.h>



#include "PkScopeLock.h"



/*

* Pthreads does not have events, the closest we have is a condition variable

*/

class PkEvent

{

public:



	// Our default constructor

	PkEvent() 

		: m_CappedSpinCounter( 0 ) 

	{

		// Initialize a conditon variable attribute structure to default values

		pthread_condattr_t nixCondAttr;

		PkVerify( 0 == pthread_condattr_init( &nixCondAttr ) );

	

		// Initialize our *nix condition variable from the specified attributes

		PkVerify( 0 == pthread_cond_init( &m_ConditionVariable, &nixCondAttr ) );



		// For completeness, destroy the attributes as well

		PkVerify( 0 == pthread_condattr_destroy( &nixCondAttr ) );

	}



	// Our destructor

	~PkEvent()

	{

		// Release condition variable to system

		PkVerify( 0 == pthread_cond_destroy( &m_ConditionVariable ) );

	}



	// Waits for the event to be signaled

	void block()

	{

		// Obtain exclusive lock on a mutex

		m_CriticalSection.lock();



		// Wait on our condition variable

		m_CappedSpinCounter = std::min<PkInt>( MAX_EVENT_SPIN_COUNT, m_CappedSpinCounter+1 );

		while ( m_CappedSpinCounter > 0 )

		{

			// wait() will internally unlock the mutex parameter

			// HACK: we're touching our critical section's internal mutex

			PkVerify( 0 == pthread_cond_wait( &m_ConditionVariable, &m_CriticalSection.m_NixMutex ) );

			// From: http://opengroup.org/onlinepubs/007908799/xsh/pthread_cond_signal.html

			// Upon waking up via pthread_cond_signal() or pthread_cond_broadcast(),

			// the returning thread owns the lock as if it had called pthread_mutex_lock()

		}



		// Release our lock

		m_CriticalSection.unlock();

		

		// HACK: block on a semaphore to gaurantee that notify() is finished!

		Pk_SCOPE_LOCK( m_EnsureNotifyIsAtomic );

	}



	// Signals a thread waiting on an event. Since events are auto-reset,

	// the event returns immediately to the non-signaled waitable state

	void notify()

	{

		// HACK: block on a semaphore to gaurantee that notify() is finished! (see issues from Boost implementation)

		Pk_SCOPE_LOCK( m_EnsureNotifyIsAtomic );



		// Wake up anyone waiting on us

		{

			Pk_SCOPE_LOCK( m_CriticalSection );

			m_CappedSpinCounter = std::max<PkInt>( MIN_EVENT_SPIN_COUNT, m_CappedSpinCounter-1 );

		}



		PkVerify( 0 == pthread_cond_signal( &m_ConditionVariable ) );

	}



private:



	// The range of allowed spin count values

	enum { MIN_EVENT_SPIN_COUNT = -1, MAX_EVENT_SPIN_COUNT = 1 };



	// Disallow copy and assignment

	PkEvent( const PkEvent& );

	PkEvent& operator=( const PkEvent& );



	// A capped spin counter between [MIN_EVENT_SPIN_COUNT, MAX_EVENT_SPIN_COUNT] 

	// to simulate Windows event semantics

	PkInt m_CappedSpinCounter;

	// Our *nix condition variable

	pthread_cond_t m_ConditionVariable;

	// Our "mutex" which must be locked when passed to the condition variable

	PkCriticalSection m_CriticalSection;



	// Critical section used to ensure that notify() operations are atomic

	PkCriticalSection m_EnsureNotifyIsAtomic;

};



#endif // Pk_ENABLE_UNIX_THREADS



#endif // PkEventUnix_h

