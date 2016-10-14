/**
 * @author Alan Perez-Rathke 
 *
 * @date February 20, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkAssert.h"

// Support for printing out callstack bactrace on UNIX
#if UNIX
	#include <execinfo.h>
	#include <stdio.h>
	#include <stdlib.h>
	#define MAX_CALLSTACK_DEPTH 100
	void PkPrintBacktrace()
	{
		 void *buffer[MAX_CALLSTACK_DEPTH];
		 char **strings;

		 PkInt depth = backtrace(buffer, MAX_CALLSTACK_DEPTH); 
		 fprintf(stderr,"backtrace() returned %d addresses\n", depth);

		 strings = backtrace_symbols(buffer, depth);
		 if (strings == NULL) {
			 perror("backtrace_symbols");
			 exit(EXIT_FAILURE);
		 }

		 for (PkInt i = 0; i < depth; i++)
			 fprintf(stderr,"%s\n", strings[i]);

		 fprintf(stderr,"done printing backtrace.\n");

		 free(strings);
	}
#else
	// On Windows and other platforms we don't support printing backtrace yet
	void PkPrintBacktrace() {}
#endif 
