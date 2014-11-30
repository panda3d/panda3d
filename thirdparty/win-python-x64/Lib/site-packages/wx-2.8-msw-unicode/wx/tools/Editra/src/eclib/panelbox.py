###############################################################################
# Name: panelbox.py                                                           #
# Purpose: Advanced listbox control                                           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: PanelBox

Class PanelBox:

ListBox like container class that accepts custom panels as items in the list.

Class PanelBoxItemBase:

Base class for all PanelBoxItems. Provides the basic functionality for a custom
subclass to interact with the PanelBox container list.

Class PanelBoxItem:

Simple PanelBoxItem that has support for displaying an Icon, Main text, and a
a user defined sub item.

+-------------------------+
|                         |
| ICON   label            |
|        sub item         |
+-------------------------+

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: panelbox.py 67123 2011-03-04 00:02:35Z CJP $"
__revision__ = "$Revision: 67123 $"

#--------------------------------------------------------------------------#
# Imports
import wx
import wx.lib.scrolledpanel as scrolled

#--------------------------------------------------------------------------#

edEVT_ITEM_SELECTED = wx.NewEventType()
EVT_ITEM_SELECTED = wx.PyEventBinder(edEVT_ITEM_SELECTED, 1)

class PanelBoxEventEvent(wx.PyCommandEvent):
    """Panel Box Event Object"""
    pass

#--------------------------------------------------------------------------#

class PanelBox(scrolled.ScrolledPanel):
    """Scrolled container window for managing and displaying PanelBox items"""
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.HSCROLL|wx.VSCROLL,
                 name=u"PanelBox"):
        super(PanelBox, self).__init__(parent, id, pos, size, style, name)

        # Attributes
        self._items = list()
        self._last_sel = -1
        self._sizer = wx.BoxSizer(wx.VERTICAL)

        # Setup
        bkgrnd = wx.SystemSettings.GetColour(wx.SYS_COLOUR_LISTBOX)
        self.SetBackgroundColour(bkgrnd)

        self.SetSizer(self._sizer)
        self.SetAutoLayout(True)

        self.SetupScrolling()

        # Event Handlers
