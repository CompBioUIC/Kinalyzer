/**
 * @author Alan Perez-Rathke 
 *
 * @date April 21, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"

#if Pk_ENABLE_UNIX_THREADS

#include "PkPlatformThreadPolicy.h"
#include "PkPoolThread.h"
#include "PkAssert.h"

namespace
{

// Actual thread entry point in Unix.
// Simply forwards to the thread entry member function
void* StaticThreadMain( void* pThread )
{
	PkAssert( pThread );
	((PkPoolThread*)pThread)->threadMain();
	return NULL;
}

} // end of unnamed namespace

// Namespace containing platform specific implementations for
// creating and destroying threads
namespace PkPlatformThreadPolicy
{

// Platform specific hook to create a thread
void create( const PkUInt stackSizeBytes, PkPoolThread& poolThread, struct tPlatformData& platformData )
{
	// Initialize a *nix pthread attribute structure to default values
	pthread_attr_t nixPThreadAttr;
	PkVerify( 0 == pthread_attr_init( &nixPThreadAttr ) );
	
	// Override the atttribute structure to specify the stack size
	PkVerify( 0 == pthread_attr_setstacksize( &nixPThreadAttr, stackSizeBytes ) );

	// Assert that stack size attribute was set properly
#if Pk_ENABLE_ASSERT
	{
		size_t checkStackSizeBytes = 0;
		PkVerify( 0 == pthread_attr_getstacksize( &nixPThreadAttr, &checkStackSizeBytes ) );
		PkAssert( checkStackSizeBytes == stackSizeBytes );
	}
#endif

	// Create our *nix pthread
	PkVerify( 0 == pthread_create(
		  &platformData.m_NixPThread /* pthread_t *thread */
		, &nixPThreadAttr /* const pthread_attr_t *attr */
		, StaticThreadMain /* void *(*start_routine)(void*) */
		, (void*)&poolThread /* void *arg */
		) );

	// Don't really think this is necessary, but for the sake of completeness, also destroy the attribute structure
	PkVerify( 0 == pthread_attr_destroy( &nixPThreadAttr ) );
}

// Platform specific hook to kill a thread
void kill( struct tPlatformData& platformData, const PkBool bShouldWait, const PkUInt maxWaitTime )
{
	// If specified, wait for thread to finish or until the wait timeouts
	if ( bShouldWait )
	{
		// Wait for thread to finish
		// Join operation should also detach the thread
		// @todo: timed wait?
		PkVerify( 0 == pthread_join( platformData.m_NixPThread, NULL /* void **value_ptr */ ) );
	}
	else
	{
		// Tell system that it's okay to reclaim thread resources
		PkVerify( 0 == pthread_detach( platformData.m_NixPThread ) );
	}
}

} // end of PkPlatformThreadPolicy namespace

#endif // Pk_ENABLE_UNIX_THREADS
