"""
A ribbon gallery is like a `wx.ListBox`, but for bitmaps rather than strings.


Description
===========

It displays a collection of bitmaps arranged in a grid and allows the user to
choose one. As there are typically more bitmaps in a gallery than can be displayed
in the space used for a ribbon, a gallery always has scroll buttons to allow the
user to navigate through the entire gallery.

It also has an "extension" button, the behaviour of which is outside the scope of
the gallery control itself, though it typically displays some kind of dialog related
to the gallery.


Events Processing
=================

This class processes the following events:

=================================== ===================================
Event Name                          Description
=================================== ===================================
``EVT_RIBBONGALLERY_SELECTED``      Triggered when the user selects an item from the gallery. Note that the ID is that of the gallery, not of the item.
``EVT_RIBBONGALLERY_HOVER_CHANGED`` Triggered when the item being hovered over by the user changes. The item in the event will be the new item being hovered, or ``None`` if there is no longer an item being hovered. Note that the ID is that of the gallery, not of the item.
``EVT_BUTTON``                      Triggered when the "extension" button of the gallery is pressed.
=================================== ===================================

"""

import wx

from control import RibbonControl
from art import *

wxEVT_COMMAND_RIBBONGALLERY_HOVER_CHANGED = wx.NewEventType()
wxEVT_COMMAND_RIBBONGALLERY_SELECTED = wx.NewEventType()

EVT_RIBBONGALLERY_HOVER_CHANGED = wx.PyEventBinder(wxEVT_COMMAND_RIBBONGALLERY_HOVER_CHANGED, 1)
EVT_RIBBONGALLERY_SELECTED = wx.PyEventBinder(wxEVT_COMMAND_RIBBONGALLERY_SELECTED, 1)


class RibbonGalleryEvent(wx.PyCommandEvent):

    def __init__(self, command_type=None, win_id=0, gallery=None, item=None):
        
        wx.PyCommandEvent.__init__(self, command_type, win_id)
        self._gallery = gallery
        self._item = item


    def GetGallery(self):

        return self._gallery

    
    def GetGalleryItem(self):

        return self._item

    
    def SetGallery(self, gallery):

        self._gallery = gallery

        
    def SetGalleryItem(self, item):

        self._item = item


class RibbonGalleryItem(object):

    def __init__(self):
        
        self._id = 0
        self._is_visible = False
        self._client_data = None
        self._position = wx.Rect()
    

    def SetId(self, id):

        self._id = id

        
    def SetBitmap(self, bitmap):

        self._bitmap = bitmap

        
    def GetBitmap(self):

        return self._bitmap

    
    def SetIsVisible(self, visible):

        self._is_visible = visible

        
    def SetPosition(self, x, y, size):
    
        self._position = wx.RectPS(wx.Point(x, y), size)
    

    def IsVisible(self):

        return self._is_visible

    
    def GetPosition(self):

        return self._position

    
    def SetClientData(self, data):

        self._client_data = data

        
    def GetClientData(self):

        return self._client_data


