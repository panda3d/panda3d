# given a possibly empty string, format it into a comment or -nc
# input is in:
# $_[0] = possible comment string
#
# output is:
# string for use by CVS functions
sub CTCvsFormatComment {
    local( $ret ) = "" ;
    if ( $_[0] ne "" ) {
        $ret = "-m \"" . $_[0] . "\"" ;
    }
    $ret ;
}

# given a project and spec line, compute the server line
# input is in:
# $_[0] = project
# $_[1] = spec line
#
# output:
# return a sever line, or "" if not a croot
sub CTCvsServerLine {
    &CTUDebug( "in CTCvsServerLine\n" ) ;
    local( $ret ) = "" ;
    local( $type ) = &CTSpecType( $_[1] ) ;
    if ( $type eq "croot" ) {
	local( $options ) = &CTSpecOptions( $_[1] ) ;
	local( $sline ) = &CTSpecFindOption( $options, "server" ) ;
	if ( $sline ne "" ) {
	    $ret = join( ":", split( /,/, $sline ));
	}
    }
    &CTUDebug( "out of CTCvsServerLine\n" ) ;
    $ret ;
}

# if needed log into a cvs server
# input is in:
# $_[0] = server line
#
# output:
# return success or failure
sub CTCvsLogin {
    &CTUDebug( "in CTCvsLogin\n" ) ;
    local( $ret ) = 0 ;
    &CTUDebug( "server line is '" . $_[0] . "'\n" ) ;
    if ( $_[0] ne "" ) {
	# ok.  we actually have something, lets look in .cvspass
	local( $path ) ;
	local( *PASSFILE ) ;
	if ( $ENV{"PENV"} eq "WIN32" ) {
	    $path = $ENV{"HOME"} . "/.cvspass" ;
	} else {
	    # $path = "~/.cvspass" ;
	    $path = $ENV{"HOME"} . "/.cvspass" ;
	}
	&CTUDebug( "looking for '" . $path . "'\n" ) ;
	if ( -e $path ) {
	    local( $passdone ) = 0 ;
	    local( $ok ) = 0 ;
	    open( PASSFILE, "< $path" ) ;
	    while ( <PASSFILE> ) {
		s/\n$// ;
		local( @line ) = split ;
		# ok, the server line is in [0] and the password in [1].
		&CTUDebug( "server line from .cvspass is '" . $line[0] .
			   "'\n" ) ;
		if ( $line[0] eq $_[0] ) {
		    # we're fine, we're already logged in to that
		    $ret = 1 ;
		    $passdone = 1;
		}
	    }
	    if ( ! $passdone ) {
		# ran out of lines in the file
		local( $line ) = "cvs -d " . $_[0] . " login >/dev/null" ;
		&CTUDebug( "about to run '" . $line . "'\n" ) ;
		$ret = &CTURetCode( system( $line )) ;
	    }
	} else {
	    &CTUDebug( $path . " file does not exist\n" ) ;
	    local( $line ) = "cvs -d " . $_[0] . " login >/dev/null" ;
	    &CTUDebug( "about to run '" . $line . "'\n" ) ;
	    $ret = &CTURetCode( system( $line )) ;
	}
    }
    &CTUDebug( "out of CTCvsLogin\n" ) ;
    $ret ;
}

require "$tool/built/include/ctproj.pl" ;

# add a versioned element to the repository
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
# $_[3] = possible comment
#
# output:
# return success or failure
sub CTCvsAdd {
    &CTUDebug( "in CTCvsAdd\n" ) ;
    # first we need to 'login' to the repository
    local( $comment ) = &CTCvsFormatComment( $_[3] ) ;
    local( $serve ) = &CTCvsServerLine( $_[1], $_[2] ) ;
    local( $ret ) = &CTCvsLogin( $serve ) ;
    if ( $ret ) {
	# now issue the add command
	local( $root ) = &CTProjRoot( $_[1] ) ;
	local( $line ) = "" ;
	local( $elem ) = $_[0] ;
	if ( $elem =~ /^\// ) {
	    local( $proj ) = $_[1] ;
	    $proj =~ tr/a-z/A-Z/ ;
	    $line = "cd \$" . $proj . "; " ;
	    $elem =~ s/^$root\/// ;
	}
	$line = $line . "cvs -d " . $serve . " add " . $comment . " $elem" ;
	&CTUDebug( "about to execute '" . $line . "'\n" ) ;
	$ret = &CTURetCode( system( $line )) ;
    }
    &CTUDebug( "out of CTCvsAdd\n" ) ;
    $ret ;
}

