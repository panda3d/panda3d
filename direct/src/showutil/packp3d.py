#! /usr/bin/env python

"""

This command will pack a Panda application, consisting of a directory
tree of .py files and models, into a p3d file for convenient
distribution.  The resulting p3d file can be run by the Panda3D
runtime executable, or by the Panda3D web browser plugin.

Also see ppackage.py, which can be used to build p3d files more
generally, using a pdef description file.

Usage:

  packp3d.py [opts] app.p3d

Options:

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

  -r package
     Names an additional package that this application requires at
     startup time.  The default package is 'panda3d'; you may repeat
     this option to indicate dependencies on additional packages.

  -s search_dir
     Additional directories to search for previously-built packages.
     This option may be repeated as necessary.

  -x
     If this is specified, a version-independent application is built.
     This stores source py files instead of compiled pyc files, and
     egg files instead of bam files.  This application can then be run
     with any version of Panda (provided you are careful not to make
     any Python or Panda calls that are version-specific).  This is
     not recommended except for very small, simple applications.

"""

import sys
import os
import getopt
import glob
import direct
from direct.showutil import Packager 
from pandac.PandaModules import *

class ArgumentError(StandardError):
    pass

def makePackedApp(args):
    opts, args = getopt.getopt(args, 'd:m:r:s:xh')

    packager = Packager.Packager()

    root = Filename('.')
    main = None
    requires = []
    versionIndependent = False
    
    for option, value in opts:
        if option == '-d':
            root = Filename.fromOsSpecific(value)
        elif option == '-m':
            main = value
        elif option == '-r':
            requires.append(value)
        elif option == '-s':
            packager.installSearch.appendDirectory(Filename.fromOsSpecific(value))
        elif option == '-x':
            versionIndependent = True
        elif option == '-h':
            print __doc__
            sys.exit(1)
    
    if not args:
        raise ArgumentError, "No destination app specified.  Use:\npackp3d.py app.p3d"

    if len(args) > 1:
        raise ArgumentError, "Too many arguments."

    appFilename = Filename.fromOsSpecific(args[0])
    if appFilename.getExtension() != 'p3d':
        raise ArgumentError, 'Application filename must end in ".p3d".'

    appDir = Filename(appFilename.getDirname())
    appBase = appFilename.getBasenameWoExtension()

    if not main:
        main = Filename(root, 'main.py')
        if main.exists():
            main = 'main.py'
        else:
            main = glob.glob(os.path.join(root.toOsSpecific(), '*.py'))
            if len(main) == 0:
                raise ArgumentError, 'No Python files in root directory.'
            elif len(main) > 1:
                raise ArgumentError, 'Multiple Python files in root directory; specify the main application with -m "main".'
            main = os.path.split(main[0])[1]

    main = Filename.fromOsSpecific(main)
    main.setExtension('')

    mainModule = main.cStr().replace('/', '.')
    
    packager.installDir = appDir
    getModelPath().appendDirectory(root)

    packager.setup()
    packager.beginPackage(appBase, p3dApplication = True)
    for requireName in requires:
        packager.require(requireName)
        
    packager.dir(root)
    packager.mainModule(mainModule)
        
    packager.endPackage(appBase, p3dApplication = True)

def main(appRunner):
    """ This function is called when this module is invoked as
    packp3d.p3d. """

    makePackedApp(appRunner.argv[1:])

if __name__ == '__main__':
    try:
        makePackedApp(sys.argv[1:])
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
