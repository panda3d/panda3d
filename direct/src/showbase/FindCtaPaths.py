"""Undocumented Module"""

__all__ = ['deCygwinify', 'getPaths']

"""This module is used only by the VR Studio programmers who are using
the ctattach tools.  It is imported before any other package, and its
job is to figure out the correct paths to each of the packages.

This module is not needed if you are not using ctattach; in this case
all of the Panda packages will be collected under a common directory,
which you will presumably have already on your PYTHONPATH. """

import os
import sys

def deCygwinify(path):
    if os.name in ['nt'] and path[0] == '/':
        # On Windows, we may need to convert from a Cygwin-style path
        # to a native Windows path.

        # Check for a case like /i/ or /p/: this converts
        # to i:\ or p:\.

        dirs = path.split('/')
        if len(dirs) > 2 and len(dirs[1]) == 1:
            path = '%s:\%s' % (dirs[1], '\\'.join(dirs[2:]))

        else:
            # Otherwise, prepend $PANDA_ROOT and flip the slashes.
            pandaRoot = os.getenv('PANDA_ROOT')
            if pandaRoot:
                path = os.path.normpath(pandaRoot + path)

    return path
    
def getPaths():
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
        
        print 'Appending to sys.path based on $CTPROJS:'

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
            if not tree:
                print "  CTPROJS contains %s, but $%s is not defined." % (package, package)
                sys.exit(1)

            tree = deCygwinify(tree)

            parent, base = os.path.split(tree)
            if base != package.lower():
                print "  Warning: $%s refers to a directory named %s (instead of %s)" % (package, base, package.lower())
            
            if parent not in parents:
                parents.append(parent)


            # We also put tree/built/lib on sys.path by hand, because we
            # will need to load up the generated C++ modules that got
            # put there.  Also, we will find the output of genPyCode
            # in $DIRECT/built/lib/pandac.
            libdir = os.path.join(tree, 'built', 'lib')
            if os.path.isdir(libdir):
                if libdir not in sys.path:
                    sys.path.append(libdir)


        # Now the result goes onto sys.path.
        for parent in parents:
            print "  %s" % (parent)
            if parent not in sys.path:
                sys.path.append(parent)
    

getPaths()
