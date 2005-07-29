require "$tool/built/include/ctvspec.pl" ;
require "$tool/built/include/ctquery.pl" ;

# given a spec line, do the 'correct' setup for it
# input is in:
# $_[0] = project
# $_[1] = spec line
# $_[2] = flavor
sub CTCMSetup {
    local( $type ) = &CTSpecType( $_[1] ) ;
    if ( $type eq "vroot" ) {
	&CTUDebug( "running setup for an atria tree\n" ) ;
	if ( $ENV{"HAVE_ATRIA"} eq "yes" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    &CTAttachCCSetup( $_[0], $_[1], $_[2] ) ;
	} else {
	    &CTUDebug( "don't HAVE_ATRIA!\n" ) ;
	}
	# if we don't have atria, and it's a vroot, well..
    } elsif ( $type eq "croot" ) {
	&CTUDebug( "running setup for CVS\n" ) ;
	require "$tool/built/include/ctcvs.pl" ;
	local( $serve ) = &CTCvsServerLine( $_[0], $_[1] ) ;
	local( $thing ) = &CTCvsLogin( $serve ) ;
	if ( ! $thing ) {
	    print STDERR "CVS login failed given server line '" . $serve .
		"'\n" ;
	}
    }
    # no other types have any work that needs to be done at this time
}

# given a directory, make sure it's versioned
# input is in:
# $_[0] = directory
# $_[1] = project
# $_[2] = spec line
# $_[3] = comment (optional, "" if none)
#
# output:
# return success or failure
sub CTCMMkdir {
    &CTUDebug( "in CTCMMkdir\n" ) ;
    local( $ret ) = 0 ;
    # first check that the directory is in the project, and is not the root
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( $_[0] =~ /^\// ) {
	# starts with a /, might not be in the project we are
	if (( $_[0] =~ /^$root/ ) && ( $_[0] ne $root )) {
	    $isok = 1 ;
	} else {
	    $isok = 0 ;
	}
    } else {
	# are we sitting in the project?
	if ( $pwd =~ /^$root/ ) {
	    $isok = 1 ;
	} else {
	    $isok = 0 ;
	}
    }
    if ( $isok ) {
	# ok, it is.  Does one already exist?
	if ( -e $_[0] ) {
	    # already one there, nothing to do
	    &CTUDebug( "directory '" . $_[0] . "' already exists\n" ) ;
	    $ret = 1 ;
	} else {
	    # now switch off on how to actually do it
	    local( $type ) = &CTSpecType( $_[2] ) ;
	    if ( $type eq "vroot" ) {
		require "$tool/built/include/ctccase.pl" ;
		$ret = &CTCcaseMkdir( $_[0], $pwd, $_[3] ) ;
	    } elsif ( $type eq "root" ) {
		require "$tool/built/include/ctntool.pl" ;
		$ret = &CTNtoolMkdir( $_[0], $pwd, $_[3] ) ;
	    } elsif ( $type eq "croot" ) {
		require "$tool/built/include/ctcvs.pl" ;
		$ret = &CTCvsMkdir( $_[0], $_[1], $_[2], $_[3] ) ;
	    } else {
		print STDERR "CTCMMkdir::error! got invalid spec type '" .
		    $type . "'\n" ;
	    }
	}
    } else {
	print STDERR "directory '" . $_[0] . "' not in project '" . $_[1] .
	    "' or is the root.\n" ;
    }
    &CTUDebug( "out of CTCMMkdir\n" ) ;
    $ret ;
}

