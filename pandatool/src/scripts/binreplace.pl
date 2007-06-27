#!perl
if($#ARGV<2) {
    print "Usage: perl binreplace.pl [-i] [findstr] [replacestr] [filename]\n";
    exit(1);
}

$argnum=0;
$case_insensitive = 0;
if($ARGV[0] eq "-i") {
  $argnum=1;
  $case_insensitive = 1;
}
$findstr =  $ARGV[$argnum];   $argnum++;
$replacestr = $ARGV[$argnum]; $argnum++;
$filename = $ARGV[$argnum];   $argnum++;

$filenamebak = $filename.".bak";
$retval = rename($filename,$filenamebak);
if(! $retval) {
   die "rename(".$filename.",".$filenamebak.") failed!\n";
}

print "replacing '".$findstr."' with '".$replacestr."' in ".$filename."\n";

# for some reason, you need to double the $findstr \'s but not the replacestr ones

$findstr =~ s/\\/\\\\/g;
#$replacestr =~ s/\\/\\\\/g;

open IN, $filenamebak or die "can't read ".$filenamebak." $!";
open OUT, ">$filename" or die "can't create ".$filename.": $!";
binmode IN;
binmode OUT;

if($case_insensitive) {
  s/$findstr/$replacestr/gi, print OUT while <IN>;
} else {
  s/$findstr/$replacestr/g, print OUT while <IN>;
}
close OUT;
close IN;
unlink($filenamebak);
