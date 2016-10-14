/**
 * @author Alan Perez-Rathke 
 *
 * @date April 13, 2011
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkPooledRawBitSetArray_h
#define PkPooledRawBitSetArray_h

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkTypes.h"
#include "PkRawBitSetCompileTimeProperties.h"
#include "TPkRawBitSetHandle.h"

#include <boost/pool/pool.hpp>

// A pooled bit set array
class PkPooledRawBitSetArray : public PkRawBitSetCompileTimeProperties
{
private:

	// Run time properties policy for use in TPkRawBitSetHandle
	class PkPooledRawBitSetRunTimeProperties
	{
	public:

		// @return The number of bits in the bit set
		inline size_type size() const { return m_parent_bit_set_array.num_bits(); }

		// @return The number of blocks necessary to represent a bit set
		inline size_type num_blocks() const { return m_parent_bit_set_array.num_blocks(); }

		// @return The number of bytes necessary to represent a bit set
		inline size_type num_bytes() const { return m_parent_bit_set_array.num_bytes(); }

	private:

		// Private constructor
		explicit PkPooledRawBitSetRunTimeProperties(
			  const class PkPooledRawBitSetArray& parent_bit_set_array
			) : m_parent_bit_set_array( parent_bit_set_array )
		{}

		// A reference to parent bit set array containing this bit set
		const class PkPooledRawBitSetArray& m_parent_bit_set_array;

		// Allow PkPooledRawBitSetArray to do as it pleases
		friend class PkPooledRawBitSetArray;
	};

public:
	
	// Typedefs
	
	// A handle to non-const bit set
	typedef TPkRawBitSetHandle< PkPooledRawBitSetRunTimeProperties > PooledRawBitSetHandle;

	// A handle to a const bit set
	typedef const TPkRawBitSetHandle< PkPooledRawBitSetRunTimeProperties > PooledRawBitSetConstHandle;

	// Our underlying pool type
	typedef boost::pool<> PooledAllocator;

	// A pair for representing a contiguous chunk of memory
	// first member is the underlying bit buffer
	// second member is the number of bytes allocated to the buffer
	typedef std::pair<byte_type*, size_type> PkPooledRawBitSetChunkInfo;
	
	// A list of contiguous blocks (aka chunks)
	typedef std::vector< PkPooledRawBitSetChunkInfo > PkPooledRawBitSetChunkArray;

	// The number of pool elements to reserve by default
	enum { num_initial_reserved_pool_elements_default = 32 };

	// Default constructor
	PkPooledRawBitSetArray()
		: m_num_bits( 0 )
		, m_num_blocks( 0 )
		, m_size( 0 )
		, mp_pool_alloc( NULL )
	{}

	// Constructor
	// @param num_bits - the size of each bit field within the array
	// @param num_elements - number of addressable bit sets
	// @param num_initial_reserved_pool_elements - the number of contiguous elements to reserve - must be power of 2
	PkPooledRawBitSetArray(
		  const size_type num_bits
		, const size_type num_elements = 0
		, const size_type num_initial_reserved_pool_elements = use_default_value
		);

	// Constructor
	// @param p_chunk - pointer to chunk of allocated memory - it will be soft copied!
	// @param b_owned - true if chunk is now owned by this array (it will be freed upon this array's destruction)
	// @param num_bits - the size of each bit field within the array
	// @param num_elements - number of addressable bit sets
	// @param num_initial_reserved_pool_elements - the number of contiguous elements to reserve - must be power of 2
	PkPooledRawBitSetArray(
		  buffer_type p_chunk
		, const bool b_owned
		, const size_type num_bits
		, const size_type num_elements
		, const size_type num_initial_reserved_pool_elements = use_default_value
		);

	// Copy constructor
	PkPooledRawBitSetArray( const PkPooledRawBitSetArray& bitSetArray );

	// Destructor in which we deallocate our owned chunks (assumes C free allocation)
	~PkPooledRawBitSetArray() { clear(); }

	// Assignment operator
	PkPooledRawBitSetArray& operator=( const PkPooledRawBitSetArray& bitSetArray );

	// @return The number of bits in each bit set
	inline size_type num_bits() const { return m_num_bits; }

	// @return The number of blocks necessary to represent a bit set
	inline size_type num_blocks() const { return m_num_blocks; }

	// @return The number of bytes necessary to represent a bit set
	inline size_type num_bytes() const { return num_blocks() * sizeof( block_type ); }

	// @return The number of bytes necessary to represent all bit sets in this array
	inline size_type num_total_bytes() const { return size() * num_bytes(); }

	// @return The number of chunks (continuous memory blocks) within array
	inline size_type num_chunks() const { return get_chunks().size(); }

	// @return The number of bit sets in this pooled array
	inline size_type size() const { return m_size; }

	// @return TRUE if there are no elements in this pooled array, FALSE otherwise
	inline bool empty() const { return get_chunks().empty(); }

	// @return A bit set interface to front bit buffer
	inline PooledRawBitSetHandle front() {
		return PooledRawBitSetHandle( get_front_bit_buffer(), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// @return A const bit set interface to front bit buffer
	inline PooledRawBitSetConstHandle front() const {
		return PooledRawBitSetHandle( get_front_bit_buffer(), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// @return a bit set interface to back bit buffer
	inline PooledRawBitSetHandle back() {
		return PooledRawBitSetHandle( get_back_bit_buffer(), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// @return a const bit set interface to back bit buffer
	inline PooledRawBitSetConstHandle back() const {
		return PooledRawBitSetConstHandle( get_back_bit_buffer(), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// @return a bit set interface to bit buffer at parameter idx
	inline PooledRawBitSetHandle operator[]( const size_type idx ) {
		return PooledRawBitSetHandle( get_bit_buffer( idx ), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// @return a bit set interface to bit buffer at parameter idx
	inline PooledRawBitSetConstHandle operator[]( const size_type idx ) const {
		return PooledRawBitSetHandle( get_bit_buffer( idx ), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// Appends parameter bit set to pooled array
	template< typename tBitSet >
	void push_back( const tBitSet& bit_set );

	// Appends bit set collection to pooled array
	template< typename tBitSetArray >
	void push_back_array( const tBitSetArray& bit_set_array );

	// Removes last element from pooled array
	void pop_back();

	// Releases all memory and resets state
	void clear();

	// DOES NOTHING - just need to be more compliant with std::vector interface
	inline void reserve( const size_type n ) { /* do nothing! */ }

	// Re-initializes a constructed array to the following parameters (wipes any stored bit sets)
	void reinit(
		  const size_type num_bits
		, const size_type num_elements = 0
		, const size_type num_initial_reserved_pool_elements = use_default_value
		);

	// Re-initializes a constructed array to the following parameters (wipes any stored bit sets)
	// @param p_chunk - pointer to chunk of allocated memory - it will be soft copied!
	// @param b_owned - true if chunk is now owned by this array (it will be freed upon this array's destruction)
	// @param num_bits - the size of each bit field within the array
	// @param num_elements - number of addressable bit sets
	// @param num_initial_reserved_pool_elements - the number of contiguous elements to reserve - must be power of 2
	void reinit(
		  buffer_type p_chunk
		, const bool b_owned
		, const size_type num_bits
		, const size_type num_elements
		, const size_type num_initial_reserved_pool_elements = use_default_value
		);

	// Appends a new buffer
	inline PooledRawBitSetHandle allocate_zeroed() {
		return PooledRawBitSetHandle( allocate_zeroed_bit_buffer(), PkPooledRawBitSetRunTimeProperties( *this ) );
	}

	// @return Const reference to allocated chunks
	inline const PkPooledRawBitSetChunkArray& get_chunks() const { return m_contiguous_chunks; }

