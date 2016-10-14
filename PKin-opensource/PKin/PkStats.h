/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkStats_h
#define PkStats_h

#include "PkBuild.h"

#if Pk_ENABLE_STATS

#include "PkAssert.h"
#include "PkClock.h"
#include "PkTypes.h"

enum EPkAllTimerStats
{
	ePkSTAT_TotalTime = 0,
	ePkSTAT_PhaseLocalLociReconstructionTime,
	ePkSTAT_IntersectSiblingSetsTotalTime,
	ePkSTAT_IntersectSiblingSetsBlockTime,
	ePkSTAT_GlobalSubsetPolicyOnIntersectionJobFinishTime,
	ePkSTAT_GlobalSubsetPolicyOnIntersectionLoopFinishTime,
	ePkSTAT_GlobalSubsetFrequencyMappingTime,
	ePkSTAT_GlobalSubsetCullingTime,
	ePkSTAT_PhaseOutputResultsTime,
	ePkSTAT_FileIO,
	ePkSTAT_MoveFilesTime,
	ePkSTAT_DeleteFilesTime,
	ePkSTAT_MakeDirectoriesTime,
	ePkSTAT_DeleteDirectoriesTime,
	// Consensus stats
	ePkSTAT_ConsensusLociIntersectionAndOutputResultsTime,
	ePkSTAT_ConsensusExecGamsAndAllScriptsTime,
	ePkSTAT_ConsensusStrictVoteTime,
	ePkSTAT_ConsensusGreedyMergeGroupsTime,
	ePkSTAT_ConsensusOutputSolutionTime,
	ePkSTAT_TimerMAX
};

enum EPkAllCounterStats
{
	ePkSTAT_IntersectSiblingSetsCallCount = 0,
	ePkSTAT_CounterMAX
};

enum EPkAllThreadedTimerStats
{
	ePkSTAT_ThreadedIntersectionTotalTime = 0,
	ePkSTAT_ThreadedLocalSubsetPolicyProcessIntersectionTime,
	ePkSTAT_ThreadedLocalSubsetPolicyOnIntersectionLoopFinishedTime,
	ePkSTAT_ThreadedTimerMAX
};

enum EPkAllThreadedCounterStats
{
	ePkSTAT_ThreadedIntersectionsCount = 0,
	ePkSTAT_ThreadedCounterMAX
};

#define Pk_SCOPED_STAT_TIMER( statId ) PkStats::ScopedStatTimer ScopedTimer_##statId(statId)

#define Pk_INC_STAT_COUNTER( statId ) PkStats::GCounterStats[ statId ].incBy( 1.0 )

#define Pk_INC_STAT_COUNTER_BY( statId, amount ) PkStats::GCounterStats[ statId ].incBy( amount )

#define Pk_SCOPED_STAT_THREADED_TIMER( statId, threadId ) PkStats::ScopedStatThreadedTimer ScopedTimer_##statId(statId,threadId)

#define Pk_INC_STAT_THREADED_COUNTER( statId, threadId ) PkStats::GThreadedCounterStats[ statId ][ threadId ].incBy( 1.0 )

#define Pk_INC_STAT_THREADED_COUNTER_BY( statId, threadId, amount ) PkStats::GThreadedCounterStats[ statId ][ threadId ].incBy( amount )

namespace PkStats
{

class TimerStat
{
public:
	TimerStat(): m_totalTime(0) {}

	inline void incTotalTime( const time_t t ) { m_totalTime += t; }
	inline time_t getTotalTime() const { return m_totalTime; }

private:
	// total time of this stat
	time_t m_totalTime;
};

class CounterStat
{
public:
	CounterStat() : m_counter( 0.0 ) {}

	inline void incBy( const double amount ) { m_counter += amount; }
	inline double getCounter() const { return m_counter; }

private:
	double m_counter;
};

// All the timer stat instances
extern TimerStat GTimerStats[ePkSTAT_TimerMAX];

// All the counter stat instances
extern CounterStat GCounterStats[ ePkSTAT_CounterMAX ];

// All threaded timer stat instances
extern PkArray( TimerStat ) GThreadedTimerStats[ePkSTAT_ThreadedTimerMAX];

// All the threaded counter stat instances
extern PkArray( CounterStat ) GThreadedCounterStats[ePkSTAT_ThreadedCounterMAX];

// A synchronous stat timer
class ScopedStatTimer
{
public:
	explicit ScopedStatTimer( const EPkAllTimerStats statId )
		: m_statId(statId)
	{
		m_startTime = PkClock::getSeconds();
	}

	~ScopedStatTimer()
	{
		GTimerStats[m_statId].incTotalTime( PkClock::getSeconds() - m_startTime );		
	}

private:
	EPkAllTimerStats  m_statId;
	time_t            m_startTime;
};

// A per-thread stat timer
class ScopedStatThreadedTimer
{
public:
	explicit ScopedStatThreadedTimer( const EPkAllThreadedTimerStats statId, const PkUInt threadId )
		: m_statId(statId)
		, m_threadId(threadId)
	{
		m_startTime = PkClock::getSeconds();
	}

	~ScopedStatThreadedTimer()
	{
		GThreadedTimerStats[m_statId][m_threadId].incTotalTime( PkClock::getSeconds() - m_startTime );		
	}

private:
	EPkAllThreadedTimerStats    m_statId;
	PkUInt                      m_threadId;
	time_t                      m_startTime;
};

// Initializes the per-thread stats
extern void initThreadedStats();

// Report stats
extern void dumpStats( const PkInt argc, char** argv );

} // namespace PkStats

#define Pk_STATS_INIT_THREADED() PkStats::initThreadedStats()
#define Pk_STATS_DUMP( argc, argv ) PkStats::dumpStats( argc, argv )

#else // !Pk_ENABLE_STATS

#define Pk_SCOPED_STAT_TIMER( statId )
#define Pk_INC_STAT_COUNTER( statId )
#define Pk_INC_STAT_COUNTER_BY( statId, amount )
#define Pk_SCOPED_STAT_THREADED_TIMER( statId, threadId ) 
#define Pk_INC_STAT_THREADED_COUNTER( statId, threadId )
#define Pk_INC_STAT_THREADED_COUNTER_BY( statId, threadId, amount )
#define Pk_STATS_INIT_THREADED()
#define Pk_STATS_DUMP( argc, argv )

#endif // Pk_ENABLE_STATS

#endif //PkStats_h
