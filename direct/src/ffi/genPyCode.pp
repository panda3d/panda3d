//
// genPyCode.pp
//
// This file defines the script to auto-generate a sensible genPyCode
// for the user based on the Config.pp variables in effect at the time
// ppremake is run.  The generated script will know which directories
// to generate its output to, as well as which source files to read
// for the input.
//

#define install_dir $[$[upcase $[PACKAGE]]_INSTALL]
#define install_lib_dir $[or $[INSTALL_LIB_DIR],$[install_dir]/lib]
#define install_bin_dir $[or $[INSTALL_BIN_DIR],$[install_dir]/bin]
#define install_igatedb_dir $[or $[INSTALL_IGATEDB_DIR],$[install_dir]/etc]

#define python $[PYTHON_COMMAND]
#if $[USE_DEBUG_PYTHON]
  #define python $[PYTHON_DEBUG_COMMAND]
#endif

// If we're on Win32 without Cygwin, generate a genPyCode.bat file;
// for all other platforms, generate a genPyCode sh script.  Although
// it's true that on non-Win32 platforms we don't need the script
// (since the python file itself could be made directly executable),
// we generate the script anyway to be consistent with Win32, which
// does require it.

#if $[MAKE_BAT_SCRIPTS]

#output genPyCode.bat
@echo off
rem #### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
rem ################################# DO NOT EDIT ###########################

$[python] -u $[osfilename $[install_bin_dir]/genPyCode.py] %1 %2 %3 %4 %5 %6 %7 %8 %9
#end genPyCode.bat

#else  // MAKE_BAT_SCRIPTS

#output genPyCode $[if $[>= $[PPREMAKE_VERSION],1.21],binary]
#! /bin/sh
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
################################# DO NOT EDIT ###########################

#if $[CTPROJS]
# This script was generated while the user was using the ctattach
# tools.  That had better still be the case.
#if $[WINDOWS_PLATFORM]
$[python] -u `cygpath -w $DIRECT/built/bin/genPyCode.py` "$@"
#else
$[python] -u $DIRECT/built/bin/genPyCode.py "$@"
#endif
#else
$[python] -u '$[osfilename $[install_bin_dir]/genPyCode.py]' "$@"
#endif
#end genPyCode

#endif  // MAKE_BAT_SCRIPTS

#output genPyCode.py
#! /usr/bin/env $[python]
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
################################# DO NOT EDIT ###########################

import os
import sys
import glob

#if $[CTPROJS]
# This script was generated while the user was using the ctattach
# tools.  That had better still be the case.

def deCygwinify(path):
    if os.name in ['nt'] and path[0] == '/':
        # On Windows, we may need to convert from a Cygwin-style path
        # to a native Windows path.

        # Check for a case like /i/ or /p/: this converts
        # to i:/ or p:/.

        dirs = path.split('/')
        if len(dirs) > 2 and len(dirs[1]) == 1:
            path = '%s:\%s' % (dirs[1], '\\'.join(dirs[2:]))

        else:
            # Otherwise, prepend $PANDA_ROOT and flip the slashes.
            pandaRoot = os.getenv('PANDA_ROOT')
            if pandaRoot:
                path = os.path.normpath(pandaRoot + path)

    return path

ctprojs = os.getenv('CTPROJS')
if not ctprojs:
    print "You are no longer attached to any trees!"
    sys.exit(1)
    
directDir = os.getenv('DIRECT')
if not directDir:
    print "You are not attached to DIRECT!"
    sys.exit(1)

directDir = deCygwinify(directDir)

# Make sure that direct.showbase.FindCtaPaths gets imported.
parent, base = os.path.split(directDir)

if parent not in sys.path:
    sys.path.append(parent)

import direct.showbase.FindCtaPaths

#endif  // CTPROJS

from direct.ffi import DoGenPyCode
from direct.ffi import FFIConstants

