"""
.. deprecated:: 1.10.0
   The p3d packaging system has been replaced with the new setuptools-based
   system.  See the :ref:`distribution` manual section.
"""
__all__ = ["DWBPackageInstaller"]

from direct.p3d.PackageInstaller import PackageInstaller
from direct.gui.DirectWaitBar import DirectWaitBar
from direct.gui import DirectGuiGlobals as DGG

class DWBPackageInstaller(DirectWaitBar, PackageInstaller):
    """ This class presents a PackageInstaller that also inherits from
    DirectWaitBar, so it updates its own GUI as it downloads.

    Specify perPackage = True to make the progress bar reset for each
    package, or False (the default) to show one continuous progress
    bar for all packages.

    Specify updateText = True (the default) to update the text label
    with the name of the package or False to leave it up to you to set
    it.

    You can specify a callback function with finished = func; this
    function will be called, with one boolean parameter, when the
    download has completed.  The parameter will be true on success, or
    false on failure.
    """

    def __init__(self, appRunner, parent = None, **kw):
        PackageInstaller.__init__(self, appRunner)

        optiondefs = (
            ('borderWidth',    (0.01, 0.01),       None),
            ('relief',         DGG.SUNKEN,         self.setRelief),
            ('range',          1,                  self.setRange),
            ('barBorderWidth', (0.01, 0.01),       self.setBarBorderWidth),
            ('barColor',       (0.424, 0.647, 0.878, 1),  self.setBarColor),
            ('barRelief',      DGG.RAISED,         self.setBarRelief),
            ('text',           'Starting',         self.setText),
            ('text_pos',       (0, -0.025),        None),
            ('text_scale',     0.1,                None),
            ('perPackage',     False,              None),
            ('updateText',     True,               None),
            ('finished',       None,               None),
            )
        self.defineoptions(kw, optiondefs)
        DirectWaitBar.__init__(self, parent, **kw)
        self.initialiseoptions(DWBPackageInstaller)
        self.updateBarStyle()

        # Hidden by default until the download begins.
        self.hide()

    def cleanup(self):
        PackageInstaller.cleanup(self)
        DirectWaitBar.destroy(self)

    def destroy(self):
        PackageInstaller.cleanup(self)
        DirectWaitBar.destroy(self)

    def packageStarted(self, package):
        """ This callback is made for each package between
        downloadStarted() and downloadFinished() to indicate the start
        of a new package. """
        if self['updateText']:
            self['text'] = 'Installing %s' % (package.getFormattedName())
        self.show()

    def packageProgress(self, package, progress):
        """ This callback is made repeatedly between packageStarted()
        and packageFinished() to update the current progress on the
        indicated package only.  The progress value ranges from 0
        (beginning) to 1 (complete). """

        if self['perPackage']:
            self['value'] = progress * self['range']

    def downloadProgress(self, overallProgress):
        """ This callback is made repeatedly between downloadStarted()
        and downloadFinished() to update the current progress through
        all packages.  The progress value ranges from 0 (beginning) to
        1 (complete). """

        if not self['perPackage']:
            self['value'] = overallProgress * self['range']

    def downloadFinished(self, success):
        """ This callback is made when all of the packages have been
        downloaded and installed (or there has been some failure).  If
        all packages where successfully installed, success is True.

        If there were no packages that required downloading, this
        callback will be made immediately, *without* a corresponding
        call to downloadStarted(). """

        self.hide()

        if self['finished']:
            self['finished'](success)
