#! /usr/bin/env python

"""
This script can be used to produce a downloadable "Package", which may
contain arbitrary files--for instance, Python code, bam files, and/or
compiled DLL's--and which may be downloaded by application code to
extend an application at runtime.

In addition to building the package in the first place, this script
can also be used to generate downloadable patches of the package
against previous versions, and manage the whole tree of patches in a
directory structure suitable for hosting on a web server somewhere.

This script is actually a wrapper around Panda's Packager.py.

Usage:

  ppackage.py [opts] package.pdef

Required:

  package.pdef
    The config file that describes the contents of the package file(s)
    to be built, in excruciating detail.  Use "ppackage.py -H" to
    describe the syntax of this file.

Options:

  -i install_dir
     The full path to a local directory to copy the
     ready-to-be-published files into.  This directory structure may
     contain multiple different packages from multiple different
     invocations of this script.  It is the user's responsibility to
     copy this directory structure to a web host where it may be
     downloaded by the client.

  -s search_dir
     Additional directories to search for previously-built packages.
     This option may be repeated as necessary.

  -d persist_dir
     The full path to a local directory that retains persistant state
     between publishes.  This directory structure keeps files that are
     used to build patches for future releases.  You should keep this
     directory structure around for as long as you plan to support
     this package.  If this directory structure does not exist or is
     empty, patches will not be created for this publish; but the
     directory structure will be populated for the next publish.

  -p platform
     Specify the platform to masquerade as.  The default is whatever
     platform Panda has been built for.  It is probably unwise to set
     this, unless you know what you are doing.

  -H
     Describe the syntax of the package.pdef input file.

  -h
     Display this help
"""

import sys
import getopt
import os

from direct.showutil import Packager
from direct.showutil import make_contents
from pandac.PandaModules import *

def usage(code, msg = ''):
    print >> sys.stderr, __doc__
    print >> sys.stderr, msg
    sys.exit(code)

packager = Packager.Packager()

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:s:d:p:Hh')
except getopt.error, msg:
    usage(1, msg)

for opt, arg in opts:
    if opt == '-i':
        packager.installDir = Filename.fromOsSpecific(arg)
    elif opt == '-s':
        packager.installSearch.appendDirectory(Filename.fromOsSpecific(arg))
    elif opt == '-d':
        packager.persistDir = Filename.fromOsSpecific(arg)
    elif opt == '-p':
        packager.platform = arg
        
    elif opt == '-h':
        usage(0)
    elif opt == '-H':
        print 'Not yet implemented.'
        sys.exit(1)
    else:
        print 'illegal option: ' + flag
        sys.exit(1)

if not args:
    usage(0)
    
if len(args) != 1:
    usage(1)

packageDef = Filename.fromOsSpecific(args[0])

if not packager.installDir:
    packager.installDir = Filename('install')
packager.installSearch.prependDirectory(packager.installDir)

packager.setup()
packages = packager.readPackageDef(packageDef)

# Look to see if we built any true packages, or if all of them were
# p3d files.
anyPackages = False
for package in packages:
    if not package.p3dApplication:
        anyPackages = True
        break

if anyPackages:
    # If we built any true packages, then update the contents.xml at
    # the root of the install directory.
    cm = make_contents.ContentsMaker()
    cm.installDir = packager.installDir.toOsSpecific()
    cm.build()

