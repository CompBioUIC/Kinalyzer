/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkEvent_h
#define PkEvent_h

#include "PkBuild.h"

// Include our platform specific event implementation
#if Pk_ENABLE_WINDOWS_THREADS
	#include "PkEventWindows.h"
#elif Pk_ENABLE_UNIX_THREADS
	#include "PkEventUnix.h"
#elif Pk_ENABLE_BOOST_THREADS
	#include "PkEventBoost.h"
#else
	#error Unsupported thread platform
#endif

#endif // PkEvent_h
