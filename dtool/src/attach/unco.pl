require "$tool/built/include/ctutils.pl" ;
require "$tool/built/include/ctdelta.pl" ;

# Remove a branch for an element if needed
# input is in:
# $_[0] = element
sub CTUncoDoIt {
   local( $elem ) = $_[0] ;
   local( $ver ) = &CTDeltaGetVersion( $elem ) ;
   if ( $ctdebug ne "" ) {
      print STDERR "Unco script: got version '" . $ver . "'\n" ;
   }
   local( @verlist ) ;
   @verlist = split( /\//, $ver ) ;
   local( $vlast ) = pop( @verlist ) ;
   if ( $ctdebug ne "" ) {
      print STDERR "Unco script: last part of version is '" . $vlast . "'\n" ;
   }
   if ( $#verlist > 1 ) {
      local( $branch ) = join( "/", @verlist ) ;
      if ( $vlast == 0 ) {
         local( $cmd ) = "cleartool rmbranch -force -nc $elem" . "@@" . "$branch" ;
         if ( $ctdebug ne "" ) {
	    print STDERR "Unco script: command is '" . $cmd . "'\n" ;
	 }
	 system $cmd ;
      }
   }
}

1;
