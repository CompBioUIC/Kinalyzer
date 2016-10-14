/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkMiscUtil.h"
#include "PkClock.h"
#include "PkStats.h"
#include "PkGlobalData.h"
#include "PkPhases.h"
#include "PkAppHooks.h"

// Consensus
#include "PkConsensus.h"

// Commandlet integration
#include "PkCommandletHooks.inl"

// The main for default 2-allele implementation
static int Vanilla2AlleleMain( const PkInt argc, char** argv )
{
	// Initialize global data from commandline
	PkGlobalData::init( argc, argv );

	// Reconstruct possible sibling groups locally at each locus
	PkPhase::localLociReconstruction();

	// Intersect across all loci to determine feasible sibling sets
	PkPhase::lociIntersection();
	
	// Output results
	PkPhase::outputResults( PkGlobalData::getOutputFileName(), PkGlobalData::getGlobalSiblingSets() );

	// Signify we finished okay
	return 0;
}

// Register applications here
const struct PkAppInfo GApps[] =
{
  { "2allele", &Vanilla2AlleleMain }
, { "consensus", &PkConsensus::Main }
};

inline int RunApps( const PkInt argc, char** argv )
{
	//******************************
	// Main application starts here:
	//******************************
	Pk_CONDITIONAL_RUN_APPS( argc, argv, GApps );
	// If we reach here, we didn't find an application to run
	return -1;
}

// Main
int main( PkInt argc, char** argv )
{
	// Commandlets!
	Pk_CONDITIONAL_RUN_COMMANDLETS( argc, argv );

	// Begin scoped stats
	int exit_code = 0;
	{
		Pk_SCOPED_STAT_TIMER( ePkSTAT_TotalTime );
		exit_code = RunApps( argc, argv );
	}
	// End scoped stats

	Pk_STATS_DUMP( argc, argv );

	return exit_code;
}
