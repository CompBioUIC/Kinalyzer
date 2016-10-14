/**
 * @author Alan Perez-Rathke 
 *
 * @date June 27, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkConsensus_h
#define PkConsensus_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkConsensusSiblingGroup.h"

namespace PkConsensus
{
	// Main entry point for consensus application
	extern int Main( const PkInt argc, char** argv );

	// Generates each single allele drop out solution.
	// Assumes that a full loci decomposition was used prior
	// Intersect across all loci to determine feasible sibling sets
	// Computes several solutions using consensus metho
	extern void lociIntersectionAndOutputResults();

	// Runs Gams, extract solution, and write solution on all intermediate results
	extern void execGamsAndAllScripts();

	// Performs a strict consensus vote for all resulting set covers
	extern void strictVote( PkBitSetArray& outGroups );

	// Greedily merges groups
	extern void greedyMergeGroups( PkArray( PkConsensusSiblingGroup )& outSiblingGroups, const PkBitSetArray& groupsToMerge );

	// Writes the final consensus solution to disk
	extern void outputSolution( const char* strFileName, const PkArray( PkConsensusSiblingGroup )& siblingGroups );

	// Obtains the filename for the solution with parameter dropped locus
	extern void getIntersectionOutputFileName( PkString& outFileName, const PkUInt droppedLocus );

	// Obtains the file name that is the result of running the write solution script
	extern void getWriteSolutionScriptOutputFileName( PkString& outFileName, const PkString& baseFileName );

} // end of PkConsensus namespace

#endif // PkConsensus_h

