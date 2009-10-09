import os
from pandac.PandaModules import Filename, HashVal, VirtualFileSystem

class FileSpec:
    """ This class represents a disk file whose hash and size
    etc. were read from an xml file.  This class provides methods to
    verify whether the file on disk matches the version demanded by
    the xml. """

    def __init__(self):
        self.actualFile = None

    def fromFile(self, packageDir, filename, pathname = None, st = None):
        """ Reads the file information from the indicated file.  If st
        is supplied, it is the result of os.stat on the filename. """
        
        vfs = VirtualFileSystem.getGlobalPtr()

        filename = Filename(filename)
        if pathname is None:
            pathname = Filename(packageDir, filename)
        
        self.filename = filename.cStr()
        self.basename = filename.getBasename()

        if st is None:
            st = os.stat(pathname.toOsSpecific())
        self.size = st.st_size
        self.timestamp = st.st_mtime

        hv = HashVal()
        hv.hashFile(pathname)
        self.hash = hv.asHex()

    def loadXml(self, xelement):
        """ Reads the file information from the indicated XML
        element. """
        
        self.filename = xelement.Attribute('filename')
        self.basename = None
        if self.filename:
            self.basename = Filename(self.filename).getBasename()
            
        size = xelement.Attribute('size')
        try:
            self.size = int(size)
        except:
            self.size = 0

        timestamp = xelement.Attribute('timestamp')
        try:
            self.timestamp = int(timestamp)
        except:
            self.timestamp = 0

        self.hash = xelement.Attribute('hash')

    def storeXml(self, xelement):
        """ Adds the file information to the indicated XML
        element. """

        if self.filename:
            xelement.SetAttribute('filename', self.filename)
        if self.size:
            xelement.SetAttribute('size', str(self.size))
        if self.timestamp:
            xelement.SetAttribute('timestamp', str(int(self.timestamp)))
        if self.hash:
            xelement.SetAttribute('hash', self.hash)

    def storeMiniXml(self, xelement):
        """ Adds the just the "mini" file information--size and
        hash--to the indicated XML element. """

        if self.size:
            xelement.SetAttribute('size', str(self.size))
        if self.hash:
            xelement.SetAttribute('hash', self.hash)
            
    def quickVerify(self, packageDir = None, pathname = None):
        """ Performs a quick test to ensure the file has not been
        modified.  This test is vulnerable to people maliciously
        attempting to fool the program (by setting datestamps etc.).

        Returns true if it is intact, false if it needs to be
        redownloaded. """

        if not pathname:
            pathname = Filename(packageDir, self.filename)
        try:
            st = os.stat(pathname.toOsSpecific())
        except OSError:
            # If the file is missing, the file fails.
            #print "file not found: %s" % (pathname)
            return False

        if st.st_size != self.size:
            # If the size is wrong, the file fails.
            #print "size wrong: %s" % (pathname)
            return False

        if st.st_mtime == self.timestamp:
            # If the size is right and the timestamp is right, the
            # file passes.
            #print "file ok: %s" % (pathname)
            return True

        #print "modification time wrong: %s" % (pathname)

        # If the size is right but the timestamp is wrong, the file
        # soft-fails.  We follow this up with a hash check.
        if not self.checkHash(packageDir, pathname, st):
            # Hard fail, the hash is wrong.
            #print "hash check wrong: %s" % (pathname)
            return False

        #print "hash check ok: %s" % (pathname)

        # The hash is OK after all.  Change the file's timestamp back
        # to what we expect it to be, so we can quick-verify it
        # successfully next time.
        self.__updateTimestamp(pathname, st)

        return True
        
            
    def fullVerify(self, packageDir = None, pathname = None):
        """ Performs a more thorough test to ensure the file has not
        been modified.  This test is less vulnerable to malicious
        attacks, since it reads and verifies the entire file.

        Returns true if it is intact, false if it needs to be
        redownloaded. """

        if not pathname:
            pathname = Filename(packageDir, self.filename)
        try:
            st = os.stat(pathname.toOsSpecific())
        except OSError:
            # If the file is missing, the file fails.
            #print "file not found: %s" % (pathname)
            return False

        if st.st_size != self.size:
            # If the size is wrong, the file fails;
            #print "size wrong: %s" % (pathname)
            return False

        if not self.checkHash(packageDir, pathname, st):
            # Hard fail, the hash is wrong.
            #print "hash check wrong: %s" % (pathname)
            return False

        #print "hash check ok: %s" % (pathname)

        # The hash is OK.  If the timestamp is wrong, change it back
        # to what we expect it to be, so we can quick-verify it
        # successfully next time.
        if st.st_mtime != self.timestamp:
            self.__updateTimestamp(pathname, st)

        return True

    def __updateTimestamp(self, pathname, st):
        # On Windows, we have to change the file to read-write before
        # we can successfully update its timestamp.
        try:
            os.chmod(pathname.toOsSpecific(), 0755)
            os.utime(pathname.toOsSpecific(), (st.st_atime, self.timestamp))
            os.chmod(pathname.toOsSpecific(), 0555)
        except OSError:
            pass

    def checkHash(self, packageDir, pathname, st):
        """ Returns true if the file has the expected md5 hash, false
        otherwise.  As a side effect, stores a FileSpec corresponding
        to the on-disk file in self.actualFile. """

        fileSpec = FileSpec()
        fileSpec.fromFile(packageDir, self.filename,
                          pathname = pathname, st = st)
        self.actualFile = fileSpec

        return (fileSpec.hash == self.hash)
    
