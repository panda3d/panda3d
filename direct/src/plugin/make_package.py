#! /usr/bin/env python

"""
This command is used to build a downloadable package for the p3d
plugin to retrieve and install.  It examines the files in the current
directory, assumes they are all intended to be part of the package,
and constructs the necessary package xml file and archive file for
hosting on the web serevr.

make_package.py [opts]

Options:

  -s source_dir

     Specify the source directory.  This is the root directory that
     contains the package contents.  The default is the current
     directory.

  -d stage_dir

     Specify the staging directory.  This is a temporary directory on
     the local machine that will be filled with the contents of the
     package directory for the web server.

  -p name_version[_platform]

     Specify the package name, version, and optional platfom, of the
     package to build.

"""

import sys
import getopt
import os
import zlib

import direct
from pandac.PandaModules import *

from FileSpec import FileSpec

class ArgumentError(AttributeError):
    pass

class PackageMaker:
    def __init__(self):
        self.startDir = None
        self.stageDir = None
        self.packageName = None
        self.packageVersion = None
        self.packagePlatform = None

        self.bogusExtensions = []

    def build(self):
        if not self.startDir:
            raise ArgumentError, "Start directory not specified."
        if not self.stageDir:
            raise ArgumentError, "Stage directory not specified."
        if not self.packageName or not self.packageVersion:
            raise ArgumentError, "Package name and version not specified."

        # First, scan the components.  If we find any
        # platform-dependent files, we automatically assume we're
        # building a platform-specific package.
        self.components = []
        self.findComponents()

        for ext in self.bogusExtensions:
            print "Warning: Found files of type %s, inconsistent with declared platform %s" % (
                ext, self.packagePlatform)

        self.packageFullname = '%s_%s' % (
            self.packageName, self.packageVersion)

        self.packageStageDir = Filename(self.stageDir, '%s/%s' % (self.packageName, self.packageVersion))

        if self.packagePlatform:
            self.packageFullname += '_%s' % (self.packagePlatform)
            self.packageStageDir = Filename(self.packageStageDir, self.packagePlatform)

        Filename(self.packageStageDir, '.').makeDir()
        self.cleanDir(self.packageStageDir)

        uncompressedArchiveBasename = '%s.mf' % (self.packageFullname)
        uncompressedArchivePathname = Filename(self.packageStageDir, uncompressedArchiveBasename)
        self.archive = Multifile()
        if not self.archive.openWrite(uncompressedArchivePathname):
            raise IOError, "Couldn't open %s for writing" % (uncompressedArchivePathname)

        self.addComponents()
        self.archive.close()

        uncompressedArchive = FileSpec(uncompressedArchiveBasename, uncompressedArchivePathname)

        compressedArchiveBasename = '%s.mf.pz' % (self.packageFullname)
        compressedArchivePathname = Filename(self.packageStageDir, compressedArchiveBasename)

        print "\ncompressing"

        source = open(uncompressedArchivePathname.toOsSpecific(), 'rb')
        target = open(compressedArchivePathname.toOsSpecific(), 'wb')
        #z = zlib.compressobj(9)  # Temporary.
        z = zlib.compressobj(1)
        data = source.read(4096)
        while data:
            target.write(z.compress(data))
            data = source.read(4096)
        target.write(z.flush())
        target.close()
        source.close()

        compressedArchive = FileSpec(compressedArchiveBasename, compressedArchivePathname)

        uncompressedArchivePathname.unlink()

        descFileBasename = '%s.xml' % (self.packageFullname)
        descFilePathname = Filename(self.packageStageDir, descFileBasename)

        f = open(descFilePathname.toOsSpecific(), 'w')
        print >> f, '<?xml version="1.0" ?>'
        print >> f, ''
        print >> f, '<package name="%s" version="%s" platform="%s">' % (self.packageName, self.packageVersion, self.packagePlatform or '')
        print >> f, '  <uncompressed_archive %s />' % (uncompressedArchive.getParams())
        print >> f, '  <compressed_archive %s />' % (compressedArchive.getParams())
        for file in self.components:
            print >> f, '  <component %s />' % (file.getParams())
        print >> f, '</package>'
        f.close()
        

    def cleanDir(self, dirname):
        """ Remove all the files in the named directory.  Does not
        operate recursively. """

        for filename in os.listdir(dirname.toOsSpecific()):
            pathname = Filename(dirname, filename)
            pathname.unlink()

    def findComponents(self):
        """ Walks through all the files in the start directory and
        adds them to self.components.  Recursively visits
        sub-directories. """

        startDir = self.startDir.toOsSpecific()
        if startDir.endswith(os.sep):
            startDir = startDir[:-1]
        prefix = startDir + os.sep
        for dirpath, dirnames, filenames in os.walk(startDir):
            if dirpath == startDir:
                localpath = ''
            else:
                assert dirpath.startswith(prefix)
                localpath = dirpath[len(prefix):].replace(os.sep, '/') + '/'

            for basename in filenames:
                file = FileSpec(localpath + basename,
                                Filename(self.startDir, localpath + basename))
                self.components.append(file)
                print file.filename

                ext = os.path.splitext(basename)[1]
                if ext in ['.exe', '.dll', '.pyd']:
                    self.ensurePlatform('win32', ext, file)
                elif ext in ['.dylib']:
                    self.ensurePlatform('osx', ext, file)
                elif ext in ['.so']:
                    self.ensurePlatform(None, ext, file)

    def ensurePlatform(self, platform, ext, file):
        """ We have just encountered a file that is specific to a
        particular platform.  Checks that the declared platform is
        consistent with this.  """

        if platform and self.packagePlatform:
            if self.packagePlatform.startswith(platform):
                # This platform is consistent with our declared
                # platform.
                return
        elif self.packagePlatform:
            # This platform is consistent with any particular
            # platform.
            return

        if ext in self.bogusExtensions:
            # Already reported this one.
            return
        
        self.bogusExtensions.append(ext)
            

    def addComponents(self):
        for file in self.components:
            self.archive.addSubfile(file.filename, file.pathname, 0)
                
def makePackage(args):
    opts, args = getopt.getopt(args, 's:d:p:h')

    pm = PackageMaker()
    pm.startDir = Filename('.')
    for option, value in opts:
        if option == '-s':
            pm.startDir = Filename.fromOsSpecific(value)            
        elif option == '-d':
            pm.stageDir = Filename.fromOsSpecific(value)
        elif option == '-p':
            tokens = value.split('_')
            if len(tokens) >= 1:
                pm.packageName = tokens[0]
            if len(tokens) >= 2:
                pm.packageVersion = tokens[1]
            if len(tokens) >= 3:
                pm.packagePlatform = tokens[2]
            if len(tokens) >= 4:
                raise ArgumentError, 'Too many tokens in string: %s' % (value)
            
        elif option == '-h':
            print __doc__
            sys.exit(1)

    pm.build()
        

if __name__ == '__main__':
    try:
        makePackage(sys.argv[1:])
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
