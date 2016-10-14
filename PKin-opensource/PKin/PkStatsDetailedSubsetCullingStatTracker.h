/**s * @author Alan Perez-Rathke s *
 * @date September 13, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkStatsDetailedSubsetCullingStatTracker_h
#define PkStatsDetailedSubsetCullingStatTracker_h

#include "PkBuild.h"
#include "PkStats.h"

// Enable this switch to generate histogram statistics for sibling group sparsity
#define Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS 0

#if (Pk_ENABLE_STATS && Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS)

#include "PkAssert.h"
#include "PkTypes.h"
#include "PkFixedSizeArray.h"

namespace PkStats
{

// A class for tracking detailed subset culling statistics
// Yes, there's lots of LHS from PkInt to float casts, I don't care!!
class DetailedSubsetCullingStatTracker
{
public:
	
	// The percentage steps for sparsity tracking
	enum { eSPARSITY_INTERVAL_PERCENTAGE = 2 };

	// Constructor, intializes bit counts to zero
	DetailedSubsetCullingStatTracker()
		: m_NumSiblingGroups( 0 )
		, m_NumSubsets( 0 )
	{
		PkAssert( getNumBuckets() > 0 );
		// zero our internal statistics (could use memset, but am choosing not to)
		std::fill( m_SparsityPercentages.begin(), m_SparsityPercentages.end(), 0 );
	}

	// Updates the bit count tracking for parameter sibling group
	void updateStatsFor( const PkBitSet& siblingSet, const PkBool bIsSubset )
	{
		// Update total number of sibling groups encountered
		++m_NumSiblingGroups;
		// Update total number of subsets encountered
		m_NumSubsets += bIsSubset;
		// Ensure that bitset library isn't bonkers
		PkAssert( siblingSet.size() >= siblingSet.count() );
		PkAssert( siblingSet.size() > 0 );
		// Determine which bucket to increment count for
		double percentZeros = 100.0 * ( siblingSet.size() - siblingSet.count() );
		percentZeros /= (double)siblingSet.size();
		percentZeros /= (double)eSPARSITY_INTERVAL_PERCENTAGE;
		const PkUInt idxBucket = std::min<PkUInt>( (PkUInt)percentZeros, getNumBuckets()-1 );
		++m_SparsityPercentages[ idxBucket ];
	}
	
	void updateStats( const PkBitSetArray& siblingSets, const PkBitSet& subsetsMask )
	{
		PkAssert( siblingSets.size() == subsetsMask.size() );
		for ( PkUInt i=0; i<siblingSets.size(); ++i )
		{
			updateStatsFor( siblingSets[ i ], !subsetsMask[ i ] );
		}
	}

	// @return number of histogram buckets we're tracking
	inline PkUInt getNumBuckets() const
	{
		return m_SparsityPercentages.size();
	}

	// @return frequency of bucket occurence in range [0,1]
	double getNormalizedBucketCount( const unsigned PkInt idxBucket ) const
	{
		PkAssert( m_SparsityPercentages[ idxBucket ] <= m_NumSiblingGroups );
		const double fSparsityPercentage = m_SparsityPercentages[ idxBucket ];
		return fSparsityPercentage / ( (double) m_NumSiblingGroups );
	}

	// @return average sparsity percentage of buckets from [0,100]
	double getAverageSparsityPercentage() const
	{
		double average = 0.0;
		PkUInt weight = 0;
		for ( PkUInt idxBucket=0; idxBucket<getNumBuckets(); ++idxBucket )
		{
			average += ((double) weight) * getNormalizedBucketCount( idxBucket );
			weight += eSPARSITY_INTERVAL_PERCENTAGE;
		}
		PkAssert( weight == 100 );
		return average;
	}

	// @return proportion of generated sibling groups which are subsets from [0,100]
	double getNormalizedSubsetsCount() const
	{
		PkAssert( m_NumSubsets <= m_NumSiblingGroups );
		return 100.0 * ((double) m_NumSubsets) / std::max<double>( m_NumSiblingGroups , 1.0 );
	}

private:

	// Keep track of sparsity percentages over granular intervals
	PkFixedSizeArray< unsigned PkInt, (100 / eSPARSITY_INTERVAL_PERCENTAGE) > m_SparsityPercentages;

	// Keep track of how many sibling groups we've encountered
	unsigned PkInt m_NumSiblingGroups;

	// Keep track of how many subsets we've encountered
	unsigned PkInt m_NumSubsets;
};

// Declare our global instance for detailed stat tracking
extern DetailedSubsetCullingStatTracker GDetailedSubsetCullingStatTracker;

} // end of PkStats namespace

// Active macro
#define Pk_STATS_UPDATE_FOR_SUBSET_CULLING( bitsets, subsetsMask ) PkStats::GDetailedSubsetCullingStatTracker.updateStats( bitsets, subsetsMask )

#else // !(Pk_ENABLE_STATS && Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS)

// Empty macro
#define Pk_STATS_UPDATE_FOR_SUBSET_CULLING( bitsets, subsetsMask )

#endif // (Pk_ENABLE_STATS && Pk_ENABLE_DETAILED_SUBSET_CULLING_STATS)

#endif // PkStatsDetailedSubsetCullingStatTracker_h
