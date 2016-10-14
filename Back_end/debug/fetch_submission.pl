#!/usr/bin/perl

#Load libraries
use DBI;
use IO::Handle;

sub main() {

	#Kinalyzer database	
	my $dsn = 'DBI:mysql:kinalyzer:192.168.102.2';
	my $db_username = 'kinalyzer';
	my $db_password = 'OMDHaF!';

	#Autoflushing 
	$| = 1;

	#Connect to database
	my $dbh = DBI->connect($dsn, $db_username, $db_password);
	if ($dbh && $#ARGV==0){

		#Fetch new jobs
		my $get_new_jobs = $dbh->prepare("SELECT * FROM requests WHERE id=$ARGV[0];");
        	$get_new_jobs->execute();
			
		#If there is a job jobs
		if($get_new_jobs->rows>0){ 
						
			#Bind columns
			$get_new_jobs->bind_columns(\$id,\$time,\$user,\$email,\$population,\$loci,\$upload,\$algorithm,\$state,\$result);		
			#Iterate over the jobs
			while($get_new_jobs->fetch())  {

				#Store upload in a temporary file
                                my $filename = "submission_".$id."_".$population."_".$loci.".txt";
				open FILE, ">$filename";
                                print FILE $upload;
                                close FILE;
			}
		}
	}
}
main();

