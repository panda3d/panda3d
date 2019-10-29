""" This module is used to build a "Package", a collection of files
within a Panda3D Multifile, which can be easily be downloaded and/or
patched onto a client machine, for the purpose of running a large
application.

.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""

__all__ = ["Packager", "PackagerError", "OutsideOfPackageError", "ArgumentError"]

# Important to import panda3d first, to avoid naming conflicts with
# Python's "string" and "Loader" names that are imported later.
from panda3d.core import *
import sys
import os
import glob
import struct
import subprocess
import copy
from direct.p3d.FileSpec import FileSpec
from direct.p3d.SeqValue import SeqValue
from direct.p3d.HostInfo import HostInfo
from direct.showbase import Loader
from direct.showbase import AppRunnerGlobal
from direct.dist import FreezeTool
from direct.directnotify.DirectNotifyGlobal import *

vfs = VirtualFileSystem.getGlobalPtr()

class PackagerError(Exception):
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
                     text = None, unprocessed = None,
                     executable = None, dependencyDir = None,
                     platformSpecific = None, required = False):
            assert isinstance(filename, Filename)
            self.filename = Filename(filename)
            self.newName = newName
            self.deleteTemp = deleteTemp
            self.explicit = explicit
            self.compress = compress
            self.extract = extract
            self.text = text
            self.unprocessed = unprocessed
            self.executable = executable
            self.dependencyDir = dependencyDir
            self.platformSpecific = platformSpecific
            self.required = required

            if not self.newName:
                self.newName = str(self.filename)

            ext = Filename(self.newName).getExtension()
            if ext == 'pz' or ext == 'gz':
                # Strip off a .pz extension; we can compress files
                # within the Multifile without it.
                filename = Filename(self.newName)
                filename.setExtension('')
                self.newName = str(filename)
                ext = Filename(self.newName).getExtension()
                if self.compress is None:
                    self.compress = True

            packager = package.packager
            if self.compress is None:
                self.compress = (ext not in packager.uncompressibleExtensions and ext not in packager.imageExtensions)

            if self.executable is None:
                self.executable = (ext in packager.executableExtensions)

            if self.executable and self.dependencyDir is None:
                # By default, install executable dependencies in the
                # root directory, which is the one that's added to PATH.
                self.dependencyDir = ''

            if self.extract is None:
                self.extract = self.executable or (ext in packager.extractExtensions)
            if self.platformSpecific is None:
                self.platformSpecific = self.executable or (ext in packager.platformSpecificExtensions)

            if self.unprocessed is None:
                self.unprocessed = self.executable or (ext in packager.unprocessedExtensions)

            if self.executable:
                # Look up the filename along the system PATH, if necessary.
                if not packager.resolveLibrary(self.filename):
                    # If it wasn't found, try looking it up under its
                    # basename only.  Sometimes a Mac user will copy
                    # the library file out of a framework and put that
                    # along the PATH, instead of the framework itself.
                    basename = Filename(self.filename.getBasename())
                    if packager.resolveLibrary(basename):
                        self.filename = basename

            if ext in packager.textExtensions and not self.executable:
                self.filename.setText()
            else:
                self.filename.setBinary()

            # Convert the filename to an unambiguous filename for
            # searching.
            self.filename.makeTrueCase()
            if self.filename.exists() or not self.filename.isLocal():
                self.filename.makeCanonical()

        def isExcluded(self, package):
            """ Returns true if this file should be excluded or
            skipped, false otherwise. """

            if self.newName.lower() in package.skipFilenames:
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

                # A platform-specific file is implicitly excluded from
                # not-platform-specific packages.
                if self.platformSpecific and package.platformSpecificConfig is False:
                    return True

            return False

    class ExcludeFilename:
        def __init__(self, packager, filename, caseSensitive):
            self.packager = packager
            self.localOnly = (not filename.getDirname())
            if not self.localOnly:
                filename = Filename(filename)
                filename.makeCanonical()
            self.glob = GlobPattern(str(filename))

            if self.packager.platform.startswith('win'):
                self.glob.setCaseSensitive(False)
            elif self.packager.platform.startswith('osx'):
                self.glob.setCaseSensitive(False)

        def matches(self, filename):
            if self.localOnly:
                return self.glob.matches(filename.getBasename())
            else:
                return self.glob.matches(str(filename))

    class PackageEntry:
        """ This corresponds to a <package> entry in the contents.xml
        file. """

        def __init__(self):
            # The "seq" value increments automatically with each publish.
            self.packageSeq = SeqValue()

            # The "set_ver" value is optionally specified in the pdef
            # file and does not change unless the user says it does.
            self.packageSetVer = SeqValue()

        def getKey(self):
            """ Returns a tuple used for sorting the PackageEntry
            objects uniquely per package. """
            return (self.packageName, self.platform or "", self.version or "")

        def fromFile(self, packageName, platform, version, solo, perPlatform,
                     installDir, descFilename, importDescFilename):
            self.packageName = packageName
            self.platform = platform
            self.version = version
            self.solo = solo
            self.perPlatform = perPlatform

            self.descFile = FileSpec()
            self.descFile.fromFile(installDir, descFilename)

            self.importDescFile = None
            if importDescFilename:
                self.importDescFile = FileSpec()
                self.importDescFile.fromFile(installDir, importDescFilename)

        def loadXml(self, xpackage):
            self.packageName = xpackage.Attribute('name')
            self.platform = xpackage.Attribute('platform')
            self.version = xpackage.Attribute('version')
            solo = xpackage.Attribute('solo')
            self.solo = int(solo or '0')
            perPlatform = xpackage.Attribute('per_platform')
            self.perPlatform = int(perPlatform or '0')

            self.packageSeq = SeqValue()
            self.packageSeq.loadXml(xpackage, 'seq')

            self.packageSetVer = SeqValue()
            self.packageSetVer.loadXml(xpackage, 'set_ver')

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
            if self.perPlatform:
                xpackage.SetAttribute('per_platform', '1')

            self.packageSeq.storeXml(xpackage, 'seq')
            self.packageSetVer.storeXml(xpackage, 'set_ver')
            self.descFile.storeXml(xpackage)

            if self.importDescFile:
                ximport = TiXmlElement('import')
                self.importDescFile.storeXml(ximport)
                xpackage.InsertEndChild(ximport)

            return xpackage

    class HostEntry:
        def __init__(self, url = None, downloadUrl = None,
                     descriptiveName = None, hostDir = None,
                     mirrors = None):
            self.url = url
            self.downloadUrl = downloadUrl
            self.descriptiveName = descriptiveName
            self.hostDir = hostDir
            self.mirrors = mirrors or []
            self.altHosts = {}

        def loadXml(self, xhost, packager):
            self.url = xhost.Attribute('url')
            self.downloadUrl = xhost.Attribute('download_url')
            self.descriptiveName = xhost.Attribute('descriptive_name')
            self.hostDir = xhost.Attribute('host_dir')
            self.mirrors = []
            xmirror = xhost.FirstChildElement('mirror')
            while xmirror:
                url = xmirror.Attribute('url')
                self.mirrors.append(url)
                xmirror = xmirror.NextSiblingElement('mirror')

            xalthost = xhost.FirstChildElement('alt_host')
            while xalthost:
                url = xalthost.Attribute('url')
                he = packager.addHost(url)
                he.loadXml(xalthost, packager)
                xalthost = xalthost.NextSiblingElement('alt_host')

        def makeXml(self, packager = None):
            """ Returns a new TiXmlElement. """
            xhost = TiXmlElement('host')
            xhost.SetAttribute('url', self.url)
            if self.downloadUrl and self.downloadUrl != self.url:
                xhost.SetAttribute('download_url', self.downloadUrl)
            if self.descriptiveName:
                xhost.SetAttribute('descriptive_name', self.descriptiveName)
            if self.hostDir:
                xhost.SetAttribute('host_dir', self.hostDir)

            for mirror in self.mirrors:
                xmirror = TiXmlElement('mirror')
                xmirror.SetAttribute('url', mirror)
                xhost.InsertEndChild(xmirror)

            if packager:
                altHosts = sorted(self.altHosts.items())
                for keyword, alt in altHosts:
                    he = packager.hosts.get(alt, None)
                    if he:
                        xalthost = he.makeXml()
                        xalthost.SetValue('alt_host')
                        xalthost.SetAttribute('keyword', keyword)
                        xhost.InsertEndChild(xalthost)

            return xhost


    class Package:
        """ This is the full information on a particular package we
        are constructing.  Don't confuse it with PackageEntry, above,
        which contains only the information found in the toplevel
        contents.xml file."""

        def __init__(self, packageName, packager):
            self.packageName = packageName
            self.packager = packager
            self.notify = packager.notify

            # The platform is initially None until we know the file is
            # platform-specific.
            self.platform = None

            # This is always true on modern packages.
            self.perPlatform = True

            # The arch string, though, is pre-loaded from the system
            # arch string, so we can sensibly call otool.
            self.arch = self.packager.arch

            self.version = None
            self.host = None
            self.p3dApplication = False
            self.solo = False
            self.compressionLevel = 0
            self.importedMapsDir = 'imported_maps'
            self.mainModule = None
            self.signParams = []
            self.requires = []

            # This may be set explicitly in the pdef file to a
            # particular sequence value.
            self.packageSetVer = SeqValue()

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

            # This is the set of files and modules that are
            # required and may not be excluded from the package.
            self.requiredFilenames = []
            self.requiredModules = []

            # A list of required packages that were missing.
            self.missingPackages = []

            # This records the current list of modules we have added so
            # far.
            self.freezer = FreezeTool.Freezer(platform = self.packager.platform)
            self.freezer.storePythonSource = self.packager.storePythonSource

            # Map of extensions to files to number (ignored by dir)
            self.ignoredDirFiles = {}

        def close(self):
            """ Writes out the contents of the current package.  Returns True
            if the package was constructed successfully, False if one or more
            required files or modules are missing. """

            if not self.p3dApplication and not self.packager.allowPackages:
                message = 'Cannot generate packages without an installDir; use -i'
                raise PackagerError(message)

            if self.ignoredDirFiles:
                exts = sorted(self.ignoredDirFiles.keys())
                total = sum([x for x in self.ignoredDirFiles.values()])
                self.notify.warning("excluded %s files not marked for inclusion: %s" \
                                    % (total, ", ".join(["'" + ext + "'" for ext in exts])))

            if not self.host:
                self.host = self.packager.host

            # Check the version config variable.
            version = self.configs.get('version', None)
            if version is not None:
                self.version = version
                del self.configs['version']

            # Check the platform_specific config variable.  This has
            # only three settings: None (unset), True, or False.
            self.platformSpecificConfig = self.configs.get('platform_specific', None)
            if self.platformSpecificConfig is not None:
                # First, convert it to an int, in case it's "0" or "1".
                try:
                    self.platformSpecificConfig = int(self.platformSpecificConfig)
                except ValueError:
                    pass
                # Then, make it a bool.
                self.platformSpecificConfig = bool(self.platformSpecificConfig)
                del self.configs['platform_specific']

            # A special case when building the "panda3d" package.  We
            # enforce that the version number matches what we've been
            # compiled with.
            if self.packageName == 'panda3d':
                if self.version is None:
                    self.version = PandaSystem.getPackageVersionString()

                if self.version != PandaSystem.getPackageVersionString():
                    message = 'mismatched Panda3D version: requested %s, but Panda3D is built as %s' % (self.version, PandaSystem.getPackageVersionString())
                    raise PackagerError(message)

                if self.host != PandaSystem.getPackageHostUrl():
                    message = 'mismatched Panda3D host: requested %s, but Panda3D is built as %s' % (self.host, PandaSystem.getPackageHostUrl())
                    raise PackagerError(message)

            if self.p3dApplication:
                # Default compression level for an app.
                self.compressionLevel = 6

                # Every p3dapp requires panda3d.
                if 'panda3d' not in [p.packageName for p in self.requires]:
                    assert not self.packager.currentPackage
                    self.packager.currentPackage = self
                    self.packager.do_require('panda3d')
                    self.packager.currentPackage = None

                # If this flag is set, enable allow_python_dev.
                if self.packager.allowPythonDev:
                    self.configs['allow_python_dev'] = True

            if not self.p3dApplication and not self.version:
                # If we don't have an implicit version, inherit the
                # version from the 'panda3d' package on our require
                # list.
                for p2 in self.requires:
                    if p2.packageName == 'panda3d' and p2.version:
                        self.version = p2.version
                        break

            if self.solo:
                result = self.installSolo()
            else:
                result = self.installMultifile()

            if self.p3dApplication:
                allowPythonDev = self.configs.get('allow_python_dev', 0)
                if int(allowPythonDev):
                    print("\n*** Generating %s.p3d with allow_python_dev enabled ***\n" % (self.packageName))

            return result


        def considerPlatform(self):
            # Check to see if any of the files are platform-specific,
            # making the overall package platform-specific.

            platformSpecific = self.platformSpecificConfig
            for file in self.files:
                if file.isExcluded(self):
                    # Skip this file.
                    continue
                if file.platformSpecific:
                    platformSpecific = True

            if platformSpecific and self.platformSpecificConfig is not False:
                if not self.platform:
                    self.platform = self.packager.platform

            if self.platform and self.platform.startswith('osx_'):
                # Get the OSX "arch" specification.
                self.arch = self.platform[4:]


        def installMultifile(self):
            """ Installs the package, either as a p3d application, or
            as a true package.  Either is implemented with a
            Multifile. """

            if self.missingPackages:
                missing = ', '.join([name for name, version in self.missingPackages])
                self.notify.warning("Cannot build package %s due to missing dependencies: %s" % (self.packageName, missing))
                self.cleanup()
                return False

            self.multifile = Multifile()

            # Write the multifile to a temporary filename until we
            # know enough to determine the output filename.
            multifileFilename = Filename.temporary('', self.packageName + '.', '.mf')
            self.multifile.openReadWrite(multifileFilename)

            if self.p3dApplication:
                # p3d files should be tagged to make them executable.
                self.multifile.setHeaderPrefix('#! /usr/bin/env panda3d\n')
            else:
                # Package multifiles might be patched, and therefore
                # don't want to record an internal timestamp, which
                # would make patching less efficient.
                self.multifile.setRecordTimestamp(False)

            # Make sure that all required files are present.
            missing = []
            for file in self.requiredFilenames:
                if file not in self.files or file.isExcluded(self):
                    missing.append(file.filename.getBasename())
            if len(missing) > 0:
                self.notify.warning("Cannot build package %s, missing required files: %r" % (self.packageName, missing))
                self.cleanup()
                return False

            self.extracts = []
            self.components = []

            # Add the explicit py files that were requested by the
            # pdef file.  These get turned into Python modules.
            for file in self.files:
                if file.isExcluded(self):
                    # Skip this file.
                    continue
                if file.unprocessed:
                    # Unprocessed files get dealt with below.
                    continue

                ext = Filename(file.newName).getExtension()
                if ext == 'dc':
                    # Add the modules named implicitly in the dc file.
                    self.addDcImports(file)

                elif ext == 'py':
                    self.addPyFile(file)

            # Add the main module, if any.
            if not self.mainModule and self.p3dApplication:
                message = 'No main_module specified for application %s' % (self.packageName)
                raise PackagerError(message)
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

            # But first, make sure that all required modules are present.
            missing = []
            moduleDict = dict(self.freezer.getModuleDefs())
            for module in self.requiredModules:
                if module not in moduleDict:
                    missing.append(module)
            if len(missing) > 0:
                self.notify.warning("Cannot build package %s, missing required modules: %r" % (self.packageName, missing))
                self.cleanup()
                return False

            # OK, we can add it.
            self.freezer.addToMultifile(self.multifile, self.compressionLevel)
            self.addExtensionModules()

            # Add known module names.
            self.moduleNames = {}
            modules = sorted(self.freezer.modules.items())
            for newName, mdef in modules:
                if mdef.guess:
                    # Not really a module.
                    continue

                if mdef.fromSource == 'skip':
                    # This record already appeared in a required
                    # module; don't repeat it now.
                    continue

                if mdef.exclude and mdef.implicit:
                    # Don't bother mentioning implicitly-excluded
                    # (i.e. missing) modules.
                    continue

                #if newName == '__main__':
                #    # Ignore this special case.
                #    continue

                self.moduleNames[newName] = mdef

                xmodule = TiXmlElement('module')
                xmodule.SetAttribute('name', newName)
                if mdef.exclude:
                    xmodule.SetAttribute('exclude', '1')
                if mdef.forbid:
                    xmodule.SetAttribute('forbid', '1')
                if mdef.exclude and mdef.allowChildren:
                    xmodule.SetAttribute('allowChildren', '1')
                self.components.append(('m', newName.lower(), xmodule))

            # Now look for implicit shared-library dependencies.
            if self.packager.platform.startswith('win'):
                self.__addImplicitDependenciesWindows()
            elif self.packager.platform.startswith('osx'):
                self.__addImplicitDependenciesOSX()
            else:
                self.__addImplicitDependenciesPosix()

            # Now add all the real, non-Python files (except model
            # files).  This will include the extension modules we just
            # discovered above.
            for file in self.files:
                if file.isExcluded(self):
                    # Skip this file.
                    continue
                ext = Filename(file.newName).getExtension()
                if file.unprocessed:
                    # Add an unprocessed file verbatim.
                    self.addComponent(file)
                elif ext == 'py':
                    # Already handled, above.
                    pass
                elif file.isExcluded(self):
                    # Skip this file.
                    pass
                elif ext == 'egg' or ext == 'bam':
                    # Skip model files this pass.
                    pass
                elif ext == 'dc':
                    # dc files get a special treatment.
                    self.addDcFile(file)
                elif ext == 'prc':
                    # So do prc files.
                    self.addPrcFile(file)
                else:
                    # Any other file.
                    self.addComponent(file)

            # Now add the model files.  It's important to add these
            # after we have added all of the texture files, so we can
            # determine which textures need to be implicitly pulled
            # in.

            # We walk through a copy of the files list, since we might
            # be adding more files (textures) to this list as we
            # discover them in model files referenced in this list.
            for file in self.files[:]:
                if file.isExcluded(self):
                    # Skip this file.
                    continue
                ext = Filename(file.newName).getExtension()
                if file.unprocessed:
                    # Already handled, above.
                    pass
                elif ext == 'py':
                    # Already handled, above.
                    pass
                elif file.isExcluded(self):
                    # Skip this file.
                    pass
                elif ext == 'egg':
                    self.addEggFile(file)
                elif ext == 'bam':
                    self.addBamFile(file)
                else:
                    # Handled above.
                    pass

            # Check to see if we should be platform-specific.
            self.considerPlatform()

            # Now that we've processed all of the component files,
            # (and set our platform if necessary), we can generate the
            # output filename and write the output files.

            self.packageBasename = self.packageName
            packageDir = self.packageName
            if self.version:
                self.packageBasename += '.' + self.version
                packageDir += '/' + self.version
            if self.platform:
                self.packageBasename += '.' + self.platform
                packageDir += '/' + self.platform

            self.packageDesc = self.packageBasename + '.xml'
            self.packageImportDesc = self.packageBasename + '.import.xml'
            if self.p3dApplication:
                self.packageBasename += self.packager.p3dSuffix
                self.packageBasename += '.p3d'
                packageDir = ''
            else:
                self.packageBasename += '.mf'
                packageDir += '/'

            self.packageDir = packageDir
            self.packageFilename = packageDir + self.packageBasename
            self.packageDesc = packageDir + self.packageDesc
            self.packageImportDesc = packageDir + self.packageImportDesc

            print("Generating %s" % (self.packageFilename))

            if self.p3dApplication:
                self.packageFullpath = Filename(self.packager.p3dInstallDir, self.packageFilename)
                self.packageFullpath.makeDir()
                self.makeP3dInfo()
            else:
                self.packageFullpath = Filename(self.packager.installDir, self.packageFilename)
                self.packageFullpath.makeDir()

            self.multifile.repack()

            # Also sign the multifile before we close it.
            for certificate, chain, pkey, password in self.signParams:
                self.multifile.addSignature(certificate, chain or '', pkey or '', password or '')

            self.multifile.close()

            if not multifileFilename.renameTo(self.packageFullpath):
                self.notify.error("Cannot move %s to %s" % (multifileFilename, self.packageFullpath))

            if self.p3dApplication:
                # No patches for an application; just move it into place.
                # Make the application file executable.
                os.chmod(self.packageFullpath.toOsSpecific(), 0o755)
            else:
                self.readDescFile()
                self.packageSeq += 1
                self.perPlatform = True  # always true on modern packages.
                self.compressMultifile()
                self.writeDescFile()
                self.writeImportDescFile()

                # Now that we've written out the desc file, we don't
                # need to keep around the uncompressed archive
                # anymore.
                self.packageFullpath.unlink()

                # Replace or add the entry in the contents.
                pe = Packager.PackageEntry()
                pe.fromFile(self.packageName, self.platform, self.version,
                            False, self.perPlatform, self.packager.installDir,
                            self.packageDesc, self.packageImportDesc)
                pe.packageSeq = self.packageSeq
                pe.packageSetVer = self.packageSetVer

                self.packager.contents[pe.getKey()] = pe
                self.packager.contentsChanged = True

            self.cleanup()
            return True

        def installSolo(self):
            """ Installs the package as a "solo", which means we
            simply copy the one file into the install directory.  This
            is primarily intended for the "coreapi" plugin, which is
            just a single dll and a jpg file; but it can support other
            kinds of similar "solo" packages as well. """

            self.considerPlatform()
            self.perPlatform = False  # Not true on "solo" packages.

            packageDir = self.packageName
            if self.platform:
                packageDir += '/' + self.platform
            if self.version:
                packageDir += '/' + self.version

            if not self.packager.allowPackages:
                message = 'Cannot generate packages without an installDir; use -i'
                raise PackagerError(message)

            installPath = Filename(self.packager.installDir, packageDir)
            # Remove any files already in the installPath.
            origFiles = vfs.scanDirectory(installPath)
            if origFiles:
                for origFile in origFiles:
                    origFile.getFilename().unlink()

            files = []
            for file in self.files:
                if file.isExcluded(self):
                    # Skip this file.
                    continue
                files.append(file)

            if not files:
                # No files, never mind.
                return

            if len(files) != 1:
                raise PackagerError('Multiple files in "solo" package %s' % (self.packageName))

            Filename(installPath, '').makeDir()

            file = files[0]
            targetPath = Filename(installPath, file.newName)
            targetPath.setBinary()
            file.filename.setBinary()
            if not file.filename.copyTo(targetPath):
                self.notify.warning("Could not copy %s to %s" % (
                    file.filename, targetPath))

            # Replace or add the entry in the contents.
            pe = Packager.PackageEntry()
            pe.fromFile(self.packageName, self.platform, self.version,
                        True, self.perPlatform, self.packager.installDir,
                        Filename(packageDir, file.newName), None)
            peOrig = self.packager.contents.get(pe.getKey(), None)
            if peOrig:
                pe.packageSeq = peOrig.packageSeq + 1
                pe.packageSetVer = peOrig.packageSetVer
            if self.packageSetVer:
                pe.packageSetVer = self.packageSetVer

            self.packager.contents[pe.getKey()] = pe
            self.packager.contentsChanged = True

            # Hack for coreapi package, to preserve backward compatibility
            # with old versions of the runtime, which still called the
            # 32-bit Windows platform "win32".
            if self.packageName == "coreapi" and self.platform == "win_i386":
                pe2 = copy.copy(pe)
                pe2.platform = "win32"
                self.packager.contents[pe2.getKey()] = pe2

            self.cleanup()
            return True

        def cleanup(self):
            # Now that all the files have been packed, we can delete
            # the temporary files.
            for file in self.files:
                if file.deleteTemp:
                    file.filename.unlink()

        def addFile(self, *args, **kw):
            """ Adds the named file to the package.  Returns the file
            object, or None if it was not added by this call. """

            file = Packager.PackFile(self, *args, **kw)
            if file.filename in self.sourceFilenames:
                # Don't bother, it's already here.
                return None

            lowerName = file.newName.lower()
            if lowerName in self.targetFilenames:
                # Another file is already in the same place.
                file2 = self.targetFilenames[lowerName]
                self.packager.notify.warning(
                    "%s is shadowing %s" % (file2.filename, file.filename))
                return None

            self.sourceFilenames[file.filename] = file
            if file.required:
                self.requiredFilenames.append(file)

            if file.text is None and not file.filename.exists():
                if not file.isExcluded(self):
                    self.packager.notify.warning("No such file: %s" % (file.filename))
                return None

            self.files.append(file)
            self.targetFilenames[lowerName] = file

            return file

        def excludeFile(self, filename):
            """ Excludes the named file (or glob pattern) from the
            package. """
            xfile = Packager.ExcludeFilename(self.packager, filename, self.packager.caseSensitive)
            self.excludedFilenames.append(xfile)

        def __addImplicitDependenciesWindows(self):
            """ Walks through the list of files, looking for dll's and
            exe's that might include implicit dependencies on other
            dll's and assembly manifests.  Tries to determine those
            dependencies, and adds them back into the filelist. """

            # We walk through the list as we modify it.  That's OK,
            # because we want to follow the transitive closure of
            # dependencies anyway.
            for file in self.files:
                if not file.executable:
                    continue

                if file.isExcluded(self):
                    # Skip this file.
                    continue

                if file.filename.getExtension().lower() == "manifest":
                    filenames = self.__parseManifest(file.filename)
                    if filenames is None:
                        self.notify.warning("Unable to determine dependent assemblies from %s" % (file.filename))
                        continue

                else:
                    tempFile = Filename.temporary('', 'p3d_', '.txt')
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
                        tempFile.unlink()
                    if filenames is None:
                        self.notify.warning("Unable to determine dependencies from %s" % (file.filename))
                        filenames = []

                    # Extract the manifest file so we can figure out
                    # the dependent assemblies.
                    tempFile = Filename.temporary('', 'p3d_', '.manifest')
                    resindex = 2
                    if file.filename.getExtension().lower() == "exe":
                        resindex = 1
                    command = 'mt -inputresource:"%s";#%d -out:"%s" > nul' % (
                        file.filename.toOsSpecific(),
                        resindex, tempFile.toOsSpecific())
                    try:
                        out = os.system(command)
                    except:
                        pass
                    afilenames = None

                    if tempFile.exists():
                        afilenames = self.__parseManifest(tempFile)
                        tempFile.unlink()

                    # Also check for an explicit private-assembly
                    # manifest file on disk.
                    mfile = file.filename + '.manifest'
                    if mfile.exists():
                        if afilenames is None:
                            afilenames = []
                        afilenames += self.__parseManifest(mfile)
                        # Since it's an explicit manifest file, it
                        # means we should include the manifest
                        # file itself in the package.
                        newName = Filename(file.dependencyDir, mfile.getBasename())
                        self.addFile(mfile, newName = str(newName),
                                     explicit = False, executable = True)

                    if afilenames is None and out != 31:
                        self.notify.warning("Unable to determine dependent assemblies from %s" % (file.filename))

                    if afilenames is not None:
                        filenames += afilenames

                # Attempt to resolve the dependent filename relative
                # to the original filename, before we resolve it along
                # the PATH.
                path = DSearchPath(Filename(file.filename.getDirname()))

                for filename in filenames:
                    filename = Filename.fromOsSpecific(filename)
                    filename.resolveFilename(path)
                    filename.makeTrueCase()

                    newName = Filename(file.dependencyDir, filename.getBasename())
                    self.addFile(filename, newName = str(newName),
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

        def __parseManifest(self, tempFile):
            """ Reads the indicated application manifest file, to
            determine the list of dependent assemblies this
            executable file depends on. """

            doc = TiXmlDocument(tempFile.toOsSpecific())
            if not doc.LoadFile():
                return None

            assembly = doc.FirstChildElement("assembly")
            if not assembly:
                return None

            # Pick up assemblies that it depends on
            filenames = []
            dependency = assembly.FirstChildElement("dependency")
            while dependency:
                depassembly = dependency.FirstChildElement("dependentAssembly")
                if depassembly:
                    ident = depassembly.FirstChildElement("assemblyIdentity")
                    if ident:
                        name = ident.Attribute("name")
                        if name:
                            filenames.append(name + ".manifest")

                dependency = dependency.NextSiblingElement("dependency")

            # Pick up direct dll dependencies that it lists
            dfile = assembly.FirstChildElement("file")
            while dfile:
                name = dfile.Attribute("name")
                if name:
                    filenames.append(name)
                dfile = dfile.NextSiblingElement("file")

            return filenames

        def __locateFrameworkLibrary(self, library):
            """ Locates the given library inside its framework on the
            default framework paths, and returns its location as Filename. """

            # If it's already a full existing path, we
            # don't search for it anymore, of course.
            if Filename.fromOsSpecific(library).exists():
                return Filename.fromOsSpecific(library)

            # DSearchPath appears not to work for directories.
            fpath = []
            fpath.append(Filename("/Library/Frameworks"))
            fpath.append(Filename("/System/Library/Frameworks"))
            fpath.append(Filename("/Developer/Library/Frameworks"))
            fpath.append(Filename(os.path.expanduser("~"), "Library/Frameworks"))
            if "HOME" in os.environ:
                fpath.append(Filename(os.environ["HOME"], "Library/Frameworks"))
            ffilename = Filename(library.split('.framework/', 1)[0].split('/')[-1] + '.framework')
            ffilename = Filename(ffilename, library.split('.framework/', 1)[-1])

            # Look under the system root first, if supplied.
            if self.packager.systemRoot:
                for i in fpath:
                    fw = Filename(self.packager.systemRoot, i)
                    if Filename(fw, ffilename).exists():
                        return Filename(fw, ffilename)

            for i in fpath:
                if Filename(i, ffilename).exists():
                    return Filename(i, ffilename)

            # Not found? Well, let's just return the framework + file
            # path, the user will be presented with a warning later.
            return ffilename

        def __alterFrameworkDependencies(self, file, framework_deps):
            """ Copies the given library file to a temporary directory,
            and alters the dependencies so that it doesn't contain absolute
            framework dependencies. """

            if not file.deleteTemp:
                # Copy the file to a temporary location because we
                # don't want to modify the original (there's a big
                # chance that we break it).

                # Copy it every time, because the source file might
                # have changed since last time we ran.
                assert file.filename.exists(), "File doesn't exist: %s" % file.filename
                tmpfile = Filename.temporary('', "p3d_" + file.filename.getBasename())
                tmpfile.setBinary()
                file.filename.copyTo(tmpfile)
                file.filename = tmpfile
                file.deleteTemp = True

            # Alter the dependencies to have a relative path rather than absolute
            for filename in framework_deps:
                loc = self.__locateFrameworkLibrary(filename)

                if loc == file.filename:
                    os.system('install_name_tool -id "%s" "%s"' % (os.path.basename(filename), file.filename.toOsSpecific()))
                elif "/System/" in loc.toOsSpecific():
                    # Let's keep references to system frameworks absolute
                    os.system('install_name_tool -change "%s" "%s" "%s"' % (filename, loc.toOsSpecific(), file.filename.toOsSpecific()))
                else:
                    os.system('install_name_tool -change "%s" "%s" "%s"' % (filename, os.path.basename(filename), file.filename.toOsSpecific()))

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

                origFilename = Filename(file.filename)

                tempFile = Filename.temporary('', 'p3d_', '.txt')
                command = '/usr/bin/otool -arch all -L "%s" >"%s"' % (
                    origFilename.toOsSpecific(),
                    tempFile.toOsSpecific())
                if self.arch:
                    arch = self.arch
                    if arch == "amd64":
                        arch = "x86_64"
                    command = '/usr/bin/otool -arch %s -L "%s" >"%s"' % (
                        arch,
                        origFilename.toOsSpecific(),
                        tempFile.toOsSpecific())
                exitStatus = os.system(command)
                if exitStatus != 0:
                    self.notify.warning('Command failed: %s' % (command))
                filenames = None

                if tempFile.exists():
                    filenames = self.__parseDependenciesOSX(tempFile)
                    tempFile.unlink()
                if filenames is None:
                    self.notify.warning("Unable to determine dependencies from %s" % (origFilename))
                    continue

                # Attempt to resolve the dependent filename relative
                # to the original filename, before we resolve it along
                # the PATH.
                path = DSearchPath(Filename(origFilename.getDirname()))

                # Find the dependencies that are referencing a framework
                framework_deps = []
                for filename in filenames:
                    if '.framework/' in filename:
                        framework_deps.append(filename)

                if len(framework_deps) > 0:
                    # Fixes dependencies like @executable_path/../Library/Frameworks/Cg.framework/Cg
                    self.__alterFrameworkDependencies(file, framework_deps)

                for filename in filenames:
                    if '@loader_path' in filename:
                        filename = filename.replace('@loader_path', origFilename.getDirname())

                    if False and '.framework/' in filename:
                        # It references a framework, and besides the fact
                        # that those often contain absolute paths, they
                        # aren't commonly on the library path either.
                        filename = self.__locateFrameworkLibrary(filename)
                        filename.setBinary()
                    else:
                        # It's just a normal library - find it on the path.
                        filename = Filename.fromOsSpecific(filename)
                        filename.setBinary()

                        if filename.isLocal():
                            filename.resolveFilename(path)
                        else:
                            # It's a fully-specified filename; look
                            # for it under the system root first.
                            if self.packager.systemRoot:
                                f2 = Filename(self.packager.systemRoot, filename)
                                if f2.exists():
                                    filename = f2

                    # Skip libraries and frameworks in system directory
                    if "/System/" in filename.toOsSpecific():
                        continue

                    newName = Filename(file.dependencyDir, filename.getBasename())
                    self.addFile(filename, newName = str(newName),
                                 explicit = False, executable = True)

        def __parseDependenciesOSX(self, tempFile):
            """ Reads the indicated temporary file, the output from
            otool -L, to determine the list of dylibs this
            executable file depends on. """

            lines = open(tempFile.toOsSpecific(), 'rU').readlines()

            filenames = []
            for line in lines:
                if not line[0].isspace():
                    continue
                line = line.strip()
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

        def __readAndStripELF(self, file):
            """ Reads the indicated ELF binary, and returns a list with
            dependencies.  If it contains data that should be stripped,
            it writes the stripped library to a temporary file.  Returns
            None if the file failed to read (e.g. not an ELF file). """

            # Read the first 16 bytes, which identify the ELF file.
            elf = open(file.filename.toOsSpecific(), 'rb')
            try:
                ident = elf.read(16)
            except IOError:
                elf.close()
                return None

            if not ident.startswith(b"\177ELF"):
                # No elf magic!  Beware of orcs.
                return None

            # Make sure we read in the correct endianness and integer size
            byteOrder = "<>"[ord(ident[5:6]) - 1]
            elfClass = ord(ident[4:5]) - 1 # 0 = 32-bits, 1 = 64-bits
            headerStruct = byteOrder + ("HHIIIIIHHHHHH", "HHIQQQIHHHHHH")[elfClass]
            sectionStruct = byteOrder + ("4xI8xIII8xI", "4xI16xQQI12xQ")[elfClass]
            dynamicStruct = byteOrder + ("iI", "qQ")[elfClass]

            type, machine, version, entry, phoff, shoff, flags, ehsize, phentsize, phnum, shentsize, shnum, shstrndx \
              = struct.unpack(headerStruct, elf.read(struct.calcsize(headerStruct)))
            dynamicSections = []
            stringTables = {}

            # Seek to the section header table and find the .dynamic section.
            elf.seek(shoff)
            for i in range(shnum):
                type, offset, size, link, entsize = struct.unpack_from(sectionStruct, elf.read(shentsize))
                if type == 6 and link != 0: # DYNAMIC type, links to string table
                    dynamicSections.append((offset, size, link, entsize))
                    stringTables[link] = None

            # Read the relevant string tables.
            for idx in stringTables.keys():
                elf.seek(shoff + idx * shentsize)
                type, offset, size, link, entsize = struct.unpack_from(sectionStruct, elf.read(shentsize))
                if type != 3: continue
                elf.seek(offset)
                stringTables[idx] = elf.read(size)

            # Loop through the dynamic sections and rewrite it if it has an rpath/runpath.
            rewriteSections = []
            filenames = []
            rpath = []
            for offset, size, link, entsize in dynamicSections:
                elf.seek(offset)
                data = elf.read(entsize)
                tag, val = struct.unpack_from(dynamicStruct, data)
                newSectionData = b""
                startReplace = None
                pad = 0

                # Read tags until we find a NULL tag.
                while tag != 0:
                    if tag == 1: # A NEEDED entry.  Read it from the string table.
                        filenames.append(stringTables[link][val : stringTables[link].find(b'\0', val)])

                    elif tag == 15 or tag == 29:
                        rpath += stringTables[link][val : stringTables[link].find(b'\0', val)].split(b':')
                        # An RPATH or RUNPATH entry.
                        if not startReplace:
                            startReplace = elf.tell() - entsize
                        if startReplace:
                            pad += entsize

                    elif startReplace is not None:
                        newSectionData += data

                    data = elf.read(entsize)
                    tag, val = struct.unpack_from(dynamicStruct, data)

                if startReplace is not None:
                    newSectionData += data + (b"\0" * pad)
                    rewriteSections.append((startReplace, newSectionData))
            elf.close()

            # No rpaths/runpaths found, so nothing to do any more.
            if len(rewriteSections) == 0:
                return filenames

            # Attempt to resolve any of the directly
            # dependent filenames along the RPATH.
            for f in range(len(filenames)):
                filename = filenames[f]
                for rdir in rpath:
                    if os.path.isfile(os.path.join(rdir, filename)):
                        filenames[f] = os.path.join(rdir, filename)
                        break

            if not file.deleteTemp:
                # Copy the file to a temporary location because we
                # don't want to modify the original (there's a big
                # chance that we break it).

                tmpfile = Filename.temporary('', "p3d_" + file.filename.getBasename())
                tmpfile.setBinary()
                file.filename.copyTo(tmpfile)
                file.filename = tmpfile
                file.deleteTemp = True

            # Open the temporary file and rewrite the dynamic sections.
            elf = open(file.filename.toOsSpecific(), 'r+b')
            for offset, data in rewriteSections:
                elf.seek(offset)
                elf.write(data)
            elf.write(b"\0" * pad)
            elf.close()
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

                # Check if this is an ELF binary.
                filenames = self.__readAndStripELF(file)

                # If that failed, perhaps ldd will help us.
                if filenames is None:
                    self.notify.warning("Reading ELF library %s failed, using ldd instead" % (file.filename))
                    tempFile = Filename.temporary('', 'p3d_', '.txt')
                    command = 'ldd "%s" >"%s"' % (
                        file.filename.toOsSpecific(),
                        tempFile.toOsSpecific())
                    try:
                        os.system(command)
                    except:
                        pass

                    if tempFile.exists():
                        filenames = self.__parseDependenciesPosix(tempFile)
                        tempFile.unlink()

                if filenames is None:
                    self.notify.warning("Unable to determine dependencies from %s" % (file.filename))
                    continue

                # Attempt to resolve the dependent filename relative
                # to the original filename, before we resolve it along
                # the PATH.
                path = DSearchPath(Filename(file.filename.getDirname()))

                for filename in filenames:
                    # These vDSO's provided by Linux aren't
                    # supposed to be anywhere on the system.
                    if filename in ["linux-gate.so.1", "linux-vdso.so.1"]:
                        continue

                    filename = Filename.fromOsSpecific(filename)
                    filename.resolveFilename(path)
                    filename.setBinary()

                    newName = Filename(file.dependencyDir, filename.getBasename())
                    self.addFile(filename, newName = str(newName),
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
            for moduleName, filename in freezer.extras:
                filename = Filename.fromOsSpecific(filename)
                newName = filename.getBasename()
                if '.' in moduleName:
                    newName = '/'.join(moduleName.split('.')[:-1])
                    newName += '/' + filename.getBasename()
                # Sometimes the PYTHONPATH has the wrong case in it.
                filename.makeTrueCase()
                self.addFile(filename, newName = newName,
                             explicit = False, extract = True,
                             executable = True,
                             platformSpecific = True)
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

            self.__addConfigs(xpackage)

            requireHosts = {}
            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                xrequires.SetAttribute('host', package.host)
                package.packageSeq.storeXml(xrequires, 'seq')
                package.packageSetVer.storeXml(xrequires, 'set_ver')
                requireHosts[package.host] = True
                xpackage.InsertEndChild(xrequires)

            for host in requireHosts.keys():
                he = self.packager.hosts.get(host, None)
                if he:
                    xhost = he.makeXml(packager = self.packager)
                    xpackage.InsertEndChild(xhost)

            self.extracts.sort()
            for name, xextract in self.extracts:
                xpackage.InsertEndChild(xextract)

            doc.InsertEndChild(xpackage)

            # Write the xml file to a temporary file on disk, so we
            # can add it to the multifile.
            filename = Filename.temporary('', 'p3d_', '.xml')

            # This should really be setText() for an xml file, but it
            # doesn't really matter that much since tinyxml can read
            # it either way; and if we use setBinary() it will remain
            # compatible with older versions of the core API that
            # didn't understand the SF_text flag.
            filename.setBinary()

            doc.SaveFile(filename.toOsSpecific())

            # It's important not to compress this file: the core API
            # runtime can't decode compressed subfiles.
            self.multifile.addSubfile('p3d_info.xml', filename, 0)

            self.multifile.flush()
            filename.unlink()


        def compressMultifile(self):
            """ Compresses the .mf file into an .mf.pz file. """

            if self.oldCompressedBasename:
                # Remove the previous compressed file first.
                compressedPath = Filename(self.packager.installDir, Filename(self.packageDir, self.oldCompressedBasename))
                compressedPath.unlink()

            newCompressedFilename = '%s.pz' % (self.packageFilename)

            # Now build the new version.
            compressedPath = Filename(self.packager.installDir, newCompressedFilename)
            if not compressFile(self.packageFullpath, compressedPath, 6):
                message = 'Unable to write %s' % (compressedPath)
                raise PackagerError(message)

        def readDescFile(self):
            """ Reads the existing package.xml file before rewriting
            it.  We need this to preserve the list of patches, and
            similar historic data, between sessions. """

            self.packageSeq = SeqValue()
            self.packageSetVer = SeqValue()
            self.patchVersion = None
            self.patches = []

            self.oldCompressedBasename = None

            packageDescFullpath = Filename(self.packager.installDir, self.packageDesc)
            doc = TiXmlDocument(packageDescFullpath.toOsSpecific())
            if not doc.LoadFile():
                return

            xpackage = doc.FirstChildElement('package')
            if not xpackage:
                return

            perPlatform = xpackage.Attribute('per_platform')
            self.perPlatform = int(perPlatform or '0')

            self.packageSeq.loadXml(xpackage, 'seq')
            self.packageSetVer.loadXml(xpackage, 'set_ver')

            xcompressed = xpackage.FirstChildElement('compressed_archive')
            if xcompressed:
                compressedFilename = xcompressed.Attribute('filename')
                if compressedFilename:
                    self.oldCompressedBasename = compressedFilename

            patchVersion = xpackage.Attribute('patch_version')
            if not patchVersion:
                patchVersion = xpackage.Attribute('last_patch_version')
            if patchVersion:
                self.patchVersion = patchVersion

            # Extract the base_version, top_version, and patch
            # entries, if any, and preserve these entries verbatim for
            # the next version.
            xbase = xpackage.FirstChildElement('base_version')
            if xbase:
                self.patches.append(xbase.Clone())
            xtop = xpackage.FirstChildElement('top_version')
            if xtop:
                self.patches.append(xtop.Clone())

            xpatch = xpackage.FirstChildElement('patch')
            while xpatch:
                self.patches.append(xpatch.Clone())
                xpatch = xpatch.NextSiblingElement('patch')

        def writeDescFile(self):
            """ Makes the package.xml file that describes the package
            and its contents, for download. """

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
            if self.perPlatform:
                xpackage.SetAttribute('per_platform', '1')

            if self.patchVersion:
                xpackage.SetAttribute('last_patch_version', self.patchVersion)

            self.packageSeq.storeXml(xpackage, 'seq')
            self.packageSetVer.storeXml(xpackage, 'set_ver')

            self.__addConfigs(xpackage)

            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if self.platform and package.platform:
                    xrequires.SetAttribute('platform', package.platform)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                package.packageSeq.storeXml(xrequires, 'seq')
                package.packageSetVer.storeXml(xrequires, 'set_ver')
                xrequires.SetAttribute('host', package.host)
                xpackage.InsertEndChild(xrequires)

            xuncompressedArchive = self.getFileSpec(
                'uncompressed_archive', self.packageFullpath,
                self.packageBasename)
            xpackage.InsertEndChild(xuncompressedArchive)

            xcompressedArchive = self.getFileSpec(
                'compressed_archive', self.packageFullpath + '.pz',
                self.packageBasename + '.pz')
            xpackage.InsertEndChild(xcompressedArchive)

            # Copy in the patch entries read from the previous version
            # of the desc file.
            for xpatch in self.patches:
                xpackage.InsertEndChild(xpatch)

            self.extracts.sort()
            for name, xextract in self.extracts:
                xpackage.InsertEndChild(xextract)

            doc.InsertEndChild(xpackage)
            doc.SaveFile()

        def __addConfigs(self, xpackage):
            """ Adds the XML config values defined in self.configs to
            the indicated XML element. """

            if self.configs:
                xconfig = TiXmlElement('config')

                for variable, value in self.configs.items():
                    if sys.version_info < (3, 0) and isinstance(value, unicode):
                        xconfig.SetAttribute(variable, value.encode('utf-8'))
                    elif isinstance(value, bool):
                        # True or False must be encoded as 1 or 0.
                        xconfig.SetAttribute(variable, str(int(value)))
                    else:
                        xconfig.SetAttribute(variable, str(value))

                xpackage.InsertEndChild(xconfig)

        def writeImportDescFile(self):
            """ Makes the package.import.xml file that describes the
            package and its contents, for other packages and
            applications that may wish to "require" this one. """

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
            xpackage.SetAttribute('host', self.host)

            self.packageSeq.storeXml(xpackage, 'seq')
            self.packageSetVer.storeXml(xpackage, 'set_ver')

            requireHosts = {}
            requireHosts[self.host] = True

            for package in self.requires:
                xrequires = TiXmlElement('requires')
                xrequires.SetAttribute('name', package.packageName)
                if self.platform and package.platform:
                    xrequires.SetAttribute('platform', package.platform)
                if package.version:
                    xrequires.SetAttribute('version', package.version)
                xrequires.SetAttribute('host', package.host)
                package.packageSeq.storeXml(xrequires, 'seq')
                package.packageSetVer.storeXml(xrequires, 'set_ver')
                requireHosts[package.host] = True
                xpackage.InsertEndChild(xrequires)

            # Make sure we also write the full host descriptions for
            # any hosts we reference, so we can find these guys later.
            for host in requireHosts.keys():
                he = self.packager.hosts.get(host, None)
                if he:
                    xhost = he.makeXml(packager = self.packager)
                    xpackage.InsertEndChild(xhost)

            self.components.sort()
            for type, name, xcomponent in self.components:
                xpackage.InsertEndChild(xcomponent)

            doc.InsertEndChild(xpackage)
            doc.SaveFile()

        def readImportDescFile(self, filename):
            """ Reads the import desc file.  Returns True on success,
            False on failure. """

            self.packageSeq = SeqValue()
            self.packageSetVer = SeqValue()

            doc = TiXmlDocument(filename.toOsSpecific())
            if not doc.LoadFile():
                return False
            xpackage = doc.FirstChildElement('package')
            if not xpackage:
                return False

            self.packageName = xpackage.Attribute('name')
            self.platform = xpackage.Attribute('platform')
            self.version = xpackage.Attribute('version')
            self.host = xpackage.Attribute('host')

            # Get any new host descriptors.
            xhost = xpackage.FirstChildElement('host')
            while xhost:
                he = self.packager.HostEntry()
                he.loadXml(xhost, self)
                if he.url not in self.packager.hosts:
                    self.packager.hosts[he.url] = he
                xhost = xhost.NextSiblingElement('host')

            self.packageSeq.loadXml(xpackage, 'seq')
            self.packageSetVer.loadXml(xpackage, 'set_ver')

            self.requires = []
            xrequires = xpackage.FirstChildElement('requires')
            while xrequires:
                packageName = xrequires.Attribute('name')
                platform = xrequires.Attribute('platform')
                version = xrequires.Attribute('version')
                host = xrequires.Attribute('host')
                if packageName:
                    package = self.packager.findPackage(
                        packageName, platform = platform, version = version,
                        host = host, requires = self.requires)
                    if package:
                        self.requires.append(package)
                xrequires = xrequires.NextSiblingElement('requires')

            self.targetFilenames = {}
            xcomponent = xpackage.FirstChildElement('component')
            while xcomponent:
                name = xcomponent.Attribute('filename')
                if name:
                    self.targetFilenames[name.lower()] = True
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

            # Make sure that it is actually in a package.
            parentName = moduleName
            while '.' in parentName:
                parentName = parentName.rsplit('.', 1)[0]
                if parentName not in self.freezer.modules:
                    message = 'Cannot add Python file %s; not in package' % (file.newName)
                    if file.required or file.explicit:
                        raise Exception(message)
                    else:
                        self.notify.warning(message)
                    return

            if file.text:
                self.freezer.addModule(moduleName, filename = file.filename, text = file.text)
            else:
                self.freezer.addModule(moduleName, filename = file.filename)

        def addEggFile(self, file):
            # Precompile egg files to bam's.
            np = self.packager.loader.loadModel(file.filename, self.packager.loaderOptions)
            if not np:
                raise Exception('Could not read egg file %s' % (file.filename))

            bamName = Filename(file.newName)
            bamName.setExtension('bam')
            self.addNode(np.node(), file.filename, str(bamName))

        def addBamFile(self, file):
            # Load the bam file so we can massage its textures.
            bamFile = BamFile()
            if not bamFile.openRead(file.filename):
                raise Exception('Could not read bam file %s' % (file.filename))

            bamFile.getReader().setLoaderOptions(self.packager.loaderOptions)

            if not bamFile.resolve():
                raise Exception('Could not resolve bam file %s' % (file.filename))

            node = bamFile.readNode()
            if not node:
                raise Exception('Not a model file: %s' % (file.filename))

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
            self.components.append(('c', newName.lower(), xcomponent))

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
            while newName.lower() in self.targetFilenames:
                uniqueId += 1
                newName = '%s/%s_%s.%s' % (
                    self.importedMapsDir, filename.getBasenameWoExtension(),
                    uniqueId, filename.getExtension())

            file = self.addFile(
                filename, newName = newName, explicit = False,
                compress = False)

            if file:
                # If we added the file in this pass, then also
                # immediately add it to the multifile (because we
                # won't be visiting the files list again).
                self.addComponent(file)

            return newName

        def addDcFile(self, file):
            """ Adds a dc file to the archive.  A dc file gets its
            internal comments and parameter names stripped out of the
            final result automatically.  This is as close as we can
            come to "compiling" a dc file, since all of the remaining
            symbols are meaningful at runtime. """

            # First, read in the dc file
            from panda3d.direct import DCFile
            dcFile = DCFile()
            if not dcFile.read(file.filename):
                self.notify.error("Unable to parse %s." % (file.filename))

            # And then write it out without the comments and such.
            stream = StringStream()
            if not dcFile.write(stream, True):
                self.notify.error("Unable to write %s." % (file.filename))

            file.text = stream.getData()
            self.addComponent(file)

        def addDcImports(self, file):
            """ Adds the Python modules named by the indicated dc
            file. """

            from panda3d.direct import DCFile
            dcFile = DCFile()
            if not dcFile.read(file.filename):
                self.notify.error("Unable to parse %s." % (file.filename))

            for n in range(dcFile.getNumImportModules()):
                moduleName = dcFile.getImportModule(n)
                moduleSuffixes = []
                if '/' in moduleName:
                    moduleName, suffixes = moduleName.split('/', 1)
                    moduleSuffixes = suffixes.split('/')
                self.freezer.addModule(moduleName)

                for suffix in self.packager.dcClientSuffixes:
                    if suffix in moduleSuffixes:
                        self.freezer.addModule(moduleName + suffix)

                for i in range(dcFile.getNumImportSymbols(n)):
                    symbolName = dcFile.getImportSymbol(n, i)
                    symbolSuffixes = []
                    if '/' in symbolName:
                        symbolName, suffixes = symbolName.split('/', 1)
                        symbolSuffixes = suffixes.split('/')

                    # "from moduleName import symbolName".

                    # Maybe this symbol is itself a module; if that's
                    # the case, we need to add it to the list also.
                    self.freezer.addModule('%s.%s' % (moduleName, symbolName),
                                           implicit = True)
                    for suffix in self.packager.dcClientSuffixes:
                        if suffix in symbolSuffixes:
                            self.freezer.addModule('%s.%s%s' % (moduleName, symbolName, suffix),
                                                   implicit = True)


        def addPrcFile(self, file):
            """ Adds a prc file to the archive.  Like the dc file,
            this strips comments and such before adding.  It's also
            possible to set prcEncryptionKey and/or prcSignCommand to
            further manipulate prc files during processing. """

            # First, read it in.
            if file.text:
                textLines = file.text.split('\n')
            else:
                textLines = open(file.filename.toOsSpecific(), 'rU').readlines()

            # Then write it out again, without the comments.
            tempFilename = Filename.temporary('', 'p3d_', '.prc')
            tempFilename.setBinary()  # Binary is more reliable for signing.
            temp = open(tempFilename.toOsSpecific(), 'w')
            for line in textLines:
                line = line.strip()
                if line and line[0] != '#':
                    # Write the line out only if it's not a comment.
                    temp.write(line + '\n')
            temp.close()

            if self.packager.prcSignCommand:
                # Now sign the file.
                command = '%s -n "%s"' % (
                    self.packager.prcSignCommand, tempFilename.toOsSpecific())
                self.notify.info(command)
                exitStatus = os.system(command)
                if exitStatus != 0:
                    self.notify.error('Command failed: %s' % (command))

            if self.packager.prcEncryptionKey:
                # And now encrypt it.
                if file.newName.endswith('.prc'):
                    # Change .prc -> .pre
                    file.newName = file.newName[:-1] + 'e'

                preFilename = Filename.temporary('', 'p3d_', '.pre')
                preFilename.setBinary()
                tempFilename.setText()
                encryptFile(tempFilename, preFilename, self.packager.prcEncryptionKey)
                tempFilename.unlink()
                tempFilename = preFilename

            if file.deleteTemp:
                file.filename.unlink()

            file.filename = tempFilename
            file.text = None
            file.deleteTemp = True

            self.addComponent(file)

        def addComponent(self, file):
            compressionLevel = 0
            if file.compress:
                compressionLevel = self.compressionLevel

            if file.text:
                stream = StringStream(file.text)
                self.multifile.addSubfile(file.newName, stream, compressionLevel)
                self.multifile.flush()

            elif file.executable and self.arch:
                if not self.__addOsxExecutable(file):
                    return

            else:
                # Copy an ordinary file into the multifile.
                self.multifile.addSubfile(file.newName, file.filename, compressionLevel)
            if file.extract:
                if file.text:
                    # Better write it to a temporary file, so we can
                    # get its hash.
                    tfile = Filename.temporary('', 'p3d_')
                    open(tfile.toOsSpecific(), 'wb').write(file.text)
                    xextract = self.getFileSpec('extract', tfile, file.newName)
                    tfile.unlink()

                else:
                    # The file data exists on disk already.
                    xextract = self.getFileSpec('extract', file.filename, file.newName)
                self.extracts.append((file.newName.lower(), xextract))

            xcomponent = TiXmlElement('component')
            xcomponent.SetAttribute('filename', file.newName)
            self.components.append(('c', file.newName.lower(), xcomponent))

        def __addOsxExecutable(self, file):
            """ Adds an executable or shared library to the multifile,
            with respect to OSX's fat-binary features.  Returns true
            on success, false on failure. """

            compressionLevel = 0
            if file.compress:
                compressionLevel = self.compressionLevel

            # If we're on OSX and adding only files for a
            # particular architecture, use lipo to strip out the
            # part of the file for that architecture.

            arch = self.arch
            if arch == "amd64":
                arch = "x86_64"

            # First, we need to verify that it is in fact a
            # universal binary.
            tfile = Filename.temporary('', 'p3d_')
            tfile.setBinary()
            command = '/usr/bin/lipo -info "%s" >"%s"' % (
                file.filename.toOsSpecific(),
                tfile.toOsSpecific())
            exitStatus = os.system(command)
            if exitStatus != 0:
                self.notify.warning("Not an executable file: %s" % (file.filename))
                # Just add it anyway.
                file.filename.setBinary()
                self.multifile.addSubfile(file.newName, file.filename, compressionLevel)
                return True

            # The lipo command succeeded, so it really is an
            # executable file.  Parse the lipo output to figure out
            # which architectures the file supports.
            arches = []
            lipoData = open(tfile.toOsSpecific(), 'r').read()
            tfile.unlink()
            if ':' in lipoData:
                arches = lipoData.rsplit(':', 1)[1]
                arches = arches.split()

            if arches == [arch]:
                # The file only contains the one architecture that
                # we want anyway.
                file.filename.setBinary()
                self.multifile.addSubfile(file.newName, file.filename, compressionLevel)
                return True

            if arch not in arches:
                # The file doesn't support the architecture that we
                # want at all.  Omit the file.
                self.notify.warning("%s doesn't support architecture %s" % (
                    file.filename, self.arch))
                return False

            # The file contains multiple architectures.  Get
            # out just the one we want.
            command = '/usr/bin/lipo -thin %s -output "%s" "%s"' % (
                arch, tfile.toOsSpecific(),
                file.filename.toOsSpecific())
            exitStatus = os.system(command)
            if exitStatus != 0:
                self.notify.error('Command failed: %s' % (command))
            self.multifile.addSubfile(file.newName, tfile, compressionLevel)
            if file.deleteTemp:
                file.filename.unlink()
            file.filename = tfile
            file.deleteTemp = True
            return True


        def requirePackage(self, package):
            """ Indicates a dependency on the given package.  This
            also implicitly requires all of the package's requirements
            as well (though this transitive requirement happens at
            runtime, not here at build time). """

            if package not in self.requires:
                self.requires.append(package)
                for lowerName in package.targetFilenames:
                    ext = Filename(lowerName).getExtension()
                    if ext not in self.packager.nonuniqueExtensions:
                        self.skipFilenames[lowerName] = True

                for moduleName, mdef in package.moduleNames.items():
                    if not mdef.exclude:
                        self.skipModules[moduleName] = mdef

    # Packager constructor
    def __init__(self, platform = None):

        # The following are config settings that the caller may adjust
        # before calling any of the command methods.

        # The platform string.
        self.setPlatform(platform)

        # This should be set to a Filename.
        self.installDir = None

        # If specified, this is a directory to search first for any
        # library references, before searching the system.
        # Particularly useful on OSX to reference the universal SDK.
        self.systemRoot = None

        # Set this true to treat setHost() the same as addHost(), thus
        # ignoring any request to specify a particular download host,
        # e.g. for testing and development.
        self.ignoreSetHost = False

        # Set this to true to verbosely log files ignored by dir().
        self.verbosePrint = False

        # This will be appended to the basename of any .p3d package,
        # before the .p3d extension.
        self.p3dSuffix = ''

        # The download URL at which these packages will eventually be
        # hosted.
        self.hosts = {}
        self.host = PandaSystem.getPackageHostUrl()
        self.addHost(self.host)

        # This will be used when we're not compiling in the packaged
        # environment.
        self.__hostInfos = {}
        self.http = HTTPClient.getGlobalPtr()

        # The maximum amount of time a client should cache the
        # contents.xml before re-querying the server, in seconds.
        self.maxAge = 0

        # The contents seq: a tuple of integers, representing the
        # current seq value.  The contents seq generally increments
        # with each modification to the contents.xml file.  There is
        # also a package seq for each package, which generally
        # increments with each modification to the package.

        # The contents seq and package seq are used primarily for
        # documentation purposes, to note when a new version is
        # released.  The package seq value can also be used to verify
        # that the contents.xml, desc.xml, and desc.import.xml files
        # were all built at the same time.

        # Although the package seqs are used at runtime to verify that
        # the latest contents.xml file has been downloaded, they are
        # not otherwise used at runtime, and they are not binding on
        # the download version.  The md5 hash, not the package seq, is
        # actually used to differentiate different download versions.
        self.contentsSeq = SeqValue()

        # A search list for previously-built local packages.

        # We use a bit of caution to read the Filenames out of the
        # config variable.  Since cvar.getDirectories() returns a list
        # of references to Filename objects stored within the config
        # variable itself, we have to make a copy of each Filename
        # returned, so they will persist beyond the lifespan of the
        # config variable.
        cvar = ConfigVariableSearchPath('pdef-path')
        self.installSearch = list(map(Filename, cvar.getDirectories()))

        # This is where we cache the location of libraries.
        self.libraryCache = {}

        # The system PATH, for searching dll's and exe's.
        self.executablePath = DSearchPath()

        # By convention, we include sys.path at the front of
        # self.executablePath, mainly to aid makepanda when building
        # an rtdist build.
        for dirname in sys.path:
            self.executablePath.appendDirectory(Filename.fromOsSpecific(dirname))

        # Now add the actual system search path.
        if self.platform.startswith('win'):
            self.addWindowsSearchPath(self.executablePath, "PATH")

        else:
            if self.platform.startswith('osx'):
                self.addPosixSearchPath(self.executablePath, "DYLD_LIBRARY_PATH")

            self.addPosixSearchPath(self.executablePath, "LD_LIBRARY_PATH")
            self.addPosixSearchPath(self.executablePath, "PATH")

            if self.platform.startswith('linux'):
                # It used to be okay to just add some common paths on Linux.
                # But nowadays, each distribution has their own convention for
                # where they put their libraries.  Instead, we query the ldconfig
                # cache, which contains the location of all libraries.

                if not self.loadLdconfigCache():
                    # Ugh, failure.  All that remains is to guess.  This should
                    # work for the most common Debian configurations.
                    multiarchDir = "/lib/%s-linux-gnu" % (os.uname()[4])
                    if os.path.isdir(multiarchDir):
                        self.executablePath.appendDirectory(multiarchDir)
                    if os.path.isdir("/usr/" + multiarchDir):
                        self.executablePath.appendDirectory("/usr/" + multiarchDir)

            else:
                # FreeBSD, or some other system that still makes sense.
                self.executablePath.appendDirectory('/lib')
                self.executablePath.appendDirectory('/usr/lib')
                self.executablePath.appendDirectory('/usr/local/lib')

        if self.platform.startswith('freebsd') and os.uname()[1] == "pcbsd":
            self.executablePath.appendDirectory('/usr/PCBSD/local/lib')

        # Set this flag true to automatically add allow_python_dev to
        # any applications.
        self.allowPythonDev = False

        # Set this flag to store the original Python source files,
        # without compiling them to .pyc or .pyo.
        self.storePythonSource = False

        # Fill this with a list of (certificate, chain, pkey,
        # password) tuples to automatically sign each p3d file
        # generated.
        self.signParams = []

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
        # can't really be patched.  But it's here if you really want
        # it. ** Note: Actually, this isn't implemented yet.
        #self.encryptExtensions = []
        #self.encryptFiles = []

        # This is the list of DC import suffixes that should be
        # available to the client.  Other suffixes, like AI and UD,
        # are server-side only and should be ignored by the Scrubber.
        self.dcClientSuffixes = ['OV']

        # Is this file system case-sensitive?
        self.caseSensitive = True
        if self.platform.startswith('win'):
            self.caseSensitive = False
        elif self.platform.startswith('osx'):
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
        # with end-of-line conversion.
        self.textExtensions = [ 'prc', 'ptf', 'txt', 'cg', 'sha', 'dc', 'xml' ]

        # Binary files that are copied (and compressed) without
        # processing.
        self.binaryExtensions = [ 'ttf', 'TTF', 'mid', 'ico', 'cur' ]

        # Files that can have an existence in multiple different
        # packages simultaneously without conflict.
        self.nonuniqueExtensions = [ 'prc' ]

        # Files that represent an executable or shared library.
        if self.platform.startswith('win'):
            self.executableExtensions = [ 'dll', 'pyd', 'exe' ]
        elif self.platform.startswith('osx'):
            self.executableExtensions = [ 'so', 'dylib' ]
        else:
            self.executableExtensions = [ 'so' ]

        # Files that represent a Windows "manifest" file.  These files
        # must be explicitly extracted to disk so the OS can find
        # them.
        if self.platform.startswith('win'):
            self.manifestExtensions = [ 'manifest' ]
        else:
            self.manifestExtensions = [ ]

        # Extensions that are automatically remapped by convention.
        self.remapExtensions = {}
        if self.platform.startswith('win'):
            pass
        elif self.platform.startswith('osx'):
            self.remapExtensions = {
                'dll' : 'dylib',
                'pyd' : 'so',
                'exe' : ''
                }
        else:
            self.remapExtensions = {
                'dll' : 'so',
                'pyd' : 'so',
                'exe' : ''
                }

        # Files that should be extracted to disk.
        self.extractExtensions = self.executableExtensions[:] + self.manifestExtensions[:] + [ 'ico', 'cur' ]

        # Files that indicate a platform dependency.
        self.platformSpecificExtensions = self.executableExtensions[:]

        # Binary files that are considered uncompressible, and are
        # copied without compression.
        self.uncompressibleExtensions = [ 'mp3', 'ogg', 'ogv', 'wav', 'rml', 'rcss', 'otf' ]
        # wav files are compressible, but p3openal_audio won't load
        # them compressed.
        # rml, rcss and otf files must be added here because
        # libRocket wants to be able to seek in these files.

        # Files which are not to be processed further, but which
        # should be added exactly byte-for-byte as they are.
        self.unprocessedExtensions = []

        # Files for which warnings should be suppressed when they are
        # not handled by dir()
        self.suppressWarningForExtensions = ['', 'pyc', 'pyo',
                                             'p3d', 'pdef',
                                             'c', 'C', 'cxx', 'cpp', 'h', 'H',
                                             'hpp', 'pp', 'I', 'pem', 'p12', 'crt',
                                             'o', 'obj', 'a', 'lib', 'bc', 'll']

        # System files that should never be packaged.  For
        # case-insensitive filesystems (like Windows and OSX), put the
        # lowercase filename here.  Case-sensitive filesystems should
        # use the correct case.
        self.excludeSystemFiles = [
            'kernel32.dll', 'user32.dll', 'wsock32.dll', 'ws2_32.dll',
            'advapi32.dll', 'opengl32.dll', 'glu32.dll', 'gdi32.dll',
            'shell32.dll', 'ntdll.dll', 'ws2help.dll', 'rpcrt4.dll',
            'imm32.dll', 'ddraw.dll', 'shlwapi.dll', 'secur32.dll',
            'dciman32.dll', 'comdlg32.dll', 'comctl32.dll', 'ole32.dll',
            'oleaut32.dll', 'gdiplus.dll', 'winmm.dll', 'iphlpapi.dll',
            'msvcrt.dll', 'kernelbase.dll', 'msimg32.dll', 'msacm32.dll',

            'libsystem.b.dylib', 'libmathcommon.a.dylib', 'libmx.a.dylib',
            'libstdc++.6.dylib', 'libobjc.a.dylib', 'libauto.dylib',
            ]

        # As above, but with filename globbing to catch a range of
        # filenames.
        self.excludeSystemGlobs = [
            GlobPattern('d3dx9_*.dll'),
            GlobPattern('api-ms-win-*.dll'),

            GlobPattern('libGL.so*'),
            GlobPattern('libGLU.so*'),
            GlobPattern('libGLcore.so*'),
            GlobPattern('libGLES*.so*'),
            GlobPattern('libEGL.so*'),
            GlobPattern('libX11.so*'),
            GlobPattern('libXau.so*'),
            GlobPattern('libXdmcp.so*'),
            GlobPattern('libxcb*.so*'),
            GlobPattern('libc.so*'),
            GlobPattern('libgcc_s.so*'),
            GlobPattern('libdl.so*'),
            GlobPattern('libm.so*'),
            GlobPattern('libnvidia*.so*'),
            GlobPattern('libpthread.so*'),
            GlobPattern('libthr.so*'),
            GlobPattern('ld-linux.so*'),
            GlobPattern('ld-linux-*.so*'),
            GlobPattern('librt.so*'),
            ]

        # A Loader for loading models.
        self.loader = Loader.Loader(self)
        self.sfxManagerList = None
        self.musicManager = None

        # These options will be used when loading models and textures.  By
        # default we don't load textures beyond the header and don't store
        # models in the RAM cache in order to conserve on memory usage.
        opts = LoaderOptions()
        opts.setFlags(opts.getFlags() | LoaderOptions.LFNoRamCache)
        opts.setTextureFlags(opts.getTextureFlags() & ~LoaderOptions.TFPreload)
        self.loaderOptions = opts

        # This is filled in during readPackageDef().
        self.packageList = []

        # A table of all known packages by name.
        self.packages = {}

        # A list of PackageEntry objects read from the contents.xml
        # file.
        self.contents = {}

    def loadLdconfigCache(self):
        """ On GNU/Linux, runs ldconfig -p to find out where all the
        libraries on the system are located.  Assumes that the platform
        has already been set. """

        if not os.path.isfile('/sbin/ldconfig'):
            return False

        handle = subprocess.Popen(['/sbin/ldconfig', '-p'], stdout=subprocess.PIPE, universal_newlines=True)
        out, err = handle.communicate()

        if handle.returncode != 0:
            self.notify.warning("/sbin/ldconfig -p returned code %d" %(handle.returncode))
            return False

        for line in out.splitlines():
            if '=>' not in line:
                continue

            prefix, location = line.rsplit('=>', 1)
            prefix = prefix.strip()
            location = location.strip()

            if not location or not prefix or ' ' not in prefix:
                self.notify.warning("Ignoring malformed ldconfig -p line: " + line)
                continue

            lib, opts = prefix.split(' ', 1)
            if ('x86-64' in opts) != self.platform.endswith('_amd64'):
                # This entry isn't meant for our architecture.  I think
                # x86-64 is the only platform where ldconfig supplies
                # this extra arch string.
                continue

            self.libraryCache[lib] = Filename.fromOsSpecific(location)

        return True

    def resolveLibrary(self, filename):
        """ Resolves the given shared library filename along the executable path,
        or by cross-referencing it with the library cache. """

        path = str(filename)

        if path in self.libraryCache:
            filename.setFullpath(self.libraryCache[path].getFullpath())
            return True

        if filename.resolveFilename(self.executablePath):
            self.libraryCache[path] = Filename(filename)
            return True

        return False

    def setPlatform(self, platform = None):
        """ Sets the platform that this Packager will compute for.  On
        OSX, this can be used to specify the particular architecture
        we are building; on other platforms, it is probably a mistake
        to set this.

        You should call this before doing anything else with the
        Packager.  It's even better to pass the platform string to the
        constructor.  """

        self.platform = platform or PandaSystem.getPlatform()

        # OSX uses this "arch" string for the otool and lipo commands.
        self.arch = None
        if self.platform.startswith('osx_'):
            self.arch = self.platform[4:]


    def setHost(self, host, downloadUrl = None,
                descriptiveName = None, hostDir = None,
                mirrors = None):
        """ Specifies the URL that will ultimately host these
        contents. """

        if not self.ignoreSetHost:
            self.host = host

        self.addHost(host, downloadUrl = downloadUrl,
                     descriptiveName = descriptiveName, hostDir = hostDir,
                     mirrors = mirrors)

    def addHost(self, host, downloadUrl = None, descriptiveName = None,
                hostDir = None, mirrors = None):
        """ Adds a host to the list of known download hosts.  This
        information will be written into any p3d files that reference
        this host; this can be used to pre-define the possible mirrors
        for a given host, for instance.  Returns the newly-created
        HostEntry object."""

        scheme = URLSpec(host).getScheme()
        if scheme == 'https' and downloadUrl is None:
            # If we specified an SSL-protected host URL, but no
            # explicit download URL, then assume the download URL is
            # the same, over cleartext.
            url = URLSpec(host)
            url.setScheme('http')
            downloadUrl = url.getUrl()

        he = self.hosts.get(host, None)
        if he is None:
            # Define a new host entry
            he = self.HostEntry(host, downloadUrl = downloadUrl,
                                descriptiveName = descriptiveName,
                                hostDir = hostDir, mirrors = mirrors)
            self.hosts[host] = he
        else:
            # Update an existing host entry
            if downloadUrl is not None:
                he.downloadUrl = downloadUrl
            if descriptiveName is not None:
                he.descriptiveName = descriptiveName
            if hostDir is not None:
                he.hostDir = hostDir
            if mirrors is not None:
                he.mirrors = mirrors

        return he

    def addAltHost(self, keyword, altHost, origHost = None,
                   downloadUrl = None, descriptiveName = None,
                   hostDir = None, mirrors = None):
        """ Adds an alternate host to any already-known host.  This
        defines an alternate server that may be contacted, if
        specified on the HTML page, which hosts a different version of
        the server's contents.  (This is different from a mirror,
        which hosts an identical version of the server's contents.)
        """

        if not origHost:
            origHost = self.host

        self.addHost(altHost, downloadUrl = downloadUrl,
                     descriptiveName = descriptiveName, hostDir = hostDir,
                     mirrors = mirrors)
        he = self.addHost(origHost)
        he.altHosts[keyword] = altHost

    def addWindowsSearchPath(self, searchPath, varname):
        """ Expands $varname, interpreting as a Windows-style search
        path, and adds its contents to the indicated DSearchPath. """

        path = ExecutionEnvironment.getEnvironmentVariable(varname)
        if len(path) == 0:
            if varname not in os.environ:
                return
            path = os.environ[varname]
        for dirname in path.split(';'):
            dirname = Filename.fromOsSpecific(dirname)
            if dirname.makeTrueCase():
                searchPath.appendDirectory(dirname)

    def addPosixSearchPath(self, searchPath, varname):
        """ Expands $varname, interpreting as a Posix-style search
        path, and adds its contents to the indicated DSearchPath. """

        path = ExecutionEnvironment.getEnvironmentVariable(varname)
        if len(path) == 0:
            if varname not in os.environ:
                return
            path = os.environ[varname]
        for dirname in path.split(':'):
            dirname = Filename.fromOsSpecific(dirname)
            if dirname.makeTrueCase():
                searchPath.appendDirectory(dirname)

    def _ensureExtensions(self):
        self.knownExtensions = \
            self.imageExtensions + \
            self.modelExtensions + \
            self.textExtensions + \
            self.binaryExtensions + \
            self.uncompressibleExtensions + \
            self.unprocessedExtensions

    def setup(self):
        """ Call this method to initialize the class after filling in
        some of the values in the constructor. """

        self._ensureExtensions()

        self.currentPackage = None

        if self.installDir:
            # If we were given an install directory, we can build
            # packages as well as plain p3d files, and it all goes
            # into the specified directory.
            self.p3dInstallDir = self.installDir
            self.allowPackages = True
        else:
            # If we don't have an actual install directory, we can
            # only build p3d files, and we drop them into the current
            # directory.
            self.p3dInstallDir = '.'
            self.allowPackages = False

        if not PandaSystem.getPackageVersionString() or not PandaSystem.getPackageHostUrl():
            raise PackagerError('This script must be run using a version of Panda3D that has been built\nfor distribution.  Try using ppackage.p3d or packp3d.p3d instead.\nIf you are running this script for development purposes, you may also\nset the Config variable panda-package-host-url to the URL you expect\nto download these contents from (for instance, a file:// URL).')

        self.readContentsFile()

    def close(self):
        """ Called after reading all of the package def files, this
        performs any final cleanup appropriate. """

        self.writeContentsFile()

    def buildPatches(self, packages):
        """ Call this after calling close(), to build patches for the
        indicated packages. """

        # We quietly ignore any p3d applications or solo packages
        # passed in the packages list; we only build patches for
        # actual Multifile-based packages.
        packageNames = []
        for package in packages:
            if not package.p3dApplication and not package.solo:
                packageNames.append(package.packageName)

        if packageNames:
            from .PatchMaker import PatchMaker
            pm = PatchMaker(self.installDir)
            pm.buildPatches(packageNames = packageNames)

    def readPackageDef(self, packageDef, packageNames = None):
        """ Reads the named .pdef file and constructs the named
        packages, or all packages if packageNames is None.  Raises an
        exception if the pdef file is invalid.  Returns the list of
        packages constructed. """

        self.notify.info('Reading %s' % (packageDef))

        # We use exec to "read" the .pdef file.  This has the nice
        # side-effect that the user can put arbitrary Python code in
        # there to control conditional execution, and such.

        # Set up the namespace dictionary for exec.
        globals = {}
        globals['__name__'] = packageDef.getBasenameWoExtension()
        globals['__dir__'] = Filename(packageDef.getDirname()).toOsSpecific()
        globals['__file__'] = packageDef.toOsSpecific()
        globals['packageDef'] = packageDef

        globals['platform'] = self.platform
        globals['packager'] = self

        # We'll stuff all of the predefined functions, and the
        # predefined classes, in the global dictionary, so the pdef
        # file can reference them.

        # By convention, the existence of a method of this class named
        # do_foo(self) is sufficient to define a pdef method call
        # foo().
        for methodName in list(self.__class__.__dict__.keys()):
            if methodName.startswith('do_'):
                name = methodName[3:]
                c = func_closure(name)
                globals[name] = c.generic_func

        globals['p3d'] = class_p3d
        globals['package'] = class_package
        globals['solo'] = class_solo

        # Now exec the pdef file.  Assuming there are no syntax
        # errors, and that the pdef file doesn't contain any really
        # crazy Python code, all this will do is fill in the
        # '__statements' list in the module scope.
        fn = packageDef.toOsSpecific()
        f = open(fn)
        code = compile(f.read(), fn, 'exec')
        f.close()

        # It appears that having a separate globals and locals
        # dictionary causes problems with resolving symbols within a
        # class scope.  So, we just use one dictionary, the globals.
        exec(code, globals)

        packages = []

        # Now iterate through the statements and operate on them.
        statements = globals.get('__statements', [])
        if not statements:
            self.notify.info("No packages defined.")

        try:
            for (lineno, stype, name, args, kw) in statements:
                if stype == 'class':
                    if packageNames is None or name in packageNames:
                        classDef = globals[name]
                        p3dApplication = (class_p3d in classDef.__bases__)
                        solo = (class_solo in classDef.__bases__)
                        self.beginPackage(name, p3dApplication = p3dApplication,
                                          solo = solo)
                        statements = classDef.__dict__.get('__statements', [])
                        if not statements:
                            self.notify.info("No files added to %s" % (name))
                        for (lineno, stype, sname, args, kw) in statements:
                            if stype == 'class':
                                raise PackagerError('Nested classes not allowed')
                            self.__evalFunc(sname, args, kw)
                        package = self.endPackage()
                        if package is not None:
                            packages.append(package)
                        elif packageNames is not None:
                            # If the name is explicitly specified, this means
                            # we should abort if the package faild to construct.
                            raise PackagerError('Failed to construct %s' % name)
                else:
                    self.__evalFunc(name, args, kw)
        except PackagerError:
            # Append the line number and file name to the exception
            # error message.
            inst = sys.exc_info()[1]
            if not inst.args:
                inst.args = ('Error',)

            inst.args = (inst.args[0] + ' on line %s of %s' % (lineno, packageDef),)
            raise

        return packages

    def __evalFunc(self, name, args, kw):
        """ This is called from readPackageDef(), above, to call the
        function do_name(*args, **kw), as extracted from the pdef
        file. """

        funcname = 'do_%s' % (name)
        func = getattr(self, funcname)
        try:
            func(*args, **kw)
        except OutsideOfPackageError:
            message = '%s encountered outside of package definition' % (name)
            raise OutsideOfPackageError(message)

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
                raise PackagerError(message)
            if parameter in args:
                message = 'Duplicate parameter %s' % (parameter)
                raise PackagerError(message)

            args[parameter] = value

            del words[-1]


    def beginPackage(self, packageName, p3dApplication = False,
                     solo = False):
        """ Begins a new package specification.  packageName is the
        basename of the package.  Follow this with a number of calls
        to file() etc., and close the package with endPackage(). """

        if self.currentPackage:
            raise PackagerError('unclosed endPackage %s' % (self.currentPackage.packageName))

        package = self.Package(packageName, self)
        self.currentPackage = package

        package.p3dApplication = p3dApplication
        package.solo = solo

        if not package.p3dApplication and not self.allowPackages:
            message = 'Cannot generate packages without an installDir; use -i'
            raise PackagerError(message)


    def endPackage(self):
        """ Closes the current package specification.  This actually
        generates the package file.  Returns the finished package,
        or None if the package failed to close (e.g. missing files). """

        if not self.currentPackage:
            raise PackagerError('unmatched endPackage')

        package = self.currentPackage
        package.signParams += self.signParams[:]

        self.currentPackage = None
        if not package.close():
            return None

        self.packageList.append(package)
        self.packages[(package.packageName, package.platform, package.version)] = package
        self.currentPackage = None

        return package

    def findPackage(self, packageName, platform = None, version = None,
                    host = None, requires = None):
        """ Searches for the named package from a previous publish
        operation along the install search path.

        If requires is not None, it is a list of Package objects that
        are already required.  The new Package object must be
        compatible with the existing Packages, or an error is
        returned.  This is also useful for determining the appropriate
        package version to choose when a version is not specified.

        Returns the Package object, or None if the package cannot be
        located. """

        # Is it a package we already have resident?
        package = self.packages.get((packageName, platform or self.platform, version, host), None)
        if package:
            return package

        # Look on the searchlist.
        for dirname in self.installSearch:
            package = self.__scanPackageDir(dirname, packageName, platform or self.platform, version, host, requires = requires)
            if not package:
                package = self.__scanPackageDir(dirname, packageName, platform, version, host, requires = requires)

            if package and host and package.host != host:
                # Wrong host.
                package = None

            if package:
                break

        if not package:
            # Query the indicated host.
            package = self.__findPackageOnHost(packageName, platform or self.platform, version or None, host, requires = requires)
            if not package:
                package = self.__findPackageOnHost(packageName, platform, version, host, requires = requires)

        if package:
            package = self.packages.setdefault((package.packageName, package.platform, package.version, package.host), package)
            self.packages[(packageName, platform or self.platform, version, host)] = package
            return package

        return None

    def __scanPackageDir(self, rootDir, packageName, platform, version,
                         host, requires = None):
        """ Scans a directory on disk, looking for *.import.xml files
        that match the indicated packageName and optional version.  If a
        suitable xml file is found, reads it and returns the assocated
        Package definition.

        If a version is not specified, and multiple versions are
        available, the highest-numbered version that matches will be
        selected.
        """

        packages = []

        if version:
            # A specific version package.
            versionList = [version]
        else:
            # An unversioned package, or any old version.
            versionList = [None, '*']

        for version in versionList:
            packageDir = Filename(rootDir, packageName)
            basename = packageName

            if version:
                # A specific or nonspecific version package.
                packageDir = Filename(packageDir, version)
                basename += '.%s' % (version)

            if platform:
                packageDir = Filename(packageDir, platform)
                basename += '.%s' % (platform)

            # Actually, the host means little for this search, since we're
            # only looking in a local directory at this point.

            basename += '.import.xml'
            filename = Filename(packageDir, basename)
            filelist = glob.glob(filename.toOsSpecific())
            if not filelist:
                # It doesn't exist in the nested directory; try the root
                # directory.
                filename = Filename(rootDir, basename)
                filelist = glob.glob(filename.toOsSpecific())

            for file in filelist:
                package = self.__readPackageImportDescFile(Filename.fromOsSpecific(file))
                packages.append(package)

        self.__sortImportPackages(packages)
        for package in packages:
            if package and self.__packageIsValid(package, requires, platform):
                return package

        return None

    def __findPackageOnHost(self, packageName, platform, version, hostUrl, requires = None):
        appRunner = AppRunnerGlobal.appRunner

        # Make sure we have a fresh version of the contents file.
        host = self.__getHostInfo(hostUrl)
        if not host.downloadContentsFile(self.http):
            return None

        packageInfos = []
        packageInfo = host.getPackage(packageName, version, platform = platform)
        if not packageInfo and not version:
            # No explicit version is specified, first fallback: look
            # for the compiled-in version.
            packageInfo = host.getPackage(packageName, PandaSystem.getPackageVersionString(), platform = platform)

        if not packageInfo and not version:
            # No explicit version is specified, second fallback: get
            # the highest-numbered version available.
            packageInfos = host.getPackages(packageName, platform = platform)
            self.__sortPackageInfos(packageInfos)

        if packageInfo and not packageInfos:
            packageInfos = [packageInfo]

        for packageInfo in packageInfos:
            if not packageInfo or not packageInfo.importDescFile:
                continue

            # Now we've retrieved a PackageInfo.  Get the import desc file
            # from it.
            if host.hostDir:
                filename = Filename(host.hostDir, 'imports/' + packageInfo.importDescFile.basename)
            else:
                # We're not running in the packaged environment, so download
                # to a temporary file instead of the host directory.
                filename = Filename.temporary('', 'import_' + packageInfo.importDescFile.basename, '.xml')

            if not host.freshenFile(self.http, packageInfo.importDescFile, filename):
                self.notify.error("Couldn't download import file.")
                continue

            # Now that we have the import desc file, use it to load one of
            # our Package objects.
            package = self.Package('', self)
            success = package.readImportDescFile(filename)

            if not host.hostDir:
                # Don't forget to delete the temporary file we created.
                filename.unlink()

            if success and self.__packageIsValid(package, requires, platform):
                return package

        # Couldn't find a suitable package.
        return None

    def __getHostInfo(self, hostUrl = None):
        """ This shadows appRunner.getHost(), for the purpose of running
        outside the packaged environment. """

        if not hostUrl:
            hostUrl = PandaSystem.getPackageHostUrl()

        if AppRunnerGlobal.appRunner:
            return AppRunnerGlobal.appRunner.getHost(hostUrl)

        host = self.__hostInfos.get(hostUrl, None)
        if not host:
            host = HostInfo(hostUrl)
            self.__hostInfos[hostUrl] = host
        return host

    def __sortImportPackages(self, packages):
        """ Given a list of Packages read from *.import.xml filenames,
        sorts them in reverse order by version, so that the
        highest-numbered versions appear first in the list. """

        tuples = []
        for package in packages:
            version = self.__makeVersionTuple(package.version)
            tuples.append((version, package))
        tuples.sort(reverse = True)

        return [t[1] for t in tuples]

    def __sortPackageInfos(self, packages):
        """ Given a list of PackageInfos retrieved from a Host, sorts
        them in reverse order by version, so that the highest-numbered
        versions appear first in the list. """

        tuples = []
        for package in packages:
            version = self.__makeVersionTuple(package.packageVersion)
            tuples.append((version, package))
        tuples.sort(reverse = True)

        return [t[1] for t in tuples]

    def __makeVersionTuple(self, version):
        """ Converts a version string into a tuple for sorting, by
        separating out numbers into separate numeric fields, so that
        version numbers sort numerically where appropriate. """

        if not version:
            return ('',)

        words = []
        p = 0
        while p < len(version):
            # Scan to the first digit.
            w = ''
            while p < len(version) and not version[p].isdigit():
                w += version[p]
                p += 1
            words.append(w)

            # Scan to the end of the string of digits.
            w = ''
            while p < len(version) and version[p].isdigit():
                w += version[p]
                p += 1
            if w:
                words.append(int(w))

        return tuple(words)

    def __packageIsValid(self, package, requires, platform):
        """ Returns true if the package is valid, meaning it can be
        imported without conflicts with existing packages already
        required (such as different versions of panda3d). """

        if package.platform and package.platform != platform:
            # Incorrect platform.
            return False

        if not requires:
            # No other restrictions.
            return True

        # Really, we only check the panda3d package.  The other
        # packages will list this as a dependency, and this is all
        # that matters.

        panda1 = self.__findPackageInRequires('panda3d', [package] + package.requires)
        panda2 = self.__findPackageInRequires('panda3d', requires)

        if not panda1 or not panda2:
            return True

        if panda1.version == panda2.version:
            return True

        print('Rejecting package %s, version "%s": depends on %s, version "%s" instead of version "%s"' % (
            package.packageName, package.version,
            panda1.packageName, panda1.version, panda2.version))
        return False

    def __findPackageInRequires(self, packageName, list):
        """ Returns the first package with the indicated name in the
        list of packages, or in the list of packages required by the
        packages in the list. """

        for package in list:
            if package.packageName == packageName:
                return package
            p2 = self.__findPackageInRequires(packageName, package.requires)
            if p2:
                return p2

        return None

    def __readPackageImportDescFile(self, filename):
        """ Reads the named xml file as a Package, and returns it if
        valid, or None otherwise. """

        package = self.Package('', self)
        if package.readImportDescFile(filename):
            return package

        return None

    def do_setVer(self, value):
        """ Sets an explicit set_ver number for the package, as a tuple
        of integers, or as a string of dot-separated integers. """

        self.currentPackage.packageSetVer = SeqValue(value)

    def do_config(self, **kw):
        """ Called with any number of keyword parameters.  For each
        keyword parameter, sets the corresponding p3d config variable
        to the given value.  This will be written into the
        p3d_info.xml file at the top of the application, or to the
        package desc file for a package file. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        for keyword, value in list(kw.items()):
            self.currentPackage.configs[keyword] = value

    def do_require(self, *args, **kw):
        """ Indicates a dependency on the named package(s), supplied
        as a name.

        Attempts to install this package will implicitly install the
        named package also.  Files already included in the named
        package will be omitted from this one when building it. """

        self.requirePackagesNamed(args, **kw)

    def requirePackagesNamed(self, names, version = None, host = None):
        """ Indicates a dependency on the named package(s), supplied
        as a name.

        Attempts to install this package will implicitly install the
        named package also.  Files already included in the named
        package will be omitted from this one when building it. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        for packageName in names:
            # A special case when requiring the "panda3d" package.  We
            # supply the version number which we've been compiled with
            # as a default.
            pversion = version
            phost = host
            if packageName == 'panda3d':
                if not pversion:
                    pversion = PandaSystem.getPackageVersionString()
                if not phost:
                    phost = PandaSystem.getPackageHostUrl()

            package = self.findPackage(packageName, version = pversion, host = phost,
                                       requires = self.currentPackage.requires)
            if not package:
                message = 'Unknown package %s, version "%s"' % (packageName, version)
                self.notify.warning(message)
                self.currentPackage.missingPackages.append((packageName, pversion))
                continue

            self.requirePackage(package)

    def requirePackage(self, package):
        """ Indicates a dependency on the indicated package, supplied
        as a Package object.

        Attempts to install this package will implicitly install the
        named package also.  Files already included in the named
        package will be omitted from this one. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        # A special case when requiring the "panda3d" package.  We
        # complain if the version number doesn't match what we've been
        # compiled with.
        if package.packageName == 'panda3d':
            if package.version != PandaSystem.getPackageVersionString():
                self.notify.warning("Requiring panda3d version %s, which does not match the current build of Panda, which is version %s." % (package.version, PandaSystem.getPackageVersionString()))
            elif package.host != PandaSystem.getPackageHostUrl():
                self.notify.warning("Requiring panda3d host %s, which does not match the current build of Panda, which is host %s." % (package.host, PandaSystem.getPackageHostUrl()))

        self.currentPackage.requirePackage(package)

    def do_module(self, *args, **kw):
        """ Adds the indicated Python module(s) to the current package. """
        self.addModule(args, **kw)

    def addModule(self, moduleNames, newName = None, filename = None, required = False):
        if not self.currentPackage:
            raise OutsideOfPackageError

        if (newName or filename) and len(moduleNames) != 1:
            raise PackagerError('Cannot specify newName with multiple modules')

        if required:
            self.currentPackage.requiredModules += moduleNames

        for moduleName in moduleNames:
            self.currentPackage.freezer.addModule(moduleName, newName = newName, filename = filename)

    def do_excludeModule(self, *args):
        """ Marks the indicated Python module as not to be included. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        for moduleName in args:
            self.currentPackage.freezer.excludeModule(moduleName)

    def do_main(self, filename):
        """ Includes the indicated file as __main__ module of the application.
        Also updates mainModule to point to this module. """

        self.addModule(['__main__'], '__main__', filename, required = True)
        self.currentPackage.mainModule = ('__main__', '__main__')

    def do_mainModule(self, moduleName, newName = None, filename = None):
        """ Names the indicated module as the "main" module of the
        application or exe.  In most cases, you will want to use main()
        instead. """

        if not self.currentPackage:
            raise OutsideOfPackageError

        if self.currentPackage.mainModule and self.currentPackage.mainModule[0] != moduleName:
            self.notify.warning("Replacing mainModule %s with %s" % (
                self.currentPackage.mainModule[0], moduleName))

        if not newName:
            newName = moduleName

        if filename:
            filename = Filename(filename)
            newFilename = Filename('/'.join(moduleName.split('.')))
            newFilename.setExtension(filename.getExtension())
            self.currentPackage.addFile(
                filename, newName = str(newFilename),
                explicit = True, extract = True, required = True)

        self.currentPackage.mainModule = (moduleName, newName)

    def do_sign(self, certificate, chain = None, pkey = None, password = None):
        """ Signs the resulting p3d file (or package multifile) with
        the indicated certificate.  If needed, the chain file should
        contain the list of additional certificate authorities needed
        to validate the signing certificate.  The pkey file should
        contain the private key.

        It is also legal for the certificate file to contain the chain
        and private key embedded within it.

        If the private key is encrypted, the password should be
        supplied. """

        self.currentPackage.signParams.append((certificate, chain, pkey, password))

    def do_setupPanda3D(self, p3dpythonName=None, p3dpythonwName=None):
        """ A special convenience command that adds the minimum
        startup modules for a panda3d package, intended for developers
        producing their own custom panda3d for download.  Should be
        called before any other Python modules are named. """

        # This module and all its dependencies come frozen into p3dpython.
        # We should mark them as having already been added so that we don't
        # add them again to the Multifile.
        self.do_module('direct.showbase.VFSImporter')
        self.currentPackage.freezer.done(addStartupModules=True)
        self.currentPackage.freezer.writeCode(None)
        self.currentPackage.addExtensionModules()
        self.currentPackage.freezer.reset()

        self.do_file('panda3d/core.pyd', newDir='panda3d')

        # This is the key Python module that is imported at runtime to
        # start an application running.
        self.do_module('direct.p3d.AppRunner')

        # This is the main program that drives the runtime Python.  It
        # is responsible for importing direct.p3d.AppRunner to start an
        # application running.  The program comes in two parts: an
        # executable, and an associated dynamic library.  Note that the
        # .exe and .dll extensions are automatically replaced with the
        # appropriate platform-specific extensions.

        if self.platform.startswith('osx'):
            # On Mac, we package up a P3DPython.app bundle.  This
            # includes specifications in the plist file to avoid
            # creating a dock icon and stuff.

            resources = []

            # Find p3dpython.plist in the direct source tree.
            import direct
            plist = Filename(direct.__path__[0], 'plugin/p3dpython.plist')

##             # Find panda3d.icns in the models tree.
##             filename = Filename('plugin_images/panda3d.icns')
##             found = filename.resolveFilename(getModelPath().getValue())
##             if not found:
##                 found = filename.resolveFilename("models")
##             if found:
##                 resources.append(filename)

            self.do_makeBundle('P3DPython.app', plist, executable = 'p3dpython',
                               resources = resources, dependencyDir = '')

        else:
            # Anywhere else, we just ship the executable file p3dpython.exe.
            if p3dpythonName is None:
                p3dpythonName = 'p3dpython'
            else:
                self.do_config(p3dpython_name=p3dpythonName)

            if self.platform.startswith('win'):
                self.do_file('p3dpython.exe', newName=p3dpythonName+'.exe')
            else:
                self.do_file('p3dpython.exe', newName=p3dpythonName)

            # The "Windows" executable appends a 'w' to whatever name is used
            # above, unless an override name is explicitly specified.
            if self.platform.startswith('win'):
                if p3dpythonwName is None:
                    p3dpythonwName = p3dpythonName+'w'
                else:
                    self.do_config(p3dpythonw_name=p3dpythonwName)

                if self.platform.startswith('win'):
                    self.do_file('p3dpythonw.exe', newName=p3dpythonwName+'.exe')
                else:
                    self.do_file('p3dpythonw.exe', newName=p3dpythonwName)

        self.do_file('libp3dpython.dll')

    def do_freeze(self, filename, compileToExe = False):
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
            raise PackagerError(message)

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
        freezer.done(addStartupModules = compileToExe)

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
            package.platform = self.platform

        # Reset the freezer for more Python files.
        freezer.reset()
        package.mainModule = None

    def do_makeBundle(self, bundleName, plist, executable = None,
                      resources = None, dependencyDir = None):
        """ Constructs a minimal OSX "bundle" consisting of an
        executable and a plist file, with optional resource files
        (such as icons), and adds it to the package under the given
        name. """

        contents = bundleName + '/Contents'

        self.addFiles([plist], newName = contents + '/Info.plist',
                      extract = True)
        if executable:
            basename = Filename(executable).getBasename()
            self.addFiles([executable], newName = contents + '/MacOS/' + basename,
                          extract = True, executable = True, dependencyDir = dependencyDir)
        if resources:
            self.addFiles(resources, newDir = contents + '/Resources',
                          extract = True, dependencyDir = dependencyDir)



    def do_file(self, *args, **kw):
        """ Adds the indicated file or files to the current package.
        See addFiles(). """

        self.addFiles(args, **kw)

    def addFiles(self, filenames, text = None, newName = None,
                 newDir = None, extract = None, executable = None,
                 deleteTemp = False, literal = False,
                 dependencyDir = None, required = False):

        """ Adds the indicated arbitrary files to the current package.

        filenames is a list of Filename or string objects, and each
        may include shell globbing characters.

        Each file is placed in the named directory, or the toplevel
        directory if no directory is specified.

        Certain special behavior is invoked based on the filename
        extension.  For instance, .py files may be automatically
        compiled and stored as Python modules.

        If newDir is not None, it specifies the directory in which the
        file should be placed.  In this case, all files matched by the
        filename expression are placed in the named directory.

        If newName is not None, it specifies a new filename.  In this
        case, newDir is ignored, and the filename expression must
        match only one file.

        If newName and newDir are both None, the file is placed in the
        toplevel directory, regardless of its source directory.

        If text is nonempty, it contains the text of the file.  In
        this case, the filename is not read, but the supplied text is
        used instead.

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

        If required is true, then the file is marked a vital part of
        the package.  The package will not be built if this file
        somehow cannot be added to the package.

        """

        if not self.currentPackage:
            raise OutsideOfPackageError

        files = []
        explicit = True

        for filename in filenames:
            filename = Filename(filename)

            if literal:
                thisFiles = [filename.toOsSpecific()]

            else:
                ext = filename.getExtension()

                # A special case, since OSX and Linux don't have a
                # standard extension for program files.
                if executable is None and ext == 'exe':
                    executable = True

                newExt = self.remapExtensions.get(ext, None)
                if newExt is not None:
                    filename.setExtension(newExt)

                thisFiles = glob.glob(filename.toOsSpecific())
                if not thisFiles:
                    thisFiles = [filename.toOsSpecific()]

                if newExt == 'dll' or (ext == 'dll' and newExt is None):
                    # Go through the dsoFilename interface on Windows,
                    # to insert a _d if we are running on a debug
                    # build.
                    dllFilename = Filename(filename)
                    dllFilename.setExtension('so')
                    dllFilename = Filename.dsoFilename(str(dllFilename))
                    if dllFilename != filename:
                        thisFiles = glob.glob(filename.toOsSpecific())
                        if not thisFiles:
                            # We have to resolve this filename to
                            # determine if it's a _d or not.
                            if self.resolveLibrary(dllFilename):
                                thisFiles = [dllFilename.toOsSpecific()]
                            else:
                                thisFiles = [filename.toOsSpecific()]

            if len(thisFiles) > 1:
                explicit = False
            files += thisFiles

        prefix = ''
        if newDir is not None:
            prefix = str(Filename(newDir))
            if prefix and prefix[-1] != '/':
                prefix += '/'

        if newName:
            if len(files) != 1:
                message = 'Cannot install multiple files on target filename %s' % (newName)
                raise PackagerError(message)

        if text:
            if len(files) != 1:
                message = 'Cannot install text to multiple files'
                raise PackagerError(message)
            if not newName:
                newName = str(filenames[0])

        for filename in files:
            filename = Filename.fromOsSpecific(filename)
            basename = filename.getBasename()
            name = newName
            if not name:
                name = prefix + basename

            self.currentPackage.addFile(
                filename, newName = name, extract = extract,
                explicit = explicit, executable = executable,
                text = text, deleteTemp = deleteTemp,
                dependencyDir = dependencyDir, required = required)

    def do_exclude(self, filename):
        """ Marks the indicated filename as not to be included.  The
        filename may include shell globbing characters, and may or may
        not include a dirname.  (If it does not include a dirname, it
        refers to any file with the given basename from any
        directory.)"""

        if not self.currentPackage:
            raise OutsideOfPackageError

        filename = Filename(filename)
        self.currentPackage.excludeFile(filename)


    def do_includeExtensions(self, executableExtensions = None, extractExtensions = None,
                         imageExtensions = None, textExtensions = None,
                         uncompressibleExtensions = None, unprocessedExtensions = None,
                         suppressWarningForExtensions = None):
        """ Ensure that dir() will include files with the given extensions.
        The extensions should not have '.' prefixes.

        All except 'suppressWarningForExtensions' allow the given kinds of files
        to be packaged with their respective semantics (read the source).

        'suppressWarningForExtensions' lists extensions *expected* to be ignored,
        so no warnings will be emitted for them.
        """
        if executableExtensions:
            self.executableExtensions += executableExtensions

        if extractExtensions:
            self.extractExtensions += extractExtensions

        if imageExtensions:
            self.imageExtensions += imageExtensions

        if textExtensions:
            self.textExtensions += textExtensions

        if uncompressibleExtensions:
            self.uncompressibleExtensions += uncompressibleExtensions

        if unprocessedExtensions:
            self.unprocessedExtensions += unprocessedExtensions

        if suppressWarningForExtensions:
            self.suppressWarningForExtensions += suppressWarningForExtensions

        self._ensureExtensions()

    def do_dir(self, dirname, newDir = None, unprocessed = None):

        """ Adds the indicated directory hierarchy to the current
        package.  The directory hierarchy is walked recursively, and
        all files that match a known extension are added to the package.

        newDir specifies the directory name within the package which
        the contents of the named directory should be installed to.
        If it is omitted, the contents of the named directory are
        installed to the root of the package.

        If unprocessed is false (the default), bam files are loaded and
        scanned for textures, and these texture paths within the bam
        files are manipulated to point to the new paths within the
        package.  If unprocessed is true, this operation is bypassed,
        and bam files are packed exactly as they are.
        """

        if not self.currentPackage:
            raise OutsideOfPackageError

        dirname = Filename(dirname)
        if not newDir:
            newDir = ''

        # Adding the directory to sys.path is a cheesy way to help the
        # modulefinder find it.
        sys.path.append(dirname.toOsSpecific())
        self.__recurseDir(dirname, newDir, unprocessed = unprocessed)

    def __recurseDir(self, filename, newName, unprocessed = None, packageTree = None):
        if filename.isDirectory():
            # It's a directory name.  Recurse.
            prefix = newName
            if prefix and prefix[-1] != '/':
                prefix += '/'

            # First check if this is a Python package tree.  If so, add it
            # implicitly as a module.
            dirList = vfs.scanDirectory(filename)
            for subfile in dirList:
                filename = subfile.getFilename()
                if filename.getBasename() == '__init__.py':
                    moduleName = newName.replace("/", ".")
                    self.addModule([moduleName], filename=filename)

            for subfile in dirList:
                filename = subfile.getFilename()
                self.__recurseDir(filename, prefix + filename.getBasename(),
                                  unprocessed = unprocessed)
            return
        elif not filename.exists():
            # It doesn't exist.  Perhaps it's a virtual file.  Ignore it.
            return

        # It's a file name.  Add it.
        ext = filename.getExtension()
        if ext == 'py':
            self.currentPackage.addFile(filename, newName = newName,
                                        explicit = False, unprocessed = unprocessed)
        else:
            if ext == 'pz' or ext == 'gz':
                # Strip off an implicit .pz extension.
                newFilename = Filename(filename)
                newFilename.setExtension('')
                newFilename = Filename(str(newFilename))
                ext = newFilename.getExtension()

            if ext in self.knownExtensions:
                if ext in self.textExtensions:
                    filename.setText()
                else:
                    filename.setBinary()
                self.currentPackage.addFile(filename, newName = newName,
                                            explicit = False, unprocessed = unprocessed)
            elif not ext in self.suppressWarningForExtensions:
                newCount = self.currentPackage.ignoredDirFiles.get(ext, 0) + 1
                self.currentPackage.ignoredDirFiles[ext] = newCount

                if self.verbosePrint:
                    self.notify.warning("ignoring file %s" % filename)

    def readContentsFile(self):
        """ Reads the contents.xml file at the beginning of
        processing. """

        self.hosts = {}
        # Since we've blown away the self.hosts map, we have to make
        # sure that our own host at least is added to the map.
        self.addHost(self.host)

        self.maxAge = 0
        self.contentsSeq = SeqValue()
        self.contents = {}
        self.contentsChanged = False

        if not self.allowPackages:
            # Don't bother.
            return

        contentsFilename = Filename(self.installDir, 'contents.xml')
        doc = TiXmlDocument(contentsFilename.toOsSpecific())
        if not doc.LoadFile():
            # Couldn't read file.
            return

        xcontents = doc.FirstChildElement('contents')
        if xcontents:
            maxAge = xcontents.Attribute('max_age')
            if maxAge:
                self.maxAge = int(maxAge)

            self.contentsSeq.loadXml(xcontents)

            xhost = xcontents.FirstChildElement('host')
            if xhost:
                he = self.HostEntry()
                he.loadXml(xhost, self)
                self.hosts[he.url] = he
                self.host = he.url

            xpackage = xcontents.FirstChildElement('package')
            while xpackage:
                pe = self.PackageEntry()
                pe.loadXml(xpackage)
                self.contents[pe.getKey()] = pe
                xpackage = xpackage.NextSiblingElement('package')

    def writeContentsFile(self):
        """ Rewrites the contents.xml file at the end of
        processing. """

        if not self.contentsChanged:
            # No need to rewrite.
            return

        contentsFilename = Filename(self.installDir, 'contents.xml')
        doc = TiXmlDocument(contentsFilename.toOsSpecific())
        decl = TiXmlDeclaration("1.0", "utf-8", "")
        doc.InsertEndChild(decl)

        xcontents = TiXmlElement('contents')
        if self.maxAge:
            xcontents.SetAttribute('max_age', str(self.maxAge))

        self.contentsSeq += 1
        self.contentsSeq.storeXml(xcontents)

        if self.host:
            he = self.hosts.get(self.host, None)
            if he:
                xhost = he.makeXml(packager = self)
                xcontents.InsertEndChild(xhost)

        contents = sorted(self.contents.items())
        for key, pe in contents:
            xpackage = pe.makeXml()
            xcontents.InsertEndChild(xpackage)

        doc.InsertEndChild(xcontents)
        doc.SaveFile()


# The following class and function definitions represent a few sneaky
# Python tricks to allow the pdef syntax to contain the pseudo-Python
# code they do.  These tricks bind the function and class definitions
# into a bit table as they are parsed from the pdef file, so we can
# walk through that table later and perform the operations requested
# in order.

class metaclass_def(type):
    """ A metaclass is invoked by Python when the class definition is
    read, for instance to define a child class.  By defining a
    metaclass for class_p3d and class_package, we can get a callback
    when we encounter "class foo(p3d)" in the pdef file.  The callback
    actually happens after all of the code within the class scope has
    been parsed first. """

    def __new__(self, name, bases, dict):

        # At the point of the callback, now, "name" is the name of the
        # class we are instantiating, "bases" is the list of parent
        # classes, and "dict" is the class dictionary we have just
        # parsed.

        # If "dict" contains __metaclass__, then we must be parsing
        # class_p3d or class_ppackage, below--skip it.  But if it
        # doesn't contain __metaclass__, then we must be parsing
        # "class foo(p3d)" (or whatever) from the pdef file.

        if '__metaclass__' not in dict:
            # Get the context in which this class was created
            # (presumably, the module scope) out of the stack frame.
            frame = sys._getframe(1)
            mdict = frame.f_locals
            lineno = frame.f_lineno

            # Store the class name on a statements list in that
            # context, so we can later resolve the class names in
            # the order they appeared in the file.
            mdict.setdefault('__statements', []).append((lineno, 'class', name, None, None))

        return type.__new__(self, name, bases, dict)


# Define these dynamically to stay compatible with Python 2 and 3.
class_p3d = metaclass_def(str('class_p3d'), (), {})
class_package = metaclass_def(str('class_package'), (), {})
class_solo = metaclass_def(str('class_solo'), (), {})


class func_closure:
    """ This class is used to create a closure on the function name,
    and also allows the ``*args, **kw`` syntax.  In Python, the lambda
    syntax, used with default parameters, is used more often to create
    a closure (that is, a binding of one or more variables into a
    callable object), but that syntax doesn't work with ``**kw``.
    Fortunately, a class method is also a form of a closure, because
    it binds self; and this doesn't have any syntax problems with
    ``**kw``. """

    def __init__(self, name):
        self.name = name

    def generic_func(self, *args, **kw):
        """ This method is bound to all the functions that might be
        called from the pdef file.  It's a special function; when it is
        called, it does nothing but store its name and arguments in the
        caller's local scope, where they can be pulled out later. """

        # Get the context in which this function was called (presumably,
        # the class dictionary) out of the stack frame.
        frame = sys._getframe(1)
        cldict = frame.f_locals
        lineno = frame.f_lineno

        # Store the function on a statements list in that context, so we
        # can later walk through the function calls for each class.
        cldict.setdefault('__statements', []).append((lineno, 'func', self.name, args, kw))
