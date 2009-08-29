from pandac.PandaModules import TiXmlDocument, HashVal, Filename, PandaSystem, URLSpec, Ramfile
from direct.p3d.PackageInfo import PackageInfo
from direct.p3d.FileSpec import FileSpec

class HostInfo:
    """ This class represents a particular download host serving up
    Panda3D packages.  It is the Python equivalent of the P3DHost
    class in the core API. """

    def __init__(self, hostUrl, appRunner):
        self.hostUrl = hostUrl

        # hostUrlPrefix is the host URL, but it is guaranteed to end
        # with a slash.
        self.hostUrlPrefix = hostUrl
        if self.hostUrlPrefix[-1] != '/':
            self.hostUrlPrefix += '/'

        # Initially false, this is set true when the contents file is
        # successfully read.
        self.hasContentsFile = False

        # descriptiveName will be filled in later, when the
        # contents file is read.
        self.descriptiveName = ''

        # This is a dictionary of packages by (name, version).  It
        # will be filled in when the contents file is read.
        self.packages = {}

        self.__determineHostDir(appRunner)
        self.importsDir = Filename(self.hostDir, 'imports')

    def downloadContentsFile(self, http):
        """ Downloads the contents.xml file for this particular host,
        synchronously, and then reads it.  Returns true on success,
        false on failure. """

        if self.hasContentsFile:
            # We've already got one.
            return True

        url = URLSpec(self.hostUrlPrefix + 'contents.xml')
        print "Downloading %s" % (url)

        rf = Ramfile()
        channel = http.getDocument(url)
        if not channel.downloadToRam(rf):
            print "Unable to download %s" % (url)

        filename = Filename(self.hostDir, 'contents.xml')
        filename.makeDir()
        f = open(filename.toOsSpecific(), 'wb')
        f.write(rf.getData())
        f.close()

        try:
            self.readContentsFile()
        except ValueError:
            print "Failure reading %s" % (filename)
            return False

        return True

    def readContentsFile(self):
        """ Reads the contents.xml file for this particular host.
        Presumably this has already been downloaded and installed. """

        if self.hasContentsFile:
            # No need to read it again.
            return

        filename = Filename(self.hostDir, 'contents.xml')

        doc = TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            raise ValueError

        xcontents = doc.FirstChildElement('contents')
        if not xcontents:
            raise ValueError

        self.descriptiveName = xcontents.Attribute('descriptive_name')

        # Get the list of packages available for download and/or import.
        xpackage = xcontents.FirstChildElement('package')
        while xpackage:
            name = xpackage.Attribute('name')
            platform = xpackage.Attribute('platform')
            version = xpackage.Attribute('version')
            package = self.__makePackage(name, platform, version)
            package.descFile = FileSpec()
            package.descFile.loadXml(xpackage)
            package.setupFilenames()

            package.importDescFile = None
            ximport = xpackage.FirstChildElement('import')
            if ximport:
                package.importDescFile = FileSpec()
                package.importDescFile.loadXml(ximport)

            xpackage = xpackage.NextSiblingElement('package')

        self.hasContentsFile = True

    def __makePackage(self, name, platform, version):
        """ Creates a new PackageInfo entry for the given name,
        version, and platform.  If there is already a matching
        PackageInfo, returns it. """

        if not platform:
            # Ensure that we're on the same page with non-specified
            # platforms.  We always use None, not empty string.
            platform = None

        platforms = self.packages.setdefault((name, version), {})
        package = platforms.get(platform, None)
        if not package:
            package = PackageInfo(self, name, version, platform = platform)
            platforms[platform] = package

        return package

    def getPackage(self, name, version, platform = None):
        """ Returns a PackageInfo that matches the indicated name and
        version and the indicated platform or the current runtime
        platform, if one is provided by this host, or None if not. """

        assert self.hasContentsFile
        platforms = self.packages.get((name, version or None), {})

        if platform is not None:
            # In this case, we are looking for a specific platform
            # only.
            return platforms.get(platform or None, None)

        # We are looking for one matching the current runtime
        # platform.  First, look for a package matching the current
        # platform exactly.
        package = platforms.get(PandaSystem.getPlatform(), None)

        # If not found, look for one matching no particular platform.
        if not package:
            package = platforms.get(None, None)

        return package

    def __determineHostDir(self, appRunner):
        """ Hashes the host URL into a (mostly) unique directory
        string, which will be the root of the host's install tree.
        Stores the resulting path, as a Filename, in self.hostDir.

        This code is duplicated in C++, in
        P3DHost::determine_host_dir(). """

        hostDir = ''

        # Look for a server name in the URL.  Including this string in the
        # directory name makes it friendlier for people browsing the
        # directory.

        # We could use URLSpec, but we do it by hand instead, to make
        # it more likely that our hash code will exactly match the
        # similar logic in P3DHost.
        p = self.hostUrl.find('://')
        if p != -1:
            start = p + 3
            end = self.hostUrl.find('/', start)
            # Now start .. end is something like "username@host:port".

            at = self.hostUrl.find('@', start)
            if at != -1 and at < end:
                start = at + 1

            colon = self.hostUrl.find(':', start)
            if colon != -1 and colon < end:
                end = colon

            # Now start .. end is just the hostname.
            hostname = self.hostUrl[start : end]

        # Now build a hash string of the whole URL.  We'll use MD5 to
        # get a pretty good hash, with a minimum chance of collision.
        # Even if there is a hash collision, though, it's not the end
        # of the world; it just means that both hosts will dump their
        # packages into the same directory, and they'll fight over the
        # toplevel contents.xml file.  Assuming they use different
        # version numbers (which should be safe since they have the
        # same hostname), there will be minimal redownloading.

        hashSize = 16
        keepHash = hashSize
        if hostname:
            hostDir += hostname + '_'

            # If we successfully got a hostname, we don't really need the
            # full hash.  We'll keep half of it.
            keepHash = keepHash / 2;

        md = HashVal()
        md.hashString(self.hostUrl)
        hostDir += md.asHex()[:keepHash * 2]

        self.hostDir = Filename(appRunner.rootDir, hostDir)
