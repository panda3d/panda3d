require "$tool/built/include/ctutils.pl" ;

$shell_type = "csh" ;
if ( $ENV{"SHELL_TYPE"} ne "" ) {
    if ( $ENV{"SHELL_TYPE"} eq "sh" ) {
    $shell_type = "sh" ;
    }
}

$docnt = 0 ;
@attachqueue = () ;

require "$tool/built/include/ctquery.pl" ;

# force set a variable in the 'new' environment
# input is in:
# $_[0] = variable
# $_[1] = value
#
# output is in:
# %newenv = variable marked to be set to value
sub CTAttachSet {
    if ( ( $_[0] ne "" ) && ( $_[1] ne "" ) ) {
    &CTUDebug( "setting " . $_[0] . " to '" . $_[1] . "'\n" ) ;
    $newenv{$_[0]} = $_[1] ;
    }
}

# get a variable from the environment and split it out to unified format
# (ie: space separated)
# input is in:
# $_[0] = variable to get
#
# output is in:
# string returned with value
sub CTSpoolEnv {
    local( $ret ) = $ENV{$_[0]} ;
    if ( $envsep{$_[0]} ne "" ) {
    local( @splitlist ) = split( $envsep{$_[0]}, $ret );
    $ret = join( " ", @splitlist ) ;
    }
    $ret ;
}

# modify a possibly existing variable to have a value in the 'new' environment
# input is in:
# $_[0] = variable
# $_[1] = value
# $_[2] = root
# $_[3] = project
#
# output is in:
# %newenv = variable adjusted to have the new value
sub CTAttachMod {
    &CTUDebug( "in CTAttachMod\n" ) ;
    if ( $_[0] eq "CTPROJS" ) {
    # as part of the system, this one is special
    &CTUDebug( "doing a mod on $CTPROJS\n" ) ;
    if ( $newenv{$_[0]} eq "" ) {
        $newenv{$_[0]} = $ENV{$_[0]} ;
    }
    local( $proj ) = $_[3] ;
    $proj =~ tr/A-Z/a-z/ ;
    local( $curflav ) = &CTQueryProj( $proj ) ;
    if ( $curflav ne "" ) {
        local( $tmp ) = $_[3] . ":" . $curflav ;
        if ( $newenv{$_[0]} =~ /$tmp/ ) {
        local( $hold ) = $newenv{$_[0]} ;
        $hold =~ s/$tmp/$_[1]/ ;
        &CTUDebug( "already attached to " . $_[3] . " changing '" .
               $tmp . "' to '" . $_[1] . "' yielding '" . $hold .
               "'\n" ) ;
        $newenv{$_[0]} = $hold ;
        } else {
        &CTUDebug( "prepending '" . $_[1] . "' to CTPROJS\n" ) ;
        $newenv{$_[0]} = $_[1] . " " . $newenv{$_[0]} ;
        }
    } else {
        &CTUDebug( "writing '" . $_[1] . "' to CTPROJS\n" ) ;
        if ( $newenv{$_[0]} eq "" ) {
        $newenv{$_[0]} = $_[1] ;
        } else {
        $newenv{$_[0]} = $_[1] . " " . $newenv{$_[0]} ;
        }
    }
    } elsif ( ( $_[0] ne "" ) && ( $_[1] ne "" ) ) {
    local( $dosimple ) = 0 ;
    if ( $newenv{$_[0]} eq "" ) {
        # not in our 'new' environment yet, add it.
        # may still be empty
        $newenv{$_[0]} = &CTSpoolEnv( $_[0] ) ;
    }
    if ( ! ( $newenv{$_[0]} =~ /$_[1]/ )) {
        &CTUDebug( "'" . $_[1] . "' exists in " . $_[0] .
               " testing for simple modification\n" ) ;
        # if it's in there already, we're done before we started.
        if ( $_[1] =~ /^$_[2]/ ) {
        &CTUDebug( "new value contains root '" . $_[2] .
               "', may not be able to do simple edit\n" ) ;
        # damn, might need to do an in-place edit
        local( $curroot ) = $ENV{$_[3]} ;
        &CTUDebug( "current root for '" . $_[3] . "' is '" .
               $curroot . "'\n" ) ;
        if ( $curroot eq "" ) {
            &CTUDebug( "can do simple edit\n" ) ;
            $dosimple = 1 ;
        } else {
            local( $test ) = $_[1] ;
            $test =~ s/^$_[2]// ;
            $test = $curroot . $test ;
            if ( $newenv{$_[0]} =~ /$test/ ) {
            # there it is.  in-place edit
            local( $foo ) = $newenv{$_[0]} ;
            $foo =~ s/$test/$_[1]/ ;
            &CTUDebug( "doing in-place edit on " . $_[0] .
                   " changing '" . $test . "' to '" .
                   $_[1] . "' yielding '" . $foo . "'\n" ) ;
            $newenv{$_[0]} = $foo ;
            } else {
            &CTUDebug( "'" . $test . "' did not appear in $_[0]." .
                                   "  Simple edit\n" ) ;
            $dosimple = 1 ;
            }
        }
        } else {
        &CTUDebug( "new value does not contain root '" . $_[2] .
               "', can do simple edit\n" ) ;
        # don't have to sweat in-place edits
        $dosimple = 1 ;
        }
    }
    if ( $dosimple ) {
        if ( $newenv{$_[0]} eq "" ) {
        &CTUDebug( "no pre-existing value in " . $_[0] .
               " setting it to '" . $_[1] . "'\n" ) ;
        $newenv{$_[0]} = $_[1] ;
        } elsif ( $envpostpend{$_[0]} ) {
        &CTUDebug( "post-pending '" . $_[1] . "' to " . $_[0] .
               "\n" ) ;
        $newenv{$_[0]} = $newenv{$_[0]} . " " . $_[1] ;
        } elsif ( $envpostpendexceptions{$_[0]}{$_[1]} ) {
        &CTUDebug( "post-pending (by exception) '" . $_[1] . "' to '" . $_[0] .
               "'\n" ) ;
        $newenv{$_[0]} = $newenv{$_[0]} . " " . $_[1] ;
        } else {
        &CTUDebug( "pre-pending '" . $_[1] . "' to " . $_[0] .
               "\n" ) ;
        $newenv{$_[0]} = $_[1] . " " . $newenv{$_[0]} ;
        }
    }
    }
}

