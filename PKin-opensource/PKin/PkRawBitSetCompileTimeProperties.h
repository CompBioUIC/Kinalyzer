/**
 * @author Alan Perez-Rathke 
 *
 * @date April 14, 2011
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkRawBitSetCompileTimeProperties_h
#define PkRawBitSetCompileTimeProperties_h

#include "PkBuild.h"
#include <limits>

// Properties common to all (or many) bit set structures
struct PkRawBitSetCompileTimeProperties
{
	// Typedefs
	typedef unsigned int  block_type;
	typedef size_t        size_type;
	typedef block_type    block_width_type;
	typedef block_type*   buffer_type;
	typedef unsigned char byte_type;

	// Enums
	static const block_width_type bits_per_block = ((block_width_type)(std::numeric_limits<block_type>::digits));
	static const size_type npos = static_cast<size_type>(-1);
	static const size_type use_default_value = static_cast<size_type>(-1);

	// From boost::dynamic_bitset - calculates number of blocks necessary to contain the bit set
	static inline size_type calc_num_blocks( size_type num_bits )
	{
		return ( num_bits / bits_per_block )
           + static_cast<int>( num_bits % bits_per_block != 0 );
	}
};

#endif // PkRawBitSetCompileTimeProperties_h
