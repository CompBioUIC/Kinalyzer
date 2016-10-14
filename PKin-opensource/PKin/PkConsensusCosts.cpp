/**
 * @author Alan Perez-Rathke 
 *
 * @date July 11th, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkAssert.h"
#include "PkConsensusCosts.h"

namespace
{
	// A hard-coded cost lookup table for now...
	static double GCosts[ PkConsensus::EC_NUM_COST_VALUES ] =
	{
	  0.34   // EC_NULL_ALLELE_ERROR
	, 1.0    // EC_HOMO_MISTYPE_ERROR
	, 0.7    // EC_HETERO_MISTYPE_ERROR
	, 0.4    // EC_POSSIBILITY_MISFIT
	, 300    // EC_MAX_GROUP_EDIT
	, 0.45   // EC_MAX_ERROR_RATE
	, 2.0    // EC_MAX_EDIT
	, 9999.0 // EC_INFINITY
	};
} // end of unnamed namespace

namespace PkConsensus
{

// @return value associated with cost enumeration
double getCost( const ECosts cost )
{
	PkAssert( cost >= 0 );
	PkAssert( cost < EC_NUM_COST_VALUES );
	return GCosts[ cost ];
}

} // end of PkConsensusSetCoverParser namespace