require "$tool/built/include/ctcm.pl" ;

# given the project and flavor, build the lists of variables to set/modify
# input is in:
# $_[0] = project
# $_[1] = flavor
# $_[2] = is some kind of default?
#
# output is in:
# return value is config line
# %newenv      = an image of those parts of the environment we want to change
# %envsep      = seperator
# %envcmd      = set or setenv
# %envdo       = direct commands to add to attach script
# %envpostpend = flag that variable should be postpended
sub CTAttachCompute {
   &CTUDebug( "in CTAttachCompute\n" ) ;
   local( $done ) = 0 ;
   local( $flav ) = $_[1] ;
   local( $prevflav ) = &CTQueryProj( $_[0] ) ;
   local( $spec ) ;
   local( $root ) ;
   if ( $_[2] && ( $prevflav ne "" )) {
      # want some form of default attachment, and are already attached.  Short
      # circuit.
      $done = 1 ;
   }

   #
   # choose real flavor and find/validate root
   #
   while ( ! $done ) {
      $spec = &CTResolveSpec( $_[0], $flav ) ;
      #print STDERR  "spec line '" . $spec . "' flav: '" . $flav ."'\n";
      &CTUDebug( "spec line = '$spec'\n" ) ;
      if ( $spec ne "" ) {
         $root = &CTComputeRoot( $_[0], $flav, $spec ) ;
         &CTCMSetup( $_[0], $spec, $flav ) ;
         if ( -e $root ) {
            $done = 1 ;
         }
      } else {
         print STDERR "could not resolve '" . $flav . "'\n" ;
         $done = 1 ;
      }
      if (( ! $done ) && $_[2] ) {
         if ( $flav eq "install" ) {
            # oh my! are we ever in trouble
            # want some sort of default, but couldn't get to what we wanted
            print STDERR "ctattach to install failed\n" ;
            $spec = "" ;
            $done = 1 ;
         } elsif ( $flav eq "release" ) {
            $flav = "install" ;
         } elsif ( $flav eq "ship" ) {
            $flav = "release" ;
         } else {
            $flav = "install" ;
         }
      } elsif ( ! $done ) {
         $spec = "" ;
         print STDERR "resolved '" . $flav . "' but '" . $root .
             "' does not exist\n" ;
         $done = 1 ;
      }
   }

   #
   # start real work
   #
   if ( $spec ne "" ) {
      local( $proj ) = $_[0] ;
      $proj =~ tr/a-z/A-Z/ ;
      local( $item ) ;

      # we scan the .init file first because if there are needed sub-attaches
      # they must happen before the rest of our work
      local( $init ) = "$root/built/etc/$_[0].init" ;
      local( %localmod );
      local( %localset );
      local( %localsep );
      local( %localcmd );
      local( %localdo );
      local( $localdocnt ) = 0 ;
      local( %localpost );
      local( %localpostexceptions ) = () ;
      if ( -e $init ) {
      &CTUDebug( "scanning " . $_[0] . ".init\n" ) ;
      local( @linesplit ) ;
      local( $linetmp ) ;
      local( $loop ) ;
      local( $looptmp ) ;
      local( *INITFILE ) ;
      open( INITFILE, "< $init" ) ;
      while ( <INITFILE> ) {
          s/\n$// ;
          @linesplit = split( /\#/ ) ;
          $_ = $linesplit[0] ;
          if ( $_ =~ /^MODABS/ ) {
          @linesplit = split ;
          $linetmp = $linesplit[1] ;
          shift( @linesplit ) ;
          shift( @linesplit ) ;
          $linesplitjoin = join( " ", @linesplit ) ;
          if ( $linesplit[0] eq "-" ) {
              shift( @linesplit ) ;
              $linesplitjoin = join( " ", @linesplit ) ;
              $localpostexceptions{$linetmp}{$linesplitjoin} = 1 ;
              &CTUDebug( "Creating post-pend exception for '" . 
                        $linetmp . "':'" . $linesplitjoin . "'\n" ) ;
          }
          if ( $localmod{$linetmp} eq "" ) {
              $localmod{$linetmp} = $linesplitjoin ;
          } else {
              $localmod{$linetmp} = $localmod{$linetmp} . " " .
                  $linesplitjoin ;
          }
          } elsif ( $_ =~ /^MODREL/ ) {
          @linesplit = split ;
          $linetmp = $linesplit[1] ;
          shift( @linesplit ) ;
          shift( @linesplit ) ;
          $postexception = 0 ;
          foreach $loop ( @linesplit ) {
              if ( $loop eq "-" ) {
                  $postexception = 1 ;
                  next ;
              }
              $looptmp = $root . "/" . &CTUShellEval($loop) ;
              if ( $postexception ) {
                  $localpostexceptions{$linetmp}{$looptmp} = 1 ;
                  &CTUDebug( "Creating post-pend exception for '" . 
                             $linetmp . "':'" . $looptmp . "'\n" ) ;
              }
              if ( -e $looptmp ) {
              if ( $localmod{$linetmp} eq "" ) {
                  $localmod{$linetmp} = $looptmp ;
              } else {
                  $localmod{$linetmp} = $localmod{$linetmp} . " " .
                  $looptmp ;
              }
              }
          }
          } elsif ( $_ =~ /^SETABS/ ) {
          @linesplit = split ;
          $linetmp = $linesplit[1] ;
          shift( @linesplit ) ;
          shift( @linesplit ) ;
          if ( $localset{$linetmp} eq "" ) {
              $localset{$linetmp} = join( " ", @linesplit ) ;
          } else {
              $localset{$linetmp} = $localset{$linetmp} . " " .
              join( " ", @linesplit ) ;
          }
          } elsif ( $_ =~ /^SETREL/ ) {
          @linesplit = split ;
          $linetmp = $linesplit[1] ;
          shift( @linesplit ) ;
          shift( @linesplit ) ;
          foreach $loop ( @linesplit ) {
              $looptmp = $root . "/" . &CTUShellEval($loop) ;
              if ( -e $looptmp ) {
              if ( $localset{$linetmp} eq "" ) {
                  $localset{$linetmp} = $looptmp ;
              } else {
                  $localset{$linetmp} = $localset{$linetmp} . " " .
                  $looptmp ;
              }
              }
          }
          } elsif ( $_ =~ /^SEP/ ) {
          @linesplit = split ;
          $localsep{$linesplit[1]} = $linesplit[2] ;
          } elsif ( $_ =~ /^CMD/ ) {
          @linesplit = split ;
          $localcmd{$linesplit[1]} = $linesplit[2] ;
          } elsif ( $_ =~ /^DOCSH/ ) {
          if ( $shell_type ne "sh" ) {
              @linesplit = split ;
              shift( @linesplit ) ;
              $localdo{$localdocnt} = join( " ", @linesplit ) ;
              $localdocnt++ ;
          }
          } elsif ( $_ =~ /^DOSH/ ) {
          if ( $shell_type eq "sh" ) {
              @linesplit = split ;
              shift( @linesplit ) ;
              $localdo{$localdocnt} = join( " ", @linesplit ) ;
              $localdocnt++ ;
          }
          } elsif ( $_ =~ /^DO/ ) {
          @linesplit = split ;
          shift( @linesplit ) ;
          $localdo{$localdocnt} = join( " ", @linesplit ) ;
          $localdocnt++ ;
          } elsif ( $_ =~ /^POSTPEND/ ) {
          @linesplit = split ;
          $localpost{$linesplit[1]} = 1 ;
          } elsif ( $_ =~ /^ATTACH/ ) {
          @linesplit = split ;
          shift( @linesplit ) ;
          foreach $loop ( @linesplit ) {
              push( @attachqueue, $loop ) ;
          }
          } elsif ( $_ ne "" ) {
          print STDERR "Unknown .init directive '$_'\n" ;
          }
      }
      close( INITFILE ) ;
      }

      # now handle sub-attaches
      &CTUDebug( "performing sub-attaches\n" ) ;
      while ( @attachqueue != () ) {
      $item = shift( @attachqueue ) ;
      &CTUDebug( "attaching to " . $item . "\n" ) ;
      &CTAttachCompute( $item, $defflav, 1 ) ;
      }

      # now we will do our extentions, then apply the mods from the .init
      # file, if any
      &CTUDebug( "extending paths\n" ) ;
      local( $type ) = &CTSpecType( $spec ) ;
      if ( $type eq "vroot" ) {
      &CTAttachMod( "PATH", "/usr/atria/bin", $root, $proj ) ;
      }

      # For now, we will not check whether the various /bin, /lib,
      # /inc directories exist before adding them to the paths.  This
      # helps when attaching to unitialized trees that do not have
      # these directories yet (but will shortly).

      # However, we *will* filter out any trees whose name ends in
      # "MODELS".  These don't have subdirectories that we care about
      # in the normal sense.
      if ( ! ( $proj =~ /MODELS$/ ) ) {

          $item = $root . "/built/bin" ;
          #if ( -e $item ) {
      &CTAttachMod( "PATH", $item, $root, $proj ) ;
          #}

          $item = $root . "/built/lib" ;
          #if ( -e $item ) {
          &CTAttachMod( "PATH", $item, $root, $proj ) ;
      &CTAttachMod( "LD_LIBRARY_PATH", $item, $root, $proj ) ;
      &CTAttachMod( "DYLD_LIBRARY_PATH", $item, $root, $proj ) ;
          #}

          $item = $root . "/built/include" ;
          #if ( -e $item ) {
      &CTAttachMod( "CT_INCLUDE_PATH", $item, $root, $proj ) ;
          #}

          $item = $root . "/built/etc" ;
          #if ( -e $item ) {
      &CTAttachMod( "ETC_PATH", $item, $root, $proj ) ;
          #}
      }

      &CTAttachMod( "CTPROJS", $proj . ":" . $flav, $root, $proj ) ;
      &CTAttachSet( $proj, $root ) ;

      # run thru the stuff saved up from the .init file
      foreach $item ( keys %localsep ) {
      $envsep{$item} = $localsep{$item} ;
      }
      foreach $item ( keys %localpost ) {
      $envpostpend{$item} = $localpost{$item} ;
      }
      %envpostpendexceptions = %localpostexceptions;
      foreach $item ( keys %localmod ) {
      local( @splitthis ) = split( / +/, $localmod{$item} ) ;
      local( $thing ) ;
      foreach $thing ( @splitthis ) {
          &CTAttachMod( $item, $thing, $root, $proj ) ;
      }
      }
      foreach $item ( keys %localset ) {
      &CTAttachSet( $item, $localset{$item} ) ;
      }
      foreach $item ( keys %localcmd ) {
      $envcmd{$item} = $localcmd{$item} ;
      }
      for ($item = 0; $item < $localdocnt; $item++) {
      $envdo{$docnt} = $localdo{$item} ;
      $docnt++ ;
      }
      %envpostpendexceptions = () ;
   }

   &CTUDebug( "out of CTAttachCompute\n" ) ;
   $spec ;
}

# write a script to NOT change the environment
# Input is:
# $_[0] = filename
sub CTAttachWriteNullScript {
   &CTUDebug( "in CTAttachWriteNullScript\n" ) ;
   local( *OUTFILE ) ;
   open( OUTFILE, ">$_[0]" ) ;
   print OUTFILE "#!/bin/" . $shell_type . " -f\n" ;
   print OUTFILE "echo No attachment actions performed\n" ;
   print OUTFILE "rm -f $_[0]\n" ;
   close( OUTFILE ) ;
   &CTUDebug( "out of CTAtachWriteNullScript\n" ) ;
}

# write a script to setup the environment
# Input is:
# $_[0] = filename
sub CTAttachWriteScript {
   &CTUDebug( "in CTAttachWriteScript\n" ) ;
   local( *OUTFILE ) ;
   open( OUTFILE, ">$_[0]" ) ;
   print OUTFILE "#!/bin/" . $shell_type . " -f\n" ;
   local( $item ) ;

   foreach $item ( keys %newenv ) {
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
           print OUTFILE $envcmd{$item} . " $item " ;
           if ( $envcmd{$item} eq "set" ) {
           print OUTFILE "= ( " ;
           }
           print OUTFILE $outval ;
           if ( $envcmd{$item} eq "set" ) {
           print OUTFILE ")" ;
           }
           print OUTFILE "\n" ;
       } else {
           print OUTFILE "setenv $item \"$outval\"\n" ;
               if ( $ctdebug ) {
                   print OUTFILE "echo setting " . $item . " to '" . $outval . "'\n" ;
               }
       }
       }
   }

   #if ( $newenv{"CDPATH"} ne "" ) {
   #    if ( $shell_type ne "sh" ) {
   #    print OUTFILE "set cdpath = ( \$" . "CDPATH )\n" ;
   #        if ( $ctdebug ) {
   #            print OUTFILE "echo assigning cdpath\n" ;
   #        }
   #    }
   #}
   for ($item = 0; $item < $docnt; $item++) {
      print OUTFILE $envdo{$item} . "\n" ;
      if ( $ctdebug ) {
        print OUTFILE "echo doing '" . $envdo{$item} . "'\n" ;
      }
   }
   if (! $ctdebug) {
      print OUTFILE "rm -f $_[0]\n" ;
   } else {
      print OUTFILE "echo end of script $_[0]\n" ;
      print STDERR "no self-destruct script '" . $_[0] . "'\n" ;
   }
   close( OUTFILE ) ;
   &CTUDebug( "out of CTAttachWriteScript\n" ) ;
}

1;
