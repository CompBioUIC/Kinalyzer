#!/usr/bin/perl

# Routine(s) for generating and mailing the final report

use strict;

# Allow database connectivity
use DBI;

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
	print "Connected to database.\n";
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

my %emailsHash = ();

# Generates and mails the final report
# param data_dir - path to uploaded database files
# param finished_dir - path to finished data files
# param job_input_path - path to uploaded job input data from database
# param job_intermediate_dir - directory of intermediate files of completed job
# param job_final_output_path - path to final output of PKin, GAMS, and scripts
# param job_record_attribs_array - array storing attributes of current job record from database
sub sendEmail
{	
	my $email = shift;
	my $content = "Hello,\n\nYou are receiving this e-mail because you have submitted a Kinalyzer request within the past few weeks.  To accomodate for possible lost Kinalyzer results during the recent network outage, we are re-running all jobs that have been submitted during this period.  Please ignore these results if you have already received them!\n\nThank you,\n-The Kinalyzer Team\n";

	# Generate e-mail
	my $sendmail="/usr/sbin/sendmail -t";
	my $reply_to = "Reply-to:tanyabw\@uic.edu\n";
	my $cc_to = "Cc: perezrat\@uic.edu\n";
	my $send_to = "To:".$email."\n";
	my $subject="Subject: Kinalyzer is back up!\n";
	open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
	print SENDMAIL $reply_to;
	print SENDMAIL $subject;
	print SENDMAIL $send_to;
	print SENDMAIL $cc_to;
	print SENDMAIL "Content-type: text/plain\n\n";
	print SENDMAIL $content;
	close(SENDMAIL);
}			

sub doWork
{
	# Connect to database
	pkDbConnect();

	pkDbInitQuery( 2 ); # 2 is base value

	while ( pkDbGetNextRec() )
	{
		my ( $name, $email, $loci, $algo, $status, $id, $count ) = pkDbGetRecAttribsAsArray();
		if ( $id >= 1550 )
		{
			print "The current request is --- $id of $name\n";
			print "Processing tuple(name=$name,email=$email,loci=$loci,id=$id,algo=$algo,count=$count)\n";
			if ( not exists $emailsHash{ $email } )
			{
				$emailsHash{ $email } = $email;
			}
			pkDbUpdateStatus( $id, 1 );
		}
	}

	$emailsHash{ "tanyabw\@uic.edu" } = "tanyabw\@uic.edu";
	foreach my $emailKey ( keys %emailsHash )
	{
		print "Found e-mail = $emailKey\n";
		sendEmail( $emailKey );
	}
}

doWork();

