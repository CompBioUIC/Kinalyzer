/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkLociArray_h
#define PkLociArray_h

#include "PkBuild.h"

#if Pk_ENABLE_SMALL_LOCI_ARRAY

#include "PkFixedSizeStack.h"
#include "PkAssert.h"

// The maximum allowed loci assignable to a thread pool job
#define Pk_MAX_LOCI_ARRAY_SIZE 5

// Optimization:
// Avoid large quantities of small dynamic memory allocations
// by capping the number of loci assignable to a thread pool job
// to a small number.  This allows more memory to be allocated
// from the stack instead of dynamically from the heap.
template < typename T >
class PkLociArray_t : public PkFixedSizeStack< T, Pk_MAX_LOCI_ARRAY_SIZE > 
{
public:

	// Expose types

	typedef PkFixedSizeStack< T, Pk_MAX_LOCI_ARRAY_SIZE > stack_type;
	typedef typename stack_type::size_type size_type;
	typedef typename stack_type::const_param_type const_param_type;

	// Route constructors:

	// STL conformance - default constructor, initializes size to zero
	inline PkLociArray() {}

	// STL conformance, allow initial stack size
	// n cannot exceed maximum size
	inline explicit PkLociArray( size_type n ) : PkFixedSizeStack< T, Pk_MAX_LOCI_ARRAY_SIZE >( n ) {}

	// STL conformance, initializes array with n copies of parameter value
	// n cannot exceed maximum size
	inline PkLociArray( size_type n, const_param_type init_val ) : PkFixedSizeStack< T, Pk_MAX_LOCI_ARRAY_SIZE >( n, init_val ) {}

	// STL conformance, asserts the reserve call will fit in the stack memory
	inline void reserve( const size_type n ) { PkAssert( n <= Pk_MAX_LOCI_ARRAY_SIZE ); }
};

#define PkLociArray( type ) PkLociArray< type >

#else

// Else, if we're not comfortable with bounding the loci arrays,
// then we'll have to resort to using dynamic arrays instead
#include "PkTypes.h"
#define PkLociArray PkArray

#endif // Pk_ENABLE_SMALL_LOCI_ARRAY

#endif // PkLociArray_h
