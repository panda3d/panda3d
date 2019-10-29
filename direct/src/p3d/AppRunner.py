"""
This module is intended to be compiled into the Panda3D runtime
distributable, to execute a packaged p3d application, but it can also
be run directly via the Python interpreter (if the current Panda3D and
Python versions match the version expected by the application).  See
runp3d.py for a command-line tool to invoke this module.

The global AppRunner instance may be imported as follows::

   from direct.showbase.AppRunnerGlobal import appRunner

This will be None if Panda was not run from the runtime environment.

.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""

__all__ = ["AppRunner", "dummyAppRunner", "ArgumentError"]

import sys
import os

if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins

from direct.showbase import VFSImporter
from direct.showbase.DirectObject import DirectObject
from panda3d.core import VirtualFileSystem, Filename, Multifile, loadPrcFileData, unloadPrcFile, getModelPath, WindowProperties, ExecutionEnvironment, PandaSystem, Notify, StreamWriter, ConfigVariableString, ConfigPageManager
from panda3d.direct import init_app_for_gui
from panda3d import core
from direct.stdpy import file, glob
from direct.task.TaskManagerGlobal import taskMgr
from direct.showbase.MessengerGlobal import messenger
from direct.showbase import AppRunnerGlobal
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.p3d.HostInfo import HostInfo
from direct.p3d.ScanDirectoryNode import ScanDirectoryNode
from direct.p3d.InstalledHostData import InstalledHostData
from direct.p3d.InstalledPackageData import InstalledPackageData

# These imports are read by the C++ wrapper in p3dPythonRun.cxx.
from direct.p3d.JavaScript import Undefined, ConcreteStruct

class ArgumentError(AttributeError):
    pass

class ScriptAttributes:
    """ This dummy class serves as the root object for the scripting
    interface.  The Python code can store objects and functions here
    for direct inspection by the browser's JavaScript code. """
    pass

