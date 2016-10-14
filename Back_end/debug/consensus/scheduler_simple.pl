#!/usr/bin/perl

use DBI;

my $log_handle;

sub main() {
	my $dsn = 'DBI:mysql:interface_data:localhost';
	my $db_username = 'www_user';
	my $db_password = 'sibs';
	my $dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;

	open $log_handle, ">> log_file.txt" or die;
	status("Daemon Started. ");
	$|=1;

	while(1) {
		my $get_requests = $dbh->prepare("SELECT * FROM requests WHERE status=1 ORDER BY request_id ASC ");
		$get_requests->execute;		
		$get_requests->bind_columns( \$name, \$email, \$loci, \$status, \$id);
		while($get_requests->fetch())  {
			status("The current request is --- $id of $name \n");
			## call method to run algo
			### Running the algo
			system("./sets test $loci ../uploads/".$id."_new output 2>>errstr.txt 1>>errstr2.txt");
			if(not -e "output.gms")	{
				#print to errorlog file
				status("output.gms not found");
				#exit(0);
			}	
			#open(OUT, "./sets test $loci ../uploads/".$id."_new output |");
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
							status("output.soln found. Copying it to uploads dir and deleting temp files.");		
							system("cp output.soln /var/www/uploads/".$id."_soln.txt");
							system("rm output output.lst output.sol output.gms output.soln");
						}		
					}
				}
			}
			
							
			open(INP, "< /var/www/uploads/".$id."_new");
			$inplinecnt = 0;
			while(<INP>)	{
				@words_temp = split(/,/, $_);
				$animal_id[$inplinecnt] = $words_temp[0];
				$inplinecnt++;				
			}
			close(INP);
			
			if(not -e "/var/www/uploads/".$id."_soln.txt")	{
				open(MAIL, "> /var/www/uploads/".$id."_mailop.txt");
				print MAIL "There were some errors and the solution could not be computed.";
				close(MAIL);
			}
			else 	{
				open(SOL, "< /var/www/uploads/".$id."_soln.txt");
				open(SOL2, "> /var/www/uploads/".$id."_mailop.txt");
				
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
			$reply_to = "Reply-to: pgovin2\@uic.edu\n";
			$send_to = "To: ".$email."\n";
			$content = "Hi $name! Thank you for using Kinalyzer.\n Here is the Output to the input(testudo.cs.uic.edu/uploads/".$id."_new) you had uploaded, for $loci loci. The numbers in each sibs set are individual IDs from the input file. \n The entire original input, submitted input, and the output files will be available for 7 days at: http://testudo.cs.uic.edu/uploads/".$id."_mailop.txt. Use the request id and your email address to access the information. \n\n

If you publish results using analysis performed by Kinalyzer please 
acknowledge it by the following citation:
T.Y. Berger-Wolf, S.I. Sheikh, B. DasGupta, M.V. Ashley, I.C. Caballero, 
W. Chaovalitwongse, S.L. Putrevu, 'Reconstructing Sibling Relationships 
in Wild Populations', Bioinformatics, 23(13) --\n\n";
			#$content = "";
			$outputfile = "/var/www/uploads/".$id."_mailop.txt";
			#$outputfile = "/var/www/uploads/".$id."_soln.txt";
			$subject = "Subject: Kinalyzer output for $name, request id= $id\n";
			###
			open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
			print SENDMAIL $reply_to;
			print SENDMAIL $subject;
			print SENDMAIL $send_to;
			print SENDMAIL "Content-type: text/plain\n\n";
			print SENDMAIL $content;
			print SENDMAIL "\n\n ------------------ OUTPUT START-----------------\n\n";
			open(FILE, "$outputfile") or die "Cannot open $outputfile: $!";
			print SENDMAIL <FILE>;
			close (FILE);
			print SENDMAIL "\n ------------------ OUTPUT END-----------------\n";
			close(SENDMAIL);
			### mailed output
			
			my $update = $dbh->prepare("UPDATE requests SET status=2 WHERE request_id= $id");
			$update->execute;
			#system("rm -f output output.lst output.sol output.gms output.soln");
		}

		
		#my $update = $dbh->prepare("UPDATE requests SET status=0 WHERE request_id=?", [$req->[4]]);   #$update->execute;
		
		sleep(10);
	}
	close $log_handle;
}
main();


sub status {
	my $tm = scalar localtime;
	print $log_handle "$tm @_\n";
}


