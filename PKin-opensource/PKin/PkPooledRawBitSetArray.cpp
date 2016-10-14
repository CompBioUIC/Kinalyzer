/**
 * @author Alan Perez-Rathke 
 *
 * @date April 13, 2011
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkPooledRawBitSetArray.h"

namespace
{
	// @return TRUE if reserve pool size is power of two, FALSE otherwise
	inline bool is_pow2_reserved_pool_size( const size_t num_initial_reserved_pool_elements )
	{
		return ( PkPooledRawBitSetArray::use_default_value == num_initial_reserved_pool_elements )
			|| ( 0 == ( num_initial_reserved_pool_elements & ( num_initial_reserved_pool_elements - 1 ) ) );
	}

	// @return Pointer to allocated pool
	inline PkPooledRawBitSetArray::PooledAllocator* get_allocated_pool(
		  const size_t num_bits
		, const size_t num_initial_reserved_pool_elements
		)
	{
		// Assert reserved pool size is default or power of two
		PkAssert( is_pow2_reserved_pool_size( num_initial_reserved_pool_elements ) );
		return new PkPooledRawBitSetArray::PooledAllocator(
			  // Determine number of bytes to allocate for each bit set
			  PkPooledRawBitSetArray::calc_num_blocks( num_bits ) * sizeof( PkPooledRawBitSetArray::block_type )
			  // Determine how many elements to reserve
			, ( PkPooledRawBitSetArray::use_default_value == num_initial_reserved_pool_elements ) 
				? PkPooledRawBitSetArray::num_initial_reserved_pool_elements_default
				: num_initial_reserved_pool_elements
			);	
	}
} // end of anonymous namespace

// Constructor
// @param num_bits - the size of each bit field within the array
// @param num_elements - number of addressable bit sets
// @param num_initial_reserved_pool_elements - the number of contiguous elements to reserve - must be power of 2
PkPooledRawBitSetArray::PkPooledRawBitSetArray(
	  const size_type num_bits
	, const size_type num_elements
	, const size_type num_initial_reserved_pool_elements
	) : m_num_bits( num_bits )
	  , m_num_blocks( calc_num_blocks( num_bits ) )
	  , m_size( 0 )
	  // Initialize memory pool
	  , mp_pool_alloc( get_allocated_pool( num_bits, num_initial_reserved_pool_elements ) )
{
	// Assert allocator is non-null
	PkAssert( mp_pool_alloc );
	// Assert number of bits is positive
	PkAssert( 0 < num_bits );
	// Allocate bit buffers
	for ( size_t i=0; i<num_elements; ++i )
	{
		PkVerify( NULL != allocate_zeroed_bit_buffer() );
	}
}

// Constructor
// @param p_chunk - pointer to chunk of allocated memory - it will be soft copied!
// @param b_owned - true if chunk is now owned by this array (it will be freed upon this array's destruction)
// @param num_bits - the size of each bit field within the array
// @param num_elements - number of addressable bit sets
// @param num_initial_reserved_pool_elements - the number of contiguous elements to reserve - must be power of 2
PkPooledRawBitSetArray::PkPooledRawBitSetArray(
	  buffer_type p_chunk
	, const bool b_owned
	, const size_type num_bits
	, const size_type num_elements
	, const size_type num_initial_reserved_pool_elements
	) : m_num_bits( num_bits )
	  , m_num_blocks( calc_num_blocks( num_bits ) )
	  , m_size( num_elements )
	  // Initialize memory pool
	  , mp_pool_alloc( get_allocated_pool( num_bits, num_initial_reserved_pool_elements ) )
{
	// Assert allocator is non-null
	PkAssert( mp_pool_alloc );
	// Assert number of bits is positive
	PkAssert( 0 < m_num_bits );
	// Append chunk
	append_chunk( (byte_type*) p_chunk, num_total_bytes(), b_owned );
}

// Copy constructor
PkPooledRawBitSetArray::PkPooledRawBitSetArray( const PkPooledRawBitSetArray& bitSetArray )
	: m_num_bits( bitSetArray.num_bits() )
	, m_num_blocks( bitSetArray.num_blocks() )
	, m_size( 0 )
	  // Initialize memory pool
	, mp_pool_alloc( bitSetArray.mp_pool_alloc
		? get_allocated_pool( bitSetArray.num_bits(), bitSetArray.get_pool_alloc().get_next_size() )
		: NULL )
{
	if ( bitSetArray.size() > 0 )
	{
		push_back_chunks( bitSetArray );
	}
}

// Assignment operator
PkPooledRawBitSetArray& PkPooledRawBitSetArray::operator=( const PkPooledRawBitSetArray& bitSetArray )
{
	clear();
	force_set_num_bits( bitSetArray.num_bits() );
	force_set_num_blocks( bitSetArray.num_blocks() );
	PkAssert( NULL == mp_pool_alloc );
	PkVerify( mp_pool_alloc = get_allocated_pool( 
		  bitSetArray.num_bits()
		, bitSetArray.mp_pool_alloc->get_next_size()
		) );
	push_back_chunks( bitSetArray );
	return *this;
}

// Removes last element from pooled array
void PkPooledRawBitSetArray::pop_back()
{
	// Assert we have elements to pop
	PkAssert( !empty() );
	// Assert that our byte offset indicates we have elements as well
	PkAssert( get_chunks().back().second >= num_bytes() );
	// Assert that byte offset is proper multiple of number of blocks to represent a bit set
	PkAssert( byte_offset_is_proper_multiple( get_chunks().back().second ) );
	// If last element is from pooled chunk, then release it back to pool
	if ( !is_owned_chunk( num_chunks()-1 ) )
	{
		get_pool_alloc().free( get_back_bit_buffer() );
	}
	// If chunk now has zero elements, remove the chunk
	if ( 0 == ( get_chunks().back().second -= num_bytes() ) ) 
	{
		remove_back_chunk();
	}
	// Update our size
	--m_size;
	// Assert that we are empty or new back chunk has elements
	PkAssert( empty() || (get_chunks().back().second >= num_bytes()) );
}

// Releases all memory and reset state
void PkPooledRawBitSetArray::clear()
{
	release_owned_chunks();
	m_owned_chunks_mask.clear();
	get_chunks().clear();
	if ( mp_pool_alloc )
	{
		get_pool_alloc().purge_memory();
		delete mp_pool_alloc;
		mp_pool_alloc = NULL;
	}
	force_set_num_bits( 0 );
	force_set_num_blocks( 0 );
	m_size = 0;
}

// Re-initializes a constructed array to the following parameters (wipes any stored bit sets)
void PkPooledRawBitSetArray::reinit(
	  const size_type num_bits
	, const size_type num_elements
	, const size_type num_initial_reserved_pool_elements
	)
{
	// Wipe everything!
	clear();
	// Store number of bits
	force_set_num_bits( num_bits );
	if ( num_bits > 0 )
	{
		// Determine number of blocks
		force_set_num_blocks( calc_num_blocks( num_bits ) );
		// Allocate a new memory pool
		PkVerify( mp_pool_alloc = get_allocated_pool( num_bits, num_initial_reserved_pool_elements ) );
		// Allocate bit buffers
		for ( size_t i=0; i<num_elements; ++i )
		{
			PkVerify( NULL != allocate_zeroed_bit_buffer() );
		}
	}
}

// Re-initializes a constructed array to the following parameters (wipes any stored bit sets)
// @param p_chunk - pointer to chunk of allocated memory - it will be soft copied!
// @param b_owned - true if chunk is now owned by this array (it will be freed upon this array's destruction)
// @param num_bits - the size of each bit field within the array
// @param num_elements - number of addressable bit sets
// @param num_initial_reserved_pool_elements - the number of contiguous elements to reserve - must be power of 2
void PkPooledRawBitSetArray::reinit(
	  buffer_type p_chunk
	, const bool b_owned
	, const size_type num_bits
	, const size_type num_elements
	, const size_type num_initial_reserved_pool_elements
	)
{
	// Wipe everything!
	clear();
	// Store number of bits
	force_set_num_bits( num_bits );
	// Assert number of bits is positive
	PkAssert( 0 < m_num_bits );
	// Determine number of blocks
	force_set_num_blocks( calc_num_blocks( num_bits ) );
	// Allocate a new memory pool
	PkVerify( mp_pool_alloc = get_allocated_pool( num_bits, num_initial_reserved_pool_elements ) );
	// Update our size
	m_size = num_elements;
	// Append chunk
	append_chunk( (byte_type*) p_chunk, num_total_bytes(), b_owned );
}

// Deallocates all owned chunks
void PkPooledRawBitSetArray::release_owned_chunks()
{
	for ( 
		  PkBitSet::size_type itr_chunk = m_owned_chunks_mask.find_first()
		; itr_chunk != PkBitSet::npos
		; itr_chunk = m_owned_chunks_mask.find_next( itr_chunk )
		)
	{
		release_owned_chunk( itr_chunk );
		get_chunks()[ itr_chunk ].first = NULL;
		get_chunks()[ itr_chunk ].second = 0;
	}
}

// Copies chunks from parameter bit set array and appends as single chunk
// Updates size
void PkPooledRawBitSetArray::push_back_chunks( const PkPooledRawBitSetArray& bitSetArray )
{
	// Allocate single contiguous chunk
	byte_type* p_chunk = (byte_type*) malloc( bitSetArray.num_total_bytes() );
	PkAssert( NULL != p_chunk );
	// Append chunk
	append_chunk( p_chunk, bitSetArray.num_total_bytes(), true /* b_owned */ );
	// Copy all bit sets
	const PkPooledRawBitSetChunkArray& other_chunks = bitSetArray.get_chunks();
	for ( size_type i=0; i<other_chunks.size(); ++i )
	{
		memcpy( p_chunk, other_chunks[ i ].first, other_chunks[ i ].second );
		p_chunk += other_chunks[ i ].second;
	}
	// Update size
	m_size += bitSetArray.size();
}

