/**
 * @author Alan Perez-Rathke 
 *
 * @date March 17, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkBitSetUtils_h
#define PkBitSetUtils_h

#include "PkBuild.h"
#include "PkTypes.h"

// Utility functions for bit sets
namespace PkBitSetUtils
{
	// Removes all subsets of parameter bit set unless parameter bit set is itself a subset
	// If parameter bit set is not a subset, the bit set is appended to end of the output sets
	// Iterates over index range [0, outBitSets.size())
	template< typename tBitSet, typename tBitSetArray >
	void conditionalAppendAndCullSubSets( const tBitSet& bitSet, tBitSetArray& outBitSets )
	{
		conditionalAppendAndCullSubSets(
			  bitSet
			, outBitSets
			, 0 /* startItr */
			, (PkInt)outBitSets.size() /* endItr */
			);
	}

	// Removes all subsets of parameter bit set unless parameter bit set is itself a subset
	// If parameter bit set is not a subset, the bit set is appended to end of the output sets
	// Iterates over index range [startItr, endItr)
	template< typename tBitSet, typename tBitSetArray >
	void conditionalAppendAndCullSubSets( const tBitSet& bitSet, tBitSetArray& outBitSets, const PkInt startItr, const PkInt endItr )
	{
		// Determine if potential output sibling group is a subset of an existing output set
		PkInt itrOutputSets=endItr;
		while ( (--itrOutputSets) >= startItr )
		{
			// Store reference to current output set
			typename PkBitTraits<tBitSetArray>::tBitSetConstHandle currentOutputSet = outBitSets[ itrOutputSets ];

			// If parameter set is a subset, mark it as a subset and break loop
			if ( PkBitSetBridgeUtils::is_subset_of( bitSet, currentOutputSet ) )
			{
				// Early out if we're a subset
				return;
			}
			// Else if current output set is a subset of parameter set, remove from output sets
			else if ( PkBitSetBridgeUtils::is_subset_of( currentOutputSet, bitSet ) )
			{
				// Remove unordered by swapping with last element which we've already
				// checked since we're iterating backwards
				PkBitSetBridgeUtils::copy_bits( outBitSets.back(), outBitSets[ itrOutputSets ] ); 
				outBitSets.pop_back();
				// Break into simpler loop as we should no longer have to check if this is a subset of any other set
				break;
			}
		}

		// If we enter this loop, we've established that parameter bitset is a superset so only check for subsets of it
		while ( (--itrOutputSets) >= startItr )
		{
			typename PkBitTraits<tBitSetArray>::tBitSetConstHandle currentOutputSet = outBitSets[ itrOutputSets ];
			// We should no longer have a subset of parameter set
			PkAssert( false == PkBitSetBridgeUtils::is_subset_of( bitSet, currentOutputSet ) );
			if ( PkBitSetBridgeUtils::is_subset_of( currentOutputSet, bitSet ) )
			{
				// Remove unordered by swapping with last element which we've already
				// checked since we're iterating backwards
				PkBitSetBridgeUtils::copy_bits( outBitSets.back(), outBitSets[ itrOutputSets ] );
				outBitSets.pop_back();
			}
		}

		// Append parameter bit set if it's not a sub set
		// If we made it here, we're not a subset as we return early within the loop
		outBitSets.push_back( bitSet );
	}

} // end of PkBitSetUtils namespace

#endif // PkBitSetUtils_h
