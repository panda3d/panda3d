# --------------------------------------------------------------------------- #
# LABELBOOK And FLATIMAGEBOOK Widgets wxPython IMPLEMENTATION
#
# Original C++ Code From Eran, embedded in the FlatMenu source code
#
#
# License: wxWidgets license
#
#
# Python Code By:
#
# Andrea Gavana, @ 03 Nov 2006
# Latest Revision: 17 Jan 2011, 15.00 GMT
#
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# andrea.gavana@gmail.com
# gavana@kpo.kz
#
# Or, Obviously, To The wxPython Mailing List!!!
#
# TODO:
# LabelBook - Support IMB_SHOW_ONLY_IMAGES
# LabelBook - An option for the draw border to only draw the border 
#             between the controls and the pages so the background
#             colour can flow into the window background
#
#
#
# End Of Comments
# --------------------------------------------------------------------------- #

"""
LabelBook and FlatImageBook are a quasi-full generic and owner-drawn
implementations of `wx.Notebook`.


Description
===========

LabelBook and FlatImageBook are a quasi-full implementations of the `wx.Notebook`,
and designed to be a drop-in replacement for `wx.Notebook`. The API functions are
similar so one can expect the function to behave in the same way.
LabelBook anf FlatImageBook share their appearance with `wx.Toolbook` and
`wx.Listbook`, while having more options for custom drawings, label positioning,
mouse pointing and so on. Moreover, they retain also some visual characteristics
of the Outlook address book.

Some features:

- They are generic controls;
- Supports for left, right, top (FlatImageBook only), bottom (FlatImageBook
  only) book styles;
- Possibility to draw images only, text only or both (FlatImageBook only);
- Support for a "pin-button", that allows the user to shrink/expand the book
  tab area;
- Shadows behind tabs (LabelBook only);
- Gradient shading of the tab area (LabelBook only);
- Web-like mouse pointing on tabs style (LabelBook only);
- Many customizable colours (tab area, active tab text, tab borders, active
  tab, highlight) - LabelBook only.
  
And much more. See the demo for a quasi-complete review of all the functionalities
of LabelBook and FlatImageBook.


Supported Platforms
===================

LabelBook and FlatImageBook have been tested on the following platforms:
  * Windows (Windows XP);
  * Linux Ubuntu (Dapper 6.06)


Window Styles
=============

This class supports the following window styles:

=========================== =========== ==================================================
Window Styles               Hex Value   Description
=========================== =========== ==================================================
``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for `FlatImageBook`.
``INB_LEFT``                        0x2 Place labels on the left side. Available only for `FlatImageBook`.
``INB_RIGHT``                       0x4 Place labels on the right side.
``INB_TOP``                         0x8 Place labels above the page area.
``INB_BORDER``                     0x10 Draws a border around `LabelBook` or `FlatImageBook`.
``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for `LabelBook`.
``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for `LabelBook`.
``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for `LabelBook`.
``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for `LabelBook`.
``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for `LabelBook`.
``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
=========================== =========== ==================================================


Events Processing
=================

This class processes the following events:

=================================== ==================================================
Event Name                          Description
=================================== ==================================================
``EVT_IMAGENOTEBOOK_PAGE_CHANGED``  Notify client objects when the active page in `ImageNotebook` has changed.
``EVT_IMAGENOTEBOOK_PAGE_CHANGING`` Notify client objects when the active page in `ImageNotebook` is about to change.
``EVT_IMAGENOTEBOOK_PAGE_CLOSED``   Notify client objects when a page in `ImageNotebook` has been closed.
``EVT_IMAGENOTEBOOK_PAGE_CLOSING``  Notify client objects when a page in `ImageNotebook` is closing.
=================================== ==================================================


License And Version
===================

LabelBook and FlatImageBook are distributed under the wxPython license. 

Latest Revision: Andrea Gavana @ 17 Jan 2011, 15.00 GMT

Version 0.5.

"""

__docformat__ = "epytext"


#----------------------------------------------------------------------
# Beginning Of IMAGENOTEBOOK wxPython Code
#----------------------------------------------------------------------

import wx

from artmanager import ArtManager, DCSaver
from fmresources import *

# Check for the new method in 2.7 (not present in 2.6.3.3)
if wx.VERSION_STRING < "2.7":
    wx.Rect.Contains = lambda self, point: wx.Rect.Inside(self, point)

# FlatImageBook and LabelBook styles
INB_BOTTOM = 1
""" Place labels below the page area. Available only for `FlatImageBook`."""
INB_LEFT = 2
""" Place labels on the left side. Available only for `FlatImageBook`."""
INB_RIGHT = 4
""" Place labels on the right side. """
INB_TOP = 8
""" Place labels above the page area. """
INB_BORDER = 16
""" Draws a border around `LabelBook` or `FlatImageBook`. """
INB_SHOW_ONLY_TEXT = 32
""" Shows only text labels and no images. Available only for `LabelBook`."""
INB_SHOW_ONLY_IMAGES = 64
""" Shows only tab images and no label texts. Available only for `LabelBook`."""
INB_FIT_BUTTON = 128
""" Displays a pin button to show/hide the book control. """
INB_DRAW_SHADOW = 256
""" Draw shadows below the book tabs. Available only for `LabelBook`."""
INB_USE_PIN_BUTTON = 512
""" Displays a pin button to show/hide the book control. """
INB_GRADIENT_BACKGROUND = 1024
""" Draws a gradient shading on the tabs background. Available only for `LabelBook`."""
INB_WEB_HILITE = 2048
""" On mouse hovering, tabs behave like html hyperlinks. Available only for `LabelBook`."""
INB_NO_RESIZE = 4096
""" Don't allow resizing of the tab area. """
INB_FIT_LABELTEXT = 8192
""" Will fit the tab area to the longest text (or text+image if you have images) in all the tabs. """

wxEVT_IMAGENOTEBOOK_PAGE_CHANGED = wx.wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED
wxEVT_IMAGENOTEBOOK_PAGE_CHANGING = wx.wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING
wxEVT_IMAGENOTEBOOK_PAGE_CLOSING = wx.NewEventType()
wxEVT_IMAGENOTEBOOK_PAGE_CLOSED = wx.NewEventType()

#-----------------------------------#
#        ImageNotebookEvent
#-----------------------------------#

EVT_IMAGENOTEBOOK_PAGE_CHANGED = wx.EVT_NOTEBOOK_PAGE_CHANGED
""" Notify client objects when the active page in `ImageNotebook` has changed. """
EVT_IMAGENOTEBOOK_PAGE_CHANGING = wx.EVT_NOTEBOOK_PAGE_CHANGING
""" Notify client objects when the active page in `ImageNotebook` is about to change. """
EVT_IMAGENOTEBOOK_PAGE_CLOSING = wx.PyEventBinder(wxEVT_IMAGENOTEBOOK_PAGE_CLOSING, 1)
""" Notify client objects when a page in `ImageNotebook` is closing. """
EVT_IMAGENOTEBOOK_PAGE_CLOSED = wx.PyEventBinder(wxEVT_IMAGENOTEBOOK_PAGE_CLOSED, 1)
""" Notify client objects when a page in `ImageNotebook` has been closed. """


# ---------------------------------------------------------------------------- #
# Class ImageNotebookEvent
# ---------------------------------------------------------------------------- #

class ImageNotebookEvent(wx.PyCommandEvent):
    """
    This events will be sent when a ``EVT_IMAGENOTEBOOK_PAGE_CHANGED``,
    ``EVT_IMAGENOTEBOOK_PAGE_CHANGING``, ``EVT_IMAGENOTEBOOK_PAGE_CLOSING``,
    ``EVT_IMAGENOTEBOOK_PAGE_CLOSED`` is mapped in the parent.
    """

    def __init__(self, eventType, eventId=1, sel=-1, oldsel=-1):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier;
        :param `sel`: the current selection;
        :param `oldsel`: the old selection.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self._eventType = eventType
        self._sel = sel
        self._oldsel = oldsel
        self._allowed = True


    def SetSelection(self, s):
        """
        Sets the event selection.

        :param `s`: an integer specifying the new selection.
        """

        self._sel = s


    def SetOldSelection(self, s):
        """
        Sets the event old selection.

        :param `s`: an integer specifying the old selection.
        """

        self._oldsel = s


    def GetSelection(self):
        """ Returns the event selection. """

        return self._sel


    def GetOldSelection(self):
        """ Returns the old event selection. """

        return self._oldsel


    def Veto(self):
        """
        Prevents the change announced by this event from happening.

        :note: It is in general a good idea to notify the user about the reasons
         for vetoing the change because otherwise the applications behaviour (which
         just refuses to do what the user wants) might be quite surprising.
        """

        self._allowed = False


    def Allow(self):
        """
        This is the opposite of L{Veto}: it explicitly allows the event to be processed.
        For most events it is not necessary to call this method as the events are
        allowed anyhow but some are forbidden by default (this will be mentioned
        in the corresponding event description).
        """

        self._allowed = True


    def IsAllowed(self):
        """
        Returns ``True`` if the change is allowed (L{Veto} hasn't been called) or
        ``False`` otherwise (if it was).
        """

        return self._allowed


# ---------------------------------------------------------------------------- #
# Class ImageInfo
# ---------------------------------------------------------------------------- #

