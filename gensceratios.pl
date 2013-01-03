#!/usr/bin/perl -w

$min_proc_num=$ARGV[0];
$max_proc_num=$ARGV[1];
#op_number will contain operations without first block all processes and last unlock bloced processes. Soo this can be in real $onumber+2*(RAND($min_proc_num,$max_proc_num)
$op_number=int($ARGV[2]);

$proc_num=int(rand($max_proc_num-$min_proc_num))+$min_proc_num;
@my_procs_state=(1 .. $proc_num);

open MYFILE, '>output.txt' or die $!;
print MYFILE "$proc_num\n";
for($i=1;$i<$proc_num+1;$i++) {
	$my_procs_state[$i]=1;
	print MYFILE "$i.1\n"
}

for($i=0;$i<$op_number;$i++) {
	$j=int(rand($proc_num))+1;
	if ($my_procs_state[$j] == 2) { $my_procs_state[$j]=1; 
	} else { $my_procs_state[$j]=2; }
	print MYFILE $j.".".$my_procs_state[$j]."\n";
}

for($i=1;$i<$proc_num+1;$i++) {
	if ($my_procs_state[$i]==1) {
		$my_procs_state[$i]=2;
		print MYFILE "$i.2\n"
	}
}
close (MYFILE)
