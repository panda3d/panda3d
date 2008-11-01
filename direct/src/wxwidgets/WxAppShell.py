"""
WxAppShell provides a GUI application framework using wxPython.
This is an wxPython version of AppShell.py
"""
import wx, sys

class WxAppShell(wx.Frame):
    appversion      = '1.0'
    appname         = 'Generic Application Frame'
    copyright       = ('Copyright 2008 Walt Disney Internet Group.' +
                       '\nAll Rights Reserved.')
    contactname     = 'Gyedo Jeon'
    contactemail    = 'Gyedo.Jeon@disney.com'

    frameWidth      = 450
    frameHeight     = 320
    padx            = 5
    pady            = 5
    usecommandarea  = 0
    usestatusarea   = 0
    balloonState    = 'none'
    panelCount      = 0

    def __init__(self, *args, **kw):
        # Initialize the base class
        if not kw.get(''):
            kw['title'] = self.appname
        if not kw.get('size'):
            kw['size'] = wx.Size(self.frameWidth, self.frameHeight)
        wx.Frame.__init__(self, None, -1, *args, **kw)

        # Initialize the application
        self.appInit()

        self.__createInterface()
        self.Show()
        
    def __createInterface(self):
        self.__createMenuBar()
        self.__createAboutBox()
        # Add binding for panel cleanup code
        self.Bind(wx.EVT_CLOSE, self.quit)
        #
        # Create the parts of the interface
        # which can be modified by subclasses
        #
        self.createMenuBar()
        self.createInterface()

    def __createMenuBar(self):
        self.menuBar = wx.MenuBar()
        self.SetMenuBar(self.menuBar)

    def __createAboutBox(self):
        self.about = wx.MessageDialog(None,
                                      self.appname + "\n\n" +
                                      'Version %s'%self.appversion + "\n\n" +
                                      self.copyright + "\n\n" +
                                      'For more information, contact:\n%s\nEmail: %s' %\
                                      (self.contactname, self.contactemail),
                                      "About %s"%self.appname, wx.OK | wx.ICON_INFORMATION)

    def showAbout(self, event):
        # Create the dialog to display about and contact information.
        self.about.ShowModal()

    def quit(self, event):
        self.onDestroy(event)

        # to close Panda
        try:
            base
        except NameError:
            sys.exit()

        base.userExit()

    ### USER METHODS ###
    # To be overridden
    def appInit(self):
        # Called before interface is created (should be overridden).
        pass

    def createInterface(self):
        # Override this method to create the interface for the app.
        pass

    def onDestroy(self, event):
        # Override this method with actions to be performed on panel shutdown
        pass

    def createMenuBar(self):
        # Creates default menus.
        # Override if you don't want to use default menus
        self.menuFile = wx.Menu()
        self.menuBar.Append(self.menuFile, "&File")
        
        self.menuHelp = wx.Menu()
        self.menuBar.Append(self.menuHelp, "&Help")

        menuItem = self.menuFile.Append(wx.ID_EXIT, "&Quit")
        self.Bind(wx.EVT_MENU, self.quit, menuItem)

        menuItem = self.menuHelp.Append(wx.ID_ABOUT, "&About...")
        self.Bind(wx.EVT_MENU, self.showAbout, menuItem)