class ImageInfo(object):
    """
    This class holds all the information (caption, image, etc...) belonging to a
    single tab in L{LabelBook}.
    """
    def __init__(self, strCaption="", imageIndex=-1):    
        """
        Default class constructor.

        :param `strCaption`: the tab caption;
        :param `imageIndex`: the tab image index based on the assigned (set)
         `wx.ImageList` (if any).
        """
        
        self._pos = wx.Point()
        self._size = wx.Size()
        self._strCaption = strCaption
        self._ImageIndex = imageIndex
        self._captionRect = wx.Rect()


    def SetCaption(self, value):
        """
        Sets the tab caption.

        :param `value`: the new tab caption.
        """

        self._strCaption = value


    def GetCaption(self):
        """ Returns the tab caption. """

        return self._strCaption


    def SetPosition(self, value):
        """
        Sets the tab position.

        :param `value`: the new tab position, an instance of `wx.Point`.
        """

        self._pos = value


    def GetPosition(self):
        """ Returns the tab position. """

        return self._pos


    def SetSize(self, value):
        """
        Sets the tab size.

        :param `value`:  the new tab size, an instance of `wx.Size`.
        """

        self._size = value


    def GetSize(self):
        """ Returns the tab size. """

        return self._size


    def SetImageIndex(self, value):
        """
        Sets the tab image index.

        :param `value`: an index into the image list..
        """

        self._ImageIndex = value


    def GetImageIndex(self):
        """ Returns the tab image index. """

        return self._ImageIndex


    def SetTextRect(self, rect):
        """
        Sets the client rectangle available for the tab text.

        :param `rect`: the tab text client rectangle, an instance of `wx.Rect`.
        """

        self._captionRect = rect


    def GetTextRect(self):
        """ Returns the client rectangle available for the tab text. """

        return self._captionRect


# ---------------------------------------------------------------------------- #
# Class ImageContainerBase
# ---------------------------------------------------------------------------- #

