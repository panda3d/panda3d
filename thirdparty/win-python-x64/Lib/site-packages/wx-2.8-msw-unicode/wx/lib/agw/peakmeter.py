# --------------------------------------------------------------------------------- #
# PEAKMETERCTRL wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 07 October 2008
# Latest Revision: 14 Apr 2010, 12.00 GMT
#
#
# TODO List
#
# 1) Falloff effect for vertical bands;
#
# 2) Possibly some nicer drawing of bands and leds (using GraphicsContext).
#
#
# For all kind of problems, requests of enhancements and bug reports, please
# write to me at:
#
# andrea.gavana@gmail.com
# gavana@kpo.kz
#
# Or, obviously, to the wxPython mailing list!!!
#
#
# End Of Comments
# --------------------------------------------------------------------------------- #

"""
PeakMeterCtrl mimics the behaviour of equalizers that are usually found in stereos
and MP3 players.


Description
===========

PeakMeterCtrl mimics the behaviour of equalizers that are usually found in stereos
and MP3 players. This widgets supports:

* Vertical and horizontal led bands;
* Settings number of bands and leds per band;
* Possibility to change the colour for low/medium/high band frequencies;
* Falloff effects;
* Showing a background grid for the bands.

And a lot more. Check the demo for an almost complete review of the functionalities.


Supported Platforms
===================

PeakMeterCtrl has been tested on the following platforms:
  * Windows (Windows XP).


Window Styles
=============

This class supports the following window styles:

================= =========== ==================================================
Window Styles     Hex Value   Description
================= =========== ==================================================
``PM_HORIZONTAL``         0x0 Shows horizontal bands in `PeakMeterCtrl`.
``PM_VERTICAL``           0x1 Shows vertical bands in `PeakMeterCtrl`.
================= =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

PeakMeterCtrl is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 12 Apr 2010, 12.00 GMT

Version 0.3

"""

import wx

# Horizontal or vertical PeakMeterCtrl
PM_HORIZONTAL = 0
""" Shows horizontal bands in `PeakMeterCtrl`. """
PM_VERTICAL = 1
""" Shows vertical bands in `PeakMeterCtrl`. """

# Some useful constants...
BAND_DEFAULT = 8
LEDS_DEFAULT = 8
BAND_PERCENT = 10       # 10% of Max Range (Auto Decrease)
GRID_INCREASEBY = 15    # Increase Grid colour based on Background colour
FALL_INCREASEBY = 60    # Increase Falloff colour based on Background
DEFAULT_SPEED = 10


def InRange(val, valMin, valMax):
    """
    Returns whether the value `val` is between `valMin` and `valMax`.

    :param `val`: the value to test;
    :param `valMin`: the minimum range value;
    :param `valMax`: the maximum range value.    
    """

    return val >= valMin and val <= valMax


def LightenColour(crColour, byIncreaseVal):
    """
    Lightens a colour.

    :param `crColour`: a valid `wx.Colour` object;
    :param `byIncreaseVal`: an integer specifying the amount for which the input
     colour should be brightened.
    """

    byRed = crColour.Red()
    byGreen = crColour.Green()
    byBlue = crColour.Blue()

    byRed = (byRed + byIncreaseVal <= 255 and [byRed + byIncreaseVal] or [255])[0]
    byGreen = (byGreen + byIncreaseVal <= 255 and [byGreen + byIncreaseVal] or [255])[0]
    byBlue = (byBlue + byIncreaseVal <= 255 and [byBlue + byIncreaseVal] or [255])[0]

    return wx.Colour(byRed, byGreen, byBlue)


def DarkenColour(crColour, byReduceVal):
    """
    Darkens a colour.

    :param `crColour`: a valid `wx.Colour` object;
    :param `byReduceVal`: an integer specifying the amount for which the input
     colour should be darkened.
    """

    byRed = crColour.Red()
    byGreen = crColour.Green()
    byBlue = crColour.Blue()

    byRed = (byRed >= byReduceVal and [byRed - byReduceVal] or [0])[0]
    byGreen = (byGreen >= byReduceVal and [byGreen - byReduceVal] or [0])[0]
    byBlue = (byBlue >= byReduceVal and [byBlue - byReduceVal] or [0])[0]

    return wx.Colour(byRed, byGreen, byBlue)


