/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkScopeLock_h
#define PkScopeLock_h

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkCriticalSection.h"

/**
* Utility class for locking within the lifetime (scope) of an instance of this class
*/
class PkScopeLock
{
public:

	// A constructor that acquires a lock on parameter object
	explicit PkScopeLock(PkCriticalSection& synch) 
	: m_Synch(synch)
	{
		m_Synch.lock();
	}

	// Destructor releases lock
	~PkScopeLock(void)
	{
		m_Synch.unlock();
	}

private:

	// Disallow default constructor, copy constructor, and assignment
	PkScopeLock();
	PkScopeLock( const PkScopeLock& );
	PkScopeLock& operator=( const PkScopeLock& );

	// A critical section for managing access to scoped code
	PkCriticalSection& m_Synch;
};

// Doesn't work with with function arguments, but still useful macro
#define Pk_SCOPE_LOCK( critical_section ) PkScopeLock lock_##critical_section( critical_section )

#endif // PkScopeLock_h
