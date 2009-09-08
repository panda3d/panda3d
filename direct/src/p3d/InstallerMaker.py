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

    def __init__(self, shortname, fullname, p3dfile, version):
        self.shortname = shortname
        self.fullname = fullname
        self.version = str(version)
        self.licensename = ""
        # All paths given must be a Filename instance!
        assert isinstance(p3dfile, Filename)
        self.p3dfile = p3dfile
        self.licensefile = Filename()

    def build(self):
        """ Creates the installer. Call this after you have set all the parameters. """
        self.__buildDEB()
        self.__buildNSIS()

    def __buildDEB(self):
        debfn = "%s_%s_all.deb" % (self.shortname, self.version)
        InstallerMaker.notify.info("Creating %s..." % debfn)

        # Create a temporary directory and write the control file + launcher to it
        tempdir = Filename.temporary("", self.shortname + "_deb_", "") + "/"
        tempdir.makeDir()
        tempdir = tempdir.toOsSpecific()
        controlfile = open(os.path.join(tempdir, "control"), "w")
        controlfile.write("Package: %s\n" % self.shortname)
        controlfile.write("Version: %s\n" % self.version)
        controlfile.write("Section: games\n")
        controlfile.write("Priority: optional\n")
        controlfile.write("Architecture: all\n")
        controlfile.write("Depends: panda3d-runtime\n")
        controlfile.write("Description: %s\n" % self.fullname)
        controlfile.close()
        os.makedirs(os.path.join(tempdir, "usr", "bin"))
        os.makedirs(os.path.join(tempdir, "usr", "share", "games", self.shortname))
        if not self.licensefile.empty():
            os.makedirs(os.path.join(tempdir, "usr", "share", "doc", self.shortname))
        launcherfile = open(os.path.join(tempdir, "usr", "bin", self.shortname), "w")
        launcherfile.write("#!/bin/sh\n")
        launcherfile.write("/usr/bin/env panda3d /usr/share/games/%s/data.p3d\n" % self.shortname)
        launcherfile.close()
        os.chmod(os.path.join(tempdir, "usr", "bin", self.shortname), 0755)
        shutil.copyfile(self.p3dfile.toOsSpecific(), os.path.join(tempdir, "usr", "share", "games", self.shortname, "data.p3d"))
        if not self.licensefile.empty():
            shutil.copyfile(self.licensefile.toOsSpecific(), os.path.join(tempdir, "usr", "share", "doc", self.shortname, "copyright"))

        # Create a control.tar.gz file in memory
        controltargz = CachedFile()
        controltarfile = tarfile.TarFile.gzopen("control.tar.gz", "w", controltargz, 9)
        controltarfile.add(os.path.join(tempdir, "control"), "control")
        controltarfile.close()
        os.remove(os.path.join(tempdir, "control"))

        # Create the data.tar.gz file in the temporary directory
        datatargz = CachedFile()
        datatarfile = tarfile.TarFile.gzopen("data.tar.gz", "w", datatargz, 9)
        datatarfile.add(tempdir + "/usr", "/usr")
        datatarfile.close()

        # Open the deb file and write to it. It's actually
        # just an AR file, which is very easy to make.
        modtime = int(time.time())
        if os.path.isfile(debfn):
            os.remove(debfn)
        debfile = open(debfn, "wb")
        debfile.write("!<arch>\x0A")
        debfile.write("debian-binary   %-12lu0     0     100644  %-10ld\x60\x0A" % (modtime, 4))
        debfile.write("2.0\x0A")
        debfile.write("control.tar.gz  %-12lu0     0     100644  %-10ld\x60\x0A" % (modtime, len(controltargz.str)))
        debfile.write(controltargz.str)
        if (len(controltargz.str) & 1): debfile.write("\x0A")
        debfile.write("data.tar.gz     %-12lu0     0     100644  %-10ld\x60\x0A" % (modtime, len(datatargz.str)))
        debfile.write(datatargz.str)
        if (len(datatargz.str) & 1): debfile.write("\x0A")
        debfile.close()
        shutil.rmtree(tempdir)

    def __buildNSIS(self):
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

        tempfile = self.shortname + ".nsi"
        nsi = open(tempfile, "w")

        # Some global info
        nsi.write('Name "%s"\n' % self.fullname)
        nsi.write('OutFile "%s.exe"\n' % self.shortname)
        nsi.write('InstallDir "$PROGRAMFILES\%s"\n' % self.fullname)
        nsi.write('SetCompress auto\n')
        nsi.write('SetCompressor lzma\n')
        nsi.write('ShowInstDetails nevershow\n')
        nsi.write('ShowUninstDetails nevershow\n')
        nsi.write('InstType "Typical"\n')

        # Tell Vista that we require admin rights
        nsi.write('RequestExecutionLevel admin\n')
        nsi.write('\n')
        nsi.write('Function launch\n')
        nsi.write('  ExecShell "open" "$INSTDIR\%s.bat"\n' % self.shortname)
        nsi.write('FunctionEnd\n')
        nsi.write('\n')
        nsi.write('!include "MUI2.nsh"\n')
        nsi.write('!define MUI_ABORTWARNING\n')
        nsi.write('!define MUI_FINISHPAGE_RUN\n')
        nsi.write('!define MUI_FINISHPAGE_RUN_FUNCTION launch\n')
        nsi.write('!define MUI_FINISHPAGE_RUN_TEXT "Play %s"\n' % self.fullname)
        nsi.write('\n')
        nsi.write('Var StartMenuFolder\n')
        nsi.write('!insertmacro MUI_PAGE_WELCOME\n')
        if not self.licensefile.empty():
            nsi.write('!insertmacro MUI_PAGE_LICENSE "%s"\n' % self.licensefile.toOsSpecific())
        nsi.write('!insertmacro MUI_PAGE_DIRECTORY\n')
        nsi.write('!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder\n')
        nsi.write('!insertmacro MUI_PAGE_INSTFILES\n')
        nsi.write('!insertmacro MUI_PAGE_FINISH\n')
        nsi.write('!insertmacro MUI_UNPAGE_WELCOME\n')
        nsi.write('!insertmacro MUI_UNPAGE_CONFIRM\n')
        nsi.write('!insertmacro MUI_UNPAGE_INSTFILES\n')
        nsi.write('!insertmacro MUI_UNPAGE_FINISH\n')
        nsi.write('!insertmacro MUI_LANGUAGE "English"\n')

        # This section defines the installer.
        nsi.write('Section "Install"\n')
        nsi.write('  SetOutPath "$INSTDIR"\n')
        nsi.write('  WriteUninstaller "$INSTDIR\Uninstall.exe"\n')
        nsi.write('  ; Start menu items\n')
        nsi.write('  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application\n')
        nsi.write('    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"\n')
        nsi.write('    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"\n')
        nsi.write('  !insertmacro MUI_STARTMENU_WRITE_END\n')
        nsi.write('SectionEnd\n')

        # This section defines the uninstaller.
        nsi.write('Section "Uninstall"\n')
        nsi.write('  Delete "$INSTDIR\Uninstall.exe"\n')
        nsi.write('  RMDir "$INSTDIR"\n')
        nsi.write('  ; Start menu items\n')
        nsi.write('  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder\n')
        nsi.write('  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"\n')
        nsi.write('  RMDir "$SMPROGRAMS\$StartMenuFolder"\n')
        nsi.write('SectionEnd')
        nsi.close()

        options = ["V2"]
        cmd = makensis
        for o in options:
            if sys.platform.startswith("win"):
                cmd += " /" + o
            else:
                cmd += " -" + o
        cmd += " " + tempfile
        os.system(cmd)

        os.remove(tempfile)

