#!/usr/bin/perl

use DBI;

my $log_handle;

sub main() {
	my $dsn = 'DBI:mysql:interface_data:localhost';
	my $db_username = 'www_user';
	my $db_password = 'sibs';
	my $dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;

	open $log_handle, ">> /home/priya/consensus/log_file.txt" or die;
	status("Daemon Started. ");
	$|=1;
	#system("PATH=$PATH:/opt/lx3gams:/home/priya/priyascripts");
	#system("export PATH");

	while(1) {
		my $get_requests_new = $dbh->prepare("SELECT * FROM requests_new WHERE status=1 ORDER BY request_id ASC ");
		$get_requests_new->execute;		
		$get_requests_new->bind_columns( \$name, \$email, \$loci, \$algo, \$status, \$id, \$count);
		#status("Getting new requests");
		while($get_requests_new->fetch())  {
			status("The current request is --- $id of $name");
			## call method to run algo
			### Running the algo

			## Open inputfile and check for no-of-line*loci less than a specified limit	
			$lines=countlines($id);	
			$linesbyloci=$lines*$loci;	
			my $update = $dbh->prepare("UPDATE requests_new SET count=$linesbyloci WHERE request_id= $id");
			$update->execute;

			if($linesbyloci>2000)	{
				my $update = $dbh->prepare("UPDATE requests_new SET status=3 WHERE request_id= $id");
				$update->execute;

			############## Email telling them that their job is too big ##############

				$sendmail = "/usr/sbin/sendmail -t";
				$from = "From: tanyabw\@uic.edu\n";
				$reply_to = "Reply-to: tanyabw\@uic.edu\n"  ;
				$send_to = "To: tanyabw\@uic.edu\n";
				#$send_to = "To: wandathefish\@gmail.com\n";
				$content = "Hi, \nThis is just to let you know that $name uploaded a file (request id: $id) recently thats $lines lines and $loci loci (i.e $linesbyloci lines by loci). We would have to process this job on some other machine since its too big for Kinalyzer to handle. \n\n Kinalyzer";
				$subject = "Subject: Kinalyzer - Big file submitted by $name, request id $id\n";
			###
				open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
				print SENDMAIL $reply_to;
				print SENDMAIL $subject;
				print SENDMAIL $send_to;
				print SENDMAIL "Content-type: text/plain\n\n";
				print SENDMAIL $content;
				close(SENDMAIL);
			

			############## End of mail ##############
				next;
			}
		
			if($algo=='2allele')	{

				status("Running this --> nice ./sets test $loci /var/www/uploads_new/".$id."_new output 2>>errstr.txt 1>>errstr2.txt");
				chdir "/home/priya/consensus/";
				system("nice ./sets test $loci /var/www/uploads_new/".$id."_new output ");
				if(not -e "output.gms")	{
					#print to errorlog file
					status("output.gms not found");
					#exit(0);
				}	
				#open(OUT, "./sets test $loci ../uploads_new/".$id."_new output |");
				#$out_str = <OUT>;
				#status($out_str);
				#close(OUT);
				
				else	{				
					status("output.gms found. Running gams");		
					system("/opt/lx3gams/gams output.gms 1>errstrgams.txt 2>>errstr.txt");
					if(not -e "output.lst")	{
						#print to errorlog file
						status("output.lst not found");		
					}	
			
					else	{
						status("output.lst found. Running extract-solution-param.pl");		
						system("perl /var/www/newscripts/extract-solution-param.pl output.lst 2>>errstr.txt 1>>errstr2.txt");
						if(not -e "output.sol")	{
							#print to errorlog file
							status("output.sol not found");		
						}					
					
						else	{
							status("output.sol found. Running write-solution.pl");							
							system("perl /var/www/newscripts/write-solution.pl -i output -s output.sol -o output.soln 2>>errstr.txt 1>>errstr2.txt");
							if(not -e "output.soln")	{
								#print to errorlog file
								status("output.soln not found");		
							}			
							else	{
								status("output.soln found. Copying it to uploads_new dir and deleting temp files.");		
								system("cp output.soln /var/www/uploads_new/".$id."_soln.txt");
								system("rm output output.lst output.sol output.gms output.soln");
							}		
						}
					}
				}
			
			} #end of 2 allele computation

			else {				
				system("cp /var/www/uploads_new/".$id."_new /home/priya/consensus/");
				status("Running this --> nice ./sets autogreedyconsensus $loci ".$id."_new ");
				chdir "/home/priya/consensus/";
				#Running consensus
				system("nice ./sets autogreedyconsensus $loci ".$id."_new 2>>errstr2.txt 1>>errstr1.txt");
				system("cp /home/priya/consensus/consensus_".$id."_new /var/www/uploads_new/".$id."_soln.txt");
				#system("rm *".$id."_new*");
				#system("rm  *.gms  *.lst  *_set*  *.sol*");
				status("Solution written to /var/www/uploads_new/".$id."_soln.txt");
				system("rm *_new  *.gms  *.lst  *_set*  *.sol*");
				
			} #end of consensus computation
							
			open(INP, "< /var/www/uploads_new/".$id."_new");
			$inplinecnt = 0;
			while(<INP>)	{
				@words_temp = split(/,/, $_);
				$animal_id[$inplinecnt] = $words_temp[0];
				$inplinecnt++;				
			}
			close(INP);
			
			if(not -e "/var/www/uploads_new/".$id."_soln.txt")	{
				open(MAIL, "> /var/www/uploads_new/".$id."_mailop.txt");
				print MAIL "There were some errors and the solution could not be computed.";
				close(MAIL);
			}
			else 	{
				open(SOL, "< /var/www/uploads_new/".$id."_soln.txt");
				open(SOL2, "> /var/www/uploads_new/".$id."_mailop.txt");
				
				my(@lines) = <SOL>;
				$linecnt =0;
				foreach $line (@lines) {
					my @sibs = split(/\s+/, $line);
					$linecnt++;
					my $printline = "Sibs Set ".$linecnt.": ".$animal_id[$sibs[1]];
					for (2..$#sibs) {
						$printline = $printline.", ".$animal_id[$sibs[$_]];
					}	
					print SOL2 $printline."\n";	
				}			
				close(SOL);
				close(SOL2);
				
				for($p=0; $p<=$#animal_id; $p++) {
					$animal_id[$p]= "";
				}
			}
		
			### Mailing the output
			$sendmail = "/usr/sbin/sendmail -t";
			$from = "From: pgovin2\@uic.edu\n";
			$reply_to = "Reply-to: pgovin2\@uic.edu\n";
			$send_to = "To: ".$email."\n";
			$content = "Hi $name! \n\nThank you for using Kinalyzer to run $algo algorithm.\n The output, the input and the file you uploaded are given below. The individuals in each set of siblings are given by the individual IDs from the input file. \n  You can also retrieve the results by going to \"Check Status\"(http://kinalyzer.cs.uic.edu/status.html) page on the website. Use the request id and your email address to access the information. \n

If you publish these results using analysis performed by Kinalyzer please 
acknowledge it by the following citations: \n";


			if ($algo=='2allele') {
				$content= $content."\nT.Y. Berger-Wolf, S.I. Sheikh, B. DasGupta, M.V. Ashley, I.C. Caballero, 
W. Chaovalitwongse, S.L. Putrevu, 'Reconstructing Sibling Relationships in Wild Populations', Bioinformatics, 23(13)\n\nM. V. Ashley, I. C. Caballero, W. Chaovalitwongse, B. DasGupta, P. Govindan, S. Sheikh and T. Y. Berger-Wolf. 'KINALYZER, A Computer Program for Reconstructing Sibling Groups', Molecular Ecology Resources \n \n";
			}
			else {
				$content= $content."\nS. I. Sheikh, T. Y. Berger-Wolf, Ashfaq A. Khokhar and B. DasGupta 'Consensus Methods for Reconstruction of Sibling Relationships from Genetic Data', Proceedings of the 4th Multidisciplinary Workshop on Advances in Preference Handling at AAAI 08.\n \nM. V. Ashley, I. C. Caballero, W. Chaovalitwongse, B. DasGupta, P. Govindan, S. Sheikh and T. Y. Berger-Wolf. 'KINALYZER, A Computer Program for Reconstructing Sibling Groups', Molecular Ecology Resources \n \n";
			}

			$content= $content."\n\n ------------------ OUTPUT START -----------------\n\n";			
			open( OUTPUT, "/var/www/uploads_new/".$id."_mailop.txt") or die "Can't open output file : $!";
				while( <OUTPUT> ) {
			        $content= $content.$_."\n";
			}
			close OUTPUT;
			$content= $content." ------------------ OUTPUT END -------------------\n\n";			

			$content= $content."\n\n------------ Input sent to Kinalyzer -------------\n\n";			
			open( INPUT, "/var/www/uploads_new/".$id."_new") or die "Can't open input file : $!";
				while( <INPUT> ) {
			        $content= $content.$_."\n";
			}
			close INPUT;
			$content= $content."----------- End of Input to Kinalyzer ------------\n\n";			

			$content= $content."\n\n----------------- Uploaded file ------------------\n\n";			
			open( IN, "/var/www/uploads_new/".$id) or die "Can't open original file : $!";
				while( <IN> ) {
			        $content= $content.$_."\n";
			}
			close IN;
			$content= $content."------------- End of Uploaded file --------------\n\n";			

			$content= $content."\n\nWe would like to know your comments about our algorithms and the website. Please feel free to post them at the 'Contact Us'(http://kinalyzer.cs.uic.edu/contact.html) page on the website. \n\nRegards, \nThe Kinalyzer team.";


			$subject = "Subject: Kinalyzer output for $name, request id=$id\n";
			###
			open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
			print SENDMAIL $reply_to;
			print SENDMAIL $subject;
			print SENDMAIL $send_to;
			print SENDMAIL "Content-type: text/plain\n\n";
			print SENDMAIL $content;
			close(SENDMAIL);
			### mailed output
			
			my $update = $dbh->prepare("UPDATE requests_new SET status=2 WHERE request_id= $id");
			$update->execute;
			#system("rm -f output output.lst output.sol output.gms output.soln");
		}

		
		#my $update = $dbh->prepare("UPDATE requests_new_new SET status=0 WHERE request_id=?", [$req->[4]]);   #$update->execute;
		
		sleep(10);
	}
	close $log_handle;
}
main();


sub status {
	my $tm = scalar localtime;
	print $log_handle "$tm @_\n";
}

sub countlines {
	open(IN, "/var/www/uploads_new/".$id."_new")|| die "ERROR: $!"."-- /var/www/uploads_new/".$_."_new";
	my $count;
	while(<IN>) {
		chomp;
		$count++;
	}	
	close(IN);
	print "The number of lines is : $count \n";
	return $count;
}


