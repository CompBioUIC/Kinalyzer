/**
 * @author Alan Perez-Rathke 
 *
 * @date April 16, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroupUnorderedSet_h
#define PkSiblingGroupUnorderedSet_h

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkSiblingGroup.h"
#include <functional>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>

// An equivalence predicate that only compares against mapped alleles and parent sets
struct PkSiblingGroupEquivPred : std::binary_function<PkSiblingGroup, PkSiblingGroup, bool>
{
	bool operator()( const PkSiblingGroup& groupX, const PkSiblingGroup& groupY) const
	{
		return groupX.isEquivalent( groupY );
	}
};

// A hasher that hashes on parent sets and mapped alleles
struct PkSiblingGroupHasher : std::unary_function<PkSiblingGroup, std::size_t>
{
	// A utility function which computes the group hash and caches the result
	static inline void computeAndCacheHashFor( PkSiblingGroup& group )
	{
		group.setHash( computeHashFor( group ) );	
	}

	// A utility function which computes the hash for the parameter sibling group
	static std::size_t computeHashFor( const PkSiblingGroup& group )
	{
		std::size_t seed = 0;
		boost::hash_range( seed, group.getParentSets().begin(), group.getParentSets().end() );
		for ( PkInt iLocus=((PkInt)group.getMappedAlleles().size())-1; iLocus>=0; --iLocus )
		{
			boost::hash_range( seed, group.getMappedAlleles()[iLocus].begin(), group.getMappedAlleles()[iLocus].end() );
		}
		return seed;
	}

	// Returns the cached hash of the group
	std::size_t operator()(const PkSiblingGroup& group) const
	{
		// Ensure that the computed hash is the same as the cached hash value
		PkAssert( group.getHash() == computeHashFor( group ) );
		return group.getHash();
	}
};

// An unordered set of feasible sibling groups
typedef boost::unordered_set< PkSiblingGroup, PkSiblingGroupHasher, PkSiblingGroupEquivPred > PkSiblingGroupUnorderedSet;

#endif // PkSiblingGroupUnorderedSet_h
