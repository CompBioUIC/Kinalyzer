/**
 * @author Alan Perez-Rathke 
 *
 * @date July 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSubsetOracle_h
#define PkSubsetOracle_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"

// A class which will tell you if a sibling set is a subset of another sibling set
class PkSubsetOracle
{
public:

	// Default constructor
	PkSubsetOracle() {}

	// Forwards to initialize from population size and total number of sibling sets
	PkSubsetOracle( const PkUInt populationSize, const PkUInt numSets )
	{
		init( populationSize, numSets );
	}

	// Initializes from population size and total number of sibling sets
	void init( const PkUInt populationSize, const PkUInt numSets )
	{
		this->getFrequencyMap().resize( populationSize );
		for ( PkUInt itrPop=0; itrPop<populationSize; ++itrPop )
		{
			this->getFrequencyMap()[ itrPop ].resize( numSets, 0 );
		}
	}

	// Updates a frequency map with the parameter sibling sets population membership
	template < typename tBitSet >
	void computeFrequencyMapFor( const tBitSet& siblingSet, const PkUInt setId )
	{
		PkAssert( setId >= 0 );
		for ( PkBitSet::size_type itrBit = siblingSet.find_first()
			; itrBit != tBitSet::npos
			; itrBit = siblingSet.find_next( itrBit ) )
		{
			PkAssert( itrBit >= 0 );
			PkAssert( itrBit < this->getFrequencyMap().size() );
			PkAssert( setId < this->getFrequencyMap()[ itrBit ].size() );
			this->getFrequencyMap()[ itrBit ].set( setId, true );
		}
	}

	// @return true if parameter sibling set is a subset, false otherwise
	template< typename tBitSet, typename tBitSetArray >
	bool isSubset( const tBitSet& siblingSet, const PkUInt setId, const tBitSetArray& siblingSets ) const
	{
		PkAssert( this->getFrequencyMap().size() > 0 );
		PkAssert( this->getFrequencyMap()[ 0 ].size() > 0 );
		PkAssert( this->getFrequencyMap()[ 0 ].size() > setId );
		PkAssert( setId >= 0 );

		// Early out if sibling set is empty!
		// @TODO: Need to pre-filter this sets out
		if ( !siblingSet.any() )
		{
			return true;
		}

		tBitSet::size_type itrBit = siblingSet.find_first();
		PkAssert( itrBit >= 0 );
		PkAssert( itrBit < this->getFrequencyMap().size() );

		PkBitSet superSets( this->getFrequencyMap()[ itrBit ] );

		for ( itrBit = siblingSet.find_next( itrBit )
			; itrBit != tBitSet::npos
			; itrBit = siblingSet.find_next( itrBit ) )
		{
			PkAssert( itrBit >= 0 );
			PkAssert( itrBit < this->getFrequencyMap().size() );
			PkAssert( setId < this->getFrequencyMap()[ itrBit ].size() );
			superSets &= this->getFrequencyMap()[ itrBit ];
		}

		PkAssert( superSets.test( setId ) );
		superSets.set( setId, false );

		const bool bIsSubset = superSets.any();
		if ( !bIsSubset )
		{
			// Early out of equality checks if we're not a subset
			return false;
		}

		itrBit = superSets.find_first();
		PkAssert( itrBit != setId );
		PkAssert( itrBit != PkBitSet::npos );
		if ( itrBit < setId )
		{
			// Early out of equality checks if we're not the lowest set id
			return true;
		}

		// We don't want to cull any subsets were they are all equal to each other.
		// If this is the case, accept the lowest set id as the super set.
		do
		{
			PkAssert( itrBit >= 0 );
			PkAssert( itrBit < siblingSets.size() );
			if ( siblingSets[ itrBit ] != siblingSet )
			{
				PkAssert( siblingSet.is_proper_subset_of( siblingSets[ itrBit ] ) );
				return true;
			}
			itrBit = superSets.find_next( itrBit );
		} while ( itrBit != tBitSet::npos );

		// We're the lowest set id and we're equal to all other subsets
		return false;
	}

private:

	// accessor for 2-d frequency table
	inline PkBitSetArray& getFrequencyMap()
	{
		return this->m_FrequencyMap;
	}

	// const accessor for 2-d frequency table
	inline const PkBitSetArray& getFrequencyMap() const
	{
		return this->m_FrequencyMap;
	}

	PkBitSetArray m_FrequencyMap;
};

#endif // PkSubsetOracle_h
