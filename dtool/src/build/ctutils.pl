# evaluate the given parameter to expand shell variables
sub CTUShellEval {
   local( *CTUSEFILE ) ;
   open( CTUSEFILE, "echo $_[0] |" ) ;
   local( $CTUSEret ) = <CTUSEFILE> ;
   close( CTUSEFILE ) ;
   $CTUSEret =~ s/\n$// ;
   $CTUSEret ;
}

# if debug is on, print the argument
sub CTUDebug {
    if ( $ctdebug ) {
	print STDERR $_[0] ;
    }
}

use Cwd ;
# get current directory
sub CTUCurrDir {
    local( $pwd ) = getcwd() ;
    if ( $pwd =~ /^\/vobs/ ) {
	local( *VFILE ) ;
	open( VFILE, "cleartool pwv -short |" ) ;
	local( $view ) = <VFILE> ;
	close( VFILE ) ;
	$view =~ s/\n$// ;
	$pwd = "/view/" . $view . $pwd ;
    }
    $pwd ;
}

# turn a shell return code into a success/fail flag
sub CTURetCode {
    local( $ret ) ;
    if ( $_[0] == 0 ) {
	$ret = 1 ;
    } else {
	$ret = 0 ;
    }
    $ret ;
}

$ctdebug = $ENV{"CTATTACH_DEBUG"} ;
$ctvspec_path = '/usr/local/etc' unless $ctvspec_path = $ENV{'CTVSPEC_PATH'};

1;
