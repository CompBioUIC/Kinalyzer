/**

 * @author Alan Perez-Rathke 

 *

 * @date March 11, 2010

 *

 * Department of Computer Science

 * University of Illinois at Chicago 

 */



#ifndef PkFixedSizeArray_h

#define PkFixedSizeArray_h



#include "PkBuild.h"

#include "PkTypes.h"

#include "PkAssert.h"

#include "PkTypeTraits.h"



/**

* A bounds-checked in debug, fixed-size (static) array

* Mimics stl vector interface where appropriate

*/

template < typename T, PkUInt unsigned_maximum_size >

class PkFixedSizeArray

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



	// Default constructor

	PkFixedSizeArray() {}



	// STL conformance, this is only exposed to allow some flexibility in container types used

	explicit PkFixedSizeArray( size_type n )

	{

		PkAssert( n == maximum_size );

	}



	// STL conformance, initializes array with n copies of parameter value

	// n cannot exceed maximum size

	PkFixedSizeArray( size_type n, const_param_type init_val )

	{

		PkAssert( n < maximum_size );

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

		return &m_Data[maximum_size];

	}



	// STL conformance - return const iterator to one past last element in container

	inline const_iterator end() const

	{

		return &m_Data[maximum_size];

	}



	// STL conformance - random access to elements

	inline T& operator[]( const PkInt idx )

	{

		// bounds checking if enabled

		PkAssert( idx >= 0 );

		PkAssert( idx < maximum_size );

		return m_Data[ idx ];

	}



	// STL conformance - random access to const elements

	inline const T& operator[]( const PkInt idx ) const

	{

		// bounds checking if enabled

		PkAssert( idx >= 0 );

		PkAssert( idx < maximum_size );

		return m_Data[ idx ];

	}



	// STL conformance - return size of array

	inline size_type size() const

	{

		return maximum_size;

	}



	// STL conformance - return maximum size of array

	inline size_type max_size() const

	{

		return maximum_size;

	}



	// Partial STL conformance - expose a resize function, no default value used

	// This is only exposed to allow some flexibility in container types used

	inline void resize( size_type n )

	{

		// Only allow resize to same size!

		PkAssert( n == maximum_size );

	}



private:



	// Fixed size buffer for storing our elements

	T m_Data[ maximum_size ];

};



#endif // PkFixedSizeArray_h

