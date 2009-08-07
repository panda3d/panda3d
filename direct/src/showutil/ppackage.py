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

  ppackage.py [opts] package.pdef command

Required:

  package.pdef
    The config file that describes the contents of the package file(s)
    to be built, in excruciating detail.  Use "ppackage.py -H" to
    describe the syntax of this file.

  command
    The action to perform.  The following commands are supported:

    "build": Builds the package file(s) named in the package.pdef, and
        places the result in the install_dir.  Does not attempt to
        generate any patches.

    "publish": Builds a package file, as above, and then generates
        patches against previous builds, and updates install_dir and
        persist_dir appropriately.  Instead of the current directory,
        the built package file(s) are placed in the install_dir where
        they may be downloaded.

Options:

  -i install_dir
     The full path to a local directory to copy the
     ready-to-be-published files into.  This directory structure is
     populated by the "publish" command, and it may contain multiple
     different packages from multiple different invocations of the
     "publish" command.  It is the user's responsibility to copy this
     directory structure to a web host where it may be downloaded by
     the client.

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


#
# package.pdef syntax:
#
#   multifile <mfname> <phase>
#
#     Begins a new multifile.  All files named after this line and
#     until the next multifile line will be a part of this multifile.
#
#     <mfname>
#       The filename of the multifile, no directory.
#
#     <phase>
#       The numeric phase in which this multifile should be downloaded.
#
#
#   file <extractFlag> <filename> <dirname> <platforms>
#
#     Adds a single file to the current multifile.
#
#     <extractFlag>
#       One of:
#         0 - Leave this file within the multifile; it can be read by
#             Panda from there.
#         1 - Extract this file from the multifile, but do not bother
#             to hash check it on restart.
#         2 - Extract this file, and hash check it every time the
#             client starts, to ensure it is not changed.
#
#     <filename>
#       The name of the file to add.  This is the full path to the
#       file on the publishing machine at the time this script is run.
#
#     <dirname>
#       The directory in which to install the file, on the client.
#       This should be a relative pathname from the game directory.
#       The file is written to the multifile with its directory part
#       taken from this, and its basename taken from the source
#       filename, above.  Also, if the file is extracted, it will be
#       written into this directory on the client machine.
#
#       The directory name "toplevel" is treated as a special case;
#       this maps to the game directory itself, and is used for files
#       in the initial download.
#
#     <platforms>
#       A comma-delimited list of platforms for which this file should
#       be included with the distribution.  Presently, the only
#       options are WIN32 and/or OSX.
#
#
#   dir <extractFlag> <localDirname> <dirname>
#
#     Adds an entire directory tree to the current multifile.  The
#     named directory is searched recursively and all files found are
#     added to the current multifile, as if they were listed one a
#     time with a file command.
#
#     <extractFlag>
#       (Same as for the file command, above.)
#
#     <localDirname>
#       The name of the local directory to scan on the publishing
#       machine.
#
#     <dirname>
#       The name of the corresponding local directory on the client
#       machine; similar to <dirname> in the file command, above.
#
#
#   module modulename
#
#     Adds the named Python module to the exe or dll archive.  All files
#     named by module, until the next freeze_exe or freeze_dll command (below),
#     will be compiled and placed into the same archive.  
#
#   exclude_module modulename
#
#     Excludes the named Python module from the archive.  This module
#     will not be included in the archive, but an import command may
#     still find it if a .py file exists on disk.
#
#   forbid_module modulename
#
#     Excludes the named Python module from the archive.  This module
#     will specifically never be imported by the resulting executable,
#     even if a .py file exists on disk.  (However, a module command
#     appearing in a later phase--e.g. Phase3.pyd--can override an
#     earlier forbid_module command.)
#
#   dc_module file.dc
#
#     Adds the modules imported by the indicated dc file to the
#     archive.  Normally this is not necessary if the file.dc is
#     explicitly included in the package.pdef; but this command may be
#     useful if the file.dc is imported in a later phase.
#
#   freeze_exe <extractFlag> <exeFilename> <mainModule> <dirname>
#
#     <extractFlag>
#       (Same as for the file command, above.)
#
#     <exeFilename>
#       The name of the executable file to generate.  Do not include
#       an extension name; on Windows, the default extension is .exe;
#       on OSX there is no extension.
#
#     <mainModule>
#       The name of the python module that will be invoked first (like
#       main()) when the resulting executable is started.
#
#     <dirname>
#       (Same as for the file command, above.)
#
#   freeze_dll <extractFlag> <dllFilename> <dirname>
#
#     <extractFlag>
#       (Same as for the file command, above.)
#
#     <dllFilename>
#       The name of the shared library file to generate.  Do not include
#       an extension name; on Windows, the default extension is .pyd;
#       on OSX the extension is .so.
#
#     <dirname>
#       (Same as for the file command, above.)

import sys
import getopt
import os

from direct.showutil import Packager
from pandac.PandaModules import *

def usage(code, msg = ''):
    print >> sys.stderr, __doc__
    print >> sys.stderr, msg
    sys.exit(code)

packager = Packager.Packager()

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:d:p:Hh')
except getopt.error, msg:
    usage(1, msg)

for opt, arg in opts:
    if opt == '-i':
        packager.installDir = Filename.fromOsSpecific(arg)
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
    
if len(args) != 2:
    usage(1)

packageDef = Filename.fromOsSpecific(args[0])
command = args[1]


if command == 'build':
    #packager.doBuild()
    if not packager.persistDir:
        packager.persistDir = Filename('.')
    packager.setup()
    packager.readPackageDef(packageDef)
elif command == 'publish':
    packager.setup()
    packager.doPublish()
else:
    print 'Undefined command: ' + command
    sys.exit(1)
    

