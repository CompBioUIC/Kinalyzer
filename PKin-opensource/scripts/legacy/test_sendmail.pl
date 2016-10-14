#!/usr/bin/perl

$email="perezrat\@uic.edu";

$content="TESTING SENDMAIL FROM RUTGERS!\n";

# Generate e-mail
$sendmail="/usr/sbin/sendmail -t";
$reply_to="Reply-to: perezrat\@uic.edu\n";
$send_to="To: ".$email."\n";
$cc_to="Cc: perezrat\@uic.edu\n";
$subject="Subject: Kinalyzer output for TEST, request id=TEST\n";
open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
print SENDMAIL $reply_to;
print SENDMAIL $subject;
print SENDMAIL $send_to;
print SENDMAIL $cc_to;
print SENDMAIL "Content-type: text/plain\n\n";
print SENDMAIL $content;
close(SENDMAIL);
