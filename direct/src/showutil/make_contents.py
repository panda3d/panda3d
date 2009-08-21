#! /usr/bin/env python

"""
This command will build the contents.xml file at the top of a Panda3D
download hierarchy.  This file lists all of the packages hosted here,
along with their current versions.

This program runs on a local copy of the hosting directory hierarchy;
it must be a complete copy to generate a complete contents.xml file.

make_contents.py [opts]

Options:

  -d stage_dir

     Specify the staging directory.  This is a temporary directory on
     the local machine that contains a copy of the web server
     contents.  The default is the current directory.

"""

import sys
import getopt
import os

try:
    import hashlib
except ImportError:
    # Legacy Python support
    import md5 as hashlib

class ArgumentError(AttributeError):
    pass

class FileSpec:
    """ Represents a single file in the directory, and its associated
    timestamp, size, and md5 hash. """
    
    def __init__(self, filename, pathname):
        self.filename = filename
        self.pathname = pathname

        s = os.stat(pathname)
        self.size = s.st_size
        self.timestamp = int(s.st_mtime)

        m = hashlib.md5()
        m.update(open(pathname, 'rb').read())
        self.hash = m.hexdigest()

    def getParams(self):
        return 'filename="%s" size="%s" timestamp="%s" hash="%s"' % (
            self.filename, self.size, self.timestamp, self.hash)

class ContentsMaker:
    def __init__(self):
        self.installDir = None

    def build(self):
        if not self.installDir:
            raise ArgumentError, "Stage directory not specified."

        self.packages = []
        self.scanDirectory()

        if not self.packages:
            raise ArgumentError, "No packages found."

        # Now write the contents.xml file.
        contentsFileBasename = 'contents.xml'
        contentsFilePathname = os.path.join(self.installDir, contentsFileBasename)

        f = open(contentsFilePathname, 'w')
        print >> f, '<?xml version="1.0" ?>'
        print >> f, ''
        print >> f, '<contents>'
        for type, packageName, packagePlatform, packageVersion, file in self.packages:
            print >> f, '  <%s name="%s" platform="%s" version="%s" %s />' % (
                type, packageName, packagePlatform or '', packageVersion, file.getParams())
        print >> f, '</contents>'
        f.close()

    def scanDirectory(self):
        """ Walks through all the files in the stage directory and
        looks for the package directory xml files. """

        startDir = self.installDir
        if startDir.endswith(os.sep):
            startDir = startDir[:-1]
        prefix = startDir + os.sep
        for dirpath, dirnames, filenames in os.walk(startDir):
            if dirpath == startDir:
                localpath = ''
                xml = ''
            else:
                assert dirpath.startswith(prefix)
                localpath = dirpath[len(prefix):].replace(os.sep, '/') + '/'
                xml = dirpath[len(prefix):].replace(os.sep, '_') + '.xml'

            type = 'package'

            # A special case: the "plugin" and "coreapi" directories
            # don't have xml files, just dll's.
            if xml.startswith('plugin_') or xml.startswith('coreapi_'):
                if filenames:
                    assert len(filenames) == 1
                    xml = filenames[0]
                    type = 'plugin'

            if xml not in filenames:
                continue
            
            if localpath.count('/') == 2:
                packageName, packageVersion, junk = localpath.split('/')
                packagePlatform = None

            elif localpath.count('/') == 3:
                packageName, packagePlatform, packageVersion, junk = localpath.split('/')
            else:
                continue

            file = FileSpec(localpath + xml,
                            os.path.join(self.installDir, localpath + xml))
            print file.filename
            self.packages.append((type, packageName, packagePlatform, packageVersion, file))

            if type == 'package':
                # Look for an _import.xml file, too.
                xml = xml[:-4] + '_import.xml'
                try:
                    file = FileSpec(localpath + xml,
                                    os.path.join(self.installDir, localpath + xml))
                except OSError:
                    file = None
                if file:
                    print file.filename
                    self.packages.append(('import', packageName, packagePlatform, packageVersion, file))
        
                
def makeContents(args):
    opts, args = getopt.getopt(args, 'd:h')

    cm = ContentsMaker()
    cm.installDir = '.'
    for option, value in opts:
        if option == '-d':
            cm.installDir = value
            
        elif option == '-h':
            print __doc__
            sys.exit(1)

    cm.build()
        

if __name__ == '__main__':
    try:
        makeContents(sys.argv[1:])
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
