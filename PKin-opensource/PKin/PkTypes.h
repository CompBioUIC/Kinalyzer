/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkTypes_h
#define PkTypes_h

#include "PkBuild.h"

// Hack: Windows int is always 32-bit, but other types change in size causing precision issues
#if Pk_FORCE_64_BIT_INT
	typedef Pk_64_BIT_INT PkInt;
	typedef unsigned Pk_64_BIT_INT PkUInt; // Shorthand for an unsigned integer
#else
	typedef int PkInt;
	typedef unsigned int PkUInt;
#endif

// Use 4-byte integer for boolean operations when possible
typedef PkInt PkBool;

// Our string type, hooked here in case we need more allocation control
#include <string>
typedef std::string PkString;

// Our vector type, hooked here in case we need more allocation control
#include <vector>
#define PkArray( type ) std::vector< type >

// Our dynamic bitfield type, hooked here in case we decide to change our dependencies
#include <boost/dynamic_bitset.hpp>
typedef boost::dynamic_bitset<> PkBitSet;

// An array of bitsets
typedef PkArray( PkBitSet ) PkBitSetArray;

// Bridge utility types for converting between bit set implementation
#include "PkBitSetBridgeUtils.h"

#endif // PkTypes_h
