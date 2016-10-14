/**

 * @author Alan Perez-Rathke 

 *

 * @date March 11, 2010 

 *

 * Department of Computer Science

 * University of Illinois at Chicago 

 */



#ifndef PkCommandletThreadPoolTest_h

#define PkCommandletThreadPoolTest_h



#include "PkBuild.h"



#if Pk_ENABLE_COMMANDLETS



/**

* Commandlet for testing thread pool system

*/

namespace PkCommandletThreadPoolTest

{

	// Entry point for commandlet

	// Sample commandline:

	// <app-name> threadpooltest <uint:num threads> <uint:stack size kb>

	extern PkInt Main( const PkInt argc, char** argv );

}



#endif // Pk_ENABLE_COMMANDLETS



#endif // PkCommandletThreadPoolTest_h

