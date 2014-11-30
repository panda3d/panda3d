"""
A ribbon tool bar is similar to a traditional toolbar which has no labels.


Description
===========

It contains one or more tool groups, each of which contains one or more tools.
Each tool is represented by a (generally small, i.e. 16x15) bitmap.


Events Processing
=================

This class processes the following events:

====================================== ======================================
Event Name                             Description
====================================== ======================================
``EVT_RIBBONTOOLBAR_CLICKED``          Triggered when the normal (non-dropdown) region of a tool on the tool bar is clicked.
``EVT_RIBBONTOOLBAR_DROPDOWN_CLICKED`` Triggered when the dropdown region of a tool on the tool bar is clicked. L{RibbonToolBarEvent.PopupMenu} should be called by the event handler if it wants to display a popup menu (which is what most dropdown tools should be doing).
====================================== ======================================

"""

import wx

from control import RibbonControl
from art import *

wxEVT_COMMAND_RIBBONTOOL_CLICKED = wx.NewEventType()
wxEVT_COMMAND_RIBBONTOOL_DROPDOWN_CLICKED = wx.NewEventType()

EVT_RIBBONTOOLBAR_CLICKED = wx.PyEventBinder(wxEVT_COMMAND_RIBBONTOOL_CLICKED, 1)
EVT_RIBBONTOOLBAR_DROPDOWN_CLICKED = wx.PyEventBinder(wxEVT_COMMAND_RIBBONTOOL_DROPDOWN_CLICKED, 1)


def GetSizeInOrientation(size, orientation):

    if orientation == wx.HORIZONTAL:
        return size.GetWidth()
    
    if orientation == wx.VERTICAL:
        return size.GetHeight()
    
    if orientation == wx.BOTH:
        return size.GetWidth() * size.GetHeight()
    
    return 0


class RibbonToolBarEvent(wx.PyCommandEvent):

    def __init__(self, command_type=None, win_id=0, bar=None):
        
        wx.PyCommandEvent.__init__(self, command_type, win_id)
        self._bar = bar


    def GetBar(self):

        return self._bar

    
    def SetBar(self, bar):

        self._bar = bar
        

    def PopupMenu(self, menu):

        pos = wx.Point()
        
        if self._bar._active_tool:        
            # Find the group which contains the tool
            group_count = len(self._bar._groups)
            tobreak = False

            for g in xrange(group_count):            
                group = self._bar._groups[g]
                tool_count = len(group.tools)
                
                for t in xrange(tool_count):                
                    tool = group.tools[t]
                    if tool == self._bar._active_tool:                    
                        pos = wx.Point(*group.position)
                        pos += tool.position
                        pos.y += tool.size.GetHeight()
                        g = group_count
                        tobreak = True
                        break

                if tobreak:
                    break
                    
        return self._bar.PopupMenu(menu, pos)


class RibbonToolBarToolBase(object):

    def __init__(self):
        
        self.help_string = ""
        self.bitmap = wx.NullBitmap
        self.bitmap_disabled = wx.NullBitmap
        self.dropdown = wx.Rect()
        self.position = wx.Point()
        self.size = wx.Size()
        self.client_data = None
        self.id = -1
        self.kind = RIBBON_BUTTON_NORMAL
        self.state = None


class RibbonToolBarToolGroup(object):

    def __init__(self):

        # To identify the group as a wxRibbonToolBarToolBase*
        self.dummy_tool = None

        self.tools = []
        self.position = wx.Point()
        self.size = wx.Size()


