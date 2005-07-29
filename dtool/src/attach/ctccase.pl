# given the config line, determine the view name
# $_[0] = config line
# $_[1] = flavor
# $_[2] = project
sub CTAttachComputeView {
   &CTUDebug( "in CTAttachComputeView\n" ) ;
   local( $ret ) = &CTResolveSpecName( $_[2], $_[1] ) ;
   local( $options ) = &CTSpecOptions( $_[0] ) ;
   if ( $options ne "" ) {
       local( $name ) = &CTSpecFindOption( $options, "name" ) ;
       if ( $name ne "" ) {
	   &CTUDebug( "found a name '" . $name . "'\n" ) ;
	   $ret = $name ;
       } else {
	   &CTUDebug( "no name option found, going with default\n" ) ;
       }
   }
   &CTUDebug( "config line '" . $_[0] . "' yields view name '" . $ret .
	      "'\n" . "out of CTAttachComputeView\n" ) ;
   $ret ;
}

# given the config line, determine the branch name
# $_[0] = config line
# $_[1] = flavor
# $_[2] = project
sub CTAttachComputeBranch {
   &CTUDebug( "in CTAttachComputeBranch\n" ) ;
   local( $ret ) = &CTAttachComputeView( $_[0], $_[1], $_[2] ) ;
   &CTUDebug( "config line '" . $_[0] . "' yields branch name '" . $ret .
	      "'\n" . "out of CTAttachComputeBranch\n" ) ;
   $ret ;
}

# given the config line, determine the label name
# $_[0] = config line
# $_[1] = flavor
# $_[2] = project
sub CTAttachComputeLabel {
   &CTUDebug( "in CTAttachComputeLabel\n" ) ;
   local( $ret ) = &CTAttachComputeView( $_[0], $_[1], $_[2] ) ;
   $ret =~ tr/a-z/A-Z/ ;
   &CTUDebug( "config line '" . $_[0] . "' yields label name '" . $ret .
	      "'\n" . "out of CTAttachComputeLabel\n" ) ;
   $ret ;
}

# given the project name and config line, determine the root of the project as
# needed by the config spec.
# $_[0] = project
# $_[1] = config line
# $_[2] = flavor
sub CTAttachComputeElemRoot {
   &CTUDebug( "in CTAttachComputeElemRoot\n" ) ;
   local( $ret ) = "/vobs/$_[0]" ;
   &CTUDebug( "out of CTAttachComputeElemRoot\n" ) ;
   $ret ;
}

