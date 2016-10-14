/**
 * @author Alan Perez-Rathke 
 *
 * @date July 3rd, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkMiscUtil.h"
#include <fstream>
#include <sstream>

namespace PkConsensusSetCoverParser
{

// @return true if set cover file was successfully parsed, false otherwise
PkBool parseFile
(
  const char* strSetCoverFileName
, const PkUInt populationSize
, PkBitSetArray& outSiblingSets
)
{
	// Verify we have an empty output array
	PkAssert( outSiblingSets.empty() );

	// Open set cover data file
	std::ifstream inFile( strSetCoverFileName );

	while ( inFile )
	{
		// Read line in set cover data file
		PkString line;
		std::getline( inFile, line );

		// Termine loop if we read the last empty line
		if ( !inFile )
		{
			break;
		}

		// Initialize a new sibling bit set
		outSiblingSets.push_back( PkBitSet() );
		PkBitSet& siblingSet = outSiblingSets.back();
		siblingSet.resize( populationSize, false );

		// Eat up set name prefix
		std::stringstream lineStream( line );
		PkString setNamePrefix;
		std::getline( lineStream, setNamePrefix, '\t' );
		
		// Eat up white space padding prefix
		char whiteSpacePaddingPrefix[3];
		lineStream.get( whiteSpacePaddingPrefix, sizeof( whiteSpacePaddingPrefix ) / sizeof( char ) );

		// Now, we should have a list of numbers remaining in the line, parse each one
		while ( lineStream )
		{
			// Extract number string from line stream
			PkString strIndividualIndex;
			std::getline( lineStream, strIndividualIndex, ' ' );

			// Terminate loop if we're at the end of the line string
			if ( !lineStream )
			{
				break;
			}

			// Convert number string to actual integer
			const PkInt individualIndex = PkAtoi( strIndividualIndex );
			PkAssert( individualIndex >= 0 );
			PkAssert( (PkUInt)individualIndex < populationSize );

			// Flip the bit for current set
			siblingSet.set( individualIndex, true );
		}

		// Assert this set contains individuals
		PkAssert( siblingSet.any() );
	}

	// Close set cover data file
	inFile.close();

	// Return true if we actually loaded some sets
	return ( outSiblingSets.size() > 0 );
}

} // end of PkConsensusSetCoverParser namespace
