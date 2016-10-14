/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkSiblingGroup.h"
#include "PkMiscUtil.h"
#include "PkPossibleParentsMap.h"
#include "PkParentRemapper.h"
#include <string.h>

// Initializes a sibling group from a single individual
void PkSiblingGroup::init( const PkIndividual& individual, const PkInt idxIndividual, const PkLociArray( PkInt )& lociIndices, const PkUInt popSize )
{
	// Cache number of loci that we're working with
	const PkInt numLoci = lociIndices.size();
	PkAssert( 0 < numLoci );

	// Resize mapped alleles and parent sets
	this->getMappedAlleles().resize( numLoci );
	this->getParentSets().resize( numLoci );

	// Map alleles and compute parent set for each loci
	for ( PkInt itrLocus=0; itrLocus<numLoci; ++itrLocus )
	{
		// Obtain reference to unmapped alleles from individual at this locus
		const PkLocus& locus = individual[ lociIndices[ itrLocus ] ];

		// Obtain reference to allele map at this locus
		tAlleleMap& alleleMap = this->getMappedAlleles()[ itrLocus ];

		// Ensure that alleles are ordered properly
		PkAssert( locus.first <= locus.second );

		// Map first allele for individual - preserves ordering
		const PkInt mappedAlleleX = forceMapUniqueAllele( alleleMap, locus.first );
		PkAssert( mappedAlleleX >= 0 );
		PkAssert( ( mappedAlleleX < Pk_MAX_ALLELES_PER_LOCUS ) || ( Pk_WILDCARD_UNMAPPED_ALLELE == locus.first ) );

		// Map second allele for individual - preserves ordering
		const PkInt mappedAlleleY = forceMapUniqueAllele( alleleMap, locus.second );
		PkAssert( mappedAlleleY >= 0 );
		PkAssert( ( mappedAlleleY < Pk_MAX_ALLELES_PER_LOCUS ) || ( Pk_WILDCARD_UNMAPPED_ALLELE == locus.second ) );

		// Store parent set at this locus
		this->getParentSets()[ itrLocus ] = PkPossibleParentsMap::map( mappedAlleleX, mappedAlleleY );
		PkAssert( !Pk_IS_EMPTY_PARENT_SET( this->getParentSets()[ itrLocus ] ) );
	}

	// Size our population bitset
	PkAssert( this->getAssignedIndividuals().empty() );
	this->getAssignedIndividuals().resize( popSize );
	
	// Assign individual to group
	this->forceAssignIndividual( idxIndividual );
}

// @return - true if allele mappings and parent sets are equivalent at each locus
bool PkSiblingGroup::isEquivalent( const PkSiblingGroup& other ) const
{
	// Assert we're comparing sibling groups from same locus family
	PkAssert( m_MappedAlleles.size() == m_ParentSets.size() );
	PkAssert( m_MappedAlleles.size() == other.m_MappedAlleles.size() );
	PkAssert( other.m_MappedAlleles.size() == other.m_ParentSets.size() );

	// Test to see if parent sets match at each locus
	if ( 0 != memcmp( &(m_ParentSets[0]), &(other.m_ParentSets[0]), m_ParentSets.size() * sizeof( PkParentSet ) ) )
	{
		// Parent sets don't match and therefore, we're not equivalent
		return false;
	}
	
	// Test to see if mapped alleles match at each locus
	for ( PkInt iLocus=m_MappedAlleles.size()-1; iLocus>=0; --iLocus )
	{
		// Store references to alleleMap at current locus
		const tAlleleMap& ourAlleleMap = m_MappedAlleles[ iLocus ];
		const tAlleleMap& theirAlleleMap = other.m_MappedAlleles[ iLocus ];
		
		// Early out if sizes do not match
		if ( ourAlleleMap.size() != theirAlleleMap.size() )
		{
			return false;
		}

		// Compare equivalence for allele mappings at this locus
		if
		(
		   // Note: don't need to compare theirAlleleMap.size() because it's same as ourAlleleMap from comparison above
		   // - also, the size can be zero due to wildcards
		   ( ourAlleleMap.size() > 0 )
		&& ( 0 != memcmp( &(ourAlleleMap[0]), &(theirAlleleMap[0]), ourAlleleMap.size() * sizeof( tAlleleMap::value_type ) ) )
		)
		{
			// Alleles mapped at this locus do not match
			return false;
		}
	}

	// We're equivalent!
	return true;
}