# do whatever setup is needed for ClearCase
# input is in:
# $_[0] = project
# $_[1] = $spec
# $_[2] = flavor
sub CTAttachCCSetup {
   &CTUDebug( "in CTAttachCCSetup\n" ) ;
   local( $root ) = &CTAttachComputeElemRoot( $_[0], $_[1], $_[2] ) ;
   local( $view ) = &CTAttachComputeView( $_[1], $_[2], $_[0] ) ;
   local( $branch ) = &CTAttachComputeBranch( $_[1], $_[2], $_[0] ) ;
   local( $label ) = &CTAttachComputeLabel( $_[1], $_[2], $_[0] ) ;
   local( *CTINTERFACE ) ;
   local( *TMPFILE ) ;
   local( $tmpname ) = "/tmp/config.$$" ;
   local( $emitted ) = 0 ;

   &CTUDebug( "checking for existance of view '" . $view . "'\n" ) ;
   open( CTINTERFACE, "/usr/atria/bin/cleartool lsview $view |" ) ;
   $_ = <CTINTERFACE> ;
   close( CTINTERFACE ) ;
   if ( $_ eq "" ) {             # need to make the view
      &CTUDebug( "creating view '" . $view . "'\n" ) ;
      system "umask 2 ; /usr/atria/bin/cleartool mkview -tag $view /var/views/$view.vws 2> /dev/null > /dev/null ; /usr/atria/bin/cleartool startview $view 2> /dev/null > /dev/null\n" ;
   } elsif ( ! ( $_ =~ /\*/ )) { # need to start the view
      &CTUDebug( "starting view '" . $view . "'\n" ) ;
      system "/usr/atria/bin/cleartool startview $view 2> /dev/null > /dev/null &\n" ;
   }

   &CTUDebug( "making branch and label types for view " . $view . "\n" ) ;
   system "/usr/atria/bin/cleartool mkbrtype -vob /vobs/$vobname -c \"Branch type for the $view view\" $branch 2> /dev/null > /dev/null &\n" ;
   system "/usr/atria/bin/cleartool mklbtype -vob /vobs/$vobname -c \"Label type for the $view view\" $label 2> /dev/null > /dev/null &\n" ;

   &CTUDebug( "creating/updating the config-spec for view " . $view . "\n" ) ;
   open( CTINTERFACE, "/usr/atria/bin/cleartool catcs -tag $view |" ) ;
   open( TMPFILE, "> $tmpname" ) ;
   while ( <CTINTERFACE> ) {
      if ( $_ =~ "CHECKEDOUT" ) {
	 print TMPFILE "$_" ;
      } elsif (( $_ =~ /^element \*/ ) && ( $_ =~ "/main/LATEST" ) &&
	       !( $_ =~ /$_[0]/ )) {
	 if ( ! $emitted ) {
	    $emitted = 1 ;
	    print TMPFILE "element $root/... .../$branch/LATEST\n" ;
	    print TMPFILE "element $root/... $label -mkbranch $branch\n" ;
	    print TMPFILE "element $root/... /main/LATEST -mkbranch $branch\n" ;
	 }
	 print TMPFILE "$_" ;
      } elsif ( $_ =~ /$_[0]/ ) {
	 if ( ! $emitted ) {
	    $emitted = 1 ;
	    print TMPFILE "element $root/... .../$branch/LATEST\n" ;
	    print TMPFILE "element $root/... $label -mkbranch $branch\n" ;
	    print TMPFILE "element $root/... /main/LATEST -mkbranch $branch\n" ;
	 }
      } else {
	 print TMPFILE "$_" ;
      }
   }
   close( CTINTERFACE ) ;
   close( TMPFILE ) ;
   system "/usr/atria/bin/cleartool setcs -tag $view $tmpname ; rm -f $tmpname &\n" ;
   &CTUDebug( "out of CTAttachCCSetup\n" ) ;
}

# do whatever setup is needed for ClearCase, but do it in the background
# input is in:
# $_[0] = project
# $_[1] = $spec
# $_[2] = flavor
sub CTAttachCCSetupBG {
   &CTUDebug( "in CTAttachCCSetupBG\n" ) ;
   local( $root ) = &CTAttachComputeElemRoot( $_[0], $_[1], $_[2] ) ;
   local( $view ) = &CTAttachComputeView( $_[1], $_[2], $_[0] ) ;
   local( $branch ) = &CTAttachComputeBranch( $_[1], $_[2], $_[0] ) ;
   local( $label ) = &CTAttachComputeLabel( $_[1], $_[2], $_[0] ) ;

   system "$tool/bin/ctattachcc $root $view $branch $label $vobname $_[0]\n" ;

   &CTUDebug( "out of CTAttachCCSetupBG\n" ) ;
}

# given a possibly empty string, format it into a comment or -nc
# input is in:
# $_[0] = possible comment string
#
# output is:
# string for use by ClearCase functions
sub CTCcaseFormatComment {
    local( $ret ) = "" ;
    if ( $_[0] eq "" ) {
	$ret = "-nc" ;
    } else {
        $ret = "-c \"" . $_[0] . "\"" ;
    }
    $ret ;
}

