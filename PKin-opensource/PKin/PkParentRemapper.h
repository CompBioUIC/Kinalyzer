/**
 * @author Alan Perez-Rathke 
 *
 * @date April 15, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkParentRemapper_h
#define PkParentRemapper_h

#include "PkBuild.h"
#include "PkTypes.h"

/**
* In order to avoid redundant generation of possible sibling groups
* with the same mapped alleles and same parent sets, a global ordering
* must be applied to how alleles are mapped. This means that previously
* mapped alleles may need to be remapped.  The following accesses
* a lookup table which remaps the possible parent sets when old alleles
* are remapped.
* Note: Both alleles and possible parents are remapped
*/
namespace PkParentRemapper
{
	/**
	* @return - return remapped bit index of parent set for a mapped allele prior to allele remap
	* @param bitIdxParent - the bit index for the parent to remap
	* @param mappedAllelePriorToRemap - the value of a mapped allele *before* it's remapped
	*	- it's assumed that future remapping will simply increment this value by 1
	*	- this function does *not* actually remap the allele (only the parent index) but assumes it will be in the future
	*/
	extern PkInt getRemapBitIndex( const PkInt bitIdxParent, const PkInt mappedAllelePriorToRemap );
} // end of PkParentRemapper namespace

#endif // PkParentRemapper_h
