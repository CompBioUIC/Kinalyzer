/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkClock.h"

#if Pk_ENABLE_STATS

namespace PkClock
{

// Returns the number of seconds counted from an arbitrary point, not from the start of the program
time_t getSeconds()
{
	return time(NULL);
}

}

#endif // Pk_ENABLE_STATS
