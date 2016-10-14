/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkTypeTraits_h
#define PkTypeTraits_h

/**
* Basic type traits to facilitate optimal parameter passing,
* Mimics boost type traits library
*/
namespace Pk
{

/**
* Base value types
*/
struct true_type { enum { value = true }; }; 
struct false_type { enum { value = false }; };

/**
* Native floating point types
*/
template < typename T > struct is_float : public false_type {};

template <> struct is_float< float > : public true_type {};
template <> struct is_float< double > : public true_type {};
template <> struct is_float< long double > : public true_type {};

/**
 * Native integral types
 */
template < typename T > struct is_integral : public false_type {};

template <> struct is_integral< signed char > : public true_type {};
template <> struct is_integral< signed short > : public true_type {};
template <> struct is_integral< signed int > : public true_type {};
template <> struct is_integral< signed long > : public true_type {};

template <> struct is_integral< unsigned char > : public true_type {};
template <> struct is_integral< unsigned short > : public true_type {};
template <> struct is_integral< unsigned int > : public true_type {};
template <> struct is_integral< unsigned long > : public true_type {};

template <> struct is_integral< bool > : public true_type {};
template <> struct is_integral< char > : public true_type {};

// wchar_t, may not be a native type with all compilers
template <> struct is_integral< wchar_t > : public true_type {};

// C99, may not be compatible with all compilers
template <> struct is_integral< signed long long > : public true_type {};
template <> struct is_integral< unsigned long long > : public true_type {};

/**
 * Arithmetic types (float and integral types)
 */
template < typename T > struct is_arithmetic { enum { value = is_float< T >::value || is_integral< T >::value }; };

/**
 * Pointer types
 */
// todo: member pointers should not be included as basic pointer types
template < typename T > struct is_pointer : public false_type {};
template < typename T > struct is_pointer< T* > : public true_type {};
template < typename T > struct is_pointer< const T* > : public true_type {};
template < typename T > struct is_pointer< const T* const > : public true_type {};
template < typename T > struct is_pointer< T* volatile > : public true_type {};
template < typename T > struct is_pointer< T* const volatile > : public true_type {};

/**
 * Void types
 */
template < typename T > struct is_void : public false_type {};
template <> struct is_void< void > : public true_type {};
template <> struct is_void< void const > : public true_type {};
template <> struct is_void< void volatile > : public true_type {};
template <> struct is_void< void const volatile > : public true_type {};

/**
* Selector utility classes
*/
template < typename T, bool should_pass_by_value >
struct param_type_selector
{
	typedef const T& param_type;
	typedef const T& const_param_type;
};

template <typename T>
struct param_type_selector<T, true /* should_pass_by_value */ >
{
	typedef const T param_type;
	typedef const T const_param_type;
};

/**
* Call traits - contains optimal parameter passing type (POD not supported)
*/
template <typename T>
struct param_traits
{
private:
	enum { should_pass_by_value = is_arithmetic<T>::value || is_pointer<T>::value };
public:
	typedef typename param_type_selector< T, should_pass_by_value >::param_type param_type;
	typedef typename param_type_selector< T, should_pass_by_value >::const_param_type const_param_type;
};

} // end of Pk namespace

#endif // PkTypeTraits_h
