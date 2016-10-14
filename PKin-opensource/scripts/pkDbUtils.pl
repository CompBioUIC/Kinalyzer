#!/usr/bin/perl

# Routines for getting jobs from a database

use strict;

# Allow database connectivity
use DBI;

require "./pkLoggerUtils.pl";

# Offsets into record attributes array
use constant
{
	idxPkDbRecName => 0,
	idxPkDbRecEmail => 1,
	idxPkDbRecLoci => 2,
	idxPkDbRecAlgo => 3,
	idxPkDbRecStatus => 4,
	idxPkDbRecId => 5,
	idxPkDbRecCount => 6,
	idxPkDbNumFields => 7,
};

# closure to avoid global variables
{

# Handle for interfacing with database
my $pkDbHandle;
my $pkDbQueryHandle;

# Results for query records
my $pkDbRecName;
my $pkDbRecEmail;
my $pkDbRecLoci;
my $pkDbRecAlgo;
my $pkDbRecStatus;
my $pkDbRecId;
my $pkDbRecCount;

# Connect to database
sub pkDbConnect
{
	my $dsn = 'DBI:mysql:interface_data:localhost';
	my $db_username = 'www_user';
	my $db_password = 'sibs';
	$pkDbHandle = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ".DBI->errstr;
	status( "Connected to database." );
}

# Initialize a database query
# param queryStatus - the status of the job we are interested
sub pkDbInitQuery
{
	my $queryStatus = shift; # pop first argument off of arguments array
	$pkDbQueryHandle = $pkDbHandle->prepare("SELECT * FROM requests_new WHERE status=$queryStatus ORDER BY request_id ASC");
	$pkDbQueryHandle->execute;		
	$pkDbQueryHandle->bind_columns( \$pkDbRecName, \$pkDbRecEmail, \$pkDbRecLoci, \$pkDbRecAlgo, \$pkDbRecStatus, \$pkDbRecId, \$pkDbRecCount);
}

# Returns next record from query
sub pkDbGetNextRec
{
	return $pkDbQueryHandle->fetch();
}

# Updates the value of a field in the database
# param recordId - id of record to update status field for
# param fieldName - name of field to update
# param newValue - new value to update field with
sub pkDbUpdateFieldValue
{
	my $recordId = shift;
	my $fieldName = shift;
	my $newValue = shift;
	my $update = $pkDbHandle->prepare("UPDATE requests_new SET $fieldName=$newValue WHERE request_id=$recordId");
	$update->execute;
}

# Updates the status field in the database
# param recordId - id of record to update status field for
# param newStatus - new status value to update field with
sub pkDbUpdateStatus
{
	pkDbUpdateFieldValue( $_[0], "status", $_[1] );	
}

# Updates the count field in the database
# param recordId - id of record to update status field for
# param newCount - new status value to update field with
sub pkDbUpdateCount
{
	pkDbUpdateFieldValue( $_[0], "count", $_[1] );
}

# Accessors
sub pkDbGetRecName
{
	return $pkDbRecName;
}

sub pkDbGetRecEmail
{
	return $pkDbRecEmail;
}

sub pkDbGetRecLoci
{
	return $pkDbRecLoci
}

sub pkDbGetRecAlgo
{
	return $pkDbRecAlgo;
}

sub pkDbGetRecStatus
{
	return $pkDbRecStatus;
}

sub pkDbGetRecId
{
	return $pkDbRecId;
}

sub pkDbGetRecCount
{
	return $pkDbRecCount;
}

sub pkDbGetRecAttribsAsArray
{
	return ($pkDbRecName, $pkDbRecEmail, $pkDbRecLoci, $pkDbRecAlgo, $pkDbRecStatus, $pkDbRecId, $pkDbRecCount);
}

# In order to indicate module loading was successful, must return 1
# http://lists.netisland.net/archives/phlpm/phlpm-2001/msg00426.html
1;

# end of closure
}

# Everything after this line will be ignored by the compiler
__END__