# ci a versioned element to the repository
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
# $_[3] = possible comment
#
# output:
# return success or failure
sub CTCvsCi {
    &CTUDebug( "in CTCvsCi\n" ) ;
    # first we need to 'login' to the repository
    local( $comment ) = &CTCvsFormatComment( $_[3] ) ;
    local( $serve ) = &CTCvsServerLine( $_[1], $_[2] ) ;
    local( $ret ) = &CTCvsLogin( $serve ) ;
    if ( $ret ) {
	# now issue the add command
	local( $root ) = &CTProjRoot( $_[1] ) ;
	local( $line ) = "" ;
	local( $elem ) = $_[0] ;
	if ( $elem =~ /^\// ) {
	    local ( $proj ) = $_[1] ;
	    $proj =~ tr/a-z/A-Z/ ;
	    $line = "cd \$" . $proj . "; " ;
	    $elem =~ s/^$root\/// ;
	}
	$line = $line . "cvs -d " . $serve . " ci " . $comment . " $elem" ;
	&CTUDebug( "about to execute '" . $line . "'\n" ) ;
	$ret = &CTURetCode( system( $line )) ;
    }
    &CTUDebug( "out of CTCvsCi\n" ) ;
    $ret ;
}

# rm a versioned element from the repository
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCvsRm {
    &CTUDebug( "in CTCvsRm\n" ) ;
    # first we need to 'login' to the repository
    local( $serve ) = &CTCvsServerLine( $_[1], $_[2] ) ;
    local( $ret ) = &CTCvsLogin( $serve ) ;
    if ( $ret ) {
	# now issue the add command
	$ret = &CTURetCode( system( "cvs -d " . $serve . " rm $_[0]\n" )) ;
    }
    &CTUDebug( "out of CTCvsRm\n" ) ;
    $ret ;
}

# make a versioned directory
# input is in:
# $_[0] = directory to create
# $_[1] = project
# $_[2] = spec line
# $_[3] = possible comment
#
# output:
# return success or failure
sub CTCvsMkdir {
    &CTUDebug( "in CTCvsMkdir\n" ) ;
    local( $ret ) = 0 ;
    # first make the dir
    $ret = &CTURetCode( system( "mkdir $_[0]\n" )) ;
    if ( $ret ) {
	# now version it
	$ret = &CTCvsAdd( $_[0], $_[1], $_[2], $_[3] ) ;
    } else {
	&CTUDebug( "could not create directory '" . $_[0] . "'\n" ) ;
	$ret = 0 ;
    }
    &CTUDebug( "out of CTCvsMkdir\n" ) ;
    $ret ;
}

# make a versioned element
# input is in:
# $_[0] = element to version
# $_[1] = project
# $_[2] = spec line
# $_[3] = possible comment
#
# output:
# return success or failure
sub CTCvsMkelem {
    &CTUDebug( "in CTCvsMkelem\n" ) ;
    # first cvs add the file
    local( $ret ) = &CTCvsAdd( $_[0], $_[1], $_[2], $_[3] ) ;
    if ( $ret ) {
	# now commit it
	$ret = &CTCvsCi( $_[0], $_[1], $_[2], $_[3] ) ;
    } else {
	&CTUDebug( "could not CVS add '" . $_[0] . "'\n" ) ;
	$ret = 0 ;
    }
    &CTUDebug( "out of CTCvsMkelem\n" ) ;
    $ret ;
}

# delta an element
# input is in:
# $_[0] = element to delta
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCvsDelta {
    &CTUDebug( "in CTCvsDelta\n" ) ;
    local( $ret ) = 0 ;
    # for lack of better idea, this is going to be just checkin for now
    if ( -d $_[0] ) {
	# we don't version directories in CVS
	$ret = 1 ;
    } else {
	$ret = &CTCvsCi( $_[0], $_[1], $_[2] ) ;
    }
    &CTUDebug( "out of CTCvsDelta\n" ) ;
    $ret ;
}

# checkout an element
# input is in:
# $_[0] = element to checkout
# $_[1] = project
# $_[2] = spec line
# $_[3] = possible comment
#
# output:
# return success or failure
sub CTCvsCheckout {
    &CTUDebug( "in CTCvsCheckout\n" ) ;
    local( $ret ) = 1 ;
    # for my limited understanding of CVS, there doesn't seem to be any
    # 'checkout' for it.
    &CTUDebug( "out of CTCvsCheckout\n" ) ;
    $ret ;
}

# checkin an element
# input is in:
# $_[0] = element to checkin
# $_[1] = project
# $_[2] = spec line
# $_[3] = possible comment
#
# output:
# return success or failure
sub CTCvsCheckin {
    &CTUDebug( "in CTCvsCheckin\n" ) ;
    local( $ret ) = 0 ;
    if ( -d $_[0] ) {
	# we don't version directories in CVS
	$ret = 1 ;
    } else {
	$ret = &CTCvsCi( $_[0], $_[1], $_[2], $_[3] ) ;
    }
    &CTUDebug( "out of CTCvsCheckin\n" ) ;
    $ret ;
}

