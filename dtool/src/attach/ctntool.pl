# given a possibly empty string, format it into a comment or -nc
# input is in:
# $_[0] = possible comment string
#
# output is:
# string for use by neartool functions
sub CTNtoolFormatComment {
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
sub CTNtoolMkdir {
    &CTUDebug( "in CTNtoolMkdir\n" ) ;
    local( $ret ) = 0 ;
    local( $dir ) = $_[0] ;
    if ( ! ( $dir =~ /^\// )) {
	$dir = $_[1] . "/" . $dir ;
    }
    local( $comment ) = &CTNtoolFormatComment( $_[2] ) ;
    # first we have to check out the parent directory
    local( @alist ) = split( /\//, $dir ) ;
    pop( @alist ) ;
    local( $parent ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $dir . "' is '" . $parent . "'\n" ) ;
    $ret = system( "neartool co -nc $parent\n" ) ;
    if ( $ret == 0 ) {
	# now make the dir
	$ret = &CTURetCode( system( "neartool mkdir " . $comment .
				    " $dir\n" )) ;
    } else {
	$ret = 0 ;
    }
    &CTUDebug( "out of CTNtoolMkdir\n" ) ;
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
sub CTNtoolMkelem {
    &CTUDebug( "in CTNtoolMkelem\n" ) ;
    local( $ret ) = 0 ;
    local( $elem ) = $_[0] ;
    if ( ! ( $elem =~ /^\// )) {
	$elem = $_[1] . "/" . $elem ;
    }
    local( $comment ) = &CTNtoolFormatComment( $_[2] ) ;
    local( $eltype ) = $_[3] ;
    if ( $eltype ne "" ) {
	$eltype = "-eltype " . $eltype ;
    }
    local( $line ) = "neartool mkelem " . $comment . " " . $eltype . " " .
	$elem . "\n" ;
    &CTUDebug( $line ) ;
    $ret = &CTURetCode( system( $line )) ;
    &CTUDebug( "out of CTNtoolMkelem\n" ) ;
    $ret ;
}

# delta an element
# input is in:
# $_[0] = element to delta
#
# output:
# return success or failure
sub CTNtoolDelta {
    &CTUDebug( "in CTNtoolDelta\n" ) ;
    local( $ret ) = 0 ;
    # as Dave points out, when working off-line, delta is the same as checkin
    $ret = &CTURetCode( system( "neartool ci " . $_[0] )) ;
    &CTUDebug( "out of CTNtoolDelta\n" ) ;
    $ret ;
}

# checkout an element
# input is in:
# $_[0] = element to checkout
# $_[1] = possible comment
#
# output:
# return success or failure
sub CTNtoolCheckout {
    &CTUDebug( "in CTNtoolCheckout\n" ) ;
    local( $ret ) = 0 ;
    local( $comment ) = &CTNtoolFormatComment( $_[1] ) ;
    if ( ! -d $_[0] ) {
	$ret = &CTURetCode( system( "neartool co " . $comment . " " .
				    $_[0] )) ;
    } else {
	# neartool doesn't do anything about checking out directories
	$ret = 1 ;
    }
    &CTUDebug( "out of CTNtoolCheckout\n" ) ;
    $ret ;
}

# checkin an element
# input is in:
# $_[0] = element to checkin
# $_[1] = possible comment
#
# output:
# return success or failure
sub CTNtoolCheckin {
    &CTUDebug( "in CTNtoolCheckin\n" ) ;
    local( $ret ) = 0 ;
    local( $comment ) = &CTNtoolFormatComment( $_[1] ) ;
    $ret = &CTURetCode( system( "neartool ci " . $comment . " " . $_[0] )) ;
    &CTUDebug( "out of CTNtoolCheckin\n" ) ;
    $ret ;
}

# uncheckout an element
# input is in:
# $_[0] = element to uncheckout
#
# output:
# return success or failure
sub CTNtoolUncheckout {
    &CTUDebug( "in CTNtoolUncheckout\n" ) ;
    local( $ret ) = 0 ;
    $ret = &CTURetCode( system( "neartool unco " . $_[0] )) ;
    &CTUDebug( "out of CTNtoolUncheckout\n" ) ;
    $ret ;
}

# figure out what all I have checked out
# input is in:
# $_[0] = project
# $_[1] = flavor
# $_[2] = spec line
#
# output:
# return a \n serperated list of elements checked out
sub CTNtoolIHave {
    &CTUDebug( "in CTNtoolIHave\n" ) ;
    local( $ret ) = "" ;
    local( $root ) = &CTProjRoot( $_[0] ) ;
    local( *OUTPUT ) ;
    open( OUTPUT, "neartool find " . $root . " |" ) ;
    while ( <OUTPUT> ) {
	$ret = $ret . $_ ;
    }
    close( OUTPUT ) ;
    &CTUDebug( "out of CTNToolIHave\n" ) ;
    $ret ;
}

# remove a versioned element
# input is in:
# $_[0] = element to remove
# $_[1] = curr dir
#
# output:
# return success or failure
sub CTNtoolRmElem {
    &CTUDebug( "in CTNtoolRmElem\n" ) ;
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
    $ret = system( "neartool co -nc $parent\n" ) ;
    if ( $ret == 0 ) {
	# now nuke the element
	$ret = &CTURetCode( system( "neartool rmname $elem\n" )) ;
    } else {
	$ret = 0 ;
    }
    &CTUDebug( "out of CTNtoolRmElem\n" ) ;
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
sub CTNtoolMv {
    &CTUDebug( "in CTNtoolMv\n" ) ;
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
    @alist = split( /\//, $elem2 ) ;
    pop( @alist ) ;
    local( $parent2 ) = join( "/", @alist ) ;
    &CTUDebug( "parent directory of '" . $elem2 . "' is '" . $parent2 .
	       "'\n" ) ;
    $ret = system( "neartool co -nc $parent\n" ) ;
    if ( $ret == 0 ) {
	$ret = system( "neartool co -nc $parent2\n" ) ;
	if ( $ret == 0 ) {
	    # now move the element
	    $ret = &CTURetCode( system( "neartool mv $elem $elem2\n" )) ;
	} else {
	    $ret = 0 ;
	}
    } else {
	$ret = 0 ;
    }
    &CTUDebug( "out of CTNtoolMv\n" ) ;
    $ret ;
}

# build a list of targets
# input is in:
# $_[0] = targets
#
# output:
# return success or failure
sub CTNtoolMake {
    &CTUDebug( "in CTNtoolMake\n" ) ;
    local( $ret ) = 0 ;
    local( $line ) = "make " . $_[0] . "\n" ;
    $ret = &CTURetCode( system( $line )) ;
    &CTUDebug( "out of CTNtoolMake\n" ) ;
    $ret ;
}

1;
