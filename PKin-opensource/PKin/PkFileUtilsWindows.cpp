/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#if WINDOWS

#include "PkFileUtilsWindows.h"
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <direct.h> // for _mkdir

PkBool PkFileUtilsWindows::makeDirectory(const PkString& dirPath)
{
	return -1 != _mkdir(dirPath.c_str());
}

PkInt PkFileUtilsWindows::deleteDirectory(const PkString& refcstrRootDirectory, bool bDeleteSubdirectories /*= true*/)
{
	bool            bSubdirectory = false;       // Flag, indicating whether
												   // subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	PkString     strFilePath;                 // Filepath
	PkString     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = refcstrRootDirectory + "\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bDeleteSubdirectories)
					{
						// Delete subdirectory
						PkInt iRC = PkFileUtilsWindows::deleteDirectory(strFilePath, bDeleteSubdirectories);
						if(iRC)
						  return iRC;
					}
					else
						bSubdirectory = true;
				}
				else
				{
					// Set file attributes
					if(::SetFileAttributes(strFilePath.c_str(),
										 FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if(::DeleteFile(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if(!bSubdirectory)
			{
				// Set directory attributes
				if(::SetFileAttributes(refcstrRootDirectory.c_str(),
								   FILE_ATTRIBUTE_NORMAL) == FALSE)
				return ::GetLastError();

			// Delete directory
			if(::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
				return ::GetLastError();
			}
		}
	}

	return 0;
}

#endif // WINDOWS
