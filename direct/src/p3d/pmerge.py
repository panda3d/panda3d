#! /usr/bin/env python

usageText = """

This script can be used to merge together the contents of two or more
separately-built stage directories, built independently via ppackage,
or via Packager.py.

This script is actually a wrapper around Panda's PackageMerger.py.

Usage:

  %(prog)s [opts] [inputdir1 .. inputdirN]

Parameters:

  inputdir1 .. inputdirN
    Specify the full path to the input directories you wish to merge.
    These are the directories specified by -i on the previous
    invocations of ppackage.  The order is mostly unimportant.

Options:

  -i install_dir
     The full path to the final install directory.  This may also
     contain some pre-existing contents; if so, it is merged with all
     of the input directories as well.

  -h
     Display this help
"""

import sys
import getopt
import os

from direct.p3d import PackageMerger
from pandac.PandaModules import *

def usage(code, msg = ''):
    print >> sys.stderr, usageText % {'prog' : os.path.split(sys.argv[0])[1]}
    print >> sys.stderr, msg
    sys.exit(code)

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:h')
except getopt.error, msg:
    usage(1, msg)

installDir = None
for opt, arg in opts:
    if opt == '-i':
        installDir = Filename.fromOsSpecific(arg)
        
    elif opt == '-h':
        usage(0)
    else:
        print 'illegal option: ' + arg
        sys.exit(1)

inputDirs = []
for arg in args:
    inputDirs.append(Filename.fromOsSpecific(arg))

if not inputDirs:
    print "no input directories specified."
    sys.exit(1)

try:
    pm = PackageMerger.PackageMerger(installDir)
    for dir in inputDirs:
        pm.merge(dir)
    pm.close()
        
except PackageMerger.PackageMergerError:
    # Just print the error message and exit gracefully.
    inst = sys.exc_info()[1]
    print inst.args[0]
    sys.exit(1)


# An explicit call to exit() is required to exit the program, when
# this module is packaged in a p3d file.
sys.exit(0)
