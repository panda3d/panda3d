import os
import sys
import glob

print 'Site customize for Panda:'


def readpth(tree, fullname):
    #print "readpth(tree=%s, fullname=%s)" % (tree, fullname)
    try:
        f = open(fullname)
        print '  Appending paths in ' + fullname
    except IOError:
        print '  IOError Appending paths in ' + fullname
        return

    while 1:
        dir = f.readline()
        if not dir:
            break
        if dir[0] == '#':
            continue
        if dir[-1] == '\n':
            dir = dir[:-1]
        dir = os.path.join(tree, dir)
        if dir not in sys.path and os.path.exists(dir):
            sys.path = [dir] + sys.path

def addpackage(package):
    """
    Look in this package name for the file $PACKAGE/etc/package.pth
    which contains paths that we want prepended to sys.path
    There should be one relative pathname per line (relative to $PACKAGE)
    comments and empty lines are ok
    """
    tree = os.getenv(package)
    if tree == None:
        print "  $%s is not defined." % (package,)
        return

    lowerPackage = package.lower()
    fullname = os.path.join(tree, 'etc', lowerPackage + '.pth')

    readpth(tree, fullname)

def getPackages():
    """
    Find all the packages on your ctprojs variable and parse them
    to extract the tree name from the long details like this:
    TOONTOWN:default DIRECT:default PANDA:personal DTOOL:install
    Returns a list of the packages as strings
    Note: the ctprojs are reversed to put them in the order of attachment.
    """
    ctprojs = os.getenv("CTPROJS")
    if ctprojs == None:
        # The CTPROJS environment variable isn't defined.  Assume
        # we're running from the directory above all of the source
        # trees; look within these directories for the *.pth files.

        # If PLAYER is defined, use that as the root; otherwise, use
        # the current directory.
        player = os.getenv("PLAYER")
        if player == None:
            print '  Appending paths based on current working directory.'
            searchstr = os.path.join('*', 'src', 'configfiles', '*.pth')
        else:
            print '  Appending paths based on $PLAYER'
            searchstr = os.path.join(player, '*', 'src', 'configfiles', '*.pth')
            
        filenames = glob.glob(searchstr)
        if len(filenames) == 0:
            print ''
            print '  Warning: no files found matching %s.' % (searchstr)
            print '  Check $PLAYER, or your starting directory.'
            print ''
            
        for filename in filenames:
            tree = os.path.dirname(os.path.dirname(os.path.dirname(filename)))
            readpth(tree, filename)

    else:
        # The CTPROJS environment variable *is* defined.  We must be
        # using the ctattach tools; get the *.pth files from the set
        # of attached trees.
        print '  Appending paths based on $CTPROJS'
        packages = []

        for proj in ctprojs.split():
            projName = proj.split(':')[0]
            packages.append(projName)
        packages.reverse()

        for package in packages:
            addpackage(package)


def getPath():
    """
    Add to sys.path the appropriate director(ies) to search for the
    various Panda projects.  Typically, these will all be in the same
    directory (which is presumably already on sys.path), but if the VR
    Studio ctattach tools are in use they could be scattered around in
    several places.
    """

    ctprojs = os.getenv("CTPROJS")
    if ctprojs:
        # The CTPROJS environment variable is defined.  We must be
        # using the ctattach tools.  In this case, we need to figure
        # out the location of each of the separate trees, and put the
        # parent directory of each one on sys.path.  In many cases,
        # these will all be siblings, so we filter out duplicate
        # parent directories.
        
        print '  Appending to sys.path based on $CTPROJS:'

        # First, get the list of packages, then reverse the list to
        # put it in ctattach order.  (The reversal may not matter too
        # much these days, but let's be as correct as we can be.)
        packages = []
        for proj in ctprojs.split():
            projName = proj.split(':')[0]
            packages.append(projName)
        packages.reverse()

        # Now walk through the packages and figure out the parent of
        # each referenced directory.
        
        parents = []
        for package in packages:
            tree = os.getenv(package)
            if tree == None:
                print "  CTPROJS contains %s, but $%s is not defined." % (package, package)
                sys.exit(1)

            parent, base = os.path.split(tree)
            if base != package.lower():
                print "  Warning: $%s refers to a directory named %s (instead of %s)" % (package, base, package.lower())
            
            if parent not in parents:
                parents.append(parent)


            # We also put tree/lib on sys.path by hand, because we
            # will need to load up the generated C++ modules that got
            # put there.  Also, we will find the output of genPyCode
            # in $DIRECT/lib/pandac.
            libdir = os.path.join(tree, 'lib')
            if os.path.isdir(libdir):
                sys.path.append(libdir)


        # Now the result goes onto sys.path.
        for parent in parents:
            print "    %s" % (parent)
            sys.path.append(parent)
    

#getPackages()
getPath()
