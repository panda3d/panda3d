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

getPackages()
