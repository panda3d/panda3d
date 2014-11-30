# --------------------------------------------------------------------------------- #
# KNOBCTRL wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 03 Nov 2006
# Latest Revision: 14 Apr 2010, 12.00 GMT
#
#
# TODO List
#
# 1. Any idea?
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# gavana@kpo.kz
# andrea.gavana@gmail.com
#
# Or, Obviously, To The wxPython Mailing List!!!
#
#
# End Of Comments
# --------------------------------------------------------------------------------- #

"""
KnobCtrl lets the user select a numerical value by rotating it.


Description
===========

KnobCtrl lets the user select a numerical value by rotating it. It works like a
scrollbar: just set the ticks range property and read the value property in the
associated ``EVT_KC_ANGLE_CHANGING``/``EVT_KC_ANGLE_CHANGED`` events. Simple but
effective.

It can be easily used if you want to simulate the volume knob of a music player
or similar functionalities.


Events
======

KnobCtrl implements two events that can be intercepted by the user:

- ``EVT_KC_ANGLE_CHANGING``;
- ``EVT_KC_ANGLE_CHANGED``.

The first one can be "vetoed" by eliminating the `event.Skip()` at the end of the
event handler.


Supported Platforms
===================

KnobCtrl has been tested on the following platforms:
  * Windows (Windows XP);
  * Linux Ubuntu (Dapper 6.06)


Window Styles
=============

This class supports the following window styles:

================== =========== ==================================================
Window Styles      Hex Value   Description
================== =========== ==================================================
``KC_BUFFERED_DC``         0x1 Flag to use double buffering (recommendeded = 1).
================== =========== ==================================================


Events Processing
=================

This class processes the following events:

========================= ==================================================
Event Name                Description
========================= ==================================================
``EVT_KC_ANGLE_CHANGED``  Notify the client that the knob has changed its value.
``EVT_KC_ANGLE_CHANGING`` Notify the client that the knob is changing its value.
========================= ==================================================


License And Version
===================

KnobCtrl is distributed under the wxPython license. 

Latest Revision: Andrea Gavana @ 14 Apr 2010, 12.00 GMT

Version 0.3

"""

__docformat__ = "epytext"


import wx
import math

# Flag to use double buffering (recommendeded = 1)
KC_BUFFERED_DC = 1
"""Flag to use double buffering (recommendeded = 1)"""

# Events
wxEVT_KC_ANGLE_CHANGING = wx.NewEventType()
wxEVT_KC_ANGLE_CHANGED = wx.NewEventType()

EVT_KC_ANGLE_CHANGING = wx.PyEventBinder(wxEVT_KC_ANGLE_CHANGING, 1)
""" Notify the client that the knob is changing its value."""
EVT_KC_ANGLE_CHANGED = wx.PyEventBinder(wxEVT_KC_ANGLE_CHANGED, 1)
""" Notify the client that the knob has changed its value."""

# ---------------------------------------------------------------------------- #
# Class KnobCtrlEvent
# ---------------------------------------------------------------------------- #

