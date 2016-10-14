#!/usr/bin/perl

use strict;

require "./pkLoggerUtils.pl";
require "./pkDbUtils.pl";
require "./pkScheduler.pl";

# Force stdout to be flushed after every print call
$| = 1;

# closure to avoid global variables
{

# Returns 1 if job should be pushed to next scheduler, 0 otherwise
# param ( $name, $email, $loci, $algo, $status, $id, $count ) of current job
sub shouldPushJobToNextScheduler
{
	return 0;	
}

sub pkSchedulerMain
{
	pkSetSchedulerId( "extralong" );
	pkSetSchedulerBaseStatus( 7 );
	pkSchedulerRun(\&shouldPushJobToNextScheduler);
}

pkSchedulerMain();

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

