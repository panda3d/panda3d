from pandac.PandaModules import Filename, URLSpec, DocumentSpec, Ramfile, TiXmlDocument, Multifile, Decompressor, EUOk, EUSuccess, VirtualFileSystem, Thread
from direct.p3d.FileSpec import FileSpec
import os
import sys

class PackageInfo:

    """ This class represents a downloadable Panda3D package file that
    can be (or has been) installed into the current runtime.  It is
    the Python equivalent of the P3DPackage class in the core API. """

    def __init__(self, host, packageName, packageVersion, platform = None):
        self.host = host
        self.packageName = packageName
        self.packageVersion = packageVersion
        self.platform = platform

        self.packageDir = Filename(host.hostDir, '%s/%s' % (self.packageName, self.packageVersion))

        # These will be filled in by HostInfo when the package is read
        # from contents.xml.
        self.descFileUrl = None
        self.descFile = None
        self.importDescFile = None

        # These are filled in when the desc file is successfully read.
        self.hasDescFile = False
        self.displayName = None
        self.uncompressedArchive = None
        self.compressedArchive = None
        self.extracts = []

        # These are incremented during downloadPackage().
        self.bytesDownloaded = 0
        self.bytesUncompressed = 0
        self.bytesUnpacked = 0
        
        # This is set true when the package file has been fully
        # downloaded and unpackaged.
        self.hasPackage = False

    def getDownloadSize(self):
        """ Returns the number of bytes we will need to download in
        order to install this package. """
        if self.hasPackage:
            return 0
        return self.compressedArchive.size

    def getUncompressSize(self):
        """ Returns the number of bytes we will need to uncompress in
        order to install this package. """
        if self.hasPackage:
            return 0
        return self.uncompressedArchive.size

    def getUnpackSize(self):
        """ Returns the number of bytes that we will need to unpack
        when installing the package. """

        if self.hasPackage:
            return 0

        size = 0
        for file in self.extracts:
            size += file.size
        return size

    def setupFilenames(self):
        """ This is called by the HostInfo when the package is read
        from contents.xml, to set up the internal filenames and such
        that rely on some of the information from contents.xml. """
        
        self.descFileUrl = self.host.hostUrlPrefix + self.descFile.filename

        basename = self.descFile.filename.rsplit('/', 1)[-1]
        self.descFileBasename = basename

    def checkStatus(self):
        """ Checks the current status of the desc file and the package
        contents on disk. """

        if self.hasPackage:
            return True

        if not self.hasDescFile:
            filename = Filename(self.packageDir, self.descFileBasename)
            if self.descFile.quickVerify(self.packageDir, pathname = filename):
                self.readDescFile()

        if self.hasDescFile:
            if self.__checkArchiveStatus():
                # It's all good.
                self.hasPackage = True

        return self.hasPackage

    def downloadDescFile(self, http):
        """ Downloads the desc file for this particular package,
        synchronously, and then reads it.  Returns true on success,
        false on failure. """

        assert self.descFile

        if self.hasDescFile:
            # We've already got one.
            return True

        url = URLSpec(self.descFileUrl)
        print "Downloading %s" % (url)

        rf = Ramfile()
        channel = http.getDocument(url)
        if not channel.downloadToRam(rf):
            print "Unable to download %s" % (url)
            return False

        filename = Filename(self.packageDir, self.descFileBasename)
        filename.makeDir()
        f = open(filename.toOsSpecific(), 'wb')
        f.write(rf.getData())
        f.close()

        try:
            self.readDescFile()
        except ValueError:
            print "Failure reading %s" % (filename)
            return False

        return True

    def readDescFile(self):
        """ Reads the desc xml file for this particular package.
        Presumably this has already been downloaded and installed. """

        if self.hasDescFile:
            # No need to read it again.
            return

        filename = Filename(self.packageDir, self.descFileBasename)

        doc = TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            raise ValueError

        xpackage = doc.FirstChildElement('package')
        if not xpackage:
            raise ValueError

        # The name for display to an English-speaking user.
        self.displayName = xpackage.Attribute('display_name')

        # The uncompressed archive, which will be mounted directly,
        # and also used for patching.
        xuncompressedArchive = xpackage.FirstChildElement('uncompressed_archive')
        if xuncompressedArchive:
            self.uncompressedArchive = FileSpec()
            self.uncompressedArchive.loadXml(xuncompressedArchive)

        # The compressed archive, which is what is downloaded.
        xcompressedArchive = xpackage.FirstChildElement('compressed_archive')
        if xcompressedArchive:
            self.compressedArchive = FileSpec()
            self.compressedArchive.loadXml(xcompressedArchive)

        # The list of files that should be extracted to disk.
        xextract = xpackage.FirstChildElement('extract')
        while xextract:
            file = FileSpec()
            file.loadXml(xextract)
            self.extracts.append(file)
            xextract = xextract.NextSiblingElement('extract')

        self.hasDescFile = True

        # Now that we've read the desc file, go ahead and use it to
        # verify the download status.
        if self.__checkArchiveStatus():
            # It's all good.
            self.hasPackage = True
            return

        # We need to download an update.
        self.hasPackage = False

    def __checkArchiveStatus(self):
        """ Returns true if the archive and all extractable files are
        already correct on disk, false otherwise. """
        
        allExtractsOk = True
        if not self.uncompressedArchive.quickVerify(self.packageDir):
            #print "File is incorrect: %s" % (self.uncompressedArchive.filename)
            allExtractsOk = False

        if allExtractsOk:
            for file in self.extracts:
                if not file.quickVerify(self.packageDir):
                    #print "File is incorrect: %s" % (file.filename)
                    allExtractsOk = False
                    break

        return allExtractsOk

    def downloadPackage(self, http):
        """ Downloads the package file, synchronously, then
        uncompresses and unpacks it.  Returns true on success, false
        on failure. """

        assert self.hasDescFile

        if self.hasPackage:
            # We've already got one.
            return True

        if self.uncompressedArchive.quickVerify(self.packageDir):
            return self.__unpackArchive()

        if self.compressedArchive.quickVerify(self.packageDir):
            return self.__uncompressArchive()

        url = self.descFileUrl.rsplit('/', 1)[0]
        url += '/' + self.compressedArchive.filename
        url = DocumentSpec(url)
        print "Downloading %s" % (url)

        targetPathname = Filename(self.packageDir, self.compressedArchive.filename)
        targetPathname.setBinary()
        
        channel = http.makeChannel(False)
        channel.beginGetDocument(url)
        channel.downloadToFile(targetPathname)
        while channel.run():
            self.bytesDownloaded = channel.getBytesDownloaded()
            Thread.considerYield()
        self.bytesDownloaded = channel.getBytesDownloaded()
        if not channel.isValid():
            print "Failed to download %s" % (url)
            return False

        if not self.compressedArchive.fullVerify(self.packageDir):
            print "after downloading, %s still incorrect" % (
                self.compressedArchive.filename)
            return False
        
        return self.__uncompressArchive()

    def __uncompressArchive(self):
        """ Turns the compressed archive into the uncompressed
        archive, then unpacks it.  Returns true on success, false on
        failure. """

        sourcePathname = Filename(self.packageDir, self.compressedArchive.filename)
        targetPathname = Filename(self.packageDir, self.uncompressedArchive.filename)

        decompressor = Decompressor()
        decompressor.initiate(sourcePathname, targetPathname)
        totalBytes = self.uncompressedArchive.size
        result = decompressor.run()
        while result == EUOk:
            self.bytesUncompressed = int(totalBytes * decompressor.getProgress())
            result = decompressor.run()
            Thread.considerYield()

        if result != EUSuccess:
            return False
            
        self.bytesUncompressed = totalBytes

        if not self.uncompressedArchive.quickVerify(self.packageDir):
            print "after uncompressing, %s still incorrect" % (
                self.uncompressedArchive.filename)
            return False

        return self.__unpackArchive()
    
    def __unpackArchive(self):
        """ Unpacks any files in the archive that want to be unpacked
        to disk. """

        if not self.extracts:
            # Nothing to extract.
            self.hasPackage = True
            return True

        mfPathname = Filename(self.packageDir, self.uncompressedArchive.filename)
        mf = Multifile()
        if not mf.openRead(mfPathname):
            print "Couldn't open %s" % (mfPathname)
            return False
        
        allExtractsOk = True
        self.bytesUnpacked = 0
        for file in self.extracts:
            i = mf.findSubfile(file.filename)
            if i == -1:
                print "Not in Multifile: %s" % (file.filename)
                allExtractsOk = False
                continue

            targetPathname = Filename(self.packageDir, file.filename)
            if not mf.extractSubfile(i, targetPathname):
                print "Couldn't extract: %s" % (file.filename)
                allExtractsOk = False
                continue
            
            if not file.quickVerify(self.packageDir):
                print "After extracting, still incorrect: %s" % (file.filename)
                allExtractsOk = False
                continue

            # Make sure it's executable.
            os.chmod(targetPathname.toOsSpecific(), 0755)

            self.bytesUnpacked += file.size
            Thread.considerYield()

        if not allExtractsOk:
            return False

        self.hasPackage = True
        return True

    def installPackage(self, appRunner):
        """ Mounts the package and sets up system paths so it becomes
        available for use. """

        assert self.hasPackage
        
        mfPathname = Filename(self.packageDir, self.uncompressedArchive.filename)
        mf = Multifile()
        if not mf.openRead(mfPathname):
            print "Couldn't open %s" % (mfPathname)
            return False

        # We mount it under its actual location on disk.
        root = self.packageDir.cStr()

        vfs = VirtualFileSystem.getGlobalPtr()
        vfs.mount(mf, root, vfs.MFReadOnly)

        appRunner.loadMultifilePrcFiles(mf, root)

        if root not in sys.path:
            sys.path.append(root)
        #print "Installed %s %s" % (self.packageName, self.packageVersion)
        
        
