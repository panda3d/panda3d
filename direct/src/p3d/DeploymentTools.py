""" This module is used to build a graphical installer
or a standalone executable from a p3d file. It will try
to build for as many platforms as possible. """

__all__ = ["Standalone", "Installer"]

import os, sys, subprocess, tarfile, shutil, time
from direct.directnotify.DirectNotifyGlobal import *
from pandac.PandaModules import PandaSystem, HTTPClient, Filename, VirtualFileSystem
from direct.p3d.HostInfo import HostInfo
from direct.showbase.AppRunnerGlobal import appRunner

class CachedFile:
    def __init__(self): self.str = ""
    def write(self, data): self.str += data

# Make sure this matches with the magic in p3dEmbed.cxx.
P3DEMBED_MAGIC = "\xFF\x3D\x3D\x00"

class Standalone:
    """ This class creates a standalone executable from a given .p3d file. """
    notify = directNotify.newCategory("Standalone")
    
    def __init__(self, p3dfile, tokens = {}):
        if isinstance(p3dfile, Filename):
            self.p3dfile = p3dfile
        else:
            self.p3dfile = Filename(p3dfile)
        self.basename = self.p3dfile.getBasenameWoExtension()
        self.tokens = tokens
        
        if appRunner:
            self.host = appRunner.getHost("http://runtime.panda3d.org")
        else:
            hostDir = Filename(Filename.getTempDirectory(), 'pdeploy/')
            hostDir.makeDir()
            self.host = HostInfo("http://runtime.panda3d.org", hostDir = hostDir, asMirror = True)
        
        self.http = HTTPClient.getGlobalPtr()
        if not self.host.downloadContentsFile(self.http):
            Standalone.notify.error("couldn't read host")
            return False
    
    def buildAll(self, outputDir = "."):
        """ Builds standalone executables for every known platform,
        into the specified output directory. """
        
        platforms = set()
        for package in self.host.getPackages(name = "p3dembed"):
            platforms.add(package.platform)
        if len(platforms) == 0:
            Standalone.notify.error("No platforms found to build for!")
        
        for platform in platforms:
            if platform.startswith("win"):
                self.build(Filename(outputDir, platform + "/" + self.basename + ".exe"), platform)
            else:
                self.build(Filename(outputDir, platform + "/" + self.basename), platform)
    
    def build(self, output, platform = None):
        """ Builds a standalone executable and stores it into the path
        indicated by the 'output' argument. You can specify to build for
        other platforms by altering the 'platform' argument. """
        
        if platform == None:
            platform = PandaSystem.getPlatform()
        for package in self.host.getPackages(name = "p3dembed", platform = platform):
            if not package.downloadDescFile(self.http):
                Standalone.notify.error("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            if not package.downloadPackage(self.http):
                Standalone.notify.error("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            
            # Figure out where p3dembed might be now.
            if package.platform.startswith("win"):
                p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembed.exe" % package.platform)
            else:
                p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembed" % package.platform)
            
            # We allow p3dembed to be pzipped.
            if Filename(p3dembed + ".pz").exists():
                p3dembed = Filename(p3dembed + ".pz")
            if not p3dembed.exists():
                Standalone.notify.error("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            
            self.embed(output, p3dembed)
            return
        
        Standalone.notify.error("Failed to build standalone for platform %s" % platform)
    
    def embed(self, output, p3dembed):
        """ Embeds the p3d file into the provided p3dembed executable.
        This function is not really useful - use build() or buildAll() instead. """
        
        # Load the p3dembed data into memory
        size = p3dembed.getFileSize()
        p3dembed_data = VirtualFileSystem.getGlobalPtr().readFile(p3dembed, True)
        assert len(p3dembed_data) == size
        
        # Find the magic size string and replace it with the real size,
        # regardless of the endianness of the p3dembed executable.
        hex_size = hex(size)[2:].rjust(8, "0")
        enc_size = "".join([chr(int(hex_size[i] + hex_size[i + 1], 16)) for i in range(0, len(hex_size), 2)])
        p3dembed_data = p3dembed_data.replace(P3DEMBED_MAGIC, enc_size)
        p3dembed_data = p3dembed_data.replace(P3DEMBED_MAGIC[::-1], enc_size[::-1])
        
        # Write the output file
        Standalone.notify.info("Creating %s..." % output)
        output.makeDir()
        ohandle = open(output.toOsSpecific(), "wb")
        ohandle.write(p3dembed_data)
        for token in self.tokens.items():
            ohandle.write("\0%s=%s" % token)
        ohandle.write("\0\0")
        
        # Buffer the p3d file to the output file. 1 MB buffer size.
        phandle = open(self.p3dfile.toOsSpecific(), "rb")
        buf = phandle.read(1024 * 1024)
        while len(buf) != 0:
            ohandle.write(buf)
            buf = phandle.read(1024 * 1024)
        ohandle.close()
        phandle.close()
        
        os.chmod(output.toOsSpecific(), 0755)

class Installer:
    """ This class creates a (graphical) installer from a given .p3d file. """
    notify = directNotify.newCategory("Installer")

    def __init__(self, shortname, fullname, p3dfile, version):
        self.shortname = shortname
        self.fullname = fullname
        self.version = str(version)
        self.licensename = ""
        self.authorid = "org.panda3d"
        self.authorname = ""
        self.licensefile = Filename()
        self.builder = StandaloneBuilder(p3dfile)

    def buildAll(self):
        """ Creates a (graphical) installer for every known platform.
        Call this after you have set the desired parameters. """
        
        # Download the 'p3dembed' package
        tempdir = Filename.temporary("", self.shortname + "_p3d_", "") + "/"
        tempdir.makeDir()
        host = HostInfo("http://runtime.panda3d.org", hostDir = tempdir, asMirror = True)
        tempdir = tempdir.toOsSpecific()
        http = HTTPClient.getGlobalPtr()
        if not host.downloadContentsFile(http):
            Installer.notify.error("couldn't read host")
            return False

        for package in host.getPackages(name = "p3dembed"):
            print package.packageName, package.packageVersion, package.platform
            if not package.downloadDescFile(http):
                Installer.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            if not package.downloadPackage(http):
                Installer.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            
            if package.platform.startswith("linux_"):
                plugin_standalone = os.path.join(tempdir, "plugin_standalone", package.platform, "panda3d")
                assert os.path.isfile(plugin_standalone)
                self.__buildDEB(plugin_standalone, arch = package.platform.replace("linux_", ""))
            elif package.platform.startswith("win"):
                plugin_standalone = os.path.join(tempdir, "plugin_standalone", package.platform, "panda3d.exe")
                assert os.path.isfile(plugin_standalone)
                self.__buildNSIS(plugin_standalone)
            elif package.platform == "darwin":
                plugin_standalone = os.path.join(tempdir, "plugin_standalone", package.platform, "panda3d_mac")
                assert os.path.isfile(plugin_standalone)
                self.__buildAPP(plugin_standalone)
            else:
                Installer.notify.info("Ignoring unknown platform " + package.platform)
        
        #shutil.rmtree(tempdir)
        return True

    def __buildDEB(self, plugin_standalone, arch = "all"):
        debfn = "%s_%s_all.deb" % (self.shortname.lower(), self.version)
        Installer.notify.info("Creating %s..." % debfn)

        # Create a temporary directory and write the control file + launcher to it
        tempdir = Filename.temporary("", self.shortname.lower() + "_deb_", "") + "/"
        tempdir.makeDir()
        tempdir = tempdir.toOsSpecific()
        controlfile = open(os.path.join(tempdir, "control"), "w")
        controlfile.write("Package: %s\n" % self.shortname.lower())
        controlfile.write("Version: %s\n" % self.version)
        controlfile.write("Section: games\n")
        controlfile.write("Priority: optional\n")
        controlfile.write("Architecture: %s\n" % arch)
        controlfile.write("Description: %s\n" % self.fullname)
        controlfile.close()
        os.makedirs(os.path.join(tempdir, "usr", "bin"))
        os.makedirs(os.path.join(tempdir, "usr", "share", "games", self.shortname.lower()))
        os.makedirs(os.path.join(tempdir, "usr", "libexec", self.shortname.lower()))
        if not self.licensefile.empty():
            os.makedirs(os.path.join(tempdir, "usr", "share", "doc", self.shortname.lower()))
        launcherfile = open(os.path.join(tempdir, "usr", "bin", self.shortname.lower()), "w")
        launcherfile.write("#!/bin/sh\n")
        launcherfile.write("/usr/libexec/%s/panda3d /usr/share/games/%s/%s.p3d\n" % ((self.shortname.lower(),) * 3))
        launcherfile.close()
        os.chmod(os.path.join(tempdir, "usr", "bin", self.shortname.lower()), 0755)
        shutil.copyfile(self.p3dfile.toOsSpecific(), os.path.join(tempdir, "usr", "share", "games", self.shortname.lower(), self.shortname.lower() + ".p3d"))
        shutil.copyfile(plugin_standalone, os.path.join(tempdir, "usr", "libexec", self.shortname.lower(), "panda3d"))
        if not self.licensefile.empty():
            shutil.copyfile(self.licensefile.toOsSpecific(), os.path.join(tempdir, "usr", "share", "doc", self.shortname.lower(), "copyright"))

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

    def __buildAPP(self, plugin_standalone):
        pkgfn = "%s %s.pkg" % (self.shortname, self.version)
        appname = "/Applications/%s.app" % self.longname
        Installer.notify.info("Creating %s..." % pkgfn)
        
        # Create a temporary directory to hold the application in
        tempdir = Filename.temporary("", self.shortname.lower() + "_app_", "") + "/"
        tempdir = tempdir.toOsSpecific()
        if os.path.exists(tempdir):
            shutil.rmtree(tempdir)
        os.makedirs(tempdir)
        contents = os.path.join(tempdir, appname.lstrip("/"), "Contents")
        os.makedirs(os.path.join(contents, "MacOS"))
        os.makedirs(os.path.join(contents, "Resources"))
        
        # Create the "launch" script used to run the game.
        launch = open(os.path.join(contents, "MacOS", "launch"), "w")
        print >>launch, '#!/bin/sh'
        print >>launch, 'panda3d_mac ../Resources/%s' % self.p3dfile.getBasename()
        launch.close()
        shutil.copyfile(self.p3dfile.toOsSpecific(), os.path.join(contents, "Resources", self.p3dfile.getBasename()))
        shutil.copyfile(target, os.path.join(contents, "MacOS", "panda3d_mac"))
        
        # Create the application plist file.
        # Although it might make more sense to use Python's plistlib module here,
        # it is not available on non-OSX systems before Python 2.6.
        plist = open(os.path.join(tempdir, appname.lstrip("/"), "Contents", "Info.plist"), "w")
        print >>plist, '<?xml version="1.0" encoding="UTF-8"?>'
        print >>plist, '<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">'
        print >>plist, '<plist version="1.0">'
        print >>plist, '<dict>'
        print >>plist, '\t<key>CFBundleDevelopmentRegion</key>'
        print >>plist, '\t<string>English</string>'
        print >>plist, '\t<key>CFBundleDisplayName</key>'
        print >>plist, '\t<string>%s</string>' % self.fullname
        print >>plist, '\t<key>CFBundleExecutable</key>'
        print >>plist, '\t<string>launch</string>'
        print >>plist, '\t<key>CFBundleIdentifier</key>'
        print >>plist, '\t<string>%s.%s</string>' % (self.authorid, self.shortname)
        print >>plist, '\t<key>CFBundleInfoDictionaryVersion</key>'
        print >>plist, '\t<string>6.0</string>'
        print >>plist, '\t<key>CFBundleName</key>'
        print >>plist, '\t<string>%s</string>' % self.shortname
        print >>plist, '\t<key>CFBundlePackageType</key>'
        print >>plist, '\t<string>APPL</string>'
        print >>plist, '\t<key>CFBundleShortVersionString</key>'
        print >>plist, '\t<string>%s</string>' % self.version
        print >>plist, '\t<key>CFBundleVersion</key>'
        print >>plist, '\t<string>%s</string>' % self.version
        print >>plist, '\t<key>LSHasLocalizedDisplayName</key>'
        print >>plist, '\t<false/>'
        print >>plist, '\t<key>NSAppleScriptEnabled</key>'
        print >>plist, '\t<false/>'
        print >>plist, '\t<key>NSPrincipalClass</key>'
        print >>plist, '\t<string>NSApplication</string>'
        print >>plist, '</dict>'
        print >>plist, '</plist>'
        plist.close()

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
            Installer.notify.warning("Makensis utility not found, no Windows installer will be built!")
            return
        Installer.notify.info("Creating %s.exe..." % self.shortname)

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
