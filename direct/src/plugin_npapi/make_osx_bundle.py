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
from pandac.PandaModules import Filename

def usage(code, msg = ''):
    print >> sys.stderr, __doc__
    print >> sys.stderr, msg
    sys.exit(code)

def makeBundle(startDir):
    fstartDir = Filename.fromOsSpecific(startDir)

    # First, make sure there is only one Opt?-* directory, to avoid
    # ambiguity.
    optDirs = glob.glob(fstartDir.toOsSpecific() + '/Opt?-*')
    if len(optDirs) == 0:
        raise StandardError, 'Application has not yet been compiled.'
    if len(optDirs) > 1:
        raise StandardError, 'Too many compiled directories; ambiguous.'
    optDir = optDirs[0]

    # Generate the bundle directory structure
    rootFilename = Filename(fstartDir, 'bundle')
    bundleFilename = Filename(rootFilename, 'nppanda3d.plugin')
    plistFilename = Filename(bundleFilename, 'Contents/Info.plist')
    plistFilename.makeDir()
    exeFilename = Filename(bundleFilename, 'Contents/MacOS/nppanda3d')
    exeFilename.makeDir()
    resourceFilename = Filename(bundleFilename, 'Contents/Resources/nppanda3d.rsrc')
    resourceFilename.makeDir()
    
    # Compile the .r file to an .rsrc file.
    os.system('/Developer/Tools/Rez -useDF -o %s %s' % (
        resourceFilename.toOsSpecific(), Filename(fStartDir, "nppanda3d.r").toOsSpecific()))
    tfile.unlink()

    if not resourceFilename.exists():
        raise IOError, 'Unable to run Rez'

    # Copy in Info.plist and the compiled executable.
    shutil.copyfile(Filename(fstartDir, "nppanda3d.plist").toOsSpecific(), plistFilename.toOsSpecific())
    shutil.copyfile(optDir + '/nppanda3d', exeFilename.toOsSpecific())

    # All done!
    bundleFilename.touch()
    print bundleFilename.toOsSpecific()

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
    shutil.rmtree(rootFilename.toOsSpecific())

if __name__ == '__main__':
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'h')
    except getopt.error, msg:
        usage(1, msg)

    for opt, arg in opts:
        if opt == '-h':
            usage(0)

    if args:
        usage(1, 'No arguments are expected.')

    startDir = os.path.split(sys.argv[0])[0]
    makeBundle(startDir)
    buildDmg(startDir)
    