# The following parameters were baked in to this script at the time
# ppremake was run in Direct.
#define extensions_name $[if $[PYTHON_NATIVE],extensions_native,extensions]

#if $[>= $[OPTIMIZE], 4]
FFIConstants.wantComments = 0
FFIConstants.wantTypeChecking = 0
#endif

DoGenPyCode.interrogateLib = r'libdtoolconfig'
DoGenPyCode.codeLibs = r'$[GENPYCODE_LIBS]'.split()
DoGenPyCode.native = $[if $[PYTHON_NATIVE],1,0]

#if $[not $[CTPROJS]]
// Since the user is not using ctattach, bake these variables in too.
DoGenPyCode.directDir = r'$[osfilename $[TOPDIR]]'
DoGenPyCode.outputCodeDir = r'$[osfilename $[install_lib_dir]/pandac]'
DoGenPyCode.outputHTMLDir = r'$[osfilename $[install_data_dir]/doc]'
DoGenPyCode.extensionsDir = r'$[osfilename $[TOPDIR]/src/$[extensions_name]]'
DoGenPyCode.etcPath = [r'$[osfilename $[install_igatedb_dir]]']
DoGenPyCode.pythonSourcePath = r'$[osfilename $[TOPDIR]]'

#else
# The user is expected to be using ctattach, so don't bake in the
# following four; these instead come from the dynamic settings set by
# ctattach.

DoGenPyCode.directDir = directDir
DoGenPyCode.outputCodeDir = os.path.join(directDir, 'built', 'lib', 'pandac')
DoGenPyCode.outputHTMLDir = os.path.join(directDir, 'built', 'shared', 'doc')
DoGenPyCode.extensionsDir = os.path.join(directDir, 'src', '$[extensions_name]')
DoGenPyCode.etcPath = []
DoGenPyCode.pythonSourcePath = []

#if $[CTA_GENERIC_GENPYCODE]
# Look for additional packages (other than the basic three)
# that the user might be dynamically attached to.
packages = []
for proj in ctprojs.split():
    projName = proj.split(':')[0]
    packages.append(projName)
packages.reverse()

try:
    from direct.extensions_native.extension_native_helpers import Dtool_PreloadDLL
except ImportError:
    print "Unable to import Dtool_PreloadDLL, not trying generic libraries."
else:
    for package in packages:
        packageDir = os.getenv(package)
        if packageDir:
            packageDir = deCygwinify(packageDir)
            etcDir = os.path.join(packageDir, 'etc')
            try:
                inFiles = glob.glob(os.path.join(etcDir, 'built', '*.in'))
            except:
                inFiles = []
            if inFiles:
                DoGenPyCode.etcPath.append(etcDir)

            if package not in ['WINTOOLS', 'DTOOL', 'DIRECT', 'PANDA']:
                DoGenPyCode.pythonSourcePath.append(packageDir)

                libDir = os.path.join(packageDir, 'built', 'lib')
                try:
                    files = os.listdir(libDir)
                except:
                    files = []
                for file in files:
                    if os.path.isfile(os.path.join(libDir, file)):
                        basename, ext = os.path.splitext(file)

                        # Try to import the library.  If we can import it,
                        # instrument it.
                        try:
                            Dtool_PreloadDLL(basename)
                            # __import__(basename, globals(), locals())
                            isModule = 1
                        except:
                            isModule = 0

                        # 
                        # RHH.... hack OPT2 .. py debug libraries...
                        #
                        if not isModule:
                            # debug py library magin naming in windows..
                            basename = basename.replace('_d','')                   
                            try:
                                Dtool_PreloadDLL(basename)
                                # __import__(basename, globals(), locals())
                                isModule = 1
                            except:
                                isModule = 0                        

                        if isModule:
                            if basename not in DoGenPyCode.codeLibs:
                                DoGenPyCode.codeLibs.append(basename)
#endif  // CTA_GENERIC_GENPYCODE
#endif  // CTPROJS

DoGenPyCode.run()

#end genPyCode.py
