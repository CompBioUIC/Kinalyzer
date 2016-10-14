/**
 * @author Alan Perez-Rathke 
 *
 * @date July 3, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkMiscUtil.h"

// The GAMS resource limit threshold
#define Pk_GAMS_RESLIM 1000

namespace PkGamsUtils
{

// Determines GMS output file name from base file name
void getGMSOutputFileName( PkString& outGMSFileName, const PkString& baseFileName )
{
	outGMSFileName = baseFileName + PkString( ".gms" );
}


// Creates a GMS file to be used as input to Gams
void outputGMSFile( const char* strOutGMSFileName, const PkSiblingSetBitSetArray& outputSets )
{
	// Verify parameter file name
	PkAssert( NULL != strOutGMSFileName );

	// Verify output sets exist!
	if ( outputSets.size() <= 0 )
	{
		PkLogf( "No output sets were generated!\n" );
		return;
	}

	// Verify we can obtain a file handle
	FILE* pFile = fopen( strOutGMSFileName, "w" );
	PkAssert( NULL != pFile );

	// Cache population size
	const PkInt populationSize = (PkInt)outputSets[0].size();

	// Cache number of output sets
	const PkInt numOutputSets = (PkInt)outputSets.size();

	// Begin writing gms file
	fprintf( pFile, "$ONEMPTY \n \n");
	fprintf( pFile, "SET i   sibling   /s1*s%d/; \n", populationSize );
	fprintf( pFile, "SET j   sibset    /t1*t%d/; \n \n", numOutputSets );
	fprintf( pFile, "TABLE A(j,i)    sets covered by node i \n");
	fprintf( pFile, "     ");
 
	// Output column headings
	for ( PkInt i=1; i<=populationSize; ++i)
	{
		fprintf( pFile, "\t s%d", i );
	}
	fprintf(pFile, " \n");

	// Output computed bitsets
	for ( PkInt i=0; i<numOutputSets; ++i )
	{
		// Output row heading
		fprintf( pFile, "t%d", (i+1) );

		// Verify output set has a value for every individual of the population
		PkAssert( outputSets[ i ].size() == populationSize );

		// Output bitset as row in table
		PkBitTraits<PkSiblingSetBitSetArray>::tBitSetConstHandle outputSiblingSet = outputSets[ i ];
		for ( PkInt j=0; j<populationSize; ++j )
		{
			fprintf( pFile, "\t %d", outputSiblingSet.test( j ) );
		}

		// Skip line to begin next row
		fprintf( pFile, " \n" );
	}

	// Finish outputting gms configuration
	fprintf( pFile, "; \n \n" );
	fprintf( pFile, "$OFFLISTING \n" );
	fprintf( pFile, "***** Lst File Options ***** \n" );
	fprintf( pFile, "OPTION solprint=off; \n" );
	fprintf( pFile, "OPTION sysout=off; \n" );
	fprintf( pFile, "OPTION limrow=0; \n" );
	fprintf( pFile, "OPTION limcol=0; \n \n" );
	fprintf( pFile, "BINARY VARIABLES \n" );
	fprintf( pFile, "        xset(j)    sibling j \n" );
	fprintf( pFile, "; \n" );
	fprintf( pFile, "EQUATIONS \n" );
	fprintf( pFile, "        setcov(i) \n" );
	fprintf( pFile, "	      objective \n" );
	fprintf( pFile, "; \n" );
	fprintf( pFile, "VARIABLE TF; \n\n" );
	fprintf( pFile, "setcov(i).. \n" );
	fprintf( pFile, "sum(j, A(j,i)*xset(j)) =g= 1; \n\n" );
	fprintf( pFile, "* Objective Function \n" );
	fprintf( pFile, "objective.. \n" );
	fprintf( pFile, "TF =e= sum(j, xset(j)); \n" );
	fprintf( pFile, " \n" );
	fprintf( pFile, "***** Model Definition ****** \n" );
	fprintf( pFile, "MODEL setcover / ALL /; \n" );
	fprintf( pFile, "***** Solution Options ******* \n" );
	fprintf( pFile, "OPTION lp=cplex; \n" );
	fprintf( pFile, "OPTION mip=cplex; \n" );
	fprintf( pFile, "OPTION optcr=0.01; \n" );
	fprintf( pFile, "OPTION optca=0.1; \n" );
	fprintf( pFile, "OPTION iterlim=5000; \n" );
	fprintf( pFile, "OPTION reslim=%d; \n", Pk_GAMS_RESLIM );
	fprintf( pFile, "option rmip = cplex; \n" );
	fprintf( pFile, "SOLVE setcover using mip minimizing TF; \n \n" );
	fprintf( pFile, "DISPLAY xset.l; \n" );
    
	// Close file handle
	PkVerify( 0 == fclose( pFile ) );
}

// Executes the Gams set cover solver
void execGams( const char* gamsInputFileName )
{
	PkString strGamsCmd = (PkString( "gams " ) + PkString( gamsInputFileName ));
	system( strGamsCmd.c_str() );
}

} // end of PkGamsUtils namespace
