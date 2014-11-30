# --------------------------------------------------------------------------- #
# SPEEDMETER Control wxPython IMPLEMENTATION
# Python Code By:
#
# Andrea Gavana, @ 25 Sep 2005
# Latest Revision: 14 Apr 2010, 12.00 GMT
#
#
# TODO List/Caveats
#
# 1. Combination Of The Two Styles:
#
#    SM_DRAW_PARTIAL_FILLER
#    SM_DRAW_SECTORS
#
#    Does Not Work Very Well. It Works Well Only In Case When The Sector Colours
#    Are The Same For All Intervals.
#
#
# Thanks To Gerard Grazzini That Has Tried The Demo On MacOS, I Corrected A
# Bug On Line 246
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
#
# End Of Comments
# --------------------------------------------------------------------------- #


"""
SpeedMeter tries to reproduce the behavior of some car controls (but not only),
by creating an "angular" control (actually, circular).


Description
===========

SpeedMeter tries to reproduce the behavior of some car controls (but not only),
by creating an "angular" control (actually, circular). I remember to have seen
it somewhere, and i decided to implement it in wxPython.

SpeedMeter starts its construction from an empty bitmap, and it uses some
functions of the `wx.DC` class to create the rounded effects. everything is
processed in the `Draw()` method of SpeedMeter class.

This implementation allows you to use either directly the `wx.PaintDC`, or the
better (for me) double buffered style with `wx.BufferedPaintDC`. the double
buffered implementation has been adapted from the wxPython wiki example:

http://wiki.wxpython.org/index.cgi/doublebuffereddrawing


Usage
=====

Sample usage::

    speedwindow1 = SM.SpeedMeter(parent, bufferedstyle,
                                 agwStyle, mousestyle)


None of the options (a part of parent class) are strictly required, if you
use the defaults you get a very simple SpeedMeter. 


Methods and Settings
====================

SpeedMeter is highly customizable, and in particular you can set:

- The start and end angle of existence for SpeedMeter;
- The intervals in which you divide the SpeedMeter (numerical values);
- The corresponding thicks for the intervals;
- The interval colours (different intervals may have different filling colours);
- The ticks font and colour;
- The background colour (outsize the SpeedMeter region);
- The external arc colour;
- The hand (arrow) colour;
- The hand's shadow colour;
- The hand's style ("arrow" or "hand");
- The partial filler colour;
- The number of secondary (intermediate) ticks;
- The direction of increasing speed ("advance" or "reverse");
- The text to be drawn in the middle and its font;
- The icon to be drawn in the middle;
- The first and second gradient colours (that fills the SpeedMeter control);
- The current value.


Window Styles
=============

This class supports the following window styles:

=========================== =========== ==================================================
Window Styles               Hex Value   Description
=========================== =========== ==================================================
``SM_ROTATE_TEXT``                  0x1 Draws the ticks rotated: the ticks are rotated accordingly to the tick marks positions.
``SM_DRAW_SECTORS``                 0x2 Different intervals are painted in differend colours (every sector of the circle has its own colour).
``SM_DRAW_PARTIAL_SECTORS``         0x4 Every interval has its own colour, but only a circle corona is painted near the ticks.
``SM_DRAW_HAND``                    0x8 The hand (arrow indicator) is drawn.
``SM_DRAW_SHADOW``                 0x10 A shadow for the hand is drawn.
``SM_DRAW_PARTIAL_FILLER``         0x20 A circle corona that follows the hand position is drawn near the ticks.
``SM_DRAW_SECONDARY_TICKS``        0x40 Intermediate (smaller) ticks are drawn between principal ticks.
``SM_DRAW_MIDDLE_TEXT``            0x80 Some text is printed in the middle of the control near the center.
``SM_DRAW_MIDDLE_ICON``           0x100 An icon is drawn in the middle of the control near the center.
``SM_DRAW_GRADIENT``              0x200 A gradient of colours will fill the control.
``SM_DRAW_FANCY_TICKS``           0x400 With this style you can use xml tags to create some custom text and draw it at the ticks position. See `wx.lib.fancytext` for the tags.
=========================== =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

SpeedMeter is distributed under the wxPython license.

Latest revision: Andrea Gavana @ 14 Apr 2010, 12.00 GMT

Version 0.3

"""

#----------------------------------------------------------------------
# Beginning Of SPEEDMETER wxPython Code
#----------------------------------------------------------------------

import wx
import wx.lib.colourdb
import wx.lib.fancytext as fancytext

from math import pi, sin, cos, log, sqrt, atan2

#----------------------------------------------------------------------
# DC Drawing Options
#----------------------------------------------------------------------
# SM_NORMAL_DC Uses The Normal wx.PaintDC
# SM_BUFFERED_DC Uses The Double Buffered Drawing Style

SM_NORMAL_DC = 0
SM_BUFFERED_DC = 1

#----------------------------------------------------------------------
# SpeedMeter Styles
#----------------------------------------------------------------------
# SM_ROTATE_TEXT: Draws The Ticks Rotated: The Ticks Are Rotated
#                 Accordingly To The Tick Marks Positions
# SM_DRAW_SECTORS: Different Intervals Are Painted In Differend Colours
#                  (Every Sector Of The Circle Has Its Own Colour)
# SM_DRAW_PARTIAL_SECTORS: Every Interval Has Its Own Colour, But Only
#                          A Circle Corona Is Painted Near The Ticks
# SM_DRAW_HAND: The Hand (Arrow Indicator) Is Drawn
# SM_DRAW_SHADOW: A Shadow For The Hand Is Drawn
# SM_DRAW_PARTIAL_FILLER: A Circle Corona That Follows The Hand Position
#                         Is Drawn Near The Ticks
# SM_DRAW_SECONDARY_TICKS: Intermediate (Smaller) Ticks Are Drawn Between
#                          Principal Ticks
# SM_DRAW_MIDDLE_TEXT: Some Text Is Printed In The Middle Of The Control
#                      Near The Center
# SM_DRAW_MIDDLE_ICON: An Icon Is Drawn In The Middle Of The Control Near
#                      The Center
# SM_DRAW_GRADIENT: A Gradient Of Colours Will Fill The Control
# SM_DRAW_FANCY_TICKS: With This Style You Can Use XML Tags To Create
#                      Some Custom Text And Draw It At The Ticks Position.
#                      See wx.lib.fancytext For The Tags.
                                  
