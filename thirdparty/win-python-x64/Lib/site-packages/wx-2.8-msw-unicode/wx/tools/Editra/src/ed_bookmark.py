###############################################################################
# Name: Cody Precord                                                          #
# Purpose:                                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2010 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Bookmark manager

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_bookmark.py 67489 2011-04-14 15:39:14Z CJP $"
__revision__ = "$Revision: 67489 $"

#--------------------------------------------------------------------------#
# Imports
import os
import re
import wx

# Editra Libraries
import ed_msg
import iface
import plugin
from profiler import Profile_Get, Profile_Set
import ed_glob
import util
import eclib
import ebmlib
import ed_basewin
from ed_marker import Bookmark

#-----------------------------------------------------------------------------#
# Globals
_ = wx.GetTranslation

#-----------------------------------------------------------------------------#

# Interface Implementation
class EdBookmarks(plugin.Plugin):
    """Shelf interface implementation for the bookmark manager"""
    plugin.Implements(iface.ShelfI)

    __name__ = u'Bookmarks'

    @staticmethod
    def AllowMultiple():
        """EdBookmark only allows one instance"""
        return False

    @staticmethod
    def CreateItem(parent):
        """Returns a bookmark panel"""
        return BookmarkWindow(parent)

    def GetBitmap(self):
        """Get the log viewers tab icon
        @return: wx.Bitmap

        """
        bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_ADD_BM), wx.ART_MENU)
        return bmp

    @staticmethod
    def GetId():
        """Plugin menu identifier ID"""
        return ed_glob.ID_BOOKMARK_MGR

    @staticmethod
    def GetMenuEntry(menu):
        """Get the menu entry for the bookmark viewer
        @param menu: the menu items parent menu

        """
        item = wx.MenuItem(menu, ed_glob.ID_BOOKMARK_MGR,
                           _("Bookmarks"),
                           _("View all bookmarks"))
        bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_ADD_BM), wx.ART_MENU)
        item.SetBitmap(bmp)
        return item

    def GetName(self):
        """Return the name of this control"""
        return self.__name__

    @staticmethod
    def IsStockable():
        """EdBookmark can be saved in the shelf preference stack"""
        return True

    # Bookmark storage
    _marks = list()
    @classmethod
    def OnStoreBM(cls, msg):
        data = msg.GetData()
        buf = data.get('stc')
        line = data.get('line')
        mark = Bookmark()
        mark.Filename = buf.GetFileName()
        mark.Line = line
        if data.get('added', False):
            if mark not in cls._marks:
                # Store the stc bookmark handle
                mark.Handle = data.get('handle', None)
                # Store an alias for the bookmark
                name = u""
                cline = buf.GetCurrentLine()
                if line == cline:
                    name = buf.GetSelectedText()
                if not name:
                    name = buf.GetLine(line)
                mark.Name = name.strip()
                cls._marks.append(mark)
        else:
            if mark in cls._marks:
                idx = cls._marks.index(mark)
                cls._marks.pop(idx)

    @classmethod
    def GetMarks(cls):
        return cls._marks

ed_msg.Subscribe(EdBookmarks.OnStoreBM, ed_msg.EDMSG_UI_STC_BOOKMARK)

#-----------------------------------------------------------------------------#

class BookmarkWindow(ed_basewin.EdBaseCtrlBox):
    """Shelf window for managing bookmarks"""
    def __init__(self, parent):
        super(BookmarkWindow, self).__init__(parent)

        # Attributes
        self._list = BookmarkList(self)

        #Setup
        self.SetWindow(self._list)
        ctrlbar = self.CreateControlBar(wx.TOP)
        ctrlbar.AddStretchSpacer()
        self._delbtn = self.AddPlateButton(_("Delete"), ed_glob.ID_DELETE)
        self._delbtn.SetToolTipString(_("Delete Bookmark"))

        # Message Handlers
        ed_msg.Subscribe(self.OnBookmark, ed_msg.EDMSG_UI_STC_BOOKMARK)

        # Event Handlers
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy, self)
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.OnItemActivate, self._list)
        self.Bind(wx.EVT_BUTTON, self.OnDelBm, self._delbtn)
        # BUG in wxAUI UpdateUI events not processed when docked
        # TODO: renable when switch to agw aui
