###############################################################################
# Name: ed_toolbar.py                                                         #
# Purpose: Editra's Toolbar                                                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module creates Editra's toolbar. This toolbar is very simple and only adds
automatic icon theming to whats already available in the base toolbar class.

@summary: Editra's ToolBar class

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_toolbar.py 66745 2011-01-24 20:42:39Z CJP $"
__revision__ = "$Revision: 66745 $"

#--------------------------------------------------------------------------#
# Dependencies
import wx
import ed_glob
import ed_msg
from profiler import Profile_Get

_ = wx.GetTranslation
#--------------------------------------------------------------------------#
# Global Variables
TOOL_ID = [ ed_glob.ID_NEW, ed_glob.ID_OPEN, ed_glob.ID_SAVE, ed_glob.ID_PRINT,
            ed_glob.ID_UNDO, ed_glob.ID_REDO, ed_glob.ID_COPY, ed_glob.ID_CUT,
            ed_glob.ID_PASTE, ed_glob.ID_FIND, ed_glob.ID_FIND_REPLACE ]

#--------------------------------------------------------------------------#

class EdToolBar(wx.ToolBar):
    """Toolbar wrapper class
    @todo: make it more dynamic/configurable

    """
    def __init__(self, parent):
        """Initializes the toolbar
        @param parent: parent window of this toolbar

        """
        sstyle = wx.TB_HORIZONTAL | wx.NO_BORDER
        if wx.Platform == '__WXGTK__':
            sstyle = sstyle | wx.TB_DOCKABLE
        super(EdToolBar, self).__init__(parent, style=sstyle)

        # Attributes
        self._theme = Profile_Get('ICONS')
        self.SetToolBitmapSize(Profile_Get('ICON_SZ', 'size_tuple'))
        self._PopulateTools()

        # Event Handlers
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy, self)

        # Message Handlers
        ed_msg.Subscribe(self.OnThemeChange, ed_msg.EDMSG_THEME_CHANGED)

        self.Realize()

    def OnDestroy(self, evt):
        if evt.GetId() == self.GetId():
            ed_msg.Unsubscribe(self.OnThemeChange)
        evt.Skip()

    #---- End Init ----#

    #---- Function Definitions----#
    def _PopulateTools(self):
        """Sets the tools in the toolbar
        @postcondition: all default tools are added to toolbar

        """
        # Place Icons in toolbar
        self.AddSimpleTool(ed_glob.ID_NEW, _("New"), _("Start a New File"))
        self.AddSimpleTool(ed_glob.ID_OPEN, _("Open"), _("Open"))
        self.AddSimpleTool(ed_glob.ID_SAVE, _("Save"), _("Save Current File"))
        self.AddSimpleTool(ed_glob.ID_PRINT, _("Print"),
                           _("Print Current File"))
        self.AddSeparator()
        self.AddSimpleTool(ed_glob.ID_UNDO, _("Undo"), _("Undo Last Action"))
        self.AddSimpleTool(ed_glob.ID_REDO, _("Redo"), _("Redo Last Undo"))
        self.AddSeparator()
        self.AddSimpleTool(ed_glob.ID_CUT, _("Cut"),
                           _("Cut Selected Text from File"))
        self.AddSimpleTool(ed_glob.ID_COPY, _("Copy"),
                           _("Copy Selected Text to Clipboard"))
        self.AddSimpleTool(ed_glob.ID_PASTE, _("Paste"),
                           _("Paste Text from Clipboard to File"))
        self.AddSeparator()
        self.AddSimpleTool(ed_glob.ID_FIND, _("Find"), _("Find Text"))
        self.AddSimpleTool(ed_glob.ID_FIND_REPLACE, _("Find/Replace"),
                           _("Find and Replace Text"))
        self.AddSeparator()

    def AddSimpleTool(self, tool_id, lbl, helpstr):
        """Overides the default function to allow for easier tool
        generation/placement by automatically getting an appropriate icon from
        the art provider.
        @param tool_id: Id of tool to add
        @param lbl: tool label
        @param helpstr: tool help string

        """
        if self.GetToolBitmapSize() == (16, 16):
            client = wx.ART_MENU
        else:
            client = wx.ART_TOOLBAR
        tool_bmp = wx.ArtProvider.GetBitmap(str(tool_id), client)
        wx.ToolBar.AddSimpleTool(self, tool_id, tool_bmp, _(lbl), _(helpstr))

    def GetToolTheme(self):
        """Returns the name of the current toolsets theme
        @return: name of icon theme used by this toolbar

        """
        return self._theme

    def OnThemeChange(self, msg):
        """Update the icons when the icon theme has changed
        @param msg: Message object

        """
        self.ReInit()

    def ReInit(self):
        """Re-Initializes the tools in the toolbar
        @postcondition: all tool icons are changed to match current theme

        """
        self._theme = Profile_Get('ICONS')
        csize = self.GetToolBitmapSize()
        nsize = Profile_Get('ICON_SZ', 'size_tuple')
        if nsize != csize:
            # Size changed must recreate toolbar
            wx.CallAfter(self.GetParent().SetupToolBar)
            return

        # Change Bitmaps
        if self.GetToolBitmapSize() == (16, 16):
            client = wx.ART_MENU
        else:
            client = wx.ART_TOOLBAR

        for tool_id in TOOL_ID:
            bmp = wx.ArtProvider.GetBitmap(str(tool_id), client)
            self.SetToolNormalBitmap(tool_id, bmp)
