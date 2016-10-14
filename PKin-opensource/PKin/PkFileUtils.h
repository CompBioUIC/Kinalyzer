/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkFileUtils_h
#define PkFileUtils_h

/**
* Platform independent APIs for various file and directory operations
*/

#include "PkTypes.h"

/**
* @return size of the file in bytes, requires the file to be already open
* uses ftell to seek to the end of the, will seek back to the original position when done
*/
extern long PkGetFileSize( FILE& file );

/**
* @return true if the input file has been moved/renamed successfully to the outputFile
* requires the outputFilePath directory to exist
*/
extern PkBool PkMoveFile( const PkString& inputFilePath, const PkString& outputFilePath );

/**
* @return true if the file has been deleted
*/
extern PkBool PkDeleteFile( const PkString& filePath );

/**
* Creates a directory with the specified relative path
*/
extern void PkMakeDirectory(const PkString& dirPath, const bool bNoFail=true);

/**
* Delete the directory and all the files in it, optionally recurses to subdirectories
*/
extern void PkDeleteDirectory(const PkString& refcstrRootDirectory, bool bDeleteSubdirectories=true);

#endif //PkFileUtils_h
