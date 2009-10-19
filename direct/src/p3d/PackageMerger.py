from direct.p3d.FileSpec import FileSpec
from pandac.PandaModules import *
import copy
import shutil

class PackageMergerError(StandardError):
    pass

class PackageMerger:
    """ This class will combine two or more separately-build stage
    directories, the output of Packager.py or the ppackage tool, into
    a single output directory.  It assumes that the clocks on all
    hosts are in sync, so that the file across all builds with the
    most recent timestamp (indicated in the contents.xml file) is
    always the most current version of the file. """
 
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

            self.descFile = FileSpec()
            self.descFile.loadXml(xpackage)

            self.importDescFile = None
            ximport = xpackage.FirstChildElement('import')
            if ximport:
                self.importDescFile = FileSpec()
                self.importDescFile.loadXml(ximport)

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

            self.descFile.storeXml(xpackage)

            if self.importDescFile:
                ximport = TiXmlElement('import')
                self.importDescFile.storeXml(ximport)
                xpackage.InsertEndChild(ximport)
            
            return xpackage

    # PackageMerger constructor
    def __init__(self, installDir):
        self.installDir = installDir
        self.xhost = None
        self.contents = {}

        # We allow the first one to fail quietly.
        self.__readContentsFile(self.installDir)

    def __readContentsFile(self, sourceDir):
        """ Reads the contents.xml file from the indicated sourceDir,
        and updates the internal set of packages appropriately. """

        contentsFilename = Filename(sourceDir, 'contents.xml')
        doc = TiXmlDocument(contentsFilename.toOsSpecific())
        if not doc.LoadFile():
            # Couldn't read file.
            return False

        xcontents = doc.FirstChildElement('contents')
        if xcontents:
            xhost = xcontents.FirstChildElement('host')
            if xhost:
                self.xhost = xhost.Clone()
                
            xpackage = xcontents.FirstChildElement('package')
            while xpackage:
                pe = self.PackageEntry(xpackage, sourceDir)
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

        contents = self.contents.items()
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
        print "copying %s" % (dirname)
        sourceDirname = Filename(pe.sourceDir, dirname)
        targetDirname = Filename(self.installDir, dirname)

        if targetDirname.exists():
            # The target directory already exists; we have to clean it
            # out first.
            shutil.rmtree(targetDirname.toOsSpecific())

        shutil.copytree(sourceDirname.toOsSpecific(), targetDirname.toOsSpecific())
        


    def merge(self, sourceDir):
        """ Adds the contents of the indicated source directory into
        the current pool. """
        if not self.__readContentsFile(sourceDir):
            message = "Couldn't read %s" % (sourceDir)
            raise PackageMergerError, message            

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

        self.__writeContentsFile()
        
