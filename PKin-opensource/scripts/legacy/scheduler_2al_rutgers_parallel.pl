#!/usr/bin/perl 
 
# Root directory
$depot_dir="/home/sibgroup/sibsdepot/";

# Log paths
$log_file_dir=$depot_dir."logs/";
$log_file_name="log_file_scheduler_2al_rutgers_parallel.txt";

# Parallel implementation binary paths
$parallel_binary_dir=$depot_dir."binaries/omp-filtered/";
$parallel_binary_name="psetsfast";

# Data paths
$data_dir=$depot_dir."data/";

# Scripts paths
$scripts_dir=$depot_dir."/src/main/scripts/";
$extract_solution_param_script="extract-solution-param.pl";
$write_solution_script="write-solution.pl";

# Output paths
$output_dir=$depot_dir."output/"; # /var/www/uploads_new/

# Error paths
$error_output_dir=$log_file_dir;
$parallel_error1_file_name="errparallel1.txt";
$parallel_error2_file_name="errparallel2.txt";
$gams_error1_file_name="errgms1.txt";
$gams_error2_file_name="errgms2.txt";
$extract_solution_error1_file_name="errextsoln1.txt";
$extract_solution_error2_file_name="errextsoln2.txt";
$write_solution_error1_file_name="errwritesoln1.txt";
$write_solution_error2_file_name="errwritesoln2.txt";

$log_handle;  

# Variable to store file name of initial input data
$input_file;
# Variable to store path to initial input data
$input_path;
# Variable to store path to final output data
$final_output_path;
# Varialbe to store path to final e-mail
$final_patched_output_path;
# Variable to store path to initial uploaded data
$uploaded_input_path;

# Currently, algorithm is always set to 2-allele
$algo="2allele";

