/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkCriticalSection_h
#define PkCriticalSection_h

#include "PkBuild.h"

// Include our platform specific critical section implementation
#if Pk_ENABLE_WINDOWS_THREADS
	#include "PkCriticalSectionWindows.h"
#elif Pk_ENABLE_UNIX_THREADS
	#include "PkCriticalSectionUnix.h"
#elif Pk_ENABLE_BOOST_THREADS
	#include "PkCriticalSectionBoost.h"
#else
	#error Unsupported thread platform
#endif

#endif // PkCriticalSection_h
