"""
Description
===========

Container for related ribbon panels, and a tab within a ribbon bar.


See Also
========

L{RibbonBar}, L{RibbonPanel}

"""

import wx

from control import RibbonControl
from panel import RibbonPanel

from art import *

# As scroll buttons need to be rendered on top of a page's child windows, the
# buttons themselves have to be proper child windows (rather than just painted
# onto the page). In order to get proper clipping of a page's children (with
# regard to the scroll button), the scroll buttons are created as children of
# the ribbon bar rather than children of the page. This could not have been
# achieved by creating buttons as children of the page and then doing some Z-order
# manipulation, as self causes problems on win32 due to ribbon panels having the
# transparent flag set.

def GetSizeInOrientation(size, orientation):

    if orientation == wx.HORIZONTAL:
        return size.GetWidth()
    elif orientation == wx.VERTICAL:
        return size.GetHeight()
    elif orientation == wx.BOTH:
        return size.GetWidth() * size.GetHeight()
    
    return 0


class RibbonPageScrollButton(RibbonControl):

    def __init__(self, sibling, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize, style=0):

        RibbonControl.__init__(self, sibling.GetParent(), id, pos, size, style=wx.BORDER_NONE)

        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        self._sibling = sibling
        self._flags = (style & RIBBON_SCROLL_BTN_DIRECTION_MASK) | RIBBON_SCROLL_BTN_FOR_PAGE


        self.Bind(wx.EVT_ENTER_WINDOW, self.OnMouseEnter)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeave)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnMouseUp)
        self.Bind(wx.EVT_PAINT, self.OnPaint)


    def OnEraseBackground(self, event):

        # Do nothing - all painting done in main paint handler
        pass


    def OnPaint(self, event):

        dc = wx.AutoBufferedPaintDC(self)
        
        if self._art:        
            self._art.DrawScrollButton(dc, self, wx.Rect(0, 0, *self.GetSize()), self._flags)


    def OnMouseEnter(self, event):

        self._flags |= RIBBON_SCROLL_BTN_HOVERED
        self.Refresh(False)


    def OnMouseLeave(self, event):

        self._flags &= ~RIBBON_SCROLL_BTN_HOVERED
        self._flags &= ~RIBBON_SCROLL_BTN_ACTIVE
        self.Refresh(False)


    def OnMouseDown(self, event):

        self._flags |= RIBBON_SCROLL_BTN_ACTIVE
        self.Refresh(False)


    def OnMouseUp(self, event):

        if self._flags & RIBBON_SCROLL_BTN_ACTIVE:
            
            self._flags &= ~RIBBON_SCROLL_BTN_ACTIVE
            self.Refresh(False)
            result = self._flags & RIBBON_SCROLL_BTN_DIRECTION_MASK
            
            if result in [RIBBON_SCROLL_BTN_DOWN, RIBBON_SCROLL_BTN_RIGHT]:
                self._sibling.ScrollLines(1)
            elif result in [RIBBON_SCROLL_BTN_UP, RIBBON_SCROLL_BTN_LEFT]:
                self._sibling.ScrollLines(-1)