#        self.Bind(wx.EVT_KEY_UP, self.OnNavigate)
        self.GetParent().Bind(wx.EVT_KEY_UP, self.OnNavigate)

    #---- Event Handlers ----#

    def OnItemClicked(self, evt):
        """Callback from when children are clicked on
        @param evt: wx.MouseEvent

        """
        item = evt.GetEventObject()
        selected = item.IsSelected()
        idx = self.FindIndex(item)
        if idx == -1:
            return

        if evt.CmdDown():
            # Add/Remove from selection
            item.SetSelection(not selected)
        elif evt.ShiftDown():
            # Select all items between this item and the next selected one
            if idx < self._last_sel:
                inc = -1
            else:
                inc = 1

            for index in range(self._last_sel, idx + inc, inc):
                self.SetSelection(index, True)
        else:
            # Move selection to this item
            self.ClearSelections()

            if not selected:
                item.SetSelection(True)

        if not selected:
            self._last_sel = idx
        else:
            self._last_sel = -1

    def OnNavigate(self, evt):
        """Handle navigation key events"""
        key_code = evt.GetKeyCode()
        nsel = None
        isup = False
        if key_code == wx.WXK_UP:
            if self._last_sel in (0, -1):
                nsel = len(self._items) - 1
            else:
                nsel = self._last_sel - 1
            isup = True
        elif key_code == wx.WXK_DOWN:
            if self._last_sel in (-1, len(self._items) - 1):
                nsel = 0
            else:
                nsel = self._last_sel + 1
        else:
            evt.Skip()
            return

        if evt.ShiftDown():
            self.SetSelection(nsel, True)
        else:
            self.ClearSelections()
            self.SetSelection(nsel, True)

        evt.Skip()

    #---- Public Api ----#
    def AppendItem(self, item):
        """Append an item to the list
        @param item: PanelBoxItem

        """
        self._items.append(item)
        self._sizer.Add(item, 0, wx.EXPAND)
        item.Realize()

    def ClearSelections(self):
        """Unselect all items"""
        for item in self._items:
            item.SetSelection(False)

    def DeleteAllItems(self):
        """Delete all the items in the list"""
        for item in self._items:
            self._sizer.Remove(item)
            try:
                item.Destroy()
            except wx.PyDeadObjectError:
                pass

        del self._items
        self._items = list()
        self.Layout()

    def FindIndex(self, item):
        """Find the index of a given L{PanelBoxItem}
        @param item: instance of PanelBoxItemBase
        @return: int (-1 on failure)

        """
        for idx, pbitem in enumerate(self._items):
            if pbitem is item:
                return idx
        else:
            return -1

    def GetItemCount(self):
        """Get the number of items in the control
        @return: int

        """
        return len(self._items)

    def GetItems(self):
        """Get the list of items held by this control
        @return: list of PanelBoxItems
        @todo: should probably return a list of shadow items so that orignals
               are not modified.

        """
        return self._items

    def GetSelection(self):
        """Get the (first) selected item"""
        for item in self._items:
            if item.IsSelected():
                return item
        else:
            return None

    def GetSelections(self):
        """Get the list of selected items
        @return: list

        """
        return [item for item in self._items if item.IsSelected()]

    def InsertItem(self, index, item):
        """Insert an item into the list
        @param index: index to insert at
        @param item: PanelBoxItem

        """
        if index <= len(self._items):
            self._items.insert(index, item)
            self._sizer.Insert(index, item, 0, wx.EXPAND)
        else:
            raise IndexError, "Index %d: out of range" % index

    def Remove(self, index):
        """Remove an item from the list
        @param index: item index

        """
        if index < len(self._items):
            item = self._items.pop(index)
            self._sizer.Remove(item)
            self.Layout()
        else:
            raise IndexError, "Index %d: out of range" % index

    def RemoveAll(self):
        """Remove all items from the list"""
        for item in self._items:
            self._sizer.Remove(item)

        del self._items
        self._items = list()

        self.Layout()

    def SetSelection(self, idx, select=True):
        """Set the selection on a given index
        @param idx: int
        @keyword select: bool

        """
        if idx < len(self._items):
            item = self._items[idx]
            item.SetSelection(select)
            self._last_sel = idx
        else:
            raise IndexError, "Index out of range: %d > %d" (idx, len(self._items))

#--------------------------------------------------------------------------#

class PanelBoxItemBase(wx.PyPanel):
    """Base L{PanelBox} Item"""
    def __init__(self, parent):
        """Create a PanelBoxItem"""
        super(PanelBoxItemBase, self).__init__(parent,
                                               style=wx.NO_BORDER|wx.TAB_TRAVERSAL)

        # Attributes
        self._selected = False

        # Event Handlers
        self.Bind(wx.EVT_PAINT, self.OnPaint)
#        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOUSEWHEEL, self.OnMouseWheel)

    def _UpdateForeground(self, color):
        """Update foreground colors when selection changes
        @param color: selection color
        @todo: should cache text's original color to restore
               on de-selection.

        """
        if sum(color.Get()[:3]) < (127 * 3):
            ncolor = wx.WHITE
        else:
            ncolor = wx.BLACK

        for child in self.GetChildren():
            if hasattr(child, 'SetForegroundColour') and \
               not isinstance(child, wx.Button):
                child.SetForegroundColour(ncolor)

    def OnKeyUp(self, evt):
        """Handle key navigation events"""
        self.GetParent().OnNavigate(evt)
        evt.Skip()

    def OnLeftUp(self, evt):
        """Handle when the item is clicked on"""
        e_obj = evt.GetEventObject()
        evt.SetEventObject(self)
        self.GetParent().OnItemClicked(evt)
        evt.SetEventObject(e_obj)
        evt.Skip()

    def OnMouseWheel(self, evt):
        """Relay the mouse wheel events to the panel box"""
        self.GetParent().GetEventHandler().ProcessEvent(evt)
        evt.Skip()

    def OnPaint(self, evt):
        """Paint the items background"""
        dc = wx.PaintDC(self)
        rect = self.GetClientRect()

