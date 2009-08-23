
class PackageInfo:

    """ This class represents a downloadable Panda3D package file that
    can be (or has been) installed into the current runtime. """

    def __init__(self, name, platform, version, host, installDir):
        self.name = name
        self.platform = platform
        self.version = version
        self.host = host
        self.installDir = installDir
