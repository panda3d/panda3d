""" This module is used to build a "Package", a collection of files
within a Panda3D Multifile, which can be easily be downloaded and/or
patched onto a client machine, for the purpose of running a large
application. """

import sys
import os
import glob
import marshal
import new
import string
from direct.showbase import Loader
from direct.showutil import FreezeTool
from direct.directnotify.DirectNotifyGlobal import *
from pandac.PandaModules import *

vfs = VirtualFileSystem.getGlobalPtr()

class PackagerError(StandardError):
    pass

class OutsideOfPackageError(PackagerError):
    pass

class ArgumentError(PackagerError):
    pass

class Packager:
    notify = directNotify.newCategory("Packager")

    class PackFile:
        def __init__(self, filename, newName = None, deleteTemp = False,
                     extract = None):
            assert isinstance(filename, Filename)
            self.filename = filename
            self.newName = newName
            self.deleteTemp = deleteTemp
            self.extract = extract

    class Package:
        def __init__(self, packageName, packager):
            self.packageName = packageName
            self.packager = packager
            self.version = None
            self.platform = None
            self.p3dApplication = False
            self.displayName = None
            self.files = []
            self.compressionLevel = 0
            self.importedMapsDir = 'imported_maps'
            self.mainModule = None
            self.requires = []

            # This is the set of files and modules, already included
            # by required packages, that we can skip.
            self.skipFilenames = {}
            self.skipModules = {}

            # This records the current list of modules we have added so
            # far.
            self.freezer = FreezeTool.Freezer()

            # Set this true to parse and build up the internal
            # filelist, but not generate any output.
            self.dryRun = False

        def close(self):
            """ Writes out the contents of the current package. """

            if not self.p3dApplication and not self.version:
                # We must have a version string for packages.
                self.version = '0.0'

            self.packageBasename = self.packageName
            packageDir = self.packageName
            if self.platform:
                self.packageBasename += '_' + self.platform
                packageDir += '/' + self.platform
            if self.version:
                self.packageBasename += '_' + self.version
                packageDir += '/' + self.version

            self.packageDesc = self.packageBasename + '.xml'
            self.packageImportDesc = self.packageBasename + '_import.xml'
            if self.p3dApplication:
                self.packageBasename += '.p3d'
                packageDir = ''
            else:
                self.packageBasename += '.mf'
                packageDir += '/'

            self.packageFilename = packageDir + self.packageBasename
            self.packageDesc = packageDir + self.packageDesc
            self.packageImportDesc = packageDir + self.packageImportDesc

            self.packageFullpath = Filename(self.packager.installDir, self.packageFilename)
            self.packageFullpath.makeDir()
            self.packageFullpath.unlink()

            if self.dryRun:
                self.multifile = None
            else:
                self.multifile = Multifile()
                self.multifile.openReadWrite(self.packageFullpath)

            self.extracts = []
            self.components = []

            # Exclude modules already imported in a required package.
            for moduleName in self.skipModules.keys():
                self.freezer.excludeModule(moduleName)

            # Build up a cross-reference of files we've already
            # discovered.
            self.sourceFilenames = {}
            self.targetFilenames = {}
            processFiles = []
            for file in self.files:
                if not file.newName:
                    file.newName = file.filename
                if file.newName in self.skipFilenames:
                    # Skip this file.
                    continue

                # Convert the source filename to an unambiguous
                # filename for searching.
                filename = Filename(file.filename)
                filename.makeCanonical()
                
                self.sourceFilenames[filename] = file
                self.targetFilenames[file.newName] = file
                processFiles.append(file)

            for file in processFiles:
                ext = file.filename.getExtension()
                if ext == 'py':
                    self.addPyFile(file)
                elif not self.dryRun:
                    if ext == 'pz':
                        # Strip off an implicit .pz extension.
                        filename = Filename(file.filename)
                        filename.setExtension('')
                        filename = Filename(filename.cStr())
                        ext = filename.getExtension()

                        filename = Filename(file.newName)
                        if filename.getExtension() == 'pz':
                            filename.setExtension('')
                            file.newName = filename.cStr()

                    if ext == 'egg':
                        self.addEggFile(file)
                    elif ext == 'bam':
                        self.addBamFile(file)
                    elif ext in self.packager.imageExtensions:
                        self.addTexture(file)
                    else:
                        # Any other file.
                        self.addComponent(file)

            if not self.mainModule and self.p3dApplication:
                message = 'No main_module specified for application %s' % (self.packageName)
                raise PackagerError, message
            if self.mainModule:
                if self.mainModule not in self.freezer.modules:
                    self.freezer.addModule(self.mainModule)

            # Pick up any unfrozen Python files.
            self.freezer.done()

            # Add known module names.
            self.moduleNames = {}
            for moduleName in self.freezer.getAllModuleNames():
                if moduleName == '__main__':
                    # Ignore this special case.
                    continue
                
                self.moduleNames[moduleName] = True

                xmodule = TiXmlElement('module')
                xmodule.SetAttribute('name', moduleName)
                self.components.append(xmodule)

            if not self.dryRun:
                self.freezer.addToMultifile(self.multifile, self.compressionLevel)
                if self.p3dApplication:
                    self.makeP3dInfo()
                self.multifile.repack()
                self.multifile.close()

                if not self.p3dApplication:
                    self.compressMultifile()
                    self.writeDescFile()
                    self.writeImportDescFile()

            # Now that all the files have been packed, we can delete
            # the temporary files.
            for file in self.files:
                if file.deleteTemp:
                    file.filename.unlink()

        def makeP3dInfo(self):
            """ Makes the p3d_info.xml file that defines the
            application startup parameters and such. """

            doc = TiXmlDocument()
            decl = TiXmlDeclaration("1.0", "utf-8", "")
            doc.InsertEndChild(decl)

            xpackage = TiXmlElement('package')
            xpackage.SetAttribute('name', self.packageName)
            if self.platform:
                xpackage.SetAttribute('platform', self.platform)
            if self.version:
                xpackage.SetAttribute('version', self.version)

            if self.displayName:
                xpackage.SetAttribute('display_name', self.displayName)

            xpackage.SetAttribute('main_module', self.mainModule)

            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                xpackage.InsertEndChild(xrequires)

            doc.InsertEndChild(xpackage)

            # Write the xml file to a temporary file on disk, so we
            # can add it to the multifile.
            filename = Filename.temporary('', 'p3d_', '.xml')
            doc.SaveFile(filename.toOsSpecific())

            # It's important not to compress this file: the core API
            # runtime can't decode compressed subfiles.
            self.multifile.addSubfile('p3d_info.xml', filename, 0)
            
            self.multifile.flush()
            filename.unlink()
            

        def compressMultifile(self):
            """ Compresses the .mf file into an .mf.pz file. """

            compressedName = self.packageFilename + '.pz'
            compressedPath = Filename(self.packager.installDir, compressedName)
            if not compressFile(self.packageFullpath, compressedPath, 6):
                message = 'Unable to write %s' % (compressedPath)
                raise PackagerError, message

        def writeDescFile(self):
            packageDescFullpath = Filename(self.packager.installDir, self.packageDesc)
            doc = TiXmlDocument(packageDescFullpath.toOsSpecific())
            decl = TiXmlDeclaration("1.0", "utf-8", "")
            doc.InsertEndChild(decl)

            xpackage = TiXmlElement('package')
            xpackage.SetAttribute('name', self.packageName)
            if self.platform:
                xpackage.SetAttribute('platform', self.platform)
            if self.version:
                xpackage.SetAttribute('version', self.version)

            if self.displayName:
                xpackage.SetAttribute('display_name', self.displayName)

            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if package.platform:
                    xrequires.SetAttribute('platform', package.platform)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                xpackage.InsertEndChild(xrequires)

            xuncompressedArchive = self.getFileSpec(
                'uncompressed_archive', self.packageFullpath,
                self.packageBasename)
            xcompressedArchive = self.getFileSpec(
                'compressed_archive', self.packageFullpath + '.pz',
                self.packageBasename + '.pz')
            xpackage.InsertEndChild(xuncompressedArchive)
            xpackage.InsertEndChild(xcompressedArchive)

            for xextract in self.extracts:
                xpackage.InsertEndChild(xextract)

            doc.InsertEndChild(xpackage)
            doc.SaveFile()

        def writeImportDescFile(self):
            packageImportDescFullpath = Filename(self.packager.installDir, self.packageImportDesc)
            doc = TiXmlDocument(packageImportDescFullpath.toOsSpecific())
            decl = TiXmlDeclaration("1.0", "utf-8", "")
            doc.InsertEndChild(decl)

            xpackage = TiXmlElement('package')
            xpackage.SetAttribute('name', self.packageName)
            if self.platform:
                xpackage.SetAttribute('platform', self.platform)
            if self.version:
                xpackage.SetAttribute('version', self.version)

            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if package.platform:
                    xrequires.SetAttribute('platform', package.platform)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                xpackage.InsertEndChild(xrequires)

            for xcomponent in self.components:
                xpackage.InsertEndChild(xcomponent)

            doc.InsertEndChild(xpackage)
            doc.SaveFile()

        def readImportDescFile(self, filename):
            """ Reads the import desc file.  Returns True on success,
            False on failure. """

            doc = TiXmlDocument(filename.toOsSpecific())
            if not doc.LoadFile():
                return False
            xpackage = doc.FirstChildElement('package')
            if not xpackage:
                return False

            self.packageName = xpackage.Attribute('name')
            self.platform = xpackage.Attribute('platform')
            self.version = xpackage.Attribute('version')

            self.requires = []
            xrequires = xpackage.FirstChildElement('requires')
            while xrequires:
                packageName = xrequires.Attribute('name')
                platform = xrequires.Attribute('platform')
                version = xrequires.Attribute('version')
                if packageName:
                    package = self.packager.findPackage(packageName, platform = platform, version = version, requires = self.requires)
                    if package:
                        self.requires.append(package)
                xrequires = xrequires.NextSiblingElement()

            self.targetFilenames = {}
            xcomponent = xpackage.FirstChildElement('component')
            while xcomponent:
                name = xcomponent.Attribute('filename')
                if name:
                    self.targetFilenames[name] = True
                xcomponent = xcomponent.NextSiblingElement()

            self.moduleNames = {}
            xmodule = xpackage.FirstChildElement('module')
            while xmodule:
                moduleName = xmodule.Attribute('name')
                if moduleName:
                    self.moduleNames[moduleName] = True
                xmodule = xmodule.NextSiblingElement()

            return True

        def getFileSpec(self, element, pathname, newName):
            """ Returns an xcomponent or similar element with the file
            information for the indicated file. """
            
            xspec = TiXmlElement(element)

            size = pathname.getFileSize()
            timestamp = pathname.getTimestamp()

            hv = HashVal()
            hv.hashFile(pathname)
            hash = hv.asHex()

            xspec.SetAttribute('filename', newName)
            xspec.SetAttribute('size', str(size))
            xspec.SetAttribute('timestamp', str(timestamp))
            xspec.SetAttribute('hash', hash)

            return xspec

            

        def addPyFile(self, file):
            """ Adds the indicated python file, identified by filename
            instead of by module name, to the package. """

            # Convert the raw filename back to a module name, so we
            # can see if we've already loaded this file.  We assume
            # that all Python files within the package will be rooted
            # at the top of the package.

            filename = file.newName.rsplit('.', 1)[0]
            moduleName = filename.replace("/", ".")
            if moduleName.endswith('.__init__'):
                moduleName = moduleName.rsplit('.', 1)[0]

            if moduleName in self.freezer.modules:
                # This Python file is already known.  We don't have to
                # deal with it again.
                return

            self.freezer.addModule(moduleName, newName = moduleName,
                                   filename = file.filename)

        def addEggFile(self, file):
            # Precompile egg files to bam's.
            np = self.packager.loader.loadModel(file.filename, okMissing = True)
            if not np:
                raise StandardError, 'Could not read egg file %s' % (file.filename)

            bamName = Filename(file.newName)
            bamName.setExtension('bam')
            self.addNode(np.node(), file.filename, bamName.cStr())

        def addBamFile(self, file):
            # Load the bam file so we can massage its textures.
            bamFile = BamFile()
            if not bamFile.openRead(file.filename):
                raise StandardError, 'Could not read bam file %s' % (file.filename)

            if not bamFile.resolve():
                raise StandardError, 'Could not resolve bam file %s' % (file.filename)

            node = bamFile.readNode()
            if not node:
                raise StandardError, 'Not a model file: %s' % (file.filename)

            self.addNode(node, file.filename, file.newName)

        def addNode(self, node, filename, newName):
            """ Converts the indicated node to a bam stream, and adds the
            bam file to the multifile under the indicated newName. """

            # If the Multifile already has a file by this name, don't
            # bother adding it again.
            if self.multifile.findSubfile(newName) >= 0:
                return

            # Be sure to import all of the referenced textures, and tell
            # them their new location within the multifile.

            for tex in NodePath(node).findAllTextures():
                if not tex.hasFullpath() and tex.hasRamImage():
                    # We need to store this texture as a raw-data image.
                    # Clear the newName so this will happen
                    # automatically.
                    tex.clearFilename()
                    tex.clearAlphaFilename()

                else:
                    # We can store this texture as a file reference to its
                    # image.  Copy the file into our multifile, and rename
                    # its reference in the texture.
                    if tex.hasFilename():
                        tex.setFilename(self.addFoundTexture(tex.getFullpath()))
                    if tex.hasAlphaFilename():
                        tex.setAlphaFilename(self.addFoundTexture(tex.getAlphaFullpath()))

            # Now generate an in-memory bam file.  Tell the bam writer to
            # keep the textures referenced by their in-multifile path.
            bamFile = BamFile()
            stream = StringStream()
            bamFile.openWrite(stream)
            bamFile.getWriter().setFileTextureMode(bamFile.BTMUnchanged)
            bamFile.writeObject(node)
            bamFile.close()

            # Clean the node out of memory.
            node.removeAllChildren()

            # Now we have an in-memory bam file.
            stream.seekg(0)
            self.multifile.addSubfile(newName, stream, self.compressionLevel)

            # Flush it so the data gets written to disk immediately, so we
            # don't have to keep it around in ram.
            self.multifile.flush()
            
            xcomponent = TiXmlElement('component')
            xcomponent.SetAttribute('filename', newName)
            self.components.append(xcomponent)

        def addFoundTexture(self, filename):
            """ Adds the newly-discovered texture to the output, if it has
            not already been included.  Returns the new name within the
            package tree. """

            assert not filename.isLocal()

            filename = Filename(filename)
            filename.makeCanonical()

            file = self.sourceFilenames.get(filename, None)
            if file:
                # Never mind, it's already on the list.
                return file.newName

            # We have to copy the image into the plugin tree somewhere.
            newName = self.importedMapsDir + '/' + filename.getBasename()
            uniqueId = 0
            while newName in self.targetFilenames:
                uniqueId += 1
                newName = '%s/%s_%s.%s' % (
                    self.importedMapsDir, filename.getBasenameWoExtension(),
                    uniqueId, filename.getExtension())

            file = Packager.PackFile(filename, newName = newName)
            self.sourceFilenames[filename] = file
            self.targetFilenames[newName] = file
            self.addTexture(file)

            return newName

        def addTexture(self, file):
            """ Adds a texture image to the output. """

            if self.multifile.findSubfile(file.newName) >= 0:
                # Already have this texture.
                return

            # Texture file formats are generally already compressed and
            # not further compressible.
            self.addComponent(file, compressible = False)

        def addComponent(self, file, compressible = True, extract = None):
            ext = Filename(file.newName).getExtension()
            if ext in self.packager.uncompressibleExtensions:
                compressible = False

            extract = file.extract
            if extract is None and ext in self.packager.extractExtensions:
                extract = True

            if ext in self.packager.platformSpecificExtensions:
                if not self.platform:
                    self.platform = PandaSystem.getPlatform()
                
            compressionLevel = 0
            if compressible:
                compressionLevel = self.compressionLevel
                
            self.multifile.addSubfile(file.newName, file.filename, compressionLevel)
            if extract:
                xextract = self.getFileSpec('extract', file.filename, file.newName)
                self.extracts.append(xextract)

            xcomponent = TiXmlElement('component')
            xcomponent.SetAttribute('filename', file.newName)
            self.components.append(xcomponent)

        def requirePackage(self, package):
            """ Indicates a dependency on the given package.  This
            also implicitly requires all of the package's requirements
            as well. """

            for p2 in package.requires + [package]:
                if p2 not in self.requires:
                    self.requires.append(p2)
                    for filename in p2.targetFilenames.keys():
                        self.skipFilenames[filename] = True
                    for moduleName in p2.moduleNames.keys():
                        self.skipModules[moduleName] = True

    def __init__(self):

        # The following are config settings that the caller may adjust
        # before calling any of the command methods.

        # These should each be a Filename, or None if they are not
        # filled in.
        self.installDir = None
        self.persistDir = None

        # A search list of directories and/or URL's to search for
        # installed packages.
        self.installSearch = []

        # The platform string.
        self.platform = PandaSystem.getPlatform()

        # Optional signing and encrypting features.
        self.encryptionKey = None
        self.prcEncryptionKey = None
        self.prcSignCommand = None

        # This is a list of filename extensions and/or basenames that
        # indicate files that should be encrypted within the
        # multifile.  This provides obfuscation only, not real
        # security, since the decryption key must be part of the
        # client and is therefore readily available to any hacker.
        # Not only is this feature useless, but using it also
        # increases the size of your patchfiles, since encrypted files
        # don't patch as tightly as unencrypted files.  But it's here
        # if you really want it.
        self.encryptExtensions = ['ptf', 'dna', 'txt', 'dc']
        self.encryptFiles = []

        # This is the list of DC import suffixes that should be
        # available to the client.  Other suffixes, like AI and UD,
        # are server-side only and should be ignored by the Scrubber.
        self.dcClientSuffixes = ['OV']

        # Get the list of filename extensions that are recognized as
        # image files.
        self.imageExtensions = []
        for type in PNMFileTypeRegistry.getGlobalPtr().getTypes():
            self.imageExtensions += type.getExtensions()

        # Other useful extensions.  The .pz extension is implicitly
        # stripped.

        # Model files.
        self.modelExtensions = [ 'egg', 'bam' ]

        # Text files that are copied (and compressed) to the package
        # without processing.
        self.textExtensions = [ 'prc', 'ptf', 'txt' ]

        # Binary files that are copied (and compressed) without
        # processing.
        self.binaryExtensions = [ 'ttf', 'wav', 'mid' ]

        # Files that should be extracted to disk.
        self.extractExtensions = [ 'dll', 'so', 'dylib', 'exe' ]

        # Files that indicate a platform dependency.
        self.platformSpecificExtensions = [ 'dll', 'so', 'dylib', 'exe' ]

        # Binary files that are considered uncompressible, and are
        # copied without compression.
        self.uncompressibleExtensions = [ 'mp3', 'ogg' ]

        # A Loader for loading models.
        self.loader = Loader.Loader(self)
        self.sfxManagerList = None
        self.musicManager = None

        # This is filled in during readPackageDef().
        self.packageList = []

        # A table of all known packages by name.
        self.packages = {}

        self.dryRun = False


    def setup(self):
        """ Call this method to initialize the class after filling in
        some of the values in the constructor. """

        self.knownExtensions = self.imageExtensions + self.modelExtensions + self.textExtensions + self.binaryExtensions + self.uncompressibleExtensions

        self.currentPackage = None

        # We must have an actual install directory.
        assert(self.installDir)