sub main() 
{ 
	my $log_file_path=$log_file_dir.$log_file_name;
	open $log_handle, ">>$log_file_path" or die; 
	
	# Open file that has ID, loci information, read into an array
	$name_email_loci_id_map_path=$data_dir."name_email_loci_id_map.txt";
	status("Opening $name_email_loci_id_map_path");
	open(DAT, $name_email_loci_id_map_path) || die("Could not open $name_email_loci_id_map_path!");
	@name_email_loci_id_tuples=<DAT>;
	close(DAT); 

	# Loop through array 
	foreach $name_email_loci_id_tuple (@name_email_loci_id_tuples)
	{
		# Read ID, Loci into corresponding variables
		chomp($name_email_loci_id_tuple);
		($name,$email,$loci,$id)=split(/\|/,$name_email_loci_id_tuple);
		status("Processing tuple(name=$name,email=$email,loci=$loci,id=$id");
		$input_file=$id."_new";
		$input_path=$data_dir.$input_file;
		
		# Run 2-allele
		my $psets_input_data_path=$input_path;
		my $intermediate_base_file_name=$id."_sets";
		my $psets_output_data_path=$output_dir.$intermediate_base_file_name;
		my $psets_cmd_line="nice ./$parallel_binary_name $loci $psets_input_data_path $psets_output_data_path";
		status("Running this --> $psets_cmd_line");
		chdir $parallel_binary_dir;
		system($psets_cmd_line); 

		# Run gams
		my $gms_file_path=$psets_output_data_path.".gms";
		if(not -e $gms_file_path)
		{ 
			status("$gms_file_path not found"); 
		} 				
		else
		{
			# Switch to output dir so data is generated in the output directory
			chdir $output_dir;
			my $gams_error1_path=$error_output_dir.$gams_error1_file_name;
			my $gams_error2_path=$error_output_dir.$gams_error2_file_name;
			my $gams_cmd_line="gams $gms_file_path 1>$gams_error1_path 2>>$gams_error2_path";
			status("$gms_file_path found. Running $gams_cmd_line"); ;
			system($gams_cmd_line);    
 
			# Run extract-solution
			my $lst_file_path=$psets_output_data_path.".lst";
			if(not -e $lst_file_path)
			{ 
				status("$lst_file_path not found");	
			}  			    	 		
			else 
			{		
				my $extract_solution_script_path=$scripts_dir.$extract_solution_param_script;
				my $extract_solution_error1_path=$error_output_dir.$extract_solution_error1_file_name;
				my $extract_solution_error2_path=$error_output_dir.$extract_solution_error2_file_name;
				my $extract_solution_cmd_line="perl $extract_solution_script_path $lst_file_path 2>>$extract_solution_error2_path 1>>$extract_solution_error1_path";
				status("$lst_file_path found. Running $extract_solution_cmd_line");
				system($extract_solution_cmd_line);
				
				# Run write-solution
				my $sol_file_path=$psets_output_data_path.".sol";
				if(not -e $sol_file_path)	
				{  					
					status("$sol_file_path not found");	
				} 					 			
				else
				{ 
					$final_output_path=$psets_output_data_path.".soln";
					my $soln_file_path=$final_output_path;
					my $write_solution_script_path=$scripts_dir.$write_solution_script;
					my $write_solution_error1_path=$error_output_dir.$write_solution_error1_file_name;
					my $write_solution_error2_path=$error_output_dir.$write_solution_error2_file_name;
					my $write_solution_cmd_line="perl $write_solution_script_path -i $psets_output_data_path -s $sol_file_path -o $soln_file_path 2>>$write_solution_error2_path 1>>$write_solution_error1_path";
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

		# Extract animal id's from input file
		open(INP, "<$input_path");
		$inp_line_cnt = 0;
		while(<INP>)
		{
			@words_in_row = split(/,/, $_);
			$animal_id[$inp_line_cnt] = $words_in_row[0];
			$inp_line_cnt++;				
		}
		close(INP);
	
		# Verify copied solution exists
		$final_patched_output_path=$output_dir.$id."_mailop.txt";
		if(not -e $final_output_path)	
		{
			open(MAIL, ">$final_patched_output_path");
			print MAIL "There were some errors and the solution could not be computed.";
			close(MAIL);
			status("$final_output_path not found");
		}
		
		# Perform patching of animal id's in final e-mail file
		open(SOL, "<$final_output_path");
		open(SOL2, ">$final_patched_output_path");
		status("patching $final_output_path to $final_patched_output_path");
		my(@lines) = <SOL>;
		$line_cnt =0;
		foreach $line (@lines) 
		{
			my @sibs = split(/\s+/, $line);
			$line_cnt++;
			my $line_to_print = "Sibs Set ".$line_cnt.": ".$animal_id[$sibs[1]];
			for (2..$#sibs) 
			{
				$line_to_print = $line_to_print.", ".$animal_id[$sibs[$_]];
			}	
			print SOL2 $line_to_print."\n";	
		}			
		close(SOL);
		close(SOL2);

		# Clear animal id's array because we are in a loop?
		for($p=0; $p<=$#animal_id; $p++) 
		{
			$animal_id[$p]= "";
		}
		
		# E-mail body with greeting and citation
		$content = "Hi $name! \n\nThank you for using Kinalyzer to run $algo algorithm.\n";
		$content = $content."The output, the input and the file you uploaded are given below. The individuals in each set of siblings are given by the individual IDs from the input file.\n";
		$content = $content."You can also retrieve the results by going to \"Check Status\"(http://kinalyzer.cs.uic.edu/status.html) page on the website. Use the request id and your email address to access the information.\n";
		$content = $content."If you publish these results using analysis performed by Kinalyzer please acknowledge it by the following citations:\n";
		if ($algo=='2allele') 
		{
			$content=$content."\nT.Y. Berger-Wolf, S.I. Sheikh, B. DasGupta, M.V. Ashley, I.C. Caballero, W. Chaovalitwongse, S.L. Putrevu, 'Reconstructing Sibling Relationships in Wild Populations', Bioinformatics, 23(13)\n";
			$content=$content."\nM. V. Ashley, I. C. Caballero, W. Chaovalitwongse, B. DasGupta, P. Govindan, S. Sheikh and T. Y. Berger-Wolf. 'KINALYZER, A Computer Program for Reconstructing Sibling Groups', Molecular Ecology Resources\n\n";
		}
		else 
		{
			$content=$content."\nS. I. Sheikh, T. Y. Berger-Wolf, Ashfaq A. Khokhar and B. DasGupta 'Consensus Methods for Reconstruction of Sibling Relationships from Genetic Data', Proceedings of the 4th Multidisciplinary Workshop on Advances in Preference Handling at AAAI 08.\n";
			$content=$content."\nM. V. Ashley, I. C. Caballero, W. Chaovalitwongse, B. DasGupta, P. Govindan, S. Sheikh and T. Y. Berger-Wolf. 'KINALYZER, A Computer Program for Reconstructing Sibling Groups', Molecular Ecology Resources\n\n";
		}
		
		# Paste patched output
		$content= $content."\n\n------------------ OUTPUT START -----------------\n\n";			
		open(OUTPUT, $final_patched_output_path) or die "Can't open output file : $!";
		while(<OUTPUT>) 
		{
			$content=$content.$_."\n";
		}
		close OUTPUT;
		$content= $content."------------------ OUTPUT END -------------------\n\n";			

		# Paste input to kinalyzer
		$content= $content."\n\n------------ Input sent to Kinalyzer -------------\n\n";			
		open(INPUT, $input_path) or die "Can't open input file : $!";
		while(<INPUT>) 
		{
			$content=$content.$_."\n";
		}
		close INPUT;
		$content= $content."----------- End of Input to Kinalyzer ------------\n\n";			

		# Paste original uploaded data
		$content= $content."\n\n----------------- Uploaded file ------------------\n\n";
		$uploaded_input_path=$data_dir.$id;	
		open(IN, $uploaded_input_path) or die "Can't open original file : $!";
		while(<IN>) 
		{
			$content=$content.$_."\n";
		}
		close IN;
		$content=$content."------------- End of Uploaded file --------------\n\n";
	
		# Closing statements
		$content=$content."\n\nWe would like to know your comments about our algorithms and the website. Please feel free to post them at the 'Contact Us'(http://kinalyzer.cs.uic.edu/contact.html) page on the website.\n";
		$content=$content."\nRegards,\nThe Kinalyzer team.\n";
		
		# Generate e-mail
		#$sendmail="/usr/sbin/sendmail -t";
		$sendmail=$output_dir.$id."_email";
		$from = "From:tanyabw\@uic.edu\n";
        $reply_to = "Reply-to:tanyabw\@uic.edu\n";
        $send_to = "To:".$email."\n";
		$subject="Subject: Kinalyzer output for $name, request id=$id\n";
		#open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
		open(SENDMAIL, ">$sendmail") or die "Cannot open $sendmail: $!";
		#print SENDMAIL $reply_to;
		#print SENDMAIL $subject;
		#print SENDMAIL $send_to;
		#print SENDMAIL "Content-type: text/plain\n\n";
		print SENDMAIL $content;
		close(SENDMAIL);
	} #end of foreach  			
						 			 		 
	close $log_handle; 
} 

main();   

sub status
{ 	
	my $tm = scalar localtime; 
	print $log_handle "$tm @_\n"; 
}  

