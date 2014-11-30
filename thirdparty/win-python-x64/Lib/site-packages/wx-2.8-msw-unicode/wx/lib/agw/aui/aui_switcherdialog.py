"""
Description
===========

The idea of `SwitcherDialog` is to make it easier to implement keyboard
navigation in AUI and other applications that have multiple panes and
tabs.

A key combination with a modifier (such as ``Ctrl`` + ``Tab``) shows the
dialog, and the user holds down the modifier whilst navigating with
``Tab`` and arrow keys before releasing the modifier to dismiss the dialog
and activate the selected pane.

The switcher dialog is a multi-column menu with no scrolling, implemented
by the `MultiColumnListCtrl` class. You can have headings for your items
for logical grouping, and you can force a column break if you need to.

The modifier used for invoking and dismissing the dialog can be customised,
as can the colours, number of rows, and the key used for cycling through
the items. So you can use different keys on different platforms if
required (especially since ``Ctrl`` + ``Tab`` is reserved on some platforms).

Items are shown as names and optional 16x16 images.


Base Functionalities
====================

To use the dialog, you set up the items in a `SwitcherItems` object,
before passing this to the `SwitcherDialog` instance.

Call L{SwitcherItems.AddItem} and optionally L{SwitcherItems.AddGroup} to add items and headings. These
functions take a label (to be displayed to the user), an identifying name,
an integer id, and a bitmap. The name and id are purely for application-defined
identification. You may also set a description to be displayed when each
item is selected; and you can set a window pointer for convenience when
activating the desired window after the dialog returns.

Have created the dialog, you call `ShowModal()`, and if the return value is
``wx.ID_OK``, retrieve the selection from the dialog and activate the pane.

The sample code below shows a generic method of finding panes and notebook
tabs within the current L{AuiManager}, and using the pane name or notebook
tab position to display the pane.

The only other code to add is a menu item with the desired accelerator,
whose modifier matches the one you pass to L{SwitcherDialog.SetModifierKey} 
(the default being ``wx.WXK_CONTROL``).


Usage
=====

Menu item::

    if wx.Platform == "__WXMAC__":
        switcherAccel = "Alt+Tab"
    elif wx.Platform == "__WXGTK__":
        switcherAccel = "Ctrl+/"
    else:
        switcherAccel = "Ctrl+Tab"

    view_menu.Append(ID_SwitchPane, _("S&witch Window...") + "\t" + switcherAccel)


Event handler::

    def OnSwitchPane(self, event):

        items = SwitcherItems()
        items.SetRowCount(12)

        # Add the main windows and toolbars, in two separate columns
        # We'll use the item 'id' to store the notebook selection, or -1 if not a page

        for k in xrange(2):
            if k == 0:
                items.AddGroup(_("Main Windows"), "mainwindows")
            else:
                items.AddGroup(_("Toolbars"), "toolbars").BreakColumn()

            for pane in self._mgr.GetAllPanes():
                name = pane.name
                caption = pane.caption

                toolbar = isinstance(info.window, wx.ToolBar) or isinstance(info.window, aui.AuiToolBar)
                if caption and (toolBar  and k == 1) or (not toolBar and k == 0):
                    items.AddItem(caption, name, -1).SetWindow(pane.window)

        # Now add the wxAuiNotebook pages

        items.AddGroup(_("Notebook Pages"), "pages").BreakColumn()

        for pane in self._mgr.GetAllPanes():
            nb = pane.window
            if isinstance(nb, aui.AuiNotebook):
                for j in xrange(nb.GetPageCount()):

                    name = nb.GetPageText(j)
                    win = nb.GetPage(j)

                    items.AddItem(name, name, j, nb.GetPageBitmap(j)).SetWindow(win)

        # Select the focused window

        idx = items.GetIndexForFocus()
        if idx != wx.NOT_FOUND:
            items.SetSelection(idx)

        if wx.Platform == "__WXMAC__":
            items.SetBackgroundColour(wx.WHITE)
        
        # Show the switcher dialog

        dlg = SwitcherDialog(items, wx.GetApp().GetTopWindow())

        # In GTK+ we can't use Ctrl+Tab; we use Ctrl+/ instead and tell the switcher
        # to treat / in the same was as tab (i.e. cycle through the names)

        if wx.Platform == "__WXGTK__":
            dlg.SetExtraNavigationKey(wxT('/'))

        if wx.Platform == "__WXMAC__":
            dlg.SetBackgroundColour(wx.WHITE)
            dlg.SetModifierKey(wx.WXK_ALT)

        ans = dlg.ShowModal()

        if ans == wx.ID_OK and dlg.GetSelection() != -1:
            item = items.GetItem(dlg.GetSelection())

            if item.GetId() == -1:
                info = self._mgr.GetPane(item.GetName())
                info.Show()
                self._mgr.Update()
                info.window.SetFocus()

            else:
                nb = item.GetWindow().GetParent()
                win = item.GetWindow();
                if isinstance(nb, aui.AuiNotebook):
                    nb.SetSelection(item.GetId())
                    win.SetFocus()


"""