#        self.Bind(wx.EVT_UPDATE_UI,
#                  lambda evt: evt.Enable(bool(len(self._list.GetSelections()))),
#                  self._delbtn)

    def OnDestroy(self, evt):
        """Unsubscribe message handlers on delete"""
        if evt.GetId() == self.GetId():
            ed_msg.Unsubscribe(self.OnBookmark)
        evt.Skip()

    def OnBookmark(self, msg):
        """Bookmark added or removed callback"""
        # Update on next iteration to ensure that handler
        # in the singleton data store have been updated.
        wx.CallAfter(self.DoUpdateListCtrl)

    def OnDelBm(self, evt):
        """Remove the selected bookmark(s) from the list and the buffer"""
        items = self._list.GetSelections()
        if len(items):
            items.reverse()
            marks = EdBookmarks.GetMarks()
            for item in items:
                if item < len(marks):
                    mark = marks.pop(item)
                    app = wx.GetApp()
                    mw = app.GetActiveWindow()
                    if mw:
                        nb = mw.GetNotebook()
                        buf = nb.FindBuffer(mark.Filename)
                        if buf:
                            buf.MarkerDeleteHandle(mark.Handle)
            self.DoUpdateListCtrl()

    def DoUpdateListCtrl(self):
        """Update the listctrl for changes in the cache"""
        nMarks = len(EdBookmarks.GetMarks())
        self._list.SetItemCount(nMarks)
        # Refresh everything
        # XXX: if optimization is needed only refresh visible items
        self._list.RefreshItems(0, nMarks)
        self._list.Refresh()

    def OnItemActivate(self, evt):
        """Handle double clicks on items to navigate to the
        selected bookmark.

        """
        index = evt.m_itemIndex
        marks = EdBookmarks.GetMarks()
        if index < len(marks):
            mark = marks[index]
            self.GotoBookmark(mark)

    def GotoBookmark(self, mark):
        """Goto the bookmark in the editor
        @param mark: BookMark

        """
        app = wx.GetApp()
        mw = app.GetActiveWindow()
        if mw:
            nb = mw.GetNotebook()
            buf = nb.FindBuffer(mark.Filename)
            use_handle = True
            if not buf:
                nb.OpenPage(ebmlib.GetPathName(mark.Filename),
                            ebmlib.GetFileName(mark.Filename))
                buf = nb.GetCurrentPage()
                use_handle = False # Handle is invalid so use line number

            if buf:
                # Ensure the tab is the current one
                nb.GotoPage(mark.Filename)
                # Jump to the bookmark line
                if use_handle:
                    lnum = buf.MarkerLineFromHandle(mark.Handle)
                else:
                    lnum = mark.Line
                buf.GotoLine(lnum)
        else:
            util.Log("[ed_bookmark][err] Failed to locate mainwindow")

#-----------------------------------------------------------------------------#

class BookmarkList(eclib.EBaseListCtrl):
    """ListCtrl for displaying the bookmarks in"""
    BOOKMARK = 0
    FILE_NAME = 1
    LINE_NUM = 2
    def __init__(self, parent):
        super(BookmarkList, self).__init__(parent,
                                           style=wx.LC_REPORT|\
                                                 wx.LC_EDIT_LABELS|\
                                                 wx.LC_VIRTUAL)

        # Setup
        self._il = wx.ImageList(16,16)
        self._idx = self._il.Add(Bookmark().Bitmap)
        self.SetImageList(self._il, wx.IMAGE_LIST_SMALL)
        self.InsertColumn(BookmarkList.BOOKMARK, _("Bookmark"))
        self.InsertColumn(BookmarkList.FILE_NAME, _("File Location"))
        self.InsertColumn(BookmarkList.LINE_NUM, _("Line Number"))
        self.setResizeColumn(BookmarkList.FILE_NAME+1) #NOTE: +1 bug in mixin
        self.SetItemCount(len(EdBookmarks.GetMarks()))

    def OnGetItemImage(self, item):
        return 0

    def OnGetItemText(self, item, column):
        """Override for virtual control"""
        marks = EdBookmarks.GetMarks()
        val = u""
        if item < len(marks):
            mark = marks[item]
            if column == BookmarkList.BOOKMARK:
                val = mark.Name
                if not val:
                    val = _("Bookmark%d") % item
            elif column == BookmarkList.FILE_NAME:
                val = mark.Filename
            elif column == BookmarkList.LINE_NUM:
                val = unicode(mark.Line + 1)
        return val
