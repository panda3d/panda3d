"""
auibook contains a notebook control which implements many features common in
applications with dockable panes. Specifically, L{AuiNotebook} implements functionality
which allows the user to rearrange tab order via drag-and-drop, split the tab window
into many different splitter configurations, and toggle through different themes to
customize the control's look and feel.

An effort has been made to try to maintain an API as similar to that of `wx.Notebook`.

The default theme that is used is L{AuiDefaultTabArt}, which provides a modern, glossy
look and feel. The theme can be changed by calling L{AuiNotebook.SetArtProvider}.
"""

__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx
import types
import datetime

from wx.lib.expando import ExpandoTextCtrl

import framemanager
import tabart as TA

from aui_utilities import LightColour, MakeDisabledBitmap, TabDragImage
from aui_utilities import TakeScreenShot, RescaleScreenShot

from aui_constants import *

# AuiNotebook events
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BUTTON = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_END_DRAG = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_TAB_DCLICK = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BG_MIDDLE_DOWN = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BG_MIDDLE_UP = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BG_RIGHT_DOWN = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BG_RIGHT_UP = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK = wx.NewEventType()

# Define a new event for a drag cancelled
wxEVT_COMMAND_AUINOTEBOOK_CANCEL_DRAG = wx.NewEventType()

# Define events for editing a tab label
wxEVT_COMMAND_AUINOTEBOOK_BEGIN_LABEL_EDIT = wx.NewEventType()
wxEVT_COMMAND_AUINOTEBOOK_END_LABEL_EDIT = wx.NewEventType()

# Create event binders
EVT_AUINOTEBOOK_PAGE_CLOSE = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, 1)
""" A tab in `AuiNotebook` is being closed. Can be vetoed by calling `Veto()`. """
EVT_AUINOTEBOOK_PAGE_CLOSED = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED, 1)
""" A tab in `AuiNotebook` has been closed. """
EVT_AUINOTEBOOK_PAGE_CHANGED = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, 1)
""" The page selection was changed. """
EVT_AUINOTEBOOK_PAGE_CHANGING = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, 1)
""" The page selection is being changed. """
EVT_AUINOTEBOOK_BUTTON = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BUTTON, 1)
""" The user clicked on a button in the `AuiNotebook` tab area. """
EVT_AUINOTEBOOK_BEGIN_DRAG = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG, 1)
""" A drag-and-drop operation on a notebook tab has started. """
EVT_AUINOTEBOOK_END_DRAG = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_END_DRAG, 1)
""" A drag-and-drop operation on a notebook tab has finished. """
EVT_AUINOTEBOOK_DRAG_MOTION = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION, 1)
""" A drag-and-drop operation on a notebook tab is ongoing. """
EVT_AUINOTEBOOK_ALLOW_DND = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND, 1)
""" Fires an event asking if it is OK to drag and drop a tab. """
EVT_AUINOTEBOOK_DRAG_DONE = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE, 1)
""" A drag-and-drop operation on a notebook tab has finished. """
EVT_AUINOTEBOOK_TAB_MIDDLE_DOWN = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN, 1)
""" The user clicked with the middle mouse button on a tab. """
EVT_AUINOTEBOOK_TAB_MIDDLE_UP = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP, 1)
""" The user clicked with the middle mouse button on a tab. """
EVT_AUINOTEBOOK_TAB_RIGHT_DOWN = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN, 1)
""" The user clicked with the right mouse button on a tab. """
EVT_AUINOTEBOOK_TAB_RIGHT_UP = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, 1)
""" The user clicked with the right mouse button on a tab. """
EVT_AUINOTEBOOK_BG_MIDDLE_DOWN = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BG_MIDDLE_DOWN, 1)
""" The user middle-clicked in the tab area but not over a tab or a button. """
EVT_AUINOTEBOOK_BG_MIDDLE_UP = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BG_MIDDLE_UP, 1)
""" The user middle-clicked in the tab area but not over a tab or a button. """
EVT_AUINOTEBOOK_BG_RIGHT_DOWN = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BG_RIGHT_DOWN, 1)
""" The user right-clicked in the tab area but not over a tab or a button. """
EVT_AUINOTEBOOK_BG_RIGHT_UP = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BG_RIGHT_UP, 1)
""" The user right-clicked in the tab area but not over a tab or a button. """
EVT_AUINOTEBOOK_BG_DCLICK = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK, 1)
""" The user left-clicked on the tab area not occupied by `AuiNotebook` tabs. """
EVT_AUINOTEBOOK_CANCEL_DRAG = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_CANCEL_DRAG, 1)
""" A drag and drop operation has been cancelled. """
EVT_AUINOTEBOOK_TAB_DCLICK = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_TAB_DCLICK, 1)
""" The user double-clicked with the left mouse button on a tab. """
EVT_AUINOTEBOOK_BEGIN_LABEL_EDIT = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_BEGIN_LABEL_EDIT, 1)
""" The user double-clicked with the left mouse button on a tab which text is editable. """
EVT_AUINOTEBOOK_END_LABEL_EDIT = wx.PyEventBinder(wxEVT_COMMAND_AUINOTEBOOK_END_LABEL_EDIT, 1)
""" The user finished editing a tab label. """


# -----------------------------------------------------------------------------
# Auxiliary class: TabTextCtrl
# This is the temporary ExpandoTextCtrl created when you edit the text of a tab
# -----------------------------------------------------------------------------

class TabTextCtrl(ExpandoTextCtrl):
    """ Control used for in-place edit. """

    def __init__(self, owner, tab, page_index):
        """
        Default class constructor.
        For internal use: do not call it in your code!

        :param `owner`: the L{AuiTabCtrl} owning the tab;
        :param `tab`: the actual L{AuiNotebookPage} tab;
        :param `page_index`: the L{AuiNotebook} page index for the tab.
        """

        self._owner = owner
        self._tabEdited = tab
        self._pageIndex = page_index
        self._startValue = tab.caption
        self._finished = False
        self._aboutToFinish = False
        self._currentValue = self._startValue

        x, y, w, h = self._tabEdited.rect

        wnd = self._tabEdited.control
        if wnd:
            x += wnd.GetSize()[0] + 2
            h = 0

        image_h = 0
        image_w = 0

        image = tab.bitmap

        if image.IsOk():
            image_w, image_h = image.GetWidth(), image.GetHeight()
            image_w += 6

        dc = wx.ClientDC(self._owner)
        h = max(image_h, dc.GetMultiLineTextExtent(tab.caption)[1])
        h = h + 2

        # FIXME: what are all these hardcoded 4, 8 and 11s really?
        x += image_w
        w -= image_w + 4

        y = (self._tabEdited.rect.height - h)/2 + 1

        expandoStyle = wx.WANTS_CHARS
        if wx.Platform in ["__WXGTK__", "__WXMAC__"]:
            expandoStyle |= wx.SIMPLE_BORDER
            xSize, ySize = w + 2, h
        else:
            expandoStyle |= wx.SUNKEN_BORDER
            xSize, ySize = w + 2, h+2

        ExpandoTextCtrl.__init__(self, self._owner, wx.ID_ANY, self._startValue,
                                 wx.Point(x, y), wx.Size(xSize, ySize),
                                 expandoStyle)

        if wx.Platform == "__WXMAC__":
            self.SetFont(owner.GetFont())
            bs = self.GetBestSize()
            self.SetSize((-1, bs.height))

        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)


    def AcceptChanges(self):
        """ Accepts/refuses the changes made by the user. """

        value = self.GetValue()
        notebook = self._owner.GetParent()

        if value == self._startValue:
            # nothing changed, always accept
            # when an item remains unchanged, the owner
            # needs to be notified that the user decided
            # not to change the tree item label, and that
            # the edit has been cancelled
            notebook.OnRenameCancelled(self._pageIndex)
            return True

        if not notebook.OnRenameAccept(self._pageIndex, value):
            # vetoed by the user
            return False

        # accepted, do rename the item
        notebook.SetPageText(self._pageIndex, value)

        return True


    def Finish(self):
        """ Finish editing. """

        if not self._finished:

            notebook = self._owner.GetParent()

            self._finished = True
            self._owner.SetFocus()
            notebook.ResetTextControl()


    def OnChar(self, event):
        """
        Handles the ``wx.EVT_CHAR`` event for L{TabTextCtrl}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        keycode = event.GetKeyCode()
        shiftDown = event.ShiftDown()

        if keycode == wx.WXK_RETURN:
            if shiftDown and self._tabEdited.IsMultiline():
                event.Skip()
            else:
                self._aboutToFinish = True
                self.SetValue(self._currentValue)
                # Notify the owner about the changes
                self.AcceptChanges()
                # Even if vetoed, close the control (consistent with MSW)
                wx.CallAfter(self.Finish)

        elif keycode == wx.WXK_ESCAPE:
            self.StopEditing()

        else:
            event.Skip()


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` event for L{TabTextCtrl}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        if not self._finished:

            # auto-grow the textctrl:
            mySize = self.GetSize()

            dc = wx.ClientDC(self)
            sx, sy, dummy = dc.GetMultiLineTextExtent(self.GetValue() + "M")

            self.SetSize((sx, -1))
            self._currentValue = self.GetValue()

        event.Skip()


    def OnKillFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{TabTextCtrl}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        if not self._finished and not self._aboutToFinish:

            # We must finish regardless of success, otherwise we'll get
            # focus problems:
            if not self.AcceptChanges():
                self._owner.GetParent().OnRenameCancelled(self._pageIndex)

        # We must let the native text control handle focus, too, otherwise
        # it could have problems with the cursor (e.g., in wxGTK).
        event.Skip()
        wx.CallAfter(self._owner.GetParent().ResetTextControl)


    def StopEditing(self):
        """ Suddenly stops the editing. """

        self._owner.GetParent().OnRenameCancelled(self._pageIndex)
        self.Finish()


    def item(self):
        """ Returns the item currently edited. """

        return self._tabEdited


# ----------------------------------------------------------------------

class AuiNotebookPage(object):
    """
    A simple class which holds information about tab captions, bitmaps and
    colours.
    """

    def __init__(self):
        """
        Default class constructor.
        Used internally, do not call it in your code!
        """

        self.window = None              # page's associated window
        self.caption = ""               # caption displayed on the tab
        self.bitmap = wx.NullBitmap     # tab's bitmap
        self.dis_bitmap = wx.NullBitmap # tab's disabled bitmap
        self.rect = wx.Rect()           # tab's hit rectangle
        self.active = False             # True if the page is currently active
        self.enabled = True             # True if the page is currently enabled
        self.hasCloseButton = True      # True if the page has a close button using the style
                                        # AUI_NB_CLOSE_ON_ALL_TABS
        self.control = None             # A control can now be inside a tab
        self.renamable = False          # If True, a tab can be renamed by a left double-click

        self.text_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_BTNTEXT)

        self.access_time = datetime.datetime.now() # Last time this page was selected


    def IsMultiline(self):
        """ Returns whether the tab contains multiline text. """

        return "\n" in self.caption


# ----------------------------------------------------------------------

class AuiTabContainerButton(object):
    """
    A simple class which holds information about tab buttons and their state.
    """

    def __init__(self):
        """
        Default class constructor.
        Used internally, do not call it in your code!
        """

        self.id = -1                                      # button's id
        self.cur_state = AUI_BUTTON_STATE_NORMAL          # current state (normal, hover, pressed, etc.)
        self.location = wx.LEFT                           # buttons location (wxLEFT, wxRIGHT, or wxCENTER)
        self.bitmap = wx.NullBitmap                       # button's hover bitmap
        self.dis_bitmap = wx.NullBitmap                   # button's disabled bitmap
        self.rect = wx.Rect()                             # button's hit rectangle


# ----------------------------------------------------------------------

class CommandNotebookEvent(wx.PyCommandEvent):
    """ A specialized command event class for events sent by L{AuiNotebook} . """

    def __init__(self, command_type=None, win_id=0):
        """
        Default class constructor.

        :param `command_type`: the event kind or an instance of `wx.PyCommandEvent`.
        :param `win_id`: the window identification number.
        """

        if type(command_type) == types.IntType:
            wx.PyCommandEvent.__init__(self, command_type, win_id)
        else:
            wx.PyCommandEvent.__init__(self, command_type.GetEventType(), command_type.GetId())

        self.old_selection = -1
        self.selection = -1
        self.drag_source = None
        self.dispatched = 0
        self.label = ""
        self.editCancelled = False


    def SetSelection(self, s):
        """
        Sets the selection member variable.

        :param `s`: the new selection.
        """

        self.selection = s
        self._commandInt = s


    def GetSelection(self):
        """ Returns the currently selected page, or -1 if none was selected. """

        return self.selection


    def SetOldSelection(self, s):
        """
        Sets the id of the page selected before the change.

        :param `s`: the old selection.
        """

        self.old_selection = s


    def GetOldSelection(self):
        """
        Returns the page that was selected before the change, or -1 if none was
        selected.
        """

        return self.old_selection


    def SetDragSource(self, s):
        """
        Sets the drag and drop source.

        :param `s`: the drag source.
        """

        self.drag_source = s


    def GetDragSource(self):
        """ Returns the drag and drop source. """

        return self.drag_source


    def SetDispatched(self, b):
        """
        Sets the event as dispatched (used for automatic L{AuiNotebook} ).

        :param `b`: whether the event was dispatched or not.
        """

        self.dispatched = b


    def GetDispatched(self):
        """ Returns whether the event was dispatched (used for automatic L{AuiNotebook} ). """

        return self.dispatched


    def IsEditCancelled(self):
        """ Returns the edit cancel flag (for ``EVT_AUINOTEBOOK_BEGIN`` | ``END_LABEL_EDIT`` only)."""

        return self.editCancelled


    def SetEditCanceled(self, editCancelled):
        """
        Sets the edit cancel flag (for ``EVT_AUINOTEBOOK_BEGIN`` | ``END_LABEL_EDIT`` only).

        :param `editCancelled`: whether the editing action has been cancelled or not.
        """

        self.editCancelled = editCancelled


    def GetLabel(self):
        """Returns the label-itemtext (for ``EVT_AUINOTEBOOK_BEGIN`` | ``END_LABEL_EDIT`` only)."""

        return self.label


    def SetLabel(self, label):
        """
        Sets the label. Useful only for ``EVT_AUINOTEBOOK_END_LABEL_EDIT``.

        :param `label`: the new label.
        """

        self.label = label


# ----------------------------------------------------------------------

class AuiNotebookEvent(CommandNotebookEvent):
    """ A specialized command event class for events sent by L{AuiNotebook}. """

    def __init__(self, command_type=None, win_id=0):
        """
        Default class constructor.

        :param `command_type`: the event kind or an instance of `wx.PyCommandEvent`.
        :param `win_id`: the window identification number.
        """

        CommandNotebookEvent.__init__(self, command_type, win_id)

        if type(command_type) == types.IntType:
            self.notify = wx.NotifyEvent(command_type, win_id)
        else:
            self.notify = wx.NotifyEvent(command_type.GetEventType(), command_type.GetId())


    def GetNotifyEvent(self):
        """ Returns the actual `wx.NotifyEvent`. """

        return self.notify


    def IsAllowed(self):
        """ Returns whether the event is allowed or not. """

        return self.notify.IsAllowed()


    def Veto(self):
        """
        Prevents the change announced by this event from happening.

        It is in general a good idea to notify the user about the reasons for
        vetoing the change because otherwise the applications behaviour (which
        just refuses to do what the user wants) might be quite surprising.
        """

        self.notify.Veto()


    def Allow(self):
        """
        This is the opposite of L{Veto}: it explicitly allows the event to be
        processed. For most events it is not necessary to call this method as the
        events are allowed anyhow but some are forbidden by default (this will
        be mentioned in the corresponding event description).
        """

        self.notify.Allow()


# ---------------------------------------------------------------------------- #
# Class TabNavigatorWindow
# ---------------------------------------------------------------------------- #

class TabNavigatorWindow(wx.Dialog):
    """
    This class is used to create a modal dialog that enables "Smart Tabbing",
    similar to what you would get by hitting ``Alt`` + ``Tab`` on Windows.
    """

    def __init__(self, parent=None, icon=None):
        """
        Default class constructor. Used internally.

        :param `parent`: the L{TabNavigatorWindow} parent;
        :param `icon`: the L{TabNavigatorWindow} icon.
        """

        wx.Dialog.__init__(self, parent, wx.ID_ANY, "", style=0)

        self._selectedItem = -1
        self._indexMap = []

        if icon is None:
            self._bmp = Mondrian.GetBitmap()
        else:
            self._bmp = icon

        if self._bmp.GetSize() != (16, 16):
            img = self._bmp.ConvertToImage()
            img.Rescale(16, 16, wx.IMAGE_QUALITY_HIGH)
            self._bmp = wx.BitmapFromImage(img)

        sz = wx.BoxSizer(wx.VERTICAL)

        self._listBox = wx.ListBox(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(200, 150), [], wx.LB_SINGLE | wx.NO_BORDER)

        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(wx.EmptyBitmap(1,1))
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetWeight(wx.BOLD)
        mem_dc.SetFont(font)

        panelHeight = mem_dc.GetCharHeight()
        panelHeight += 4 # Place a spacer of 2 pixels

        # Out signpost bitmap is 24 pixels
        if panelHeight < 24:
            panelHeight = 24

        self._panel = wx.Panel(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(200, panelHeight))

        sz.Add(self._panel)
        sz.Add(self._listBox, 1, wx.EXPAND)

        self.SetSizer(sz)

        # Connect events to the list box
        self._listBox.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self._listBox.Bind(wx.EVT_NAVIGATION_KEY, self.OnNavigationKey)
        self._listBox.Bind(wx.EVT_LISTBOX_DCLICK, self.OnItemSelected)

        # Connect paint event to the panel
        self._panel.Bind(wx.EVT_PAINT, self.OnPanelPaint)
        self._panel.Bind(wx.EVT_ERASE_BACKGROUND, self.OnPanelEraseBg)

        self.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self._listBox.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self.PopulateListControl(parent)

        self.GetSizer().Fit(self)
        self.GetSizer().SetSizeHints(self)
        self.GetSizer().Layout()
        self.Centre()

        # Set focus on the list box to avoid having to click on it to change
        # the tab selection under GTK.
        self._listBox.SetFocus()


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` for the L{TabNavigatorWindow}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        if event.GetKeyCode() == wx.WXK_CONTROL:
            self.CloseDialog()


    def OnNavigationKey(self, event):
        """
        Handles the ``wx.EVT_NAVIGATION_KEY`` for the L{TabNavigatorWindow}.

        :param `event`: a `wx.NavigationKeyEvent` event to be processed.
        """

        selected = self._listBox.GetSelection()
        bk = self.GetParent()
        maxItems = bk.GetPageCount()

        if event.GetDirection():

            # Select next page
            if selected == maxItems - 1:
                itemToSelect = 0
            else:
                itemToSelect = selected + 1

        else:

            # Previous page
            if selected == 0:
                itemToSelect = maxItems - 1
            else:
                itemToSelect = selected - 1

        self._listBox.SetSelection(itemToSelect)


    def PopulateListControl(self, book):
        """
        Populates the L{TabNavigatorWindow} listbox with a list of tabs.

        :param `book`: the actual L{AuiNotebook}.
        """
        # Index of currently selected page
        selection = book.GetSelection()
        # Total number of pages
        count = book.GetPageCount()
        # List of (index, AuiNotebookPage)
        pages = list(enumerate(book.GetTabContainer().GetPages()))
        if book.GetAGWWindowStyleFlag() & AUI_NB_ORDER_BY_ACCESS:
            # Sort pages using last access time. Most recently used is the
            # first in line
            pages.sort(
                key = lambda element: element[1].access_time,
                reverse = True
            )
        else:
            # Manually add the current selection as first item
            # Remaining ones are added in the next loop
            del pages[selection]
            self._listBox.Append(book.GetPageText(selection))
            self._indexMap.append(selection)

        for (index, page) in pages:
            self._listBox.Append(book.GetPageText(index))
            self._indexMap.append(index)

        # Select the next entry after the current selection
        self._listBox.SetSelection(0)
        dummy = wx.NavigationKeyEvent()
        dummy.SetDirection(True)
        self.OnNavigationKey(dummy)


    def OnItemSelected(self, event):
        """
        Handles the ``wx.EVT_LISTBOX_DCLICK`` event for the wx.ListBox inside L{TabNavigatorWindow}.

        :param `event`: a `wx.ListEvent` event to be processed.
        """

        self.CloseDialog()


    def CloseDialog(self):
        """ Closes the L{TabNavigatorWindow} dialog, setting selection in L{AuiNotebook}. """

        bk = self.GetParent()
        self._selectedItem = self._listBox.GetSelection()
        self.EndModal(wx.ID_OK)


    def GetSelectedPage(self):
        """ Gets the page index that was selected when the dialog was closed. """

        return self._indexMap[self._selectedItem]


    def OnPanelPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{TabNavigatorWindow} top panel.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.PaintDC(self._panel)
        rect = self._panel.GetClientRect()

        bmp = wx.EmptyBitmap(rect.width, rect.height)

        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(bmp)

        endColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)
        startColour = LightColour(endColour, 50)
        mem_dc.GradientFillLinear(rect, startColour, endColour, wx.SOUTH)

        # Draw the caption title and place the bitmap
        # get the bitmap optimal position, and draw it
        bmpPt, txtPt = wx.Point(), wx.Point()
        bmpPt.y = (rect.height - self._bmp.GetHeight())/2
        bmpPt.x = 3
        mem_dc.DrawBitmap(self._bmp, bmpPt.x, bmpPt.y, True)

        # get the text position, and draw it
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetWeight(wx.BOLD)
        mem_dc.SetFont(font)
        fontHeight = mem_dc.GetCharHeight()

        txtPt.x = bmpPt.x + self._bmp.GetWidth() + 4
        txtPt.y = (rect.height - fontHeight)/2
        mem_dc.SetTextForeground(wx.WHITE)
        mem_dc.DrawText("Opened tabs:", txtPt.x, txtPt.y)
        mem_dc.SelectObject(wx.NullBitmap)

        dc.DrawBitmap(bmp, 0, 0)


    def OnPanelEraseBg(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{TabNavigatorWindow} top panel.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This is intentionally empty, to reduce flicker.
        """

        pass


