#!perl

#NOTE:  this script assumes you are running the Cygwin perl, which uses the
#       Cygwin file paths (i.e. '/' corresponds to 'C:\cygwin')

$WIN_INSTALLDIR="\\\\nufat\\mass\\pandabuilds\\win";

#$WIN_INSTALLDIR="\\\\cxgeorge-d01\\c\\pandabuilds\\win";

#$DEBUG_TREECOPY = 1;

$DEBUG_GENERATE_PYTHON_CODE_ONLY = 0;
$DONT_ARCHIVE_OLD_BUILDS = 0;


$BLD_DTOOL_ONLY=0;
$DIRPATH_SEPARATOR=':';   # set to ';' for non-cygwin NT perl

$ENV{'PANDA_OPTIMIZE'}='1';  # var has meaning to my special Config.pp
$ENV{'PPREMAKE_CONFIG'} = '/usr/local/etc/Config.pp';
$ENV{'TCSH_NO_CSHRC_CHDIR'}='1';

$ENV{'HOME'}="/home/builder";
$ENV{'USER'}="builder";
$ENV{'USERNAME'}=$ENV{'USER'};


$DONT_ARCHIVE_OLD_BUILDS = (($ENV{'DONT_ARCHIVE_OLD_BUILDS'} ne "") || $DONT_ARCHIVE_OLD_BUILDS);

sub logmsg() {
   my $msg = shift;
   print $msg."\n";
   open(LOGFILE,">>".$fulllogfilename) || die "can't open log file '$fulllogfilename'\n";
   print LOGFILE $msg."\n";
   close(LOGFILE);
}

#could change this to use eval, but it would require doubling the '\''s again in filename
sub mychdir() {
  my $newdir = shift;
  my $retval = chdir($newdir);
  &logmsg("chdir(".$newdir.")");
  if(! $retval) {
      &logmsg("chdir(".$newdir.") failed!!");
      exit(1);
  }
}

sub mymkdir() {
    my $newdir=shift;
    if(!(-e $newdir)) {
      if(!mkdir($newdir,0xfff)) {
       &logmsg("cant create new dir '".$newdir."' !!");
       exit(1);
      }
    }
}

sub myrename() {
  my $n1 = shift;
  my $n2 = shift;
  my $retval;

  &logmsg("rename(".$n1.",".$n2.")");

  if(-e $n2) {

      # find name we can move old target to
      my $newnum=1;
      while(-e $n2.".old.".$newnum) {
          $newnum++;
      }

      $newconflicttargetname=$n2.".old.".$newnum;
      &logmsg("avoiding rename conflict, renaming old ".$n2." to ".$newconflicttargetname);  
      $retval = rename($n2,$newconflicttargetname);
      if(! $retval) {
          &logmsg("rename failing, giving up (check if files or dirs are open in other apps)");
          exit(1);
      }
  }

  $retval = rename($n1,$n2);
  if(! $retval) {
      &logmsg("rename(".$n1.",".$n2.") failed!!!  (check if files or dirs are open in other apps)");
      exit(1);
  }
}

sub myexecstr() {
  my $cmdstr = shift;
  my $errstr = shift;
  my $dologstr = shift;
  my $exec_cshrc_type = shift;

  if($dologstr eq "DO_LOG") {
      if($exec_cshrc_type eq "NT cmd") {
          $cmdstr.=" >> ".$fulllogfilename_win." 2>&1";  # 2>&1 tells nt cmd.exe to redirect stderr to stdout
      } else {
          $cmdstr.=" >>& ".$fulllogfilename;   # for tcsh
      }

      &logmsg($cmdstr);
  }

  my $savedhome = $ENV{'HOME'};

  if( $exec_cshrc_type eq "NO_CSHRC") {
     # change $HOME so .cshrc doesn't get sources by tcsh
     $ENV{'HOME'}="/";  
  } elsif( $exec_cshrc_type eq "NO_PANDA_ATTACH") {
     $ENV{'TCSH_NO_PANDA_ATTACH'}="1";  
  }

  if($exec_cshrc_type eq "NT cmd") {
      my @args = ("cmd.exe", "/c", "$cmdstr");
      $retval = system(@args);
  } else {
      my @args = ("tcsh.exe", "-c", "$cmdstr");
      $retval = system(@args);
  }

  if($retval!=0) {
      $retval= $retval/256;  # actual retval
      if($errstr eq "") {
         &logmsg($cmdstr." failed!!!!!  continuing anyway...\nError return value=".$retval);
      } elsif($errstr ne "nomsg") {
         &logmsg($errstr."\nError return value=".$retval);
         exit($retval);
      }
  }

  if($exec_cshrc_type eq "NO_CSHRC") {
      $ENV{'HOME'}=$savedhome;
  } elsif( $exec_cshrc_type eq "NO_PANDA_ATTACH") {
     delete $ENV{'TCSH_NO_PANDA_ATTACH'};  
  }
}

