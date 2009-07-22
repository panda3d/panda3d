#! /usr/bin/env python

"""

This script constructs the bundle directory structure for the OSX web
plugin that is built by the code in this directory.  It takes no
parameters, and produces the plugin bundle in the same place.

"""

# The contents of the Info.plist file.
InfoPlist = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en-US</string>
	<key>CFBundleExecutable</key>
	<string>nppanda3d</string>
	<key>CFBundleIdentifier</key>
	<string>org.panda3d.nppanda3d</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>Panda3D</string>
	<key>CFBundlePackageType</key>
	<string>BRPL</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
	<key>WebPluginName</key>
	<string>Panda3D Game Engine Plug-In</string>
	<key>WebPluginDescription</key>
	<string>Runs 3-D games and interactive applets</string>
	<key>WebPluginMIMETypes</key>
	<dict>
		<key>application/x-panda3d</key>
		<dict>
			<key>WebPluginExtensions</key>
			<array>
				<string>p3d</string>
			</array>
			<key>WebPluginTypeDescription</key>
			<string>Panda3D applet</string>
		</dict>
	</dict>
</dict>
</plist>
"""

# The contents of the source nppanda3d.r file.  Apparently Firefox
# ignores the Info.plist file, above, and looks only in the .rsrc file
# within the bundle, which is compiled from the following source file.
ResourceFile = """
#include <Carbon/Carbon.r>

resource 'STR#' (126) {
	{ "Runs 3-D games and interactive applets", 
          "Panda3D Game Engine Plug-In" }
};

resource 'STR#' (127) {
	{ "Panda3D applet" }
};

resource 'STR#' (128) {
	{ "application/x-panda3d", "p3d" }
};

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

    # Generate the resource file.
    tfile = Filename.temporary('', 'rsrc')
    f = open(tfile.toOsSpecific(), 'w')
    f.write(ResourceFile)
    f.close()
    
    os.system('/Developer/Tools/Rez -useDF -o %s %s' % (
        resourceFilename.toOsSpecific(), tfile.toOsSpecific()))
    tfile.unlink()

    if not resourceFilename.exists():
        raise IOError, 'Unable to run Rez'

    # Generate the Info.plist file.
    f = open(plistFilename.toOsSpecific(), 'w')
    f.write(InfoPlist)
    f.close()

    # Copy in the compiled executable.
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
    