private:

	//************ Utilities for assertion checking

	// @return TRUE if index is within bounds [0, max_size), FALSE otherwises
	inline static bool bounds_check( const size_type idx, const size_type max_size )
	{
		return (0 <= idx) && (max_size > idx);
	}

	// @return TRUE if byte offset is multiple of number of bytes to represent a bit buffer, FALSE otherwise
	inline bool byte_offset_is_proper_multiple( const size_type byte_offset ) const
	{
		return ( ( byte_offset % num_bytes() ) == 0 );
	}

	//************ Internal accessors and mutators

	// @return Reference to contiguous chunks
	inline PkPooledRawBitSetChunkArray& get_chunks() { return m_contiguous_chunks; }

	// @return Reference to allocated pool
	inline PooledAllocator& get_pool_alloc() {
		PkAssert( mp_pool_alloc );
		return *mp_pool_alloc;
	}

	// @return Const reference to allocated pool
	inline const PooledAllocator& get_pool_alloc() const {
		PkAssert( mp_pool_alloc );
		return *mp_pool_alloc;
	}

	// Force sets our number of bits to parameter value
	inline void force_set_num_bits( const size_type num_bits ) {
		*((size_type*)(&m_num_bits)) = num_bits;
	}

	// Force sets our number of blocks to parameter value
	inline void force_set_num_blocks( const size_type num_blocks ) {
		*((size_type*)(&m_num_blocks)) = num_blocks;
	}

	//************ Internal chunk utilities

	// @return TRUE if chunked is owned by this pool array, FALSE otherwise
	inline bool is_owned_chunk( const size_t idx_chunk ) const {
		PkAssert( bounds_check( idx_chunk, m_owned_chunks_mask.size() ) );
		return m_owned_chunks_mask.test( idx_chunk );
	}

	// Deallocates memory associated with an owned chunk, assumes C style/malloc free
	// Note: does not update owned_chunks_mask or size
	inline void release_owned_chunk( const size_t idx_chunk ) {
		PkAssert( bounds_check( idx_chunk, get_chunks().size() ) );
		PkAssert( is_owned_chunk( idx_chunk ) );
		free( m_contiguous_chunks[ idx_chunk ].first );
	}

	// Conditionally deallocates memory associated with an owned chunk
	// Note: does not update owned_chunks_mask or size
	inline void conditional_release_owned_chunk( const size_t idx_chunk ) {
		if ( is_owned_chunk( idx_chunk ) )
		{
			release_owned_chunk( idx_chunk );
		}
	}

	// Deallocates all owned chunks
	// Note: does not update size
	void release_owned_chunks();

	// Releases back chunk and updates owned chunks mask
	// Note: does not update size
	void remove_back_chunk() {
		conditional_release_owned_chunk( num_chunks()-1 );
		get_chunks().pop_back();
		m_owned_chunks_mask.resize( num_chunks() );
	}

	// Appends information necessary to manage a single chunk of contiguous memory
	// Note: does not update size
	inline void append_chunk( byte_type* const p_chunk, const size_type num_chunk_bytes, bool b_owned  ) {
		PkAssert( NULL != p_chunk );
		PkAssert( byte_offset_is_proper_multiple( num_chunk_bytes ) );
		get_chunks().push_back( PkPooledRawBitSetChunkInfo( (byte_type*)p_chunk, num_chunk_bytes ) );
		m_owned_chunks_mask.push_back( b_owned );
	}

	// Copies chunks from parameter bit set array and appends as single chunk
	// Updates size
	void push_back_chunks( const PkPooledRawBitSetArray& bitSetArray );

	//************ Internal byte buffer buffer utilities

	// @return Non-constant pointer to last byte buffer allocated
	inline byte_type* get_back_byte_buffer()
	{
		return const_cast<byte_type*>
			(static_cast<const PkPooledRawBitSetArray *>(this)->get_back_byte_buffer());
	}

	// @return constant pointer to last byte buffer allocated
	inline const byte_type* get_back_byte_buffer() const
	{
		// Assert that we have bit buffers
		PkAssert( !empty() );
		// Assert that back chunk is not empty
		PkAssert( get_chunks().back().second > 0 );
		// Assert that back chunk total bytes is a proper multiple
		PkAssert( byte_offset_is_proper_multiple( get_chunks().back().second ) );
		return (&get_chunks().back().first[ get_chunks().back().second - num_bytes() ]);
	}

	// @return TRUE if bit buffer is contiguous with last allocation, false otherwise
	inline bool is_contiguous_byte_buffer( const byte_type* const p_buffer ) const
	{
		return !get_chunks().empty()
			&& ((p_buffer - get_back_byte_buffer()) == num_bytes());
	}

	//************ Internal bit set buffer utilities

	// Sets all memory in bit buffer to zero
	inline buffer_type zero_bit_buffer( buffer_type const p_bit_buffer ) {
		PkAssert( NULL != p_bit_buffer );
		return ( ( buffer_type ) memset( p_bit_buffer, 0, num_bytes() ) );
	}

	// @return The bit buffer at parameter idx
	inline buffer_type get_bit_buffer( size_type idx )
	{
		return const_cast<buffer_type>
			(static_cast<const PkPooledRawBitSetArray *>(this)->get_bit_buffer( idx ));
	}

	// @return The const bit buffer at parameter idx
	inline const buffer_type get_bit_buffer( size_type idx ) const
	{
		PkAssert( bounds_check( idx, size() ) );
		// Convert index to byte offset
		size_t byte_offset = idx * num_bytes();
		// Start search at first chunk
		idx = 0;
		// Determine which contiguous chunk we are interested in
		while ( byte_offset >= get_chunks()[ idx ].second )
		{
			byte_offset -= get_chunks()[ idx ].second;
			++idx;
		}
		// Assert we fit within the chunk
		PkAssert( byte_offset_is_proper_multiple( get_chunks()[ idx ].second ) );
		PkAssert( byte_offset_is_proper_multiple( byte_offset ) );
		// Return found chunk offset
		return (const buffer_type)(&get_chunks()[ idx ].first[ byte_offset ]);
	}

	// @return Non-constant pointer to first bit buffer allocated
	inline buffer_type get_front_bit_buffer()
	{
		return const_cast<buffer_type>
			(static_cast<const PkPooledRawBitSetArray *>(this)->get_front_bit_buffer());
	}

	// @return Constant pointer to first bit buffer allocated
	inline const buffer_type get_front_bit_buffer() const
	{
		PkAssert( !empty() );
		return (const buffer_type)get_chunks().front().first;
	}

	// @return Non-constant pointer to last bit buffer allocated
	inline buffer_type get_back_bit_buffer()
	{
		return const_cast<buffer_type>
			(static_cast<const PkPooledRawBitSetArray *>(this)->get_back_bit_buffer());
	}

	// @ return Constant pointer to last bit buffer allocated
	inline const buffer_type get_back_bit_buffer() const
	{
		return (const buffer_type)get_back_byte_buffer();
	}

	// Allocates a bit buffer from pooled memory
	// Updates size
	buffer_type allocate_bit_buffer();

	// Allocates a single bit buffer, zeros it, and returns it
	buffer_type allocate_zeroed_bit_buffer() { return zero_bit_buffer( allocate_bit_buffer() ); }

	// The number of bits within each bit set
	const size_type m_num_bits;

	// The number of blocks necessary to represent a bit set
	const size_type m_num_blocks;

	// The number of bit buffers in this collection
	size_type m_size;

	// A list of contiguous blocks
	// first member is the underlying bit buffer
	// second member is the number of bytes allocated to the buffer
	PkPooledRawBitSetChunkArray m_contiguous_chunks;

	// A bit set for keeping track of which chunks we are responsible for freeing
	PkBitSet m_owned_chunks_mask;
	
	// Our pool allocator
	PooledAllocator* mp_pool_alloc;
};

