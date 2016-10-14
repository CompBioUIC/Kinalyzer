/**
 * @author Alan Perez-Rathke 
 *
 * @date June 26, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkParallelIntersectorUtil_h
#define PkParallelIntersectorUtil_h

#include "PkBuild.h"
#include "PkTypes.h"

namespace Pk
{
	// Intersects each set of outputSets with each set of partitionSets and append to outputSets
	extern void IntersectSiblingSets(
		  PkSiblingSetBitSetArray& outputSets
		, const PkSiblingSetBitSetArray& partitionSets
		);
}

#endif // PkParallelIntersectorUtil_h
