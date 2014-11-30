#!/usr/bin/env python
"""PySlicesShell is a python shell application."""

# The next two lines, and the other code below that makes use of
# ``__main__`` and ``original``, serve the purpose of cleaning up the
# main namespace to look as much as possible like the regular Python
# shell environment.
import __main__
original = __main__.__dict__.keys()

__author__ = "Patrick K. O'Brien <pobrien@orbtech.com>"
__cvsid__ = "$Id: PySlicesShell.py 41078 2006-09-09 00:38:53Z RD $"
__revision__ = "$Revision: 41078 $"[11:-2]

import wx
import os

class App(wx.App):
    """PySlicesShell standalone application."""

    def __init__(self, filename=None):
        self.filename = filename
        import wx
        wx.App.__init__(self, redirect=False)

    def OnInit(self):
        import os
        import wx
        from wx import py

        self.SetAppName("pysliceshell")
        confDir = wx.StandardPaths.Get().GetUserDataDir()
        if not os.path.exists(confDir):
            os.mkdir(confDir)
        fileName = os.path.join(confDir, 'config')
        self.config = wx.FileConfig(localFilename=fileName)
        self.config.SetRecordDefaults(True)

        self.frame = py.sliceshell.SlicesShellFrame(config=self.config,
                                                    dataDir=confDir,
                                                    filename=self.filename)
        self.frame.Show()
        self.SetTopWindow(self.frame)
        return True

'''
The main() function needs to handle being imported, such as with the
pyshell script that wxPython installs:

    #!/usr/bin/env python

    from wx.py.PySlicesShell import main
    main()
'''

def main(filename=None):
    """The main function for the PySlicesShell program."""
    # Cleanup the main namespace, leaving the App class.
    import sys
    if not filename and len(sys.argv) > 1:
        filename = sys.argv[1]
    if filename:
        filename = os.path.realpath(filename)
    
    import __main__
    md = __main__.__dict__
    keepers = original
    keepers.append('App')
    keepers.append('filename')
    for key in md.keys():
        if key not in keepers:
            del md[key]
    # Create an application instance.
    app = App(filename=filename)
    # Cleanup the main namespace some more.
    if md.has_key('App') and md['App'] is App:
        del md['App']
    if md.has_key('filename') and md['filename'] is filename:
        del md['filename']
    if md.has_key('__main__') and md['__main__'] is __main__:
        del md['__main__']
    # Mimic the contents of the standard Python shell's sys.path.
    import sys
    if sys.path[0]:
        sys.path[0] = ''
    # Add the application object to the sys module's namespace.
    # This allows a shell user to do:
    # >>> import sys
    # >>> sys.app.whatever
    sys.app = app
    del sys
    # Start the wxPython event loop.
    app.MainLoop()

if __name__ == '__main__':
    main()
