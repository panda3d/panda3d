require "$tool/built/include/ctutils.pl" ;

# read a .vspec file into a map
# $_[0] = project
# on exit $ctvspecs{} will contain the data
#
# vspec format:
#   tag:type:other data
#
#   type: ref, root, vroot, croot
#   other data:
#     ref: name=_____ - required, take to refference
#     root: path=_____ - required, path of tree root
#     vroot: name=_____ - optional, name of view to use (if not tag)
#     croot: path=_____ - required, local path of tree root
#            server=_____ - required, CVS server string, ',' for ':'

sub CTReadVSpec {
    &CTUDebug( "reading vspec file for project " . $_[0] . "\n" ) ;
    local( $ret ) = "" ;
    local( $thisproj ) = $_[0] ;
    if ( -e "$ctvspec_path/$thisproj.vspec" ) {
        %ctvspecs = () ;
        local( *SPECFILE ) ;
        open( SPECFILE, "<$ctvspec_path/$thisproj.vspec" ) ;
        local( @partlist ) ;
        while ( $_ = <SPECFILE> ) {
            s/\n$// ;
            @partlist = split( /\#/ ) ;
            $_ = $partlist[0] ;
            if ( $_ ne "" ) {
                @partlist = split( /:/ );
                local( $tag ) = $partlist[0] ;
                shift( @partlist ) ;
                local( $spec ) = join( ":", @partlist ) ;
                if ( &CTValidateSpec( $spec ) ) {
                    $ctvspecs{$tag} = $spec ;
                    if ( $ctdebug ) {
                        print STDERR "tag(" . $tag . ") = " . $spec . "\n" ;
                    }
                }
            }
        }
        close( SPECFILE ) ;
        $ctvspec_read = $_[0] ;
    } else {
        print STDERR "CTReadVSpec: cannot locate '$ctvspec_path/$thisproj.vspec'\n" ;
        print STDERR "(did you forget to run the \$WINTOOLS/cp_vspec script?)\n" ;
    }
}

# given a spec line return it's type
# $_[0] = spec line

sub CTSpecType {
    local( @speclist ) = split( /:/, $_[0] ) ;
    $speclist[0] ;
}

# given a spec line return it's options if any
# $_[0] = spec line

sub CTSpecOptions {
    local( @speclist ) = split( /:/, $_[0] ) ;
    shift( @speclist ) ;
    join( ":", @speclist ) ;
}

# given the options part of a spec line, find a given option
# $_[0] = options line
# $_[1] = desired option

sub CTSpecFindOption {
    local( $ret ) = "" ;
    local( @options ) = split( /:/, $_[0] ) ;
    local( $item ) ;
    local( @itemlist ) ;
    foreach $item ( @options ) {
        @itemlist = split( /=/, $item ) ;
        if ( $itemlist[0] eq $_[1] ) {
            $ret = $itemlist[1] ;
        }
    }
    $ret ;
}

# resolve a final spec line for a given flavor
# $_[0] = project
# $_[1] = flavor

sub CTResolveSpec {
    &CTUDebug( "in CTResolveSpec\n" ) ;
    local( $proj ) = $_[0] ;
    $proj =~ tr/A-Z/a-z/ ;
    if ( $ctvspec_read ne $proj ) {
        &CTReadVSpec( $proj ) ;
    }
    local( $spec ) = $ctvspecs{$_[1]} ;
    local( $ret ) = "" ;
    if ( $spec ne "" ) {
        local( $type ) = &CTSpecType( $spec ) ;
        local( @speclist ) = split( /:/, &CTSpecOptions( $spec ) ) ;
        if ( $type eq "ref" ) {
            local( @optionlist ) = split( /=/, $speclist[0] ) ;
            if ( $optionlist[0] ne "name" ) {
                print STDERR "bad data attached to flavor " . $_[1] .
                    " of project " . $proj . "\n" ;
            } else {
                local( $tmp ) = &CTUShellEval( $optionlist[1] ) ;
                if ( $ctdebug ) {
                    print STDERR "resolved a 'ref' to " . $tmp .
                        ", recuring\n" ;
                }
                $ret = &CTResolveSpec( $proj, $tmp ) ;
            }
        } else {
            $ret = $spec ;
        }
    }
    if ( $ret eq "" ) {
        print STDERR "unknown flavor " . $_[1] . " of project " . $proj .
            "\n" ;
    }
    &CTUDebug( "out of CTResolveSpec\n" ) ;
    $ret ;
}

# resolve the final name for a given flavor
# $_[0] = project
# $_[1] = flavor

sub CTResolveSpecName {
    &CTUDebug( "in CTResolveSpecName\n" ) ;
    local( $proj ) = $_[0] ;
    $proj =~ tr/A-Z/a-z/ ;
    if ( $ctvspec_read ne $proj ) {
        &CTReadVSpec( $proj ) ;
    }
    local( $spec ) = $ctvspecs{$_[1]} ;
    local( $ret ) = $_[1] ;
    if ( $spec ne "" ) {
        local( $type ) = &CTSpecType( $spec ) ;
        local( @speclist ) = split( /:/, &CTSpecOptions( $spec ) ) ;
        if ( $type eq "ref" ) {
            local( @optionlist ) = split( /=/, $speclist[0] ) ;
            if ( $optionlist[0] ne "name" ) {
                print STDERR "bad data attached to flavor " . $_[1] .
                    " of project " . $proj . "\n" ;
            } else {
                local( $tmp ) = &CTUShellEval( $optionlist[1] ) ;
                if ( $ctdebug ) {
                    print STDERR "resolved a 'ref' to " . $tmp .
                        ", recuring\n" ;
                }
                $ret = &CTResolveSpecName( $proj, $tmp ) ;
            }
        }
    }
    if ( $ret eq "" ) {
        print STDERR "unknown flavor " . $_[1] . " of project " . $proj .
            "\n" ;
    }
    &CTUDebug( "out of CTResolveSpecName\n" ) ;
    $ret ;
}

# validate a spec line
# $_[0] = spec line

sub CTValidateSpec {
    local( $ret ) = 0 ;
    local( $type ) = &CTSpecType( $_[0] ) ;
    local( @speclist ) = split( /:/, &CTSpecOptions( $_[0] ) ) ;
    local( $have_error ) = 0 ;
    local( $item ) ;
    local( @itemlist ) ;
    if ( $type eq "ref" ) {
        local( $have_name ) = 0 ;
        foreach $item ( @speclist ) {
            @itemlist = split( /=/, $item ) ;
            if ( $itemlist[0] eq "name" ) {
                if ( $have_name ) {
                    $have_error = 1;
                    &CTUDebug( "multiple name options on 'ref'\n" ) ;
                }
                $have_name = 1;
            } else {
                &CTUDebug( "invalid option on 'ref' = " . $item . "\n" ) ;
                $have_error = 1 ;
            }
        }
        if ( ! $have_error ) {
            if ( $have_name ) {
                $ret = 1 ;
            }
        }
    } elsif ( $type eq "root" ) {
        local( $have_path ) = 0 ;
        foreach $item ( @speclist ) {
            @itemlist = split( /=/, $item ) ;
            if ( $itemlist[0] eq "path" ) {
                if ( $have_path ) {
                    $have_error = 1 ;
                    &CTUDebug( "multiple path options on 'root'\n" ) ;
                }
                $have_path = 1 ;
            } else {
                &CTUDebug( "invalid option on 'root' = " . $item . "\n" ) ;
                $have_error = 1 ;
            }
        }
        if ( ! $have_error ) {
            if ( $have_path ) {
                $ret = 1 ;
            }
        }
    } elsif ( $type eq "vroot" ) {
        local( $have_name ) = 0 ;
        foreach $item ( @speclist ) {
            @itemlist = split( /=/, $item ) ;
            if ( $itemlist[0] eq "name" ) {
                if ( $have_name ) {
                    $have_error = 1 ;
                    &CTUDebug( "multiple name options on 'vroot'\n" ) ;
                }
                $have_name = 1 ;
            } else {
                &CTUDebug( "invalid option on 'vroot' = " . $item . "\n" ) ;
                $have_error = 1 ;
            }
        }
        if ( ! $have_error ) {
            $ret = 1 ;
        }
    } elsif ( $type eq "croot" ) {
        local( $have_path ) = 0 ;
        local( $have_server ) = 0 ;
        foreach $item ( @speclist ) {
            @itemlist = split( /=/, $item ) ;
            if ( $itemlist[0] eq "path" ) {
                if ( $have_path ) {
                    $have_error = 1 ;
                    &CTUDebug( "multiple path options on 'croot'\n" ) ;
                }
                $have_path = 1 ;
            } elsif ( $itemlist[0] eq "server" ) {
                if ( $have_server ) {
                    $have_error = 1 ;
                    &CTUDebug( "multiple server options on 'croot'\n" ) ;
                }
                $have_server = 1 ;
            } else {
                &CTUDebug( "invalid option on 'croot' = " . $item . "\n" ) ;
                $have_error = 1 ;
            }
        }
        if ( ! $have_error ) {
            if ( $have_path && $have_server ) {
                $ret = 1 ;
            }
        }
    } else {
        &CTUDebug( "unknow spec type '" . $speclist[0] . "'\n" ) ;
    }
    $ret ;
}

# get a list of all projects

sub CTListAllProjects {
    &CTUDebug( "in CTListAllProjects\n" ) ;
    local( $ret ) = "" ;
    local( $done ) = 0 ;
    local( *DIRFILES ) ;
    open( DIRFILES, "(cd $ctvspec_path ; /bin/ls -1 *.vspec ; echo blahblah) |" ) ;
    while ( ! $done ) {
        $_ = <DIRFILES> ;
        s/\n$// ;
        if ( $_ eq "blahblah" ) {
            $done = 1 ;
        } else {
            s/.vspec$// ;
            if ( $_ ne "" ) {
                if ( $ret eq "" ) {
                    $ret = $_ ;
                } else {
                    $ret = $ret . " " . $_ ;
                }
            }
        }
    }
    close( DIRFILES ) ;
    &CTUDebug( "final list of projects '" . $ret . "'\n" .
               "out of CTListAllProjects\n" ) ;
    $ret ;
}

# list all flavors of a project
# $_[0] = project

sub CTListAllFlavors {
    &CTUDebug( "in CTListAllFlavors\n" ) ;
    local( $proj ) = $_[0] ;
    $proj =~ tr/A-Z/a-z/ ;
    if ( $ctvspec_read ne $proj ) {
        &CTReadVSpec( $proj ) ;
    }
    local( $ret ) = "";
    local( $item ) ;
    foreach $item ( keys %ctvspecs ) {
        if ( $ret eq "" ) {
            $ret = $item ;
        } else {
            $ret = $ret . " " . $item ;
        }
    }
    &CTUDebug( "out of CTListAllFlavors\n" ) ;
    $ret ;
}

# given a project and a spec, determine the local root of the project
# $_[0] = project
# $_[1] = flavor
# $_[2] = spec line

sub CTComputeRoot {
    &CTUDebug( "in CTComputeRoot\n" ) ;
    local( $proj ) = $_[0] ;
    $proj =~ tr/A-Z/a-z/ ;
    if ( $ctvspec_read ne $proj ) {
        &CTReadVSpec( $proj ) ;
    }
    local( $ret ) = "" ;
    local( $type ) = &CTSpecType( $_[2] ) ;
    local( $options ) = &CTSpecOptions( $_[2] ) ;
    local( $vname ) = &CTResolveSpecName( $proj, $_[1] ) ;
    &CTUDebug( "type = '" . $type . "' with options '" . $options . "'\n" ) ;
    if ( $type eq "root" ) {
        $ret = &CTSpecFindOption( $options, "path" ) ;
    } elsif ( $type eq "vroot" ) {
        local( $name ) = &CTSpecFindOption( $options, "name" ) ;
        if ( $name ne "" ) {
            $ret = "/view/$name/vobs/$proj" ;
        } else {
            $ret = "/view/$vname/vobs/$proj" ;
        }
    } elsif ( $type eq "croot" ) {
        $ret = &CTSpecFindOption( $options, "path" ) ;
    } elsif ( $ctdebug) {
        print STDERR "unknown flavor type '" . $type . "'\n" ;
    }
    &CTUDebug( "returning '" . $ret . "'\n" ) ;
    &CTUDebug( "out of CTComputeRoot\n" ) ;
    $ret ;
}

%ctvspecs = () ;
$ctvspec_read = "" ;

1;
