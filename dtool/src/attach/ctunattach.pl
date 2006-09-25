require "$tool/built/include/ctquery.pl" ;

$shell_type = "csh" ;
if ( $ENV{"SHELL_TYPE"} ne "" ) {
    if ( $ENV{"SHELL_TYPE"} eq "sh" ) {
	$shell_type = "sh" ;
    }
}

# remove a value from a variable.  If it is the only thing remaining in the
# variable, add it to the unset list.
# input is in:
# $_[0] = variable
# $_[1] = value
#
# output is in:
# %newenv = an image of how we want the environment to be
# @unset = a list of variables to unset
sub CTUnattachMod {
    &CTUDebug( "in CTUnattachMod\n" ) ;
    local( $done ) = 0 ;
    # if we didn't get any data, nothing really to do
    if ( $_[0] eq "" ) { $done = 1 ; }
    if ( $_[1] eq "" ) { $done = 1 ; }
    # if the variable is already set to be unset, nothing really to do
    if ( join( " ", @unset ) =~ /$_[0]/ ) { $done = 1 ; }
    # if the variable isn't in newenv, move it there, if it's empty mark it
    # for unsetting
    if ( $newenv{$_[0]} eq "" ) {
	$newenv{$_[0]} = &CTSpoolEnv( $_[0] ) ;
	if ( $newenv{$_[0]} eq "" ) {
	    push( @unset, $_[0] ) ;
	    delete $newenv{$_[0]} ;
	    $done = 1 ;
	}
    }
    # if the value does not appear in the variable, nothing really to do
    if ( ! ( $newenv{$_[0]} =~ /$_[1]/ ) ) { $done = 1 ; }
    # now down to the real work
    if ( ! $done ) {
	# if the variable is exactly the value, mark it for unsetting
	if ( $newenv{$_[0]} eq $_[1] ) {
	    push( @unset, $_[0] ) ;
	    delete $newenv{$_[0]} ;
	} elsif ( $newenv{$_[0]} =~ / $_[1]/ ) {
	    local( $tmp ) = $newenv{$_[0]} ;
	    $tmp =~ s/ $_[1]// ;
	    $newenv{$_[0]} = $tmp ;
	} elsif ( $newenv{$_[0]} =~ /$_[1] / ) {
	    local( $tmp ) = $newenv{$_[0]} ;
	    $tmp =~ s/$_[1] // ;
	    $newenv{$_[0]} = $tmp ;
	} else {
	    print STDERR "ERROR: variable '" . $_[0] . "' contains '" .
		$_[1] . "' (in '" . $newenv{$_[0]} .
	        "'), but I am too stupid to figure out how to remove it.\n" ;
	}
    }
}

