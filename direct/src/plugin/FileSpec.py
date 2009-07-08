from pandac.PandaModules import HashVal

class FileSpec:
    """ Used by make_package and make_contents.  Represents a single
    file in the directory, and its associated timestamp, size, and md5
    hash. """
    
    def __init__(self, filename, pathname):
        self.filename = filename
        self.pathname = pathname

        self.size = pathname.getFileSize()
        self.timestamp = pathname.getTimestamp()

        hv = HashVal()
        hv.hashFile(pathname)
        self.hash = hv.asHex()

    def getParams(self):
        return 'filename="%s" size="%s" timestamp="%s" hash="%s"' % (
            self.filename, self.size, self.timestamp, self.hash)
