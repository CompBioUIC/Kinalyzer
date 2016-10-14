/**
 * @author Alan Perez-Rathke 
 *
 * @date April 16, 2011 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkBitSetBridgeUtils_h
#define PkBitSetBridgeUtils_h

#include "PkBuild.h"
#include "PkPooledRawBitSetArray.h"
#include "PkAssert.h"

// Base traits for PkBitSet related structures
struct PkBitSetTraits
{
	typedef PkBitSet& tBitSetHandle;
	typedef const PkBitSet& tBitSetConstHandle;
};

// Base traits for pooled raw bit set structure
struct PkPooledRawBitSetTraits
{
	typedef PkPooledRawBitSetHandle tBitSetHandle;
	typedef PkPooledRawBitSetConstHandle tBitSetConstHandle;
};

// The generic traits
template< typename tBitType >
struct PkBitTraits
{
	// Probably won't compile, need specialization!
	typedef typename tBitType::tBitSetHandle tBitSetHandle;
	typedef typename tBitType::tBitSetConstHandle tBitSetConstHandle;
};

// Traits for PkBitSetArray
template<>
struct PkBitTraits< PkBitSetArray > : public PkBitSetTraits
{
};

// Traits for PkBitSet
template<>
struct PkBitTraits< PkBitSet > : public PkBitSetTraits
{
};

// Traits for PkPooledRawBitSetArray
template<>
struct PkBitTraits< PkPooledRawBitSetArray > : public PkPooledRawBitSetTraits
{
};

// Traits for PkPooledRawBitSetHandle
template<>
struct PkBitTraits< PkPooledRawBitSetHandle > : public PkPooledRawBitSetTraits
{
};

// Flip this switch to swap between pooled and dynamic bit sets
#define Pk_ENABLE_POOLED_SIBLING_SET_ARRAY 1

// Determine typedef for the array type to use for storing sibling sets
#if Pk_ENABLE_POOLED_SIBLING_SET_ARRAY 
	typedef PkPooledRawBitSetArray PkSiblingSetBitSetArray;
#else
	typedef PkBitSetArray PkSiblingSetBitSetArray;
#endif

// Bridge utilities for coverting from bit set array implementations
namespace PkBitSetBridgeUtils
{
	// Sort the loci according to the predicate
	template < typename tBitSetArray >
	struct IndirectLociSortingPredicate
	{
		inline bool operator()( const tBitSetArray* a, const tBitSetArray* b ) const
		{
			PkAssert( a );
			PkAssert( b );
			return ( a->size() > b->size() );
		}
	};

	// @returns TRUE if a is a subset of b, FALSE otherwise
	template < typename tBitSetA, typename tBitSetB >
	bool is_subset_of( const tBitSetA& a, const tBitSetB& b )
	{
		PkAssert( a.size() == b.size() );
		PkAssert( a.num_blocks() == b.num_blocks() );
		PkAssert( sizeof( typename tBitSetA::block_type ) == sizeof( typename tBitSetB::block_type ) );
		for ( typename tBitSetA::size_type i = 0; i < a.num_blocks(); ++i)
		{
			if (a.get_buffer()[i] & ~b.get_buffer()[i])
			{
				return false;
			}
		}
		return true;
	}

	// Copies bits of set 'from' into set 'to'
	template < typename tBitSetA, typename tBitSetB >
	inline void copy_bits( const tBitSetA& from, tBitSetB& to )
	{
		PkAssert( from.size() == to.size() );
		PkAssert( from.num_blocks() == to.num_blocks() );
		PkAssert( sizeof( typename tBitSetA::block_type ) == sizeof( typename tBitSetB::block_type ) );
		PkVerify( memcpy(
			  (void*)(&to.get_buffer()[0])
			, &from.get_buffer()[0]
			, from.num_blocks() * sizeof( typename tBitSetA::block_type ) 
			) );
	}

	// Overload to append siblings sets from parameter collections
	inline void push_back_sibling_sets( const PkBitSetArray& from, PkBitSetArray& to )
	{
		to.insert( to.end(), from.begin(), from.end() );
	}

	// Overload to append sibling sets from parameter collections
	template< typename tBitSetArray >
	inline void push_back_sibling_sets( const tBitSetArray& from, PkPooledRawBitSetArray& to )
	{
		to.push_back_array( from ); 
	}

	inline void reinit(
		  PkBitSetArray& store
		, const PkBitSet::size_type num_bits
		, const PkBitSet::size_type num_elements = 0
		, const PkBitSet::size_type num_initial_reserved_pool_elements = PkRawBitSetCompileTimeProperties::use_default_value
		)
	{ /* do nothing! */ }

	inline void reinit(
		  PkPooledRawBitSetArray& store
		, const PkPooledRawBitSetArray::size_type num_bits
		, const PkPooledRawBitSetArray::size_type num_elements = 0
		, const PkPooledRawBitSetArray::size_type num_initial_reserved_pool_elements = PkRawBitSetCompileTimeProperties::use_default_value
		)
	{
		store.reinit( num_bits, num_elements, num_initial_reserved_pool_elements );
	}

} // end of PkBitSetBridgeUtils

#endif // PkBitSetBridgeUtils_h
