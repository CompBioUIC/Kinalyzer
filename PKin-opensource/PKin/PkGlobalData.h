/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkGlobalData_h
#define PkGlobalData_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkIndividual.h"
#include "PkThreadPool.h"
#include "PkPhases.h"

// Global data is bad in multi-threaded applications!
// But, sometimes you have to bite the bullet
namespace PkGlobalData
{
	// Enumerated commandline arguments
	enum ECmdArgs
	{
	  // Executable name
	  ECmdArgs_ExeName=0
	  // Application to run: e.g. consensus or 2allele
	, ECmdArgs_AppName
	  // Path to population file
	, ECmdArgs_PopulationPath
	  // Number of threads to test
	, ECmdArgs_NumThreads
	  // Number of logical threads for local loci reconstruction
	, ECmdArgs_NumLogicalThreadsLocalLociReconstruction
	  // Number of logical threads for loci intersection
	, ECmdArgs_NumLogicalThreadsLociIntersection
	  // Stack size of each pool thread in kilobytes
	, ECmdArgs_StackSizeKb
	  // Which reconstruction algorithm to use
	, ECmdArgs_LocalLociSiblingReconstructionAlgorithm
	  // The number of loci clusters to use, can be used to produce loci level parallelism
	, ECmdArgs_NumLociClusters
	  // The name of the file to output results to
	, ECmdArgs_OutputFileName
	  // Max number of arguments
	, ECmdArgs_MaxArgs
	};

	// @return - collection of all individuals in the population
	extern const PkArray(PkIndividual)& getPopulation();	

	// @return - global thread pool instance for running jobs on
	extern PkThreadPool& getThreadPool();

	// @return - number of logical threads to work with for local loci reconstruction
	extern PkInt getNumLogicalThreadsLocalLociReconstruction();

	// @return - number of logical threads to work with for local loci reconstruction
	extern PkInt getNumLogicalThreadsLociIntersection();

	// @return - number of loci at each individual of the population
	extern PkInt getNumLoci();

	// @return - the output of local loci reconstruction phase
	// All local sibling groups encoded as bitfields
	extern PkArray( PkSiblingSetBitSetArray )& getEncodedSiblingSetPartitions(); 

	// @return - the output of the loci intersection phase
	// All global feasible sibling sets encoded as bitfields
	extern PkSiblingSetBitSetArray& getGlobalSiblingSets();

	// @return - the reconstruction algorithm to use for the local loci reconstruction phase
	extern EPkLocalLociReconstructionAlgorithm getLocalLociReconstructionAlgorithm();

	// @return - the number of clusters to partition the loci in, used to produce loci level parallelism
	extern PkInt getNumLociClusters();

	// @return - the name of the file to output results to
	extern const char* getOutputFileName();

	// Initializes global data, should be called at start-up
	extern void init( const PkInt argc, char** argv );
} // end of PkGlobalData namespace

#endif // PkGlobalData_h
