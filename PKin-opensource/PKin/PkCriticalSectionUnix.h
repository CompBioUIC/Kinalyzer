/**

 * @author Alan Perez-Rathke 

 *

 * @date April 21, 2010

 *

 * Department of Computer Science

 * University of Illinois at Chicago 

 */



#ifndef PkCriticalSectionUnix_h

#define PkCriticalSectionUnix_h



#include "PkBuild.h"



#if Pk_ENABLE_UNIX_THREADS



#include "PkAssert.h"

#include <pthread.h>

#include <errno.h>



/**

 * *Nix version of a critical section

 */

class PkCriticalSection

{

public:



	// Default constructor initializes internal mutex

	PkCriticalSection()

	{

		// Initialize a mutex attribute structure to default values

		pthread_mutexattr_t nixMutexAttr;

		PkVerify( 0 == pthread_mutexattr_init( &nixMutexAttr ) );



		// If we're enabling asserts, then override the mutex type to be error checked

#if Pk_ENABLE_ASSERT

		PkVerify( 0 == pthread_mutexattr_settype( &nixMutexAttr, PTHREAD_MUTEX_ERRORCHECK  ) );

#else

		PkVerify( 0 == pthread_mutexattr_settype( &nixMutexAttr, PTHREAD_MUTEX_NORMAL ) );

#endif

		

		// Initialize our *nix mutex from the specified attributes

		PkVerify( 0 == pthread_mutex_init( &m_NixMutex, &nixMutexAttr ) );



		// For completeness, destroy the attributes as well

		PkVerify( 0 == pthread_mutexattr_destroy( &nixMutexAttr ) );

	}



	// Release system resources once mutex is no longer needed

	~PkCriticalSection()

	{

		PkVerify( 0 == pthread_mutex_destroy( &m_NixMutex ) );

	}



	// Attempts non-blocking lock first, then does a full lock of the internal *nix mutex 

	inline void lock()

	{

		// Try to see if the lock is available before trapping to the OS

		const PkInt result = pthread_mutex_trylock( &m_NixMutex );

		PkAssert( result == 0 || result == EBUSY );

		if ( EBUSY == result )

		{

			PkVerify( 0 == pthread_mutex_lock( &m_NixMutex ) );

		}

	}



	// Unlock internal windows critical sections

	inline void unlock(void)

	{

		PkVerify( 0 == pthread_mutex_unlock( &m_NixMutex ) );

	}



private:



	// Disallow copy and assignment

	PkCriticalSection( const PkCriticalSection& );

	PkCriticalSection& operator=( const PkCriticalSection& );



	// Our *nix mutex

	pthread_mutex_t m_NixMutex;



	// Allow PkEvent to access our internal *nix mutex

	friend class PkEvent;

};



#endif // Pk_ENABLE_UNIX_THREADS



#endif // PkCriticalSectionUnix_h

