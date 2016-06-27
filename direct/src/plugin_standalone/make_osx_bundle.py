#! /usr/bin/env python

"""

This script constructs the bundle directory structure for the OSX
program panda3d_mac, which is built by the code in this directory.  It
takes no parameters, and produces the app bundle in the same place.

"""

import getopt
import sys
import os
import glob
import shutil

import direct
from panda3d.core import Filename, DSearchPath, getModelPath, ExecutionEnvironment

def usage(code, msg = ''):
    sys.stderr.write(__doc__)
    sys.stderr.write(msg + '\n')
    sys.exit(code)

def makeBundle(startDir):
    fstartDir = Filename.fromOsSpecific(startDir)

    # Search for panda3d_mac along $PATH.
    path = DSearchPath()
    if 'PATH' in os.environ:
        path.appendPath(os.environ['PATH'])
    path.appendPath(os.defpath)
    panda3d_mac = path.findFile('panda3d_mac')
    if not panda3d_mac:
        raise Exception("Couldn't find panda3d_mac on path.")

    # Construct a search path to look for the images.
    search = DSearchPath()

    # First on the path: an explicit $PLUGIN_IMAGES env var.
    if ExecutionEnvironment.hasEnvironmentVariable('PLUGIN_IMAGES'):
        search.appendDirectory(Filename.expandFrom('$PLUGIN_IMAGES'))

    # Next on the path: the models/plugin_images directory within the
    # current directory.
    search.appendDirectory('models/plugin_images')

    # Finally on the path: models/plugin_images within the model
    # search path.
    for dir in getModelPath().getDirectories():
        search.appendDirectory(Filename(dir, 'plugin_images'))

    # Now find the icon file on the above search path.
    icons = search.findFile('panda3d.icns')
    if not icons:
        raise Exception("Couldn't find panda3d.icns on model-path.")

    # Generate the bundle directory structure
    rootFilename = Filename(fstartDir)
    bundleFilename = Filename(rootFilename, 'Panda3D.app')
    if os.path.exists(bundleFilename.toOsSpecific()):
        shutil.rmtree(bundleFilename.toOsSpecific())

    plistFilename = Filename(bundleFilename, 'Contents/Info.plist')
    plistFilename.makeDir()
    exeFilename = Filename(bundleFilename, 'Contents/MacOS/panda3d_mac')
    exeFilename.makeDir()
    iconFilename = Filename(bundleFilename, 'Contents/Resources/panda3d.icns')
    iconFilename.makeDir()

    # Copy in Info.plist, the icon file, and the compiled executable.
    shutil.copyfile(Filename(fstartDir, "panda3d_mac.plist").toOsSpecific(), plistFilename.toOsSpecific())
    shutil.copyfile(icons.toOsSpecific(), iconFilename.toOsSpecific())
    print('%s %s' % (panda3d_mac, exeFilename))
    shutil.copyfile(panda3d_mac.toOsSpecific(), exeFilename.toOsSpecific())
    os.chmod(exeFilename.toOsSpecific(), 0o755)

    # All done!
    bundleFilename.touch()
    print(bundleFilename.toOsSpecific())

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

