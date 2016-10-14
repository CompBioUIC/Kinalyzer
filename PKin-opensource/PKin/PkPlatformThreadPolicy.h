/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPlatformThreadPolicy_h
#define PkPlatformThreadPolicy_h

#include "PkBuild.h"
#include "PkTypes.h"

// Forward declarations
class PkPoolThread;

#define Pk_THREAD_WAIT_TIME_INFINITY -1

// Namespace containing platform specific implementations for
// creating and destroying threads
namespace PkPlatformThreadPolicy
{
	// Platform specific hook to create a thread
	extern void create( const PkUInt stackSizeBytes, PkPoolThread& poolThread, struct tPlatformData& platformData );

	// Platform specific hook to kill a thread
	extern void kill( struct tPlatformData& platformData, const PkBool bShouldWait=true, const PkUInt maxWaitTime=Pk_THREAD_WAIT_TIME_INFINITY ); 
}

// Include our platform specific thread policy
#if Pk_ENABLE_WINDOWS_THREADS
	#include "PkPlatformThreadPolicyWindows.h"
#elif Pk_ENABLE_UNIX_THREADS
	#include "PkPlatformThreadPolicyUnix.h"
#elif Pk_ENABLE_BOOST_THREADS
	#include "PkPlatformThreadPolicyBoost.h"
#else
	#error Unsupported thread platform
#endif

#endif // PkPlatformThreadPolicy_h
