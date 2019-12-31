"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["PackageInfo"]

from panda3d.core import Filename, DocumentSpec, Multifile, Decompressor, EUOk, EUSuccess, VirtualFileSystem, Thread, getModelPath, ExecutionEnvironment, PStatCollector, TiXmlDocument, TiXmlDeclaration, TiXmlElement
import panda3d.core as core
from direct.p3d.FileSpec import FileSpec
from direct.p3d.ScanDirectoryNode import ScanDirectoryNode
from direct.showbase import VFSImporter
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.task.TaskManagerGlobal import taskMgr
import os
import sys
import random
import time
import copy

class PackageInfo:

    """ This class represents a downloadable Panda3D package file that
    can be (or has been) installed into the current runtime.  It is
    the Python equivalent of the P3DPackage class in the core API. """

    notify = directNotify.newCategory("PackageInfo")

    # Weight factors for computing download progress.  This
    # attempts to reflect the relative time-per-byte of each of
    # these operations.
    downloadFactor = 1
    uncompressFactor = 0.01
    unpackFactor = 0.01
    patchFactor = 0.01

    # These tokens are yielded (not returned) by __downloadFile() and
    # other InstallStep functions.
    stepComplete = 1
    stepFailed = 2
    restartDownload = 3
    stepContinue = 4

    UsageBasename = 'usage.xml'

    class InstallStep:
        """ This class is one step of the installPlan list; it
        represents a single atomic piece of the installation step, and
        the relative effort of that piece.  When the plan is executed,
        it will call the saved function pointer here. """
        def __init__(self, func, bytes, factor, stepType):
            self.__funcPtr = func
            self.bytesNeeded = bytes
            self.bytesDone = 0
            self.bytesFactor = factor
            self.stepType = stepType
            self.pStatCol = PStatCollector(':App:PackageInstaller:%s' % (stepType))

        def func(self):
            """ self.__funcPtr(self) will return a generator of
            tokens.  This function defines a new generator that yields
            each of those tokens, but wraps each call into the nested
            generator within a pair of start/stop collector calls. """

            self.pStatCol.start()
            for token in self.__funcPtr(self):
                self.pStatCol.stop()
                yield token
                self.pStatCol.start()

            # Shouldn't ever get here.
            self.pStatCol.stop()
            raise StopIteration

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
                 solo = False, asMirror = False, perPlatform = False):
        self.host = host
        self.packageName = packageName
        self.packageVersion = packageVersion
        self.platform = platform
        self.solo = solo
        self.asMirror = asMirror
        self.perPlatform = perPlatform

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

        # This is set true when the package has been updated in this
        # session, but not yet written to usage.xml.
        self.updated = False
        self.diskSpace = None

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

            if self.host.perPlatform:
                # If we're running on a special host that wants us to
                # include the platform, we include it.
                includePlatform = True
            elif self.perPlatform and self.host.appRunner.respectPerPlatform:
                # Otherwise, if our package spec wants us to include
                # the platform (and our plugin knows about this), then
                # we also include it.
                includePlatform = True
            else:
                # Otherwise, we must be running legacy code
                # somewhere--either an old package or an old
                # plugin--and we therefore shouldn't include the
                # platform in the directory hierarchy.
                includePlatform = False

            if includePlatform and self.platform:
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
        size = sum([step.getEffort() for step in plan])

        return size

    def getPrevDownloadedEffort(self):
        """ Returns a rough estimate of this package's total download
        effort, even if it is already downloaded. """

        effort = 0
        if self.compressedArchive:
            effort += self.compressedArchive.size * self.downloadFactor
        if self.uncompressedArchive:
            effort += self.uncompressedArchive.size * self.uncompressFactor
        # Don't bother counting unpacking.

        return effort

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
            if self.descFile.quickVerify(self.getPackageDir(), pathname = filename, notify = self.notify):
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

    def hasCurrentDescFile(self):
        """ Returns true if a desc file file has been successfully
        read for this package and is still current, false
        otherwise. """

        if not self.host.hasCurrentContentsFile():
            return False

        return self.hasDescFile

    def downloadDescFile(self, http):
        """ Downloads the desc file for this particular package,
        synchronously, and then reads it.  Returns true on success,
        false on failure. """

        for token in self.downloadDescFileGenerator(http):
            if token != self.stepContinue:
                break
            Thread.considerYield()

        return (token == self.stepComplete)

    def downloadDescFileGenerator(self, http):
        """ A generator function that implements downloadDescFile()
        one piece at a time.  It yields one of stepComplete,
        stepFailed, or stepContinue. """

        assert self.descFile

        if self.hasDescFile:
            # We've already got one.
            yield self.stepComplete; return

        if not self.host.appRunner or self.host.appRunner.verifyContents != self.host.appRunner.P3DVCNever:
            # We're allowed to download it.
            self.http = http

            func = lambda step, self = self: self.__downloadFile(
                None, self.descFile,
                urlbase = self.descFile.filename,
                filename = self.descFileBasename)
            step = self.InstallStep(func, self.descFile.size, self.downloadFactor, 'downloadDesc')

            for token in step.func():
                if token == self.stepContinue:
                    yield token
                else:
                    break

            while token == self.restartDownload:
                # Try again.
                func = lambda step, self = self: self.__downloadFile(
                    None, self.descFile,
                    urlbase = self.descFile.filename,
                    filename = self.descFileBasename)
                step = self.InstallStep(func, self.descFile.size, self.downloadFactor, 'downloadDesc')
                for token in step.func():
                    if token == self.stepContinue:
                        yield token
                    else:
                        break

            if token == self.stepFailed:
                # Couldn't download the desc file.
                yield self.stepFailed; return

            assert token == self.stepComplete

            filename = Filename(self.getPackageDir(), self.descFileBasename)
            # Now that we've written the desc file, make it read-only.
            os.chmod(filename.toOsSpecific(), 0o444)

        if not self.__readDescFile():
            # Weird, it passed the hash check, but we still can't read
            # it.
            filename = Filename(self.getPackageDir(), self.descFileBasename)
            self.notify.warning("Failure reading %s" % (filename))
            yield self.stepFailed; return

        yield self.stepComplete; return

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

        if not hasattr(core, 'TiXmlDocument'):
            return False
        doc = core.TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            return False

        xpackage = doc.FirstChildElement('package')
        if not xpackage:
            return False

        try:
            self.patchVersion = int(xpackage.Attribute('patch_version') or '')
        except ValueError:
            self.patchVersion = None

        try:
            perPlatform = int(xpackage.Attribute('per_platform') or '')
        except ValueError:
            perPlatform = False
        if perPlatform != self.perPlatform:
            self.notify.warning("per_platform disagreement on package %s" % (self.packageName))

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

        pc = PStatCollector(':App:PackageInstaller:buildInstallPlans')
        pc.start()

        self.hasPackage = False

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # We're not allowed to download anything.
            self.installPlans = []
            pc.stop()
            return

        if self.asMirror:
            # If we're just downloading a mirror archive, we only need
            # to get the compressed archive file.

            # Build a one-item install plan to download the compressed
            # archive.
            downloadSize = self.compressedArchive.size
            func = lambda step, fileSpec = self.compressedArchive: self.__downloadFile(step, fileSpec, allowPartial = True)

            step = self.InstallStep(func, downloadSize, self.downloadFactor, 'download')
            installPlan = [step]
            self.installPlans = [installPlan]
            pc.stop()
            return

        # The normal download process.  Determine what we will need to
        # download, and build a plan (or two) to download it all.
        self.installPlans = None

        # We know we will at least need to unpack the archive contents
        # at the end.
        unpackSize = 0
        for file in self.extracts:
            unpackSize += file.size
        step = self.InstallStep(self.__unpackArchive, unpackSize, self.unpackFactor, 'unpack')
        planA = [step]

        # If the uncompressed archive file is good, that's all we'll
        # need to do.
        self.uncompressedArchive.actualFile = None
        if self.uncompressedArchive.quickVerify(self.getPackageDir(), notify = self.notify):
            self.installPlans = [planA]
            pc.stop()
            return

        # Maybe the compressed archive file is good.
        if self.compressedArchive.quickVerify(self.getPackageDir(), notify = self.notify):
            uncompressSize = self.uncompressedArchive.size
            step = self.InstallStep(self.__uncompressArchive, uncompressSize, self.uncompressFactor, 'uncompress')
            planA = [step] + planA
            self.installPlans = [planA]
            pc.stop()
            return

        # Maybe we can download one or more patches.  We'll come back
        # to that in a minute as plan A.  For now, construct plan B,
        # which will be to download the whole archive.
        planB = planA[:]

        uncompressSize = self.uncompressedArchive.size
        step = self.InstallStep(self.__uncompressArchive, uncompressSize, self.uncompressFactor, 'uncompress')
        planB = [step] + planB

        downloadSize = self.compressedArchive.size
        func = lambda step, fileSpec = self.compressedArchive: self.__downloadFile(step, fileSpec, allowPartial = True)

        step = self.InstallStep(func, downloadSize, self.downloadFactor, 'download')
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

        # In case of unexpected failures on the internet, we will retry
        # the full download instead of just giving up.
        retries = core.ConfigVariableInt('package-full-dl-retries', 1).getValue()
        for retry in range(retries):
            self.installPlans.append(planB[:])

        pc.stop()

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

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # Assume that everything is just fine.
            return True

        # Get a list of all of the files in the directory, so we can
        # remove files that don't belong.
        contents = self.__scanDirectoryRecursively(self.getPackageDir())
        self.__removeFileFromList(contents, self.descFileBasename)
        self.__removeFileFromList(contents, self.compressedArchive.filename)
        self.__removeFileFromList(contents, self.UsageBasename)
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
            self.notify.info("Removing %s" % (filename))
            pathname = Filename(self.getPackageDir(), filename)
            pathname.unlink()
            self.updated = True

        if self.asMirror:
            return self.compressedArchive.quickVerify(self.getPackageDir(), notify = self.notify)

        allExtractsOk = True
        if not self.uncompressedArchive.quickVerify(self.getPackageDir(), notify = self.notify):
            self.notify.debug("File is incorrect: %s" % (self.uncompressedArchive.filename))
            allExtractsOk = False

        if allExtractsOk:
            # OK, the uncompressed archive is good; that means there
            # shouldn't be a compressed archive file here.
            pathname = Filename(self.getPackageDir(), self.compressedArchive.filename)
            pathname.unlink()

            for file in self.extracts:
                if not file.quickVerify(self.getPackageDir(), notify = self.notify):
                    self.notify.debug("File is incorrect: %s" % (file.filename))
                    allExtractsOk = False
                    break

        if allExtractsOk:
            self.notify.debug("All %s extracts of %s seem good." % (
                len(self.extracts), self.packageName))

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

        for token in self.downloadPackageGenerator(http):
            if token != self.stepContinue:
                break
            Thread.considerYield()

        return (token == self.stepComplete)

    def downloadPackageGenerator(self, http):
        """ A generator function that implements downloadPackage() one
        piece at a time.  It yields one of stepComplete, stepFailed,
        or stepContinue. """

        assert self.hasDescFile

        if self.hasPackage:
            # We've already got one.
            yield self.stepComplete; return

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # We're not allowed to download anything. Assume it's already downloaded.
            yield self.stepComplete; return

        # We should have an install plan by the time we get here.
        assert self.installPlans

        self.http = http
        for token in self.__followInstallPlans():
            if token == self.stepContinue:
                yield token
            else:
                break

        while token == self.restartDownload:
            # Try again.
            for token in self.downloadDescFileGenerator(http):
                if token == self.stepContinue:
                    yield token
                else:
                    break
            if token == self.stepComplete:
                for token in self.__followInstallPlans():
                    if token == self.stepContinue:
                        yield token
                    else:
                        break

        if token == self.stepFailed:
            yield self.stepFailed; return

        assert token == self.stepComplete
        yield self.stepComplete; return


    def __followInstallPlans(self):
        """ Performs all of the steps in self.installPlans.  Yields
        one of stepComplete, stepFailed, restartDownload, or
        stepContinue. """

        if not self.installPlans:
            self.__buildInstallPlans()

        installPlans = self.installPlans
        self.installPlans = None
        for plan in installPlans:
            self.totalPlanSize = sum([step.getEffort() for step in plan])
            self.totalPlanCompleted = 0
            self.downloadProgress = 0

            planFailed = False
            for step in plan:
                self.currentStepEffort = step.getEffort()

                for token in step.func():
                    if token == self.stepContinue:
                        yield token
                    else:
                        break

                if token == self.restartDownload:
                    yield token
                if token == self.stepFailed:
                    planFailed = True
                    break
                assert token == self.stepComplete

                self.totalPlanCompleted += self.currentStepEffort

            if not planFailed:
                # Successfully downloaded!
                yield self.stepComplete; return

            if taskMgr.destroyed:
                yield self.stepFailed; return

        # All plans failed.
        yield self.stepFailed; return

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
            step = self.InstallStep(func, downloadSize, self.downloadFactor, 'download')
            plan.append(step)

            patchSize = patchfile.targetFile.size
            func = lambda step, patchfile = patchfile: self.__applyPatch(step, patchfile)
            step = self.InstallStep(func, patchSize, self.patchFactor, 'patch')
            plan.append(step)

        patchMaker.cleanup()
        return plan

    def __downloadFile(self, step, fileSpec, urlbase = None, filename = None,
                       allowPartial = False):
        """ Downloads the indicated file from the host into
        packageDir.  Yields one of stepComplete, stepFailed,
        restartDownload, or stepContinue. """

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # We're not allowed to download anything.
            yield self.stepFailed; return

        self.updated = True

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

            self.notify.info("%s downloading %s" % (self.packageName, url))

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
                self.notify.info("Resuming %s after %s bytes already downloaded" % (url, bytesStarted))
                # Make sure the file is writable.
                os.chmod(targetPathname.toOsSpecific(), 0o644)
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
                        self.notify.warning("Got more data than expected for download %s" % (url))
                        break

                    self.__updateStepProgress(step)

                if taskMgr.destroyed:
                    # If the task manager has been destroyed, we must
                    # be shutting down.  Get out of here.
                    self.notify.warning("Task Manager destroyed, aborting %s" % (url))
                    yield self.stepFailed; return

                yield self.stepContinue

            if step:
                step.bytesDone = channel.getBytesDownloaded() + channel.getFirstByteDelivered()
                self.__updateStepProgress(step)

            if not channel.isValid():
                self.notify.warning("Failed to download %s" % (url))

            elif not fileSpec.fullVerify(self.getPackageDir(), pathname = targetPathname, notify = self.notify):
                self.notify.warning("After downloading, %s incorrect" % (Filename(fileSpec.filename).getBasename()))

                # This attempt failed.  Maybe the original contents.xml
                # file is stale.  Try re-downloading it now, just to be
                # sure.
                if self.host.redownloadContentsFile(self.http):
                    # Yes!  Go back and start over from the beginning.
                    yield self.restartDownload; return

            else:
                # Success!
                yield self.stepComplete; return

            # Maybe the mirror is bad.  Go back and try the next
            # mirror.

        # All attempts failed.  Maybe the original contents.xml file
        # is stale.  Try re-downloading it now, just to be sure.
        if self.host.redownloadContentsFile(self.http):
            # Yes!  Go back and start over from the beginning.
            yield self.restartDownload; return

        # All mirrors failed; the server (or the internet connection)
        # must be just fubar.
        yield self.stepFailed; return

    def __applyPatch(self, step, patchfile):
        """ Applies the indicated patching in-place to the current
        uncompressed archive.  The patchfile is removed after the
        operation.  Yields one of stepComplete, stepFailed,
        restartDownload, or stepContinue. """

        self.updated = True

        origPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        patchPathname = Filename(self.getPackageDir(), patchfile.file.filename)
        result = Filename.temporary('', 'patch_')
        self.notify.info("Patching %s with %s" % (origPathname, patchPathname))

        p = core.Patchfile()  # The C++ class

        ret = p.initiate(patchPathname, origPathname, result)
        if ret == EUSuccess:
            ret = p.run()
        while ret == EUOk:
            step.bytesDone = step.bytesNeeded * p.getProgress()
            self.__updateStepProgress(step)
            if taskMgr.destroyed:
                # If the task manager has been destroyed, we must
                # be shutting down.  Get out of here.
                self.notify.warning("Task Manager destroyed, aborting patch %s" % (origPathname))
                yield self.stepFailed; return

            yield self.stepContinue
            ret = p.run()
        del p
        patchPathname.unlink()

        if ret < 0:
            self.notify.warning("Patching of %s failed." % (origPathname))
            result.unlink()
            yield self.stepFailed; return

        if not result.renameTo(origPathname):
            self.notify.warning("Couldn't rename %s to %s" % (result, origPathname))
            yield self.stepFailed; return

        yield self.stepComplete; return

    def __uncompressArchive(self, step):
        """ Turns the compressed archive into the uncompressed
        archive.  Yields one of stepComplete, stepFailed,
        restartDownload, or stepContinue. """

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # We're not allowed to!
            yield self.stepFailed; return

        self.updated = True

        sourcePathname = Filename(self.getPackageDir(), self.compressedArchive.filename)
        targetPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        targetPathname.unlink()
        self.notify.info("Uncompressing %s to %s" % (sourcePathname, targetPathname))
        decompressor = Decompressor()
        decompressor.initiate(sourcePathname, targetPathname)
        totalBytes = self.uncompressedArchive.size
        result = decompressor.run()
        while result == EUOk:
            step.bytesDone = int(totalBytes * decompressor.getProgress())
            self.__updateStepProgress(step)
            result = decompressor.run()
            if taskMgr.destroyed:
                # If the task manager has been destroyed, we must
                # be shutting down.  Get out of here.
                self.notify.warning("Task Manager destroyed, aborting decompresss %s" % (sourcePathname))
                yield self.stepFailed; return

            yield self.stepContinue

        if result != EUSuccess:
            yield self.stepFailed; return

        step.bytesDone = totalBytes
        self.__updateStepProgress(step)

        if not self.uncompressedArchive.quickVerify(self.getPackageDir(), notify= self.notify):
            self.notify.warning("after uncompressing, %s still incorrect" % (
                self.uncompressedArchive.filename))
            yield self.stepFailed; return

        # Now that we've verified the archive, make it read-only.
        os.chmod(targetPathname.toOsSpecific(), 0o444)

        # Now we can safely remove the compressed archive.
        sourcePathname.unlink()
        yield self.stepComplete; return

    def __unpackArchive(self, step):
        """ Unpacks any files in the archive that want to be unpacked
        to disk.  Yields one of stepComplete, stepFailed,
        restartDownload, or stepContinue. """

        if not self.extracts:
            # Nothing to extract.
            self.hasPackage = True
            yield self.stepComplete; return

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # We're not allowed to!
            yield self.stepFailed; return

        self.updated = True

        mfPathname = Filename(self.getPackageDir(), self.uncompressedArchive.filename)
        self.notify.info("Unpacking %s" % (mfPathname))
        mf = Multifile()
        if not mf.openRead(mfPathname):
            self.notify.warning("Couldn't open %s" % (mfPathname))
            yield self.stepFailed; return

        allExtractsOk = True
        step.bytesDone = 0
        for file in self.extracts:
            i = mf.findSubfile(file.filename)
            if i == -1:
                self.notify.warning("Not in Multifile: %s" % (file.filename))
                allExtractsOk = False
                continue

            targetPathname = Filename(self.getPackageDir(), file.filename)
            targetPathname.setBinary()
            targetPathname.unlink()
            if not mf.extractSubfile(i, targetPathname):
                self.notify.warning("Couldn't extract: %s" % (file.filename))
                allExtractsOk = False
                continue

            if not file.quickVerify(self.getPackageDir(), notify = self.notify):
                self.notify.warning("After extracting, still incorrect: %s" % (file.filename))
                allExtractsOk = False
                continue

            # Make sure it's executable, and not writable.
            os.chmod(targetPathname.toOsSpecific(), 0o555)

            step.bytesDone += file.size
            self.__updateStepProgress(step)
            if taskMgr.destroyed:
                # If the task manager has been destroyed, we must
                # be shutting down.  Get out of here.
                self.notify.warning("Task Manager destroyed, aborting unpacking %s" % (mfPathname))
                yield self.stepFailed; return

            yield self.stepContinue

        if not allExtractsOk:
            yield self.stepFailed; return

        self.hasPackage = True
        yield self.stepComplete; return

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
            self.notify.warning("Couldn't open %s" % (mfPathname))
            return False

        # We mount it under its actual location on disk.
        root = self.getPackageDir()

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

        # Add the package root to the system paths.
        if sys.platform.startswith('win'):
            path = os.environ.get('PATH', '')
            os.environ['PATH'] = "%s;%s" % (osRoot, path)
        else:
            path = os.environ.get('PATH', '')
            os.environ['PATH'] = "%s:%s" % (osRoot, path)
            path = os.environ.get('LD_LIBRARY_PATH', '')
            os.environ['LD_LIBRARY_PATH'] = "%s:%s" % (osRoot, path)

        if sys.platform == "darwin":
            path = os.environ.get('DYLD_LIBRARY_PATH', '')
            os.environ['DYLD_LIBRARY_PATH'] = "%s:%s" % (osRoot, path)

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

        self.markUsed()

        return True

    def __measureDiskSpace(self):
        """ Returns the amount of space used by this package, in
        bytes, as determined by examining the actual contents of the
        package directory and its subdirectories. """

        thisDir = ScanDirectoryNode(self.getPackageDir(), ignoreUsageXml = True)
        diskSpace = thisDir.getTotalSize()
        self.notify.info("Package %s uses %s MB" % (
            self.packageName, (diskSpace + 524288) // 1048576))
        return diskSpace

    def markUsed(self):
        """ Marks the package as having been used.  This is normally
        called automatically by installPackage(). """

        if not hasattr(core, 'TiXmlDocument'):
            return

        if self.host.appRunner and self.host.appRunner.verifyContents == self.host.appRunner.P3DVCNever:
            # Not allowed to write any files to the package directory.
            return

        if self.updated:
            # If we've just installed a new version of the package,
            # re-measure the actual disk space used.
            self.diskSpace = self.__measureDiskSpace()

        filename = Filename(self.getPackageDir(), self.UsageBasename)
        doc = TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            decl = TiXmlDeclaration("1.0", "utf-8", "")
            doc.InsertEndChild(decl)

        xusage = doc.FirstChildElement('usage')
        if not xusage:
            doc.InsertEndChild(TiXmlElement('usage'))
            xusage = doc.FirstChildElement('usage')

        now = int(time.time())

        count = xusage.Attribute('count_app')
        try:
            count = int(count or '')
        except ValueError:
            count = 0
            xusage.SetAttribute('first_use', str(now))
        count += 1
        xusage.SetAttribute('count_app', str(count))

        xusage.SetAttribute('last_use', str(now))

        if self.updated:
            xusage.SetAttribute('last_update', str(now))
            self.updated = False
        else:
            # Since we haven't changed the disk space, we can just
            # read it from the previous xml file.
            diskSpace = xusage.Attribute('disk_space')
            try:
                diskSpace = int(diskSpace or '')
            except ValueError:
                # Unless it wasn't set already.
                self.diskSpace = self.__measureDiskSpace()

        xusage.SetAttribute('disk_space', str(self.diskSpace))

        # Write the file to a temporary filename, then atomically move
        # it to its actual filename, to avoid race conditions when
        # updating this file.
        tfile = Filename.temporary(str(self.getPackageDir()), '.xml')
        if doc.SaveFile(tfile.toOsSpecific()):
            tfile.renameTo(filename)

    def getUsage(self):
        """ Returns the xusage element that is read from the usage.xml
        file, or None if there is no usage.xml file. """

        if not hasattr(core, 'TiXmlDocument'):
            return None

        filename = Filename(self.getPackageDir(), self.UsageBasename)
        doc = TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            return None

        xusage = doc.FirstChildElement('usage')
        if not xusage:
            return None

        return copy.copy(xusage)

