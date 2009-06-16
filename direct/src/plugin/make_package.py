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

  -p package_name
  -v package_version

     Specify the name and version of the package to build.

"""

import sys
import getopt
import os
import zlib

import direct
from pandac.PandaModules import *

class ArgumentError(AttributeError):
    pass

class FileSpec:
    def __init__(self, filename, pathname):
        self.filename = filename
        self.pathname = pathname

        self.size = pathname.getFileSize()
        self.timestamp = pathname.getTimestamp()

        hv = HashVal()
        hv.hashFile(pathname)
        self.hash = hv.asHex()

    def getParams(self):
        return 'filename="%s" size=%s timestamp=%s hash="%s"' % (
            self.filename, self.size, self.timestamp, self.hash)

class PackageMaker:
    def __init__(self):
        self.startDir = None
        self.stageDir = None
        self.packageName = None
        self.packageVersion = None


    def build(self):
        if not self.startDir:
            raise ArgumentError, "Start directory not specified."
        if not self.stageDir:
            raise ArgumentError, "Stage directory not specified."
        if not self.packageName or not self.packageVersion:
            raise ArgumentError, "Package name and version not specified."

        self.packageFullname = '%s_%s' % (
            self.packageName, self.packageVersion)

        self.cleanDir(self.stageDir)

        uncompressedArchiveBasename = '%s.mf' % (self.packageFullname)
        uncompressedArchivePathname = Filename(self.stageDir, uncompressedArchiveBasename)
        self.archive = Multifile()
        if not self.archive.openWrite(uncompressedArchivePathname):
            raise IOError, "Couldn't open %s for writing" % (uncompressedArchivePathname)

        self.components = []

        self.addComponents()
        self.archive.close()

        uncompressedArchive = FileSpec(uncompressedArchiveBasename, uncompressedArchivePathname)

        compressedArchiveBasename = '%s.mf.pz' % (self.packageFullname)
        compressedArchivePathname = Filename(self.stageDir, compressedArchiveBasename)

        print "\ncompressing"

        source = open(uncompressedArchivePathname.toOsSpecific(), 'rb')
        target = open(compressedArchivePathname.toOsSpecific(), 'wb')
        z = zlib.compressobj(9)
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
        descFilePathname = Filename(self.stageDir, descFileBasename)

        f = open(descFilePathname.toOsSpecific(), 'w')
        print >> f, '<?xml version="1.0" ?>'
        print >> f, ''
        print >> f, '<package name="%s" version="%s">' % (self.packageName, self.packageVersion)
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

    def addComponents(self):
        """ Walks through all the files in the start directory and
        adds them to the archive.  Recursively visits
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
                localpath = dirpath[len(prefix):] + '/'

            for basename in filenames:
                file = FileSpec(localpath + basename,
                                Filename(self.startDir, basename))
                print file.filename
                self.components.append(file)
                self.archive.addSubfile(file.filename, file.pathname, 0)
                
def makePackage(args):
    opts, args = getopt.getopt(args, 's:d:p:v:h')

    pm = PackageMaker()
    pm.startDir = Filename('.')
    for option, value in opts:
        if option == '-s':
            pm.startDir = Filename.fromOsSpecific(value)            
        elif option == '-d':
            pm.stageDir = Filename.fromOsSpecific(value)
        elif option == '-p':
            pm.packageName = value
        elif option == '-v':
            pm.packageVersion = value
            
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
