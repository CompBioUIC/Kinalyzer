/**
 * @author Alan Perez-Rathke 
 *
 * @date July 12th, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkConsensusSiblingGroup.h"
#include "PkStats.h"

namespace PkConsensus
{

// Writes the final consensus solution to disk
void outputSolution
( 
  const char* strFileName
, const PkArray( PkConsensusSiblingGroup )& siblingGroups 
)
{
	PkLogf( "Outputting solution file: %s for %u groups\n", strFileName, siblingGroups.size() );
	Pk_SCOPED_STAT_TIMER( ePkSTAT_ConsensusOutputSolutionTime );

	PkAssert( strFileName );
	FILE* pFile = fopen( strFileName, "w" );
	PkAssert( pFile );

	for ( PkUInt itrGroup=0; itrGroup<siblingGroups.size();  ++itrGroup )
	{
		fprintf( pFile, "\"Set(%u):\"", itrGroup );
		
		const PkBitSet& individuals = siblingGroups[ itrGroup ].getAssignedIndividuals();

		for 
		( 
		  PkBitSet::size_type idxIndividual  = individuals.find_first()
		; idxIndividual != PkBitSet::npos
		; idxIndividual = individuals.find_next( idxIndividual )
		)
		{
			fprintf( pFile, " %u", idxIndividual );
		}

		fprintf( pFile, "\n" );
	}

	PkVerify( 0 == fclose( pFile ) );
}

} // end of PkConsensus namespace
