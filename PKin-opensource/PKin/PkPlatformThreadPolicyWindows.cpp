/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"

#if Pk_ENABLE_WINDOWS_THREADS

#ifdef WINDOWS

#include "PkPlatformThreadPolicy.h"
#include "PkPoolThread.h"
#include "PkAssert.h"

#include <windows.h>

namespace
{

// Actual thread entry point in windows.
// Simply forwards to the thread entry member function
DWORD __stdcall StaticThreadMain(LPVOID pThread)
{
	PkAssert( pThread );
	((PkPoolThread*)pThread)->threadMain();
	return 0;
}

// Code for setting name of thread in debugger:
// http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
#define Pk_MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType;     // Must be 0x1000.
   LPCSTR szName;    // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName( DWORD dwThreadID, char* threadName)
{
   Sleep(10);
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( Pk_MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}

} // end of unnamed namespace

// Namespace containing platform specific implementations for
// creating and destroying threads
namespace PkPlatformThreadPolicy
{

// Platform specific hook to create a thread
void create( const PkUInt stackSizeBytes, PkPoolThread& poolThread, struct tPlatformData& platformData )
{
	// Create the new thread
	platformData.m_Handle = CreateThread(
		  NULL /* lpThreadAttributes */
		, stackSizeBytes /* dwStackSize */
		, StaticThreadMain /* lpStartAddress */
		, &poolThread /* lpParameter */
		, 0 /* dwCreationFlags */
		, &platformData.m_ThreadId /* lpThreadId */
		);
	PkAssert( platformData.m_Handle );

	// Let the thread start up, then set the name for debug purposes.
	Sleep( 1 );
	SetThreadName( platformData.m_ThreadId, "PkPoolThread" );
}

// Platform specific hook to kill a thread
void kill( struct tPlatformData& platformData, const PkBool bShouldWait, const PkUInt maxWaitTime )
{
	// If specified, wait for thread to finish or until the wait timeouts
	if ( bShouldWait )
	{
		// If this times out, kill the thread
		if ( WAIT_TIMEOUT == WaitForSingleObject( platformData.m_Handle, (DWORD)maxWaitTime ) )
		{
			// Bad, kill the thread (this can leak thread local storage data)
			TerminateThread( platformData.m_Handle, -1 );
		}
	}

	// Free thread resource handle
	CloseHandle( platformData.m_Handle );
	platformData.m_Handle = NULL;
}

} // end of PkPlatformThreadPolicy namespace

#endif // WINDOWS

#endif // Pk_ENABLE_WINDOWS_THREADS
