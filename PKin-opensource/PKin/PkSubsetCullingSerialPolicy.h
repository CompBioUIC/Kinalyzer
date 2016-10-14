/**
 * @author Alan Perez-Rathke 
 *
 * @date August 4, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSubsetCullingSerialPolicy_h
#define PkSubsetCullingSerialPolicy_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkBitSetUtils.h"

class PkSubsetCullingSerialPolicy
{
public:

	static void OnIntersectionJobFinish( const PkSiblingSetBitSetArray& jobOutputSets, PkSiblingSetBitSetArray& outputSets )
	{
		GlobalConditionalAppendAndCullSubSetsFor( jobOutputSets, outputSets );
	}

	static void OnIntersectionLoopFinish( PkSiblingSetBitSetArray& outputSets )
	{}

private:

	// Serially culls the intersector job results against the existing global output
	static void GlobalConditionalAppendAndCullSubSetsFor
	( 
	  const PkSiblingSetBitSetArray& jobOutputSets
	, PkSiblingSetBitSetArray& outputSets
	)
	{
		Pk_SCOPED_STAT_TIMER( ePkSTAT_LociIntersectionSynchronousSetCopyTime );

		// Early out if outputSets is empty
		if ( outputSets.empty() )
		{
			// Avoid all the conditional checks
			outputSets = jobOutputSets;
			return;
		}

		// Reserve a heuristic number of elements to reduce inner loop allocations
		// Heuristic: assuming ~25% of sets get culled
		outputSets.reserve( outputSets.size() + ( ( jobOutputSets.size() * 3 ) / 4 ) );

		// Determine range of sets to cull against to avoid redundant set cull checks
		// This is because the intersector jobs performs set culls internally
		const PkInt numSetsToCullAgainst = (PkInt)outputSets.size();
		for ( PkInt itrSets=((PkInt)jobOutputSets.size())-1; itrSets>=0; --itrSets )
		{
			PkBitSetUtils::conditionalAppendAndCullSubSets(
				  jobOutputSets[ itrSets ]
				, outputSets
				, 0 /* startItr */
				, numSetsToCullAgainst /* endItr */
				);
		}
	}

};

#endif // PkSubsetCullingSerialPolicy_h