# given a file, make sure it's versioned
# input is in:
# $_[0] = file
# $_[1] = project
# $_[2] = spec line
# $_[3] = comment (optional, "" if none)
# $_[4] = eltype (optional, "" if none)
#
# output:
# return success or failure
sub CTCMMkelem {
    &CTUDebug( "in CTCMMkelem\n" ) ;
    local( $ret ) = 0;
    # first check that the directory is in the project
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $isok ) = 0 ;
    local( $pwd ) = &CTUCurrDir() ;
    # synth an eltype if there is none
    if ( ! -e $_[0] ) {
	# need it to already exist
	$isok = 0 ;
    } else {
	if ( -d $_[0] ) {
	    # wrong command for a directory
	    $isok = 0 ;
	} else {
	    if ( $_[0] =~ /^\// ) {
		# starts with a /, might not be in the project we are
		if ( $_[0] =~ /^$root/ ) {
		    $isok = 1 ;
		} else {
		    $isok = 0 ;
		}
	    } else {
		# are we sitting in the project?
		if ( $pwd =~ /^$root/ ) {
		    $isok = 1 ;
		} else {
		    $isok = 0 ;
		}
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[2] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseMkelem( $_[0], $pwd, $_[3], $_[4] ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolMkelem( $_[0], $pwd, $_[3], $_[4] ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsMkelem( $_[0], $_[1], $_[2], $_[3] ) ;
	} else {
	    print STDERR "CTCMMkelem::error! got invalid spec type '" .
		$type . "'\n" ;
	}
    }
    &CTUDebug( "out of CTCMMkelem\n" ) ;
    $ret ;
}

# given an element, delta it in
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCMDelta {
    &CTUDebug( "in CTCMDelta\n" ) ;
    local( $ret ) = 0 ;
    # first check that the element is in the project
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( ! -e $_[0] ) {
	# can't delta something that doesn't exist
	$isok = 0 ;
    } else {
	if ( $_[0] =~ /^\// ) {
	    # starts with a /, might not be in the project we are
	    if ( $_[0] =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	} else {
	    # are we sitting in the project?
	    if ( $pwd =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[2] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseDelta( $_[0] ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolDelta( $_[0] ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsDelta( $_[0], $_[1], $_[2] ) ;
	} else {
	    print STDERR "CTCMDelta::error! got invalid spec type '" . $type .
		"'\n" ;
	}
    } else {
	&CTUDebug( "failed delta pre-checks\n" ) ;
    }
    &CTUDebug( "out of CTCMDelta\n" ) ;
    $ret ;
}

# given an element, check it out
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
# $_[3] = comment (optional, "" if none)
#
# output:
# return success or failure
sub CTCMCheckout {
    &CTUDebug( "in CTCMCheckout\n" ) ;
    local( $ret ) = 0 ;
    # first check that the element is in the project
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( ! -e $_[0] ) {
	# can't checkout something that doesn't exist
	$isok = 0 ;
    } else {
	if ( $_[0] =~ /^\// ) {
	    # starts with a /, might not be in the project we are
	    if ( $_[0] =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	} else {
	    # are we sitting in the project?
	    if ( $pwd =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[2] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseCheckout( $_[0], $_[3] ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolCheckout( $_[0], $_[3] ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsCheckout( $_[0], $_[1], $_[2], $_[3] ) ;
	} else {
	    print STDERR "CTCMCheckout::error! got invalid spec type '" .
		$type . "'\n" ;
	}
    }
    &CTUDebug( "out of CTCMCheckout\n" ) ;
    $ret ;
}

# given an element, check it in
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
# $_[3] = comment (optional, "" if none)
#
# output:
# return success or failure
sub CTCMCheckin {
    &CTUDebug( "in CTCMCheckin\n" ) ;
    local( $ret ) = 0 ;
    # first check that the element is in the project
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( ! -e $_[0] ) {
	# can't checkin something that doesn't exist
	$isok = 0 ;
    } else {
	if ( $_[0] =~ /^\// ) {
	    # starts with a /, might not be in the project we are
	    if ( $_[0] =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	} else {
	    # are we sitting in the project?
	    if ( $pwd =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[2] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseCheckin( $_[0], $_[3] ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolCheckin( $_[0], $_[3] ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsCheckin( $_[0], $_[1], $_[2], $_[3] ) ;
	} else {
	    print STDERR "CTCMCheckin::error! got invalid spec type '" .
		$type . "'\n" ;
	}
    }
    &CTUDebug( "out of CTCMCheckin\n" ) ;
    $ret ;
}

# given an element, uncheck it out
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCMUncheckout {
    &CTUDebug( "in CTCMUncheckout\n" ) ;
    local( $ret ) = 0 ;
    # first check that the element is in the project
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( ! -e $_[0] ) {
	# can't uncheckout something that doesn't exist
	$isok = 0 ;
    } else {
	if ( $_[0] =~ /^\// ) {
	    # starts with a /, might not be in the project we are
	    if ( $_[0] =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	} else {
	    # are we sitting in the project?
	    if ( $pwd =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[2] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseUncheckout( $_[0] ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolUncheckout( $_[0] ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsUncheckout( $_[0], $_[1], $_[2] ) ;
	} else {
	    print STDERR "CTCMUncheckout::error! got invalid spec type '" .
		$type . "'\n" ;
	}
    }
    &CTUDebug( "out of CTCMUncheckout\n" ) ;
    $ret ;
}

# figure out what all I have checked out in a project
# input is in:
# $_[0] = project
# $_[1] = flavor
# $_[2] = spec line
#
# output:
# return a \n serperated list of elements checked out
sub CTCMIHave {
    &CTUDebug( "in CTCMIHave\n" ) ;
    local( $ret ) = "" ;
    local( $type ) = &CTSpecType( $_[2] ) ;
    if ( $type eq "vroot" ) {
	require "$tool/built/include/ctccase.pl" ;
	$ret = &CTCcaseIHave( $_[0], $_[1], $_[2] ) ;
    } elsif ( $type eq "root" ) {
	require "$tool/built/include/ctntool.pl" ;
	$ret = &CTNtoolIHave( $_[0], $_[1], $_[2] ) ;
    } elsif ( $type eq "croot" ) {
	require "$tool/built/include/ctcvs.pl" ;
	$ret = &CTCvsIHave( $_[0], $_[1], $_[2] ) ;
    } else {
	print STDERR "CTCMIHave::error! got invalid spec type '" . $type .
	    "'\n" ;
    }
    &CTUDebug( "out of CTCMIHave\n" ) ;
    $ret ;
}

# given an element, remove it from the repository
# input is in:
# $_[0] = element
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCMRmElem {
    &CTUDebug( "in CTCMRmElem\n" ) ;
    local( $ret ) = 0 ;
    # first check that the element is in the project
    local( $flav ) = &CTQueryProj( $_[1] ) ;
    local( $root ) = &CTComputeRoot( $_[1], $flav, $_[2] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( ! -e $_[0] ) {
	# can't rmname something that doesn't exist
	$isok = 0 ;
    } else {
	if ( $_[0] =~ /^\// ) {
	    # starts with a /, might not be in the project we are
	    if ( $_[0] =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	} else {
	    # are we sitting in the project?
	    if ( $pwd =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[2] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseRnElem( $_[0], $pwd ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolRmElem( $_[0], $pwd ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsRmElem( $_[0], $_[1], $_[2] ) ;
	} else {
	    print STDERR "CTCMRmElem::error! got invalid spec type '" .
		$type . "'\n" ;
	}
    }
    &CTUDebug( "out of CTCMRmElem\n" ) ;
    $ret ;
}

# move an element from one name to another
# input is in:
# $_[0] = from element
# $_[1] = to element
# $_[2] = project
# $_[3] = spec line
#
# output:
# return success or failure
sub CTCMMv {
    &CTUDebug( "in CTCMMv\n" ) ;
    local( $ret ) = 0 ;
    # first check that the from and to are in the project
    local( $flav ) = &CTQueryProj( $_[2] ) ;
    local( $root ) = &CTComputeRoot( $_[2], $flav, $_[3] ) ;
    local( $pwd ) = &CTUCurrDir() ;
    local( $isok ) = 0 ;
    if ( $_[0] =~ /^\// ) {
	# starts with a /, might not be in the project we are
	if ( $_[0] =~ /^$root/ ) {
	    $isok = 1 ;
	} else {
	    $isok = 0 ;
	}
    } else {
	# are we sitting in the project?
	if ( $pwd =~ /^$root/ ) {
	    $isok = 1 ;
	} else {
	    $isok = 0 ;
	}
    }
    if ( $isok ) {
	if ( $_[1] =~ /^\// ) {
	    # starts with a /, might not be in the project we are
	    if ( $_[1] =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	} else {
	    # are we sitting in the project?
	    if ( $pwd =~ /^$root/ ) {
		$isok = 1 ;
	    } else {
		$isok = 0 ;
	    }
	}
    }
    if ( $isok ) {
	# now switch off on how to actually do the work
	local( $type ) = &CTSpecType( $_[3] ) ;
	if ( $type eq "vroot" ) {
	    require "$tool/built/include/ctccase.pl" ;
	    $ret = &CTCcaseMv( $_[0], $_[1], $pwd ) ;
	} elsif ( $type eq "root" ) {
	    require "$tool/built/include/ctntool.pl" ;
	    $ret = &CTNtoolMv( $_[0], $_[1], $pwd ) ;
	} elsif ( $type eq "croot" ) {
	    require "$tool/built/include/ctcvs.pl" ;
	    $ret = &CTCvsMv( $_[0], $_[1], $_[2], $_[3] ) ;
	} else {
	    print STDERR "CTCMMv::error! got invalid spec type '" .
		$type . "'\n" ;
	}
    }
    &CTUDebug( "out of CTCMMv\n" ) ;
    $ret ;
}

# give a list of targets, build them
# input is in:
# $_[0] = targets
# $_[1] = project
# $_[2] = spec line
#
# output:
# return success or failure
sub CTCMMake {
    &CTUDebug( "in CTCMMake\n" ) ;
    local( $ret ) = 0 ;
    # now switch off on how to actually do the work
    local( $type ) = &CTSpecType( $_[2] ) ;
    if ( $type eq "vroot" ) {
	require "$tool/built/include/ctccase.pl" ;
	$ret = &CTCcaseMake( $_[0] ) ;
    } elsif ( $type eq "root" ) {
	require "$tool/built/include/ctntool.pl" ;
	$ret = &CTNtoolMake( $_[0] ) ;
    } elsif ( $type eq "croot" ) {
	require "$tool/built/include/ctcvs.pl" ;
	$ret = &CTCvsMake( $_[0] ) ;
    } else {
	print STDERR "CTCMMake::error! got invalid spec type '" . $type .
	    "'\n" ;
    }
    &CTUDebug( "out of CTCMMake\n" ) ;
    $ret ;
}

1;
