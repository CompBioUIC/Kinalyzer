#! /usr/bin/perl 
#   w2csv.pl
#
#   Alan Perez-Rathke
#   November 2010
#
#   Converts white space separated values to comma separated values (csv)
#
#   Usage:
#      w2csv.pl <input file> <output file>

##################### Read command line arguments

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

$numArgs = $#ARGV + 1;

if ($numArgs != 2) { # Wrong number of arguments
	print "Please specify two input arguments:\n\tw2csv.pl <input file> <output file>\n";
	exit (1);
}

$input = $ARGV[0]; # Input file
$output = $ARGV[1]; # Output file

open (IN, "< $input") || die "ABORTING! Cannot open $input!\n";
open (OUT, "> $output") || die "ABORTING! Cannot open $output!\n";

print "Successfully opened $input for reading.\nSuccessfully opened $output for writing.\n";

while ($line = <IN>)
{
	$line = trim($line);
	$line =~ s/\s+/, /g;
	print "$line\n";
	print OUT "$line\n"; 
}

print "Finished!\n";

close(OUT);
close(IN);
