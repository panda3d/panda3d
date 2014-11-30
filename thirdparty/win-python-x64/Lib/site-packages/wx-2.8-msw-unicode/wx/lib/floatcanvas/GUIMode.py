#!/usr/bin/env python
"""

Module that holds the GUI modes used by FloatCanvas

Note that this can only be imported after a wx.App() has been created.

This approach was inpired by Christian Blouin, who also wrote the initial
version of the code.

"""

import wx
## fixme: events should live in their own module, so all of FloatCanvas
##        wouldn't have to be imported here.
import FloatCanvas, Resources
from Utilities import BBox
import numpy as N

class Cursors(object):
    """
    Class to hold the standard Cursors
    
    """
    def __init__(self):
        if "wxMac" in wx.PlatformInfo: # use 16X16 cursors for wxMac
            self.HandCursor = wx.CursorFromImage(Resources.getHand16Image())
            self.GrabHandCursor = wx.CursorFromImage(Resources.getGrabHand16Image())
        
            img = Resources.getMagPlus16Image()
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_X, 6)
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_Y, 6)
            self.MagPlusCursor = wx.CursorFromImage(img)
        
            img = Resources.getMagMinus16Image()
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_X, 6)
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_Y, 6)
            self.MagMinusCursor = wx.CursorFromImage(img)
        else: # use 24X24 cursors for GTK and Windows
            self.HandCursor = wx.CursorFromImage(Resources.getHandImage())
            self.GrabHandCursor = wx.CursorFromImage(Resources.getGrabHandImage())
        
            img = Resources.getMagPlusImage()
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_X, 9)
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_Y, 9)
            self.MagPlusCursor = wx.CursorFromImage(img)
        
            img = Resources.getMagMinusImage()
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_X, 9)
            img.SetOptionInt(wx.IMAGE_OPTION_CUR_HOTSPOT_Y, 9)
            self.MagMinusCursor = wx.CursorFromImage(img)


class GUIBase(object):
    """
    Basic Mouse mode and baseclass for other GUImode.

    This one does nothing with any event

    """
    def __init__(self, Canvas=None):
        self.Canvas = Canvas # set the FloatCanvas for the mode
                             # it gets set when the Mode is set on the Canvas.
        self.Cursors = Cursors()

    Cursor = wx.NullCursor
    def UnSet(self):
        """
        this method gets called by FloatCanvas when a new mode is being set
        on the Canvas
        """
        pass
    # Handlers
    def OnLeftDown(self, event):
        pass
    def OnLeftUp(self, event):
        pass
    def OnLeftDouble(self, event):
        pass
    def OnRightDown(self, event):
        pass
    def OnRightUp(self, event):
        pass
    def OnRightDouble(self, event):
        pass
    def OnMiddleDown(self, event):
        pass
    def OnMiddleUp(self, event):
        pass
    def OnMiddleDouble(self, event):
        pass
    def OnWheel(self, event):
        pass
    def OnMove(self, event):
        pass
    def OnKeyDown(self, event):
        pass
    def OnKeyUp(self, event):
        pass
    def UpdateScreen(self):
        """
        Update gets called if the screen has been repainted in the middle of a zoom in
        so the Rubber Band Box can get updated. Other GUIModes may require something similar
        """
        pass

class GUIMouse(GUIBase):
    """

    Mouse mode checks for a hit test, and if nothing is hit,
    raises a FloatCanvas mouse event for each event.

    """

    Cursor = wx.NullCursor

    # Handlers
    def OnLeftDown(self, event):
        EventType = FloatCanvas.EVT_FC_LEFT_DOWN
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnLeftUp(self, event):
        EventType = FloatCanvas.EVT_FC_LEFT_UP
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnLeftDouble(self, event):
        EventType = FloatCanvas.EVT_FC_LEFT_DCLICK
        if not self.Canvas.HitTest(event, EventType):
                self.Canvas._RaiseMouseEvent(event, EventType)

    def OnMiddleDown(self, event):
        EventType = FloatCanvas.EVT_FC_MIDDLE_DOWN
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnMiddleUp(self, event):
        EventType = FloatCanvas.EVT_FC_MIDDLE_UP
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnMiddleDouble(self, event):
        EventType = FloatCanvas.EVT_FC_MIDDLE_DCLICK
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnRightDown(self, event):
        EventType = FloatCanvas.EVT_FC_RIGHT_DOWN
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnRightUp(self, event):
        EventType = FloatCanvas.EVT_FC_RIGHT_UP
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnRightDouble(self, event):
        EventType = FloatCanvas.EVT_FC_RIGHT_DCLICK
        if not self.Canvas.HitTest(event, EventType):
            self.Canvas._RaiseMouseEvent(event, EventType)

    def OnWheel(self, event):
        EventType = FloatCanvas.EVT_FC_MOUSEWHEEL
        self.Canvas._RaiseMouseEvent(event, EventType)

    def OnMove(self, event):
        ## The Move event always gets raised, even if there is a hit-test
        self.Canvas.MouseOverTest(event)
        self.Canvas._RaiseMouseEvent(event,FloatCanvas.EVT_FC_MOTION)


