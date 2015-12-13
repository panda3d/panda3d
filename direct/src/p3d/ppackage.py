#! /usr/bin/env python

usageText = """
This script can be used to produce a Panda3D downloadable "package",
which may contain arbitrary files--for instance, Python code, bam
files, and/or compiled DLL's--and which may be downloaded by
application code to extend an application at runtime.

In addition to packages, this script can also be used to build
standalone p3d applications, that is, packaged Python code in a p3d
file, for execution by the Panda3D plugin or runtime.  (But also see
packp3d, which is designed to be a simpler interface for building
applications.)

This command will build p3d files that reference Panda3D %(version)s,
from host %(host)s .

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
    packages are built, and packages that cannot be built are silently
    ignored.

Options:

  -i install_dir
     The full path to a local directory to copy the
     ready-to-be-published files into.  This directory structure may
     contain multiple different packages from multiple different
     invocations of this script.  It is the user's responsibility to
     copy this directory structure to a server, which will have the
     URL specified by the packager.setHost() call appearing within the
     pdef file.

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

  -S file.crt[,chain.crt[,file.key[,\"password\"]]]
     Signs any resulting p3d file(s) with the indicated certificate.
     You may specify the signing certificate, the optional
     authorization chain, and the private key in three different
     files, or they may all be combined in the first file.  If the
     private key is encrypted, the password will be required to
     decrypt it.

  -D
     Sets the allow_python_dev flag in any applications built with
     this command.  This enables additional runtime debug operations,
     particularly the -i option to the panda3d command, which enables
     a live Python prompt within the application's environment.
     Setting this flag may be useful to develop an application
     initially, but should not be set on an application intended for
     deployment.

  -N
     If this option is set, Packager will not try to compile any Python
     files to .pyc or .pyo, instead storing the original source files.

  -u
     On the Mac OSX platform, this means that Panda was built with
     universal binaries, and the package should be built that way as
     well (that is, a version of the the package should be created for
     each supported architecture).  On other platforms, this option
     does nothing.  This is therefore safe to apply in all cases, if
     you wish to take advantage of universal binaries.  This is
     equivalent to "-P osx_i386 -P osx_amd64" on Mac platforms.

  -P platform
     Specify the platform to masquerade as.  The default is whatever
     platform Panda has been built for.  You can use this on Mac OSX
     in order to build packages for an alternate architecture if you
     have built Panda with universal binaries; you may repeat this
     option for each architecture you wish to support.  For other
     platforms, it is probably a mistake to set this.  However, see
     the option -u.

  -R sysroot
     Specify the sysroot that these files were compiled against.  This
     will shadow the system shared libraries, so that alternate
     versions are used instead of the system versions.  If any program
     references a library, say /usr/lib/libfoo.so, and
     /sysroot/usr/lib/libfoo.so exists instead, that file will be used
     instead of the system library.  This is particularly useful for
     cross-compilation.  At the moment, this is supported only on OSX.

  -H
     Treats a packager.setHost() call in the pdef file as if it were
     merely a call to packager.addHost().  This allows producing a
     package for an alternate host than its normally configured host,
     which is sometimes useful in development.

  -a suffix
     Appends the given suffix to the p3d filename, before the extension.
     This is useful when the same packages are compiled several times
     but using different settings, and you want to mark those packages
     as such.  This only applies to .p3d packages, not to other types
     of packages!

  -v
     Emit a warning for any file not recognized by the dir() command
     (indicating there may be a need for addExtensions(...)).

  -h
     Display this help
"""

import sys
import getopt
import os

from direct.p3d import Packager
from panda3d.core import *

def usage(code, msg = ''):
    sys.stderr.write(usageText % {
        'version' : PandaSystem.getPackageVersionString(),
        'host' : PandaSystem.getPackageHostUrl(),
        'prog' : os.path.split(sys.argv[0])[1]
        })
    sys.stderr.write(msg + '\n')
    sys.exit(code)

installDir = None
buildPatches = False
installSearch = []
signParams = []
allowPythonDev = False
storePythonSource = False
universalBinaries = False
systemRoot = None
ignoreSetHost = False
verbosePrint = False
p3dSuffix = ''
platforms = []

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:ps:S:DNuP:R:Ha:hv')
except getopt.error as msg:
    usage(1, msg)

for opt, arg in opts:
    if opt == '-i':
        installDir = Filename.fromOsSpecific(arg)
    elif opt == '-p':
        buildPatches = True
    elif opt == '-s':
        installSearch.append(Filename.fromOsSpecific(arg))
    elif opt == '-S':
        tokens = arg.split(',')
        while len(tokens) < 4:
            tokens.append('')
        certificate, chain, pkey, password = tokens[:4]
        signParams.append((Filename.fromOsSpecific(certificate),
                           Filename.fromOsSpecific(chain),
                           Filename.fromOsSpecific(pkey),
                           Filename.fromOsSpecific(password)))
    elif opt == '-D':
        allowPythonDev = True
    elif opt == '-N':
        storePythonSource = True
    elif opt == '-u':
        universalBinaries = True
    elif opt == '-P':
        platforms.append(arg)
    elif opt == '-R':
        systemRoot = arg
    elif opt == '-H':
        ignoreSetHost = True
    elif opt == '-a':
        p3dSuffix = arg

    elif opt == '-v':
        verbosePrint = True

    elif opt == '-h':
        usage(0)
    else:
        print('illegal option: ' + arg)
        sys.exit(1)

if not args:
    usage(0)

packageDef = Filename.fromOsSpecific(args[0])
packageNames = None
if len(args) > 1:
    packageNames = args[1:]

# Add the directory containing the pdef file itself to sys.path, to
# help the Packager locate modules where a pathname isn't specified.
dirname = packageDef.getDirname()
if dirname:
    sys.path.append(Filename(dirname).toOsSpecific())
else:
    sys.path.append('.')

if universalBinaries:
    if platforms:
        print('\nYou may not specify both -u and -P.\n')
        sys.exit(1)
    if PandaSystem.getPlatform().startswith('osx_'):
        platforms = ['osx_i386', 'osx_amd64']

if not platforms:
    platforms = [PandaSystem.getPlatform()]

for platform in platforms:
    packager = Packager.Packager(platform = platform)
    packager.installDir = installDir
    packager.installSearch = installSearch + packager.installSearch
    if installDir is not None:
        packager.installSearch = [installDir] + packager.installSearch
    packager.signParams = signParams
    packager.allowPythonDev = allowPythonDev
    packager.storePythonSource = storePythonSource
    packager.systemRoot = systemRoot
    packager.ignoreSetHost = ignoreSetHost
    packager.verbosePrint = verbosePrint
    packager.p3dSuffix = p3dSuffix

    try:
        packager.setup()
        packages = packager.readPackageDef(packageDef, packageNames = packageNames)
        packager.close()
        if buildPatches:
            packager.buildPatches(packages)

    except Packager.PackagerError:
        # Just print the error message and exit gracefully.
        inst = sys.exc_info()[1]
        print(inst.args[0])
        sys.exit(1)

# An explicit call to exit() is required to exit the program, when
# this module is packaged in a p3d file.
sys.exit(0)
