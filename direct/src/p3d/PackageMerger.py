"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["PackageMerger", "PackageMergerError"]

from direct.p3d.FileSpec import FileSpec
from direct.p3d.SeqValue import SeqValue
from direct.directnotify.DirectNotifyGlobal import *
from panda3d.core import *
import shutil
import os

class PackageMergerError(Exception):
    pass

class PackageMerger:
    """ This class will combine two or more separately-built stage
    directories, the output of Packager.py or the ppackage tool, into
    a single output directory.  It assumes that the clocks on all
    hosts are in sync, so that the file across all builds with the
    most recent timestamp (indicated in the contents.xml file) is
    always the most current version of the file. """

    notify = directNotify.newCategory("PackageMerger")

    class PackageEntry:
        """ This corresponds to a <package> entry in the contents.xml
        file. """

        def __init__(self, xpackage, sourceDir):
            self.sourceDir = sourceDir
            self.loadXml(xpackage)

        def getKey(self):
            """ Returns a tuple used for sorting the PackageEntry
            objects uniquely per package. """
            return (self.packageName, self.platform, self.version)

        def isNewer(self, other):
            return self.descFile.timestamp > other.descFile.timestamp

        def loadXml(self, xpackage):
            self.packageName = xpackage.Attribute('name')
            self.platform = xpackage.Attribute('platform')
            self.version = xpackage.Attribute('version')
            solo = xpackage.Attribute('solo')
            self.solo = int(solo or '0')
            perPlatform = xpackage.Attribute('per_platform')
            self.perPlatform = int(perPlatform or '0')

            self.descFile = FileSpec()
            self.descFile.loadXml(xpackage)

            self.validatePackageContents()

            self.descFile.quickVerify(packageDir = self.sourceDir, notify = PackageMerger.notify, correctSelf = True)

            self.packageSeq = SeqValue()
            self.packageSeq.loadXml(xpackage, 'seq')
            self.packageSetVer = SeqValue()
            self.packageSetVer.loadXml(xpackage, 'set_ver')

            self.importDescFile = None
            ximport = xpackage.FirstChildElement('import')
            if ximport:
                self.importDescFile = FileSpec()
                self.importDescFile.loadXml(ximport)
                self.importDescFile.quickVerify(packageDir = self.sourceDir, notify = PackageMerger.notify, correctSelf = True)

        def makeXml(self):
            """ Returns a new TiXmlElement. """
            xpackage = TiXmlElement('package')
            xpackage.SetAttribute('name', self.packageName)
            if self.platform:
                xpackage.SetAttribute('platform', self.platform)
            if self.version:
                xpackage.SetAttribute('version', self.version)
            if self.solo:
                xpackage.SetAttribute('solo', '1')
            if self.perPlatform:
                xpackage.SetAttribute('per_platform', '1')

            self.descFile.storeXml(xpackage)
            self.packageSeq.storeXml(xpackage, 'seq')
            self.packageSetVer.storeXml(xpackage, 'set_ver')

            if self.importDescFile:
                ximport = TiXmlElement('import')
                self.importDescFile.storeXml(ximport)
                xpackage.InsertEndChild(ximport)

            return xpackage

        def validatePackageContents(self):
            """ Validates the contents of the package directory itself
            against the expected hashes and timestamps.  Updates
            hashes and timestamps where needed. """

            if self.solo:
                return

            needsChange = False
            packageDescFullpath = Filename(self.sourceDir, self.descFile.filename)
            packageDir = Filename(packageDescFullpath.getDirname())
            doc = TiXmlDocument(packageDescFullpath.toOsSpecific())
            if not doc.LoadFile():
                message = "Could not read XML file: %s" % (self.descFile.filename)
                raise OSError(message)

            xpackage = doc.FirstChildElement('package')
            if not xpackage:
                message = "No package definition: %s" % (self.descFile.filename)
                raise OSError(message)

            xcompressed = xpackage.FirstChildElement('compressed_archive')
            if xcompressed:
                spec = FileSpec()
                spec.loadXml(xcompressed)
                if not spec.quickVerify(packageDir = packageDir, notify = PackageMerger.notify, correctSelf = True):
                    spec.storeXml(xcompressed)
                    needsChange = True

            xpatch = xpackage.FirstChildElement('patch')
            while xpatch:
                spec = FileSpec()
                spec.loadXml(xpatch)
                if not spec.quickVerify(packageDir = packageDir, notify = PackageMerger.notify, correctSelf = True):
                    spec.storeXml(xpatch)
                    needsChange = True

                xpatch = xpatch.NextSiblingElement('patch')

            if needsChange:
                PackageMerger.notify.info("Rewriting %s" % (self.descFile.filename))
                doc.SaveFile()
                self.descFile.quickVerify(packageDir = self.sourceDir, notify = PackageMerger.notify, correctSelf = True)

    # PackageMerger constructor
    def __init__(self, installDir):
        self.installDir = installDir
        self.xhost = None
        self.contents = {}
        self.maxAge = None
        self.contentsSeq = SeqValue()

        # We allow the first one to fail quietly.
        self.__readContentsFile(self.installDir, None)

    def __readContentsFile(self, sourceDir, packageNames):
        """ Reads the contents.xml file from the indicated sourceDir,
        and updates the internal set of packages appropriately. """

        assert sourceDir != None, "No source directory was specified!"
        contentsFilename = Filename(sourceDir, 'contents.xml')
        doc = TiXmlDocument(contentsFilename.toOsSpecific())
        if not doc.LoadFile():
            # Couldn't read file.
            return False

        xcontents = doc.FirstChildElement('contents')
        if xcontents:
            maxAge = xcontents.Attribute('max_age')
            if maxAge:
                maxAge = int(maxAge)
                if self.maxAge is None:
                    self.maxAge = maxAge
                else:
                    self.maxAge = min(self.maxAge, maxAge)

            contentsSeq = SeqValue()
            if contentsSeq.loadXml(xcontents):
                self.contentsSeq = max(self.contentsSeq, contentsSeq)

            xhost = xcontents.FirstChildElement('host')
            if xhost:
                self.xhost = xhost.Clone()

            xpackage = xcontents.FirstChildElement('package')
            while xpackage:
                pe = self.PackageEntry(xpackage, sourceDir)

                # Filter out any packages not listed in
                # packageNames (unless packageNames is None,
                # in which case don't filter anything).
                if packageNames is None or pe.packageName in packageNames:
                    other = self.contents.get(pe.getKey(), None)
                    if not other or pe.isNewer(other):
                        # Store this package in the resulting output.
                        self.contents[pe.getKey()] = pe

                xpackage = xpackage.NextSiblingElement('package')

        self.contentsDoc = doc

        return True

    def __writeContentsFile(self):
        """ Writes the contents.xml file at the end of processing. """

        filename = Filename(self.installDir, 'contents.xml')
        doc = TiXmlDocument(filename.toOsSpecific())
        decl = TiXmlDeclaration("1.0", "utf-8", "")
        doc.InsertEndChild(decl)

        xcontents = TiXmlElement('contents')
        if self.xhost:
            xcontents.InsertEndChild(self.xhost)

        if self.maxAge is not None:
            xcontents.SetAttribute('max_age', str(self.maxAge))
        self.contentsSeq.storeXml(xcontents)

        contents = list(self.contents.items())
        contents.sort()
        for key, pe in contents:
            xpackage = pe.makeXml()
            xcontents.InsertEndChild(xpackage)

        doc.InsertEndChild(xcontents)
        doc.SaveFile()

    def __copySubdirectory(self, pe):
        """ Copies the subdirectory referenced in the indicated
        PackageEntry object into the installDir, replacing the
        contents of any similarly-named subdirectory already
        there. """

        dirname = Filename(pe.descFile.filename).getDirname()
        self.notify.info("copying %s" % (dirname))
        sourceDirname = Filename(pe.sourceDir, dirname)
        targetDirname = Filename(self.installDir, dirname)

        self.__rCopyTree(sourceDirname, targetDirname)

    def __rCopyTree(self, sourceFilename, targetFilename):
        """ Recursively copies the contents of sourceDirname onto
        targetDirname.  This behaves like shutil.copytree, but it does
        not remove pre-existing subdirectories. """

        if targetFilename.exists():
            if not targetFilename.isDirectory():
                # Delete any regular files in the way.
                targetFilename.unlink()

            elif not sourceFilename.isDirectory():
                # If the source file is a regular file, but the target
                # file is a directory, completely remove the target
                # file.
                shutil.rmtree(targetFilename.toOsSpecific())

            else:
                # Both the source file and target file are
                # directories.

                # We have to clean out the target directory first.
                # Instead of using shutil.rmtree(), remove the files in
                # this directory one at a time, so we don't inadvertently
                # clean out subdirectories too.
                files = os.listdir(targetFilename.toOsSpecific())
                for file in files:
                    f = Filename(targetFilename, file)
                    if f.isRegularFile():
                        f.unlink()

        if sourceFilename.isDirectory():
            # Recursively copying a directory.
            Filename(targetFilename, '').makeDir()
            files = os.listdir(sourceFilename.toOsSpecific())
            for file in files:
                self.__rCopyTree(Filename(sourceFilename, file),
                                 Filename(targetFilename, file))
        else:
            # Copying a regular file.
            sourceFilename.copyTo(targetFilename)

            # Also try to copy the timestamp, but don't fuss too much
            # if it doesn't work.
            try:
                st = os.stat(sourceFilename.toOsSpecific())
                os.utime(targetFilename.toOsSpecific(), (st.st_atime, st.st_mtime))
            except OSError:
                pass

    def merge(self, sourceDir, packageNames = None):
        """ Adds the contents of the indicated source directory into
        the current pool.  If packageNames is not None, it is a list
        of package names that we wish to include from the source;
        packages not named in this list will be unchanged. """

        if not self.__readContentsFile(sourceDir, packageNames):
            message = "Couldn't read %s" % (sourceDir)
            raise PackageMergerError(message)

    def close(self):
        """ Finalizes the results of all of the previous calls to
        merge(), writes the new contents.xml file, and copies in all
        of the new contents. """

        dirname = Filename(self.installDir, '')
        dirname.makeDir()

        for pe in self.contents.values():
            if pe.sourceDir != self.installDir:
                # Here's a new subdirectory we have to copy in.
                self.__copySubdirectory(pe)

        self.contentsSeq += 1
        self.__writeContentsFile()

