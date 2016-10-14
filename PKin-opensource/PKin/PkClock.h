/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkClock_h
#define PkClock_h

#include "PkBuild.h"

#if Pk_ENABLE_STATS

#include <time.h>

namespace PkClock
{

// @return - number of seconds counted from an arbitrary point, not from the start of the program
extern time_t getSeconds();

}

#endif // Pk_ENABLE_STATS

#endif // PkClock_h
