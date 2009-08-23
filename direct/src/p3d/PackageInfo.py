from pandac.PandaModules import Filename

class PackageInfo:

    """ This class represents a downloadable Panda3D package file that
    can be (or has been) installed into the current runtime.  It is
    the Python equivalent of the P3DPackage class in the core API. """

    def __init__(self, host, packageName, packageVersion, platform = None):
        self.host = host
        self.packageName = packageName
        self.packageVersion = packageVersion
        self.platform = platform

        self.packageFullname = '%s_%s' % (self.packageName, self.packageVersion)
        self.packageDir = Filename(host.hostDir, 'packages/%s/%s' % (self.packageName, self.packageVersion))
        self.descFileBasename = self.packageFullname + '.xml'

        # These will be filled in by HostInfo when the package is read
        # from contents.xml.
        self.descFile = None
        self.importDescFile = None
