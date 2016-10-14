/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkMiscUtil_h
#define PkMiscUtil_h

#include "PkBuild.h"
#include "PkTypes.h"
#include <sstream>
#include <iomanip>

/**
* @return the string version of the parameter object
*/
template <class T>
PkString PkStringOf(const T& object)
{
	std::ostringstream os;
	os << object;
	return os.str();
}

/**
* Overloaded template function that allows for precision stream modifiers
*/
template <class tFloatingPoint>
PkString PkStringOf( const tFloatingPoint object, const PkInt precision )
{
	std::ostringstream os;
	os << /*std::fixed <<*/ std::setprecision( precision ) << object;
	return os.str();
}

/**
* @return the integer version of a string
*/
inline PkInt PkAtoi( const PkString& str )
{
	std::istringstream buffer(str);
	PkInt value = 0;
	buffer >> value;
	return value;
}

/**
* @returns string to the left and to the right of parameter split marker 
*/
inline PkBool PkSplitString( const PkString& str, const PkString& strSplit, PkString& strLeft, PkString& strRight )
{
	const size_t index = str.find( strSplit );
	if ( index == PkString::npos )
	{
		return false;
	}
	strLeft = str.substr(0, index);
	const size_t dataStart = index + strSplit.length();
	strRight = str.substr( dataStart, str.length() - dataStart ); 
	return true;
}

/**
* @return path to file
*/
inline PkString PkGetPathFromFileName( const PkString& strFileName )
{
	return strFileName.substr( 0, strFileName.rfind(Pk_PATH_SEPARATOR_STRING)+1 );
}

/**
* @return filename without the extension or the path information
*/
inline PkString PkGetCleanFileName( const PkString& str )
{
	PkString workingStr = str;

	// Strip away the extension
	const size_t dotIdx = workingStr.rfind(".");
	if( dotIdx != PkString::npos )
	{
		workingStr = workingStr.substr(0,dotIdx);
	}

	// Strip away the path info for *nix
	const size_t pathSeparatorIdxNx = workingStr.rfind("/");
	if( pathSeparatorIdxNx != PkString::npos )
	{
		workingStr = workingStr.substr(pathSeparatorIdxNx+1, workingStr.length() - pathSeparatorIdxNx);
	}

	// Strip away the path info for windows
	const size_t pathSeparatorIdxWin = workingStr.rfind("\\");
	if( pathSeparatorIdxWin != PkString::npos )
	{
		workingStr = workingStr.substr(pathSeparatorIdxWin+1, workingStr.length() - pathSeparatorIdxWin);
	}

	return workingStr;
}

/**
* A utility to add a unique item to a container class
* @return - iterator to element in array
*/
#include "PkTypeTraits.h"
#include <algorithm>
template < typename tContainer >
typename tContainer::iterator Pk_push_back_unique( tContainer& container, typename Pk::param_traits< typename tContainer::value_type >::const_param_type value )
{
	const typename tContainer::iterator find_iter = std::find( container.begin(), container.end(), value );
	if ( container.end() == find_iter )
	{
		container.push_back( value );
		return container.end()-1;
	}
	return find_iter;
}

/**
* A logging macro useful for debugging
*/
#if Pk_ENABLE_LOGGING
	// Flush stdout so that batch scripts can keep us updated
	#define PkLogf( ... ) { printf(__VA_ARGS__); fflush(stdout); }
#else
	#define PkLogf( ... )
#endif

/**
* Unix doesn't have stricmp
*/
#include <string.h>
#if !WINDOWS
	#define stricmp strcasecmp
#endif

#endif // PkMiscUtil_h
