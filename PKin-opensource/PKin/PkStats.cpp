/**
 * @author Alan Perez-Rathke 
 * @author Jaroslaw Gwarnicki
 *
 * @date November 13, 2009 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkStats.h"

#if Pk_ENABLE_STATS

#include "PkAssert.h"
#include "PkFileUtils.h"
#include "PkGlobalData.h"
#include "PkMiscUtil.h"
#include "PkStatsDetailedSubsetCullingStatTracker.h"

namespace PkStats
{

#define TO_MINUTES( name ) (((double)GTimerStats[name].getTotalTime())/60.0)
#define TO_MINUTES_THREADED( name, threadId ) (((double)GThreadedTimerStats[name][threadId].getTotalTime())/60.0)

// Initializes the stat system, requires thread pool to exist
void initThreadedStats()
{
	PkAssert( ( sizeof( GThreadedTimerStats ) / sizeof( PkArray( TimerStat ) ) ) == ePkSTAT_ThreadedTimerMAX );
	PkAssert( ( sizeof( GThreadedCounterStats ) / sizeof( PkArray( CounterStat ) ) ) == ePkSTAT_ThreadedCounterMAX );
	const PkUInt numThreads = PkGlobalData::getThreadPool().numThreads();
	// Resize timers
	PkUInt itrStat=0;
	for ( ; itrStat<ePkSTAT_ThreadedTimerMAX; ++itrStat )
	{
		// Allocate a slot for each pool thread
		GThreadedTimerStats[ itrStat ].resize( numThreads );
	}
	// Resize counters
	for ( itrStat=0; itrStat<ePkSTAT_ThreadedCounterMAX; ++itrStat )
	{
		// Allocate a slot for each pool thread
		GThreadedCounterStats[ itrStat ].resize( numThreads );
	}
}

// A structure for storing the information necessary to output a stat
struct tPkOutStatInfo
{
	tPkOutStatInfo() {}
	tPkOutStatInfo( const PkString& strLabel, const PkString& strValue ) : m_strLabel(strLabel), m_strValue(strValue) {}
	PkString m_strLabel;
	PkString m_strValue;
};

// Extracts the label from an out stat info
struct tPkStatLabelSelector
{
	inline const char* operator()( const tPkOutStatInfo& info ) const { return info.m_strLabel.c_str(); }
};

// Extracts the value from an out stat info
struct tPkStatValueSelector
{
	inline const char* operator()( const tPkOutStatInfo& info ) const { return info.m_strValue.c_str(); }
};

// Generic function to write a row of stats data
template < typename tPkStatSelectorPolicy >
void outputStatsRow
( 
  const tPkOutStatInfo* outStatInfos
, const PkUInt numOutStatInfos
, FILE* pFile
, const tPkStatSelectorPolicy selector )
{
	// Validate arguments
	PkAssert( outStatInfos );
	PkAssert( pFile );

	// Write first stat value if we have one as we may not require commas
	if ( numOutStatInfos > 0 )
	{
		fprintf( pFile, "%s", selector( outStatInfos[0] ) );
	}

	// Write rest of the stat values, prepending a comma
	for ( PkUInt i=1; i<numOutStatInfos; ++i )
	{
		fprintf( pFile, ", %s", selector( outStatInfos[i] ) );
	}

	// Skip to the next line
	fprintf( pFile, "\n" );
}

// Report stats
void dumpStats( const PkInt argc, char** argv )
{
	// The array of stats to output:
	const tPkOutStatInfo nonThreadedStatInfos[] = 
	{
		  tPkOutStatInfo( PkString("PopulationFile"), argv[ PkGlobalData::ECmdArgs_PopulationPath ] )
		, tPkOutStatInfo( PkString("PopulationSize"), PkStringOf( PkGlobalData::getPopulation().size() ) )
#if Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS
		, tPkOutStatInfo( PkString( "AverageSparsity" ), PkStringOf( GDetailedSubsetCullingStatTracker.getAverageSparsityPercentage() ) )
		, tPkOutStatInfo( PkString( "ProportionSubsets" ), PkStringOf( GDetailedSubsetCullingStatTracker.getNormalizedSubsetsCount() ) )
#endif // Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS
		, tPkOutStatInfo( PkString("NumThreads"), argv[ PkGlobalData::ECmdArgs_NumThreads ] )
		, tPkOutStatInfo( PkString("NumLogicalThreadsLocalLociReconstruction"), PkStringOf( PkGlobalData::getNumLogicalThreadsLocalLociReconstruction() ) )
		, tPkOutStatInfo( PkString("NumLogicalThreadsLociIntersection"), PkStringOf( PkGlobalData::getNumLogicalThreadsLociIntersection() ) )
		, tPkOutStatInfo( PkString("NumLoci"), PkStringOf( PkGlobalData::getNumLoci() ) )
		, tPkOutStatInfo( PkString("NumLociClusters"), argv[ PkGlobalData::ECmdArgs_NumLociClusters ] )
		, tPkOutStatInfo( PkString("NumOutputSets"), PkStringOf( PkGlobalData::getGlobalSiblingSets().size() ) )
		, tPkOutStatInfo( PkString("ReconstructionAlgorithm"), argv[ PkGlobalData::ECmdArgs_LocalLociSiblingReconstructionAlgorithm ] )
		, tPkOutStatInfo( PkString("TotalTime"), PkStringOf( TO_MINUTES( ePkSTAT_TotalTime ), 2 ) )
		, tPkOutStatInfo( PkString("PhaseLocalLociReconstructionTime"), PkStringOf( TO_MINUTES( ePkSTAT_PhaseLocalLociReconstructionTime ), 2 ) )
		, tPkOutStatInfo( PkString("IntersectSiblingSetsTotalTime"), PkStringOf( TO_MINUTES( ePkSTAT_IntersectSiblingSetsTotalTime ), 2 ) )
		, tPkOutStatInfo( PkString("IntersectSiblingSetsCallCount"), PkStringOf( (PkUInt) GCounterStats[ ePkSTAT_IntersectSiblingSetsCallCount ].getCounter() ) )
		, tPkOutStatInfo( PkString("IntersectSiblingSetsBlockTime"), PkStringOf( TO_MINUTES( ePkSTAT_IntersectSiblingSetsBlockTime ), 2 ) )
		, tPkOutStatInfo( PkString("GlobalSubsetPolicyOnIntersectionJobFinishTime"), PkStringOf( TO_MINUTES( ePkSTAT_GlobalSubsetPolicyOnIntersectionJobFinishTime ), 2 ) )
		, tPkOutStatInfo( PkString("GlobalSubsetPolicyOnIntersectionLoopFinishTime"), PkStringOf( TO_MINUTES( ePkSTAT_GlobalSubsetPolicyOnIntersectionLoopFinishTime ), 2 ) )
		, tPkOutStatInfo( PkString("GlobalSubsetFrequencyMappingTime"), PkStringOf( TO_MINUTES( ePkSTAT_GlobalSubsetFrequencyMappingTime ), 2 ) )
		, tPkOutStatInfo( PkString("GlobalSubsetCullingTime"), PkStringOf( TO_MINUTES( ePkSTAT_GlobalSubsetCullingTime ), 2 ) )
		, tPkOutStatInfo( PkString("PhaseOutputResultsTime"), PkStringOf( TO_MINUTES( ePkSTAT_PhaseOutputResultsTime ), 2 ) )
		// Consensus stats
		, tPkOutStatInfo( PkString("ConsensusLociIntersectionAndOutputResultsTime"), PkStringOf( TO_MINUTES( ePkSTAT_ConsensusLociIntersectionAndOutputResultsTime ), 2 ) )
		, tPkOutStatInfo( PkString("ConsensusExecGamsAndAllScriptsTime"), PkStringOf( TO_MINUTES( ePkSTAT_ConsensusExecGamsAndAllScriptsTime ), 2 ) )
		, tPkOutStatInfo( PkString("ConsensusStrictVoteTime"), PkStringOf( TO_MINUTES( ePkSTAT_ConsensusStrictVoteTime ), 2 ) )
		, tPkOutStatInfo( PkString("ConsensusGreedyMergeGroupsTime"), PkStringOf( TO_MINUTES( ePkSTAT_ConsensusGreedyMergeGroupsTime ), 2 ) )
		, tPkOutStatInfo( PkString("ConsensusOutputSolutionTime"), PkStringOf( TO_MINUTES( ePkSTAT_ConsensusOutputSolutionTime ), 2 ) )
	};

	// Compute how many non-threaded stat infos we have
	const PkInt numNonThreadedStatInfos = sizeof( nonThreadedStatInfos ) / sizeof( tPkOutStatInfo );

	// A type for matching a string label to a threaded stat identifier
	typedef std::pair< PkString, PkUInt > tPkThreadedStatInfo;

	// The array of threaded timer stats to output
	tPkThreadedStatInfo threadedTimerStatInfos[] =
	{
		  tPkThreadedStatInfo( PkString("IntersectionTotalTime"), ePkSTAT_ThreadedIntersectionTotalTime )
		, tPkThreadedStatInfo( PkString("LocalSubsetPolicyProcessIntersectionTime"), ePkSTAT_ThreadedLocalSubsetPolicyProcessIntersectionTime )
		, tPkThreadedStatInfo( PkString("LocalSubsetPolicyOnIntersectionLoopFinishedTime"), ePkSTAT_ThreadedLocalSubsetPolicyOnIntersectionLoopFinishedTime )
	};

	// The array of threaded counter stats to output
	tPkThreadedStatInfo threadedCounterStatInfos[] =
	{
		  tPkThreadedStatInfo( PkString("IntersectionsCount"), ePkSTAT_ThreadedIntersectionsCount )
	};

	// Compute how many threaded time stat infos we have
	const PkInt numThreadedTimerStatInfos = sizeof( threadedTimerStatInfos ) / sizeof( tPkThreadedStatInfo );

	// Compute how many threaded counter stat infos we have
	const PkInt numThreadedCounterStatInfos = sizeof( threadedCounterStatInfos ) / sizeof( tPkThreadedStatInfo );

	// Convert to dynamic array so that we can append threaded stat infos
	PkArray( tPkOutStatInfo ) outStatInfos;

	// Apppend non-threaded stats
	outStatInfos.resize( numNonThreadedStatInfos );
	std::copy( nonThreadedStatInfos, nonThreadedStatInfos+numNonThreadedStatInfos, outStatInfos.begin() );

	// Now, expand threaded stats and append them
	if ( ( numThreadedTimerStatInfos + numThreadedCounterStatInfos ) > 0 ) 
	{
		// Verify that we have at least one actual threaded stat
		PkAssert( ePkSTAT_ThreadedTimerMAX >= 1 );
		PkAssert( ePkSTAT_ThreadedCounterMAX >= 1 );
		PkAssert( ( sizeof( GThreadedTimerStats ) / sizeof( PkArray( TimerStat ) ) ) == ePkSTAT_ThreadedTimerMAX );
		PkAssert( ( sizeof( GThreadedCounterStats ) / sizeof( PkArray( CounterStat ) ) ) == ePkSTAT_ThreadedCounterMAX );
		
		// Determine number of threads from first threaded stat
		const PkUInt numThreads = GThreadedTimerStats[0].size();
		// Appends stats for each thread
		for ( PkUInt itrThread=0; itrThread<numThreads; ++itrThread )
		{
			PkInt itrStat=0;
			// Append timer stats
			for ( ; itrStat<numThreadedTimerStatInfos; ++itrStat )
			{
				PkAssert( GThreadedTimerStats[ threadedTimerStatInfos[itrStat].second ].size() == numThreads );
				outStatInfos.push_back( tPkOutStatInfo(
						  // Generate a label
						  threadedTimerStatInfos[itrStat].first+PkString(":")+PkStringOf(itrThread)
						  // Generate a time value
						, PkStringOf( TO_MINUTES_THREADED( threadedTimerStatInfos[itrStat].second, itrThread ), 2 )
						) );
			}
			// Append counter stats
			for ( itrStat=0; itrStat<numThreadedCounterStatInfos; ++itrStat )
			{
				PkAssert( GThreadedCounterStats[ threadedCounterStatInfos[itrStat].second ].size() == numThreads );
				outStatInfos.push_back( tPkOutStatInfo(
						  // Generate a label
						  threadedCounterStatInfos[itrStat].first+PkString(":")+PkStringOf(itrThread)
						  // Generate a counter value
						  , PkStringOf( GThreadedCounterStats[ threadedCounterStatInfos[itrStat].second ][ itrThread ].getCounter(), 2 )
						) );
			}
		}
	}

	// The output file to dump stats to
	static const char* Pk_STATS_FILE_NAME = "PkStats.csv";

	// Open file to append stats data
	FILE* pFile = fopen( Pk_STATS_FILE_NAME, "a" );
	PkAssert( pFile );

	// If we have stats to dump, then lets dump them finally!
	if ( outStatInfos.size() > 0 )
	{
		// If file didn't previously exist, print the stats key
		if ( 0 == PkGetFileSize( *pFile ) )
		{
			outputStatsRow( &outStatInfos[0], outStatInfos.size(), pFile, tPkStatLabelSelector() );
		}

		// Write stat values
		outputStatsRow( &outStatInfos[0], outStatInfos.size(), pFile, tPkStatValueSelector() );
	}

	PkAssert( 0 == fclose( pFile ) );
}

// All the timer stat instances
TimerStat GTimerStats[ePkSTAT_TimerMAX];

// All the counter stat instances
CounterStat GCounterStats[ ePkSTAT_CounterMAX ];

// All threaded timer stat instances
PkArray( TimerStat ) GThreadedTimerStats[ePkSTAT_ThreadedTimerMAX];

// All the threaded counter stat instances
PkArray( CounterStat ) GThreadedCounterStats[ePkSTAT_ThreadedCounterMAX];

#if Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS
// Allocate our global instance for detailed stat tracking
DetailedSubsetCullingStatTracker GDetailedSubsetCullingStatTracker;
#endif // Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS

} // end of PkStats namespace

#endif // Pk_ENABLE_STATS
