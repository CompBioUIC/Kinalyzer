#!/usr/bin/perl

# Routine(s) for generating and mailing the final report

use strict;

require "./pkLoggerUtils.pl";

# closure to avoid global variables
{

# The url at which persons may check their job status at with a job id
my $kinalyzer_status_url = "http://kinalyzer.rutgers.edu/status.html";

# Generates and mails the final report
# param data_dir - path to uploaded database files
# param finished_dir - path to finished data files
# param job_input_path - path to uploaded job input data from database
# param job_intermediate_dir - directory of intermediate files of completed job
# param job_final_output_path - path to final output of PKin, GAMS, and scripts
# param job_record_attribs_array - array storing attributes of current job record from database
sub pkGenerateAndMailFinalReport
{
	# Extract arguments
	my $data_dir = shift;
	my $finished_dir = shift;
	my $job_input_path = shift;
	my $job_intermediate_dir = shift;
	my $job_final_output_path = shift;
	my ( $name, $email, $loci, $algo, $status, $id, $count ) = @_;
	
	# Extract animal id's from input file
	my @animal_id;
	{	
		open(INP, "<$job_input_path");	
		my $inp_line_cnt = 0;
		while(<INP>)
		{
			my @words_in_row = split(/,/, $_);
			$animal_id[$inp_line_cnt] = $words_in_row[0];
			$inp_line_cnt++;				
		}
		close(INP);
	}

	# Verify copied solution exists
	my $job_final_patched_output_path=$job_intermediate_dir.$id."_mailop.txt";
	if(not -e $job_final_output_path)	
	{
		open(MAIL, ">$job_final_patched_output_path");
		print MAIL "There were some errors and the solution could not be computed.";
		close(MAIL);
		status("$job_final_output_path not found");
	}
	
	# Perform patching of animal id's in final e-mail file
	{
		open(SOL, "<$job_final_output_path");
		open(SOL2, ">$job_final_patched_output_path");
		status("patching $job_final_output_path to $job_final_patched_output_path");
		my(@lines) = <SOL>;
		my $line_cnt =0;
		foreach my $line (@lines) 
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
	}

	# Clear animal id's array because we are in a loop?
	for(my $p=0; $p<=$#animal_id; $p++) 
	{
		$animal_id[$p]= "";
	}
	
	# E-mail body with greeting and citation
	my $content = "Hi $name! \n\nThank you for using Kinalyzer to run $algo algorithm.\n";
	$content = $content."The output, the input and the file you uploaded are given below. The individuals in each set of siblings are given by the individual IDs from the input file.\n";
	$content = $content."You can also retrieve the results by going to \"Check Status\"($kinalyzer_status_url) page on the website. Use the request id and your email address to access the information.\n";
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
	open(OUTPUT, $job_final_patched_output_path) or die "Can't open output file : $!";
	while(<OUTPUT>) 
	{
		$content=$content.$_."\n";
	}
	close OUTPUT;
	$content= $content."------------------ OUTPUT END -------------------\n\n";			

	# Paste input to kinalyzer
	$content= $content."\n\n------------ Input sent to Kinalyzer -------------\n\n";			
	open(INPUT, $job_input_path) or die "Can't open input file : $!";
	while(<INPUT>) 
	{
		$content=$content.$_."\n";
	}
	close INPUT;
	$content= $content."----------- End of Input to Kinalyzer ------------\n\n";			

	# Paste original uploaded data
	{
		$content= $content."\n\n----------------- Uploaded file ------------------\n\n";
		my $uploaded_input_path=$data_dir.$id;	
		open(IN, $uploaded_input_path) or die "Can't open original file : $!";
		while(<IN>) 
		{
			$content=$content.$_."\n";
		}
		close IN;
		$content=$content."------------- End of Uploaded file --------------\n\n";
	}

	# Closing statements
	$content=$content."\n\nWe would like to know your comments about our algorithms and the website. Please feel free to post them at the 'Contact Us'(http://kinalyzer.cs.uic.edu/contact.html) page on the website.\n";
	$content=$content."\nRegards,\nThe Kinalyzer team.\n";
	
	# Generate e-mail
	my $sendmail="/usr/sbin/sendmail -t";
	my $sendmailbody=$finished_dir.$id."_email";
	my $reply_to = "Reply-to:tanyabw\@uic.edu\n";
	my $cc_to="Cc: perezrat\@uic.edu\n";
	my $send_to = "To:".$email."\n";
	my $subject="Subject: Kinalyzer output for $name, request id=$id\n";
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
}			

# In order to indicate module loading was successful, must return 1
# http://lists.netisland.net/archives/phlpm/phlpm-2001/msg00426.html
1;

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

