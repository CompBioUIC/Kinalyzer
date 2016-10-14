#!/usr/bin/perl
#
#commands to produce .gms, .lst, .soln

#print header();
print "Content-type: text/html\n\n";

use CGI;
$query2 = new CGI;
$nloci = $query2->param("nloci");
$email = $query2->param("email");
$username = $query2->param("name");




### Running the algo
#system("./sets","test",$nloci,"../uploads/newinput","output");
#system("/opt/lx3gams/gams output.gms");
#system("perl /var/www/newscripts/extract-solution-param.pl output.lst");
#system("perl /var/www/newscripts/write-solution.pl -i output -s output.sol -o output.soln");
#system("cp output.soln ../uploads/output.soln.txt");
###

$q = new CGI;
#print $q->header();
#print "Content-type: text/html\n\n";
print<<END_HTML;
<HTML>
<BODY>
<P>$username, thanks for using our algorithm. The output will be emailed to you.</P>

</BODY>
</HTML>
END_HTML

### Mailing the output
#$sendmail = "/usr/sbin/sendmail -t";
#$reply_to = "Reply-to: wandathefish\@gmail.com\n";
#$send_to = "To: ".$email."\n";
#$content = "Here is the Output --\n\n";
#$outputfile = "/var/www/uploads/output.soln.txt";
#$subject = "Subject: Output file\n";
###
#open(SENDMAIL, "|$sendmail") or die "Cannot open $sendmail: $!";
#print SENDMAIL $reply_to;
#print SENDMAIL $subject;
#print SENDMAIL $send_to;
#print SENDMAIL "Content-type: text/plain\n\n";
#print SENDMAIL $content;
#open(FILE, "$outputfile") or die "Cannot open $outputfile: $!";
#print SENDMAIL <FILE>;
#close (FILE);
#close(SENDMAIL);
###
