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
			}	
			#open(OUT, "./sets test $loci ../uploads/".$id."_new output |");
			#$out_str = <OUT>;
			#status($out_str);
			#close(OUT);
			
			system("/opt/lx3gams/gams output.gms 1>errstrgams.txt 2>>errstr.txt");
			#open(OUT, "/opt/lx3gams/gams output.gms |") ;
			#$out_str = <OUT>;
			#status("Comments from running gams --");
			#status($out_str);
			#close(OUT);

			system("perl /var/www/newscripts/extract-solution-param.pl output.lst 2>>errstr.txt 1>>errstr2.txt");
			#open(OUT, "perl /var/www/newscripts/extract-solution-param.pl output.lst |") ;
			#$out_str = <OUT>;
			#status("comments from running extract script --");
			#status($out_str);
			#close(OUT);
			
			system("perl /var/www/newscripts/write-solution.pl -i output -s output.sol -o output.soln 2>>errstr.txt 1>>errstr2.txt");
			#open(OUT,"perl /var/www/newscripts/write-solution.pl -i output -s output.sol -o output.soln |") ;
			#$out_str = <OUT>;
			#status("**write soln performed**");
			#status($out_str);
			#close(OUT);

			open(OUT,"cp output.soln /var/www/uploads/".$id."_soln.txt |");
			$out_str= <OUT>;
			status($out_str);
			close(OUT);

			system("rm output output.lst output.sol output.gms output.soln");
			###

			$getnames = "SELECT name, ind_id FROM data".$id;
			my $names = $dbh->prepare($getnames);
			$names->execute;
			my %animal_id = ();
			while($row = $names->fetchrow_hashref)
			{
				$key = $row->{"ind_id"}; 
				$value =$row->{"name"};
				$animal_id{$key} = $value;
			}
			open(SOL, "< /var/www/uploads/".$id."_soln.txt");
			open(SOL2, "> /var/www/uploads/".$id."_mailop.txt");
			my(@lines) = <SOL>;
			$linecnt =0;
			foreach $line (@lines) {
				my @sibs = split(/\s+/, $line);
				$linecnt++;
				my $printline = "Set ".$linecnt.": ".$animal_id{$sibs[1]+1};
				for (2..$#sibs) {
					$printline = $printline.", ".$animal_id{$sibs[$_]+1};
				}	

				print SOL2 $printline."\n";	
			}
			
			close(SOL);
			close(SOL2);	

	
			### Mailing the output
			$sendmail = "/usr/sbin/sendmail -t";
			$reply_to = "Reply-to: pgovin2\@uic.edu\n";
			$send_to = "To: ".$email."\n";
			$content = "Hi $name ! Here is the Output to the input(testudo.cs.uic.edu/uploads/".$id.") you had uploaded, for $loci loci --\n\n";
			$outputfile = "/var/www/uploads/".$id."_mailop.txt";
			$subject = "Subject: Output file\n";
			###
			open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
			print SENDMAIL $reply_to;
			print SENDMAIL $subject;
			print SENDMAIL $send_to;
			print SENDMAIL "Content-type: text/plain\n\n";
			print SENDMAIL $content;
			open(FILE, "$outputfile") or die "Cannot open $outputfile: $!";
			print SENDMAIL <FILE>;
			close (FILE);
			close(SENDMAIL);
			### mailed output
			
			my $update = $dbh->prepare("UPDATE requests SET status=0 WHERE request_id= $id");
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


