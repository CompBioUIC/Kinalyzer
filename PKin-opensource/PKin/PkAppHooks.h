/**
 * @author Alan Perez-Rathke 
 *
 * @date July 13, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

// Implementation of commandlet hooks extracted to separate file
// to avoid cluttering PkMain.cpp

#ifndef PkAppHooks_h
#define PkAppHooks_h

#include "PkBuild.h"

// Application info structure
struct PkAppInfo
{
	const char* m_Name;
	int (*m_fpMain)(const PkInt, char**);    
};

// @return true if command line says we should run parameter commandlet, false otherwise
inline PkBool PkShouldRunApp( char** argv, const char* pstrAppName )
{
	PkAssert( argv );
	return ( 0 == stricmp( argv[ 1 ], pstrAppName ) );
}

// Commandlets are enabled, route to the proper commandlet hook:
// Parse commandline and run any commandlets specified
#define Pk_CONDITIONAL_RUN_APPS( argc, argv, appsArray ) \
	if ( argc >= 2 ) \
	{ \
		const PkInt numApps = sizeof( appsArray ) / sizeof( PkAppInfo ); \
		for ( PkInt iApp=0; iApp<numApps; ++iApp ) \
		{ \
			if ( PkShouldRunApp( argv, appsArray[ iApp ].m_Name ) ) \
			{ \
				return (*(appsArray[ iApp ].m_fpMain))( argc, argv ); \
			} \
		} \
	} 

#endif // PkAppHooks_h
