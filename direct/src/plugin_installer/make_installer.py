#! /usr/bin/env python

import os
import sys
from optparse import OptionParser
import subprocess
import direct
from pandac.PandaModules import *

usage = """
This command invokes NSIS to construct a Windows installer for the
Panda3D plugin and runtime environment.

  %prog [opts]"""

parser = OptionParser(usage = usage)
parser.add_option('-n', '--short', dest = 'short_name',
                  help = 'The product short name',
                  default = 'Panda3D')
parser.add_option('-N', '--long', dest = 'long_name',
                  help = 'The product long name',
                  default = 'Panda3D Game Engine')
parser.add_option('-p', '--publisher', dest = 'publisher',
                  help = 'The name of the publisher',
                  default = 'Carnegie Mellon Entertainment Technology Center')
parser.add_option('-i', '--install', dest = 'install_dir',
                  help = "The install directory on the user's machine",
                  default = '$PROGRAMFILES\\Panda3D')
parser.add_option('-l', '--license', dest = 'license',
                  help = 'A file containing the license or EULA text',
                  default = None)
parser.add_option('-w', '--website', dest = 'website',
                  help = 'The product website',
                  default = 'http://www.panda3d.org')
parser.add_option('', '--start', dest = 'start',
                  help = 'Specify this option to add a start menu',
                  action = 'store_false', default = False)
parser.add_option('', '--nsis', dest = 'nsis',
                  help = 'The directory containing NSIS',
                  default = None)
parser.add_option('', '--welcome_image', dest = 'welcome_image',
                  help = 'The image to display on the installer',
                  default = None)
parser.add_option('', '--install_icon', dest = 'install_icon',
                  help = 'The icon to give to the installer',
                  default = None)

(options, args) = parser.parse_args()

this_dir = os.path.split(sys.argv[0])[0]

##############################################################################
#
# Locate the relevant trees.
#
##############################################################################

if not options.nsis or not options.license:
    PANDA=None
    for dir in sys.path:
        if (dir != "") and os.path.exists(os.path.join(dir,"direct")) and os.path.exists(os.path.join(dir,"pandac")):
            PANDA=os.path.abspath(dir)
    if (PANDA is None):
      sys.exit("Cannot locate the panda root directory in the python path (cannot locate directory containing direct and pandac).")
    print "PANDA located at "+PANDA

    if (os.path.exists(os.path.join(PANDA,"..","makepanda","makepanda.py"))) and (sys.platform != "win32" or os.path.exists(os.path.join(PANDA,"..","thirdparty","win-nsis","makensis.exe"))):
      PSOURCE=os.path.abspath(os.path.join(PANDA,".."))
      if (sys.platform == "win32"):
        NSIS=os.path.abspath(os.path.join(PANDA,"..","thirdparty","win-nsis"))
    else:
      PSOURCE=PANDA
      if (sys.platform == "win32"):
        NSIS=os.path.join(PANDA,"nsis")

    if not options.nsis:
        options.nsis = NSIS
    if not options.license:
        options.license = os.path.join(PANDA, 'LICENSE')

if not options.welcome_image:
    filename = Filename('plugin_images/download.jpg')
    found = filename.resolveFilename(getModelPath().getValue())
    if not found:
        found = filename.resolveFilename("models")
    if not found:
        sys.exit("Couldn't find download.jpg for welcome_image.")
    options.welcome_image = filename

def parseDependenciesWindows(tempFile):
    """ Reads the indicated temporary file, the output from
    dumpbin /dependents, to determine the list of dll's this
    executable file depends on. """

    lines = open(tempFile.toOsSpecific(), 'rU').readlines()
    li = 0
    while li < len(lines):
        line = lines[li]
        li += 1
        if line.find(' has the following dependencies') != -1:
            break

    if li < len(lines):
        line = lines[li]
        if line.strip() == '':
            # Skip a blank line.
            li += 1

    # Now we're finding filenames, until the next blank line.
    filenames = []
    while li < len(lines):
        line = lines[li]
        li += 1
        line = line.strip()
        if line == '':
            # We're done.
            return filenames
        filenames.append(line)

    # Hmm, we ran out of data.  Oh well.
    if not filenames:
        # Some parse error.
        return None

    # At least we got some data.
    return filenames

