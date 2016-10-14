/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkAssert_h
#define PkAssert_h

#include "PkBuild.h"

#if Pk_ENABLE_ASSERT

#include <assert.h>
extern void PkPrintBacktrace(); // Currently we only print backtrace on UNIX
#define PkAssert( expr ) if(!(expr)){PkPrintBacktrace(); assert( false );} 
#define PkVerify( expr ) PkAssert( expr );

#else

#define PkAssert( expr ) 
#define PkVerify( expr ) expr

#endif

#endif // PkAssert_h