class GUIMove(GUIBase):
    """
    Mode that moves the image (pans).
    It doesn't change any coordinates, it only changes what the viewport is
    """
    def __init__(self, canvas=None):
        GUIBase.__init__(self, canvas)
        self.Cursor = self.Cursors.HandCursor
        self.GrabCursor = self.Cursors.GrabHandCursor
        self.StartMove = None
        self.MidMove = None
        self.PrevMoveXY = None
        
        ## timer to give a delay when moving so that buffers aren't re-built too many times.
        self.MoveTimer = wx.PyTimer(self.OnMoveTimer)

    def OnLeftDown(self, event):
        self.Canvas.SetCursor(self.GrabCursor)
        self.Canvas.CaptureMouse()
        self.StartMove = N.array( event.GetPosition() )
        self.MidMove = self.StartMove
        self.PrevMoveXY = (0,0)

    def OnLeftUp(self, event):
        self.Canvas.SetCursor(self.Cursor)
        if self.StartMove is not None:
            self.EndMove = N.array(event.GetPosition())
            DiffMove = self.MidMove-self.EndMove
            self.Canvas.MoveImage(DiffMove, 'Pixel', ReDraw=True)

    def OnMove(self, event):
        # Always raise the Move event.
        self.Canvas._RaiseMouseEvent(event, FloatCanvas.EVT_FC_MOTION)
        if event.Dragging() and event.LeftIsDown() and not self.StartMove is None:
            self.EndMove = N.array(event.GetPosition())
            self.MoveImage(event)
            DiffMove = self.MidMove-self.EndMove
            self.Canvas.MoveImage(DiffMove, 'Pixel', ReDraw=False)# reset the canvas without re-drawing
            self.MidMove = self.EndMove
            self.MoveTimer.Start(30, oneShot=True)

    def OnMoveTimer(self, event=None):
        self.Canvas.Draw()

    def UpdateScreen(self):
        ## The screen has been re-drawn, so StartMove needs to be reset.
        self.StartMove = self.MidMove

    def MoveImage(self, event ):
        #xy1 = N.array( event.GetPosition() )
        xy1 = self.EndMove
        wh = self.Canvas.PanelSize
        xy_tl = xy1 - self.StartMove
        dc = wx.ClientDC(self.Canvas)
        dc.BeginDrawing()
        x1,y1 = self.PrevMoveXY
        x2,y2 = xy_tl
        w,h = self.Canvas.PanelSize
        ##fixme: This sure could be cleaner!
        ##   This is all to fill in the background with the background color
        ##   without flashing as the image moves.
        if x2 > x1 and y2 > y1:
            xa = xb = x1
            ya = yb = y1
            wa = w
            ha = y2 - y1
            wb = x2-  x1
            hb = h
        elif x2 > x1 and y2 <= y1:
            xa = x1
            ya = y1
            wa = x2 - x1
            ha = h
            xb = x1
            yb = y2 + h
            wb = w
            hb = y1 - y2
        elif x2 <= x1 and y2 > y1:
            xa = x1
            ya = y1
            wa = w
            ha = y2 - y1
            xb = x2 + w
            yb = y1
            wb = x1 - x2
            hb = h - y2 + y1
        elif x2 <= x1 and y2 <= y1:
            xa = x2 + w
            ya = y1
            wa = x1 - x2
            ha = h
            xb = x1
            yb = y2 + h
            wb = w
            hb = y1 - y2

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self.Canvas.BackgroundBrush)
        dc.DrawRectangle(xa, ya, wa, ha)
        dc.DrawRectangle(xb, yb, wb, hb)
        self.PrevMoveXY = xy_tl
        if self.Canvas._ForeDrawList:
            dc.DrawBitmapPoint(self.Canvas._ForegroundBuffer,xy_tl)
        else:
            dc.DrawBitmapPoint(self.Canvas._Buffer,xy_tl)
        dc.EndDrawing()
        #self.Canvas.Update()

    def OnWheel(self, event):
        """
           By default, zoom in/out by a 0.1 factor per Wheel event.
        """
        if event.GetWheelRotation() < 0:
            self.Canvas.Zoom(0.9)
        else:
            self.Canvas.Zoom(1.1)

