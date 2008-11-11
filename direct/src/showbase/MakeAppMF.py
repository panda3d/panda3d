""" This module will pack a Panda application, consisting of a
directory tree of .py files and models, into a multifile for
distribution and running with RunAppMF.py.  To run it, use:

python MakeAppMF.py app.mf [application_root]

where application_root is the root directory of the application.  If
it is omitted, the default is the current directory.  The root
directory should contain at a file named main.py, which is imported to
start the application.  If main.py contains a function called main(),
this function is called after importing it; otherwise, taskMgr.run()
is called.
"""

import sys
import direct
from pandac.PandaModules import *

vfs = VirtualFileSystem.getGlobalPtr()

class ArgumentError(AttributeError):
    pass

class AppPacker:

    compression_level = 6
    imported_maps = 'imported_maps'

    # Text files that are copied (and compressed) to the multifile
    # without processing.
    text_extensions = [ 'prc' ]

    # Binary files that are copied and compressed without processing.
    binary_extensions = [ 'ttf', 'wav', 'mid' ]

    # Binary files that are considered uncompressible, and are copied
    # without compression.
    uncompressible_extensions = [ 'mp3' ]

    def __init__(self, multifile_name):
        # Make sure any pre-existing file is removed.
        Filename(multifile_name).unlink()
            
        self.multifile = Multifile()
        if not self.multifile.openReadWrite(multifile_name):
            raise ArgumentError, 'Could not open %s for writing' % (multifile_name)
        self.imported_textures = {}

        # Get the list of filename extensions that are recognized as
        # image files.
        self.image_extensions = []
        for type in PNMFileTypeRegistry.getGlobalPtr().getTypes():
            self.image_extensions += type.getExtensions()

    def scan(self, root):
        self.root = Filename(root)
        self.root.makeAbsolute(vfs.getCwd())
        self._recurse(self.root)

        self.multifile.repack()

    def _recurse(self, filename):
        dirList = vfs.scanDirectory(filename)
        if dirList:
            # It's a directory name.  Recurse.
            for subfile in dirList:
                self._recurse(subfile.getFilename())
            return

        # It's a real file.  Is it something we care about?
        ext = filename.getExtension().lower()
        outFilename = filename
        if ext == 'pz':
            # Strip off an implicit .pz extension.
            outFilename = Filename(filename)
            outFilename.setExtension('')
            outFilename = Filename(outFilename.cStr())
            ext = outFilename.getExtension().lower()
            
        if ext == 'py':
            self.addPyFile(filename)
        elif ext == 'egg':
            self.addEggFile(filename, outFilename)
        elif ext == 'bam':
            self.addBamFile(filename, outFilename)
        elif ext in self.image_extensions:
            self.addTexture(filename)
        elif ext in self.text_extensions:
            self.addTextFile(filename)
        elif ext in self.binary_extensions:
            self.addBinaryFile(filename)
        elif ext in self.uncompressible_extensions:
            self.addUncompressibleFile(filename)

    def addPyFile(self, filename):
        # For now, just add it as an ordinary file.  Later we'll
        # precompile these to .pyo's.
        self.addTextFile(filename)

    def addEggFile(self, filename, outFilename):
        # Precompile egg files to bam's.
        node = loadEggFile(filename)
        if not node:
            raise StandardError, 'Could not read egg file %s' % (filename)

        self.addNode(node, outFilename)

    def addBamFile(self, filename, outFilename):
        # Load the bam file so we can massage its textures.
        bamFile = BamFile()
        if not bamFile.openRead(filename):
            raise StandardError, 'Could not read bam file %s' % (filename)

        if not bamFile.resolve():
            raise StandardError, 'Could not resolve bam file %s' % (filename)

        node = bamFile.readNode()
        if not node:
            raise StandardError, 'Not a model file: %s' % (filename)
            
        self.addNode(node, outFilename)

    def addNode(self, node, filename):
        """ Converts the indicated node to a bam stream, and adds the
        bam file to the multifile under the indicated filename. """
        
        # Be sure to import all of the referenced textures, and tell
        # them their new location within the multifile.
        
        for tex in NodePath(node).findAllTextures():
            tex.setFilename(self.addTexture(tex.getFullpath()))
            if tex.hasAlphaFilename():
                tex.setAlphaFilename(self.addTexture(tex.getAlphaFullpath()))
                
        # Now generate an in-memory bam file.  Tell the bam writer to
        # keep the textures referenced by their in-multifile path.
        bamFile = BamFile()
        stream = StringStream()
        bamFile.openWrite(stream)
        bamFile.getWriter().setFileTextureMode(BTMUnchanged)
        bamFile.writeObject(node)
        bamFile.close()
        node = None

        # Now we have an in-memory bam file.
        rel = self.makeRelFilename(filename)
        rel.setExtension('bam')
        stream.seekg(0)
        self.multifile.addSubfile(rel.cStr(), stream, self.compression_level)

        # Flush it so the data gets written to disk immediately, so we
        # don't have to keep it around in ram.
        self.multifile.flush()
    
    def addTexture(self, filename):
        """ Adds the texture to the multifile, if it has not already
        been added.  If it is not within the root directory, copies it
        in (virtually) into a directory within the multifile named
        imported_maps.  Returns the new filename within the
        multifile. """

        assert not filename.isLocal()

        filename = Filename(filename)
        filename.makeAbsolute(vfs.getCwd())
        filename.makeTrueCase()

        rel = self.imported_textures.get(filename.cStr())
        if not rel:
            rel = Filename(filename)
            if not rel.makeRelativeTo(self.root, False):
                # Not within the multifile.
                rel = Filename(self.imported_maps, filename.getBasename())

            # Need to add it now.
            self.imported_textures[filename.cStr()] = rel
            filename = Filename(filename)
            filename.setBinary()
            self.multifile.addSubfile(rel.cStr(), filename, 0)

        return rel

    def mapTextureFilename(self, filename):
        """ Returns the filename within the multifile of the
        already-added texture. """

        filename = Filename(filename)
        filename.makeAbsolute(vfs.getCwd())
        filename.makeTrueCase()

        return self.imported_textures[filename.cStr()]

    def addTextFile(self, filename):
        """ Adds a generic text file to the multifile. """

        rel = self.makeRelFilename(filename)
        filename = Filename(filename)
        filename.setText()
        self.multifile.addSubfile(rel.cStr(), filename, self.compression_level)

    def addBinaryFile(self, filename):
        """ Adds a generic binary file to the multifile. """

        rel = self.makeRelFilename(filename)
        filename = Filename(filename)
        filename.setBinary()
        self.multifile.addSubfile(rel.cStr(), filename, self.compression_level)

    def addUncompressibleFile(self, filename):
        """ Adds a generic binary file to the multifile, without compression. """

        rel = self.makeRelFilename(filename)
        filename = Filename(filename)
        filename.setBinary()
        self.multifile.addSubfile(rel.cStr(), filename, 0)

    def makeRelFilename(self, filename):
        """ Returns the same filename, relative to self.root """
        rel = Filename(filename)
        result = rel.makeRelativeTo(self.root, False)
        assert result
        return rel


def makePackedApp(args):
    if not args:
        raise ArgumentError, "No destination app specified.  Use:\npython MakeAppMF.py app.mf"

    multifile_name = args[0]
    root = '.'
    if len(args) > 1:
        root = args[1]

    if len(args) > 2:
        raise ArgumentError, "Too many arguments."

    p = AppPacker(multifile_name)
    p.scan(root)

if __name__ == '__main__':
    try:
        makePackedApp(sys.argv[1:])
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
