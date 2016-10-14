#!/usr/bin/perl

use DBI;

$home_dir="/home/ramji/";
$log_file_dir=$home_dir;
$log_file_name="log_file_transfer.txt";
$local_data_dir="/var/www/uploads_new/";
$target_data_dir=$home_dir."new_parallel_requests/";
$name_email_loci_id_map_algo_dir=$target_data_dir;
$name_email_loci_id_map_algo_file_name="name_email_loci_id_algo_map.txt";

$log_handle;

sub main()
{
	my $dsn = 'DBI:mysql:interface_data:localhost';
	my $db_username = 'www_user';
	my $db_password = 'sibs';
	my $dbh = DBI->connect($dsn, $db_username, $db_password) or die "Couldn't connect to database: ". DBI->errstr;

	my $log_file_path=$log_file_dir.$log_file_name;
	open $log_handle, ">>$log_file_path" or die;
	
	$name_email_loci_id_algo_map_path=$name_email_loci_id_map_algo_dir.$name_email_loci_id_map_algo_file_name;
	open(DAT,">>$name_email_loci_id_algo_map_path") || die("Cannot Open File");
	status("transfer job started.");
	$|=1;

	my $get_requests_new = $dbh->prepare("SELECT * FROM requests_new WHERE status=3 ORDER BY request_id DESC");
	$get_requests_new->execute;		
	$get_requests_new->bind_columns( \$name, \$email, \$loci, \$algo, \$status, \$id, \$count);
	status("Getting new requests");

	while($get_requests_new->fetch())
	{
		if ($algo=='2allele')
		{
			status("The current request is --- $id of $name");
			# No need to update count in table, already updated before status=3 is set                       	
			# write relevant database info about this job to a plain-text file		
            print DAT "$name\|$email\|$loci\|$id\|$algo\n";
			 
			#copy new requests to new_requests subdirectory
			my $local_orig_data_file_path=$local_data_dir.$id;
			my $local_new_data_file_name=$id."_new";
			my $local_new_data_file_path=$local_data_dir.$local_new_data_file_name;
			system("cp $local_orig_data_file_path $target_data_dir");
            system("cp $local_new_data_file_path $target_data_dir");
		}
	}

	sleep(10);
	close $log_handle;
}

main();

sub status 
{
	my $tm = scalar localtime;
	print $log_handle "$tm @_\n";
	print "$tm @_\n";
}
