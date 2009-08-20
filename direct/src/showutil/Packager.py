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
import types
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
        def __init__(self, package, filename,
                     newName = None, deleteTemp = False,
                     explicit = False, compress = None, extract = None,
                     executable = None):
            assert isinstance(filename, Filename)
            self.filename = Filename(filename)
            self.newName = newName
            self.deleteTemp = deleteTemp
            self.explicit = explicit
            self.compress = compress
            self.extract = extract
            self.executable = executable

            if not self.newName:
                self.newName = self.filename.cStr()

            ext = Filename(self.newName).getExtension()
            if ext == 'pz':
                # Strip off a .pz extension; we can compress files
                # within the Multifile without it.
                filename = Filename(self.newName)
                filename.setExtension('')
                self.newName = filename.cStr()
                ext = Filename(self.newName).getExtension()
                if self.compress is None:
                    self.compress = True

            packager = package.packager
            if self.compress is None:
                self.compress = (ext not in packager.uncompressibleExtensions and ext not in packager.imageExtensions)

            if self.executable is None:
                self.executable = (ext in packager.executableExtensions)

            if self.extract is None:
                self.extract = self.executable or (ext in packager.extractExtensions)
            self.platformSpecific = self.executable or (ext in packager.platformSpecificExtensions)
                

            if self.executable:
                # Look up the filename along the system PATH, if necessary.
                self.filename.resolveFilename(packager.executablePath)

            # Convert the filename to an unambiguous filename for
            # searching.
            self.filename.makeTrueCase()
            if self.filename.exists() or not self.filename.isLocal():
                self.filename.makeCanonical()

        def isExcluded(self, package):
            """ Returns true if this file should be excluded or
            skipped, false otherwise. """

            if self.newName in package.skipFilenames:
                return True

            if not self.explicit:
                # Make sure it's not one of our auto-excluded system
                # files.  (But only make this check if this file was
                # not explicitly added.)

                basename = Filename(self.newName).getBasename()
                if not package.packager.caseSensitive:
                    basename = basename.lower()
                if basename in package.packager.excludeSystemFiles:
                    return True
                for exclude in package.packager.excludeSystemGlobs:
                    if exclude.matches(basename):
                        return True

                # Also check if it was explicitly excluded.  As above,
                # omit this check for an explicitly-added file: if you
                # both include and exclude a file, the file is
                # included.
                for exclude in package.excludedFilenames:
                    if exclude.matches(self.filename):
                        return True

            return False
                
    class ExcludeFilename:
        def __init__(self, filename, caseSensitive):
            self.localOnly = (not filename.get_dirname())
            if not self.localOnly:
                filename = Filename(filename)
                filename.makeCanonical()
            self.glob = GlobPattern(filename.cStr())

            if PandaSystem.getPlatform().startswith('win'):
                self.glob.setCaseSensitive(False)
            elif PandaSystem.getPlatform().startswith('osx'):
                self.glob.setCaseSensitive(False)

        def matches(self, filename):
            if self.localOnly:
                return self.glob.matches(filename.getBasename())
            else:
                return self.glob.matches(filename.cStr())

    class Package:
        def __init__(self, packageName, packager):
            self.packageName = packageName
            self.packager = packager
            self.version = None
            self.platform = None
            self.p3dApplication = False
            self.compressionLevel = 0
            self.importedMapsDir = 'imported_maps'
            self.mainModule = None
            self.requires = []

            # This is the set of config variables assigned to the
            # package.
            self.configs = {}

            # This is the set of files and modules, already included
            # by required packages, that we can skip.
            self.skipFilenames = {}
            self.skipModules = {}

            # This is a list of ExcludeFilename objects, representing
            # the files that have been explicitly excluded.
            self.excludedFilenames = []

            # This is the list of files we will be adding, and a pair
            # of cross references.
            self.files = []
            self.sourceFilenames = {}
            self.targetFilenames = {}

            # This records the current list of modules we have added so
            # far.
            self.freezer = FreezeTool.Freezer()

            # Set this true to parse and build up the internal
            # filelist, but not generate any output.
            self.dryRun = False

        def close(self):
            """ Writes out the contents of the current package. """

            if self.dryRun:
                self.multifile = None
            else:
                # Write the multifile to a temporary filename until we
                # know enough to determine the output filename.
                multifileFilename = Filename.temporary('', self.packageName)
                self.multifile = Multifile()
                self.multifile.openReadWrite(multifileFilename)

            self.extracts = []
            self.components = []

            # Add the explicit py files that were requested by the
            # pdef file.  These get turned into Python modules.
            for file in self.files:
                ext = Filename(file.newName).getExtension()
                if ext != 'py':
                    continue

                if file.isExcluded(self):
                    # Skip this file.
                    continue

                self.addPyFile(file)

            # Add the main module, if any.
            if not self.mainModule and self.p3dApplication:
                message = 'No main_module specified for application %s' % (self.packageName)
                raise PackagerError, message
            if self.mainModule:
                moduleName, newName = self.mainModule
                if newName not in self.freezer.modules:
                    self.freezer.addModule(moduleName, newName = newName)

            # Now all module files have been added.  Exclude modules
            # already imported in a required package, and not
            # explicitly included by this package.
            for moduleName, mdef in self.skipModules.items():
                if moduleName not in self.freezer.modules:
                    self.freezer.excludeModule(
                        moduleName, allowChildren = mdef.allowChildren,
                        forbid = mdef.forbid, fromSource = 'skip')

            # Pick up any unfrozen Python files.
            self.freezer.done()
            self.freezer.addToMultifile(self.multifile, self.compressionLevel)
            self.addExtensionModules()

            # Add known module names.
            self.moduleNames = {}
            modules = self.freezer.modules.items()
            modules.sort()
            for newName, mdef in modules:
                if mdef.guess:
                    # Not really a module.
                    continue

                if mdef.fromSource == 'skip':
                    # This record already appeared in a required
                    # module; don't repeat it now.
                    continue

                if mdef.exclude and mdef.implicit:
                    # Don't bother mentioning implicity-excluded
                    # (i.e. missing) modules.
                    continue

                if newName == '__main__':
                    # Ignore this special case.
                    continue

                self.moduleNames[newName] = mdef

                xmodule = TiXmlElement('module')
                xmodule.SetAttribute('name', newName)
                if mdef.exclude:
                    xmodule.SetAttribute('exclude', '1')
                if mdef.forbid:
                    xmodule.SetAttribute('forbid', '1')
                if mdef.exclude and mdef.allowChildren:
                    xmodule.SetAttribute('allowChildren', '1')
                self.components.append((newName.lower(), xmodule))

            # Now look for implicit shared-library dependencies.
            if PandaSystem.getPlatform().startswith('win'):
                self.__addImplicitDependenciesWindows()
            elif PandaSystem.getPlatform().startswith('osx'):
                self.__addImplicitDependenciesOSX()
            else:
                self.__addImplicitDependenciesPosix()

            # Now add all the real, non-Python files (except model
            # files).  This will include the extension modules we just
            # discovered above.
            for file in self.files:
                ext = Filename(file.newName).getExtension()
                if ext == 'py':
                    # Already handled, above.
                    continue

                if file.isExcluded(self):
                    # Skip this file.
                    continue
                
                if not self.dryRun:
                    if ext == 'egg' or ext == 'bam':
                        # Skip model files this pass.
                        pass
                    else:
                        # Any other file.
                        self.addComponent(file)

            # Finally, now add the model files.  It's important to add
            # these after we have added all of the texture files, so
            # we can determine which textures need to be implicitly
            # pulled in.

            # We walk through the list as we modify it.  That's OK,
            # because we may add new files that we want to process.
            for file in self.files:
                ext = Filename(file.newName).getExtension()
                if ext == 'py':
                    # Already handled, above.
                    continue

                if file.isExcluded(self):
                    # Skip this file.
                    continue
                
                if not self.dryRun:
                    if ext == 'egg':
                        self.addEggFile(file)
                    elif ext == 'bam':
                        self.addBamFile(file)
                    else:
                        # Handled above.
                        pass

            # Now that we've processed all of the component files,
            # (and set our platform if necessary), we can generate the
            # output filename and write the output files.
            
            if not self.p3dApplication and not self.version:
                # We must have a version string for packages.  Use the
                # first versioned string on our require list.
                self.version = '0.0'
                for p2 in self.requires:
                    if p2.version:
                        self.version = p2.version
                        break

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

            if not self.dryRun:
                if self.p3dApplication:
                    self.makeP3dInfo()
                self.multifile.repack()
                self.multifile.close()

                multifileFilename.renameTo(self.packageFullpath)

                if not self.p3dApplication:
                    self.compressMultifile()
                    self.writeDescFile()
                    self.writeImportDescFile()

            # Now that all the files have been packed, we can delete
            # the temporary files.
            for file in self.files:
                if file.deleteTemp:
                    file.filename.unlink()

        def addFile(self, *args, **kw):
            """ Adds the named file to the package. """

            file = Packager.PackFile(self, *args, **kw)
            if file.filename in self.sourceFilenames:
                # Don't bother, it's already here.
                return

            if file.newName in self.targetFilenames:
                # Another file is already in the same place.
                file2 = self.targetFilenames[file.newName]
                self.packager.notify.warning(
                    "%s is shadowing %s" % (file2.filename, file.filename))
                return

            self.sourceFilenames[file.filename] = file

            if not file.filename.exists():
                if not file.isExcluded(self):
                    self.packager.notify.warning("No such file: %s" % (file.filename))
                return
            
            self.files.append(file)
            self.targetFilenames[file.newName] = file

        def excludeFile(self, filename):
            """ Excludes the named file (or glob pattern) from the
            package. """
            xfile = Packager.ExcludeFilename(filename, self.packager.caseSensitive)
            self.excludedFilenames.append(xfile)

        def __addImplicitDependenciesWindows(self):
            """ Walks through the list of files, looking for dll's and
            exe's that might include implicit dependencies on other
            dll's.  Tries to determine those dependencies, and adds
            them back into the filelist. """

            # We walk through the list as we modify it.  That's OK,
            # because we want to follow the transitive closure of
            # dependencies anyway.
            for file in self.files:
                if not file.executable:
                    continue
                
                if file.isExcluded(self):
                    # Skip this file.
                    continue

                tempFile = Filename.temporary('', 'p3d_')
                command = 'dumpbin /dependents "%s" >"%s"' % (
                    file.filename.toOsSpecific(),
                    tempFile.toOsSpecific())
                try:
                    os.system(command)
                except:
                    pass
                filenames = None

                if tempFile.exists():
                    filenames = self.__parseDependenciesWindows(tempFile)
                if filenames is None:
                    print "Unable to determine dependencies from %s" % (file.filename)
                    continue

                # Attempt to resolve the dependent filename relative
                # to the original filename, before we resolve it along
                # the PATH.
                path = DSearchPath(Filename(file.filename.getDirname()))

                for filename in filenames:
                    filename = Filename.fromOsSpecific(filename)
                    filename.resolveFilename(path)
                    self.addFile(filename, newName = filename.getBasename(),
                                 explicit = False, executable = True)
                    
        def __parseDependenciesWindows(self, tempFile):
            """ Reads the indicated temporary file, the output from
            dumpbin /dependents, to determine the list of dll's this
            executable file depends on. """

            lines = open(tempFile.toOsSpecific(), 'rU').readlines()
            li = 0
            while li < len(lines):
                line = lines[li]
                li += 1
                if line.find(' has the following dependencies') != -1:
                    break

            if li < len(lines):
                line = lines[li]
                if line.strip() == '':
                    # Skip a blank line.
                    li += 1

            # Now we're finding filenames, until the next blank line.
            filenames = []
            while li < len(lines):
                line = lines[li]
                li += 1
                line = line.strip()
                if line == '':
                    # We're done.
                    return filenames
                filenames.append(line)

            # Hmm, we ran out of data.  Oh well.
            if not filenames:
                # Some parse error.
                return None

            # At least we got some data.
            return filenames

        def __addImplicitDependenciesOSX(self):
            """ Walks through the list of files, looking for dylib's
            and executables that might include implicit dependencies
            on other dylib's.  Tries to determine those dependencies,
            and adds them back into the filelist. """

            # We walk through the list as we modify it.  That's OK,
            # because we want to follow the transitive closure of
            # dependencies anyway.
            for file in self.files:
                if not file.executable:
                    continue
                
                if file.isExcluded(self):
                    # Skip this file.
                    continue

                tempFile = Filename.temporary('', 'p3d_')
                command = 'otool -L "%s" >"%s"' % (
                    file.filename.toOsSpecific(),
                    tempFile.toOsSpecific())
                try:
                    os.system(command)
                except:
                    pass
                filenames = None

                if tempFile.exists():
                    filenames = self.__parseDependenciesOSX(tempFile)
                if filenames is None:
                    print "Unable to determine dependencies from %s" % (file.filename)
                    continue

                # Attempt to resolve the dependent filename relative
                # to the original filename, before we resolve it along
                # the PATH.
                path = DSearchPath(Filename(file.filename.getDirname()))

                for filename in filenames:
                    filename = Filename.fromOsSpecific(filename)
                    filename.resolveFilename(path)
                    self.addFile(filename, newName = filename.getBasename(),
                                 explicit = False, executable = True)
                    
        def __parseDependenciesOSX(self, tempFile):
            """ Reads the indicated temporary file, the output from
            otool -L, to determine the list of dylib's this
            executable file depends on. """

            lines = open(tempFile.toOsSpecific(), 'rU').readlines()

            filenames = []
            for line in lines:
                if line[0] not in string.whitespace:
                    continue
                line = line.strip()
                if line.startswith('/System/'):
                    continue
                s = line.find(' (compatibility')
                if s != -1:
                    line = line[:s]
                else:
                    s = line.find('.dylib')
                    if s != -1:
                        line = line[:s + 6]
                    else:
                        continue
                filenames.append(line)

            return filenames

        def __addImplicitDependenciesPosix(self):
            """ Walks through the list of files, looking for so's
            and executables that might include implicit dependencies
            on other so's.  Tries to determine those dependencies,
            and adds them back into the filelist. """

            # We walk through the list as we modify it.  That's OK,
            # because we want to follow the transitive closure of
            # dependencies anyway.
            for file in self.files:
                if not file.executable:
                    continue
                
                if file.isExcluded(self):
                    # Skip this file.
                    continue

                tempFile = Filename.temporary('', 'p3d_')
                command = 'ldd "%s" >"%s"' % (
                    file.filename.toOsSpecific(),
                    tempFile.toOsSpecific())
                try:
                    os.system(command)
                except:
                    pass
                filenames = None

                if tempFile.exists():
                    filenames = self.__parseDependenciesPosix(tempFile)
                if filenames is None:
                    print "Unable to determine dependencies from %s" % (file.filename)
                    continue

                # Attempt to resolve the dependent filename relative
                # to the original filename, before we resolve it along
                # the PATH.
                path = DSearchPath(Filename(file.filename.getDirname()))

                for filename in filenames:
                    filename = Filename.fromOsSpecific(filename)
                    filename.resolveFilename(path)
                    self.addFile(filename, newName = filename.getBasename(),
                                 explicit = False, executable = True)
                    
        def __parseDependenciesPosix(self, tempFile):
            """ Reads the indicated temporary file, the output from
            ldd, to determine the list of so's this executable file
            depends on. """

            lines = open(tempFile.toOsSpecific(), 'rU').readlines()

            filenames = []
            for line in lines:
                line = line.strip()
                s = line.find(' => ')
                if s == -1:
                    continue

                line = line[:s].strip()
                filenames.append(line)

            return filenames

        def addExtensionModules(self):
            """ Adds the extension modules detected by the freezer to
            the current list of files. """

            freezer = self.freezer
            if freezer.extras:
                if not self.platform:
                    self.platform = PandaSystem.getPlatform()
                
            for moduleName, filename in freezer.extras:
                filename = Filename.fromOsSpecific(filename)
                newName = filename.getBasename()
                if '.' in moduleName:
                    newName = '/'.join(moduleName.split('.')[:-1])
                    newName += '/' + filename.getBasename()
                # Sometimes the PYTHONPATH has the wrong case in it.
                filename.makeTrueCase()
                self.addFile(filename, newName = newName,
                             explicit = False, extract = True)
            freezer.extras = []


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

            xpackage.SetAttribute('main_module', self.mainModule[1])

            for variable, value in self.configs.items():
                if isinstance(value, types.UnicodeType):
                    xpackage.SetAttribute(variable, value.encode('utf-8'))
                else:
                    xpackage.SetAttribute(variable, str(value))

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

            for variable, value in self.configs.items():
                if isinstance(value, types.UnicodeType):
                    xpackage.SetAttribute(variable, value.encode('utf-8'))
                else:
                    xpackage.SetAttribute(variable, str(value))

            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if self.platform and package.platform:
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

            self.extracts.sort()
            for name, xextract in self.extracts:
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
                if self.platform and package.platform:
                    xrequires.SetAttribute('platform', package.platform)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                xpackage.InsertEndChild(xrequires)

            self.components.sort()
            for name, xcomponent in self.components:
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
                xrequires = xrequires.NextSiblingElement('requires')

            self.targetFilenames = {}
            xcomponent = xpackage.FirstChildElement('component')
            while xcomponent:
                name = xcomponent.Attribute('filename')
                if name:
                    self.targetFilenames[name] = True
                xcomponent = xcomponent.NextSiblingElement('component')

            self.moduleNames = {}
            xmodule = xpackage.FirstChildElement('module')
            while xmodule:
                moduleName = xmodule.Attribute('name')
                exclude = int(xmodule.Attribute('exclude') or 0)
                forbid = int(xmodule.Attribute('forbid') or 0)
                allowChildren = int(xmodule.Attribute('allowChildren') or 0)
                
                if moduleName:
                    mdef = FreezeTool.Freezer.ModuleDef(
                        moduleName, exclude = exclude, forbid = forbid,
                        allowChildren = allowChildren)
                    self.moduleNames[moduleName] = mdef
                xmodule = xmodule.NextSiblingElement('module')

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
            np = self.packager.loader.loadModel(file.filename)
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
            self.components.append((newName.lower(), xcomponent))

        def addFoundTexture(self, filename):
            """ Adds the newly-discovered texture to the output, if it has
            not already been included.  Returns the new name within the
            package tree. """

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

            self.addFile(filename, newName = newName, explicit = False,
                         compress = False)
            return newName

        def addComponent(self, file):
            if file.platformSpecific:
                if not self.platform:
                    self.platform = PandaSystem.getPlatform()
                
            compressionLevel = 0
            if file.compress:
                compressionLevel = self.compressionLevel
                
            self.multifile.addSubfile(file.newName, file.filename, compressionLevel)
            if file.extract:
                xextract = self.getFileSpec('extract', file.filename, file.newName)
                self.extracts.append((file.newName.lower(), xextract))

            xcomponent = TiXmlElement('component')
            xcomponent.SetAttribute('filename', file.newName)
            self.components.append((file.newName.lower(), xcomponent))

        def requirePackage(self, package):
            """ Indicates a dependency on the given package.  This
            also implicitly requires all of the package's requirements
            as well. """

            for p2 in package.requires + [package]:
                if p2 not in self.requires:
                    self.requires.append(p2)
                    for filename in p2.targetFilenames.keys():
                        self.skipFilenames[filename] = True
                    for moduleName, mdef in p2.moduleNames.items():
                        self.skipModules[moduleName] = mdef

    def __init__(self):

        # The following are config settings that the caller may adjust
        # before calling any of the command methods.

        # These should each be a Filename, or None if they are not
        # filled in.
        self.installDir = None
        self.persistDir = None

        # A search list of directories and/or URL's to search for
        # installed packages.  We query it from a config variable
        # initially, but we may also be extending it at runtime.
        self.installSearch = ConfigVariableSearchPath('pdef-path')

        # The system PATH, for searching dll's and exe's.
        self.executablePath = DSearchPath()
        if PandaSystem.getPlatform().startswith('win'):
            self.addWindowsSearchPath(self.executablePath, "PATH")
        elif PandaSystem.getPlatform().startswith('osx'):
            self.addPosixSearchPath(self.executablePath, "DYLD_LIBRARY_PATH")
            self.addPosixSearchPath(self.executablePath, "LD_LIBRARY_PATH")
            self.addPosixSearchPath(self.executablePath, "PATH")
            self.executablePath.appendDirectory('/lib')
            self.executablePath.appendDirectory('/usr/lib')
        else:
            self.addPosixSearchPath(self.executablePath, "LD_LIBRARY_PATH")
            self.addPosixSearchPath(self.executablePath, "PATH")
            self.executablePath.appendDirectory('/lib')
            self.executablePath.appendDirectory('/usr/lib')

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

        # Is this file system case-sensitive?
        self.caseSensitive = True
        if PandaSystem.getPlatform().startswith('win'):
            self.caseSensitive = False
        elif PandaSystem.getPlatform().startswith('osx'):
            self.caseSensitive = False

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

        # Files that represent an executable or shared library.
        if self.platform.startswith('win'):
            self.executableExtensions = [ 'dll', 'pyd', 'exe' ]
        elif self.platform.startswith('osx'):
            self.executableExtensions = [ 'so', 'dylib' ]
        else:
            self.executableExtensions = [ 'so' ]

        # Extensions that are automatically remapped by convention.
        self.remapExtensions = {}
        if self.platform.startswith('win'):
            pass
        elif self.platform.startswith('osx'):
            self.remapExtensions = {
                'dll' : 'dylib',
                'pyd' : 'dylib',
                'exe' : ''
                }
        else:
            self.remapExtensions = {
                'dll' : 'so',
                'pyd' : 'so',
                'exe' : ''
                }

        # Files that should be extracted to disk.
        self.extractExtensions = self.executableExtensions[:]

        # Files that indicate a platform dependency.
        self.platformSpecificExtensions = self.executableExtensions[:]

        # Binary files that are considered uncompressible, and are
        # copied without compression.
        self.uncompressibleExtensions = [ 'mp3', 'ogg' ]

        # System files that should never be packaged.  For
        # case-insensitive filesystems (like Windows), put the
        # lowercase filename here.  Case-sensitive filesystems should
        # use the correct case.
        self.excludeSystemFiles = [
            'kernel32.dll', 'user32.dll', 'wsock32.dll', 'ws2_32.dll',
            'advapi32.dll', 'opengl32.dll', 'glu32.dll', 'gdi32.dll',
            'shell32.dll', 'ntdll.dll', 'ws2help.dll', 'rpcrt4.dll',
            'imm32.dll', 'ddraw.dll', 'shlwapi.dll', 'secur32.dll',
            'dciman32.dll', 'comdlg32.dll', 'comctl32.dll', 'ole32.dll',
            'oleaut32.dll', 'gdiplus.dll', 'winmm.dll',

            'libsystem.b.dylib', 'libmathcommon.a.dylib', 'libmx.a.dylib',
            'libstdc++.6.dylib',
            ]

        # As above, but with filename globbing to catch a range of
        # filenames.
        self.excludeSystemGlobs = [
            GlobPattern('d3dx9_*.dll'),

            GlobPattern('linux-gate.so*'),
            GlobPattern('libdl.so*'),
            GlobPattern('libm.so*'),
            GlobPattern('libc.so*'),
            GlobPattern('libGL.so*'),
            GlobPattern('libGLU.so*'),
            GlobPattern('libX*.so*'),
            ]

        # A Loader for loading models.
        self.loader = Loader.Loader(self)
        self.sfxManagerList = None
        self.musicManager = None

        # This is filled in during readPackageDef().
        self.packageList = []

        # A table of all known packages by name.
        self.packages = {}

        self.dryRun = False

    def addWindowsSearchPath(self, searchPath, varname):
        """ Expands $varname, interpreting as a Windows-style search
        path, and adds its contents to the indicated DSearchPath. """

        path = ExecutionEnvironment.getEnvironmentVariable(varname)
        for dirname in path.split(';'):
            dirname = Filename.fromOsSpecific(dirname)
            if dirname.makeTrueCase():
                searchPath.appendDirectory(dirname)

    def addPosixSearchPath(self, searchPath, varname):
        """ Expands $varname, interpreting as a Posix-style search
        path, and adds its contents to the indicated DSearchPath. """

        path = ExecutionEnvironment.getEnvironmentVariable(varname)
        for dirname in path.split(':'):
            dirname = Filename.fromOsSpecific(dirname)
            if dirname.makeTrueCase():
                searchPath.appendDirectory(dirname)


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

    def __getNextLine(self):
        """ Extracts the next line from self.inFile, and splits it
        into words.  Returns the list of words, or None at end of
        file. """

        line = self.inFile.readline()
        self.lineNum += 1
        while line:
            line = line.strip()
            if not line:
                # Skip the line, it was just a blank line
                pass
            
            elif line[0] == '#':
                # Eat python-style comments.
                pass

            else:
                return self.__splitLine(line)

            line = self.inFile.readline()
            self.lineNum += 1

        # End of file.
        return None

    def readPackageDef(self, packageDef):
        """ Reads the lines in the .pdef file named by packageDef and
        dispatches to the appropriate handler method for each
        line.  Returns the list of package files."""

        self.packageList = []

        self.notify.info('Reading %s' % (packageDef))
        self.inFile = open(packageDef.toOsSpecific())
        self.lineNum = 0

        # Now start parsing the packageDef lines
        try:
            words = self.__getNextLine()
            while words is not None:
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

                words = self.__getNextLine()

        except PackagerError:
            # Append the line number and file name to the exception
            # error message.
            inst = sys.exc_info()[1]
            inst.args = (inst.args[0] + ' on line %s of %s' % (self.lineNum, packageDef),)
            del self.inFile
            del self.lineNum
            raise

        del self.inFile
        del self.lineNum

        packageList = self.packageList
        self.packageList = []

        return packageList

    def __expandTabs(self, line, tabWidth = 8):
        """ Expands tab characters in the line to 8 spaces. """
        p = 0
        while p < len(line):
            if line[p] == '\t':
                # Expand a tab.
                nextStop = ((p + tabWidth) / tabWidth) * tabWidth
                numSpaces = nextStop - p
                line = line[:p] + ' ' * numSpaces + line[p + 1:]
                p = nextStop
            else:
                p += 1

        return line

    def __countLeadingWhitespace(self, line):
        """ Returns the number of leading whitespace characters in the
        line. """

        line = self.__expandTabs(line)
        return len(line) - len(line.lstrip())

    def __stripLeadingWhitespace(self, line, whitespaceCount):
        """ Removes the indicated number of whitespace characters, but
        no more. """

        line = self.__expandTabs(line)
        line = line[:whitespaceCount].lstrip() + line[whitespaceCount:]
        return line

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

    def parse_model_path(self, words):
        """
        model_path directory
        """
        newName = None

        try:
            command, dirName = words
        except ValueError:
            raise ArgumentError

        getModelPath().appendDirectory(Filename.fromOsSpecific(dirName))

    def parse_reset_model_path(self, words):
        """
        reset_model_path
        """
        newName = None

        try:
            (command,) = words
        except ValueError:
            raise ArgumentError

        getModelPath().clear()

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

    def parse_config(self, words):
        """
        config variable=value
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
        self.config(variable, value)

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

    def parse_exclude_module(self, words):
        """
        exclude_module moduleName [forbid=1]
        """
        newName = None

        args = self.__parseArgs(words, ['forbid'])

        try:
            command, moduleName = words
        except ValueError:
            raise ArgumentError

        forbid = args.get('forbid', None)
        if forbid is not None:
            forbid = int(forbid)
        self.excludeModule(moduleName, forbid = forbid)

    def parse_main_module(self, words):
        """
        main_module moduleName [newName]
        """
        newName = None

        try:
            if len(words) == 2:
                command, moduleName = words
            else:
                command, moduleName, newName = words
        except ValueError:
            raise ArgumentError

        self.mainModule(moduleName, newName = newName)

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
        file filename [newNameOrDir] [extract=1] [executable=1] [literal=1]
        """

        args = self.__parseArgs(words, ['extract', 'executable', 'literal'])

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

        executable = args.get('executable', None)
        if executable is not None:
            executable = int(executable)

        literal = args.get('literal', None)
        if literal is not None:
            literal = int(literal)

        self.file(Filename.fromOsSpecific(filename),
                  newNameOrDir = newNameOrDir, extract = extract,
                  executable = executable, literal = literal)

    def parse_inline_file(self, words):
        """
        inline_file newName [extract=1] <<[-] eof-symbol
           .... file text
           .... file text
        eof-symbol
        """

        if not self.currentPackage:
            raise OutsideOfPackageError

        # Look for the eof-symbol at the end of the command line.
        eofSymbols = None
        for i in range(len(words)):
            if words[i].startswith('<<'):
                eofSymbols = words[i:]
                words = words[:i]
                break
        if eofSymbols is None:
            raise PackagerError, 'No << appearing on inline_file'

        # Following the bash convention, <<- means to strip leading
        # whitespace, << means to keep it.
        stripWhitespace = False
        if eofSymbols[0][0:3] == '<<-':
            stripWhitespace = True
            eofSymbols[0] = eofSymbols[0][3:]
        else:
            eofSymbols[0] = eofSymbols[0][2:]

        # If there's nothing left in the first word, look to the next
        # word.
        if not eofSymbols[0]:
            del eofSymbols[0]
            
        if len(eofSymbols) == 1:
            # The eof symbol is the only word.
            eofSymbol = eofSymbols[0]
        else:
            # It must be only one word.
            raise PackagerError, 'Only one word may follow <<'

        # Now parse the remaining args.
        args = self.__parseArgs(words, ['extract'])

        newName = None
        try:
            command, newName = words
        except ValueError:
            raise ArgumentError

        extract = args.get('extract', None)
        if extract is not None:
            extract = int(extract)

        tempFile = Filename.temporary('', self.currentPackage.packageName)
        temp = open(tempFile.toOsSpecific(), 'w')

        # Now read text from the input file until we come across
        # eofSymbol on a line by itself.
        lineNum = self.lineNum
        line = self.inFile.readline()
        if not line:
            raise PackagerError, 'No text following inline_file'
        lineNum += 1
        line = line.rstrip()
        if stripWhitespace:
            # For the first line, we count up the amount of
            # whitespace.
            whitespaceCount = self.__countLeadingWhitespace(line)
            line = self.__stripLeadingWhitespace(line, whitespaceCount)
            
        while line != eofSymbol:
            print >> temp, line
            line = self.inFile.readline()
            if not line:
                raise PackagerError, 'EOF indicator not found following inline_file'
            lineNum += 1
            line = line.rstrip()
            if stripWhitespace:
                line = self.__stripLeadingWhitespace(line, whitespaceCount)

        temp.close()
        self.lineNum = lineNum
        self.file(tempFile, deleteTemp = True,
                  newNameOrDir = newName, extract = extract)

    def parse_exclude(self, words):
        """
        exclude filename
        """

        try:
            command, filename = words
        except ValueError:
            raise ArgumentError

        self.exclude(Filename.fromOsSpecific(filename))

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
        for dirname in self.installSearch.getDirectories():
            package = self.__scanPackageDir(dirname, packageName, platform, version, requires = requires)
            if not package:
                package = self.__scanPackageDir(dirname, packageName, None, version, requires = requires)

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

        # Really, we only check the panda3d package.  The other
        # packages will list this as a dependency, and this is all
        # that matters.

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

    def config(self, variable, value):
        """ Sets the indicated p3d config variable to the given value.
        This will be written into the p3d_info.xml file at the top of
        the application, or to the package desc file for a package
        file. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        self.currentPackage.configs[variable] = value

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

    def excludeModule(self, moduleName, forbid = False):
        """ Marks the indicated Python module as not to be included. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        self.currentPackage.freezer.excludeModule(moduleName, forbid = forbid)

    def mainModule(self, moduleName, newName = None, filename = None):
        """ Names the indicated module as the "main" module of the
        application or exe. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        if self.currentPackage.mainModule and self.currentPackage.mainModule[0] != moduleName:
            self.notify.warning("Replacing main_module %s with %s" % (
                self.currentPackage.mainModule[0], moduleName))

        if not newName:
            newName = moduleName

        if filename:
            newFilename = Filename('/'.join(moduleName.split('.')))
            newFilename.setExtension(filename.getExtension())
            self.currentPackage.addFile(
                filename, newName = newFilename.cStr(),
                deleteTemp = True, explicit = True, extract = True)

        self.currentPackage.mainModule = (moduleName, newName)

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
            moduleName, newName = package.mainModule
            if compileToExe:
                # If we're producing an exe, the main module must
                # be called "__main__".
                newName = '__main__'
                package.mainModule = (moduleName, newName)

            if newName not in freezer.modules:
                freezer.addModule(moduleName, newName = newName)
            else:
                freezer.modules[newName] = freezer.modules[moduleName]
        freezer.done(compileToExe = compileToExe)

        if not package.dryRun:
            dirname = ''
            basename = filename
            if '/' in basename:
                dirname, basename = filename.rsplit('/', 1)
                dirname += '/'

            basename = freezer.generateCode(basename, compileToExe = compileToExe)

            package.addFile(Filename(basename), newName = dirname + basename,
                            deleteTemp = True, explicit = True, extract = True)
            package.addExtensionModules()
            if not package.platform:
                package.platform = PandaSystem.getPlatform()

        # Reset the freezer for more Python files.
        freezer.reset()
        package.mainModule = None

    def file(self, filename, source = None, newNameOrDir = None,
             extract = None, executable = None, deleteTemp = False,
             literal = False):
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
        named directory.  If newNameOrDir ends in something other than
        a slash character, it specifies a new filename.  In this case,
        the filename expression must match only one file.  If
        newNameOrDir is unspecified or None, the file is placed in the
        toplevel directory, regardless of its source directory.

        If extract is true, the file is explicitly extracted at
        runtime.

        If executable is true, the file is marked as an executable
        filename, for special treatment.

        If deleteTemp is true, the file is a temporary file and will
        be deleted after its contents are copied to the package.

        If literal is true, then the file extension will be respected
        exactly as it appears, and glob characters will not be
        expanded.  If this is false, then .dll or .exe files will be
        renamed to .dylib and no extension on OSX (or .so on Linux);
        and glob characters will be expanded.
        
        """

        if not self.currentPackage:
            raise OutsideOfPackageError

        filename = Filename(filename)

        if literal:
            files = [filename.toOsSpecific()]

        else:
            ext = filename.getExtension()

            # A special case, since OSX and Linux don't have a
            # standard extension for program files.
            if executable is None and ext == 'exe':
                executable = True

            newExt = self.remapExtensions.get(ext, None)
            if newExt is not None:
                filename.setExtension(newExt)

            files = glob.glob(filename.toOsSpecific())
            if not files:
                files = [filename.toOsSpecific()]

        explicit = (len(files) == 1)

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
            name = newName
            if not name:
                name = prefix + basename
                
            self.currentPackage.addFile(
                filename, newName = name, extract = extract,
                explicit = explicit, executable = executable,
                deleteTemp = deleteTemp)

    def exclude(self, filename):
        """ Marks the indicated filename as not to be included.  The
        filename may include shell globbing characters, and may or may
        not include a dirname.  (If it does not include a dirname, it
        refers to any file with the given basename from any
        directory.)"""

        if not self.currentPackage:
            raise OutsideOfPackageError

        self.currentPackage.excludeFile(filename)

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
            self.currentPackage.addFile(filename, newName = newName,
                                        explicit = False)
        else:
            if ext == 'pz':
                # Strip off an implicit .pz extension.
                newFilename = Filename(filename)
                newFilename.setExtension('')
                newFilename = Filename(newFilename.cStr())
                ext = newFilename.getExtension()

            if ext in self.knownExtensions:
                self.currentPackage.addFile(filename, newName = newName,
                                            explicit = False)