# ----------------------------------------------------------------------
# -- AuiTabContainer class implementation --

class AuiTabContainer(object):
    """
    AuiTabContainer is a class which contains information about each
    tab. It also can render an entire tab control to a specified DC.
    It's not a window class itself, because this code will be used by
    the L{AuiManager}, where it is disadvantageous to have separate
    windows for each tab control in the case of "docked tabs".

    A derived class, L{AuiTabCtrl}, is an actual `wx.Window`-derived window
    which can be used as a tab control in the normal sense.
    """

    def __init__(self, auiNotebook):
        """
        Default class constructor.
        Used internally, do not call it in your code!

        :param `auiNotebook`: the parent L{AuiNotebook} window.        
        """

        self._tab_offset = 0
        self._agwFlags = 0
        self._art = TA.AuiDefaultTabArt()

        self._buttons = []
        self._pages = []
        self._tab_close_buttons = []

        self._rect = wx.Rect()
        self._auiNotebook = auiNotebook

        self.AddButton(AUI_BUTTON_LEFT, wx.LEFT)
        self.AddButton(AUI_BUTTON_RIGHT, wx.RIGHT)
        self.AddButton(AUI_BUTTON_WINDOWLIST, wx.RIGHT)
        self.AddButton(AUI_BUTTON_CLOSE, wx.RIGHT)


    def SetArtProvider(self, art):
        """
        Instructs L{AuiTabContainer} to use art provider specified by parameter `art`
        for all drawing calls. This allows plugable look-and-feel features.

        :param `art`: an art provider.

        :note: The previous art provider object, if any, will be deleted by L{AuiTabContainer}.
        """

        del self._art
        self._art = art

        if self._art:
            self._art.SetAGWFlags(self._agwFlags)


    def GetArtProvider(self):
        """ Returns the current art provider being used. """

        return self._art


    def SetAGWFlags(self, agwFlags):
        """
        Sets the tab art flags.

        :param `agwFlags`: a combination of the following values:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by L{AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ``AUI_NB_ORDER_BY_ACCESS``           Tab navigation order by last access time for the tabs
         ``AUI_NB_NO_TAB_FOCUS``              Don't draw tab focus rectangle
         ==================================== ==================================

        :todo: Implementation of flags ``AUI_NB_RIGHT`` and ``AUI_NB_LEFT``.

        """

        self._agwFlags = agwFlags

        # check for new close button settings
        self.RemoveButton(AUI_BUTTON_LEFT)
        self.RemoveButton(AUI_BUTTON_RIGHT)
        self.RemoveButton(AUI_BUTTON_WINDOWLIST)
        self.RemoveButton(AUI_BUTTON_CLOSE)

        if agwFlags & AUI_NB_SCROLL_BUTTONS:
            self.AddButton(AUI_BUTTON_LEFT, wx.LEFT)
            self.AddButton(AUI_BUTTON_RIGHT, wx.RIGHT)

        if agwFlags & AUI_NB_WINDOWLIST_BUTTON:
            self.AddButton(AUI_BUTTON_WINDOWLIST, wx.RIGHT)

        if agwFlags & AUI_NB_CLOSE_BUTTON:
            self.AddButton(AUI_BUTTON_CLOSE, wx.RIGHT)

        if self._art:
            self._art.SetAGWFlags(self._agwFlags)


    def GetAGWFlags(self):
        """
        Returns the tab art flags.

        See L{SetAGWFlags} for a list of possible return values.

        :see: L{SetAGWFlags}
        """

        return self._agwFlags


    def SetNormalFont(self, font):
        """
        Sets the normal font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """

        self._art.SetNormalFont(font)


    def SetSelectedFont(self, font):
        """
        Sets the selected tab font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """

        self._art.SetSelectedFont(font)


    def SetMeasuringFont(self, font):
        """
        Sets the font for calculating text measurements.

        :param `font`: a `wx.Font` object.
        """

        self._art.SetMeasuringFont(font)


    def SetTabRect(self, rect):
        """
        Sets the tab area rectangle.

        :param `rect`: an instance of `wx.Rect`, specifying the available area for L{AuiTabContainer}.
        """

        self._rect = rect

        if self._art:
            minMaxTabWidth = self._auiNotebook.GetMinMaxTabWidth()
            self._art.SetSizingInfo(rect.GetSize(), len(self._pages), minMaxTabWidth)


    def AddPage(self, page, info):
        """
        Adds a page to the tab control.

        :param `page`: the window associated with this tab;
        :param `info`: an instance of L{AuiNotebookPage}.
        """

        page_info = info
        page_info.window = page

        self._pages.append(page_info)

        # let the art provider know how many pages we have
        if self._art:
            minMaxTabWidth = self._auiNotebook.GetMinMaxTabWidth()
            self._art.SetSizingInfo(self._rect.GetSize(), len(self._pages), minMaxTabWidth)

        return True


    def InsertPage(self, page, info, idx):
        """
        Inserts a page in the tab control in the position specified by `idx`.

        :param `page`: the window associated with this tab;
        :param `info`: an instance of L{AuiNotebookPage};
        :param `idx`: the page insertion index.
        """

        page_info = info
        page_info.window = page

        if idx >= len(self._pages):
            self._pages.append(page_info)
        else:
            self._pages.insert(idx, page_info)

        # let the art provider know how many pages we have
        if self._art:
            minMaxTabWidth = self._auiNotebook.GetMinMaxTabWidth()
            self._art.SetSizingInfo(self._rect.GetSize(), len(self._pages), minMaxTabWidth)

        return True


    def MovePage(self, page, new_idx):
        """
        Moves a page in a new position specified by `new_idx`.

        :param `page`: the window associated with this tab;
        :param `new_idx`: the new page position.
        """

        idx = self.GetIdxFromWindow(page)
        if idx == -1:
            return False

        # get page entry, make a copy of it
        p = self.GetPage(idx)

        # remove old page entry
        self.RemovePage(page)

        # insert page where it should be
        self.InsertPage(page, p, new_idx)

        return True


    def RemovePage(self, wnd):
        """
        Removes a page from the tab control.

        :param `wnd`: an instance of `wx.Window`, a window associated with this tab.
        """

        minMaxTabWidth = self._auiNotebook.GetMinMaxTabWidth()

        for page in self._pages:
            if page.window == wnd:
                self._pages.remove(page)
                self._tab_offset = min(self._tab_offset, len(self._pages) - 1)

                # let the art provider know how many pages we have
                if self._art:
                    self._art.SetSizingInfo(self._rect.GetSize(), len(self._pages), minMaxTabWidth)

                return True

        return False


    def SetActivePage(self, wndOrInt):
        """
        Sets the L{AuiTabContainer} active page.

        :param `wndOrInt`: an instance of `wx.Window` or an integer specifying a tab index.
        """

        if type(wndOrInt) == types.IntType:

            if wndOrInt >= len(self._pages):
                return False

            wnd = self._pages[wndOrInt].window

        else:
            wnd = wndOrInt

        found = False

        for indx, page in enumerate(self._pages):
            if page.window == wnd:
                page.active = True
                found = True
            else:
                page.active = False

        return found


    def SetNoneActive(self):
        """ Sets all the tabs as inactive (non-selected). """

        for page in self._pages:
            page.active = False


    def GetActivePage(self):
        """ Returns the current selected tab or ``wx.NOT_FOUND`` if none is selected. """

        for indx, page in enumerate(self._pages):
            if page.active:
                return indx

        return wx.NOT_FOUND


    def GetWindowFromIdx(self, idx):
        """
        Returns the window associated with the tab with index `idx`.

        :param `idx`: the tab index.
        """

        if idx >= len(self._pages):
            return None

        return self._pages[idx].window


    def GetIdxFromWindow(self, wnd):
        """
        Returns the tab index based on the window `wnd` associated with it.

        :param `wnd`: an instance of `wx.Window`.
        """

        for indx, page in enumerate(self._pages):
            if page.window == wnd:
                return indx

        return wx.NOT_FOUND


    def GetPage(self, idx):
        """
        Returns the page specified by the given index.

        :param `idx`: the tab index.
        """

        if idx < 0 or idx >= len(self._pages):
            raise Exception("Invalid Page index")

        return self._pages[idx]


    def GetPages(self):
        """ Returns a list of all the pages in this L{AuiTabContainer}. """

        return self._pages


    def GetPageCount(self):
        """ Returns the number of pages in the L{AuiTabContainer}. """

        return len(self._pages)


    def GetEnabled(self, idx):
        """
        Returns whether a tab is enabled or not.

        :param `idx`: the tab index.
        """

        if idx < 0 or idx >= len(self._pages):
            return False

        return self._pages[idx].enabled


    def EnableTab(self, idx, enable=True):
        """
        Enables/disables a tab in the L{AuiTabContainer}.

        :param `idx`: the tab index;
        :param `enable`: ``True`` to enable a tab, ``False`` to disable it.
        """

        if idx < 0 or idx >= len(self._pages):
            raise Exception("Invalid Page index")

        self._pages[idx].enabled = enable
        wnd = self.GetWindowFromIdx(idx)
        wnd.Enable(enable)


    def AddButton(self, id, location, normal_bitmap=wx.NullBitmap, disabled_bitmap=wx.NullBitmap):
        """
        Adds a button in the tab area.

        :param `id`: the button identifier. This can be one of the following:

         ==============================  =================================
         Button Identifier               Description
         ==============================  =================================
         ``AUI_BUTTON_CLOSE``            Shows a close button on the tab area
         ``AUI_BUTTON_WINDOWLIST``       Shows a window list button on the tab area
         ``AUI_BUTTON_LEFT``             Shows a left button on the tab area
         ``AUI_BUTTON_RIGHT``            Shows a right button on the tab area
         ==============================  =================================

        :param `location`: the button location. Can be ``wx.LEFT`` or ``wx.RIGHT``;
        :param `normal_bitmap`: the bitmap for an enabled tab;
        :param `disabled_bitmap`: the bitmap for a disabled tab.
        """

        button = AuiTabContainerButton()
        button.id = id
        button.bitmap = normal_bitmap
        button.dis_bitmap = disabled_bitmap
        button.location = location
        button.cur_state = AUI_BUTTON_STATE_NORMAL

        self._buttons.append(button)


    def RemoveButton(self, id):
        """
        Removes a button from the tab area.

        :param `id`: the button identifier. See L{AddButton} for a list of button identifiers.

        :see: L{AddButton}
        """

        for button in self._buttons:
            if button.id == id:
                self._buttons.remove(button)
                return


    def GetTabOffset(self):
        """ Returns the tab offset. """

        return self._tab_offset


    def SetTabOffset(self, offset):
        """
        Sets the tab offset.

        :param `offset`: the tab offset.
        """

        self._tab_offset = offset


    def MinimizeTabOffset(self, dc, wnd, max_width):
        """
        Minimize `self._tab_offset` to fit as many tabs as possible in the available space.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: an instance of `wx.Window`;
        :param `max_width`: the maximum available width for the tabs.
        """

        total_width = 0

        for i, page in reversed(list(enumerate(self._pages))):

            tab_button = self._tab_close_buttons[i]
            (tab_width, tab_height), x_extent = self._art.GetTabSize(dc, wnd, page.caption, page.bitmap, page.active, tab_button.cur_state, page.control)
            total_width += tab_width

            if total_width > max_width:

                tab_offset = i + 1

                if tab_offset < self._tab_offset and tab_offset < len(self._pages):
                    self._tab_offset = tab_offset

                break

        if i == 0:
            self._tab_offset = 0


    def Render(self, raw_dc, wnd):
        """
        Renders the tab catalog to the specified `wx.DC`.

        It is a virtual function and can be overridden to provide custom drawing
        capabilities.

        :param `raw_dc`: a `wx.DC` device context;
        :param `wnd`: an instance of `wx.Window`.
        """

        if not raw_dc or not raw_dc.IsOk():
            return

        dc = wx.MemoryDC()

        # use the same layout direction as the window DC uses to ensure that the
        # text is rendered correctly
        dc.SetLayoutDirection(raw_dc.GetLayoutDirection())

        page_count = len(self._pages)
        button_count = len(self._buttons)

        # create off-screen bitmap
        bmp = wx.EmptyBitmap(self._rect.GetWidth(), self._rect.GetHeight())
        dc.SelectObject(bmp)

        if not dc.IsOk():
            return

        # find out if size of tabs is larger than can be
        # afforded on screen
        total_width = visible_width = 0

        for i in xrange(page_count):
            page = self._pages[i]

            # determine if a close button is on this tab
            close_button = False
            if (self._agwFlags & AUI_NB_CLOSE_ON_ALL_TABS and page.hasCloseButton) or \
               (self._agwFlags & AUI_NB_CLOSE_ON_ACTIVE_TAB and page.active and page.hasCloseButton):

                close_button = True

            control = page.control
            if control:
                try:
                    control.GetSize()
                except wx.PyDeadObjectError:
                    page.control = None

            size, x_extent = self._art.GetTabSize(dc, wnd, page.caption, page.bitmap, page.active,
                                                  (close_button and [AUI_BUTTON_STATE_NORMAL] or \
                                                   [AUI_BUTTON_STATE_HIDDEN])[0], page.control)

            if i+1 < page_count:
                total_width += x_extent
            else:
                total_width += size[0]

            if i >= self._tab_offset:
                if i+1 < page_count:
                    visible_width += x_extent
                else:
                    visible_width += size[0]

        if total_width > self._rect.GetWidth() or self._tab_offset != 0:

            # show left/right buttons
            for button in self._buttons:
                if button.id == AUI_BUTTON_LEFT or \
                   button.id == AUI_BUTTON_RIGHT:

                    button.cur_state &= ~AUI_BUTTON_STATE_HIDDEN

        else:

            # hide left/right buttons
            for button in self._buttons:
                if button.id == AUI_BUTTON_LEFT or \
                   button.id == AUI_BUTTON_RIGHT:

                    button.cur_state |= AUI_BUTTON_STATE_HIDDEN

        # determine whether left button should be enabled
        for button in self._buttons:
            if button.id == AUI_BUTTON_LEFT:
                if self._tab_offset == 0:
                    button.cur_state |= AUI_BUTTON_STATE_DISABLED
                else:
                    button.cur_state &= ~AUI_BUTTON_STATE_DISABLED

            if button.id == AUI_BUTTON_RIGHT:
                if visible_width < self._rect.GetWidth() - 16*button_count:
                    button.cur_state |= AUI_BUTTON_STATE_DISABLED
                else:
                    button.cur_state &= ~AUI_BUTTON_STATE_DISABLED

        # draw background
        self._art.DrawBackground(dc, wnd, self._rect)

        # draw buttons
        left_buttons_width = 0
        right_buttons_width = 0

        # draw the buttons on the right side
        offset = self._rect.x + self._rect.width

        for i in xrange(button_count):
            button = self._buttons[button_count - i - 1]

            if button.location != wx.RIGHT:
                continue
            if button.cur_state & AUI_BUTTON_STATE_HIDDEN:
                continue

            button_rect = wx.Rect(*self._rect)
            button_rect.SetY(1)
            button_rect.SetWidth(offset)

            button.rect = self._art.DrawButton(dc, wnd, button_rect, button, wx.RIGHT)

            offset -= button.rect.GetWidth()
            right_buttons_width += button.rect.GetWidth()

        offset = 0

        # draw the buttons on the left side
        for i in xrange(button_count):
            button = self._buttons[button_count - i - 1]

            if button.location != wx.LEFT:
                continue
            if button.cur_state & AUI_BUTTON_STATE_HIDDEN:
                continue

            button_rect = wx.Rect(offset, 1, 1000, self._rect.height)

            button.rect = self._art.DrawButton(dc, wnd, button_rect, button, wx.LEFT)

            offset += button.rect.GetWidth()
            left_buttons_width += button.rect.GetWidth()

        offset = left_buttons_width

        if offset == 0:
            offset += self._art.GetIndentSize()

        # prepare the tab-close-button array
        # make sure tab button entries which aren't used are marked as hidden
        for i in xrange(page_count, len(self._tab_close_buttons)):
            self._tab_close_buttons[i].cur_state = AUI_BUTTON_STATE_HIDDEN

        # make sure there are enough tab button entries to accommodate all tabs
        while len(self._tab_close_buttons) < page_count:
            tempbtn = AuiTabContainerButton()
            tempbtn.id = AUI_BUTTON_CLOSE
            tempbtn.location = wx.CENTER
            tempbtn.cur_state = AUI_BUTTON_STATE_HIDDEN
            self._tab_close_buttons.append(tempbtn)

        # buttons before the tab offset must be set to hidden
        for i in xrange(self._tab_offset):
            self._tab_close_buttons[i].cur_state = AUI_BUTTON_STATE_HIDDEN
            if self._pages[i].control:
                if self._pages[i].control.IsShown():
                    self._pages[i].control.Hide()

        self.MinimizeTabOffset(dc, wnd, self._rect.GetWidth() - right_buttons_width - offset - 2)

        # draw the tabs
        active = 999
        active_offset = 0

        rect = wx.Rect(*self._rect)
        rect.y = 0
        rect.height = self._rect.height

        for i in xrange(self._tab_offset, page_count):

            page = self._pages[i]
            tab_button = self._tab_close_buttons[i]

            # determine if a close button is on this tab
            if (self._agwFlags & AUI_NB_CLOSE_ON_ALL_TABS and page.hasCloseButton) or \
               (self._agwFlags & AUI_NB_CLOSE_ON_ACTIVE_TAB and page.active and page.hasCloseButton):

                if tab_button.cur_state == AUI_BUTTON_STATE_HIDDEN:

                    tab_button.id = AUI_BUTTON_CLOSE
                    tab_button.cur_state = AUI_BUTTON_STATE_NORMAL
                    tab_button.location = wx.CENTER

            else:

                tab_button.cur_state = AUI_BUTTON_STATE_HIDDEN

            rect.x = offset
            rect.width = self._rect.width - right_buttons_width - offset - 2

            if rect.width <= 0:
                break

            page.rect, tab_button.rect, x_extent = self._art.DrawTab(dc, wnd, page, rect, tab_button.cur_state)

            if page.active:
                active = i
                active_offset = offset
                active_rect = wx.Rect(*rect)

            offset += x_extent

        lenPages = len(self._pages)
        # make sure to deactivate buttons which are off the screen to the right
        for j in xrange(i+1, len(self._tab_close_buttons)):
            self._tab_close_buttons[j].cur_state = AUI_BUTTON_STATE_HIDDEN
            if j > 0 and j <= lenPages:
                if self._pages[j-1].control:
                    if self._pages[j-1].control.IsShown():
                        self._pages[j-1].control.Hide()

        # draw the active tab again so it stands in the foreground
        if active >= self._tab_offset and active < len(self._pages):

            page = self._pages[active]
            tab_button = self._tab_close_buttons[active]

            rect.x = active_offset
            dummy = self._art.DrawTab(dc, wnd, page, active_rect, tab_button.cur_state)

        raw_dc.Blit(self._rect.x, self._rect.y, self._rect.GetWidth(), self._rect.GetHeight(), dc, 0, 0)


    def IsTabVisible(self, tabPage, tabOffset, dc, wnd):
        """
        Returns whether a tab is visible or not.

        :param `tabPage`: the tab index;
        :param `tabOffset`: the tab offset;
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: an instance of `wx.Window` derived window.
        """

        if not dc or not dc.IsOk():
            return False

        page_count = len(self._pages)
        button_count = len(self._buttons)
        self.Render(dc, wnd)

        # Hasn't been rendered yet assume it's visible
        if len(self._tab_close_buttons) < page_count:
            return True

        if self._agwFlags & AUI_NB_SCROLL_BUTTONS:
            # First check if both buttons are disabled - if so, there's no need to
            # check further for visibility.
            arrowButtonVisibleCount = 0
            for i in xrange(button_count):

                button = self._buttons[i]
                if button.id == AUI_BUTTON_LEFT or \
                   button.id == AUI_BUTTON_RIGHT:

                    if button.cur_state & AUI_BUTTON_STATE_HIDDEN == 0:
                        arrowButtonVisibleCount += 1

            # Tab must be visible
            if arrowButtonVisibleCount == 0:
                return True

        # If tab is less than the given offset, it must be invisible by definition
        if tabPage < tabOffset:
            return False

        # draw buttons
        left_buttons_width = 0
        right_buttons_width = 0

        offset = 0

        # calculate size of the buttons on the right side
        offset = self._rect.x + self._rect.width

        for i in xrange(button_count):
            button = self._buttons[button_count - i - 1]

            if button.location != wx.RIGHT:
                continue
            if button.cur_state & AUI_BUTTON_STATE_HIDDEN:
                continue

            offset -= button.rect.GetWidth()
            right_buttons_width += button.rect.GetWidth()

        offset = 0

        # calculate size of the buttons on the left side
        for i in xrange(button_count):
            button = self._buttons[button_count - i - 1]

            if button.location != wx.LEFT:
                continue
            if button.cur_state & AUI_BUTTON_STATE_HIDDEN:
                continue

            offset += button.rect.GetWidth()
            left_buttons_width += button.rect.GetWidth()

        offset = left_buttons_width

        if offset == 0:
            offset += self._art.GetIndentSize()

        rect = wx.Rect(*self._rect)
        rect.y = 0
        rect.height = self._rect.height

        # See if the given page is visible at the given tab offset (effectively scroll position)
        for i in xrange(tabOffset, page_count):

            page = self._pages[i]
            tab_button = self._tab_close_buttons[i]

            rect.x = offset
            rect.width = self._rect.width - right_buttons_width - offset - 2

            if rect.width <= 0:
                return False # haven't found the tab, and we've run out of space, so return False

            size, x_extent = self._art.GetTabSize(dc, wnd, page.caption, page.bitmap, page.active, tab_button.cur_state, page.control)
            offset += x_extent

            if i == tabPage:

                # If not all of the tab is visible, and supposing there's space to display it all,
                # we could do better so we return False.
                if (self._rect.width - right_buttons_width - offset - 2) <= 0 and (self._rect.width - right_buttons_width - left_buttons_width) > x_extent:
                    return False
                else:
                    return True

        # Shouldn't really get here, but if it does, assume the tab is visible to prevent
        # further looping in calling code.
        return True


    def MakeTabVisible(self, tabPage, win):
        """
        Make the tab visible if it wasn't already.

        :param `tabPage`: the tab index;
        :param `win`: an instance of `wx.Window` derived window.
        """

        dc = wx.ClientDC(win)

        if not self.IsTabVisible(tabPage, self.GetTabOffset(), dc, win):
            for i in xrange(len(self._pages)):
                if self.IsTabVisible(tabPage, i, dc, win):
                    self.SetTabOffset(i)
                    win.Refresh()
                    return


    def TabHitTest(self, x, y):
        """
        TabHitTest() tests if a tab was hit, passing the window pointer
        back if that condition was fulfilled.

        :param `x`: the mouse `x` position;
        :param `y`: the mouse `y` position.
        """

        if not self._rect.Contains((x,y)):
            return None

        btn = self.ButtonHitTest(x, y)
        if btn:
            if btn in self._buttons:
                return None

        for i in xrange(self._tab_offset, len(self._pages)):
            page = self._pages[i]
            if page.rect.Contains((x,y)):
                return page.window

        return None


    def ButtonHitTest(self, x, y):
        """
        Tests if a button was hit.

        :param `x`: the mouse `x` position;
        :param `y`: the mouse `y` position.

        :returns: and instance of L{AuiTabContainerButton} if a button was hit, ``None`` otherwise.
        """

        if not self._rect.Contains((x,y)):
            return None

        for button in self._buttons:
            if button.rect.Contains((x,y)) and \
               (button.cur_state & (AUI_BUTTON_STATE_HIDDEN|AUI_BUTTON_STATE_DISABLED)) == 0:
                return button

        for button in self._tab_close_buttons:
            if button.rect.Contains((x,y)) and \
               (button.cur_state & (AUI_BUTTON_STATE_HIDDEN|AUI_BUTTON_STATE_DISABLED)) == 0:
                return button

        return None


    def DoShowHide(self):
        """
        This function shows the active window, then hides all of the other windows
        (in that order).
        """

        pages = self.GetPages()

        # show new active page first
        for page in pages:
            if page.active:
                page.window.Show(True)
                break

        # hide all other pages
        for page in pages:
            if not page.active:
                page.window.Show(False)