import wx

import auibook
from aui_utilities import FindFocusDescendant
from aui_constants import SWITCHER_TEXT_MARGIN_X, SWITCHER_TEXT_MARGIN_Y


# Define a translation function
_ = wx.GetTranslation

    
class SwitcherItem(object):
    """ An object containing information about one item. """
    
    def __init__(self, item=None):
        """ Default class constructor. """

        self._id = 0
        self._isGroup = False
        self._breakColumn = False
        self._rowPos = 0
        self._colPos = 0
        self._window = None
        self._description = ""

        self._textColour = wx.NullColour
        self._bitmap = wx.NullBitmap
        self._font = wx.NullFont
        
        if item:
            self.Copy(item)


    def Copy(self, item):
        """
        Copy operator between 2 L{SwitcherItem} instances.

        :param `item`: another instance of L{SwitcherItem}.
        """

        self._id = item._id
        self._name = item._name
        self._title = item._title
        self._isGroup = item._isGroup
        self._breakColumn = item._breakColumn
        self._rect = item._rect
        self._font = item._font
        self._textColour = item._textColour
        self._bitmap = item._bitmap
        self._description = item._description
        self._rowPos = item._rowPos
        self._colPos = item._colPos
        self._window = item._window


    def SetTitle(self, title):

        self._title = title
        return self
    

    def GetTitle(self):
        
        return self._title


    def SetName(self, name):

        self._name = name
        return self

    
    def GetName(self):

        return self._name


    def SetDescription(self, descr):

        self._description = descr
        return self


    def GetDescription(self):

        return self._description
    

    def SetId(self, id):

        self._id = id
        return self

    
    def GetId(self):

        return self._id


    def SetIsGroup(self, isGroup):

        self._isGroup = isGroup
        return self

    
    def GetIsGroup(self):

        return self._isGroup
    

    def BreakColumn(self, breakCol=True):

        self._breakColumn = breakCol
        return self

    
    def GetBreakColumn(self):

        return self._breakColumn


    def SetRect(self, rect):

        self._rect = rect
        return self

    
    def GetRect(self):
        
        return self._rect


    def SetTextColour(self, colour):

        self._textColour = colour
        return self

    
    def GetTextColour(self):

        return self._textColour
    

    def SetFont(self, font):

        self._font = font
        return self

    
    def GetFont(self):

        return self._font
    

    def SetBitmap(self, bitmap):

        self._bitmap = bitmap
        return self

    
    def GetBitmap(self):

        return self._bitmap


    def SetRowPos(self, pos):

        self._rowPos = pos
        return self

    
    def GetRowPos(self):

        return self._rowPos
    

    def SetColPos(self, pos):

        self._colPos = pos
        return self

    
    def GetColPos(self):

        return self._colPos
    

    def SetWindow(self, win):

        self._window = win
        return self
    

    def GetWindow(self):

        return self._window

    
