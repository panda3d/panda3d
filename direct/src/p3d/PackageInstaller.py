"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["PackageInstaller"]

from direct.showbase.DirectObject import DirectObject
from direct.stdpy.threading import Lock, RLock
from direct.showbase.MessengerGlobal import messenger
from direct.task.TaskManagerGlobal import taskMgr
from direct.p3d.PackageInfo import PackageInfo
from panda3d.core import TPLow, PStatCollector
from direct.directnotify.DirectNotifyGlobal import directNotify

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

    """

    notify = directNotify.newCategory("PackageInstaller")

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

        notify = directNotify.newCategory("PendingPackage")

        def __init__(self, packageName, version, host):
            self.packageName = packageName
            self.version = version
            self.host = host

            # This will be filled in properly by checkDescFile() or
            # getDescFile(); in the meantime, set a placeholder.
            self.package = PackageInfo(host, packageName, version)

            # Set true when the package has finished downloading,
            # either successfully or unsuccessfully.
            self.done = False

            # Set true or false when self.done has been set.
            self.success = False

            # Set true when the packageFinished() callback has been
            # delivered.
            self.notified = False

            # These are used to ensure the callbacks only get
            # delivered once for a particular package.
            self.calledPackageStarted = False
            self.calledPackageFinished = False

            # This is the amount of stuff we have to process to
            # install this package, and the amount of stuff we have
            # processed so far.  "Stuff" includes bytes downloaded,
            # bytes uncompressed, and bytes extracted; and each of
            # which is weighted differently into one grand total.  So,
            # the total doesn't really represent bytes; it's a
            # unitless number, which means something only as a ratio
            # to other packages.  Filled in by checkDescFile() or
            # getDescFile().
            self.downloadEffort = 0

            # Similar, but this is the theoretical effort if the
            # package were already downloaded.
            self.prevDownloadedEffort = 0

        def __cmp__(self, pp):
            """ Python comparision function.  This makes all
            PendingPackages withe same (packageName, version, host)
            combination be deemed equivalent. """
            return cmp((self.packageName, self.version, self.host),
                       (pp.packageName, pp.version, pp.host))

        def getProgress(self):
            """ Returns the download progress of this package in the
            range 0..1. """

            return self.package.downloadProgress

        def checkDescFile(self):
            """ Returns true if the desc file is already downloaded
            and good, or false if it needs to be downloaded. """

            if not self.host.hasCurrentContentsFile():
                # If the contents file isn't ready yet, we can't check
                # the desc file yet.
                return False

            # All right, get the package info now.
            package = self.host.getPackage(self.packageName, self.version)
            if not package:
                self.notify.warning("Package %s %s not known on %s" % (
                    self.packageName, self.version, self.host.hostUrl))
                return False

            self.package = package
            self.package.checkStatus()

            if not self.package.hasDescFile:
                return False

            self.downloadEffort = self.package.getDownloadEffort()
            self.prevDownloadEffort = 0
            if self.downloadEffort == 0:
                self.prevDownloadedEffort = self.package.getPrevDownloadedEffort()

            return True


        def getDescFile(self, http):
            """ Synchronously downloads the desc files required for
            the package. """

            if not self.host.downloadContentsFile(http):
                return False

            # All right, get the package info now.
            package = self.host.getPackage(self.packageName, self.version)
            if not package:
                self.notify.warning("Package %s %s not known on %s" % (
                    self.packageName, self.version, self.host.hostUrl))
                return False

            self.package = package
            if not self.package.downloadDescFile(http):
                return False

            self.package.checkStatus()
            self.downloadEffort = self.package.getDownloadEffort()
            self.prevDownloadEffort = 0
            if self.downloadEffort == 0:
                self.prevDownloadedEffort = self.package.getPrevDownloadedEffort()

            return True

    def __init__(self, appRunner, taskChain = 'default'):
        self.globalLock.acquire()
        try:
            self.uniqueId = PackageInstaller.nextUniqueId
            PackageInstaller.nextUniqueId += 1
        finally:
            self.globalLock.release()

        self.appRunner = appRunner
        self.taskChain = taskChain

        # If we're to be running on an asynchronous task chain, and
        # the task chain hasn't yet been set up already, create the
        # default parameters now.
        if taskChain != 'default' and not taskMgr.hasTaskChain(self.taskChain):
            taskMgr.setupTaskChain(self.taskChain, numThreads = 1,
                                   threadPriority = TPLow)

        self.callbackLock = Lock()
        self.calledDownloadStarted = False
        self.calledDownloadFinished = False

        # A list of all packages that have been added to the
        # installer.
        self.packageLock = RLock()
        self.packages = []
        self.state = self.S_initial

        # A list of packages that are waiting for their desc files.
        self.needsDescFile = []
        self.descFileTask = None

        # A list of packages that are waiting to be downloaded and
        # installed.
        self.needsDownload = []
        self.downloadTask = None

        # A list of packages that were already done at the time they
        # were passed to addPackage().
        self.earlyDone = []

        # A list of packages that have been successfully installed, or
        # packages that have failed.
        self.done = []
        self.failed = []

        # This task is spawned on the default task chain, to update
        # the status during the download.
        self.progressTask = None

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
            raise ValueError('addPackage called after donePackages')

        host = self.appRunner.getHostWithAlt(hostUrl)
        pp = self.PendingPackage(packageName, version, host)

        self.packageLock.acquire()
        try:
            self.__internalAddPackage(pp)
        finally:
            self.packageLock.release()

    def __internalAddPackage(self, pp):
        """ Adds the indicated "pending package" to the appropriate
        list(s) for downloading and installing.  Assumes packageLock
        is already held."""

        if pp in self.packages:
            # Already added.
            return

        self.packages.append(pp)

        # We always add the package to needsDescFile, even if we
        # already have its desc file; this guarantees that packages
        # are downloaded in the order they are added.
        self.needsDescFile.append(pp)
        if not self.descFileTask:
            self.descFileTask = taskMgr.add(
                self.__getDescFileTask, 'getDescFile',
                taskChain = self.taskChain)

    def donePackages(self):
        """ After calling addPackage() for each package to be
        installed, call donePackages() to mark the end of the list.
        This is necessary to determine what the complete set of
        packages is (and therefore how large the total download size
        is).  None of the low-level callbacks will be made before this
        call. """

        if self.state != self.S_initial:
            # We've already been here.
            return

        # Throw the messages for packages that were already done
        # before we started.
        for pp in self.earlyDone:
            self.__donePackage(pp, True)
        self.earlyDone = []

        self.packageLock.acquire()
        try:
            if self.state != self.S_initial:
                return
            self.state = self.S_ready
            if not self.needsDescFile:
                # All package desc files are already available; so begin.
                self.__prepareToStart()
        finally:
            self.packageLock.release()

        if not self.packages:
            # Trivial no-op.
            self.__callDownloadFinished(True)

    def downloadStarted(self):
        """ This callback is made at some point after donePackages()
        is called; at the time of this callback, the total download
        size is known, and we can sensibly report progress through the
        whole. """

        self.notify.info("downloadStarted")

    def packageStarted(self, package):
        """ This callback is made for each package between
        downloadStarted() and downloadFinished() to indicate the start
        of a new package. """

        self.notify.debug("packageStarted: %s" % (package.packageName))

    def packageProgress(self, package, progress):
        """ This callback is made repeatedly between packageStarted()
        and packageFinished() to update the current progress on the
        indicated package only.  The progress value ranges from 0
        (beginning) to 1 (complete). """

        self.notify.debug("packageProgress: %s %s" % (package.packageName, progress))

    def downloadProgress(self, overallProgress):
        """ This callback is made repeatedly between downloadStarted()
        and downloadFinished() to update the current progress through
        all packages.  The progress value ranges from 0 (beginning) to
        1 (complete). """

        self.notify.debug("downloadProgress: %s" % (overallProgress))

    def packageFinished(self, package, success):
        """ This callback is made for each package between
        downloadStarted() and downloadFinished() to indicate that a
        package has finished downloading.  If success is true, there
        were no problems and the package is now installed.

        If this package did not require downloading (because it was
        already downloaded), this callback will be made immediately,
        *without* a corresponding call to packageStarted(), and may
        even be made before downloadStarted(). """

        self.notify.info("packageFinished: %s %s" % (package.packageName, success))

    def downloadFinished(self, success):
        """ This callback is made when all of the packages have been
        downloaded and installed (or there has been some failure).  If
        all packages where successfully installed, success is True.

        If there were no packages that required downloading, this
        callback will be made immediately, *without* a corresponding
        call to downloadStarted(). """

        self.notify.info("downloadFinished: %s" % (success))

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

        self.__callDownloadStarted()
        self.__callPackageStarted(pp)

    def __packageDone(self, pp):
        """ This method is called when a single package has been
        downloaded and installed, or has failed. """

        self.__callPackageFinished(pp, pp.success)
        pp.notified = True

        # See if there are more packages to go.
        success = True
        allDone = True
        self.packageLock.acquire()
        try:
            for pp in self.packages:
                if pp.notified:
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

                eventName = 'PackageInstaller-%s-allHaveDesc' % self.uniqueId
                messenger.send(eventName, taskChain = 'default')

                return task.done
            pp = self.needsDescFile[0]
            del self.needsDescFile[0]
        finally:
            self.packageLock.release()

        # Now serve this one package.
        if not pp.checkDescFile():
            if not pp.getDescFile(self.appRunner.http):
                self.__donePackage(pp, False)
                return task.cont

        # This package is now ready to be downloaded.  We always add
        # it to needsDownload, even if it's already downloaded, to
        # guarantee ordering of packages.

        self.packageLock.acquire()
        try:
            # Also add any packages required by this one.
            for packageName, version, host in pp.package.requires:
                pp2 = self.PendingPackage(packageName, version, host)
                self.__internalAddPackage(pp2)
            self.needsDownload.append(pp)
        finally:
            self.packageLock.release()

        return task.cont

    def __downloadPackageTask(self, task):

        """ This task runs on the aysynchronous task chain; each pass,
        it extracts one package from self.needsDownload and downloads
        it. """

        while True:
            self.packageLock.acquire()
            try:
                # If we're done downloading, stop the task.
                if self.state == self.S_done or not self.needsDownload:
                    self.downloadTask = None
                    self.packageLock.release()
                    yield task.done; return

                assert self.state == self.S_started
                pp = self.needsDownload[0]
                del self.needsDownload[0]
            except:
                self.packageLock.release()
                raise
            self.packageLock.release()

            # Now serve this one package.
            eventName = 'PackageInstaller-%s-packageStarted' % self.uniqueId
            messenger.send(eventName, [pp], taskChain = 'default')

            if not pp.package.hasPackage:
                for token in pp.package.downloadPackageGenerator(self.appRunner.http):
                    if token == pp.package.stepContinue:
                        yield task.cont
                    else:
                        break

                if token != pp.package.stepComplete:
                    pc = PStatCollector(':App:PackageInstaller:donePackage:%s' % (pp.package.packageName))
                    pc.start()
                    self.__donePackage(pp, False)
                    pc.stop()
                    yield task.cont
                    continue

            # Successfully downloaded and installed.
            pc = PStatCollector(':App:PackageInstaller:donePackage:%s' % (pp.package.packageName))
            pc.start()
            self.__donePackage(pp, True)
            pc.stop()

            # Continue the loop without yielding, so we pick up the
            # next package within this same frame.

    def __donePackage(self, pp, success):
        """ Marks the indicated package as done, either successfully
        or otherwise. """
        assert not pp.done

        if success:
            pc = PStatCollector(':App:PackageInstaller:install:%s' % (pp.package.packageName))
            pc.start()
            pp.package.installPackage(self.appRunner)
            pc.stop()

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

        eventName = 'PackageInstaller-%s-packageDone' % self.uniqueId
        messenger.send(eventName, [pp], taskChain = 'default')

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

            downloadEffort = 0
            currentDownloadSize = 0
            for pp in self.packages:
                downloadEffort += pp.downloadEffort + pp.prevDownloadedEffort
                packageProgress = pp.getProgress()
                currentDownloadSize += pp.downloadEffort * packageProgress + pp.prevDownloadedEffort
                if pp.calledPackageStarted and not pp.calledPackageFinished:
                    self.packageProgress(pp.package, packageProgress)

            if not downloadEffort:
                progress = 1
            else:
                progress = float(currentDownloadSize) / float(downloadEffort)
            self.downloadProgress(progress)

        finally:
            self.callbackLock.release()

        return task.cont

