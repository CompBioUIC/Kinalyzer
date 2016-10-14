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
	my ( $name, $email, $loci, $algo, $status, $id, $count ) = @_;
	# Open inputfile and check for no-of-line*loci less than a specified limit	

	my $bShouldPushToNextQueue = ($count > 7000);

	# Push to next queue if lines by loci is too huge
	if ( $bShouldPushToNextQueue )
	{
		# Set status to let next scheduler pick it up
		status( "Pushing $id to next scheduler" );
		pkDbUpdateStatus( $id, pkGetSchedulerBaseStatus()+2 );
	}

	return $bShouldPushToNextQueue;		
}

sub pkSchedulerMain
{
	pkSetSchedulerId( "long" );
	pkSetSchedulerBaseStatus( 5 );
	pkSchedulerRun(\&shouldPushJobToNextScheduler);
}

pkSchedulerMain();

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

