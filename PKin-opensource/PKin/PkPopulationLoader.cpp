/**
 * @author Alan Perez-Rathke 
 *
 * @date March 6, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkMiscUtil.h"
#include "PkPopulationLoader.h"
#include "csvparser.h"
#include <fstream>

// The character used for dividing alleles in polyploid populations
#define Pk_ALT_ALLELE_DIVIDER_CHAR '/'

// The character used for dividing alleles in CSV format data
#define Pk_CSV_DIVIDER_CHAR ','

namespace
{

/**
* @return number of occurences of a character within the string
*/
PkInt getCharCountInString( const PkString& str, const char findChar )
{
	PkInt outCount = 0;
	// Determine number of alleles by counting the allele divider character
	for ( PkString::const_iterator strItr = str.begin(); strItr != str.end(); ++strItr )
	{
		outCount += ( findChar == *strItr );
	}
	return outCount;
}

/**
* @return true if allele count is greater than zero, false otherwise
*/
PkBool getAlleleCountAlt( std::ifstream& inFile, PkInt& outAlleleCount )
{
	// Extract first line
	PkString firstLine;
	std::getline( inFile, firstLine );

	// Determine number of alleles by counting the allele divider character
	outAlleleCount = getCharCountInString( firstLine, Pk_ALT_ALLELE_DIVIDER_CHAR );

	// Rewind inFile to beginning so we can parse everything uniformly
	inFile.seekg( 0, std::ios::beg );
	return outAlleleCount > 0;
}

/**
* @return true if allele count is greater than zero, false otherwise
*/
PkBool getAlleleCountCSV( std::ifstream& inFile, PkInt& outAlleleCount )
{
	// Extract first line
	PkString firstLine;
	std::getline( inFile, firstLine );

	// Determine number of alleles by counting the allele divider character
	outAlleleCount = getCharCountInString( firstLine, Pk_CSV_DIVIDER_CHAR ) / 2;

	// Rewind inFile to beginning so we can parse everything uniformly
	inFile.seekg( 0, std::ios::beg );
	return outAlleleCount > 0;
}

/**
* @return true if data file is in CSV format
*/
PkBool getIsCSVFormat( std::ifstream& inFile )
{
	PkInt dummy=0;
	return getAlleleCountCSV( inFile, dummy );
}

/**
* Parses a population file in the 'alt' format
*/
PkBool loadAlt( std::ifstream& inFile, PkArray(PkIndividual)& outPopulation, const PkUInt populationSizeHint )
{
	// Verify we found some alleles
	PkInt alleleCount = 0;
	PkVerify( getAlleleCountAlt( inFile, alleleCount ) );

	// Temporary strings to store our parsed data
	PkString strToken, strAlleleX, strAlleleY;
	
	// Convert our divider character into string format
	const PkString strSplitMarker( PkStringOf( Pk_ALT_ALLELE_DIVIDER_CHAR ) );

	// Parse data file for each individual's alleles
	while ( inFile.good() )
	{
		// Eat up name of individual, not sure where this would be used yet
		inFile >> strToken;
		
		// Check if we reached end of file
		if ( !inFile.good() )
		{
			break;
		}

		// Reserve a new individual
		outPopulation.push_back( PkIndividual() );
		outPopulation.back().reserve( alleleCount );

		// Read allele pairs and convert to integers
		for ( PkInt tokenItr = 0; tokenItr < alleleCount; ++tokenItr )
		{
			inFile >> strToken;
			PkAssert( inFile.good() );
			PkVerify( PkSplitString( strToken, strSplitMarker, strAlleleX, strAlleleY ) );
			PkLocus locus( PkAtoi( strAlleleX ), PkAtoi( strAlleleY ) );
			// Sort alleles by putting the minimum allele first
			if ( locus.first > locus.second ) { std::swap( locus.first, locus.second ); }
			PkAssert( locus.first <= locus.second );
			outPopulation.back().push_back( locus );
		}
	}

	// Return true if we loaded some individuals
	return outPopulation.size() > 0;
}

/**
* Parses a population file in CSV format
*/
PkBool loadCSV( std::ifstream& inFile, PkArray(PkIndividual)& outPopulation, const PkUInt populationSizeHint )
{
	// Reset the population array and reserve an initial size
	outPopulation.clear();
	outPopulation.reserve( populationSizeHint );

	// Verify we found some alleles
	PkInt alleleCount = 0;
	PkVerify( getAlleleCountCSV( inFile, alleleCount ) );

	// Temporary strings to store our parsed data
	PkString strLine, strName, strAlleleX, strAlleleY;
	
	// A parser object to parse csv data
	CSVParser csvParser;

	// Parse data file for each individual's alleles
	while ( inFile.good() )
	{
		// Extract line from data file
		std::getline( inFile, strLine );

		// Check if we reached end of file
		if ( !inFile.good() )
		{
			break;
		}
		
		// Make sure line isn't empty
		if ( strLine == "" )
		{
			continue;
		}

		// Reserve a new individual
		outPopulation.push_back( PkIndividual() );
		outPopulation.back().reserve( alleleCount );

		// Feed line to csv parser
		csvParser << strLine;

		// Eat up individual name
		csvParser >> strName;

		// Load alleleles for individual		
		for ( PkInt i=0; i<alleleCount; ++i )
		{
			// Extract allele pair
			csvParser >> strAlleleX >> strAlleleY;
			// Create a locus structure from parsed alleles
			PkLocus locus( PkAtoi( strAlleleX ), PkAtoi( strAlleleY ) );
			// Sort alleles by putting the minimum allele first
			if ( locus.first > locus.second ) { std::swap( locus.first, locus.second ); }
			PkAssert( locus.first <= locus.second );
			outPopulation.back().push_back( locus );
		}
	}

	// Return true if we loaded some individuals
	return outPopulation.size() > 0;
}


} // end of unnamed namespace

namespace PkPopulationLoader
{

/**
* @return true if population successfully loaded from file, false otherwise
*/
PkBool load( const char* fileName, PkArray(PkIndividual)& outPopulation, const PkUInt populationSizeHint )
{
	// Reset the population array and reserve an initial size
	outPopulation.clear();
	outPopulation.reserve( populationSizeHint );

	// Open our data file for parsing
	std::ifstream inFile( fileName, std::ifstream::in );
	
	// Our return value - true if we loaded data, false otherwise
	PkBool bResult = false;

	// Determine if we're in CSV format
	if ( getIsCSVFormat( inFile ) )
	{
		bResult = loadCSV( inFile, outPopulation, populationSizeHint );
	}
	else // else, default to loading 'alt' format
	{
		bResult = loadAlt( inFile, outPopulation, populationSizeHint );
	}

	// Close our data
	inFile.close();

	return bResult;
}

} // end of PkPopulationLoader namespace
