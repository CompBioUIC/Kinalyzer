/**
 * @author Alan Perez-Rathke 
 *
 * @date March 5, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkParentSet_h
#define PkParentSet_h

#include "PkBuild.h"
#include "PkTypes.h"

// A simple typedef of an unsigned integer
typedef PkUInt PkParentSet;

// Returns true if parent set contains no possible parents
#define Pk_IS_EMPTY_PARENT_SET( a ) ((a) == 0)

#endif // PkParentSet_h
