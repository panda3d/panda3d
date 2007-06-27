#!perl -w
use strict;
#use File::Copy;
use File::Basename;

my ($destdir, $printfilesCmd, $debug_state);

if ($ARGV[0] && $ARGV[0] eq '-d') {
    $destdir = $ARGV[1];
    $printfilesCmd = $ARGV[2];
    $debug_state = "-d";
}
else {
    $destdir = $ARGV[0];
    $printfilesCmd = $ARGV[1];
    $debug_state = "";
}

if (!$destdir || !$printfilesCmd) {
    print "Usage: copyfiles.pl [-d] destdir printfilesCmd\n";
    exit 1;
}

if (! -d $destdir ) {
    print "Error: destdir must be a directory\n";
    exit 1;
}

my @fileline = `$printfilesCmd $debug_state`;

my (%tree, %package);
my ($moduleDir, $packageDir, $installDir);
my ($file);

sub notsource($)
{
    my ($line) = @_;
    print "unrecognized line:|$line|\n";
}

sub add_file($ $)
{
    my ($dir,$line) = @_;
    if (! exists $package{$dir}) {
        $package{$dir} = [];                                # create a new array entry if module/package key doesn't exist
    }
    push @{$package{$dir}}, $line;                          # add the source dir of this file to module/package's list
}

foreach my $line (@fileline)
{
    chomp($line);
    #print "$line\n";


    if ( ($line =~ /CVS/)
        || ($line =~ /Opt\d\-/)
        || ($line =~ /\.cxx|\.obj|\.h|\.I|\.in|\.pdb|\.pp|\.cvsignore|Makefile/)
       )
    {                                                       # skip if ...
        print "skipping $line\n";
        next;
    }

    $installDir = '';

    if ($line =~ /\/([^\/]+)\/src\/([^\/]+)\/(.+)$/ && $3)
    {   # $1 is module aka dtool or pirates; $2 is package aka ai or battle
        if ($3)
        {
            $moduleDir = $1;
            $packageDir = $2;
            $file = $3;

            if (-d $file) {                                 # don't handle bare directories
                 notsource($file);
                 next;
            }
            $installDir = "$moduleDir/$packageDir"
                if $file =~ /\.py/;                         # tree install only for Python files

            $tree{$moduleDir} = 1;
            #print "recognized module:$moduleDir package:$packageDir\n";
        }
        else {
            notsource($line);                               # don't know how to handle this file
        }
    }
    elsif ($line =~ /pandac/)
    {
        $installDir = 'pandac';
    }

    add_file($installDir, $line);
    #print "line:|$line|";
    #print "$1 $2 $3 $4\n";
}

#############################################################################

print "\nSTARTING COPY\n\n";

sub echo_cmd($)
{
    my ($cmd) = @_;
    print "$cmd\n";
    system($cmd);
}

my $cmd;
foreach my $dir (keys %tree)
{                                                                   # create the master directories
    echo_cmd("mkdir -p $destdir/$dir");
    echo_cmd("touch $destdir/$dir/__init__.py");                    # linkage file for python
}

my ($fileline, $finaldir, $files);
foreach my $key (keys %package)
{                                                                   # loop and copy each cluster of files
    $finaldir = "$destdir/$key";
    print "\ncopying to $finaldir:\n";

    $files = $package{$key};
    foreach my $file (@{$files})
    {                                                               # dump list of files being copied
        print "$file\n";
    }

    $fileline = join(' ', @{$files});                               # create clustered command line
    #print "-t $destdir/$key $fileline\n";
    system("mkdir $destdir/$key");
    system("cp -rpt $destdir/$key $fileline");                      # copy
}
print "\n";

0;
