/**
 * @author Alan Perez-Rathke 
 *
 * @date July 3, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkGamsUtils_h
#define PkGamsUtils_h

#include "PkBuild.h"
#include "PkTypes.h"

namespace PkGamsUtils
{
	// Determines GMS output file name from base file name
	extern void getGMSOutputFileName( PkString& outGMSFileName, const PkString& baseFileName );

	// Creates a GMS file to be used as input to Gams
	extern void outputGMSFile( const char* strOutGMSFileName, const PkSiblingSetBitSetArray& outputSets );

	// Executes the Gams set cover solver
	extern void execGams( const char* gamsInputFileName );

} // end of PkGamsUtils namespace

#endif // PkGamsUtils_h