# ----------------------------------------------------------------------
# -- AuiTabCtrl class implementation --

class AuiTabCtrl(wx.PyControl, AuiTabContainer):
    """
    This is an actual `wx.Window`-derived window which can be used as a tab
    control in the normal sense.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.NO_BORDER|wx.WANTS_CHARS|wx.TAB_TRAVERSAL):
        """
        Default class constructor.
        Used internally, do not call it in your code!

        :param `parent`: the L{AuiTabCtrl} parent;
        :param `id`: an identifier for the control: a value of -1 is taken to mean a default;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style.
        """

        wx.PyControl.__init__(self, parent, id, pos, size, style, name="AuiTabCtrl")
        AuiTabContainer.__init__(self, parent)

        self._click_pt = wx.Point(-1, -1)
        self._is_dragging = False
        self._hover_button = None
        self._pressed_button = None
        self._drag_image = None
        self._drag_img_offset = (0, 0)
        self._on_button = False

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDClick)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MIDDLE_DOWN, self.OnMiddleDown)
        self.Bind(wx.EVT_MIDDLE_UP, self.OnMiddleUp)
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        self.Bind(wx.EVT_RIGHT_UP, self.OnRightUp)
        self.Bind(wx.EVT_SET_FOCUS, self.OnSetFocus)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_MOUSE_CAPTURE_LOST, self.OnCaptureLost)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)
        self.Bind(EVT_AUINOTEBOOK_BUTTON, self.OnButton)


    def IsDragging(self):
        """ Returns whether the user is dragging a tab with the mouse or not. """

        return self._is_dragging


    def GetDefaultBorder(self):
        """ Returns the default border style for L{AuiTabCtrl}. """

        return wx.BORDER_NONE


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.PaintDC(self)
        dc.SetFont(self.GetFont())

        if self.GetPageCount() > 0:
            self.Render(dc, self)


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This is intentionally empty, to reduce flicker.
        """

        pass


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same
        size as it would have after a call to `Fit()`.

        :note: Overridden from `wx.PyControl`.
        """

        return wx.Size(self._rect.width, self._rect.height)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        s = event.GetSize()
        self.SetTabRect(wx.Rect(0, 0, s.GetWidth(), s.GetHeight()))


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self.CaptureMouse()
        self._click_pt = wx.Point(-1, -1)
        self._is_dragging = False
        self._click_tab = None
        self._pressed_button = None

        wnd = self.TabHitTest(event.GetX(), event.GetY())

        if wnd is not None:
            new_selection = self.GetIdxFromWindow(wnd)

            # AuiNotebooks always want to receive this event
            # even if the tab is already active, because they may
            # have multiple tab controls
            if new_selection != self.GetActivePage() or isinstance(self.GetParent(), AuiNotebook):

                e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, self.GetId())
                e.SetSelection(new_selection)
                e.SetOldSelection(self.GetActivePage())
                e.SetEventObject(self)
                self.GetEventHandler().ProcessEvent(e)

            self._click_pt.x = event.GetX()
            self._click_pt.y = event.GetY()
            self._click_tab = wnd
        else:
            page_index = self.GetActivePage()
            if page_index != wx.NOT_FOUND:
                self.GetWindowFromIdx(page_index).SetFocus()

        if self._hover_button:
            self._pressed_button = self._hover_button
            self._pressed_button.cur_state = AUI_BUTTON_STATE_PRESSED
            self._on_button = True
            self.Refresh()
            self.Update()


    def OnCaptureLost(self, event):
        """
        Handles the ``wx.EVT_MOUSE_CAPTURE_LOST`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseCaptureLostEvent` event to be processed.
        """

        if self._is_dragging:
            self._is_dragging = False
            self._on_button = False

            if self._drag_image:
                self._drag_image.EndDrag()
                del self._drag_image
                self._drag_image = None

            event = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_CANCEL_DRAG, self.GetId())
            event.SetSelection(self.GetIdxFromWindow(self._click_tab))
            event.SetOldSelection(event.GetSelection())
            event.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(event)


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self._on_button = False

        if self._is_dragging:

            if self.HasCapture():
                self.ReleaseMouse()

            self._is_dragging = False
            if self._drag_image:
                self._drag_image.EndDrag()
                del self._drag_image
                self._drag_image = None
                self.GetParent().Refresh()

            evt = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_END_DRAG, self.GetId())
            evt.SetSelection(self.GetIdxFromWindow(self._click_tab))
            evt.SetOldSelection(evt.GetSelection())
            evt.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(evt)

            return

        self.GetParent()._mgr.HideHint()

        if self.HasCapture():
            self.ReleaseMouse()

        if self._hover_button:
            self._pressed_button = self._hover_button

        if self._pressed_button:

            # make sure we're still clicking the button
            button = self.ButtonHitTest(event.GetX(), event.GetY())

            if button is None:
                return

            if button != self._pressed_button:
                self._pressed_button = None
                return

            self.Refresh()
            self.Update()

            if self._pressed_button.cur_state & AUI_BUTTON_STATE_DISABLED == 0:

                evt = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BUTTON, self.GetId())
                evt.SetSelection(self.GetIdxFromWindow(self._click_tab))
                evt.SetInt(self._pressed_button.id)
                evt.SetEventObject(self)
                eventHandler = self.GetEventHandler()

                if eventHandler is not None:
                    eventHandler.ProcessEvent(evt)

            self._pressed_button = None

        self._click_pt = wx.Point(-1, -1)
        self._is_dragging = False
        self._click_tab = None


    def OnMiddleUp(self, event):
        """
        Handles the ``wx.EVT_MIDDLE_UP`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        eventHandler = self.GetEventHandler()
        if not isinstance(eventHandler, AuiTabCtrl):
            event.Skip()
            return

        x, y = event.GetX(), event.GetY()
        wnd = self.TabHitTest(x, y)

        if wnd:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP, self.GetId())
            e.SetEventObject(self)
            e.SetSelection(self.GetIdxFromWindow(wnd))
            self.GetEventHandler().ProcessEvent(e)
        elif not self.ButtonHitTest(x, y):
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BG_MIDDLE_UP, self.GetId())
            e.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(e)


    def OnMiddleDown(self, event):
        """
        Handles the ``wx.EVT_MIDDLE_DOWN`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        eventHandler = self.GetEventHandler()
        if not isinstance(eventHandler, AuiTabCtrl):
            event.Skip()
            return
        
        x, y = event.GetX(), event.GetY()
        wnd = self.TabHitTest(x, y)

        if wnd:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN, self.GetId())
            e.SetEventObject(self)
            e.SetSelection(self.GetIdxFromWindow(wnd))
            self.GetEventHandler().ProcessEvent(e)
        elif not self.ButtonHitTest(x, y):
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BG_MIDDLE_DOWN, self.GetId())
            e.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(e)


    def OnRightUp(self, event):
        """
        Handles the ``wx.EVT_RIGHT_UP`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        x, y = event.GetX(), event.GetY()
        wnd = self.TabHitTest(x, y)

        if wnd:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, self.GetId())
            e.SetEventObject(self)
            e.SetSelection(self.GetIdxFromWindow(wnd))
            self.GetEventHandler().ProcessEvent(e)
        elif not self.ButtonHitTest(x, y):
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BG_RIGHT_UP, self.GetId())
            e.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(e)


    def OnRightDown(self, event):
        """
        Handles the ``wx.EVT_RIGHT_DOWN`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        x, y = event.GetX(), event.GetY()
        wnd = self.TabHitTest(x, y)

        if wnd:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN, self.GetId())
            e.SetEventObject(self)
            e.SetSelection(self.GetIdxFromWindow(wnd))
            self.GetEventHandler().ProcessEvent(e)
        elif not self.ButtonHitTest(x, y):
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BG_RIGHT_DOWN, self.GetId())
            e.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(e)


    def OnLeftDClick(self, event):
        """
        Handles the ``wx.EVT_LEFT_DCLICK`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        x, y = event.GetX(), event.GetY()
        wnd = self.TabHitTest(x, y)

        if wnd:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_DCLICK, self.GetId())
            e.SetEventObject(self)
            e.SetSelection(self.GetIdxFromWindow(wnd))
            self.GetEventHandler().ProcessEvent(e)
        elif not self.ButtonHitTest(x, y):
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK, self.GetId())
            e.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(e)


    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        pos = event.GetPosition()

        # check if the mouse is hovering above a button

        button = self.ButtonHitTest(pos.x, pos.y)
        wnd = self.TabHitTest(pos.x, pos.y)

        if wnd is not None:
            mouse_tab = self.GetIdxFromWindow(wnd)
            if not self._pages[mouse_tab].enabled:
                self._hover_button = None
                return

        if self._on_button:
            return

        if button:

            if self._hover_button and button != self._hover_button:
                self._hover_button.cur_state = AUI_BUTTON_STATE_NORMAL
                self._hover_button = None
                self.Refresh()
                self.Update()

            if button.cur_state != AUI_BUTTON_STATE_HOVER:
                button.cur_state = AUI_BUTTON_STATE_HOVER
                self.Refresh()
                self.Update()
                self._hover_button = button
                return

        else:

            if self._hover_button:
                self._hover_button.cur_state = AUI_BUTTON_STATE_NORMAL
                self._hover_button = None
                self.Refresh()
                self.Update()

        if not event.LeftIsDown() or self._click_pt == wx.Point(-1, -1):
            return

        if not self.HasCapture():
            return

        wnd = self.TabHitTest(pos.x, pos.y)

        if not self._is_dragging:

            drag_x_threshold = wx.SystemSettings.GetMetric(wx.SYS_DRAG_X)
            drag_y_threshold = wx.SystemSettings.GetMetric(wx.SYS_DRAG_Y)

            if abs(pos.x - self._click_pt.x) > drag_x_threshold or \
               abs(pos.y - self._click_pt.y) > drag_y_threshold:

                self._is_dragging = True

                if self._drag_image:
                    self._drag_image.EndDrag()
                    del self._drag_image
                    self._drag_image = None

                if self._agwFlags & AUI_NB_DRAW_DND_TAB:
                    # Create the custom draw image from the icons and the text of the item
                    mouse_tab = self.GetIdxFromWindow(wnd)
                    page = self._pages[mouse_tab]
                    tab_button = self._tab_close_buttons[mouse_tab]
                    self._drag_image = TabDragImage(self, page, tab_button.cur_state, self._art)

                    if self._agwFlags & AUI_NB_TAB_FLOAT:
                        self._drag_image.BeginDrag(wx.Point(0,0), self, fullScreen=True)
                    else:
                        self._drag_image.BeginDragBounded(wx.Point(0,0), self, self.GetParent())

                    # Capture the mouse cursor position offset relative to
                    # The tab image location
                    self._drag_img_offset = (pos[0] - page.rect.x,
                                             pos[1] - page.rect.y)

                    self._drag_image.Show()

        if not wnd:
            evt2 = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BEGIN_DRAG, self.GetId())
            evt2.SetSelection(self.GetIdxFromWindow(self._click_tab))
            evt2.SetOldSelection(evt2.GetSelection())
            evt2.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(evt2)
            if evt2.GetDispatched():
                return

        evt3 = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_DRAG_MOTION, self.GetId())
        evt3.SetSelection(self.GetIdxFromWindow(self._click_tab))
        evt3.SetOldSelection(evt3.GetSelection())
        evt3.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(evt3)

        if self._drag_image:
            # Apply the drag images offset
            pos -= self._drag_img_offset
            self._drag_image.Move(pos)


    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self._hover_button:
            self._hover_button.cur_state = AUI_BUTTON_STATE_NORMAL
            self._hover_button = None
            self.Refresh()
            self.Update()


    def OnButton(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_BUTTON`` event for L{AuiTabCtrl}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        button = event.GetInt()

        if button == AUI_BUTTON_LEFT or button == AUI_BUTTON_RIGHT:
            if button == AUI_BUTTON_LEFT:
                if self.GetTabOffset() > 0:

                    self.SetTabOffset(self.GetTabOffset()-1)
                    self.Refresh()
                    self.Update()
            else:
                self.SetTabOffset(self.GetTabOffset()+1)
                self.Refresh()
                self.Update()

        elif button == AUI_BUTTON_WINDOWLIST:
            idx = self.GetArtProvider().ShowDropDown(self, self._pages, self.GetActivePage())

            if idx != -1:

                e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, self.GetId())
                e.SetSelection(idx)
                e.SetOldSelection(self.GetActivePage())
                e.SetEventObject(self)
                self.GetEventHandler().ProcessEvent(e)

        else:
            event.Skip()


    def OnSetFocus(self, event):
        """
        Handles the ``wx.EVT_SET_FOCUS`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self.Refresh()


    def OnKillFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self.Refresh()


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        key = event.GetKeyCode()
        nb = self.GetParent()

        if key == wx.WXK_LEFT:
            nb.AdvanceSelection(False)
            self.SetFocus()

        elif key == wx.WXK_RIGHT:
            nb.AdvanceSelection(True)
            self.SetFocus()

        elif key == wx.WXK_HOME:
            newPage = 0
            nb.SetSelection(newPage)
            self.SetFocus()

        elif key == wx.WXK_END:
            newPage = nb.GetPageCount() - 1
            nb.SetSelection(newPage)
            self.SetFocus()

        elif key == wx.WXK_TAB:
            if not event.ControlDown():
                flags = 0
                if not event.ShiftDown(): flags |= wx.NavigationKeyEvent.IsForward
                if event.CmdDown():       flags |= wx.NavigationKeyEvent.WinChange
                self.Navigate(flags)
            else:

                if not nb or not isinstance(nb, AuiNotebook):
                    event.Skip()
                    return

                bForward = bWindowChange = 0
                if not event.ShiftDown(): bForward |= wx.NavigationKeyEvent.IsForward
                if event.CmdDown():       bWindowChange |= wx.NavigationKeyEvent.WinChange

                keyEvent = wx.NavigationKeyEvent()
                keyEvent.SetDirection(bForward)
                keyEvent.SetWindowChange(bWindowChange)
                keyEvent.SetFromTab(True)
                keyEvent.SetEventObject(nb)

                if not nb.GetEventHandler().ProcessEvent(keyEvent):

                    # Not processed? Do an explicit tab into the page.
                    win = self.GetWindowFromIdx(self.GetActivePage())
                    if win:
                        win.SetFocus()

                self.SetFocus()

                return

        else:
            event.Skip()


    def OnKeyDown2(self, event):
        """
        Deprecated.

        Handles the ``wx.EVT_KEY_DOWN`` event for L{AuiTabCtrl}.

        :param `event`: a `wx.KeyEvent` event to be processed.

        :warning: This method implementation is now deprecated. Refer to L{OnKeyDown}
         for the correct one.
        """

        if self.GetActivePage() == -1:
            event.Skip()
            return

        # We can't leave tab processing to the system on Windows, tabs and keys
        # get eaten by the system and not processed properly if we specify both
        # wxTAB_TRAVERSAL and wxWANTS_CHARS. And if we specify just wxTAB_TRAVERSAL,
        # we don't key arrow key events.

        key = event.GetKeyCode()

        if key == wx.WXK_NUMPAD_PAGEUP:
            key = wx.WXK_PAGEUP
        if key == wx.WXK_NUMPAD_PAGEDOWN:
            key = wx.WXK_PAGEDOWN
        if key == wx.WXK_NUMPAD_HOME:
            key = wx.WXK_HOME
        if key == wx.WXK_NUMPAD_END:
            key = wx.WXK_END
        if key == wx.WXK_NUMPAD_LEFT:
            key = wx.WXK_LEFT
        if key == wx.WXK_NUMPAD_RIGHT:
            key = wx.WXK_RIGHT

        if key == wx.WXK_TAB or key == wx.WXK_PAGEUP or key == wx.WXK_PAGEDOWN:

            bCtrlDown = event.ControlDown()
            bShiftDown = event.ShiftDown()

            bForward = (key == wx.WXK_TAB and not bShiftDown) or (key == wx.WXK_PAGEDOWN)
            bWindowChange = (key == wx.WXK_PAGEUP) or (key == wx.WXK_PAGEDOWN) or bCtrlDown
            bFromTab = (key == wx.WXK_TAB)

            nb = self.GetParent()
            if not nb or not isinstance(nb, AuiNotebook):
                event.Skip()
                return

            keyEvent = wx.NavigationKeyEvent()
            keyEvent.SetDirection(bForward)
            keyEvent.SetWindowChange(bWindowChange)
            keyEvent.SetFromTab(bFromTab)
            keyEvent.SetEventObject(nb)

            if not nb.GetEventHandler().ProcessEvent(keyEvent):

                # Not processed? Do an explicit tab into the page.
                win = self.GetWindowFromIdx(self.GetActivePage())
                if win:
                    win.SetFocus()

            return

        if len(self._pages) < 2:
            event.Skip()
            return

        newPage = -1

        if self.GetLayoutDirection() == wx.Layout_RightToLeft:
            forwardKey = wx.WXK_LEFT
            backwardKey = wx.WXK_RIGHT
        else:
            forwardKey = wx.WXK_RIGHT
            backwardKey = wx.WXK_LEFT

        if key == forwardKey:
            if self.GetActivePage() == -1:
                newPage = 0
            elif self.GetActivePage() < len(self._pages) - 1:
                newPage = self.GetActivePage() + 1

        elif key == backwardKey:
            if self.GetActivePage() == -1:
                newPage = len(self._pages) - 1
            elif self.GetActivePage() > 0:
                newPage = self.GetActivePage() - 1

        elif key == wx.WXK_HOME:
            newPage = 0

        elif key == wx.WXK_END:
            newPage = len(self._pages) - 1

        else:
            event.Skip()

        if newPage != -1:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, self.GetId())
            e.SetSelection(newPage)
            e.SetOldSelection(newPage)
            e.SetEventObject(self)
            self.GetEventHandler().ProcessEvent(e)

        else:
            event.Skip()


# ----------------------------------------------------------------------

class TabFrame(wx.PyWindow):
    """
    TabFrame is an interesting case. It's important that all child pages
    of the multi-notebook control are all actually children of that control
    (and not grandchildren). TabFrame facilitates this. There is one
    instance of TabFrame for each tab control inside the multi-notebook.

    It's important to know that TabFrame is not a real window, but it merely
    used to capture the dimensions/positioning of the internal tab control and
    it's managed page windows.
    """

    def __init__(self, parent):
        """
        Default class constructor.
        Used internally, do not call it in your code!
        """

        pre = wx.PrePyWindow()

        self._tabs = None
        self._rect = wx.Rect(0, 0, 200, 200)
        self._tab_ctrl_height = 20
        self._tab_rect = wx.Rect()
        self._parent = parent

        self.PostCreate(pre)


    def SetTabCtrlHeight(self, h):
        """
        Sets the tab control height.

        :param `h`: the tab area height.
        """

        self._tab_ctrl_height = h


    def DoSetSize(self, x, y, width, height, flags=wx.SIZE_AUTO):
        """
        Sets the position and size of the window in pixels. The `flags`
        parameter indicates the interpretation of the other params if they are
        equal to -1.

        :param `x`: the window `x` position;
        :param `y`: the window `y` position;
        :param `width`: the window width;
        :param `height`: the window height;
        :param `flags`: may have one of this bit set:

         ===================================  ======================================
         Size Flags                           Description
         ===================================  ======================================
         ``wx.SIZE_AUTO``                     A -1 indicates that a class-specific default should be used.
         ``wx.SIZE_AUTO_WIDTH``               A -1 indicates that a class-specific default should be used for the width.
         ``wx.SIZE_AUTO_HEIGHT``              A -1 indicates that a class-specific default should be used for the height.
         ``wx.SIZE_USE_EXISTING``             Existing dimensions should be used if -1 values are supplied.
         ``wx.SIZE_ALLOW_MINUS_ONE``          Allow dimensions of -1 and less to be interpreted as real dimensions, not default values.
         ``wx.SIZE_FORCE``                    Normally, if the position and the size of the window are already the same as the parameters of this function, nothing is done. but with this flag a window resize may be forced even in this case (supported in wx 2.6.2 and later and only implemented for MSW and ignored elsewhere currently)
         ===================================  ======================================

        :note: Overridden from `wx.PyControl`.
        """

        self._rect = wx.Rect(x, y, max(1, width), max(1, height))
        self.DoSizing()


    def DoGetSize(self):
        """
        Returns the window size.

        :note: Overridden from `wx.PyControl`.
        """

        return self._rect.width, self._rect.height


    def DoGetClientSize(self):
        """
        Returns the window client size.

        :note: Overridden from `wx.PyControl`.
        """

        return self._rect.width, self._rect.height


    def Show(self, show=True):
        """
        Shows/hides the window.

        :param `show`: ``True`` to show the window, ``False`` otherwise.

        :note: Overridden from `wx.PyControl`, this method always returns ``False`` as
         L{TabFrame} should never be phisically shown on screen.
        """

        return False


    def DoSizing(self):
        """ Does the actual sizing of the tab control. """

        if not self._tabs:
            return

        hideOnSingle = ((self._tabs.GetAGWFlags() & AUI_NB_HIDE_ON_SINGLE_TAB) and \
                        self._tabs.GetPageCount() <= 1)

        if not hideOnSingle and not self._parent._hide_tabs:
            tab_height = self._tab_ctrl_height

            self._tab_rect = wx.Rect(self._rect.x, self._rect.y, self._rect.width, self._tab_ctrl_height)

            if self._tabs.GetAGWFlags() & AUI_NB_BOTTOM:
                self._tab_rect = wx.Rect(self._rect.x, self._rect.y + self._rect.height - tab_height,
                                         self._rect.width, tab_height)
                self._tabs.SetDimensions(self._rect.x, self._rect.y + self._rect.height - tab_height,
                                         self._rect.width, tab_height)
                self._tabs.SetTabRect(wx.Rect(0, 0, self._rect.width, tab_height))

            else:

                self._tab_rect = wx.Rect(self._rect.x, self._rect.y, self._rect.width, tab_height)
                self._tabs.SetDimensions(self._rect.x, self._rect.y, self._rect.width, tab_height)
                self._tabs.SetTabRect(wx.Rect(0, 0, self._rect.width, tab_height))

            # TODO: elif (GetAGWFlags() & AUI_NB_LEFT)
            # TODO: elif (GetAGWFlags() & AUI_NB_RIGHT)

            self._tabs.Refresh()
            self._tabs.Update()

        else:

            tab_height = 0
            self._tabs.SetDimensions(self._rect.x, self._rect.y, self._rect.width, tab_height)
            self._tabs.SetTabRect(wx.Rect(0, 0, self._rect.width, tab_height))

        pages = self._tabs.GetPages()

        for page in pages:

            height = self._rect.height - tab_height

            if height < 0:
                # avoid passing negative height to wx.Window.SetSize(), this
                # results in assert failures/GTK+ warnings
                height = 0

            if self._tabs.GetAGWFlags() & AUI_NB_BOTTOM:
                page.window.SetDimensions(self._rect.x, self._rect.y, self._rect.width, height)

            else:
                page.window.SetDimensions(self._rect.x, self._rect.y + tab_height,
                                          self._rect.width, height)

            # TODO: elif (GetAGWFlags() & AUI_NB_LEFT)
            # TODO: elif (GetAGWFlags() & AUI_NB_RIGHT)

            if repr(page.window.__class__).find("AuiMDIChildFrame") >= 0:
                page.window.ApplyMDIChildFrameRect()


    def Update(self):
        """
        Calling this method immediately repaints the invalidated area of the window
        and all of its children recursively while this would usually only happen when
        the flow of control returns to the event loop.

        :note: Notice that this function doesn't invalidate any area of the window so
         nothing happens if nothing has been invalidated (i.e. marked as requiring a redraw).
         Use `Refresh` first if you want to immediately redraw the window unconditionally.

        :note: Overridden from `wx.PyControl`.
        """

        # does nothing
        pass


# ----------------------------------------------------------------------
# -- AuiNotebook class implementation --

class AuiNotebook(wx.PyPanel):
    """
    AuiNotebook is a notebook control which implements many features common in
    applications with dockable panes. Specifically, AuiNotebook implements functionality
    which allows the user to rearrange tab order via drag-and-drop, split the tab window
    into many different splitter configurations, and toggle through different themes to
    customize the control's look and feel.

    An effort has been made to try to maintain an API as similar to that of `wx.Notebook`.

    The default theme that is used is L{AuiDefaultTabArt}, which provides a modern, glossy
    look and feel. The theme can be changed by calling L{AuiNotebook.SetArtProvider}.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=AUI_NB_DEFAULT_STYLE):
        """
        Default class constructor.

        :param `parent`: the L{AuiNotebook} parent;
        :param `id`: an identifier for the control: a value of -1 is taken to mean a default;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyPanel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the following bits:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet.
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet.
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by L{AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ``AUI_NB_ORDER_BY_ACCESS``           Tab navigation order by last access time for the tabs
         ``AUI_NB_NO_TAB_FOCUS``              Don't draw tab focus rectangle
         ==================================== ==================================

         Default value for `agwStyle` is:
         ``AUI_NB_DEFAULT_STYLE`` = ``AUI_NB_TOP`` | ``AUI_NB_TAB_SPLIT`` | ``AUI_NB_TAB_MOVE`` | ``AUI_NB_SCROLL_BUTTONS`` | ``AUI_NB_CLOSE_ON_ACTIVE_TAB`` | ``AUI_NB_MIDDLE_CLICK_CLOSE`` | ``AUI_NB_DRAW_DND_TAB``

        """

        self._curpage = -1
        self._tab_id_counter = AuiBaseTabCtrlId
        self._dummy_wnd = None
        self._hide_tabs = False
        self._sash_dclick_unsplit = False
        self._tab_ctrl_height = 20
        self._requested_bmp_size = wx.Size(-1, -1)
        self._requested_tabctrl_height = -1
        self._textCtrl = None
        self._tabBounds = (-1, -1)
        self._click_tab = None

        wx.PyPanel.__init__(self, parent, id, pos, size, style|wx.BORDER_NONE|wx.TAB_TRAVERSAL)
        self._mgr = framemanager.AuiManager()
        self._tabs = AuiTabContainer(self)

        self.InitNotebook(agwStyle)


    def GetTabContainer(self):
        """ Returns the instance of L{AuiTabContainer}. """

        return self._tabs


    def InitNotebook(self, agwStyle):
        """
        Contains common initialization code called by all constructors.

        :param `agwStyle`: the notebook style.

        :see: L{__init__}
        """

        self.SetName("AuiNotebook")
        self._agwFlags = agwStyle

        self._popupWin = None
        self._naviIcon = None
        self._imageList = None
        self._last_drag_x = 0

        self._normal_font = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self._selected_font = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self._selected_font.SetWeight(wx.BOLD)

        self.SetArtProvider(TA.AuiDefaultTabArt())

        self._dummy_wnd = wx.Window(self, wx.ID_ANY, wx.Point(0, 0), wx.Size(0, 0))
        self._dummy_wnd.SetSize((200, 200))
        self._dummy_wnd.Show(False)

        self._mgr.SetManagedWindow(self)
        self._mgr.SetAGWFlags(AUI_MGR_DEFAULT)
        self._mgr.SetDockSizeConstraint(1.0, 1.0) # no dock size constraint

        self._mgr.AddPane(self._dummy_wnd, framemanager.AuiPaneInfo().Name("dummy").Bottom().CaptionVisible(False).Show(False))
        self._mgr.Update()

        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_CHILD_FOCUS, self.OnChildFocusNotebook)
        self.Bind(EVT_AUINOTEBOOK_PAGE_CHANGING, self.OnTabClicked,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_BEGIN_DRAG, self.OnTabBeginDrag,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_END_DRAG, self.OnTabEndDrag,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_DRAG_MOTION, self.OnTabDragMotion,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_CANCEL_DRAG, self.OnTabCancelDrag,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_BUTTON, self.OnTabButton,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_TAB_MIDDLE_DOWN, self.OnTabMiddleDown,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_TAB_MIDDLE_UP, self.OnTabMiddleUp,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_TAB_RIGHT_DOWN, self.OnTabRightDown,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_TAB_RIGHT_UP, self.OnTabRightUp,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_BG_DCLICK, self.OnTabBgDClick,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)
        self.Bind(EVT_AUINOTEBOOK_TAB_DCLICK, self.OnTabDClick,
                  id=AuiBaseTabCtrlId, id2=AuiBaseTabCtrlId+500)

        self.Bind(wx.EVT_NAVIGATION_KEY, self.OnNavigationKeyNotebook)


    def SetArtProvider(self, art):
        """
        Sets the art provider to be used by the notebook.

        :param `art`: an art provider.
        """

        self._tabs.SetArtProvider(art)
        self.UpdateTabCtrlHeight(force=True)


    def SavePerspective(self):
        """
        Saves the entire user interface layout into an encoded string, which can then
        be stored by the application (probably using `wx.Config`). When a perspective
        is restored using L{LoadPerspective}, the entire user interface will return
        to the state it was when the perspective was saved.
        """

        # Build list of panes/tabs
        tabs = ""
        all_panes = self._mgr.GetAllPanes()

        for pane in all_panes:

            if pane.name == "dummy":
                continue

            tabframe = pane.window

            if tabs:
                tabs += "|"

            tabs += pane.name + "="

            # add tab id's
            page_count = tabframe._tabs.GetPageCount()

            for p in xrange(page_count):

                page = tabframe._tabs.GetPage(p)
                page_idx = self._tabs.GetIdxFromWindow(page.window)

                if p:
                    tabs += ","

                if p == tabframe._tabs.GetActivePage():
                    tabs += "+"
                elif page_idx == self._curpage:
                    tabs += "*"

                tabs += "%u"%page_idx

        tabs += "@"

        # Add frame perspective
        tabs += self._mgr.SavePerspective()

        return tabs


    def LoadPerspective(self, layout):
        """
        Loads a layout which was saved with L{SavePerspective}.

        :param `layout`: a string which contains a saved L{AuiNotebook} layout.
        """

        # Remove all tab ctrls (but still keep them in main index)
        tab_count = self._tabs.GetPageCount()
        for i in xrange(tab_count):
            wnd = self._tabs.GetWindowFromIdx(i)

            # find out which onscreen tab ctrl owns this tab
            ctrl, ctrl_idx = self.FindTab(wnd)
            if not ctrl:
                return False

            # remove the tab from ctrl
            if not ctrl.RemovePage(wnd):
                return False

        self.RemoveEmptyTabFrames()

        sel_page = 0
        tabs = layout[0:layout.index("@")]
        to_break1 = False

        while 1:

            if "|" not in tabs:
                to_break1 = True
                tab_part = tabs
            else:
                tab_part = tabs[0:tabs.index('|')]

            if "=" not in tab_part:
                # No pages in this perspective...
                return False

            # Get pane name
            pane_name = tab_part[0:tab_part.index("=")]

            # create a new tab frame
            new_tabs = TabFrame(self)
            self._tab_id_counter += 1
            new_tabs._tabs = AuiTabCtrl(self, self._tab_id_counter)
            new_tabs._tabs.SetArtProvider(self._tabs.GetArtProvider().Clone())
            new_tabs.SetTabCtrlHeight(self._tab_ctrl_height)
            new_tabs._tabs.SetAGWFlags(self._agwFlags)
            dest_tabs = new_tabs._tabs

            # create a pane info structure with the information
            # about where the pane should be added
            pane_info = framemanager.AuiPaneInfo().Name(pane_name).Bottom().CaptionVisible(False)
            self._mgr.AddPane(new_tabs, pane_info)

            # Get list of tab id's and move them to pane
            tab_list = tab_part[tab_part.index("=")+1:]
            to_break2, active_found = False, False

            while 1:
                if "," not in tab_list:
                    to_break2 = True
                    tab = tab_list
                else:
                    tab = tab_list[0:tab_list.index(",")]
                    tab_list = tab_list[tab_list.index(",")+1:]

                # Check if this page has an 'active' marker
                c = tab[0]
                if c in ['+', '*']:
                    tab = tab[1:]

                tab_idx = int(tab)
                if tab_idx >= self.GetPageCount():
                    to_break1 = True
                    break

                # Move tab to pane
                page = self._tabs.GetPage(tab_idx)
                newpage_idx = dest_tabs.GetPageCount()
                dest_tabs.InsertPage(page.window, page, newpage_idx)

                if c == '+':
                    dest_tabs.SetActivePage(newpage_idx)
                    active_found = True
                elif c == '*':
                    sel_page = tab_idx

                if to_break2:
                    break

            if not active_found:
                dest_tabs.SetActivePage(0)

            new_tabs.DoSizing()
            dest_tabs.DoShowHide()
            dest_tabs.Refresh()

            if to_break1:
                break

            tabs = tabs[tabs.index('|')+1:]

        # Load the frame perspective
        frames = layout[layout.index('@')+1:]
        self._mgr.LoadPerspective(frames)

        # Force refresh of selection
        self._curpage = -1
        self.SetSelection(sel_page)

        return True


    def SetTabCtrlHeight(self, height):
        """
        Sets the tab height.

        By default, the tab control height is calculated by measuring the text
        height and bitmap sizes on the tab captions.

        Calling this method will override that calculation and set the tab control
        to the specified height parameter. A call to this method will override
        any call to L{SetUniformBitmapSize}. Specifying -1 as the height will
        return the control to its default auto-sizing behaviour.

        :param `height`: the tab control area height.
        """

        self._requested_tabctrl_height = height

        # if window is already initialized, recalculate the tab height
        if self._dummy_wnd:
            self.UpdateTabCtrlHeight()


    def SetUniformBitmapSize(self, size):
        """
        Ensures that all tabs will have the same height, even if some tabs
        don't have bitmaps. Passing ``wx.DefaultSize`` to this
        function will instruct the control to use dynamic tab height, which is
        the default behaviour. Under the default behaviour, when a tab with a
        large bitmap is added, the tab control's height will automatically
        increase to accommodate the larger bitmap.

        :param `size`: an instance of `wx.Size` specifying the tab bitmap size.
        """

        self._requested_bmp_size = wx.Size(*size)

        # if window is already initialized, recalculate the tab height
        if self._dummy_wnd:
            self.UpdateTabCtrlHeight()


    def UpdateTabCtrlHeight(self, force=False):
        """
        UpdateTabCtrlHeight() does the actual tab resizing. It's meant
        to be used interally.

        :param `force`: ``True`` to force the tab art to repaint.
        """

        # get the tab ctrl height we will use
        height = self.CalculateTabCtrlHeight()

        # if the tab control height needs to change, update
        # all of our tab controls with the new height
        if self._tab_ctrl_height != height or force:
            art = self._tabs.GetArtProvider()

            self._tab_ctrl_height = height

            all_panes = self._mgr.GetAllPanes()
            for pane in all_panes:

                if pane.name == "dummy":
                    continue

                tab_frame = pane.window
                tabctrl = tab_frame._tabs
                tab_frame.SetTabCtrlHeight(self._tab_ctrl_height)
                tabctrl.SetArtProvider(art.Clone())
                tab_frame.DoSizing()


    def UpdateHintWindowSize(self):
        """ Updates the L{AuiManager} hint window size. """

        size = self.CalculateNewSplitSize()

        # the placeholder hint window should be set to this size
        info = self._mgr.GetPane("dummy")

        if info.IsOk():
            info.MinSize(size)
            info.BestSize(size)
            self._dummy_wnd.SetSize(size)


    def CalculateNewSplitSize(self):
        """ Calculates the size of the new split. """

        # count number of tab controls
        tab_ctrl_count = 0
        all_panes = self._mgr.GetAllPanes()

        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tab_ctrl_count += 1

        # if there is only one tab control, the first split
        # should happen around the middle
        if tab_ctrl_count < 2:
            new_split_size = self.GetClientSize()
            new_split_size.x /= 2
            new_split_size.y /= 2

        else:

            # this is in place of a more complicated calculation
            # that needs to be implemented
            new_split_size = wx.Size(180, 180)

        return new_split_size


    def CalculateTabCtrlHeight(self):
        """ Calculates the tab control area height. """

        # if a fixed tab ctrl height is specified,
        # just return that instead of calculating a
        # tab height
        if self._requested_tabctrl_height != -1:
            return self._requested_tabctrl_height

        # find out new best tab height
        art = self._tabs.GetArtProvider()

        return art.GetBestTabCtrlSize(self, self._tabs.GetPages(), self._requested_bmp_size)


    def GetArtProvider(self):
        """ Returns the associated art provider. """

        return self._tabs.GetArtProvider()


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the AGW-specific style of the window.

        :param `agwStyle`: the new window style. This can be a combination of the following bits:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet.
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet.
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by L{AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ``AUI_NB_ORDER_BY_ACCESS``           Tab navigation order by last access time for the tabs
         ``AUI_NB_NO_TAB_FOCUS``              Don't draw tab focus rectangle
         ==================================== ==================================

        :note: Please note that some styles cannot be changed after the window
         creation and that `Refresh` might need to be be called after changing the
         others for the change to take place immediately.

        :todo: Implementation of flags ``AUI_NB_RIGHT`` and ``AUI_NB_LEFT``.
        """

        self._agwFlags = agwStyle

        # if the control is already initialized
        if self._mgr.GetManagedWindow() == self:

            # let all of the tab children know about the new style

            all_panes = self._mgr.GetAllPanes()
            for pane in all_panes:
                if pane.name == "dummy":
                    continue

                tabframe = pane.window
                tabctrl = tabframe._tabs
                tabctrl.SetAGWFlags(self._agwFlags)
                tabframe.DoSizing()
                tabctrl.Refresh()
                tabctrl.Update()


    def GetAGWWindowStyleFlag(self):
        """
        Returns the AGW-specific style of the window.

        :see: L{SetAGWWindowStyleFlag} for a list of possible AGW-specific window styles.
        """

        return self._agwFlags


    def AddPage(self, page, caption, select=False, bitmap=wx.NullBitmap, disabled_bitmap=wx.NullBitmap, control=None):
        """
        Adds a page. If the `select` parameter is ``True``, calling this will generate a
        page change event.

        :param `page`: the page to be added;
        :param `caption`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `bitmap`: the `wx.Bitmap` to display in the enabled tab;
        :param `disabled_bitmap`: the `wx.Bitmap` to display in the disabled tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """

        return self.InsertPage(self.GetPageCount(), page, caption, select, bitmap, disabled_bitmap, control)


    def InsertPage(self, page_idx, page, caption, select=False, bitmap=wx.NullBitmap, disabled_bitmap=wx.NullBitmap,
                   control=None):
        """
        This is similar to L{AddPage}, but allows the ability to specify the insert location.

        :param `page_idx`: specifies the position for the new page;
        :param `page`: the page to be added;
        :param `caption`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `bitmap`: the `wx.Bitmap` to display in the enabled tab;
        :param `disabled_bitmap`: the `wx.Bitmap` to display in the disabled tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """

        if not page:
            return False

        page.Reparent(self)
        info = AuiNotebookPage()
        info.window = page
        info.caption = caption
        info.bitmap = bitmap
        info.active = False
        info.control = control

        originalPaneMgr = framemanager.GetManager(page)
        if originalPaneMgr:
            originalPane = originalPaneMgr.GetPane(page)

            if originalPane:
                info.hasCloseButton = originalPane.HasCloseButton()

        if bitmap.IsOk() and not disabled_bitmap.IsOk():
            disabled_bitmap = MakeDisabledBitmap(bitmap)
            info.dis_bitmap = disabled_bitmap

        # if there are currently no tabs, the first added
        # tab must be active
        if self._tabs.GetPageCount() == 0:
            info.active = True

        self._tabs.InsertPage(page, info, page_idx)

        # if that was the first page added, even if
        # select is False, it must become the "current page"
        # (though no select events will be fired)
        if not select and self._tabs.GetPageCount() == 1:
            select = True

        active_tabctrl = self.GetActiveTabCtrl()
        if page_idx >= active_tabctrl.GetPageCount():
            active_tabctrl.AddPage(page, info)
        else:
            active_tabctrl.InsertPage(page, info, page_idx)

        force = False
        if control:
            force = True
            control.Reparent(active_tabctrl)
            control.Show()

        self.UpdateTabCtrlHeight(force=force)
        self.DoSizing()
        active_tabctrl.DoShowHide()

        # adjust selected index
        if self._curpage >= page_idx:
            self._curpage += 1

        if select:
            self.SetSelectionToWindow(page)

        return True


    def DeletePage(self, page_idx):
        """
        Deletes a page at the given index. Calling this method will generate a page
        change event.

        :param `page_idx`: the page index to be deleted.

        :note: L{DeletePage} removes a tab from the multi-notebook, and destroys the window as well.

        :see: L{RemovePage}
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        wnd = self._tabs.GetWindowFromIdx(page_idx)
        # hide the window in advance, as this will
        # prevent flicker
        wnd.Show(False)

        self.RemoveControlFromPage(page_idx)

        if not self.RemovePage(page_idx):
            return False

        wnd.Destroy()

        return True


    def RemovePage(self, page_idx):
        """
        Removes a page, without deleting the window pointer.

        :param `page_idx`: the page index to be removed.

        :note: L{RemovePage} removes a tab from the multi-notebook, but does not destroy the window.

        :see: L{DeletePage}
        """

        # save active window pointer
        active_wnd = None
        if self._curpage >= 0:
            active_wnd = self._tabs.GetWindowFromIdx(self._curpage)

        # save pointer of window being deleted
        wnd = self._tabs.GetWindowFromIdx(page_idx)
        new_active = None

        # make sure we found the page
        if not wnd:
            return False

        # find out which onscreen tab ctrl owns this tab
        ctrl, ctrl_idx = self.FindTab(wnd)
        if not ctrl:
            return False

        currentPage = ctrl.GetPage(ctrl_idx)
        is_curpage = (self._curpage == page_idx)
        is_active_in_split = currentPage.active

        # remove the tab from main catalog
        if not self._tabs.RemovePage(wnd):
            return False

        # remove the tab from the onscreen tab ctrl
        ctrl.RemovePage(wnd)

        if is_active_in_split:

            ctrl_new_page_count = ctrl.GetPageCount()

            if ctrl_idx >= ctrl_new_page_count:
                ctrl_idx = ctrl_new_page_count - 1

            if ctrl_idx >= 0 and ctrl_idx < ctrl.GetPageCount():

                ctrl_idx = self.FindNextActiveTab(ctrl_idx, ctrl)

                # set new page as active in the tab split
                ctrl.SetActivePage(ctrl_idx)

                # if the page deleted was the current page for the
                # entire tab control, then record the window
                # pointer of the new active page for activation
                if is_curpage:
                    new_active = ctrl.GetWindowFromIdx(ctrl_idx)

        else:

            # we are not deleting the active page, so keep it the same
            new_active = active_wnd

        if not new_active:

            # we haven't yet found a new page to active,
            # so select the next page from the main tab
            # catalogue

            if 0 <= page_idx < self._tabs.GetPageCount():
                new_active = self._tabs.GetPage(page_idx).window
            if not new_active and self._tabs.GetPageCount() > 0:
                new_active = self._tabs.GetPage(0).window

        self.RemoveEmptyTabFrames()

        # set new active pane
        if new_active:
            if not self.IsBeingDeleted():
                self._curpage = -1
                self.SetSelectionToWindow(new_active)
        else:
            self._curpage = -1
            self._tabs.SetNoneActive()

        return True


    def FindNextActiveTab(self, ctrl_idx, ctrl):
        """
        Finds the next active tab (used mainly when L{AuiNotebook} has inactive/disabled
        tabs in it).

        :param `ctrl_idx`: the index of the first (most obvious) tab to check for active status;
        :param `ctrl`: an instance of L{AuiTabCtrl}.
        """

        if self.GetEnabled(ctrl_idx):
            return ctrl_idx

        for indx in xrange(ctrl_idx, ctrl.GetPageCount()):
            if self.GetEnabled(indx):
                return indx

        for indx in xrange(ctrl_idx, -1, -1):
            if self.GetEnabled(indx):
                return indx

        return 0


    def HideAllTabs(self, hidden=True):
        """
        Hides all tabs on the L{AuiNotebook} control.

        :param `hidden`: if ``True`` hides all tabs.
        """

        self._hide_tabs = hidden


    def SetSashDClickUnsplit(self, unsplit=True):
        """
        Sets whether to unsplit a splitted L{AuiNotebook} when double-clicking on a sash.

        :param `unsplit`: ``True`` to unsplit on sash double-clicking, ``False`` otherwise.
        """

        self._sash_dclick_unsplit = unsplit


    def GetSashDClickUnsplit(self):
        """
        Returns whether a splitted L{AuiNotebook} can be unsplitted by double-clicking
        on the splitter sash.
        """

        return self._sash_dclick_unsplit


    def SetMinMaxTabWidth(self, minTabWidth, maxTabWidth):
        """
        Sets the minimum and/or the maximum tab widths for L{AuiNotebook} when the
        ``AUI_NB_TAB_FIXED_WIDTH`` style is defined.

        Pass -1 to either `minTabWidth` or `maxTabWidth` to reset to the default tab
        width behaviour for L{AuiNotebook}.

        :param `minTabWidth`: the minimum allowed tab width, in pixels;
        :param `maxTabWidth`: the maximum allowed tab width, in pixels.

        :note: Minimum and maximum tabs widths are used only when the ``AUI_NB_TAB_FIXED_WIDTH``
         style is present.
        """

        if minTabWidth > maxTabWidth:
            raise Exception("Minimum tab width must be less or equal than maximum tab width")

        self._tabBounds = (minTabWidth, maxTabWidth)
        self.SetAGWWindowStyleFlag(self._agwFlags)


    def GetMinMaxTabWidth(self):
        """
        Returns the minimum and the maximum tab widths for L{AuiNotebook} when the
        ``AUI_NB_TAB_FIXED_WIDTH`` style is defined.

        :note: Minimum and maximum tabs widths are used only when the ``AUI_NB_TAB_FIXED_WIDTH``
         style is present.

        :see: L{SetMinMaxTabWidth} for more information.
        """

        return self._tabBounds


    def GetPageIndex(self, page_wnd):
        """
        Returns the page index for the specified window. If the window is not
        found in the notebook, ``wx.NOT_FOUND`` is returned.
        """

        return self._tabs.GetIdxFromWindow(page_wnd)


    def SetPageText(self, page_idx, text):
        """
        Sets the tab label for the page.

        :param `page_idx`: the page index;
        :param `text`: the new tab label.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        should_refresh = page_info.caption != text
        page_info.caption = text

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        info = ctrl.GetPage(ctrl_idx)
        should_refresh = should_refresh or info.caption != text
        info.caption = text

        if should_refresh:
            ctrl.Refresh()
            ctrl.Update()

        self.UpdateTabCtrlHeight(force=True)

        return True


    def GetPageText(self, page_idx):
        """
        Returns the tab label for the page.

        :param `page_idx`: the page index.
        """

        if page_idx >= self._tabs.GetPageCount():
            return ""

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        return page_info.caption


    def SetPageBitmap(self, page_idx, bitmap):
        """
        Sets the tab bitmap for the page.

        :param `page_idx`: the page index;
        :param `bitmap`: an instance of `wx.Bitmap`.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        should_refresh = page_info.bitmap is not bitmap
        page_info.bitmap = bitmap
        if bitmap.IsOk() and not page_info.dis_bitmap.IsOk():
            page_info.dis_bitmap = MakeDisabledBitmap(bitmap)

        # tab height might have changed
        self.UpdateTabCtrlHeight()

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        info = ctrl.GetPage(ctrl_idx)
        should_refresh = should_refresh or info.bitmap is not bitmap
        info.bitmap = bitmap
        info.dis_bitmap = page_info.dis_bitmap
        if should_refresh:
            ctrl.Refresh()
            ctrl.Update()

        return True


    def GetPageBitmap(self, page_idx):
        """
        Returns the tab bitmap for the page.

        :param `page_idx`: the page index.
        """

        if page_idx >= self._tabs.GetPageCount():
            return wx.NullBitmap

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        return page_info.bitmap


    def SetImageList(self, imageList):
        """
        Sets the image list for the L{AuiNotebook} control.

        :param `imageList`: an instance of `wx.ImageList`.
        """

        self._imageList = imageList


    def AssignImageList(self, imageList):
        """
        Sets the image list for the L{AuiNotebook} control.

        :param `imageList`: an instance of `wx.ImageList`.
        """

        self.SetImageList(imageList)


    def GetImageList(self):
        """ Returns the associated image list (if any). """

        return self._imageList


    def SetPageImage(self, page, image):
        """
        Sets the image index for the given page.

        :param `page`: the page index;
        :param `image`: an index into the image list which was set with L{SetImageList}.
        """

        if page >= self._tabs.GetPageCount():
            return False

        if not isinstance(image, types.IntType):
            raise Exception("The image parameter must be an integer, you passed " \
                            "%s"%repr(image))

        if not self._imageList:
            raise Exception("To use SetPageImage you need to associate an image list " \
                            "Using SetImageList or AssignImageList")

        if image >= self._imageList.GetImageCount():
            raise Exception("Invalid image index (%d), the image list contains only" \
                            " (%d) bitmaps"%(image, self._imageList.GetImageCount()))

        if image == -1:
            self.SetPageBitmap(page, wx.NullBitmap)
            return

        bitmap = self._imageList.GetBitmap(image)
        self.SetPageBitmap(page, bitmap)


    def GetPageImage(self, page):
        """
        Returns the image index for the given page.

        :param `page`: the given page for which to retrieve the image index.
        """

        if page >= self._tabs.GetPageCount():
            return False

        bitmap = self.GetPageBitmap(page)
        for indx in xrange(self._imageList.GetImageCount()):
            imgListBmp = self._imageList.GetBitmap(indx)
            if imgListBmp == bitmap:
                return indx

        return wx.NOT_FOUND


    def SetPageTextColour(self, page_idx, colour):
        """
        Sets the tab text colour for the page.

        :param `page_idx`: the page index;
        :param `colour`: an instance of `wx.Colour`.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        should_refresh = page_info.text_colour != colour
        page_info.text_colour = colour

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        info = ctrl.GetPage(ctrl_idx)
        should_refresh = should_refresh or info.text_colour != colour
        info.text_colour = page_info.text_colour

        if should_refresh:
            ctrl.Refresh()
            ctrl.Update()

        return True


    def GetPageTextColour(self, page_idx):
        """
        Returns the tab text colour for the page.

        :param `page_idx`: the page index.
        """

        if page_idx >= self._tabs.GetPageCount():
            return wx.NullColour

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        return page_info.text_colour


    def AddControlToPage(self, page_idx, control):
        """
        Adds a control inside a tab (not in the tab area).

        :param `page_idx`: the page index;
        :param `control`: an instance of `wx.Window`.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        page_info.control = control

        # tab height might have changed
        self.UpdateTabCtrlHeight(force=True)

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        control.Reparent(ctrl)

        info = ctrl.GetPage(ctrl_idx)
        info.control = control
        ctrl.Refresh()
        ctrl.Update()

        return True


    def RemoveControlFromPage(self, page_idx):
        """
        Removes a control from a tab (not from the tab area).

        :param `page_idx`: the page index.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        page_info = self._tabs.GetPage(page_idx)
        if page_info.control is None:
            return False

        page_info.control.Destroy()
        page_info.control = None

        # tab height might have changed
        self.UpdateTabCtrlHeight(force=True)

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        info = ctrl.GetPage(ctrl_idx)
        info.control = None
        ctrl.Refresh()
        ctrl.Update()

        return True


    def SetCloseButton(self, page_idx, hasCloseButton):
        """
        Sets whether a tab should display a close button or not.

        :param `page_idx`: the page index;
        :param `hasCloseButton`: ``True`` if the page displays a close button.

        :note: This can only be called if ``AUI_NB_CLOSE_ON_ALL_TABS`` is specified.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        if self._agwFlags & AUI_NB_CLOSE_ON_ALL_TABS == 0:
            raise Exception("SetCloseButton can only be used with AUI_NB_CLOSE_ON_ALL_TABS style.")

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        page_info.hasCloseButton = hasCloseButton

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        info = ctrl.GetPage(ctrl_idx)
        info.hasCloseButton = page_info.hasCloseButton
        ctrl.Refresh()
        ctrl.Update()

        return True


    def HasCloseButton(self, page_idx):
        """
        Returns whether a tab displays a close button or not.

        :param `page_idx`: the page index.

        :note: This can only be called if ``AUI_NB_CLOSE_ON_ALL_TABS`` is specified.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        page_info = self._tabs.GetPage(page_idx)
        return page_info.hasCloseButton


    def GetSelection(self):
        """ Returns the index of the currently active page, or -1 if none was selected. """

        return self._curpage


    def GetCurrentPage(self):
        """ Returns the currently active page (not the index), or ``None`` if none was selected. """

        if self._curpage >= 0 and self._curpage < self._tabs.GetPageCount():
            return self.GetPage(self._curpage)

        return None


    def EnsureVisible(self, indx):
        """
        Ensures the input page index `indx` is visible.

        :param `indx`: the page index.
        """

        self._tabs.MakeTabVisible(indx, self)


    def SetSelection(self, new_page, force=False):
        """
        Sets the page selection. Calling this method will generate a page change event.

        :param `new_page`: the index of the new selection;
        :param `force`: whether to force the selection or not.
        """
        wnd = self._tabs.GetWindowFromIdx(new_page)

        #Update page access time
        self._tabs.GetPages()[new_page].access_time = datetime.datetime.now()

        if not wnd or not self.GetEnabled(new_page):
            return self._curpage

        # don't change the page unless necessary
        # however, clicking again on a tab should give it the focus.
        if new_page == self._curpage and not force:

            ctrl, ctrl_idx = self.FindTab(wnd)
            if wx.Window.FindFocus() != ctrl:
                ctrl.SetFocus()

            return self._curpage

        evt = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, self.GetId())
        evt.SetSelection(new_page)
        evt.SetOldSelection(self._curpage)
        evt.SetEventObject(self)

        if not self.GetEventHandler().ProcessEvent(evt) or evt.IsAllowed():

            old_curpage = self._curpage
            self._curpage = new_page

            # program allows the page change
            evt.SetEventType(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED)
            self.GetEventHandler().ProcessEvent(evt)

            if not evt.IsAllowed(): # event is no longer allowed after handler
                return self._curpage

            ctrl, ctrl_idx = self.FindTab(wnd)

            if ctrl:
                self._tabs.SetActivePage(wnd)
                ctrl.SetActivePage(ctrl_idx)
                self.DoSizing()
                ctrl.DoShowHide()
                ctrl.MakeTabVisible(ctrl_idx, ctrl)

                # set fonts
                all_panes = self._mgr.GetAllPanes()
                for pane in all_panes:
                    if pane.name == "dummy":
                        continue

                    tabctrl = pane.window._tabs
                    if tabctrl != ctrl:
                        tabctrl.SetSelectedFont(self._normal_font)
                    else:
                        tabctrl.SetSelectedFont(self._selected_font)

                    tabctrl.Refresh()
                    tabctrl.Update()

                # Set the focus to the page if we're not currently focused on the tab.
                # This is Firefox-like behaviour.
                if wnd.IsShownOnScreen() and wx.Window.FindFocus() != ctrl:
                    wnd.SetFocus()

                return old_curpage

        return self._curpage


    def SetSelectionToWindow(self, win):
        """
        Sets the selection based on the input window `win`.

        :param `win`: a `wx.Window` derived window.
        """

        idx = self._tabs.GetIdxFromWindow(win)

        if idx == wx.NOT_FOUND:
            raise Exception("invalid notebook page")

        if not self.GetEnabled(idx):
            return

        # since a tab was clicked, let the parent know that we received
        # the focus, even if we will assign that focus immediately
        # to the child tab in the SetSelection call below
        # (the child focus event will also let AuiManager, if any,
        # know that the notebook control has been activated)

        parent = self.GetParent()
        if parent:
            eventFocus = wx.ChildFocusEvent(self)
            parent.GetEventHandler().ProcessEvent(eventFocus)

        self.SetSelection(idx)


    def SetSelectionToPage(self, page):
        """
        Sets the selection based on the input page.

        :param `page`: an instance of L{AuiNotebookPage}.
        """

        self.SetSelectionToWindow(page.window)


    def GetPageCount(self):
        """ Returns the number of pages in the notebook. """

        return self._tabs.GetPageCount()


    def GetPage(self, page_idx):
        """
        Returns the page specified by the given index.

        :param `page_idx`: the page index.
        """

        if page_idx >= self._tabs.GetPageCount():
            raise Exception("invalid notebook page")

        return self._tabs.GetWindowFromIdx(page_idx)


    def GetPageInfo(self, page_idx):
        """
        Returns the L{AuiNotebookPage} info structure specified by the given index.

        :param `page_idx`: the page index.
        """

        if page_idx >= self._tabs.GetPageCount():
            raise Exception("invalid notebook page")

        return self._tabs.GetPage(page_idx)


    def GetEnabled(self, page_idx):
        """
        Returns whether the page specified by the index `page_idx` is enabled.

        :param `page_idx`: the page index.
        """

        return self._tabs.GetEnabled(page_idx)


    def EnableTab(self, page_idx, enable=True):
        """
        Enables/disables a page in the notebook.

        :param `page_idx`: the page index;
        :param `enable`: ``True`` to enable the page, ``False`` to disable it.
        """

        self._tabs.EnableTab(page_idx, enable)
        self.Refresh()


    def DoSizing(self):
        """ Performs all sizing operations in each tab control. """

        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tabframe = pane.window
            tabframe.DoSizing()


    def GetAuiManager(self):
        """ Returns the associated L{AuiManager}. """

        return self._mgr


    def GetActiveTabCtrl(self):
        """
        Returns the active tab control. It is called to determine which control
        gets new windows being added.
        """

        if self._curpage >= 0 and self._curpage < self._tabs.GetPageCount():

            # find the tab ctrl with the current page
            ctrl, idx = self.FindTab(self._tabs.GetPage(self._curpage).window)
            if ctrl:
                return ctrl

        # no current page, just find the first tab ctrl
        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tabframe = pane.window
            return tabframe._tabs

        # If there is no tabframe at all, create one
        tabframe = TabFrame(self)
        tabframe.SetTabCtrlHeight(self._tab_ctrl_height)
        self._tab_id_counter += 1
        tabframe._tabs = AuiTabCtrl(self, self._tab_id_counter)

        tabframe._tabs.SetAGWFlags(self._agwFlags)
        tabframe._tabs.SetArtProvider(self._tabs.GetArtProvider().Clone())
        self._mgr.AddPane(tabframe, framemanager.AuiPaneInfo().Center().CaptionVisible(False).
                          PaneBorder((self._agwFlags & AUI_NB_SUB_NOTEBOOK) == 0))

        self._mgr.Update()

        return tabframe._tabs


    def FindTab(self, page):
        """
        Finds the tab control that currently contains the window as well
        as the index of the window in the tab control. It returns ``True`` if the
        window was found, otherwise ``False``.

        :param `page`: an instance of L{AuiNotebookPage}.
        """

        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tabframe = pane.window

            page_idx = tabframe._tabs.GetIdxFromWindow(page)

            if page_idx != -1:

                ctrl = tabframe._tabs
                idx = page_idx
                return ctrl, idx

        return None, wx.NOT_FOUND


    def Split(self, page, direction):
        """
        Performs a split operation programmatically.

        :param `page`: indicates the page that will be split off. This page will also become
         the active page after the split.
        :param `direction`: specifies where the pane should go, it should be one of the
         following: ``wx.TOP``, ``wx.BOTTOM``, ``wx.LEFT``, or ``wx.RIGHT``.
        """

        cli_size = self.GetClientSize()

        # get the page's window pointer
        wnd = self.GetPage(page)
        if not wnd:
            return

        # notebooks with 1 or less pages can't be split
        if self.GetPageCount() < 2:
            return

        # find out which tab control the page currently belongs to

        src_tabs, src_idx = self.FindTab(wnd)
        if not src_tabs:
            return

        # choose a split size
        if self.GetPageCount() > 2:
            split_size = self.CalculateNewSplitSize()
        else:
            # because there are two panes, always split them
            # equally
            split_size = self.GetClientSize()
            split_size.x /= 2
            split_size.y /= 2

        # create a new tab frame
        new_tabs = TabFrame(self)
        new_tabs._rect = wx.RectPS(wx.Point(0, 0), split_size)
        new_tabs.SetTabCtrlHeight(self._tab_ctrl_height)
        self._tab_id_counter += 1
        new_tabs._tabs = AuiTabCtrl(self, self._tab_id_counter)

        new_tabs._tabs.SetArtProvider(self._tabs.GetArtProvider().Clone())
        new_tabs._tabs.SetAGWFlags(self._agwFlags)
        dest_tabs = new_tabs._tabs

        page_info = src_tabs.GetPage(src_idx)
        if page_info.control:
            self.ReparentControl(page_info.control, dest_tabs)

        # create a pane info structure with the information
        # about where the pane should be added
        pane_info = framemanager.AuiPaneInfo().Bottom().CaptionVisible(False)

        if direction == wx.LEFT:

            pane_info.Left()
            mouse_pt = wx.Point(0, cli_size.y/2)

        elif direction == wx.RIGHT:

            pane_info.Right()
            mouse_pt = wx.Point(cli_size.x, cli_size.y/2)

        elif direction == wx.TOP:

            pane_info.Top()
            mouse_pt = wx.Point(cli_size.x/2, 0)

        elif direction == wx.BOTTOM:

            pane_info.Bottom()
            mouse_pt = wx.Point(cli_size.x/2, cli_size.y)

        self._mgr.AddPane(new_tabs, pane_info, mouse_pt)
        self._mgr.Update()

        # remove the page from the source tabs
        page_info.active = False

        src_tabs.RemovePage(page_info.window)

        if src_tabs.GetPageCount() > 0:
            src_tabs.SetActivePage(0)
            src_tabs.DoShowHide()
            src_tabs.Refresh()

        # add the page to the destination tabs
        dest_tabs.InsertPage(page_info.window, page_info, 0)

        if src_tabs.GetPageCount() == 0:
            self.RemoveEmptyTabFrames()

        self.DoSizing()
        dest_tabs.DoShowHide()
        dest_tabs.Refresh()

        # force the set selection function reset the selection
        self._curpage = -1

        # set the active page to the one we just split off
        self.SetSelectionToPage(page_info)

        self.UpdateHintWindowSize()


    def UnSplit(self):
        """ Restores original view after a tab split. """

        self.Freeze()

        # remember the tab now selected
        nowSelected = self.GetSelection()
        # select first tab as destination
        self.SetSelection(0)
        # iterate all other tabs
        for idx in xrange(1, self.GetPageCount()):
            # get win reference
            win = self.GetPage(idx)
            # get tab title
            title = self.GetPageText(idx)
            # get page bitmap
            bmp = self.GetPageBitmap(idx)
            # remove from notebook
            self.RemovePage(idx)
            # re-add in the same position so it will tab
            self.InsertPage(idx, win, title, False, bmp)
        # restore orignial selected tab
        self.SetSelection(nowSelected)

        self.Thaw()


    def ReparentControl(self, control, dest_tabs):
        """
        Reparents a control added inside a tab.

        :param `control`: an instance of `wx.Window`;
        :param `dest_tabs`: the destination L{AuiTabCtrl}.
        """

        control.Hide()
        control.Reparent(dest_tabs)


    def UnsplitDClick(self, part, sash_size, pos):
        """
        Unsplit the L{AuiNotebook} on sash double-click.

        :param `part`: an UI part representing the sash;
        :param `sash_size`: the sash size;
        :param `pos`: the double-click mouse position.

        :warning: Due to a bug on MSW, for disabled pages `wx.FindWindowAtPoint`
         returns the wrong window. See http://trac.wxwidgets.org/ticket/2942
        """

        if not self._sash_dclick_unsplit:
            # Unsplit not allowed
            return

        pos1 = wx.Point(*pos)
        pos2 = wx.Point(*pos)
        if part.orientation == wx.HORIZONTAL:
            pos1.y -= 2*sash_size
            pos2.y += 2*sash_size + self.GetTabCtrlHeight()
        elif part.orientation == wx.VERTICAL:
            pos1.x -= 2*sash_size
            pos2.x += 2*sash_size
        else:
            raise Exception("Invalid UI part orientation")

        pos1, pos2 = self.ClientToScreen(pos1), self.ClientToScreen(pos2)
        win1, win2 = wx.FindWindowAtPoint(pos1), wx.FindWindowAtPoint(pos2)

        if isinstance(win1, wx.ScrollBar):
            # Hopefully it will work
            pos1 = wx.Point(*pos)
            shift = wx.SystemSettings.GetMetric(wx.SYS_VSCROLL_X) + 2*(sash_size+1)
            if part.orientation == wx.HORIZONTAL:
                pos1.y -= shift
            else:
                pos1.x -= shift

            pos1 = self.ClientToScreen(pos1)
            win1 = wx.FindWindowAtPoint(pos1)

        if isinstance(win2, wx.ScrollBar):
            pos2 = wx.Point(*pos)
            shift = wx.SystemSettings.GetMetric(wx.SYS_VSCROLL_X) + 2*(sash_size+1)
            if part.orientation == wx.HORIZONTAL:
                pos2.y += shift
            else:
                pos2.x += shift

            pos2 = self.ClientToScreen(pos2)
            win2 = wx.FindWindowAtPoint(pos2)

        if not win1 or not win2:
            # How did we get here?
            return

        if isinstance(win1, AuiNotebook) or isinstance(win2, AuiNotebook):
            # This is a bug on MSW, for disabled pages wx.FindWindowAtPoint
            # returns the wrong window.
            # See http://trac.wxwidgets.org/ticket/2942
            return

        tab_frame1, tab_frame2 = self.GetTabFrameFromWindow(win1), self.GetTabFrameFromWindow(win2)

        if not tab_frame1 or not tab_frame2:
            return

        tab_ctrl_1, tab_ctrl_2 = tab_frame1._tabs, tab_frame2._tabs

        if tab_ctrl_1.GetPageCount() > tab_ctrl_2.GetPageCount():
            src_tabs = tab_ctrl_2
            dest_tabs = tab_ctrl_1
        else:
            src_tabs = tab_ctrl_1
            dest_tabs = tab_ctrl_2

        selection = -1
        page_count = dest_tabs.GetPageCount()

        for page in xrange(src_tabs.GetPageCount()-1, -1, -1):
            # remove the page from the source tabs
            page_info = src_tabs.GetPage(page)
            if page_info.active:
                selection = page_count + page
            src_tabs.RemovePage(page_info.window)

            # add the page to the destination tabs
            dest_tabs.AddPage(page_info.window, page_info)
            if page_info.control:
                self.ReparentControl(page_info.control, dest_tabs)

        self.RemoveEmptyTabFrames()

        dest_tabs.DoShowHide()
        self.DoSizing()
        dest_tabs.Refresh()
        self._mgr.Update()
        if selection > 0:
            wx.CallAfter(dest_tabs.MakeTabVisible, selection, self)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{AuiNotebook}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.UpdateHintWindowSize()
        event.Skip()


    def OnTabClicked(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_PAGE_CHANGING`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        if self._textCtrl is not None:
            self._textCtrl.StopEditing()

        ctrl = event.GetEventObject()
        assert ctrl != None

        wnd = ctrl.GetWindowFromIdx(event.GetSelection())
        assert wnd != None

        self.SetSelectionToWindow(wnd)


    def OnTabBgDClick(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_BG_DCLICK`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        if self._textCtrl is not None:
            self._textCtrl.StopEditing()

        # notify owner that the tabbar background has been double-clicked
        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BG_DCLICK, self.GetId())
        e.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(e)


    def OnTabDClick(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_TAB_DCLICK`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        # notify owner that the tabbar background has been double-clicked
        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_DCLICK, self.GetId())
        e.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(e)

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        if not self.IsRenamable(event.GetSelection()):
            return

        self.EditTab(event.GetSelection())


    def OnTabBeginDrag(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_BEGIN_DRAG`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        self._last_drag_x = 0


    def OnTabDragMotion(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_DRAG_MOTION`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        if self._textCtrl is not None:
            self._textCtrl.StopEditing()

        screen_pt = wx.GetMousePosition()
        client_pt = self.ScreenToClient(screen_pt)
        zero = wx.Point(0, 0)

        src_tabs = event.GetEventObject()
        dest_tabs = self.GetTabCtrlFromPoint(client_pt)

        if dest_tabs == src_tabs:

            # always hide the hint for inner-tabctrl drag
            self._mgr.HideHint()

            # if tab moving is not allowed, leave
            if not self._agwFlags & AUI_NB_TAB_MOVE:
                return

            pt = dest_tabs.ScreenToClient(screen_pt)

            # this is an inner-tab drag/reposition
            dest_location_tab = dest_tabs.TabHitTest(pt.x, pt.y)

            if dest_location_tab:

                src_idx = event.GetSelection()
                dest_idx = dest_tabs.GetIdxFromWindow(dest_location_tab)

                # prevent jumpy drag
                if (src_idx == dest_idx) or dest_idx == -1 or \
                   (src_idx > dest_idx and self._last_drag_x <= pt.x) or \
                   (src_idx < dest_idx and self._last_drag_x >= pt.x):

                    self._last_drag_x = pt.x
                    return

                src_tab = dest_tabs.GetWindowFromIdx(src_idx)
                dest_tabs.MovePage(src_tab, dest_idx)
                self._tabs.MovePage(self._tabs.GetPage(src_idx).window, dest_idx)
                dest_tabs.SetActivePage(dest_idx)
                dest_tabs.DoShowHide()
                dest_tabs.Refresh()
                self._last_drag_x = pt.x

            return

        # if external drag is allowed, check if the tab is being dragged
        # over a different AuiNotebook control
        if self._agwFlags & AUI_NB_TAB_EXTERNAL_MOVE:

            tab_ctrl = wx.FindWindowAtPoint(screen_pt)

            # if we aren't over any window, stop here
            if not tab_ctrl:
                if self._agwFlags & AUI_NB_TAB_FLOAT:
                    if self.IsMouseWellOutsideWindow():
                        hintRect = wx.RectPS(screen_pt, (400, 300))
                        # Use CallAfter so we overwrite the hint that might be
                        # shown by our superclass:
                        wx.CallAfter(self._mgr.ShowHint, hintRect)
                return

            # make sure we are not over the hint window
            if not isinstance(tab_ctrl, wx.Frame):
                while tab_ctrl:
                    if isinstance(tab_ctrl, AuiTabCtrl):
                        break

                    tab_ctrl = tab_ctrl.GetParent()

                if tab_ctrl:
                    nb = tab_ctrl.GetParent()

                    if nb != self:

                        hint_rect = tab_ctrl.GetClientRect()
                        hint_rect.x, hint_rect.y = tab_ctrl.ClientToScreenXY(hint_rect.x, hint_rect.y)
                        self._mgr.ShowHint(hint_rect)
                        return

            else:

                if not dest_tabs:
                    # we are either over a hint window, or not over a tab
                    # window, and there is no where to drag to, so exit
                    return

        if self._agwFlags & AUI_NB_TAB_FLOAT:
            if self.IsMouseWellOutsideWindow():
                hintRect = wx.RectPS(screen_pt, (400, 300))
                # Use CallAfter so we overwrite the hint that might be
                # shown by our superclass:
                wx.CallAfter(self._mgr.ShowHint, hintRect)
                return

        # if there are less than two panes, split can't happen, so leave
        if self._tabs.GetPageCount() < 2:
            return

        # if tab moving is not allowed, leave
        if not self._agwFlags & AUI_NB_TAB_SPLIT:
            return

        if dest_tabs:

            hint_rect = dest_tabs.GetRect()
            hint_rect.x, hint_rect.y = self.ClientToScreenXY(hint_rect.x, hint_rect.y)
            self._mgr.ShowHint(hint_rect)

        else:
            rect = self._mgr.CalculateHintRect(self._dummy_wnd, client_pt, zero)
            if rect.IsEmpty():
                self._mgr.HideHint()
                return

            hit_wnd = wx.FindWindowAtPoint(screen_pt)
            if hit_wnd and not isinstance(hit_wnd, AuiNotebook):
                tab_frame = self.GetTabFrameFromWindow(hit_wnd)
                if tab_frame:
                    hint_rect = wx.Rect(*tab_frame._rect)
                    hint_rect.x, hint_rect.y = self.ClientToScreenXY(hint_rect.x, hint_rect.y)
                    rect.Intersect(hint_rect)
                    self._mgr.ShowHint(rect)
                else:
                    self._mgr.DrawHintRect(self._dummy_wnd, client_pt, zero)
            else:
                self._mgr.DrawHintRect(self._dummy_wnd, client_pt, zero)


    def OnTabEndDrag(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_END_DRAG`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        self._mgr.HideHint()

        src_tabs = event.GetEventObject()
        if not src_tabs:
            raise Exception("no source object?")

        # get the mouse position, which will be used to determine the drop point
        mouse_screen_pt = wx.GetMousePosition()
        mouse_client_pt = self.ScreenToClient(mouse_screen_pt)

        # check for an external move
        if self._agwFlags & AUI_NB_TAB_EXTERNAL_MOVE:
            tab_ctrl = wx.FindWindowAtPoint(mouse_screen_pt)

            while tab_ctrl:

                if isinstance(tab_ctrl, AuiTabCtrl):
                    break

                tab_ctrl = tab_ctrl.GetParent()

            if tab_ctrl:

                nb = tab_ctrl.GetParent()

                if nb != self:

                    # find out from the destination control
                    # if it's ok to drop this tab here
                    e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_ALLOW_DND, self.GetId())
                    e.SetSelection(event.GetSelection())
                    e.SetOldSelection(event.GetSelection())
                    e.SetEventObject(self)
                    e.SetDragSource(self)
                    e.Veto() # dropping must be explicitly approved by control owner

                    nb.GetEventHandler().ProcessEvent(e)

                    if not e.IsAllowed():

                        # no answer or negative answer
                        self._mgr.HideHint()
                        return

                    # drop was allowed
                    src_idx = event.GetSelection()
                    src_page = src_tabs.GetWindowFromIdx(src_idx)

                    # Check that it's not an impossible parent relationship
                    p = nb
                    while p and not p.IsTopLevel():
                        if p == src_page:
                            return

                        p = p.GetParent()

                    # get main index of the page
                    main_idx = self._tabs.GetIdxFromWindow(src_page)
                    if main_idx == wx.NOT_FOUND:
                        raise Exception("no source page?")

                    # make a copy of the page info
                    page_info = self._tabs.GetPage(main_idx)

                    # remove the page from the source notebook
                    self.RemovePage(main_idx)

                    # reparent the page
                    src_page.Reparent(nb)

                    # Reparent the control in a tab (if any)
                    if page_info.control:
                        self.ReparentControl(page_info.control, tab_ctrl)

                    # find out the insert idx
                    dest_tabs = tab_ctrl
                    pt = dest_tabs.ScreenToClient(mouse_screen_pt)

                    target = dest_tabs.TabHitTest(pt.x, pt.y)
                    insert_idx = -1
                    if target:
                        insert_idx = dest_tabs.GetIdxFromWindow(target)

                    # add the page to the new notebook
                    if insert_idx == -1:
                        insert_idx = dest_tabs.GetPageCount()

                    dest_tabs.InsertPage(page_info.window, page_info, insert_idx)
                    nb._tabs.AddPage(page_info.window, page_info)

                    nb.DoSizing()
                    dest_tabs.DoShowHide()
                    dest_tabs.Refresh()

                    # set the selection in the destination tab control
                    nb.SetSelectionToPage(page_info)

                    # notify owner that the tab has been dragged
                    e2 = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE, self.GetId())
                    e2.SetSelection(event.GetSelection())
                    e2.SetOldSelection(event.GetSelection())
                    e2.SetEventObject(self)
                    self.GetEventHandler().ProcessEvent(e2)

                    # notify the target notebook that the tab has been dragged
                    e3 = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE, nb.GetId())
                    e3.SetSelection(insert_idx)
                    e3.SetOldSelection(insert_idx)
                    e3.SetEventObject(nb)
                    nb.GetEventHandler().ProcessEvent(e3)

                    return

        if self._agwFlags & AUI_NB_TAB_FLOAT:
            self._mgr.HideHint()
            if self.IsMouseWellOutsideWindow():
                # Use CallAfter so we our superclass can deal with the event first
                wx.CallAfter(self.FloatPage, self.GetSelection())
                event.Skip()
                return

        # only perform a tab split if it's allowed
        dest_tabs = None

        if self._agwFlags & AUI_NB_TAB_SPLIT and self._tabs.GetPageCount() >= 2:

            # If the pointer is in an existing tab frame, do a tab insert
            hit_wnd = wx.FindWindowAtPoint(mouse_screen_pt)
            tab_frame = self.GetTabFrameFromTabCtrl(hit_wnd)
            insert_idx = -1

            if tab_frame:

                dest_tabs = tab_frame._tabs

                if dest_tabs == src_tabs:
                    return

                pt = dest_tabs.ScreenToClient(mouse_screen_pt)
                target = dest_tabs.TabHitTest(pt.x, pt.y)

                if target:
                    insert_idx = dest_tabs.GetIdxFromWindow(target)

            else:

                zero = wx.Point(0, 0)
                rect = self._mgr.CalculateHintRect(self._dummy_wnd, mouse_client_pt, zero)

                if rect.IsEmpty():
                    # there is no suitable drop location here, exit out
                    return

                # If there is no tabframe at all, create one
                new_tabs = TabFrame(self)
                new_tabs._rect = wx.RectPS(wx.Point(0, 0), self.CalculateNewSplitSize())
                new_tabs.SetTabCtrlHeight(self._tab_ctrl_height)
                self._tab_id_counter += 1
                new_tabs._tabs = AuiTabCtrl(self, self._tab_id_counter)
                new_tabs._tabs.SetArtProvider(self._tabs.GetArtProvider().Clone())
                new_tabs._tabs.SetAGWFlags(self._agwFlags)

                self._mgr.AddPane(new_tabs, framemanager.AuiPaneInfo().Bottom().CaptionVisible(False), mouse_client_pt)
                self._mgr.Update()
                dest_tabs = new_tabs._tabs

            # remove the page from the source tabs
            page_info = src_tabs.GetPage(event.GetSelection())

            if page_info.control:
                self.ReparentControl(page_info.control, dest_tabs)

            page_info.active = False
            src_tabs.RemovePage(page_info.window)

            if src_tabs.GetPageCount() > 0:
                src_tabs.SetActivePage(0)
                src_tabs.DoShowHide()
                src_tabs.Refresh()

            # add the page to the destination tabs
            if insert_idx == -1:
                insert_idx = dest_tabs.GetPageCount()

            dest_tabs.InsertPage(page_info.window, page_info, insert_idx)

            if src_tabs.GetPageCount() == 0:
                self.RemoveEmptyTabFrames()

            self.DoSizing()
            dest_tabs.DoShowHide()
            dest_tabs.Refresh()

            # force the set selection function reset the selection
            self._curpage = -1

            # set the active page to the one we just split off
            self.SetSelectionToPage(page_info)

            self.UpdateHintWindowSize()

        # notify owner that the tab has been dragged
        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_DRAG_DONE, self.GetId())
        e.SetSelection(event.GetSelection())
        e.SetOldSelection(event.GetSelection())
        e.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(e)


    def OnTabCancelDrag(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_CANCEL_DRAG`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        self._mgr.HideHint()

        src_tabs = event.GetEventObject()
        if not src_tabs:
            raise Exception("no source object?")


    def IsMouseWellOutsideWindow(self):
        """ Returns whether the mouse is well outside the L{AuiNotebook} screen rectangle. """

        screen_rect = self.GetScreenRect()
        screen_rect.Inflate(50, 50)

        return not screen_rect.Contains(wx.GetMousePosition())


    def FloatPage(self, page_index):
        """
        Float the page in `page_index` by reparenting it to a floating frame.

        :param `page_index`: the index of the page to be floated.

        :warning: When the notebook is more or less full screen, tabs cannot be dragged far
         enough outside of the notebook to become floating pages.
        """

        root_manager = framemanager.GetManager(self)
        page_title = self.GetPageText(page_index)
        page_contents = self.GetPage(page_index)
        page_bitmap = self.GetPageBitmap(page_index)
        text_colour = self.GetPageTextColour(page_index)
        info = self.GetPageInfo(page_index)

        if root_manager and root_manager != self._mgr:
            root_manager = framemanager.GetManager(self)

            if hasattr(page_contents, "__floating_size__"):
                floating_size = wx.Size(*page_contents.__floating_size__)
            else:
                floating_size = page_contents.GetBestSize()
                if floating_size == wx.DefaultSize:
                    floating_size = wx.Size(300, 200)

            page_contents.__page_index__ = page_index
            page_contents.__aui_notebook__ = self
            page_contents.__text_colour__ = text_colour
            page_contents.__control__ = info.control

            if info.control:
                info.control.Reparent(page_contents)
                info.control.Hide()
                info.control = None

            self.RemovePage(page_index)
            self.RemoveEmptyTabFrames()

            pane_info = framemanager.AuiPaneInfo().Float().FloatingPosition(wx.GetMousePosition()). \
                        FloatingSize(floating_size).BestSize(floating_size).Name("__floating__%s"%page_title). \
                        Caption(page_title).Icon(page_bitmap)
            root_manager.AddPane(page_contents, pane_info)
            root_manager.Bind(framemanager.EVT_AUI_PANE_CLOSE, self.OnCloseFloatingPage)
            self.GetActiveTabCtrl().DoShowHide()
            self.DoSizing()
            root_manager.Update()

        else:
            frame = wx.Frame(self, title=page_title,
                             style=wx.DEFAULT_FRAME_STYLE|wx.FRAME_TOOL_WINDOW|
                                   wx.FRAME_FLOAT_ON_PARENT | wx.FRAME_NO_TASKBAR)

            if info.control:
                info.control.Reparent(frame)
                info.control.Hide()

            frame.bitmap = page_bitmap
            frame.page_index = page_index
            frame.text_colour = text_colour
            frame.control = info.control
            page_contents.Reparent(frame)
            frame.Bind(wx.EVT_CLOSE, self.OnCloseFloatingPage)
            frame.Move(wx.GetMousePosition())
            frame.Show()
            self.RemovePage(page_index)

            self.RemoveEmptyTabFrames()

        wx.CallAfter(self.RemoveEmptyTabFrames)


    def OnCloseFloatingPage(self, event):
        """
        Handles the ``wx.EVT_CLOSE`` event for a floating page in L{AuiNotebook}.

        :param `event`: a `wx.CloseEvent` event to be processed.
        """

        root_manager = framemanager.GetManager(self)
        if root_manager and root_manager != self._mgr:
            pane = event.pane
            if pane.name.startswith("__floating__"):
                self.ReDockPage(pane)
                return

            event.Skip()
        else:
            event.Skip()
            frame = event.GetEventObject()
            page_title = frame.GetTitle()
            page_contents = list(frame.GetChildren())[-1]
            page_contents.Reparent(self)
            self.InsertPage(frame.page_index, page_contents, page_title, select=True, bitmap=frame.bitmap, control=frame.control)

            if frame.control:
                src_tabs, idx = self.FindTab(page_contents)
                frame.control.Reparent(src_tabs)
                frame.control.Hide()
                frame.control = None

            self.SetPageTextColour(frame.page_index, frame.text_colour)


    def ReDockPage(self, pane):
        """
        Re-docks a floating L{AuiNotebook} tab in the original position, when possible.

        :param `pane`: an instance of L{framemanager.AuiPaneInfo}.
        """

        root_manager = framemanager.GetManager(self)

        pane.window.__floating_size__ = wx.Size(*pane.floating_size)
        page_index = pane.window.__page_index__
        text_colour = pane.window.__text_colour__
        control = pane.window.__control__

        root_manager.DetachPane(pane.window)
        self.InsertPage(page_index, pane.window, pane.caption, True, pane.icon, control=control)

        self.SetPageTextColour(page_index, text_colour)
        self.GetActiveTabCtrl().DoShowHide()
        self.DoSizing()
        if control:
            self.UpdateTabCtrlHeight(force=True)

        self._mgr.Update()
        root_manager.Update()


    def GetTabCtrlFromPoint(self, pt):
        """
        Returns the tab control at the specified point.

        :param `pt`: a `wx.Point` object.
        """

        # if we've just removed the last tab from the source
        # tab set, the remove the tab control completely
        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tabframe = pane.window
            if tabframe._tab_rect.Contains(pt):
                return tabframe._tabs

        return None


    def GetTabFrameFromTabCtrl(self, tab_ctrl):
        """
        Returns the tab frame associated with a tab control.

        :param `tab_ctrl`: an instance of L{AuiTabCtrl}.
        """

        # if we've just removed the last tab from the source
        # tab set, the remove the tab control completely
        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tabframe = pane.window
            if tabframe._tabs == tab_ctrl:
                return tabframe

        return None


    def GetTabFrameFromWindow(self, wnd):
        """
        Returns the tab frame associated with a window.

        :param `wnd`: an instance of `wx.Window`.
        """

        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            tabframe = pane.window
            for page in tabframe._tabs.GetPages():
                if wnd == page.window:
                    return tabframe

        return None


    def RemoveEmptyTabFrames(self):
        """ Removes all the empty tab frames. """

        # if we've just removed the last tab from the source
        # tab set, the remove the tab control completely
        all_panes = self._mgr.GetAllPanes()

        for indx in xrange(len(all_panes)-1, -1, -1):
            pane = all_panes[indx]
            if pane.name == "dummy":
                continue

            tab_frame = pane.window
            if tab_frame._tabs.GetPageCount() == 0:
                self._mgr.DetachPane(tab_frame)
                tab_frame._tabs.Destroy()
                tab_frame._tabs = None
                del tab_frame

        # check to see if there is still a center pane
        # if there isn't, make a frame the center pane
        first_good = None
        center_found = False

        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue

            if pane.dock_direction == AUI_DOCK_CENTRE:
                center_found = True
            if not first_good:
                first_good = pane.window

        if not center_found and first_good:
            self._mgr.GetPane(first_good).Centre()

        if not self.IsBeingDeleted():
            self._mgr.Update()


    def OnChildFocusNotebook(self, event):
        """
        Handles the ``wx.EVT_CHILD_FOCUS`` event for L{AuiNotebook}.

        :param `event`: a `wx.ChildFocusEvent` event to be processed.
        """

        # if we're dragging a tab, don't change the current selection.
        # This code prevents a bug that used to happen when the hint window
        # was hidden.  In the bug, the focus would return to the notebook
        # child, which would then enter this handler and call
        # SetSelection, which is not desired turn tab dragging.

        event.Skip()

        all_panes = self._mgr.GetAllPanes()
        for pane in all_panes:
            if pane.name == "dummy":
                continue
            tabframe = pane.window
            if tabframe._tabs.IsDragging():
                return

##        # change the tab selection to the child
##        # which was focused
##        idx = self._tabs.GetIdxFromWindow(event.GetWindow())
##        if idx != -1 and idx != self._curpage:
##            self.SetSelection(idx)


    def SetNavigatorIcon(self, bmp):
        """
        Sets the icon used by the L{TabNavigatorWindow}.

        :param `bmp`: an instance of `wx.Bitmap`.
        """

        if isinstance(bmp, wx.Bitmap) and bmp.IsOk():
            # Make sure image is proper size
            if bmp.GetSize() != (16, 16):
                img = bmp.ConvertToImage()
                img.Rescale(16, 16, wx.IMAGE_QUALITY_HIGH)
                bmp = wx.BitmapFromImage(img)
            self._naviIcon = bmp
        else:
            raise TypeError, "SetNavigatorIcon requires a valid bitmap"


    def OnNavigationKeyNotebook(self, event):
        """
        Handles the ``wx.EVT_NAVIGATION_KEY`` event for L{AuiNotebook}.

        :param `event`: a `wx.NavigationKeyEvent` event to be processed.
        """

        if event.IsWindowChange():
            if self._agwFlags & AUI_NB_SMART_TABS:
                if not self._popupWin:
                    self._popupWin = TabNavigatorWindow(self, self._naviIcon)
                    self._popupWin.SetReturnCode(wx.ID_OK)
                    self._popupWin.ShowModal()
                    idx = self._popupWin.GetSelectedPage()
                    self._popupWin.Destroy()
                    self._popupWin = None
                    # Need to do CallAfter so that the selection and its
                    # associated events get processed outside the context of
                    # this key event. Not doing so causes odd issues with the
                    # window focus under certain use cases on Windows.
                    wx.CallAfter(self.SetSelection, idx, True)
                else:
                    # a dialog is already opened
                    self._popupWin.OnNavigationKey(event)
                    return
            else:
                # change pages
                # FIXME: the problem with this is that if we have a split notebook,
                # we selection may go all over the place.
                self.AdvanceSelection(event.GetDirection())

        else:
            # we get this event in 3 cases
            #
            # a) one of our pages might have generated it because the user TABbed
            # out from it in which case we should propagate the event upwards and
            # our parent will take care of setting the focus to prev/next sibling
            #
            # or
            #
            # b) the parent panel wants to give the focus to us so that we
            # forward it to our selected page. We can't deal with this in
            # OnSetFocus() because we don't know which direction the focus came
            # from in this case and so can't choose between setting the focus to
            # first or last panel child
            #
            # or
            #
            # c) we ourselves (see MSWTranslateMessage) generated the event
            #
            parent = self.GetParent()

            # the wxObject* casts are required to avoid MinGW GCC 2.95.3 ICE
            isFromParent = event.GetEventObject() == parent
            isFromSelf = event.GetEventObject() == self

            if isFromParent or isFromSelf:

                # no, it doesn't come from child, case (b) or (c): forward to a
                # page but only if direction is backwards (TAB) or from ourselves,
                if self.GetSelection() != wx.NOT_FOUND and (not event.GetDirection() or isFromSelf):

                    # so that the page knows that the event comes from it's parent
                    # and is being propagated downwards
                    event.SetEventObject(self)

                    page = self.GetPage(self.GetSelection())
                    if not page.GetEventHandler().ProcessEvent(event):
                        page.SetFocus()

                    #else: page manages focus inside it itself

                else: # otherwise set the focus to the notebook itself

                    self.SetFocus()

            else:

                # send this event back for the 'wraparound' focus.
                winFocus = event.GetCurrentFocus()

                if winFocus:
                    event.SetEventObject(self)
                    winFocus.GetEventHandler().ProcessEvent(event)


    def OnTabButton(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_BUTTON`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        button_id = event.GetInt()

        if button_id == AUI_BUTTON_CLOSE:

            selection = event.GetSelection()

            if selection == -1:

                # if the close button is to the right, use the active
                # page selection to determine which page to close
                selection = tabs.GetActivePage()

            if selection == -1 or not tabs.GetEnabled(selection):
                return

            if selection != -1:

                close_wnd = tabs.GetWindowFromIdx(selection)

                if close_wnd.GetName() == "__fake__page__":
                    # This is a notebook preview
                    previous_active, page_status = close_wnd.__previousStatus
                    for page, status in zip(tabs.GetPages(), page_status):
                        page.enabled = status

                    main_idx = self._tabs.GetIdxFromWindow(close_wnd)
                    self.DeletePage(main_idx)

                    if previous_active >= 0:
                        tabs.SetActivePage(previous_active)
                        page_count = tabs.GetPageCount()
                        selection = -1

                        for page in xrange(page_count):
                            # remove the page from the source tabs
                            page_info = tabs.GetPage(page)
                            if page_info.active:
                                selection = page
                                break

                        tabs.DoShowHide()
                        self.DoSizing()
                        tabs.Refresh()

                        if selection >= 0:
                            wx.CallAfter(tabs.MakeTabVisible, selection, self)

                    # Don't fire the event
                    return

                # ask owner if it's ok to close the tab
                e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, self.GetId())
                idx = self._tabs.GetIdxFromWindow(close_wnd)
                e.SetSelection(idx)
                e.SetOldSelection(event.GetSelection())
                e.SetEventObject(self)
                self.GetEventHandler().ProcessEvent(e)
                if not e.IsAllowed():
                    return

                if repr(close_wnd.__class__).find("AuiMDIChildFrame") >= 0:
                    close_wnd.Close()

                else:
                    main_idx = self._tabs.GetIdxFromWindow(close_wnd)
                    self.DeletePage(main_idx)

                # notify owner that the tab has been closed
                e2 = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSED, self.GetId())
                e2.SetSelection(idx)
                e2.SetEventObject(self)
                self.GetEventHandler().ProcessEvent(e2)

                if self.GetPageCount() == 0:
                    mgr = self.GetAuiManager()
                    win = mgr.GetManagedWindow()
                    win.SendSizeEvent()


    def OnTabMiddleDown(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_TAB_MIDDLE_DOWN`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        # patch event through to owner
        wnd = tabs.GetWindowFromIdx(event.GetSelection())

        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_DOWN, self.GetId())
        e.SetSelection(self._tabs.GetIdxFromWindow(wnd))
        e.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(e)


    def OnTabMiddleUp(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_TAB_MIDDLE_UP`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        # if the AUI_NB_MIDDLE_CLICK_CLOSE is specified, middle
        # click should act like a tab close action.  However, first
        # give the owner an opportunity to handle the middle up event
        # for custom action

        wnd = tabs.GetWindowFromIdx(event.GetSelection())

        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_MIDDLE_UP, self.GetId())
        e.SetSelection(self._tabs.GetIdxFromWindow(wnd))
        e.SetEventObject(self)
        if self.GetEventHandler().ProcessEvent(e):
            return
        if not e.IsAllowed():
            return

        # check if we are supposed to close on middle-up
        if self._agwFlags & AUI_NB_MIDDLE_CLICK_CLOSE == 0:
            return

        # simulate the user pressing the close button on the tab
        event.SetInt(AUI_BUTTON_CLOSE)
        self.OnTabButton(event)


    def OnTabRightDown(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_TAB_RIGHT_DOWN`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        # patch event through to owner
        wnd = tabs.GetWindowFromIdx(event.GetSelection())

        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_DOWN, self.GetId())
        e.SetSelection(self._tabs.GetIdxFromWindow(wnd))
        e.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(e)


    def OnTabRightUp(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_TAB_RIGHT_UP`` event for L{AuiNotebook}.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        tabs = event.GetEventObject()
        if not tabs.GetEnabled(event.GetSelection()):
            return

        # patch event through to owner
        wnd = tabs.GetWindowFromIdx(event.GetSelection())

        e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_TAB_RIGHT_UP, self.GetId())
        e.SetSelection(self._tabs.GetIdxFromWindow(wnd))
        e.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(e)


    def SetNormalFont(self, font):
        """
        Sets the normal font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """

        self._normal_font = font
        self.GetArtProvider().SetNormalFont(font)


    def SetSelectedFont(self, font):
        """
        Sets the selected tab font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """

        self._selected_font = font
        self.GetArtProvider().SetSelectedFont(font)


    def SetMeasuringFont(self, font):
        """
        Sets the font for calculating text measurements.

        :param `font`: a `wx.Font` object.
        """

        self.GetArtProvider().SetMeasuringFont(font)


    def SetFont(self, font):
        """
        Sets the tab font.

        :param `font`: a `wx.Font` object.

        :note: Overridden from `wx.PyPanel`.
        """

        wx.PyPanel.SetFont(self, font)

        selectedFont = wx.Font(font.GetPointSize(), font.GetFamily(),
                               font.GetStyle(), wx.BOLD, font.GetUnderlined(),
                               font.GetFaceName(), font.GetEncoding())

        self.SetNormalFont(font)
        self.SetSelectedFont(selectedFont)
        self.SetMeasuringFont(selectedFont)

        # Recalculate tab container size based on new font
        self.UpdateTabCtrlHeight(force=False)
        self.DoSizing()

        return True


    def GetTabCtrlHeight(self):
        """ Returns the tab control height. """

        return self._tab_ctrl_height


    def GetHeightForPageHeight(self, pageHeight):
        """
        Gets the height of the notebook for a given page height.

        :param `pageHeight`: the given page height.
        """

        self.UpdateTabCtrlHeight()

        tabCtrlHeight = self.GetTabCtrlHeight()
        decorHeight = 2
        return tabCtrlHeight + pageHeight + decorHeight


    def AdvanceSelection(self, forward=True, wrap=True):
        """
        Cycles through the tabs.

        :param `forward`: whether to advance forward or backward;
        :param `wrap`: ``True`` to return to the first tab if we reach the last tab.

        :note: The call to this function generates the page changing events.
        """

        tabCtrl = self.GetActiveTabCtrl()
        newPage = -1

        focusWin = tabCtrl.FindFocus()
        activePage = tabCtrl.GetActivePage()
        lenPages = len(tabCtrl.GetPages())

        if lenPages == 1:
            return False

        if forward:
            if lenPages > 1:

                if activePage == -1 or activePage == lenPages - 1:
                    if not wrap:
                        return False

                    newPage = 0

                elif activePage < lenPages - 1:
                    newPage = activePage + 1

        else:

            if lenPages > 1:
                if activePage == -1 or activePage == 0:
                    if not wrap:
                        return False

                    newPage = lenPages - 1

                elif activePage > 0:
                    newPage = activePage - 1


        if newPage != -1:
            if not self.GetEnabled(newPage):
                return False

            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, tabCtrl.GetId())
            e.SetSelection(newPage)
            e.SetOldSelection(activePage)
            e.SetEventObject(tabCtrl)
            self.GetEventHandler().ProcessEvent(e)

##        if focusWin:
##            focusWin.SetFocus()

        return True


    def ShowWindowMenu(self):
        """
        Shows the window menu for the active tab control associated with this
        notebook, and returns ``True`` if a selection was made.
        """

        tabCtrl = self.GetActiveTabCtrl()
        idx = tabCtrl.GetArtProvider().ShowDropDown(tabCtrl, tabCtrl.GetPages(), tabCtrl.GetActivePage())

        if not self.GetEnabled(idx):
            return False

        if idx != -1:
            e = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, tabCtrl.GetId())
            e.SetSelection(idx)
            e.SetOldSelection(tabCtrl.GetActivePage())
            e.SetEventObject(tabCtrl)
            self.GetEventHandler().ProcessEvent(e)

            return True

        else:

            return False


    def AddTabAreaButton(self, id, location, normal_bitmap=wx.NullBitmap, disabled_bitmap=wx.NullBitmap):
        """
        Adds a button in the tab area.

        :param `id`: the button identifier. This can be one of the following:

         ==============================  =================================
         Button Identifier               Description
         ==============================  =================================
         ``AUI_BUTTON_CLOSE``            Shows a close button on the tab area
         ``AUI_BUTTON_WINDOWLIST``       Shows a window list button on the tab area
         ``AUI_BUTTON_LEFT``             Shows a left button on the tab area
         ``AUI_BUTTON_RIGHT``            Shows a right button on the tab area
         ==============================  =================================

        :param `location`: the button location. Can be ``wx.LEFT`` or ``wx.RIGHT``;
        :param `normal_bitmap`: the bitmap for an enabled tab;
        :param `disabled_bitmap`: the bitmap for a disabled tab.
        """

        active_tabctrl = self.GetActiveTabCtrl()
        active_tabctrl.AddButton(id, location, normal_bitmap, disabled_bitmap)


    def RemoveTabAreaButton(self, id):
        """
        Removes a button from the tab area.

        :param `id`: the button identifier. See L{AddTabAreaButton} for a list of button identifiers.

        :see: L{AddTabAreaButton}
        """

        active_tabctrl = self.GetActiveTabCtrl()
        active_tabctrl.RemoveButton(id)


    def HasMultiplePages(self):
        """
        This method should be overridden to return ``True`` if this window has multiple pages. All
        standard class with multiple pages such as `wx.Notebook`, `wx.Listbook` and `wx.Treebook`
        already override it to return ``True`` and user-defined classes with similar behaviour
        should do it as well to allow the library to handle such windows appropriately.

        :note: Overridden from `wx.PyPanel`.
        """

        return True


    def GetDefaultBorder(self):
        """ Returns the default border style for L{AuiNotebook}. """

        return wx.BORDER_NONE


    def NotebookPreview(self, thumbnail_size=200):
        """
        Generates a preview of all the pages in the notebook (MSW and GTK only).

        :param `thumbnail_size`: the maximum size of every page thumbnail.

        :note: this functionality is currently unavailable on wxMac.
        """

        if wx.Platform == "__WXMAC__":
            return False

        tabCtrl = self.GetActiveTabCtrl()
        activePage = tabCtrl.GetActivePage()
        pages = tabCtrl.GetPages()

        pageStatus, pageText = [], []

        for indx, page in enumerate(pages):

            pageStatus.append(page.enabled)

            if not page.enabled:
                continue

            self.SetSelectionToPage(page)
            pageText.append(page.caption)

            rect = page.window.GetScreenRect()
            bmp = RescaleScreenShot(TakeScreenShot(rect), thumbnail_size)

            page.enabled = False
            if indx == 0:
                il = wx.ImageList(bmp.GetWidth(), bmp.GetHeight(), True)

            il.Add(bmp)

        # create the list control
        listCtrl = wx.ListCtrl(self, style=wx.LC_ICON|wx.LC_AUTOARRANGE|wx.LC_HRULES|wx.LC_VRULES,
                               name="__fake__page__")

        # assign the image list to it
        listCtrl.AssignImageList(il, wx.IMAGE_LIST_NORMAL)
        listCtrl.__previousStatus = [activePage, pageStatus]

        # create some items for the list
        for indx, text in enumerate(pageText):
            listCtrl.InsertImageStringItem(10000, text, indx)

        self.AddPage(listCtrl, "AuiNotebook Preview", True, bitmap=auinotebook_preview.GetBitmap(), disabled_bitmap=wx.NullBitmap)
        return True


    def SetRenamable(self, page_idx, renamable):
        """
        Sets whether a tab can be renamed via a left double-click or not.

        :param `page_idx`: the page index;
        :param `renamable`: ``True`` if the page can be renamed.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        # update our own tab catalog
        page_info = self._tabs.GetPage(page_idx)
        page_info.renamable = renamable

        # update what's on screen
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        info = ctrl.GetPage(ctrl_idx)
        info.renamable = page_info.renamable

        return True


    def IsRenamable(self, page_idx):
        """
        Returns whether a tab can be renamed or not.

        :param `page_idx`: the page index.

        :returns: ``True`` is a page can be renamed, ``False`` otherwise.
        """

        if page_idx >= self._tabs.GetPageCount():
            return False

        page_info = self._tabs.GetPage(page_idx)
        return page_info.renamable


    def OnRenameCancelled(self, page_index):
        """
        Called by L{TabTextCtrl}, to cancel the changes and to send the
        `EVT_AUINOTEBOOK_END_LABEL_EDIT` event.

        :param `page_index`: the page index in the notebook.
        """

        # let owner know that the edit was cancelled
        evt = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_END_LABEL_EDIT, self.GetId())

        evt.SetSelection(page_index)
        evt.SetEventObject(self)
        evt.SetLabel("")
        evt.SetEditCanceled(True)
        self.GetEventHandler().ProcessEvent(evt)


    def OnRenameAccept(self, page_index, value):
        """
        Called by L{TabTextCtrl}, to accept the changes and to send the
        `EVT_AUINOTEBOOK_END_LABEL_EDIT` event.

        :param `page_index`: the page index in the notebook;
        :param `value`: the new label for the tab.
        """

        evt = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_END_LABEL_EDIT, self.GetId())
        evt.SetSelection(page_index)
        evt.SetEventObject(self)
        evt.SetLabel(value)
        evt.SetEditCanceled(False)

        return not self.GetEventHandler().ProcessEvent(evt) or evt.IsAllowed()


    def ResetTextControl(self):
        """ Called by L{TabTextCtrl} when it marks itself for deletion. """

        if not self._textCtrl:
            return

        self._textCtrl.Destroy()
        self._textCtrl = None

        # tab height might have changed
        self.UpdateTabCtrlHeight(force=True)


    def EditTab(self, page_index):
        """
        Starts the editing of an item label, sending a `EVT_AUINOTEBOOK_BEGIN_LABEL_EDIT` event.

        :param `page_index`: the page index we want to edit.
        """

        if page_index >= self._tabs.GetPageCount():
            return False

        if not self.IsRenamable(page_index):
            return False

        page_info = self._tabs.GetPage(page_index)
        ctrl, ctrl_idx = self.FindTab(page_info.window)
        if not ctrl:
            return False

        evt = AuiNotebookEvent(wxEVT_COMMAND_AUINOTEBOOK_BEGIN_LABEL_EDIT, self.GetId())
        evt.SetSelection(page_index)
        evt.SetEventObject(self)
        if self.GetEventHandler().ProcessEvent(evt) and not evt.IsAllowed():
            # vetoed by user
            return False

        if self._textCtrl is not None and page_info != self._textCtrl.item():
            self._textCtrl.StopEditing()

        self._textCtrl = TabTextCtrl(ctrl, page_info, page_index)
        self._textCtrl.SetFocus()

        return True
