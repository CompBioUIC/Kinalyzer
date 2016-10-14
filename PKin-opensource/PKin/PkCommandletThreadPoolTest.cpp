/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"

#if Pk_ENABLE_COMMANDLETS

#include "PkAssert.h"
#include "PkTypes.h"
#include "PkClock.h"
#include "PkThreadPool.h"
#include "PkRunnable.h"
#include "PkMiscUtil.h"
#include "PkCommandletThreadPoolTest.h"

#if WINDOWS
	#include <windows.h>
#endif

/**
* Commandlet for testing thread pool system
*/
namespace PkCommandletThreadPoolTest
{

// A runnable that busy loops for a parameter number of seconds
class BusyLoopRunnable : public PkRunnable
{
public:

	// Constructor which initializes how long job should run for and if it's managed
	BusyLoopRunnable( const time_t jobDurationSeconds, const PkBool bIsManaged = false ) 
	: PkRunnable( bIsManaged )
	, m_JobDurationSeconds( jobDurationSeconds )
	{}

	// A simple busy loop function
	virtual PkBool doWork()
	{
		PkLogf( "BusyLoopRunnable %d seconds begin\n", (PkInt)getJobDurationSeconds() );
		const time_t startSeconds = PkClock::getSeconds();
		time_t elapsedSeconds = 0;
		do
		{
			// Busy loop for a bit
			PkInt counter=0;
			for ( PkInt i=0; i<1024; ++i )
			{
				++counter;
			}
			elapsedSeconds = PkClock::getSeconds() - startSeconds;
		}
		while ( elapsedSeconds < getJobDurationSeconds() ); 
		PkLogf( "BusyLoopRunnable %d seconds end\n", (PkInt)getJobDurationSeconds() );
		incrementJobComplementionCounter();
		return true;
	}

	// @return - the total number of jobs that have completed
	static inline PkInt getJobCompletionCounter() 
	{ 
		return m_StaticJobCompletionCounter; 
	}

private:

	// updates the number of completed jobs
	static void incrementJobComplementionCounter()
	{
		++m_StaticJobCompletionCounter;
	}

	// @return - number of seconds busy loop should run
	inline time_t getJobDurationSeconds() const { return m_JobDurationSeconds; }

	// Data member storing how many seconds this job should run for
	const time_t m_JobDurationSeconds;
	
	// The counter for keeping track of number of completed jobs
	static volatile PkInt m_StaticJobCompletionCounter;
};

// Initialize job completion counter
volatile PkInt BusyLoopRunnable::m_StaticJobCompletionCounter = 0;

// Enumerated commandline arguments
enum ECmdArgs
{
  // Executable name
  ECmdArgs_ExeName=0
  // Commandlet name
, ECmdArgs_CmdName
  // Number of threads to test
, ECmdArgs_NumThreads
  // Stack size of each pool thread in kilobytes
, ECmdArgs_StackSizeKb
  // Max number of arguments
, ECmdArgs_MaxArgs
};

// Entry point for commandlet
// Sample commandline:
// <app-name> threadpooltest <uint:num threads> <uint:stack size kb>
PkInt Main( const PkInt argc, char** argv )
{
	PkAssert( ECmdArgs_MaxArgs == argc );

	// Parse commandline arguments
	const PkUInt numThreads = atoi( argv[ ECmdArgs_NumThreads ] );
	const PkUInt stackSizeKb = atoi( argv[ ECmdArgs_StackSizeKb ] );
	// Convert stack size to bytes
	const PkUInt stackSizeBytes = stackSizeKb * 1024;

	// Initialize thread pool
	PkThreadPool threadPool( numThreads, stackSizeBytes );

	// Initialize jobs to run on the pool
	BusyLoopRunnable jobs[] =
	{
	  BusyLoopRunnable( 5  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 3  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 4  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 2  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 1  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 5  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 7  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 2  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 3  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 1  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 6  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 2  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 3  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 4  /* jobDurationSeconds */ )
	, BusyLoopRunnable( 20 /* jobDurationSeconds */ )
	, BusyLoopRunnable( 30 /* jobDurationSeconds */ )
	};

	// Determine number of jobs
	const PkInt numJobs = sizeof(jobs)/sizeof(BusyLoopRunnable);

	// Queue the jobs	
	for ( PkInt i=0; i<numJobs; ++i )
	{
		threadPool.queueJob( jobs[i] );
	}

	// Wait for jobs to finish
	const time_t startTime = PkClock::getSeconds();
	while ( BusyLoopRunnable::getJobCompletionCounter() < numJobs )
	{
		// Man pages for unix indicate that sleep equivalents
		// sleep the entire process?  There is pthread_yield
		// but that does not take a time parameter.  Therefore,
		// only using Sleep for Windows at the moment
#if WINDOWS
		Sleep( 1000 /* dwMilliseconds */ );
#endif
		// Destroy thread pool while jobs are running
		if ( BusyLoopRunnable::getJobCompletionCounter() >= (PkInt)(numJobs * 0.75f) )
		{
#if WINDOWS
			Sleep(1);
#endif
			PkLogf( "Destroying thread pool while running jobs\n" );
			threadPool.destroy();
			break;
		}
	}
	const time_t elapsedTime = PkClock::getSeconds() - startTime;
	PkLogf( "Finished all jobs in %d seconds", (PkInt)elapsedTime );

	return 0;
}

} // end of PkCommandletThreadPoolTest namespace

#endif // Pk_ENABLE_COMMANDLETS
