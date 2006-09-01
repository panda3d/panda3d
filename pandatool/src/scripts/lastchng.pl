#!perl

# find last checked-in modification to [filename]

if($#ARGV==-1) {
  die "Usage: lastchng.pl [filename]\nfinds last checked-in modification to file\n";
}

if(!(-e $ARGV[0])) {
  die "file '".$ARGV[0]."' doesnt exist!\n";
}

open(INPUT_STREAM,"cvs log $ARGV[0] |");

$firstrev=null;
$secondrev=null;

while($line=<INPUT_STREAM>) {
  if($line =~ /^revision (\S*)$/) {
#     print $line."\n";
     if($firstrev==null) {
         $firstrev=$1;
     } else {
         $secondrev=$1;
         last;
     }
  }
}
close(INPUT_STREAM);

if($firstrev==null) {
  die "Couldn't find first revision of $ARGV[0]!\n";
}
$revstr="-r$firstrev";
if($secondrev!=null) {
$revstr = "-r$secondrev ".$revstr;
}

$cvsline="cvs diff $revstr ".$ARGV[0];
print $cvsline,"\n";
system($cvsline);
exit(0);
