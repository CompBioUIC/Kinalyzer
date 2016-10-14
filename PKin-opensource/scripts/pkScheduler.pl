#!/usr/bin/perl

use strict;

require "./pkDbUtils.pl";
require "./pkPKinUtils.pl";
require "./pkGamsAndScriptUtils.pl";
require "./pkMailResultsUtils.pl";

# Force stdout to be flushed after every print call
$| = 1;

# closure to avoid global variables
{

##################################### Configuration

# Root directory
my $depot_dir = "/home/kinalyzer/sibsdepot/";

# Binary paths
my $binary_dir = $depot_dir."binaries/PKin/";
my $binary_name = "PKin-local";

# Data paths
my $data_dir = "/var/www/uploads_new/";

# GAMS path
my $gams_dir = "/opt/lx3gams/";

# Scripts paths
my $scripts_dir = $depot_dir."/src/main/scripts/";
my $extract_solution_param_script = "extract-solution-param.pl";
my $write_solution_script = "write-solution.pl";

# Output paths
my $intermediate_root_dir = $depot_dir."intermediate/";
my $archives_dir = $depot_dir."archives/";
my $finished_dir = $depot_dir."finished/";

# Scheduler ID
my $scheduler_id = "default";
my $scheduler_base_status = 1;

sub pkGetSchedulerDataDir
{
	return $data_dir;
}

sub pkSetSchedulerDataDir
{
	$data_dir = shift;
}

# Accessor, returns current value of scheduler id
sub pkGetSchedulerId
{
	return $scheduler_id;
}

# Mutator, sets value of scheduler id
sub pkSetSchedulerId
{
	$scheduler_id = shift;
}

sub pkGetSchedulerBaseStatus
{
	return $scheduler_base_status;
}

sub pkSetSchedulerBaseStatus
{
	$scheduler_base_status = shift;
}

# Job completed: run GAMS, scripts, generate and mail final report, update database, archive and clean up intermediates
# param intermediate_scheduler_dir - directory for all intermediates created by this scheduler script
# param job_input_path - path to uploaded data
# param job_intermediate_dir - directory of intermediate files of completed job
# param job_intermediate_base_file_path - path to output of PKin
# param job_record_attribs_array - array storing attributes of current job record from database
sub pkSchedulerOnPKinJobCompleted
{
	# Extract parameters
	my $intermediate_scheduler_dir = shift;
	my $job_input_path = shift;
	my $job_intermediate_dir = shift;
	my $job_intermediate_base_file_path = shift;
	my @job_record_attribs_array = @_;
	my $id = $job_record_attribs_array[ idxPkDbRecId ];

	# Run GAMS and post-processing scripts
	my $job_final_output_path = pkRunGamsAndScripts( $gams_dir, $scripts_dir, $extract_solution_param_script, $write_solution_script, $job_intermediate_dir, $job_intermediate_base_file_path, @job_record_attribs_array );		
	
	# Generate e-mail body and send it out
	pkGenerateAndMailFinalReport( $data_dir, $finished_dir, $job_input_path, $job_intermediate_dir, $job_final_output_path, @job_record_attribs_array );

	# Update database to reflect that we finished the job
	pkDbUpdateStatus( $id, pkGetSchedulerBaseStatus()+1 );

	# Archive and clean intermediate files
	{
		chdir $job_intermediate_dir;
		my $tar_file_name=$id."-int.tar.gz";
		system("tar cvzf $tar_file_name *");
		system("mv $tar_file_name $archives_dir");
		system("rm *");
		chdir $intermediate_scheduler_dir;
		rmdir $job_intermediate_dir;
	}
}

# Infinite loops checking checking database for new jobs and running them
# param shouldPushJobtoNextScheduler - subroutine to determine if job should be pushed to next scheduler
sub pkSchedulerRun
{
	# Extract parameter function for determing if we should push job to next scheduler or not
	my $shouldPushJobToNextSchedulerRef = shift;	
	
	# Set intermediates folder, assumes folder exists already
	# WARNING: INTERMEDIATE_SCHEDULER_DIR and JOB_INTERMEDIATE_DIR must not be the same value! (because job intermediate dir gets deleted on completion)
	my $intermediate_scheduler_dir = $intermediate_root_dir."$scheduler_id/";
	status( "Scheduler $scheduler_id started with base status $scheduler_base_status and intermediate directory:\n\t$intermediate_scheduler_dir" );

	# Connect to database
	pkDbConnect();

	# BEGIN INFINITE LOOP
	while ( 1 )
	{
		pkDbInitQuery( pkGetSchedulerBaseStatus() );		
		
		while ( pkDbGetNextRec() )
		{
			my ( $name, $email, $loci, $algo, $status, $id, $count ) = pkDbGetRecAttribsAsArray();
			status("The current request is --- $id of $name");
			status("Processing tuple(name=$name,email=$email,loci=$loci,id=$id,algo=$algo,count=$count)");

			# Skip this job if it's too large
			if ( $shouldPushJobToNextSchedulerRef->( pkDbGetRecAttribsAsArray() ) )
			{
				next;
			}
			
			# Determine job input path
			my $job_input_path=$data_dir.$id."_new";

			# Create intermediates folder specific to this job
			chdir $intermediate_scheduler_dir;
			mkdir $id;
			my $job_intermediate_dir = $intermediate_scheduler_dir."$id/";			
			
			# Determine intermediate file destinations
			my $job_intermediate_base_file_name = $id."_sets";
			my $job_intermediate_base_file_path = $job_intermediate_dir.$job_intermediate_base_file_name;
			
			# Fork Pkin
			if ( forkPKin( $binary_dir, $binary_name, $job_input_path, $job_intermediate_dir, $job_intermediate_base_file_path, pkDbGetRecAttribsAsArray() ) )
			{
				# Job completed: run GAMS, scripts, generate and mail final report, update database status
				pkSchedulerOnPKinJobCompleted( $intermediate_scheduler_dir, $job_input_path, $job_intermediate_dir, $job_intermediate_base_file_path, pkDbGetRecAttribsAsArray() );
			} 
			else
			{
				# Job was suspended: update database to signify that job was suspended
				pkDbUpdateStatus( $id, "-".pkGetSchedulerBaseStatus() );
			}
		}		

		if ( runPKinFromSuspendedQueue() )
		{
			# Job completed: run GAMS, scripts, generate and mail final report, update database status
			pkSchedulerOnPKinJobCompleted( $intermediate_scheduler_dir, getPkinLastFinishedJobAtttribs() );
		}

		# Sleep for a bit...
		sleep(10);
	}
	# END INFINITE LOOP
}

# In order to indicate module loading was successful, must return 1
# http://lists.netisland.net/archives/phlpm/phlpm-2001/msg00426.html
1;

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

