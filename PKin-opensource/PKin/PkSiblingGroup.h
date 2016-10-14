/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroup_h
#define PkSiblingGroup_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkParentSet.h"
#include "PkFixedSizeStack.h"
#include "PkLociArray.h"
#include "PkIndividual.h"
#include "PkMiscUtil.h"

// Enumerated return codes for result of assigning an individual to a sibling group
enum EPkSiblingGroupAssignmentReturnCode
{
	// The assignment failed through violation of 4-allele
	  ESGARC_Fail = 0
	// The assignment was a success, no new alleles were mapped
	, ESGARC_Success_NoChange
	// The assignment was a success, new alleles were mapped
	, ESGARC_Success_Modified
	// The total number of return codes
	, ESGARC_Max
};

/**
* A sibling group represents a collection of individuals that could be siblings
*/
class PkSiblingGroup
{
public:

	// Default constructor
	PkSiblingGroup()
	{}

	// Constructor which forwards to init()
	inline PkSiblingGroup( const PkIndividual& individual, const PkInt idxIndividual, const PkLociArray( PkInt )& lociIndices, const PkUInt popSize )
	{
		init( individual, idxIndividual, lociIndices, popSize );
	}

	// Initializes a sibling group from a single individual
	void init( const PkIndividual& individual, const PkInt idxIndividual, const PkLociArray( PkInt )& lociIndices, const PkUInt popSize );
	
	// @return - const reference to collection of indices of individuals assigned to this group
	inline const PkBitSet& getAssignedIndividuals() const
	{
		return m_AssignedIndividuals;
	}

	// @return - the number of individuals assigned to this group
	inline PkUInt getNumAssignedIndividuals() const
	{
		return m_AssignedIndividuals.count();
	}

	// @return - true if individual belongs to this sibling group, false otherwise
	inline PkBool hasIndividual( const PkInt idxIndividual ) const
	{
		PkAssert( (idxIndividual >= 0) && (idxIndividual < (PkInt)m_AssignedIndividuals.size()) );
		return m_AssignedIndividuals.test( idxIndividual );
	}

	// @return - true if allele mappings and parent sets are equivalent at each locus
	bool isEquivalent( const PkSiblingGroup& other ) const;

	// Assigns an individual to this sibling group if possible
	// Will potentially modify group even if assignment failed!!
	// @return - enumerated code defining result of assignment
	EPkSiblingGroupAssignmentReturnCode conditionalAssignIndividual( const PkIndividual& individual, const PkInt idxIndividual, const PkLociArray( PkInt )& lociIndices );

	// Force assigns an individual to this group
	// Does not check to see if it's possible!
	inline void forceAssignIndividual( const PkInt idxIndividual )
	{
		// Assert on bounds check
		PkAssert( 0 <= idxIndividual );
		PkAssert( this->getAssignedIndividuals().size() > (PkUInt)idxIndividual );
		// Assert individual doesn't already exist
		PkAssert( false == this->getAssignedIndividuals().test( idxIndividual ) );
		// Set bit to true for this individual
		this->getAssignedIndividuals().set( idxIndividual, true );
	}

	// Performs a union operation of assigned individuals for both groups and stores result in this group
	inline void forceAssignIndividualsFrom( const PkSiblingGroup& other )
	{
		this->getAssignedIndividuals() |= other.getAssignedIndividuals();
	}

protected:

	// Data structure that contains the mapped alleles at a single locus
	// Allow an additional padding element to detect violations of 4-allele
	typedef PkFixedSizeStack< PkInt, Pk_MAX_ALLELES_PER_LOCUS+1 > tAlleleMap;

	// Data structure for storing all the mapped alleles assigned to this sibling group
	typedef PkLociArray( tAlleleMap ) tAlleleMapArray;

	// Does not check to see if mapping is possible (i.e. - if 4-allele is broken)!
	// That is left to the client code.
	// @return - mapping of allele at parameter locus
	inline static PkInt forceMapUniqueAllele( tAlleleMap& alleleMap, const PkInt unmappedAllele )
	{
		// Handle wildcards
		return ( Pk_WILDCARD_UNMAPPED_ALLELE != unmappedAllele ) ?
			((&(*Pk_push_back_unique( alleleMap, unmappedAllele ))) - (&(alleleMap[0]))) :
				Pk_WILDCARD_MAPPED_ALLELE;
	}

	// Maps an allele preserving ordering and can remap parent set
	// @return - true if 4-allele was not violated, false otherwise
	static PkBool verboseConditionalMapAllele( const PkInt unmappedAllele, PkInt& out_MappedAllele, PkBool& out_bIsModified, tAlleleMap& out_AlleleMap, PkParentSet& out_ParentSet );

	// @return - reference to mapped alleles at each loci
	inline tAlleleMapArray& getMappedAlleles()
	{
		return m_MappedAlleles;
	}

	// @return - const reference to mapped alleles at loci
	inline const tAlleleMapArray& getMappedAlleles() const
	{
		return m_MappedAlleles;
	}

	// @return - reference to bitset of parents for each locally computed loci
	inline PkLociArray( PkParentSet )& getParentSets()
	{
		return m_ParentSets;
	}

	// @return - const reference to bitset of parents for each locally computed loci
	inline const PkLociArray( PkParentSet)& getParentSets() const
	{
		return m_ParentSets;
	}

	// @return - reference to collection of indices of individuals assigned to this group
	inline PkBitSet& getAssignedIndividuals()
	{
		return m_AssignedIndividuals;
	}

	// mutator which caches a computed hash value for this sibling group
	inline void setHash( const std::size_t h )
	{
		m_CachedHash = h;
	}

	// @return the cached hash value for this sibling group
	inline std::size_t getHash() const
	{
		return m_CachedHash;
	}

private:

	// The cached hash value for this sibling group
	std::size_t m_CachedHash;

	// The mapped alleles at each loci
	tAlleleMapArray m_MappedAlleles;

	// A bitset of possible parents for each locally computed loci
	PkLociArray( PkParentSet ) m_ParentSets;

	// Indices into the global population for individuals that belong to this group
	PkBitSet m_AssignedIndividuals;

	// Allow tightly coupled hasher to access private data
	friend struct PkSiblingGroupHasher;
};

#endif // PkSiblingGroup_h
