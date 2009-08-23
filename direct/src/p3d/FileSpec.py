import os
from pandac.PandaModules import Filename, HashVal

class FileSpec:
    """ This class represents a disk file whose hash and size
    etc. were read from an xml file.  This class provides methods to
    verify whether the file on disk matches the version demanded by
    the xml. """

    def __init__(self, xelement):
        self.filename = xelement.Attribute('filename')
        self.basename = Filename(self.filename).getBasename()
        size = xelement.Attribute('size')
        try:
            self.size = int(size)
        except ValueError:
            self.size = 0

        timestamp = xelement.Attribute('timestamp')
        try:
            self.timestamp = int(timestamp)
        except ValueError:
            self.timestamp = 0

        self.hash = xelement.Attribute('hash')
            
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
        if not self.checkHash(pathname):
            # Hard fail, the hash is wrong.
            #print "hash check wrong: %s" % (pathname)
            return False

        #print "hash check ok: %s" % (pathname)

        # The hash is OK after all.  Change the file's timestamp back
        # to what we expect it to be, so we can quick-verify it
        # successfully next time.
        os.utime(pathname.toOsSpecific(), (st.st_atime, self.timestamp))

        return True
        
            
    def fullVerify(self, packageDir = None, pathname = None):
        """ Performs a more thorough test to ensure the file has not
        been modified.  This test is less vulnerable to malicious
        attacks, since it reads and verifies the entire file.

        Returns true if it is intact, false if it needs to be
        redownloaded. """

        if not pathname:
            pathname = Filename(packageDir, pathname)
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

        if not self.checkHash(pathname):
            # Hard fail, the hash is wrong.
            #print "hash check wrong: %s" % (pathname)
            return False

        #print "hash check ok: %s" % (pathname)

        # The hash is OK.  If the timestamp is wrong, change it back
        # to what we expect it to be, so we can quick-verify it
        # successfully next time.
        if st.st_mtime != self.timestamp:
            os.utime(pathname.toOsSpecific(), (st.st_atime, self.timestamp))

        return True

    def checkHash(self, pathname):
        """ Returns true if the file has the expected md5 hash, false
        otherwise. """

        hv = HashVal()
        hv.hashFile(pathname)
        return (hv.asHex() == self.hash)
    
