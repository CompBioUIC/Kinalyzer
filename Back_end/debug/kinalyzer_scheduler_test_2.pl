#!/usr/bin/perl

#Load libraries
use DBI;
use IO::Handle;

#Daemon log
my $log_handle;

#Kinalyzer database
my $dsn = 'DBI:mysql:kinalyzer:192.168.102.2';
my $db_username = 'kinalyzer';
my $db_password = 'OMDHaF!';


sub main() {

	#Kinalyzer database	
	# my $dsn = 'DBI:mysql:kinalyzer:192.168.102.2';
	# my $db_username = 'kinalyzer';
	# my $db_password = 'OMDHaF!';

	#Open kinalyzer log file	
	open $log_handle, ">> ./log_file.txt" or die;
	$log_handle->autoflush;
	status("Daemon Started.");

	#Autoflushing 
	$| = 1;

	#Daemon is an infinite loop with 60 second polling
	while(1) {
	
		#Connect to database
		my $dbh = DBI->connect($dsn, $db_username, $db_password);
		if (!$dbh){
			status("DBI->errstr");
		}
		else{

			#Fetch new jobs
			my $get_new_jobs = $dbh->prepare("SELECT * FROM requests WHERE state=0 ORDER BY id ASC ");
               		$get_new_jobs->execute();
			
			#If there are new jobs
			if($get_new_jobs->rows>0){ 
				status($get_new_jobs->rows." new jobs to kinalyze.");			
						
				#Bind columns
				$get_new_jobs->bind_columns(\$id,\$time,\$user,\$email,\$population,\$loci,\$upload,\$algorithm,\$state,\$result);		
				#Iterate over the jobs
				while($get_new_jobs->fetch())  {

					#Run different algorithms
					status("The current job is $id of $user");			
					
                                       					updateAndSendErrorMsg();
				}
			}
		}
	
		#Polling every 60 seconds
                sleep(60);
        }
        close $log_handle;
}
main();

#Print to log
sub status {
        my $tm = scalar localtime;
        print $log_handle "$tm @_\n";
}

sub updateAndSendErrorMsg {

        #Store error state into the database
        status("Error in computing solution for job [$id]");
        
        #Connect to database
        my $dbh = DBI->connect($dsn, $db_username, $db_password);
        if (!$dbh){
		status("In updateAndSendErrorMsg. Cannot connect to Database");
                status("DBI->errstr");
       }
       else
       {
        
        	$update = $dbh->prepare("UPDATE requests SET state=2 WHERE id= $id");
	        $update->execute();
        	status("Updated state=2 (Failed)  in database for job [$id]");

	        #Prepare output mail
        	status("Sending mail to $user at $email");
	        $sendmail = "/usr/sbin/sendmail -t";
        	$reply_to = "Reply-to: kinalyzer\@cs.uic.edu\n";
	        $send_to = "To: ".$email."\n";
	        $bcc = "Bcc: tanyabw\@uic.edu, abanso2\@uic.edu\n";
        	if ($algorithm==0){
                	$algo = "2 allele";
        	}
	        elsif($algorithm==1){
        	        $algo = "consensus";
        	}
	        $content = "Hi $user! \n\nThank you for using Kinalyzer to run $algo algorithm.\n Unfortunately, we encountered an error in computing output for your job $id submitted at $time, for $loci loci and $population individuals.\n\n Kinalyzer has been notified. Please follow up with kinalyzer\@cs.uic.edu .\n\n";

        	# $content = $content."Output:\nThere were some errors and the solution could not be computed.\nKindly contact kinalyzer\@cs.uic.edu .";

	        $from = "From: Kinalyzer <kinalyzer\@pachy.cs.uic.edu>\n";
        	$subject = "Subject: Kinalyzer output for $user, request id= $id\n";

	        #Send mail
        	open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
	        print SENDMAIL $from;
        	print SENDMAIL $reply_to;
	        print SENDMAIL $subject;
        	print SENDMAIL $send_to;
        	print SENDMAIL $bcc;
	        print SENDMAIL "Content-type: text/plain\n\n";
        	print SENDMAIL $content;
	        close(SENDMAIL);
		status("Mail sent to $user at $email for failed job [$id].");
	}
}

