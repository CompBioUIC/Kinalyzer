/**
 * @author Alan Perez-Rathke 
 *
 * @date July 3, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkConsensus.h"
#include "PkGlobalData.h"
#include "PkGamsUtils.h"
#include "PkStats.h"

// Utility functions for running support scripts
namespace
{

// Obtains the file name for input to the extract solution script
void getExtractSolutionScriptInputFileName( PkString& outFileName, const PkString& baseFileName )
{
	outFileName = baseFileName + PkString( ".lst" );
}

// Obtains output file name from running extract solution script
void getExtractSolutionScriptOutputFileName( PkString& outFileName, const PkString& baseFileName )
{
	outFileName = baseFileName + PkString( ".sol" );
}

// Runs the extract solution script
void execExtractSolutionScript( const char* strInputFileName )
{
	PkString extractSolutionParamCmd = PkString( "perl extract-solution-param.pl " ) + PkString( strInputFileName );
	system( extractSolutionParamCmd.c_str() );
}

// Runs the write solution script
void execWriteSolutionScript( const char* strBaseFileName, const char* strExtractSolutionFileName, const char* strOutFileName )
{
	PkString writeSolutionCmd = PkString( "perl write-solution.pl -i " ) + PkString( strBaseFileName ) + 
		PkString( " -s " ) + PkString( strExtractSolutionFileName ) + PkString( " -o " ) + PkString( strOutFileName );
	system( writeSolutionCmd.c_str() );
}

// Directory manipulation utilities
#if WINDOWS
	#include <direct.h>
	#include <stdlib.h>
	#define PkGetCwd _getcwd
	#define PkChDir _chdir
	#define Pk_MAX_PATH_LEN _MAX_PATH
#elif UNIX
	#include <unistd.h>
	#include <sys/param.h>
	#define PkGetCwd getcwd
	#define PkChDir chdir
	#define Pk_MAX_PATH_LEN MAXPATHLEN
#else
	#error "Unrecognized platform!"
#endif

// Runs gams, extract solution, and write solution scripts
void execGamsAndAllScriptsFor( const PkUInt droppedLocus, const PkString& baseFileName )
{
	// Run Gams set cover solver
	const PkString intermediateDir( PkGetPathFromFileName( baseFileName ) );
	// HACK: Set working directory to intermediate path
	char strWorkingDir[ Pk_MAX_PATH_LEN ];
	if ( !intermediateDir.empty() ) 
	{ 
		PkGetCwd( strWorkingDir, sizeof( strWorkingDir )/sizeof( char ) );
		PkChDir( intermediateDir.c_str() );
	}
	PkString strGMSFileName;
	PkGamsUtils::getGMSOutputFileName( strGMSFileName, baseFileName );
	PkGamsUtils::execGams( strGMSFileName.c_str() );
	if ( !intermediateDir.empty() ) 
	{
		PkChDir( strWorkingDir );
	}


	// Run extract solution script
	PkString strLstFileName;
	getExtractSolutionScriptInputFileName( strLstFileName, baseFileName );
	execExtractSolutionScript( strLstFileName.c_str() );

	// Run write solution script
	PkString strSolFileName;
	getExtractSolutionScriptOutputFileName( strSolFileName, baseFileName );
	PkString strSolnFileName;
	PkConsensus::getWriteSolutionScriptOutputFileName( strSolnFileName, baseFileName );
	execWriteSolutionScript( baseFileName.c_str(), strSolFileName.c_str(), strSolnFileName.c_str() );
}

} // end of unnamed namespace

namespace PkConsensus
{

// Obtains the file name that is the result of running the write solution script
void getWriteSolutionScriptOutputFileName( PkString& outFileName, const PkString& baseFileName )
{
	outFileName = baseFileName + PkString( ".soln" );
}

// Runs Gams, extract solution, and write solution on all intermediate results
void execGamsAndAllScripts()
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_ConsensusExecGamsAndAllScriptsTime );

	// Store number of loci
	const PkUInt numLoci = PkGlobalData::getNumLoci();
	
	// Drop out each allele
	for ( PkUInt itrLocus=0; itrLocus<numLoci; ++itrLocus )
	{
		// Compute intersection file name as base
		PkString intersectionFileName;
		PkConsensus::getIntersectionOutputFileName( intersectionFileName, itrLocus );
	
		PkLogf( "\tExecuting GAMS and scripts for %s\n", intersectionFileName.c_str() ); 

		// Compute set cover and extract it using scripts
		execGamsAndAllScriptsFor( itrLocus, intersectionFileName );
	}
}

} // end of PkConsensus namespace