class SwitcherItems(object):
    """ An object containing switcher items. """

    def __init__(self, items=None):
        """ Default class constructor. """

        self._selection = -1
        self._rowCount = 10
        self._columnCount = 0

        self._backgroundColour = wx.NullColour
        self._textColour = wx.NullColour
        self._selectionColour = wx.NullColour
        self._selectionOutlineColour = wx.NullColour
        self._itemFont = wx.NullFont

        self._items = []        
        
        if wx.Platform == "__WXMSW__":
            # If on Windows XP/Vista, use more appropriate colours
            self.SetSelectionOutlineColour(wx.Colour(49, 106, 197))
            self.SetSelectionColour(wx.Colour(193, 210, 238))

        if items:
            self.Copy(items)
            

    def Copy(self, items):
        """
        Copy operator between 2 L{SwitcherItems}.

        :param `items`: another instance of L{SwitcherItems}.
        """
        
        self.Clear()

        for item in items._items:
            self._items.append(item)
        
        self._selection = items._selection
        self._rowCount = items._rowCount
        self._columnCount = items._columnCount

        self._backgroundColour = items._backgroundColour
        self._textColour = items._textColour
        self._selectionColour = items._selectionColour
        self._selectionOutlineColour = items._selectionOutlineColour
        self._itemFont = items._itemFont


    def AddItem(self, titleOrItem, name=None, id=0, bitmap=wx.NullBitmap):

        if isinstance(titleOrItem, SwitcherItem):
            self._items.append(titleOrItem)
            return self._items[-1]
        
        item = SwitcherItem()
        item.SetTitle(titleOrItem)
        item.SetName(name)
        item.SetId(id)
        item.SetBitmap(bitmap)

        self._items.append(item)
        return self._items[-1]


    def AddGroup(self, title, name, id=0, bitmap=wx.NullBitmap):

        item = self.AddItem(title, name, id, bitmap)
        item.SetIsGroup(True)

        return item


    def Clear(self):

        self._items = []


    def FindItemByName(self, name):

        for i in xrange(len(self._items)):
            if self._items[i].GetName() == name:
                return i
        
        return wx.NOT_FOUND


    def FindItemById(self, id):

        for i in xrange(len(self._items)):
            if self._items[i].GetId() == id:
                return i
        
        return wx.NOT_FOUND


    def SetSelection(self, sel):

        self._selection = sel


    def SetSelectionByName(self, name):

        idx = self.FindItemByName(name)
        if idx != wx.NOT_FOUND:
            self.SetSelection(idx)


    def GetSelection(self):

        return self._selection
    

    def GetItem(self, i):

        return self._items[i]


    def GetItemCount(self):

        return len(self._items)
    

    def SetRowCount(self, rows):

        self._rowCount = rows

        
    def GetRowCount(self):

        return self._rowCount


    def SetColumnCount(self, cols):

        self._columnCount = cols

        
    def GetColumnCount(self):

        return self._columnCount
    

    def SetBackgroundColour(self, colour):

        self._backgroundColour = colour

        
    def GetBackgroundColour(self):

        return self._backgroundColour
    

    def SetTextColour(self, colour):

        self._textColour = colour

        
    def GetTextColour(self):

        return self._textColour
    

    def SetSelectionColour(self, colour):

        self._selectionColour = colour

        
    def GetSelectionColour(self):

        return self._selectionColour
    

    def SetSelectionOutlineColour(self, colour):

        self._selectionOutlineColour = colour

        
    def GetSelectionOutlineColour(self):

        return self._selectionOutlineColour
    

    def SetItemFont(self, font):

        self._itemFont = font

        
    def GetItemFont(self):

        return self._itemFont 
    

    def PaintItems(self, dc, win):

        backgroundColour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DFACE)
        standardTextColour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_WINDOWTEXT)
        selectionColour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        selectionOutlineColour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_WINDOWTEXT)
        standardFont = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        groupFont = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        groupFont.SetWeight(wx.BOLD)

        if self.GetBackgroundColour().IsOk():
            backgroundColour = self.GetBackgroundColour()

        if self.GetTextColour().IsOk():
            standardTextColour = self.GetTextColour()

        if self.GetSelectionColour().IsOk():
            selectionColour = self.GetSelectionColour()

        if self.GetSelectionOutlineColour().IsOk():
            selectionOutlineColour = self.GetSelectionOutlineColour()

        if self.GetItemFont().IsOk():
        
            standardFont = self.GetItemFont()   
            groupFont = wx.Font(standardFont.GetPointSize(), standardFont.GetFamily(), standardFont.GetStyle(),
                                wx.BOLD, standardFont.GetUnderlined(), standardFont.GetFaceName())
        
        textMarginX = SWITCHER_TEXT_MARGIN_X

        dc.SetLogicalFunction(wx.COPY)
        dc.SetBrush(wx.Brush(backgroundColour))
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.DrawRectangleRect(win.GetClientRect())
        dc.SetBackgroundMode(wx.TRANSPARENT)

        for i in xrange(len(self._items)):
            item = self._items[i]
            if i == self._selection:
                dc.SetPen(wx.Pen(selectionOutlineColour))
                dc.SetBrush(wx.Brush(selectionColour))
                dc.DrawRectangleRect(item.GetRect())
            
            clippingRect = wx.Rect(*item.GetRect())
            clippingRect.Deflate(1, 1)

            dc.SetClippingRect(clippingRect)

            if item.GetTextColour().IsOk():
                dc.SetTextForeground(item.GetTextColour())
            else:
                dc.SetTextForeground(standardTextColour)
            
            if item.GetFont().IsOk():
                dc.SetFont(item.GetFont())
            else:
                if item.GetIsGroup():
                    dc.SetFont(groupFont)
                else:
                    dc.SetFont(standardFont)
            
            w, h = dc.GetTextExtent(item.GetTitle())
            x = item.GetRect().x

            x += textMarginX

            if not item.GetIsGroup():
                if item.GetBitmap().IsOk() and item.GetBitmap().GetWidth() <= 16 \
                   and item.GetBitmap().GetHeight() <= 16:
                    x -= textMarginX
                    dc.DrawBitmap(item.GetBitmap(), x, item.GetRect().y + \
                                  (item.GetRect().height - item.GetBitmap().GetHeight())/2,
                                  True)
                    x += 16 + textMarginX
                #x += textMarginX
            
            y = item.GetRect().y + (item.GetRect().height - h)/2
            dc.DrawText(item.GetTitle(), x, y)
            dc.DestroyClippingRegion()
    

    def CalculateItemSize(self, dc):

        # Start off allowing for an icon
        sz = wx.Size(150, 16)
        standardFont = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        groupFont = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        groupFont.SetWeight(wx.BOLD)

        textMarginX = SWITCHER_TEXT_MARGIN_X
        textMarginY = SWITCHER_TEXT_MARGIN_Y
        maxWidth = 300
        maxHeight = 40

        if self.GetItemFont().IsOk():
            standardFont = self.GetItemFont()   

        for item in self._items:
            if item.GetFont().IsOk():
                dc.SetFont(item.GetFont())
            else:
                if item.GetIsGroup():
                    dc.SetFont(groupFont)
                else:
                    dc.SetFont(standardFont)

            w, h = dc.GetTextExtent(item.GetTitle())
            w += 16 + 2*textMarginX

            if w > sz.x:
                sz.x = min(w, maxWidth)
            if h > sz.y:
                sz.y = min(h, maxHeight)
        
        if sz == wx.Size(16, 16):
            sz = wx.Size(100, 25)
        else:
            sz.x += textMarginX*2
            sz.y += textMarginY*2
        
        return sz


    def GetIndexForFocus(self):

        for i, item in enumerate(self._items):        
            if item.GetWindow():
            
                if FindFocusDescendant(item.GetWindow()):
                    return i
            
        return wx.NOT_FOUND