class RibbonToolBar(RibbonControl):

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize, style=0,
                 name="RibbonToolbar"):

        """
        Default class constructor.
        
        :param `parent`: Pointer to a parent window;
        :param `id`: Window identifier. If ``wx.ID_ANY``, will automatically create
         an identifier;
        :param `pos`: Window position. ``wx.DefaultPosition`` indicates that wxPython
         should generate a default position for the window;
        :param `size`: Window size. ``wx.DefaultSize`` indicates that wxPython should
         generate a default size for the window. If no suitable size can be found, the
         window will be sized to 20x20 pixels so that the window is visible but obviously
         not correctly sized;
        :param `style`: Window style;
        :param `name`: the window name.

        """

        RibbonControl.__init__(self, parent, id, pos, size, wx.BORDER_NONE, name=name)

        self.Bind(wx.EVT_ENTER_WINDOW, self.OnMouseEnter)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeave)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnMouseUp)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)

        self.CommonInit(style)


    def CommonInit(self, style):

        self._groups = []
        
        self.AppendGroup()
        self._hover_tool = None
        self._active_tool = None
        self._nrows_min = 1
        self._nrows_max = 1
        self._sizes = [wx.Size(0, 0)]
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)


    def AddSimpleTool(self, tool_id, bitmap, help_string, kind=RIBBON_BUTTON_NORMAL):
        """
        Add a tool to the tool bar (simple version).

        :param `tool_id`: ID of the new tool (used for event callbacks);
        :param `bitmap`: Large bitmap of the new button. Must be the same size as all
         other large bitmaps used on the button bar;
        :param `help_string`: The UI help string to associate with the new button;
        :param `kind`: The kind of button to add.

        :see: L{AddDropdownTool}, L{AddHybridTool}, L{AddTool}

        """

        return self.AddTool(tool_id, bitmap, wx.NullBitmap, help_string, kind, None)


    def AddDropdownTool(self, tool_id, bitmap, help_string=""):
        """
        Add a dropdown tool to the tool bar (simple version).

        :param `tool_id`: ID of the new tool (used for event callbacks);
        :param `bitmap`: Large bitmap of the new button. Must be the same size as all
         other large bitmaps used on the button bar;
        :param `help_string`: The UI help string to associate with the new button.

        :see: L{AddTool}
        """

        return self.AddTool(tool_id, bitmap, wx.NullBitmap, help_string, RIBBON_BUTTON_DROPDOWN, None)


    def AddHybridTool(self,  tool_id, bitmap, help_string=""):
        """
        Add a hybrid tool to the tool bar (simple version).


        :param `tool_id`: ID of the new tool (used for event callbacks);
        :param `bitmap`: Large bitmap of the new button. Must be the same size as all
         other large bitmaps used on the button bar;
        :param `help_string`: The UI help string to associate with the new button.

        :see: L{AddTool}
        """

        return self.AddTool(tool_id, bitmap, wx.NullBitmap, help_string, RIBBON_BUTTON_HYBRID, None)


    def AddTool(self, tool_id, bitmap, bitmap_disabled=wx.NullBitmap, help_string="", kind=RIBBON_BUTTON_NORMAL, client_data=None):
        """
        Add a tool to the tool bar.

        :param `tool_id`: ID of the new tool (used for event callbacks);
        :param `bitmap`: Bitmap to use as the foreground for the new tool. Does not
         have to be the same size as other tool bitmaps, but should be similar as
         otherwise it will look visually odd;
        :param `bitmap_disabled`: Bitmap to use when the tool is disabled. If left
         as `wx.NullBitmap`, then a bitmap will be automatically generated from `bitmap`;
        :param `help_string`: The UI help string to associate with the new tool;
        :param `kind`: The kind of tool to add;
        :param `client_data`: Client data to associate with the new tool.

        :returns: An opaque pointer which can be used only with other tool bar methods.
        
        :see: L{AddDropdownTool}, L{AddHybridTool}, L{AddSeparator}
        """

        if not bitmap.IsOk():
            raise Exception("Exception")

        tool = RibbonToolBarToolBase()
        tool.id = tool_id
        tool.bitmap = bitmap
        
        if bitmap_disabled.IsOk():        
            if bitmap.GetSize() != bitmap_disabled.GetSize():
                raise Exception("Exception")
            
            tool.bitmap_disabled = bitmap_disabled
        else:
            tool.bitmap_disabled = self.MakeDisabledBitmap(bitmap)
            
        tool.help_string = help_string
        tool.kind = kind
        tool.client_data = client_data
        tool.position = wx.Point(0, 0)
        tool.size = wx.Size(0, 0)
        tool.state = 0

        self._groups[-1].tools.append(tool)
        return tool


    def AddSeparator(self):
        """
        Add a separator to the tool bar.

        Separators are used to separate tools into groups. As such, a separator is not
        explicity drawn, but is visually seen as the gap between tool groups.

        """

        if not self._groups[-1].tools:
            return None

        self.AppendGroup()
        return self._groups[-1].dummy_tool


    def MakeDisabledBitmap(self, original):

        img = original.ConvertToImage()
        return wx.BitmapFromImage(img.ConvertToGreyscale())


    def AppendGroup(self):

        group = RibbonToolBarToolGroup()
        group.position = wx.Point(0, 0)
        group.size = wx.Size(0, 0)
        self._groups.append(group)


    def IsSizingContinuous(self):

        return False
    

    def DoGetNextSmallerSize(self, direction, relative_to):

        result = wx.Size(*relative_to)
        area = 0
        tobreak = False

        for nrows in xrange(self._nrows_max, self._nrows_min-1, -1):
        
            size = wx.Size(*self._sizes[nrows - self._nrows_min])
            original = wx.Size(*size)
            
            if direction == wx.HORIZONTAL:
                if size.GetWidth() < relative_to.GetWidth() and size.GetHeight() <= relative_to.GetHeight():                
                    size.SetHeight(relative_to.GetHeight())
                    tobreak = True
            
            elif direction == wx.VERTICAL:
                if size.GetWidth() <= relative_to.GetWidth() and size.GetHeight() < relative_to.GetHeight():                
                    size.SetWidth(relative_to.GetWidth())
                    tobreak = True
                
            elif direction == wx.BOTH:
                if size.GetWidth() < relative_to.GetWidth() and size.GetHeight() < relative_to.GetHeight():
                    pass

            if GetSizeInOrientation(original, direction) > area:
                result = wx.Size(*size)
                area = GetSizeInOrientation(original, direction)
                if tobreak:
                    break
            
        return result


    def DoGetNextLargerSize(self, direction, relative_to):

        # Pick the smallest of our sizes which are larger than the given size
        result = wx.Size(*relative_to)
        area = 10000
        tobreak = False
        
        for nrows in xrange(self._nrows_min, self._nrows_max+1):

            size = wx.Size(*self._sizes[nrows - self._nrows_min])
            original = wx.Size(*size)

            if direction == wx.HORIZONTAL:
                if size.GetWidth() > relative_to.GetWidth() and size.GetHeight() <= relative_to.GetHeight():                
                    size.SetHeight(relative_to.GetHeight())
                    tobreak = True

            elif direction == wx.VERTICAL:
                if size.GetWidth() <= relative_to.GetWidth() and size.GetHeight() > relative_to.GetHeight():                
                    size.SetWidth(relative_to.GetWidth())
                    tobreak = True
            
            elif direction == wx.BOTH:
                if size.GetWidth() > relative_to.GetWidth() and size.GetHeight() > relative_to.GetHeight():
                    tobreak = True

            if GetSizeInOrientation(original, direction) < area:
                result = wx.Size(*size)
                area = GetSizeInOrientation(original, direction)
                if tobreak:
                    break
            
        return result


    def SetRows(self, nMin, nMax):
        """
        Set the number of rows to distribute tool groups over.

        Tool groups can be distributed over a variable number of rows. The way in which
        groups are assigned to rows is not specificed, and the order of groups may
        change, but they will be distributed in such a way as to minimise the overall
        size of the tool bar.

        :param `nMin`: The minimum number of rows to use;
        :param `nMax`: The maximum number of rows to use (defaults to `nMin`).

        """

        if nMax == -1:
            nMax = nMin

        if nMin < 1:
            raise Exception("Exception")
        if nMin > nMax:
            raise Exception("Exception")
            
        self._nrows_min = nMin
        self._nrows_max = nMax

        self._sizes = []
        self._sizes = [wx.Size(0, 0) for i in xrange(self._nrows_min, self._nrows_max + 1)]

        self.Realize()


    def Realize(self):

        if self._art == None:
            return False

        # Calculate the size of each group and the position/size of each tool
        temp_dc = wx.MemoryDC()
        group_count = len(self._groups)

        for group in self._groups:
        
            prev = None
            tool_count = len(group.tools)
            tallest = 0
            
            for t, tool in enumerate(group.tools):
            
                tool.size, tool.dropdown = self._art.GetToolSize(temp_dc, self, tool.bitmap.GetSize(), tool.kind, t==0, t==(tool_count-1))
                tool.state = tool.state & ~RIBBON_TOOLBAR_TOOL_DISABLED
                if t == 0:
                    tool.state |= RIBBON_TOOLBAR_TOOL_FIRST
                if t == tool_count - 1:
                    tool.state |= RIBBON_TOOLBAR_TOOL_LAST
                if tool.size.GetHeight() > tallest:
                    tallest = tool.size.GetHeight()
                if prev:                
                    tool.position = wx.Point(*prev.position)
                    tool.position.x += prev.size.x
                else:                
                    tool.position = wx.Point(0, 0)

                prev = tool
            
            if tool_count == 0:
                group.size = wx.Size(0, 0)
            else:
                group.size = wx.Size(prev.position.x + prev.size.x, tallest)
                for tool in group.tools:
                    tool.size.SetHeight(tallest)
            
        # Calculate the minimum size for each possible number of rows
        sep = self._art.GetMetric(RIBBON_ART_TOOL_GROUP_SEPARATION_SIZE)
        smallest_area = 10000
        row_sizes = [wx.Size(0, 0) for i in xrange(self._nrows_max)]
        major_axis = ((self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL) and [wx.VERTICAL] or [wx.HORIZONTAL])[0]
        self.SetMinSize(wx.Size(0, 0))
        
        for nrows in xrange(self._nrows_min, self._nrows_max+1):
        
            for r in xrange(nrows):
                row_sizes[r] = wx.Size(0, 0)
                
            for g in xrange(group_count):
            
                group = self._groups[g]
                shortest_row = 0

                for r in xrange(1, nrows):                
                    if row_sizes[r].GetWidth() < row_sizes[shortest_row].GetWidth():
                        shortest_row = r
                
                row_sizes[shortest_row].x += group.size.x + sep
                if group.size.y > row_sizes[shortest_row].y:
                    row_sizes[shortest_row].y = group.size.y
            
            size = wx.Size(0, 0)
            
            for r in xrange(nrows):            
                if row_sizes[r].GetWidth() != 0:
                    row_sizes[r].DecBy(sep, 0)
                if row_sizes[r].GetWidth() > size.GetWidth():
                    size.SetWidth(row_sizes[r].GetWidth())
                    
                size.IncBy(0, row_sizes[r].y)
            
            self._sizes[nrows - self._nrows_min] = size

            if GetSizeInOrientation(size, major_axis) < smallest_area:
                self.SetMinSize(size)
                smallest_area = GetSizeInOrientation(size, major_axis)

        # Position the groups
        dummy_event = wx.SizeEvent(self.GetSize())
        self.OnSize(dummy_event)

        return True


    def OnSize(self, event):

        if self._art == None:
            return

        # Choose row count with largest possible area
        size = event.GetSize()
        row_count = self._nrows_max
        major_axis = (self._art.GetFlags() & RIBBON_BAR_FLOW_VERTICAL and [wx.VERTICAL] or [wx.HORIZONTAL])[0]

        if self._nrows_max != self._nrows_min:        
            area = 0
            for i in xrange(self._nrows_max - self._nrows_min + 1):            
                if self._sizes[i].x <= size.x and self._sizes[i].y <= size.y and \
                   GetSizeInOrientation(self._sizes[i], major_axis) > area:
                    area = GetSizeInOrientation(self._sizes[i], major_axis)
                    row_count = self._nrows_min + i

        # Assign groups to rows and calculate row widths
        row_sizes = [wx.Size(0, 0) for i in xrange(row_count)]
        sep = self._art.GetMetric(RIBBON_ART_TOOL_GROUP_SEPARATION_SIZE)

        group_count = len(self._groups)
        for group in self._groups:
            shortest_row = 0
            for r in xrange(1, row_count):            
                if row_sizes[r].GetWidth() < row_sizes[shortest_row].GetWidth():
                    shortest_row = r
            
            group.position = wx.Point(row_sizes[shortest_row].x, shortest_row)
            row_sizes[shortest_row].x += group.size.x + sep
            if group.size.y > row_sizes[shortest_row].y:
                row_sizes[shortest_row].y = group.size.y

        # Calculate row positions
        total_height = 0
        for r in xrange(row_count):
            total_height += row_sizes[r].GetHeight()
            
        rowsep = (size.GetHeight() - total_height) / (row_count + 1)
        rowypos = [0]*row_count
        rowypos[0] = rowsep
        for r in xrange(1, row_count):          
            rowypos[r] = rowypos[r - 1] + row_sizes[r - 1].GetHeight() + rowsep

        # Set group y positions
        for group in self._groups:
            group.position.y = rowypos[group.position.y]
        
        
    def DoGetBestSize(self):

        return self.GetMinSize()


    def OnEraseBackground(self, event):

        # All painting done in main paint handler to minimise flicker
        pass


    def OnPaint(self, event):

        dc = wx.AutoBufferedPaintDC(self)

        if self._art == None:
            return

        self._art.DrawToolBarBackground(dc, self, wx.Rect(0, 0, *self.GetSize()))

        for group in self._groups:
            tool_count = len(group.tools)

            if tool_count != 0:            
                self._art.DrawToolGroupBackground(dc, self, wx.RectPS(group.position, group.size))

                for tool in group.tools:
                    rect = wx.RectPS(group.position + tool.position, tool.size)
                    self._art.DrawTool(dc, self, rect, tool.bitmap, tool.kind, tool.state)
            

    def OnMouseMove(self, event):

        pos = event.GetPosition()
        new_hover = None

        for group in self._groups:

            if group.position.x <= pos.x and pos.x < group.position.x + group.size.x \
               and group.position.y <= pos.y and pos.y < group.position.y + group.size.y: 
                pos -= group.position
                
                for tool in group.tools:
                    if tool.position.x <= pos.x and pos.x < tool.position.x + tool.size.x \
                       and tool.position.y <= pos.y and pos.y < tool.position.y + tool.size.y:                    
                        pos -= tool.position
                        new_hover = tool
                        break
                break
            
        if new_hover != self._hover_tool:
            if self._hover_tool:            
                self._hover_tool.state &= ~(RIBBON_TOOLBAR_TOOL_HOVER_MASK | RIBBON_TOOLBAR_TOOL_ACTIVE_MASK)
            
            self._hover_tool = new_hover
            
            if new_hover:            
                what = RIBBON_TOOLBAR_TOOL_NORMAL_HOVERED
                if new_hover.dropdown.Contains(pos):
                    what = RIBBON_TOOLBAR_TOOL_DROPDOWN_HOVERED

                new_hover.state |= what
                
                if new_hover == self._active_tool:
                
                    new_hover.state &= ~RIBBON_TOOLBAR_TOOL_ACTIVE_MASK
                    new_hover.state |= (what << 2)
                
            self.Refresh(False)
        
        elif self._hover_tool and self._hover_tool.kind == RIBBON_BUTTON_HYBRID:
            newstate = self._hover_tool.state & ~RIBBON_TOOLBAR_TOOL_HOVER_MASK
            what = RIBBON_TOOLBAR_TOOL_NORMAL_HOVERED
            
            if self._hover_tool.dropdown.Contains(pos):
                what = RIBBON_TOOLBAR_TOOL_DROPDOWN_HOVERED
                
            newstate |= what

            if newstate != self._hover_tool.state:            
                self._hover_tool.state = newstate
                if self._hover_tool == self._active_tool:                
                    self._hover_tool.state &= ~RIBBON_TOOLBAR_TOOL_ACTIVE_MASK
                    self._hover_tool.state |= (what << 2)
                
                self.Refresh(False)
            

    def OnMouseDown(self, event):

        self.OnMouseMove(event)
        
        if self._hover_tool:        
            self._active_tool = self._hover_tool
            self._active_tool.state |= (self._active_tool.state & RIBBON_TOOLBAR_TOOL_HOVER_MASK) << 2
            self.Refresh(False)
    

    def OnMouseLeave(self, event):

        if self._hover_tool:        
            self._hover_tool.state &= ~RIBBON_TOOLBAR_TOOL_HOVER_MASK
            self._hover_tool = None
            self.Refresh(False)
    

    def OnMouseUp(self, event):

        if self._active_tool:        
            if self._active_tool.state & RIBBON_TOOLBAR_TOOL_ACTIVE_MASK:            
                evt_type = wxEVT_COMMAND_RIBBONTOOL_CLICKED
                if self._active_tool.state & RIBBON_TOOLBAR_TOOL_DROPDOWN_ACTIVE:
                    evt_type = wxEVT_COMMAND_RIBBONTOOL_DROPDOWN_CLICKED
                    
                notification = RibbonToolBarEvent(evt_type, self._active_tool.id)
                notification.SetEventObject(self)
                notification.SetBar(self)
                self.ProcessEvent(notification)
            
            self._active_tool.state &= ~RIBBON_TOOLBAR_TOOL_ACTIVE_MASK
            self._active_tool = None
            self.Refresh(False)
    

    def OnMouseEnter(self, event):

        if self._active_tool and not event.LeftIsDown():
            self._active_tool = None
        

    def GetDefaultBorder(self):

        return wx.BORDER_NONE

