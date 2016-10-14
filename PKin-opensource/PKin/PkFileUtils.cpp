/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkFileUtils.h"
#include "PkAssert.h"
#include "PkStats.h"
#include "stdio.h"

#if WINDOWS
	#include "PkFileUtilsWindows.h"
	#define PkFileUtilsPlatform PkFileUtilsWindows
#else
	#include "PkFileUtilsLinux.h"
	#define PkFileUtilsPlatform PkFileUtilsLinux
#endif

long PkGetFileSize( FILE& file )
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_FileIO );
	const long currOffset = ftell(&file); // store the current position in file
	PkAssert( currOffset >= 0 );

    fseek(&file, 0, SEEK_END);            // seek to the end of the file
    const long size = ftell(&file);       // read the current position to get the size of the file
	PkAssert( size >= 0 );

	fseek(&file, currOffset, SEEK_SET);   // seek back to the original position
	return size;
}

/**
* @return true if the input file has been moved/renamed successfully to the outputFile
* requires the outputFilePath directory to exist
*/
PkBool PkMoveFile( const PkString& inputFilePath, const PkString& outputFilePath )
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_MoveFilesTime );
	PkVerify( 0 == rename( inputFilePath.c_str(), outputFilePath.c_str() ) );	
	return true;
}

/**
* @return true if the file has been deleted
*/
PkBool PkDeleteFile( const PkString& filePath )
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_DeleteFilesTime );
	PkVerify( 0 == remove( filePath.c_str() ) );
	return true;
}

void PkMakeDirectory(const PkString& dirPath, const bool bNoFail)
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_MakeDirectoriesTime );
	PkVerify( PkFileUtilsPlatform::makeDirectory(dirPath) );
}

void PkDeleteDirectory(const PkString &refcstrRootDirectory, bool bDeleteSubdirectories )
{
	Pk_SCOPED_STAT_TIMER( ePkSTAT_DeleteDirectoriesTime );
	PkVerify( 0 == PkFileUtilsPlatform::deleteDirectory(refcstrRootDirectory, bDeleteSubdirectories ) );
}