// Maps an allele preserving ordering and can remap parent set
// @return - true if 4-allele was not violated, false otherwise
PkBool PkSiblingGroup::verboseConditionalMapAllele( const PkInt unmappedAllele, PkInt& out_MappedAllele, PkBool& out_bIsModified, tAlleleMap& out_AlleleMap, PkParentSet& out_ParentSet )
{
	// Special case: early out if unmapped allele is a wildcard
	if ( Pk_WILDCARD_UNMAPPED_ALLELE == unmappedAllele )
	{
		// Note: we do not actually map the allele
		out_MappedAllele = Pk_WILDCARD_MAPPED_ALLELE;
		return true;
	}

	PkInt alleleMapSize = out_AlleleMap.size();
	PkAssert( alleleMapSize <= Pk_MAX_ALLELES_PER_LOCUS );
	for ( out_MappedAllele=0; out_MappedAllele<alleleMapSize; ++out_MappedAllele )
	{
		if ( unmappedAllele == out_AlleleMap[ out_MappedAllele ] )
		{
			// We already exist! Early out.
			return true;
		}
		
		// See if we can be mapped while preserving ordering
		if ( unmappedAllele < out_AlleleMap[ out_MappedAllele ] )
		{
			break;	
		}
	}

	// Loop postconditions:
	PkAssert( out_MappedAllele >= 0 );
	PkAssert( out_MappedAllele <= Pk_MAX_ALLELES_PER_LOCUS );

	// We're out of loop, out_MappedAllele is at the index we need to put the unmapped allele
	if ( alleleMapSize >= Pk_MAX_ALLELES_PER_LOCUS )
	{
		// We already have too many alleles! Early out.
		return false;
	}

	// Make room for new allele
	out_AlleleMap.resize( ++alleleMapSize );
	PkAssert( out_AlleleMap.size() <= Pk_MAX_ALLELES_PER_LOCUS );
	PkAssert( out_AlleleMap.size() == alleleMapSize );

	// Remap alleles and parent sets that are greater than currently mapped allele
	// Note: have to start 2 before because alleleMapSize was padded by 1 above
	for ( PkInt idxPreRemap = alleleMapSize-2; idxPreRemap >= out_MappedAllele; --idxPreRemap ) 
	{
		// Verify we have a sorted order
		PkAssert( unmappedAllele < out_AlleleMap[ idxPreRemap ] );

		// Shift allele up
		out_AlleleMap[ idxPreRemap+1 ] = out_AlleleMap[ idxPreRemap ];
		
		// Remap parent set
		const PkParentSet tempParentSet = out_ParentSet;
		out_ParentSet = 0;
		PkInt parentRemapBitIndex = Pk_INVALID_INDEX;
		for ( PkInt bitIdxParent=0; bitIdxParent<Pk_NUM_POSSIBLE_PARENT_SETS; ++bitIdxParent )
		{
			if 
			( 
			   ( ( PkInt(1) << bitIdxParent ) & tempParentSet )
			&& ( ( parentRemapBitIndex = PkParentRemapper::getRemapBitIndex( bitIdxParent, idxPreRemap ) ) != Pk_INVALID_INDEX )
			)
			{
				out_ParentSet |= PkInt(1) << parentRemapBitIndex;
			}
		}
	}

	// Finally, map the parameter allele
	out_AlleleMap[ out_MappedAllele ] = unmappedAllele;

	// We mapped a new allele, signal that we modified stuff
	out_bIsModified = true;

	// Allele was successfully mapped!
	return true;
}

// Assigns an individual to this sibling group if possible
// Will potentially modify group even if assignment failed!!
// @return - enumerated code defining result of assignment
EPkSiblingGroupAssignmentReturnCode PkSiblingGroup::conditionalAssignIndividual( const PkIndividual& individual, const PkInt idxIndividual, const PkLociArray( PkInt )& lociIndices )
{
	// This value is true if new alleles were mapped
	PkBool bIsModified = false;

	// Cache number of loci that we're working with
	const PkInt numLoci = lociIndices.size();
	PkAssert( 0 < numLoci );

	// Map alleles and compute parent set for each loci
	for ( PkInt itrLocus=0; itrLocus<numLoci; ++itrLocus )
	{
		// Obtain reference to unmapped alleles from individual at this locus
		const PkLocus& locus = individual[ lociIndices[ itrLocus ] ];

		// Obtain reference to allele map at this locus
		tAlleleMap& alleleMap = this->getMappedAlleles()[ itrLocus ];

		// Store intersection of parent set at this locus
		PkParentSet& parentSet = this->getParentSets()[ itrLocus ];

		// Map first allele for individual
		PkInt mappedAlleleX = Pk_INVALID_ALLELE;
		if ( !verboseConditionalMapAllele( locus.first, mappedAlleleX, bIsModified, alleleMap, parentSet ) )
		{
			return ESGARC_Fail;
		}

		// Map second allele for individual
		PkInt mappedAlleleY = Pk_INVALID_ALLELE;
		if ( !verboseConditionalMapAllele( locus.second, mappedAlleleY, bIsModified, alleleMap, parentSet ) )
		{
			return ESGARC_Fail;
		}

		// Store intersection of parent set at this locus
		const PkParentSet previousParentSet = parentSet;
		parentSet &= PkPossibleParentsMap::map( mappedAlleleX, mappedAlleleY );
		
		// See if we violated 2-allele resulting in an empty parent set
		if ( Pk_IS_EMPTY_PARENT_SET( parentSet ) )
		{
			return ESGARC_Fail;
		}

		// Determine if parent set was modified
		bIsModified |= ( previousParentSet != parentSet );
	}

	// Assign individual to group
	this->forceAssignIndividual( idxIndividual );

	// Determine return code based on if a modification was made or not
	return bIsModified ? ESGARC_Success_Modified : ESGARC_Success_NoChange;
}
