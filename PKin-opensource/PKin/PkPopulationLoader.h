/**
 * @author Alan Perez-Rathke 
 *
 * @date March 6, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPopulationLoader_h
#define PkPopulationLoader_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkIndividual.h"

namespace PkPopulationLoader
{
	/**
	* @return true if population successfully loaded from file, false otherwise
	*/
	extern PkBool load( const char* fileName, PkArray( PkIndividual )& outPopulation, const PkUInt populationSizeHint=16 );
}

#endif // PkPopulationLoader_h