SM_ROTATE_TEXT = 1
""" Draws the ticks rotated: the ticks are rotated accordingly to the tick marks positions. """
SM_DRAW_SECTORS = 2
""" Different intervals are painted in differend colours (every sector of the""" \
""" circle has its own colour). """
SM_DRAW_PARTIAL_SECTORS = 4
""" Every interval has its own colour, but only a circle corona is painted near the ticks. """
SM_DRAW_HAND = 8
""" The hand (arrow indicator) is drawn. """
SM_DRAW_SHADOW = 16
""" A shadow for the hand is drawn. """
SM_DRAW_PARTIAL_FILLER = 32
""" A circle corona that follows the hand position is drawn near the ticks. """
SM_DRAW_SECONDARY_TICKS = 64
""" Intermediate (smaller) ticks are drawn between principal ticks. """
SM_DRAW_MIDDLE_TEXT = 128
""" Some text is printed in the middle of the control near the center. """
SM_DRAW_MIDDLE_ICON = 256
""" An icon is drawn in the middle of the control near the center. """
SM_DRAW_GRADIENT = 512
""" A gradient of colours will fill the control. """
SM_DRAW_FANCY_TICKS = 1024
""" With this style you can use xml tags to create some custom text and""" \
""" draw it at the ticks position. See `wx.lib.fancytext` for the tags. """

#----------------------------------------------------------------------
# Event Binding
#----------------------------------------------------------------------
# SM_MOUSE_TRACK: The Mouse Left Click/Drag Allow You To Change The
#                 SpeedMeter Value Interactively

SM_MOUSE_TRACK = 1


fontfamily = range(70, 78)
familyname = ["default", "decorative", "roman", "script", "swiss", "modern", "teletype"]

weights = range(90, 93)
weightsname = ["normal", "light", "bold"]

