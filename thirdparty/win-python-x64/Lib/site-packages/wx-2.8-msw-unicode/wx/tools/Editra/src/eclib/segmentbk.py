###############################################################################
# Name: segmentbk.py                                                          #
# Purpose: SegmentBook Implementation                                         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Control Library: SegmentBook

A L{SegmentBook} is a Toolbook like class derived from a ControlBox and
SegmentBar. Allows for a multi page control with Icons w/ optional text as
page buttons.

+-----------------------------------------+
| @^@    *>                               |
| <->   /|D                               |
| frog  bird                              |
+-----------------------------------------+
|                                         |
| Main Page Area                          |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
|                                         |
+-----------------------------------------+

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: segmentbk.py 67323 2011-03-27 19:28:00Z CJP $"
__revision__ = "$Revision: 67323 $"

__all__ = ['SegmentBook', 'SegmentBookEvent', 'SEGBOOK_STYLE_DEFAULT',
           'SEGBOOK_STYLE_NO_DIVIDERS', 'SEGBOOK_STYLE_LABELS',
           'SEGBOOK_STYLE_LEFT', 'SEGBOOK_STYLE_RIGHT',
           'SEGBOOK_STYLE_TOP', 'SEGBOOK_STYLE_BOTTOM',
           'SEGBOOK_NAME_STR',
           'edEVT_SB_PAGE_CHANGING', 'EVT_SB_PAGE_CHANGING',
           'edEVT_SB_PAGE_CHANGED', 'EVT_SB_PAGE_CHANGED',
           'edEVT_SB_PAGE_CLOSED', 'EVT_SB_PAGE_CLOSED',
           'edEVT_SB_PAGE_CONTEXT_MENU', 'EVT_SB_PAGE_CONTEXT_MENU',
           'edEVT_SB_PAGE_CLOSING', 'EVT_SB_PAGE_CLOSING' ]

#-----------------------------------------------------------------------------#
# Imports
import wx

# Local Imports
import ctrlbox

#-----------------------------------------------------------------------------#
# Events
edEVT_SB_PAGE_CHANGING = wx.NewEventType()
EVT_SB_PAGE_CHANGING = wx.PyEventBinder(edEVT_SB_PAGE_CHANGING, 1)

edEVT_SB_PAGE_CHANGED = wx.NewEventType()
EVT_SB_PAGE_CHANGED = wx.PyEventBinder(edEVT_SB_PAGE_CHANGED, 1)

edEVT_SB_PAGE_CLOSING = wx.NewEventType()
EVT_SB_PAGE_CLOSING = wx.PyEventBinder(edEVT_SB_PAGE_CLOSING, 1)

edEVT_SB_PAGE_CLOSED = wx.NewEventType()
EVT_SB_PAGE_CLOSED = wx.PyEventBinder(edEVT_SB_PAGE_CLOSED, 1)

edEVT_SB_PAGE_CONTEXT_MENU = wx.NewEventType()
EVT_SB_PAGE_CONTEXT_MENU = wx.PyEventBinder(edEVT_SB_PAGE_CONTEXT_MENU, 1)
class SegmentBookEvent(wx.NotebookEvent):
    """SegmentBook event"""
    def __init__(self, etype=wx.wxEVT_NULL, id=-1, sel=-1, old_sel=-1):
        super(SegmentBookEvent, self).__init__(etype, id, sel, old_sel)

#-----------------------------------------------------------------------------#
# Global constants

# Styles
SEGBOOK_STYLE_NO_DIVIDERS = 1   # Don't put dividers between segments
SEGBOOK_STYLE_LABELS      = 2   # Use labels below the icons
SEGBOOK_STYLE_TOP         = 4   # Segments at top
SEGBOOK_STYLE_BOTTOM      = 8   # Segments at top
SEGBOOK_STYLE_LEFT        = 16  # Segments at top
SEGBOOK_STYLE_RIGHT       = 32  # Segments at top
SEGBOOK_STYLE_DEFAULT     = SEGBOOK_STYLE_TOP   # Default Style

# Misc
SEGBOOK_NAME_STR = u"EditraSegmentBook"

#-----------------------------------------------------------------------------#

