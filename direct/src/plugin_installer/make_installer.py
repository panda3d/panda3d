#! /usr/bin/env python

import os
import sys
import shutil
import platform
from optparse import OptionParser
import subprocess
import direct
from pandac.PandaModules import *

usage = """
This command creates a graphical installer for the
Panda3D plugin and runtime environment.

  %prog [opts]"""

parser = OptionParser(usage = usage)
parser.add_option('-n', '--short', dest = 'short_name',
                  help = 'The product short name',
                  default = 'Panda3D')
parser.add_option('-N', '--long', dest = 'long_name',
                  help = 'The product long name',
                  default = 'Panda3D Game Engine')
parser.add_option('-v', '--version', dest = 'version',
                  help = 'The product version',
                  default = PandaSystem.getVersionString())
parser.add_option('-p', '--publisher', dest = 'publisher',
                  help = 'The name of the publisher',
                  default = 'Carnegie Mellon Entertainment Technology Center')
parser.add_option('-i', '--install', dest = 'install_dir',
                  help = "The install directory on the user's machine (Windows-only)",
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
parser.add_option('', '--welcome_image', dest = 'welcome_image',
                  help = 'The image to display on the installer',
                  default = None)
parser.add_option('', '--install_icon', dest = 'install_icon',
                  help = 'The icon to give to the installer',
                  default = None)
parser.add_option('', '--nsis', dest = 'nsis',
                  help = 'The directory containing NSIS',
                  default = None)

(options, args) = parser.parse_args()

this_dir = os.path.split(sys.argv[0])[0]

##############################################################################
#
# Locate the relevant files.
#
##############################################################################

if not options.nsis:
    if sys.platform == "win32":
        makensis = None
        for p in os.defpath.split(";") + os.environ["PATH"].split(";"):
            if os.path.isfile(os.path.join(p, "makensis.exe")):
                makensis = os.path.join(p, "makensis.exe")
        if not makensis:
            import pandac
            makensis = os.path.dirname(os.path.dirname(pandac.__file__))
            makensis = os.path.join(makensis, "nsis", "makensis.exe")
            if not os.path.isfile(makensis):
                makensis = None
        options.nsis = makensis
    else:
        for p in os.defpath.split(":") + os.environ["PATH"].split(":"):
            if os.path.isfile(os.path.join(p, "makensis")):
                options.nsis = os.path.join(p, "makensis")

if not options.license:
    import pandac
    options.license = os.path.join(os.path.dirname(os.path.dirname(pandac.__file__)), "LICENSE")
    if not os.path.isfile(options.license): options.license = None

if sys.platform == "win32" and not options.welcome_image:
    filename = Filename('plugin_images/download.png')
    found = filename.resolveFilename(getModelPath().getValue())
    if not found:
        found = filename.resolveFilename("models")
    if not found:
        sys.exit("Couldn't find download.png for welcome_image.")
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

def parseDependenciesUnix(tempFile):
    """ Reads the indicated temporary file, the output from
    otool -XL or ldd, to determine the list of dll's this
    executable file depends on. """
    
    lines = open(tempFile.toOsSpecific(), 'rU').readlines()
    filenames = []
    for l in lines:
        filenames.append(l.strip().split(' ', 1)[0])
    return filenames

def addDependencies(path, pathname, file, pluginDependencies, dependentFiles):
    """ Checks the named file for DLL dependencies, and adds any
    appropriate dependencies found into pluginDependencies and
    dependentFiles. """
    
    tempFile = Filename.temporary('', 'p3d_', '.txt')
    if sys.platform == "darwin":
        command = 'otool -XL "%s" >"%s"'
    elif sys.platform == "win32":
        command = 'dumpbin /dependents "%s" >"%s"'
    else:
        command = 'ldd "%s" >"%s"'
    command = command % (
        pathname.toOsSpecific(),
        tempFile.toOsSpecific())
    try:
        os.system(command)
    except:
        pass
    filenames = None

    if tempFile.exists():
        if sys.platform == "win32":
            filenames = parseDependenciesWindows(tempFile)
        else:
            filenames = parseDependenciesUnix(tempFile)
        tempFile.unlink()
    if filenames is None:
        sys.exit("Unable to determine dependencies from %s" % (pathname))

    # Look for MSVC[RP]*.dll, and MFC*.dll.  These dependent files
    # have to be included too.  Also, any Panda-based libraries, or
    # the Python DLL, should be included, in case panda3d.exe wasn't
    # built static.  The Panda-based libraries begin with "lib" and
    # are all lowercase.
    for dfile in filenames:
        dfilelower = dfile.lower()
        if dfilelower not in dependentFiles:
            if dfilelower.startswith('msvc') or \
               dfilelower.startswith('mfc') or \
               (dfile.startswith('lib') and dfile == dfilelower) or \
               dfilelower.startswith('python'):
                pathname = path.findFile(dfile)
                if not pathname:
                    sys.exit("Couldn't find %s." % (dfile))
                dependentFiles[dfilelower] = pathname.toOsSpecific()

                # Also recurse.
                addDependencies(path, pathname, file, pluginDependencies, dependentFiles)

        if dfilelower in dependentFiles:
            pluginDependencies[file].append(dfilelower)

def makeInstaller():
    # Locate the plugin(s).
    pluginFiles = {}
    pluginDependencies = {}
    dependentFiles = {}

    # These are the primary files that make
    # up the plugin/runtime.
    if sys.platform == "darwin":
        npapi = 'nppanda3d.plugin'
        panda3d = 'Panda3D.app'
        baseFiles = [npapi, panda3d]
    else:
        ocx = 'p3dactivex.ocx'
        npapi = 'nppanda3d.dll'
        panda3d = 'panda3d.exe'
        panda3dw = 'panda3dw.exe'
        baseFiles = [ocx, npapi, panda3d, panda3dw]

    path = DSearchPath()
    if 'PATH' in os.environ:
        path.appendPath(os.environ['PATH'])
    if sys.platform != "win32" and 'LD_LIBRARY_PATH' in os.environ:
        path.appendPath(os.environ['LD_LIBRARY_PATH'])
    if sys.platform == "darwin" and 'DYLD_LIBRARY_PATH' in os.environ:
        path.appendPath(os.environ['DYLD_LIBRARY_PATH'])
    path.appendPath(os.defpath)
    for file in baseFiles:
        pathname = path.findFile(file)
        if not pathname:
            sys.exit("Couldn't find %s." % (file))

        pluginFiles[file] = pathname.toOsSpecific()
        pluginDependencies[file] = []

        if sys.platform == "win32":
            # Also look for the dll's that these plugins reference.
            addDependencies(path, pathname, file, pluginDependencies, dependentFiles)

    if sys.platform == "darwin":
        tmproot = Filename("/var/tmp/Panda3D Runtime/")
        if tmproot.exists():
            shutil.rmtree(tmproot.toOsSpecific())
        tmproot.makeDir()
        dst_npapi = Filename(tmproot, Filename("Library/Internet Plug-Ins", npapi))
        dst_panda3d = Filename(tmproot, Filename("Applications", panda3d))
        dst_npapi.makeDir()
        dst_panda3d.makeDir()
        shutil.copytree(pluginFiles[npapi], dst_npapi.toOsSpecific())
        shutil.copyfile(pluginFiles[panda3d], dst_panda3d.toOsSpecific())
        CMD = "/Developer/usr/bin/packagemaker"
        CMD += ' --id org.panda3d.runtime' #TODO: make this customizable
        CMD += ' --version "%s"' % options.version
        CMD += ' --title "%s"' % options.long_name
        CMD += ' --out p3d-setup.pkg'
        CMD += ' --target %s' % platform.mac_ver()[0][:4]
        CMD += ' --domain system'
        CMD += ' --root "%s"' % tmproot.toOsSpecific()
        
        print ""
        print CMD
        subprocess.call(CMD, shell = True)
        shutil.rmtree(tmproot.toOsSpecific())
        
        # Pack the .pkg into a .dmg
        tmproot.makeDir()
        shutil.copyfile("p3d-setup.pkg", Filename(tmproot, "p3d-setup.pkg").toOsSpecific())
        tmpdmg = Filename.temporary("", "p3d-setup", "").toOsSpecific() + ".dmg"
        CMD = 'hdiutil create "%s" -srcfolder "%s"' % (tmpdmg, tmproot)
        print ""
        print CMD
        subprocess.call(CMD, shell = True)
        shutil.rmtree(tmproot.toOsSpecific())
        
        # Compress the .dmg (and make it read-only)
        CMD = 'hdiutil convert "%s" -format UDBZ -o "p3d-setup.dmg"' % tmpdmg
        print ""
        print CMD
        subprocess.call(CMD, shell = True)
        
    else:
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
        CMD += '/DPANDA3DW="' + panda3dw + '" '
        CMD += '/DPANDA3DW_PATH="' + pluginFiles[panda3dw] + '" '

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
