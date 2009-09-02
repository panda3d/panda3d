#! /usr/bin/env python

"""
This script can be used to produce a Panda3D downloadable "package",
which may contain arbitrary files--for instance, Python code, bam
files, and/or compiled DLL's--and which may be downloaded by
application code to extend an application at runtime.

In addition to packages, this script can also be used to build
standalone p3d applications, that is, packaged Python code in a p3d
file, for execution by the Panda3D plugin or runtime.  (But also see
packp3d, which is designed to be a simpler interface for building
applications.)

This script is actually a wrapper around Panda's Packager.py, which
can be invoked directly by Python code that requires a programmatic
interface to building packages.

Usage:

  %(prog)s [opts] package.pdef [packageName1 .. packageNameN]

Parameters:

  package.pdef
    The config file that describes the contents of the package file(s)
    to be built, in excruciating detail.  See the Panda3D manual for
    the syntax of this file.

  packageName1 .. packageNameN
    Specify the names of the package(s) you wish to build out of the
    package.pdef file.  This allows you to build only a subset of the
    packages defined in this file.  If you omit these parameters, all
    packages are built.

Options:

  -i install_dir
     The full path to a local directory to copy the
     ready-to-be-published files into.  This directory structure may
     contain multiple different packages from multiple different
     invocations of this script.  It is the user's responsibility to
     copy this directory structure to a server, which will have the
     URL specified by -u, below.

  -p
     Automatically build patches against previous versions after
     generating the results.  Patches are difference files that users
     can download when updating a package, in lieu of redownloading
     the whole package; this happens automatically if patches are
     present.  You should generally build patches when you are
     committing to a final, public-facing release.  Patches are
     usually a good idea, but generating a patch for each internal
     test build may needlessly generate a lot of small, inefficient
     patch files instead of a few larger ones.  You can also generate
     patches after the fact, by running ppatcher on the install
     directory.

  -s search_dir
     Additional directories to search for previously-built packages.
     This option may be repeated as necessary.  These directories may
     also be specified with the pdef-path Config.prc variable.

  -d persist_dir
     The full path to a local directory that retains persistant state
     between publishes.  This directory structure keeps files that are
     used to build patches for future releases.  You should keep this
     directory structure around for as long as you plan to support
     this package.  If this directory structure does not exist or is
     empty, patches will not be created for this publish; but the
     directory structure will be populated for the next publish.

  -u host_url
     Specifies the URL to the download server that will eventually
     host these packages (that is, the public URL of the install
     directory).  This may also be overridden with a "host" command
     appearing within the pdef file.  This is used for packages only;
     it is ignored for p3d applications, which are not specific to a
     particular host.

  -n "host descriptive name"
     Specifies a descriptive name of the download server named by -u.
     This name may be presented to the user when managing installed
     packages.  This may also be overridden with a "host" command
     appearing within the pdef file.  This information is written to
     the contents.xml file at the top of the install directory.

  -D
     Sets the allow_python_dev flag in any applications built with
     this command.  This enables additional runtime debug operations,
     particularly the -i option to the panda3d command, which enables
     a live Python prompt within the application's environment.
     Setting this flag may be useful to develop an application
     initially, but should not be set on an application intended for
     deployment.

  -P platform
     Specify the platform to masquerade as.  The default is whatever
     platform Panda has been built for.  It is probably unwise to set
     this, unless you know what you are doing.

  -h
     Display this help
"""

import sys
import getopt
import os

from direct.p3d import Packager
from pandac.PandaModules import *

def usage(code, msg = ''):
    print >> sys.stderr, __doc__ % {'prog' : os.path.split(sys.argv[0])[1]}
    print >> sys.stderr, msg
    sys.exit(code)

packager = Packager.Packager()
buildPatches = False

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:ps:d:DP:u:n:h')
except getopt.error, msg:
    usage(1, msg)

for opt, arg in opts:
    if opt == '-i':
        packager.installDir = Filename.fromOsSpecific(arg)
    elif opt == '-p':
        buildPatches = True
    elif opt == '-s':
        packager.installSearch.appendDirectory(Filename.fromOsSpecific(arg))
    elif opt == '-d':
        packager.persistDir = Filename.fromOsSpecific(arg)
    elif opt == '-D':
        packager.allowPythonDev = True
    elif opt == '-P':
        packager.platform = arg
    elif opt == '-u':
        packager.host = arg
    elif opt == '-n':
        packager.hostDescriptiveName = arg
        
    elif opt == '-h':
        usage(0)
    else:
        print 'illegal option: ' + flag
        sys.exit(1)

if not args:
    usage(0)

packageDef = Filename.fromOsSpecific(args[0])
packageNames = None
if len(args) > 1:
    packageNames = args[1:]

if not packager.installDir:
    packager.installDir = Filename('install')
packager.installSearch.prependDirectory(packager.installDir)

try:
    packager.setup()
    packages = packager.readPackageDef(packageDef, packageNames = packageNames)
    packager.close()
    if buildPatches:
        packager.buildPatches(packages)
        
except Packager.PackagerError:
    # Just print the error message and exit gracefully.
    inst = sys.exc_info()[1]
    print inst.args[0]
    #raise
    sys.exit(1)