class SegmentBook(ctrlbox.ControlBox):
    """Notebook Class"""
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=SEGBOOK_STYLE_DEFAULT,
                 name=SEGBOOK_NAME_STR):
        """Initialize the SegmentBook"""
        super(SegmentBook, self).__init__(parent, id, pos, size,
                                          wx.TAB_TRAVERSAL|wx.NO_BORDER, name)

        # Attributes
        self._pages = list()
        self._imglist = None
        self._use_pylist = False
        self._style = style

        # Setup
        bstyle = ctrlbox.CTRLBAR_STYLE_BORDER_BOTTOM

        # Disable gradient on GTK due to coloring issues and having
        # to deal with various themes.
        if wx.Platform != '__WXGTK__':
            bstyle |= ctrlbox.CTRLBAR_STYLE_GRADIENT

        if style & SEGBOOK_STYLE_NO_DIVIDERS:
            bstyle |= ctrlbox.CTRLBAR_STYLE_NO_DIVIDERS
        if style & SEGBOOK_STYLE_LABELS:
            bstyle |= ctrlbox.CTRLBAR_STYLE_LABELS
        if style & SEGBOOK_STYLE_LEFT or style & SEGBOOK_STYLE_RIGHT:
            bstyle |= ctrlbox.CTRLBAR_STYLE_VERTICAL

        self._segbar = ctrlbox.SegmentBar(self, style=bstyle)
        self.SetControlBar(self._segbar, self._GetSegBarPos())

        # Event Handlers
        self.Bind(ctrlbox.EVT_SEGMENT_SELECTED, self._OnSegmentSel)
        self._segbar.Bind(wx.EVT_RIGHT_DOWN, self._OnRightDown)
        self._segbar.Bind(ctrlbox.EVT_SEGMENT_CLOSE, self._OnSegClose)

    def _GetSegBarPos(self):
        pos = wx.TOP
        if self._style & SEGBOOK_STYLE_LEFT:
            pos = wx.LEFT
        elif self._style & SEGBOOK_STYLE_RIGHT:
            pos = wx.RIGHT
        elif self._style & SEGBOOK_STYLE_BOTTOM:
            pos = wx.BOTTOM
        return pos

    def _DoPageChange(self, psel, csel):
        """Change the page and post events
        @param psel: previous selection (int)
        @param csel: current selection (int)

        """
        # Post page changing event
        event = SegmentBookEvent(edEVT_SB_PAGE_CHANGING,
                                 self.GetId(), csel, psel)
        event.SetEventObject(self)
        handler = self.GetEventHandler()
        if not handler.ProcessEvent(event) or event.IsAllowed():
            # Do the actual page change
            self.Freeze()
            self.ChangePage(csel)
            self.Thaw()

            # Post page changed event
            event.SetEventType(edEVT_SB_PAGE_CHANGED)
            handler.ProcessEvent(event)
            changed = True
        else:
            # Reset the segment selection
            self._segbar.SetSelection(max(psel, 0))
            changed = False
        return changed

    def _OnRightDown(self, evt):
        """Handle right click events"""
        pos = evt.GetPosition()
        where, index = self._segbar.HitTest(pos)
        print where, index
        if where in (ctrlbox.SEGMENT_HT_SEG, ctrlbox.SEGMENT_HT_X_BTN):
            if where == ctrlbox.SEGMENT_HT_SEG:
                self._segbar.SetSelection(index)
                changed = self._DoPageChange(self.GetSelection(), index)
                if changed:
                    # Send Context Menu Event
                    event = SegmentBookEvent(edEVT_SB_PAGE_CONTEXT_MENU,
                                             self.GetId())
                    event.SetSelection(index)
                    event.SetOldSelection(index)
                    event.SetEventObject(self)
                    self.GetEventHandler().ProcessEvent(event)
            else:
                # TODO: Handle other right clicks
                pass

        evt.Skip()

    def _OnSegClose(self, evt):
        """Handle clicks on segment close buttons"""
        index = evt.GetPreviousSelection()
        change = -1
        segcnt = self._segbar.GetSegmentCount() - 1
        if  index == 0 and segcnt:
            change = 1
        elif index > 0 and segcnt > 1:
            change = index - 1

        if change != -1:
            self._DoPageChange(index, change)

        self._pages[index]['page'].Destroy()
        del self._pages[index]

    def _OnSegmentSel(self, evt):
        """Change the page in the book"""
        psel = evt.GetPreviousSelection()
        csel = evt.GetCurrentSelection()
        self._DoPageChange(psel, csel)

    def AddPage(self, page, text, select=False, img_id=-1):
        """Add a page to the notebook
        @param page: wxWindow object
        @param text: Page text
        @keyword select: should the page be selected
        @keyword img_id: Image to use

        """
        page.Hide()
        self._pages.append(dict(page=page, img=img_id))
        segbar = self.GetControlBar(self._GetSegBarPos())
        if self._use_pylist:
            bmp = self._imglist[img_id]
        else:
            bmp = self._imglist.GetBitmap(img_id)
        segbar.AddSegment(wx.ID_ANY, bmp, text)
        idx = len(self._pages) - 1

        if select or idx == 0:
            segbar.SetSelection(idx)
            self._DoPageChange(segbar.GetSelection(), idx)

    def ChangePage(self, index):
        """Change the page to the given index"""
        cpage = self._pages[index]['page']
        page = self.ChangeWindow(cpage)
        if page is not None:
            page.Hide()
        cpage.Show()
        self.Layout()

    def DeleteAllPages(self):
        """Remove all pages from the control"""
        for page in reversed(range(len(self._pages))):
            self.DeletePage()

    def DeletePage(self, index):
        """Delete the page at the given index
        @param index: int

        """
        cpage = self._segbar.GetSelection()
        self._segbar.RemoveSegment(index)
        npage = self._segbar.GetSelection()
        self._DoPageChange(cpage, npage)

        self._pages[index]['page'].Destroy()
        del self._pages[index]

    def CurrentPage(self):
        """Get the currently selected page
        @return: wxWindow or None

        """
        idx = self._segbar.GetSelection()
        if idx != -1:
            return self._pages[idx]['page']
        else:
            return None

    def GetImageList(self):
        """Get the notebooks image list
        @return: wxImageList or None

        """
        return self._imglist

    def GetPage(self, index):
        """Get the page at the given index
        @param index: int

        """
        return self._pages[index]['page']

    def GetPageCount(self):
        """Get the number of pages in the book
        @return: int

        """
        return len(self._pages)

    def GetPageImage(self, index):
        """Get the image index of the current page
        @param index: page index
        @return: int

        """
        return self._pages[index]['img']

    def SetPageCloseButton(self, index):
        """Set the property of a page
        @param index: Segment index

        """
        if wx.Platform != '__WXMAC__':
            self._segbar.SetSegmentOption(index, ctrlbox.SEGBTN_OPT_CLOSEBTNR)
        else:
            self._segbar.SetSegmentOption(index, ctrlbox.SEGBTN_OPT_CLOSEBTNL)

    def GetPageText(self, index):
        """Get the text of the current page
        @param index: page index
        @return: string

        """
        return self._segbar.GetSegmentLabel(index)

    def SetSegmentCanClose(self, index, can_close=True):
        """Add a close button to the given segment
        @param index: segment index
        @keyword can_close: Enable/Disable

        """
        if not can_close:
            opt = ctrlbox.SEGBTN_OPT_NONE

        elif wx.Platform == '__WXMAC__':
            opt = ctrlbox.SEGBTN_OPT_CLOSEBTNL
        else:
            opt = ctrlbox.SEGBTN_OPT_CLOSEBTNR
        self._segbar.SetSegmentOption(index, opt)

    def GetSelection(self):
        """Get the current selection
        @return: int

        """
        return self._segbar.GetSelection()

    def GetSegmentBar(self):
        """Get the segment bar used by this control
        @return: SegmentBar

        """
        return self._segbar

    def HasMultiplePages(self):
        """Does the book have multiple pages
        @return: bool

        """
        return bool(self.GetPageCount())

    def HitTest(self, pt):
        """Find if/where the given point is in the window
        @param pt: wxPoint
        @return: where, index

        """
        where, index = (SEGBOOK_NO_WHERE, -1)
        index = self._segbar.GetIndexFromPosition(pt)
        if index != wx.NOT_FOUND:
            where = SEGBOOK_ON_SEGMENT

        # TOOD check for clicks elsewhere on bar
        return where, index

    def InsertPage(self, index, page, text, select=False, image_id=-1):
        """Insert a page a the given index
        @param index: index to insert page at
        @param page: page to add to book
        @param text: page text
        @keyword select: bool
        @keyword image_id: image list index

        """
        raise NotImplementedError

    def Refresh(self):
        """Refresh the segmentbar
        @todo: temporary HACK till rework of SegmentBar class image handling

        """
        segbar = self.GetSegmentBar()
        for page in range(self.GetPageCount()):
            idx = self.GetPageImage(page)
            bmp = self._imglist[idx]
            segbar.SetSegmentImage(page, bmp)
        segbar.Refresh()
        super(SegmentBook, self).Refresh()

    def SetImageList(self, imglist):
        """Set the notebooks image list
        @param imglist: wxImageList

        """
        self._imglist = imglist

    def SetPageImage(self, index, img_id):
        """Set the image to use on the given page
        @param index: page index
        @param img_id: image list index

        """
        page = self._pages[index]
        page['img'] = img_id
        self._segbar.SetSegmentImage(self._imglst.GetBitmap(img_id))
        self.Layout()

    def SetPageText(self, index, text):
        """Set the text to use on the given page
        @param index: page index
        @param text: string

        """
        self._segbar.SetSegmentLabel(index, text)

    def SetSelection(self, index):
        """Set the selected page
        @param index: index of page to select

        """
        csel = self._segbar.GetSelection()
        if csel != index:
            self._segbar.SetSelection(index)
            self._DoPageChange(csel, index)

    def SetUsePyImageList(self, use_pylist):
        """Set whether the control us using a regular python list for
        storing images or a wxImageList.
        @param use_pylist: bool

        """
        self._use_pylist = use_pylist
