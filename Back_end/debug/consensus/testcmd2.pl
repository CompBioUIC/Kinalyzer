#!/usr/bin/perl
#
#commands to produce .gms, .lst, .soln

#print header();
print "Content-type: text/html\n\n";

use CGI;
$query2 = new CGI;
$nloci = $query2->param("nloci");
$nloci="$nloci";

system("./sets","test",$nloci,"../uploads/input","output");

system("/opt/lx3gams/gams output.gms");

#system("perl extract-solution-param.pl output.lst");
open(SOLN, "|perl extract-solution-param.pl /var/www/consensus/output.lst");
close(SOLN);

system("cp output.sol ../uploads/output.sol.txt");


$q = new CGI;
#print $q->header();
#print "Content-type: text/html\n\n";
print<<END_HTML;
<HTML>
<BODY>
<P>Run Successful! </P>
<P> Here is the <a href="/uploads/output.sol.txt"> Output file </P>
</BODY>
</HTML>
END_HTML


#  system("chmod 755 output.sol ");
