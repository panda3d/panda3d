#!perl

# generate address base file for panda dlls
# to simplify things, do it for Opt1 and reuse that for Opt2, 
# and do it for Opt3 and use that for Opt4, since they will both be bigger than counterparts
# to run, in any directory, do 'perl basedlls.pl' and it will generate dllbase.txt and dllbase_d.txt in 'tmp' dir under current
# make sure Opt1 and Opt3 are current so current sizes are used in map

$archive_root="\\\\dimbo\\panda\\win\\";
#edit this to where rebase.exe is on your machine, must be cygwin-style path for cygwin perl
$rebase_path="/msvc7/Common7/Tools/bin";
$ENV{'PATH'}=$rebase_path.":".$ENV{'PATH'};

# note: this is the order dlls will be laid out in memory
@dll_names=("libpanda","libwindisplay","libpandadx8","libpandadx7","libpandagl","libpandaexpress","libpandaphysics","libpandafx","libdtool","libdtoolconfig","libpystub","libdirect","libtoontown","libmiles_audio","libpandaegg","libframework");

# old audio dlls
#            "libaudio_load_midi","libaudio_load_mp3","libaudio_load_st","libaudio_load_wav",

@headerstrs=(
"This file is used by the MSVC linker to specify DLL base addresses to",
"prevent runtime relocation of DLLs by the windows loader.  This speeds",
"loading.  This file was generated using \$PANDATOOL/src/scripts/basedlls.pl",
"which uses rebase.exe from the MS platform SDK.  It may additionally be",
"hand-hacked to simplify addresses for popular dlls to aid debugging.",
);

$headerstr="";
for($i=0;$i<=$#headerstrs;$i++) {
   $headerstr.="; ".$headerstrs[$i]."\n";
}

@modules=("dtool","panda","direct","toontown");

$dll_names_nodbg="";
$dll_names_dbg="";

foreach my $nm (@dll_names) {
   $dll_names_nodbg.=" ".$nm.".dll";
   $dll_names_dbg.=" ".$nm."_d.dll";
}

# 0x60000000 is the recommended base address for user dll addr space
$baseaddr="0x60000000";
$dllbasefile="dllbase.txt";
$basedlllogfilename="log.txt";

# rebase will modify input dlls, so make tmp copies of them
if(!(-d "tmp")) {
   system("mkdir tmp");
}
chdir("tmp");
unlink($basedlllogfilename);

############################
### do release

$archivepath=$archive_root."release";

# need to remove existing files or rebase will just append to them
open(OUTFILE, ">$dllbasefile") || die "Couldn't open $dllbasefile!\n";
print OUTFILE $headerstr;
print OUTFILE "\n; release dlls\n\n";
close(OUTFILE);

foreach my $dir1 (@modules) {    
   my @args=("cmd.exe","/c","copy ".$archivepath."\\$dir1\\lib\\*.dll .");
   system(@args);
}

$argstr="-v -b $baseaddr -c $dllbasefile -l $basedlllogfilename $dll_names_nodbg";

#@args=("cmd.exe","/C","rebase.exe $argstr");
#print $args[0]." ".$args[1]." ".$args[2]."\n";
@args=("rebase.exe $argstr");
print $args[0]."\n";
system(@args);

############################
### do debug

open(OUTFILE, ">>$dllbasefile") || die "Couldn't open $dllbasefile!\n";
print OUTFILE "\n; debug dlls\n\n";
close(OUTFILE);

$archivepath=$archive_root."debug";
foreach my $dir1 (@modules) {    
   my @args=("cmd.exe","/c","copy ".$archivepath."\\$dir1\\lib\\*.dll .");
   system(@args);
}

$argstr="-v -b $baseaddr -c $dllbasefile -l $basedlllogfilename $dll_names_dbg";
#@args=("cmd.exe","/C","rebase.exe $argstr");
#print $args[0]." ".$args[1]." ".$args[2]."\n";
@args=("rebase.exe $argstr");
print $args[0]."\n";
system(@args);

#cygwin d2u seems to not work rite
$d2u_path=$ENV{'HOME'}."/player/wintools/bin";
$ENV{'PATH'}=$d2u_path.":".$ENV{'PATH'};
print $ENV{'PATH'},"\n";
@args=("dos2unix",$dllbasefile);
print $args[0]." ".$args[1]." ".$args[2]."\n";
system(@args);

print "do cp tmp/dllbase.txt ~/player/dtool/src/dtoolbase\n";

exit(0);