class RibbonGallery(RibbonControl):

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize, agwStyle=0,
                 name="RibbonGallery"):

        """
        Default class constructor.

        :param `parent`: Pointer to a parent window;
        :param `id`: Window identifier. If ``wx.ID_ANY``, will automatically create an
         identifier;
        :param `pos`: Window position. ``wx.DefaultPosition`` indicates that wxPython
         should generate a default position for the window;
        :param `size`: Window size. ``wx.DefaultSize`` indicates that wxPython should
         generate a default size for the window. If no suitable size can be found, the
         window will be sized to 20x20 pixels so that the window is visible but obviously
         not correctly sized;
        :param `agwStyle`: the AGW-specific window style;
        :param `name`: the window name.

        """

        RibbonControl.__init__(self, parent, id, pos, size, style=wx.BORDER_NONE, name=name)

        self.CommonInit(agwStyle)
    
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnMouseEnter)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeave)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnMouseUp)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)


    def CommonInit(self, agwStyle):

        self._selected_item = None
        self._hovered_item = None
        self._active_item = None
        self._scroll_up_button_rect = wx.Rect(0, 0, 0, 0)
        self._scroll_down_button_rect = wx.Rect(0, 0, 0, 0)
        self._extension_button_rect = wx.Rect(0, 0, 0, 0)
        self._mouse_active_rect = None
        self._bitmap_size = wx.Size(64, 32)
        self._bitmap_padded_size = self._bitmap_size
        self._item_separation_x = 0
        self._item_separation_y = 0
        self._scroll_amount = 0
        self._scroll_limit = 0
        self._up_button_state = RIBBON_GALLERY_BUTTON_DISABLED
        self._down_button_state = RIBBON_GALLERY_BUTTON_NORMAL
        self._extension_button_state = RIBBON_GALLERY_BUTTON_NORMAL
        self._hovered = False
        self._items = []

        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)


    def OnMouseEnter(self, event):

        self._hovered = True
        
        if self._mouse_active_rect is not None and not event.LeftIsDown():        
            self._mouse_active_rect = None
            self._active_item = None
        
        self.Refresh(False)


    def OnMouseMove(self, event):

        refresh = False
        pos = event.GetPosition()

        result1, self._up_button_state = self.TestButtonHover(self._scroll_up_button_rect, pos, self._up_button_state)
        result2, self._down_button_state = self.TestButtonHover(self._scroll_down_button_rect, pos, self._down_button_state)
        result3, self._extension_button_state = self.TestButtonHover(self._extension_button_rect, pos, self._extension_button_state)
        
        if result1 or result2 or result3:
            refresh = True

        hovered_item = active_item = None

        if self._client_rect.Contains(pos):
        
            if self._art and self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL:
                pos.x += self._scroll_amount
            else:
                pos.y += self._scroll_amount

            item_count = len(self._items)

            for item in self._items:
                if not item.IsVisible():
                    continue

                if item.GetPosition().Contains(pos):
                    if self._mouse_active_rect == item.GetPosition():
                        active_item = item
                    hovered_item = item
                    break
                
        if active_item != self._active_item:        
            self._active_item = active_item
            refresh = True
        
        if hovered_item != self._hovered_item:
            self._hovered_item = hovered_item
            notification = RibbonGalleryEvent(wxEVT_COMMAND_RIBBONGALLERY_HOVER_CHANGED, self.GetId())
            notification.SetEventObject(self)
            notification.SetGallery(self)
            notification.SetGalleryItem(hovered_item)
            self.GetEventHandler().ProcessEvent(notification)
            refresh = True
        
        if refresh:
            self.Refresh(False)


    def TestButtonHover(self, rect, pos, state):

        if state == RIBBON_GALLERY_BUTTON_DISABLED:
            return False, state

        if rect.Contains(pos):        
            if self._mouse_active_rect == rect:
                new_state = RIBBON_GALLERY_BUTTON_ACTIVE
            else:
                new_state = RIBBON_GALLERY_BUTTON_HOVERED
        else:
            new_state = RIBBON_GALLERY_BUTTON_NORMAL

        if new_state != state:
            return True, new_state
        else:        
            return False, state
    

    def OnMouseLeave(self, event):

        self._hovered = False
        self._active_item = None

        if self._up_button_state != RIBBON_GALLERY_BUTTON_DISABLED:
            self._up_button_state = RIBBON_GALLERY_BUTTON_NORMAL
        if self._down_button_state != RIBBON_GALLERY_BUTTON_DISABLED:
            self._down_button_state = RIBBON_GALLERY_BUTTON_NORMAL
        if self._extension_button_state != RIBBON_GALLERY_BUTTON_DISABLED:
            self._extension_button_state = RIBBON_GALLERY_BUTTON_NORMAL
        if self._hovered_item != None:
            self._hovered_item = None
            notification = RibbonGalleryEvent(wxEVT_COMMAND_RIBBONGALLERY_HOVER_CHANGED, self.GetId())
            notification.SetEventObject(self)
            notification.SetGallery(self)
            self.GetEventHandler().ProcessEvent(notification)
        
        self.Refresh(False)


    def OnMouseDown(self, event):

        pos = event.GetPosition()
        self._mouse_active_rect = None
        
        if self._client_rect.Contains(pos):        
            if self._art and self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL:
                pos.x += self._scroll_amount
            else:
                pos.y += self._scroll_amount
                
            for item in self._items:
                if not item.IsVisible():
                    continue

                rect = item.GetPosition()
                if rect.Contains(pos):                
                    self._active_item = item
                    self._mouse_active_rect = rect
                    break
        
        elif self._scroll_up_button_rect.Contains(pos):        
            if self._up_button_state != RIBBON_GALLERY_BUTTON_DISABLED:            
                self._mouse_active_rect = wx.Rect(*self._scroll_up_button_rect)
                self._up_button_state = RIBBON_GALLERY_BUTTON_ACTIVE
                    
        elif self._scroll_down_button_rect.Contains(pos):        
            if self._down_button_state != RIBBON_GALLERY_BUTTON_DISABLED:            
                self._mouse_active_rect = wx.Rect(*self._scroll_down_button_rect)
                self._down_button_state = RIBBON_GALLERY_BUTTON_ACTIVE
        
        elif self._extension_button_rect.Contains(pos):        
            if self._extension_button_state != RIBBON_GALLERY_BUTTON_DISABLED:            
                self._mouse_active_rect = wx.Rect(*self._extension_button_rect)
                self._extension_button_state = RIBBON_GALLERY_BUTTON_ACTIVE
            
        if self._mouse_active_rect != None:
            self.Refresh(False)


    def OnMouseUp(self, event):

        if self._mouse_active_rect != None:
            pos = event.GetPosition()
            
            if self._active_item:            
                if self._art and self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL:
                    pos.x += self._scroll_amount
                else:
                    pos.y += self._scroll_amount
            
            if self._mouse_active_rect.Contains(pos):
                if self._mouse_active_rect == self._scroll_up_button_rect:
                    self._up_button_state = RIBBON_GALLERY_BUTTON_HOVERED
                    self.ScrollLines(-1)
                
                elif self._mouse_active_rect == self._scroll_down_button_rect:                
                    self._down_button_state = RIBBON_GALLERY_BUTTON_HOVERED
                    self.ScrollLines(1)
                
                elif self._mouse_active_rect == self._extension_button_rect:
                    self._extension_button_state = RIBBON_GALLERY_BUTTON_HOVERED
                    notification = wx.CommandEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, self.GetId())
                    notification.SetEventObject(self)
                    self.GetEventHandler().ProcessEvent(notification)
                
                elif self._active_item != None:                
                    if self._selected_item != self._active_item:                    
                        self._selected_item = self._active_item
                        notification = RibbonGalleryEvent(wxEVT_COMMAND_RIBBONGALLERY_SELECTED, self.GetId())
                        notification.SetEventObject(self)
                        notification.SetGallery(self)
                        notification.SetGalleryItem(self._selected_item)
                        self.GetEventHandler().ProcessEvent(notification)
                    
            self._mouse_active_rect = None
            self._active_item = None
            self.Refresh(False)


    def SetItemClientData(self, item, data):
        """
        Set the client data associated with a gallery item.
        
        :param `item`: MISSING DESCRIPTION;
        :param `data`: MISSING DESCRIPTION.

        """

        item.SetClientData(data)


    def GetItemClientData(self, item):
        """
        Get the client data associated with a gallery item.

        :param `item`: MISSING DESCRIPTION.

        """

        return item.GetClientData()


    def ScrollLines(self, lines):
        """
        Scroll the gallery contents by some amount.

        Reimplemented from `wx.Window`.

        :param `lines`: Positive values scroll toward the end of the gallery, while
         negative values scroll toward the start.

        :returns: ``True`` if the gallery scrolled at least one pixel in the given
         direction, ``False`` if it did not scroll.
        """

        if self._scroll_limit == 0 or self._art == None:
            return False

        line_size = self._bitmap_padded_size.GetHeight()
        if self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL:
            line_size = self._bitmap_padded_size.GetWidth()
            
        if lines < 0:        
            if self._scroll_amount > 0:            
                self._scroll_amount += lines*line_size
                
                if self._scroll_amount <= 0:                
                    self._scroll_amount = 0
                    self._up_button_state = RIBBON_GALLERY_BUTTON_DISABLED
                
                elif self._up_button_state == RIBBON_GALLERY_BUTTON_DISABLED:
                    self._up_button_state = RIBBON_GALLERY_BUTTON_NORMAL
                    
                if self._down_button_state == RIBBON_GALLERY_BUTTON_DISABLED:
                    self._down_button_state = RIBBON_GALLERY_BUTTON_NORMAL
                    
                return True
            
        
        elif lines > 0:
            if self._scroll_amount < self._scroll_limit:            
                self._scroll_amount += lines * line_size
                
                if self._scroll_amount >= self._scroll_limit:                
                    self._scroll_amount = self._scroll_limit
                    self._down_button_state = RIBBON_GALLERY_BUTTON_DISABLED
                
                elif self._down_button_state == RIBBON_GALLERY_BUTTON_DISABLED:
                    self._down_button_state = RIBBON_GALLERY_BUTTON_NORMAL
                    
                if self._up_button_state == RIBBON_GALLERY_BUTTON_DISABLED:
                    self._up_button_state = RIBBON_GALLERY_BUTTON_NORMAL
                    
                return True
            
        return False


    def EnsureVisible(self, item):
        """
        Scroll the gallery to ensure that the given item is visible.
        
        :param `item`: MISSING DESCRIPTION.

        """

        if item is None or not item.IsVisible() or self.IsEmpty():
            return

        y = item.GetPosition().GetTop()
        base_y = self._items[0].GetPosition().GetTop()
        delta = y - base_y - self._scroll_amount
        self.ScrollLines(delta/self._bitmap_padded_size.GetHeight())


    def IsHovered(self):
        """
        Query is the mouse is currently hovered over the gallery.

        :returns: ``True`` if the cursor is within the bounds of the gallery (not
         just hovering over an item), ``False`` otherwise.
        """

        return self._hovered


    def OnEraseBackground(self, event):

        # All painting done in main paint handler to minimise flicker
        pass


    def OnPaint(self, event):

        dc = wx.AutoBufferedPaintDC(self)

        if self._art == None:
            return

        cur_size = self.GetSize()
        min_size = self.GetMinSize()
        
        self._art.DrawGalleryBackground(dc, self, wx.Rect(0, 0, *cur_size))

        padding_top = self._art.GetMetric(RIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE)
        padding_left = self._art.GetMetric(RIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE)

        dc.SetClippingRect(self._client_rect)

        offset_vertical = True
        
        if self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL:
            offset_vertical = False

        for item in self._items:
            if not item.IsVisible():
                continue

            pos = item.GetPosition()
            offset_pos = wx.Rect(*pos)
            
            if offset_vertical:
                offset_pos.SetTop(offset_pos.GetTop() - self._scroll_amount)
            else:
                offset_pos.SetLeft(offset_pos.GetLeft() - self._scroll_amount)
                
            self._art.DrawGalleryItemBackground(dc, self, offset_pos, item)
            dc.DrawBitmap(item.GetBitmap(), offset_pos.GetLeft() + padding_left, offset_pos.GetTop() + padding_top)
        

    def OnSize(self, event):

        self.Layout()


    def Append(self, bitmap, id, clientData=None):
        """
        Add an item to the gallery (with complex client data).

        :param `bitmap`: The bitmap to display for the item. Note that all items must
         have equally sized bitmaps;
        :param `id`: ID number to associate with the item. Not currently used for
         anything important;
        :param `clientData`: An object which contains data to associate with the item.
         The item takes ownership of this pointer, and will delete it when the item
         client data is changed to something else, or when the item is destroyed.

        """

        if not bitmap.IsOk():
            raise Exception("exception")

        if not self._items:
            self._bitmap_size = bitmap.GetSize()
            self.CalculateMinSize()
        else:
            if bitmap.GetSize() != self._bitmap_size:
                raise Exception("exception")
                
        item = RibbonGalleryItem()
        item.SetId(id)
        item.SetBitmap(bitmap)
        self._items.append(item)

        item.SetClientData(clientData)
        
        return item


    def Clear(self):
        """
        Remove all items from the gallery.
        """
        
        self._items = []


    def IsSizingContinuous(self):

        return False


    def CalculateMinSize(self):

        if self._art == None or not self._bitmap_size.IsFullySpecified():        
            self.SetMinSize(wx.Size(20, 20))
        else:        
            self._bitmap_padded_size = wx.Size(*self._bitmap_size)
            self._bitmap_padded_size.IncBy(self._art.GetMetric(RIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE) +
                                           self._art.GetMetric(RIBBON_ART_GALLERY_BITMAP_PADDING_RIGHT_SIZE),
                                           self._art.GetMetric(RIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE) +
                                           self._art.GetMetric(RIBBON_ART_GALLERY_BITMAP_PADDING_BOTTOM_SIZE))

            dc = wx.MemoryDC()
            self.SetMinSize(self._art.GetGallerySize(dc, self, wx.Size(*self._bitmap_padded_size)))

            # The best size is displaying several items
            self._best_size = wx.Size(*self._bitmap_padded_size)
            self._best_size.x *= 3
            self._best_size = self._art.GetGallerySize(dc, self, wx.Size(*self._best_size))
        

    def Realize(self):

        self.CalculateMinSize()
        return self.Layout()


    def Layout(self):

        if self._art == None:
            return False

        dc = wx.MemoryDC()
        origin = wx.Point()

        client_size, origin, self._scroll_up_button_rect, self._scroll_down_button_rect, self._extension_button_rect = \
                     self._art.GetGalleryClientSize(dc, self, wx.Size(*self.GetSize()))
        
        self._client_rect = wx.RectPS(origin, client_size)

        x_cursor = y_cursor = 0
        art_flags = self._art.GetFlags()

        for indx, item in enumerate(self._items):
            
            item.SetIsVisible(True)
            
            if art_flags & RIBBON_BAR_FLOW_VERTICAL:            
                if y_cursor + self._bitmap_padded_size.y > client_size.GetHeight():
                    if y_cursor == 0:
                        break
                    
                    y_cursor = 0
                    x_cursor += self._bitmap_padded_size.x
                
                item.SetPosition(origin.x + x_cursor, origin.y + y_cursor, self._bitmap_padded_size)
                y_cursor += self._bitmap_padded_size.y
            
            else:
                if x_cursor + self._bitmap_padded_size.x > client_size.GetWidth():                
                    if x_cursor == 0:
                        break
                    
                    x_cursor = 0
                    y_cursor += self._bitmap_padded_size.y
                
                item.SetPosition(origin.x + x_cursor, origin.y + y_cursor, self._bitmap_padded_size)
                x_cursor += self._bitmap_padded_size.x

        for item in self._items[indx:]:
            item.SetIsVisible(False)
        
        if art_flags & RIBBON_BAR_FLOW_VERTICAL:
            self._scroll_limit = x_cursor
        else:
            self._scroll_limit = y_cursor
            
        if self._scroll_amount >= self._scroll_limit:
            self._scroll_amount = self._scroll_limit
            self._down_button_state = RIBBON_GALLERY_BUTTON_DISABLED
        
        elif self._down_button_state == RIBBON_GALLERY_BUTTON_DISABLED:
            self._down_button_state = RIBBON_GALLERY_BUTTON_NORMAL

        if self._scroll_amount <= 0:        
            self._scroll_amount = 0
            self._up_button_state = RIBBON_GALLERY_BUTTON_DISABLED
        
        elif self._up_button_state == RIBBON_GALLERY_BUTTON_DISABLED:
            self._up_button_state = RIBBON_GALLERY_BUTTON_NORMAL

        return True


    def DoGetBestSize(self):

        return self._best_size


    def DoGetNextSmallerSize(self, direction, relative_to):

        if self._art == None:
            return relative_to

        dc = wx.MemoryDC()
        client, d1, d2, d3, d4 = self._art.GetGalleryClientSize(dc, self, wx.Size(*relative_to))

        if direction == wx.HORIZONTAL:
            client.DecBy(1, 0)
        elif direction == wx.VERTICAL:
            client.DecBy(0, 1)
        elif direction == wx.BOTH:
            client.DecBy(1, 1)
        
        if client.GetWidth() < 0 or client.GetHeight() < 0:
            return relative_to

        client.x = (client.x / self._bitmap_padded_size.x) * self._bitmap_padded_size.x
        client.y = (client.y / self._bitmap_padded_size.y) * self._bitmap_padded_size.y

        size = self._art.GetGallerySize(dc, self, wx.Size(*client))
        minimum = self.GetMinSize()

        if size.GetWidth() < minimum.GetWidth() or size.GetHeight() < minimum.GetHeight():
            return relative_to

        if direction == wx.HORIZONTAL:
            size.SetHeight(relative_to.GetHeight())
        elif direction == wx.VERTICAL:
            size.SetWidth(relative_to.GetWidth())        

        return size


    def DoGetNextLargerSize(self, direction, relative_to):

        if self._art == None:
            return relative_to

        dc = wx.MemoryDC()
        client, d1, d2, d3, d4 = self._art.GetGalleryClientSize(dc, self, wx.Size(*relative_to))

        # No need to grow if the given size can already display every item
        nitems = (client.GetWidth()/self._bitmap_padded_size.x)*(client.GetHeight()/self._bitmap_padded_size.y)
        
        if nitems >= len(self._items):
            return relative_to

        if direction == wx.HORIZONTAL:
            client.IncBy(self._bitmap_padded_size.x, 0)
        elif direction == wx.VERTICAL:
            client.IncBy(0, self._bitmap_padded_size.y)
        elif direction == wx.BOTH:
            client.IncBy(self._bitmap_padded_size)

        client.x = (client.x / self._bitmap_padded_size.x) * self._bitmap_padded_size.x
        client.y = (client.y / self._bitmap_padded_size.y) * self._bitmap_padded_size.y

        size = self._art.GetGallerySize(dc, self, wx.Size(*client))
        minimum = self.GetMinSize()

        if size.GetWidth() < minimum.GetWidth() or size.GetHeight() < minimum.GetHeight():
            return relative_to
        
        if direction == wx.HORIZONTAL:
            size.SetHeight(relative_to.GetHeight())
        if direction == wx.VERTICAL:
            size.SetWidth(relative_to.GetWidth())

        return size


    def IsEmpty(self):
        """
        Query if the gallery has no items in it.
        """

        return len(self._items) == 0


    def GetCount(self):
        """
        Get the number of items in the gallery.
        """

        return len(self._items)


    def GetItem(self, n):
        """
        Get an item by index.

        :param `n`: MISSING DESCRIPTION.

        """

        if n >= self.GetCount():
            return None
        
        return self._items[n]


    def SetSelection(self, item):
        """
        Set the selection to the given item, or removes the selection if `item` == ``None``.

        Note that this not cause any events to be emitted.

        :param `item`: MISSING DESCRIPTION.

        """

        if item != self._selected_item:        
            self._selected_item = item
            self.Refresh(False)


    def GetSelection(self):
        """
        Get the currently selected item, or ``None`` if there is none.

        The selected item is set by L{SetSelection}, or by the user clicking on an item.

        """

        return self._selected_item


    def GetHoveredItem(self):
        """
        Get the currently hovered item, or ``None`` if there is none.

        The hovered item is the item underneath the mouse cursor.

        """

        return self._hovered_item


    def GetActiveItem(self):
        """
        Get the currently active item, or ``None`` if there is none.

        The active item is the item being pressed by the user, and will thus become the
        selected item if the user releases the mouse button.

        """

        return self._active_item


    def GetUpButtonState(self):
        """
        Get the state of the scroll up button.
        """

        return self._up_button_state


    def GetDownButtonState(self):
        """
        Get the state of the scroll down button.
        """

        return self._down_button_state


    def GetExtensionButtonState(self):
        """
        Get the state of the "extension" button.
        """

        return self._extension_button_state


    def GetDefaultBorder(self):

        return wx.BORDER_NONE

    