// Allocates a bit buffer from pooled memory
// Updates size
PkPooledRawBitSetArray::buffer_type PkPooledRawBitSetArray::allocate_bit_buffer()
{
	// Allocate a new buffer from our pool allocator
	byte_type* const p_buffer = (byte_type*) get_pool_alloc().malloc();
	PkAssert( NULL != p_buffer );

	// Determine if it's contiguous with our current chunk
	if ( is_contiguous_byte_buffer( p_buffer ) )
	{
		// Assert that this is a pooled chunk
		PkAssert( get_pool_alloc().is_from( get_chunks().back().first ) );
		// Assert that parallel arrays are same size
		PkAssert( m_owned_chunks_mask.size() == num_chunks() );
		// Assert that we don't own this chunk
		PkAssert( !is_owned_chunk( num_chunks()-1 ) );
		// Update current contiguous chunk
		get_chunks().back().second += num_bytes();
	}
	else
	{
		// Start a new contiguous chunk
		get_chunks().push_back( PkPooledRawBitSetChunkInfo( p_buffer, num_bytes() ) );
		// This chunk is owned by the pool; therefore, we don't have to free it explicitly
		m_owned_chunks_mask.push_back( false );
		// Assert that parallel arrays are the same size
		PkAssert(  m_owned_chunks_mask.size() == num_chunks() );
	}

	// Keep track of how many bit buffers are in this collection
	++m_size;

	// Return allocated buffer
	return (buffer_type) p_buffer;
}

// EOF