class MultiColumnListCtrl(wx.PyControl):
    """ A control for displaying several columns (not scrollable). """

    def __init__(self, parent, aui_manager, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, validator=wx.DefaultValidator, name="MultiColumnListCtrl"):

        wx.PyControl.__init__(self, parent, id, pos, size, style, validator, name)

        self._overallSize = wx.Size(200, 100)
        self._modifierKey = wx.WXK_CONTROL
        self._extraNavigationKey = 0
        self._aui_manager = aui_manager
        
        self.SetInitialSize(size)
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouseEvent)
        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKey)
        self.Bind(wx.EVT_KEY_UP, self.OnKey)


    def __del__(self):

        self._aui_manager.HideHint()

        
    def DoGetBestSize(self):

        return self._overallSize


    def OnEraseBackground(self, event):
        
        pass


    def OnPaint(self, event):

        dc = wx.AutoBufferedPaintDC(self)
        rect = self.GetClientRect()

        if self._items.GetColumnCount() == 0:
            self.CalculateLayout(dc)

        if self._items.GetColumnCount() == 0:
            return

        self._items.PaintItems(dc, self)


    def OnMouseEvent(self, event):

        if event.LeftDown():
            self.SetFocus()
    

    def OnChar(self, event):

        event.Skip()        


    def OnKey(self, event):

        if event.GetEventType() == wx.wxEVT_KEY_UP:
            if event.GetKeyCode() == self.GetModifierKey():
                topLevel = wx.GetTopLevelParent(self)
                closeEvent = wx.CloseEvent(wx.wxEVT_CLOSE_WINDOW, topLevel.GetId())
                closeEvent.SetEventObject(topLevel)
                closeEvent.SetCanVeto(False)
                
                topLevel.GetEventHandler().ProcessEvent(closeEvent)
                return
                
            event.Skip()
            return

        keyCode = event.GetKeyCode()
        
        if keyCode in [wx.WXK_ESCAPE, wx.WXK_RETURN]:
            if keyCode == wx.WXK_ESCAPE:
                self._items.SetSelection(-1)

            topLevel = wx.GetTopLevelParent(self)
            closeEvent = wx.CloseEvent(wx.wxEVT_CLOSE_WINDOW, topLevel.GetId())
            closeEvent.SetEventObject(topLevel)
            closeEvent.SetCanVeto(False)
            
            topLevel.GetEventHandler().ProcessEvent(closeEvent)
            return
        
        elif keyCode in [wx.WXK_TAB, self.GetExtraNavigationKey()]:
            if event.ShiftDown():
            
                self._items.SetSelection(self._items.GetSelection() - 1)
                if self._items.GetSelection() < 0:
                    self._items.SetSelection(self._items.GetItemCount() - 1)

                self.AdvanceToNextSelectableItem(-1)
            
            else:
            
                self._items.SetSelection(self._items.GetSelection() + 1)
                if self._items.GetSelection() >= self._items.GetItemCount():
                    self._items.SetSelection(0)

                self.AdvanceToNextSelectableItem(1)
            
            self.GenerateSelectionEvent()
            self.Refresh()
        
        elif keyCode in [wx.WXK_DOWN, wx.WXK_NUMPAD_DOWN]:
            self._items.SetSelection(self._items.GetSelection() + 1)
            if self._items.GetSelection() >= self._items.GetItemCount():
                self._items.SetSelection(0)
            
            self.AdvanceToNextSelectableItem(1)
            self.GenerateSelectionEvent()
            self.Refresh()
        
        elif keyCode in [wx.WXK_UP, wx.WXK_NUMPAD_UP]:
            self._items.SetSelection(self._items.GetSelection() - 1)
            if self._items.GetSelection() < 0:
                self._items.SetSelection(self._items.GetItemCount() - 1)
            
            self.AdvanceToNextSelectableItem(-1)
            self.GenerateSelectionEvent()
            self.Refresh()
        
        elif keyCode in [wx.WXK_HOME, wx.WXK_NUMPAD_HOME]:
            self._items.SetSelection(0)
            self.AdvanceToNextSelectableItem(1)
            self.GenerateSelectionEvent()
            self.Refresh()
        
        elif keyCode in [wx.WXK_END, wx.WXK_NUMPAD_END]:
            self._items.SetSelection(self._items.GetItemCount() - 1)
            self.AdvanceToNextSelectableItem(-1)
            self.GenerateSelectionEvent()
            self.Refresh()
        
        elif keyCode in [wx.WXK_LEFT, wx.WXK_NUMPAD_LEFT]:
            item = self._items.GetItem(self._items.GetSelection())

            row = item.GetRowPos()
            newCol = item.GetColPos() - 1
            if newCol < 0:
                newCol = self._items.GetColumnCount() - 1

            # Find the first item from the end whose row matches and whose column is equal or lower
            for i in xrange(self._items.GetItemCount()-1, -1, -1):
                item2 = self._items.GetItem(i)
                if item2.GetColPos() == newCol and item2.GetRowPos() <= row:
                    self._items.SetSelection(i)
                    break

            self.AdvanceToNextSelectableItem(-1)
            self.GenerateSelectionEvent()
            self.Refresh()
        
        elif keyCode in [wx.WXK_RIGHT, wx.WXK_NUMPAD_RIGHT]:
            item = self._items.GetItem(self._items.GetSelection())

            row = item.GetRowPos()
            newCol = item.GetColPos() + 1
            if newCol >= self._items.GetColumnCount():
                newCol = 0

            # Find the first item from the end whose row matches and whose column is equal or lower
            for i in xrange(self._items.GetItemCount()-1, -1, -1):
                item2 = self._items.GetItem(i)
                if item2.GetColPos() == newCol and item2.GetRowPos() <= row:
                    self._items.SetSelection(i)
                    break

            self.AdvanceToNextSelectableItem(1)
            self.GenerateSelectionEvent()
            self.Refresh()
        
        else:
            event.Skip()


    def AdvanceToNextSelectableItem(self, direction):

        if self._items.GetItemCount() < 2:
            return

        if self._items.GetSelection() == -1:
            self._items.SetSelection(0)

        oldSel = self._items.GetSelection()

        while 1:
        
            if self._items.GetItem(self._items.GetSelection()).GetIsGroup():
            
                self._items.SetSelection(self._items.GetSelection() + direction)
                if self._items.GetSelection() == -1:
                    self._items.SetSelection(self._items.GetItemCount()-1)
                elif self._items.GetSelection() == self._items.GetItemCount():
                    self._items.SetSelection(0)
                if self._items.GetSelection() == oldSel:
                    break
            
            else:
                break

        self.SetTransparency()
        selection = self._items.GetItem(self._items.GetSelection()).GetWindow()
        pane = self._aui_manager.GetPane(selection)

        if not pane.IsOk():
            if isinstance(selection.GetParent(), auibook.AuiNotebook):
                self.SetTransparency(selection)
                self._aui_manager.ShowHint(selection.GetScreenRect())
                wx.CallAfter(self.SetFocus)
                self.SetFocus()
                return
            else:
                self._aui_manager.HideHint()
                return
        if not pane.IsShown():
            self._aui_manager.HideHint()
            return

        self.SetTransparency(selection)
        self._aui_manager.ShowHint(selection.GetScreenRect())
        # NOTE: this is odd but it is the only way for the focus to
        #       work correctly on wxMac...
        wx.CallAfter(self.SetFocus)
        self.SetFocus()        
    

    def SetTransparency(self, selection=None):

        if not self.GetParent().CanSetTransparent():
            return
        
        if selection is not None:
            intersects = False
            if selection.GetScreenRect().Intersects(self.GetParent().GetScreenRect()):
                intersects = True
                self.GetParent().SetTransparent(200)
                return

        self.GetParent().SetTransparent(255)


    def GenerateSelectionEvent(self):

        event = wx.CommandEvent(wx.wxEVT_COMMAND_LISTBOX_SELECTED, self.GetId())
        event.SetEventObject(self)
        event.SetInt(self._items.GetSelection())
        self.GetEventHandler().ProcessEvent(event)


    def CalculateLayout(self, dc=None):

        if dc is None:
            dc = wx.ClientDC(self)

        if self._items.GetSelection() == -1:
            self._items.SetSelection(0)

        columnCount = 1

        # Spacing between edge of window or between columns
        xMargin = 4
        yMargin = 4

        # Inter-row spacing
        rowSpacing = 2

        itemSize = self._items.CalculateItemSize(dc)
        self._overallSize = wx.Size(350, 200)

        currentRow = 0
        x = xMargin
        y = yMargin

        breaking = False
        i = 0
        
        while 1:
        
            oldOverallSize = self._overallSize
            item = self._items.GetItem(i)
            
            item.SetRect(wx.Rect(x, y, itemSize.x, itemSize.y))
            item.SetColPos(columnCount-1)
            item.SetRowPos(currentRow)

            if item.GetRect().GetBottom() > self._overallSize.y:
                self._overallSize.y = item.GetRect().GetBottom() + yMargin

            if item.GetRect().GetRight() > self._overallSize.x:
                self._overallSize.x = item.GetRect().GetRight() + xMargin

            currentRow += 1

            y += rowSpacing + itemSize.y
            stopBreaking = breaking

            if currentRow > self._items.GetRowCount() or (item.GetBreakColumn() and not breaking and currentRow != 1):
                currentRow = 0
                columnCount += 1
                x += xMargin + itemSize.x
                y = yMargin

                # Make sure we don't orphan a group
                if item.GetIsGroup() or (item.GetBreakColumn() and not breaking):
                    self._overallSize = oldOverallSize

                    if item.GetBreakColumn():
                        breaking = True

                    # Repeat the last item, in the next column
                    i -= 1
                
            if stopBreaking:
                breaking = False

            i += 1
            
            if i >= self._items.GetItemCount():
                break
            
        self._items.SetColumnCount(columnCount)
        self.InvalidateBestSize()


    def SetItems(self, items):
        
        self._items = items

        
    def GetItems(self):

        return self._items

 
    def SetExtraNavigationKey(self, keyCode):
        """
        Set an extra key that can be used to cycle through items,
        in case not using the ``Ctrl`` + ``Tab`` combination.
        """

        self._extraNavigationKey = keyCode


    def GetExtraNavigationKey(self):

        return self._extraNavigationKey


    def SetModifierKey(self, modifierKey):
        """
        Set the modifier used to invoke the dialog, and therefore to test for
        release.
        """

        self._modifierKey = modifierKey

        
    def GetModifierKey(self):

        return self._modifierKey

    

