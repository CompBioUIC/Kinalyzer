#!/usr/bin/perl 

# Allow database connectivity
use DBI;
 
# Root directory
$depot_dir="/home/sibgroup/sibsdepot/";

# Parallel implementation binary paths
$parallel_binary_dir=$depot_dir."binaries/PKin/";
$parallel_binary_name="PKin-locali";

# Data paths
$data_dir="/var/www/uploads_new/";

# Scripts paths
$scripts_dir=$depot_dir."/src/main/scripts/";
$extract_solution_param_script="extract-solution-param.pl";
$write_solution_script="write-solution.pl";

# Output paths
$intermediate_dir=$depot_dir."intermediate_single/";
$archives_dir=$depot_dir."archives/";
$finished_dir=$depot_dir."finished/";

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

# Force stdout to be flushed after every print call
$| = 1;

$request_id=1780;

sub main() 
{ 
	# Connect to database
	my $dsn = 'DBI:mysql:interface_data:localhost';
	my $db_username = 'www_user';
	my $db_password = 'sibs';
	my $dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;
	status("Connected to database.");
	
	# Query database for active requests
	my $get_requests_new = $dbh->prepare("SELECT * FROM requests_new WHERE request_id=$request_id ");
	$get_requests_new->execute;		
	$get_requests_new->bind_columns( \$name, \$email, \$loci, \$algo, \$status, \$id, \$count);
		
	# Process request
	while($get_requests_new->fetch())
	{
		status("The current request is --- $id of $name");
		status("Processing tuple(name=$name,email=$email,loci=$loci,id=$id,algo=$algo)");
		$input_file=$id."_new";
		$input_path=$data_dir.$input_file;

		# Run 2-allele
		my $pkin_input_data_path=$input_path;
		my $intermediate_base_file_name=$id."_sets";
		my $pkin_output_data_path=$intermediate_dir.$intermediate_base_file_name;
		my $pkin_arg_num_threads=8;
		my $pkin_arg_num_logical_threads_local_loci_reconstruction=$loci;
		# Set to -1 to to allow for better load balancing (this is key!)
		my $pkin_arg_num_logical_threads_loci_intersection = -1; # ($pkin_arg_num_threads>$loci)?$pkin_arg_num_threads:$loci;
		my $pkin_arg_stacksizekb=1024;
		my $pkin_arg_recon_algo=1; # centralized=0, island=1, work stealing=2
		my $pkin_arg_num_loci_clusters=$loci;
		my $pkin_cmd_line="./$parallel_binary_name $algo $pkin_input_data_path $pkin_arg_num_threads $pkin_arg_num_logical_threads_local_loci_reconstruction $pkin_arg_num_logical_threads_loci_intersection $pkin_arg_stacksizekb $pkin_arg_recon_algo $pkin_arg_num_loci_clusters $pkin_output_data_path";
		my @pkin_cmd_line_array=("./$parallel_binary_name",$algo,$pkin_input_data_path,$pkin_arg_num_threads,$pkin_arg_num_logical_threads_local_loci_reconstruction,$pkin_arg_num_logical_threads_loci_intersection,$pkin_arg_stacksizekb,$pkin_arg_recon_algo,$pkin_arg_num_loci_clusters,$pkin_output_data_path);
		chdir $parallel_binary_dir;
		status("Running this --> $pkin_cmd_line");
### SKIPPING PKIN EXECUTION!!!
		#system(@pkin_cmd_line_array);
###
			
		if ( $algo eq "consensus" )
		{
			# Set up final data path for patching
			$final_output_path=$pkin_output_data_path."_consensus";
		}
		# Run 2-allele scripts
		elsif ( $algo eq "2allele" )
		{
			# Run gams
			my $gms_file_path=$pkin_output_data_path.".gms";
			if(not -e $gms_file_path)
			{ 
				status("$gms_file_path not found"); 
			} 				
			else
			{
				# Switch to output dir so data is generated in the output directory
				chdir $intermediate_dir;
				my $gams_cmd_line="gams $gms_file_path";
				status("$gms_file_path found. Running $gams_cmd_line"); ;
				system($gams_cmd_line);    

				# Run extract-solution
				my $lst_file_path=$pkin_output_data_path.".lst";
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
					my $sol_file_path=$pkin_output_data_path.".sol";
					if(not -e $sol_file_path)	
					{  					
						status("$sol_file_path not found");	
					} 					 			
					else
					{ 
						$final_output_path=$pkin_output_data_path.".soln";
						my $soln_file_path=$final_output_path;
						my $write_solution_script_path=$scripts_dir.$write_solution_script;
						my $write_solution_cmd_line="perl $write_solution_script_path -i $pkin_output_data_path -s $sol_file_path -o $soln_file_path";
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
		$final_patched_output_path=$intermediate_dir.$id."_mailop.txt";
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
		$content = $content."You can also retrieve the results by going to \"Check Status\"(http://kinalyzer.rutgers.edu/status.html) page on the website. Use the request id and your email address to access the information.\n";
		$content = $content."If you publish these results using analysis performed by Kinalyzer please acknowledge it by the following citations:\n";
		if ($algo eq "2allele") 
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
		$sendmail="/usr/sbin/sendmail -t";
		$sendmailbody=$finished_dir.$id."_email";
		$reply_to = "Reply-to:tanyabw\@uic.edu\n";
		$cc_to="Cc: perezrat\@uic.edu\n";
		$send_to = "To:".$email."\n";
		$subject="Subject: Kinalyzer output for $name, request id=$id\n";
		open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
		open(SENDMAILBODY, ">$sendmailbody") or die "Cannot open $sendmailbody: $!";
		print SENDMAIL $reply_to;
		print SENDMAIL $subject;
		print SENDMAIL $send_to;
		print SENDMAIL $cc_to;
		print SENDMAIL "Content-type: text/plain\n\n";
		print SENDMAIL $content;
		print SENDMAILBODY $content;
		close(SENDMAILBODY);
		close(SENDMAIL);
			
		# Update database to signify that request was processed
		my $update = $dbh->prepare("UPDATE requests_new SET status=4 WHERE request_id= $id");
		$update->execute;
			
		# Compress intermediate files, assumes no other jobs are running: each scheduler needs its own intermediate folder
		chdir $intermediate_dir;
		$tar_file_name=$id."-int.tar.gz";
		system("tar cvzf $tar_file_name *");
		system("mv $tar_file_name $archives_dir");
		system("rm *");
					
	} # end of fetch loop
} 

sub status
{ 	
	my $tm = scalar localtime; 
	print "$tm @_\n"; 
}  

main();
