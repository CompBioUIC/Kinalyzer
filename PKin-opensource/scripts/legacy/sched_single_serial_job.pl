#!/usr/bin/perl

#----------------------------------------#
#  PROGRAM:  runs a single serial job    #
#----------------------------------------#

use DBI;

# Paths and file names
$home_dir="/home/ramji/";
$error_output_dir=$home_dir;
$serial_binary_dir="/home/priya/consensus/";
$serial_binary_name="sets";
$serial_error1_file_name="errserial1.txt";
$serial_error2_file_name="errserial2.txt";
$log_file_dir=$home_dir;
$log_file_name="log_file_sched_single_serial_job.txt";
$data_dir="/var/www/uploads_new/";
$gams_binary_dir="/opt/lx3gams/";
$gams_binary_name="gams";
$gams_error1_file_name="errgms1.txt";
$gams_error2_file_name="errgms2.txt";
$scripts_dir="/var/www/newscripts/";
$extract_solution_param_script="extract-solution-param.pl";
$extract_solution_error1_file_name="errextsoln1.txt";
$extract_solution_error2_file_name="errextsoln2.txt";
$write_solution_script="write-solution.pl";
$write_solution_error1_file_name="errwritesoln1.txt";
$write_solution_error2_file_name="errwritesoln2.txt";
$intermediate_output_dir=$home_dir;
$final_output_dir=$home_dir; # /var/www/uploads_new/

# Log session to this file
$log_handle;
open $log_handle, ">> $log_file_dir$log_file_name" or die;

# Function to output to log file
sub status
{
	my $tm = scalar localtime;
	print $log_handle "$tm @_\n";
	print "$tm @_\n";
}

# Function to print command line usage of this script
sub print_usage()
{
	print "perl sched_single_serial_job.pl <job_id>\n"
}

# Determine number of arguments
$num_args = $#ARGV + 1;
if ( $num_args != 1 )
{
	status("Received $num_args commandline arguments.  Expect exactly 1 commandline argument");
	print_usage();
	exit(0);
}

# Get job id from commandline
$job_id_arg = $ARGV[0];
status("Running script for job-id: $job_id_arg");

# Connect to database
$dsn = 'DBI:mysql:interface_data:localhost';
$db_username = 'www_user';
$db_password = 'sibs';
$dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;

status("Daemon Started. ");
$|=1;

# Query database for specific job id
$get_requests_new = $dbh->prepare("SELECT * FROM requests_new WHERE request_id=$job_id_arg ORDER BY request_id ASC ");
$get_requests_new->execute;		
$get_requests_new->bind_columns( \$name, \$email, \$loci, \$algo, \$status, \$id, \$count);

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