sub gettimestr() {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday) = localtime(time);
    $mon++;
    return $mon."/".$mday." ".$hour.":".($min<10 ? "0":"").$min;
}

sub appendpath() {
# note cygwin perl.exe requires ':' instead of ';' as path dir separator
  foreach $dir1 (@_) {
      $ENV{'PATH'}.=$DIRPATH_SEPARATOR.$CYGBLDROOT.$dir1;
  }
}

sub make_bsc_file() {
    &logmsg("*** Generating panda.bsc at ".&gettimestr()." ***");
    
    &mychdir($CYGBLDROOT."/debug");
    $outputdir=$WINBLDROOT."\\debug";
    $outputname="panda.bsc";
    $outputfilepath=$outputdir."\\".$outputname;
    $cmdfilepath=$outputdir."\\makebsc.txt";
    
    #open(FILES, "where -r . *.sbr |") || die "Couldn't run 'where'!\n";
    #requires unix/cygwin style find.exe
    
    open(FILES, "find ".$dirstodostr." -name \"*.sbr\" -print |") || die "Couldn't run 'find'!\n";
    
    open(OUTFILE, ">".$cmdfilepath) || die "Couldn't open ".$cmdfilepath."!\n";
    
    $filename = <FILES>;  #skip initial line
    $filestr="";
    
    $duline = <FILES>;
    chop $duline;
    $filestr=$duline;
    
    while ($duline = <FILES>)
    {
            chop $duline;  # Remove the newline from the end of the filename
            $filestr=$filestr."\n".$duline;
    }
    
    print OUTFILE "/n /o ".$outputfilepath." \n";
    print OUTFILE "$filestr","\n";
    close(OUTFILE);
    close(FILES);
    
    $bscmake_str="bscmake /o ".$outputfilepath." @".$cmdfilepath."\n";
    
    &myexecstr($bscmake_str,"bscmake failed!!!", "DO_LOG","NT cmd");  

    &myexecstr("copy ".$outputfilepath." ".$opt1dir, "copy of ".$outputfilepath." failed!!", "DO_LOG","NT cmd");
    &mychdir($CYGBLDROOT);
}

