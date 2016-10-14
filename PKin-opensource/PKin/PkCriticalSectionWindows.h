/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkCriticalSectionWindows_h
#define PkCriticalSectionWindows_h

#include "PkBuild.h"

#if Pk_ENABLE_WINDOWS_THREADS

#ifndef WINDOWS
	#error This code operates only on a window's compliant compiler
#endif

// Not worrying about XTL, simply assuming MS Windows
#include <windows.h>

// Make sure version of windows supports platform specific API
#if (_WIN32_WINNT < 0x0403)
	#error SetCriticalSectionSpinCount requires _WIN32_WINNT >= 0x0403
#endif

#define Pk_CRITICAL_SECTION_SPIN_COUNT_WINDOWS 3500

/**
 * Windows version of a critical section
 */
class PkCriticalSection
{
public:

	// Default constructor, initializes internal windows critical section and spin count
	inline PkCriticalSection()
	{
		InitializeCriticalSection( &m_CriticalSection );
		
		/* http://msdn.microsoft.com/en-us/library/ms686197(VS.85).aspx
		   The spin count is useful for critical sections of short duration that
		   can experience high levels of contention. Consider a worst-case scenario,
		   in which an application on an SMP system has two or three threads
		   constantly allocating and releasing memory from the heap. The application
		   serializes the heap with a critical section. In the worst-case scenario, 
		   contention for the critical section is constant, and each thread makes
		   an expensive call to the WaitForSingleObject function. However, if the
		   spin count is set properly, the calling thread does not immediately call
		   WaitForSingleObject when contention occurs. Instead, the calling thread 
		   can acquire ownership of the critical section if it is released during the
		   spin operation. */
		SetCriticalSectionSpinCount( &m_CriticalSection, Pk_CRITICAL_SECTION_SPIN_COUNT_WINDOWS );
	}

	// Destructor for cleaning up internal windows critical section
	inline ~PkCriticalSection()
	{
		DeleteCriticalSection( &m_CriticalSection );
	}

	// Attempts non-blocking lock first, then does a full lock of the internal windows critical section 
	inline void lock()
	{
		/* http://msdn.microsoft.com/en-us/library/ms686857(VS.85).aspx
		   To enable mutually exclusive use of a shared resource, each thread
		   calls the EnterCriticalSection or TryEnterCriticalSection function
		   to request ownership of the critical section before executing any
		   section of code that uses the protected resource. The difference is
		   that TryEnterCriticalSection returns immediately, regardless of whether
		   it obtained ownership of the critical section, while EnterCriticalSection
		   blocks until the thread can take ownership of the critical section. */
		if ( 0 == TryEnterCriticalSection( &m_CriticalSection) )
		{
			EnterCriticalSection( &m_CriticalSection );
		}
	}

	// Unlock internal windows critical section
	inline void unlock(void)
	{
		LeaveCriticalSection( &m_CriticalSection );
	}

private:

	// Disallow copy and assignment
	PkCriticalSection( const PkCriticalSection& );
	PkCriticalSection& operator=( const PkCriticalSection& );

	/* http://msdn.microsoft.com/en-us/library/ms682530(VS.85).aspx
	   A critical section object provides synchronization similar to that
	   provided by a mutex object, except that a critical section can be used
	   only by the threads of a single process. Event, mutex, and semaphore 
	   objects can also be used in a single-process application, but critical 
	   section objects provide a slightly faster, more efficient mechanism 
	   for mutual-exclusion synchronization (a processor-specific test and set 
	   instruction). Like a mutex object, a critical section object can be owned 
	   by only one thread at a time, which makes it useful for protecting a shared 
	   resource from simultaneous access. Unlike a mutex object, there is no way 
	   to tell whether a critical section has been abandoned. */
	CRITICAL_SECTION m_CriticalSection;
};

#endif // Pk_ENABLE_WINDOWS_THREADS

#endif // PkCriticalSectionWindows_h
