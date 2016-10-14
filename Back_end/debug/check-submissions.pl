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
	%errors = ();
	my $dbh = DBI->connect($dsn, $db_username, $db_password);
	if ($dbh){

		#Fetch new jobs
		my $get_new_jobs;
		if (@ARGV==1){
			$get_new_jobs = $dbh->prepare("SELECT * FROM requests WHERE id=$ARGV[0];");
        	}
		else{
			$get_new_jobs = $dbh->prepare("SELECT * FROM requests WHERE state=1;");
		}
		$get_new_jobs->execute();
			
		#If there is a job jobs
		if($get_new_jobs->rows>0){ 
						
			#Bind columns
			$get_new_jobs->bind_columns(\$id,\$time,\$user,\$email,\$population,\$loci,\$upload,\$algorithm,\$state,\$result);		
			#Iterate over the jobs
			while($get_new_jobs->fetch())  {

				#Store upload in a temporary file
                                my $filename = "temp.txt";
				open FILE, ">$filename";
                                print FILE $upload;
                                close FILE;
				$filename = "temp_result.txt";
				open FILE, ">$filename";
                                print FILE $result;
                                close FILE;
				print $id." ";	
				
				system("./check-sibsets.pl -i temp.txt -s temp_result.txt ");		
				if ($? != 0){
					$errors{$id} = ();	
					$stmt = "UPDATE requests SET state=0 WHERE id=$id";
					my $update_handle = $dbh->prepare($stmt);
			                $update_handle->execute();
				}	

				#Delete temporary files
				system("rm temp.txt temp_result.txt");
			}
		}
	}
	foreach $key (keys(%errors)){
		print $key. " ";
	}
	print "\n";
    
}
main();

