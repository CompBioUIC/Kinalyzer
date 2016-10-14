/**
 * @author Alan Perez-Rathke 
 *
 * @date April 12, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkPhases.h"
#include "PkTypes.h"
#include "PkMiscUtil.h"
#include "PkGamsUtils.h"
#include "PkStats.h"

namespace
{
	// Write sets file to disk
	void outputSetsFile( const char* strOutFileName, const PkSiblingSetBitSetArray& outputSets )
	{
		// Verify parameter file name
		PkAssert( NULL != strOutFileName );

		// Verify output sets exist!
		if ( outputSets.size() <= 0 )
		{
			PkLogf( "No output sets were generated!\n" );
			return;
		}

		// Verify we can obtain a file handle
		FILE* pFile = fopen( strOutFileName, "w" );
		PkAssert( NULL != pFile );

		// Cache population size
		const PkInt populationSize = (PkInt)outputSets[0].size();

		// Cache number of output sets
		const PkInt numOutputSets = (PkInt)outputSets.size();

		// Output computed bitsets
		for ( PkInt i=0; i<numOutputSets; ++i )
		{
			// Output row heading
			fprintf( pFile, "\"Set(%d):\"", i );

			// Verify output set has a value for every individual of the population
			PkAssert( outputSets[ i ].size() == populationSize );

			// Output bitset as row in table
			PkBitTraits<PkSiblingSetBitSetArray>::tBitSetConstHandle outputSiblingSet = outputSets[ i ];
			for ( PkInt j=0; j<populationSize; ++j )
			{
				fprintf( pFile, "\t%d", outputSiblingSet.test( j ) );
			}

			// Skip line to begin next row
			fprintf( pFile, "\n" );
		}

		// Close file handle
		PkVerify( 0 == fclose( pFile ) );
	}
}

namespace PkPhase
{

// Writes output to data file
void outputResults( const char* strOutFileName, const PkSiblingSetBitSetArray& outputSets )
{
	PkLogf( "Outputting results (%u sets generated)...\n", outputSets.size() );
	Pk_SCOPED_STAT_TIMER( ePkSTAT_PhaseOutputResultsTime );
		
	// Verify parameter file name
	PkAssert( NULL != strOutFileName );

	// Verify output sets exist!
	if ( outputSets.size() <= 0 )
	{
		PkLogf( "No output sets were generated!\n" );
		return;
	}
	
	// Write GMS file to disk
	PkString strOutGMSFileName;
	PkGamsUtils::getGMSOutputFileName( strOutGMSFileName, PkString( strOutFileName ) );
	PkGamsUtils::outputGMSFile( strOutGMSFileName.c_str(), outputSets );

	// Write sets file to disk
	outputSetsFile( strOutFileName, outputSets );

	PkLogf( "Output results finished\n" );
}

} // end of PkPhase namespace