# given the project and flavor, build the lists of variables to set/modify
# input is in:
# $_[0] = project
# $_[1] = flavor
#
# output is in:
# return value is config line
# %newenv      = an image of what we want the environment to look like
# @unset       = list of variables to be unset
# %envsep      = seperator
# %envcmd      = set or setenv
# %envpostpend = flag that variable should be postpended
sub CTUnattachCompute {
   &CTUDebug( "in CTUnattachCompute\n" ) ;
   local( $flav ) = $_[1] ;
   local( $spec ) = &CTResolveSpec( $_[0], $flav ) ;
   local( $root ) = &CTComputeRoot( $_[0], $flav, $spec ) ;

   if ( $spec ne "" ) {
      local( $proj ) = $_[0] ;
      $proj =~ tr/a-z/A-Z/ ;
      local( $item ) ;

      # since we don't have to worry about sub-attaches, it doesn't matter
      # if we scan the .init file first or not.  So we won't.
      &CTUDebug( "extending paths\n" ) ;

      $item = $root . "/built/bin" ;
      &CTUnattachMod( "PATH", $item ) ;
      $item = $root . "/built/lib" ;
      if ( $ENV{"PENV"} eq "WIN32" ) {
	  &CTUnattachMod( "PATH", $item ) ;
      }
      &CTUnattachMod( "LD_LIBRARY_PATH", $item ) ;
      &CTUnattachMod( "DYLD_LIBRARY_PATH", $item ) ;
      #$item = $root . "/src/all" ;
      #&CTUnattachMod( "CDPATH", $item ) ;
      $item = $root . "/built/include" ;
      &CTUnattachMod( "CT_INCLUDE_PATH", $item ) ;
      $item = $root . "/built/etc" ;
      &CTUnattachMod( "ETC_PATH", $item ) ;
      $item = $proj . ":" . $flav ;
      &CTUnattachMod( "CTPROJS", $item ) ;
      push( @unset, $proj ) ;

      if ( -e "$root/built/etc/$_[0].init" ) {
	 &CTUDebug( "scanning $_[0].init file\n" ) ;
	 local( @linesplit ) ;
	 local( $linetmp ) ;
	 local( $loop );
	 local( *INITFILE ) ;
	 if ( -x "$root/built/etc/$_[0].init" ) {
	    open( INITFILE, "$root/built/etc/$_[0].init $_[0] $_[1] $root |" ) ;
	 } else {
	    open( INITFILE, "< $root/built/etc/$_[0].init" ) ;
	 }
	 while ( <INITFILE> ) {
	    s/\n$// ;
	    @linesplit = split( /\#/ ) ;
	    $_ = $linesplit[0] ;
	    if ( $_ =~ /^MODABS/ ) {
	       @linesplit = split ;
	       $linetmp = $linesplit[1] ;
	       shift( @linesplit ) ;
	       shift( @linesplit ) ;
	       foreach $loop ( @linesplit ) {
		   &CTUnattachMod( $linetmp, $loop ) ;
	       }
	    } elsif ( $_ =~ /^MODREL/ ) {
	       @linesplit = split ;
	       $linetmp = $linesplit[1] ;
	       shift( @linesplit ) ;
	       shift( @linesplit ) ;
	       foreach $loop ( @linesplit ) {
		   &CTUnattachMod( $linetmp, $root . "/" . $loop ) ;
	       }
	    } elsif ( $_ =~ /^SETABS/ ) {
	       @linesplit = split ;
	       $linetmp = $linesplit[1] ;
	       push( @unset, $linetmp ) ;
	    } elsif ( $_ =~ /^SETREL/ ) {
	       @linesplit = split ;
	       $linetmp = $linesplit[1] ;
	       push( @unset, $linetmp ) ;
	    } elsif ( $_ =~ /^SEP/ ) {
	       @linesplit = split ;
	       $envsep{$linesplit[1]} = $linesplit[2] ;
	    } elsif ( $_ =~ /^CMD/ ) {
	       @linesplit = split ;
	       $envcmd{$linesplit[1]} = $linesplit[2] ;
	    } elsif ( $_ =~ /^DOCSH/ ) {
	       &CTUDebug( "ignoring DO command in .init file\n" ) ;
	    } elsif ( $_ =~ /^DOSH/ ) {
	       &CTUDebug( "ignoring DO command in .init file\n" ) ;
	    } elsif ( $_ =~ /^DO/ ) {
	       &CTUDebug( "ignoring DO command in .init file\n" ) ;
	    } elsif ( $_ =~ /^POSTPEND/ ) {
	       @linesplit = split ;
	       $envpospend{$linesplit[1]} = 1 ;
	    } elsif ( $_ =~ /^ATTACH/ ) {
	       &CTUDebug( "ignoring ATTACH command in .init file\n" ) ;
	    } else {
	       print STDERR "Unknown .init directive '$_'\n" ;
	    }
	 }
	 close( INITFILE ) ;
      }
   }
   &CTUDebug( "out of CTUnattachCompute\n" ) ;
   $spec ;
}

# write a script to setup the environment
# Input is:
# $_[0] = filename
sub CTUnattachWriteScript {
   &CTUDebug( "in CTAttachWriteScript\n" ) ;
   local( *OUTFILE ) ;
   open( OUTFILE, ">$_[0]" ) ;
   print OUTFILE "#!/bin/" . $shell_type . " -f\n" ;
   local( $item ) ;
   #local( $unsetcdpath ) = 0 ;
   #local( $modcdpath ) = 0 ;

   foreach $item ( @unset ) {
       #if ( $item eq "CDPATH" ) { $unsetcdpath = 1 ; }

       if ( $shell_type eq "sh" ) {
	   print OUTFILE "$item=\n" ;
	   if ( $envcmd{$item} ne "set" ) {
	       print OUTFILE "export $item\n" ;
	   }
       } else {
	   if ( $envcmd{$item} ne "" ) {
	       print OUTFILE "un" . $envcmd{$item} . " $item\n" ;
	   } else {
	       print OUTFILE "unsetenv $item\n" ;
	   }
       }
   }
   foreach $item ( keys %newenv ) {
       #if ( $item eq "CDPATH" ) { $modcdpath = 1 ; }

       local( $sep ) = " " ;
       if ( $envsep{$item} ne "" ) {
	   $sep = $envsep{$item} ;
       }
       local( @splitlist ) = split( / +/, $newenv{$item} ) ;
       local( $outval ) = join( $sep, @splitlist ) ;

       if ( $shell_type eq "sh" ) {
	   print OUTFILE "$item=\"" . $outval . "\"\n" ;
	   if ( $envcmd{$item} ne "set" ) {
	       print OUTFILE "export $item\n" ;
	   }
       } else {
	   if ( $envcmd{$item} ne "" ) {
	       PRINT OUTFILE $envcmd{$item} . " $item " ;
	       if ( $envcmd{$item} eq "set" ) {
		   print OUTFILE " = ( " ;
	       }
	       print OUTFILE $outval ;
	       if ( $envcmd{$item} eq "set" ) {
		   print OUTFILE ")" ;
	       }
	       print OUTFILE "\n" ;
	   } else {
	       print OUTFILE "setenv $item \"$outval\"\n" ;
	   }
       }
   }
   #if ( $unsetcdpath ) {
   #    if ( $shell_type ne "sh" ) {
   #	   print OUTFILE "unset cdpath\n" ;
   #    }
   #} elsif ( $modcdpath ) {
   #    if ( $shell_type ne "sh" ) {
   #	   print OUTFILE "set cdpath = ( \$" . "CDPATH )\n" ;
   #    }
   #}

   if (! $ctdebug) {
      print OUTFILE "rm -f $_[0]\n" ;
   } else {
      print STDERR "no self-destruct script '" . $_[0] . "'\n" ;
   }
   close( OUTFILE ) ;
   &CTUDebug( "out of CTUnattachWriteScript\n" ) ;
}

1;
