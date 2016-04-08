#! /usr/bin/env python

usageText = """

This command will pack a Panda application, consisting of a directory
tree of .py files and models, into a p3d file for convenient
distribution.  The resulting p3d file can be run by the Panda3D
runtime executable, or by the Panda3D web browser plugin.

This command will build p3d files that reference Panda3D %s,
from host %s .

Also see ppackage, a more powerful (but more complex) tool that can
also be used to build p3d applications, using a pdef description file.

Usage:

  %s [opts] -o app.p3d

Options:

  -o app.p3d
     Specify the name of the p3d file to generate.  This is required.

  -d application_root
     Specify the root directory of the application source; this is a
     directory tree that contains all of your .py files and models.
     If this is omitted, the default is the current directory.

  -m main.py
     Names the Python file that begins the application.  This should
     be a file within the root directory. If this is omitted, the
     default is a file named "main.py", or if there is only one Python
     file present, it is used.  If this file contains a function
     called main(), that function will be called after importing it
     (this is preferable to having the module start itself immediately
     upon importing).

  -S file.crt[,chain.crt[,file.key[,\"password\"]]]
     Signs the resulting p3d with the indicated certificate.  You may
     specify the signing certificate, the optional authorization
     chain, and the private key in three different files, or they may
     all be combined in the first file.  If the private key is
     encrypted, the password will be required to decrypt it.

  -e ext
     Adds a new extension to be processed as a generic, compressible
     file type.  Do not include the leading dot.  Files matching this
     extension found within the root directory will be automatically
     added to the p3d file, in compressed form.  This option may be
     repeated as necessary.

  -n ext
     Adds a new extension to be processed as a noncompressible file
     type.  Files matching this extension will be added to the p3d
     file, in their original, uncompressed form.  You should use this
     instead of -e for files that are uncompressible by their nature
     (e.g. mpg files).  This option may be repeated as necessary.

  -x ext
     Marks files with the given extensions of needing to be physically
     extracted to disk before they can be loaded.  This is used for
     file types that cannot be loaded via the virtual file system,
     such as .ico files on Windows.
     This option is currently only implemented when deploying the
     application with pdeploy.

  -p python_lib_dir
     Adds a directory to search for additional Python modules.  You
     can use this to add your system's Python path, to allow packp3d
     to find any system modules not included in the standard Panda3D
     release, but your version of Python must match this one (%s).
     This option may be repeated to add multiple directories.

  -c config=value
     Sets the indicated config flag in the application.  This option
     may be repeated as necessary.

  -r package[,version[,hostURL]]
     Names an additional package that this application requires at
     startup time.  The default package is 'panda3d'; you may repeat
     this option to indicate dependencies on additional packages.

  -s search_dir
     Additional directories to search for previously-built packages.
     This option may be repeated as necessary.  These directories may
     also be specified with the pdef-path Config.prc variable.

  -D
     Sets the allow_python_dev flag in the application.  This enables
     additional runtime debug operations, particularly the -i option
     to the panda3d command, which enables a live Python prompt within
     the application's environment.  Setting this flag may be useful
     to develop an application initially, but should not be set on an
     application intended for deployment.

"""

import sys
import os
import getopt
import glob
from direct.p3d import Packager
from panda3d.core import *

# Temp hack for debugging.
#from direct.p3d.AppRunner import dummyAppRunner; dummyAppRunner()

class ArgumentError(Exception):
    pass

def makePackedApp(args):
    opts, args = getopt.getopt(args, 'o:d:m:S:e:n:x:p:c:r:s:Dh')

    packager = Packager.Packager()

    appFilename = None
    root = Filename('.')
    main = None
    configFlags = []
    requires = []
    allowPythonDev = False

    for option, value in opts:
        if option == '-o':
            appFilename = Filename.fromOsSpecific(value)
        elif option == '-d':
            root = Filename.fromOsSpecific(value)
        elif option == '-m':
            main = value
        elif option == '-S':
            tokens = value.split(',')
            while len(tokens) < 4:
                tokens.append('')
            certificate, chain, pkey, password = tokens[:4]
            packager.signParams.append((Filename.fromOsSpecific(certificate),
                                        Filename.fromOsSpecific(chain),
                                        Filename.fromOsSpecific(pkey),
                                        Filename.fromOsSpecific(password)))
        elif option == '-e':
            packager.binaryExtensions.append(value)
        elif option == '-n':
            packager.uncompressibleExtensions.append(value)
        elif option == '-x':
            packager.extractExtensions.append(value)
        elif option == '-p':
            sys.path.append(value)
        elif option == '-c':
            configFlags.append(value.split('=', 1))
        elif option == '-r':
            tokens = value.split(',')
            while len(tokens) < 3:
                tokens.append('')
            name, version, host = tokens[:3]
            requires.append((name, version, host))
        elif option == '-s':
            packager.installSearch.append(Filename.fromOsSpecific(value))
        elif option == '-D':
            allowPythonDev = True
        elif option == '-h':
            print(usageText % (
                PandaSystem.getPackageVersionString(),
                PandaSystem.getPackageHostUrl(),
                os.path.split(sys.argv[0])[1],
                '%s.%s' % (sys.version_info[0], sys.version_info[1])))
            sys.exit(0)

    if not appFilename:
        raise ArgumentError("No target app specified.  Use:\n  %s -o app.p3d\nUse -h to get more usage information." % (os.path.split(sys.argv[0])[1]))

    if args:
        raise ArgumentError("Extra arguments on command line.")

    if appFilename.getExtension() != 'p3d':
        raise ArgumentError('Application filename must end in ".p3d".')

    appDir = Filename(appFilename.getDirname())
    if not appDir:
      appDir = Filename('.')
    appBase = appFilename.getBasenameWoExtension()

    if main:
        main = Filename.fromOsSpecific(main)
        main.makeAbsolute(root)
    else:
        main = Filename(root, 'main.py')
        if not main.exists():
            main = glob.glob(os.path.join(root.toOsSpecific(), '*.py'))
            if len(main) == 0:
                raise ArgumentError('No Python files in root directory.')
            elif len(main) > 1:
                raise ArgumentError('Multiple Python files in root directory; specify the main application with -m "main".')

            main = Filename.fromOsSpecific(os.path.split(main[0])[1])
            main.makeAbsolute(root)

    packager.installDir = appDir
    packager.allowPythonDev = allowPythonDev

    # Put the root directory on the front of the model-path, so that
    # any texture references in egg or bam files that reference
    # textures from the top of the root directory will be properly
    # resolved.
    getModelPath().prependDirectory(root)

    try:
        packager.setup()
        packager.beginPackage(appBase, p3dApplication = True)

        # Pre-require panda3d, to give a less-confusing error message
        # if one of our requirements pulls in a wrong version of
        # panda3d.
        if 'panda3d' not in [t[0] for t in requires]:
            packager.do_require('panda3d')

        for name, version, host in requires:
            packager.do_require(name, version = version, host = host)

        if configFlags:
            packager.do_config(**dict(configFlags))

        packager.do_dir(root)
        packager.do_main(main)
        packager.endPackage()
        packager.close()

    except Packager.PackagerError:
        # Just print the error message and exit gracefully.
        inst = sys.exc_info()[1]
        print(inst.args[0])
        sys.exit(1)

try:
    makePackedApp(sys.argv[1:])
except ArgumentError as e:
    print(e.args[0])
    sys.exit(1)

# An explicit call to exit() is required to exit the program, when
# this module is packaged in a p3d file.
sys.exit(0)
