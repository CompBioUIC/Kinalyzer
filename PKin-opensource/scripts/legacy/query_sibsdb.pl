#!/usr/bin/perl

#--------------------------------------------#
#  PROGRAM:  runs a sql query on the sibs db #
#--------------------------------------------#

use DBI;

# Determine number of arguments
$num_args = $#ARGV + 1;
if ( $num_args != 1 )
{
	status("Received $num_args commandline arguments.  Expect exactly 1 commandline argument");
	print_usage();
	exit(0);
}

# Get job id from commandline
$job_id_arg=$ARGV[0];
$sql_query="SELECT * FROM requests_new WHERE request_id=$job_id_arg ORDER BY request_id ASC";
print "Running:\n$sql_query";


# Connect to database
$dsn = 'DBI:mysql:interface_data:localhost';
$db_username = 'www_user';
$db_password = 'sibs';
$dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;

print "Daemon Started.\n";
$|=1;

# Query database for specific job id
$get_requests_new=$dbh->prepare($sql_query);
$get_requests_new->execute;		
$get_requests_new->bind_columns( \$name, \$email, \$loci, \$algo, \$status, \$id, \$count);

# Fetch job information
if ($get_requests_new->fetch())
{
	print "Found job with name($name), email($email), loci($loci), algo($algo), status($status), id($id), count($count)\n";
}
else
{
	print "Job-id $job_id_arg not found!\n";
}
