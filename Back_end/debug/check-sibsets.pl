#! /usr/bin/perl
#   
#   check-sibsets.pl
#
#   Marco Maggioni
#   May 2004
#
#   Check 2-allele sets correctness
#
#   Usage:
#      check-sibsets.pl
#        -i <input file>    File that contains alleles for each individual
#        -s <set file>      File that contains sibsets to check
#
#   Example: ./check-sibsets.pl -i data -s sibsets
#
#   No default values

##################### Global variables
$input = "";                        # Input file
$input_sibsets = "";                # Sibsets file
%population;                        # Hash of individuals with alleles
$loci;                              # Number of loci
%sibsets ;                          # Hash of sibsets
##################### Functions
sub check_individual(\@)
{
   #Get array parameter
   my @temp = @{$_[0]};

   #Check if alleles are just numbers
   for ($i = 1; $i<=$#temp; ++$i){
      if (($temp[$i] =~ /(^\D+)/) && ($temp[$i] !~ /-1/)) {
         return 0;
      }
   }

   #If all them are number, then it a line corresponding to an individual
   return 1;
}

sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

##################### Read command line arguments

if ($#ARGV eq -1) {                 # No command line arguments
    print "Please specify the -c <type> option \n";
    print "For usage, type check-sibsets.pl -h \n";
    exit (1);
}

for ($i = 0; $i <= $#ARGV; ++$i) {
    if ($ARGV[$i] eq "-h") {
	print "-i <input file>        File that contains alleles for each individual \n";
	print "-s <output file>       File that contains sibsets to check \n";
	exit(0);
    }
    elsif ($#ARGV != 3) {
	print "Wrong number of input arguments \n";
	print "For usage, type check-sibsets.pl -h \n";
	exit(1);
    }
    elsif ($ARGV[$i] eq "-i") {
	$input = $ARGV[++$i];
    }
    elsif ($ARGV[$i] eq "-s") {
	$input_sibsets = $ARGV[++$i];
    }
    else {
	print "Invalid argument $ARGV[$i]\n";
	print "For usage, type check-sibsets.pl -h \n";
	exit (1);
    }
}

######################### Read dataset

%A = ();

print "Reading dataset file...\n";
open (IN, "< $input") || die "ABORTING! Cannot open $input input file!\n";
while ($line = <IN>) {
    chomp($line);
    @temp = split(",",$line); 
    if (check_individual(@temp)){ 
       $temp[0]=trim($temp[0]);
       $A{$temp[0]} = ();
       print $temp[0]." ";
       $population{$temp[0]} = ();
       for ($i=1; $i<@temp; ++$i){
          $population{$temp[0]}[$i-1]=$temp[$i];
       }
       $loci = (@temp-1)/2;
     }
}
close(IN);
print "\n\nPopulation : ".keys(%population)."\n";
print "Loci : ".$loci."\n";
print "How many : ".keys(%A)."\n";
######################### Read sibsets
%B = (); 
print "Reading sibsets file...\n";
open (IN, "< $input_sibsets") || die "ABORTING! Cannot open $input input file!\n";
while ($line = <IN>) {
    chomp($line);
    @temp = split(":",$line);
    @temp2 = split(",",$temp[1]);
    $sibsets{$temp[0]} = ();
    foreach $individual (@temp2){
        $individual=trim($individual);
        #print $individual." ";
        $B{$individual}= ();
        $sibsets{$temp[0]}{$individual} = ();
    }
    print $temp[0]." : [".keys(%{$sibsets{$temp[0]}})."]";
    foreach $key (keys(%{$sibsets{$temp[0]}})){
	print " ".$key;
    }
    print "\n";
}
close(IN);
print "How many : ".keys(%B)."\n";

%C = ();
foreach $item (keys(%A)){
    $C{$item}= ();
    print $item." ";
}
print "\n\n";
foreach $item (keys(%B)){
    $C{$item}= ();
    print $item." ";
}
print "How many : ".keys(%C)."\n";

######################### Check two allele rule for each sibset
my $error = 0;
foreach $sibset (keys(%sibsets)){
    for ($locus=0; $locus<$loci; ++$locus){
        my %a = ();
        my %R = ();
        foreach $individual (keys(%{$sibsets{$sibset}})){
            if ($population{$individual}[2*$locus] != -1){
	    	$a{$population{$individual}[2*$locus]} = ();
            }
	    if ($population{$individual}[2*$locus+1] != -1){
	    	$a{$population{$individual}[2*$locus+1]} = ();
            }
	    if ($population{$individual}[2*$locus]==$population{$individual}[2*$locus+1] && $population{$individual}[2*$locus] != -1){
		$R{$population{$individual}[2*$locus]} = ();
	    }
	}
        if (keys(%a)+keys(%R)>4){
            print "Error in ".$sibset." at locus ".$locus." (a=".keys(%a).",R=".keys(%R).")\n";
	    foreach $item (keys(%a)){ 
		print $item." ";
	    }
	    print "\n";
	    foreach $item (keys(%R)){
                print $item." ";
            }
            print "\n";
	    $error=1;
        } 
 }
}

if ($error){
    print "The sibsets do not respect the 2-allele rule.\n";
    exit(1);
}
else{
    print "The sibsets respect the 2-allele rule.\n";
    exit(0);
}

