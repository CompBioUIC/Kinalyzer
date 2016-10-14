/**
 * @author Alan Perez-Rathke
 *
 * @date April 15, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkParentRemapper.h"

namespace
{

/**
* Maps [ mappedAllelePreRemap ][ bitIdxParent ] to a remapped bit index for the parent
*/
static const PkInt GStaticParentRemapTable[ Pk_MAX_ALLELES_PER_LOCUS-1 ][ Pk_NUM_POSSIBLE_PARENT_SETS ] =
{
	  { 2, 1, 0, 4, 3, 5, 6, Pk_INVALID_INDEX, 13, 10, 9, 11, 12, 8 }
	, { 0, 2, 1, 3, 5, 4, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, 9, 12, Pk_INVALID_INDEX, 10, Pk_INVALID_INDEX }
	, { 2, 1, 0, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX, Pk_INVALID_INDEX }
};

} // end of unnamed namespace

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
PkInt getRemapBitIndex( const PkInt bitIdxParent, const PkInt mappedAllelePriorToRemap )
{
	// Assert on bounds checks
	PkAssert( bitIdxParent >= 0 );
	PkAssert( bitIdxParent < Pk_NUM_POSSIBLE_PARENT_SETS );
	PkAssert( mappedAllelePriorToRemap >= 0 );
	PkAssert( mappedAllelePriorToRemap < (Pk_MAX_ALLELES_PER_LOCUS-1) );

	// Return remapped bit index for parent
	return GStaticParentRemapTable[ mappedAllelePriorToRemap ][ bitIdxParent ];
}

} // end of PkParentSetRemapper namespace
