/**
 * @author Alan Perez-Rathke 
 *
 * @date July 3rd, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkConsensusSetCoverParser_h
#define PkConsensusSetCoverParser_h

#include "PkBuild.h"
#include "PkTypes.h"

namespace PkConsensusSetCoverParser
{
	// @return true if set cover file was successfully parsed, false otherwise
	extern PkBool parseFile(
		  const char* strSetCoverFileName
		, const PkUInt populationSize
		, PkBitSetArray& outSiblingSets
		);

} // end of PkConsensusSetCoverParser namespace

#endif // PkConsensusSetCoverParser_h