##         # If the persist dir names an empty or nonexistent directory,
##         # we will be generating a brand new publish with no previous
##         # patches.
##         self.persistDir.makeDir()

##         # Within the persist dir, we make a temporary holding dir for
##         # generating multifiles.
##         self.mfTempDir = Filename(self.persistDir, Filename('mftemp/'))
##         self.mfTempDir.makeDir()

##         # We also need a temporary holding dir for squeezing py files.
##         self.pyzTempDir = Filename(self.persistDir, Filename('pyz/'))
##         self.pyzTempDir.makeDir()

##         # Change to the persist directory so the temp files will be
##         # created there
##         os.chdir(self.persistDir.toOsSpecific())

    def __expandVariable(self, line, p):
        """ Given that line[p] is a dollar sign beginning a variable
        reference, advances p to the first dollar sign following the
        reference, and looks up the variable referenced.

        Returns (value, p) where value is the value of the named
        variable, and p is the first character following the variable
        reference. """

        p += 1
        if p >= len(line):
            return '', p
        
        var = ''
        if line[p] == '{':
            # Curly braces exactly delimit the variable name.
            p += 1
            while p < len(line) and line[p] != '}':
                var += line[p]
                p += 1
        else:
            # Otherwise, a string of alphanumeric characters,
            # including underscore, delimits the variable name.
            var += line[p]
            p += 1
            while p < len(line) and (line[p] in string.letters or line[p] in string.digits or line[p] == '_'):
                var += line[p]
                p += 1

        return ExecutionEnvironment.getEnvironmentVariable(var), p
        

    def __splitLine(self, line):
        """ Separates the indicated line into words at whitespace.
        Quotation marks and escape characters protect spaces.  This is
        designed to be similar to the algorithm employed by the Unix
        shell. """

        words = []

        p = 0
        while p < len(line):
            if line[p] == '#':
                # A word that begins with a hash mark indicates an
                # inline comment, and the end of the parsing.
                break
                
            # Scan to the end of the word.
            word = ''
            while p < len(line) and line[p] not in string.whitespace:
                if line[p] == '\\':
                    # Output an escaped character.
                    p += 1
                    if p < len(line):
                        word += line[p]
                        p += 1

                elif line[p] == '$':
                    # Output a variable reference.
                    expand, p = self.__expandVariable(line, p)
                    word += expand

                elif line[p] == '"':
                    # Output a double-quoted string.
                    p += 1
                    while p < len(line) and line[p] != '"':
                        if line[p] == '\\':
                            # Output an escaped character.
                            p += 1
                            if p < len(line):
                                word += line[p]
                                p += 1
                        elif line[p] == '$':
                            # Output a variable reference.
                            expand, p = self.__expandVariable(line, p)
                            word += expand
                        else:
                            word += line[p]
                            p += 1

                elif line[p] == "'":
                    # Output a single-quoted string.  Escape
                    # characters and dollar signs within single quotes
                    # are not special.
                    p += 1
                    while p < len(line) and line[p] != "'":
                        word += line[p]
                        p += 1

                else:
                    # Output a single character.
                    word += line[p]
                    p += 1

            words.append(word)

            # Scan to the beginning of the next word.
            while p < len(line) and line[p] in string.whitespace:
                p += 1

        return words

    def __getNextLine(self, file, lineNum):
        """ Extracts the next line from the input file, and splits it
        into words.  Returns a tuple (lineNum, list), or (lineNum,
        None) at end of file. """

        line = file.readline()
        lineNum += 1
        while line:
            line = line.strip()
            if not line:
                # Skip the line, it was just a blank line
                pass
            
            elif line[0] == '#':
                # Eat python-style comments.
                pass

            else:
                return (lineNum, self.__splitLine(line))

            line = file.readline()
            lineNum += 1

        # End of file.
        return (lineNum, None)

    def readPackageDef(self, packageDef):
        """ Reads the lines in the .pdef file named by packageDef and
        dispatches to the appropriate handler method for each
        line.  Returns the list of package files."""

        self.packageList = []

        self.notify.info('Reading %s' % (packageDef))
        file = open(packageDef.toOsSpecific())
        lineNum = 0

        # Now start parsing the packageDef lines
        try:
            lineNum, words = self.__getNextLine(file, lineNum)
            while words:
                command = words[0]
                try:
                    methodName = 'parse_%s' % (command)
                    method = getattr(self, methodName, None)
                    if method:
                        method(words)

                    else:
                        message = 'Unknown command %s' % (command)
                        raise PackagerError, message
                except ArgumentError:
                    message = 'Wrong number of arguments for command %s' %(command)
                    raise ArgumentError, message
                except OutsideOfPackageError:
                    message = '%s command encounted outside of package specification' %(command)
                    raise OutsideOfPackageError, message

                lineNum, words = self.__getNextLine(file, lineNum)

        except PackagerError:
            # Append the line number and file name to the exception
            # error message.
            inst = sys.exc_info()[1]
            inst.args = (inst.args[0] + ' on line %s of %s' % (lineNum, packageDef),)
            raise

        packageList = self.packageList
        self.packageList = []

        return packageList

    def parse_set(self, words):
        """
        set variable=value
        """
        
        try:
            command, assign = words
        except ValueError:
            raise ArgumentNumber
        
        try:
            variable, value = assign.split('=', 1)
        except ValueError:
            raise PackagerError, 'Equals sign required in assignment'

        variable = variable.strip()
        value = ExecutionEnvironment.expandString(value.strip())
        ExecutionEnvironment.setEnvironmentVariable(variable, value)

    def parse_begin_package(self, words):
        """
        begin_package packageName [version=v]
        """

        args = self.__parseArgs(words, ['version'])

        try:
            command, packageName = words
        except ValueError:
            raise ArgumentNumber

        version = args.get('version', None)

        self.beginPackage(packageName, version = version, p3dApplication = False)

    def parse_end_package(self, words):
        """
        end_package packageName
        """

        try:
            command, packageName = words
        except ValueError:
            raise ArgumentError

        self.endPackage(packageName, p3dApplication = False)

    def parse_begin_p3d(self, words):
        """
        begin_p3d appName
        """

        try:
            command, packageName = words
        except ValueError:
            raise ArgumentNumber

        self.beginPackage(packageName, p3dApplication = True)

    def parse_end_p3d(self, words):
        """
        end_p3d appName
        """

        try:
            command, packageName = words
        except ValueError:
            raise ArgumentError

        self.endPackage(packageName, p3dApplication = True)

    def parse_display_name(self, words):
        """
        display_name "name"
        """

        try:
            command, displayName = words
        except ValueError:
            raise ArgumentError

        if not self.currentPackage:
            raise OutsideOfPackageError

        self.currentPackage.displayName = displayName

    def parse_require(self, words):
        """
        require packageName [version=v]
        """

        args = self.__parseArgs(words, ['version'])

        try:
            command, packageName = words
        except ValueError:
            raise ArgumentError

        version = args.get('version', None)
        self.require(packageName, version = version)

    def parse_module(self, words):
        """
        module moduleName [newName]
        """
        newName = None

        try:
            if len(words) == 2:
                command, moduleName = words
            else:
                command, moduleName, newName = words
        except ValueError:
            raise ArgumentError

        self.module(moduleName, newName = newName)

    def parse_main_module(self, words):
        """
        main_module moduleName
        """

        try:
            command, moduleName = words
        except ValueError:
            raise ArgumentError

        self.mainModule(moduleName)

    def parse_freeze_exe(self, words):
        """
        freeze_exe path/to/basename
        """

        try:
            command, filename = words
        except ValueError:
            raise ArgumentError

        self.freeze(filename, compileToExe = True)

    def parse_freeze_dll(self, words):
        """
        freeze_dll path/to/basename
        """

        try:
            command, filename = words
        except ValueError:
            raise ArgumentError

        self.freeze(filename, compileToExe = False)

    def parse_file(self, words):
        """
        file filename [newNameOrDir] [extract=1]
        """

        args = self.__parseArgs(words, ['extract'])

        newNameOrDir = None

        try:
            if len(words) == 2:
                command, filename = words
            else:
                command, filename, newNameOrDir = words
        except ValueError:
            raise ArgumentError

        extract = args.get('extract', None)
        if extract is not None:
            extract = int(extract)

        self.file(Filename.fromOsSpecific(filename),
                  newNameOrDir = newNameOrDir, extract = extract)

    def parse_dir(self, words):
        """
        dir dirname [newDir]
        """

        newDir = None

        try:
            if len(words) == 2:
                command, dirname = words
            else:
                command, dirname, newDir = words
        except ValueError:
            raise ArgumentError

        self.dir(Filename.fromOsSpecific(dirname), newDir = newDir)

    def __parseArgs(self, words, argList):
        args = {}
        
        while len(words) > 1:
            arg = words[-1]
            if '=' not in arg:
                return args

            parameter, value = arg.split('=', 1)
            parameter = parameter.strip()
            value = value.strip()
            if parameter not in argList:
                message = 'Unknown parameter %s' % (parameter)
                raise PackagerError, message
            if parameter in args:
                message = 'Duplicate parameter %s' % (parameter)
                raise PackagerError, message

            args[parameter] = value

            del words[-1]
                
    
    def beginPackage(self, packageName, version = None, p3dApplication = False):
        """ Begins a new package specification.  packageName is the
        basename of the package.  Follow this with a number of calls
        to file() etc., and close the package with endPackage(). """

        if self.currentPackage:
            raise PackagerError, 'unmatched end_package %s' % (self.currentPackage.packageName)

        # A special case for the Panda3D package.  We enforce that the
        # version number matches what we've been compiled with.
        if packageName == 'panda3d':
            if version is None:
                version = PandaSystem.getPackageVersionString()
            else:
                if version != PandaSystem.getPackageVersionString():
                    message = 'mismatched Panda3D version: requested %s, but Panda3D is built as %s' % (version, PandaSystem.getPackageVersionString())
                    raise PackageError, message

        package = self.Package(packageName, self)
        self.currentPackage = package

        package.version = version
        package.p3dApplication = p3dApplication

        if package.p3dApplication:
            # Default compression level for an app.
            package.compressionLevel = 6

            # Every p3dapp requires panda3d.
            self.require('panda3d')
            
        package.dryRun = self.dryRun
        
    def endPackage(self, packageName, p3dApplication = False):
        """ Closes a package specification.  This actually generates
        the package file.  The packageName must match the previous
        call to beginPackage(). """
        
        if not self.currentPackage:
            raise PackagerError, 'unmatched end_package %s' % (packageName)
        if self.currentPackage.packageName != packageName:
            raise PackagerError, 'end_package %s where %s expected' % (
                packageName, self.currentPackage.packageName)
        if self.currentPackage.p3dApplication != p3dApplication:
            if p3dApplication:
                raise PackagerError, 'end_p3d where end_package expected'
            else:
                raise PackagerError, 'end_package where end_p3d expected'

        package = self.currentPackage
        package.close()

        self.packageList.append(package)
        self.packages[(package.packageName, package.platform, package.version)] = package
        self.currentPackage = None

    def findPackage(self, packageName, platform = None, version = None,
                    requires = None):
        """ Searches for the named package from a previous publish
        operation along the install search path.

        If requires is not None, it is a list of Package objects that
        are already required.  The new Package object must be
        compatible with the existing Packages, or an error is
        returned.  This is also useful for determining the appropriate
        package version to choose when a version is not specified.

        Returns the Package object, or None if the package cannot be
        located. """

        if not platform:
            platform = self.platform

        # Is it a package we already have resident?
        package = self.packages.get((packageName, platform, version), None)
        if package:
            return package

        # Look on the searchlist.
        for path in self.installSearch:
            package = self.__scanPackageDir(path, packageName, platform, version, requires = requires)
            if not package:
                package = self.__scanPackageDir(path, packageName, None, version, requires = requires)

            if package:
                package = self.packages.setdefault((package.packageName, package.platform, package.version), package)
                self.packages[(packageName, platform, version)] = package
                return package
                
        return None

    def __scanPackageDir(self, rootDir, packageName, platform, version,
                         requires = None):
        """ Scans a directory on disk, looking for *_import.xml files
        that match the indicated packageName and optional version.  If a
        suitable xml file is found, reads it and returns the assocated
        Package definition.

        If a version is not specified, and multiple versions are
        available, the highest-numbered version that matches will be
        selected.
        """

        packageDir = Filename(rootDir, packageName)
        basename = packageName

        if platform:
            packageDir = Filename(packageDir, platform)
            basename += '_%s' % (platform)

        if version:
            # A specific version package.
            packageDir = Filename(packageDir, version)
            basename += '_%s' % (version)
        else:
            # Scan all versions.
            packageDir = Filename(packageDir, '*')
            basename += '_%s' % ('*')

        basename += '_import.xml'
        filename = Filename(packageDir, basename)
        filelist = glob.glob(filename.toOsSpecific())
        if not filelist:
            # It doesn't exist in the nested directory; try the root
            # directory.
            filename = Filename(rootDir, basename)
            filelist = glob.glob(filename.toOsSpecific())

        self.__sortPackageImportFilelist(filelist)
        for file in filelist:
            package = self.__readPackageImportDescFile(Filename.fromOsSpecific(file))
            if package and self.__packageIsValid(package, requires):
                return package

        return None

    def __sortPackageImportFilelist(self, filelist):
        """ Given a list of *_import.xml filenames, sorts them in
        reverse order by version, so that the highest-numbered
        versions appear first in the list. """

        tuples = []
        for file in filelist:
            version = file.split('_')[-2]
            version = self.__makeVersionTuple(version)
            tuples.append((version, file))
        tuples.sort(reverse = True)

        return map(lambda t: t[1], tuples)

    def __makeVersionTuple(self, version):
        """ Converts a version string into a tuple for sorting, by
        separating out numbers into separate numeric fields, so that
        version numbers sort numerically where appropriate. """

        words = []
        p = 0
        while p < len(version):
            # Scan to the first digit.
            w = ''
            while p < len(version) and version[p] not in string.digits:
                w += version[p]
                p += 1
            words.append(w)

            # Scan to the end of the string of digits.
            w = ''
            while p < len(version) and version[p] in string.digits:
                w += version[p]
                p += 1
            words.append(int(w))

        return tuple(words)

    def __packageIsValid(self, package, requires):
        """ Returns true if the package is valid, meaning it can be
        imported without conflicts with existing packages already
        required (such as different versions of panda3d). """

        if not requires:
            return True

        # Really, we only check the panda3d package for now.  The
        # other packages will list this as a dependency, and this is
        # all that matters.

        panda1 = self.__findPackageInList('panda3d', [package] + package.requires)
        panda2 = self.__findPackageInList('panda3d', requires)

        if not panda1 or not panda2:
            return True

        if panda1.version == panda2.version:
            return True

        return False

    def __findPackageInList(self, packageName, list):
        """ Returns the first package with the indicated name in the list. """
        for package in list:
            if package.packageName == packageName:
                return package

        return None

    def __readPackageImportDescFile(self, filename):
        """ Reads the named xml file as a Package, and returns it if
        valid, or None otherwise. """

        package = self.Package('', self)
        if package.readImportDescFile(filename):
            return package

        return None

    def require(self, packageName, version = None):
        """ Indicates a dependency on the named package, supplied as
        a name.

        Attempts to install this package will implicitly install the
        named package also.  Files already included in the named
        package will be omitted from this one when building it. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        # A special case for the Panda3D package.  We enforce that the
        # version number matches what we've been compiled with.
        if packageName == 'panda3d':
            if version is None:
                version = PandaSystem.getPackageVersionString()
        
        package = self.findPackage(packageName, version = version, requires = self.currentPackage.requires)
        if not package:
            message = 'Unknown package %s, version "%s"' % (packageName, version)
            raise PackagerError, message

        self.requirePackage(package)

    def requirePackage(self, package):
        """ Indicates a dependency on the indicated package, supplied
        as a Package object.

        Attempts to install this package will implicitly install the
        named package also.  Files already included in the named
        package will be omitted from this one. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        # A special case for the Panda3D package.  We enforce that the
        # version number matches what we've been compiled with.
        if package.packageName == 'panda3d':
            if package.version != PandaSystem.getPackageVersionString():
                if not PandaSystem.getPackageVersionString():
                    # We haven't been compiled with any particular
                    # version of Panda.  This is a warning, not an
                    # error.
                    print "Warning: requiring panda3d version %s, which may or may not match the current build of Panda.  Recommend that you use only the official Panda3D build for making distributable applications to ensure compatibility." % (package.version)
                else:
                    # This particular version of Panda doesn't match
                    # the requested version.  Again, a warning, not an
                    # error.
                    print "Warning: requiring panda3d version %s, which does not match the current build of Panda, which is version %s." % (package, PandaSystem.getPackageVersionString())

        self.currentPackage.requirePackage(package)

    def module(self, moduleName, newName = None):
        """ Adds the indicated Python module to the current package. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        self.currentPackage.freezer.addModule(moduleName, newName = newName)

    def mainModule(self, moduleName, newName = None):
        """ Names the indicated module as the "main" module of the
        application or exe. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        if self.currentPackage.mainModule and self.currentPackage.mainModule != moduleName:
            self.notify.warning("Replacing main_module %s with %s" % (
                self.currentPackage.mainModule, moduleName))

        self.currentPackage.mainModule = moduleName

    def freeze(self, filename, compileToExe = False):
        """ Freezes all of the current Python code into either an
        executable (if compileToExe is true) or a dynamic library (if
        it is false).  The resulting compiled binary is added to the
        current package under the indicated filename.  The filename
        should not include an extension; that will be added. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        package = self.currentPackage
        freezer = package.freezer

        if package.mainModule and not compileToExe:
            self.notify.warning("Ignoring main_module for dll %s" % (filename))
            package.mainModule = None
        if not package.mainModule and compileToExe:
            message = "No main_module specified for exe %s" % (filename)
            raise PackagerError, message

        if package.mainModule:
            if package.mainModule not in freezer.modules:
                freezer.addModule(package.mainModule, newName = '__main__')
            else:
                freezer.modules['__main__'] = freezer.modules[package.mainModule]
        freezer.done()

        if not package.dryRun:
            dirname = ''
            basename = filename
            if '/' in basename:
                dirname, basename = filename.rsplit('/', 1)
                dirname += '/'

            basename, extras = freezer.generateCode(basename, compileToExe = compileToExe)

            package.files.append(self.PackFile(Filename(basename), newName = dirname + basename, deleteTemp = True, extract = True))
            for moduleName, filename in extras:
                filename = Filename.fromOsSpecific(filename)
                newName = filename.getBasename()
                if '.' in moduleName:
                    newName = '/'.join(moduleName.split('.')[:-1])
                    newName += '/' + filename.getBasename()
                package.files.append(self.PackFile(filename, newName = newName, extract = True))
                
            if not package.platform:
                package.platform = PandaSystem.getPlatform()

        # Reset the freezer for more Python files.
        freezer.reset()
        package.mainModule = None


    def file(self, filename, newNameOrDir = None, extract = None):
        """ Adds the indicated arbitrary file to the current package.

        The file is placed in the named directory, or the toplevel
        directory if no directory is specified.

        The filename may include shell globbing characters.

        Certain special behavior is invoked based on the filename
        extension.  For instance, .py files may be automatically
        compiled and stored as Python modules.

        If newNameOrDir ends in a slash character, it specifies the
        directory in which the file should be placed.  In this case,
        all files matched by the filename expression are placed in the
        named directory.

        If newNameOrDir ends in something other than a slash
        character, it specifies a new filename.  In this case, the
        filename expression must match only one file.

        If newNameOrDir is unspecified or None, the file is placed in
        the toplevel directory, regardless of its source directory.
        
        """

        if not self.currentPackage:
            raise OutsideOfPackageError

        filename = Filename(filename)
        files = glob.glob(filename.toOsSpecific())
        if not files:
            self.notify.warning("No such file: %s" % (filename))
            return

        newName = None
        prefix = ''
        
        if newNameOrDir:
            if newNameOrDir[-1] == '/':
                prefix = newNameOrDir
            else:
                newName = newNameOrDir
                if len(files) != 1:
                    message = 'Cannot install multiple files on target filename %s' % (newName)
                    raise PackagerError, message

        for filename in files:
            filename = Filename.fromOsSpecific(filename)
            basename = filename.getBasename()
            if newName:
                self.addFile(filename, newName = newName, extract = extract)
            else:
                self.addFile(filename, newName = prefix + basename, extract = extract)

    def dir(self, dirname, newDir = None):

        """ Adds the indicated directory hierarchy to the current
        package.  The directory hierarchy is walked recursively, and
        all files that match a known extension are added to the package.

        newDir specifies the directory name within the package which
        the contents of the named directory should be installed to.
        If it is omitted, the contents of the named directory are
        installed to the root of the package.
        """

        if not self.currentPackage:
            raise OutsideOfPackageError

        dirname = Filename(dirname)
        if not newDir:
            newDir = ''

        self.__recurseDir(dirname, newDir)

    def __recurseDir(self, filename, newName):
        dirList = vfs.scanDirectory(filename)
        if dirList:
            # It's a directory name.  Recurse.
            prefix = newName
            if prefix and prefix[-1] != '/':
                prefix += '/'
            for subfile in dirList:
                filename = subfile.getFilename()
                self.__recurseDir(filename, prefix + filename.getBasename())
            return

        # It's a file name.  Add it.
        ext = filename.getExtension()
        if ext == 'py':
            self.addFile(filename, newName = newName)
        else:
            if ext == 'pz':
                # Strip off an implicit .pz extension.
                newFilename = Filename(filename)
                newFilename.setExtension('')
                newFilename = Filename(newFilename.cStr())
                ext = newFilename.getExtension()

            if ext in self.knownExtensions:
                self.addFile(filename, newName = newName)

    def addFile(self, filename, newName = None, extract = None):
        """ Adds the named file, giving it the indicated name within
        the package. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        self.currentPackage.files.append(
            self.PackFile(filename, newName = newName, extract = extract))
