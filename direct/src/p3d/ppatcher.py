#! /usr/bin/env python

"""

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

  %(prog)s [opts]

Options:

  -i install_dir
     The full path to the install directory.  This should be the same
     directory named by the -i parameter to ppackage.

  -p packageName
     Generates patches for the named package only.  This may be
     repeated to name multiple packages.  If this is omitted, all
     packages in the directory are scanned.

  -h
     Display this help
"""

import sys
import getopt
import os

from direct.p3d.PatchMaker import PatchMaker
from pandac.PandaModules import *

def usage(code, msg = ''):
    print >> sys.stderr, __doc__ % {'prog' : os.path.split(sys.argv[0])[1]}
    print >> sys.stderr, msg
    sys.exit(code)

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:p:h')
except getopt.error, msg:
    usage(1, msg)

installDir = None
packageNames = []

for opt, arg in opts:
    if opt == '-i':
        installDir = Filename.fromOsSpecific(arg)
    elif opt == '-p':
        packageNames.append(arg)
        
    elif opt == '-h':
        usage(0)
    else:
        print 'illegal option: ' + flag
        sys.exit(1)

if args:
    usage(1)

if not installDir:
    installDir = Filename('install')

if not packageNames:
    # "None" means all packages.
    packageNames = None

pm = PatchMaker(installDir)
pm.buildPatches(packageNames = packageNames)
