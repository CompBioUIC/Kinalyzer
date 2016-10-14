/**
 * @author Alan Perez-Rathke 
 *
 * @date March 6, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkIndividual_h
#define PkIndividual_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLocus.h"

// An individual is a collection of alleles (genotypes)
typedef PkArray( PkLocus ) PkIndividual;

#endif // PkIndividual_h