# Fetch job information
if ($get_requests_new->fetch())
{
	status("Found job with name($name), email($email), loci($loci), algo($algo), status($status), id($id), count($count)");
	$input_file=$id."_new";
	$input_path=$data_dir.$input_file;
	$final_output_path=$final_output_dir.$id."_soln.txt";
	
	# Execute job
	my $serial_error1_path=$error_output_dir.$serial_error1_file_name;
	my $serial_error2_path=$error_output_dir.$serial_error2_file_name;
	
	if($algo=='2allele')
	{
		my $serial_intermediate_output_path=$intermediate_output_dir."output";
		status("Running --> nice ./sets test $loci $input_path $serial_intermediate_output_path 2>>$serial_error2_path 1>>$serial_error1_path");
		chdir $serial_binary_dir;
		system("nice ./sets test $loci $input_path $serial_intermediate_output_path");
		# Verify a gms file exists
		my $gms_file_path=$serial_intermediate_output_path.".gms";
		if(not -e $gms_file_path)	
		{
			status("$gms_file_path not found");
			exit(0);
		}	
		
		# GMS file - output.gms - found.  Run gams!
		my $gams_binary_path=$gams_binary_dir.$gams_binary_name;
		my $gams_error1_path=$error_output_dir.$gams_error1_file_name;
		my $gams_error2_path=$error_output_dir.$gams_error2_file_name;
		status("$gms_file_path found. Running $gams_binary_path $gms_file_path 1>$gams_error1_path 2>>$gams_error2_path");
		system("$gams_binary_path $gms_file_path 1>$gams_error1_path 2>>$gams_error2_path");
		if(not -e "output.lst")	
		{
			status("output.lst not found");
			exit(0);
		}	
			
		# LST file - output.list - found. Run extract-solution-param.pl!
		my $extract_solution_script_path=$scripts_dir.$extract_solution_param_script;
		my $extract_solution_error1_path=$error_output_dir.$extract_solution_error1_file_name;
		my $extract_solution_error2_path=$error_output_dir.$extract_solution_error2_file_name;
		status("output.lst found. Running perl $extract_solution_script_path output.lst 2>>$extract_solution_error2_path 1>>$extract_solution_error1_path");
		system("perl $extract_solution_script_path output.lst 2>>$extract_solution_error2_path 1>>$extract_solution_error1_path");
		if(not -e "output.sol")	
		{
			status("output.sol not found");
			exit(0);
		}					
	
		# SOL file - output.sol - found. Run write-solution.pl"
		my $write_solution_script_path=$scripts_dir.$write_solution_script;
		my $write_solution_error1_path=$error_output_dir.$write_solution_error1_file_name;
		my $write_solution_error2_path=$error_output_dir.$write_solution_error2_file_name;
		status("output.sol found. Running perl $write_solution_script_path -i output -s output.sol -o output.soln 2>>$write_solution_error2_path 1>>$write_solution_error1_path");							
		system("perl $write_solution_script_path -i output -s output.sol -o output.soln 2>>$write_solution_error2_path 1>>$write_solution_error1_path");
		if(not -e "output.soln")
		{
			status("output.soln not found");
			exit(0);
		}			
	
		# Copy output and remove any temporary files
		status("output.soln found. Copying it to $final_output_dir and deleting temp files.");		
		system("cp output.soln $final_output_path");
		system("rm $serial_intermediate_output output.lst output.sol $gms_file_path output.soln");
	} #end of 2 allele computation
	else
	{				
		system("cp $input_path $serial_binary_dir");
		status("Running this --> nice ./sets autogreedyconsensus $loci $input_file");
		chdir $serial_binary_dir;
		#Running consensus
		system("nice ./sets autogreedyconsensus $loci $input_file 2>>$serial_error2_path 1>>$serial_error1_path");
		my $serial_intermediate_output_path=$serial_binary_dir."consensus_".$id."_new";
		system("cp $serial_intermediate_output_path $final_output_path");
		status("Solution written to $final_output_path");
		system("rm *_new  *.gms  *.lst  *_set*  *.sol*");
	} #end of consensus computation
	
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
	$final_patched_output_path=$final_output_dir.$id."_mailop.txt";
	if(not -e $final_output_path)	
	{
		#open(MAIL, ">$final_patched_output_path");
		#print MAIL "There were some errors and the solution could not be computed.";
		#close(MAIL);
		status("$final_output_path not found");
		exit(0);
	}
	
	# Open files to patch animal id's
	open(SOL, "<$final_output_path");
	open(SOL2, ">$final_patched_output_path");
	status("patching $final_output_path to $final_patched_output_path");
	
	# Perform patching of animal id's in final e-mail file	
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
	
	# Clear animal id's array because in original code, this was in a loop, and maybe make a loop again
	#for($p=0; $p<=$#animal_id; $p++) 
	#{
	#	$animal_id[$p]= "";
	#}
	
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
	$sendmail=$final_output_dir.$id."_email.txt";
	$from="From: pgovin2\@uic.edu\n";
	$reply_to="Reply-to: pgovin2\@uic.edu\n";
	$send_to="To: ".$email."\n";
	$subject="Subject: Kinalyzer output for $name, request id=$id\n";
	#open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
	open(SENDMAIL, ">$sendmail") or die "Cannot open $sendmail: $!";
	print SENDMAIL $reply_to;
	print SENDMAIL $subject;
	print SENDMAIL $send_to;
	print SENDMAIL "Content-type: text/plain\n\n";
	print SENDMAIL $content;
	close(SENDMAIL);
	
	# Update database status
	my $update = $dbh->prepare("UPDATE requests_new SET status=2 WHERE request_id= $id");
	$update->execute;
}
else
{
	status("Unable to find job with id $job_id_arg in database");
	print_usage();
	exit(0);
}
