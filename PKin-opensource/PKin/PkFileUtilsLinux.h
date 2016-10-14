/**

 * @author Alan Perez-Rathke 

 *

 * @date February 20, 2010

 *

 * Department of Computer Science

 * University of Illinois at Chicago 

 */



#ifndef PkFileUtilsLinux_h

#define PkFileUtilsLinux_h



#ifndef WINDOWS



#include "PkTypes.h"



namespace PkFileUtilsLinux

{

	/**

	* Creates a directory with the specified relative path

	*/

	extern PkBool makeDirectory(const PkString& dirPath);



	/**

	* Delete the directory and all the files in it, optionally recurses to subdirectories

	*/

	extern PkInt deleteDirectory(const PkString& refcstrRootDirectory, bool bDeleteSubdirectories=true);

}



#endif // ifndef WINDOWS



#endif //PkFileUtilsLinux_h

