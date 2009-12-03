from pandac.PandaModules import Filename, URLSpec, DocumentSpec, Ramfile, Multifile, Decompressor, EUOk, EUSuccess, VirtualFileSystem, Thread, getModelPath, ExecutionEnvironment
from pandac import PandaModules
from direct.p3d.FileSpec import FileSpec
from direct.showbase import VFSImporter
import os
import sys
import random
import time

class PackageInfo:

    """ This class represents a downloadable Panda3D package file that
    can be (or has been) installed into the current runtime.  It is
    the Python equivalent of the P3DPackage class in the core API. """

    # Weight factors for computing download progress.  This
    # attempts to reflect the relative time-per-byte of each of
    # these operations.
    downloadFactor = 1
    uncompressFactor = 0.01
    unpackFactor = 0.01
    patchFactor = 0.01

    # These tokens are returned by __downloadFile() and other
    # InstallStep functions.
    stepComplete = 1
    stepFailed = 2
    restartDownload = 3

    class InstallStep:
        """ This class is one step of the installPlan list; it
        represents a single atomic piece of the installation step, and
        the relative effort of that piece.  When the plan is executed,
        it will call the saved function pointer here. """
        def __init__(self, func, bytes, factor):
            self.func = func
            self.bytesNeeded = bytes
            self.bytesDone = 0
            self.bytesFactor = factor

        def getEffort(self):
            """ Returns the relative amount of effort of this step. """
            return self.bytesNeeded * self.bytesFactor

        def getProgress(self):
            """ Returns the progress of this step, in the range
            0..1. """
            if self.bytesNeeded == 0:
                return 1
            return min(float(self.bytesDone) / float(self.bytesNeeded), 1)
    
    def __init__(self, host, packageName, packageVersion, platform = None,
                 solo = False, asMirror = False):
        self.host = host
        self.packageName = packageName
        self.packageVersion = packageVersion
        self.platform = platform
        self.solo = solo
        self.asMirror = asMirror

        # This will be active while we are in the middle of a download
        # cycle.
        self.http = None

        # This will be filled in when the host's contents.xml file is
        # read.
        self.packageDir = None
            
        # These will be filled in by HostInfo when the package is read
        # from contents.xml.
        self.descFile = None
        self.importDescFile = None

        # These are filled in when the desc file is successfully read.
        self.hasDescFile = False
        self.patchVersion = None
        self.displayName = None
        self.guiApp = False
        self.uncompressedArchive = None
        self.compressedArchive = None
        self.extracts = []
        self.requires = []
        self.installPlans = None
 
        # This is updated during downloadPackage().  It is in the
        # range 0..1.
        self.downloadProgress = 0
        
        # This is set true when the package file has been fully
        # downloaded and unpacked.
        self.hasPackage = False

        # This is set true when the package has been "installed",
        # meaning it's been added to the paths and all.
        self.installed = False

    def getPackageDir(self):
        """ Returns the directory in which this package is installed.
        This may not be known until the host's contents.xml file has
        been downloaded, which informs us of the host's own install
        directory. """
        
        if not self.packageDir:
            if not self.host.hasContentsFile:
                if not self.host.readContentsFile():
                    self.host.downloadContentsFile(self.http)
            
            # Derive the packageDir from the hostDir.
            self.packageDir = Filename(self.host.hostDir, self.packageName)
            if self.packageVersion:
                self.packageDir = Filename(self.packageDir, self.packageVersion)

            if self.asMirror:
                # The server directory contains the platform name, though
                # the client directory doesn't.
                if self.platform:
                    self.packageDir = Filename(self.packageDir, self.platform)

        return self.packageDir

    def getDownloadEffort(self):
        """ Returns the relative amount of effort it will take to
        download this package.  The units are meaningless, except
        relative to other packges."""

        if not self.installPlans:
            return 0

        # Return the size of plan A, assuming it will work.
        plan = self.installPlans[0]
        size = sum(map(lambda step: step.getEffort(), plan))
        
        return size

    def getFormattedName(self):
        """ Returns the name of this package, for output to the user.
        This will be the "public" name of the package, as formatted
        for user consumption; it will include capital letters and
        spaces where appropriate. """

        if self.displayName:
            name = self.displayName
        else:
            name = self.packageName
            if self.packageVersion:
                name += ' %s' % (self.packageVersion)

        if self.patchVersion:
            name += ' rev %s' % (self.patchVersion)

        return name
        

    def setupFilenames(self):
        """ This is called by the HostInfo when the package is read
        from contents.xml, to set up the internal filenames and such
        that rely on some of the information from contents.xml. """
        
        dirname, basename = self.descFile.filename.rsplit('/', 1)
        self.descFileDirname = dirname
        self.descFileBasename = basename

    def checkStatus(self):
        """ Checks the current status of the desc file and the package
        contents on disk. """

        if self.hasPackage:
            return True

        if not self.hasDescFile:
            filename = Filename(self.getPackageDir(), self.descFileBasename)
            if self.descFile.quickVerify(self.getPackageDir(), pathname = filename):
                if self.__readDescFile():
                    # Successfully read.  We don't need to call
                    # checkArchiveStatus again, since readDescFile()
                    # has just done it.
                    return self.hasPackage

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

        self.http = http

        token = self.__downloadFile(
            None, self.descFile,
            urlbase = self.descFile.filename,
            filename = self.descFileBasename)

        while token == self.restartDownload:
            # Try again.
            token = self.__downloadFile(
                None, self.descFile,
                urlbase = self.descFile.filename,
                filename = self.descFileBasename)

        if token == self.stepFailed:
            # Couldn't download the desc file.
            return False

        assert token == self.stepComplete

        filename = Filename(self.getPackageDir(), self.descFileBasename)
        # Now that we've written the desc file, make it read-only.
        os.chmod(filename.toOsSpecific(), 0444)

        if not self.__readDescFile():
            # Weird, it passed the hash check, but we still can't read
            # it.
            print "Failure reading %s" % (filename)
            return False

        return True

    def __readDescFile(self):
        """ Reads the desc xml file for this particular package,
        assuming it's been already downloaded and verified.  Returns
        true on success, false on failure. """

        if self.hasDescFile:
            # No need to read it again.
            return True

        if self.solo:
            # If this is a "solo" package, we don't actually "read"
            # the desc file; that's the entire contents of the
            # package.
            self.hasDescFile = True
            self.hasPackage = True
            return True

        filename = Filename(self.getPackageDir(), self.descFileBasename)

        if not hasattr(PandaModules, 'TiXmlDocument'):
            return False
        doc = PandaModules.TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            return False

        xpackage = doc.FirstChildElement('package')
        if not xpackage:
            return False

        try:
            self.patchVersion = int(xpackage.Attribute('patch_version') or '')
        except ValueError:
            self.patchVersion = None

        self.displayName = None
        xconfig = xpackage.FirstChildElement('config')
        if xconfig:
            # The name for display to an English-speaking user.
            self.displayName = xconfig.Attribute('display_name')

            # True if any apps that use this package must be GUI apps.
            guiApp = xconfig.Attribute('gui_app')
            if guiApp:
                self.guiApp = int(guiApp)

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
        self.extracts = []
        xextract = xpackage.FirstChildElement('extract')
        while xextract:
            file = FileSpec()
            file.loadXml(xextract)
            self.extracts.append(file)
            xextract = xextract.NextSiblingElement('extract')

        # The list of additional packages that must be installed for
        # this package to function properly.
        self.requires = []
        xrequires = xpackage.FirstChildElement('requires')
        while xrequires:
            packageName = xrequires.Attribute('name')
            version = xrequires.Attribute('version')
            hostUrl = xrequires.Attribute('host')
            if packageName and hostUrl:
                host = self.host.appRunner.getHostWithAlt(hostUrl)
                self.requires.append((packageName, version, host))
            xrequires = xrequires.NextSiblingElement('requires')

        self.hasDescFile = True

        # Now that we've read the desc file, go ahead and use it to
        # verify the download status.
        if self.__checkArchiveStatus():
            # It's all fully downloaded, unpacked, and ready.
            self.hasPackage = True
            return True

        # Still have to download it.
        self.__buildInstallPlans()
        return True

    def __buildInstallPlans(self):
        """ Sets up self.installPlans, a list of one or more "plans"
        to download and install the package. """

        self.hasPackage = False

        if self.asMirror:
            # If we're just downloading a mirror archive, we only need
            # to get the compressed archive file.

            # Build a one-item install plan to download the compressed
            # archive.
            downloadSize = self.compressedArchive.size
            func = lambda step, fileSpec = self.compressedArchive: self.__downloadFile(step, fileSpec, allowPartial = True)
            
            step = self.InstallStep(func, downloadSize, self.downloadFactor)
            installPlan = [step]
            self.installPlans = [installPlan]
            return 

        # The normal download process.  Determine what we will need to
        # download, and build a plan (or two) to download it all.
        self.installPlans = None

        # We know we will at least need to unpack the archive contents
        # at the end.
        unpackSize = 0
        for file in self.extracts:
            unpackSize += file.size
        step = self.InstallStep(self.__unpackArchive, unpackSize, self.unpackFactor)
        planA = [step]

        # If the uncompressed archive file is good, that's all we'll
        # need to do.
        self.uncompressedArchive.actualFile = None
        if self.uncompressedArchive.quickVerify(self.getPackageDir()):
            self.installPlans = [planA]
            return

        # Maybe the compressed archive file is good.
        if self.compressedArchive.quickVerify(self.getPackageDir()):
            uncompressSize = self.uncompressedArchive.size
            step = self.InstallStep(self.__uncompressArchive, uncompressSize, self.uncompressFactor)
            planA = [step] + planA
            self.installPlans = [planA]
            return

        # Maybe we can download one or more patches.  We'll come back
        # to that in a minute as plan A.  For now, construct plan B,
        # which will be to download the whole archive.
        planB = planA[:]

        uncompressSize = self.uncompressedArchive.size
        step = self.InstallStep(self.__uncompressArchive, uncompressSize, self.uncompressFactor)
        planB = [step] + planB

        downloadSize = self.compressedArchive.size
        func = lambda step, fileSpec = self.compressedArchive: self.__downloadFile(step, fileSpec, allowPartial = True)

        step = self.InstallStep(func, downloadSize, self.downloadFactor)
        planB = [step] + planB

        # Now look for patches.  Start with the md5 hash from the
        # uncompressedArchive file we have on disk, and see if we can
        # find a patch chain from this file to our target.
        pathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        fileSpec = self.uncompressedArchive.actualFile
        if fileSpec is None and pathname.exists():
            fileSpec = FileSpec()
            fileSpec.fromFile(self.getPackageDir(), self.uncompressedArchive.filename)
        plan = None
        if fileSpec:
            plan = self.__findPatchChain(fileSpec)
        if plan:
            # We can download patches.  Great!  That means this is
            # plan A, and the full download is plan B (in case
            # something goes wrong with the patching).
            planA = plan + planA
            self.installPlans = [planA, planB]
        else:
            # There are no patches to download, oh well.  Stick with
            # plan B as the only plan.
            self.installPlans = [planB]

    def __scanDirectoryRecursively(self, dirname):
        """ Generates a list of Filename objects: all of the files
        (not directories) within and below the indicated dirname. """
        
        contents = []
        for dirpath, dirnames, filenames in os.walk(dirname.toOsSpecific()):
            dirpath = Filename.fromOsSpecific(dirpath)
            if dirpath == dirname:
                dirpath = Filename('')
            else:
                dirpath.makeRelativeTo(dirname)
            for filename in filenames:
                contents.append(Filename(dirpath, filename))
        return contents

    def __removeFileFromList(self, contents, filename):
        """ Removes the indicated filename from the given list, if it is
        present.  """
        try:
            contents.remove(Filename(filename))
        except ValueError:
            pass

    def __checkArchiveStatus(self):
        """ Returns true if the archive and all extractable files are
        already correct on disk, false otherwise. """

        # Get a list of all of the files in the directory, so we can
        # remove files that don't belong.
        contents = self.__scanDirectoryRecursively(self.getPackageDir()) 
        self.__removeFileFromList(contents, self.descFileBasename)
        self.__removeFileFromList(contents, self.compressedArchive.filename)
        if not self.asMirror:
            self.__removeFileFromList(contents, self.uncompressedArchive.filename)
            for file in self.extracts:
                self.__removeFileFromList(contents, file.filename)

        # Now, any files that are still in the contents list don't
        # belong.  It's important to remove these files before we
        # start verifying the files that we expect to find here, in
        # case there is a problem with ambiguous filenames or
        # something (e.g. case insensitivity).
        for filename in contents:
            print "Removing %s" % (filename)
            pathname = Filename(self.getPackageDir(), filename)
            pathname.unlink()

        if self.asMirror:
            return self.compressedArchive.quickVerify(self.getPackageDir())
            
        allExtractsOk = True
        if not self.uncompressedArchive.quickVerify(self.getPackageDir()):
            #print "File is incorrect: %s" % (self.uncompressedArchive.filename)
            allExtractsOk = False

        if allExtractsOk:
            # OK, the uncompressed archive is good; that means there
            # shouldn't be a compressed archive file here.
            pathname = Filename(self.getPackageDir(), self.compressedArchive.filename)
            pathname.unlink()
            
            for file in self.extracts:
                if not file.quickVerify(self.getPackageDir()):
                    #print "File is incorrect: %s" % (file.filename)
                    allExtractsOk = False
                    break

