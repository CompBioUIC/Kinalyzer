#! /usr/bin/perl 
#   extract-solution-param.pl
#
#   Tanya Berger-Wolf
#   DIMACS
#   January 2005
#
#   Take the solution output file *.lst, extract the solution numbers from it
#   and write the corresponding sets into a sol_* file. Do that for every
#   solution file in the given directory
#
#   Usage:
#      extract-solution.pl
#        <dir>               Directory that contains solution files *.lst
#   Example: ./extract-solution.pl <dir>
#
#   No default values

##################### Global variables
$dir = "";                          # input directory
$input = "";                        # Input file  *.lst
$output = "";                       # Output file sol_*
$solutionfile = "";                 # File to contain solution line numbers *.sol

##################### Read command line arguments

if ($#ARGV == -1) {                 # Wrong number fo arguments
    print "Please specify and input argument \n";
    print "For usage, type extract-solution.pl -h \n";
    exit (1);
}

for ($i = 0; $i <= $#ARGV; $i++) {
    if ($ARGV[$i] eq "-h") {
	print "<directory>       Directory that contains solution files\n";
	exit(0);
    }
    else {
	$dir = $ARGV[$i];
    }
}

####################### Read solution directory


foreach $file (@ARGV) {
    $input = "$file";                      # Solution file *.lst
    print "Processing $input\n";
    $solutionfile = $input;                      # Solution numbers file *.sol
    $solutionfile =~ s/\.lst/\.sol/;
    $setfile = $input;                           # Set file set_*
    $setfile =~ s/\.lst//;
    $output = $setfile;                          # Output file sol_*
    $output = "$setfile".".soln";

    ######################### Read and print solution numbers

    open (IN, "< $input") || die "ABORTING! Cannot open $input input file!\n";
    
    @solution = ();
    while ($line = <IN>) 
    {
	if ($line =~ /^t\d+.*/) {                # Line starts "t<num> ..."
	    @temp = ($line =~ /t(\d+)/g);        # Read solution set numbers
	    push(@solution,@temp);
	    print "writing $solutionfile\n";
	    open (SOL, "> $solutionfile") || die "ABORTING! Cannot open $solutionfile output file!\n";
	    print SOL "@solution\n";
	    close(SOL);
	}
    }
    close(IN);

    ######################## Translate solution numbers to solution sets

    $cmd = "./write-solution.pl -i $setfile -s $solutionfile -o $output";
    print "Executing ".$cmd."\n";
    system($cmd);
}
