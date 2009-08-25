from direct.showbase.DirectObject import DirectObject
from direct.stdpy.threading import Lock

class PackageInstaller(DirectObject):

    """ This class is used in a p3d runtime environment to manage the
    asynchronous download and installation of packages.  If you just
    want to install a package synchronously, see
    appRunner.installPackage() for a simpler interface.

    To use this class, you should subclass from it and override any of
    the six callback methods: downloadStarted(), packageStarted(),
    packageProgress(), downloadProgress(), packageFinished(),
    downloadFinished().

    Also see DWBPackageInstaller, which does exactly this, to add a
    DirectWaitBar GUI.

    Note that in the default mode, with a one-thread task chain, the
    packages will all be downloaded in sequence, one after the other.
    If you add more tasks to the task chain, some of the packages may
    be downloaded in parallel, and the calls to packageStarted()
    .. packageFinished() may therefore overlap.
    """

    globalLock = Lock()
    nextUniqueId = 1

    # This is a chain of state values progressing forward in time.
    S_initial = 0    # addPackage() calls are being made
    S_ready = 1      # donePackages() has been called
    S_started = 2    # download has started
    S_done = 3       # download is over
    
    class PendingPackage:
        """ This class describes a package added to the installer for
        download. """

        # Weight factors for computing download progress.  This
        # attempts to reflect the relative time-per-byte of each of
        # these operations.
        downloadFactor = 1
        uncompressFactor = 0.02
        unpackFactor = 0.01
        
        def __init__(self, packageName, version, host):
            self.packageName = packageName
            self.version = version
            self.host = host

            # Filled in by getDescFile().
            self.package = None

            self.done = False
            self.success = False

            self.calledPackageStarted = False
            self.calledPackageFinished = False

            # This is the amount of stuff we have to process to
            # install this package, and the amount of stuff we have
            # processed so far.  "Stuff" includes bytes downloaded,
            # bytes uncompressed, and bytes extracted; and each of
            # which is weighted differently into one grand total.  So,
            # the total doesn't really represent bytes; it's a
            # unitless number, which means something only as a ratio.
            self.targetDownloadSize = 0

        def getCurrentDownloadSize(self):
            """ Returns the current amount of stuff we have processed
            so far in the download. """
            if self.done:
                return self.targetDownloadSize

            return (
                self.package.bytesDownloaded * self.downloadFactor +
                self.package.bytesUncompressed * self.uncompressFactor +
                self.package.bytesUnpacked * self.unpackFactor)

        def getProgress(self):
            """ Returns the download progress of this package in the
            range 0..1. """

            if not self.targetDownloadSize:
                return 1

            return float(self.getCurrentDownloadSize()) / float(self.targetDownloadSize)

        def getDescFile(self, http):
            """ Synchronously downloads the desc files required for
            the package. """
            
            if not self.host.downloadContentsFile(http):
                return False

            # All right, get the package info now.
            self.package = self.host.getPackage(self.packageName, self.version)
            if not self.package:
                print "Package %s %s not known on %s" % (
                    self.packageName, self.version, self.host.hostUrl)
                return False

            if not self.package.downloadDescFile(http):
                return False

            self.package.checkStatus()
            self.targetDownloadSize = (
                self.package.getDownloadSize() * self.downloadFactor +
                self.package.getUncompressSize() * self.uncompressFactor +
                self.package.getUnpackSize() * self.unpackFactor)
            
            return True

    def __init__(self, appRunner, taskChain = 'install'):
        self.globalLock.acquire()
        try:
            self.uniqueId = PackageInstaller.nextUniqueId
            PackageInstaller.nextUniqueId += 1
        finally:
            self.globalLock.release()
        
        self.appRunner = appRunner
        self.taskChain = taskChain
        
        # If the task chain hasn't yet been set up, create the
        # default parameters now.
        if not taskMgr.hasTaskChain(self.taskChain):
            taskMgr.setupTaskChain(self.taskChain, numThreads = 1)

        self.callbackLock = Lock()
        self.calledDownloadStarted = False
        self.calledDownloadFinished = False

        # A list of all packages that have been added to the
        # installer.
        self.packageLock = Lock()
        self.packages = []
        self.state = self.S_initial

        # A list of packages that are waiting for their desc files.
        self.needsDescFile = []
        self.descFileTask = None
        
        # A list of packages that are waiting to be downloaded and
        # installed.
        self.needsDownload = []
        self.downloadTask = None

        # A list of packages that have been successfully installed, or
        # packages that have failed.
        self.done = []
        self.failed = []

        # This task is spawned on the default task chain, to update
        # the status during the download.
        self.progressTask = None

        # The totalDownloadSize is None, until all package desc files
        # have been read.
        self.totalDownloadSize = None
        
        self.accept('PackageInstaller-%s-allHaveDesc' % self.uniqueId,
                    self.__allHaveDesc)
        self.accept('PackageInstaller-%s-packageStarted' % self.uniqueId,
                    self.__packageStarted)
        self.accept('PackageInstaller-%s-packageDone' % self.uniqueId,
                    self.__packageDone)

    def destroy(self):
        """ Interrupts all pending downloads.  No further callbacks
        will be made. """
        self.cleanup()

    def cleanup(self):
        """ Interrupts all pending downloads.  No further callbacks
        will be made. """

        self.packageLock.acquire()
        try:
            if self.descFileTask:
                taskMgr.remove(self.descFileTask)
                self.descFileTask = None
            if self.downloadTask:
                taskMgr.remove(self.downloadTask)
                self.downloadTask = None
        finally:
            self.packageLock.release()

        if self.progressTask:
            taskMgr.remove(self.progressTask)
            self.progressTask = None

        self.ignoreAll()
        
    def addPackage(self, packageName, version = None, hostUrl = None):
        """ Adds the named package to the list of packages to be
        downloaded.  Call donePackages() to finish the list. """

        if self.state != self.S_initial:
            raise ValueError, 'addPackage called after donePackages'

        host = self.appRunner.getHost(hostUrl)
        pp = self.PendingPackage(packageName, version, host)

        self.packageLock.acquire()
        try:
            self.packages.append(pp)
            self.needsDescFile.append(pp)
            if not self.descFileTask:
                self.descFileTask = taskMgr.add(
                    self.__getDescFileTask, 'getDescFile',
                    taskChain = self.taskChain)
        finally:
            self.packageLock.release()

    def donePackages(self):
        """ After calling addPackage() for each package to be
        installed, call donePackages() to mark the end of the list.
        This is necessary to determine what the complete set of
        packages is (and therefore how large the total download size
        is).  Until this is called, no low-level callbacks will be
        made as the packages are downloading. """

        if self.state != self.S_initial:
            # We've already been here.
            return

        working = True
        
        self.packageLock.acquire()
        try:
            if self.state != self.S_initial:
                return
            self.state = self.S_ready
            if not self.needsDescFile:
                # All package desc files are already available; so begin.
                working = self.__prepareToStart()
        finally:
            self.packageLock.release()

        if not working:
            self.downloadFinished(True)

    def downloadStarted(self):
        """ This callback is made at some point after donePackages()
        is called; at the time of this callback, the total download
        size is known, and we can sensibly report progress through the
        whole. """
        pass

    def packageStarted(self, package):
        """ This callback is made for each package between
        downloadStarted() and downloadFinished() to indicate the start
        of a new package. """
        pass

    def packageProgress(self, package, progress):
        """ This callback is made repeatedly between packageStarted()
        and packageFinished() to update the current progress on the
        indicated package only.  The progress value ranges from 0
        (beginning) to 1 (complete). """
        pass
        
    def downloadProgress(self, overallProgress):
        """ This callback is made repeatedly between downloadStarted()
        and downloadFinished() to update the current progress through
        all packages.  The progress value ranges from 0 (beginning) to
        1 (complete). """
        pass

    def packageFinished(self, package, success):
        """ This callback is made for each package between
        downloadStarted() and downloadFinished() to indicate that a
        package has finished downloading.  If success is true, there
        were no problems and the package is now installed.

        If this package did not require downloading (because it was
        already downloaded), this callback will be made immediately,
        *without* a corresponding call to packageStarted(), and may
        even be made before downloadStarted(). """
        pass

    def downloadFinished(self, success):
        """ This callback is made when all of the packages have been
        downloaded and installed (or there has been some failure).  If
        all packages where successfully installed, success is True.

        If there were no packages that required downloading, this
        callback will be made immediately, *without* a corresponding
        call to downloadStarted(). """
        pass

    def __prepareToStart(self):
        """ This is called internally when transitioning from S_ready
        to S_started.  It sets up whatever initial values are
        needed.  Assumes self.packageLock is held.  Returns False if
        there were no packages to download, and the state was
        therefore transitioned immediately to S_done. """

        if not self.needsDownload:
            self.state = self.S_done
            return False

        self.state = self.S_started

        assert not self.downloadTask
        self.downloadTask = taskMgr.add(
            self.__downloadPackageTask, 'downloadPackage',
            taskChain = self.taskChain)

        assert not self.progressTask
        self.progressTask = taskMgr.add(
            self.__progressTask, 'packageProgress')

        return True

    def __allHaveDesc(self):
        """ This method is called internally when all of the pending
        packages have their desc info. """
        working = True
        
        self.packageLock.acquire()
        try:
            if self.state == self.S_ready:
                # We've already called donePackages(), so move on now.
                working = self.__prepareToStart()
        finally:
            self.packageLock.release()

        if not working:
            self.__callDownloadFinished(True)

    def __packageStarted(self, pp):
        """ This method is called when a single package is beginning
        to download. """
        print "Downloading %s" % (pp.packageName)
        self.__callDownloadStarted()
        self.__callPackageStarted(pp)

    def __packageDone(self, pp):
        """ This method is called when a single package has been
        downloaded and installed, or has failed. """
        print "Downloaded %s: %s" % (pp.packageName, pp.success)
        self.__callPackageFinished(pp, pp.success)

        if not pp.calledPackageStarted:
            # Trivially done; this one was done before it got started.
            return

        assert self.state == self.S_started
        # See if there are more packages to go.
        success = True
        allDone = True
        self.packageLock.acquire()
        try:
            assert self.state == self.S_started
            for pp in self.packages:
                if pp.done:
                    success = success and pp.success
                else:
                    allDone = False
        finally:
            self.packageLock.release()

        if allDone:
            self.__callDownloadFinished(success)

    def __callPackageStarted(self, pp):
        """ Calls the packageStarted() callback for a particular
        package if it has not already been called, being careful to
        avoid race conditions. """

        self.callbackLock.acquire()
        try:
            if not pp.calledPackageStarted:
                self.packageStarted(pp.package)
                self.packageProgress(pp.package, 0)
                pp.calledPackageStarted = True
        finally:
            self.callbackLock.release()

    def __callPackageFinished(self, pp, success):
        """ Calls the packageFinished() callback for a paricular
        package if it has not already been called, being careful to
        avoid race conditions. """

        self.callbackLock.acquire()
        try:
            if not pp.calledPackageFinished:
                if success:
                    self.packageProgress(pp.package, 1)
                self.packageFinished(pp.package, success)
                pp.calledPackageFinished = True
        finally:
            self.callbackLock.release()

    def __callDownloadStarted(self):
        """ Calls the downloadStarted() callback if it has not already
        been called, being careful to avoid race conditions. """

        self.callbackLock.acquire()
        try:
            if not self.calledDownloadStarted:
                self.downloadStarted()
                self.downloadProgress(0)
                self.calledDownloadStarted = True
        finally:
            self.callbackLock.release()

    def __callDownloadFinished(self, success):
        """ Calls the downloadFinished() callback if it has not
        already been called, being careful to avoid race
        conditions. """

        self.callbackLock.acquire()
        try:
            if not self.calledDownloadFinished:
                if success:
                    self.downloadProgress(1)
                self.downloadFinished(success)
                self.calledDownloadFinished = True
        finally:
            self.callbackLock.release()

    def __getDescFileTask(self, task):

        """ This task runs on the aysynchronous task chain; each pass,
        it extracts one package from self.needsDescFile and downloads
        its desc file.  On success, it adds the package to
        self.needsDownload. """
        
        self.packageLock.acquire()
        try:
            # If we've finished all of the packages that need desc
            # files, stop the task.
            if not self.needsDescFile:
                self.descFileTask = None
                messenger.send('PackageInstaller-%s-allHaveDesc' % self.uniqueId,
                               taskChain = 'default')
                return task.done
            pp = self.needsDescFile[0]
            del self.needsDescFile[0]
        finally:
            self.packageLock.release()

        # Now serve this one package.
        if not pp.getDescFile(self.appRunner.http):
            self.__donePackage(pp, False)
            return task.cont

        if pp.package.hasPackage:
            # This package is already downloaded.
            self.__donePackage(pp, True)
            return task.cont

        # This package is now ready to be downloaded.
        self.packageLock.acquire()
        try:
            self.needsDownload.append(pp)
        finally:
            self.packageLock.release()

        return task.cont
        
    def __downloadPackageTask(self, task):

        """ This task runs on the aysynchronous task chain; each pass,
        it extracts one package from self.needsDownload and downloads
        it. """
        
        self.packageLock.acquire()
        try:
            # If we're done downloading, stop the task.
            if self.state == self.S_done or not self.needsDownload:
                self.downloadTask = None
                return task.done

            assert self.state == self.S_started        
            pp = self.needsDownload[0]
            del self.needsDownload[0]
        finally:
            self.packageLock.release()

        # Now serve this one package.
        messenger.send('PackageInstaller-%s-packageStarted' % self.uniqueId,
                       [pp], taskChain = 'default')
        
        if not pp.package.downloadPackage(self.appRunner.http):
            self.__donePackage(pp, False)
            return task.cont

        pp.package.installPackage(self.appRunner)

        # Successfully downloaded and installed.
        self.__donePackage(pp, True)
        
        return task.cont
        
    def __donePackage(self, pp, success):
        """ Marks the indicated package as done, either successfully
        or otherwise. """
        assert not pp.done

        self.packageLock.acquire()
        try:
            pp.done = True
            pp.success = success
            if success:
                self.done.append(pp)
            else:
                self.failed.append(pp)
        finally:
            self.packageLock.release()

        messenger.send('PackageInstaller-%s-packageDone' % self.uniqueId,
                       [pp], taskChain = 'default')

    def __progressTask(self, task):
        self.callbackLock.acquire()
        try:
            if not self.calledDownloadStarted:
                # We haven't yet officially started the download.
                return task.cont

            if self.calledDownloadFinished:
                # We've officially ended the download.
                self.progressTask = None
                return task.done

            targetDownloadSize = 0
            currentDownloadSize = 0
            for pp in self.packages:
                targetDownloadSize += pp.targetDownloadSize
                currentDownloadSize += pp.getCurrentDownloadSize()
                if pp.calledPackageStarted and not pp.calledPackageFinished:
                    self.packageProgress(pp.package, pp.getProgress())

            if not targetDownloadSize:
                progress = 1
            else:
                progress = float(currentDownloadSize) / float(targetDownloadSize)
            self.downloadProgress(progress)
            
        finally:
            self.callbackLock.release()

        return task.cont
    
