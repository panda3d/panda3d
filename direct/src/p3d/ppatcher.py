#! /usr/bin/env python

usageText = """

This script generates the patches required to support incremental
download of Panda3D packages.  It can be run as a post-process on a
directory hierarchy created by ppackage; it will examine the directory
hierarchy, and create any patches that appear to be missing.

You may run ppackage on the same directory hierarchy as many times as
you like, without creating patches.  You may then download and test
the resulting files--users connecting to the tree without fresh
patches will be forced to download the entire file, instead of making
an incremental download, but the entire process will work otherwise.
When you are satisfied that all of the files are ready to be released,
you may run ppackage on the directory hierarchy to generate the
required patches.

Generating the patches just before final release is a good idea to
limit the number of trivially small patches that are created.  Each
time this script is run, a patch is created from the previous version,
and these patches daisy-chain together to define a complete update
sequence.  If you run this script on internal releases, you will
generate a long chain of small patches that your users must download;
this is pointless if there is no possibility of anyone having
downloaded one of the intervening versions.

You can also generate patches with the -p option to ppackage, but that
only generates patches for the specific packages built by that
invocation of ppackage.  If you use the ppatcher script instead, it
will generate patches for all packages (or the set of packages that
you name specifically).

This script is actually a wrapper around Panda's PatchMaker.py.

Usage:

  %(prog)s [opts] [packageName1 .. packageNameN]

Parameters:

  packageName1 .. packageNameN
    Specify the names of the package(s) you wish to generate patches
    for.  This allows you to build patches for only a subset of the
    packages found in the tree.  If you omit these parameters, patches
    are built for all packages that require them.

Options:

  -i install_dir
     The full path to the install directory.  This should be the same
     directory named by the -i parameter to ppackage.

  -h
     Display this help

"""

import sys
import getopt
import os

from direct.p3d.PatchMaker import PatchMaker
from panda3d.core import Filename

def usage(code, msg = ''):
    sys.stderr.write(usageText % {'prog' : os.path.split(sys.argv[0])[1]})
    sys.stderr.write(msg + '\n')
    sys.exit(code)

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:h')
except getopt.error as msg:
    usage(1, msg)

installDir = None
for opt, arg in opts:
    if opt == '-i':
        installDir = Filename.fromOsSpecific(arg)

    elif opt == '-h':
        usage(0)
    else:
        print('illegal option: ' + arg)
        sys.exit(1)

packageNames = args

if not installDir:
    installDir = Filename('install')

if not packageNames:
    # "None" means all packages.
    packageNames = None

pm = PatchMaker(installDir)
pm.buildPatches(packageNames = packageNames)

# An explicit call to exit() is required to exit the program, when
# this module is packaged in a p3d file.
sys.exit(0)
