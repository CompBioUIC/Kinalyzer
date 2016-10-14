/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef WINDOWS

#include "PkFileUtilsLinux.h"
#include "PkAssert.h"
#include <sys/stat.h>

using namespace std;

PkBool PkFileUtilsLinux::makeDirectory(const string& dirPath)
{
	// -1 indicates error, anything else is success
	return -1 != mkdir(dirPath.c_str(), 0777);
}

PkInt PkFileUtilsLinux::deleteDirectory(const string& refcstrRootDirectory, bool bDeleteSubdirectories /*= true*/)
{
	if( bDeleteSubdirectories )
	{
		printf("removing directory %s\n", refcstrRootDirectory.c_str());
		// -1 indicates error, anything else is success
		return( -1 == system(string(string("rm -r ") + refcstrRootDirectory).c_str()) );
	}
	else
	{
		system(string(string("cd ") + refcstrRootDirectory).c_str());
		system("rm *");
		system("cd ..");
		// @todo: handle error conditions for this case
		return 0;
	}
}

#endif // ifndef WINDOWS
