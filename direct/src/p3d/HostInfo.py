"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["HostInfo"]

from panda3d.core import HashVal, Filename, PandaSystem, DocumentSpec, Ramfile
from panda3d.core import HTTPChannel, ConfigVariableInt
from panda3d import core
from direct.p3d.PackageInfo import PackageInfo
from direct.p3d.FileSpec import FileSpec
from direct.directnotify.DirectNotifyGlobal import directNotify
import time

class HostInfo:
    """ This class represents a particular download host serving up
    Panda3D packages.  It is the Python equivalent of the P3DHost
    class in the core API. """

    notify = directNotify.newCategory("HostInfo")

    def __init__(self, hostUrl, appRunner = None, hostDir = None,
                 rootDir = None, asMirror = False, perPlatform = None):

        """ You must specify either an appRunner or a hostDir to the
        HostInfo constructor.

        If you pass asMirror = True, it means that this HostInfo
        object is to be used to populate a "mirror" folder, a
        duplicate (or subset) of the contents hosted by a server.
        This means when you use this HostInfo to download packages, it
        will only download the compressed archive file and leave it
        there.  At the moment, mirror folders do not download old
        patch files from the server.

        If you pass perPlatform = True, then files are unpacked into a
        platform-specific directory, which is appropriate when you
        might be downloading multiple platforms.  The default is
        perPlatform = False, which means all files are unpacked into
        the host directory directly, without an intervening
        platform-specific directory name.  If asMirror is True, then
        the default is perPlatform = True.

        Note that perPlatform is also restricted by the individual
        package's specification.  """

        self.__setHostUrl(hostUrl)
        self.appRunner = appRunner
        self.rootDir = rootDir
        if rootDir is None and appRunner:
            self.rootDir = appRunner.rootDir

        if hostDir and not isinstance(hostDir, Filename):
            hostDir = Filename.fromOsSpecific(hostDir)

        self.hostDir = hostDir
        self.asMirror = asMirror
        self.perPlatform = perPlatform
        if perPlatform is None:
            self.perPlatform = asMirror

        # Initially false, this is set true when the contents file is
        # successfully read.
        self.hasContentsFile = False

        # This is the time value at which the current contents file is
        # no longer valid.
        self.contentsExpiration = 0

        # Contains the md5 hash of the original contents.xml file.
        self.contentsSpec = FileSpec()

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

        if self.appRunner and self.appRunner.verifyContents != self.appRunner.P3DVCForce:
            # Attempt to pre-read the existing contents.xml; maybe it
            # will be current enough for our purposes.
            self.readContentsFile()

    def __setHostUrl(self, hostUrl):
        """ Assigns self.hostUrl, and related values. """
        self.hostUrl = hostUrl

        if not self.hostUrl:
            # A special case: the URL will be set later.
            self.hostUrlPrefix = None
            self.downloadUrlPrefix = None
        else:
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

    def freshenFile(self, http, fileSpec, localPathname):
        """ Ensures that the localPathname is the most current version
        of the file defined by fileSpec, as offered by host.  If not,
        it downloads a new version on-the-spot.  Returns true on
        success, false on failure. """

        if fileSpec.quickVerify(pathname = localPathname):
            # It's good, keep it.
            return True

        # It's stale, get a new one.
        doc = None
        if self.appRunner and self.appRunner.superMirrorUrl:
            # Use the "super mirror" first.
            url = core.URLSpec(self.appRunner.superMirrorUrl + fileSpec.filename)
            self.notify.info("Freshening %s" % (url))
            doc = http.getDocument(url)

        if not doc or not doc.isValid():
            # Failing the super mirror, contact the actual host.
            url = core.URLSpec(self.hostUrlPrefix + fileSpec.filename)
            self.notify.info("Freshening %s" % (url))
            doc = http.getDocument(url)
            if not doc.isValid():
                return False

        file = Filename.temporary('', 'p3d_')
        if not doc.downloadToFile(file):
            # Failed to download.
            file.unlink()
            return False

        # Successfully downloaded!
        localPathname.makeDir()
        if not file.renameTo(localPathname):
            # Couldn't move it into place.
            file.unlink()
            return False

        if not fileSpec.fullVerify(pathname = localPathname, notify = self.notify):
            # No good after download.
            self.notify.info("%s is still no good after downloading." % (url))
            return False

        return True

    def downloadContentsFile(self, http, redownload = False,
                             hashVal = None):
        """ Downloads the contents.xml file for this particular host,
        synchronously, and then reads it.  Returns true on success,
        false on failure.  If hashVal is not None, it should be a
        HashVal object, which will be filled with the hash from the
        new contents.xml file."""

        if self.hasCurrentContentsFile():
            # We've already got one.
            return True

        if self.appRunner and self.appRunner.verifyContents == self.appRunner.P3DVCNever:
            # Not allowed to.
            return False

        rf = None
        if http:
            if not redownload and self.appRunner and self.appRunner.superMirrorUrl:
                # We start with the "super mirror", if it's defined.
                url = self.appRunner.superMirrorUrl + 'contents.xml'
                request = DocumentSpec(url)
                self.notify.info("Downloading contents file %s" % (request))

                rf = Ramfile()
                channel = http.makeChannel(False)
                channel.getDocument(request)
                if not channel.downloadToRam(rf):
                    self.notify.warning("Unable to download %s" % (url))
                    rf = None

            if not rf:
                # Then go to the main host, if our super mirror let us
                # down.

                url = self.hostUrlPrefix + 'contents.xml'
                # Append a uniquifying query string to the URL to force the
                # download to go all the way through any caches.  We use the
                # time in seconds; that's unique enough.
                url += '?' + str(int(time.time()))

                # We might as well explicitly request the cache to be disabled
                # too, since we have an interface for that via HTTPChannel.
                request = DocumentSpec(url)
                request.setCacheControl(DocumentSpec.CCNoCache)

                self.notify.info("Downloading contents file %s" % (request))
                statusCode = None
                statusString = ''
                for attempt in range(int(ConfigVariableInt('contents-xml-dl-attempts', 3))):
                    if attempt > 0:
                        self.notify.info("Retrying (%s)..."%(attempt,))
                    rf = Ramfile()
                    channel = http.makeChannel(False)
                    channel.getDocument(request)
                    if channel.downloadToRam(rf):
                        self.notify.info("Successfully downloaded %s" % (url,))
                        break
                    else:
                        rf = None
                        statusCode = channel.getStatusCode()
                        statusString = channel.getStatusString()
                        self.notify.warning("Could not contact download server at %s" % (url,))
                        self.notify.warning("Status code = %s %s" % (statusCode, statusString))

                if not rf:
                    self.notify.warning("Unable to download %s" % (url,))
                    try:
                        # Something screwed up.
                        if statusCode == HTTPChannel.SCDownloadOpenError or \
                           statusCode == HTTPChannel.SCDownloadWriteError:
                            launcher.setPandaErrorCode(2)
                        elif statusCode == 404:
                            # 404 not found
                            launcher.setPandaErrorCode(5)
                        elif statusCode < 100:
                            # statusCode < 100 implies the connection attempt itself
                            # failed.  This is usually due to firewall software
                            # interfering.  Apparently some firewall software might
                            # allow the first connection and disallow subsequent
                            # connections; how strange.
                            launcher.setPandaErrorCode(4)
                        else:
                            # There are other kinds of failures, but these will
                            # generally have been caught already by the first test; so
                            # if we get here there may be some bigger problem.  Just
                            # give the generic "big problem" message.
                            launcher.setPandaErrorCode(6)
                    except NameError as e:
                        # no launcher
                        pass
                    except AttributeError as e:
                        self.notify.warning("%s" % (str(e),))
                        pass
                    return False

        tempFilename = Filename.temporary('', 'p3d_', '.xml')
        if rf:
            f = open(tempFilename.toOsSpecific(), 'wb')
            f.write(rf.getData())
            f.close()
            if hashVal:
                hashVal.hashString(rf.getData())

            if not self.readContentsFile(tempFilename, freshDownload = True):
                self.notify.warning("Failure reading %s" % (url))
                tempFilename.unlink()
                return False

            tempFilename.unlink()
            return True

        # Couldn't download the file.  Maybe we should look for a
        # previously-downloaded copy already on disk?
        return False

    def redownloadContentsFile(self, http):
        """ Downloads a new contents.xml file in case it has changed.
        Returns true if the file has indeed changed, false if it has
        not. """
        assert self.hasContentsFile

        if self.appRunner and self.appRunner.verifyContents == self.appRunner.P3DVCNever:
            # Not allowed to.
            return False

        url = self.hostUrlPrefix + 'contents.xml'
        self.notify.info("Redownloading %s" % (url))

        # Get the hash of the original file.
        assert self.hostDir
        hv1 = HashVal()
        if self.contentsSpec.hash:
            hv1.setFromHex(self.contentsSpec.hash)
        else:
            filename = Filename(self.hostDir, 'contents.xml')
            hv1.hashFile(filename)

        # Now download it again.
        self.hasContentsFile = False
        hv2 = HashVal()
        if not self.downloadContentsFile(http, redownload = True,
                                         hashVal = hv2):
            return False

        if hv2 == HashVal():
            self.notify.info("%s didn't actually redownload." % (url))
            return False
        elif hv1 != hv2:
            self.notify.info("%s has changed." % (url))
            return True
        else:
            self.notify.info("%s has not changed." % (url))
            return False

    def hasCurrentContentsFile(self):
        """ Returns true if a contents.xml file has been successfully
        read for this host and is still current, false otherwise. """
        if not self.appRunner \
            or self.appRunner.verifyContents == self.appRunner.P3DVCNone \
            or self.appRunner.verifyContents == self.appRunner.P3DVCNever:
            # If we're not asking to verify contents, then
            # contents.xml files never expires.
            return self.hasContentsFile

        now = int(time.time())
        return now < self.contentsExpiration and self.hasContentsFile

    def readContentsFile(self, tempFilename = None, freshDownload = False):
        """ Reads the contents.xml file for this particular host, once
        it has been downloaded into the indicated temporary file.
        Returns true on success, false if the contents file is not
        already on disk or is unreadable.

        If tempFilename is specified, it is the filename read, and it
        is copied the file into the standard location if it's not
        there already.  If tempFilename is not specified, the standard
        filename is read if it is known. """

        if not hasattr(core, 'TiXmlDocument'):
            return False

        if not tempFilename:
            if self.hostDir:
                # If the filename is not specified, we can infer it
                # if we already know our hostDir
                hostDir = self.hostDir
            else:
                # Otherwise, we have to guess the hostDir.
                hostDir = self.__determineHostDir(None, self.hostUrl)

            tempFilename = Filename(hostDir, 'contents.xml')

        doc = core.TiXmlDocument(tempFilename.toOsSpecific())
        if not doc.LoadFile():
            return False

        xcontents = doc.FirstChildElement('contents')
        if not xcontents:
            return False

        maxAge = xcontents.Attribute('max_age')
        if maxAge:
            try:
                maxAge = int(maxAge)
            except:
                maxAge = None
        if maxAge is None:
            # Default max_age if unspecified (see p3d_plugin.h).
            from direct.p3d.AppRunner import AppRunner
            maxAge = AppRunner.P3D_CONTENTS_DEFAULT_MAX_AGE

        # Get the latest possible expiration time, based on the max_age
        # indication.  Any expiration time later than this is in error.
        now = int(time.time())
        self.contentsExpiration = now + maxAge

        if freshDownload:
            self.contentsSpec.readHash(tempFilename)

            # Update the XML with the new download information.
            xorig = xcontents.FirstChildElement('orig')
            while xorig:
                xcontents.RemoveChild(xorig)
                xorig = xcontents.FirstChildElement('orig')

            xorig = core.TiXmlElement('orig')
            self.contentsSpec.storeXml(xorig)
            xorig.SetAttribute('expiration', str(self.contentsExpiration))

            xcontents.InsertEndChild(xorig)

        else:
            # Read the download hash and expiration time from the XML.
            expiration = None
            xorig = xcontents.FirstChildElement('orig')
            if xorig:
                self.contentsSpec.loadXml(xorig)
                expiration = xorig.Attribute('expiration')
                if expiration:
                    try:
                        expiration = int(expiration)
                    except:
                        expiration = None
            if not self.contentsSpec.hash:
                self.contentsSpec.readHash(tempFilename)

            if expiration is not None:
                self.contentsExpiration = min(self.contentsExpiration, expiration)

        # Look for our own entry in the hosts table.
        if self.hostUrl:
            self.__findHostXml(xcontents)
        else:
            assert self.hostDir
            self.__findHostXmlForHostDir(xcontents)

        if self.rootDir and not self.hostDir:
            self.hostDir = self.__determineHostDir(None, self.hostUrl)

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
            try:
                perPlatform = int(xpackage.Attribute('per_platform') or '')
            except ValueError:
                perPlatform = False

            package = self.__makePackage(name, platform, version, solo, perPlatform)
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

        # Now save the contents.xml file into the standard location.
        if self.appRunner and self.appRunner.verifyContents != self.appRunner.P3DVCNever:
            assert self.hostDir
            filename = Filename(self.hostDir, 'contents.xml')
            filename.makeDir()
            if freshDownload:
                doc.SaveFile(filename.toOsSpecific())
            else:
                if filename != tempFilename:
                    tempFilename.copyTo(filename)

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

    def __findHostXmlForHostDir(self, xcontents):
        """ Looks for the <host> or <alt_host> entry in the
        contents.xml that corresponds to the host dir that we read the
        contents.xml from.  This is used when reading a contents.xml
        file found on disk, as opposed to downloading it from a
        site. """

        xhost = xcontents.FirstChildElement('host')
        while xhost:
            url = xhost.Attribute('url')
            hostDirBasename = xhost.Attribute('host_dir')
            hostDir = self.__determineHostDir(hostDirBasename, url)
            if hostDir == self.hostDir:
                self.__setHostUrl(url)
                self.readHostXml(xhost)
                return

            xalthost = xhost.FirstChildElement('alt_host')
            while xalthost:
                url = xalthost.Attribute('url')
                hostDirBasename = xalthost.Attribute('host_dir')
                hostDir = self.__determineHostDir(hostDirBasename, url)
                if hostDir == self.hostDir:
                    self.__setHostUrl(url)
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

        hostDirBasename = xhost.Attribute('host_dir')
        if self.rootDir and not self.hostDir:
            self.hostDir = self.__determineHostDir(hostDirBasename, self.hostUrl)

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

    def __makePackage(self, name, platform, version, solo, perPlatform):
        """ Creates a new PackageInfo entry for the given name,
        version, and platform.  If there is already a matching
        PackageInfo, returns it. """

        if not platform:
            platform = None

        platforms = self.packages.setdefault((name, version or ""), {})
        package = platforms.get("", None)
        if not package:
            package = PackageInfo(self, name, version, platform = platform,
                                  solo = solo, asMirror = self.asMirror,
                                  perPlatform = perPlatform)
            platforms[platform or ""] = package

        return package

    def getPackage(self, name, version, platform = None):
        """ Returns a PackageInfo that matches the indicated name and
        version and the indicated platform or the current runtime
        platform, if one is provided by this host, or None if not. """

        assert self.hasContentsFile
        platforms = self.packages.get((name, version or ""), {})

        if platform:
            # In this case, we are looking for a specific platform
            # only.
            return platforms.get(platform, None)

        # We are looking for one matching the current runtime
        # platform.  First, look for a package matching the current
        # platform exactly.
        package = platforms.get(PandaSystem.getPlatform(), None)

        # If not found, look for one matching no particular platform.
        if not package:
            package = platforms.get("", None)

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

            if not platform:
                for p2 in platforms:
                    package = self.getPackage(pn, version, platform = p2)
                    if package:
                        packages.append(package)
            else:
                package = self.getPackage(pn, version, platform = platform)
                if package:
                    packages.append(package)

        return packages

    def getAllPackages(self, includeAllPlatforms = False):
        """ Returns a list of all available packages provided by this
        host. """

        result = []

        items = sorted(self.packages.items())
        for key, platforms in items:
            if self.perPlatform or includeAllPlatforms:
                # If we maintain a different answer per platform,
                # return all of them.
                pitems = sorted(platforms.items())
                for pkey, package in pitems:
                    result.append(package)
            else:
                # If we maintain a host for the current platform
                # only (e.g. a client copy), then return only the
                # current platform, or no particular platform.
                package = platforms.get(PandaSystem.getPlatform(), None)
                if not package:
                    package = platforms.get("", None)

                if package:
                    result.append(package)

        return result

    def deletePackages(self, packages):
        """ Removes all of the indicated packages from the disk,
        uninstalling them and deleting all of their files.  The
        packages parameter must be a list of one or more PackageInfo
        objects, for instance as returned by getPackage().  Returns
        the list of packages that were NOT found. """

        packages = packages[:]

        for key, platforms in list(self.packages.items()):
            for platform, package in list(platforms.items()):
                if package in packages:
                    self.__deletePackageFiles(package)
                    del platforms[platform]
                    packages.remove(package)

            if not platforms:
                # If we've removed all the platforms for a given
                # package, remove the key from the toplevel map.
                del self.packages[key]

        return packages

    def __deletePackageFiles(self, package):
        """ Called by deletePackage(), this actually removes the files
        for the indicated package. """

        if self.appRunner:
            self.notify.info("Deleting package %s: %s" % (package.packageName, package.getPackageDir()))
            self.appRunner.rmtree(package.getPackageDir())

            self.appRunner.sendRequest('forget_package', self.hostUrl, package.packageName, package.packageVersion or '')

    def __determineHostDir(self, hostDirBasename, hostUrl):
        """ Hashes the host URL into a (mostly) unique directory
        string, which will be the root of the host's install tree.
        Returns the resulting path, as a Filename.

        This code is duplicated in C++, in
        P3DHost::determine_host_dir(). """

        if hostDirBasename:
            # If the contents.xml specified a host_dir parameter, use
            # it.
            hostDir = str(self.rootDir) + '/hosts'
            for component in hostDirBasename.split('/'):
                if component:
                    if component[0] == '.':
                        # Forbid ".foo" or "..".
                        component = 'x' + component
                    hostDir += '/'
                    hostDir += component
            return Filename(hostDir)

        hostDir = 'hosts/'

        # Look for a server name in the URL.  Including this string in the
        # directory name makes it friendlier for people browsing the
        # directory.

        # We could use URLSpec, but we do it by hand instead, to make
        # it more likely that our hash code will exactly match the
        # similar logic in P3DHost.
        p = hostUrl.find('://')
        hostname = ''
        if p != -1:
            start = p + 3
            end = hostUrl.find('/', start)
            # Now start .. end is something like "username@host:port".

            at = hostUrl.find('@', start)
            if at != -1 and at < end:
                start = at + 1

            colon = hostUrl.find(':', start)
            if colon != -1 and colon < end:
                end = colon

            # Now start .. end is just the hostname.
            hostname = hostUrl[start : end]

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
            keepHash = keepHash // 2

        md = HashVal()
        md.hashString(hostUrl)
        hostDir += md.asHex()[:keepHash * 2]

        hostDir = Filename(self.rootDir, hostDir)
        return hostDir