sub gen_python_code() {

    # ETC_PATH required by generatePythonCode
    $ENV{'ETC_PATH'}='/home/builder/player/panda/etc /home/builder/player/direct/etc /home/builder/player/dtool/etc /home/builder/player/toontown/etc';
    my $origpath=$ENV{'PATH'};
    $ENV{'PATH'}="/usr/lib:/c/python16:/bin:/contrib/bin:/mscommon/Tools/WinNT:/mscommon/MSDev98/Bin:/mscommon/Tools:/msvc98/bin:/home/builder/player/dtool/bin:/home/builder/player/dtool/lib:/home/builder/player/direct/bin:/home/builder/player/direct/lib::/home/builder/player/toontown/bin:/home/builder/player/toontown/lib:/home/builder/player/panda/lib:/home/builder/player/panda/bin:/usr/local/bin:.:/c/WINNT/system32:/c/WINNT:/c/WINNT/System32/Wbem:/c/bin:/c/PROGRA~1/TCL/bin:/mspsdk/Bin/:/mspsdk/Bin/WinNT:/mscommon/Tools/WinNT:/mscommon/MSDev98/Bin:/mscommon/Tools:/msvc98/bin::/usr/local/panda/bin:/home/builder/scripts";
    my $directsrcroot=$WINBLDROOT."\\direct\\src";
    $ENV{'PYTHONPATH'}=$WINBLDROOT."\\panda\\lib;".$WINBLDROOT."\\direct\\lib;".$WINBLDROOT."\\toontown\\lib;".$WINBLDROOT."\\dtool\\lib;".$directsrcroot."\\leveleditor;".$directsrcroot."\\tkpanels;".$directsrcroot."\\tkwidgets;".$directsrcroot."\\directutil;".$directsrcroot."\\showbase;".$directsrcroot."\\distributed;".$directsrcroot."\\actor;".$directsrcroot."\\ffi;";
    $ENV{'TCSH_NO_CHANGEPATH'}='1';

    &logmsg($ENV{'PYTHONPATH'}."\n");

    &mychdir($CYGBLDROOT."/direct/bin");

    my $genpyth_str;

    if(($ENV{'PANDA_OPTIMIZE'} eq '1') || ($ENV{'PANDA_OPTIMIZE'} eq '2')) {
       $genpyth_str="python_d ";
    } else {
       $genpyth_str="python ";
    }

    $outputdir = $WINBLDROOT."\\direct\\lib\\py";
    &mymkdir($outputdir);
    $outputdir.= "\\Opt".$ENV{'PANDA_OPTIMIZE'}."-Win32";
    &mymkdir($outputdir);

    my $genargstr="-v -d";
    if($ENV{'PANDA_OPTIMIZE'} > 2) {
        $genargstr="-O ".$genargstr;
    }

    $genpyth_str.="generatePythonCode ".$genargstr." '".$outputdir."' -e '".$WINBLDROOT."\\direct\\src\\extensions' -i libdtool libpandaexpress libpanda libdirect libtoontown";
    
    &myexecstr($genpyth_str,"generate python code failed!!!","DO_LOG","NO_PANDA_ATTACH");
    
    $ENV{'PATH'}=$origpath;
    delete $ENV{'TCSH_NO_CHANGEPATH'};
    &mychdir($CYGBLDROOT);
}

sub buildall() {

    # DTOOL ppremake may have already run by DTOOL 'initialize make'
    
    $logmsg1 = shift;
    &logmsg($logmsg1);

    # cant do attachment, since that often hangs on NT
    # must use non-attachment build system  (BUILD_TYPE 'gmsvc', not 'stopgap', in $PPREMAKE_CONFIG)
    
    # hacks to fix multiproc build issue (cp file to dir occurs before dir creation)
    foreach my $dir1 (@dirstodolist) {    
        &mymkdir($CYGBLDROOT."/".$dir1."/etc"); 
        &mymkdir($CYGBLDROOT."/".$dir1."/bin"); 
        &mymkdir($CYGBLDROOT."/".$dir1."/lib"); 
        &mymkdir($CYGBLDROOT."/".$dir1."/include");
    }
    &mymkdir($CYGBLDROOT."/dtool/include/parser-inc");  # hack to fix makefile multiproc issue

    foreach my $dir1 (@dirstodolist) {    
        my $dir1_upcase = uc($dir1);

        &logmsg("*** PPREMAKE ".$dir1_upcase." ***");
        &mychdir($CYGBLDROOT."/".$dir1);
        &myexecstr("ppremake",$dir1_upcase." ppremake failed!","DO_LOG","NO_PANDA_ATTACH");
    }

    # debug stuff
    # &mychdir($CYGBLDROOT);
    # &myexecstr("dir dtool","dir failed","DO_LOG","NT cmd");

    foreach my $dir1 (@dirstodolist) {    
        my $dir1_upcase = uc($dir1);

        &logmsg("*** BUILDING ".$dir1_upcase." ***");
        &mychdir($CYGBLDROOT."/".$dir1);
        &myexecstr("make install",$dir1_upcase." make install failed!","DO_LOG","NO_PANDA_ATTACH");
    }

    &mychdir($CYGBLDROOT);  # get out of src dirs to allow them to be moved/renamed
    unlink($CYGBLDROOT."/dtool/dtool_config.h");  # fix freakish NTFS bug, this file is regenerated by ppremake anyway

    &gen_python_code();  # must run AFTER toontown bld
}

