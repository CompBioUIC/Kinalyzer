/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPlatformThreadPolicyWindows_h
#define PkPlatformThreadPolicyWindows_h

#include "PkBuild.h"

#if Pk_ENABLE_WINDOWS_THREADS

#if WINDOWS

#include <windows.h>

namespace PkPlatformThreadPolicy
{
	// Windows specific thread data
	struct tPlatformData
	{
		// Windows thread resource handle
		HANDLE m_Handle;
		// Windows thread identifier
		DWORD m_ThreadId;
	};
}

#endif // WINDOWS

#endif // Pk_ENABLE_WINDOWS_THREADS

#endif // PkPlatformThreadPolicyWindows_h
