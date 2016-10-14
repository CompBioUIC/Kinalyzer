/**
 * @author Alan Perez-Rathke 
 *
 * @date July 13, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkConsensus.h"
#include "PkGlobalData.h"

namespace PkConsensus
{
	// Main entry point for consensus application
	int Main( const PkInt argc, char** argv )
	{
		// Initialize global data from commandline
		PkGlobalData::init( argc, argv );

		// Verify that we can handle consensus
		if ( PkGlobalData::getNumLoci() != PkGlobalData::getNumLociClusters() )
		{
			PkLogf(
				  "Currently, loci clusters %d must equal number of loci %d for consensus.  Exiting.\n"
				, PkGlobalData::getNumLociClusters()
				, PkGlobalData::getNumLoci()
				);
			exit( -1 );
		}

		PkLogf( "Running consensus application for %s\n", argv[ PkGlobalData::ECmdArgs_PopulationPath ] );

		// Reconstruct possible sibling groups locally at each locus
		PkPhase::localLociReconstruction();

		// Intersect across all loci but compute all solutions with a single loci dropped out
		PkConsensus::lociIntersectionAndOutputResults();

		// Compute set cover and extract it using scripts
		PkConsensus::execGamsAndAllScripts();

		// Run a strict consensus
		PkBitSetArray bitSetGroups;
		PkConsensus::strictVote( bitSetGroups );

		// Merge groups
		PkArray( PkConsensusSiblingGroup ) siblingGroups;
		PkConsensus::greedyMergeGroups( siblingGroups, bitSetGroups );

		// Write results
		const PkString strConsensusOutputFileName = PkString( PkGlobalData::getOutputFileName() ) + PkString( "_consensus" );
		PkConsensus::outputSolution( 
			  strConsensusOutputFileName.c_str()
			, siblingGroups
			);

		// Return 0 to signify we completed successfully
		return 0;
	}

} // end of PkConsensus namespace
