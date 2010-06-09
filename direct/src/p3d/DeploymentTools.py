""" This module is used to build a graphical installer
or a standalone executable from a p3d file. It will try
to build for as many platforms as possible. """

__all__ = ["Standalone", "Installer"]

import os, sys, subprocess, tarfile, shutil, time, zipfile, glob
from direct.directnotify.DirectNotifyGlobal import *
from pandac.PandaModules import PandaSystem, HTTPClient, Filename, VirtualFileSystem, Multifile, readXmlStream
from direct.p3d.HostInfo import HostInfo

class CachedFile:
    def __init__(self): self.str = ""
    def write(self, data): self.str += data

# Make sure this matches with the magic in p3dEmbed.cxx.
P3DEMBED_MAGIC = "\xFF\x3D\x3D\x00"

class Standalone:
    """ This class creates a standalone executable from a given .p3d file. """
    notify = directNotify.newCategory("Standalone")
    
    def __init__(self, p3dfile, tokens = {}):
        self.p3dfile = Filename(p3dfile)
        self.basename = self.p3dfile.getBasenameWoExtension()
        self.tokens = tokens
        
        hostDir = Filename(Filename.getTempDirectory(), 'pdeploy/')
        hostDir.makeDir()
        self.host = HostInfo(PandaSystem.getPackageHostUrl(), appRunner = base.appRunner, hostDir = hostDir, asMirror = False, perPlatform = True)
        
        self.http = HTTPClient.getGlobalPtr()
        if not self.host.hasContentsFile:
            if not self.host.readContentsFile():
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
            Standalone.notify.warning("No platforms found to build for!")
        
        outputDir = Filename(outputDir + "/")
        outputDir.makeDir()
        for platform in platforms:
            if platform.startswith("win"):
                self.build(Filename(outputDir, platform + "/" + self.basename + ".exe"), platform)
            else:
                self.build(Filename(outputDir, platform + "/" + self.basename), platform)
    
    def build(self, output, platform = None):
        """ Builds a standalone executable and stores it into the path
        indicated by the 'output' argument. You can specify to build for
        a different platform by altering the 'platform' argument. """
        
        if platform == None:
            platform = PandaSystem.getPlatform()
        for package in self.host.getPackages(name = "p3dembed", platform = platform):
            if not package.downloadDescFile(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            if not package.downloadPackage(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            
            # Figure out where p3dembed might be now.
            if package.platform.startswith("win"):
                p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembed.exe" % package.platform)
            else:
                p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembed" % package.platform)
            
            if not p3dembed.exists():
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            
            return self.embed(output, p3dembed)
        
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

    def getExtraFiles(self, platform):
        """ Returns a list of extra files that will need to be included
        with the standalone executable in order for it to run, such as
        dependent libraries. The returned paths are full absolute paths. """
        
        package = self.host.getPackages(name = "p3dembed", platform = platform)[0]
        
        if not package.downloadDescFile(self.http):
            Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
            return []
        if not package.downloadPackage(self.http):
            Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
            return []
        
        filenames = []
        for e in package.extracts:
            if e.basename not in ["p3dembed", "p3dembed.exe"]:
                filename = Filename(package.getPackageDir(), e.filename)
                filename.makeAbsolute()
                if filename.exists():
                    filenames.append(filename)
                else:
                    Standalone.notify.error("%s mentioned in xml, but does not exist" % e.filename)
        
        return filenames

class Installer:
    """ This class creates a (graphical) installer from a given .p3d file. """
    notify = directNotify.newCategory("Installer")

    def __init__(self, shortname, fullname, p3dfile, version, tokens = {}):
        self.shortname = shortname
        self.fullname = fullname
        self.version = str(version)
        self.includeRequires = False
        self.licensename = ""
        self.authorid = "org.panda3d"
        self.authorname = ""
        self.licensefile = Filename()
        self.standalone = Standalone(p3dfile, tokens)
        self.http = self.standalone.http
        
        # Load the p3d file to read out the required packages
        mf = Multifile()
        if not mf.openRead(p3dfile):
            Installer.notify.error("Not a Panda3D application: %s" % (p3dFilename))
            return

        # Now load the p3dInfo file.
        self.hostUrl = PandaSystem.getPackageHostUrl()
        if not self.hostUrl:
            self.hostUrl = self.standalone.host.hostUrl
        self.requirements = []
        i = mf.findSubfile('p3d_info.xml')
        if i >= 0:
            stream = mf.openReadSubfile(i)
            p3dInfo = readXmlStream(stream)
            mf.closeReadSubfile(stream)
            if p3dInfo:
                p3dPackage = p3dInfo.FirstChildElement('package')
                p3dHost = p3dPackage.FirstChildElement('host')
                if p3dHost.Attribute('url'):
                    self.hostUrl = p3dHost.Attribute('url')
                p3dRequires = p3dPackage.FirstChildElement('requires')
                while p3dRequires:
                    self.requirements.append((p3dRequires.Attribute('name'), p3dRequires.Attribute('version')))
                    p3dRequires = p3dRequires.NextSiblingElement('requires')

    def installPackagesInto(self, rootDir, platform):
        """ Installs the packages required by the .p3d file into
        the specified root directory, for the given platform. """
        
        if not self.includeRequires:
            return
        
        host = HostInfo(self.hostUrl, appRunner = base.appRunner, rootDir = rootDir, asMirror = True, perPlatform = False)
        if not host.hasContentsFile:
            if not host.readContentsFile():
                if not host.downloadContentsFile(self.http):
                    Installer.notify.error("couldn't read host")
                    return
        
        for name, version in self.requirements:
            package = host.getPackage(name, version, platform)
            if not package.downloadDescFile(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            if not package.downloadPackage(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
        
        # Also install the 'images' package from the same host that p3dembed was downloaded from.
        host = HostInfo(self.standalone.host.hostUrl, appRunner = base.appRunner, rootDir = rootDir, asMirror = False, perPlatform = False)
        if not host.hasContentsFile:
            if not host.readContentsFile():
                if not host.downloadContentsFile(self.http):
                    Installer.notify.error("couldn't read host")
                    return
        
        for package in host.getPackages(name = "images"):
            if not package.downloadDescFile(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            if not package.downloadPackage(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            break

    def buildAll(self, outputDir = "."):
        """ Creates a (graphical) installer for every known platform.
        Call this after you have set the desired parameters. """
        
        platforms = set()
        for package in self.standalone.host.getPackages(name = "p3dembed"):
            platforms.add(package.platform)
        if len(platforms) == 0:
            Installer.notify.warning("No platforms found to build for!")
        
        outputDir = Filename(outputDir + "/")
        outputDir.makeDir()
        for platform in platforms:
            output = Filename(outputDir, platform + "/")
            output.makeDir()
            self.build(output, platform)
    
    def build(self, output, platform = None):
        """ Builds a (graphical) installer and stores it into the path
        indicated by the 'output' argument. You can specify to build for
        a different platform by altering the 'platform' argument.
        If 'output' is a directory, the installer will be stored in it. """
        
        if platform == None:
            platform = PandaSystem.getPlatform()
        
        if platform == "win32":
            return self.buildNSIS(output, platform)
        elif "_" in platform:
            os, arch = platform.split("_", 1)
            if os == "linux":
                return self.buildDEB(output, platform)
            elif os == "osx":
                return self.buildPKG(output, platform)
            elif os == "freebsd":
                return self.buildDEB(output, platform)
        Installer.notify.info("Ignoring unknown platform " + platform)

    def buildDEB(self, output, platform):
        """ Builds a .deb archive and stores it in the path indicated
        by the 'output' argument. It will be built for the architecture
        specified by the 'arch' argument.
        If 'output' is a directory, the deb file will be stored in it. """
        
        arch = platform.rsplit("_", 1)[-1]
        output = Filename(output)
        if output.isDirectory():
            output = Filename(output, "%s_%s_%s.deb" % (self.shortname.lower(), self.version, arch))
        Installer.notify.info("Creating %s..." % output)

        # Create a temporary directory and write the control file + launcher to it
        tempdir = Filename.temporary("", self.shortname.lower() + "_deb_", "") + "/"
        tempdir.makeDir()
        controlfile = open(Filename(tempdir, "control").toOsSpecific(), "w")
        print >>controlfile, "Package: %s" % self.shortname.lower()
        print >>controlfile, "Version: %s" % self.version
        print >>controlfile, "Section: games"
        print >>controlfile, "Priority: optional"
        print >>controlfile, "Architecture: %s" % arch
        print >>controlfile, "Description: %s" % self.fullname
        print >>controlfile, "Depends: libc6, libgcc1, libstdc++6, libx11-6, libssl0.9.8"
        controlfile.close()
        postinst = open(Filename(tempdir, "postinst").toOsSpecific(), "w")
        print >>postinst, "#!/bin/sh"
        print >>postinst, "/usr/bin/%s --prep" % self.shortname.lower()
        print >>postinst, "chmod -R 777 /usr/share/%s" % self.shortname.lower()
        print >>postinst, "chmod -R 555 /usr/share/%s/hosts" % self.shortname.lower()
        postinst.close()
        os.chmod(Filename(tempdir, "postinst").toOsSpecific(), 0755)
        postrmfile = open(Filename(tempdir, "postrm").toOsSpecific(), "w")
        print >>postrmfile, "#!/bin/sh"
        print >>postrmfile, "rm -rf /usr/share/%s" % self.shortname.lower()
        postrmfile.close()
        os.chmod(Filename(tempdir, "postrm").toOsSpecific(), 0755)
        Filename(tempdir, "usr/bin/").makeDir()
        self.standalone.tokens["root_dir"] = "/usr/share/" + self.shortname.lower()
        self.standalone.build(Filename(tempdir, "usr/bin/" + self.shortname.lower()), platform)
        if not self.licensefile.empty():
            Filename(tempdir, "usr/share/doc/%s/" % self.shortname.lower()).makeDir()
            shutil.copyfile(self.licensefile.toOsSpecific(), Filename(tempdir, "usr/share/doc/%s/copyright" % self.shortname.lower()).toOsSpecific())
        rootDir = Filename(tempdir, "usr/share/" + self.shortname.lower())
        rootDir.makeDir()
        Filename(rootDir, "log").makeDir()
        Filename(rootDir, "prc").makeDir()
        Filename(rootDir, "start").makeDir()
        Filename(rootDir, "certs").makeDir()
        self.installPackagesInto(rootDir, platform)

        # Create a control.tar.gz file in memory
        controlfile = Filename(tempdir, "control")
        postinstfile = Filename(tempdir, "postinst")
        postrmfile = Filename(tempdir, "postrm")
        controltargz = CachedFile()
        controltarfile = tarfile.TarFile.gzopen("control.tar.gz", "w", controltargz, 9)
        controltarfile.add(controlfile.toOsSpecific(), "control")
        controltarfile.add(postinstfile.toOsSpecific(), "postinst")
        controltarfile.add(postrmfile.toOsSpecific(), "postrm")
        controltarfile.close()
        controlfile.unlink()
        postinstfile.unlink()
        postrmfile.unlink()

        # Create the data.tar.gz file in the temporary directory
        datatargz = CachedFile()
        datatarfile = tarfile.TarFile.gzopen("data.tar.gz", "w", datatargz, 9)
        datatarfile.add(Filename(tempdir, "usr").toOsSpecific(), "/usr")
        datatarfile.close()

        # Open the deb file and write to it. It's actually
        # just an AR file, which is very easy to make.
        modtime = int(time.time())
        if output.exists():
            output.unlink()
        debfile = open(output.toOsSpecific(), "wb")
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
        try:
            base.appRunner.rmtree(tempdir)
        except:
            try: shutil.rmtree(tempdir.toOsSpecific())
            except: pass

    def buildAPP(self, output, platform):
        
        output = Filename(output)
        if output.isDirectory() and output.getExtension() != 'app':
            output = Filename(output, "%s.app" % self.fullname)
        Installer.notify.info("Creating %s..." % output)
        
        # Create the executable for the application bundle
        exefile = Filename(output, "Contents/MacOS/" + self.shortname)
        exefile.makeDir()
        self.standalone.tokens["root_dir"] = "../Resources"
        self.standalone.build(exefile, platform)
        rootDir = Filename(output, "Contents/Resources/")
        rootDir.makeDir()
        self.installPackagesInto(rootDir, platform)
        
        # Create the application plist file.
        # Although it might make more sense to use Python's plistlib module here,
        # it is not available on non-OSX systems before Python 2.6.
        plist = open(Filename(output, "Contents/Info.plist").toOsSpecific(), "w")
        print >>plist, '<?xml version="1.0" encoding="UTF-8"?>'
        print >>plist, '<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">'
        print >>plist, '<plist version="1.0">'
        print >>plist, '<dict>'
        print >>plist, '\t<key>CFBundleDevelopmentRegion</key>'
        print >>plist, '\t<string>English</string>'
        print >>plist, '\t<key>CFBundleDisplayName</key>'
        print >>plist, '\t<string>%s</string>' % self.fullname
        print >>plist, '\t<key>CFBundleExecutable</key>'
        print >>plist, '\t<string>%s</string>' % exefile.getBasename()
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
        
        Filename(output, "hosts/").makeDir()
        
        return output

    def buildPKG(self, output, platform):
        appfn = self.buildAPP(output, platform)
        appname = "/Applications/" + appfn.getBasename()
        output = Filename(output)
        if output.isDirectory():
            output = Filename(output, "%s %s.pkg" % (self.fullname, self.version))
        Installer.notify.info("Creating %s..." % output)
        
        Filename(output, "Contents/Resources/en.lproj/").makeDir()
        if self.licensefile:
            shutil.copyfile(self.licensefile.toOsSpecific(), Filename(output, "Contents/Resources/License.txt").toOsSpecific())
        pkginfo = open(Filename(output, "Contents/PkgInfo").toOsSpecific(), "w")
        pkginfo.write("pkmkrpkg1")
        pkginfo.close()
        pkginfo = open(Filename(output, "Contents/Resources/package_version").toOsSpecific(), "w")
        pkginfo.write("major: 1\nminor: 9")
        pkginfo.close()
        
        # Although it might make more sense to use Python's plistlib here,
        # it is not available on non-OSX systems before Python 2.6.
        plist = open(Filename(output, "Contents/Info.plist").toOsSpecific(), "w")
        plist.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        plist.write('<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
        plist.write('<plist version="1.0">\n')
        plist.write('<dict>\n')
        plist.write('\t<key>CFBundleIdentifier</key>\n')
        plist.write('\t<string>%s.pkg.%s</string>\n' % (self.authorid, self.shortname))
        plist.write('\t<key>CFBundleShortVersionString</key>\n')
        plist.write('\t<string>%s</string>\n' % self.version)
        plist.write('\t<key>IFMajorVersion</key>\n')
        plist.write('\t<integer>1</integer>\n')
        plist.write('\t<key>IFMinorVersion</key>\n')
        plist.write('\t<integer>9</integer>\n')
        plist.write('\t<key>IFPkgFlagAllowBackRev</key>\n')
        plist.write('\t<false/>\n')
        plist.write('\t<key>IFPkgFlagAuthorizationAction</key>\n')
        plist.write('\t<string>RootAuthorization</string>\n')
        plist.write('\t<key>IFPkgFlagDefaultLocation</key>\n')
        plist.write('\t<string>/</string>\n')
        plist.write('\t<key>IFPkgFlagFollowLinks</key>\n')
        plist.write('\t<true/>\n')
        plist.write('\t<key>IFPkgFlagIsRequired</key>\n')
        plist.write('\t<false/>\n')
        plist.write('\t<key>IFPkgFlagOverwritePermissions</key>\n')
        plist.write('\t<false/>\n')
        plist.write('\t<key>IFPkgFlagRelocatable</key>\n')
        plist.write('\t<false/>\n')
        plist.write('\t<key>IFPkgFlagRestartAction</key>\n')
        plist.write('\t<string>None</string>\n')
        plist.write('\t<key>IFPkgFlagRootVolumeOnly</key>\n')
        plist.write('\t<true/>\n')
        plist.write('\t<key>IFPkgFlagUpdateInstalledLanguages</key>\n')
        plist.write('\t<false/>\n')
        plist.write('\t<key>IFPkgFormatVersion</key>\n')
        plist.write('\t<real>0.10000000149011612</real>\n')
        plist.write('\t<key>IFPkgPathMappings</key>\n')
        plist.write('\t<dict>\n')
        plist.write('\t\t<key>%s</key>\n' % appname)
        plist.write('\t\t<string>{pkmk-token-2}</string>\n')
        plist.write('\t</dict>\n')
        plist.write('</dict>\n')
        plist.write('</plist>\n')
        plist.close()
        
        plist = open(Filename(output, "Contents/Resources/TokenDefinitions.plist").toOsSpecific(), "w")
        plist.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        plist.write('<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
        plist.write('<plist version="1.0">\n')
        plist.write('<dict>\n')
        plist.write('\t<key>pkmk-token-2</key>\n')
        plist.write('\t<array>\n')
        plist.write('\t\t<dict>\n')
        plist.write('\t\t\t<key>identifier</key>\n')
        plist.write('\t\t\t<string>%s.%s</string>\n' % (self.authorid, self.shortname))
        plist.write('\t\t\t<key>path</key>\n')
        plist.write('\t\t\t<string>%s</string>\n' % appname)
        plist.write('\t\t\t<key>searchPlugin</key>\n')
        plist.write('\t\t\t<string>CommonAppSearch</string>\n')
        plist.write('\t\t</dict>\n')
        plist.write('\t</array>\n')
        plist.write('</dict>\n')
        plist.write('</plist>\n')
        plist.close()
        
        plist = open(Filename(output, "Contents/Resources/en.lproj/Description.plist").toOsSpecific(), "w")
        plist.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        plist.write('<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n')
        plist.write('<plist version="1.0">\n')
        plist.write('<dict>\n')
        plist.write('\t<key>IFPkgDescriptionDescription</key>\n')
        plist.write('\t<string></string>\n')
        plist.write('\t<key>IFPkgDescriptionTitle</key>\n')
        plist.write('\t<string>%s</string>\n' % self.fullname)
        plist.write('</dict>\n')
        plist.write('</plist>\n')
        plist.close()
        
        postflight = open(Filename(output, "Contents/Resources/postflight").toOsSpecific(), "w")
        print >>postflight, '#!/bin/sh'
        print >>postflight, 'chmod -R 777 "%s"' % appname
        print >>postflight, 'chmod -R 755 "%s/hosts/"' % appname
        postflight.close()
        os.chmod(Filename(output, "Contents/Resources/postflight").toOsSpecific(), 0755)
        
        if hasattr(tarfile, "PAX_FORMAT"):
            archive = tarfile.open(Filename(output, "Contents/Archive.pax.gz").toOsSpecific(), "w:gz", format = tarfile.PAX_FORMAT)
        else:
            archive = tarfile.open(Filename(output, "Contents/Archive.pax.gz").toOsSpecific(), "w:gz")
        archive.add(appfn.toOsSpecific(), appname)
        archive.close()
        
        # Put the .pkg into a zipfile
        archive = Filename(output.getDirname(), "%s %s.zip" % (self.fullname, self.version))
        dir = Filename(output.getDirname())
        dir.makeAbsolute()
        zip = zipfile.ZipFile(archive.toOsSpecific(), 'w')
        for root, dirs, files in os.walk(output.toOsSpecific()):
            for name in files:
                file = Filename.fromOsSpecific(os.path.join(root, name))
                file.makeAbsolute()
                file.makeRelativeTo(dir)
                zip.write(os.path.join(root, name), str(file))
        zip.close()
        
        return output

    def buildNSIS(self, output, platform):
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
        
        output = Filename(output)
        if output.isDirectory():
            output = Filename(output, "%s %s.exe" % (self.fullname, self.version))
        Installer.notify.info("Creating %s..." % output)
        output.makeAbsolute()
        extrafiles = self.standalone.getExtraFiles(platform)

        exefile = Filename(Filename.getTempDirectory(), self.shortname + ".exe")
        exefile.unlink()
        self.standalone.tokens["root_dir"] = "."
        self.standalone.build(exefile, platform)
        
        # Temporary directory to store the rootdir in
        rootDir = Filename.temporary("", self.shortname.lower() + "_exe_", "") + "/"
        rootDir.makeDir()
        self.installPackagesInto(rootDir, platform)

        nsifile = Filename(Filename.getTempDirectory(), self.shortname + ".nsi")
        nsifile.unlink()
        nsi = open(nsifile.toOsSpecific(), "w")

        # Some global info
        nsi.write('Name "%s"\n' % self.fullname)
        nsi.write('OutFile "%s"\n' % output.toOsSpecific())
        nsi.write('InstallDir "$PROGRAMFILES\\%s"\n' % self.fullname)
        nsi.write('SetCompress auto\n')
        nsi.write('SetCompressor lzma\n')
        nsi.write('ShowInstDetails nevershow\n')
        nsi.write('ShowUninstDetails nevershow\n')
        nsi.write('InstType "Typical"\n')

        # Tell Vista that we require admin rights
        nsi.write('RequestExecutionLevel admin\n')
        nsi.write('\n')
        nsi.write('Function launch\n')
        nsi.write('  ExecShell "open" "$INSTDIR\\%s.exe"\n' % self.shortname)
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
        nsi.write('Section "" SecCore\n')
        nsi.write('  SetOutPath "$INSTDIR"\n')
        nsi.write('  File "%s"\n' % exefile.toOsSpecific())
        for f in extrafiles:
            nsi.write('  File "%s"\n' % f.toOsSpecific())
        curdir = ""
        for root, dirs, files in os.walk(rootDir.toOsSpecific()):
            for name in files:
                file = Filename.fromOsSpecific(os.path.join(root, name))
                file.makeAbsolute()
                file.makeRelativeTo(rootDir)
                outdir = file.getDirname().replace('/', '\\')
                if curdir != outdir:
                    nsi.write('  SetOutPath "$INSTDIR\\%s"\n' % outdir)
                    curdir = outdir
                nsi.write('  File "%s"\n' % os.path.join(root, name))
        nsi.write('  WriteUninstaller "$INSTDIR\\Uninstall.exe"\n')
        nsi.write('  ; Start menu items\n')
        nsi.write('  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application\n')
        nsi.write('    CreateDirectory "$SMPROGRAMS\\$StartMenuFolder"\n')
        nsi.write('    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk" "$INSTDIR\\Uninstall.exe"\n')
        nsi.write('  !insertmacro MUI_STARTMENU_WRITE_END\n')
        nsi.write('SectionEnd\n')

        # This section defines the uninstaller.
        nsi.write('Section Uninstall\n')
        nsi.write('  Delete "$INSTDIR\\%s.exe"\n' % self.shortname)
        for f in extrafiles:
            nsi.write('  Delete "%s"\n' % f.getBasename())
        nsi.write('  Delete "$INSTDIR\\Uninstall.exe"\n')
        nsi.write('  RMDir /r "$INSTDIR"\n')
        nsi.write('  ; Start menu items\n')
        nsi.write('  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder\n')
        nsi.write('  Delete "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk"\n')
        nsi.write('  RMDir "$SMPROGRAMS\\$StartMenuFolder"\n')
        nsi.write('SectionEnd')
        nsi.close()

        options = ["V2"]
        cmd = "\"" + makensis + "\""
        for o in options:
            if sys.platform.startswith("win"):
                cmd += " /" + o
            else:
                cmd += " -" + o
        cmd += " " + nsifile.toOsSpecific()
        print cmd
        os.system(cmd)

        nsifile.unlink()
        try:
            base.appRunner.rmtree(rootDir)
        except:
            try: shutil.rmtree(rootDir.toOsSpecific())
            except: pass
        return output
