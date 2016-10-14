/** 
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkEventWindows_h
#define PkEventWindows_h

#include "PkBuild.h"

#if Pk_ENABLE_WINDOWS_THREADS

#ifndef WINDOWS
	#error This code operates only on a window's compliant compiler
#endif

// Not worrying about XTL, simply assuming MS Windows
#include <windows.h>
#include "PkAssert.h"

#ifndef Pk_INFINITE_WINDOWS
	#define Pk_INFINITE_WINDOWS ((unsigned int)-1)
#endif

/*
* Window's version of an event, events are auto-reset
* Allows communication between two threads
*/
class PkEvent
{
public:

	// Default constructor
	PkEvent()
	: m_hEvent( NULL )
	{
		// Create auto-reset, unnamed, non-signaled event
		m_hEvent = CreateEvent(
			  NULL /* lpEventAttribute */
			, false /* bManualReset - HARDCODED to false, we are never manual-reset! */
			, 0 /* bInitialState - in this case, non-signaled */
			, NULL /* name of event */
			);
		PkAssert( NULL != m_hEvent );
	}

	// Destructor cleans up internal handle
	~PkEvent()
	{
		PkAssert( NULL != m_hEvent );
		PkVerify( 0 < CloseHandle( m_hEvent ) );
	}

	// Waits for the event to be signaled
	void block()
	{
		PkVerify( WAIT_OBJECT_0 == WaitForSingleObject( m_hEvent, Pk_INFINITE_WINDOWS ) );
	}

	// Signals a thread waiting on an event. Since events are auto-reset,
	// the event returns immediately to the non-signaled waitable state
	void notify()
	{
		PkAssert( m_hEvent );
		PkVerify( 0 < SetEvent( m_hEvent ) );	
	}

private:

	// Disallow copy and assignment
	PkEvent( const PkEvent& );
	PkEvent& operator=( const PkEvent& );

	// Internal handle to windows event object
	HANDLE m_hEvent;
};

#endif // Pk_ENABLE_WINDOWS_THREADS

#endif // PkEventWindows_h