#        if self.IsSelected():
#            color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
#            dc.SetBrush(wx.Brush(color))
#            dc.SetPen(wx.TRANSPARENT_PEN)
#            dc.DrawRectangle(*rect)
#        else:
#            col2 = wx.SystemSettings_GetColour(wx.SYS_COLOUR_LISTBOX)
#            dc.SetBackground(wx.Brush(col2))
#            dc.SetBrush(wx.Brush(col2))
#            dc.SetPen(wx.TRANSPARENT_PEN)
#            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height - 1)

        pcolor = wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DFACE)
        dc.SetPen(wx.Pen(pcolor))
        dc.DrawLine(rect.x, rect.bottom, rect.right, rect.bottom)

    def IsSelected(self):
        """Is this item selected
        @return: bool

        """
        return self._selected

    def Realize(self):
        """Finalize initialization of the panel item"""
        for child in self.GetChildren():
            child.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)

    def SetSelection(self, select=False):
        """Set the selection state on this item
        @keyword select: bool

        """
        self._selected = select
        if self._selected:
            color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        else:
            color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_LISTBOX)
        self.SetBackgroundColour(color)
        self._UpdateForeground(color)
        self.Refresh()

#--------------------------------------------------------------------------#

class PanelBoxItem(PanelBoxItemBase):
    """L{PanelBox} Item that has an icon, main label text and sub label
    +-------------------------+
    |                         |
    | ICON   label            |
    |        sub item         |
    +-------------------------+

    """
    def __init__(self, parent, bmp=None, label=u'', sub=None):
        """Create teh PanelBoxItem
        @param parent: L{PanelBox}
        @keyword bmp: wx.Bitmap
        @keyword label: string
        @keyword sub: Window object or None

        """
        super(PanelBoxItem, self).__init__(parent)

        # Attributes
        self._bmp = bmp
        self._label = label
        self._sub = sub

        # Layout
        self.__DoLayout()
        self.SetAutoLayout(True)

    def __DoLayout(self):
        """Layout the control"""
        vsizer = wx.BoxSizer(wx.VERTICAL)
        hsizer = wx.BoxSizer(wx.HORIZONTAL)

        hsizer.Add((8, 8), 0)
        if self._bmp is not None:
            self._bmp = wx.StaticBitmap(self, bitmap=self._bmp)
            hsizer.Add(self._bmp, 0, wx.ALIGN_CENTER_VERTICAL)
            hsizer.Add((5, 5), 0)

        # Add Label Text
        isizer = wx.BoxSizer(wx.VERTICAL)
        self._label = wx.StaticText(self, label=self._label)
        isizer.Add(self._label, 0, wx.ALIGN_LEFT)
        if self._sub is not None:
            isizer.Add((3, 3), 0)

        # Add Subwindow if one is defined
        if self._sub is not None:
            self._sub.Reparent(self)
            s_sizer = wx.BoxSizer(wx.HORIZONTAL)
            s_sizer.Add(self._sub, 1, wx.EXPAND)
            isizer.Add(s_sizer, 1, wx.EXPAND)

        hsizer.Add(isizer, 1, wx.ALIGN_CENTER_VERTICAL)
        hsizer.Add((8, 8), 0)
        vsizer.AddMany([((8, 8), 0), (hsizer, 0, wx.EXPAND), ((8, 8), 0)])
        self.SetSizer(vsizer)

    def SetBitmap(self, bmp):
        """Set the items image
        param bmp: wx.Bitmap

        """
        self._bmp.SetBitmap(bmp)
        self._bmp.Refresh()
        self.Layout()

    def SetLabel(self, lbl):
        """Set the label text
        @param lbl: string

        """
        self._lbl.SetLabel(lbl)
        self._lbl.Refresh()
        self.Layout()

    def SetSecondaryCtrl(self, ctrl):
        """Set the secondary control
        @param ctrl: wxWindow

        """
        pass
