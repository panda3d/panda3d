"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["InstalledHostData"]

from panda3d.core import URLSpec

class InstalledHostData:
    """ A list of instances of this class is returned by
    AppRunner.scanInstalledPackages().  Each of these corresponds to a
    particular host that has provided packages that have been
    installed on the local client. """

    def __init__(self, host, dirnode):
        self.host = host
        self.pathname = dirnode.pathname
        self.totalSize = dirnode.getTotalSize()
        self.packages = []

        if self.host:
            self.hostUrl = self.host.hostUrl
            self.descriptiveName = self.host.descriptiveName
            if not self.descriptiveName:
                self.descriptiveName = URLSpec(self.hostUrl).getServer()
        else:
            self.hostUrl = 'unknown'
            self.descriptiveName = 'unknown'
