#!/usr/bin/perl
#
#commands to produce .gms, .lst, .soln

#print header();
print "Content-type: text/html\n\n";



system("./sets","test",$nloci,"../uploads/input","output");

system("/opt/lx3gams/gams output.gms");

system("perl /var/www/newscripts/extract-solution-param.pl output.lst");

system("perl /var/www/newscripts/write-solution.pl -i output -s output.sol -o output.soln");

system("cp output.soln ../uploads/output.soln.txt");