class SwitcherDialog(wx.Dialog):
    """
    SwitcherDialog shows a L{MultiColumnListCtrl} with a list of panes
    and tabs for the user to choose. ``Ctrl`` + ``Tab`` cycles through them.
    """

    def __init__(self, items, parent, aui_manager, id=wx.ID_ANY, title=_("Pane Switcher"), pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.STAY_ON_TOP|wx.DIALOG_NO_PARENT|wx.BORDER_SIMPLE):
        """ Default class constructor. """
        
        self._switcherBorderStyle = (style & wx.BORDER_MASK)
        if self._switcherBorderStyle == wx.BORDER_NONE:
            self._switcherBorderStyle = wx.BORDER_SIMPLE

        style &= wx.BORDER_MASK
        style |= wx.BORDER_NONE

        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        self._listCtrl = MultiColumnListCtrl(self, aui_manager,
                                             style=wx.WANTS_CHARS|wx.NO_BORDER)
        self._listCtrl.SetItems(items)
        self._listCtrl.CalculateLayout()

        self._descriptionCtrl = wx.html.HtmlWindow(self, size=(-1, 100), style=wx.BORDER_NONE)
        self._descriptionCtrl.SetBackgroundColour(self.GetBackgroundColour())

        if wx.Platform == "__WXGTK__":
            fontSize = 11
            self._descriptionCtrl.SetStandardFonts(fontSize)

        sizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(sizer)
        sizer.Add(self._listCtrl, 1, wx.ALL|wx.EXPAND, 10)
        sizer.Add(self._descriptionCtrl, 0, wx.ALL|wx.EXPAND, 10)
        sizer.SetSizeHints(self)

        self._listCtrl.SetFocus()

        self.Centre(wx.BOTH)

        if self._listCtrl.GetItems().GetSelection() == -1:
            self._listCtrl.GetItems().SetSelection(0)

        self._listCtrl.AdvanceToNextSelectableItem(1)

        self.ShowDescription(self._listCtrl.GetItems().GetSelection())

        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
        self.Bind(wx.EVT_ACTIVATE, self.OnActivate)
        self.Bind(wx.EVT_LISTBOX, self.OnSelectItem)
        self.Bind(wx.EVT_PAINT, self.OnPaint)

        # Attributes
        self._closing = False
        if wx.Platform == "__WXMSW__":
            self._borderColour = wx.Colour(49, 106, 197)
        else:
            self._borderColour = wx.BLACK

        self._aui_manager = aui_manager
        

    def OnCloseWindow(self, event):

        if self._closing:
            return

        if self.IsModal():
            self._closing = True

            if self.GetSelection() == -1:
                self.EndModal(wx.ID_CANCEL)
            else:
                self.EndModal(wx.ID_OK)
    
        self._aui_manager.HideHint()


    def GetSelection(self):

        return self._listCtrl.GetItems().GetSelection()


    def OnActivate(self, event):

        if not event.GetActive():
            if not self._closing:
                self._closing = True
                self.EndModal(wx.ID_CANCEL)
            

    def OnPaint(self, event):

        dc = wx.PaintDC(self)

        if self._switcherBorderStyle == wx.BORDER_SIMPLE:
        
            dc.SetPen(wx.Pen(self._borderColour))
            dc.SetBrush(wx.TRANSPARENT_BRUSH)

            rect = self.GetClientRect()
            dc.DrawRectangleRect(rect)

            # Draw border around the HTML control
            rect = wx.Rect(*self._descriptionCtrl.GetRect())
            rect.Inflate(1, 1)
            dc.DrawRectangleRect(rect)

    
    def OnSelectItem(self, event):

        self.ShowDescription(event.GetSelection())


