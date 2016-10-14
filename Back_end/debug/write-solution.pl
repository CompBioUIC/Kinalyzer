#! /usr/bin/perl
#   
#   write-solution.pl
#
#   Tanya Berger-Wolf
#   DIMACS
#   December 2004
#
#   Create 4-allele sets and assign the animals to them
#
#   Usage:
#      write-solution.pl
#        -i <input file>      File that contains animals data
#        -o <output file>     File that will contain sets
#        -s <solution file>   Solution file
#        -a <animal file>     Animal names files
#   Example: ./write-solution.pl -i infile -o outfile -s solution
#
#   No default values

##################### Global variables
$input = "";                        # Input file
$output = "";                       # Output file
$solutionfile = "";                 # Solution file
$animalfile = "";                   # Animal name file
@solution = ();                     # Solution
#@animal = ();                       # Animal


##################### Read command line arguments

if ($#ARGV eq -1) {                 # No command line arguments
    print "Please specify the -c <type> option \n";
    print "For usage, type write-solution.pl -h \n";
    exit (1);
}

for ($i = 0; $i <= $#ARGV; $i++) {
    if ($ARGV[$i] eq "-h") {
	print "-i <input file>        File containing set data \n";
	print "-o <output file>       File to contain solution sets \n";
	print "-s <solution file>     File containing solution numbers\n";
	print "-a <animal file>       File containing animal names\n";
	exit(0);
    }
    elsif (($#ARGV < 5) || ($#ARGV > 7)) {
	print "Wrong number of input arguments \n";
	print "For usage, type write-solution.pl -h \n";
	exit(1);
    }
    elsif ($ARGV[$i] eq "-i") {
	$input = $ARGV[++$i];
    }
    elsif ($ARGV[$i] eq "-o") {
	$output = $ARGV[++$i];
    }
    elsif ($ARGV[$i] eq "-s") {
	$solutionfile = $ARGV[++$i];
    }
    elsif ($ARGV[$i] eq "-a") {
	$animalfile = $ARGV[++$i];
    }
    else {
	print "Invalid argument $ARGV[$i]\n";
	print "For usage, type write-solution.pl -h \n";
	exit (1);
    }
}

######################### Read solution
print "Reading solution file\n";

open (IN, "< $solutionfile") || die "ABORTING! Cannot open $solutionfile input file!\n";

while ($line = <IN>) {
    @temp = ($line =~ /(\d+)\s*/g);                     # Read solution set numbers
    @solution = (@solution, @temp);
}
close(IN);

######################### Read animal names
print "Reading animal names\n";

if ($animalfile ne "") {
    open (IN, "< $animalfile") || die "ABORTING! Cannot open $animalfile input file!\n";

    while ($line = <IN>) {
	@temp = ($line =~ /(\S+)\s*/g);                 # Read animal names
	push(@animal, @temp);
    }
    close(IN);
}

######################## Print the sets in the solution
print "Printing solution\n";

open (IN, "< $input") || die "ABORTING! Cannot open $input set file!\n";
open (OUT, "> $output") || die "ABORTING! Cannot open $output output file!\n";

$i=1; $j=0;
while ($line = <IN>) {
    if ($i++ eq $solution[$j]) {                        # The set is in the solution
	($setname, $etc) = ($line =~ /(\S*)(\s+.*)/);
	print $line."\n";
	(@temp) = ($etc =~ /\s*(\d)\D*/g);
	@set=();
	for ($a=0; $a <= $#temp; $a++) {                # Find the animals in the set
	    if ($temp[$a] eq 1) {
		if ($#animal ne -1) {                    # Animal names are given
		    push(@set, $animal[$a]);
		}
		else {                                  # Animal names are not given
		    push(@set,$a);
		}
	    }
	}
	
	print  "$setname\t  @set\n";
	print OUT "$setname\t  @set\n";
	$j++;
    }
}
close(IN);
close(OUT);
