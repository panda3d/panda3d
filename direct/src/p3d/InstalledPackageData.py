"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["InstalledPackageData"]

class InstalledPackageData:
    """ A list of instances of this class is maintained by
    InstalledHostData (which is in turn returned by
    AppRunner.scanInstalledPackages()).  Each of these corresponds to
    a particular package that has been installed on the local
    client. """

    def __init__(self, package, dirnode):
        self.package = package
        self.pathname = dirnode.pathname
        self.totalSize = dirnode.getTotalSize()
        self.lastUse = None

        if self.package:
            self.displayName = self.package.getFormattedName()
            xusage = self.package.getUsage()

            if xusage:
                lastUse = xusage.Attribute('last_use')
                try:
                    lastUse = int(lastUse or '')
                except ValueError:
                    lastUse = None
                self.lastUse = lastUse

        else:
            self.displayName = dirnode.pathname.getBasename()