# Convert a colour to a 6-digit hex string
    def ColourToHexString(self, col):

        hx = '%02x%02x%02x' % tuple([int(c) for c in col])
        return hx


    def ShowDescription(self, i):

        item = self._listCtrl.GetItems().GetItem(i)
        colour = self._listCtrl.GetItems().GetBackgroundColour()
        
        if not colour.IsOk():
            colour = self.GetBackgroundColour()

        backgroundColourHex = self.ColourToHexString(colour)
        html = _("<body bgcolor=\"#") + backgroundColourHex + _("\"><b>") + item.GetTitle() + _("</b>")

        if item.GetDescription():
            html += _("<p>")
            html += item.GetDescription()
        
        html += _("</body>")
        self._descriptionCtrl.SetPage(html)


    def SetExtraNavigationKey(self, keyCode):

        self._extraNavigationKey = keyCode
        if self._listCtrl:
            self._listCtrl.SetExtraNavigationKey(keyCode)


    def GetExtraNavigationKey(self):

        return self._extraNavigationKey
    
        
    def SetModifierKey(self, modifierKey):

        self._modifierKey = modifierKey
        if self._listCtrl:
            self._listCtrl.SetModifierKey(modifierKey)


    def GetModifierKey(self):

        return self._modifierKey        


    def SetBorderColour(self, colour):

        self._borderColour = colour

        