# uncheckout an element
# input is in:
# $_[0] = element to uncheckout
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCvsUncheckout {
    &CTUDebug( "in CTCvsUncheckout\n" ) ;
    local( $ret ) = 0 ;
    if ( -d $_[0] ) {
	# we don't version directories in CVS
	$ret = 1 ;
    } else {
	$ret = &CTURetCode( system( "rm $_[0]" ) ) ;
	if ( $ret ) {
	    local( $serve ) = &CTCvsServerLine( $_[1], $_[2] ) ;
	    $ret = &CTCvsLogin( $serve ) ;
	    if ( $ret ) {
		$ret = &CTURetCode( system( "cvs -d " . $serve . " update " .
					    $_[0] )) ;
	    }
	}
    }
    &CTUDebug( "out of CTCvsUncheckout\n" ) ;
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
sub CTCvsIHave {
    &CTUDebug( "in CTCvsIHave\n" ) ;
    local( $ret ) = "" ;
    local( $proj ) = $_[0] ;
    $proj =~ tr/a-z/A-Z/ ;
    local( $line ) = "cd \$" . $proj . "; " ;
    local( $serve ) = &CTCvsServerLine( $_[0], $_[2] ) ;
    local( $ok ) = &CTCvsLogin( $serve ) ;
    if ( $ok ) {
	$line = $line . "cvs -n -d " . $serve . " update 2>/dev/null" ;
	local( $hold ) = "";
	local( *OUTPUT ) ;
	open( OUTPUT, $line . " |" ) ;
	while ( <OUTPUT> ) {
	    $hold = $hold . $_ ;
	}
	close( OUTPUT ) ;
	local( @lines ) = split( /\n/, $hold ) ;
	local( $item ) ;
	foreach $item ( @lines ) {
	    if ( $item =~ /^\?/ ) {
		# things that start with a ? are ignored
	    } elsif ( $item =~ /^cvs/ ) {
		# messages from the server are also ignored
	    } elsif ( $item =~ /^P/ ) {
		# new files are ignored
	    } elsif ( $item =~ /^U/ ) {
		# updates are ignored
	    } elsif ( $item =~ /^M/ ) {
		# here's one we modified
		local( @foo ) = split( / /, $item ) ;
		$ret = $ret . $foo[1] . "\n" ;
	    } else {
		# don't what this means, better complain
		local( @foo ) = split( / /, $item ) ;
		print STDERR "got unknown update code '" . $foo[0] .
		    "' for file '" . $foo[1] . "'\n" ;
	    }
	}
    }
    &CTUDebug( "out of CTCvsIHave\n" ) ;
    $ret ;
}

# remove an element from the repository
# input is in:
# $_[0] = element to uncheckout
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCvsRmElem {
    &CTUDebug( "in CTCvsRmElem\n" ) ;
    local( $ret ) = 0 ;
    if ( -d $_[0] ) {
	# CVS doesn't really do this.  If there are no files in the directory,
	# the next time an update -P is run, it will be deleted.
	$ret = 1 ;
    } else {
	$ret = &CTURetCode( system( "rm $_[0]" ) ) ;
	if ( $ret ) {
	    $ret = &CTCvsRm( $_[0], $_[1], $_[2] ) ;
	    if ( $ret ) {
		$ret = &CTCvsCi( $_[0], $_[1], $_[2] ) ;
	    }
	}
    }
    &CTUDebug( "out of CTCvsRmElem\n" ) ;
    $ret ;
}

# move a versioned element from one name to another
# input is in:
# $_[0] = from element
# $_[1] = to element
# $_[2] = project
# $_[3] = spec line
#
# output:
# return success or failure
sub CTCvsMv {
    &CTUDebug( "in CTCvsMv\n" ) ;
    local( $ret ) = 0 ;
    if ( -d $_[0] ) {
	# don't have code to do directories yet.  See pp 54 of the CVS book
	$ret = 0 ;
    } else {
	$ret = &CTURetCode( system( "mv $_[0] $_[1]" ) ) ;
	if ( $ret ) {
	    $ret = &CTCvsRm( $_[0], $_[2], $_[3] ) ;
	    if ( $ret ) {
		$ret = &CTCvsAdd( $_[1], $_[2], $_[3] );
		if ( $ret ) {
		    $ret = &CTCvsCi( $_[0], $_[2], $_[3] ) ;
		    if ( $ret ) {
			$ret = &CTCvsCi( $_[1], $_[2], $_[3] ) ;
		    }
		}
	    }
	}
    }
    &CTUDebug( "out of CTCvsMv\n" ) ;
    $ret ;
}

# build a list of targets
# input is in:
# $_[0] = targets
#
# output:
# return success or failure
sub CTCvsMake {
    &CTUDebug( "in CTCvsMake\n" ) ;
    local( $ret ) = 0 ;
    local( $line ) = "make " . $_[0] . "\n" ;
    $ret = &CTURetCode( system( $line )) ;
    &CTUDebug( "out of CTCvsMake\n" ) ;
    $ret ;
}

1;