##         if allExtractsOk:
##             print "All %s extracts of %s seem good." % (
##                 len(self.extracts), self.packageName)

        return allExtractsOk

    def __updateStepProgress(self, step):
        """ This callback is made from within the several step
        functions as the download step proceeds.  It updates
        self.downloadProgress with the current progress, so the caller
        can asynchronously query this value. """

        size = self.totalPlanCompleted + self.currentStepEffort * step.getProgress()
        self.downloadProgress = min(float(size) / float(self.totalPlanSize), 1)
    
    def downloadPackage(self, http):
        """ Downloads the package file, synchronously, then
        uncompresses and unpacks it.  Returns true on success, false
        on failure.

        This assumes that self.installPlans has already been filled
        in, which will have been done by self.__readDescFile().
        """

        assert self.hasDescFile

        if self.hasPackage:
            # We've already got one.
            return True

        # We should have an install plan by the time we get here.
        assert self.installPlans

        self.http = http
        token = self.__followInstallPlans()
        while token == self.restartDownload:
            # Try again.
            if not self.downloadDescFile(http):
                return False

            token = self.__followInstallPlans()

        if token == self.stepFailed:
            return False

        assert token == self.stepComplete
        return True
            

    def __followInstallPlans(self):
        """ Performs all of the steps in self.installPlans.  Returns
        one of stepComplete, stepFailed, or restartDownload. """

        if not self.installPlans:
            self.__buildInstallPlans()

        installPlans = self.installPlans
        self.installPlans = None
        for plan in installPlans:
            self.totalPlanSize = sum(map(lambda step: step.getEffort(), plan))
            self.totalPlanCompleted = 0
            self.downloadProgress = 0

            planFailed = False
            for step in plan:
                self.currentStepEffort = step.getEffort()

                token = step.func(step)
                if token == self.restartDownload:
                    return token
                if token == self.stepFailed:
                    planFailed = True
                    break
                assert token == self.stepComplete
                
                self.totalPlanCompleted += self.currentStepEffort
                
            if not planFailed:
                # Successfully downloaded!
                return self.stepComplete

        # All plans failed.
        return self.stepFailed

    def __findPatchChain(self, fileSpec):
        """ Finds the chain of patches that leads from the indicated
        patch version to the current patch version.  If found,
        constructs an installPlan that represents the steps of the
        patch installation; otherwise, returns None. """

        from direct.p3d.PatchMaker import PatchMaker

        patchMaker = PatchMaker(self.getPackageDir())
        patchChain = patchMaker.getPatchChainToCurrent(self.descFileBasename, fileSpec)
        if patchChain is None:
            # No path.
            patchMaker.cleanup()
            return None

        plan = []
        for patchfile in patchChain:
            downloadSize = patchfile.file.size
            func = lambda step, fileSpec = patchfile.file: self.__downloadFile(step, fileSpec, allowPartial = True)
            step = self.InstallStep(func, downloadSize, self.downloadFactor)
            plan.append(step)

            patchSize = patchfile.targetFile.size
            func = lambda step, patchfile = patchfile: self.__applyPatch(step, patchfile)
            step = self.InstallStep(func, patchSize, self.patchFactor)
            plan.append(step)

        patchMaker.cleanup()
        return plan

    def __downloadFile(self, step, fileSpec, urlbase = None, filename = None,
                       allowPartial = False):
        """ Downloads the indicated file from the host into
        packageDir.  Returns one of stepComplete, stepFailed, or
        restartDownload. """

        if not urlbase:
            urlbase = self.descFileDirname + '/' + fileSpec.filename

        # Build up a list of URL's to try downloading from.  Unlike
        # the C++ implementation in P3DPackage.cxx, here we build the
        # URL's in forward order.
        tryUrls = []

        if self.host.appRunner and self.host.appRunner.superMirrorUrl:
            # We start with the "super mirror", if it's defined.
            url = self.host.appRunner.superMirrorUrl + urlbase
            tryUrls.append((url, False))

        if self.host.mirrors:
            # Choose two mirrors at random.
            mirrors = self.host.mirrors[:]
            for i in range(2):
                mirror = random.choice(mirrors)
                mirrors.remove(mirror)
                url = mirror + urlbase
                tryUrls.append((url, False))
                if not mirrors:
                    break

        # After trying two mirrors and failing (or if there are no
        # mirrors), go get it from the original host.
        url = self.host.downloadUrlPrefix + urlbase
        tryUrls.append((url, False))

        # And finally, if the original host also fails, try again with
        # a cache-buster.
        tryUrls.append((url, True))

        for url, cacheBust in tryUrls:
            request = DocumentSpec(url)

            if cacheBust:
                # On the last attempt to download a particular file,
                # we bust through the cache: append a query string to
                # do this.
                url += '?' + str(int(time.time()))
                request = DocumentSpec(url)
                request.setCacheControl(DocumentSpec.CCNoCache)
             
            print "%s downloading %s" % (self.packageName, url)

            if not filename:
                filename = fileSpec.filename
            targetPathname = Filename(self.getPackageDir(), filename)
            targetPathname.setBinary()

            channel = self.http.makeChannel(False)

            # If there's a previous partial download, attempt to resume it.
            bytesStarted = 0
            if allowPartial and not cacheBust and targetPathname.exists():
                bytesStarted = targetPathname.getFileSize()

            if bytesStarted < 1024*1024:
                # Not enough bytes downloaded to be worth the risk of
                # a partial download.
                bytesStarted = 0
            elif bytesStarted >= fileSpec.size:
                # Couldn't possibly be our file.
                bytesStarted = 0

            if bytesStarted:
                print "Resuming %s after %s bytes already downloaded" % (url, bytesStarted)
                # Make sure the file is writable.
                os.chmod(targetPathname.toOsSpecific(), 0644)
                channel.beginGetSubdocument(request, bytesStarted, 0)
            else:
                # No partial download possible; get the whole file.
                targetPathname.makeDir()
                targetPathname.unlink()
                channel.beginGetDocument(request)
                
            channel.downloadToFile(targetPathname)
            while channel.run():
                if step:
                    step.bytesDone = channel.getBytesDownloaded() + channel.getFirstByteDelivered()
                    if step.bytesDone > step.bytesNeeded:
                        # Oops, too much data.  Might as well abort;
                        # it's the wrong file.
                        break
                    
                    self.__updateStepProgress(step)
                Thread.considerYield()
                
            if step:
                step.bytesDone = channel.getBytesDownloaded() + channel.getFirstByteDelivered()
                self.__updateStepProgress(step)

            if not channel.isValid():
                print "Failed to download %s" % (url)

            elif not fileSpec.fullVerify(self.getPackageDir(), pathname = targetPathname):
                print "After downloading, %s incorrect" % (Filename(fileSpec.filename).getBasename())
            else:
                # Success!
                return self.stepComplete

            # This attempt failed.  Maybe the original contents.xml
            # file is stale.  Try re-downloading it now, just to be
            # sure.
            if self.host.redownloadContentsFile(self.http):
                # Yes!  Go back and start over from the beginning.
                return self.restartDownload

            # Well, that wasn't the problem.  Maybe the mirror is bad.
            # Go back and try the next mirror.

        # All mirrors failed; the server (or the internet connection)
        # must be just fubar.
        return self.stepFailed

    def __applyPatch(self, step, patchfile):
        """ Applies the indicated patching in-place to the current
        uncompressed archive.  The patchfile is removed after the
        operation.  Returns one of stepComplete, stepFailed, or
        restartDownload. """

        origPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        patchPathname = Filename(self.getPackageDir(), patchfile.file.filename)
        result = Filename.temporary('', 'patch_')
        print "Patching %s with %s" % (origPathname, patchPathname)

        p = PandaModules.Patchfile()  # The C++ class

        ret = p.initiate(patchPathname, origPathname, result)
        if ret == EUSuccess:
            ret = p.run()
        while ret == EUOk:
            step.bytesDone = step.bytesNeeded * p.getProgress()
            self.__updateStepProgress(step)
            Thread.considerYield()
            ret = p.run()
        del p
        patchPathname.unlink()
        
        if ret < 0:
            print "Patching failed."
            result.unlink()
            return self.stepFailed

        if not result.renameTo(origPathname):
            print "Couldn't rename %s to %s" % (result, origPathname)
            return self.stepFailed
            
        return self.stepComplete

    def __uncompressArchive(self, step):
        """ Turns the compressed archive into the uncompressed
        archive.  Returns one of stepComplete, stepFailed, or
        restartDownload. """

        sourcePathname = Filename(self.getPackageDir(), self.compressedArchive.filename)
        targetPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        targetPathname.unlink()
        print "Uncompressing %s to %s" % (sourcePathname, targetPathname)
        decompressor = Decompressor()
        decompressor.initiate(sourcePathname, targetPathname)
        totalBytes = self.uncompressedArchive.size
        result = decompressor.run()
        while result == EUOk:
            step.bytesDone = int(totalBytes * decompressor.getProgress())
            self.__updateStepProgress(step)
            result = decompressor.run()
            Thread.considerYield()

        if result != EUSuccess:
            return self.stepFailed
            
        step.bytesDone = totalBytes
        self.__updateStepProgress(step)

        if not self.uncompressedArchive.quickVerify(self.getPackageDir()):
            print "after uncompressing, %s still incorrect" % (
                self.uncompressedArchive.filename)
            return self.stepFailed

        # Now that we've verified the archive, make it read-only.
        os.chmod(targetPathname.toOsSpecific(), 0444)

        # Now we can safely remove the compressed archive.
        sourcePathname.unlink()
        return self.stepComplete
    
    def __unpackArchive(self, step):
        """ Unpacks any files in the archive that want to be unpacked
        to disk.  Returns one of stepComplete, stepFailed, or
        restartDownload. """

        if not self.extracts:
            # Nothing to extract.
            self.hasPackage = True
            return self.stepComplete

        mfPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        print "Unpacking %s" % (mfPathname)
        mf = Multifile()
        if not mf.openRead(mfPathname):
            print "Couldn't open %s" % (mfPathname)
            return self.stepFailed
        
        allExtractsOk = True
        step.bytesDone = 0
        for file in self.extracts:
            i = mf.findSubfile(file.filename)
            if i == -1:
                print "Not in Multifile: %s" % (file.filename)
                allExtractsOk = False
                continue

            targetPathname = Filename(self.getPackageDir(), file.filename)
            targetPathname.unlink()
            if not mf.extractSubfile(i, targetPathname):
                print "Couldn't extract: %s" % (file.filename)
                allExtractsOk = False
                continue
            
            if not file.quickVerify(self.getPackageDir()):
                print "After extracting, still incorrect: %s" % (file.filename)
                allExtractsOk = False
                continue

            # Make sure it's executable, and not writable.
            os.chmod(targetPathname.toOsSpecific(), 0555)

            step.bytesDone += file.size
            self.__updateStepProgress(step)
            Thread.considerYield()

        if not allExtractsOk:
            return self.stepFailed

        self.hasPackage = True
        return self.stepComplete

    def installPackage(self, appRunner):
        """ Mounts the package and sets up system paths so it becomes
        available for use.  Returns true on success, false on failure. """

        assert self.hasPackage
        if self.installed:
            # Already installed.
            return True
        assert self not in appRunner.installedPackages

        mfPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        mf = Multifile()
        if not mf.openRead(mfPathname):
            print "Couldn't open %s" % (mfPathname)
            return False

        # We mount it under its actual location on disk.
        root = self.getPackageDir().cStr()

        vfs = VirtualFileSystem.getGlobalPtr()
        vfs.mount(mf, root, vfs.MFReadOnly)

        # Add this to the Python search path, if it's not already
        # there.  We have to take a bit of care to check if it's
        # already there, since there can be some ambiguity in
        # os-specific path strings.
        osRoot = self.getPackageDir().toOsSpecific()
        foundOnPath = False
        for p in sys.path:
            if osRoot == p:
                # Already here, exactly.
                foundOnPath = True
                break
            elif osRoot == Filename.fromOsSpecific(p).toOsSpecific():
                # Already here, with some futzing.
                foundOnPath = True
                break

        if not foundOnPath:
            # Not already here; add it.
            sys.path.append(osRoot)

        # Put it on the model-path, too.  We do this indiscriminantly,
        # because the Panda3D runtime won't be adding things to the
        # model-path, so it shouldn't be already there.
        getModelPath().appendDirectory(self.getPackageDir())

        # Set the environment variable to reference the package root.
        envvar = '%s_ROOT' % (self.packageName.upper())
        ExecutionEnvironment.setEnvironmentVariable(envvar, osRoot)

        # Now that the environment variable is set, read all of the
        # prc files in the package.
        appRunner.loadMultifilePrcFiles(mf, self.getPackageDir())

        # Also, find any toplevel Python packages, and add these as
        # shared packages.  This will allow different packages
        # installed in different directories to share Python files as
        # if they were all in the same directory.
        for filename in mf.getSubfileNames():
            if filename.endswith('/__init__.pyc') or \
               filename.endswith('/__init__.pyo') or \
               filename.endswith('/__init__.py'):
                components = filename.split('/')[:-1]
                moduleName = '.'.join(components)
                VFSImporter.sharedPackages[moduleName] = True

        # Fix up any shared directories so we can load packages from
        # disparate locations.
        VFSImporter.reloadSharedPackages()

        self.installed = True
        appRunner.installedPackages.append(self)

        return True