# assumes environment already attached to TOOL/PANDA/DIRECT/TOONTOWN
# assumes cygwin env, BLDROOT must use fwd slashes
if($ENV{'BLDROOT'} eq "") {
  $ENV{'BLDROOT'} = "/home/builder/player";
}

if($ENV{'CYGWIN_ROOT'} eq "") {
  $ENV{'CYGWIN_ROOT'} = "C:\\Cygwin";
}

$CYGROOT= $ENV{'CYGWIN_ROOT'};
$CYGROOT =~ s/(.*)\\$/$1/;   # get rid of trailing '\'
#$CYGROOT =~ s/\\/\//g;  # switch backslash to fwdslash  (setting up for cygwin)

$CYGBLDROOT = $ENV{'BLDROOT'};
print "\$CYGBLDROOT='",$CYGBLDROOT,"'\n";

if(($CYGBLDROOT eq "")||(!(-e $CYGBLDROOT))) {
  die "Bad \$CYGBLDROOT !!\n";
}

$WINBLDROOT=$CYGROOT.$CYGBLDROOT;
$WINBLDROOT =~ s/\//\\/g;  # switch fwdslash to backslash
print "\$WINBLDROOT='",$WINBLDROOT,"'\n";

my ($sec,$min,$hour,$mday,$mon,$year,$wday) = localtime(time);
$mon++;
$logfilenamebase="bldlog-".($mon<10 ? "0":"").$mon."-".($mday<10 ? "0":"").$mday.".txt";
$fulllogfilename = $CYGBLDROOT."/".$logfilenamebase;
$fulllogfilename_win = $WINBLDROOT."\\".$logfilenamebase;

# recreate the log to blow away any old one
open(LOGFILE,">".$fulllogfilename) || die "can't open log file '$fulllogfilename'\n";
close(LOGFILE);

&logmsg("*** Panda Build Log Started at ".&gettimestr()." ***");

if(!(-e $WIN_INSTALLDIR)) {
    &logmsg("ERROR: Cant access install directory!!  ".$WIN_INSTALLDIR);
    exit(1);
}

&mychdir($CYGBLDROOT);
# remove all old files (remove every file except for dirs and CVS files and bldlog*)
# and grab every file clean from CVS

if($BLD_DTOOL_ONLY) {
  @dirstodolist=("dtool");
} else {
  @dirstodolist=("dtool","panda","direct","toontown");
}
$dirstodostr="";
foreach my $dir1 (@dirstodolist) {
  $dirstodostr.=$dir1." ";
}

# makes ppremake build headers, libs in module dirs (panda/lib,dtool/bin,etc), not /usr/local/panda/inc...
foreach my $dir1 (@dirstodolist) {    
  my $dir1_upcase = uc($dir1);
  $ENV{$dir1_upcase}=$CYGBLDROOT."/".$dir1;

  # need this since we are building in src dirs, not install dir
  # so 'interrogate' needs to find its dlls when building panda, etc
  &appendpath("/".$dir1."/bin","/".$dir1."/lib");
}

# pick up cygwin utils
$ENV{'PATH'}="/bin".$DIRPATH_SEPARATOR."/contrib/bin".$DIRPATH_SEPARATOR.$ENV{'PATH'};

# want build to pick up python dll's from /usr/lib before /c/python16
$ENV{'PATH'}="/usr/lib".$DIRPATH_SEPARATOR.$ENV{'PATH'};

if($DEBUG_TREECOPY) {
    goto 'DBGTREECOPY';
}

if($DEBUG_GENERATE_PYTHON_CODE_ONLY) {
    &gen_python_code();
    exit(0);
}

# goto 'SKIP_REMOVE';
$existing_module_str="";
$nonexisting_module_str="";
foreach my $dir1 (@dirstodolist) {
    if(-e $dir1) {
        $existing_module_str.=$dir1." ";
    } else {
        $nonexisting_module_str.=$dir1." ";
    }
}

