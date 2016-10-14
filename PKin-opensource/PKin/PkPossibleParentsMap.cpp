/**
 * @author Alan Perez-Rathke 
 *
 * @date March 5, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkPossibleParentsMap.h"

namespace
{
	// Maps allele pair (x,y) to a bitstring which is 1 if parent is a possibility, 0 otherwise
	// Note: also allow mapping of wildcard allele as Pk_MAX_ALLELES_PER_LOCUS
	static const PkParentSet GStaticPossibleParentsLookupTable[Pk_MAX_ALLELES_PER_LOCUS+1][Pk_MAX_ALLELES_PER_LOCUS+1] =
	{
		  { 0x1c8 , 0x2f7d, 0x123b, 0x6, 0x3fff }
		, { 0x2f7d, 0x2050, 0x143e, 0x3, 0x3f7f }
		, { 0x123b, 0x143e, 0x20  , 0x5, 0x163f }
		, { 0x6   , 0x3   , 0x5   , 0x0, 0x7    }
		, { 0x3fff, 0x3f7f, 0x163f, 0x7, 0x3fff }
	};
} // end of unnamed namespace

/**
* Will return the associated set of possible parents for
* allele pair (x,y) where x,y belong to {0,1,2,3}
* these numbers are constrained by the 4-allele property
*/
namespace PkPossibleParentsMap
{

// @return ParentSet for parameter alleles
PkParentSet map( const PkInt alleleX, const PkInt alleleY )
{
	// Bounds checking on allele values, numbers constrained by 4-allele property
	PkAssert( alleleX >= 0 );
	PkAssert( alleleX <= Pk_MAX_ALLELES_PER_LOCUS );
	PkAssert( alleleY >= 0 );
	PkAssert( alleleY <= Pk_MAX_ALLELES_PER_LOCUS );

	// Use allele values as indices into look-up table
	return GStaticPossibleParentsLookupTable[ alleleX ][ alleleY ];
}

} // end of PkPossibleParentsMap namespace
