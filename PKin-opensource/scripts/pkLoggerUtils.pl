#!/usr/bin/perl

# Routines for running PKin from the commandline

use strict;

# closure to avoid global variables
{

sub status
{ 	
	my $tm = scalar localtime; 
	print "$tm @_\n"; 
}			

# In order to indicate module loading was successful, must return 1
# http://lists.netisland.net/archives/phlpm/phlpm-2001/msg00426.html
1;

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