class AppRunner(DirectObject):

    """ This class is intended to be compiled into the Panda3D runtime
    distributable, to execute a packaged p3d application.  It also
    provides some useful runtime services while running in that
    packaged environment.

    It does not usually exist while running Python directly, but you
    can use dummyAppRunner() to create one at startup for testing or
    development purposes.  """

    notify = directNotify.newCategory("AppRunner")

    ConfigBasename = 'config.xml'

    # Default values for parameters that are absent from the config file:
    maxDiskUsage = 2048 * 1048576  # 2 GB

    # Values for verifyContents, from p3d_plugin.h
    P3DVCNone = 0
    P3DVCNormal = 1
    P3DVCForce = 2
    P3DVCNever = 3

    # Also from p3d_plugin.h
    P3D_CONTENTS_DEFAULT_MAX_AGE = 5

    def __init__(self):
        DirectObject.__init__(self)

        # We direct both our stdout and stderr objects onto Panda's
        # Notify stream.  This ensures that unadorned print statements
        # made within Python will get routed into the log properly.
        stream = StreamWriter(Notify.out(), False)
        sys.stdout = stream
        sys.stderr = stream

        # This is set true by dummyAppRunner(), below.
        self.dummy = False

        # These will be set from the application flags when
        # setP3DFilename() is called.
        self.allowPythonDev = False
        self.guiApp = False
        self.interactiveConsole = False
        self.initialAppImport = False
        self.trueFileIO = False
        self.respectPerPlatform = None

        self.verifyContents = self.P3DVCNone

        self.sessionId = 0
        self.packedAppEnvironmentInitialized = False
        self.gotWindow = False
        self.gotP3DFilename = False
        self.p3dFilename = None
        self.p3dUrl = None
        self.started = False
        self.windowOpened = False
        self.windowPrc = None

        self.http = None
        if hasattr(core, 'HTTPClient'):
            self.http = core.HTTPClient.getGlobalPtr()

        self.Undefined = Undefined
        self.ConcreteStruct = ConcreteStruct

        # This is per session.
        self.nextScriptId = 0

        # TODO: we need one of these per instance, not per session.
        self.instanceId = None

        # The root Panda3D install directory.  This is filled in when
        # the instance starts up.
        self.rootDir = None

        # The log directory.  Also filled in when the instance starts.
        self.logDirectory = None

        # self.superMirrorUrl, if nonempty, is the "super mirror" URL
        # that should be contacted first before trying the actual
        # host.  This is primarily used for "downloading" from a
        # locally-stored Panda3D installation.  This is also filled in
        # when the instance starts up.
        self.superMirrorUrl = None

        # A list of the Panda3D packages that have been loaded.
        self.installedPackages = []

        # A list of the Panda3D packages that in the queue to be
        # downloaded.
        self.downloadingPackages = []

        # A dictionary of HostInfo objects for the various download
        # hosts we have imported packages from.
        self.hosts = {}

        # The altHost string that is in effect from the HTML tokens,
        # if any, and the dictionary of URL remapping: orig host url
        # -> alt host url.
        self.altHost = None
        self.altHostMap = {}

        # The URL from which Panda itself should be downloaded.
        self.pandaHostUrl = PandaSystem.getPackageHostUrl()

        # Application code can assign a callable object here; if so,
        # it will be invoked when an uncaught exception propagates to
        # the top of the TaskMgr.run() loop.
        self.exceptionHandler = None

        # Managing packages for runtime download.
        self.downloadingPackages = []
        self.downloadTask = None

        # The mount point for the multifile.  For now, this is always
        # the current working directory, for convenience; but when we
        # move to multiple-instance sessions, it may have to be
        # different for each instance.
        self.multifileRoot = str(ExecutionEnvironment.getCwd())

        # The "main" object will be exposed to the DOM as a property
        # of the plugin object; that is, document.pluginobject.main in
        # JavaScript will be appRunner.main here.  This may be
        # replaced with a direct reference to the JavaScript object
        # later, in setInstanceInfo().
        self.main = ScriptAttributes()

        # By default, we publish a stop() method so the browser can
        # easy stop the plugin.  A particular application can remove
        # this if it chooses.
        self.main.stop = self.stop

        # This will be the browser's toplevel window DOM object;
        # e.g. self.dom.document will be the document.
        self.dom = None

        # This is the list of expressions we will evaluate when
        # self.dom gets assigned.
        self.deferredEvals = []

        # This is the default requestFunc that is installed if we
        # never call setRequestFunc().
        def defaultRequestFunc(*args):
            if args[1] == 'notify':
                # Quietly ignore notifies.
                return
            self.notify.info("Ignoring request: %s" % (args,))
        self.requestFunc = defaultRequestFunc

        # This will be filled in with the default WindowProperties for
        # this instance, e.g. the WindowProperties necessary to
        # re-embed a window in the browser frame.
        self.windowProperties = None

        # Store our pointer so DirectStart-based apps can find us.
        if AppRunnerGlobal.appRunner is None:
            AppRunnerGlobal.appRunner = self

        # We use this messenger hook to dispatch this __startIfReady()
        # call back to the main thread.
        self.accept('AppRunner_startIfReady', self.__startIfReady)

    def getToken(self, tokenName):
        """ Returns the value of the indicated web token as a string,
        if it was set, or None if it was not. """

        return self.tokenDict.get(tokenName.lower(), None)

    def getTokenInt(self, tokenName):
        """ Returns the value of the indicated web token as an integer
        value, if it was set, or None if it was not, or not an
        integer. """

        value = self.getToken(tokenName)
        if value is not None:
            try:
                value = int(value)
            except ValueError:
                value = None
        return value

    def getTokenFloat(self, tokenName):
        """ Returns the value of the indicated web token as a
        floating-point value value, if it was set, or None if it was
        not, or not a number. """

        value = self.getToken(tokenName)
        if value is not None:
            try:
                value = float(value)
            except ValueError:
                value = None
        return value

    def getTokenBool(self, tokenName):
        """ Returns the value of the indicated web token as a boolean
        value, if it was set, or None if it was not. """

        value = self.getTokenInt(tokenName)
        if value is not None:
            value = bool(value)
        return value



    def installPackage(self, packageName, version = None, hostUrl = None):

        """ Installs the named package, downloading it first if
        necessary.  Returns true on success, false on failure.  This
        method runs synchronously, and will block until it is
        finished; see the PackageInstaller class if you want this to
        happen asynchronously instead. """

        host = self.getHostWithAlt(hostUrl)
        if not host.downloadContentsFile(self.http):
            return False

        # All right, get the package info now.
        package = host.getPackage(packageName, version)
        if not package:
            self.notify.warning("Package %s %s not known on %s" % (
                packageName, version, hostUrl))
            return False

        return self.__rInstallPackage(package, [])

    def __rInstallPackage(self, package, nested):
        """ The recursive implementation of installPackage().  The new
        parameter, nested, is a list of packages that we are
        recursively calling this from, to avoid recursive loops. """

        package.checkStatus()
        if not package.downloadDescFile(self.http):
            return False

        # Now that we've downloaded and read the desc file, we can
        # install all of the required packages first.
        nested = nested[:] + [self]
        for packageName, version, host in package.requires:
            if host.downloadContentsFile(self.http):
                p2 = host.getPackage(packageName, version)
                if not p2:
                    self.notify.warning("Couldn't find %s %s on %s" % (packageName, version, host.hostUrl))
                else:
                    if p2 not in nested:
                        self.__rInstallPackage(p2, nested)

        # Now that all of the required packages are installed, carry
        # on to download and install this package.
        if not package.downloadPackage(self.http):
            return False

        if not package.installPackage(self):
            return False

        self.notify.info("Package %s %s installed." % (
            package.packageName, package.packageVersion))
        return True

    def getHostWithAlt(self, hostUrl):
        """ Returns a suitable HostInfo object for downloading
        contents from the indicated URL.  This is almost always the
        same thing as getHost(), except in the rare case when we have
        an alt_host specified in the HTML tokens; in this case, we may
        actually want to download the contents from a different URL
        than the one given, for instance to download a version in
        testing. """

        if hostUrl is None:
            hostUrl = self.pandaHostUrl

        altUrl = self.altHostMap.get(hostUrl, None)
        if altUrl:
            # We got an alternate host.  Use it.
            return self.getHost(altUrl)

        # We didn't get an aternate host, use the original.
        host = self.getHost(hostUrl)

        # But we might need to consult the host itself to see if *it*
        # recommends an altHost.
        if self.altHost:
            # This means forcing the host to download its contents
            # file on the spot, a blocking operation.  This is a
            # little unfortunate, but since alt_host is so rarely
            # used, probably not really a problem.
            host.downloadContentsFile(self.http)
            altUrl = host.altHosts.get(self.altHost, None)
            if altUrl:
                return self.getHost(altUrl)

        # No shenanigans, just return the requested host.
        return host

    def getHost(self, hostUrl, hostDir = None):
        """ Returns a new HostInfo object corresponding to the
        indicated host URL.  If we have already seen this URL
        previously, returns the same object.

        This returns the literal referenced host.  To return the
        mapped host, which is the one we should actually download
        from, see getHostWithAlt().  """

        if not hostUrl:
            hostUrl = self.pandaHostUrl

        host = self.hosts.get(hostUrl, None)
        if not host:
            host = HostInfo(hostUrl, appRunner = self, hostDir = hostDir)
            self.hosts[hostUrl] = host
        return host

    def getHostWithDir(self, hostDir):
        """ Returns the HostInfo object that corresponds to the
        indicated on-disk host directory.  This would be used when
        reading a host directory from disk, instead of downloading it
        from a server.  Supply the full path to the host directory, as
        a Filename.  Returns None if the contents.xml in the indicated
        host directory cannot be read or doesn't seem consistent. """

        host = HostInfo(None, hostDir = hostDir, appRunner = self)
        if not host.hasContentsFile:
            if not host.readContentsFile():
                # Couldn't read the contents.xml file
                return None

        if not host.hostUrl:
            # The contents.xml file there didn't seem to indicate the
            # same host directory.
            return None

        host2 = self.hosts.get(host.hostUrl)
        if host2 is None:
            # No such host already; store this one.
            self.hosts[host.hostUrl] = host
            return host

        if host2.hostDir != host.hostDir:
            # Hmm, we already have that host somewhere else.
            return None

        # We already have that host, and it's consistent.
        return host2

    def deletePackages(self, packages):
        """ Removes all of the indicated packages from the disk,
        uninstalling them and deleting all of their files.  The
        packages parameter must be a list of one or more PackageInfo
        objects, for instance as returned by getHost().getPackage().
        Returns the list of packages that were NOT found. """

        for hostUrl, host in self.hosts.items():
            packages = host.deletePackages(packages)

            if not host.packages:
                # If that's all of the packages for this host, delete
                # the host directory too.
                del self.hosts[hostUrl]
                self.__deleteHostFiles(host)

        return packages

    def __deleteHostFiles(self, host):
        """ Called by deletePackages(), this removes all the files for
        the indicated host (for which we have presumably already
        removed all of the packages). """

        self.notify.info("Deleting host %s: %s" % (host.hostUrl, host.hostDir))
        self.rmtree(host.hostDir)

        self.sendRequest('forget_package', host.hostUrl, '', '')


    def freshenFile(self, host, fileSpec, localPathname):
        """ Ensures that the localPathname is the most current version
        of the file defined by fileSpec, as offered by host.  If not,
        it downloads a new version on-the-spot.  Returns true on
        success, false on failure. """

        assert self.http
        return host.freshenFile(self.http, fileSpec, localPathname)

    def scanInstalledPackages(self):
        """ Scans the hosts and packages already installed locally on
        the system.  Returns a list of InstalledHostData objects, each
        of which contains a list of InstalledPackageData objects. """

        result = []
        hostsFilename = Filename(self.rootDir, 'hosts')
        hostsDir = ScanDirectoryNode(hostsFilename)
        for dirnode in hostsDir.nested:
            host = self.getHostWithDir(dirnode.pathname)
            hostData = InstalledHostData(host, dirnode)

            if host:
                for package in host.getAllPackages(includeAllPlatforms = True):
                    packageDir = package.getPackageDir()
                    if not packageDir.exists():
                        continue

                    subdir = dirnode.extractSubdir(packageDir)
                    if not subdir:
                        # This package, while defined by the host, isn't installed
                        # locally; ignore it.
                        continue

                    packageData = InstalledPackageData(package, subdir)
                    hostData.packages.append(packageData)

            # Now that we've examined all of the packages for the host,
            # anything left over is junk.
            for subdir in dirnode.nested:
                packageData = InstalledPackageData(None, subdir)
                hostData.packages.append(packageData)

            result.append(hostData)

        return result

    def readConfigXml(self):
        """ Reads the config.xml file that may be present in the root
        directory. """

        if not hasattr(core, 'TiXmlDocument'):
            return

        filename = Filename(self.rootDir, self.ConfigBasename)
        doc = core.TiXmlDocument(filename.toOsSpecific())
        if not doc.LoadFile():
            return

        xconfig = doc.FirstChildElement('config')
        if xconfig:
            maxDiskUsage = xconfig.Attribute('max_disk_usage')
            try:
                self.maxDiskUsage = int(maxDiskUsage or '')
            except ValueError:
                pass

    def writeConfigXml(self):
        """ Rewrites the config.xml to the root directory.  This isn't
        called automatically; an application may call this after
        adjusting some parameters (such as self.maxDiskUsage). """

        from panda3d.core import TiXmlDocument, TiXmlDeclaration, TiXmlElement

        filename = Filename(self.rootDir, self.ConfigBasename)
        doc = TiXmlDocument(filename.toOsSpecific())
        decl = TiXmlDeclaration("1.0", "utf-8", "")
        doc.InsertEndChild(decl)

        xconfig = TiXmlElement('config')
        xconfig.SetAttribute('max_disk_usage', str(self.maxDiskUsage))
        doc.InsertEndChild(xconfig)

        # Write the file to a temporary filename, then atomically move
        # it to its actual filename, to avoid race conditions when
        # updating this file.
        tfile = Filename.temporary(str(self.rootDir), '.xml')
        if doc.SaveFile(tfile.toOsSpecific()):
            tfile.renameTo(filename)


    def checkDiskUsage(self):
        """ Checks the total disk space used by all packages, and
        removes old packages if necessary. """

        totalSize = 0
        hosts = self.scanInstalledPackages()
        for hostData in hosts:
            for packageData in hostData.packages:
                totalSize += packageData.totalSize
        self.notify.info("Total Panda3D disk space used: %s MB" % (
            (totalSize + 524288) // 1048576))

        if self.verifyContents == self.P3DVCNever:
            # We're not allowed to delete anything anyway.
            return

        self.notify.info("Configured max usage is: %s MB" % (
            (self.maxDiskUsage + 524288) // 1048576))
        if totalSize <= self.maxDiskUsage:
            # Still within budget; no need to clean up anything.
            return

        # OK, we're over budget.  Now we have to remove old packages.
        usedPackages = []
        for hostData in hosts:
            for packageData in hostData.packages:
                if packageData.package and packageData.package.installed:
                    # Don't uninstall any packages we're currently using.
                    continue

                usedPackages.append((packageData.lastUse, packageData))

        # Sort the packages into oldest-first order.
        usedPackages.sort()

        # Delete packages until we free up enough space.
        packages = []
        for lastUse, packageData in usedPackages:
            if totalSize <= self.maxDiskUsage:
                break
            totalSize -= packageData.totalSize

            if packageData.package:
                packages.append(packageData.package)
            else:
                # If it's an unknown package, just delete it directly.
                print("Deleting unknown package %s" % (packageData.pathname))
                self.rmtree(packageData.pathname)

        packages = self.deletePackages(packages)
        if packages:
            print("Unable to delete %s packages" % (len(packages)))

        return

    def stop(self):
        """ This method can be called by JavaScript to stop the
        application. """

        # We defer the actual exit for a few frames, so we don't raise
        # an exception and invalidate the JavaScript call; and also to
        # help protect against race conditions as the application
        # shuts down.
        taskMgr.doMethodLater(0.5, sys.exit, 'exit')

    def run(self):
        """ This method calls taskMgr.run(), with an optional
        exception handler.  This is generally the program's main loop
        when running in a p3d environment (except on unusual platforms
        like the iPhone, which have to hand the main loop off to the
        OS, and don't use this interface). """

        try:
            taskMgr.run()

        except SystemExit as err:
            # Presumably the window has already been shut down here, but shut
            # it down again for good measure.
            if hasattr(builtins, "base"):
                base.destroy()

            self.notify.info("Normal exit with status %s." % repr(err.code))
            raise

        except:
            # Some unexpected Python exception; pass it to the
            # optional handler, if it is defined.
            if self.exceptionHandler and not self.interactiveConsole:
                self.exceptionHandler()
            else:
                raise

    def rmtree(self, filename):
        """ This is like shutil.rmtree(), but it can remove read-only
        files on Windows.  It receives a Filename, the root directory
        to delete. """
        if filename.isDirectory():
            for child in filename.scanDirectory():
                self.rmtree(Filename(filename, child))
            if not filename.rmdir():
                print("could not remove directory %s" % (filename))
        else:
            if not filename.unlink():
                print("could not delete %s" % (filename))

    def setSessionId(self, sessionId):
        """ This message should come in at startup. """
        self.sessionId = sessionId
        self.nextScriptId = self.sessionId * 1000 + 10000

    def initPackedAppEnvironment(self):
        """ This function sets up the Python environment suitably for
        running a packed app.  It should only run once in any given
        session (and it includes logic to ensure this). """

        if self.packedAppEnvironmentInitialized:
            return

        self.packedAppEnvironmentInitialized = True

        vfs = VirtualFileSystem.getGlobalPtr()

        # Now set up Python to import this stuff.
        VFSImporter.register()
        sys.path.append(self.multifileRoot)

        # Make sure that $MAIN_DIR is set to the p3d root before we
        # start executing the code in this file.
        ExecutionEnvironment.setEnvironmentVariable("MAIN_DIR", Filename(self.multifileRoot).toOsSpecific())

        # Put our root directory on the model-path, too.
        getModelPath().appendDirectory(self.multifileRoot)

        if not self.trueFileIO:
            # Replace the builtin open and file symbols so user code will get
            # our versions by default, which can open and read files out of
            # the multifile.
            builtins.open = file.open
            if sys.version_info < (3, 0):
                builtins.file = file.open
                builtins.execfile = file.execfile
            os.listdir = file.listdir
            os.walk = file.walk
            os.path.join = file.join
            os.path.isfile = file.isfile
            os.path.isdir = file.isdir
            os.path.exists = file.exists
            os.path.lexists = file.lexists
            os.path.getmtime = file.getmtime
            os.path.getsize = file.getsize
            sys.modules['glob'] = glob

        self.checkDiskUsage()

    def __startIfReady(self):
        """ Called internally to start the application. """
        if self.started:
            return

        if self.gotWindow and self.gotP3DFilename:
            self.started = True

            # Now we can ignore future calls to startIfReady().
            self.ignore('AppRunner_startIfReady')

            # Hang a hook so we know when the window is actually opened.
            self.acceptOnce('window-event', self.__windowEvent)

            # Look for the startup Python file.  This might be a magic
            # filename (like "__main__", or any filename that contains
            # invalid module characters), so we can't just import it
            # directly; instead, we go through the low-level importer.

            # If there's no p3d_info.xml file, we look for "main".
            moduleName = 'main'
            if self.p3dPackage:
                mainName = self.p3dPackage.Attribute('main_module')
                if mainName:
                    moduleName = mainName

            # Temporarily set this flag while we import the app, so
            # that if the app calls run() within its own main.py, it
            # will properly get ignored by ShowBase.
            self.initialAppImport = True

            # Python won't let us import a module named __main__.  So,
            # we have to do that manually, via the VFSImporter.
            if moduleName == '__main__':
                dirName = Filename(self.multifileRoot).toOsSpecific()
                importer = VFSImporter.VFSImporter(dirName)
                loader = importer.find_module('__main__')
                if loader is None:
                    raise ImportError('No module named __main__')

                mainModule = loader.load_module('__main__')
            else:
                __import__(moduleName)
                mainModule = sys.modules[moduleName]

            # Check if it has a main() function.  If so, call it.
            if hasattr(mainModule, 'main') and hasattr(mainModule.main, '__call__'):
                mainModule.main(self)

            # Now clear this flag.
            self.initialAppImport = False

            if self.interactiveConsole:
                # At this point, we have successfully loaded the app.
                # If the interactive_console flag is enabled, stop the
                # main loop now and give the user a Python prompt.
                taskMgr.stop()

    def getPandaScriptObject(self):
        """ Called by the browser to query the Panda instance's
        toplevel scripting object, for querying properties in the
        Panda instance.  The attributes on this object are mapped to
        document.pluginobject.main within the DOM. """

        return self.main

    def setBrowserScriptObject(self, dom):
        """ Called by the browser to supply the browser's toplevel DOM
        object, for controlling the JavaScript and the document in the
        same page with the Panda3D plugin. """

        self.dom = dom

        # Now evaluate any deferred expressions.
        for expression in self.deferredEvals:
            self.scriptRequest('eval', self.dom, value = expression,
                               needsResponse = False)
        self.deferredEvals = []

    def setInstanceInfo(self, rootDir, logDirectory, superMirrorUrl,
                        verifyContents, main, respectPerPlatform):
        """ Called by the browser to set some global information about
        the instance. """

        # rootDir is the root Panda3D install directory on the local
        # machine.
        self.rootDir = Filename.fromOsSpecific(rootDir)

        # logDirectory is the directory name where all log files end
        # up.
        if logDirectory:
            self.logDirectory = Filename.fromOsSpecific(logDirectory)
        else:
            self.logDirectory = Filename(rootDir, 'log')

        # The "super mirror" URL, generally used only by panda3d.exe.
        self.superMirrorUrl = superMirrorUrl

        # How anxious should we be about contacting the server for
        # the latest code?
        self.verifyContents = verifyContents

        # The initial "main" object, if specified.
        if main is not None:
            self.main = main

        self.respectPerPlatform = respectPerPlatform
        #self.notify.info("respectPerPlatform = %s" % (self.respectPerPlatform))

        # Now that we have rootDir, we can read the config file.
        self.readConfigXml()


    def addPackageInfo(self, name, platform, version, hostUrl, hostDir = None,
                       recurse = False):
        """ Called by the browser for each one of the "required"
        packages that were preloaded before starting the application.
        If for some reason the package isn't already downloaded, this
        will download it on the spot.  Raises OSError on failure. """

        host = self.getHost(hostUrl, hostDir = hostDir)

        if not host.hasContentsFile:
            # Always pre-read these hosts' contents.xml files, even if
            # we have P3DVCForce in effect, since presumably we've
            # already forced them on the plugin side.
            host.readContentsFile()

        if not host.downloadContentsFile(self.http):
            # Couldn't download?  Must have failed to download in the
            # plugin as well.  But since we launched, we probably have
            # a copy already local; let's use it.
            message = "Host %s cannot be downloaded, cannot preload %s." % (hostUrl, name)
            if not host.hasContentsFile:
                # This is weird.  How did we launch without having
                # this file at all?
                raise OSError(message)

            # Just make it a warning and continue.
            self.notify.warning(message)

        if name == 'panda3d' and not self.pandaHostUrl:
            # A special case: in case we don't have the PackageHostUrl
            # compiled in, infer it from the first package we
            # installed named "panda3d".
            self.pandaHostUrl = hostUrl

        if not platform:
            platform = None
        package = host.getPackage(name, version, platform = platform)
        if not package:
            if not recurse:
                # Maybe the contents.xml file isn't current.  Re-fetch it.
                if host.redownloadContentsFile(self.http):
                    return self.addPackageInfo(name, platform, version, hostUrl, hostDir = hostDir, recurse = True)

            message = "Couldn't find %s %s on %s" % (name, version, hostUrl)
            raise OSError(message)

        package.checkStatus()
        if not package.downloadDescFile(self.http):
            message = "Couldn't get desc file for %s" % (name)
            raise OSError(message)

        if not package.downloadPackage(self.http):
            message = "Couldn't download %s" % (name)
            raise OSError(message)

        if not package.installPackage(self):
            message = "Couldn't install %s" % (name)
            raise OSError(message)

        if package.guiApp:
            self.guiApp = True
            init_app_for_gui()

    def setP3DFilename(self, p3dFilename, tokens, argv, instanceId,
                       interactiveConsole, p3dOffset = 0, p3dUrl = None):
        """ Called by the browser to specify the p3d file that
        contains the application itself, along with the web tokens
        and/or command-line arguments.  Once this method has been
        called, the application is effectively started. """

        # One day we will have support for multiple instances within a
        # Python session.  Against that day, we save the instance ID
        # for this instance.
        self.instanceId = instanceId

        self.tokens = tokens
        self.argv = argv

        # We build up a token dictionary with care, so that if a given
        # token appears twice in the token list, we record only the
        # first value, not the second or later.  This is consistent
        # with the internal behavior of the core API.
        self.tokenDict = {}
        for token, keyword in tokens:
            self.tokenDict.setdefault(token, keyword)

        # Also store the arguments on sys, for applications that
        # aren't instance-ready.
        sys.argv = argv

        # That means we now know the altHost in effect.
        self.altHost = self.tokenDict.get('alt_host', None)

        # Tell the browser that Python is up and running, and ready to
        # respond to queries.
        self.notifyRequest('onpythonload')

        # Now go load the applet.
        fname = Filename.fromOsSpecific(p3dFilename)
        vfs = VirtualFileSystem.getGlobalPtr()

        if not vfs.exists(fname):
            raise ArgumentError("No such file: %s" % (p3dFilename))

        fname.makeAbsolute()
        fname.setBinary()
        mf = Multifile()
        if p3dOffset == 0:
            if not mf.openRead(fname):
                raise ArgumentError("Not a Panda3D application: %s" % (p3dFilename))
        else:
            if not mf.openRead(fname, p3dOffset):
                raise ArgumentError("Not a Panda3D application: %s at offset: %s" % (p3dFilename, p3dOffset))

        # Now load the p3dInfo file.
        self.p3dInfo = None
        self.p3dPackage = None
        self.p3dConfig = None
        self.allowPythonDev = False

        i = mf.findSubfile('p3d_info.xml')
        if i >= 0 and hasattr(core, 'readXmlStream'):
            stream = mf.openReadSubfile(i)
            self.p3dInfo = core.readXmlStream(stream)
            mf.closeReadSubfile(stream)
        if self.p3dInfo:
            self.p3dPackage = self.p3dInfo.FirstChildElement('package')
        if self.p3dPackage:
            self.p3dConfig = self.p3dPackage.FirstChildElement('config')

            xhost = self.p3dPackage.FirstChildElement('host')
            while xhost:
                self.__readHostXml(xhost)
                xhost = xhost.NextSiblingElement('host')

        if self.p3dConfig:
            allowPythonDev = self.p3dConfig.Attribute('allow_python_dev')
            if allowPythonDev:
                self.allowPythonDev = int(allowPythonDev)
            guiApp = self.p3dConfig.Attribute('gui_app')
            if guiApp:
                self.guiApp = int(guiApp)

            trueFileIO = self.p3dConfig.Attribute('true_file_io')
            if trueFileIO:
                self.trueFileIO = int(trueFileIO)

        # The interactiveConsole flag can only be set true if the
        # application has allow_python_dev set.
        if not self.allowPythonDev and interactiveConsole:
            raise Exception("Impossible, interactive_console set without allow_python_dev.")
        self.interactiveConsole = interactiveConsole

        if self.allowPythonDev:
            # Set the fps text to remind the user that
            # allow_python_dev is enabled.
            ConfigVariableString('frame-rate-meter-text-pattern').setValue('allow_python_dev %0.1f fps')

        if self.guiApp:
            init_app_for_gui()

        self.initPackedAppEnvironment()

        # Mount the Multifile under self.multifileRoot.
        vfs.mount(mf, self.multifileRoot, vfs.MFReadOnly)
        self.p3dMultifile = mf
        VFSImporter.reloadSharedPackages()

        self.loadMultifilePrcFiles(mf, self.multifileRoot)
        self.gotP3DFilename = True
        self.p3dFilename = fname
        if p3dUrl:
            # The url from which the p3d file was downloaded is
            # provided if available.  It is only for documentation
            # purposes; the actual p3d file has already been
            # downloaded to p3dFilename.
            self.p3dUrl = core.URLSpec(p3dUrl)

        # Send this call to the main thread; don't call it directly.
        messenger.send('AppRunner_startIfReady', taskChain = 'default')

    def __readHostXml(self, xhost):
        """ Reads the data in the indicated <host> entry. """

        url = xhost.Attribute('url')
        host = self.getHost(url)
        host.readHostXml(xhost)

        # Scan for a matching <alt_host>.  If found, it means we
        # should use the alternate URL instead of the original URL.
        if self.altHost:
            xalthost = xhost.FirstChildElement('alt_host')
            while xalthost:
                keyword = xalthost.Attribute('keyword')
                if keyword == self.altHost:
                    origUrl = xhost.Attribute('url')
                    newUrl = xalthost.Attribute('url')
                    self.altHostMap[origUrl] = newUrl
                    break

                xalthost = xalthost.NextSiblingElement('alt_host')

    def loadMultifilePrcFiles(self, mf, root):
        """ Loads any prc files in the root of the indicated
        Multifile, which is presumed to have been mounted already
        under root. """

        # We have to load these prc files explicitly, since the
        # ConfigPageManager can't directly look inside the vfs.  Use
        # the Multifile interface to find the prc files, rather than
        # vfs.scanDirectory(), so we only pick up the files in this
        # particular multifile.
        cpMgr = ConfigPageManager.getGlobalPtr()
        for f in mf.getSubfileNames():
            fn = Filename(f)
            if fn.getDirname() == '' and fn.getExtension() == 'prc':
                pathname = '%s/%s' % (root, f)

                alreadyLoaded = False
                for cpi in range(cpMgr.getNumImplicitPages()):
                    if cpMgr.getImplicitPage(cpi).getName() == pathname:
                        # No need to load this file twice.
                        alreadyLoaded = True
                        break

                if not alreadyLoaded:
                    data = file.open(Filename(pathname), 'r').read()
                    cp = loadPrcFileData(pathname, data)
                    # Set it to sort value 20, behind the implicit pages.
                    cp.setSort(20)


    def __clearWindowProperties(self):
        """ Clears the windowPrc file that was created in a previous
        call to setupWindow(), if any. """

        if self.windowPrc:
            unloadPrcFile(self.windowPrc)
            self.windowPrc = None
        WindowProperties.clearDefault()

        # However, we keep the self.windowProperties object around, in
        # case an application wants to return the window to the
        # browser frame.

    def setupWindow(self, windowType, x, y, width, height,
                    parent):
        """ Applies the indicated window parameters to the prc
        settings, for future windows; or applies them directly to the
        main window if the window has already been opened.  This is
        called by the browser. """

        if self.started and base.win:
            # If we've already got a window, this must be a
            # resize/reposition request.
            wp = WindowProperties()
            if x or y or windowType == 'embedded':
                wp.setOrigin(x, y)
            if width or height:
                wp.setSize(width, height)
            if windowType == 'embedded':
                wp.setParentWindow(parent)
            wp.setFullscreen(False)
            base.win.requestProperties(wp)
            self.windowProperties = wp
            return

        # If we haven't got a window already, start 'er up.  Apply the
        # requested setting to the prc file, and to the default
        # WindowProperties structure.

        self.__clearWindowProperties()

        if windowType == 'hidden':
            data = 'window-type none\n'
        else:
            data = 'window-type onscreen\n'

        wp = WindowProperties.getDefault()

        wp.clearParentWindow()
        wp.clearOrigin()
        wp.clearSize()

        wp.setFullscreen(False)
        if windowType == 'fullscreen':
            wp.setFullscreen(True)

        if windowType == 'embedded':
            wp.setParentWindow(parent)

        if x or y or windowType == 'embedded':
            wp.setOrigin(x, y)

        if width or height:
            wp.setSize(width, height)

        self.windowProperties = wp
        self.windowPrc = loadPrcFileData("setupWindow", data)
        WindowProperties.setDefault(wp)

        self.gotWindow = True

        # Send this call to the main thread; don't call it directly.
        messenger.send('AppRunner_startIfReady', taskChain = 'default')

    def setRequestFunc(self, func):
        """ This method is called by the browser at startup to supply a
        function that can be used to deliver requests upstream, to the
        core API, and thereby to the browser. """
        self.requestFunc = func

    def sendRequest(self, request, *args):
        """ Delivers a request to the browser via self.requestFunc.
        This low-level function is not intended to be called directly
        by user code. """

        assert self.requestFunc
        return self.requestFunc(self.instanceId, request, args)

    def __windowEvent(self, win):
        """ This method is called when we get a window event.  We
        listen for this to detect when the window has been
        successfully opened. """

        if not self.windowOpened:
            self.windowOpened = True

            # Now that the window is open, we don't need to keep those
            # prc settings around any more.
            self.__clearWindowProperties()

            # Inform the plugin and browser.
            self.notifyRequest('onwindowopen')

    def notifyRequest(self, message):
        """ Delivers a notify request to the browser.  This is a "this
        happened" type notification; it also triggers some JavaScript
        code execution, if indicated in the HTML tags, and may also
        trigger some internal automatic actions.  (For instance, the
        plugin takes down the splash window when it sees the
        onwindowopen notification. """

        self.sendRequest('notify', message.lower())

    def evalScript(self, expression, needsResponse = False):
        """ Evaluates an arbitrary JavaScript expression in the global
        DOM space.  This may be deferred if necessary if needsResponse
        is False and self.dom has not yet been assigned.  If
        needsResponse is true, this waits for the value and returns
        it, which means it cannot be deferred. """

        if not self.dom:
            # Defer the expression.
            assert not needsResponse
            self.deferredEvals.append(expression)
        else:
            # Evaluate it now.
            return self.scriptRequest('eval', self.dom, value = expression,
                                      needsResponse = needsResponse)

    def scriptRequest(self, operation, object, propertyName = '',
                      value = None, needsResponse = True):
        """ Issues a new script request to the browser.  This queries
        or modifies one of the browser's DOM properties.  This is a
        low-level method that user code should not call directly;
        instead, just operate on the Python wrapper objects that
        shadow the DOM objects, beginning with appRunner.dom.

        operation may be one of [ 'get_property', 'set_property',
        'call', 'evaluate' ].

        object is the browser object to manipulate, or the scope in
        which to evaluate the expression.

        propertyName is the name of the property to manipulate, if
        relevant (set to None for the default method name).

        value is the new value to assign to the property for
        set_property, or the parameter list for call, or the string
        expression for evaluate.

        If needsResponse is true, this method will block until the
        return value is received from the browser, and then it returns
        that value.  Otherwise, it returns None immediately, without
        waiting for the browser to process the request.
        """
        uniqueId = self.nextScriptId
        self.nextScriptId = (self.nextScriptId + 1) % 0xffffffff
        self.sendRequest('script', operation, object,
                         propertyName, value, needsResponse, uniqueId)

        if needsResponse:
            # Now wait for the response to come in.
            result = self.sendRequest('wait_script_response', uniqueId)
            return result

    def dropObject(self, objectId):
        """ Inform the parent process that we no longer have an
        interest in the P3D_object corresponding to the indicated
        objectId.  Not intended to be called by user code. """

        self.sendRequest('drop_p3dobj', objectId)

def dummyAppRunner(tokens = [], argv = None):
    """ This function creates a dummy global AppRunner object, which
    is useful for testing running in a packaged environment without
    actually bothering to package up the application.  Call this at
    the start of your application to enable it.

    It places the current working directory under /mf, as if it were
    mounted from a packed multifile.  It doesn't convert egg files to
    bam files, of course; and there are other minor differences from
    running in an actual packaged environment.  But it can be a useful
    first-look sanity check. """

    if AppRunnerGlobal.appRunner:
        print("Already have AppRunner, not creating a new one.")
        return AppRunnerGlobal.appRunner

    appRunner = AppRunner()
    appRunner.dummy = True
    AppRunnerGlobal.appRunner = appRunner

    platform = PandaSystem.getPlatform()
    version = PandaSystem.getPackageVersionString()
    hostUrl = PandaSystem.getPackageHostUrl()

    if platform.startswith('win'):
        rootDir = Filename(Filename.getUserAppdataDirectory(), 'Panda3D')
    elif platform.startswith('osx'):
        rootDir = Filename(Filename.getHomeDirectory(), 'Library/Caches/Panda3D')
    else:
        rootDir = Filename(Filename.getHomeDirectory(), '.panda3d')

    appRunner.rootDir = rootDir
    appRunner.logDirectory = Filename(rootDir, 'log')

    # Of course we will have the panda3d application loaded.
    appRunner.addPackageInfo('panda3d', platform, version, hostUrl)

    appRunner.tokens = tokens
    appRunner.tokenDict = dict(tokens)
    if argv is None:
        argv = sys.argv
    appRunner.argv = argv
    appRunner.altHost = appRunner.tokenDict.get('alt_host', None)

    appRunner.p3dInfo = None
    appRunner.p3dPackage = None

    # Mount the current directory under the multifileRoot, as if it
    # were coming from a multifile.
    cwd = ExecutionEnvironment.getCwd()
    vfs = VirtualFileSystem.getGlobalPtr()
    vfs.mount(cwd, appRunner.multifileRoot, vfs.MFReadOnly)

    appRunner.initPackedAppEnvironment()

    return appRunner

