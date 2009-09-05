""" This module is used to build a graphical installer
from a p3d file. It will try to build installers for as
many platforms as possible. """

__all__ = ["InstallerMaker"]

import os, sys, subprocess, tarfile, shutil, time
from direct.directnotify.DirectNotifyGlobal import *
from pandac.PandaModules import Filename

class CachedFile:
    def __init__(self): self.str = ""
    def write(self, data): self.str += data

class InstallerMaker:
    notify = directNotify.newCategory("InstallerMaker")

    def __init__(self, shortname, fullname, p3dfile):
        self.shortname = shortname
        self.fullname = fullname
        self.p3dfile = p3dfile
        self.__makeNSIS()
        self.__makeDEB()

    def __makeDEB(self):
        InstallerMaker.notify.info("Creating %s.deb..." % self.shortname)

        # Create a temporary directory and write the control file + launcher to it
        tempdir = Filename.temporary("", self.shortname + "_deb_", "") + "/"
        tempdir.makeDir()
        tempdir = tempdir.toOsSpecific()
        controlfile = open(os.path.join(tempdir, "control"), "w")
        controlfile.write("Package: %s\n" % self.shortname)
        controlfile.write("Version: 1\n")
        controlfile.write("Section: games\n")
        controlfile.write("Priority: optional\n")
        controlfile.write("Architecture: all\n")
        controlfile.write("Depends: panda3d-runtime\n")
        controlfile.write("Description: %s\n" % self.fullname)
        controlfile.close()
        os.mkdir(os.path.join(tempdir, "usr"))
        os.mkdir(os.path.join(tempdir, "usr", "bin"))
        os.mkdir(os.path.join(tempdir, "usr", "share"))
        os.mkdir(os.path.join(tempdir, "usr", "share", "games"))
        launcherfile = open(os.path.join(tempdir, "usr", "bin", self.shortname), "w")
        launcherfile.write("#!/bin/sh\n")
        launcherfile.write("/usr/bin/panda3d /usr/share/games/%s/data.p3d\n" % self.shortname)
        launcherfile.close()
        shutil.copyfile(self.p3dfile, os.path.join(tempdir, "usr", "share", "games", "data.p3d"))

        # Create a control.tar.gz file in memory
        controltargz = CachedFile()
        controltarfile = tarfile.TarFile.gzopen("control.tar.gz", "w", controltargz)
        controltarfile.add(os.path.join(tempdir, "control"), "control")
        controltarfile.close()
        os.remove(os.path.join(tempdir, "control"))

        # Create the data.tar.gz file in the temporary directory
        datatarfile = tarfile.TarFile.gzopen(os.path.join(tempdir, "data.tar.gz"), "w")
        datatarfile.add(tempdir + "/usr", "/usr")
        datatarfile.close()

        # Open the deb file and write to it. It's actually
        # just an AR file, which is very easy to make.
        modtime = str(int(time.time())).ljust(11)
        debfile = open(self.shortname + ".deb", "wb")
        debfile.write("!<arch>\x0A")
        debfile.write("debian-binary   %s 0     0     100644  4         \x60\x0A" % modtime)
        debfile.write("2.0\x0A")
        debfile.write("control.tar.gz  %s 0     0     100644  %s \x60\x0A" % (modtime, str(len(controltargz.str)-1).ljust(9)))
        debfile.write(controltargz.str)
        debfile.write("data.tar.gz     %s 0     0     100644  %s \x60\x0A" % (modtime, str(os.path.getsize(os.path.join(tempdir, "data.tar.gz"))).ljust(9)))

        # Copy everything from data.tar.gz to the deb file megabyte by megabyte.
        datatargz = open(os.path.join(tempdir, "data.tar.gz"), "rb")
        data = datatargz.read(1024 * 1024)
        while data != "":
            debfile.write(data)
            data = datatargz.read(1024 * 1024)
        datatargz.close()
        debfile.close()

    def __makeNSIS(self):
        # Check if we have makensis first
        makensis = None
        if (sys.platform.startswith("win")):
            for p in os.defpath.split(";") + os.environ["PATH"].split(";"):
                if os.path.isfile(os.path.join(p, "makensis.exe")):
                    makensis = os.path.join(p, "makensis.exe")
            if not makensis:
                import pandac
                makensis = os.path.dirname(os.path.dirname(pandac.__file__))
                makensis = os.path.join(makensis, "nsis", "makensis.exe")
                if not os.path.isfile(makensis): makensis = None
        else:
            for p in os.defpath.split(":") + os.environ["PATH"].split(":"):
                if os.path.isfile(os.path.join(p, "makensis")):
                    makensis = os.path.join(p, "makensis")
        
        if makensis == None:
            InstallerMaker.notify.warning("Makensis utility not found, no Windows installer will be built!")
            return
        InstallerMaker.notify.info("Creating %s.exe..." % self.shortname)

        # Some global info
        nsi = 'Name "%s"' % self.fullname
        nsi += 'OutFile "%s.exe"' % self.shortname
        nsi += 'InstallDir "$PROGRAMFILES\%s"' % self.fullname
        nsi += 'SetCompress auto'
        nsi += 'SetCompressor lzma'
        nsi += 'ShowInstDetails nevershow'
        nsi += 'ShowUninstDetails nevershow'
        nsi += 'InstType "Typical"'

        # Tell Vista that we require admin rights
        nsi += 'RequestExecutionLevel admin'
        nsi += ''
        nsi += 'Function launch'
        nsi += '  ExecShell "open" "$INSTDIR\%s.bat"' % self.shortname
        nsi += 'FunctionEnd'
        nsi += ''
        nsi += '!include "MUI2.nsh"'
        nsi += '!define MUI_HEADERIMAGE'
        nsi += '!define MUI_HEADERIMAGE_BITMAP "/home/pro-rsoft/Projects/vikings-old/Vikings/installer.bmp"'
        nsi += '!define MUI_HEADERIMAGE_UNBITMAP "/home/pro-rsoft/Projects/vikings-old/Vikings/installer.bmp"'
        nsi += '!define MUI_WELCOMEFINISHPAGE_BITMAP "/home/pro-rsoft/Projects/vikings-old/Vikings/installer.bmp"'
        nsi += '!define MUI_UNWELCOMEFINISHPAGE_BITMAP "/home/pro-rsoft/Projects/vikings-old/Vikings/installer.bmp"'
        nsi += '!define MUI_ABORTWARNING'
        nsi += '!define MUI_FINISHPAGE_RUN'
        nsi += '!define MUI_FINISHPAGE_RUN_FUNCTION launch'
        nsi += '!define MUI_FINISHPAGE_RUN_TEXT "Play %s"' % self.fullname
        nsi += ''
        nsi += 'Var StartMenuFolder'
        nsi += '!insertmacro MUI_PAGE_WELCOME'
        nsi += ';!insertmacro MUI_PAGE_LICENSE "weirdo.egg"'
        nsi += '!insertmacro MUI_PAGE_DIRECTORY'
        nsi += '!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder'
        nsi += '!insertmacro MUI_PAGE_INSTFILES'
        nsi += '!insertmacro MUI_PAGE_FINISH'
        nsi += '!insertmacro MUI_UNPAGE_WELCOME'
        nsi += '!insertmacro MUI_UNPAGE_CONFIRM'
        nsi += '!insertmacro MUI_UNPAGE_INSTFILES'
        nsi += '!insertmacro MUI_UNPAGE_FINISH'
        nsi += '!insertmacro MUI_LANGUAGE "English"'

        # This section defines the installer.
        nsi += 'Section "Install"'
        nsi += '  SetOutPath "$INSTDIR"'
        nsi += '  WriteUninstaller "$INSTDIR\Uninstall.exe"'
        nsi += '  ; Start menu items'
        nsi += '  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application'
        nsi += '    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"'
        nsi += '    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"'
        nsi += '  !insertmacro MUI_STARTMENU_WRITE_END'
        nsi += 'SectionEnd'

        # This section defines the uninstaller.
        nsi += 'Section "Uninstall"'
        nsi += '  Delete "$INSTDIR\Uninstall.exe"'
        nsi += '  RMDir "$INSTDIR"'
        nsi += '  ; Start menu items'
        nsi += '  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder'
        nsi += '  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"'
        nsi += '  RMDir "$SMPROGRAMS\$StartMenuFolder"'
        nsi += 'SectionEnd'

        subprocess.call(makensis)

