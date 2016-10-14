/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

// Implementation of commandlet hooks extracted to separate file
// to avoid cluttering PkMain.cpp

#include "PkBuild.h"
#include "PkAppHooks.h"

#if Pk_ENABLE_COMMANDLETS

// Include additional commandlets here
#include "PkCommandletThreadPoolTest.h"

// Placeholder main that dumps information about kinship reconstruction project
static PkInt PkInfoMain( PkInt argc, char** argv )
{
	printf("Kinship Reconstruction.  University of Illinois at Chicago.\n");
	return 0;
}

// Register commandlets here
static struct PkAppInfo GCommandlets[] =
{
	  { "info", &PkInfoMain }
	  // <app-name> threadpooltest <uint:num threads> <uint:stack size kb>
	, { "threadpooltest", &PkCommandletThreadPoolTest::Main }
};

// Commandlets are enabled, route to the proper commandlet hook:
// Parse commandline and run any commandlets specified
#define Pk_CONDITIONAL_RUN_COMMANDLETS( argc, argv ) Pk_CONDITIONAL_RUN_APPS( argc, argv, GCommandlets )

#else

// Commandlets are not enabled, strip away call to commandlet hook
#define Pk_CONDITIONAL_RUN_COMMANDLETS( argc, argv )

#endif // Pk_ENABLE_COMMANDLETS
