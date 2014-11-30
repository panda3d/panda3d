###############################################################################
# Name: ed_mpane.py                                                           #
# Purpose: Main panel containing notebook and command bar.                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module provides the L{MainPanel} component. That contains the editors main
notebook and command bar. 

@summary: Main Panel

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_mpane.py 67423 2011-04-09 22:43:57Z CJP $"
__revision__ = "$Revision: 67423 $"

#-----------------------------------------------------------------------------#
# Imports
import wx

# Editra Libraries
import ed_glob
import ed_pages
import ed_cmdbar
import eclib

#-----------------------------------------------------------------------------#

class MainPanel(eclib.ControlBox):
    """Main panel view
    @todo: Add interface for registering additional commandbars.

    """
    def __init__(self, parent):
        """Initialize the panel"""
        super(MainPanel, self).__init__(parent)

        # Attributes
        self.nb = ed_pages.EdPages(self)
        self._search = None
        self._line = None
        self._cmd = None

        # Layout
        self.SetWindow(self.nb)

        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEB)

    def OnEB(self, evt):
        """Empty method to fix notebook flashing issue on MSW"""
        pass

    def GetNotebook(self):
        """Get the main notebook control
        @return: EdPages instance

        """
        return self.nb

    def HideCommandBar(self):
        """Hide the command bar"""
        self.GetControlBar(wx.BOTTOM).Hide()
        self.Layout()

    def InitCommandBar(self):
        """Initialize the commandbar"""
        if self._search is None:
            self._search = ed_cmdbar.SearchBar(self)
            self.SetControlBar(self._search, wx.BOTTOM)

    def ShowCommandControl(self, ctrlid):
        """Change the mode of the commandbar
        @param ctrlid: CommandBar control id

        """
        cur_bar = None
        if ctrlid == ed_glob.ID_QUICK_FIND:
            cur_bar = self.ReplaceControlBar(self._search, wx.BOTTOM)
        elif ctrlid == ed_glob.ID_GOTO_LINE:
            # Lazy init
            if self._line is None:
                self._line = ed_cmdbar.GotoLineBar(self)
            cur_bar = self.ReplaceControlBar(self._line, wx.BOTTOM)
        elif ctrlid == ed_glob.ID_COMMAND :
            # Lazy init
            if self._cmd is None:
                self._cmd = ed_cmdbar.CommandEntryBar(self)
            cur_bar = self.ReplaceControlBar(self._cmd, wx.BOTTOM)
        else:
            return

        if cur_bar is not None:
            cur_bar.Hide()

        cbar = self.GetControlBar(wx.BOTTOM)
        if cbar is not None:
            cbar.Show()
            cbar.Layout()
            cbar.SetFocus()

        self.Layout()