class KnobCtrlEvent(wx.PyCommandEvent):
    """
    Represent details of the events that the L{KnobCtrl} object sends.
    """

    def __init__(self, eventType, eventId=1):
        """
        Default class constructor.
        For internal use: do not call it in your code!

        :param `eventType`: the event type;
        :param `eventId`: the event identifier.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)


    def SetOldValue(self, oldValue):
        """
        Sets the old L{KnobCtrl} value for this event.

        :param `oldValue`: an integer representing the old value.
        """

        self._oldValue = oldValue


    def GetOldValue(self):
        """ Returns the old L{KnobCtrl} value for this event. """

        return self._oldValue


    def SetValue(self, value):
        """
        Sets the new L{KnobCtrl} value for this event.

        :param `value`: an integer representing the new value.
        """

        self._value = value


    def GetValue(self):
        """ Returns the new L{KnobCtrl} value for this event. """

        return self._value

    
#----------------------------------------------------------------------
# BUFFERENDWINDOW Class
# This Class Has Been Taken From The wxPython Wiki, And Slightly
# Adapted To Fill My Needs. See:
#
# http://wiki.wxpython.org/index.cgi/DoubleBufferedDrawing
#
# For More Info About DC And Double Buffered Drawing.
#----------------------------------------------------------------------

class BufferedWindow(wx.Window):
    """
    A Buffered window class.

    To use it, subclass it and define a `Draw(dc)` method that takes a `dc`
    to draw to. In that method, put the code needed to draw the picture
    you want. The window will automatically be double buffered, and the
    screen will be automatically updated when a Paint event is received.

    When the drawing needs to change, you app needs to call the
    L{BufferedWindow.UpdateDrawing} method. Since the drawing is stored in a bitmap, you
    can also save the drawing to file by calling the
    `SaveToFile(self, file_name, file_type)` method.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.NO_FULL_REPAINT_ON_RESIZE, agwStyle=KC_BUFFERED_DC):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `agwStyle`: if set to ``KC_BUFFERED_DC``, double-buffering will
         be used.
        """

        wx.Window.__init__(self, parent, id, pos, size, style)

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_ERASE_BACKGROUND, lambda x: None)

        # OnSize called to make sure the buffer is initialized.
        # This might result in OnSize getting called twice on some
        # platforms at initialization, but little harm done.
        self.OnSize(None)
        

    def Draw(self, dc):
        """
        This method should be overridden when sub-classed.

        :param `dc`: an instance of `wx.DC`.        
        """
        
        pass


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{BufferedWindow}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """
        
        # All that is needed here is to draw the buffer to screen        
        if self._agwStyle == KC_BUFFERED_DC:
            dc = wx.BufferedPaintDC(self, self._Buffer)
        else:
            dc = wx.PaintDC(self)
            dc.DrawBitmap(self._Buffer,0,0)


    def OnSize(self,event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{BufferedWindow}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        # The Buffer init is done here, to make sure the buffer is always
        # the same size as the Window
        self.Width, self.Height = self.GetClientSizeTuple()

        # Make new off screen bitmap: this bitmap will always have the
        # current drawing in it, so it can be used to save the image to
        # a file, or whatever.

        # This seems required on MacOS, it doesn't like wx.EmptyBitmap with
        # size = (0, 0)
        # Thanks to Gerard Grazzini
        
        if "__WXMAC__" in wx.Platform:
            if self.Width == 0:
                self.Width = 1
            if self.Height == 0:
                self.Height = 1
        
        self._Buffer = wx.EmptyBitmap(self.Width, self.Height)

        memory = wx.MemoryDC()
        memory.SelectObject(self._Buffer)
        memory.SetBackground(wx.Brush(self.GetBackgroundColour()))
        memory.SetPen(wx.TRANSPARENT_PEN)
        memory.Clear()

        minradius = min(0.9*self.Width/2, 0.9*self.Height/2)
        memory.DrawCircle(self.Width/2, self.Height/2, minradius)
        memory.SelectObject(wx.NullBitmap)
        self._region = wx.RegionFromBitmapColour(self._Buffer, self.GetBackgroundColour())
        self._minradius = minradius

        self.UpdateDrawing()


    def UpdateDrawing(self):
        """
        This would get called if the drawing needed to change, for whatever reason.

        The idea here is that the drawing is based on some data generated
        elsewhere in the system. If that data changes, the drawing needs to
        be updated.
        """

        if self._agwStyle == KC_BUFFERED_DC:
            dc = wx.BufferedDC(wx.ClientDC(self), self._Buffer)
            self.Draw(dc)
        else:
            # update the buffer
            dc = wx.MemoryDC()
            dc.SelectObject(self._Buffer)

            self.Draw(dc)
            # update the screen
            wx.ClientDC(self).Blit(0, 0, self.Width, self.Height, dc, 0, 0)
        

# ---------------------------------------------------------------------------- #
# Class KnobCtrl
# ---------------------------------------------------------------------------- #

class KnobCtrl(BufferedWindow):
    """
    This class can be used to simulate a knob volume control often found in
    PC music players.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize,
                 agwStyle=KC_BUFFERED_DC):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `agwStyle`: if set to ``KC_BUFFERED_DC``, double-buffering will
         be used.
        """

        self._agwStyle = agwStyle
        self._knobcolour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)

        self._startcolour = wx.WHITE
        self._endcolour = wx.Colour(170, 170, 150)
        self._tagscolour = wx.BLACK
        self._boundingcolour = wx.WHITE
        self._tags = []
        self._anglestart = -45
        self._angleend = 180
        self._state = 0
        self._minvalue = 0
        self._maxvalue = 100
        self._old_ang = 0
        self._trackposition = 0
        self._knobradius = 4
        
        BufferedWindow.__init__(self, parent, id, pos, size,
                                style=wx.NO_FULL_REPAINT_ON_RESIZE,
                                agwStyle=agwStyle)

        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouseEvents)
        self.SetValue(self._trackposition)

        
    def OnMouseEvents(self, event):
        """
        Handles the ``wx.EVT_MOUSE_EVENTS`` event for L{KnobCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self._state == 0 and event.Entering():
            self._state = 1 
        elif self._state >= 1 and event.Leaving():
            self._state = 0 
        elif self._state == 1 and event.LeftDown():
            self._state = 2 
            self._mousePosition = event.GetPosition()
            self.SetTrackPosition() 
        elif self._state == 2 and event.LeftIsDown():
            self._mousePosition = event.GetPosition()
            self.SetTrackPosition() 
        elif self._state == 2 and event.LeftUp():
            self._state = 1


    def SetTags(self, tags):
        """
        Sets the tags for L{KnobCtrl}.

        :param `tags`: a list of integers ranging from `minvalue` to `maxvalue`.
        """
        
        self._tags = tags
        if min(tags) < self._minvalue:
            self._minvalue = min(tags)

        if max(tags) > self._maxvalue:
            self._maxvalue = max(tags)

        self.OnSize(None)


    def GetMinValue(self):
        """ Returns the minimum value for L{KnobCtrl}. """

        return self._minvalue


    def GetMaxValue(self):
        """ Returns the maximum value for L{KnobCtrl}. """

        return self._maxvalue


    def GetKnobRadius(self):
        """ Returns the knob radius, in pixels. """

        return self._knobradius


    def SetKnobRadius(self, radius):
        """
        Sets the knob radius.

        :param `radius`: the knob radius, in pixels.
        """

        if radius <= 0:
            return

        self._knobradius = radius
        self.UpdateDrawing()

        
    def GetTags(self):
        """ Returns the L{KnobCtrl} tags. """

        return self._tags        


    def SetTagsColour(self, colour):
        """
        Sets the tags colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._tagscolour = colour
        self.UpdateDrawing()


    def GetTagsColour(self):
        """ Returns the tags colour. """

        return self._tagscolour        


    def SetBoundingColour(self, colour):
        """
        Sets the bounding circle colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._boundingcolour = colour
        self.UpdateDrawing()


    def GetBoundingColour(self):
        """ Returns the bounding circle colour. """

        return self._boundingcolour


    def SetFirstGradientColour(self, colour):
        """
        Sets the first gradient colour for shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._startcolour = colour
        self.UpdateDrawing()


    def GetFirstGradientColour(self):
        """ Returns the first gradient colour for shading. """

        return self._startcolour

    
    def SetSecondGradientColour(self, colour):
        """
        Sets the second gradient colour for shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._endcolour = colour
        self.UpdateDrawing()


    def GetSecondGradientColour(self):
        """ Returns the second gradient colour for shading. """

        return self._endcolour


    def SetAngularRange(self, start, end):
        """
        Sets the angular range for L{KnobCtrl}.

        :param `start`: the starting angle, in degrees, clockwise;
        :param `start`: the ending angle, in degrees, clockwise.
        """

        self._anglestart = start
        self._angleend = end
        self.UpdateDrawing()
        

    def GetAngularRange(self):
        """
        Returns the angular range for L{KnobCtrl} as a tuple. The `start` and `end`
        angles in the returned tuple are given in degrees, clockwise.
        """

        return self._anglestart, self._angleend        

                    
    def Draw(self, dc):
        """
        Draws everything on the empty bitmap.
        Here all the chosen styles are applied.

        :param `dc`: an instance of `wx.DC`.
        """
        
        size  = self.GetClientSize()
        
        if size.x < 21 or size.y < 21:
            return

        dc.SetClippingRegionAsRegion(self._region)
        self.DrawDiagonalGradient(dc, size)
        self.DrawInsetCircle(dc, self._knobcolour)
        dc.DestroyClippingRegion()
        self.DrawBoundingCircle(dc, size)

        if self._tags:
            self.DrawTags(dc, size)


    def DrawTags(self, dc, size):
        """
        Draws the tags.

        :param `dc`: an instance of `wx.DC`;
        :param `size`: the control size.
        """

        deltarange = abs(self._tags[-1] - self._tags[0])
        deltaangle = self._angleend - self._anglestart

        width = size.x
        height = size.y

        xshift = 0
        yshift = 0

        if width > height:
            xshift = width - height
        elif width < height:
            yshift = height - width

        coeff = float(deltaangle)/float(deltarange)

        dcPen = wx.Pen(self._tagscolour, 1)

        for tags in self._tags:

            if tags == self._tags[0] or tags == self._tags[-1]:
                # draw first and last tags bigger
                dcPen.SetWidth(2)
                tagLen = 8
            else:
                dcPen.SetWidth(1)
                tagLen = 6
                
            dc.SetPen(dcPen)
                
            tg = tags - self._tags[0]
            angle = tg*coeff + self._anglestart
            angle = angle*math.pi/180.0

            sxi = math.cos(angle)*(width - xshift + tagLen - 6)/2.0
            syi = math.sin(angle)*(height - yshift + tagLen - 6)/2.0

            dxi = math.cos(angle)*((width - xshift + tagLen - 6)/2.0 - tagLen)
            dyi = math.sin(angle)*((height - yshift + tagLen - 6)/2.0 - tagLen)

            dc.DrawLine(width/2 - sxi, height/2 - syi,
                        width/2 - dxi, height/2 - dyi) 
                

    def DrawDiagonalGradient(self, dc, size):
        """
        Draw a shading of diagonal gradient to L{KnobCtrl}.

        :param `dc`: an instance of `wx.DC`;
        :param `size`: the control size.
        """

        col1 = self._startcolour
        col2 = self._endcolour
        
        r1, g1, b1 = int(col1.Red()), int(col1.Green()), int(col1.Blue())
        r2, g2, b2 = int(col2.Red()), int(col2.Green()), int(col2.Blue())

        maxsize = max(size.x, size.y)
        
        flrect = maxsize

        rstep = float((r2 - r1)) / flrect
        gstep = float((g2 - g1)) / flrect
        bstep = float((b2 - b1)) / flrect

        rf, gf, bf = 0, 0, 0
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
            
        for ii in xrange(0, maxsize, 2):
            currCol = (r1 + rf, g1 + gf, b1 + bf)                
            dc.SetPen(wx.Pen(currCol, 2))
            dc.DrawLine(0, ii+2, ii+2, 0)
            rf = rf + rstep
            gf = gf + gstep
            bf = bf + bstep

        for ii in xrange(0, maxsize, 2):
            currCol = (r1 + rf, g1 + gf, b1 + bf)                
            dc.SetPen(wx.Pen(currCol, 2))
            dc.DrawLine(ii+2, maxsize, maxsize, ii+2)
            rf = rf + rstep
            gf = gf + gstep
            bf = bf + bstep


    def OffsetColour(self, colour, offset):
        """
        Changes the input colour by the `offset` value. Used internally.

        :param `colour`: a valid `wx.Colour` object;
        :param `offset`: an integer value for offsetting the input colour.
        """

        byRed = 0
        byGreen = 0
        byBlue = 0
        offsetR = offset
        offsetG = offset
        offsetB = offset

        if offset < -255 or offset> 255:
            return colour

        # Get RGB components of specified colour
        byRed = colour.Red()
        byGreen = colour.Green()
        byBlue = colour.Blue()

        # Calculate max. allowed real offset
        if offset > 0:
        
            if byRed + offset > 255:
                offsetR = 255 - byRed
            if byGreen + offset > 255:
                offsetG = 255 - byGreen
            if byBlue + offset > 255:
                offsetB = 255 - byBlue

            offset = min(min(offsetR, offsetG), offsetB)

        else:
        
            if byRed + offset < 0:
                offsetR = -byRed
            if byGreen + offset < 0:
                offsetG = -byGreen
            if byBlue + offset < 0:
                offsetB = -byBlue

            offset = max(max(offsetR, offsetG), offsetB)

        c1 = wx.Colour(byRed + offset, byGreen + offset, byBlue + offset)
        
        return c1


    def DrawInsetCircle(self, dc, pencolour):
        """
        Draws the small knob.

        :param `dc`: an instance of `wx.DC`;
        :param `pencolour`: the colour to use for drawing the inset circle.
        """

        self._knobcenter = self.CircleCoords(self._minradius*0.8, self.GetTrackPosition(),
                                             self.Width/2, self.Height/2)

        cx, cy = self._knobcenter
        r = self._knobradius
        
        p1 = wx.Pen(self.OffsetColour(pencolour, -70), 2)
        p2 = wx.Pen(self.OffsetColour(pencolour, 10), 1)

        pt1 = wx.Point(cx-r*math.sqrt(2)/2, cy+r*math.sqrt(2)/2)
        pt2 = wx.Point(cx+r*math.sqrt(2)/2, cy-r*math.sqrt(2)/2)
        
        dc.SetPen(p2)
        dc.DrawArcPoint(pt1, pt2, (cx, cy))
        dc.SetPen(p1)
        dc.DrawArcPoint(pt2, pt1, (cx, cy))


    def DrawBoundingCircle(self, dc, size):
        """
        Draws the L{KnobCtrl} bounding circle.

        :param `dc`: an instance of `wx.DC`;
        :param `size`: the control size.
        """

        radius = 0.9*min(size.x, size.y)/2
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(wx.Pen(self._boundingcolour))
        dc.DrawCircle(self.Width/2, self.Height/2, radius)
        
    
    def CircleCoords(self, radius, angle, centerX, centerY):
        """
        Converts the input values into logical `x` and `y` coordinates.

        :param `radius`: the L{KnobCtrl} radius;
        :param `angle`: the angular position of the mouse;
        :param `centerX`: the `x` position of the L{KnobCtrl} center;
        :param `centerX`: the `y` position of the L{KnobCtrl} center.        
        """
        
        x = radius*math.cos(angle) + centerX
        y = radius*math.sin(angle) + centerY
        
        return x, y


    def SetTrackPosition(self):
        """ Used internally. """

        width, height = self.GetSize()
        
        x = self._mousePosition.x 
        y = self._mousePosition.y 
        
        ang = self.GetAngleFromCoord(x, y)
        val = ang*180.0/math.pi
        
        deltarange = self._maxvalue - self._minvalue
        deltaangle = self._angleend - self._anglestart

        coeff = float(deltaangle)/float(deltarange)

        if self._anglestart < 0 and val >= 360.0 + self._anglestart:
            scaledval = (val  - (360.0 + self._anglestart))/coeff
        else:
            scaledval = (val  - self._anglestart)/coeff

        if scaledval > self._maxvalue or scaledval < self._minvalue:
            ang = self._old_ang 
        else:
            event = KnobCtrlEvent(wxEVT_KC_ANGLE_CHANGING, self.GetId())
            event.SetEventObject(self)
            event.SetOldValue(self.GetValue())
            event.SetValue(int(round(scaledval)))
            
            if self.GetEventHandler().ProcessEvent(event):
                # the caller didn't use event.Skip()
                return

            self.SetValue(scaledval)
            event.SetEventType(wxEVT_KC_ANGLE_CHANGED)
            event.SetOldValue(scaledval)
            self.GetEventHandler().ProcessEvent(event)
            
        self._old_ang = ang


    def SetValue(self, val):
        """
        Sets programmatically the value of L{KnobCtrl}.

        :param `val`: an integer specifying the new L{KnobCtrl} value.

        :note: This method does not send a L{KnobCtrlEvent}.        
        """

        if val < self._minvalue or val > self._maxvalue:
            return

        width, height = self.GetSize()
        
        deltarange = self._maxvalue - self._minvalue
        deltaangle = self._angleend - self._anglestart

        coeff = float(deltaangle)/float(deltarange)

        ang = 360.0 + val*coeff + self._anglestart
        
        ang = ang*math.pi/180.0
        self._old_ang = ang

        self._trackposition = int(round(val))
        self.UpdateDrawing()


    def GetValue(self):
        """ Returns the value of L{KnobCtrl}. """

        return self._trackposition


    def GetTrackPosition(self):
        """ Used internally. """
        
        return self._old_ang - math.pi


    def GetAngleFromCoord(self, cx, cy):
        """
        Returns the angular position based on the input logical coordinates.
        Used internally.

        :param `cx`: the `x` logical position;
        :param `cy`: the `y` logical position.
        """

        width, height = self.GetSize()
        
        ang = 0
        y = (height/2 - float(cy))/(height/2) 
        x = (float(cx) - width/2)/(height/2)

        ang = ang - math.atan2(-y, -x)

        if ang < 0:
            ang = ang + 2.0*math.pi

        return  ang 

