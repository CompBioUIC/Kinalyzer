#!/usr/bin/perl 
 
# Root directory
$depot_dir="/home/sibgroup/sibsdepot/";

# Parallel implementation binary paths
$parallel_binary_dir=$depot_dir."binaries/PKin/";
$parallel_binary_name="PKin-locali";

# Data paths
$data_dir=$depot_dir."data/test/";

# Output paths
$intermediate_dir=$depot_dir."intermediate/"; # /var/www/uploads_new/

# Currently, algorithm is always set to 2-allele
$algo="consensus";

# Force stdout to be flushed after every print call
$| = 1;

# Maximum redundancy count
$max_redundancy_count = 4;

sub main() 
{ 
	# Open file that has input data and read into array
	$fileNames_loci_threads_list_path=$data_dir."list_".$algo.".txt";
	status("Opening $fileNames_loci_threads_list_path");
	open(DAT, $fileNames_loci_threads_list_path) || die("Could not open $fileNames_loci_threads_list_path!");
	@fileNames_loci_threads_list=<DAT>;
	close(DAT); 

	# Fix number of logical threads at 24
	my $pkin_arg_num_logical_threads_loci_intersection=24;


	# Loop through array 
	foreach $fileName_loci_threads_tuple (@fileNames_loci_threads_list)
	{
		# Read file name and loci
		chomp($fileName_loci_threads_tuple);
		($file_name,$loci,$threads)=split(/\|/,$fileName_loci_threads_tuple);
		status("Processing file=$file_name with loci=$loci and threads=$threads)");

		my $pkin_input_data_path=$data_dir.$file_name;
		my $intermediate_base_file_name=$file_name."_sets";
		my $pkin_output_data_path=$intermediate_dir.$intermediate_base_file_name;
		my $pkin_arg_num_logical_threads_local_loci_reconstruction=$loci;
		my $pkin_arg_stacksizekb=1024;
		my $pkin_arg_recon_algo=1; # centralized=0, island=1, work stealing=2
		my $pkin_arg_num_loci_clusters=$loci;
		my $pkin_arg_num_threads=$threads;
		
		# Run consensus algorithm
		for ($redundancy_count = 1; $redundancy_count <= $max_redundancy_count; $redundancy_count++)
		{
			my $pkin_cmd_line="./$parallel_binary_name $algo $pkin_input_data_path $pkin_arg_num_threads $pkin_arg_num_logical_threads_local_loci_reconstruction $pkin_arg_num_logical_threads_loci_intersection $pkin_arg_stacksizekb $pkin_arg_recon_algo $pkin_arg_num_loci_clusters $pkin_output_data_path";
			my @pkin_cmd_line_array=("./$parallel_binary_name",$algo,$pkin_input_data_path,$pkin_arg_num_threads,$pkin_arg_num_logical_threads_local_loci_reconstruction,$pkin_arg_num_logical_threads_loci_intersection,$pkin_arg_stacksizekb,$pkin_arg_recon_algo,$pkin_arg_num_loci_clusters,$pkin_output_data_path);
			chdir $parallel_binary_dir;
			status("Running this --> $pkin_cmd_line");
			system(@pkin_cmd_line_array);
		}
	} #end of foreach
} 

main();   

sub status
{ 	
	my $tm = scalar localtime; 
	print "$tm @_\n"; 
}  
