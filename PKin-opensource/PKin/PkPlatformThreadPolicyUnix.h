/**
 * @author Alan Perez-Rathke 
 *
 * @date April 21, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPlatformThreadPolicyUnix_h
#define PkPlatformThreadPolicyUnix_h

#include "PkBuild.h"

#if Pk_ENABLE_UNIX_THREADS

#include <pthread.h>

namespace PkPlatformThreadPolicy
{
	// *Nix specific thread data
	struct tPlatformData
	{
		pthread_t m_NixPThread;
	};
}

#endif // Pk_ENABLE_UNIX_THREADS

#endif // PkPlatformThreadPolicyUnix_h
