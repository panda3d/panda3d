import os
import sys

print 'Site customize for Panda'

def getPackages():
    """
    Find all the packages on your ctprojs variable and parse them
    to extract the tree name from the long details like this:
    TOONTOWN:default DIRECT:default PANDA:personal DTOOL:install
    Returns a list of the packages as strings
    Note: the ctprojs are reversed to put them in the order of attachment.
    """
    packages = []
    ctprojs = os.getenv("CTPROJS").split()
    for proj in ctprojs:
        projName = proj.split(':')[0]
        packages.append(projName)
    packages.reverse()
    return packages


def addpackage(package):
    """
    Look in this package name for the file $PACKAGE/etc/package.pth
    which contains paths that we want prepended to sys.path
    There should be one relative pathname per line (relative to $PACKAGE)
    comments and empty lines are ok
    """
    tree = os.getenv(package)
    lowerPackage = package.lower()
    fullname = os.path.join(tree, 'etc' + os.sep + (lowerPackage + '.pth'))
    try:
        f = open(fullname)
        print 'Appending paths in ' + fullname
    except IOError:
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

for package in getPackages():
    addpackage(package)

