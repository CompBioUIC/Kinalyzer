/**
 * @author Alan Perez-Rathke 
 *
 * @date July 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSubsetFrequencyMapper_h
#define PkSubsetFrequencyMapper_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkRunnable.h"
#include "PkSubsetOracle.h"

// A class which maps sibling sets into a frequency map
class PkSubsetFrequencyMapper : public PkRunnable
{
public:

	PkSubsetFrequencyMapper()
		: m_pSubsetOracle( NULL )
		, m_pSiblingSets( NULL )
		, m_BeginIndex( -1 )
		, m_EndIndex( -1 )
	{}

	void init( PkSubsetOracle& subsetOracle, const PkSiblingSetBitSetArray& siblingSets, const PkUInt beginIndex, const PkUInt endIndex )
	{
		m_pSubsetOracle = &subsetOracle;
		m_pSiblingSets = &siblingSets;
		m_BeginIndex = beginIndex;
		m_EndIndex = endIndex;
		checkConfig();
	}

	virtual PkBool doWork( const PkUInt threadId )
	{
		checkConfig();
		PkSubsetOracle& oracle( this->getSubsetOracle() );
		const PkSiblingSetBitSetArray& siblingSets( this->getSiblingSets() );
		for ( PkUInt itrSets=this->getBeginIndex(); itrSets < this->getEndIndex(); ++itrSets )
		{
			oracle.computeFrequencyMapFor( siblingSets[ itrSets ], itrSets );
		}
		return true;
	}

private:

	inline void checkConfig() const
	{
		PkAssert( this->getBeginIndex() <= this->getEndIndex() );
		PkAssert( this->getBeginIndex() >= 0 );
		PkAssert( this->getEndIndex() <= this->getSiblingSets().size() );
	}

	inline PkSubsetOracle& getSubsetOracle()
	{
		PkAssert( NULL != this->m_pSubsetOracle );
		return *(this->m_pSubsetOracle);
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

	inline PkUInt getBeginIndex() const { return m_BeginIndex; }

	inline PkUInt getEndIndex() const { return m_EndIndex; }

	PkSubsetOracle* m_pSubsetOracle;
	const PkSiblingSetBitSetArray* m_pSiblingSets;
	PkUInt m_BeginIndex;
	PkUInt m_EndIndex;
};

#endif // PkSubsetFrequencyMapper_h
