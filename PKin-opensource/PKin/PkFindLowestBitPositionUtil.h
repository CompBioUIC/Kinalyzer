/**
 * @author Alan Perez-Rathke 
 *
 * @date April 15, 2011
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkFindLowestBitPositionUtil_h
#define PkFindLowestBitPositionUtil_h

#include "PkBuild.h"

// From suggestions found here:
// http://stackoverflow.com/questions/757059/position-of-least-significant-bit-that-is-set

#if WINDOWS

	// Use bit scan forward
	// http://msdn.microsoft.com/en-us/library/wfd9z0bb.aspx
	#include <intrin.h>
	#pragma intrinsic(_BitScanForward)

	// @return 0 if Mask is 0, nonzero otherwise
	// Position of lowest bit is returned in index if found
	inline unsigned char PkFindLowestBitPosition( unsigned long* Index, unsigned long Mask )
	{
		return _BitScanForward( Index, Mask );
	}

	// 64-bit version of above
	inline unsigned char PkFindLowestBitPosition64( unsigned long* Index, unsigned __int64 Mask )
	{
		return _BitScanForward64( Index, Mask );
	}

#elif UNIX

	#include "PkAssert.h"

	// http://gcc.gnu.org/onlinedocs/gcc-4.4.5/gcc/Other-Builtins.html
	// Other alternative is __builtin_ffs intrinsic

	// @return 0 if Mask is 0, nonzero otherwise
	// Position of lowest bit is returned in index if found
	inline unsigned char PkFindLowestBitPosition( unsigned long* Index, unsigned long Mask )
	{
		*Index = __builtin_ctz( Mask );
		return 0 != Mask;
	}

	// 64-bit version of above
	inline unsigned char PkFindLowestBitPosition64( unsigned long* Index, unsigned long long Mask )
	{
		PkAssert( sizeof( Mask ) == 8 );
		*Index = __builtin_ctzll( Mask );
		return 0 != Mask;
	}

#else

	#error Unrecognized platform

#endif // platform

#endif // PkFindLowestBitPositionUtil_h
