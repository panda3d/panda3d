# copy the template makefile to output, recursively expanding marked inclusions
sub CTInstallMakeWrite {
   local( *INMAKE ) ;
   open( INMAKE, "<$_[0]" ) ;
   while ( <INMAKE> ) {
      if ( /^#CTINCLUDE/ ) {
	 s/\n$// ;
	 local( @CTIMWlist ) ;
	 @CTIMWlist = split ;
	 local( $tool ) = $ENV{ "DTOOL" } ;
	 require "$tool/inc/ctutils.pl" ;
	 local( $CTIMWtmp ) = &CTUShellEval( $CTIMWlist[1] ) ;
	 &CTInstallMakeWrite( $CTIMWtmp ) ;
      } elsif (( /^#CTPROJECT/ ) && ( $projname ne "" )){
	 local( $CTIMWtmp ) = $projname ;
         $CTIMWtmp =~ tr/A-Z/a-z/ ; # tolower
	 print OUTMAKE "CTPROJECT = $CTIMWtmp\n" ;
      } elsif (( /^#CTPROJROOT/ ) && ( $projname ne "" )) {
	 local( $CTIMWtmp ) = $projname ;
         $CTIMWtmp =~ tr/a-z/A-Z/ ; # toupper
	 print OUTMAKE "CTPROJROOT = \$($CTIMWtmp)\n" ;
      } elsif (( /^#CTPACKAGE/ ) && ( $pkgname ne "" )) {
	 print OUTMAKE "PACKAGE = $pkgname\n" ;
      } elsif (( /^#CTINSTPKG/ ) && ( $pkgname ne "" )) {
	 print OUTMAKE "include src/all/$pkgname/Makefile.install\n" ;
	 print OUTMAKE "#CTINSTPKG\n" ;
      } elsif (( /^#CTTARGET/ ) && ( $fulltgtname ne "" )) {
	 print OUTMAKE "TARGET = $fulltgtname\n" ;
      } elsif (( /^SUBMAKES/ ) && ( $tgtname ne "" )) {
	 s/\n$// ;
	 print OUTMAKE "$_ $tgtname\n" ;
      } else {
	 print OUTMAKE $_ ;
      }
   }
   close( INMAKE ) ;
}

# install a given makefile template, expanding any internal directives
sub CTInstallMake {
   local( *OUTMAKE ) ;
   open( OUTMAKE, ">$_[1]" ) ;
   &CTInstallMakeWrite( $_[0] ) ;
   close( OUTMAKE );
}

# Scan the given makefile for CT markers to be expanded
sub CTInstallScanMake {
   local( $CTISMname ) = "/tmp/make.$$" ;
   &CTInstallMake( $_[0], $CTISMname ) ;
   system( "mv $CTISMname $_[0]" ) ;
}

1;