// Typedef for a non const pooled bit set handle
typedef PkPooledRawBitSetArray::PooledRawBitSetHandle PkPooledRawBitSetHandle;

// Typedef for a const pooled bit set handle
typedef PkPooledRawBitSetArray::PooledRawBitSetConstHandle  PkPooledRawBitSetConstHandle;

// Appends parameter bit set to pooled array
template< typename tBitSet >
void PkPooledRawBitSetArray::push_back( const tBitSet& bit_set )
{
	PkAssert( bit_set.size() == num_bits() );
	memcpy(
		  // note: allocate_bit_buffer() automatically updates size
		  ( sizeof( block_type ) == sizeof( tBitSet::block_type ) ) 
		  // if block sizes are different, then we need to zero unused bits
			? allocate_bit_buffer() : allocate_zeroed_bit_buffer()
		, &bit_set.get_buffer()[0]
		, std::min<>( num_bytes(), PkGetBitSetNumBytes( bit_set ) )
		);
}

// Appends bit set collection to pooled array
template< typename tBitSetArray >
void PkPooledRawBitSetArray::push_back_array( const tBitSetArray& bit_set_array )
{
	for ( size_type i=0; i<bit_set_array.size(); ++i )
	{
		push_back( bit_set_array[ i ] );
	}
}

#endif // PkPooledRawBitSetArray_h