styles = [90, 93, 94]
stylesname = ["normal", "italic", "slant"]

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
                 style=wx.NO_FULL_REPAINT_ON_RESIZE, bufferedstyle=SM_BUFFERED_DC):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `bufferedstyle`: if set to ``SM_BUFFERED_DC``, double-buffering will
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

        if self._bufferedstyle == SM_BUFFERED_DC:
            dc = wx.BufferedPaintDC(self, self._Buffer)
        else:
            dc = wx.PaintDC(self)
            dc.DrawBitmap(self._Buffer,0,0)


    def OnSize(self,event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{BufferedWindow}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """
        
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
        self.UpdateDrawing()


    def UpdateDrawing(self):
        """
        This would get called if the drawing needed to change, for whatever reason.

        The idea here is that the drawing is based on some data generated
        elsewhere in the system. if that data changes, the drawing needs to
        be updated.
        """

        if self._bufferedstyle == SM_BUFFERED_DC:
            dc = wx.BufferedDC(wx.ClientDC(self), self._Buffer)
            self.Draw(dc)
        else:
            # update the buffer
            dc = wx.MemoryDC()
            dc.SelectObject(self._Buffer)

            self.Draw(dc)
            # update the screen
            wx.ClientDC(self).Blit(0, 0, self.Width, self.Height, dc, 0, 0)


#----------------------------------------------------------------------
# SPEEDMETER Class
# This Is The Main Class Implementation. See __init__() Method For
# Details.
#----------------------------------------------------------------------

class SpeedMeter(BufferedWindow):
    """
    SpeedMeter tries to reproduce the behavior of some car controls (but not only),
    by creating an "angular" control (actually, circular).

    This is the main class implementation.
    """
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, agwStyle=SM_DRAW_HAND,
                 bufferedstyle=SM_BUFFERED_DC,
                 mousestyle=0):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `agwStyle`: this value specifies the L{SpeedMeter} styles, and can be a
         combination of the following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         ``SM_ROTATE_TEXT``                  0x1 Draws the ticks rotated: the ticks are rotated accordingly to the tick marks positions.
         ``SM_DRAW_SECTORS``                 0x2 Different intervals are painted in differend colours (every sector of the circle has its own colour).
         ``SM_DRAW_PARTIAL_SECTORS``         0x4 Every interval has its own colour, but only a circle corona is painted near the ticks.
         ``SM_DRAW_HAND``                    0x8 The hand (arrow indicator) is drawn.
         ``SM_DRAW_SHADOW``                 0x10 A shadow for the hand is drawn.
         ``SM_DRAW_PARTIAL_FILLER``         0x20 A circle corona that follows the hand position is drawn near the ticks.
         ``SM_DRAW_SECONDARY_TICKS``        0x40 Intermediate (smaller) ticks are drawn between principal ticks.
         ``SM_DRAW_MIDDLE_TEXT``            0x80 Some text is printed in the middle of the control near the center.
         ``SM_DRAW_MIDDLE_ICON``           0x100 An icon is drawn in the middle of the control near the center.
         ``SM_DRAW_GRADIENT``              0x200 A gradient of colours will fill the control.
         ``SM_DRAW_FANCY_TICKS``           0x400 With this style you can use xml tags to create some custom text and draw it at the ticks position. See `wx.lib.fancytext` for the tags.
         =========================== =========== ==================================================
         
        :param `bufferedstyle`: this value allows you to use the normal `wx.PaintDC` or the
         double buffered drawing options:

         =========================== =========== ==================================================
         Buffered Styles             Hex Value   Description
         =========================== =========== ==================================================
         ``SM_NORMAL_DC``                    0x0 Uses the normal `wx.PaintDC`
         ``SM_BUFFERED_DC``                  0x1 Uses the double buffered drawing style
         =========================== =========== ==================================================

        :param `mousestyle`: this value allows you to use the mouse to change the L{SpeedMeter}
         value interactively with left click/drag events. If set to ``SM_MOUSE_TRACK``, the mouse
         left click/drag allow you to change the L{SpeedMeter} value interactively.
        """

        self._agwStyle = agwStyle
        self._bufferedstyle = bufferedstyle
        self._mousestyle = mousestyle

        if self._agwStyle & SM_DRAW_SECTORS and self._agwStyle & SM_DRAW_GRADIENT:
            errstr = "\nERROR: Incompatible Options: SM_DRAW_SECTORS Can Not Be Used In "
            errstr = errstr + "Conjunction With SM_DRAW_GRADIENT."
            raise Exception(errstr)

        if self._agwStyle & SM_DRAW_PARTIAL_SECTORS and self._agwStyle & SM_DRAW_SECTORS:
            errstr = "\nERROR: Incompatible Options: SM_DRAW_SECTORS Can Not Be Used In "
            errstr = errstr + "Conjunction With SM_DRAW_PARTIAL_SECTORS."
            raise Exception(errstr)

        if self._agwStyle & SM_DRAW_PARTIAL_SECTORS and self._agwStyle & SM_DRAW_PARTIAL_FILLER:
            errstr = "\nERROR: Incompatible Options: SM_DRAW_PARTIAL_SECTORS Can Not Be Used In "
            errstr = errstr + "Conjunction With SM_DRAW_PARTIAL_FILLER."
            raise Exception(errstr)

        if self._agwStyle & SM_DRAW_FANCY_TICKS and self._agwStyle & SM_ROTATE_TEXT:
            errstr = "\nERROR: Incompatible Options: SM_DRAW_FANCY_TICKS Can Not Be Used In "
            errstr = errstr + "Conjunction With SM_ROTATE_TEXT."
            raise Exception(errstr)

        if self._agwStyle & SM_DRAW_SHADOW and self._agwStyle & SM_DRAW_HAND == 0:
            errstr = "\nERROR: Incompatible Options: SM_DRAW_SHADOW Can Be Used Only In "
            errstr = errstr + "Conjunction With SM_DRAW_HAND."
            raise Exception(errstr)

        if self._agwStyle & SM_DRAW_FANCY_TICKS:
            wx.lib.colourdb.updateColourDB()


        self.SetAngleRange()
        self.SetIntervals()
        self.SetSpeedValue()
        self.SetIntervalColours()
        self.SetArcColour()
        self.SetTicks()
        self.SetTicksFont()
        self.SetTicksColour()
        self.SetSpeedBackground()
        self.SetHandColour()
        self.SetShadowColour()
        self.SetFillerColour()
        self.SetDirection()
        self.SetNumberOfSecondaryTicks()
        self.SetMiddleText()
        self.SetMiddleTextFont()
        self.SetMiddleTextColour()
        self.SetFirstGradientColour()
        self.SetSecondGradientColour()
        self.SetHandStyle()
        self.DrawExternalArc()

        BufferedWindow.__init__(self, parent, id, pos, size,
                                style=wx.NO_FULL_REPAINT_ON_RESIZE,
                                bufferedstyle=bufferedstyle)

        if self._mousestyle & SM_MOUSE_TRACK:
            self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouseMotion)


    def Draw(self, dc):
        """
        Draws everything on the empty bitmap.
        Here all the chosen styles are applied.

        :param `dc`: an instance of `wx.DC`.        
        """

        size  = self.GetClientSize()

        if size.x < 21 or size.y < 21:
            return

        new_dim = size.Get()

        if not hasattr(self, "dim"):
            self.dim = new_dim

        self.scale = min([float(new_dim[0]) / self.dim[0],
                          float(new_dim[1]) / self.dim[1]])

        # Create An Empty Bitmap
        self.faceBitmap = wx.EmptyBitmap(size.width, size.height)

        dc.BeginDrawing()

        speedbackground = self.GetSpeedBackground()
        # Set Background Of The Control
        dc.SetBackground(wx.Brush(speedbackground))
        dc.Clear()

        centerX = self.faceBitmap.GetWidth()/2
        centerY = self.faceBitmap.GetHeight()/2

        self.CenterX = centerX
        self.CenterY = centerY

        # Get The Radius Of The Sector. Set It A Bit Smaller To Correct Draw After
        radius = min(centerX, centerY) - 2

        self.Radius = radius

        # Get The Angle Of Existance Of The Sector
        anglerange = self.GetAngleRange()
        startangle = anglerange[1]
        endangle = anglerange[0]

        self.StartAngle = startangle
        self.EndAngle = endangle

        # Initialize The Colours And The Intervals - Just For Reference To The
        # Children Functions
        colours = None
        intervals = None

        if self._agwStyle & SM_DRAW_SECTORS or self._agwStyle & SM_DRAW_PARTIAL_SECTORS:
            # Get The Intervals Colours
            colours = self.GetIntervalColours()[:]

        textangles = []
        colourangles = []
        xcoords = []
        ycoords = []

        # Get The Intervals (Partial Sectors)
        intervals = self.GetIntervals()[:]

        start = min(intervals)
        end = max(intervals)
        span = end - start

        self.StartValue = start
        self.EndValue = end

        self.Span = span

        # Get The Current Value For The SpeedMeter
        currentvalue = self.GetSpeedValue()

        # Get The Direction Of The SpeedMeter
        direction = self.GetDirection()
        if direction == "Reverse":
            intervals.reverse()

            if self._agwStyle & SM_DRAW_SECTORS or self._agwStyle & SM_DRAW_PARTIAL_SECTORS:
                colours.reverse()

            currentvalue = end - currentvalue

        # This Because DrawArc Does Not Draw Last Point
        offset = 0.1*self.scale/180.0

        xstart, ystart = self.CircleCoords(radius+1, -endangle, centerX, centerY)
        xend, yend = self.CircleCoords(radius+1, -startangle-offset, centerX, centerY)

        # Calculate The Angle For The Current Value Of SpeedMeter
        accelangle = (currentvalue - start)/float(span)*(startangle-endangle) - startangle

        dc.SetPen(wx.TRANSPARENT_PEN)

        if self._agwStyle & SM_DRAW_PARTIAL_FILLER:

            # Get Some Data For The Partial Filler
            fillercolour = self.GetFillerColour()
            fillerendradius = radius - 10.0*self.scale
            fillerstartradius = radius

            if direction == "Advance":
                fillerstart = accelangle
                fillerend = -startangle
            else:
                fillerstart = -endangle
                fillerend = accelangle

            xs1, ys1 = self.CircleCoords(fillerendradius, fillerstart, centerX, centerY)
            xe1, ye1 = self.CircleCoords(fillerendradius, fillerend, centerX, centerY)
            xs2, ys2 = self.CircleCoords(fillerstartradius, fillerstart, centerX, centerY)
            xe2, ye2 = self.CircleCoords(fillerstartradius, fillerend, centerX, centerY)

            # Get The Sector In Which The Current Value Is
            intersection = self.GetIntersection(currentvalue, intervals)
            sectorradius = radius - 10*self.scale

        else:

            sectorradius = radius

        if self._agwStyle & SM_DRAW_PARTIAL_FILLER:
            # Draw The Filler (Both In "Advance" And "Reverse" Directions)

            dc.SetBrush(wx.Brush(fillercolour))
            dc.DrawArc(xs2, ys2, xe2, ye2, centerX, centerY)

            if self._agwStyle & SM_DRAW_SECTORS == 0:
                dc.SetBrush(wx.Brush(speedbackground))
                xclean1, yclean1 = self.CircleCoords(sectorradius, -endangle, centerX, centerY)
                xclean2, yclean2 = self.CircleCoords(sectorradius, -startangle-offset, centerX, centerY)
                dc.DrawArc(xclean1, yclean1, xclean2, yclean2, centerX, centerY)


        # This Is Needed To Fill The Partial Sector Correctly
        xold, yold = self.CircleCoords(radius, startangle+endangle, centerX, centerY)

        # Draw The Sectors
        for ii, interval in enumerate(intervals):

            if direction == "Advance":
                current = interval - start
            else:
                current = end - interval

            angle = (current/float(span))*(startangle-endangle) - startangle
            angletext = -((pi/2.0) + angle)*180/pi
            textangles.append(angletext)
            colourangles.append(angle)
            xtick, ytick = self.CircleCoords(radius, angle, centerX, centerY)

            # Keep The Coordinates, We Will Need Them After To Position The Ticks
            xcoords.append(xtick)
            ycoords.append(ytick)
            x = xtick
            y = ytick

            if self._agwStyle & SM_DRAW_SECTORS:
                if self._agwStyle & SM_DRAW_PARTIAL_FILLER:
                    if direction == "Advance":
                        if current > currentvalue:
                            x, y = self.CircleCoords(radius, angle, centerX, centerY)
                        else:
                            x, y = self.CircleCoords(sectorradius, angle, centerX, centerY)
                    else:
                        if current < end - currentvalue:
                            x, y = self.CircleCoords(radius, angle, centerX, centerY)
                        else:
                            x, y = self.CircleCoords(sectorradius, angle, centerX, centerY)
                else:
                    x, y = self.CircleCoords(radius, angle, centerX, centerY)


            if ii > 0:
                if self._agwStyle & SM_DRAW_PARTIAL_FILLER and ii == intersection:
                    # We Got The Interval In Which There Is The Current Value. If We Choose
                    # A "Reverse" Direction, First We Draw The Partial Sector, Next The Filler

                    dc.SetBrush(wx.Brush(speedbackground))

                    if direction == "Reverse":
                        if self._agwStyle & SM_DRAW_SECTORS:
                            dc.SetBrush(wx.Brush(colours[ii-1]))

                        dc.DrawArc(xe2, ye2, xold, yold, centerX, centerY)

                    if self._agwStyle & SM_DRAW_SECTORS:
                        dc.SetBrush(wx.Brush(colours[ii-1]))
                    else:
                        dc.SetBrush(wx.Brush(speedbackground))


                    dc.DrawArc(xs1, ys1, xe1, ye1, centerX, centerY)

                    if self._agwStyle & SM_DRAW_SECTORS:
                        dc.SetBrush(wx.Brush(colours[ii-1]))
                        # Here We Draw The Rest Of The Sector In Which The Current Value Is
                        if direction == "Advance":
                            dc.DrawArc(xs1, ys1, x, y, centerX, centerY)
                            x = xs1
                            y = ys1
                        else:
                            dc.DrawArc(xe2, ye2, x, y, centerX, centerY)

                elif self._agwStyle & SM_DRAW_SECTORS:
                    dc.SetBrush(wx.Brush(colours[ii-1]))

                    # Here We Still Use The SM_DRAW_PARTIAL_FILLER Style, But We Are Not
                    # In The Sector Where The Current Value Resides
                    if self._agwStyle & SM_DRAW_PARTIAL_FILLER and ii != intersection:
                        if direction == "Advance":
                            dc.DrawArc(x, y, xold, yold, centerX, centerY)
                        else:
                            if ii < intersection:
                                dc.DrawArc(x, y, xold, yold, centerX, centerY)

                    # This Is The Case Where No SM_DRAW_PARTIAL_FILLER Has Been Chosen
                    else:
                        dc.DrawArc(x, y, xold, yold, centerX, centerY)

            else:
                if self._agwStyle & SM_DRAW_PARTIAL_FILLER and self._agwStyle & SM_DRAW_SECTORS:
                    dc.SetBrush(wx.Brush(fillercolour))
                    dc.DrawArc(xs2, ys2, xe2, ye2, centerX, centerY)
                    x, y = self.CircleCoords(sectorradius, angle, centerX, centerY)
                    dc.SetBrush(wx.Brush(colours[ii]))
                    dc.DrawArc(xs1, ys1, xe1, ye1, centerX, centerY)
                    x = xs2
                    y = ys2

            xold = x
            yold = y

            if self._agwStyle & SM_DRAW_PARTIAL_SECTORS:

                sectorendradius = radius - 10.0*self.scale
                sectorstartradius = radius

                xps, yps = self.CircleCoords(sectorstartradius, angle, centerX, centerY)

                if ii > 0:
                    dc.SetBrush(wx.Brush(colours[ii-1]))
                    dc.DrawArc(xps, yps, xpsold, ypsold, centerX, centerY)

                xpsold = xps
                ypsold = yps


        if self._agwStyle & SM_DRAW_PARTIAL_SECTORS:

            xps1, yps1 = self.CircleCoords(sectorendradius, -endangle+2*offset, centerX, centerY)
            xps2, yps2 = self.CircleCoords(sectorendradius, -startangle-2*offset, centerX, centerY)

            dc.SetBrush(wx.Brush(speedbackground))
            dc.DrawArc(xps1, yps1, xps2, yps2, centerX, centerY)


        if self._agwStyle & SM_DRAW_GRADIENT:

            dc.SetPen(wx.TRANSPARENT_PEN)

            xcurrent, ycurrent = self.CircleCoords(radius, accelangle, centerX, centerY)

            # calculate gradient coefficients
            col2 = self.GetSecondGradientColour()
            col1 = self.GetFirstGradientColour()

            r1, g1, b1 = int(col1.Red()), int(col1.Green()), int(col1.Blue())
            r2, g2, b2 = int(col2.Red()), int(col2.Green()), int(col2.Blue())

            flrect = float(radius+self.scale)

            numsteps = 200

            rstep = float((r2 - r1)) / numsteps
            gstep = float((g2 - g1)) / numsteps
            bstep = float((b2 - b1)) / numsteps

            rf, gf, bf = 0, 0, 0

            radiusteps = flrect/numsteps
            interface = 0

            for ind in range(numsteps+1):
                currCol = (r1 + rf, g1 + gf, b1 + bf)
                dc.SetBrush(wx.Brush(currCol))

                gradradius = flrect - radiusteps*ind
                xst1, yst1 = self.CircleCoords(gradradius, -endangle, centerX, centerY)
                xen1, yen1 = self.CircleCoords(gradradius, -startangle-offset, centerX, centerY)

                if self._agwStyle & SM_DRAW_PARTIAL_FILLER:
                    if gradradius >= fillerendradius:
                        if direction == "Advance":
                            dc.DrawArc(xstart, ystart, xcurrent, ycurrent, centerX, centerY)
                        else:
                            dc.DrawArc(xcurrent, ycurrent, xend, yend, centerX, centerY)
                    else:
                        if interface == 0:
                            interface = 1
                            myradius = fillerendradius + 1
                            xint1, yint1 = self.CircleCoords(myradius, -endangle, centerX, centerY)
                            xint2, yint2 = self.CircleCoords(myradius, -startangle-offset, centerX, centerY)
                            dc.DrawArc(xint1, yint1, xint2, yint2, centerX, centerY)

                        dc.DrawArc(xst1, yst1, xen1, yen1, centerX, centerY)
                else:
                    if self._agwStyle & SM_DRAW_PARTIAL_SECTORS:
                        if gradradius <= sectorendradius:
                            if interface == 0:
                                interface = 1
                                myradius = sectorendradius + 1
                                xint1, yint1 = self.CircleCoords(myradius, -endangle, centerX, centerY)
                                xint2, yint2 = self.CircleCoords(myradius, -startangle-offset, centerX, centerY)
                                dc.DrawArc(xint1, yint1, xint2, yint2, centerX, centerY)
                            else:
                                dc.DrawArc(xst1, yst1, xen1, yen1, centerX, centerY)
                    else:
                        dc.DrawArc(xst1, yst1, xen1, yen1, centerX, centerY)

                rf = rf + rstep
                gf = gf + gstep
                bf = bf + bstep

        textheight = 0

        # Get The Ticks And The Ticks Colour
        ticks = self.GetTicks()[:]
        tickscolour = self.GetTicksColour()

        if direction == "Reverse":
            ticks.reverse()

        if self._agwStyle & SM_DRAW_SECONDARY_TICKS:
            ticknum = self.GetNumberOfSecondaryTicks()
            oldinterval = intervals[0]

        dc.SetPen(wx.Pen(tickscolour, 1))
        dc.SetBrush(wx.Brush(tickscolour))
        dc.SetTextForeground(tickscolour)

        # Get The Font For The Ticks
        tfont, fontsize = self.GetTicksFont()
        tfont = tfont[0]
        myfamily = tfont.GetFamily()

        fsize = self.scale*fontsize
        tfont.SetPointSize(int(fsize))
        tfont.SetFamily(myfamily)
        dc.SetFont(tfont)

        if self._agwStyle & SM_DRAW_FANCY_TICKS:
            facename = tfont.GetFaceName()
            ffamily = familyname[fontfamily.index(tfont.GetFamily())]
            fweight = weightsname[weights.index(tfont.GetWeight())]
            fstyle = stylesname[styles.index(tfont.GetStyle())]
            fcolour = wx.TheColourDatabase.FindName(tickscolour)

        textheight = 0

        # Draw The Ticks And The Markers (Text Ticks)
        for ii, angles in enumerate(textangles):

            strings = ticks[ii]
            if self._agwStyle & SM_DRAW_FANCY_TICKS == 0:
                width, height, dummy, dummy = dc.GetFullTextExtent(strings, tfont)
                textheight = height
            else:
                width, height, dummy = fancytext.GetFullExtent(strings, dc)
                textheight = height

            lX = dc.GetCharWidth()/2.0
            lY = dc.GetCharHeight()/2.0

            if self._agwStyle & SM_ROTATE_TEXT:
                angis = colourangles[ii] - float(width)/(2.0*radius)
                x, y = self.CircleCoords(radius-10.0*self.scale, angis, centerX, centerY)
                dc.DrawRotatedText(strings, x, y, angles)
            else:
                angis = colourangles[ii]
                if self._agwStyle & SM_DRAW_FANCY_TICKS == 0:
                    x, y = self.CircleCoords(radius-10*self.scale, angis, centerX, centerY)
                    lX = lX*len(strings)
                    x = x - lX - width*cos(angis)/2.0
                    y = y - lY - height*sin(angis)/2.0

                if self._agwStyle & SM_DRAW_FANCY_TICKS:
                    fancystr = '<font family="' + ffamily + '" size="' + str(int(fsize)) + '" weight="' + fweight + '"'
                    fancystr = fancystr + ' color="' + fcolour + '"' + ' style="' + fstyle + '"> ' + strings + ' </font>'

                    width, height, dummy = fancytext.GetFullExtent(fancystr, dc)
                    x, y = self.CircleCoords(radius-10*self.scale, angis, centerX, centerY)
                    x = x - width/2.0 - width*cos(angis)/2.0
                    y = y - height/2.0 - height*sin(angis)/2.0
                    fancytext.RenderToDC(fancystr, dc, x, y)
                else:
                    dc.DrawText(strings, x, y)

            # This Is The Small Rectangle --> Tick Mark
            rectangle = colourangles[ii] + pi/2.0

            sinrect = sin(rectangle)
            cosrect = cos(rectangle)
            x1 = xcoords[ii] - self.scale*cosrect
            y1 = ycoords[ii] - self.scale*sinrect
            x2 = x1 + 3*self.scale*cosrect
            y2 = y1 + 3*self.scale*sinrect
            x3 = x1 - 10*self.scale*sinrect
            y3 = y1 + 10*self.scale*cosrect
            x4 = x3 + 3*self.scale*cosrect
            y4 = y3 + 3*self.scale*sinrect

            points = [(x1, y1), (x2, y2), (x4, y4), (x3, y3)]

            dc.DrawPolygon(points)

            if self._agwStyle & SM_DRAW_SECONDARY_TICKS:
                if ii > 0:
                    newinterval = intervals[ii]
                    oldinterval = intervals[ii-1]

                    spacing = (newinterval - oldinterval)/float(ticknum+1)

                    for tcount in xrange(ticknum):
                        if direction == "Advance":
                            oldinterval = (oldinterval + spacing) - start
                            stint = oldinterval
                        else:
                            oldinterval = start + (oldinterval + spacing)
                            stint = end - oldinterval

                        angle = (stint/float(span))*(startangle-endangle) - startangle
                        rectangle = angle + pi/2.0
                        sinrect = sin(rectangle)
                        cosrect = cos(rectangle)
                        xt, yt = self.CircleCoords(radius, angle, centerX, centerY)
                        x1 = xt - self.scale*cosrect
                        y1 = yt - self.scale*sinrect
                        x2 = x1 + self.scale*cosrect
                        y2 = y1 + self.scale*sinrect
                        x3 = x1 - 6*self.scale*sinrect
                        y3 = y1 + 6*self.scale*cosrect
                        x4 = x3 + self.scale*cosrect
                        y4 = y3 + self.scale*sinrect

                        points = [(x1, y1), (x2, y2), (x4, y4), (x3, y3)]

                        dc.DrawPolygon(points)

                    oldinterval = newinterval

        tfont.SetPointSize(fontsize)
        tfont.SetFamily(myfamily)

        self.SetTicksFont(tfont)

        # Draw The External Arc
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        if self._drawarc:
            dc.SetPen(wx.Pen(self.GetArcColour(), 2.0))
            # If It's Not A Complete Circle, Draw The Connecting Lines And The Arc
            if abs(abs(startangle - endangle) - 2*pi) > 1.0/180.0:
                dc.DrawArc(xstart, ystart, xend, yend, centerX, centerY)
                dc.DrawLine(xstart, ystart, centerX, centerY)
                dc.DrawLine(xend, yend, centerX, centerY)
            else:
                # Draw A Circle, Is A 2*pi Extension Arc = Complete Circle
                dc.DrawCircle(centerX, centerY, radius)


        # Here We Draw The Text In The Middle, Near The Start Of The Arrow (If Present)
        # This Is Like The "Km/h" Or "mph" Text In The Cars
        if self._agwStyle & SM_DRAW_MIDDLE_TEXT:

            middlecolour = self.GetMiddleTextColour()
            middletext = self.GetMiddleText()
            middleangle = (startangle + endangle)/2.0

            middlefont, middlesize = self.GetMiddleTextFont()
            middlesize = self.scale*middlesize
            middlefont.SetPointSize(int(middlesize))
            dc.SetFont(middlefont)

            mw, mh, dummy, dummy = dc.GetFullTextExtent(middletext, middlefont)

            newx = centerX + 1.5*mw*cos(middleangle) - mw/2.0
            newy = centerY - 1.5*mh*sin(middleangle) - mh/2.0
            dc.SetTextForeground(middlecolour)
            dc.DrawText(middletext, newx, newy)

        # Here We Draw The Icon In The Middle, Near The Start Of The Arrow (If Present)
        # This Is Like The "Fuel" Icon In The Cars
        if self._agwStyle & SM_DRAW_MIDDLE_ICON:

            middleicon = self.GetMiddleIcon()
            middlewidth, middleheight = self.GetMiddleIconDimens()
            middleicon.SetWidth(middlewidth*self.scale)
            middleicon.SetHeight(middleheight*self.scale)
            middleangle = (startangle + endangle)/2.0

            mw = middleicon.GetWidth()
            mh = middleicon.GetHeight()

            newx = centerX + 1.5*mw*cos(middleangle) - mw/2.0
            newy = centerY - 1.5*mh*sin(middleangle) - mh/2.0

            dc.DrawIcon(middleicon, newx, newy)

            # Restore Icon Dimension, If Not Something Strange Happens
            middleicon.SetWidth(middlewidth)
            middleicon.SetHeight(middleheight)


        # Requested To Draw The Hand
        if self._agwStyle & SM_DRAW_HAND:

            handstyle = self.GetHandStyle()
            handcolour = self.GetHandColour()

            # Calculate The Data For The Hand
            if textheight == 0:
                maxradius = radius-10*self.scale
            else:
                maxradius = radius-5*self.scale-textheight

            xarr, yarr = self.CircleCoords(maxradius, accelangle, centerX, centerY)

            if handstyle == "Arrow":
                x1, y1 = self.CircleCoords(maxradius, accelangle - 4.0/180, centerX, centerY)
                x2, y2 = self.CircleCoords(maxradius, accelangle + 4.0/180, centerX, centerY)
                x3, y3 = self.CircleCoords(maxradius+3*(abs(xarr-x1)), accelangle, centerX, centerY)

                newx = centerX + 4*cos(accelangle)*self.scale
                newy = centerY + 4*sin(accelangle)*self.scale

            else:

                x1 = centerX + 4*self.scale*sin(accelangle)
                y1 = centerY - 4*self.scale*cos(accelangle)
                x2 = xarr
                y2 = yarr
                x3 = centerX - 4*self.scale*sin(accelangle)
                y3 = centerY + 4*self.scale*cos(accelangle)

                x4, y4 = self.CircleCoords(5*self.scale*sqrt(3), accelangle+pi, centerX, centerY)

            if self._agwStyle & SM_DRAW_SHADOW:

                if handstyle == "Arrow":
                    # Draw The Shadow
                    shadowcolour = self.GetShadowColour()
                    dc.SetPen(wx.Pen(shadowcolour, 5*log(self.scale+1)))
                    dc.SetBrush(wx.Brush(shadowcolour))
                    shadowdistance = 2.0*self.scale
                    dc.DrawLine(newx + shadowdistance, newy + shadowdistance,
                                xarr + shadowdistance, yarr + shadowdistance)

                    dc.DrawPolygon([(x1+shadowdistance, y1+shadowdistance),
                                    (x2+shadowdistance, y2+shadowdistance),
                                    (x3+shadowdistance, y3+shadowdistance)])
                else:
                    # Draw The Shadow
                    shadowcolour = self.GetShadowColour()
                    dc.SetBrush(wx.Brush(shadowcolour))
                    dc.SetPen(wx.Pen(shadowcolour, 1.0))
                    shadowdistance = 1.5*self.scale

                    dc.DrawPolygon([(x1+shadowdistance, y1+shadowdistance),
                                    (x2+shadowdistance, y2+shadowdistance),
                                    (x3+shadowdistance, y3+shadowdistance),
                                    (x4+shadowdistance, y4+shadowdistance)])

            if handstyle == "Arrow":

                dc.SetPen(wx.Pen(handcolour, 1.5))

                # Draw The Small Circle In The Center --> The Hand "Holder"
                dc.SetBrush(wx.Brush(speedbackground))
                dc.DrawCircle(centerX, centerY, 4*self.scale)

                dc.SetPen(wx.Pen(handcolour, 5*log(self.scale+1)))
                # Draw The "Hand", An Arrow
                dc.DrawLine(newx, newy, xarr, yarr)

                # Draw The Arrow Pointer
                dc.SetBrush(wx.Brush(handcolour))
                dc.DrawPolygon([(x1, y1), (x2, y2), (x3, y3)])

            else:

                # Draw The Hand Pointer
                dc.SetPen(wx.Pen(handcolour, 1.5))
                dc.SetBrush(wx.Brush(handcolour))
                dc.DrawPolygon([(x1, y1), (x2, y2), (x3, y3), (x4, y4)])

                # Draw The Small Circle In The Center --> The Hand "Holder"
                dc.SetBrush(wx.Brush(speedbackground))
                dc.DrawCircle(centerX, centerY, 4*self.scale)


        dc.EndDrawing()


    def SetIntervals(self, intervals=None):
        """
        Sets the intervals for L{SpeedMeter} (main ticks numeric values).

        :param `intervals`: a Python list of main ticks to be displayed. If defaulted
         to ``None``, the list `[0, 50, 100]` is used.
        """

        if intervals is None:
            intervals = [0, 50, 100]

        self._intervals = intervals


    def GetIntervals(self):
        """ Returns the intervals for L{SpeedMeter}, a Python list of main ticks displayed. """

        return self._intervals


    def SetSpeedValue(self, value=None):
        """
        Sets the current value for L{SpeedMeter}.

        :param `value`: a floating point number representing the current value. If defaulted
         to ``None``, the L{SpeedMeter} value will be the middle point of the control range.
        """

        if value is None:
            value = (max(self._intervals) - min(self._intervals))/2.0
        else:
            if value < min(self._intervals):
                raise Exception("\nERROR: Value Is Smaller Than Minimum Element In Points List")
                return
            elif value > max(self._intervals):
                raise Exception("\nERROR: Value Is Greater Than Maximum Element In Points List")
                return

        self._speedvalue = value
        try:
            self.UpdateDrawing()
        except:
            pass


    def GetSpeedValue(self):
        """ Returns the current value for L{SpeedMeter}. """

        return self._speedvalue


    def SetAngleRange(self, start=0, end=pi):
        """
        Sets the range of existence for L{SpeedMeter}.

        :param `start`: the starting angle, in radians;
        :param `end`: the ending angle, in radians.        
        """

        self._anglerange = [start, end]


    def GetAngleRange(self):
        """
        Returns the range of existence for L{SpeedMeter}.
        The returned values are in radians.
        """

        return self._anglerange


    def SetIntervalColours(self, colours=None):
        """
        Sets the colours for the intervals.

        :param `colours`: a Python list of colours. The length of this list should be
         the same as the number of circle sectors in L{SpeedMeter}. If defaulted to ``None``,
         all the intervals will have a white colour.
         
        :note: Every interval (circle sector) should have a colour.
        """

        if colours is None:
            if not hasattr(self, "_anglerange"):
                errstr = "\nERROR: Impossible To Set Interval Colours,"
                errstr = errstr + " Please Define The Intervals Ranges Before."
                raise Exception(errstr)
                return

            colours = [wx.WHITE]*len(self._intervals)
        else:
            if len(colours) != len(self._intervals) - 1:
                errstr = "\nERROR: Length Of Colour List Does Not Match Length"
                errstr = errstr + " Of Intervals Ranges List."
                raise Exception(errstr)
                return

        self._intervalcolours = colours


    def GetIntervalColours(self):
        """ Returns the colours for the intervals."""

        if hasattr(self, "_intervalcolours"):
            return self._intervalcolours
        else:
            raise Exception("\nERROR: No Interval Colours Have Been Defined")


    def SetTicks(self, ticks=None):
        """
        Sets the ticks for L{SpeedMeter} intervals (main ticks string values).

        :param `ticks`: a Python list of strings, representing the ticks values.
         If defaulted to ``None``, the ticks will be taken from the interval values.
        """

        if ticks is None:
            if not hasattr(self, "_anglerange"):
                errstr = "\nERROR: Impossible To Set Interval Ticks,"
                errstr = errstr + " Please Define The Intervals Ranges Before."
                raise Exception(errstr)
                return

            ticks = []

            for values in self._intervals:
                ticks.append(str(values))

        else:
            if len(ticks) != len(self._intervals):
                errstr = "\nERROR: Length Of Ticks List Does Not Match Length"
                errstr = errstr + " Of Intervals Ranges List."
                raise Exception(errstr)
                return

        self._intervalticks = ticks


    def GetTicks(self):
        """ Returns the ticks for L{SpeedMeter} intervals (main ticks string values)."""

        if hasattr(self, "_intervalticks"):
            return self._intervalticks
        else:
            raise Exception("\nERROR: No Interval Ticks Have Been Defined")


    def SetTicksFont(self, font=None):
        """
        Sets the ticks font.

        :param `font`: a valid `wx.Font` object. If defaulted to ``None``, some standard
         font will be chosen automatically.
        """

        if font is None:
            self._originalfont = [wx.Font(10, wx.SWISS, wx.NORMAL, wx.BOLD, False)]
            self._originalsize = 10
        else:
            self._originalfont = [font]
            self._originalsize = font.GetPointSize()


    def GetTicksFont(self):
        """ Returns the ticks font."""

        return self._originalfont[:], self._originalsize


    def SetTicksColour(self, colour=None):
        """
        Sets the ticks colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, the
         ticks colour will be set as blue.
        """

        if colour is None:
            colour = wx.BLUE

        self._tickscolour = colour


    def GetTicksColour(self):
        """ Returns the ticks colour."""

        return self._tickscolour


    def SetSpeedBackground(self, colour=None):
        """
        Sets the background colour outside the L{SpeedMeter} control.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, the
         L{SpeedMeter} background will be taken from the system default.
        """

        if colour is None:
            colour = wx.SystemSettings_GetColour(0)

        self._speedbackground = colour


    def GetSpeedBackground(self):
        """ Returns the background colour outside the L{SpeedMeter} control."""

        return self._speedbackground


    def SetHandColour(self, colour=None):
        """
        Sets the hand (arrow indicator) colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, the arrow
         indicator will be red.
        """

        if colour is None:
            colour = wx.RED

        self._handcolour = colour


    def GetHandColour(self):
        """ Returns the hand (arrow indicator) colour."""

        return self._handcolour


    def SetArcColour(self, colour=None):
        """
        Sets the external arc colour (thicker line).

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, the arc
         colour will be black.
        """

        if colour is None:
            colour = wx.BLACK

        self._arccolour = colour


    def GetArcColour(self):
        """ Returns the external arc colour."""

        return self._arccolour


    def SetShadowColour(self, colour=None):
        """
        Sets the hand's shadow colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, the shadow
         colour will be light grey.
        """

        if colour is None:
            colour = wx.Colour(150, 150, 150)

        self._shadowcolour = colour


    def GetShadowColour(self):
        """ Returns the hand's shadow colour."""

        return self._shadowcolour


    def SetFillerColour(self, colour=None):
        """
        Sets the partial filler colour.

        A circle corona near the ticks will be filled with this colour, from
        the starting value to the current value of L{SpeedMeter}.

        :param `colour`: a valid `wx.Colour` object.
        """

        if colour is None:
            colour = wx.Colour(255, 150, 50)

        self._fillercolour = colour


    def GetFillerColour(self):
        """ Returns the partial filler colour."""

        return self._fillercolour


    def SetDirection(self, direction=None):
        """
        Sets the direction of advancing L{SpeedMeter} value.

        :param `direction`: specifying "advance" will move the hand in clock-wise direction
         (like normal car speed control), while using "reverse" will move it counterclock-wise
         direction. If defaulted to ``None``, then "advance" will be used.
        """

        if direction is None:
            direction = "Advance"

        if direction not in ["Advance", "Reverse"]:
            raise Exception('\nERROR: Direction Parameter Should Be One Of "Advance" Or "Reverse".')

        self._direction = direction


    def GetDirection(self):
        """ Returns the direction of advancing L{SpeedMeter} value."""

        return self._direction


    def SetNumberOfSecondaryTicks(self, ticknum=None):
        """
        Sets the number of secondary (intermediate) ticks.

        :param `ticknum`: the number of intermediate ticks. If defaulted to ``None``,
         3 ticks are used.
        """

        if ticknum is None:
            ticknum = 3

        if ticknum < 1:
            raise Exception("\nERROR: Number Of Ticks Must Be Greater Than 1.")

        self._secondaryticks = ticknum


    def GetNumberOfSecondaryTicks(self):
        """ Returns the number of secondary (intermediate) ticks. """

        return self._secondaryticks


    def SetMiddleText(self, text=None):
        """
        Sets the text to be drawn near the center of L{SpeedMeter}.

        :param `text`: the text to be drawn near the center of L{SpeedMeter}. If
         defaulted to ``None``, an empty string will be used.
        """

        if text is None:
            text = ""

        self._middletext = text


    def GetMiddleText(self):
        """ Returns the text to be drawn near the center of L{SpeedMeter}. """

        return self._middletext


    def SetMiddleTextFont(self, font=None):
        """
        Sets the font for the text in the middle.

        :param `font`: a valid `wx.Font` object. If defaulted to ``None``, some
         standard font will be generated.
        """

        if font is None:
            self._middletextfont = wx.Font(1, wx.SWISS, wx.NORMAL, wx.BOLD, False)
            self._middletextsize = 10.0
            self._middletextfont.SetPointSize(self._middletextsize)
        else:
            self._middletextfont = font
            self._middletextsize = font.GetPointSize()
            self._middletextfont.SetPointSize(self._middletextsize)


    def GetMiddleTextFont(self):
        """ Returns the font for the text in the middle."""

        return self._middletextfont, self._middletextsize


    def SetMiddleTextColour(self, colour=None):
        """
        Sets the colour for the text in the middle.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, the text
         in the middle will be painted in blue.
        """

        if colour is None:
            colour = wx.BLUE

        self._middlecolour = colour


    def GetMiddleTextColour(self):
        """ Returns the colour for the text in the middle."""

        return self._middlecolour


    def SetMiddleIcon(self, icon):
        """
        Sets the icon to be drawn near the center of L{SpeedMeter}.

        :param `icon`: a valid `wx.Bitmap` object.
        """

        if icon.Ok():
            self._middleicon = icon
        else:
            raise Exception("\nERROR: Invalid Icon Passed To SpeedMeter.")


    def GetMiddleIcon(self):
        """ Returns the icon to be drawn near the center of L{SpeedMeter}. """

        return self._middleicon


    def GetMiddleIconDimens(self):
        """ Used internally. """

        return self._middleicon.GetWidth(), self._middleicon.GetHeight()


    def CircleCoords(self, radius, angle, centerX, centerY):
        """
        Converts the input values into logical x, y coordinates.

        :param `radius`: the L{SpeedMeter} radius;
        :param `angle`: the angular position of the mouse;
        :param `centerX`: the `x` position of the L{SpeedMeter} center;
        :param `centerX`: the `y` position of the L{SpeedMeter} center.        
        """

        x = radius*cos(angle) + centerX
        y = radius*sin(angle) + centerY

        return x, y


    def GetIntersection(self, current, intervals):
        """ Used internally. """

        if self.GetDirection() == "Reverse":
            interval = intervals[:]
            interval.reverse()
        else:
            interval = intervals

        indexes = range(len(intervals))
        try:
            intersection = [ind for ind in indexes if interval[ind] <= current <= interval[ind+1]]
        except:
            if self.GetDirection() == "Reverse":
                intersection = [len(intervals) - 1]
            else:
                intersection = [0]

        return intersection[0]


    def SetFirstGradientColour(self, colour=None):
        """
        Sets the first gradient colour (near the ticks).

        :param `colour`: a valid `wx.Colour` object.
        """

        if colour is None:
            colour = wx.Colour(145, 220, 200)

        self._firstgradientcolour = colour


    def GetFirstGradientColour(self):
        """ Returns the first gradient colour (near the ticks). """

        return self._firstgradientcolour


    def SetSecondGradientColour(self, colour=None):
        """
        Sets the second gradient colour (near the center).

        :param `colour`: a valid `wx.Colour` object.
        """

        if colour is None:
            colour = wx.WHITE

        self._secondgradientcolour = colour


    def GetSecondGradientColour(self):
        """ Returns the first gradient colour (near the center). """

        return self._secondgradientcolour


    def SetHandStyle(self, style=None):
        """
        Sets the style for the hand (arrow indicator).

        :param `style`: by specifying "Hand", L{SpeedMeter} will draw a polygon
         that simulates the car speed control indicator. Using "Arrow" will force
         L{SpeedMeter} to draw a simple arrow. If defaulted to ``None``, "Hand" will
         be used.
         """

        if style is None:
            style = "Hand"

        if style not in ["Hand", "Arrow"]:
            raise Exception('\nERROR: Hand Style Parameter Should Be One Of "Hand" Or "Arrow".')
            return

        self._handstyle = style


    def GetHandStyle(self):
        """ Returns the style for the hand (arrow indicator)."""

        return self._handstyle


    def DrawExternalArc(self, draw=True):
        """
        Specify wheter or not you wish to draw the external (thicker) arc.

        :param `draw`: ``True`` to draw the external arc, ``False`` otherwise.
        """

        self._drawarc = draw


    def OnMouseMotion(self, event):
        """
        Handles the ``wx.EVT_MOUSE_EVENTS`` event for L{SpeedMeter}.

        :param `event`: a `wx.MouseEvent` event to be processed.

        :note: Here only left clicks/drags are involved. Should L{SpeedMeter}
         have something more?
        """

        mousex = event.GetX()
        mousey = event.GetY()

        if event.Leaving():
            return

        pos = self.GetClientSize()
        size = self.GetPosition()
        centerX = self.CenterX
        centerY = self.CenterY

        direction = self.GetDirection()

        if event.LeftIsDown():

            angle = atan2(float(mousey) - centerY, centerX - float(mousex)) + pi - self.EndAngle
            if angle >= 2*pi:
                angle = angle - 2*pi

            if direction == "Advance":
                currentvalue = (self.StartAngle - self.EndAngle - angle)*float(self.Span)/(self.StartAngle - self.EndAngle) + self.StartValue
            else:
                currentvalue = (angle)*float(self.Span)/(self.StartAngle - self.EndAngle) + self.StartValue

            if currentvalue >= self.StartValue and currentvalue <= self.EndValue:
                self.SetSpeedValue(currentvalue)

        event.Skip()


    def GetSpeedStyle(self):
        """ Returns a list of strings and a list of integers containing the styles. """

        stringstyle = []
        integerstyle = []

        if self._agwStyle & SM_ROTATE_TEXT:
            stringstyle.append("SM_ROTATE_TEXT")
            integerstyle.append(SM_ROTATE_TEXT)

        if self._agwStyle & SM_DRAW_SECTORS:
            stringstyle.append("SM_DRAW_SECTORS")
            integerstyle.append(SM_DRAW_SECTORS)

        if self._agwStyle & SM_DRAW_PARTIAL_SECTORS:
            stringstyle.append("SM_DRAW_PARTIAL_SECTORS")
            integerstyle.append(SM_DRAW_PARTIAL_SECTORS)

        if self._agwStyle & SM_DRAW_HAND:
            stringstyle.append("SM_DRAW_HAND")
            integerstyle.append(SM_DRAW_HAND)

        if self._agwStyle & SM_DRAW_SHADOW:
            stringstyle.append("SM_DRAW_SHADOW")
            integerstyle.append(SM_DRAW_SHADOW)

        if self._agwStyle & SM_DRAW_PARTIAL_FILLER:
            stringstyle.append("SM_DRAW_PARTIAL_FILLER")
            integerstyle.append(SM_DRAW_PARTIAL_FILLER)

        if self._agwStyle & SM_DRAW_SECONDARY_TICKS:
            stringstyle.append("SM_DRAW_SECONDARY_TICKS")
            integerstyle.append(SM_DRAW_SECONDARY_TICKS)

        if self._agwStyle & SM_DRAW_MIDDLE_TEXT:
            stringstyle.append("SM_DRAW_MIDDLE_TEXT")
            integerstyle.append(SM_DRAW_MIDDLE_TEXT)

        if self._agwStyle & SM_DRAW_MIDDLE_ICON:
            stringstyle.append("SM_DRAW_MIDDLE_ICON")
            integerstyle.append(SM_DRAW_MIDDLE_ICON)

        if self._agwStyle & SM_DRAW_GRADIENT:
            stringstyle.append("SM_DRAW_GRADIENT")
            integerstyle.append(SM_DRAW_GRADIENT)

        if self._agwStyle & SM_DRAW_FANCY_TICKS:
            stringstyle.append("SM_DRAW_FANCY_TICKS")
            integerstyle.append(SM_DRAW_FANCY_TICKS)

        return stringstyle, integerstyle
