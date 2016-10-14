#!/usr/bin/perl

#-----------------------------------------------------------------------------------#
#  PROGRAM:  Utility to send an e-mail once a day so we know machine is still alive #
#-----------------------------------------------------------------------------------#

$twenty_four_hours=3600*24;
$send_to="To: perezrat\@uic.edu\n";
$sendmail="/usr/sbin/sendmail -t";

while( true ) 
{
	$time_stamp=localtime();
	$subject="Subject: Kinalyzer heartbeat $time_stamp\n";
	$content="Kinalyzer is still running at this time: $time_stamp!  Have a good day!\n\nKinalyzer";

	# Generate e-mail
	open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
	print SENDMAIL $subject;
	print SENDMAIL $send_to;
	print SENDMAIL "Content-type: text/plain\n\n";
	print SENDMAIL $content;
	close(SENDMAIL);

	sleep( $twenty_four_hours );
}

