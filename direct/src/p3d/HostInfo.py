from pandac.PandaModules import TiXmlDocument, HashVal, Filename, PandaSystem, DocumentSpec, Ramfile
from direct.p3d.PackageInfo import PackageInfo
from direct.p3d.FileSpec import FileSpec
import time

class HostInfo:
    """ This class represents a particular download host serving up
    Panda3D packages.  It is the Python equivalent of the P3DHost
    class in the core API. """

    def __init__(self, hostUrl, appRunner = None, hostDir = None,
                 asMirror = False):

        """ You must specify either an appRunner or a hostDir to the
        HostInfo constructor.

        If you pass asMirror = True, it means that this HostInfo
        object is to be used to populate a "mirror" folder, a
        duplicate (or subset) of the contents hosted by a server.
        This means when you use this HostInfo to download packages, it
        will only download the compressed archive file and leave it
        there.  At the moment, mirror folders do not download old
        patch files from the server. """
        
        assert appRunner or hostDir
        
        self.hostUrl = hostUrl
        self.appRunner = appRunner
        self.hostDir = hostDir
        self.asMirror = asMirror

        # hostUrlPrefix is the host URL, but it is guaranteed to end
        # with a slash.
        self.hostUrlPrefix = hostUrl
        if self.hostUrlPrefix[-1] != '/':
            self.hostUrlPrefix += '/'

        # downloadUrlPrefix is the URL prefix that should be used for
        # everything other than the contents.xml file.  It might be
        # the same as hostUrlPrefix, but in the case of an
        # https-protected hostUrl, it will be the cleartext channel.
        self.downloadUrlPrefix = self.hostUrlPrefix

        # Initially false, this is set true when the contents file is
        # successfully read.
        self.hasContentsFile = False

        # descriptiveName will be filled in later, when the
        # contents file is read.
        self.descriptiveName = None

        # A list of known mirrors for this host, all URL's guaranteed
        # to end with a slash.
        self.mirrors = []

        # A map of keyword -> altHost URL's.  An altHost is different
        # than a mirror; an altHost is an alternate URL to download a
        # different (e.g. testing) version of this host's contents.
        # It is rarely used.
        self.altHosts = {}

        # This is a dictionary of packages by (name, version).  It
        # will be filled in when the contents file is read.
        self.packages = {}

        if appRunner:
            self.__determineHostDir(appRunner)

        assert self.hostDir
        self.importsDir = Filename(self.hostDir, 'imports')

    def downloadContentsFile(self, http):
        """ Downloads the contents.xml file for this particular host,
        synchronously, and then reads it.  Returns true on success,
        false on failure. """

        if self.hasContentsFile:
            # We've already got one.
            return True

        url = self.hostUrlPrefix + 'contents.xml'
        # Append a uniquifying query string to the URL to force the
        # download to go all the way through any caches.  We use the
        # time in seconds; that's unique enough.
        url += '?' + str(int(time.time()))

        # We might as well explicitly request the cache to be disabled
        # too, since we have an interface for that via HTTPChannel.
        request = DocumentSpec(url)
        request.setCacheControl(DocumentSpec.CCNoCache)

        print "Downloading contents file %s" % (request)

        rf = Ramfile()
        channel = http.makeChannel(False)
        channel.getDocument(request)
        if not channel.downloadToRam(rf):
            print "Unable to download %s" % (url)

        filename = Filename(self.hostDir, 'contents.xml')
        filename.makeDir()
        f = open(filename.toOsSpecific(), 'wb')
        f.write(rf.getData())
        f.close()

        if not self.readContentsFile():
            print "Failure reading %s" % (filename)
            return False

        return True

    def redownloadContentsFile(self, http):
        """ Downloads a new contents.xml file in case it has changed.
        Returns true if the file has indeed changed, false if it has
        not. """
        assert self.hasContentsFile

        url = self.hostUrlPrefix + 'contents.xml'
        print "Redownloading %s" % (url)

        # Get the hash of the original file.
        filename = Filename(self.hostDir, 'contents.xml')
        hv1 = HashVal()
        hv1.hashFile(filename)

        # Now download it again.
        self.hasContentsFile = False
        if not self.downloadContentsFile(http):
            return False

        hv2 = HashVal()
        hv2.hashFile(filename)

        if hv1 != hv2:
            print "%s has changed." % (url)
            return True
        else:
            print "%s has not changed." % (url)
            return False


    def readContentsFile(self):
        """ Reads the contents.xml file for this particular host, once
        it has been downloaded.  Returns true on success, false if the
        contents file is not already on disk or is unreadable. """

        if self.hasContentsFile:
            # No need to read it again.
            return True

        filename = Filename(self.hostDir, 'contents.xml')

        doc = TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            return False
        
        xcontents = doc.FirstChildElement('contents')
        if not xcontents:
            return False

        # Look for our own entry in the hosts table.
        self.__findHostXml(xcontents)

        # Get the list of packages available for download and/or import.
        xpackage = xcontents.FirstChildElement('package')
        while xpackage:
            name = xpackage.Attribute('name')
            platform = xpackage.Attribute('platform')
            version = xpackage.Attribute('version')
            try:
                solo = int(xpackage.Attribute('solo') or '')
            except ValueError:
                solo = False
                
            package = self.__makePackage(name, platform, version, solo)
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

        return True

    def __findHostXml(self, xcontents):
        """ Looks for the <host> or <alt_host> entry in the
        contents.xml that corresponds to the URL that we actually
        downloaded from. """
        
        xhost = xcontents.FirstChildElement('host')
        while xhost:
            url = xhost.Attribute('url')
            if url == self.hostUrl:
                self.readHostXml(xhost)
                return

            xalthost = xhost.FirstChildElement('alt_host')
            while xalthost:
                url = xalthost.Attribute('url')
                if url == self.hostUrl:
                    self.readHostXml(xalthost)
                    return
                xalthost = xalthost.NextSiblingElement('alt_host')
            
            xhost = xhost.NextSiblingElement('host')

    def readHostXml(self, xhost):
        """ Reads a <host> or <alt_host> entry and applies the data to
        this object. """

        descriptiveName = xhost.Attribute('descriptive_name')
        if descriptiveName and not self.descriptiveName:
            self.descriptiveName = descriptiveName

        # Get the "download" URL, which is the source from which we
        # download everything other than the contents.xml file.
        downloadUrl = xhost.Attribute('download_url')
        if downloadUrl:
            self.downloadUrlPrefix = downloadUrl
            if self.downloadUrlPrefix[-1] != '/':
                self.downloadUrlPrefix += '/'
        else:
            self.downloadUrlPrefix = self.hostUrlPrefix
            
        xmirror = xhost.FirstChildElement('mirror')
        while xmirror:
            url = xmirror.Attribute('url')
            if url:
                if url[-1] != '/':
                    url += '/'
                if url not in self.mirrors:
                    self.mirrors.append(url)
            xmirror = xmirror.NextSiblingElement('mirror')

        xalthost = xhost.FirstChildElement('alt_host')
        while xalthost:
            keyword = xalthost.Attribute('keyword')
            url = xalthost.Attribute('url')
            if url and keyword:
                self.altHosts[keyword] = url
            xalthost = xalthost.NextSiblingElement('alt_host')

    def __makePackage(self, name, platform, version, solo):
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
            package = PackageInfo(self, name, version, platform = platform,
                                  solo = solo, asMirror = self.asMirror)
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

    def getPackages(self, name = None, platform = None):
        """ Returns a list of PackageInfo objects that match the
        indicated name and/or platform, with no particular regards to
        version.  If name is None, all packages are returned. """

        assert self.hasContentsFile

        packages = []
        for (pn, version), platforms in self.packages.items():
            if name and pn != name:
                continue

            package = self.getPackage(pn, version, platform = platform)
            if package:
                packages.append(package)

        return packages

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
