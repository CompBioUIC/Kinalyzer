/**
 * @author Alan Perez-Rathke 
 *
 * @date August 4, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSubsetCuller_h
#define PkSubsetCuller_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkRunnable.h"
#include "PkSubsetOracle.h"

// A class which will cull the subsets within a parameter interval
class PkSubsetCuller : public PkRunnable
{
public:

	PkSubsetCuller()
		: m_pSubsetsMask( NULL )
		, m_pSubsetOracle( NULL )
		, m_pSiblingSets( NULL )
		, m_BeginIndex( -1 )
		, m_EndIndex( -1 )
	{}

	void init( 
		  PkBitSet& outSubsetsMask
		, const PkSubsetOracle& subsetOracle
		, const PkSiblingSetBitSetArray& siblingSets
		, const PkUInt beginIndex
		, const PkUInt endIndex
		)
	{
		m_pSubsetsMask = &outSubsetsMask;
		m_pSubsetOracle = &subsetOracle;
		m_pSiblingSets = &siblingSets;
		m_BeginIndex = beginIndex;
		m_EndIndex = endIndex;
		checkConfig();
	}

	virtual PkBool doWork( const PkUInt threadId )
	{
		checkConfig();
		const PkSiblingSetBitSetArray& siblingSets( this->getSiblingSets() );
		PkBitSet& subsetsMask( this->getSubsetsMask() );
		const PkSubsetOracle& oracle( this->getSubsetOracle() );
		for ( PkUInt itrSets=this->getBeginIndex(); itrSets < this->getEndIndex(); ++itrSets )
		{
			subsetsMask.set( itrSets, !oracle.isSubset( siblingSets[ itrSets ], itrSets, siblingSets ) );
		}
		return true;
	}

	inline PkUInt getBeginIndex() const { return m_BeginIndex; }

	inline PkUInt getEndIndex() const { return m_EndIndex; }

private:

	inline void checkConfig() const
	{
		PkAssert( this->getBeginIndex() <= this->getEndIndex() );
		PkAssert( this->getBeginIndex() >= 0 );
		PkAssert( this->getEndIndex() <= this->getSiblingSets().size() );
		PkAssert( this->getSiblingSets().size() == this->getSubsetsMask().size() );
	}

	inline PkBitSet& getSubsetsMask()
	{
		PkAssert( NULL != this->m_pSubsetsMask );
		return *(this->m_pSubsetsMask);
	}

	inline const PkBitSet& getSubsetsMask() const
	{
		PkAssert( NULL != m_pSubsetsMask );
		return *(this->m_pSubsetsMask);
	}

	inline const PkSubsetOracle& getSubsetOracle() const
	{
		PkAssert( NULL != this->m_pSubsetOracle );
		return *(this->m_pSubsetOracle);
	}

	inline const PkSiblingSetBitSetArray& getSiblingSets() const
	{
		PkAssert( NULL != m_pSiblingSets );
		return *(this->m_pSiblingSets);
	}

	PkBitSet* m_pSubsetsMask;
	const PkSubsetOracle* m_pSubsetOracle;
	const PkSiblingSetBitSetArray* m_pSiblingSets;
	PkUInt m_BeginIndex;
	PkUInt m_EndIndex;
};

#endif // PkSubsetCuller_h