class ImageContainerBase(wx.Panel):
    """
    Base class for L{FlatImageBook} image container.
    """
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="ImageContainerBase"):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.Panel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================

        :param `name`: the window name.         
        """
        
        self._nIndex = -1
        self._nImgSize = 16
        self._ImageList = None
        self._nHoeveredImgIdx = -1
        self._bCollapsed = False
        self._tabAreaSize = (-1, -1)
        self._nPinButtonStatus = INB_PIN_NONE
        self._pagesInfoVec = []
        self._pinBtnRect = wx.Rect()

        wx.Panel.__init__(self, parent, id, pos, size, style | wx.NO_BORDER | wx.NO_FULL_REPAINT_ON_RESIZE, name)


    def HasAGWFlag(self, flag):
        """
        Tests for existance of flag in the style.

        :param `flag`: a window style. This can be a combination of the following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================
        """
        
        style = self.GetParent().GetAGWWindowStyleFlag()
        res = (style & flag and [True] or [False])[0]
        return res


    def ClearFlag(self, flag):
        """
        Removes flag from the style.

        :param `flag`: a window style flag.

        :see: L{HasAGWFlag} for a list of possible window style flags.        
        """

        parent = self.GetParent()
        agwStyle = parent.GetAGWWindowStyleFlag()
        agwStyle &= ~(flag)
        parent.SetAGWWindowStyleFlag(agwStyle)


    def AssignImageList(self, imglist):
        """
        Assigns an image list to the L{ImageContainerBase}.

        :param `imglist`: an instance of `wx.ImageList`.
        """
  
        if imglist and imglist.GetImageCount() != 0:
            self._nImgSize = imglist.GetBitmap(0).GetHeight()

        self._ImageList = imglist
        parent = self.GetParent()
        agwStyle = parent.GetAGWWindowStyleFlag()
        parent.SetAGWWindowStyleFlag(agwStyle)
        

    def GetImageList(self):
        """ Return the image list for L{ImageContainerBase}. """

        return self._ImageList


    def GetImageSize(self):
        """ Returns the image size inside the L{ImageContainerBase} image list. """

        return self._nImgSize

    
    def FixTextSize(self, dc, text, maxWidth):
        """
        Fixes the text, to fit `maxWidth` value. If the text length exceeds
        `maxWidth` value this function truncates it and appends two dots at
        the end. ("Long Long Long Text" might become "Long Long...").

        :param `dc`: an instance of `wx.DC`;
        :param `text`: the text to fix/truncate;
        :param `maxWidth`: the maximum allowed width for the text, in pixels.
        """

        return ArtManager.Get().TruncateText(dc, text, maxWidth)


    def CanDoBottomStyle(self):
        """
        Allows the parent to examine the children type. Some implementation
        (such as L{LabelBook}), does not support top/bottom images, only left/right.
        """
        
        return False
    
        
    def AddPage(self, caption, selected=False, imgIdx=-1):
        """
        Adds a page to the container.

        :param `caption`: specifies the text for the new tab;
        :param `selected`: specifies whether the page should be selected;
        :param `imgIdx`: specifies the optional image index for the new tab.
        """
        
        self._pagesInfoVec.append(ImageInfo(caption, imgIdx))
        if selected or len(self._pagesInfoVec) == 1:
            self._nIndex = len(self._pagesInfoVec)-1

        self.Refresh()


    def InsertPage(self, page_idx, caption, selected=False, imgIdx=-1):
        """
        Inserts a page into the container at the specified position.

        :param `page_idx`: specifies the position for the new tab;
        :param `caption`: specifies the text for the new tab;
        :param `selected`: specifies whether the page should be selected;
        :param `imgIdx`: specifies the optional image index for the new tab.
        """
        
        self._pagesInfoVec.insert(page_idx, ImageInfo(caption, imgIdx))
        if selected or len(self._pagesInfoVec) == 1:
            self._nIndex = len(self._pagesInfoVec)-1

        self.Refresh()


    def SetPageImage(self, page, imgIdx):
        """
        Sets the image for the given page.

        :param `page`: the index of the tab;
        :param `imgIdx`: specifies the optional image index for the tab.
        """

        imgInfo = self._pagesInfoVec[page]
        imgInfo.SetImageIndex(imgIdx)


    def SetPageText(self, page, text):
        """
        Sets the tab caption for the given page.

        :param `page`: the index of the tab;
        :param `text`: the new tab caption.
        """

        imgInfo = self._pagesInfoVec[page]
        imgInfo.SetCaption(text)


    def GetPageImage(self, page):
        """
        Returns the image index for the given page.
        
        :param `page`: the index of the tab.
        """

        imgInfo = self._pagesInfoVec[page]
        return imgInfo.GetImageIndex()


    def GetPageText(self, page):
        """
        Returns the tab caption for the given page.
        
        :param `page`: the index of the tab.
        """

        imgInfo = self._pagesInfoVec[page]
        return imgInfo.GetCaption()

        
    def ClearAll(self):
        """ Deletes all the pages in the container. """

        self._pagesInfoVec = []
        self._nIndex = wx.NOT_FOUND


    def DoDeletePage(self, page):
        """
        Does the actual page deletion.

        :param `page`: the index of the tab.
        """

        # Remove the page from the vector
        book = self.GetParent()
        self._pagesInfoVec.pop(page)

        if self._nIndex >= page:
            self._nIndex = self._nIndex - 1

        # The delete page was the last first on the array,
        # but the book still has more pages, so we set the
        # active page to be the first one (0)
        if self._nIndex < 0 and len(self._pagesInfoVec) > 0:
            self._nIndex = 0

        # Refresh the tabs
        if self._nIndex >= 0:
        
            book._bForceSelection = True
            book.SetSelection(self._nIndex)
            book._bForceSelection = False
        
        if not self._pagesInfoVec:        
            # Erase the page container drawings
            dc = wx.ClientDC(self)
            dc.Clear()

            
    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{ImageContainerBase}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.Refresh() # Call on paint
        event.Skip()


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{ImageContainerBase}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass

    
    def HitTest(self, pt):
        """
        Returns the index of the tab at the specified position or ``wx.NOT_FOUND``
        if ``None``, plus the flag style of L{HitTest}.

        :param `pt`: an instance of `wx.Point`, to test for hits.

        :return: The index of the tab at the specified position plus the hit test
         flag, which can be one of the following bits:

         ====================== ======= ================================
         HitTest Flags           Value  Description
         ====================== ======= ================================
         ``IMG_OVER_IMG``             0 The mouse is over the tab icon
         ``IMG_OVER_PIN``             1 The mouse is over the pin button
         ``IMG_OVER_EW_BORDER``       2 The mouse is over the east-west book border
         ``IMG_NONE``                 3 Nowhere
         ====================== ======= ================================
         
        """
        
        style = self.GetParent().GetAGWWindowStyleFlag()
        
        if style & INB_USE_PIN_BUTTON:
            if self._pinBtnRect.Contains(pt):
                return -1, IMG_OVER_PIN        

        for i in xrange(len(self._pagesInfoVec)):
        
            if self._pagesInfoVec[i].GetPosition() == wx.Point(-1, -1):
                break
            
            # For Web Hover style, we test the TextRect
            if not self.HasAGWFlag(INB_WEB_HILITE):
                buttonRect = wx.RectPS(self._pagesInfoVec[i].GetPosition(), self._pagesInfoVec[i].GetSize())
            else:
                buttonRect = self._pagesInfoVec[i].GetTextRect()
                
            if buttonRect.Contains(pt):
                return i, IMG_OVER_IMG
            
        if self.PointOnSash(pt):
            return -1, IMG_OVER_EW_BORDER
        else:
            return -1, IMG_NONE


    def PointOnSash(self, pt):
        """
        Tests whether pt is located on the sash.

        :param `pt`: an instance of `wx.Point`, to test for hits.
        """

        # Check if we are on a the sash border
        cltRect = self.GetClientRect()
        
        if self.HasAGWFlag(INB_LEFT) or self.HasAGWFlag(INB_TOP):
            if pt.x > cltRect.x + cltRect.width - 4:
                return True
        
        else:
            if pt.x < 4:
                return True
        
        return False


    def OnMouseLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{ImageContainerBase}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        newSelection = -1
        event.Skip()

        # Support for collapse/expand
        style = self.GetParent().GetAGWWindowStyleFlag()
        if style & INB_USE_PIN_BUTTON:

            if self._pinBtnRect.Contains(event.GetPosition()):
            
                self._nPinButtonStatus = INB_PIN_PRESSED
                dc = wx.ClientDC(self)
                self.DrawPin(dc, self._pinBtnRect, not self._bCollapsed)
                return
            
        # Incase panel is collapsed, there is nothing 
        # to check 
        if self._bCollapsed:
            return

        tabIdx, where = self.HitTest(event.GetPosition())

        if where == IMG_OVER_IMG:
            self._nHoeveredImgIdx = -1        

        if tabIdx == -1:
            return
        
        self.GetParent().SetSelection(tabIdx)


    def OnMouseLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{ImageContainerBase}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        bRepaint = self._nHoeveredImgIdx != -1
        self._nHoeveredImgIdx = -1

        # Make sure the pin button status is NONE
        # incase we were in pin button style
        style = self.GetParent().GetAGWWindowStyleFlag()
        
        if style & INB_USE_PIN_BUTTON:
        
            self._nPinButtonStatus = INB_PIN_NONE
            dc = wx.ClientDC(self)
            self.DrawPin(dc, self._pinBtnRect, not self._bCollapsed)
        
        # Restore cursor
        wx.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
        
        if bRepaint:
            self.Refresh()


    def OnMouseLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{ImageContainerBase}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        style = self.GetParent().GetAGWWindowStyleFlag()
        
        if style & INB_USE_PIN_BUTTON:
        
            bIsLabelContainer = not self.CanDoBottomStyle()
            
            if self._pinBtnRect.Contains(event.GetPosition()):
            
                self._nPinButtonStatus = INB_PIN_NONE
                self._bCollapsed = not self._bCollapsed

                if self._bCollapsed:
                
                    # Save the current tab area width
                    self._tabAreaSize = self.GetSize()
                    
                    if bIsLabelContainer:
                    
                        self.SetSizeHints(20, self._tabAreaSize.y)
                    
                    else:
                    
                        if style & INB_BOTTOM or style & INB_TOP:
                            self.SetSizeHints(self._tabAreaSize.x, 20)
                        else:
                            self.SetSizeHints(20, self._tabAreaSize.y)
                    
                else:
                
                    if bIsLabelContainer:
                    
                        self.SetSizeHints(self._tabAreaSize.x, -1)
                    
                    else:
                    
                        # Restore the tab area size
                        if style & INB_BOTTOM or style & INB_TOP:
                            self.SetSizeHints(-1, self._tabAreaSize.y)
                        else:
                            self.SetSizeHints(self._tabAreaSize.x, -1)
                    
                self.GetParent().GetSizer().Layout()
                self.Refresh()
                return
            

    def OnMouseMove(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{ImageContainerBase}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        style = self.GetParent().GetAGWWindowStyleFlag()
        if style & INB_USE_PIN_BUTTON:
        
            # Check to see if we are in the pin button rect
            if not self._pinBtnRect.Contains(event.GetPosition()) and self._nPinButtonStatus == INB_PIN_PRESSED:
            
                self._nPinButtonStatus = INB_PIN_NONE
                dc = wx.ClientDC(self)
                self.DrawPin(dc, self._pinBtnRect, not self._bCollapsed)
            
        imgIdx, where = self.HitTest(event.GetPosition())
        self._nHoeveredImgIdx = imgIdx
        
        if not self._bCollapsed:
        
            if self._nHoeveredImgIdx >= 0 and self._nHoeveredImgIdx < len(self._pagesInfoVec):
            
                # Change the cursor to be Hand
                if self.HasAGWFlag(INB_WEB_HILITE) and self._nHoeveredImgIdx != self._nIndex:
                    wx.SetCursor(wx.StockCursor(wx.CURSOR_HAND))
            
            else:
            
                # Restore the cursor only if we have the Web hover style set,
                # and we are not currently hovering the sash
                if self.HasAGWFlag(INB_WEB_HILITE) and not self.PointOnSash(event.GetPosition()):
                    wx.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
            
        # Dont display hover effect when hoevering the 
        # selected label
        
        if self._nHoeveredImgIdx == self._nIndex:
            self._nHoeveredImgIdx = -1
        
        self.Refresh()


    def DrawPin(self, dc, rect, downPin):
        """
        Draw a pin button, that allows collapsing of the image panel.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the pin button client rectangle;
        :param `downPin`: ``True`` if the pin button is facing downwards, ``False``
         if it is facing leftwards.
        """

        # Set the bitmap according to the button status

        if downPin:
            pinBmp = wx.BitmapFromXPMData(pin_down_xpm)
        else:
            pinBmp = wx.BitmapFromXPMData(pin_left_xpm)

        xx = rect.x + 2
        
        if self._nPinButtonStatus in [INB_PIN_HOVER, INB_PIN_NONE]:
            
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetPen(wx.BLACK_PEN)
            dc.DrawRectangle(xx, rect.y, 16, 16)

            # Draw upper and left border with grey colour
            dc.SetPen(wx.WHITE_PEN)
            dc.DrawLine(xx, rect.y, xx + 16, rect.y)
            dc.DrawLine(xx, rect.y, xx, rect.y + 16)
            
        elif self._nPinButtonStatus == INB_PIN_PRESSED:
            
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetPen(wx.Pen(wx.NamedColour("LIGHT GREY")))
            dc.DrawRectangle(xx, rect.y, 16, 16)

            # Draw upper and left border with grey colour
            dc.SetPen(wx.BLACK_PEN)
            dc.DrawLine(xx, rect.y, xx + 16, rect.y)
            dc.DrawLine(xx, rect.y, xx, rect.y + 16)
            
        # Set the masking
        pinBmp.SetMask(wx.Mask(pinBmp, wx.WHITE))

        # Draw the new bitmap
        dc.DrawBitmap(pinBmp, xx, rect.y, True)

        # Save the pin rect
        self._pinBtnRect = rect


# ---------------------------------------------------------------------------- #
# Class ImageContainer
# ---------------------------------------------------------------------------- #

class ImageContainer(ImageContainerBase):
    """
    Base class for L{FlatImageBook} image container.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="ImageContainer"):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.Panel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================

        :param `name`: the window name.         
        """

        ImageContainerBase.__init__(self, parent, id, pos, size, style, agwStyle, name)
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnMouseLeftUp)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeaveWindow)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{ImageContainer}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        ImageContainerBase.OnSize(self, event)
        event.Skip()
        

    def OnMouseLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{ImageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """
        
        ImageContainerBase.OnMouseLeftDown(self, event)
        event.Skip()
            

    def OnMouseLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{ImageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        ImageContainerBase.OnMouseLeftUp(self, event)
        event.Skip()


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{ImageContainer}.

        :param `event`: a `wx.EraseEvent` event to be processed.
        """

        ImageContainerBase.OnEraseBackground(self, event)


    def OnMouseMove(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{ImageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        ImageContainerBase.OnMouseMove(self, event)
        event.Skip()


    def OnMouseLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{ImageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        ImageContainerBase.OnMouseLeaveWindow(self, event)
        event.Skip()

        
    def CanDoBottomStyle(self):
        """
        Allows the parent to examine the children type. Some implementation
        (such as L{LabelBook}), does not support top/bottom images, only left/right.
        """

        return True


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{ImageContainer}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.BufferedPaintDC(self)
        style = self.GetParent().GetAGWWindowStyleFlag()

        backBrush = wx.WHITE_BRUSH
        if style & INB_BORDER:
            borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DSHADOW))
        else:
            borderPen = wx.TRANSPARENT_PEN

        size = self.GetSize()

        # Background
        dc.SetBrush(backBrush)

        borderPen.SetWidth(1)
        dc.SetPen(borderPen)
        dc.DrawRectangle(0, 0, size.x, size.y)
        bUsePin = (style & INB_USE_PIN_BUTTON and [True] or [False])[0]

        if bUsePin:

            # Draw the pin button
            clientRect = self.GetClientRect()
            pinRect = wx.Rect(clientRect.GetX() + clientRect.GetWidth() - 20, 2, 20, 20)
            self.DrawPin(dc, pinRect, not self._bCollapsed)

            if self._bCollapsed:
                return

        borderPen = wx.BLACK_PEN
        borderPen.SetWidth(1)
        dc.SetPen(borderPen)
        dc.DrawLine(0, size.y, size.x, size.y)
        dc.DrawPoint(0, size.y)

        clientSize = 0
        bUseYcoord = (style & INB_RIGHT or style & INB_LEFT)

        if bUseYcoord:
            clientSize = size.GetHeight()
        else:
            clientSize = size.GetWidth()

        # We reserver 20 pixels for the 'pin' button
        
        # The drawing of the images start position. This is 
        # depenedent of the style, especially when Pin button
        # style is requested

        if bUsePin:
            if style & INB_TOP or style & INB_BOTTOM:
                pos = (style & INB_BORDER and [0] or [1])[0]
            else:
                pos = (style & INB_BORDER and [20] or [21])[0]
        else:
            pos = (style & INB_BORDER and [0] or [1])[0]

        nPadding = 4    # Pad text with 2 pixels on the left and right
        nTextPaddingLeft = 2

        count = 0
        
        for i in xrange(len(self._pagesInfoVec)):

            count = count + 1            
        
            # incase the 'fit button' style is applied, we set the rectangle width to the
            # text width plus padding
            # Incase the style IS applied, but the style is either LEFT or RIGHT
            # we ignore it
            normalFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            dc.SetFont(normalFont)

            textWidth, textHeight = dc.GetTextExtent(self._pagesInfoVec[i].GetCaption())

            # Restore font to be normal
            normalFont.SetWeight(wx.FONTWEIGHT_NORMAL)
            dc.SetFont(normalFont)

            # Default values for the surronounding rectangle 
            # around a button
            rectWidth = self._nImgSize * 2  # To avoid the recangle to 'touch' the borders
            rectHeight = self._nImgSize * 2

            # Incase the style requires non-fixed button (fit to text)
            # recalc the rectangle width
            if style & INB_FIT_BUTTON and \
               not ((style & INB_LEFT) or (style & INB_RIGHT)) and \
               not self._pagesInfoVec[i].GetCaption() == "" and \
               not (style & INB_SHOW_ONLY_IMAGES):
            
                rectWidth = ((textWidth + nPadding * 2) > rectWidth and [nPadding * 2 + textWidth] or [rectWidth])[0]

                # Make the width an even number
                if rectWidth % 2 != 0:
                    rectWidth += 1

            # Check that we have enough space to draw the button
            # If Pin button is used, consider its space as well (applicable for top/botton style)
            # since in the left/right, its size is already considered in 'pos'
            pinBtnSize = (bUsePin and [20] or [0])[0]
            
            if pos + rectWidth + pinBtnSize > clientSize:
                break

            # Calculate the button rectangle
            modRectWidth = ((style & INB_LEFT or style & INB_RIGHT) and [rectWidth - 2] or [rectWidth])[0]
            modRectHeight = ((style & INB_LEFT or style & INB_RIGHT) and [rectHeight] or [rectHeight - 2])[0]

            if bUseYcoord:
                buttonRect = wx.Rect(1, pos, modRectWidth, modRectHeight)
            else:
                buttonRect = wx.Rect(pos , 1, modRectWidth, modRectHeight)

            # Check if we need to draw a rectangle around the button
            if self._nIndex == i:
            
                # Set the colours
                penColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
                brushColour = ArtManager.Get().LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION), 75)

                dc.SetPen(wx.Pen(penColour))
                dc.SetBrush(wx.Brush(brushColour))

                # Fix the surrounding of the rect if border is set
                if style & INB_BORDER:
                
                    if style & INB_TOP or style & INB_BOTTOM:
                        buttonRect = wx.Rect(buttonRect.x + 1, buttonRect.y, buttonRect.width - 1, buttonRect.height)
                    else:
                        buttonRect = wx.Rect(buttonRect.x, buttonRect.y + 1, buttonRect.width, buttonRect.height - 1)
                
                dc.DrawRectangleRect(buttonRect)
            
            if self._nHoeveredImgIdx == i:
            
                # Set the colours
                penColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
                brushColour = ArtManager.Get().LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION), 90)

                dc.SetPen(wx.Pen(penColour))
                dc.SetBrush(wx.Brush(brushColour))

                # Fix the surrounding of the rect if border is set
                if style & INB_BORDER:
                
                    if style & INB_TOP or style & INB_BOTTOM:
                        buttonRect = wx.Rect(buttonRect.x + 1, buttonRect.y, buttonRect.width - 1, buttonRect.height)
                    else:
                        buttonRect = wx.Rect(buttonRect.x, buttonRect.y + 1, buttonRect.width, buttonRect.height - 1)
                
                dc.DrawRectangleRect(buttonRect)
            
            if bUseYcoord:
                rect = wx.Rect(0, pos, rectWidth, rectWidth)
            else:
                rect = wx.Rect(pos, 0, rectWidth, rectWidth)

            # Incase user set both flags:
            # INB_SHOW_ONLY_TEXT and INB_SHOW_ONLY_IMAGES
            # We override them to display both

            if style & INB_SHOW_ONLY_TEXT and style & INB_SHOW_ONLY_IMAGES:
            
                style ^= INB_SHOW_ONLY_TEXT
                style ^= INB_SHOW_ONLY_IMAGES
                self.GetParent().SetAGWWindowStyleFlag(style)
            
            # Draw the caption and text
            imgTopPadding = 10
            if not style & INB_SHOW_ONLY_TEXT and self._pagesInfoVec[i].GetImageIndex() != -1:
            
                if bUseYcoord:
                
                    imgXcoord = self._nImgSize / 2
                    imgYcoord = (style & INB_SHOW_ONLY_IMAGES and [pos + self._nImgSize / 2] or [pos + imgTopPadding])[0]
                
                else:
                
                    imgXcoord = pos + (rectWidth / 2) - (self._nImgSize / 2)
                    imgYcoord = (style & INB_SHOW_ONLY_IMAGES and [self._nImgSize / 2] or [imgTopPadding])[0]

                self._ImageList.Draw(self._pagesInfoVec[i].GetImageIndex(), dc,
                                     imgXcoord, imgYcoord,
                                     wx.IMAGELIST_DRAW_TRANSPARENT, True)
                            
            # Draw the text
            if not style & INB_SHOW_ONLY_IMAGES and not self._pagesInfoVec[i].GetCaption() == "":
            
                dc.SetFont(normalFont)
                            
                # Check if the text can fit the size of the rectangle,
                # if not truncate it 
                fixedText = self._pagesInfoVec[i].GetCaption()
                if not style & INB_FIT_BUTTON or (style & INB_LEFT or (style & INB_RIGHT)):
                
                    fixedText = self.FixTextSize(dc, self._pagesInfoVec[i].GetCaption(), self._nImgSize *2 - 4)

                    # Update the length of the text
                    textWidth, textHeight = dc.GetTextExtent(fixedText)
                
                if bUseYcoord:
                
                    textOffsetX = ((rectWidth - textWidth) / 2 )
                    textOffsetY = (not style & INB_SHOW_ONLY_TEXT  and [pos + self._nImgSize  + imgTopPadding + 3] or \
                                       [pos + ((self._nImgSize * 2 - textHeight) / 2 )])[0]
                
                else:
                
                    textOffsetX = (rectWidth - textWidth) / 2  + pos + nTextPaddingLeft
                    textOffsetY = (not style & INB_SHOW_ONLY_TEXT and [self._nImgSize + imgTopPadding + 3] or \
                                       [((self._nImgSize * 2 - textHeight) / 2 )])[0]
                
                dc.SetTextForeground(wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOWTEXT))
                dc.DrawText(fixedText, textOffsetX, textOffsetY)
            
            # Update the page info
            self._pagesInfoVec[i].SetPosition(buttonRect.GetPosition())
            self._pagesInfoVec[i].SetSize(buttonRect.GetSize())

            pos += rectWidth
        
        # Update all buttons that can not fit into the screen as non-visible
        for ii in xrange(count, len(self._pagesInfoVec)):
            self._pagesInfoVec[ii].SetPosition(wx.Point(-1, -1))

        # Draw the pin button
        if bUsePin:
        
            clientRect = self.GetClientRect()
            pinRect = wx.Rect(clientRect.GetX() + clientRect.GetWidth() - 20, 2, 20, 20)
            self.DrawPin(dc, pinRect, not self._bCollapsed)
        

# ---------------------------------------------------------------------------- #
# Class LabelContainer
# ---------------------------------------------------------------------------- #

class LabelContainer(ImageContainerBase):
    """ Base class for L{LabelBook}. """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="LabelContainer"):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.Panel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================

        :param `name`: the window name.         
        """

        ImageContainerBase.__init__(self, parent, id, pos, size, style, agwStyle, name)
        self._nTabAreaWidth = 100
        self._oldCursor = wx.NullCursor
        self._coloursMap = {}
        self._skin = wx.NullBitmap
        self._sashRect = wx.Rect()

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnMouseLeftUp)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeaveWindow)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{LabelContainer}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        ImageContainerBase.OnSize(self, event)
        event.Skip()


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{LabelContainer}.

        :param `event`: a `wx.EraseEvent` event to be processed.
        """

        ImageContainerBase.OnEraseBackground(self, event)        

        
    def GetTabAreaWidth(self):
        """ Returns the width of the tab area. """

        return self._nTabAreaWidth


    def SetTabAreaWidth(self, width):
        """
        Sets the width of the tab area.

        :param `width`: the width of the tab area, in pixels.
        """

        self._nTabAreaWidth = width


    def CanDoBottomStyle(self):
        """
        Allows the parent to examine the children type. Some implementation
        (such as L{LabelBook}), does not support top/bottom images, only left/right.
        """

        return False        


    def SetBackgroundBitmap(self, bmp):
        """
        Sets the background bitmap for the control.

        :param `bmp`: a valid `wx.Bitmap` object.
        """

        self._skin = bmp


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{LabelContainer}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """
        
        style = self.GetParent().GetAGWWindowStyleFlag()

        dc = wx.BufferedPaintDC(self)
        backBrush = wx.Brush(self._coloursMap[INB_TAB_AREA_BACKGROUND_COLOUR])
        if self.HasAGWFlag(INB_BORDER):
            borderPen = wx.Pen(self._coloursMap[INB_TABS_BORDER_COLOUR])
        else:
            borderPen = wx.TRANSPARENT_PEN
            
        size = self.GetSize()
        
        # Set the pen & brush
        dc.SetBrush(backBrush)
        dc.SetPen(borderPen)
        
        # Incase user set both flags, we override them to display both
        # INB_SHOW_ONLY_TEXT and INB_SHOW_ONLY_IMAGES
        if style & INB_SHOW_ONLY_TEXT and style & INB_SHOW_ONLY_IMAGES:
        
            style ^= INB_SHOW_ONLY_TEXT
            style ^= INB_SHOW_ONLY_IMAGES
            self.GetParent().SetAGWWindowStyleFlag(style)

        if self.HasAGWFlag(INB_GRADIENT_BACKGROUND) and not self._skin.Ok():
        
            # Draw graident in the background area
            startColour = self._coloursMap[INB_TAB_AREA_BACKGROUND_COLOUR]
            endColour   = ArtManager.Get().LightColour(self._coloursMap[INB_TAB_AREA_BACKGROUND_COLOUR], 50)
            ArtManager.Get().PaintStraightGradientBox(dc, wx.Rect(0, 0, size.x / 2, size.y), startColour, endColour, False)
            ArtManager.Get().PaintStraightGradientBox(dc, wx.Rect(size.x / 2, 0, size.x / 2, size.y), endColour, startColour, False)
        
        else:
        
            # Draw the border and background
            if self._skin.Ok():
            
                dc.SetBrush(wx.TRANSPARENT_BRUSH)
                self.DrawBackgroundBitmap(dc)
            
            dc.DrawRectangleRect(wx.Rect(0, 0, size.x, size.y))
        
        # Draw border
        if self.HasAGWFlag(INB_BORDER) and self.HasAGWFlag(INB_GRADIENT_BACKGROUND):
        
            # Just draw the border with transparent brush
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.DrawRectangleRect(wx.Rect(0, 0, size.x, size.y))

        bUsePin = (self.HasAGWFlag(INB_USE_PIN_BUTTON) and [True] or [False])[0]

        if bUsePin:
        
            # Draw the pin button
            clientRect = self.GetClientRect()
            pinRect = wx.Rect(clientRect.GetX() + clientRect.GetWidth() - 20, 2, 20, 20)
            self.DrawPin(dc, pinRect, not self._bCollapsed)

            if self._bCollapsed:
                return
        
        dc.SetPen(wx.BLACK_PEN)
        self.SetSizeHints(self._nTabAreaWidth, -1)

        # We reserve 20 pixels for the pin button
        posy = 20 
        count = 0
        
        for i in xrange(len(self._pagesInfoVec)):
            count = count+1        
            # Default values for the surronounding rectangle 
            # around a button
            rectWidth = self._nTabAreaWidth  
            
            if self.HasAGWFlag(INB_SHOW_ONLY_TEXT):
                font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
                font.SetPointSize(font.GetPointSize() * self.GetParent().GetFontSizeMultiple())
                if self.GetParent().GetFontBold():
                    font.SetWeight(wx.FONTWEIGHT_BOLD)
                dc.SetFont(font)
                w, h = dc.GetTextExtent(self._pagesInfoVec[i].GetCaption())
                rectHeight = h * 2
            else:
                rectHeight = self._nImgSize * 2

            # Check that we have enough space to draw the button
            if posy + rectHeight > size.GetHeight():
                break

            # Calculate the button rectangle
            posx = 0

            buttonRect = wx.Rect(posx, posy, rectWidth, rectHeight)
            indx = self._pagesInfoVec[i].GetImageIndex()

            if indx == -1:
                bmp = wx.NullBitmap
            else:
                bmp = self._ImageList.GetBitmap(indx)

            self.DrawLabel(dc, buttonRect, self._pagesInfoVec[i].GetCaption(), bmp,
                           self._pagesInfoVec[i], self.HasAGWFlag(INB_LEFT) or self.HasAGWFlag(INB_TOP),
                           i, self._nIndex == i, self._nHoeveredImgIdx == i)

            posy += rectHeight
        
        # Update all buttons that can not fit into the screen as non-visible
        for ii in xrange(count, len(self._pagesInfoVec)):
            self._pagesInfoVec[i].SetPosition(wx.Point(-1, -1))

        if bUsePin:
        
            clientRect = self.GetClientRect()
            pinRect = wx.Rect(clientRect.GetX() + clientRect.GetWidth() - 20, 2, 20, 20)
            self.DrawPin(dc, pinRect, not self._bCollapsed)
        

    def DrawBackgroundBitmap(self, dc):
        """
        Draws a bitmap as the background of the control.

        :param `dc`: an instance of `wx.DC`.
        """

        clientRect = self.GetClientRect()
        width = clientRect.GetWidth()
        height = clientRect.GetHeight()
        coveredY = coveredX = 0
        xstep = self._skin.GetWidth()
        ystep = self._skin.GetHeight()
        bmpRect = wx.Rect(0, 0, xstep, ystep)
        if bmpRect != clientRect:
        
            mem_dc = wx.MemoryDC()
            bmp = wx.EmptyBitmap(width, height)
            mem_dc.SelectObject(bmp)

            while coveredY < height:
            
                while coveredX < width:
                
                    mem_dc.DrawBitmap(self._skin, coveredX, coveredY, True)
                    coveredX += xstep
                
                coveredX = 0
                coveredY += ystep
            
            mem_dc.SelectObject(wx.NullBitmap)
            #self._skin = bmp
            dc.DrawBitmap(bmp, 0, 0)
        
        else:
        
            dc.DrawBitmap(self._skin, 0, 0)
        

    def OnMouseLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{LabelContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.HasAGWFlag(INB_NO_RESIZE):
        
            ImageContainerBase.OnMouseLeftUp(self, event)
            return
        
        if self.HasCapture():
            self.ReleaseMouse()

        # Sash was being dragged?
        if not self._sashRect.IsEmpty():
        
            # Remove sash
            ArtManager.Get().DrawDragSash(self._sashRect)
            self.Resize(event)

            self._sashRect = wx.Rect()
            return
        
        self._sashRect = wx.Rect()

        # Restore cursor
        if self._oldCursor.Ok():
        
            wx.SetCursor(self._oldCursor)
            self._oldCursor = wx.NullCursor
        
        ImageContainerBase.OnMouseLeftUp(self, event)


    def Resize(self, event):
        """
        Actually resizes the tab area.

        :param `event`: an instance of `wx.SizeEvent`.
        """

        # Resize our size
        self._tabAreaSize = self.GetSize()
        newWidth = self._tabAreaSize.x
        x = event.GetX()

        if self.HasAGWFlag(INB_BOTTOM) or self.HasAGWFlag(INB_RIGHT):
        
            newWidth -= event.GetX()
        
        else:
        
            newWidth = x
        
        if newWidth < 100: # Dont allow width to be lower than that 
            newWidth = 100

        self.SetSizeHints(newWidth, self._tabAreaSize.y)

        # Update the tab new area width
        self._nTabAreaWidth = newWidth
        self.GetParent().Freeze()
        self.GetParent().GetSizer().Layout()
        self.GetParent().Thaw()


    def OnMouseMove(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{LabelContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.HasAGWFlag(INB_NO_RESIZE):
        
            ImageContainerBase.OnMouseMove(self, event)
            return

        # Remove old sash
        if not self._sashRect.IsEmpty():
            ArtManager.Get().DrawDragSash(self._sashRect)

        if event.LeftIsDown():
        
            if not self._sashRect.IsEmpty():
            
                # Progress sash, and redraw it
                clientRect = self.GetClientRect()
                pt = self.ClientToScreen(wx.Point(event.GetX(), 0))
                self._sashRect = wx.RectPS(pt, wx.Size(4, clientRect.height))
                ArtManager.Get().DrawDragSash(self._sashRect)
            
            else:
            
                # Sash is not being dragged
                if self._oldCursor.Ok():
                    wx.SetCursor(self._oldCursor)
                    self._oldCursor = wx.NullCursor
                
        else:
        
            if self.HasCapture():
                self.ReleaseMouse()

            if self.PointOnSash(event.GetPosition()):
            
                # Change cursor to EW cursor
                self._oldCursor = self.GetCursor()
                wx.SetCursor(wx.StockCursor(wx.CURSOR_SIZEWE))
            
            elif self._oldCursor.Ok():
            
                wx.SetCursor(self._oldCursor)
                self._oldCursor = wx.NullCursor
            
            self._sashRect = wx.Rect()
            ImageContainerBase.OnMouseMove(self, event)
        

    def OnMouseLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{LabelContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.HasAGWFlag(INB_NO_RESIZE):
        
            ImageContainerBase.OnMouseLeftDown(self, event)
            return

        imgIdx, where = self.HitTest(event.GetPosition())

        if IMG_OVER_EW_BORDER == where and not self._bCollapsed:
            
            # We are over the sash
            if not self._sashRect.IsEmpty():
                ArtManager.Get().DrawDragSash(self._sashRect)
            else:
                # first time, begin drawing sash
                self.CaptureMouse()

                # Change mouse cursor
                self._oldCursor = self.GetCursor()
                wx.SetCursor(wx.StockCursor(wx.CURSOR_SIZEWE))
            
            clientRect = self.GetClientRect()
            pt = self.ClientToScreen(wx.Point(event.GetX(), 0))
            self._sashRect = wx.RectPS(pt, wx.Size(4, clientRect.height))

            ArtManager.Get().DrawDragSash(self._sashRect)
        
        else:
            ImageContainerBase.OnMouseLeftDown(self, event)


    def OnMouseLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{LabelContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.HasAGWFlag(INB_NO_RESIZE):
        
            ImageContainerBase.OnMouseLeaveWindow(self, event)
            return
        
        # If Sash is being dragged, ignore this event
        if not self.HasCapture():        
            ImageContainerBase.OnMouseLeaveWindow(self, event)      

        
    def DrawRegularHover(self, dc, rect):
        """
        Draws a rounded rectangle around the current tab.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the current tab client rectangle.
        """
        
        # The hovered tab with default border
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(wx.Pen(wx.WHITE))

        # We draw CCW
        if self.HasAGWFlag(INB_RIGHT) or self.HasAGWFlag(INB_TOP):
        
            # Right images
            # Upper line
            dc.DrawLine(rect.x + 1, rect.y, rect.x + rect.width, rect.y)

            # Right line (white)
            dc.DrawLine(rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height)

            # Bottom diagnol - we change pen
            dc.SetPen(wx.Pen(self._coloursMap[INB_TABS_BORDER_COLOUR]))

            # Bottom line
            dc.DrawLine(rect.x + rect.width, rect.y + rect.height, rect.x, rect.y + rect.height)
        
        else:
        
            # Left images
            # Upper line white
            dc.DrawLine(rect.x, rect.y, rect.x + rect.width - 1, rect.y)

            # Left line
            dc.DrawLine(rect.x, rect.y, rect.x, rect.y + rect.height)

            # Bottom diagnol, we change the pen
            dc.SetPen(wx.Pen(self._coloursMap[INB_TABS_BORDER_COLOUR]))

            # Bottom line
            dc.DrawLine(rect.x, rect.y + rect.height, rect.x + rect.width, rect.y + rect.height)
        

    def DrawWebHover(self, dc, caption, xCoord, yCoord):
        """
        Draws a web style hover effect (cursor set to hand & text is underlined).

        :param `dc`: an instance of `wx.DC`;
        :param `caption`: the tab caption text;
        :param `xCoord`: the x position of the tab caption;
        :param `yCoord`: the y position of the tab caption.
        """

        # Redraw the text with underlined font
        underLinedFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        underLinedFont.SetPointSize(underLinedFont.GetPointSize() * self.GetParent().GetFontSizeMultiple())
        if self.GetParent().GetFontBold():
            underLinedFont.SetWeight(wx.FONTWEIGHT_BOLD)
        underLinedFont.SetUnderlined(True)
        dc.SetFont(underLinedFont)
        dc.DrawText(caption, xCoord, yCoord)


    def SetColour(self, which, colour):
        """
        Sets a colour for a parameter.

        :param `which`: can be one of the following parameters:

         ================================== ======= ==================================
         Colour Key                          Value  Description
         ================================== ======= ==================================         
         ``INB_TAB_AREA_BACKGROUND_COLOUR``     100 The tab area background colour
         ``INB_ACTIVE_TAB_COLOUR``              101 The active tab background colour
         ``INB_TABS_BORDER_COLOUR``             102 The tabs border colour
         ``INB_TEXT_COLOUR``                    103 The tab caption text colour
         ``INB_ACTIVE_TEXT_COLOUR``             104 The active tab caption text colour
         ``INB_HILITE_TAB_COLOUR``              105 The tab caption highlight text colour
         ================================== ======= ==================================         

        :param `colour`: a valid `wx.Colour` object.        
        """

        self._coloursMap[which] = colour


    def GetColour(self, which):
        """
        Returns a colour for a parameter.

        :param `which`: the colour key.

        :see: L{SetColour} for a list of valid colour keys.
        """

        if not self._coloursMap.has_key(which):
            return wx.Colour()

        return self._coloursMap[which]        


    def InitializeColours(self):
        """ Initializes the colours map to be used for this control. """

        # Initialize map colours
        self._coloursMap.update({INB_TAB_AREA_BACKGROUND_COLOUR: ArtManager.Get().LightColour(ArtManager.Get().FrameColour(), 50)})
        self._coloursMap.update({INB_ACTIVE_TAB_COLOUR: ArtManager.Get().GetMenuFaceColour()})
        self._coloursMap.update({INB_TABS_BORDER_COLOUR: wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DSHADOW)})
        self._coloursMap.update({INB_HILITE_TAB_COLOUR: wx.NamedColour("LIGHT BLUE")})
        self._coloursMap.update({INB_TEXT_COLOUR: wx.WHITE})
        self._coloursMap.update({INB_ACTIVE_TEXT_COLOUR: wx.BLACK})

        # dont allow bright colour one on the other
        if not ArtManager.Get().IsDark(self._coloursMap[INB_TAB_AREA_BACKGROUND_COLOUR]) and \
           not ArtManager.Get().IsDark(self._coloursMap[INB_TEXT_COLOUR]):
        
            self._coloursMap[INB_TEXT_COLOUR] = ArtManager.Get().DarkColour(self._coloursMap[INB_TEXT_COLOUR], 100)
        

    def DrawLabel(self, dc, rect, text, bmp, imgInfo, orientationLeft, imgIdx, selected, hover):
        """
        Draws a label using the specified dc.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the text client rectangle;
        :param `text`: the actual text string;
        :param `bmp`: a bitmap to be drawn next to the text;
        :param `imgInfo`: an instance of L{ImageInfo};
        :param `orientationLeft`: ``True`` if the book has the ``INB_RIGHT`` or ``INB_LEFT``
         style set;
        :param `imgIdx`: the tab image index;
        :param `selected`: ``True`` if the tab is selected, ``False`` otherwise;
        :param `hover`: ``True`` if the tab is being hovered with the mouse, ``False`` otherwise.
        """

        dcsaver = DCSaver(dc)
        nPadding = 6
        
        if orientationLeft:
        
            rect.x += nPadding
            rect.width -= nPadding
        
        else:
        
            rect.width -= nPadding
        
        textRect = wx.Rect(*rect)
        imgRect = wx.Rect(*rect)
        
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetPointSize(font.GetPointSize() * self.GetParent().GetFontSizeMultiple())
        if self.GetParent().GetFontBold():
            font.SetWeight(wx.FONTWEIGHT_BOLD)
        dc.SetFont(font)

        # First we define the rectangle for the text
        w, h = dc.GetTextExtent(text)
        
        #-------------------------------------------------------------------------
        # Label layout:
        # [ nPadding | Image | nPadding | Text | nPadding ]
        #-------------------------------------------------------------------------

        # Text bounding rectangle
        textRect.x += nPadding
        textRect.y = rect.y + (rect.height - h)/2
        textRect.width = rect.width - 2 * nPadding

        if bmp.Ok() and not self.HasAGWFlag(INB_SHOW_ONLY_TEXT):
            textRect.x += (bmp.GetWidth() + nPadding)
            textRect.width -= (bmp.GetWidth() + nPadding)
        
        textRect.height = h

        # Truncate text if needed
        caption = ArtManager.Get().TruncateText(dc, text, textRect.width)

        # Image bounding rectangle
        if bmp.Ok() and not self.HasAGWFlag(INB_SHOW_ONLY_TEXT):
        
            imgRect.x += nPadding
            imgRect.width = bmp.GetWidth()
            imgRect.y = rect.y + (rect.height - bmp.GetHeight())/2
            imgRect.height = bmp.GetHeight()
        
        # Draw bounding rectangle
        if selected:
        
            # First we colour the tab
            dc.SetBrush(wx.Brush(self._coloursMap[INB_ACTIVE_TAB_COLOUR]))

            if self.HasAGWFlag(INB_BORDER):
                dc.SetPen(wx.Pen(self._coloursMap[INB_TABS_BORDER_COLOUR]))
            else: 
                dc.SetPen(wx.Pen(self._coloursMap[INB_ACTIVE_TAB_COLOUR]))
            
            labelRect = wx.Rect(*rect)

            if orientationLeft: 
                labelRect.width += 3
            else: 
                labelRect.width += 3
                labelRect.x -= 3
            
            dc.DrawRoundedRectangleRect(labelRect, 3)

            if not orientationLeft and self.HasAGWFlag(INB_DRAW_SHADOW):
                dc.SetPen(wx.BLACK_PEN)
                dc.DrawPoint(labelRect.x + labelRect.width - 1, labelRect.y + labelRect.height - 1)
            
        # Draw the text & bitmap
        if caption != "":
        
            if selected:
                dc.SetTextForeground(self._coloursMap[INB_ACTIVE_TEXT_COLOUR])
            else:
                dc.SetTextForeground(self._coloursMap[INB_TEXT_COLOUR])
                
            dc.DrawText(caption, textRect.x, textRect.y)
            imgInfo.SetTextRect(textRect)
        
        else:
        
            imgInfo.SetTextRect(wx.Rect())
        
        if bmp.Ok() and not self.HasAGWFlag(INB_SHOW_ONLY_TEXT):
            dc.DrawBitmap(bmp, imgRect.x, imgRect.y, True)

        # Drop shadow
        if self.HasAGWFlag(INB_DRAW_SHADOW) and selected:
        
            sstyle = 0
            if orientationLeft:
                sstyle = BottomShadow
            else:
                sstyle = BottomShadowFull | RightShadow
            
            if self.HasAGWFlag(INB_WEB_HILITE):
            
                # Always drop shadow for this style
                ArtManager.Get().DrawBitmapShadow(dc, rect, sstyle)
            
            else:
            
                if imgIdx+1 != self._nHoeveredImgIdx:
                
                    ArtManager.Get().DrawBitmapShadow(dc, rect, sstyle)
                
        # Draw hover effect 
        if hover:
        
            if self.HasAGWFlag(INB_WEB_HILITE) and caption != "":   
                self.DrawWebHover(dc, caption, textRect.x, textRect.y)
            else:
                self.DrawRegularHover(dc, rect)
        
        # Update the page information bout position and size
        imgInfo.SetPosition(rect.GetPosition())
        imgInfo.SetSize(rect.GetSize())


# ---------------------------------------------------------------------------- #
# Class FlatBookBase
# ---------------------------------------------------------------------------- #

class FlatBookBase(wx.Panel):
    """ Base class for the containing window for L{LabelBook} and L{FlatImageBook}. """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="FlatBookBase"):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.Panel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================

        :param `name`: the window name.         
        """
        
        self._pages = None
        self._bInitializing = True
        self._pages = None
        self._bForceSelection = False
        self._windows = []
        self._fontSizeMultiple = 1.0
        self._fontBold = False

        style |= wx.TAB_TRAVERSAL
        self._agwStyle = agwStyle

        wx.Panel.__init__(self, parent, id, pos, size, style, name)
        self._bInitializing = False


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the window style.

        :param `agwStyle`: can be a combination of the following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================
        
        """

        self._agwStyle = agwStyle
        
        # Check that we are not in initialization process
        if self._bInitializing:
            return

        if not self._pages:
            return

        # Detach the windows attached to the sizer
        if self.GetSelection() >= 0:
            self._mainSizer.Detach(self._windows[self.GetSelection()])

        self._mainSizer.Detach(self._pages)
        
        # Create new sizer with the requested orientaion
        className = self.GetName()

        if className == "LabelBook":
            self._mainSizer = wx.BoxSizer(wx.HORIZONTAL)
        else:
            if agwStyle & INB_LEFT or agwStyle & INB_RIGHT:
                self._mainSizer = wx.BoxSizer(wx.HORIZONTAL)
            else:
                self._mainSizer = wx.BoxSizer(wx.VERTICAL)
        
        self.SetSizer(self._mainSizer)
        
        # Add the tab container and the separator
        self._mainSizer.Add(self._pages, 0, wx.EXPAND)

        if className == "FlatImageBook":
        
            if agwStyle & INB_LEFT or agwStyle & INB_RIGHT:
                self._pages.SetSizeHints(self._pages._nImgSize * 2, -1)
            else:
                self._pages.SetSizeHints(-1, self._pages._nImgSize * 2)
        
        # Attach the windows back to the sizer to the sizer
        if self.GetSelection() >= 0:
            self.DoSetSelection(self._windows[self.GetSelection()])

        if agwStyle & INB_FIT_LABELTEXT:
            self.ResizeTabArea()
            
        self._mainSizer.Layout()
        dummy = wx.SizeEvent()
        wx.PostEvent(self, dummy)
        self._pages.Refresh()


    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{FlatBookBase} window style.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.
        """

        return self._agwStyle


    def HasAGWFlag(self, flag):
        """
        Returns whether a flag is present in the L{FlatBookBase} style.

        :param `flag`: one of the possible L{FlatBookBase} window styles.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.
        """

        agwStyle = self.GetAGWWindowStyleFlag()
        res = (agwStyle & flag and [True] or [False])[0]
        return res


    def AddPage(self, page, text, select=False, imageId=-1):
        """
        Adds a page to the book.

        :param `page`: specifies the new page;
        :param `text`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `imageId`: specifies the optional image index for the new page.
        
        :note: The call to this function generates the page changing events.
        """

        if not page:
            return

        page.Reparent(self)

        self._windows.append(page)
        
        if select or len(self._windows) == 1:
            self.DoSetSelection(page)
        else:
            page.Hide()

        self._pages.AddPage(text, select, imageId)
        self.ResizeTabArea()
        self.Refresh()


    def InsertPage(self, page_idx, page, text, select=False, imageId=-1):
        """
        Inserts a page into the book at the specified position.

        :param `page_idx`: specifies the position for the new page;
        :param `page`: specifies the new page;
        :param `text`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `imageId`: specifies the optional image index for the new page.
        
        :note: The call to this function generates the page changing events.
        """

        if not page:
            return

        page.Reparent(self)

        self._windows.insert(page_idx, page)
        
        if select or len(self._windows) == 1:
            self.DoSetSelection(page)
        else:
            page.Hide()

        self._pages.InsertPage(page_idx, text, select, imageId)
        self.ResizeTabArea()
        self.Refresh()


    def DeletePage(self, page):
        """
        Deletes the specified page, and the associated window.

        :param `page`: an integer specifying the page to be deleted.
        
        :note: The call to this function generates the page changing events.
        """

        if page >= len(self._windows) or page < 0:
            return

        # Fire a closing event
        event = ImageNotebookEvent(wxEVT_IMAGENOTEBOOK_PAGE_CLOSING, self.GetId())
        event.SetSelection(page)
        event.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(event)

        # The event handler allows it?
        if not event.IsAllowed():
            return False

        self.Freeze()

        # Delete the requested page
        pageRemoved = self._windows[page]

        # If the page is the current window, remove it from the sizer
        # as well
        if page == self.GetSelection():
            self._mainSizer.Detach(pageRemoved)
        
        # Remove it from the array as well
        self._windows.pop(page)

        # Now we can destroy it in wxWidgets use Destroy instead of delete
        pageRemoved.Destroy()
        self._mainSizer.Layout()
        
        self._pages.DoDeletePage(page)
        self.ResizeTabArea()
        self.Thaw()

        # Fire a closed event
        closedEvent = ImageNotebookEvent(wxEVT_IMAGENOTEBOOK_PAGE_CLOSED, self.GetId())
        closedEvent.SetSelection(page)
        closedEvent.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(closedEvent)


    def RemovePage(self, page):
        """
        Deletes the specified page, without deleting the associated window.

        :param `page`: an integer specifying the page to be removed.
        
        :note: The call to this function generates the page changing events.
        """

        if page >= len(self._windows):
            return False

        # Fire a closing event
        event = ImageNotebookEvent(wxEVT_IMAGENOTEBOOK_PAGE_CLOSING, self.GetId())
        event.SetSelection(page)
        event.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(event)

        # The event handler allows it?
        if not event.IsAllowed():
            return False

        self.Freeze()

        # Remove the requested page
        pageRemoved = self._windows[page]

        # If the page is the current window, remove it from the sizer
        # as well
        if page == self.GetSelection():
            self._mainSizer.Detach(pageRemoved)
        
        # Remove it from the array as well
        self._windows.pop(page)
        self._mainSizer.Layout()
        self.ResizeTabArea()
        self.Thaw()

        self._pages.DoDeletePage(page)

        # Fire a closed event
        closedEvent = ImageNotebookEvent(wxEVT_IMAGENOTEBOOK_PAGE_CLOSED, self.GetId())
        closedEvent.SetSelection(page)
        closedEvent.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(closedEvent)
        
        return True

    
    def ResizeTabArea(self):
        """ Resizes the tab area if the control has the ``INB_FIT_LABELTEXT`` style set. """

        agwStyle = self.GetAGWWindowStyleFlag()
        
        if agwStyle & INB_FIT_LABELTEXT == 0:
            return

        if agwStyle & INB_LEFT or agwStyle & INB_RIGHT:
            dc = wx.MemoryDC()
            dc.SelectObject(wx.EmptyBitmap(1, 1))
            font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            font.SetPointSize(font.GetPointSize()*self._fontSizeMultiple)
            if self.GetFontBold():
                font.SetWeight(wx.FONTWEIGHT_BOLD)
            dc.SetFont(font)
            maxW = 0
            
            for page in xrange(self.GetPageCount()):
                caption = self._pages.GetPageText(page)
                w, h = dc.GetTextExtent(caption)
                maxW = max(maxW, w)
               
            maxW += 24 #TODO this is 6*4 6 is nPadding from drawlabel

            if not agwStyle & INB_SHOW_ONLY_TEXT:
                maxW += self._pages._nImgSize * 2
                
            maxW = max(maxW, 100)
            self._pages.SetSizeHints(maxW, -1)
            self._pages._nTabAreaWidth = maxW

        
    def DeleteAllPages(self):
        """ Deletes all the pages in the book. """

        if not self._windows:
            return

        self.Freeze()
        
        for win in self._windows:
            win.Destroy()
        
        self._windows = []
        self.Thaw()

        # remove old selection
        self._pages.ClearAll()
        self._pages.Refresh()


    def SetSelection(self, page):
        """
        Changes the selection from currently visible/selected page to the page
        given by page.

        :param `page`: an integer specifying the page to be selected.

        :note: The call to this function generates the page changing events.        
        """

        if page >= len(self._windows):
            return

        if page == self.GetSelection() and not self._bForceSelection:
            return

        oldSelection = self.GetSelection()

        # Generate an event that indicates that an image is about to be selected
        event = ImageNotebookEvent(wxEVT_IMAGENOTEBOOK_PAGE_CHANGING, self.GetId())
        event.SetSelection(page)
        event.SetOldSelection(oldSelection)
        event.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(event)

        # The event handler allows it?
        if not event.IsAllowed() and not self._bForceSelection:
            return

        self.DoSetSelection(self._windows[page])
        # Now we can update the new selection
        self._pages._nIndex = page

        # Refresh calls the OnPaint of this class
        self._pages.Refresh()

        # Generate an event that indicates that an image was selected
        eventChanged = ImageNotebookEvent(wxEVT_IMAGENOTEBOOK_PAGE_CHANGED, self.GetId())
        eventChanged.SetEventObject(self)
        eventChanged.SetOldSelection(oldSelection)
        eventChanged.SetSelection(page)
        self.GetEventHandler().ProcessEvent(eventChanged)


    def AssignImageList(self, imglist):
        """
        Assigns an image list to the control.

        :param `imglist`: an instance of `wx.ImageList`.
        """

        self._pages.AssignImageList(imglist)

        # Force change
        self.SetAGWWindowStyleFlag(self.GetAGWWindowStyleFlag())


    def GetSelection(self):
        """ Returns the current selection. """

        if self._pages:
            return self._pages._nIndex
        else:
            return -1


    def DoSetSelection(self, window):
        """
        Select the window by the provided pointer.

        :param `window`: an instance of `wx.Window`.
        """

        curSel = self.GetSelection()
        agwStyle = self.GetAGWWindowStyleFlag()
        # Replace the window in the sizer
        self.Freeze()

        # Check if a new selection was made
        bInsertFirst = (agwStyle & INB_BOTTOM or agwStyle & INB_RIGHT)

        if curSel >= 0:
        
            # Remove the window from the main sizer
            self._mainSizer.Detach(self._windows[curSel])
            self._windows[curSel].Hide()
        
        if bInsertFirst:
            self._mainSizer.Insert(0, window, 1, wx.EXPAND)
        else:
            self._mainSizer.Add(window, 1, wx.EXPAND)

        window.Show()
        self._mainSizer.Layout()
        self.Thaw()


    def GetImageList(self):
        """ Returns the associated image list. """

        return self._pages.GetImageList()


    def GetPageCount(self):
        """ Returns the number of pages in the book. """

        return len(self._windows)
    
    
    def GetFontBold(self):
        """ Gets the font bold status. """
        
        return self._fontBold
    
    
    def SetFontBold(self, bold):
        """ 
        Sets whether the page captions are bold or not.
        
        :param `bold`: ``True`` or ``False``.
        """
        
        self._fontBold = bold
   
    
    def GetFontSizeMultiple(self):
        """ Gets the font size multiple for the page captions. """
        
        return self._fontSizeMultiple
   
    
    def SetFontSizeMultiple(self, multiple):
        """ 
        Sets the font size multiple for the page captions. 
        
        :param `multiple`: The multiple to be applied to the system font to get the our font size.
        """
        
        self._fontSizeMultiple = multiple
    

    def SetPageImage(self, page, imageId):
        """
        Sets the image index for the given page.

        :param `page`: an integer specifying the page index;
        :param `image`: an index into the image list.
        """

        self._pages.SetPageImage(page, imageId)
        self._pages.Refresh()


    def SetPageText(self, page, text):
        """
        Sets the text for the given page.

        :param `page`: an integer specifying the page index;
        :param `text`: the new tab label.
        """

        self._pages.SetPageText(page, text)
        self._pages.Refresh()


    def GetPageText(self, page):
        """
        Returns the text for the given page.

        :param `page`: an integer specifying the page index.
        """

        return self._pages.GetPageText(page)


    def GetPageImage(self, page):
        """
        Returns the image index for the given page.

        :param `page`: an integer specifying the page index.
        """

        return self._pages.GetPageImage(page)


    def GetPage(self, page):
        """
        Returns the window at the given page position.

        :param `page`: an integer specifying the page to be returned.
        """

        if page >= len(self._windows):
            return

        return self._windows[page]


    def GetCurrentPage(self):
        """ Returns the currently selected notebook page or ``None``. """

        if self.GetSelection() < 0:
            return

        return self.GetPage(self.GetSelection())


    def AdvanceSelection(self, forward=True):
        """
        Cycles through the tabs.

        :param `forward`: if ``True``, the selection is advanced in ascending order
         (to the right), otherwise the selection is advanced in descending order.
         
        :note: The call to this function generates the page changing events.
        """

        nSel = self.GetSelection()

        if nSel < 0:
            return

        nMax = self.GetPageCount() - 1
        
        if forward:
            newSelection = (nSel == nMax and [0] or [nSel + 1])[0]
        else:
            newSelection = (nSel == 0 and [nMax] or [nSel - 1])[0]

        self.SetSelection(newSelection)


    def ChangeSelection(self, page):
        """
        Changes the selection for the given page, returning the previous selection.

        :param `page`: an integer specifying the page to be selected.

        :note: The call to this function does not generate the page changing events.
        """

        if page < 0 or page >= self.GetPageCount():
            return

        oldPage = self.GetSelection()
        self.DoSetSelection(page)

        return oldPage

    CurrentPage = property(GetCurrentPage, doc="See `GetCurrentPage`")
    Page = property(GetPage, doc="See `GetPage`") 
    PageCount = property(GetPageCount, doc="See `GetPageCount`") 
    PageImage = property(GetPageImage, SetPageImage, doc="See `GetPageImage, SetPageImage`") 
    PageText = property(GetPageText, SetPageText, doc="See `GetPageText, SetPageText`") 
    Selection = property(GetSelection, SetSelection, doc="See `GetSelection, SetSelection`") 
    
    
# ---------------------------------------------------------------------------- #
# Class FlatImageBook
# ---------------------------------------------------------------------------- #
        
class FlatImageBook(FlatBookBase):
    """
    Default implementation of the image book, it is like a `wx.Notebook`, except that
    images are used to control the different pages. This container is usually used
    for configuration dialogs etc.
    
    :note: Currently, this control works properly for images of size 32x32 and bigger.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="FlatImageBook"):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.Panel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================

        :param `name`: the window name.         
        """
        
        FlatBookBase.__init__(self, parent, id, pos, size, style, agwStyle, name)
        
        self._pages = self.CreateImageContainer()

        if agwStyle & INB_LEFT or agwStyle & INB_RIGHT:
            self._mainSizer = wx.BoxSizer(wx.HORIZONTAL)
        else:
            self._mainSizer = wx.BoxSizer(wx.VERTICAL)

        self.SetSizer(self._mainSizer)

        # Add the tab container to the sizer
        self._mainSizer.Add(self._pages, 0, wx.EXPAND)

        if agwStyle & INB_LEFT or agwStyle & INB_RIGHT:
            self._pages.SetSizeHints(self._pages.GetImageSize() * 2, -1)
        else:
            self._pages.SetSizeHints(-1, self._pages.GetImageSize() * 2)

        self._mainSizer.Layout()
        
        
    def CreateImageContainer(self):
        """ Creates the image container class for L{FlatImageBook}. """

        return ImageContainer(self, wx.ID_ANY, agwStyle=self.GetAGWWindowStyleFlag())


# ---------------------------------------------------------------------------- #
# Class LabelBook
# ---------------------------------------------------------------------------- #

class LabelBook(FlatBookBase):
    """
    An implementation of a notebook control - except that instead of having
    tabs to show labels, it labels to the right or left (arranged horizontally).
    """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="LabelBook"):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.Panel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``INB_BOTTOM``                      0x1 Place labels below the page area. Available only for L{FlatImageBook}.
         ``INB_LEFT``                        0x2 Place labels on the left side. Available only for L{FlatImageBook}.
         ``INB_RIGHT``                       0x4 Place labels on the right side.
         ``INB_TOP``                         0x8 Place labels above the page area.
         ``INB_BORDER``                     0x10 Draws a border around L{LabelBook} or L{FlatImageBook}.
         ``INB_SHOW_ONLY_TEXT``             0x20 Shows only text labels and no images. Available only for L{LabelBook}.
         ``INB_SHOW_ONLY_IMAGES``           0x40 Shows only tab images and no label texts. Available only for L{LabelBook}.
         ``INB_FIT_BUTTON``                 0x80 Displays a pin button to show/hide the book control.
         ``INB_DRAW_SHADOW``               0x100 Draw shadows below the book tabs. Available only for L{LabelBook}.
         ``INB_USE_PIN_BUTTON``            0x200 Displays a pin button to show/hide the book control.
         ``INB_GRADIENT_BACKGROUND``       0x400 Draws a gradient shading on the tabs background. Available only for L{LabelBook}.
         ``INB_WEB_HILITE``                0x800 On mouse hovering, tabs behave like html hyperlinks. Available only for L{LabelBook}.
         ``INB_NO_RESIZE``                0x1000 Don't allow resizing of the tab area.
         ``INB_FIT_LABELTEXT``            0x2000 Will fit the tab area to the longest text (or text+image if you have images) in all the tabs.
         =========================== =========== ==================================================

        :param `name`: the window name.         
        """
        
        FlatBookBase.__init__(self, parent, id, pos, size, style, agwStyle, name)
        
        self._pages = self.CreateImageContainer()

        # Label book specific initialization
        self._mainSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.SetSizer(self._mainSizer)

        # Add the tab container to the sizer
        self._mainSizer.Add(self._pages, 0, wx.EXPAND)
        self._pages.SetSizeHints(self._pages.GetTabAreaWidth(), -1)

        # Initialize the colours maps
        self._pages.InitializeColours()

        self.Bind(wx.EVT_SIZE, self.OnSize)
        

    def CreateImageContainer(self):
        """ Creates the image container (LabelContainer) class for L{FlatImageBook}. """

        return LabelContainer(self, wx.ID_ANY, agwStyle=self.GetAGWWindowStyleFlag())


    def SetColour(self, which, colour):
        """
        Sets the colour for the specified parameter.

        :param `which`: the colour key;
        :param `colour`: a valid `wx.Colour` instance.

        :see: L{LabelContainer.SetColour} for a list of valid colour keys.
        """

        self._pages.SetColour(which, colour)


    def GetColour(self, which):
        """
        Returns the colour for the specified parameter.

        :param `which`: the colour key.

        :see: L{LabelContainer.SetColour} for a list of valid colour keys.
        """

        return self._pages.GetColour(which)
    
    
    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{LabelBook}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self._pages.Refresh()
        event.Skip()


