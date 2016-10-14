/**
 * @author Alan Perez-Rathke 
 *
 * @date March 11, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroupIntersector_h
#define PkSiblingGroupIntersector_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkRunnable.h"

// A job that intersects sets of siblings across other sets of siblings
class PkSiblingSetIntersector : public PkRunnable
{
public:

	// Default constructor
	PkSiblingSetIntersector()
	: PkRunnable( false /* bIsManaged */ )
	, m_idxStart(-1)
	, m_idxEnd(-1)
	, m_pFirstSets( NULL )
	, m_pSecondSets( NULL )
	{}

	// Initializes job
	void init( const PkUInt idxStart, const PkUInt idxEnd, const PkSiblingSetBitSetArray& firstSets, const PkSiblingSetBitSetArray& secondSets );

	// Intersects parameter data sets across each other
	virtual PkBool doWork( const PkUInt threadId );

	// @return const reference to output sets
	inline const PkSiblingSetBitSetArray& getOutputSets() const
	{
		return m_OutputSets;
	}

private:

	// Avoid memory contention by starting intersections at different offsets based on thread id
	PkUInt getThreadOffset() const;

	// @return - const collection of second sets to intersect with
	inline const PkSiblingSetBitSetArray& getFirstSets() const
	{
		PkAssert( m_pFirstSets );
		return *m_pFirstSets;
	}

	// @return - const collection of second sets to intersect with
	inline const PkSiblingSetBitSetArray& getSecondSets() const
	{
		PkAssert( m_pSecondSets );
		return *m_pSecondSets;
	}

	// @return reference to output sets
	inline PkSiblingSetBitSetArray& getOutputSets()
	{
		return m_OutputSets;
	}

	// The start index of first sets to intersect with
	PkUInt m_idxStart;

	// One past the last index of first sets to intersect with
	PkUInt m_idxEnd;

	// A collection of sibling groups to intersect
	const PkSiblingSetBitSetArray* m_pFirstSets;
	
	// The collection of sibling groups to intersect against
	const PkSiblingSetBitSetArray* m_pSecondSets;

	// The final output set for this job
	PkSiblingSetBitSetArray m_OutputSets;
};

#endif // PkLocalSiblingReconstructor_h
