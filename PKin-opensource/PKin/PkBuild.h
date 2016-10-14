/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkBuild_h
#define PkBuild_h

#if WINDOWS
	// Windows specific defines go here
	#define Pk_ENABLE_WINDOWS_THREADS 1
	#define Pk_ENABLE_BOOST_THREADS 0
	#define Pk_ENABLE_UNIX_THREADS 0
	#define Pk_PATH_SEPARATOR_STRING "\\"
	#define stricmp _stricmp // HACK to remove VC++ complaining about POSIX
	#define Pk_64_BIT_INT __int64
#elif UNIX
	#if ARGO
		// Argo specific defines go here
		#define Pk_ENABLE_WINDOWS_THREADS 0
		#define Pk_ENABLE_BOOST_THREADS 0
		#define Pk_ENABLE_UNIX_THREADS 1
	#else
		// Default unix defines go here
		#define Pk_ENABLE_WINDOWS_THREADS 0
		#define Pk_ENABLE_BOOST_THREADS 0
		#define Pk_ENABLE_UNIX_THREADS 1
	#endif
	#define Pk_PATH_SEPARATOR_STRING "/"
#endif

#ifndef Pk_ENABLE_ASSERT
	// This switch toggles assertion checking
	#define Pk_ENABLE_ASSERT 1
#endif

#ifndef Pk_ENABLE_LOGGING
	// This switch toggles logging
	#define Pk_ENABLE_LOGGING 1
#endif

#ifndef Pk_ENABLE_STATS
	// This switch toggles stats
	#define Pk_ENABLE_STATS 1
#endif

#ifndef Pk_ENABLE_COMMANDLETS
	// This switch toggles whether or not commandlets are compiled
	#define Pk_ENABLE_COMMANDLETS 0
#endif

// Application specific defines begin here

#ifndef Pk_ENABLE_SMALL_LOCI_ARRAY
	// This switch toggles whether or not we use small loci arrays
	#define Pk_ENABLE_SMALL_LOCI_ARRAY 0
#endif

// The number of parent sets that are possible for alleles mapped 1 to 4
// that satisfy 4-allele and 2-allele properties
#define Pk_NUM_POSSIBLE_PARENT_SETS 14

// Maximum alleles allowed at each locus as defined by the 4-allele property
#define Pk_MAX_ALLELES_PER_LOCUS 4

// Define a constant for an invalid allele
#define Pk_INVALID_ALLELE -2

// Define a constant for an unmapped wildcard allele
#define Pk_WILDCARD_UNMAPPED_ALLELE -1

// Define a constant for a mapped wildcard allele
#define Pk_WILDCARD_MAPPED_ALLELE Pk_MAX_ALLELES_PER_LOCUS

// A constant for invalid indices
#define Pk_INVALID_INDEX -1

#endif // PkBuild_h
