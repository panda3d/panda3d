""" This module is used to build a graphical installer
or a standalone executable from a p3d file. It will try
to build for as many platforms as possible.

.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""

__all__ = ["Standalone", "Installer"]

import os, sys, subprocess, tarfile, shutil, time, zipfile, socket, getpass, struct
import gzip, plistlib
from direct.directnotify.DirectNotifyGlobal import *
from direct.showbase.AppRunnerGlobal import appRunner
from panda3d.core import PandaSystem, Filename, VirtualFileSystem, Multifile
from panda3d.core import TiXmlDocument, TiXmlDeclaration, TiXmlElement, readXmlStream
from panda3d.core import PNMImage, PNMFileTypeRegistry, StringStream
from panda3d import core
from direct.stdpy.file import *
from direct.p3d.HostInfo import HostInfo
# This is important for some reason
import encodings

try:
    import pwd
except ImportError:
    pwd = None

if sys.version_info >= (3, 0):
    xrange = range
    from io import BytesIO, TextIOWrapper
else:
    from io import BytesIO
    from StringIO import StringIO

# Make sure this matches with the magic in p3dEmbedMain.cxx.
P3DEMBED_MAGIC = 0xFF3D3D00

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
        self.host = HostInfo(PandaSystem.getPackageHostUrl(), appRunner = appRunner, hostDir = self.tempDir, asMirror = False, perPlatform = True)

        self.http = core.HTTPClient.getGlobalPtr()
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

        if 'win32' in platforms and 'win_i386' in platforms:
            platforms.remove('win32')

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
        p3dembed_data = p3dembed_data.replace(struct.pack('>I', P3DEMBED_MAGIC),
                                              struct.pack('>I', size))
        p3dembed_data = p3dembed_data.replace(struct.pack('<I', P3DEMBED_MAGIC),
                                              struct.pack('<I', size))

        # Write the output file
        Standalone.notify.info("Creating %s..." % output)
        output.makeDir()
        ohandle = open(output.toOsSpecific(), "wb")
        ohandle.write(p3dembed_data)

        # Write out the tokens. Set log_basename to the basename by default
        tokens = {"log_basename": self.basename}
        tokens.update(self.tokens)
        tokens.update(extraTokens)
        for key, value in tokens.items():
            ohandle.write(b"\0")
            ohandle.write(key.encode('ascii'))
            ohandle.write(b"=")
            ohandle.write(value.encode())
        ohandle.write(b"\0\0")

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
        self.http = core.HTTPClient.getGlobalPtr()

    def getHost(self, hostUrl):
        if hostUrl in self.hosts:
            return self.hosts[hostUrl]

        host = HostInfo(hostUrl, appRunner = appRunner, hostDir = self.hostDir, asMirror = False, perPlatform = True)
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

    def generateMissingImages(self):
        """ Generates image sizes that should be present but aren't by scaling
        from the next higher size. """

        for required_size in (256, 128, 48, 32, 16):
            if required_size in self.images:
                continue

            sizes = sorted(self.images.keys())
            if required_size * 2 in sizes:
                from_size = required_size * 2
            else:
                from_size = 0
                for from_size in sizes:
                    if from_size > required_size:
                        break

            if from_size > required_size:
                Icon.notify.warning("Generating %dx%d icon by scaling down %dx%d image" % (required_size, required_size, from_size, from_size))

                image = PNMImage(required_size, required_size)
                if self.images[from_size].hasAlpha():
                    image.addAlpha()
                image.quickFilterFrom(self.images[from_size])
                self.images[required_size] = image
            else:
                Icon.notify.warning("Cannot generate %dx%d icon; no higher resolution image available" % (required_size, required_size))

    def _write_bitmap(self, fp, image, size, bpp):
        """ Writes the bitmap header and data of an .ico file. """

        fp.write(struct.pack('<IiiHHIIiiII', 40, size, size * 2, 1, bpp, 0, 0, 0, 0, 0, 0))

        # XOR mask
        if bpp == 24:
            # Align rows to 4-byte boundary
            rowalign = b'\0' * (-(size * 3) & 3)
            for y in xrange(size):
                for x in xrange(size):
                    r, g, b = image.getXel(x, size - y - 1)
                    fp.write(struct.pack('<BBB', int(b * 255), int(g * 255), int(r * 255)))
                fp.write(rowalign)

        elif bpp == 32:
            for y in xrange(size):
                for x in xrange(size):
                    r, g, b, a = image.getXelA(x, size - y - 1)
                    fp.write(struct.pack('<BBBB', int(b * 255), int(g * 255), int(r * 255), int(a * 255)))

        elif bpp == 8:
            # We'll have to generate a palette of 256 colors.
            hist = PNMImage.Histogram()
            image2 = PNMImage(image)
            if image2.hasAlpha():
                image2.premultiplyAlpha()
                image2.removeAlpha()
            image2.quantize(256)
            image2.make_histogram(hist)
            colors = list(hist.get_pixels())
            assert len(colors) <= 256

            # Write the palette.
            i = 0
            while i < 256 and i < len(colors):
                r, g, b, a = colors[i]
                fp.write(struct.pack('<BBBB', b, g, r, 0))
                i += 1
            if i < 256:
                # Fill the rest with zeroes.
                fp.write(b'\x00' * (4 * (256 - i)))

            # Write indices.  Align rows to 4-byte boundary.
            rowalign = b'\0' * (-size & 3)
            for y in xrange(size):
                for x in xrange(size):
                    pixel = image2.get_pixel(x, size - y - 1)
                    index = colors.index(pixel)
                    if index >= 256:
                        # Find closest pixel instead.
                        index = closest_indices[index - 256]
                    fp.write(struct.pack('<B', index))
                fp.write(rowalign)
        else:
            raise ValueError("Invalid bpp %d" % (bpp))

        # Create an AND mask, aligned to 4-byte boundary
        if image.hasAlpha() and bpp <= 8:
            rowalign = b'\0' * (-((size + 7) >> 3) & 3)
            for y in xrange(size):
                mask = 0
                num_bits = 7
                for x in xrange(size):
                    a = image.get_alpha_val(x, size - y - 1)
                    if a <= 1:
                        mask |= (1 << num_bits)
                    num_bits -= 1
                    if num_bits < 0:
                        fp.write(struct.pack('<B', mask))
                        mask = 0
                        num_bits = 7
                if num_bits < 7:
                    fp.write(struct.pack('<B', mask))
                fp.write(rowalign)
        else:
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            fp.write(b'\x00' * (andsize * size))

    def makeICO(self, fn):
        """ Writes the images to a Windows ICO file.  Returns True on success. """

        if not isinstance(fn, Filename):
            fn = Filename.fromOsSpecific(fn)
        fn.setBinary()

        # ICO files only support resolutions up to 256x256.
        count = 0
        for size in self.images.keys():
            if size < 256:
                count += 1
            if size <= 256:
                count += 1
        dataoffs = 6 + count * 16

        ico = open(fn, 'wb')
        ico.write(struct.pack('<HHH', 0, 1, count))

        # Write 8-bpp image headers for sizes under 256x256.
        for size, image in self.images.items():
            if size >= 256:
                continue
            ico.write(struct.pack('<BB', size, size))

            # Calculate row sizes
            xorsize = size
            if xorsize % 4 != 0:
                xorsize += 4 - (xorsize % 4)
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            datasize = 40 + 256 * 4 + (xorsize + andsize) * size

            ico.write(struct.pack('<BBHHII', 0, 0, 1, 8, datasize, dataoffs))
            dataoffs += datasize

        # Write 24/32-bpp image headers.
        for size, image in self.images.items():
            if size > 256:
                continue
            elif size == 256:
                ico.write(b'\0\0')
            else:
                ico.write(struct.pack('<BB', size, size))

            # Calculate the size so we can write the offset within the file.
            if image.hasAlpha():
                bpp = 32
                xorsize = size * 4
            else:
                bpp = 24
                xorsize = size * 3 + (-(size * 3) & 3)
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            datasize = 40 + (xorsize + andsize) * size

            ico.write(struct.pack('<BBHHII', 0, 0, 1, bpp, datasize, dataoffs))
            dataoffs += datasize

        # Now write the actual icon bitmap data.
        for size, image in self.images.items():
            if size < 256:
                self._write_bitmap(ico, image, size, 8)

        for size, image in self.images.items():
            if size <= 256:
                bpp = 32 if image.hasAlpha() else 24
                self._write_bitmap(ico, image, size, bpp)

        assert ico.tell() == dataoffs
        ico.close()

        return True

    def makeICNS(self, fn):
        """ Writes the images to an Apple ICNS file.  Returns True on success. """

        if not isinstance(fn, Filename):
            fn = Filename.fromOsSpecific(fn)
        fn.setBinary()

        icns = open(fn, 'wb')
        icns.write(b'icns\0\0\0\0')

        icon_types = {16: b'is32', 32: b'il32', 48: b'ih32', 128: b'it32'}
        mask_types = {16: b's8mk', 32: b'l8mk', 48: b'h8mk', 128: b't8mk'}
        png_types = {256: b'ic08', 512: b'ic09', 1024: b'ic10'}

        pngtype = PNMFileTypeRegistry.getGlobalPtr().getTypeFromExtension("png")

        for size, image in sorted(self.images.items(), key=lambda item:item[0]):
            if size in png_types and pngtype is not None:
                stream = StringStream()
                image.write(stream, "", pngtype)
                pngdata = stream.data

                icns.write(png_types[size])
                icns.write(struct.pack('>I', len(pngdata)))
                icns.write(pngdata)

            elif size in icon_types:
                # If it has an alpha channel, we write out a mask too.
                if image.hasAlpha():
                    icns.write(mask_types[size])
                    icns.write(struct.pack('>I', size * size + 8))

                    for y in xrange(size):
                        for x in xrange(size):
                            icns.write(struct.pack('<B', int(image.getAlpha(x, y) * 255)))

                icns.write(icon_types[size])
                icns.write(struct.pack('>I', size * size * 4 + 8))

                for y in xrange(size):
                    for x in xrange(size):
                        r, g, b = image.getXel(x, y)
                        icns.write(struct.pack('>BBBB', 0, int(r * 255), int(g * 255), int(b * 255)))

        length = icns.tell()
        icns.seek(4)
        icns.write(struct.pack('>I', length))
        icns.close()

        return True


class Installer:
    """ This class creates a (graphical) installer from a given .p3d file. """
    notify = directNotify.newCategory("Installer")

    def __init__(self, p3dfile, shortname, fullname, version, tokens = {}):
        self.p3dFilename = p3dfile
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

        # Load the p3d file to read out the required packages
        mf = Multifile()
        if not mf.openRead(self.p3dFilename):
            Installer.notify.error("Not a Panda3D application: %s" % (p3dfile))
            return

        # Now load the p3dInfo file.
        self.hostUrl = None
        self.requires = []
        self.extracts = []
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

                p3dExtract = p3dPackage.FirstChildElement('extract')
                while p3dExtract:
                    filename = p3dExtract.Attribute('filename')
                    self.extracts.append(filename)
                    p3dExtract = p3dExtract.NextSiblingElement('extract')

                if not self.fullname:
                    p3dConfig = p3dPackage.FirstChildElement('config')
                    if p3dConfig:
                        self.fullname = p3dConfig.Attribute('display_name')
        else:
            Installer.notify.warning("No p3d_info.xml was found in .p3d archive.")

        mf.close()

        if not self.hostUrl:
            self.hostUrl = PandaSystem.getPackageHostUrl()
            if not self.hostUrl:
                self.hostUrl = self.standalone.host.hostUrl
            Installer.notify.warning("No host URL was specified by .p3d archive.  Falling back to %s" % (self.hostUrl))

        if not self.fullname:
            self.fullname = self.shortname

        self.tempDir = Filename.temporary("", self.shortname, "") + "/"
        self.tempDir.makeDir()
        self.__tempRoots = {}

        if self.extracts:
            # Copy .p3d to a temporary file so we can remove the extracts.
            p3dfile = Filename(self.tempDir, self.p3dFilename.getBasename())
            shutil.copyfile(self.p3dFilename.toOsSpecific(), p3dfile.toOsSpecific())
            mf = Multifile()
            if not mf.openReadWrite(p3dfile):
                Installer.notify.error("Failure to open %s for writing." % (p3dfile))

            # We don't really need this silly thing when embedding, anyway.
            mf.setHeaderPrefix("")

            for fn in self.extracts:
                if not mf.removeSubfile(fn):
                    Installer.notify.error("Failure to remove %s from multifile." % (p3dfile))

            mf.repack()
            mf.close()

        self.standalone = Standalone(p3dfile, tokens)

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

        # Write out the extracts from the original .p3d.
        if self.extracts:
            mf = Multifile()
            if not mf.openRead(self.p3dFilename):
                Installer.notify.error("Failed to open .p3d archive: %s" % (filename))

            for filename in self.extracts:
                i = mf.findSubfile(filename)
                if i < 0:
                    Installer.notify.error("Cannot find extract in .p3d archive: %s" % (filename))
                    continue

                if not mf.extractSubfile(i, Filename(hostDir, filename)):
                    Installer.notify.error("Failed to extract file from .p3d archive: %s" % (filename))
            mf.close()

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

        if 'win32' in platforms and 'win_i386' in platforms:
            platforms.remove('win32')

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
            extraTokens = {"host_dir" : "/usr/lib/" + self.shortname.lower(),
                           "start_dir" : "/usr/lib/" + self.shortname.lower()}
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
        desktop.write("[Desktop Entry]\n")
        desktop.write("Name=%s\n" % self.fullname)
        desktop.write("Exec=%s\n" % self.shortname.lower())
        if iconFile is not None:
            desktop.write("Icon=%s\n" % iconFile.getBasename())

        # Set the "Terminal" option based on whether or not a console env is requested
        cEnv = self.standalone.tokens.get("console_environment", "")
        if cEnv == "" or int(cEnv) == 0:
            desktop.write("Terminal=false\n")
        else:
            desktop.write("Terminal=true\n")

        desktop.write("Type=Application\n")
        desktop.close()

        if self.includeRequires or self.extracts:
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
        controlfile = BytesIO()
        if sys.version_info >= (3, 0):
            cout = TextIOWrapper(controlfile, encoding='utf-8', newline='')
        else:
            cout = StringIO()

        cout.write("Package: %s\n" % self.shortname.lower())
        cout.write("Version: %s\n" % self.version)
        cout.write("Maintainer: %s <%s>\n" % (self.authorname, self.authoremail))
        cout.write("Section: games\n")
        cout.write("Priority: optional\n")
        cout.write("Architecture: %s\n" % arch)
        cout.write("Installed-Size: %d\n" % -(-totsize // 1024))
        cout.write("Description: %s\n" % self.fullname)
        cout.write("Depends: libc6, libgcc1, libstdc++6, libx11-6\n")
        cout.flush()
        if sys.version_info < (3, 0):
            controlfile.write(cout.getvalue().encode('utf-8'))

        controlinfo = TarInfoRoot("control")
        controlinfo.mtime = modtime
        controlinfo.size = controlfile.tell()
        controlfile.seek(0)

        # Open the deb file and write to it. It's actually
        # just an AR file, which is very easy to make.
        if output.exists():
            output.unlink()
        debfile = open(output.toOsSpecific(), "wb")
        debfile.write(b"!<arch>\x0A")
        pad_mtime = str(modtime).encode().ljust(12, b' ')

        # The first entry is a special file that marks it a .deb.
        debfile.write(b"debian-binary   ")
        debfile.write(pad_mtime)
        debfile.write(b"0     0     100644  4         \x60\x0A")
        debfile.write(b"2.0\x0A")

        # Write the control.tar.gz to the archive.  We'll leave the
        # size 0 for now, and go back and fill it in later.
        debfile.write(b"control.tar.gz  ")
        debfile.write(pad_mtime)
        debfile.write(b"0     0     100644  0         \x60\x0A")
        ctaroffs = debfile.tell()
        ctarfile = tarfile.open("control.tar.gz", "w:gz", debfile, tarinfo = TarInfoRoot)
        ctarfile.addfile(controlinfo, controlfile)
        ctarfile.close()
        ctarsize = debfile.tell() - ctaroffs
        if (ctarsize & 1): debfile.write(b"\x0A")

        # Write the data.tar.gz to the archive.  Again, leave size 0.
        debfile.write(b"data.tar.gz     ")
        debfile.write(pad_mtime)
        debfile.write(b"0     0     100644  0         \x60\x0A")
        dtaroffs = debfile.tell()
        dtarfile = tarfile.open("data.tar.gz", "w:gz", debfile, tarinfo = TarInfoRoot)
        dtarfile.add(Filename(tempdir, "usr").toOsSpecific(), "/usr")
        dtarfile.close()
        dtarsize = debfile.tell() - dtaroffs
        if (dtarsize & 1): debfile.write(b"\x0A")

        # Write the correct sizes of the archives.
        debfile.seek(ctaroffs - 12)
        debfile.write(str(ctarsize).encode().ljust(10, b' '))
        debfile.seek(dtaroffs - 12)
        debfile.write(str(dtarsize).encode().ljust(10, b' '))

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
        pkginfo = BytesIO()
        if sys.version_info >= (3, 0):
            pout = TextIOWrapper(pkginfo, encoding='utf-8', newline='')
        else:
            pout = StringIO()

        pout.write("# Generated using pdeploy\n")
        pout.write("# %s\n" % time.ctime(modtime))
        pout.write("pkgname = %s\n" % self.shortname.lower())
        pout.write("pkgver = %s\n" % pkgver)
        pout.write("pkgdesc = %s\n" % self.fullname)
        pout.write("builddate = %s\n" % modtime)
        pout.write("packager = %s <%s>\n" % (self.authorname, self.authoremail))
        pout.write("size = %d\n" % totsize)
        pout.write("arch = %s\n" % arch)
        if self.licensename != "":
            pout.write("license = %s\n" % self.licensename)
        pout.flush()
        if sys.version_info < (3, 0):
            pkginfo.write(pout.getvalue().encode('utf-8'))

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
            extraTokens = {"host_dir": "../Resources", "start_dir": "../Resources"}
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

        # Create the application plist file using Python's plistlib module.
        plist = {
            'CFBundleDevelopmentRegion': 'English',
            'CFBundleDisplayName': self.fullname,
            'CFBundleExecutable': exefile.getBasename(),
            'CFBundleIdentifier': '%s.%s' % (self.author, self.shortname),
            'CFBundleInfoDictionaryVersion': '6.0',
            'CFBundleName': self.shortname,
            'CFBundlePackageType': 'APPL',
            'CFBundleShortVersionString': self.version,
            'CFBundleVersion': self.version,
            'LSHasLocalizedDisplayName': False,
            'NSAppleScriptEnabled': False,
            'NSPrincipalClass': 'NSApplication',
        }
        if hasIcon:
            plist['CFBundleIconFile'] = self.shortname + '.icns'

        plistlib.writePlist(plist, Filename(output, "Contents/Info.plist").toOsSpecific())
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
        pkginfo.write("pmkrpkg1")
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

        # OS X El Capitan no longer accepts .pax archives - it must be a CPIO archive named .pax.
        archive = gzip.open(Filename(output, "Contents/Archive.pax.gz").toOsSpecific(), 'wb')
        self.__ino = 0
        self.__writeCPIO(archive, appfn, appname)
        archive.write(b"0707070000000000000000000000000000000000010000000000000000000001300000000000TRAILER!!!\0")
        archive.close()

        # Put the .pkg into a zipfile
        zip_fn = Filename(output.getDirname(), "%s %s.pkg.zip" % (self.fullname, self.version))
        dir = Filename(output.getDirname())
        dir.makeAbsolute()
        zip = zipfile.ZipFile(zip_fn.toOsSpecific(), 'w')
        for root, dirs, files in self.os_walk(output.toOsSpecific()):
            for name in files:
                file = Filename.fromOsSpecific(os.path.join(root, name))
                file.makeAbsolute()
                file.makeRelativeTo(dir)
                zip.write(os.path.join(root, name), str(file))
        zip.close()

        return output

    def __writeCPIO(self, archive, fn, name):
        """ Adds the given fn under the given name to the CPIO archive. """

        st = os.lstat(fn.toOsSpecific())

        archive.write(b"070707") # magic
        archive.write(b"000000") # dev

        # Synthesize an inode number, different for each entry.
        self.__ino += 1
        archive.write("%06o" % (self.__ino))

        # Determine based on the type which mode to write.
        if os.path.islink(fn.toOsSpecific()):
            archive.write("%06o" % (st.st_mode))
            target = os.path.readlink(fn.toOsSpecific()).encode('utf-8')
            size = len(target)
        elif os.path.isdir(fn.toOsSpecific()):
            archive.write(b"040755")
            size = 0
        elif not fn.getExtension():  # Binary file?
            archive.write(b"100755")
            size = st.st_size
        else:
            archive.write(b"100644")
            size = st.st_size

        archive.write("000000") # uid (root)
        archive.write("000000") # gid (wheel)
        archive.write("%06o" % (st.st_nlink))
        archive.write("000000") # rdev
        archive.write("%011o" % (st.st_mtime))
        archive.write("%06o" % (len(name) + 1))
        archive.write("%011o" % (size))

        # Write the filename, plus terminating NUL byte.
        archive.write(name.encode('utf-8'))
        archive.write(b"\0")

        # Copy the file data to the archive.
        if os.path.islink(fn.toOsSpecific()):
            archive.write(target)
        elif size:
            handle = open(fn.toOsSpecific(), 'rb')
            data = handle.read(1024 * 1024)
            while data:
                archive.write(data)
                data = handle.read(1024 * 1024)
            handle.close()

        # If this is a directory, recurse.
        if os.path.isdir(fn.toOsSpecific()):
            for child in os.listdir(fn.toOsSpecific()):
                self.__writeCPIO(archive, Filename(fn, child), name + "/" + child)

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
            extraTokens = {"host_dir": ".", "start_dir": "."}
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
        nsi.write('Name "%s"\n' % self.fullname)
        nsi.write('OutFile "%s"\n' % output.toOsSpecific())
        if platform == 'win_amd64':
            nsi.write('InstallDir "$PROGRAMFILES64\\%s"\n' % self.fullname)
        else:
            nsi.write('InstallDir "$PROGRAMFILES\\%s"\n' % self.fullname)
        nsi.write('SetCompress auto\n')
        nsi.write('SetCompressor lzma\n')
        nsi.write('ShowInstDetails nevershow\n')
        nsi.write('ShowUninstDetails nevershow\n')
        nsi.write('InstType "Typical"\n')

        # Tell Vista that we require admin rights
        nsi.write('RequestExecutionLevel admin\n')
        nsi.write('\n')
        if self.offerRun:
            nsi.write('Function launch\n')
            nsi.write('  ExecShell "open" "$INSTDIR\\%s.exe"\n' % self.shortname)
            nsi.write('FunctionEnd\n')
            nsi.write('\n')

        if self.offerDesktopShortcut:
            nsi.write('Function desktopshortcut\n')
            if icofile is None:
                nsi.write('  CreateShortcut "$DESKTOP\\%s.lnk" "$INSTDIR\\%s.exe"\n' % (self.fullname, self.shortname))
            else:
                nsi.write('  CreateShortcut "$DESKTOP\\%s.lnk" "$INSTDIR\\%s.exe" "" "$INSTDIR\\%s.ico"\n' % (self.fullname, self.shortname, self.shortname))
            nsi.write('FunctionEnd\n')
            nsi.write('\n')

        nsi.write('!include "MUI2.nsh"\n')
        nsi.write('!define MUI_ABORTWARNING\n')
        if self.offerRun:
            nsi.write('!define MUI_FINISHPAGE_RUN\n')
            nsi.write('!define MUI_FINISHPAGE_RUN_NOTCHECKED\n')
            nsi.write('!define MUI_FINISHPAGE_RUN_FUNCTION launch\n')
            nsi.write('!define MUI_FINISHPAGE_RUN_TEXT "Run %s"\n' % self.fullname)
        if self.offerDesktopShortcut:
            nsi.write('!define MUI_FINISHPAGE_SHOWREADME ""\n')
            nsi.write('!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED\n')
            nsi.write('!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"\n')
            nsi.write('!define MUI_FINISHPAGE_SHOWREADME_FUNCTION desktopshortcut\n')
        nsi.write('\n')
        nsi.write('Var StartMenuFolder\n')
        nsi.write('!insertmacro MUI_PAGE_WELCOME\n')
        if not self.licensefile.empty():
            abs = Filename(self.licensefile)
            abs.makeAbsolute()
            nsi.write('!insertmacro MUI_PAGE_LICENSE "%s"\n' % abs.toOsSpecific())
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
        if icofile is not None:
            nsi.write('  File "%s"\n' % icofile.toOsSpecific())
        for f in extrafiles:
            nsi.write('  File "%s"\n' % f.toOsSpecific())
        curdir = ""
        for root, dirs, files in self.os_walk(hostDir.toOsSpecific()):
            for name in files:
                basefile = Filename.fromOsSpecific(os.path.join(root, name))
                file = Filename(basefile)
                file.makeAbsolute()
                file.makeRelativeTo(hostDir)
                outdir = file.getDirname().replace('/', '\\')
                if curdir != outdir:
                    nsi.write('  SetOutPath "$INSTDIR\\%s"\n' % outdir)
                    curdir = outdir
                nsi.write('  File "%s"\n' % (basefile.toOsSpecific()))
        nsi.write('  SetOutPath "$INSTDIR"\n')
        nsi.write('  WriteUninstaller "$INSTDIR\\Uninstall.exe"\n')
        nsi.write('  ; Start menu items\n')
        nsi.write('  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application\n')
        nsi.write('    CreateDirectory "$SMPROGRAMS\\$StartMenuFolder"\n')
        if icofile is None:
            nsi.write('    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk" "$INSTDIR\\%s.exe"\n' % (self.fullname, self.shortname))
        else:
            nsi.write('    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk" "$INSTDIR\\%s.exe" "" "$INSTDIR\\%s.ico"\n' % (self.fullname, self.shortname, self.shortname))
        nsi.write('    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk" "$INSTDIR\\Uninstall.exe"\n')
        nsi.write('  !insertmacro MUI_STARTMENU_WRITE_END\n')
        nsi.write('SectionEnd\n')

        # This section defines the uninstaller.
        nsi.write('Section Uninstall\n')
        nsi.write('  Delete "$INSTDIR\\%s.exe"\n' % self.shortname)
        if icofile is not None:
            nsi.write('  Delete "$INSTDIR\\%s.ico"\n' % self.shortname)
        for f in extrafiles:
            nsi.write('  Delete "%s"\n' % f.getBasename())
        nsi.write('  Delete "$INSTDIR\\Uninstall.exe"\n')
        nsi.write('  RMDir /r "$INSTDIR"\n')
        nsi.write('  ; Desktop icon\n')
        nsi.write('  Delete "$DESKTOP\\%s.lnk"\n' % self.fullname)
        nsi.write('  ; Start menu items\n')
        nsi.write('  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder\n')
        nsi.write('  Delete "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk"\n' % self.fullname)
        nsi.write('  Delete "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk"\n')
        nsi.write('  RMDir "$SMPROGRAMS\\$StartMenuFolder"\n')
        nsi.write('SectionEnd\n')
        nsi.close()

        cmd = [makensis]
        for o in ["V2"]:
            if sys.platform.startswith("win"):
                cmd.append("/" + o)
            else:
                cmd.append("-" + o)
        cmd.append(nsifile.toOsSpecific())
        print(cmd)
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
