#----------------------------------------------------------------------
# Name:        wx.lib.resizewidget
# Purpose:     Adds a resize handle to any widget, with support for
#              notifying parents when layout needs done.
#
# Author:      Robin Dunn
#
# Created:     12-June-2008
# RCS-ID:      $Id: $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

"""
Reparents a given widget into a specialized panel that provides a resize
handle for the widget. When the user drags the resize handle the widget is
resized accordingly, and an event is sent to notify parents that they should
recalculate their layout.
"""

import wx
import wx.lib.newevent

#-----------------------------------------------------------------------------

# dimensions used for the handle
RW_THICKNESS = 4
RW_LENGTH = 12

# colors for the handle
RW_PEN   = 'black'
RW_FILL  = '#A0A0A0'
RW_FILL2 = '#E0E0E0'

# An event and event binder that will notify the containers that they should
# redo the layout in whatever way makes sense for their particular content.
_RWLayoutNeededEvent, EVT_RW_LAYOUT_NEEDED = wx.lib.newevent.NewCommandEvent()


# TODO: Add a style flag that indicates that the ResizeWidget should
# try to adjust the layout itself by looking up the sizer and
# containment hierachy.  Maybe also a style that says that it is okay
# to adjust the size of top-level windows too.

#-----------------------------------------------------------------------------

class ResizeWidget(wx.PyPanel):
    def __init__(self, *args, **kw):
        wx.PyPanel.__init__(self, *args, **kw)
        self._init()

        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOTION,  self.OnMouseMove)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeave)
        
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        
        
    def _init(self):
        self._managedChild = None
        self._bestSize = wx.Size(100,25)
        self._resizeCursor = False
        self._dragPos = None
        self._resizeEnabled = True
        self._reparenting = False
        
        
    def SetManagedChild(self, child):
         self._reparenting = True
         child.Reparent(self)  # This calls AddChild, so do the rest of the init there
         self._reparenting = False
         self.AdjustToChild()
     
    def GetManagedChild(self):
        return self._managedChild
     
    ManagedChild = property(GetManagedChild, SetManagedChild)
     
        
    def AdjustToChild(self):
        self.AdjustToSize(self._managedChild.GetEffectiveMinSize())

        
    def AdjustToSize(self, size):
        size = wx.Size(*size)
        self._bestSize = size + (RW_THICKNESS, RW_THICKNESS)
        self.SetSize(self._bestSize)


    def EnableResize(self, enable=True):
        self._resizeEnabled = enable
        self.Refresh(False)


    def IsResizeEnabled(self):
        return self._resizeEnabled
    

    #=== Event handler methods ===
    def OnLeftDown(self, evt): 
        if self._hitTest(evt.GetPosition()) and self._resizeEnabled:
            self.CaptureMouse()
            self._dragPos = evt.GetPosition()
    
            
    def OnLeftUp(self, evt):
        if self.HasCapture():
            self.ReleaseMouse()
        self._dragPos = None
            
    
    def OnMouseMove(self, evt):
        # set or reset the drag cursor
        pos = evt.GetPosition()
        if self._hitTest(pos) and self._resizeEnabled:
            if not self._resizeCursor:
                self.SetCursor(wx.StockCursor(wx.CURSOR_SIZENWSE))
                self._resizeCursor = True
        else:
            if self._resizeCursor:
                self.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
                self._resizeCursor = False
    
        # determine if a new size is needed
        if evt.Dragging() and self._dragPos is not None:
            delta = self._dragPos - pos
            newSize = self.GetSize() - delta.Get()
            self._adjustNewSize(newSize)
            if newSize != self.GetSize():
                self.SetSize(newSize)
                self._dragPos = pos
                self._bestSize = newSize
                self._sendEvent()
                
                
    def _sendEvent(self):
        event = _RWLayoutNeededEvent(self.GetId())
        event.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(event)
                

    def _adjustNewSize(self, newSize):
        if newSize.width < RW_LENGTH:
            newSize.width = RW_LENGTH
        if newSize.height < RW_LENGTH:
            newSize.height = RW_LENGTH
            
        if self._managedChild:
            minsize = self._managedChild.GetMinSize()
            if minsize.width != -1 and newSize.width - RW_THICKNESS < minsize.width:
                newSize.width = minsize.width + RW_THICKNESS
            if minsize.height != -1 and newSize.height - RW_THICKNESS < minsize.height:
                newSize.height = minsize.height + RW_THICKNESS
            maxsize = self._managedChild.GetMaxSize()
            if maxsize.width != -1 and newSize.width - RW_THICKNESS > maxsize.width:
                newSize.width = maxsize.width + RW_THICKNESS
            if maxsize.height != -1 and newSize.height - RW_THICKNESS > maxsize.height:
                newSize.height = maxsize.height + RW_THICKNESS
    
                
    def OnMouseLeave(self, evt):
        if self._resizeCursor:
            self.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
            self._resizeCursor = False
                        
            
    def OnSize(self, evt): 
        if not self._managedChild:
            return
        sz = self.GetSize() 
        self._managedChild.SetRect(
            wx.RectPS((0,0), sz - (RW_THICKNESS, RW_THICKNESS)))
        r = wx.Rect(sz.width - RW_LENGTH, 
                    sz.height - RW_LENGTH,
                    RW_LENGTH, RW_LENGTH)
        r.Inflate(2,2)
        self.RefreshRect(r)
        
        
    def OnPaint(self, evt): 
        # draw the resize handle
        dc = wx.PaintDC(self)
        w,h = self.GetSize()
        points = [ (w - 1,            h - RW_LENGTH),
                   (w - RW_THICKNESS, h - RW_LENGTH),
                   (w - RW_THICKNESS, h - RW_THICKNESS),
                   (w - RW_LENGTH,    h - RW_THICKNESS),
                   (w - RW_LENGTH,    h - 1),
                   (w - 1,            h - 1),
                   (w - 1,            h - RW_LENGTH),
                   ]
        dc.SetPen(wx.Pen(RW_PEN, 1))
        if self._resizeEnabled:
            fill = RW_FILL
        else:
            fill = RW_FILL2
        dc.SetBrush(wx.Brush(fill))
        dc.DrawPolygon(points)
        
        
    def _hitTest(self, pos):
        # is the position in the area to be used for the resize handle?
        w, h = self.GetSize()
        if ( w - RW_THICKNESS <= pos.x <= w 
             and h - RW_LENGTH <= pos.y <= h ):
            return True
        if ( w - RW_LENGTH <= pos.x <= w 
             and h - RW_THICKNESS <= pos.y <= h ):
            return True
        return False

    
    #=== Overriden virtuals from the base class ===
    def AddChild(self, child):
        assert self._managedChild is None, "Already managing a child widget, can only do one"
        self._managedChild = child
        wx.PyPanel.AddChild(self, child)
        
        # This little hack is needed because if this AddChild was called when
        # the widget was first created, then the OOR values will get reset
        # after this function call, and so the Python proxy object saved in
        # the window may be different than the child object we have now, so we
        # need to reset which proxy object we're using.  Look for it by ID.
        def _doAfterAddChild(self, id):
            if not self:
                return
            child = self.FindWindowById(id)
            self._managedChild = child
            self.AdjustToChild()
            self._sendEvent()
        if self._reparenting:
            _doAfterAddChild(self, child.GetId())
        else:
            wx.CallAfter(_doAfterAddChild, self, child.GetId())
        
    def RemoveChild(self, child):
        self._init()
        wx.PyPanel.RemoveChild(self, child)
        
        
    def DoGetBestSize(self):
        return self._bestSize
        
    
#-----------------------------------------------------------------------------
