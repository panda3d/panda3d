#! /usr/bin/env python

"""

This script constructs the bundle directory structure for the OSX web
plugin that is built by the code in this directory.  It takes no
parameters, and produces the plugin bundle in the same place.

"""

import getopt
import sys
import os
import glob
import shutil

import direct
from panda3d.core import Filename, DSearchPath

def usage(code, msg = ''):
    sys.stderr.write(__doc__)
    sys.stderr.write(msg + '\n')
    sys.exit(code)

def makeBundle(startDir):
    fstartDir = Filename.fromOsSpecific(startDir)

    # Search for nppandad along $DYLD_LIBRARY_PATH.
    path = DSearchPath()
    if 'LD_LIBRARY_PATH' in os.environ:
        path.appendPath(os.environ['LD_LIBRARY_PATH'])
    if 'DYLD_LIBRARY_PATH' in os.environ:
        path.appendPath(os.environ['DYLD_LIBRARY_PATH'])
    nppanda3d = path.findFile('nppanda3d')
    if not nppanda3d:
        raise Exception("Couldn't find nppanda3d on path.")

    # Generate the bundle directory structure
    rootFilename = Filename(fstartDir, 'bundle')

    if os.path.exists(rootFilename.toOsSpecific()):
        shutil.rmtree(rootFilename.toOsSpecific())

    bundleFilename = Filename(rootFilename, 'nppanda3d.plugin')
    plistFilename = Filename(bundleFilename, 'Contents/Info.plist')
    plistFilename.makeDir()
    exeFilename = Filename(bundleFilename, 'Contents/MacOS/nppanda3d')
    exeFilename.makeDir()
    resourceFilename = Filename(bundleFilename, 'Contents/Resources/nppanda3d.rsrc')
    resourceFilename.makeDir()

    # Compile the .r file to an .rsrc file.
    os.system('/Developer/Tools/Rez -useDF -o %s %s' % (
        resourceFilename.toOsSpecific(), Filename(fstartDir, "nppanda3d.r").toOsSpecific()))

    if not resourceFilename.exists():
        raise IOError('Unable to run Rez')

    # Copy in Info.plist and the compiled executable.
    shutil.copyfile(Filename(fstartDir, "nppanda3d.plist").toOsSpecific(), plistFilename.toOsSpecific())
    shutil.copyfile(nppanda3d.toOsSpecific(), exeFilename.toOsSpecific())

    # All done!
    bundleFilename.touch()
    print(bundleFilename.toOsSpecific())

def buildDmg(startDir):
    fstartDir = Filename.fromOsSpecific(startDir)
    rootFilename = Filename(fstartDir, 'bundle')
    output = Filename(fstartDir, 'nppanda3d.dmg')
    output.unlink()
    cmd = 'hdiutil create -fs HFS+ -srcfolder "%(dir)s" -volname "%(volname)s" "%(output)s"' % {
        'dir' : rootFilename.toOsSpecific(),
        'volname' : 'nppanda3d',
        'output' : output.toOsSpecific(),
        }
    os.system(cmd)

if __name__ == '__main__':
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'h')
    except getopt.error as msg:
        usage(1, msg)

    for opt, arg in opts:
        if opt == '-h':
            usage(0)

    if args:
        usage(1, 'No arguments are expected.')

    startDir = os.path.split(sys.argv[0])[0]
    makeBundle(startDir)

    # We don't need the dmg these days; the installer is better.
    #buildDmg(startDir)

