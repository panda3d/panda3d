"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["ScanDirectoryNode"]

from panda3d.core import VirtualFileSystem, VirtualFileMountSystem, Filename, TiXmlDocument
vfs = VirtualFileSystem.getGlobalPtr()

class ScanDirectoryNode:
    """ This class is used to scan a list of files on disk. """

    def __init__(self, pathname, ignoreUsageXml = False):
        self.pathname = pathname
        self.filenames = []
        self.fileSize = 0
        self.nested = []
        self.nestedSize = 0

        xusage = None
        if not ignoreUsageXml:
            # Look for a usage.xml file in this directory.  If we find
            # one, we read it for the file size and then stop here, as
            # an optimization.
            usageFilename = Filename(pathname, 'usage.xml')
            doc = TiXmlDocument(usageFilename.toOsSpecific())
            if doc.LoadFile():
                xusage = doc.FirstChildElement('usage')
                if xusage:
                    diskSpace = xusage.Attribute('disk_space')
                    try:
                        diskSpace = int(diskSpace or '')
                    except ValueError:
                        diskSpace = None
                    if diskSpace is not None:
                        self.fileSize = diskSpace
                        return

        files = vfs.scanDirectory(self.pathname)
        if files is None:
            files = []
        for vfile in files:
            if hasattr(vfile, 'getMount'):
                if not isinstance(vfile.getMount(), VirtualFileMountSystem):
                    # Not a real file; ignore it.
                    continue

            if vfile.isDirectory():
                # A nested directory.
                subdir = ScanDirectoryNode(vfile.getFilename(), ignoreUsageXml = ignoreUsageXml)
                self.nested.append(subdir)
                self.nestedSize += subdir.getTotalSize()

            elif vfile.isRegularFile():
                # A nested file.
                self.filenames.append(vfile.getFilename())
                self.fileSize += vfile.getFileSize()

            else:
                # Some other wacky file thing.
                self.filenames.append(vfile.getFilename())

        if xusage:
            # Now update the usage.xml file with the newly-determined
            # disk space.
            xusage.SetAttribute('disk_space', str(self.getTotalSize()))
            tfile = Filename.temporary(str(pathname), '.xml')
            if doc.SaveFile(tfile.toOsSpecific()):
                tfile.renameTo(usageFilename)

    def getTotalSize(self):
        return self.nestedSize + self.fileSize

    def extractSubdir(self, pathname):
        """ Finds the ScanDirectoryNode within this node that
        corresponds to the indicated full pathname.  If it is found,
        removes it from its parent, and returns it.  If it is not
        found, returns None. """

        # We could be a little smarter here, but why bother.  Just
        # recursively search all children.
        for subdir in self.nested:
            if subdir.pathname == pathname:
                self.nested.remove(subdir)
                self.nestedSize -= subdir.getTotalSize()
                return subdir

            result = subdir.extractSubdir(pathname)
            if result:
                self.nestedSize -= result.getTotalSize()
                if subdir.getTotalSize() == 0:
                    # No other files in the subdirectory that contains
                    # this package; remove it too.
                    self.nested.remove(subdir)
                return result

        return None