def makeInstaller():
    # Locate the plugin(s).
    pluginFiles = {}
    pluginDependencies = {}
    dependentFiles = {}

    # These are the three primary files that make up the
    # plugin/runtime.
    ocx = 'p3dactivex.ocx'
    npapi = 'nppanda3d.dll'
    panda3d = 'panda3d.exe'

    path = DSearchPath()
    path.appendPath(os.environ['PATH'])
    for file in [ocx, npapi, panda3d]:
        pathname = path.findFile(file)
        if not pathname:
            sys.exit("Couldn't find %s." % (file))

        pluginFiles[file] = pathname.toOsSpecific()
        pluginDependencies[file] = []

        # Also look for the dll's that these plugins reference.
        tempFile = Filename.temporary('', 'p3d_', '.txt')
        command = 'dumpbin /dependents "%s" >"%s"' % (
            pathname.toOsSpecific(),
            tempFile.toOsSpecific())
        try:
            os.system(command)
        except:
            pass
        filenames = None

        if tempFile.exists():
            filenames = parseDependenciesWindows(tempFile)
            tempFile.unlink()
        if filenames is None:
            sys.exit("Unable to determine dependencies from %s" % (pathname))

        # Look for MSVC[RP]*.dll, and MFC*.dll.  These dependent files
        # have to be included too.
        for dfile in filenames:
            dfile = dfile.lower()
            if dfile.startswith('msvc') or dfile.startswith('mfc'):
                pathname = path.findFile(dfile)
                if not pathname:
                    sys.exit("Couldn't find %s." % (dfile))
                pluginDependencies[file].append(dfile)
                dependentFiles[dfile] = pathname.toOsSpecific()

    welcomeBitmap = None
    if options.welcome_image:
        # Convert the image from its current format to a bmp file, for NSIS.
        p = PNMImage()
        if not p.read(options.welcome_image):
            sys.exit("Couldn't read %s" % (options.welcome_image))

        # We also must scale it to fit within 170x312, the space
        # allotted within the installer window.
        size = (170, 312)
        xscale = float(size[0]) / p.getXSize()
        yscale = float(size[1]) / p.getYSize()
        scale = min(xscale, yscale, 1)
        resized = PNMImage((int)(scale * p.getXSize() + 0.5),
                           (int)(scale * p.getYSize() + 0.5))
        resized.quickFilterFrom(p)

        # Now center it within the full window.
        result = PNMImage(size[0], size[1])
        result.fill(1, 1, 1)
        xc = (size[0] - resized.getXSize()) / 2
        yc = (size[1] - resized.getYSize()) / 2
        result.copySubImage(resized, xc, yc)

        welcomeBitmap = Filename.temporary('', 'p3d_', '.bmp')
        result.write(welcomeBitmap)

    # Now build the NSIS command.
    CMD = "\"" + options.nsis + "\\makensis.exe\" /V3 "
    CMD += '/DPRODUCT_NAME="' + options.long_name + '" '
    CMD += '/DPRODUCT_NAME_SHORT="' + options.short_name + '" '
    CMD += '/DPRODUCT_PUBLISHER="' + options.publisher + '" '
    CMD += '/DPRODUCT_WEB_SITE="' + options.website + '" '
    CMD += '/DINSTALL_DIR="' + options.install_dir + '" '
    CMD += '/DLICENSE_FILE="' + options.license + '" '
    CMD += '/DOCX="' + ocx + '" '
    CMD += '/DOCX_PATH="' + pluginFiles[ocx] + '" '
    CMD += '/DNPAPI="' + npapi + '" '
    CMD += '/DNPAPI_PATH="' + pluginFiles[npapi] + '" '
    CMD += '/DPANDA3D="' + panda3d + '" '
    CMD += '/DPANDA3D_PATH="' + pluginFiles[panda3d] + '" '

    dependencies = dependentFiles.items()
    for i in range(len(dependencies)):
        CMD += '/DDEP%s="%s" ' % (i, dependencies[i][0])
        CMD += '/DDEP%sP="%s" ' % (i, dependencies[i][1])
    dependencies = pluginDependencies[npapi]
    for i in range(len(dependencies)):
        CMD += '/DNPAPI_DEP%s="%s" ' % (i, dependencies[i])

    if options.start:
        CMD += '/DADD_START_MENU '
    
    if welcomeBitmap:
        CMD += '/DMUI_WELCOMEFINISHPAGE_BITMAP="' + welcomeBitmap.toOsSpecific() + '" '
        CMD += '/DMUI_UNWELCOMEFINISHPAGE_BITMAP="' + welcomeBitmap.toOsSpecific() + '" '
    if options.install_icon:
        CMD += '/DINSTALL_ICON="' + options.install_icon + '" '

    CMD += '"' + this_dir + '\\p3d_installer.nsi"' 
    
    print ""
    print CMD
    print "packing..."
    subprocess.call(CMD)

    if welcomeBitmap:
        welcomeBitmap.unlink()

makeInstaller()
