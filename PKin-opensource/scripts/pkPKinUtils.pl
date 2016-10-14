#!/usr/bin/perl

# Routines for running PKin from the commandline

use strict;
use Config;
use POSIX;

require "./pkLoggerUtils.pl";
require "./pkDbUtils.pl";

# closure to avoid global variables
{

# The maximum number of seconds to wait for a PKin job to finish, if not finished, then it becomes queued
my $pkin_max_wait_seconds = 3600; # default is 1 hour

# The queue of suspended jobs along with their necessary attributes - contains multiple sets of the following:
# Format: pid, input_path, intermediate_dir, intermediate_base_file_path, job_record_attribs_array
my @pkin_suspended_jobs_queue;

# Attributes of last finished job - only contains one set of the following:
# Format: input_path, intermediate_dir, intermediate_base_file_path, job_record_attribs_array
my @pkin_last_finished_job_attributes;

# See if we support non-blocking wait calls
my $pkin_has_nonblocking_wait = $Config{d_waitpid} eq "define" || $Config{d_wait4}   eq "define";
status( "We support non-blocking wait? $pkin_has_nonblocking_wait" );

# Mutator, configures the maximum number of seconds to wait for a PKin job to finish
# param max_wait_seconds - input maximum number of seconds to wait for a PKin job to finish before queueing
sub setPKinMaxWaitSeconds
{
	$pkin_max_wait_seconds = shift;
}

# Accessor, returns the maximum number of seconds to wait for a PKin job to finsh before queueing
sub getPKinMaxWaitSeconds
{
	return $pkin_max_wait_seconds;
}

# Accessor, returns attributes of last successfully completed job
sub getPkinLastFinishedJobAtttribs
{
	return @pkin_last_finished_job_attributes;
}

sub doesPKinSuspendedJobExist
{
	my $pkin_suspended_jobs_queue_size = @pkin_suspended_jobs_queue;
	return $pkin_suspended_jobs_queue_size > 0;
}

# Runs an exec command to execute pkinperl waitpid WNOHANG
# Warning: changes directory to PKin binary
# param binary_dir - directory containing PKin binary
# param binary_name - name of PKin binary
# param input_path - path to input data from db
# param intermediate_base_file_path - intermediate base file name used by by all subsequent apps of the pipeline
# param loci - number of loci for current job
# param algo - algorithm to run (consensus or 2allele)
sub execPKin
{
	# Get external parameters
	my $pkin_binary_dir = shift;
	my $pkin_binary_name = shift;
	my $pkin_input_data_path = shift;
	my $pkin_output_data_path = shift;
	my $pkin_num_loci = shift;
	my $pkin_algo = shift;

	# Determine commandline parameters to PKin
	my $pkin_arg_num_threads = 8;
	my $pkin_arg_num_logical_threads_local_loci_reconstruction = $pkin_num_loci;
	my $pkin_arg_num_logical_threads_loci_intersection = -1; # Set to -1 to allow better load balancing (this is key!)
	my $pkin_arg_stacksizekb = 1024;
	my $pkin_arg_recon_algo = 1; # centralized=0, island=1, work stealing=2
	my $pkin_arg_num_loci_clusters = $pkin_num_loci;
	my $pkin_cmd_line = "./$pkin_binary_name $pkin_algo $pkin_input_data_path $pkin_arg_num_threads $pkin_arg_num_logical_threads_local_loci_reconstruction $pkin_arg_num_logical_threads_loci_intersection $pkin_arg_stacksizekb $pkin_arg_recon_algo $pkin_arg_num_loci_clusters $pkin_output_data_path";

	# Convert commandline to array form to avoid shell call (supposedly more efficient)	
	my @pkin_cmd_line_array = ("./$pkin_binary_name",$pkin_algo,$pkin_input_data_path,$pkin_arg_num_threads,$pkin_arg_num_logical_threads_local_loci_reconstruction,$pkin_arg_num_logical_threads_loci_intersection,$pkin_arg_stacksizekb,$pkin_arg_recon_algo,$pkin_arg_num_loci_clusters,$pkin_output_data_path);
	
	# Execute PKin
	chdir $pkin_binary_dir;
	status("Running this --> $pkin_cmd_line");
	exec(@pkin_cmd_line_array);
	exit(0);
}			

# Helper function for non-blocking wait on a child PKin routine
# param pid - process id of previously forked child
# return 1 if we finished the job, 0 otherwise
sub timedWaitForPKinChild
{
	my $pid = shift;
	my $total_wait_seconds = 0; # current running total of seconds we've blocked waiting for child to finish
	my $sleep_interval_seconds = 10; # the number of seconds to sleep before polling the child process for completion
	my $is_finished = 0;
	
	status( "Waiting for child process $pid" );
	while ( ($is_finished == 0) && ($total_wait_seconds < getPKinMaxWaitSeconds()) )
	{
		# waitpid returns pid if child process finished, -1 if no child process with that pid exists, and (possibly) 0 if process is still running
		my $wait_result = waitpid($pid, &WNOHANG);
		if ( $wait_result != 0 )
		{
			# See if we reaped ourselves or if it was auto reaped(-1)
			# todo: parse return code bits stored in $? which is a 2 byte word
			status( "Finished waiting with wait return code: $wait_result" );
			$is_finished = 1;
		}
		else
		{
			$total_wait_seconds = $total_wait_seconds + $sleep_interval_seconds;
			sleep( $sleep_interval_seconds );
		}
	}

	# Signal if we finished the PKin job
	return $is_finished;
}

# Helper function for non-blocking wait on a child PKin routine, if time limit is exceeded, child process is suspended and queued
# Upon successful completion, last finished attributes are stored
# param pid - process id of previously forked child
# param input_path - path to input data from db
# param intermediate_dir - directory of pkin output
# param intermediate_base_file_path - intermediate base file name used by all subsequent apps of the pipeline
# param job_record_attribs_array - array storing attributes of current job record from database
# return 1 if we finished the job, 0 job was suspended and queued
sub timedWaitForPKinChildWithSuspendAndQueue
{
	# Extract arguments
	my $pid = shift;
	my $pkin_input_data_path = shift;
	my $pkin_output_dir = shift;
	my $pkin_output_data_path = shift;
	my @pkin_job_record_attribs = @_;	
	
	# Timed wait on child process
	if ( timedWaitForPKinChild( $pid ) )
	{
		# Store attributes of completed job
		@pkin_last_finished_job_attributes = ( $pkin_input_data_path, $pkin_output_dir, $pkin_output_data_path, @pkin_job_record_attribs );		

		# Return that process was finished
		return 1;
	}
	else
	{
		# Process did not finish, suspend and queue it
		my $num_procs_signaled = kill 'STOP', $pid;
		status( "Signaled $pid to stop was successful? $num_procs_signaled" );

		# Queue process for later running
		push @pkin_suspended_jobs_queue, $pid, $pkin_input_data_path, $pkin_output_dir, $pkin_output_data_path, @pkin_job_record_attribs;

		# Return that process was suspended and queued
		return 0;
	}
}

# Forks a child process to execute PKin, monitors child process to see if it's exceeded alloted time limit
# Warning: may change directory to PKin binary
# param binary_dir - directory containing PKin binary
# param binary_name - name of PKin binary
# param input_path - path to input data from db
# param intermediate_dir - directory of pkin output
# param intermediate_base_file_path - intermediate base file name used by by all subsequent apps of the pipeline
# param job_record_attribs_array - array storing attributes of current job record from database
# return 1 if we finished the job, 0 if it was queued and needs to be resumed
sub forkPKin
{
	# Get external parameters
	my $pkin_binary_dir = shift;
	my $pkin_binary_name = shift;
	my $pkin_input_data_path = shift;
	my $pkin_output_dir = shift;
	my $pkin_output_data_path = shift;
	my @pkin_job_record_attribs = @_;
	((scalar @pkin_job_record_attribs) == idxPkDbNumFields) || die "Not enough arguments";
	my $pkin_num_loci  = $pkin_job_record_attribs[ idxPkDbRecLoci ];
	my $pkin_algo = $pkin_job_record_attribs[ idxPkDbRecAlgo ];

	# Our return code
	my $retVal = 0;

	# Fork off a child process so that we may suspend it if it exceeds a time limit
	my $pid = fork();

	if ( not defined $pid )
	{
		# Not good, what to do here?
		status( "ERROR: Unable to spawn PKin child process!" );
	}
	elsif ( $pid == 0 )
	{
		# We are in the child process! ... we should never return from this exec call
		execPKin( $pkin_binary_dir, $pkin_binary_name, $pkin_input_data_path, $pkin_output_data_path, $pkin_num_loci, $pkin_algo );
	}
	else
	{
		# We are in the parent process!
		$retVal = timedWaitForPKinChildWithSuspendAndQueue( $pid, $pkin_input_data_path, $pkin_output_dir, $pkin_output_data_path, @pkin_job_record_attribs );
	}

	return $retVal;
}

# Resumes a single PKin job at front of queue that was previously suspended
# returns 1 if job finished successfully, 0 otherwise
sub runPKinFromSuspendedQueue
{
	my $retVal = 0;

	if ( doesPKinSuspendedJobExist() )
	{
		# Extract job parameters from suspended queue
		my $pid = shift @pkin_suspended_jobs_queue;
		my $pkin_input_data_path = shift @pkin_suspended_jobs_queue;
		my $pkin_output_dir = shift @pkin_suspended_jobs_queue;
		my $pkin_output_data_path = shift @pkin_suspended_jobs_queue;
		((scalar @pkin_suspended_jobs_queue) >= idxPkDbNumFields) || die "Not enough arguments";
		my @pkin_job_record_attribs;
		for ( my $idxAttr=0; $idxAttr<idxPkDbNumFields; $idxAttr++ )
		{
			my $dbFieldAttrib = shift @pkin_suspended_jobs_queue;
			push @pkin_job_record_attribs, $dbFieldAttrib;
		}

		# Wake up suspended job
		my $num_procs_signaled = kill 'CONT', $pid;
		status( "Signaled $pid to continue was successful? $num_procs_signaled" );

		# Run suspended job
		$retVal = timedWaitForPKinChildWithSuspendAndQueue( $pid, $pkin_input_data_path, $pkin_output_dir, $pkin_output_data_path, @pkin_job_record_attribs );
	}
	
	return $retVal;
}

# In order to indicate module loading was successful, must return 1
# http://lists.netisland.net/archives/phlpm/phlpm-2001/msg00426.html
1;

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

