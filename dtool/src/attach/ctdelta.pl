# Check in element if needed
# Input is:
#   $_[0] = element name
sub CTDeltaCheckin {
   local( $cmd ) = "cleartool ci -nc $_[0] 2> /dev/null > /dev/null" ;
   system $cmd ;
}

# get the version of an element
# Input is:
#   $_[0] = element name
sub CTDeltaGetVersion {
   local( *CMDFILE ) ;
   open( CMDFILE, "cleartool describe -short $_[0] |" ) ;
   $_ = <CMDFILE> ;
   close( CMDFILE ) ;
   s/\n$// ;
   s/^.*@@// ;
   s/\"$// ;
   $_ ;
}

# Is it ok to try a merge on this version?
# Input is:
#   $_[0] = version
sub CTDeltaOk {
   local( $ret ) ;
   local( @verlist ) ;
   @verlist = split( /\//, $_[0] ) ;
   pop( @verlist ) ;
   if ( $#verlist > 1 ) {
      $ret = 1 ;
   } else {
      $ret = 0 ;
   }
   $ret ;
}

# get the comments from a version of an element
# Input is:
#   $_[0] = element name
#   $_[1] = version
#
# output in:
#   @CTDeltaComments
sub CTDeltaGetComments {
   local( *CMDFILE ) ;
   local( $done ) = 0 ;
   local( $end ) = "  element type:" ;
   local( $tmp ) = "cleartool describe $_[0]" . "@@" . "$_[1] |" ;
   open( CMDFILE, $tmp ) ;
   $_ = <CMDFILE> ;
   $_ = <CMDFILE> ;
   while ( ! $done ) {
      $_ = <CMDFILE> ;
      if ( $_ =~ /^$end/ ) {
	 $done = 1 ;
      } else {
	 s/^  // ;
	 s/^ // ;
	 s/^\"// ;
	 s/\n$// ;
	 s/\"$// ;
	 push( @CTDeltaComments, $_ ) ;
      }
   }
   close( CMDFILE ) ;
}

# try automatic merge.  If it fails, use xmerge
# Input is:
#   $_[0] = element name
#   $_[1] = source version
#   $_[2] = target version
sub CTDeltaSafeMerge {
   @CTDeltaComments = ();
   &CTDeltaGetComments($_[0], $_[1]);
   local( $ret ) ;
   $ret = "cleartool checkout -branch $_[2] -nc $_[0] 2> /dev/null > /dev/null" ;
   $ret = system $ret ;
   if ( $ret != 0 ) {
      print STDERR "got return value $ret from checkout on '$_[0]" . "@@" . "$_[2]'\n" ;
      exit -1;
   }
   local( $item ) ;
   foreach $item ( @CTDeltaComments ) {
      $ret = "cleartool chevent -append -c \"" . $item . "\" $_[0]" . "@@" . "$_[2]" . "/LATEST 2> /dev/null > /dev/null" ;
      system $ret ;
   }
   print STDERR "merging '$_[0]'...\n" ;
   $ret = "cleartool merge -abort -to $_[0] -version $_[1] 2> /dev/null > /dev/null" ;
   $ret = system $ret ;
   if ( $ret != 0 ) {
      $ret = system "cleartool xmerge -to $_[0] -version $_[1]" ;
   }
   if ( ! -d $_[0] ) {
      system "rm $_[0]" . ".contrib" ;
   }
   $ret ;
}

# test a branch for 'triviality'
# Input is:
#   $_[0] = element name
#   $_[1] = branch name
#
# Output is:
#   true/false
sub CTDeltaTestBranch {
   local( *CTCMD ) ;
   local( $ret ) ;
   local( $done ) = 0 ;
   local( $bfrom ) ;
   local( @blist ) ;
   local( $bto ) ;
   local( $bdiff ) ;
   local( $blast ) ;
   @blist = split( /\//, $_[1] ) ;
   pop( @blist ) ;
   $ret = join( "/", @blist ) ;
   $ret = "cleartool describe $_[0]" . "@@" . "$ret |" ;
   open( CTCMD, $ret ) ;
   while ( ! $done ) {
      $_ = <CTCMD> ;
      if ( $_ =~ /^  branched from version/ ) {
	 $done = 1 ;
      }
   }
   close( CTCMD ) ;
   s/^  branched from version: // ;
   s/\n$// ;
   $bfrom = $_ ;
   @blist = split( /\//, $_ ) ;
   pop( @blist ) ;
   push( @blist, "LATEST" ) ;
   $ret = join( "/", @blist ) ;
   $ret = "cleartool describe $_[0]" . "@@" . "$ret |" ;
   open( CTCMD, $ret ) ;
   $_ = <CTCMD> ;
   close( CTCMD ) ;
   s/\n$// ;
   s/^.*@@// ;
   s/\"$// ;
   $bto = $_ ;
   @blist = split( /\//, $bfrom ) ;
   $bfrom = pop( @blist ) ;
   @blist = split( /\//, $bto ) ;
   $bto = pop( @blist ) ;
   $bdiff = $bto - $bfrom ;
   $ret = "cleartool describe $_[0]" . "@@" . "$_[1] |" ;
   open( CTCMD, $ret ) ;
   $_ = <CTCMD> ;
   close( CTCMD ) ;
   s/\n$// ;
   s/^.*@@// ;
   s/\"$// ;
   @blist = split( /\//, $_ ) ;
   $blast = pop( @blist ) ;
   if (( $bdiff > 1 ) || ( $blast > 1 )) {
      $ret = 0 ;
   } else {
      $ret = 1 ;
   }
}

# check for trivial branch elimination
# Input is:
#   $_[0] = element name
#   $_[1] = last branch version
#   $_[2] = timestamp string
sub CTDeltaBranchCheck {
   local( $test ) = &CTDeltaTestBranch( $_[0], $_[1] ) ;
   local( $cmd ) ;
   local( @blist ) ;
   local( $branch ) ;
   @blist = split( /\//, $_[1] ) ;
   if ( $test ) {
       pop( @blist ) ;
       $cmd = join( "/", @blist ) ;
       $branch = join( "/", @blist ) ;
       $cmd = "cleartool rmbranch -force $_[0]" . "@@" . "$cmd 2> /dev/null > /dev/null" ;
       print STDERR "deleting branch '$branch'...\n" ;
       system $cmd ;
   } else {
       pop( @blist ) ;
       $branch = join( "/", @blist ) ;
       $test = pop( @blist ) ;
       $test = $test . $_[2] ;
       $cmd = "cleartool mkbrtype -c \"non-trivial branch\" $test 2> /dev/null > /dev/null" ;
       system $cmd ;
       $cmd = "cleartool chtype -c \"renaming non-trivial branch\" $test $_[0]" . "@@" . "$branch 2> /dev/null > /dev/null" ;
       print STDERR "renaming branch '$branch'...\n" ;
       system $cmd ;
   }
}

# log merge to /usr/local/etc/delta_log
# Input is:
#   $_[0] = element name
#   $_[1] = source version
#   $_[2] = target version
sub CTDeltaLog {
   local( *LOGFILE ) ;
   local( *CMDFILE ) ;
   local( $cmd ) ;
   open( LOGFILE, ">>/usr/local/etc/delta_log" ) ;
   print LOGFILE $_[0] . ": " . $_[1] . " -> " . $_[2] . " : " ;
   if ( $ctdebug ne "" ) {
      print STDERR "CTDeltaLog: outputting '" . $_[0] . ": " . $_[1] . " -> " . $_[2] . " : '\n" ;
   }
   $cmd = "ypmatch `whoami` passwd | cut -d: -f5 |" ;
   open( CMDFILE, $cmd ) ;
   $_ = <CMDFILE> ;
   s/\n$//;
   print LOGFILE $_ . " " ;
   if ( $ctdebug ne "" ) {
      print STDERR "CTDeltaLog: outputting '" . $_ . " '\n" ;
   }
   close( CMDFILE ) ;
   $cmd = "/bin/date '+%m/%d/%y %H:%M:%S' |" ;
   open( CMDFILE, $cmd ) ;
   $_ = <CMDFILE> ;
   s/\n$//;
   print LOGFILE $_ . "\n" ;
   if ( $ctdebug ne "" ) {
      print STDERR "CTDeltaLog: outputting '" . $_ . " '\n" ;
   }
   close( CMDFILE ) ;
   close( LOGFILE ) ;
}

1;
