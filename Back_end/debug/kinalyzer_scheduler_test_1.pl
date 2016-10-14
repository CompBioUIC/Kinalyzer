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
					
					#2 allele algorithm
					if ($algorithm==0){
						
						#Store upload in a temporary file
						open FILE, ">temp.txt" or die $!;					
						@temp = split ('\n',$upload);
						if (@temp==$population){
							print FILE $upload;
						}
						else{
                                                        for ($i=1; $i<=$#temp; ++$i){
                                                                print FILE $temp[$i]."\n";
                                                        }
						}
   						close FILE;					
							
						#Create maximal sibsets
						status("Running 2-allele algorithm");
						system("./sets test $loci temp.txt output 2>>./err.txt");
						if(not -e "output.gms") {
	                               			status("output.gms not found");
							updateAndSendErrorMsg();
							next;
	                       			}
						else{
							#Solve ILP problem with gams
							status("output.gms found. Running gams");
							system("/opt/gams/24.0.2/gams output.gms 2>>./err.txt");
							if(not -e "output.lst") {
	                                       			status("output.lst not found");
        	                       				updateAndSendErrorMsg();
								next;
							}	
							else {
								#Extract solution from ILP result
								status("output.lst found. Running extract-solution-param.pl");	
								system("/usr/bin/perl ./extract-solution-param.pl output.lst 2>>./err.txt");
								if(not -e "output.sol") {
			                                        	status("output.sol not found");
                                       					updateAndSendErrorMsg();
									next;
								}
								else{
									#Write final solution
									status("output.sol found. Running write-solution.pl"); 
       				                                        system("/usr/bin/perl ./write-solution.pl -i output -s output.sol -o output.soln 2>>err.txt");
			 	                                        if(not -e "output.soln") {
			                                                	status("output.soln not found after write-solution");
                                               				}
			 	                                        else    {
										#Store solution into the database
	                                                       			status("output.soln found");
			 
										#Copy solution into a temporary variable
										open FILE, "output.soln" or die "Couldn't open file: $!";
										$temp_result = join("", <FILE>);
										close(FILE);							
													
										#Delete temporary files
			                                                	system("rm output output.lst output.sol output.gms output.soln temp.txt"); 
									}
								}
							}
						}
					}
			
					#Consensus algorithm
					elsif($algorithm==1){

						#Store upload in a temporary file
                                                open FILE, ">temp.txt" or die $!;
                                                @temp = split ('\n',$upload);
                                                if (@temp==$population){
                                                        print FILE $upload;
                                                }
                                                else{   
                                                        for ($i=1; $i<=$#temp; ++$i){
                                                                print FILE $temp[$i]."\n";
                                                        }
                                                }
                                                close FILE;

						#Execute greedy consensus algorithm
						status("Running greedy consensus algorithm");
						system("./sets autogreedyconsensus $loci temp.txt 2>>./err.txt");
						
						#Copy solution into a temporary variable
                                                open FILE, "consensus_temp.txt" or die "Couldn't open file: $!";
                                                $temp_result = join("", <FILE>);
                                                close(FILE);
						
						#Delete temporary files
                                                system("rm *temp.txt*");					
					}

					#Check if valid solution
					if ($temp_result){
						status("Valid solution for job $id");
							
						#Get animal id from uploaded files
						@lines = split (/\n/, $upload);
						for($i=0; $i<$population;++$i) {
							
							#Depending by the header
							if($population < @lines){
								@words = split(/,/,$lines[$i+1]);
							}
							else{
								@words = split(/,/,$lines[$i]);
							}
							$animal_id[$i]=$words[0];
						}
					
						#Now write solution sibsets
						$result="";
						@lines = split (/\n/, $temp_result);
						for ($i=0;$i<@lines;++$i){
					
							#Get sibling individual in the current set
							@sibs = split(/\s+/, $lines[$i]);
							$line = "SibsSet ".$i.": ".$animal_id[$sibs[1]];
							for ($j=2; $j<@sibs; ++$j){
								$line = $line.",".$animal_id[$sibs[$j]];
							}
							$result = $result.$line."\n";							
						}

						#Store result into the database
						print $$temp_result;
						print $result;
						status("Store formatted result into database");
						$update = $dbh->prepare("UPDATE requests SET result=? WHERE id= $id");
						$update->execute($result);
						$update = $dbh->prepare("UPDATE requests SET state=1 WHERE id= $id");				
						$update->execute();
					}				
					else{
						#Store error state into the database
						status("Valid solution for job $id");
						$update = $dbh->prepare("UPDATE requests SET state=2 WHERE id= $id");
                                                $update->execute();
					}
					
					#Prepare output mail
					status("Send mail to $user at $email");
		        		$sendmail = "/usr/sbin/sendmail -t";
		                        $reply_to = "Reply-to: kinalyzer\@cs.uic.edu\n";
					$send_to = "To: ".$email."\n";
		                        if ($algorithm==0){
						$algo = "2 allele";
					}
					elsif($algorithm==1){
						$algo = "consensus";
					}
					$content = "Hi $user! Thank you for using Kinalyzer to run $algo algorithm.\n Here is the output to your job $id submitted $time, for $loci loci and $population individuals\n The numbers in each sibs set are individual IDs from the input file. \n Use the request id and your email address on http://kinalyzer.cs.uic.edu/status.html to access the information about the input and the output of the submitted job. \n\n If you publish results using analysis performed by Kinalyzer please acknowledge it by the following citation:\n T.Y. Berger-Wolf, S.I. Sheikh, B. DasGupta, M.V. Ashley, I.C. Caballero, W. Chaovalitwongse, S.L. Putrevu, 'Reconstructing Sibling Relationships in Wild Populations', Bioinformatics(2007), 23(13)\n\n";

					if ($result){
						$content = $content."Output:\n".$result;
					}
					else{
						$content = $content."Output:\n Unfortunately, we encountered an error in computing output for your job $id submitted at $time, for $loci loci and $population individuals.\n\n Kinalyzer has been notified. Please follow up with kinalyzer\@cs.uic.edu .\n\n";
					}
		                 	$from = "From: Kinalyzer <kinalyzer\@pachy.cs.uic.edu>\n";
		                       	$subject = "Subject: Kinalyzer output for $user, request id= $id\n";

					#Send mail	
		                     	open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
                      			print SENDMAIL $from;
		                        print SENDMAIL $reply_to;
                       			print SENDMAIL $subject;
		                        print SENDMAIL $send_to;
		                        print SENDMAIL "Content-type: text/plain\n\n";
		                        print SENDMAIL $content;
		                        close(SENDMAIL);
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
                $bcc = "Bcc: tanyabw\@uic.edu\n";
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

