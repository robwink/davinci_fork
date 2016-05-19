#!/usr/bin/perl -w

$dv_team_addr="Davinci team <davinci\@mars.asu.edu>";

# initially output is disabled
$output=0;

while($line=<>){
	# stop output when the end of comment is hit in version.h
	if ($line =~ m/\*\//){
		$output=0;
	}

	if ($output){
		# when a version line is hit output "package (version) distribution; urgency=urgency"
		# and save the version date to be output at the end of the block
		if ($line =~ m/Version +([^: ]*):? +(.*)/){
			print "davinci ($1) debian; urgency=standard\n";
			$date=$2;
		}
		else {
			# when first blank line is hit after seeing a version entry, output
			# the "-- maintener name <email>[two spaces] date"
			if ($line =~ m/^\s*$/ && defined($date)){
				$out_date=`date -d\"$date\" -R`;
				print " -- $dv_team_addr  $out_date\n";
			}
			print $line;
		}
	}

	# start output when the start of comment is hit in version.h
	if ($line =~ m/\/\*/){
		$output = 1;
	}
}

# handle any lingering dates at the end of output 
if (defined($date)){
	$out_date=`date -d\"$date\" -R`;
	print " -- $dv_team_addr  $out_date\n";
}

