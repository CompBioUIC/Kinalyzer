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
	if ($dbh && $#ARGV==1){

		open FILE, "$ARGV[0]" or die "Couldn't open file: $!"; 
		$upload = join("", <FILE>); 
		close FILE;
	
		#Insert request into the database
		$stmt = "UPDATE requests SET upload = \"$upload\" WHERE id=$ARGV[1]";
		print $stmt;
		print "\n";
		my $update_handle = $dbh->prepare($stmt);
		$update_handle->execute();
	}
}
main();

