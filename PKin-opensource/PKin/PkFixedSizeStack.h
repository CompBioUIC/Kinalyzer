/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkFixedSizeStack_h
#define PkFixedSizeStack_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkTypeTraits.h"

/**
* A bounds-checked in debug, fixed-size (static) stack
* Mimics stl vector interface where appropriate
*/
template < typename T, PkUInt unsigned_maximum_size >
class PkFixedSizeStack
{
public:

	// STL conformance - our simple, non-const iterator type
	typedef T* iterator;

	// STL conformance - our simple, const iterator type
	typedef const T* const_iterator;

	// STL conformance - used for returning size of array
	typedef PkUInt size_type;

	// STL conformance - expose the type of our template parameter for stl algorithms
	typedef T value_type;

	// Use type traits to determine optimal non-const parameter passing type
	typedef typename Pk::param_traits< value_type >::param_type param_type;

	// Use type traits to determine optimal const parameter passing type
	typedef typename Pk::param_traits< value_type >::const_param_type const_param_type;

	// Compile-time constant, store our fixed size
	enum { maximum_size = unsigned_maximum_size };

	// STL conformance - default constructor, initializes size to zero
	PkFixedSizeStack() 
	: m_Size( 0 ) 
	{}

	// STL conformance, allow initial stack size
	// n cannot exceed maximum size
	explicit PkFixedSizeStack( size_type n )
	{
		PkAssert( n <= this->max_size() );
		PkAssert( n >= 0 );
		m_Size = n;
	}

	// STL conformance, initializes array with n copies of parameter value
	// n cannot exceed maximum size
	PkFixedSizeStack( size_type n, const_param_type init_val )
	: m_Size( n )
	{
		PkAssert( n <= this->max_size() );
		PkAssert( n >= 0 );
		m_Size = n;
		// @todo - potentially use a memset for small native types
		for ( size_type i=0; i<n; ++i )
		{
			m_Data[ i ] = init_val;
		}
	}

	// STL conformance - return iterator to first element in container
	inline iterator begin()
	{
		return &m_Data[0];
	}

	// STL conformance - return const iterator to first element in container
	inline const_iterator begin() const
	{
		return &m_Data[0];
	}

	// STL conformance - return const iterator to one past last element in container
	inline iterator end()
	{
		return &m_Data[this->size()];
	}

	// STL conformance - return const iterator to one past last element in container
	inline const_iterator end() const
	{
		return &m_Data[this->size()];
	}

	// STL conformance - random access to elements
	inline T& operator[]( const PkInt idx )
	{
		PkAssert( idx >= 0 );
		PkAssert( idx < (PkInt)this->size() );
		return m_Data[ idx ];
	}

	// STL conformance - random access to const elements
	inline const T& operator[]( const PkInt idx ) const
	{
		PkAssert( idx >= 0 );
		PkAssert( idx < (PkInt)this->size() );
		return m_Data[ idx ];
	}

	// STL conformance - return size of array
	inline size_type size() const
	{
		return m_Size;
	}

	// STL conformance - return maximum size of array
	inline size_type max_size() const
	{
		return maximum_size;
	}

	// STL conformance - not exactly same return type
	// @return true if no elements have been added to the stack, false otherwise
	inline PkBool empty() const
	{
		return ( this->size() == 0 );
	}

	// STL conformance - sets size of stack back to zero
	inline void clear()
	{
		m_Size = 0;
	}

	// Partial STL conformance - expose a resize function, no default value used
	// sets new size of stack, n cannot exceed maximum size
	inline void resize( size_type n )
	{
		PkAssert( n <= this->max_size() );
		PkAssert( n >= 0 );
		m_Size = n;
	}

	// STL conformance - add new element on top of stack
	// bounds checking if asserts are enabled
	inline void push_back( const_param_type value )
	{
		PkAssert( this->size() < this->max_size() );
		m_Data[ m_Size++ ] = value;
	}

	// STL conformance - remove top element of stack
	// bounds checking if asserts are enabled
	inline void pop_back()
	{
		PkAssert( this->size() > 0 );
		--m_Size;
	}

	// STL conformance - return element on bottom of stack
	inline T& front()
	{
		PkAssert( this->size() > 0 );
		return m_Data[ 0 ];
	}

	// STL conformance - return const element on bottom of stack
	inline const T& front() const
	{
		PkAssert( this->size() > 0 );
		return m_Data[ 0 ];
	}

	// STL conformance - return element on top of stack
	inline T& back()
	{
		PkAssert( this->size() > 0 );
		return m_Data[ this->size() - 1 ];
	}

	// STL conformance - return const element on top of stack
	inline const T& back() const
	{
		PkAssert( this->size() > 0 );
		return m_Data[ this->size() - 1 ];
	}

private:

	// Fixed size buffer for storing our elements
	T m_Data[ maximum_size ];

	// Data member for keeping track of how many elements have been added
	size_type m_Size;
};

#endif // PkFixedSizeStack_h
