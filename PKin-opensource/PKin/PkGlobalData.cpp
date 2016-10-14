/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkGlobalData.h"
#include "PkPopulationLoader.h"
#include "PkAssert.h"
#include "PkMiscUtil.h"
#include "PkLociArray.h"
#include "PkStats.h"

// Global data is bad in multi-threaded applications!
// But, sometimes you have to bite the bullet
namespace PkGlobalData
{

// All individuals in the population
PkArray(PkIndividual) m_Population;

// The global thread pool instance for running jobs on
PkThreadPool m_ThreadPool;

// The number of logical threads to work with for local loci reconstruction
PkInt m_NumLogicalThreadsLocalLociReconstruction = 0;

// The number of logical threads to work with for local loci reconstruction
PkInt m_NumLogicalThreadsLociIntersection = 0;

// The number of loci at each individual of the population
PkInt m_NumLoci = 0;

// All sibling groups at each locus encoded as bitfields
PkArray( PkSiblingSetBitSetArray ) m_EncodedSiblingSetPartitions;

// All global feasible sibling sets encoded as bitfields
PkSiblingSetBitSetArray m_GlobalSiblingSets;

// The local loci reconstruction algorithm as specified in the command line
EPkLocalLociReconstructionAlgorithm m_LocalLociReconstructionAlgorithm = EPkReconAlg_Island;

// The number of clusters to partition the loci in, used to produce loci level parallelism
PkInt m_NumLociClusters = 0;

// The name of the file to output results to
char* m_strOutputFileName = NULL;

// True if we're using consensus, false otherwise
PkBool m_bIsConsensus = false;

// @return - collection of all individuals in the population
const PkArray(PkIndividual)& getPopulation()
{
	return m_Population;
}

// @return - global thread pool instance for running jobs on
PkThreadPool& getThreadPool()
{
	return m_ThreadPool;
}

// @return - number of logical threads to work with for local loci reconstruction
PkInt getNumLogicalThreadsLocalLociReconstruction()
{
	PkAssert( m_NumLogicalThreadsLocalLociReconstruction > 0 );
	return m_NumLogicalThreadsLocalLociReconstruction;
}

// @return - number of logical threads to work with for local loci reconstruction
PkInt getNumLogicalThreadsLociIntersection()
{
	return m_NumLogicalThreadsLociIntersection;
}

// @return - number of loci at each individual of the population
PkInt getNumLoci()
{
	return m_NumLoci;
}

// @return - the output of local loci reconstruction phase
// All local sibling groups encoded as bitfields
PkArray( PkSiblingSetBitSetArray )& getEncodedSiblingSetPartitions()
{
	return m_EncodedSiblingSetPartitions;
}

// @return - the output of the loci intersection phase
// All global feasible sibling sets encoded as bitfields
PkSiblingSetBitSetArray& getGlobalSiblingSets()
{
	return m_GlobalSiblingSets;
}

// @return - the reconstruction algorithm to use for the local loci reconstruction phase
EPkLocalLociReconstructionAlgorithm getLocalLociReconstructionAlgorithm()
{
	return m_LocalLociReconstructionAlgorithm;
}

// @return - the number of clusters to partition the loci in, used to produce loci level parallelism
PkInt getNumLociClusters()
{
	return m_NumLociClusters;
}

// @return - the name of the file to output results to
const char* getOutputFileName()
{
	return m_strOutputFileName;
}

// Initializes global data, should be called at start-up
void init( const PkInt argc, char** argv )
{
	// Assert we have the proper number of arguments
	PkAssert( ECmdArgs_MaxArgs == argc );

	// Load population file from disk
	PkLogf( "Loading population file %s...\n", argv[ ECmdArgs_PopulationPath ] );
	if ( !PkPopulationLoader::load( argv[ ECmdArgs_PopulationPath ], m_Population ) )
	{
		PkLogf( "Failed to load population file %s!\n", argv[ ECmdArgs_PopulationPath ] );
		exit( -1 );
	}
	PkLogf( "Population size: %u\n", m_Population.size() );

	// Determine number of loci from first individual in population
	PkAssert( m_Population.size() > 0 );
	m_NumLoci = (PkInt)m_Population[ 0 ].size();
	PkLogf( "Number of loci: %d\n", m_NumLoci );

	// Verify that we can work with this number of loci
#if Pk_ENABLE_SMALL_LOCI_ARRAY
	if ( m_NumLoci > Pk_MAX_LOCI_ARRAY_SIZE )
	{
		PkLogf( "%d loci exceeds the maximum allowed (%d) by this build.\nRecompile with a larger set or switch to dynamic loci arrays!\n", m_NumLoci, Pk_MAX_LOCI_ARRAY_SIZE );
		exit( -1 );
	}
#endif

	// Initialize thread pool with parameter number of threads
	const PkUInt numThreads = atoi( argv[ ECmdArgs_NumThreads ] );
	const PkUInt stackSizeBytes = 1024 * atoi( argv[ ECmdArgs_StackSizeKb ] );
	m_ThreadPool.initialize( numThreads, stackSizeBytes );
	PkLogf( "Number of threads: %u\n", numThreads );
	PkLogf( "Thread stack size (bytes): %u\n", stackSizeBytes );

	// Initialize per-thread stats (must come after thread pool initialization)
	Pk_STATS_INIT_THREADED();

	// Initialze the logical number of threads we're working with for local loci reconstruction
	m_NumLogicalThreadsLocalLociReconstruction = atoi( argv[ ECmdArgs_NumLogicalThreadsLocalLociReconstruction ] );	
	if ( m_NumLogicalThreadsLocalLociReconstruction <= 0 )
	{
		PkLogf( "Number of logical threads for local loci reconstruction: %d must be greater than 0! ...exiting\n", m_NumLogicalThreadsLocalLociReconstruction );
		exit( -1 ); 
	}
	PkLogf( "Number of logical threads for local loci reconstruction: %d\n", m_NumLogicalThreadsLocalLociReconstruction );

	// Initialze the logical number of threads we're working with for loci intersection
	m_NumLogicalThreadsLociIntersection = atoi( argv[ ECmdArgs_NumLogicalThreadsLociIntersection ] );	
	PkLogf( "Number of logical threads for loci intersection: %s\n", m_NumLogicalThreadsLociIntersection <= 0 ? "dynamic" : argv[ ECmdArgs_NumLogicalThreadsLociIntersection ] );

	// Initialize our local loci reconstruction algorithm
	m_LocalLociReconstructionAlgorithm =
		// Ugly code to clamp user input value to range of allowed enumerations
		(EPkLocalLociReconstructionAlgorithm) std::min<PkInt>(
			  EPkReconAlg_NumAlgorithms-1
			, std::max<PkInt>( EPkReconAlg_MinVal, atoi( argv[ ECmdArgs_LocalLociSiblingReconstructionAlgorithm ] ) ) 
			);
	PkLogf( "Local loci reconstruction algorithm: %d\n", m_LocalLociReconstructionAlgorithm );

	// Initialize the number of loci clusters to use
	m_NumLociClusters = atoi( argv[ ECmdArgs_NumLociClusters ] );
	PkLogf( "Number of loci clusters: %d\n", m_NumLociClusters );

	// Store output file name
	m_strOutputFileName = argv[ ECmdArgs_OutputFileName ];
	PkLogf( "Output file name: %s\n", m_strOutputFileName );

	// Verify we have enough loci for each cluster
	if ( m_NumLociClusters > m_NumLoci )
	{
		PkLogf( "Number of loci clusters (%d) cannot exceed number of loci (%d) - Exiting.\n", m_NumLociClusters, m_NumLoci );
		exit( -1 );
	}

	// Verify we have enough threads for each cluster
	if ( m_NumLociClusters > m_NumLogicalThreadsLocalLociReconstruction )
	{
		PkLogf( "Number of loci clusters (%d) cannot exceed number of logical threads (%d) - Exiting.\n", m_NumLociClusters, m_NumLogicalThreadsLocalLociReconstruction );
		exit( -1 );
	}
}

} // end of PkGlobalData namespace
