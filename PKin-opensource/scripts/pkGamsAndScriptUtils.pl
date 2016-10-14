#!/usr/bin/perl

# Routine(s) for running GAMS and scripts on resulting PKin output

use strict;

require "./pkLoggerUtils.pl";

# closure to avoid global variables
{

# Runs GAMS and scripts on resulting PKin output
# param scripts_dir - directory containing necessary post-processing scripts
# param extract_solution_param_script - name of script for extracting solution params
# param write_solution_script - name of script for writing solution
# param job_intermediate_dir - directory for all intermediate files relevant to this job
# param job_intermediate_base_file_path - path to output of PKin
# param job_record_attribs_array - array storing attributes of current job record from database
# returns final output path of job for post processed data
sub pkRunGamsAndScripts
{ 	
	# Extract parameters
	my $gams_dir = shift;
	my $scripts_dir = shift;
	my $extract_solution_param_script = shift;
	my $write_solution_script = shift;
	my $job_intermediate_dir = shift;
	my $job_intermediate_base_file_path = shift;
	my ( $name, $email, $loci, $algo, $status, $id, $count ) = @_;

	# Our output path that we must return
	my $job_final_output_path;

	if ( $algo eq "consensus" )
	{
		# Set up final data path for patching
		$job_final_output_path = $job_intermediate_base_file_path."_consensus";
	}
	# Run 2-allele scripts
	elsif ( $algo eq "2allele" )
	{
		# Run gams
		my $gms_file_path=$job_intermediate_base_file_path.".gms";
		if(not -e $gms_file_path)
		{ 
			status("$gms_file_path not found"); 
		} 				
		else
		{
			# Switch to output dir so data is generated in the output directory
			chdir $job_intermediate_dir;
			my $gams_cmd_line=$gams_dir."gams $gms_file_path";
			status("$gms_file_path found. Running $gams_cmd_line"); ;
			system($gams_cmd_line);    

			# Run extract-solution
			my $lst_file_path=$job_intermediate_base_file_path.".lst";
			if(not -e $lst_file_path)
			{ 
				status("$lst_file_path not found");	
			}  			    	 		
			else 
			{		
				my $extract_solution_script_path=$scripts_dir.$extract_solution_param_script;
				my $extract_solution_cmd_line="perl $extract_solution_script_path $lst_file_path";
				status("$lst_file_path found. Running $extract_solution_cmd_line");
				system($extract_solution_cmd_line);
				
				# Run write-solution
				my $sol_file_path=$job_intermediate_base_file_path.".sol";
				if(not -e $sol_file_path)	
				{  					
					status("$sol_file_path not found");	
				} 					 			
				else
				{ 
					$job_final_output_path=$job_intermediate_base_file_path.".soln";
					my $soln_file_path=$job_final_output_path;
					my $write_solution_script_path=$scripts_dir.$write_solution_script;
					my $write_solution_cmd_line="perl $write_solution_script_path -i $job_intermediate_base_file_path -s $sol_file_path -o $soln_file_path";
					status("$sol_file_path found. Running $write_solution_cmd_line");	
					system($write_solution_cmd_line);
					
					# Log if final solution exists
					if (not -e $soln_file_path)	
					{ 						
						status("$soln_file_path not found");		 					
					}
					else
					{
						status("$soln_file_path found.");
					}
				} # end of write script					
			} # end of extract script
		} # end of gams
	} # end of 2-allele specific branch

	return $job_final_output_path;
}			

# In order to indicate module loading was successful, must return 1
# http://lists.netisland.net/archives/phlpm/phlpm-2001/msg00426.html
1;

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

