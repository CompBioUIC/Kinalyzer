/**
 * @author Alan Perez-Rathke 
 *
 * @date July 11th, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkConsensusCosts_h
#define PkConsensusCosts_h

#include "PkBuild.h"

namespace PkConsensus
{
	// Enumerated costs
	enum ECosts
	{
	  EC_NULL_ALLELE_ERROR = 0
	, EC_HOMO_MISTYPE_ERROR
	, EC_HETERO_MISTYPE_ERROR
	, EC_POSSIBILITY_MISFIT
	, EC_MAX_GROUP_EDIT
	, EC_MAX_ERROR_RATE
	, EC_MAX_EDIT
	, EC_INFINITY
	, EC_NUM_COST_VALUES
	};

	// @return value associated with cost enumeration
	extern double getCost( const ECosts cost );

} // end of PkConsensus namespace

#endif // PkConsensusCosts_h
