""" This module is used to build a graphical installer
or a standalone executable from a p3d file. It will try
to build for as many platforms as possible. """

__all__ = ["Standalone", "Installer"]

import os, sys, subprocess, tarfile, shutil, time, zipfile, socket, getpass, struct
from cStringIO import StringIO
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.AppRunnerGlobal import appRunner
from panda3d.core import PandaSystem, HTTPClient, Filename, VirtualFileSystem, Multifile
from panda3d.core import TiXmlDocument, TiXmlDeclaration, TiXmlElement, readXmlStream
from panda3d.core import PNMImage, PNMFileTypeRegistry
from direct.stdpy.file import *
from direct.p3d.HostInfo import HostInfo
# This is important for some reason
import encodings

try:
    import pwd
except ImportError:
    pwd = None

# Make sure this matches with the magic in p3dEmbed.cxx.
P3DEMBED_MAGIC = "\xFF\x3D\x3D\x00"

# This filter function is used when creating
# an archive that should be owned by root.
def archiveFilter(info):
    basename = os.path.basename(info.name)
    if basename in [".DS_Store", "Thumbs.db"]:
        return None

    info.uid = info.gid = 0
    info.uname = info.gname = "root"

    # All files without an extension get mode 0755,
    # all other files are chmodded to 0644.
    # Somewhat hacky, but it's the only way
    # permissions can work on a Windows box.
    if info.type != tarfile.DIRTYPE and '.' in info.name.rsplit('/', 1)[-1]:
        info.mode = 0o644
    else:
        info.mode = 0o755

    return info

# Hack to make all files in a tar file owned by root.
# The tarfile module doesn't have filter functionality until Python 2.7.
# Yay duck typing!
class TarInfoRoot(tarfile.TarInfo):
    uid = property(lambda self: 0, lambda self, x: None)
    gid = property(lambda self: 0, lambda self, x: None)
    uname = property(lambda self: "root", lambda self, x: None)
    gname = property(lambda self: "root", lambda self, x: None)
    mode = property(lambda self: 0o644 if self.type != tarfile.DIRTYPE and \
                    '.' in self.name.rsplit('/', 1)[-1] else 0o755,
                    lambda self, x: None)

# On OSX, the root group is named "wheel".
class TarInfoRootOSX(TarInfoRoot):
    gname = property(lambda self: "wheel", lambda self, x: None)