if($existing_module_str ne "") {
    &myexecstr("( for /D /R . %i in (Opt*Win32) do rd /s /q %i )","nomsg","DO_LOG","NT cmd");

    &logmsg("*** REMOVING ALL FILES IN OLD SRC TREES ***");
    # use cvs update to grab new copy
    # note: instead of blowing these away, may want to rename and save them
    # also, I could just blow everything away and check out again
    $rmcmd="find ".$existing_module_str." -path '*CVS*' -prune -or -not -name 'bldlog*' -and -not -type d -print | xargs --no-run-if-empty -n 40 rm";
    #&myexecstr($rmcmd,"Removal of old files failed!","DO_LOG","NO_CSHRC");
    &myexecstr($rmcmd,"Removal of old files failed!","DO_LOG","NO_PANDA_ATTACH");

    &myexecstr("cvs update -d -R ".$existing_module_str." |& egrep -v 'Updating|^\\?'",
               "cvs update failed!","DO_LOG","NO_PANDA_ATTACH");
}


if($nonexisting_module_str ne "") {
    &myexecstr("cvs checkout -R ".$nonexisting_module_str." |& egrep -v 'Updating|^\\?'",
               "cvs checkout failed!","DO_LOG","NO_PANDA_ATTACH");
}

SKIP_REMOVE:

# this doesnt work unless you can completely remove the dirs, since cvs checkout
# bombs if dirs exist but CVS dirs do not.  since sometimes shells will have
# the subdirs open preventing full deletion of a project, I will use the update method 
# above instead
# just delete dtool, panda, etc...
#$rmcmd= "rd /s /q ".$dirstodostr;
#&myexecstr($rmcmd,"","DO_LOG","NT cmd");  # dont bother checking errors here, probably just some shell has the dir cd'd to
#&myexecstr("cvs checkout -R ".$dirstodostr." |& egrep -v 'Updating|^\\?'",
#          "cvs checkout failed!","DO_LOG","NO_PANDA_ATTACH");

    
# could skip this and just ppremake dtool, but useful for interactive login environment
#&mychdir($CYGBLDROOT."/dtool/src/build");
#&myexecstr("./initialize make","DTOOL initialize make failed!","DO_LOG","");
# sometimes hangs, so I wont do this

$ENV{'USE_BROWSEINFO'}='1';   # make .sbr files
$ENV{'PANDA_OPTIMIZE'}='1';

# remove old stored debug build
if(-e $CYGBLDROOT."/debug") {
  &myexecstr("rd /s /q ".$WINBLDROOT."\\debug", # rd /s /q is fastest deltree method
             "",  # if we cant delete all of tree (because someone is in a subdir), should be ok.  file open would be bad
             "DO_LOG","NT cmd");  
}

BEFORE_DBGBUILD:

&buildall("*** Starting Debug Build (Opt=".$ENV{'PANDA_OPTIMIZE'}.") at ".&gettimestr()." ***");

AFTER_DBGBUILD:

&myexecstr("cvs checkout -R ".$dirstodostr." |& egrep -v 'Updating|^\\?'",
          "cvs checkout failed!","DO_LOG","NO_PANDA_ATTACH");

# tmp save debug build in "debug subdir"
# we can only hold 1 dbg and 1 opt build in same src tree and these are used by opt2 & opt4, so 
# move current tree to local dir until we are finished building, when we can copy it
my $localdebugdirname=$CYGBLDROOT."/debug";
&mymkdir($localdebugdirname);

foreach my $dir1 (@dirstodolist) {
    &myrename($CYGBLDROOT."/".$dir1,$localdebugdirname."/".$dir1);
}

# old dirs completely gone, need to re-checkout to setup for buildall
&mychdir($CYGBLDROOT);
&myexecstr("cvs checkout -R ".$dirstodostr." |& egrep -v 'Updating|^\\?'",
          "cvs checkout failed!","DO_LOG","NO_PANDA_ATTACH");

delete $ENV{'USE_BROWSEINFO'};
$ENV{'PANDA_OPTIMIZE'}='2';
&buildall("*** Starting Release Build (Opt=".$ENV{'PANDA_OPTIMIZE'}.") at ".&gettimestr()." ***");

# opt2 creates .sbr files when it should not
#foreach my $dir1 (@dirstodolist) {
#   &mychdir($CYGBLDROOT."/".$dir1);
#   &myexecstr("del /q /s *.sbr ".$dirstodostr,"","DO_LOG","NT cmd");
#
#hack:  having problems w/interrogate files being opened before they are ready, see if this helps
#   &myexecstr("del /q /s *igate.cxx *.in ".$dirstodostr,"","DO_LOG","NT cmd");
#}

