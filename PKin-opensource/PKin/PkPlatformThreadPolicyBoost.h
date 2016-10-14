/**
 * @author Alan Perez-Rathke 
 *
 * @date March 10, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPlatformThreadPolicyBoost_h
#define PkPlatformThreadPolicyBoost_h

#include "PkBuild.h"

#if Pk_ENABLE_BOOST_THREADS

#include <boost/thread.hpp>

namespace PkPlatformThreadPolicy
{
	// Boost specific thread data
	struct tPlatformData
	{
		boost::thread m_BoostThread;
	};
}

#endif // Pk_ENABLE_BOOST_THREADS

#endif // PkPlatformThreadPolicyBoost_h
