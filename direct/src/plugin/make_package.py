#! /usr/bin/env python

"""
This command is used to build a downloadable package for the p3d
plugin to retrieve and install.  It examines the files in the current
directory, assumes they are all intended to be part of the package,
and constructs the necessary package xml file and archive file for
hosting on the web serevr.

make_package.py [opts]

Options:

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
import tarfile
import gzip
import stat
import md5

import direct
from pandac.PandaModules import *

class ArgumentError(AttributeError):
    pass

class FileSpec:
    def __init__(self, filename, pathname):
        self.filename = filename
        self.pathname = pathname
        self.size = 0
        self.timestamp = 0
        self.hash = None

        s = os.stat(self.pathname)
        self.size = s[stat.ST_SIZE]
        self.timestamp = s[stat.ST_MTIME]

        m = md5.new()
        f = open(self.pathname, 'rb')
        data = f.read(4096)
        while data:
            m.update(data)
            data = f.read(4096)
        f.close()

        self.hash = m.hexdigest()

    def get_params(self):
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

        uncompressedArchiveBasename = '%s.tar' % (self.packageFullname)
        uncompressedArchivePathname = os.path.join(self.stageDir, uncompressedArchiveBasename)
        self.archive = tarfile.open(uncompressedArchivePathname, 'w')

        self.components = []

        self.addComponents()
        self.archive.close()

        uncompressedArchive = FileSpec(uncompressedArchiveBasename, uncompressedArchivePathname)

        compressedArchiveBasename = '%s.tgz' % (self.packageFullname)
        compressedArchivePathname = os.path.join(self.stageDir, compressedArchiveBasename)

        print "\ncompressing"
        f = open(uncompressedArchivePathname, 'rb')
        gz = gzip.open(compressedArchivePathname, 'w', 9)
        data = f.read(4096)
        while data:
            gz.write(data)
            data = f.read(4096)
        gz.close()
        f.close()

        compressedArchive = FileSpec(compressedArchiveBasename, compressedArchivePathname)

        os.unlink(uncompressedArchivePathname)

        descFileBasename = '%s.xml' % (self.packageFullname)
        descFilePathname = os.path.join(self.stageDir, descFileBasename)

        f = open(descFilePathname, 'w')
        print >> f, '<?xml version="1.0" ?>'
        print >> f, ''
        print >> f, '<package name="%s" version="%s">' % (self.packageName, self.packageVersion)
        print >> f, '  <uncompressed_archive %s />' % (uncompressedArchive.get_params())
        print >> f, '  <compressed_archive %s />' % (compressedArchive.get_params())
        for file in self.components:
            print >> f, '  <component %s />' % (file.get_params())
        print >> f, '</package>'
        f.close()
        

    def cleanDir(self, dirname):
        """ Remove all the files in the named directory.  Does not
        operate recursively. """

        for filename in os.listdir(dirname):
            pathname = os.path.join(dirname, filename)
            try:
                os.unlink(pathname)
            except OSError:
                pass

    def addComponents(self):
        """ Walks through all the files in the start directory and
        adds them to the archive.  Recursively visits
        sub-directories. """

        startDir = self.startDir
        if startDir.endswith(os.sep):
            startDir = startDir[:-1]
        elif os.altsep and startDir.endswith(os.altsep):
            startDir = startDir[:-1]
        prefix = startDir + os.sep
        for dirpath, dirnames, filenames in os.walk(startDir):
            if dirpath == startDir:
                localpath = ''
            else:
                assert dirpath.startswith(prefix)
                localpath = dirpath[len(prefix):]

            for basename in filenames:
                file = FileSpec(os.path.join(localpath, basename),
                                os.path.join(startDir, basename))
                print file.filename
                self.components.append(file)
                self.archive.add(file.pathname, file.filename, recursive = False)
                
def makePackage(args):
    opts, args = getopt.getopt(args, 'd:p:v:h')

    pm = PackageMaker()
    pm.startDir = '.'
    for option, value in opts:
        if option == '-d':
            pm.stageDir = Filename.fromOsSpecific(value).toOsSpecific()
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