$ENV{'PANDA_OPTIMIZE'}='3';
&buildall("*** Starting Release Build (Opt=".$ENV{'PANDA_OPTIMIZE'}.") at ".&gettimestr()." ***");

# opt2 creates .sbr files when it should not
#foreach my $dir1 (@dirstodolist) {
#   &mychdir($CYGBLDROOT."/".$dir1);
#   &myexecstr("del /q /s *.sbr ".$dirstodostr,"","DO_LOG","NT cmd");
#}

DBGTREECOPY:

# begin tree copying

if(!(-e $WIN_INSTALLDIR)) {
    &logmsg("ERROR: Cant access install directory!!  ".$WIN_INSTALLDIR);
    exit(1);
}

$opt1dir=$WIN_INSTALLDIR."\\debug";
$opt2dir=$WIN_INSTALLDIR."\\install";
$archivedir=$WIN_INSTALLDIR."\\archive";

($devicenum,$inodenum,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)
           = stat($opt2dir);
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) 
           = localtime($ctime);
$mon++;

$newdayarchivedirname=$archivedir."\\".($mon<10 ? "0":"").$mon."-".($mday<10 ? "0":"").$mday;

sub archivetree() {
    $olddirname=shift;
    $archdirname=shift;

    &mymkdir($newdayarchivedirname);
    &myrename($olddirname,$archdirname);

    # now delete old objs/pdbs/etc out of archived trees (just blow away the Opt[Win32] dir)
    foreach my $dir1 (@dirstodolist) {    
        # NT cmd 'for' always returns 144 for some reason, impossible to detect error cond, so just dont check retval
        &myexecstr("( for /D /R ".$archdirname."\\".$dir1."\\src %i in (Opt*Win32) do rd /s /q %i )","nomsg","DO_LOG","NT cmd");
        # delete old browse files
        &myexecstr("del /q ".$archdirname."\\debug\\*.bsc ".$dirstodostr,"nomsg","DO_LOG","NT cmd");
    }
}

#goto 'StartTreeCopy';  #DBG

if($DONT_ARCHIVE_OLD_BUILDS) {
   &myexecstr("rd /s /q ".$opt1dir." ".$opt2dir,"","DO_LOG","NT cmd");  # dont bother checking errors here, probably just some shell has the dir cd'd to

} elsif((-e $opt1dir) || (-e $opt2dir)) {
    &logmsg("*** Archiving old builds on install server at ".&gettimestr()." ***");

    if(-e $opt1dir) {
        &archivetree($opt1dir,$newdayarchivedirname."\\debug");
    }

    if(-e $opt2dir) {
        &archivetree($opt2dir,$newdayarchivedirname."\\install");
    }
}

StartTreeCopy:

&logmsg("*** Copying new builds to install server at ".&gettimestr()." ***");

&mymkdir($opt1dir);
&mymkdir($opt2dir);

$xcopy_opt_str="/E /K /C /R /Y /H ";

if($DEBUG_TREECOPY) {
    $xcopy_opt_str.="/T";  #debug only
}

&myexecstr("xcopy ".$xcopy_opt_str." ".$WINBLDROOT."\\debug\\* ".$opt1dir, 
            "xcopy of debug tree failed!!", "DO_LOG","NT cmd");  

foreach my $dir1 (@dirstodolist) {    
  # cant do a single xcopy since dont want to copy local "debug" or any other subdir crap in ~/player
   &mymkdir($opt2dir."/".$dir1);
   &myexecstr("xcopy ".$xcopy_opt_str." ".$WINBLDROOT."\\".$dir1."\\* ".$opt2dir."\\".$dir1, 
            "xcopy of ".$dir1." tree failed!!", "DO_LOG","NT cmd");  
}

&make_bsc_file();

&logmsg("*** Panda Build Log Finished at ".&gettimestr()." ***");

# store log in 'install' dir
&myexecstr("copy ".$fulllogfilename_win." ".$opt2dir, "copy of ".$fulllogfilename_win." failed!!", "","NT cmd");  

exit(0);

# TODO:
# possibly auto delete or compress old archived blds
# build DLLs with version stamp set by this script
# implement no-archive mode
# implement build-specific opttype mode


