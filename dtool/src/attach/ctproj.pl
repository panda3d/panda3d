require "$tool/built/include/ctutils.pl" ;

# return the root of the given project.
sub CTProjRoot {
    local( $CTPRtmp ) = $_[0] ;
    $CTPRtmp =~ tr/a-z/A-Z/ ;
    local( $CTPRret ) = $ENV{ $CTPRtmp } ;
    $CTPRret ;
}

# return the package we're currently in.
# input:
#   $_[0] = project
sub CTProjPkg {
    local( $CTPPret ) = &CTUCurrDir() ;
    local( $CTPPtmp ) = $_[0] ;
    $CTPPtmp  =~ tr/a-z/A-Z/ ;
    $CTPPret =~ s/$ENV{ $CTPPtmp }// ;
    $CTPPret =~ s/\/src\/// ;
    $CTPPret =~ s/\/metalibs\/// ;
    $CTPPret ;
}

# reutrn the project containing the given directory.  If no directory is given,
# return the project containing the current directory.
sub CTProj {
   local( $CTPdir ) ;
   if ($_[0] eq "") {
      $CTPdir = &CTUCurrDir() ;
   } else {
      # provided directory
      $CTPdir = $_[0] ;
   }
   local( $CTPprojs ) = $ENV{"CTPROJS"} ;
   local( $CTPdone ) = "" ;
   local( @CTPlist ) ;
   @CTPlist = split( / /, $CTPprojs ) ;
   local( @CTPlist2 ) ;
   local( $CTPtry ) ;
   while (( $CTPdone eq "" ) && ( @CTPlist != () )){
      # pop the first one off the list
      $CTPtmp = $CTPlist[0] ;
      shift( @CTPlist ) ;
      # split the project from it's flavor
      @CTPlist2 = split( /:/, $CTPtmp );
      $CTPtry = &CTProjRoot( $CTPlist2[0] ) ;
      # is CTPtry prefix of CTPdir?  if so we have our winner
      if ( $CTPdir =~ /^$CTPtry/ ) {
	 $CTPdone = "yep" ;
      }
   }
   if ( $CTPdone eq "" ) {
      $CTPtry = "" ;
   } else {
      $CTPtry = $CTPlist2[0] ;
   }
   $CTPtry ;
}

1;