class GUIZoomIn(GUIBase):
 
    def __init__(self, canvas=None):
        GUIBase.__init__(self, canvas)
        self.StartRBBox = None
        self.PrevRBBox = None
        self.Cursor = self.Cursors.MagPlusCursor

    def OnLeftDown(self, event):
        self.StartRBBox = N.array( event.GetPosition() )
        self.PrevRBBox = None
        self.Canvas.CaptureMouse()

    def OnLeftUp(self, event):
        if event.LeftUp() and not self.StartRBBox is None:
            self.PrevRBBox = None
            EndRBBox = event.GetPosition()
            StartRBBox = self.StartRBBox
            # if mouse has moved less that ten pixels, don't use the box.
            if ( abs(StartRBBox[0] - EndRBBox[0]) > 10
                    and abs(StartRBBox[1] - EndRBBox[1]) > 10 ):
                EndRBBox = self.Canvas.PixelToWorld(EndRBBox)
                StartRBBox = self.Canvas.PixelToWorld(StartRBBox)
                self.Canvas.ZoomToBB( BBox.fromPoints(N.r_[EndRBBox,StartRBBox]) )
            else:
                Center = self.Canvas.PixelToWorld(StartRBBox)
                self.Canvas.Zoom(1.5,Center)
            self.StartRBBox = None

    def OnMove(self, event):
        # Always raise the Move event.
        self.Canvas._RaiseMouseEvent(event,FloatCanvas.EVT_FC_MOTION)
        if event.Dragging() and event.LeftIsDown() and not (self.StartRBBox is None):
            xy0 = self.StartRBBox
            xy1 = N.array( event.GetPosition() )
            wh  = abs(xy1 - xy0)
            wh[0] = max(wh[0], int(wh[1]*self.Canvas.AspectRatio))
            wh[1] = int(wh[0] / self.Canvas.AspectRatio)
            xy_c = (xy0 + xy1) / 2
            dc = wx.ClientDC(self.Canvas)
            dc.BeginDrawing()
            dc.SetPen(wx.Pen('WHITE', 2, wx.SHORT_DASH))
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetLogicalFunction(wx.XOR)
            if self.PrevRBBox:
                dc.DrawRectanglePointSize(*self.PrevRBBox)
            self.PrevRBBox = ( xy_c - wh/2, wh )
            dc.DrawRectanglePointSize( *self.PrevRBBox )
            dc.EndDrawing()
            
    def UpdateScreen(self):
        """
        Update gets called if the screen has been repainted in the middle of a zoom in
        so the Rubber Band Box can get updated
        """
        #if False:
        if self.PrevRBBox is not None:
            dc = wx.ClientDC(self.Canvas)
            dc.SetPen(wx.Pen('WHITE', 2, wx.SHORT_DASH))
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetLogicalFunction(wx.XOR)
            dc.DrawRectanglePointSize(*self.PrevRBBox)

    def OnRightDown(self, event):
        self.Canvas.Zoom(1/1.5, event.GetPosition(), centerCoords="pixel")

    def OnWheel(self, event):
        if event.GetWheelRotation() < 0:
            self.Canvas.Zoom(0.9)
        else:
            self.Canvas.Zoom(1.1)

class GUIZoomOut(GUIBase):

    def __init__(self, Canvas=None):
        GUIBase.__init__(self, Canvas)
        self.Cursor = self.Cursors.MagMinusCursor
        
    def OnLeftDown(self, event):
        self.Canvas.Zoom(1/1.5, event.GetPosition(), centerCoords="pixel")

    def OnRightDown(self, event):
        self.Canvas.Zoom(1.5, event.GetPosition(), centerCoords="pixel")

    def OnWheel(self, event):
        if event.GetWheelRotation() < 0:
            self.Canvas.Zoom(0.9)
        else:
            self.Canvas.Zoom(1.1)

    def OnMove(self, event):
        # Always raise the Move event.
        self.Canvas._RaiseMouseEvent(event,FloatCanvas.EVT_FC_MOTION)

