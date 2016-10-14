#!/usr/bin/perl

#----------------------------------------#
#  PROGRAM:  runs a single serial job    #
#----------------------------------------#

use DBI;

# Paths and file names
$home_dir="/home/ramji/";
$data_dir=$home_dir."finished_8_13_2010/";

# Function to output to log file
sub status
{
	my $tm = scalar localtime;
	print "$tm @_\n";
}

# Connect to database
$dsn = 'DBI:mysql:interface_data:localhost';
$db_username = 'www_user';
$db_password = 'sibs';
$dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;
status("Daemon Started.");
$|=1;

# Iterate over finished files in data dir
opendir(DIRHANDLE, $data_dir) or die "Can't open $data_dir: $!";
while( defined ($file = readdir DIRHANDLE) ) 
{
	next if $file =~ /^\.\.?$/;     # skip . and ..
	status("Processing file: $file");
	chomp($file);
	($id_from_file_name,$unused)=split(/\_/,$file);
	status("Extracted job id $id_from_file_name from file name $file");
	
	# Query database for specific job id
	$get_requests_new = $dbh->prepare("SELECT * FROM requests_new WHERE request_id=$id_from_file_name ORDER BY request_id ASC ");
	$get_requests_new->execute;		
	$get_requests_new->bind_columns( \$name, \$email, \$loci, \$algo, \$status, \$id, \$count);
	
	# Fetch job information
	if ($get_requests_new->fetch())
	{
		# Read in e-mail body
		$content="";
		open(IN, $data_dir.$file) or die "Can't open original file : $!";
		while(<IN>) 
		{
			$content=$content.$_."\n";
		}
		close IN;
		status( "Loaded email body from file ".$data_dir.$file );
	
		# Generate e-mail
		$sendmail="/usr/sbin/sendmail -t";
		$reply_to="Reply-to: tanyabw\@uic.edu\n";
		$send_to="To: ".$email."\n";
		$cc_to="Cc: perezrat\@uic.edu\n";
		$subject="Subject: Kinalyzer output for $name, request id=$id\n";
		open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
		print SENDMAIL $reply_to;
		print SENDMAIL $subject;
		print SENDMAIL $send_to;
		print SENDMAIL $cc_to;
		print SENDMAIL "Content-type: text/plain\n\n";
		print SENDMAIL $content;
		close(SENDMAIL);
		
		# Update database status
		my $update = $dbh->prepare("UPDATE requests_new SET status=4 WHERE request_id= $id");
		$update->execute;
	}
}
closedir(DIRHANDLE);
