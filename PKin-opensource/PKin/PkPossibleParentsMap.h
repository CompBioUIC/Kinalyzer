/**
 * @author Alan Perez-Rathke 
 *
 * @date March 5, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPossibleParentsMap_h
#define PkPossibleParentsMap_h

#include "PkBuild.h"
#include "PkParentSet.h"

/**
* Will return the associated set of possible parents for
* allele pair (x,y) where x,y belong to {0,1,2,3} where
* these numbers are constrained by the 4-allele property
*/
namespace PkPossibleParentsMap
{
	// @return ParentSet for parameter alleles
	extern PkParentSet map( const PkInt alleleX, const PkInt alleleY );
}

#endif // PkPossibleParentsMap_h
