from pandac.PandaModules import Filename, URLSpec, DocumentSpec, Ramfile, TiXmlDocument, Multifile, Decompressor, EUOk, EUSuccess, VirtualFileSystem, Thread, getModelPath, Patchfile, ExecutionEnvironment
from direct.p3d.FileSpec import FileSpec
from direct.showbase import VFSImporter
import os
import sys

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
    
    def __init__(self, host, packageName, packageVersion, platform = None):
        self.host = host
        self.packageName = packageName
        self.packageVersion = packageVersion
        self.platform = platform

        self.packageDir = Filename(host.hostDir, self.packageName)
        if self.packageVersion:
            self.packageDir = Filename(self.packageDir, self.packageVersion)

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
        self.installPlans = None
 
        # This is updated during downloadPackage().  It is in the
        # range 0..1.
        self.downloadProgress = 0
        
        # This is set true when the package file has been fully
        # downloaded and unpackaged.
        self.hasPackage = False

        # This is set true when the package has been "installed",
        # meaning it's been added to the paths and all.
        self.installed = False

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

        self.displayName = None
        
        xconfig = xpackage.FirstChildElement('config')
        if xconfig:
            # The name for display to an English-speaking user.
            self.displayName = xconfig.Attribute('display_name')

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

        # Now determine what we will need to download, and build a
        # plan (or two) to download it all.
        self.installPlans = None

        # We know we will at least need to unpackage the archive at
        # the end.
        unpackSize = 0
        for file in self.extracts:
            unpackSize += file.size
        step = self.InstallStep(self.__unpackArchive, unpackSize, self.unpackFactor)
        planA = [step]

        # If the uncompressed archive file is good, that's all we'll
        # need to do.
        self.uncompressedArchive.actualFile = None
        if self.uncompressedArchive.quickVerify(self.packageDir):
            self.installPlans = [planA]
            return

        # Maybe the compressed archive file is good.
        if self.compressedArchive.quickVerify(self.packageDir):
            uncompressSize = self.uncompressedArchive.size
            step = self.InstallStep(self.__uncompressArchive, uncompressSize, self.uncompressFactor)
            planA = [step] + planA
            self.installPlans = [planA]
            return

        # Maybe we can download one or more patches.  We'll come back
        # to that in a minute as plan A.  For now, construct on plan
        # B, which will be to download the whole archive.
        planB = planA[:]

        uncompressSize = self.uncompressedArchive.size
        step = self.InstallStep(self.__uncompressArchive, uncompressSize, self.uncompressFactor)
        planB = [step] + planB

        downloadSize = self.compressedArchive.size
        func = lambda step, fileSpec = self.compressedArchive: self.__downloadFile(step, fileSpec)

        step = self.InstallStep(func, downloadSize, self.downloadFactor)
        planB = [step] + planB

        # Now look for patches.  Start with the md5 hash from the
        # uncompressedArchive file we have on disk, and see if we can
        # find a patch chain from this file to our target.
        pathname = Filename(self.packageDir, self.uncompressedArchive.filename)
        fileSpec = self.uncompressedArchive.actualFile
        if fileSpec is None and pathname.exists():
            fileSpec = FileSpec()
            fileSpec.fromFile(self.packageDir, self.uncompressedArchive.filename)
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
        on failure. """

        assert self.hasDescFile

        if self.hasPackage:
            # We've already got one.
            return True

        # We should have an install plan by the time we get here.
        assert self.installPlans

        self.http = http
        for plan in self.installPlans:
            self.totalPlanSize = sum(map(lambda step: step.getEffort(), plan))
            self.totalPlanCompleted = 0
            self.downloadProgress = 0

            planFailed = False
            for step in plan:
                self.currentStepEffort = step.getEffort()

                if not step.func(step):
                    planFailed = True
                    break
                self.totalPlanCompleted += self.currentStepEffort
                
            if not planFailed:
                # Successfully downloaded!
                return True

        # All plans failed.
        return False

    def __findPatchChain(self, fileSpec):
        """ Finds the chain of patches that leads from the indicated
        patch version to the current patch version.  If found,
        constructs an installPlan that represents the steps of the
        patch installation; otherwise, returns None. """

        from direct.p3d.PatchMaker import PatchMaker

        patchMaker = PatchMaker(self.packageDir)
        package = patchMaker.readPackageDescFile(self.descFileBasename)
        patchMaker.buildPatchChains()
        fromPv = patchMaker.getPackageVersion(package.getGenericKey(fileSpec))
        toPv = package.currentPv

        patchChain = None
        if toPv and fromPv:
            patchChain = toPv.getPatchChain(fromPv)

        if patchChain is None:
            # No path.
            patchMaker.cleanup()
            return None

        plan = []
        for patchfile in patchChain:
            downloadSize = patchfile.file.size
            func = lambda step, fileSpec = patchfile.file: self.__downloadFile(step, fileSpec)
            step = self.InstallStep(func, downloadSize, self.downloadFactor)
            plan.append(step)

            patchSize = patchfile.targetFile.size
            func = lambda step, patchfile = patchfile: self.__applyPatch(step, patchfile)
            step = self.InstallStep(func, patchSize, self.patchFactor)
            plan.append(step)

        patchMaker.cleanup()
        return plan

    def __downloadFile(self, step, fileSpec):
        """ Downloads the indicated file from the host into
        packageDir.  Returns true on success, false on failure. """
        
        url = self.descFileUrl.rsplit('/', 1)[0]
        url += '/' + fileSpec.filename
        url = DocumentSpec(url)
        print "Downloading %s" % (url)

        targetPathname = Filename(self.packageDir, fileSpec.filename)
        targetPathname.setBinary()
        
        channel = self.http.makeChannel(False)
        # TODO: check for a previous partial download, and resume it.
        targetPathname.unlink()
        channel.beginGetDocument(url)
        channel.downloadToFile(targetPathname)
        while channel.run():
            step.bytesDone = channel.getBytesDownloaded()
            self.__updateStepProgress(step)
            Thread.considerYield()
        step.bytesDone = channel.getBytesDownloaded()
        self.__updateStepProgress(step)
        if not channel.isValid():
            print "Failed to download %s" % (url)
            return False

        if not fileSpec.fullVerify(self.packageDir):
            print "after downloading, %s incorrect" % (
                fileSpec.filename)
            return False
        
        return True

    def __applyPatch(self, step, patchfile):
        """ Applies the indicated patching in-place to the current
        uncompressed archive.  The patchfile is removed after the
        operation.  Returns true on success, false on failure. """

        origPathname = Filename(self.packageDir, self.uncompressedArchive.filename)
        patchPathname = Filename(self.packageDir, patchfile.file.filename)
        result = Filename.temporary('', 'patch_')
        print "Patching %s with %s" % (origPathname, patchPathname)

        p = Patchfile()  # The C++ class

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
            return False

        if not result.renameTo(origPathname):
            print "Couldn't rename %s to %s" % (result, origPathname)
            return False
            
        return True

    def __uncompressArchive(self, step):
        """ Turns the compressed archive into the uncompressed
        archive.  Returns true on success, false on failure. """

        sourcePathname = Filename(self.packageDir, self.compressedArchive.filename)
        targetPathname = Filename(self.packageDir, self.uncompressedArchive.filename)
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
            return False
            
        step.bytesDone = totalBytes
        self.__updateStepProgress(step)

        if not self.uncompressedArchive.quickVerify(self.packageDir):
            print "after uncompressing, %s still incorrect" % (
                self.uncompressedArchive.filename)
            return False

        # Now we can safely remove the compressed archive.
        sourcePathname.unlink()
        return True
    
    def __unpackArchive(self, step):
        """ Unpacks any files in the archive that want to be unpacked
        to disk. """

        if not self.extracts:
            # Nothing to extract.
            self.hasPackage = True
            return True

        mfPathname = Filename(self.packageDir, self.uncompressedArchive.filename)
        print "Unpacking %s" % (mfPathname)
        mf = Multifile()
        if not mf.openRead(mfPathname):
            print "Couldn't open %s" % (mfPathname)
            return False
        
        allExtractsOk = True
        step.bytesDone = 0
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

            step.bytesDone += file.size
            self.__updateStepProgress(step)
            Thread.considerYield()

        if not allExtractsOk:
            return False

        self.hasPackage = True
        return True

    def installPackage(self, appRunner):
        """ Mounts the package and sets up system paths so it becomes
        available for use. """

        assert self.hasPackage
        if self.installed:
            # Already installed.
            return True
        assert self not in appRunner.installedPackages

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

        # Add this to the Python search path, if it's not already
        # there.  We have to take a bit of care to check if it's
        # already there, since there can be some ambiguity in
        # os-specific path strings.
        root = self.packageDir.toOsSpecific()
        foundOnPath = False
        for p in sys.path:
            if root == p:
                # Already here, exactly.
                foundOnPath = True
                break
            elif root == Filename.fromOsSpecific(p).toOsSpecific():
                # Already here, with some futzing.
                foundOnPath = True
                break

        if not foundOnPath:
            # Not already here; add it.
            sys.path.insert(0, root)

        # Put it on the model-path, too.  We do this indiscriminantly,
        # because the Panda3D runtime won't be adding things to the
        # model-path, so it shouldn't be already there.
        getModelPath().prependDirectory(self.packageDir)

        # Set the environment variable to reference the package root.
        envvar = '%s_ROOT' % (self.packageName.upper())
        ExecutionEnvironment.setEnvironmentVariable(envvar, self.packageDir.toOsSpecific())

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
