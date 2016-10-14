/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"

#if Pk_ENABLE_BOOST_THREADS

#include "PkPlatformThreadPolicy.h"
#include "PkPoolThread.h"
#include "PkAssert.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

// Namespace containing platform specific implementations for
// creating and destroying threads
namespace PkPlatformThreadPolicy
{

// Platform specific hook to create a thread
void create( const PkUInt stackSizeBytes, PkPoolThread& poolThread, struct tPlatformData& platformData )
{
	// A functor for our thread entry point
	struct BoostThreadMain
	{
		// Constructor which initializes our thread reference
		explicit BoostThreadMain( PkPoolThread& inPoolThread )
		: m_PoolThread( inPoolThread )
		{}
		
		// Our callback which simply forwards to the real entry point
		void operator()()
		{
			m_PoolThread.threadMain();
		}

	private:
		PkPoolThread& m_PoolThread;
	};

	// Create the new thread
	platformData.m_BoostThread = boost::thread( BoostThreadMain( poolThread ) );
}

// Platform specific hook to kill a thread
void kill( struct tPlatformData& platformData, const PkBool bShouldWait, const PkUInt maxWaitTime )
{
	// If specified, wait for thread to finish or until the wait timeouts
	if ( bShouldWait )
	{
		// Wait for thread to finish
		if ( Pk_THREAD_WAIT_TIME_INFINITY == maxWaitTime )
		{
			// If infinite wait, just do a regular join
			platformData.m_BoostThread.join();
		}
		else 
		{
			// If an actual wait time is specified, then do a timed join
			platformData.m_BoostThread.timed_join(
				boost::posix_time::from_time_t( maxWaitTime ) 
				);
		}
	}

	// Now, detach the thread
	platformData.m_BoostThread.detach();
}

} // end of PkPlatformThreadPolicy namespace

#endif // Pk_ENABLE_BOOST_THREADS