# make a versioned directory
# input is in:
# $_[0] = directory to create
# $_[1] = curr dir
# $_[2] = possible comment
#
# output:
# return success or failure
sub CTCcaseMkdir {
    &CTUDebug( "in CTCcaseMkdir\n" ) ;
    local( $ret ) = 0 ;
    local( $dir ) = $_[0] ;
    if ( ! ( $dir =~ /^\// )) {
	$dir = $_[1] . "/" . $dir ;
    }
    local( $comment) = &CTCcaseFormatComment( $_[2] ) ;
    # first we have to check out the parent directory
    local( @alist ) = split( /\//, $dir ) ;
    pop( @alist ) ;
    local( $parent ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $dir . "' is '" . $parent . "'\n" ) ;
    $ret = system( "cleartool co -nc $parent\n" ) ;
    if ( $ret == 0 ) {
	# now make the dir
	$ret = &CTURetCode( system( "cleartool mkdir " . $comment .
				    " $dir\n" )) ;
    } else {
	$ret = 0 ;
    }
    &CTUDebug( "out of CTCcaseMkdir\n" ) ;
    $ret ;
}

# make a versioned element
# input is in:
# $_[0] = element to version
# $_[1] = curr dir
# $_[2] = possible comment
# $_[3] = possible eltype
#
# output:
# return success or failure
sub CTCcaseMkelem {
    &CTUDebug( "in CTCcaseMkelem\n" ) ;
    local( $ret ) = 0 ;
    local( $elem ) = $_[0] ;
    if ( ! ( $elem =~ /^\// )) {
	$elem = $_[1] . "/" . $elem ;
    }
    local( $comment) = &CTCcaseFormatComment( $_[2] ) ;
    local( $eltype ) = $_[3] ;
    if ( $eltype ne "" ) {
	$eltype = "-eltype " . $eltype ;
    }
    # first we have to check out the parent directory
    local( @alist ) = split( /\//, $elem ) ;
    pop( @alist ) ;
    local( $parent ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $elem . "' is '" . $parent . "'\n" ) ;
    $ret = system( "cleartool co -nc $parent\n" ) ;
    if ( $ret != 0 ) {
	&CTUDebug( "checking out the dirctory gave return code: " . $ret .
		  "\n" ) ;
	$ret = 0 ;
    }
    # now make the elem
    $ret = &CTURetCode( system( "cleartool mkelem " . $comment . " " .
			       $eltype . " $elem\n" )) ;
    &CTUDebug( "out of CTCcaseMkelem\n" ) ;
    $ret ;
}

# done here so there will be coherence if multiple deltas are done
require "ctime.pl" ;
$timestamp = &ctime(time) ;
$timestamp =~ s/\n$// ;
@timelist = split( /\s+/, $timestamp ) ;
$timestamp = $timelist[2] . $timelist[1] . $timelist[5] . "_" . $timelist[3] ;
$timestamp =~ s/:/_/g ;

# delta an element
# input is in:
# $_[0] = element to delta
#
# output:
# return success or failure
sub CTCcaseDelta {
    require "$tool/built/include/ctdelta.pl" ;

    &CTUDebug( "in CTCcaseDelta\n" ) ;
    local( $ret ) = 0 ;
    # this is ripped from the old ctdelta script
    &CTDeltaCheckin( $_[0] ) ;
    local( $ver ) = &CTDeltaGetVersion( $_[0] ) ;
    &CTUDebug( "got version '" . $ver . "'\n" ) ;
    if ( &CTDeltaOk( $ver )) {
	local( @verlist ) = split( /\//, $ver ) ;
	pop( @verlist ) ;
	pop( @verlist ) ;
	local( $ver2 ) = join( "/", @verlist ) ;
	&CTUDebug( "ver2 = '" . $ver2 . "'\n" ) ;
	&CTDeltaSafeMerge( $_[0], $ver, $ver2 ) ;
	system "cleartool checkin -nc $_[0] 2> /dev/null > /dev/null" ;
	&CTUDebug( "merge complete, doing branch check\n" ) ;
	&CTDeltaBranchCheck( $_[0], $ver, $timestamp ) ;
	&CTUDebug( "logging potentially felonious activity for future" .
		   "  incrimination\n" ) ;
	&CTDeltaLog( $_[0], $ver, $ver2 ) ;
	# better detection needs to be done
	$ret = 1 ;
    } else {
	&CTUDebug( "cannot merge '" . $_[0] . "', no branches.\n" ) ;
    }
    &CTUDebug( "out of CTCcaseDelta\n" ) ;
    $ret ;
}

# checkout an element
# input is in:
# $_[0] = element to checkout
# $_[1] = possible comment
#
# output:
# return success or failure
sub CTCcaseCheckout {
    &CTUDebug( "in CTCcaseCheckout\n" ) ;
    local( $comment) = &CTCcaseFormatComment( $_[1] ) ;
    local( $ret ) = &CTURetCode( system( "cleartool co " . $comment .
					 " $_[0]\n" )) ;
    &CTUDebug( "out of CTCcaseCheckout\n" ) ;
    $ret ;
}

# checkin an element
# input is in:
# $_[0] = element to checkin

#
# output:
# return success or failure
sub CTCcaseCheckin {
    &CTUDebug( "in CTCcaseCheckin\n" ) ;
    local( $comment) = &CTCcaseFormatComment( $_[1] ) ;
    local( $ret ) = &CTURetCode( system( "cleartool ci " . $comment .
					 " $_[0]\n" )) ;
    &CTUDebug( "out of CTCcaseCheckin\n" ) ;
    $ret ;
}

# uncheckout an element
# input is in:
# $_[0] = element to uncheckout
#
# output:
# return success or failure
sub CTCcaseUncheckout {
    require "$tool/built/include/unco.pl" ;
    &CTUDebug( "in CTCcaseUncheckout\n" ) ;
    local( $ret ) = 1 ;
    # need better error checking on this
    system( "cleartool unco -rm $_[0]\n" ) ;
    &CTUncoDoIt( $_[0] ) ;
    &CTUDebug( "out of CTCcaseUncheckout\n" ) ;
    $ret ;
}

# figure out what all I have checked out or on my branch
# input is in:
# $_[0] = project
# $_[1] = flavor
# $_[2] = spec line
#
# output:
# return a \n serperated list of elements checked out
sub CTCcaseIHave {
    &CTUDebug( "in CTCcaseIHave\n" ) ;
    local( $ret ) = "" ;
    local( $branch ) = &CTAttachComputeBranch( $_[2], $_[1], $_[0] ) ;
    local( $root ) = &CTProjRoot( $_[0] ) ;
    local( *OUTPUT ) ;
    open( OUTPUT, "cleartool find " . $root . " -element \"brtype(" .
	  $branch . ")\" -nxn -print |" ) ;
    while ( <OUTPUT> ) {
	$ret = $ret . $_ ;
    }
    close( OUTPUT ) ;
    &CTUDebug( "out of CTCcaseIHave\n" ) ;
    $ret ;
}

# remove a versioned element
# input is in:
# $_[0] = element to remove
# $_[1] = curr dir
#
# output:
# return success or failure
sub CTCcaseRmElem {
    &CTUDebug( "in CTCcaseRmElem\n" ) ;
    local( $ret ) = 0 ;
    local( $elem ) = $_[0] ;
    if ( ! ( $elem =~ /^\// )) {
	$elem = $_[1] . "/" . $elem ;
    }
    # first we have to check out the parent directory
    local( @alist ) = split( /\//, $elem ) ;
    pop( @alist ) ;
    local( $parent ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $elem . "' is '" . $parent . "'\n" ) ;
    $ret = system( "cleartool co -nc $parent\n" ) ;
    if ( $ret == 0 ) {
	# now nuke the element
	$ret = &CTURetCode( system( "cleartool rmname $elem\n" )) ;
    } else {
	$ret = 0 ;
    }
    &CTUDebug( "out of CTCcaseRmElem\n" ) ;
    $ret ;
}

# mv a versioned element from one name to another
# input is in:
# $_[0] = from element
# $_[1] = to element
# $_[2] = current directory
#
# output:
# return success or failure
sub CTCcaseMv {
    &CTUDebug( "in CTCcaseMv\n" ) ;
    local( $ret ) = 0 ;
    local( $elem ) = $_[0] ;
    if ( ! ( $elem =~ /^\// )) {
	$elem = $_[2] . "/" . $elem ;
    }
    # first we have to check out the parent directory
    local( @alist ) = split( /\//, $elem ) ;
    pop( @alist ) ;
    local( $parent ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $elem . "' is '" . $parent . "'\n" ) ;
    local( $elem2 ) = $_[1] ;
    if ( ! ( $elem2 =~ /^\// )) {
	$elem2 = $_[2] . "/" . $elem2 ;
    }
    local( @alist ) = split( /\//, $elem2 ) ;
    pop( @alist ) ;
    local( $parent2 ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $elem2 . "' is '" . $parent2 .
	       "'\n" ) ;
    system( "cleartool co -nc $parent\n" ) ;
    system( "cleartool co -nc $parent2\n" ) ;
    $ret = &CTURetCode( system( "cleartool mv $elem $elem2\n" )) ;
    &CTUDebug( "out of CTCcaseMv\n" ) ;
    $ret ;
}

# build a list of targets
# input is in:
# $_[0] = targets
#
# output:
# return success or failure
sub CTCcaseMake {
    &CTUDebug( "in CTCcaseMake\n" ) ;
    local( $ret ) = 0 ;
    local( $line ) = "clearmake -C gnu " . $_[0] .
	" |& grep -v \"^clearmake: Warning: Config\"\n" ;
    &CTUDebug( "line = '" . $line . "'\n" ) ;
    $ret = &CTURetCode( system( $line )) ;
    &CTUDebug( "out of CTCcaseMake\n" ) ;
    $ret ;
}

1;
