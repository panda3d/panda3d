# return the attached flavor of given project (or empty string)
sub CTQueryProj {
   local( $projs ) = $ENV{"CTPROJS"} ;
   local( @projlist ) ;
   @projlist = split( / +/, $projs ) ;
   local( $pair ) ;
   local( @pairlist ) ;
   local( $ret ) = "" ;
   foreach $pair ( @projlist ) {
      @pairlist = split( /:/, $pair ) ;
      $pairlist[0] =~ tr/A-Z/a-z/ ;
      if ( $pairlist[0] eq $_[0] ) {
	 $ret = $pairlist[1] ;
      }
   }
   $ret ;
}

# return all projects attached with a given flavor
sub CTQueryFlav {
   local( $projs ) = $ENV{"CTPROJS"} ;
   local( @projlist ) ;
   @projlist = split( / +/, $projs ) ;
   local( $pair ) ;
   local( @pairlist ) ;
   local( $ret ) = "" ;
   foreach $pair ( @projlist ) {
      @pairlist = split( /:/, $pair ) ;
      if ( $pairlist[1] eq $_[0] ) {
	 $pairlist[0] =~ tr/A-Z/a-z/ ;
	 $ret = $ret . " $pairlist[0]" ;
      }
   }
   $ret ;
}

1;