class PeakMeterData(object):
    """ A simple class which holds data for our L{PeakMeterCtrl}. """

    def __init__(self, value=0, falloff=0, peak=0):
        """
        Default class constructor.

        :param `value`: the current L{PeakMeterCtrl} value;
        :param `falloff`: the falloff effect. ``True`` to enable it, ``False`` to
         disable it;
        :param `peak`: the peak value.
        """

        self._value = value
        self._falloff = falloff
        self._peak = peak

        
    def IsEqual(self, pm):
        """
        Returns whether 2 instances of L{PeakMeterData} are the same.

        :param `pm`: another instance of L{PeakMeterData}.
        """

        return self._value == pm._value


    def IsGreater(self, pm):
        """
        Returns whether one L{PeakMeterData} is greater than another.

        :param `pm`: another instance of L{PeakMeterData}.
        """

        return self._value > pm._value


    def IsLower(self, pm):
        """
        Returns whether one L{PeakMeterData} is smaller than another.

        :param `pm`: another instance of L{PeakMeterData}.
        """

        return self._value < pm._value


class PeakMeterCtrl(wx.PyControl):
    """ The main L{PeakMeterCtrl} implementation. """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=PM_VERTICAL):
        """
        Default class constructor.

        :param parent: the L{PeakMeterCtrl} parent. Must not be ``None``
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyControl` window style;
        :param `agwStyle`: the AGW-specific window style, which can be one of the following bits:

         ================= =========== ==================================================
         Window Styles     Hex Value   Description
         ================= =========== ==================================================
         ``PM_HORIZONTAL``         0x0 Shows horizontal bands in L{PeakMeterCtrl}.
         ``PM_VERTICAL``           0x1 Shows vertical bands in L{PeakMeterCtrl}.
         ================= =========== ==================================================
        
        """

        wx.PyControl.__init__(self, parent, id, pos, size, style)
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)

        self._agwStyle = agwStyle        
        # Initializes all data
        self.InitData()

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_TIMER, self.OnTimer)


    def InitData(self):
        """ Initializes the control. """

        colLime = wx.Colour(0, 255, 0)
        colRed = wx.Colour(255, 0, 0)
        colYellow = wx.Colour(255, 255, 0)

        self._showGrid = False
        self._showFalloff = True
        self._delay = 10
        self._minValue = 60         # Min Range 0-60
        self._medValue = 80         # Med Range 60-80
        self._maxValue = 100        # Max Range 80-100
        self._numBands = BAND_DEFAULT
        self._ledBands = LEDS_DEFAULT
        self._clrBackground = self.GetBackgroundColour()
        self._clrNormal = colLime
        self._clrMedium = colYellow
        self._clrHigh = colRed
        self._speed = DEFAULT_SPEED
        self._timer = wx.Timer(self)
        
        # clear vector data
        self._meterData = []


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the L{PeakMeterCtrl} window style flags.

        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         ================= =========== ==================================================
         Window Styles     Hex Value   Description
         ================= =========== ==================================================
         ``PM_HORIZONTAL``         0x0 Shows horizontal bands in L{PeakMeterCtrl}.
         ``PM_VERTICAL``           0x1 Shows vertical bands in L{PeakMeterCtrl}.
         ================= =========== ==================================================

        """

        self._agwStyle = agwStyle            
        self.Refresh()
        

    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{PeakMeterCtrl} window style.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.        
        """

        return self._agwStyle

    
    def ResetControl(self):
        """ Resets the L{PeakMeterCtrl}. """

        # Initialize vector
        for i in xrange(self._numBands):
            pm = PeakMeterData(self._maxValue, self._maxValue, self._speed)
            self._meterData.append(pm)
        
        self.Refresh()

        
    def SetBackgroundColour(self, colourBgnd):
        """
        Changes the background colour of L{PeakMeterCtrl}.

        :param `colourBgnd`: the colour to be used as the background colour, pass
         `wx.NullColour` to reset to the default colour.

        :note: The background colour is usually painted by the default `wx.EraseEvent`
         event handler function under Windows and automatically under GTK.

        :note: Setting the background colour does not cause an immediate refresh, so
         you may wish to call `wx.Window.ClearBackground` or `wx.Window.Refresh` after
         calling this function.

        :note: Overridden from `wx.PyControl`.
        """

        wx.PyControl.SetBackgroundColour(self, colourBgnd)
        self._clrBackground = colourBgnd
        self.Refresh()


    def SetBandsColour(self, colourNormal, colourMedium, colourHigh):
        """
        Set bands colour for L{PeakMeterCtrl}.

        :param `colourNormal`: the colour for normal (low) bands, a valid `wx.Colour`
         object;
        :param `colourMedium`: the colour for medium bands, a valid `wx.Colour`
         object;
        :param `colourHigh`: the colour for high bands, a valid `wx.Colour`
         object.
        """

        self._clrNormal = colourNormal
        self._clrMedium = colourMedium
        self._clrHigh   = colourHigh

        self.Refresh()        


    def SetMeterBands(self, numBands, ledBands):
        """
        Set number of vertical or horizontal bands to display.

        :param `numBands`: number of bands to display (either vertical or horizontal);
        :param `ledBands`: the number of leds per band.
        
        :note: You can obtain a smooth effect by setting `nHorz` or `nVert` to "1", these
         cannot be 0.
        """

        assert (numBands > 0 and ledBands > 0)
        
        self._numBands = numBands
        self._ledBands = ledBands

        # Reset vector
        self.ResetControl()


    def SetRangeValue(self, minVal, medVal, maxVal):
        """
        Sets the ranges for low, medium and high bands.

        :param `minVal`: the value for low bands;
        :param `medVal`: the value for medium bands;
        :param `maxVal`: the value for high bands.
        
        :note: The conditions to be satisfied are:

         Min: [0 - nMin[,  Med: [nMin - nMed[,  Max: [nMed - nMax]
         
        """

        assert (maxVal > medVal and medVal > minVal and minVal > 0)

        self._minValue = minVal
        self._medValue = medVal
        self._maxValue = maxVal


    def GetRangeValue(self):
        """ Get range value of L{PeakMeterCtrl}. """

        return self._minValue, self._medValue, self._maxValue


    def SetFalloffDelay(self, speed):
        """
        Set peak value speed before falling off.

        :param `speed`: the speed at which the falloff happens.
        """

        self._speed = speed


    def SetFalloffEffect(self, falloffEffect):
        """
        Set falloff effect flag.

        :param `falloffEffect`: ``True`` to enable the falloff effect, ``False``
         to disable it.
        """

        if self._showFalloff != falloffEffect:
        
            self._showFalloff = falloffEffect
            self.Refresh()


    def GetFalloffEffect(self):
        """ Returns the falloff effect flag. """

        return self._showFalloff


    def ShowGrid(self, showGrid):
        """
        Request to have gridlines visible or not.

        :param `showGrid`: ``True`` to show grid lines, ``False`` otherwise.
        """

        if self._showGrid != showGrid:
        
            self._showGrid = showGrid
            self.Refresh()


    def IsGridVisible(self):
        """ Returns if gridlines are visible. """

        return self._showGrid


    def SetData(self, arrayValue, offset, size):
        """
        Change data value. Use this function to change only
        a set of values. All bands can be changed or only 1 band,
        depending on the application.

        :param `arrayValue`: a Python list containing the L{PeakMeterData} values;
        :param `offset`: the (optional) offset where to start applying the new data;
        :param `size`: the size of the input data.
        """

        assert (offset >= 0 and arrayValue != [])
        
        isRunning = self.IsStarted()

        # Stop timer if Animation is active
        if isRunning:
            self.Stop()

        maxSize = offset + size
        
        for i in xrange(offset, maxSize):
        
            if i < len(self._meterData):
            
                pm = self._meterData[i]
                pm._value = arrayValue[i]
                
                if pm._falloff < pm._value:
                
                    pm._falloff = pm._value
                    pm._peak = self._speed
                
                self._meterData[i] = pm
            
        # Auto-restart
        if isRunning:
            return self.Start(self._delay)

        self.Refresh()
        
        return True


    def IsStarted(self):
        """ Check if animation is active. """

        return self._timer.IsRunning()


    def Start(self, delay):
        """
        Start the timer and animation effect.

        :param `delay`: the animation effect delay, in milliseconds.
        """

        if not self.IsStarted():        
            self._delay = delay
            self._timer.Start(self._delay)
        else:
            return False
        
        return True


    def Stop(self):
        """ Stop the timer and animation effect. """

        if self.IsStarted():
            self._timer.Stop()
            return True
        
        return False


    def DoTimerProcessing(self):
        """ L{PeakMeterCtrl} animation, does the ``wx.EVT_TIMER`` processing. """

        self.Refresh()

        decValue  = self._maxValue/self._ledBands
        noChange = True

        for pm in self._meterData:
        
            if pm._value > 0:
            
                pm._value -= (self._ledBands > 1 and [decValue] or [self._maxValue*BAND_PERCENT/100])[0]
                if pm._value < 0:
                    pm._value = 0
                    
                noChange = False
            
            if pm._peak > 0:
            
                pm._peak -= 1
                noChange = False
            

            if pm._peak == 0 and pm._falloff > 0:
            
                pm._falloff -= (self._ledBands > 1 and [decValue >> 1] or [5])[0]
                if pm._falloff < 0:
                    pm._falloff = 0
                    
                noChange = False        

        if noChange: # Stop timer if no more data
            
            self.Stop()


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same size
        as it would have after a call to `Fit()`.
        """

        # something is better than nothing...
        return wx.Size(200, 150)
        

    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{PeakMeterCtrl}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.AutoBufferedPaintDC(self)
        self._clrBackground = self.GetBackgroundColour()
        dc.SetBackground(wx.Brush(self._clrBackground))
        dc.Clear()
        rc = self.GetClientRect()

        pen = wx.Pen(self._clrBackground)
        dc.SetPen(pen)
        
        if self.GetAGWWindowStyleFlag() & PM_VERTICAL:
            self.DrawVertBand(dc, rc)
        else:
            self.DrawHorzBand(dc, rc)


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{PeakMeterCtrl}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        # This is intentionally empty, to reduce flicker
        pass


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{PeakMeterCtrl}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.Refresh()
        event.Skip()


    def OnTimer(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{PeakMeterCtrl}.

        :param `event`: a `wx.TimerEvent` event to be processed.
        """

        self.DoTimerProcessing()


    def DrawHorzBand(self, dc, rect):
        """
        Draws horizontal bands.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the horizontal bands client rectangle.

        :todo: Implement falloff effect for horizontal bands.        
        """

        horzBands = (self._ledBands > 1 and [self._ledBands] or [self._maxValue*BAND_PERCENT/100])[0]
        minHorzLimit = self._minValue*horzBands/self._maxValue
        medHorzLimit = self._medValue*horzBands/self._maxValue
        maxHorzLimit = horzBands

        size = wx.Size(rect.width/horzBands, rect.height/self._numBands)
        rectBand = wx.RectPS(rect.GetTopLeft(), size)

        # Draw band from top
        rectBand.OffsetXY(0, rect.height-size.y*self._numBands)
        xDecal = (self._ledBands > 1 and [1] or [0])[0]
        yDecal = (self._numBands > 1 and [1] or [0])[0]

        for vert in xrange(self._numBands):
        
            self._value = self._meterData[vert]._value
            horzLimit = self._value*horzBands/self._maxValue

            for horz in xrange(horzBands):
            
                rectBand.Deflate(0, yDecal)

                # Find colour based on range value
                colourRect = self._clrBackground
                if self._showGrid:
                    colourRect = DarkenColour(self._clrBackground, GRID_INCREASEBY)

                if self._showGrid and (horz == minHorzLimit or horz == (horzBands-1)):
                
                    points = [wx.Point() for i in xrange(2)]
                    points[0].x = rectBand.GetTopLeft().x + (rectBand.width >> 1)
                    points[0].y = rectBand.GetTopLeft().y - yDecal
                    points[1].x = points[0].x
                    points[1].y = rectBand.GetBottomRight().y + yDecal
                    dc.DrawLinePoint(points[0], points[1])
                
                if horz < horzLimit:
                
                    if InRange(horz, 0, minHorzLimit-1):
                        colourRect = self._clrNormal
                    elif InRange(horz, minHorzLimit, medHorzLimit-1):
                        colourRect = self._clrMedium
                    elif InRange(horz, medHorzLimit, maxHorzLimit):
                        colourRect = self._clrHigh

                dc.SetBrush(wx.Brush(colourRect))                
                dc.DrawRectangleRect(rectBand)

                rectBand.Inflate(0, yDecal)
                rectBand.OffsetXY(size.x, 0)
            
            # Move to Next Vertical band
            rectBand.OffsetXY(-size.x*horzBands, size.y)


    def DrawVertBand(self, dc, rect):
        """
        Draws vertical bands.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the vertical bands client rectangle.
        """

        vertBands = (self._ledBands > 1 and [self._ledBands] or [self._maxValue*BAND_PERCENT/100])[0]
        minVertLimit = self._minValue*vertBands/self._maxValue
        medVertLimit = self._medValue*vertBands/self._maxValue
        maxVertLimit = vertBands

        size = wx.Size(rect.width/self._numBands, rect.height/vertBands)
        rectBand = wx.RectPS(rect.GetTopLeft(), size)

        # Draw band from bottom
        rectBand.OffsetXY(0, rect.bottom-size.y)
        xDecal = (self._numBands > 1 and [1] or [0])[0]
        yDecal = (self._ledBands > 1 and [1] or [0])[0]

        for horz in xrange(self._numBands):
        
            self._value = self._meterData[horz]._value
            vertLimit = self._value*vertBands/self._maxValue
            rectPrev = wx.Rect(*rectBand)

            for vert in xrange(vertBands):
            
                rectBand.Deflate(xDecal, 0)

                # Find colour based on range value
                colourRect = self._clrBackground
                if self._showGrid:
                    colourRect = DarkenColour(self._clrBackground, GRID_INCREASEBY)

                # Draw grid line (level) bar
                if self._showGrid and (vert == minVertLimit or vert == (vertBands-1)):
                
                    points = [wx.Point() for i in xrange(2)]
                    points[0].x = rectBand.GetTopLeft().x - xDecal
                    points[0].y = rectBand.GetTopLeft().y + (rectBand.height >> 1)
                    points[1].x = rectBand.GetBottomRight().x + xDecal
                    points[1].y = points[0].y
                    dc.DrawLinePoint(points[0], points[1])
                
                if vert < vertLimit:
                
                    if InRange(vert, 0, minVertLimit-1):
                        colourRect = self._clrNormal
                    elif InRange(vert, minVertLimit, medVertLimit-1):
                        colourRect = self._clrMedium
                    elif InRange(vert, medVertLimit, maxVertLimit):
                        colourRect = self._clrHigh
                
                dc.SetBrush(wx.Brush(colourRect))
                dc.DrawRectangleRect(rectBand)

                rectBand.Inflate(xDecal, 0)
                rectBand.OffsetXY(0, -size.y)
            
            # Draw falloff effect
            if self._showFalloff:

                oldPen = dc.GetPen()            
                pen = wx.Pen(DarkenColour(self._clrBackground, FALL_INCREASEBY))
                maxHeight = size.y*vertBands
                points = [wx.Point() for i in xrange(2)]
                points[0].x = rectPrev.GetTopLeft().x + xDecal
                points[0].y = rectPrev.GetBottomRight().y - self._meterData[horz]._falloff*maxHeight/self._maxValue
                points[1].x = rectPrev.GetBottomRight().x - xDecal
                points[1].y = points[0].y
                dc.SetPen(pen)
                dc.DrawLinePoint(points[0], points[1])
                dc.SetPen(oldPen)
            
            # Move to Next Horizontal band
            rectBand.OffsetXY(size.x, size.y*vertBands)
        
