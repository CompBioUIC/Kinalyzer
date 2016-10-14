/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkCriticalSectionBoost_h
#define PkCriticalSectionBoost_h

#include "PkBuild.h"

#if Pk_ENABLE_BOOST_THREADS

#include <boost/version.hpp>
#include <boost/thread.hpp>

// Create macros to deal with Boost API changes
#if BOOST_VERSION >= 103500
	typedef boost::mutex tPkBoostMutex;
	#define Pk_BOOST_MUTEX_LOCK( m ) (m).lock()
	#define Pk_BOOST_MUTEX_UNLOCK( m ) (m).unlock()
	#define Pk_BOOST_MUTEX_TRY_LOCK( m ) (m).try_lock()
#else	
	#include <boost/thread/detail/lock.hpp>
	typedef boost::try_mutex tPkBoostMutex;
	typedef boost::detail::thread::lock_ops< tPkBoostMutex > tPkBoostLockOps; 
	#define Pk_BOOST_MUTEX_LOCK( m ) tPkBoostLockOps::lock((m))
	#define Pk_BOOST_MUTEX_UNLOCK( m ) tPkBoostLockOps::unlock((m))
	#define Pk_BOOST_MUTEX_TRY_LOCK( m ) tPkBoostLockOps::trylock((m))
#endif // BOOST_VERSION >= 103500

/**
 * Boost version of a critical section
 */
class PkCriticalSection
{
public:

	// Our default constructor
	PkCriticalSection() {}

	// Attempts non-blocking lock first, then does a full lock of the internal boost mutex 
	inline void lock()
	{
		// Try to see if the lock is available before trapping to the OS
		if ( !Pk_BOOST_MUTEX_TRY_LOCK( m_Mutex ) )
		{
			Pk_BOOST_MUTEX_LOCK( m_Mutex );
		}
	}

	// Unlock internal boost mutex
	inline void unlock(void)
	{
		Pk_BOOST_MUTEX_UNLOCK( m_Mutex );
	}

private:

	// Explicitly disallow copy and assignment to conform with other platform implementations
	PkCriticalSection( const PkCriticalSection& );
	PkCriticalSection& operator=( const PkCriticalSection& );

	// Our boost mutex
	tPkBoostMutex m_Mutex;
};

#endif // Pk_ENABLE_BOOST_THREADS

#endif // PkCriticalSectionBoost_h