class RibbonPage(RibbonControl):

    def __init__(self, parent, id=wx.ID_ANY, label="", icon=wx.NullBitmap, style=0):
        """
        Default class constructor.

        :param `parent`: Pointer to a parent window;
        :param `id`: Window identifier. If ``wx.ID_ANY``, will automatically create an identifier;
        :param `label`: Label of the new button;
        :param `icon`: the icon used for the page in the ribbon bar tab area;
        :param `style`: Window style.

        """

        RibbonControl.__init__(self, parent, id, wx.DefaultPosition, wx.DefaultSize, wx.BORDER_NONE)

        self.CommonInit(label, icon)
        
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        

    def CommonInit(self, label, icon):

        self.SetName(label)
        self.SetLabel(label)
        
        self._old_size = wx.Size(0, 0)
        self._icon = icon
        self._scroll_left_btn = None
        self._scroll_right_btn = None
        self._scroll_amount = 0
        self._scroll_buttons_visible = False
        self._collapse_stack = []

        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)

        self.GetParent().AddPage(self)


    def SetArtProvider(self, art):
        """
        Set the art provider to be used.

        Normally called automatically by L{RibbonBar} when the page is created, or the
        art provider changed on the bar. The new art provider will be propagated to the
        children of the page.

        Reimplemented from L{RibbonControl}.

        :param `art`: MISSING DESCRIPTION.

        """

        self._art = art
        for child in self.GetChildren():
            if isinstance(child, RibbonControl):
                child.SetArtProvider(art)
            

    def AdjustRectToIncludeScrollButtons(self, rect):
        """
        Expand a rectangle of the page to include external scroll buttons (if any).

        When no scroll buttons are shown, has no effect.

        :param `rect`: The rectangle to adjust. The width and height will not be
         reduced, and the x and y will not be increased.

        """

        if self._scroll_buttons_visible:        
            if self.GetMajorAxis() == wx.VERTICAL:
                if self._scroll_left_btn:
                    rect.SetY(rect.GetY() - self._scroll_left_btn.GetSize().GetHeight())
                    rect.SetHeight(rect.GetHeight() + self._scroll_left_btn.GetSize().GetHeight())
                
                if self._scroll_right_btn:                
                    rect.SetHeight(rect.GetHeight() + self._scroll_right_btn.GetSize().GetHeight())
                
            else:            
                if self._scroll_left_btn:                
                    rect.SetX(rect.GetX() - self._scroll_left_btn.GetSize().GetWidth())
                    rect.SetWidth(rect.GetWidth() + self._scroll_left_btn.GetSize().GetWidth())
                
                if self._scroll_right_btn:                
                    rect.SetWidth(rect.GetWidth() + self._scroll_right_btn.GetSize().GetWidth())

        return rect
    

    def OnEraseBackground(self, event):

        # All painting done in main paint handler to minimise flicker
        pass


    def OnPaint(self, event):

        # No foreground painting done by the page itself, but a paint DC
        # must be created anyway.
        dc = wx.AutoBufferedPaintDC(self)
        rect = wx.Rect(0, 0, *self.GetSize())
        rect = self.AdjustRectToIncludeScrollButtons(rect)
        self._art.DrawPageBackground(dc, self, rect)


    def GetMajorAxis(self):
        """
        Get the direction in which ribbon panels are stacked within the page.

        This is controlled by the style of the containing L{RibbonBar}, meaning that all
        pages within a bar will have the same major axis. As well as being the direction
        in which panels are stacked, it is also the axis in which scrolling will occur
        (when required).

        :returns: ``wx.HORIZONTAL`` or ``wx.VERTICAL`` (never ``wx.BOTH``).
        """

        if self._art and (self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL):
            return wx.VERTICAL
        else:        
            return wx.HORIZONTAL
    

    def ScrollLines(self, lines):
        """
        Scroll the page by some amount up / down / left / right.

        When the page's children are too big to fit in the onscreen area given to the
        page, scroll buttons will appear, and the page can be programatically scrolled.
        Positive values of will scroll right or down, while negative values will scroll
        up or left (depending on the direction in which panels are stacked). A line is
        equivalent to a constant number of pixels.

        Reimplemented from `wx.Window`.

        :param `lines`: MISSING DESCRIPTION.

        :returns: ``True`` if the page scrolled at least one pixel in the given direction,
         ``False`` if it did not scroll.
         
        :see: L{GetMajorAxis}, L{ScrollPixels}
        """

        return self.ScrollPixels(lines * 8)


    def ScrollPixels(self, pixels):
        """
        Scroll the page by a set number of pixels up / down / left / right.

        When the page's children are too big to fit in the onscreen area given to the
        page, scroll buttons will appear, and the page can be programatically scrolled.
        Positive values of will scroll right or down, while negative values will scroll
        up or left (depending on the direction in which panels are stacked).

        :param `pixels`: MISSING DESCRIPTION.

        :returns: ``True`` if the page scrolled at least one pixel in the given direction,
         ``False`` if it did not scroll.
         
        :see: L{GetMajorAxis}, L{ScrollLines}
        """

        if pixels < 0:        
            if self._scroll_amount == 0:
                return False
            
            if self._scroll_amount < -pixels:
                pixels = -self._scroll_amount
        
        elif pixels > 0:        
            if self._scroll_amount == self._scroll_amount_limit:
                return False
            
            if self._scroll_amount + pixels > self._scroll_amount_limit:
                pixels = self._scroll_amount_limit - self._scroll_amount
        
        else:
            return False

        self._scroll_amount += pixels

        for child in self.GetChildren():
            x, y = child.GetPosition()
            if self.GetMajorAxis() == wx.HORIZONTAL:
                x -= pixels
            else:
                y -= pixels
                
            child.SetPosition(wx.Point(x, y))
        
        self.ShowScrollButtons()
        self.Refresh()
        return True


    def SetSizeWithScrollButtonAdjustment(self, x, y, width, height):
        """
        Set the size of the page and the external scroll buttons (if any).

        When a page is too small to display all of its children, scroll buttons will
        appear (and if the page is sized up enough, they will disappear again). Slightly
        counter-intuively, these buttons are created as siblings of the page rather than
        children of the page (to achieve correct cropping and paint ordering of the
        children and the buttons).

        When there are no scroll buttons, this function behaves the same as `SetSize`,
        however when there are scroll buttons, it positions them at the edges of the
        given area, and then calls `SetSize` with the remaining area. This is provided
        as a separate function to `SetSize` rather than within the implementation
        of `SetSize`, as iteracting algorithms may not expect `SetSize` to also
        set the size of siblings.

        :param `x`: MISSING DESCRIPTION;
        :param `y`: MISSING DESCRIPTION;
        :param `width`: MISSING DESCRIPTION;
        :param `height`: MISSING DESCRIPTION.

        """

        if self._scroll_buttons_visible:        
            if self.GetMajorAxis() == wx.HORIZONTAL:
                if self._scroll_left_btn:
                    w = self._scroll_left_btn.GetSize().GetWidth()
                    self._scroll_left_btn.SetPosition(wx.Point(x, y))
                    x += w
                    width -= w
                
                if self._scroll_right_btn:                
                    w = self._scroll_right_btn.GetSize().GetWidth()
                    width -= w
                    self._scroll_right_btn.SetPosition(wx.Point(x + width, y))
                
            else:            
                if self._scroll_left_btn:                
                    h = self._scroll_left_btn.GetSize().GetHeight()
                    self._scroll_left_btn.SetPosition(wx.Point(x, y))
                    y += h
                    height -= h
                
                if self._scroll_right_btn:                
                    h = self._scroll_right_btn.GetSize().GetHeight()
                    height -= h
                    self._scroll_right_btn.SetPosition(wx.Point(x, y + height))

        self.SetDimensions(x, y, width, height)


    def DoSetSize(self, x, y, width, height, sizeFlags=wx.SIZE_AUTO):

        # When a resize triggers the scroll buttons to become visible, the page is resized.
        # This resize from within a resize event can cause (MSW) wxWidgets some confusion,
        # and report the 1st size to the 2nd size event. Hence the most recent size is
        # remembered internally and used in Layout() where appropiate.

        if self.GetMajorAxis() == wx.HORIZONTAL:
            self._size_in_major_axis_for_children = width
        else:
            self._size_in_major_axis_for_children = height

        RibbonControl.DoSetSize(self, x, y, width, height, sizeFlags)


    def OnSize(self, event):

        new_size = event.GetSize()

        temp_dc = wx.MemoryDC()
        invalid_rect = self._art.GetPageBackgroundRedrawArea(temp_dc, self, self._old_size, new_size)
        self.Refresh(True, invalid_rect)

        self._old_size = wx.Size(*new_size)
        x, y = new_size
        
        if x > 0 and y > 0:
            self.Layout()
        else:        
            # Simplify other calculations by pretending new size is zero in both
            # X and Y
            new_size = wx.Size(0, 0)
            # When size == 0, no point in doing any layout
        
        event.Skip()


    def RemoveChild(self, child):

        # Remove all references to the child from the collapse stack

        try:
            self._collapse_stack.remove(child)
        except ValueError:
            pass

        # ... and then proceed as normal
        RibbonControl.RemoveChild(self, child)
        

    def Realize(self):
        """
        Perform a full re-layout of all panels on the page.

        Should be called after panels are added to the page, or the sizing behaviour of
        a panel on the page changes (i.e. due to children being added to it). Usually
        called automatically when L{RibbonBar.Realize} is called. Will invoke
        L{RibbonPanel.Realize} for all child panels.

        Reimplemented from L{RibbonControl}.

        """

        status = True
        self._collapse_stack = []
        
        for child in self.GetChildren():
            if not isinstance(child, RibbonControl):            
                continue
            
            if not child.Realize():            
                status = False
            
            child.SetSize(wx.Size(*child.GetMinSize()))

        x, y = self.GetSize()        
        if x > 0 and y > 0:
            status = self.Layout() and status
        
        return status


    def Layout(self):

        if len(self.GetChildren()) == 0:        
            return True

        origin_ = wx.Point(self._art.GetMetric(RIBBON_ART_PAGE_BORDER_LEFT_SIZE), self._art.GetMetric(RIBBON_ART_PAGE_BORDER_TOP_SIZE))
        major_axis = self.GetMajorAxis()
        
        if self._scroll_buttons_visible:        
            if major_axis == wx.HORIZONTAL:            
                origin_.x -= self._scroll_amount
                if self._scroll_left_btn:
                    origin_.x -= self._scroll_left_btn.GetSize().GetWidth()
            else:            
                origin_.y -= self._scroll_amount
                if self._scroll_left_btn:
                    origin_.y -= self._scroll_left_btn.GetSize().GetHeight()
            
        origin = wx.Point(*origin_)
        
        if major_axis == wx.HORIZONTAL:        
            gap = self._art.GetMetric(RIBBON_ART_PANEL_X_SEPARATION_SIZE)
            minor_axis_size = self.GetSize().GetHeight() - origin.y - self._art.GetMetric(RIBBON_ART_PAGE_BORDER_BOTTOM_SIZE)
        else:        
            gap = self._art.GetMetric(RIBBON_ART_PANEL_Y_SEPARATION_SIZE)
            minor_axis_size = self.GetSize().GetWidth() - origin.x - self._art.GetMetric(RIBBON_ART_PAGE_BORDER_RIGHT_SIZE)

        for iteration in xrange(1, 3):
        
            for child in self.GetChildren():
                w, h = child.GetSize()
                if major_axis == wx.HORIZONTAL:
                    child.SetDimensions(origin.x, origin.y, w, minor_axis_size)
                    origin.x += w + gap
                else:                
                    child.SetDimensions(origin.x, origin.y, minor_axis_size, h)
                    origin.y += h + gap

            if iteration == 1:
                if major_axis == wx.HORIZONTAL:
                    available_space = self._size_in_major_axis_for_children - self._art.GetMetric(RIBBON_ART_PAGE_BORDER_RIGHT_SIZE) - origin.x + gap
                else:
                    available_space = self._size_in_major_axis_for_children - self._art.GetMetric(RIBBON_ART_PAGE_BORDER_BOTTOM_SIZE) - origin.y + gap

                if self._scroll_buttons_visible:                
                    available_space -= self._scroll_amount
                    if self._scroll_right_btn != None:
                        available_space += GetSizeInOrientation(self._scroll_right_btn.GetSize(), major_axis)

                if available_space > 0:                
                    if self._scroll_buttons_visible:
                        self.HideScrollButtons()
                        break

                    result = self.ExpandPanels(major_axis, available_space)
                    if not result:
                        break
                
                elif available_space < 0:                
                    if self._scroll_buttons_visible:                    
                        # Scroll buttons already visible - not going to be able to downsize any more
                        self._scroll_amount_limit = -available_space
                        if self._scroll_amount > self._scroll_amount_limit:
                            self.ScrollPixels(self._scroll_amount_limit - self._scroll_amount)                    
                    else:
                        result = self.CollapsePanels(major_axis, -available_space)
                        if not result:
                            self._scroll_amount = 0
                            self._scroll_amount_limit = -available_space
                            self.ShowScrollButtons()
                            break
                        
                else:                
                    break
                
            origin = wx.Point(*origin_) # Reset the origin

        return True


    def Show(self, show=True):

        if self._scroll_left_btn:
            self._scroll_left_btn.Show(show)
        if self._scroll_right_btn:
            self._scroll_right_btn.Show(show)
            
        return RibbonControl.Show(self, show)


    def GetIcon(self):
        """
        Get the icon used for the page in the ribbon bar tab area (only displayed if the
        ribbon bar is actually showing icons).
        """

        return self._icon

    
    def HideScrollButtons(self):

        self._scroll_amount = 0
        self._scroll_amount_limit = 0
        self.ShowScrollButtons()


    def ShowScrollButtons(self):

        show_left = True
        show_right = True
        reposition = False
        
        if self._scroll_amount == 0:        
            show_left = False
        
        if self._scroll_amount >= self._scroll_amount_limit:        
            show_right = False
            self._scroll_amount = self._scroll_amount_limit
        
        self._scroll_buttons_visible = show_left or show_right

        if show_left:
            if self._scroll_left_btn == None:
            
                temp_dc = wx.MemoryDC()
                
                if self.GetMajorAxis() == wx.HORIZONTAL:                
                    direction = RIBBON_SCROLL_BTN_LEFT
                    size = self._art.GetScrollButtonMinimumSize(temp_dc, self.GetParent(), direction)
                    size.SetHeight(self.GetSize().GetHeight())                
                else:                
                    direction = RIBBON_SCROLL_BTN_UP
                    size = self._art.GetScrollButtonMinimumSize(temp_dc, self.GetParent(), direction)
                    size.SetWidth(self.GetSize().GetWidth())
                
                self._scroll_left_btn = RibbonPageScrollButton(self, -1, self.GetPosition(), size, direction)
                if not self.IsShown():                
                    self._scroll_left_btn.Hide()
                
                reposition = True
            
        else:        
            if self._scroll_left_btn != None:            
                self._scroll_left_btn.Destroy()
                self._scroll_left_btn = None
                reposition = True
            
        if show_right:        
            if self._scroll_right_btn == None:            

                temp_dc = wx.MemoryDC()

                if self.GetMajorAxis() == wx.HORIZONTAL:                
                    direction = RIBBON_SCROLL_BTN_RIGHT
                    size = self._art.GetScrollButtonMinimumSize(temp_dc, self.GetParent(), direction)
                    size.SetHeight(self.GetSize().GetHeight())                
                else:                
                    direction = RIBBON_SCROLL_BTN_DOWN
                    size = self._art.GetScrollButtonMinimumSize(temp_dc, self.GetParent(), direction)
                    size.SetWidth(self.GetSize().GetWidth())
                
                initial_pos = self.GetPosition() + wx.Point(*self.GetSize()) - wx.Point(*size)
                self._scroll_right_btn = RibbonPageScrollButton(self, -1, initial_pos, size, direction)
                if not self.IsShown():                
                    self._scroll_right_btn.Hide()
                
                reposition = True
            
        else:        
            if self._scroll_right_btn != None:            
                self._scroll_right_btn.Destroy()
                self._scroll_right_btn = None
                reposition = True
                
        if reposition:        
            self.GetParent().RepositionPage(self)
        

    def ExpandPanels(self, direction, maximum_amount):

        expanded_something = False

        while maximum_amount > 0:
            smallest_size = 10000
            smallest_panel = None
            
            for panel in self.GetChildren():
                
                if not isinstance(panel, RibbonPanel):
                    continue
                
                if panel.IsSizingContinuous():
                    size = GetSizeInOrientation(panel.GetSize(), direction)
                    if size < smallest_size:                    
                        smallest_size = size
                        smallest_panel = panel                    
                else:                
                    current = panel.GetSize()
                    size = GetSizeInOrientation(current, direction)
                    if size < smallest_size:
                        larger = panel.GetNextLargerSize(direction)
                        if larger != current and GetSizeInOrientation(larger, direction) > size:
                            smallest_size = size
                            smallest_panel = panel

            if smallest_panel != None:            
                if smallest_panel.IsSizingContinuous():
                    size = wx.Size(*smallest_panel.GetSize())
                    amount = maximum_amount
                    
                    if amount > 32:                    
                        # For "large" growth, grow self panel a bit, and then re-allocate
                        # the remainder (which may come to self panel again anyway)
                        amount = 32
                    
                    if direction & wx.HORIZONTAL:                    
                        size.x += amount
                    
                    if direction & wx.VERTICAL:                    
                        size.y += amount
                    
                    smallest_panel.SetSize(size)
                    maximum_amount -= amount
                    self._collapse_stack.append(smallest_panel)
                    expanded_something = True
                
                else:                
                    current = smallest_panel.GetSize()
                    larger = smallest_panel.GetNextLargerSize(direction)
                    delta = larger - current
                    if GetSizeInOrientation(delta, direction) <= maximum_amount:
                        smallest_panel.SetSize(wx.Size(*larger))
                        maximum_amount -= GetSizeInOrientation(delta, direction)
                        self._collapse_stack.append(smallest_panel)
                        expanded_something = True
                    else:                    
                        break
                    
            else:            
                break

        if expanded_something:
            self.Refresh()
            return True
        else:        
            return False

    
    def CollapsePanels(self, direction, minimum_amount):

        collapsed_something = False

        while minimum_amount > 0:
            largest_size = 0
            largest_panel = None
            
            if self._collapse_stack:            
                # For a more consistent panel layout, try to collapse panels which
                # were recently expanded.
                largest_panel = self._collapse_stack[-1]
                self._collapse_stack.pop(len(self._collapse_stack)-1)
            else:            
                for panel in self.GetChildren():
                    if not isinstance(panel, RibbonPanel):
                        continue
                    
                    if panel.IsSizingContinuous():
                        size = GetSizeInOrientation(panel.GetSize(), direction)
                        if size > largest_size:                        
                            largest_size = size
                            largest_panel = panel
                    else:                    
                        current = panel.GetSize()
                        size = GetSizeInOrientation(current, direction)
                        if size > largest_size:
                            smaller = panel.GetNextSmallerSize(direction)
                            if smaller != current and GetSizeInOrientation(smaller, direction) < size:
                                largest_size = size
                                largest_panel = panel
            
            if largest_panel != None:            
                if largest_panel.IsSizingContinuous():
                    size = largest_panel.GetSize()
                    amount = minimum_amount
                    
                    if amount > 32:                    
                        # For "large" contraction, reduce self panel a bit, and
                        # then re-allocate the remainder of the quota (which may
                        # come to this panel again anyway)
                        amount = 32
                    
                    if direction & wx.HORIZONTAL:                    
                        size.x -= amount
                    
                    if direction & wx.VERTICAL:                    
                        size.y -= amount
                    
                    largest_panel.SetSize(size)
                    minimum_amount -= amount
                    collapsed_something = True
                
                else:                
                    current = largest_panel.GetSize()
                    smaller = largest_panel.GetNextSmallerSize(direction)
                    delta = current - smaller
                    largest_panel.SetSize(smaller)
                    minimum_amount -= GetSizeInOrientation(delta, direction)
                    collapsed_something = True
                
            else:            
                break

        if collapsed_something:        
            self.Refresh()
            return True       
        else:        
            return False
        

    def DismissExpandedPanel(self):
        """
        Dismiss the current externally expanded panel, if there is one.

        When a ribbon panel automatically minimises, it can be externally expanded into
        a floating window. When the user clicks a button in such a panel, the panel
        should generally re-minimise. Event handlers for buttons on ribbon panels should
        call this method to achieve this behaviour.

        :returns: ``True`` if a panel was minimised, ``False`` otherwise.
        """

        for panel in self.GetChildren():
            if not isinstance(panel, RibbonPanel):
                continue
            
            if panel.GetExpandedPanel() != None:            
                return panel.HideExpanded()
            
        return False


    def GetMinSize(self):

        minSize = wx.Size(-1, -1)

        for child in self.GetChildren():
            child_min = child.GetMinSize()
            minSize.x = max(minSize.x, child_min.x)
            minSize.y = max(minSize.y, child_min.y)
        
        if self.GetMajorAxis() == wx.HORIZONTAL:        
            minSize.x = -1
            if minSize.y != -1:
                minSize.y += self._art.GetMetric(RIBBON_ART_PAGE_BORDER_TOP_SIZE) + self._art.GetMetric(RIBBON_ART_PAGE_BORDER_BOTTOM_SIZE)
            
        else:        
            if minSize.x != -1:            
                minSize.x += self._art.GetMetric(RIBBON_ART_PAGE_BORDER_LEFT_SIZE) + self._art.GetMetric(RIBBON_ART_PAGE_BORDER_RIGHT_SIZE)
            
            minSize.y = -1
        
        return minSize


    def DoGetBestSize(self):

        best = wx.Size(0, 0)
        count = 0

        if self.GetMajorAxis() == wx.HORIZONTAL:
            best.y = -1

            for child in self.GetChildren():
                child_best = child.GetBestSize()
                if child_best.x != -1:                
                    best.IncBy(child_best.x, 0)
                
                best.y = max(best.y, child_best.y)
                count += 1

            if count > 1:            
                best.IncBy((count - 1) * self._art.GetMetric(RIBBON_ART_PANEL_X_SEPARATION_SIZE), 0)
            
        else:        
            best.x = -1

            for child in self.GetChildren():
                child_best = child.GetBestSize()
                best.x = max(best.x, child_best.x)
                
                if child_best.y != -1:                
                    best.IncBy(0, child_best.y)
                
                count += 1
            
            if count > 1:            
                best.IncBy(0, (count - 1) * self._art.GetMetric(RIBBON_ART_PANEL_Y_SEPARATION_SIZE))

        if best.x != -1:        
            best.x += self._art.GetMetric(RIBBON_ART_PAGE_BORDER_LEFT_SIZE) + self._art.GetMetric(RIBBON_ART_PAGE_BORDER_RIGHT_SIZE)
        
        if best.y != -1:        
            best.y += self._art.GetMetric(RIBBON_ART_PAGE_BORDER_TOP_SIZE) + self._art.GetMetric(RIBBON_ART_PAGE_BORDER_BOTTOM_SIZE)

        return best


    def GetDefaultBorder(self):

        return wx.BORDER_NONE


