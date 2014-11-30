###############################################################################
# Name: efilehist.py                                                          #
# Purpose: Enhanced FileHistory                                               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2011 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model: EFileHistory

Enhanced File History - Provides more consistent behavior than wxFileHistory

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: efilehist.py 67571 2011-04-22 01:10:57Z CJP $"
__revision__ = "$Revision: 67571 $"

__all__ = ['EFileHistory',]

#-----------------------------------------------------------------------------#
# Imports
import os
import wx

# Local Imports
import txtutil

#-----------------------------------------------------------------------------#

class EFileHistory(object):
    """FileHistory Menu Manager"""
    def __init__(self, maxFile=9):
        assert maxFile <= 9, "supports at most 9 files"
        super(EFileHistory, self).__init__()

        # Attributes
        self._history = list()
        self._maxFiles = maxFile
        self._menu = None

    def _UpdateMenu(self):
        """Update the filehistory menu"""
        menu = self.Menu # optimization
        assert menu is not None
        for item in menu.GetMenuItems():
            menu.RemoveItem(item)
        for index, histfile in enumerate(self.History):
            menuid = wx.ID_FILE1 + index
            menu.Append(menuid, histfile)

    Count = property(lambda self: self.GetCount())
    History = property(lambda self: self._history,
                       lambda self, hist: self.SetHistory(hist))
    MaxFiles = property(lambda self: self._maxFiles)
    Menu = property(lambda self: self._menu,
                    lambda self, menu: self.UseMenu(menu))

    def AddFileToHistory(self, fname):
        """Add a file to the history
        @param fname: unicode

        """
        assert txtutil.IsUnicode(fname)
        assert self.Menu is not None
        # Shuffle to top of history if already in there
        if fname in self.History:
            self.History.remove(fname)
        self.History.insert(0, fname)
        # Maintain set length
        if self.Count > self.MaxFiles:
            self._history.pop()
        # Update menu object for new history list
        self._UpdateMenu()

    def GetCount(self):
        """Get the number of files in the history
        @return: int

        """
        return len(self._history)

    def GetHistoryFile(self, index):
        """Get the history file at the given index
        @param index: int
        @return: Unicode

        """
        assert self.MaxFiles > index, "Index out of range"
        return self.History[index] 

    def RemoveFileFromHistory(self, index):
        """Remove a file from the history"""
        assert self.MaxFiles > index, "Index out of range"
        self.History.pop(index)
        self._UpdateMenu()

    def SetHistory(self, hist):
        """Set the file history from a list
        @param hist: list of unicode

        """
        # Ensure list is unique
        hist = list(set(hist))
        assert len(hist) <= self.MaxFiles
        self._history = hist
        self._UpdateMenu()

    def UseMenu(self, menu):
        """Set the menu for the file history to use
        @param menu: wx.Menu

        """
        assert isinstance(menu, wx.Menu)
        if self.Menu is not None:
            self._menu.Destroy()
        self._menu = menu
        self._UpdateMenu()
