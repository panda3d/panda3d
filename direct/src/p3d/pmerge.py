#! /usr/bin/env python

usageText = """

This script can be used to merge together the contents of two or more
separately-built stage directories, built independently via ppackage,
or via Packager.py.  This script also verifies the hash, file size,
and timestamp values in the stage directory as it runs, so it can be
run on a single standalone directory just to perform this validation.

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
     of the input directories as well.  The contents of this directory
     are checked for self-consistency with regards to hashes and
     timestamps.

  -p packageName[,packageName...]
     Specifies one or more particular packages by name that are to be
     included from the input directories.  Any packages not in this
     list are left unchanged in the install directory, even if there
     is a newer version in one of the input directories.  If no
     packages are named, all packages are involved.  This option may
     be repeated.

  -h
     Display this help

"""

import sys
import getopt
import os

from direct.p3d import PackageMerger
from panda3d.core import Filename

def usage(code, msg = ''):
    sys.stderr.write(usageText % {'prog' : os.path.split(sys.argv[0])[1]})
    sys.stderr.write(msg + '\n')
    sys.exit(code)

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:p:h')
except getopt.error as msg:
    usage(1, msg)

installDir = None
packageNames = []
for opt, arg in opts:
    if opt == '-i':
        installDir = Filename.fromOsSpecific(arg)
    elif opt == '-p':
        packageNames += arg.split(',')

    elif opt == '-h':
        usage(0)
    else:
        print('illegal option: ' + arg)
        sys.exit(1)

if not packageNames:
    # No package names means allow all packages.
    packageNames = None

inputDirs = []
for arg in args:
    inputDirs.append(Filename.fromOsSpecific(arg))

# It's now legal to have no input files if you only want to verify
# timestamps and hashes.
## if not inputDirs:
##     print "no input directories specified."
##     sys.exit(1)

try:
    pm = PackageMerger.PackageMerger(installDir)
    for dir in inputDirs:
        pm.merge(dir, packageNames = packageNames)
    pm.close()

except PackageMerger.PackageMergerError:
    # Just print the error message and exit gracefully.
    inst = sys.exc_info()[1]
    print(inst.args[0])
    sys.exit(1)


# An explicit call to exit() is required to exit the program, when
# this module is packaged in a p3d file.
sys.exit(0)