class Standalone:
    """ This class creates a standalone executable from a given .p3d file. """
    notify = directNotify.newCategory("Standalone")

    def __init__(self, p3dfile, tokens = {}):
        self.p3dfile = Filename(p3dfile)
        self.basename = self.p3dfile.getBasenameWoExtension()
        self.tokens = tokens

        self.tempDir = Filename.temporary("", self.basename, "") + "/"
        self.tempDir.makeDir()
        self.host = HostInfo(PandaSystem.getPackageHostUrl(), appRunner = appRunner, hostDir = self.tempDir, asMirror = False)

        self.http = HTTPClient.getGlobalPtr()
        if not self.host.hasContentsFile:
            if not self.host.readContentsFile():
                if not self.host.downloadContentsFile(self.http):
                    Standalone.notify.error("couldn't read host")
                    return

    def __del__(self):
        try:
            appRunner.rmtree(self.tempDir)
        except:
            try: shutil.rmtree(self.tempDir.toOsSpecific())
            except: pass

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

    def build(self, output, platform = None, extraTokens = {}):
        """ Builds a standalone executable and stores it into the path
        indicated by the 'output' argument. You can specify to build for
        a different platform by altering the 'platform' argument. """

        if platform == None:
            platform = PandaSystem.getPlatform()

        vfs = VirtualFileSystem.getGlobalPtr()

        for package in self.host.getPackages(name = "p3dembed", platform = platform):
            if not package.downloadDescFile(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue
            if not package.downloadPackage(self.http):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue

            # Figure out where p3dembed might be now.
            if package.platform.startswith("win"):
                # Use p3dembedw unless console_environment was set.
                cEnv = extraTokens.get("console_environment", self.tokens.get("console_environment", 0))
                if cEnv != "" and int(cEnv) != 0:
                    p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembed.exe" % package.platform)
                else:
                    p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembedw.exe" % package.platform)
                    # Fallback for older p3dembed versions
                    if not vfs.exists(p3dembed):
                        Filename(self.host.hostDir, "p3dembed/%s/p3dembed.exe" % package.platform)
            else:
                p3dembed = Filename(self.host.hostDir, "p3dembed/%s/p3dembed" % package.platform)

            if not vfs.exists(p3dembed):
                Standalone.notify.warning("  -> %s failed for platform %s" % (package.packageName, package.platform))
                continue

            return self.embed(output, p3dembed, extraTokens)

        Standalone.notify.error("Failed to build standalone for platform %s" % platform)

    def embed(self, output, p3dembed, extraTokens = {}):
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

        # Write out the tokens. Set log_basename to the basename by default
        tokens = {"log_basename" : self.basename}
        tokens.update(self.tokens)
        tokens.update(extraTokens)
        for token in tokens.items():
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

        os.chmod(output.toOsSpecific(), 0o755)

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
        vfs = VirtualFileSystem.getGlobalPtr()
        for e in package.extracts:
            if e.basename not in ["p3dembed", "p3dembed.exe", "p3dembed.exe.manifest", "p3dembedw.exe", "p3dembedw.exe.manifest"]:
                filename = Filename(package.getPackageDir(), e.filename)
                filename.makeAbsolute()
                if vfs.exists(filename):
                    filenames.append(filename)
                else:
                    Standalone.notify.error("%s mentioned in xml, but does not exist" % e.filename)

        return filenames


class PackageTree:
    """ A class used internally to build a temporary package
    tree for inclusion into an installer. """

    def __init__(self, platform, hostDir, hostUrl):
        self.platform = platform
        self.hosts = {}
        self.packages = {}
        self.hostUrl = hostUrl
        self.hostDir = Filename(hostDir)
        self.hostDir.makeDir()
        self.http = HTTPClient.getGlobalPtr()

    def getHost(self, hostUrl):
        if hostUrl in self.hosts:
            return self.hosts[hostUrl]

        host = HostInfo(hostUrl, appRunner = appRunner, hostDir = self.hostDir, asMirror = False)
        if not host.hasContentsFile:
            if not host.readContentsFile():
                if not host.downloadContentsFile(self.http):
                    Installer.notify.error("couldn't read host %s" % host.hostUrl)
                    return None
        self.hosts[hostUrl] = host
        return host

    def installPackage(self, name, version, hostUrl = None):
        """ Installs the named package into the tree. """

        if hostUrl is None:
            hostUrl = self.hostUrl

        pkgIdent = (name, version)
        if pkgIdent in self.packages:
            return self.packages[pkgIdent]

        package = None
        # Always try the super host first, if any.
        if appRunner and appRunner.superMirrorUrl:
            superHost = self.getHost(appRunner.superMirrorUrl)
            if self.platform:
                package = superHost.getPackage(name, version, self.platform)
            if not package:
                package = superHost.getPackage(name, version)

        if not package:
            host = self.getHost(hostUrl)
            if self.platform:
                package = host.getPackage(name, version, self.platform)
            if not package:
                package = host.getPackage(name, version)

        if not package:
            Installer.notify.error("Package %s %s for %s not known on %s" % (
                name, version, self.platform, hostUrl))
            return

        package.installed = True # Hack not to let it unnecessarily install itself
        if not package.downloadDescFile(self.http):
            Installer.notify.error("  -> %s failed for platform %s" % (package.packageName, package.platform))
            return
        if not package.downloadPackage(self.http):
            Installer.notify.error("  -> %s failed for platform %s" % (package.packageName, package.platform))
            return

        self.packages[pkgIdent] = package

        # Check for any dependencies.
        for rname, rversion, rhost in package.requires:
            self.installPackage(rname, rversion, rhost.hostUrl)

        return package


class Icon:
    """ This class is used to create an icon for various platforms. """
    notify = directNotify.newCategory("Icon")

    def __init__(self):
        self.images = {}

    def addImage(self, image):
        """ Adds an image to the icon.  Returns False on failure, True on success.
        Only one image per size can be loaded, and the image size must be square. """

        if not isinstance(image, PNMImage):
            fn = image
            if not isinstance(fn, Filename):
                fn = Filename.fromOsSpecific(fn)

            image = PNMImage()
            if not image.read(fn):
                Icon.notify.warning("Image '%s' could not be read" % fn.getBasename())
                return False

        if image.getXSize() != image.getYSize():
            Icon.notify.warning("Ignoring image without square size")
            return False

        self.images[image.getXSize()] = image

        return True

    def makeICO(self, fn):
        """ Writes the images to a Windows ICO file.  Returns True on success. """

        if not isinstance(fn, Filename):
            fn = Filename.fromOsSpecific(fn)
        fn.setBinary()

        count = 0
        for size in self.images.keys():
            if size <= 256:
                count += 1

        ico = open(fn, 'wb')
        ico.write(struct.pack('<HHH', 0, 1, count))

        # Write the directory
        for size, image in self.images.items():
            if size == 256:
                ico.write('\0\0')
            else:
                ico.write(struct.pack('<BB', size, size))
            bpp = 32 if image.hasAlpha() else 24
            ico.write(struct.pack('<BBHHII', 0, 0, 1, bpp, 0, 0))

        # Now write the actual icons
        ptr = 14
        for size, image in self.images.items():
            loc = ico.tell()
            bpp = 32 if image.hasAlpha() else 24
            ico.write(struct.pack('<IiiHHIIiiII', 40, size, size * 2, 1, bpp, 0, 0, 0, 0, 0, 0))

            # XOR mask
            if bpp == 24:
                # Align rows to 4-byte boundary
                rowalign = '\0' * (-(size * 3) & 3)
                for y in xrange(size):
                    for x in xrange(size):
                        r, g, b = image.getXel(x, size - y - 1)
                        ico.write(struct.pack('<BBB', int(b * 255), int(g * 255), int(r * 255)))
                    ico.write(rowalign)
            else:
                for y in xrange(size):
                    for x in xrange(size):
                        r, g, b, a = image.getXelA(x, size - y - 1)
                        ico.write(struct.pack('<BBBB', int(b * 255), int(g * 255), int(r * 255), int(a * 255)))

            # Empty AND mask, aligned to 4-byte boundary
            #TODO: perhaps we should convert alpha into an AND mask
            # to support older versions of Windows that don't support alpha.
            ico.write('\0' * (size * (size / 8 + (-((size / 8) * 3) & 3))))

            # Go back to write the location
            dataend = ico.tell()
            ico.seek(ptr)
            ico.write(struct.pack('<II', dataend - loc, loc))
            ico.seek(dataend)
            ptr += 16

        ico.close()

        return True

    def makeICNS(self, fn):
        """ Writes the images to an Apple ICNS file.  Returns True on success. """

        if not isinstance(fn, Filename):
            fn = Filename.fromOsSpecific(fn)
        fn.setBinary()

        vfs = VirtualFileSystem.getGlobalPtr()
        stream = vfs.openWriteFile(fn, False, True)
        icns = open(stream, 'wb')
        icns.write('icns\0\0\0\0')

        icon_types = {16: 'is32', 32: 'il32', 48: 'ih32', 128: 'it32'}
        mask_types = {16: 's8mk', 32: 'l8mk', 48: 'h8mk', 128: 't8mk'}
        png_types = {256: 'ic08', 512: 'ic09'}

        pngtype = PNMFileTypeRegistry.getGlobalPtr().getTypeFromExtension("png")

        for size, image in self.images.items():
            if size in png_types:
                if pngtype is None:
                    continue
                icns.write(png_types[size])
                icns.write('\0\0\0\0')
                start = icns.tell()

                image.write(stream, "", pngtype)
                pngsize = icns.tell() - start
                icns.seek(start - 4)
                icns.write(struct.pack('>I', pngsize + 8))
                icns.seek(start + pngsize)

            elif size in icon_types:
                icns.write(icon_types[size])
                icns.write(struct.pack('>I', size * size * 4 + 8))

                for y in xrange(size):
                    for x in xrange(size):
                        r, g, b = image.getXel(x, y)
                        icns.write(struct.pack('>BBBB', 0, int(r * 255), int(g * 255), int(b * 255)))

                if not image.hasAlpha():
                    continue
                icns.write(mask_types[size])
                icns.write(struct.pack('>I', size * size + 8))

                for y in xrange(size):
                    for x in xrange(size):
                        icns.write(struct.pack('<B', int(image.getAlpha(x, y) * 255)))

        length = icns.tell()
        icns.seek(4)
        icns.write(struct.pack('>I', length))
        icns.close()

        return True


class Installer:
    """ This class creates a (graphical) installer from a given .p3d file. """
    notify = directNotify.newCategory("Installer")

    def __init__(self, p3dfile, shortname, fullname, version, tokens = {}):
        if not shortname:
            shortname = p3dfile.getBasenameWoExtension()
        self.shortname = shortname
        self.fullname = fullname
        self.version = str(version)
        self.includeRequires = False
        self.offerRun = True
        self.offerDesktopShortcut = True
        self.licensename = ""
        self.licensefile = Filename()
        self.authorid = "org.panda3d"
        self.authorname = os.environ.get("DEBFULLNAME", "")
        self.authoremail = os.environ.get("DEBEMAIL", "")
        self.icon = None

        # Try to determine a default author name ourselves.
        uname = None
        if pwd is not None and hasattr(os, 'getuid'):
            uinfo = pwd.getpwuid(os.getuid())
            if uinfo:
                uname = uinfo.pw_name
                if not self.authorname:
                    self.authorname = \
                        uinfo.pw_gecos.split(',', 1)[0]

        # Fallbacks in case that didn't work or wasn't supported.
        if not uname:
            uname = getpass.getuser()
        if not self.authorname:
            self.authorname = uname
        if not self.authoremail and ' ' not in uname:
            self.authoremail = "%s@%s" % (uname, socket.gethostname())

        self.standalone = Standalone(p3dfile, tokens)
        self.tempDir = Filename.temporary("", self.shortname, "") + "/"
        self.tempDir.makeDir()
        self.__tempRoots = {}

        # Load the p3d file to read out the required packages
        mf = Multifile()
        if not mf.openRead(p3dfile):
            Installer.notify.error("Not a Panda3D application: %s" % (p3dfile))
            return

        # Now load the p3dInfo file.
        self.hostUrl = PandaSystem.getPackageHostUrl()
        if not self.hostUrl:
            self.hostUrl = self.standalone.host.hostUrl
        self.requires = []
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
                    self.requires.append((
                        p3dRequires.Attribute('name'),
                        p3dRequires.Attribute('version'),
                        p3dRequires.Attribute('host')))
                    p3dRequires = p3dRequires.NextSiblingElement('requires')

                if not self.fullname:
                    p3dConfig = p3dPackage.FirstChildElement('config')
                    if p3dConfig:
                        self.fullname = p3dConfig.Attribute('display_name')

        if not self.fullname:
            self.fullname = self.shortname

    def __del__(self):
        try:
            appRunner.rmtree(self.tempDir)
        except:
            try: shutil.rmtree(self.tempDir.toOsSpecific())
            except: pass

    def installPackagesInto(self, hostDir, platform):
        """ Installs the packages required by the .p3d file into
        the specified directory, for the given platform. """

        if not self.includeRequires:
            return

        pkgTree = PackageTree(platform, hostDir, self.hostUrl)
        pkgTree.installPackage("images", None, self.standalone.host.hostUrl)

        for name, version, hostUrl in self.requires:
            pkgTree.installPackage(name, version, hostUrl)

        # Remove the extracted files from the compressed archive, to save space.
        vfs = VirtualFileSystem.getGlobalPtr()
        for package in pkgTree.packages.values():
            if package.uncompressedArchive:
                archive = Filename(package.getPackageDir(), package.uncompressedArchive.filename)
                if not archive.exists():
                    continue

                mf = Multifile()
                # Make sure that it isn't mounted before altering it, just to be safe
                vfs.unmount(archive)
                os.chmod(archive.toOsSpecific(), 0o644)
                if not mf.openReadWrite(archive):
                    Installer.notify.warning("Failed to open archive %s" % (archive))
                    continue

                # We don't iterate over getNumSubfiles because we're
                # removing subfiles while we're iterating over them.
                subfiles = mf.getSubfileNames()
                for subfile in subfiles:
                    # We do *NOT* call vfs.exists here in case the package is mounted.
                    if Filename(package.getPackageDir(), subfile).exists():
                        Installer.notify.debug("Removing already-extracted %s from multifile" % (subfile))
                        mf.removeSubfile(subfile)

                # This seems essential for mf.close() not to crash later.
                mf.repack()

                # If we have no subfiles left, we can just remove the multifile.
                #XXX rdb: it seems that removing it causes trouble, so let's not.
                #if mf.getNumSubfiles() == 0:
                #    Installer.notify.info("Removing empty archive %s" % (package.uncompressedArchive.filename))
                #    mf.close()
                #    archive.unlink()
                #else:
                mf.close()
                try: os.chmod(archive.toOsSpecific(), 0o444)
                except: pass

        # Write out our own contents.xml file.
        doc = TiXmlDocument()
        decl = TiXmlDeclaration("1.0", "utf-8", "")
        doc.InsertEndChild(decl)

        xcontents = TiXmlElement("contents")
        for package in pkgTree.packages.values():
            xpackage = TiXmlElement('package')
            xpackage.SetAttribute('name', package.packageName)

            filename = package.packageName + "/"

            if package.packageVersion:
                xpackage.SetAttribute('version', package.packageVersion)
                filename += package.packageVersion + "/"

            if package.platform:
                xpackage.SetAttribute('platform', package.platform)
                filename += package.platform + "/"
                assert package.platform == platform

            xpackage.SetAttribute('per_platform', '1')

            filename += package.descFileBasename
            xpackage.SetAttribute('filename', filename)
            xcontents.InsertEndChild(xpackage)

        doc.InsertEndChild(xcontents)
        doc.SaveFile(Filename(hostDir, "contents.xml").toOsSpecific())

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
        """ Builds (graphical) installers and stores it into the path
        indicated by the 'output' argument. You can specify to build for
        a different platform by altering the 'platform' argument.
        If 'output' is a directory, the installer will be stored in it. """

        if platform == None:
            platform = PandaSystem.getPlatform()

        if platform.startswith("win"):
            self.buildNSIS(output, platform)
            return
        elif "_" in platform:
            osname, arch = platform.split("_", 1)
            if osname == "linux":
                self.buildDEB(output, platform)
                self.buildArch(output, platform)
                return
            elif osname == "osx":
                self.buildPKG(output, platform)
                return
        Installer.notify.info("Ignoring unknown platform " + platform)

    def __buildTempLinux(self, platform):
        """ Builds a filesystem for Linux.  Used so that buildDEB,
        buildRPM and buildArch can share the same temp directory. """

        if platform in self.__tempRoots:
            return self.__tempRoots[platform]

        tempdir = Filename(self.tempDir, platform)
        tempdir.makeDir()

        Filename(tempdir, "usr/bin/").makeDir()
        if self.includeRequires:
            extraTokens = {"host_dir" : "/usr/lib/" + self.shortname.lower()}
        else:
            extraTokens = {}
        self.standalone.build(Filename(tempdir, "usr/bin/" + self.shortname.lower()), platform, extraTokens)
        if not self.licensefile.empty():
            Filename(tempdir, "usr/share/doc/%s/" % self.shortname.lower()).makeDir()
            shutil.copyfile(self.licensefile.toOsSpecific(), Filename(tempdir, "usr/share/doc/%s/copyright" % self.shortname.lower()).toOsSpecific())
            shutil.copyfile(self.licensefile.toOsSpecific(), Filename(tempdir, "usr/share/doc/%s/LICENSE" % self.shortname.lower()).toOsSpecific())

        # Add an image file to /usr/share/pixmaps/
        iconFile = None
        if self.icon is not None:
            iconImage = None
            if 48 in self.icon.images:
                iconImage = self.icon.images[48]
            elif 64 in self.icon.images:
                iconImage = self.icon.images[64]
            elif 32 in self.icon.images:
                iconImage = self.icon.images[32]
            else:
                Installer.notify.warning("No suitable icon image for Linux provided, should preferably be 48x48 in size")

            if iconImage is not None:
                iconFile = Filename(tempdir, "usr/share/pixmaps/%s.png" % self.shortname)
                iconFile.setBinary()
                iconFile.makeDir()
                if not iconImage.write(iconFile):
                    Installer.notify.warning("Failed to write icon file for Linux")
                    iconFile.unlink()
                    iconFile = None

        # Write a .desktop file to /usr/share/applications/
        desktopFile = Filename(tempdir, "usr/share/applications/%s.desktop" % self.shortname.lower())
        desktopFile.setText()
        desktopFile.makeDir()
        desktop = open(desktopFile.toOsSpecific(), 'w')
        print >>desktop, "[Desktop Entry]"
        print >>desktop, "Name=%s" % self.fullname
        print >>desktop, "Exec=%s" % self.shortname.lower()
        if iconFile is not None:
            print >>desktop, "Icon=%s" % iconFile.getBasename()

        # Set the "Terminal" option based on whether or not a console env is requested
        cEnv = self.standalone.tokens.get("console_environment", "")
        if cEnv == "" or int(cEnv) == 0:
            print >>desktop, "Terminal=false"
        else:
            print >>desktop, "Terminal=true"

        print >>desktop, "Type=Application"
        desktop.close()

        if self.includeRequires:
            hostDir = Filename(tempdir, "usr/lib/%s/" % self.shortname.lower())
            hostDir.makeDir()
            self.installPackagesInto(hostDir, platform)

        totsize = 0
        for root, dirs, files in self.os_walk(tempdir.toOsSpecific()):
            for name in files:
                totsize += os.path.getsize(os.path.join(root, name))

        self.__tempRoots[platform] = (tempdir, totsize)
        return self.__tempRoots[platform]

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
        modtime = int(time.time())

        # Create a temporary directory and write the launcher and dependencies to it.
        tempdir, totsize = self.__buildTempLinux(platform)

        # Create a control file in memory.
        controlfile = StringIO()
        print >>controlfile, "Package: %s" % self.shortname.lower()
        print >>controlfile, "Version: %s" % self.version
        print >>controlfile, "Maintainer: %s <%s>" % (self.authorname, self.authoremail)
        print >>controlfile, "Section: games"
        print >>controlfile, "Priority: optional"
        print >>controlfile, "Architecture: %s" % arch
        print >>controlfile, "Installed-Size: %d" % -(-totsize / 1024)
        print >>controlfile, "Description: %s" % self.fullname
        print >>controlfile, "Depends: libc6, libgcc1, libstdc++6, libx11-6"
        controlinfo = TarInfoRoot("control")
        controlinfo.mtime = modtime
        controlinfo.size = controlfile.tell()
        controlfile.seek(0)

        # Open the deb file and write to it. It's actually
        # just an AR file, which is very easy to make.
        if output.exists():
            output.unlink()
        debfile = open(output.toOsSpecific(), "wb")
        debfile.write("!<arch>\x0A")
        debfile.write("debian-binary   %-12lu0     0     100644  %-10ld\x60\x0A" % (modtime, 4))
        debfile.write("2.0\x0A")

        # Write the control.tar.gz to the archive.
        debfile.write("control.tar.gz  %-12lu0     0     100644  %-10ld\x60\x0A" % (modtime, 0))
        ctaroffs = debfile.tell()
        ctarfile = tarfile.open("control.tar.gz", "w:gz", debfile, tarinfo = TarInfoRoot)
        ctarfile.addfile(controlinfo, controlfile)
        ctarfile.close()
        ctarsize = debfile.tell() - ctaroffs
        if (ctarsize & 1): debfile.write("\x0A")

        # Write the data.tar.gz to the archive.
        debfile.write("data.tar.gz     %-12lu0     0     100644  %-10ld\x60\x0A" % (modtime, 0))
        dtaroffs = debfile.tell()
        dtarfile = tarfile.open("data.tar.gz", "w:gz", debfile, tarinfo = TarInfoRoot)
        dtarfile.add(Filename(tempdir, "usr").toOsSpecific(), "/usr")
        dtarfile.close()
        dtarsize = debfile.tell() - dtaroffs
        if (dtarsize & 1): debfile.write("\x0A")

        # Write the correct sizes of the archives.
        debfile.seek(ctaroffs - 12)
        debfile.write("%-10ld" % ctarsize)
        debfile.seek(dtaroffs - 12)
        debfile.write("%-10ld" % dtarsize)

        debfile.close()

        return output

    def buildArch(self, output, platform):
        """ Builds an ArchLinux package and stores it in the path
        indicated by the 'output' argument. It will be built for the
        architecture specified by the 'arch' argument.
        If 'output' is a directory, the deb file will be stored in it. """

        arch = platform.rsplit("_", 1)[-1]
        assert arch in ("i386", "amd64")
        arch = {"i386" : "i686", "amd64" : "x86_64"}[arch]
        pkgver = self.version + "-1"

        output = Filename(output)
        if output.isDirectory():
            output = Filename(output, "%s-%s-%s.pkg.tar.gz" % (self.shortname.lower(), pkgver, arch))
        Installer.notify.info("Creating %s..." % output)
        modtime = int(time.time())

        # Create a temporary directory and write the launcher and dependencies to it.
        tempdir, totsize = self.__buildTempLinux(platform)

        # Create a pkginfo file in memory.
        pkginfo = StringIO()
        print >>pkginfo, "# Generated using pdeploy"
        print >>pkginfo, "# %s" % time.ctime(modtime)
        print >>pkginfo, "pkgname = %s" % self.shortname.lower()
        print >>pkginfo, "pkgver = %s" % pkgver
        print >>pkginfo, "pkgdesc = %s" % self.fullname
        print >>pkginfo, "builddate = %s" % modtime
        print >>pkginfo, "packager = %s <%s>" % (self.authorname, self.authoremail)
        print >>pkginfo, "size = %d" % totsize
        print >>pkginfo, "arch = %s" % arch
        if self.licensename != "":
            print >>pkginfo, "license = %s" % self.licensename
        pkginfoinfo = TarInfoRoot(".PKGINFO")
        pkginfoinfo.mtime = modtime
        pkginfoinfo.size = pkginfo.tell()
        pkginfo.seek(0)

        # Create the actual package now.
        pkgfile = tarfile.open(output.toOsSpecific(), "w:gz", tarinfo = TarInfoRoot)
        pkgfile.addfile(pkginfoinfo, pkginfo)
        pkgfile.add(tempdir.toOsSpecific(), "/")
        if not self.licensefile.empty():
            pkgfile.add(self.licensefile.toOsSpecific(), "/usr/share/licenses/%s/LICENSE" % self.shortname.lower())
        pkgfile.close()

        return output

    def buildAPP(self, output, platform):

        output = Filename(output)
        if output.isDirectory() and output.getExtension() != 'app':
            output = Filename(output, "%s.app" % self.fullname)
        Installer.notify.info("Creating %s..." % output)

        # Create the executable for the application bundle
        exefile = Filename(output, "Contents/MacOS/" + self.shortname)
        exefile.makeDir()
        if self.includeRequires:
            extraTokens = {"host_dir" : "../Resources"}
        else:
            extraTokens = {}
        self.standalone.build(exefile, platform, extraTokens)
        hostDir = Filename(output, "Contents/Resources/")
        hostDir.makeDir()
        self.installPackagesInto(hostDir, platform)

        hasIcon = False
        if self.icon is not None:
            Installer.notify.info("Generating %s.icns..." % self.shortname)
            hasIcon = self.icon.makeICNS(Filename(hostDir, "%s.icns" % self.shortname))

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
        if hasIcon:
            print >>plist, '\t<key>CFBundleIconFile</key>'
            print >>plist, '\t<string>%s.icns</string>' % self.shortname
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

        if hasattr(tarfile, "PAX_FORMAT"):
            archive = tarfile.open(Filename(output, "Contents/Archive.pax.gz").toOsSpecific(), "w:gz", format = tarfile.PAX_FORMAT, tarinfo = TarInfoRootOSX)
        else:
            archive = tarfile.open(Filename(output, "Contents/Archive.pax.gz").toOsSpecific(), "w:gz", tarinfo = TarInfoRootOSX)
        archive.add(appfn.toOsSpecific(), appname)
        archive.close()

        # Put the .pkg into a zipfile
        archive = Filename(output.getDirname(), "%s %s.zip" % (self.fullname, self.version))
        dir = Filename(output.getDirname())
        dir.makeAbsolute()
        zip = zipfile.ZipFile(archive.toOsSpecific(), 'w')
        for root, dirs, files in self.os_walk(output.toOsSpecific()):
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
            syspath = os.defpath.split(";") + os.environ["PATH"].split(";")
            for p in set(syspath):
                p1 = os.path.join(p, "makensis.exe")
                p2 = os.path.join(os.path.dirname(p), "nsis", "makensis.exe")
                if os.path.isfile(p1):
                    makensis = p1
                    break
                elif os.path.isfile(p2):
                    makensis = p2
                    break
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
            return None

        output = Filename(output)
        if output.isDirectory():
            output = Filename(output, "%s %s.exe" % (self.fullname, self.version))
        Installer.notify.info("Creating %s..." % output)
        output.makeAbsolute()
        extrafiles = self.standalone.getExtraFiles(platform)

        exefile = Filename(Filename.getTempDirectory(), self.shortname + ".exe")
        exefile.unlink()
        if self.includeRequires:
            extraTokens = {"host_dir" : "."}
        else:
            extraTokens = {}
        self.standalone.build(exefile, platform, extraTokens)

        # Temporary directory to store the hostdir in
        hostDir = Filename(self.tempDir, platform + "/")
        if not hostDir.exists():
            hostDir.makeDir()
            self.installPackagesInto(hostDir, platform)

        # See if we can generate an icon
        icofile = None
        if self.icon is not None:
            icofile = Filename(Filename.getTempDirectory(), self.shortname + ".ico")
            icofile.unlink()
            Installer.notify.info("Generating %s.ico..." % self.shortname)
            if not self.icon.makeICO(icofile):
                icofile = None

        # Create the .nsi installer script
        nsifile = Filename(Filename.getTempDirectory(), self.shortname + ".nsi")
        nsifile.unlink()
        nsi = open(nsifile.toOsSpecific(), "w")

        # Some global info
        print >>nsi, 'Name "%s"' % self.fullname
        print >>nsi, 'OutFile "%s"' % output.toOsSpecific()
        if platform == 'win_amd64':
            print >>nsi, 'InstallDir "$PROGRAMFILES64\\%s"' % self.fullname
        else:
            print >>nsi, 'InstallDir "$PROGRAMFILES\\%s"' % self.fullname
        print >>nsi, 'SetCompress auto'
        print >>nsi, 'SetCompressor lzma'
        print >>nsi, 'ShowInstDetails nevershow'
        print >>nsi, 'ShowUninstDetails nevershow'
        print >>nsi, 'InstType "Typical"'

        # Tell Vista that we require admin rights
        print >>nsi, 'RequestExecutionLevel admin'
        print >>nsi
        if self.offerRun:
            print >>nsi, 'Function launch'
            print >>nsi, '  ExecShell "open" "$INSTDIR\\%s.exe"' % self.shortname
            print >>nsi, 'FunctionEnd'
            print >>nsi

        if self.offerDesktopShortcut:
            print >>nsi, 'Function desktopshortcut'
            if icofile is None:
                print >>nsi, '  CreateShortcut "$DESKTOP\\%s.lnk" "$INSTDIR\\%s.exe"' % (self.fullname, self.shortname)
            else:
                print >>nsi, '  CreateShortcut "$DESKTOP\\%s.lnk" "$INSTDIR\\%s.exe" "" "$INSTDIR\\%s.ico"' % (self.fullname, self.shortname, self.shortname)
            print >>nsi, 'FunctionEnd'
            print >>nsi

        print >>nsi, '!include "MUI2.nsh"'
        print >>nsi, '!define MUI_ABORTWARNING'
        if self.offerRun:
            print >>nsi, '!define MUI_FINISHPAGE_RUN'
            print >>nsi, '!define MUI_FINISHPAGE_RUN_NOTCHECKED'
            print >>nsi, '!define MUI_FINISHPAGE_RUN_FUNCTION launch'
            print >>nsi, '!define MUI_FINISHPAGE_RUN_TEXT "Run %s"' % self.fullname
        if self.offerDesktopShortcut:
            print >>nsi, '!define MUI_FINISHPAGE_SHOWREADME ""'
            print >>nsi, '!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED'
            print >>nsi, '!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"'
            print >>nsi, '!define MUI_FINISHPAGE_SHOWREADME_FUNCTION desktopshortcut'
        print >>nsi
        print >>nsi, 'Var StartMenuFolder'
        print >>nsi, '!insertmacro MUI_PAGE_WELCOME'
        if not self.licensefile.empty():
            abs = Filename(self.licensefile)
            abs.makeAbsolute()
            print >>nsi, '!insertmacro MUI_PAGE_LICENSE "%s"' % abs.toOsSpecific()
        print >>nsi, '!insertmacro MUI_PAGE_DIRECTORY'
        print >>nsi, '!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder'
        print >>nsi, '!insertmacro MUI_PAGE_INSTFILES'
        print >>nsi, '!insertmacro MUI_PAGE_FINISH'
        print >>nsi, '!insertmacro MUI_UNPAGE_WELCOME'
        print >>nsi, '!insertmacro MUI_UNPAGE_CONFIRM'
        print >>nsi, '!insertmacro MUI_UNPAGE_INSTFILES'
        print >>nsi, '!insertmacro MUI_UNPAGE_FINISH'
        print >>nsi, '!insertmacro MUI_LANGUAGE "English"'

        # This section defines the installer.
        print >>nsi, 'Section "" SecCore'
        print >>nsi, '  SetOutPath "$INSTDIR"'
        print >>nsi, '  File "%s"' % exefile.toOsSpecific()
        if icofile is not None:
            print >>nsi, '  File "%s"' % icofile.toOsSpecific()
        for f in extrafiles:
            print >>nsi, '  File "%s"' % f.toOsSpecific()
        curdir = ""
        for root, dirs, files in self.os_walk(hostDir.toOsSpecific()):
            for name in files:
                basefile = Filename.fromOsSpecific(os.path.join(root, name))
                file = Filename(basefile)
                file.makeAbsolute()
                file.makeRelativeTo(hostDir)
                outdir = file.getDirname().replace('/', '\\')
                if curdir != outdir:
                    print >>nsi, '  SetOutPath "$INSTDIR\\%s"' % outdir
                    curdir = outdir
                print >>nsi, '  File "%s"' % (basefile.toOsSpecific())
        print >>nsi, '  SetOutPath "$INSTDIR"'
        print >>nsi, '  WriteUninstaller "$INSTDIR\\Uninstall.exe"'
        print >>nsi, '  ; Start menu items'
        print >>nsi, '  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application'
        print >>nsi, '    CreateDirectory "$SMPROGRAMS\\$StartMenuFolder"'
        if icofile is None:
            print >>nsi, '    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk" "$INSTDIR\\%s.exe"' % (self.fullname, self.shortname)
        else:
            print >>nsi, '    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk" "$INSTDIR\\%s.exe" "" "$INSTDIR\\%s.ico"' % (self.fullname, self.shortname, self.shortname)
        print >>nsi, '    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk" "$INSTDIR\\Uninstall.exe"'
        print >>nsi, '  !insertmacro MUI_STARTMENU_WRITE_END'
        print >>nsi, 'SectionEnd'

        # This section defines the uninstaller.
        print >>nsi, 'Section Uninstall'
        print >>nsi, '  Delete "$INSTDIR\\%s.exe"' % self.shortname
        if icofile is not None:
            print >>nsi, '  Delete "$INSTDIR\\%s.ico"' % self.shortname
        for f in extrafiles:
            print >>nsi, '  Delete "%s"' % f.getBasename()
        print >>nsi, '  Delete "$INSTDIR\\Uninstall.exe"'
        print >>nsi, '  RMDir /r "$INSTDIR"'
        print >>nsi, '  ; Desktop icon'
        print >>nsi, '  Delete "$DESKTOP\\%s.lnk"' % self.fullname
        print >>nsi, '  ; Start menu items'
        print >>nsi, '  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder'
        print >>nsi, '  Delete "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk"' % self.fullname
        print >>nsi, '  Delete "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk"'
        print >>nsi, '  RMDir "$SMPROGRAMS\\$StartMenuFolder"'
        print >>nsi, 'SectionEnd'
        nsi.close()

        cmd = [makensis]
        for o in ["V2"]:
            if sys.platform.startswith("win"):
                cmd.append("/" + o)
            else:
                cmd.append("-" + o)
        cmd.append(nsifile.toOsSpecific())
        print cmd
        try:
            retcode = subprocess.call(cmd, shell = False)
            if retcode != 0:
                self.notify.warning("Failure invoking NSIS command.")
            else:
                nsifile.unlink()
        except OSError:
            self.notify.warning("Unable to invoke NSIS command.")

        if icofile is not None:
            icofile.unlink()

        return output

    def os_walk(self, top):
        """ Re-implements os.walk().  For some reason the built-in
        definition is failing on Windows when this is run within a p3d
        environment!? """

        dirnames = []
        filenames = []

        dirlist = os.listdir(top)
        if dirlist:
            for file in dirlist:
                path = os.path.join(top, file)
                if os.path.isdir(path):
                    dirnames.append(file)
                else:
                    filenames.append(file)

        yield (top, dirnames, filenames)

        for dir in dirnames:
            next = os.path.join(top, dir)
            for tuple in self.os_walk(next):
                yield tuple
