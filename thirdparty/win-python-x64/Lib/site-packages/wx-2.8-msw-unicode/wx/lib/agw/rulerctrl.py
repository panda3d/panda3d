# --------------------------------------------------------------------------------- #
# RULERCTRL wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 03 Nov 2006
# Latest Revision: 01 Dec 2009, 09.00 GMT
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
RulerCtrl implements a ruler window that can be placed on top, bottom, left or right
to any wxPython widget.


Description
===========

RulerCtrl implements a ruler window that can be placed on top, bottom, left or right
to any wxPython widget. It is somewhat similar to the rulers you can find in text
editors software, though not so powerful.

RulerCtrl has the following characteristics:

- Can be horizontal or vertical;
- 4 built-in formats: integer, real, time and linearDB formats;
- Units (as ``cm``, ``dB``, ``inches``) can be displayed together with the label values;
- Possibility to add a number of "paragraph indicators", small arrows that point at
  the current indicator position;
- Customizable background colour, tick colour, label colour;
- Possibility to flip the ruler (i.e. changing the tick alignment);
- Changing individually the indicator colour (requires PIL at the moment);
- Different window borders are supported (``wx.STATIC_BORDER``, ``wx.SUNKEN_BORDER``,
  ``wx.DOUBLE_BORDER``, ``wx.NO_BORDER``, ``wx.RAISED_BORDER``, ``wx.SIMPLE_BORDER``);
- Logarithmic scale available;
- Possibility to draw a thin line over a selected window when moving an indicator,
  which emulates the text editors software.
  

And a lot more. See the demo for a review of the functionalities.


Events
======

RulerCtrl implements the following events related to indicators:

- ``EVT_INDICATOR_CHANGING``: the user is about to change the position of one indicator;
- ``EVT_INDICATOR_CHANGED``: the user has changed the position of one indicator.


Supported Platforms
===================

RulerCtrl has been tested on the following platforms:
  * Windows (Windows XP);
  * Linux Ubuntu (Dapper 6.06)


Window Styles
=============

`No particular window styles are available for this class.`


Events Processing
=================

This class processes the following events:

========================== ==================================================
Event Name                 Description
========================== ==================================================
``EVT_INDICATOR_CHANGED``  The user has changed the indicator value.
``EVT_INDICATOR_CHANGING`` The user is about to change the indicator value.
========================== ==================================================


License And Version
===================

RulerCtrl is distributed under the wxPython license. 

Latest Revision: Andrea Gavana @ 01 Dec 2009, 09.00 GMT

Version 0.3

"""

__docformat__ = "epytext"


import wx
import math
import cStringIO, zlib

# Try to import PIL, if possible.
# This is used only to change the colour for an indicator arrow.
_hasPIL = False
try:
    import Image
    _hasPIL = True
except:
    pass

# Built-in formats
IntFormat = 1
""" Integer format. """
RealFormat = 2
""" Real format. """
TimeFormat = 3
""" Time format. """
LinearDBFormat = 4
""" Linear DB format. """
HHMMSS_Format = 5
""" HHMMSS format. """

# Events
wxEVT_INDICATOR_CHANGING = wx.NewEventType()
wxEVT_INDICATOR_CHANGED = wx.NewEventType()

EVT_INDICATOR_CHANGING = wx.PyEventBinder(wxEVT_INDICATOR_CHANGING, 2)
""" The user is about to change the indicator value. """
EVT_INDICATOR_CHANGED = wx.PyEventBinder(wxEVT_INDICATOR_CHANGED, 2)
""" The user has changed the indicator value. """


# Some accessor functions
#----------------------------------------------------------------------

def GetIndicatorData():
    """ Returns the image indicator as a decompressed stream of characters. """
    
    return zlib.decompress(
'x\xda\x01x\x01\x87\xfe\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\n\x00\
\x00\x00\n\x08\x06\x00\x00\x00\x8d2\xcf\xbd\x00\x00\x00\x04sBIT\x08\x08\x08\
\x08|\x08d\x88\x00\x00\x01/IDAT\x18\x95m\xceO(\x83q\x1c\xc7\xf1\xf7\xef\xf9\
\xcd\xf6D6\xca\x1c\xc8\x9f\x14\'J-\xc4A9(9(-\xe5 \xed\xe4\xe2\xe2\xb2\x928\
\xb9\xec\xc2\x01\x17.\x0e\xe4\xe6B\xed\xb2\x1c\xdc$5\x97\xf9S\xb3\x14+\x0eO\
\xdb\xccZ\x9e\xfd\xf9\xba\x98E{\x1d\xbf\xbd\xfb\xf4U\x00\x18\x9d\xc3\xad\x1d\
\xa1+\xa7S\x15\xf8\xa1\xb5i\xbc\xc4\xd7\x0f\xca\xc5\xd82U3[\x97\xb1\x82\xc4S\
"\x89\xb4\xc8SZ\xc4\xb2E\xfa\x06CR)\x1c\x00\xb8\x8cb"-|\x94@\x01\x0e\r\xee&\
\xf8\x12\xc5\xdf\xd0\xd4\xf2\xf6i\x90/\x82\xe9\x82\xdb\xe72\xa7\xe7%\x92\x99\
\xdfA\xb4j\x9b]\xa5\xaek\xbag|\xaa\xdd\xca)\xceb\x10\xbe\x87\xacm VT\xd0N\
\x0f\xf9\xd7\x94\xd6\xde\xb1\xdd\xf9\xcdm_\x83\xdb\x81\x95W\x88\x02\xad\x159\
\x01\xcc!U2}\xa3$\x0f\x1dZR\xd1\xfd\xbb\x9b\xc7\x89\xc99\x7f\xb7\xb7\xd1\x00\
\xc0.B\xbe\xac\xc8\xbe?P\x8e\x8c\x1ccg\x02\xd5\x1f\x9a\x07\xf6\x82a[6.D\xfc\
\'"\x9e\xc0\xb5\xa0\xeb\xd7\xa8\xc9\xdd\xbf\xb3pdI\xefRD\xc0\x08\xd6\x8e*\\-\
+\xa0\x17\xff\x9f\xbf\x01{\xb5t\x9e\x99]a\x97\x00\x00\x00\x00IEND\xaeB`\x82G\
\xbf\xa8>' )


def GetIndicatorBitmap():
    """ Returns the image indicator as a `wx.Bitmap`. """

    return wx.BitmapFromImage(GetIndicatorImage())


def GetIndicatorImage():
    """ Returns the image indicator as a `wx.Image`. """
    
    stream = cStringIO.StringIO(GetIndicatorData())
    return wx.ImageFromStream(stream)


def MakePalette(tr, tg, tb):
    """
    Creates a palette to be applied on an image based on input colour.

    :param `tr`: the red intensity of the input colour;
    :param `tg`: the green intensity of the input colour;
    :param `tb`: the blue intensity of the input colour.
    """
    
    l = []
    for i in range(255):
        l.extend([tr*i / 255, tg*i / 255, tb*i / 255])
        
    return l


def ConvertWXToPIL(bmp):
    """
    Converts a `wx.Image` into a PIL image.

    :param `bmp`: an instance of `wx.Image`.

    :note: Requires PIL (Python Imaging Library), which can be downloaded from
     http://www.pythonware.com/products/pil/
    """
    
    width = bmp.GetWidth()
    height = bmp.GetHeight()
    img = Image.fromstring("RGBA", (width, height), bmp.GetData())

    return img


def ConvertPILToWX(pil, alpha=True):
    """
    Converts a PIL image into a `wx.Image`.

    :param `pil`: a PIL image;
    :param `alpha`: ``True`` if the image contains alpha transparency, ``False``
     otherwise.

    :note: Requires PIL (Python Imaging Library), which can be downloaded from
     http://www.pythonware.com/products/pil/
    """
    
    if alpha:
        image = apply(wx.EmptyImage, pil.size)
        image.SetData(pil.convert("RGB").tostring())
        image.SetAlphaData(pil.convert("RGBA").tostring()[3::4])
    else:
        image = wx.EmptyImage(pil.size[0], pil.size[1])
        new_image = pil.convert('RGB')
        data = new_image.tostring()
        image.SetData(data)
       
    return image

# ---------------------------------------------------------------------------- #
# Class RulerCtrlEvent
# ---------------------------------------------------------------------------- #

class RulerCtrlEvent(wx.PyCommandEvent):
    """
    Represent details of the events that the L{RulerCtrl} object sends.
    """
    
    def __init__(self, eventType, eventId=1):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)


    def SetValue(self, value):
        """
        Sets the event value.

        :param `value`: the new indicator position.
        """

        self._value = value


    def GetValue(self):
        """ Returns the event value. """

        return self._value
    

    def SetOldValue(self, oldValue):
        """
        Sets the event old value.

        :param `value`: the old indicator position.
        """

        self._oldValue = oldValue


    def GetOldValue(self):
        """ Returns the event old value. """

        return self._oldValue        


# ---------------------------------------------------------------------------- #
# Class Label
# ---------------------------------------------------------------------------- #

class Label(object):
    """
    Auxilary class. Just holds information about a label in L{RulerCtrl}.
    """
    
    def __init__(self, pos=-1, lx=-1, ly=-1, text=""):
        """
        Default class constructor.

        :param `pos`: the indicator position;
        :param `lx`: the indicator `x` coordinate;
        :param `ly`: the indicator `y` coordinate;
        :param `text`: the label text.
        """

        self.pos = pos
        self.lx = lx
        self.ly = ly
        self.text = text


# ---------------------------------------------------------------------------- #
# Class Indicator
# ---------------------------------------------------------------------------- #

class Indicator(object):
    """
    This class holds all the information about a single indicator inside L{RulerCtrl}.
    
    You should not call this class directly. Use::

        ruler.AddIndicator(id, value)


    to add an indicator to your L{RulerCtrl}.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, value=0):
        """
        Default class constructor.

        :param `parent`: the parent window, an instance of L{RulerCtrl};
        :param `id`: the indicator identifier;
        :param `value`: the initial value of the indicator.
        """

        self._parent = parent
        if id == wx.ID_ANY:
            id = wx.NewId()

        self._id = id
        self._colour = None

        self.RotateImage(GetIndicatorImage())
        self.SetValue(value)


    def GetPosition(self):
        """ Returns the position at which we should draw the indicator bitmap. """

        orient = self._parent._orientation
        flip = self._parent._flip
        left, top, right, bottom = self._parent.GetBounds()
        minval = self._parent._min
        maxval = self._parent._max
        
        value = self._value
        
        if self._parent._log:
            value = math.log10(value)
            maxval = math.log10(maxval)
            minval = math.log10(minval)

        pos = float(value-minval)/abs(maxval - minval)

        if orient == wx.HORIZONTAL:
            xpos = int(pos*right) - self._img.GetWidth()/2
            if flip:
                ypos = top
            else:
                ypos = bottom - self._img.GetHeight()
        else:
            ypos = int(pos*bottom) - self._img.GetHeight()/2
            if flip:
                xpos = left
            else:
                xpos = right - self._img.GetWidth()

        return xpos, ypos
                    

    def GetImageSize(self):
        """ Returns the indicator bitmap size. """

        return self._img.GetWidth(), self._img.GetHeight()
    

    def GetRect(self):
        """ Returns the indicator client rectangle. """

        return self._rect

    
    def RotateImage(self, img=None):
        """
        Rotates the default indicator bitmap.

        :param `img`: if not ``None``, the indicator image.
        """

        if img is None:
            img = GetIndicatorImage()
            
        orient = self._parent._orientation
        flip = self._parent._flip
        left, top, right, bottom = self._parent.GetBounds()

        if orient == wx.HORIZONTAL:
            if flip:
                img = img.Rotate(math.pi, (5, 5), True)
        else:
            if flip:
                img = img.Rotate(-math.pi/2, (5, 5), True)
            else:
                img = img.Rotate(math.pi/2, (5, 5), True)
        
        self._img = img
        

    def SetValue(self, value):
        """
        Sets the indicator value.

        :param `value`: the new indicator value.
        """
        
        if value < self._parent._min:
            value = self._parent._min
        if value > self._parent._max:
            value = self._parent._max

        self._value = value
        self._rect = wx.Rect()
        
        self._parent.Refresh()


    def GetValue(self):
        """ Returns the indicator value. """

        return self._value        


    def Draw(self, dc):
        """
        Actually draws the indicator.

        :param `dc`: an instance of `wx.DC`.
        """

        xpos, ypos = self.GetPosition()
        bmp = wx.BitmapFromImage(self._img)
        dc.DrawBitmap(bmp, xpos, ypos, True)
        self._rect = wx.Rect(xpos, ypos, self._img.GetWidth(), self._img.GetHeight())


    def GetId(self):
        """ Returns the indicator id. """

        return self._id

    
    def SetColour(self, colour):
        """
        Sets the indicator colour.

        :param `colour`: the new indicator colour, an instance of `wx.Colour`.
        
        :note: Requires PIL (Python Imaging Library), which can be downloaded from
         http://www.pythonware.com/products/pil/
        """

        if not _hasPIL:
            return
        
        palette = colour.Red(), colour.Green(), colour.Blue()

        img = ConvertWXToPIL(GetIndicatorBitmap())
        l = MakePalette(*palette)
        # The Palette Can Be Applied Only To "L" And "P" Images, Not "RGBA"
        img = img.convert("L")
        # Apply The New Palette
        img.putpalette(l)
        # Convert The Image Back To RGBA
        img = img.convert("RGBA")
        img = ConvertPILToWX(img)
        self.RotateImage(img)

        self._parent.Refresh()
        

# ---------------------------------------------------------------------------- #
# Class RulerCtrl
# ---------------------------------------------------------------------------- #

class RulerCtrl(wx.PyPanel):
    """
    RulerCtrl implements a ruler window that can be placed on top, bottom, left or right
    to any wxPython widget. It is somewhat similar to the rulers you can find in text
    editors software, though not so powerful.
    """

    def __init__(self, parent, id=-1, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.STATIC_BORDER, orient=wx.HORIZONTAL):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `orient`: sets the orientation of the L{RulerCtrl}, and can be either
         ``wx.HORIZONTAL`` of ``wx.VERTICAL``.
        """
        
        wx.PyPanel.__init__(self, parent, id, pos, size, style)

        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)        
        width, height = size

        self._min = 0.0
        self._max = 10.0
        self._orientation = orient
        self._spacing = 5
        self._hassetspacing = False
        self._format = RealFormat
        self._flip = False
        self._log = False
        self._labeledges = False
        self._units = ""
        
        self._drawingparent = None
        self._drawingpen = wx.Pen(wx.BLACK, 2)

        self._left = -1
        self._top = -1
        self._right = -1
        self._bottom = -1

        self._major = 1
        self._minor = 1

        self._indicators = []
        self._currentIndicator = None

        fontsize = 10
        if wx.Platform == "__WXMSW__":
            fontsize = 8

        self._minorfont = wx.Font(fontsize, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self._majorfont = wx.Font(fontsize, wx.SWISS, wx.NORMAL, wx.BOLD)

        if wx.Platform == "__WXMAC__":
            self._minorfont.SetNoAntiAliasing(True)
            self._majorfont.SetNoAntiAliasing(True)

        self._bits = []
        self._userbits = []
        self._userbitlen = 0
        self._tickmajor = True
        self._tickminor = True
        self._timeformat = IntFormat
        self._labelmajor = True
        self._labelminor = True
        self._tickpen = wx.Pen(wx.BLACK)
        self._textcolour = wx.BLACK
        self._background = wx.WHITE

        self._valid = False
        self._state = 0

        self._style = style
        self._orientation = orient
        wbound, hbound = self.CheckStyle()

        if orient & wx.VERTICAL:
            self.SetBestSize((28, height))
        else:
            self.SetBestSize((width, 28))

        self.SetBounds(0, 0, wbound, hbound)

        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouseEvents)
        self.Bind(wx.EVT_MOUSE_CAPTURE_LOST, lambda evt: True)


    def OnMouseEvents(self, event):
        """
        Handles the ``wx.EVT_MOUSE_EVENTS`` event for L{RulerCtrl}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """

        if not self._indicators:
            event.Skip()
            return

        mousePos = event.GetPosition()

        if event.LeftDown():
            self.CaptureMouse()
            self.GetIndicator(mousePos)
            self._mousePosition = mousePos
            self.SetIndicatorValue(sendEvent=False)
        elif event.Dragging() and self._currentIndicator:
            self._mousePosition = mousePos
            self.SetIndicatorValue()
        elif event.LeftUp():
            if self.HasCapture():
                self.ReleaseMouse()
            self.SetIndicatorValue(sendEvent=False)
            if self._drawingparent:
                self._drawingparent.Refresh()
            self._currentIndicator = None
        #else:
        #    self._currentIndicator = None

        event.Skip()
        
        
    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{RulerCtrl}.

        :param `event`: a `wx.PaintEvent` event to be processed.        
        """

        dc = wx.BufferedPaintDC(self)
        dc.SetBackground(wx.Brush(self._background))
        dc.Clear()
        self.Draw(dc)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{RulerCtrl}.

        :param `event`: a `wx.SizeEvent` event to be processed.        
        """

        width, height = self.CheckStyle()
        self.SetBounds(0, 0, width, height)

        self.Invalidate()        
        event.Skip()


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{RulerCtrl}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass        


    def SetIndicatorValue(self, sendEvent=True):
        """
        Sets the indicator value.

        :param `sendEvent`: ``True`` to send a L{RulerCtrlEvent}, ``False`` otherwise.
        """

        if self._currentIndicator is None:
            return
        
        left, top, right, bottom = self.GetBounds()
        
        x = self._mousePosition.x 
        y = self._mousePosition.y

        maxvalue = self._max
        minvalue = self._min

        if self._log:
            minvalue = math.log10(minvalue)
            maxvalue = math.log10(maxvalue)
            
        deltarange = abs(self._max - self._min)
        
        if self._orientation == wx.HORIZONTAL:  # only x moves
            value = deltarange*float(x)/(right - left)
        else:
            value = deltarange*float(y)/(bottom - top)

        value += minvalue
        
        if self._log:
            value = 10**value

        if value < self._min or value > self._max:
            return

        self.DrawOnParent(self._currentIndicator)
        
        if sendEvent:
            event = RulerCtrlEvent(wxEVT_INDICATOR_CHANGING, self._currentIndicator.GetId())
            event.SetOldValue(self._currentIndicator.GetValue())
            
            event.SetValue(value)
            event.SetEventObject(self)

            if self.GetEventHandler().ProcessEvent(event):
                self.DrawOnParent(self._currentIndicator)
                return

            self._currentIndicator.SetValue(value)
            
        if sendEvent:
            event.SetEventType(wxEVT_INDICATOR_CHANGED)
            event.SetOldValue(value)
            self.GetEventHandler().ProcessEvent(event)
            self.DrawOnParent(self._currentIndicator)
                
        self.Refresh()
        

    def GetIndicator(self, mousePos):
        """
        Returns the indicator located at the mouse position `mousePos` (if any).

        :param `mousePos`: the mouse position, an instance of `wx.Point`.
        """

        for indicator in self._indicators:
            if indicator.GetRect().Contains(mousePos):
                self._currentIndicator = indicator
                break


    def CheckStyle(self):
        """ Adjust the L{RulerCtrl} style accordingly to borders, units, etc..."""

        width, height = self.GetSize()
        
        if self._orientation & wx.HORIZONTAL:
            if self._style & wx.NO_BORDER:
                hbound = 28
                wbound = width-1
            elif self._style & wx.SIMPLE_BORDER:
                hbound = 27
                wbound = width-1
            elif self._style & wx.STATIC_BORDER:
                hbound = 26
                wbound = width-3
            elif self._style & wx.SUNKEN_BORDER:
                hbound = 24
                wbound = width-5
            elif self._style & wx.RAISED_BORDER:
                hbound = 22
                wbound = width-7
            elif self._style & wx.DOUBLE_BORDER:
                hbound = 22
                wbound = width-7
        else:
            if self._style & wx.NO_BORDER:
                wbound = 28
                hbound = height-1
            elif self._style & wx.SIMPLE_BORDER:
                wbound = 27
                hbound = height-1
            elif self._style & wx.STATIC_BORDER:
                wbound = 26
                hbound = height-3
            elif self._style & wx.SUNKEN_BORDER:
                wbound = 24
                hbound = height-5
            elif self._style & wx.RAISED_BORDER:
                wbound = 22
                hbound = height-7
            elif self._style & wx.DOUBLE_BORDER:
                wbound = 22
                hbound = height-7

        minText = self.LabelString(self._min, major=True)
        maxText = self.LabelString(self._max, major=True)

        dc = wx.ClientDC(self)
        minWidth, minHeight = dc.GetTextExtent(minText)
        maxWidth, maxHeight = dc.GetTextExtent(maxText)

        maxWidth = max(maxWidth, minWidth)
        maxHeight = max(maxHeight, minHeight)
        
        if self._orientation == wx.HORIZONTAL:
            if maxHeight + 4 > hbound:
                hbound = maxHeight
                self.SetBestSize((-1, maxHeight + 4))
                if self.GetContainingSizer():
                    self.GetContainingSizer().Layout()
        else:
            if maxWidth + 4 > wbound:
                wbound = maxWidth
                self.SetBestSize((maxWidth + 4, -1))
                if self.GetContainingSizer():
                    self.GetContainingSizer().Layout()
                
        return wbound, hbound
    

    def TickMajor(self, tick=True):
        """
        Sets whether the major ticks should be ticked or not.

        :param `tick`: ``True`` to show major ticks, ``False`` otherwise.
        """

        if self._tickmajor != tick:
            self._tickmajor = tick
            self.Invalidate()


    def TickMinor(self, tick=True):
        """
        Sets whether the minor ticks should be ticked or not.

        :param `tick`: ``True`` to show minor ticks, ``False`` otherwise.
        """

        if self._tickminor != tick:
            self._tickminor = tick
            self.Invalidate()
        

    def LabelMinor(self, label=True):
        """
        Sets whether the minor ticks should be labeled or not.

        :param `label`: ``True`` to label minor ticks, ``False`` otherwise.
        """

        if self._labelminor != label:
            self._labelminor = label
            self.Invalidate()


    def LabelMajor(self, label=True):
        """
        Sets whether the major ticks should be labeled or not.

        :param `label`: ``True`` to label major ticks, ``False`` otherwise.
        """

        if self._labelmajor != label:
            self._labelmajor = label
            self.Invalidate()


    def GetTimeFormat(self):
        """ Returns the time format. """

        return self._timeformat        


    def SetTimeFormat(self, format=TimeFormat):
        """
        Sets the time format.

        :param `format`: the format used to display time values.
        """

        if self._timeformat != format:
            self._timeformat = format
            self.Invalidate()
            

    def SetFormat(self, format):
        """
        Sets the format for L{RulerCtrl}.

        :param `format`: the format used to display values in L{RulerCtrl}. This can be
         one of the following bits:

         ====================== ======= ==============================
         Format                  Value  Description
         ====================== ======= ==============================
         ``IntFormat``                1 Integer format
         ``RealFormat``               2 Real format
         ``TimeFormat``               3 Time format
         ``LinearDBFormat``           4 Linear DB format
         ``HHMMSS_Format``            5 HHMMSS format
         ====================== ======= ==============================
         """

        if self._format != format:
            self._format = format
            self.Invalidate()
    

    def GetFormat(self):
        """
        Returns the format used to display values in L{RulerCtrl}.

        :see: L{SetFormat} for a list of possible formats.
        """

        return self._format
    

    def SetLog(self, log=True):
        """
        Sets whether L{RulerCtrl} should have a logarithmic scale or not.

        :param `log`: ``True`` to use a logarithmic scake, ``False`` to use a
         linear one.
        """

        if self._log != log:
            self._log = log
            self.Invalidate()


    def SetUnits(self, units):
        """
        Sets the units that should be displayed beside the labels.

        :param `units`: the units that should be displayed beside the labels.
        """

        # Specify the name of the units (like "dB") if you
        # want numbers like "1.6" formatted as "1.6 dB".

        if self._units != units:
            self._units = units             
            self.Invalidate()
    

    def SetBackgroundColour(self, colour):
        """
        Sets the L{RulerCtrl} background colour.

        :param `colour`: an instance of `wx.Colour`.

        :note: Overridden from `wx.PyPanel`.        
        """

        self._background = colour
        wx.PyPanel.SetBackgroundColour(self, colour)
        self.Refresh()

        
    def SetOrientation(self, orient=None):
        """
        Sets the L{RulerCtrl} orientation.

        :param `orient`: can be ``wx.HORIZONTAL`` or ``wx.VERTICAL``.        
        """

        if orient is None:
            orient = wx.HORIZONTAL

        if self._orientation != orient:
            self._orientation = orient
             
            if self._orientation == wx.VERTICAL and not self._hassetspacing:
                self._spacing = 2
             
            self.Invalidate()
    

    def SetRange(self, minVal, maxVal):
        """
        Sets the L{RulerCtrl} range.

        :param `minVal`: the minimum value of L{RulerCtrl};
        :param `maxVal`: the maximum value of L{RulerCtrl}.
        """

        # For a horizontal ruler,
        # minVal is the value in the center of pixel "left",
        # maxVal is the value in the center of pixel "right".

        if self._min != minVal or self._max != maxVal:
            self._min = minVal
            self._max = maxVal
            self.Invalidate()

    
    def SetSpacing(self, spacing):
        """
        Sets the L{RulerCtrl} spacing between labels.

        :param `spacing`: the spacing between labels, in pixels.
        """

        self._hassetspacing = True

        if self._spacing != spacing:
            self._spacing = spacing
            self.Invalidate()
    

    def SetLabelEdges(self, labelEdges=True):
        """
        Sets whether the edge values should always be displayed or not.

        :param `labelEdges`: ``True`` to always display edge labels, ``False`` otherwise/
        """

        # If this is True, the edges of the ruler will always
        # receive a label.  If not, the nearest round number is
        # labeled (which may or may not be the edge).

        if self._labeledges != labelEdges: 
            self._labeledges = labelEdges        
            self.Invalidate()
    

    def SetFlip(self, flip=True):
        """
        Sets whether the orientation of the tick marks should be reversed.

        :param `flip`: ``True`` to reverse the orientation of the tick marks, ``False``
         otherwise.
        """

        # If this is True, the orientation of the tick marks
        # is reversed from the default eg. above the line
        # instead of below

        if self._flip != flip:
            self._flip = flip
            self.Invalidate()
            for indicator in self._indicators:
                indicator.RotateImage()
    

    def SetFonts(self, minorFont, majorFont):
        """
        Sets the fonts for minor and major tick labels.

        :param `minorFont`: the font used to draw minor ticks, a valid `wx.Font` object;
        :param `majorFont`: the font used to draw major ticks, a valid `wx.Font` object.        
        """

        self._minorfont = minorFont
        self._majorfont = majorFont

        if wx.Platform == "__WXMAC__":
            self._minorfont.SetNoAntiAliasing(True)
            self._majorfont.SetNoAntiAliasing(True)

        self.Invalidate()


    def SetTickPenColour(self, colour=wx.BLACK):
        """
        Sets the pen colour to draw the ticks.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._tickpen = wx.Pen(colour)
        self.Refresh()


    def SetLabelColour(self, colour=wx.BLACK):
        """
        Sets the labels colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._textcolour = colour
        self.Refresh()
    

    def OfflimitsPixels(self, start, end):
        """ Used internally. """

        if not self._userbits:
            if self._orientation == wx.HORIZONTAL:
                self._length = self._right-self._left
            else:
                self._length = self._bottom-self._top
                
            self._userbits = [0]*self._length            
            self._userbitlen  = self._length+1
        
        if end < start:
            i = end
            end = start
            start = i
        
        if start < 0:
            start = 0
        if end > self._length:
            end = self._length

        for ii in xrange(start, end+1):
            self._userbits[ii] = 1


    def SetBounds(self, left, top, right, bottom):
        """
        Sets the bounds for L{RulerCtrl} (its client rectangle).

        :param `left`: the left corner of the client rectangle;
        :param `top`: the top corner of the client rectangle;
        :param `right`: the right corner of the client rectangle;
        :param `bottom`: the bottom corner of the client rectangle.
        """

        if self._left != left or self._top != top or self._right != right or \
           self._bottom != bottom:
            
            self._left = left
            self._top = top
            self._right = right
            self._bottom = bottom

            self.Invalidate()


    def GetBounds(self):
        """ Returns the L{RulerCtrl} bounds (its client rectangle). """

        return self._left, self._top, self._right, self._bottom        


    def AddIndicator(self, id, value):
        """
        Adds an indicator to L{RulerCtrl}. You should pass a unique `id` and a starting
        `value` for the indicator.

        :param `id`: the indicator identifier;
        :param `value`: the indicator initial value.
        """

        self._indicators.append(Indicator(self, id, value))
        self.Refresh()
        

    def SetIndicatorColour(self, id, colour=None):
        """
        Sets the indicator colour.

        :param `id`: the indicator identifier;
        :param `colour`: a valid `wx.Colour` object.
        
        :note: This method requires PIL to change the image palette.
        """

        if not _hasPIL:
            return
        
        if colour is None:
            colour = wx.WHITE

        for indicator in self._indicators:
            if indicator.GetId() == id:
                indicator.SetColour(colour)
                break

        
    def Invalidate(self):
        """ Invalidates the ticks calculations. """

        self._valid = False

        if self._orientation == wx.HORIZONTAL:
            self._length = self._right - self._left
        else:
            self._length = self._bottom - self._top

        self._majorlabels = []
        self._minorlabels = []
        self._bits = []
        self._userbits = []
        self._userbitlen = 0
        self.Refresh()
        

    def FindLinearTickSizes(self, UPP):
        """ Used internally. """

        # Given the dimensions of the ruler, the range of values it
        # has to display, and the format (i.e. Int, Real, Time),
        # figure out how many units are in one Minor tick, and
        # in one Major tick.
        #
        # The goal is to always put tick marks on nice round numbers
        # that are easy for humans to grok.  This is the most tricky
        # with time.

        # As a heuristic, we want at least 16 pixels
        # between each minor tick
        units = 16.0*abs(UPP)

        self._digits = 0

        if self._format == LinearDBFormat:
            
            if units < 0.1:
                self._minor = 0.1
                self._major = 0.5
                return
            
            if units < 1.0:
                self._minor = 1.0
                self._major = 6.0
                return
            
            self._minor = 3.0
            self._major = 12.0
            return

        elif self._format == IntFormat:
            
            d = 1.0
            while 1:
                if units < d:
                    self._minor = d
                    self._major = d*5.0
                    return
                
                d = d*5.0
                if units < d:
                    self._minor = d
                    self._major = d*2.0
                    return
                
                d = 2.0*d
            
        elif self._format == TimeFormat:

            if units > 0.5:
                if units < 1.0:  # 1 sec
                    self._minor = 1.0
                    self._major = 5.0
                    return
                
                if units < 5.0:  # 5 sec
                    self._minor = 5.0
                    self._major = 15.0
                    return
                
                if units < 10.0:
                    self._minor = 10.0
                    self._major = 30.0
                    return
                
                if units < 15.0:
                    self._minor = 15.0
                    self._major = 60.0
                    return
                
                if units < 30.0: 
                    self._minor = 30.0
                    self._major = 60.0
                    return
                
                if units < 60.0:  # 1 min
                    self._minor = 60.0
                    self._major = 300.0
                    return
                
                if units < 300.0:  # 5 min
                    self._minor = 300.0
                    self._major = 900.0
                    return
                
                if units < 600.0:  # 10 min
                    self._minor = 600.0
                    self._major = 1800.0
                    return
                
                if units < 900.0:  # 15 min
                    self._minor = 900.0
                    self._major = 3600.0
                    return
                
                if units < 1800.0:  # 30 min
                    self._minor = 1800.0
                    self._major = 3600.0
                    return
                
                if units < 3600.0:  # 1 hr
                    self._minor = 3600.0
                    self._major = 6*3600.0
                    return
                
                if units < 6*3600.0:  # 6 hrs
                    self._minor = 6*3600.0
                    self._major = 24*3600.0
                    return
                
                if units < 24*3600.0:  # 1 day
                    self._minor = 24*3600.0
                    self._major = 7*24*3600.0
                    return
                
                self._minor = 24.0*7.0*3600.0 # 1 week
                self._major = 24.0*7.0*3600.0

        # Otherwise fall through to RealFormat
        # (fractions of a second should be dealt with
        # the same way as for RealFormat)

        elif self._format == RealFormat:
            
            d = 0.000001
            self._digits = 6
            
            while 1:
                if units < d:
                    self._minor = d
                    self._major = d*5.0
                    return
                
                d = d*5.0
                if units < d:
                    self._minor = d
                    self._major = d*2.0
                    return
                
                d = d*2.0
                self._digits = self._digits - 1
            

    def LabelString(self, d, major=None):
        """ Used internally. """

        # Given a value, turn it into a string according
        # to the current ruler format.  The number of digits of
        # accuracy depends on the resolution of the ruler,
        # i.e. how far zoomed in or out you are.

        s = ""
        
        if d < 0.0 and d + self._minor > 0.0:
            d = 0.0

        if self._format == IntFormat:
            s = "%d"%int(math.floor(d+0.5))
            
        elif self._format == LinearDBFormat:
            if self._minor >= 1.0:
                s = "%d"%int(math.floor(d+0.5))
            else:
                s = "%0.1f"%d

        elif self._format == RealFormat:
            if self._minor >= 1.0:
                s = "%d"%int(math.floor(d+0.5))
            else:
                s = (("%." + str(self._digits) + "f")%d).strip()
            
        elif self._format == TimeFormat:
            if major:
                if d < 0:
                    s = "-"
                    d = -d

                if self.GetTimeFormat() == HHMMSS_Format:
                    
                    secs = int(d + 0.5)
                    if self._minor >= 1.0:
                        s = ("%d:%02d:%02d")%(secs/3600, (secs/60)%60, secs%60)
                    
                    else:
                        t1 = ("%d:%02d:")%(secs/3600, (secs/60)%60)
                        format = "%" + "%0d.%dlf"%(self._digits+3, self._digits)
                        t2 = format%(d%60.0)
                        s = s + t1 + t2
                        
                else:
                    
                    if self._minor >= 3600.0:
                        hrs = int(d/3600.0 + 0.5)
                        h = "%d:00:00"%hrs
                        s = s + h
                    
                    elif self._minor >= 60.0:
                        minutes = int(d/60.0 + 0.5)
                        if minutes >= 60:
                            m = "%d:%02d:00"%(minutes/60, minutes%60)
                        else:
                            m = "%d:00"%minutes
                            
                        s = s + m
                    
                    elif self._minor >= 1.0:
                        secs = int(d + 0.5)
                        if secs >= 3600:
                            t = "%d:%02d:%02d"%(secs/3600, (secs/60)%60, secs%60)
                        elif secs >= 60:
                            t = "%d:%02d"%(secs/60, secs%60)
                        else:
                            t = "%d"%secs
                            
                        s = s + t
                    
                    else:
                        secs = int(d)
                        if secs >= 3600:
                            t1 = "%d:%02d:"%(secs/3600, (secs/60)%60)
                        elif secs >= 60:
                            t1 = "%d:"%(secs/60)

                        if secs >= 60:
                            format = "%%0%d.%dlf"%(self._digits+3, self._digits)
                        else:
                            format = "%%%d.%dlf"%(self._digits+3, self._digits)
                            
                        t2 = format%(d%60.0)

                        s = s + t1 + t2
   
        if self._units != "":
            s = s + " " + self._units

        return s


    def Tick(self, dc, pos, d, major):
        """
        Ticks a particular position.

        :param `dc`: an instance of `wx.DC`;
        :param `pos`: the label position;
        :param `d`: the current label value;
        :param `major`: ``True`` if it is a major ticks, ``False`` if it is a minor one.
        """

        if major:
            label = Label()
            self._majorlabels.append(label)
        else:
            label = Label()
            self._minorlabels.append(label)

        label.pos = pos
        label.lx = self._left - 2000 # don't display
        label.ly = self._top - 2000  # don't display
        label.text = ""

        dc.SetFont((major and [self._majorfont] or [self._minorfont])[0])

        l = self.LabelString(d, major)
        strw, strh = dc.GetTextExtent(l)
        
        if self._orientation == wx.HORIZONTAL: 
            strlen = strw
            strpos = pos - strw/2
            if strpos < 0:
                strpos = 0
            if strpos + strw >= self._length:
                strpos = self._length - strw
            strleft = self._left + strpos
            if self._flip:
                strtop = self._top + 4
                self._maxheight = max(self._maxheight, 4 + strh)
            else:
                strtop = self._bottom - strh - 6
                self._maxheight = max(self._maxheight, strh + 6)
            
        else: 
            strlen = strh
            strpos = pos - strh/2
            if strpos < 0:
                strpos = 0
            if strpos + strh >= self._length:
                strpos = self._length - strh
            strtop = self._top + strpos
            if self._flip:
                strleft = self._left + 5
                self._maxwidth = max(self._maxwidth, 5 + strw)
            else:
                strleft = self._right - strw - 6
                self._maxwidth = max(self._maxwidth, strw + 6)

        # See if any of the pixels we need to draw this
        # label is already covered

        if major and self._labelmajor or not major and self._labelminor:
            for ii in xrange(strlen):
                if self._bits[strpos+ii]:
                    return

        # If not, position the label and give it text

        label.lx = strleft
        label.ly = strtop
        label.text = l

        if major:
            if self._tickmajor and not self._labelmajor:
                label.text = ""
            self._majorlabels[-1] = label
            
        else:
            if self._tickminor and not self._labelminor:
                label.text = ""
            label.text = label.text.replace(self._units, "")
            self._minorlabels[-1] = label
            
        # And mark these pixels, plus some surrounding
        # ones (the spacing between labels), as covered

        if (not major and self._labelminor) or (major and self._labelmajor):
            
            leftmargin = self._spacing
            
            if strpos < leftmargin:
                leftmargin = strpos
                
            strpos = strpos - leftmargin
            strlen = strlen + leftmargin

            rightmargin = self._spacing
            
            if strpos + strlen > self._length - self._spacing:
                rightmargin = self._length - strpos - strlen
                
            strlen = strlen + rightmargin

            for ii in xrange(strlen):
                self._bits[strpos+ii] = 1


    def Update(self, dc):
        """
        Updates all the ticks calculations.

        :param `dc`: an instance of `wx.DC`.
        """

        # This gets called when something has been changed
        # (i.e. we've been invalidated).  Recompute all
        # tick positions.

        if self._orientation == wx.HORIZONTAL:
            self._maxwidth = self._length
            self._maxheight = 0
        else:
            self._maxwidth = 0
            self._maxheight = self._length

        self._bits = [0]*(self._length+1)
        self._middlepos = []

        if self._userbits:
            for ii in xrange(self._length):
                self._bits[ii] = self._userbits[ii]
        else:
            for ii in xrange(self._length):
                self._bits[ii] = 0

        if not self._log:

            UPP = (self._max - self._min)/float(self._length)  # Units per pixel

            self.FindLinearTickSizes(UPP)
            
            # Left and Right Edges
            if self._labeledges:
                self.Tick(dc, 0, self._min, True)
                self.Tick(dc, self._length, self._max, True)
            
            # Zero (if it's in the middle somewhere)
            if self._min*self._max < 0.0: 
                mid = int(self._length*(self._min/(self._min-self._max)) + 0.5)
                self.Tick(dc, mid, 0.0, True)
            
            sg = ((UPP > 0.0) and [1.0] or [-1.0])[0]
            
            # Major ticks
            d = self._min - UPP/2
            lastd = d
            majorint = int(math.floor(sg*d/self._major))
            ii = -1
            
            while ii <= self._length:
                ii = ii + 1
                lastd = d
                d = d + UPP
                
                if int(math.floor(sg*d/self._major)) > majorint:
                    majorint = int(math.floor(sg*d/self._major))
                    self.Tick(dc, ii, sg*majorint*self._major, True)
                
            # Minor ticks
            d = self._min - UPP/2
            lastd = d
            minorint = int(math.floor(sg*d/self._minor))
            ii = -1

            while ii <= self._length:
                ii = ii + 1
                lastd = d
                d = d + UPP
                
                if int(math.floor(sg*d/self._minor)) > minorint:
                    minorint = int(math.floor(sg*d/self._minor))
                    self.Tick(dc, ii, sg*minorint*self._minor, False)
            
            # Left and Right Edges
            if self._labeledges:
                self.Tick(dc, 0, self._min, True)
                self.Tick(dc, self._length, self._max, True)
            
        else: 
            # log case
            
            lolog = math.log10(self._min)
            hilog = math.log10(self._max)
            scale = self._length/(hilog - lolog)
            lodecade = int(math.floor(lolog))
            hidecade = int(math.ceil(hilog))

            # Left and Right Edges
            if self._labeledges:
                self.Tick(dc, 0, self._min, True)
                self.Tick(dc, self._length, self._max, True)
                
            startdecade = 10.0**lodecade
            
            # Major ticks are the decades
            decade = startdecade
            for ii in xrange(lodecade, hidecade):
                if ii != lodecade:
                    val = decade
                    if val > self._min and val < self._max:
                        pos = int(((math.log10(val) - lolog)*scale)+0.5)
                        self.Tick(dc, pos, val, True)
                    
                decade = decade*10.0
            
            # Minor ticks are multiples of decades
            decade = startdecade

            for ii in xrange(lodecade, hidecade):
                for jj in xrange(2, 10):
                    val = decade*jj
                    if val >= self._min and val < self._max:
                        pos = int(((math.log10(val) - lolog)*scale)+0.5)
                        self.Tick(dc, pos, val, False)
                    
                decade = decade*10.0
            
        self._valid = True


    def Draw(self, dc):
        """
        Actually draws the whole L{RulerCtrl}.

        :param `dc`: an instance of `wx.DC`.
        """

        if not self._valid:
            self.Update(dc)

        dc.SetBrush(wx.Brush(self._background))
        dc.SetPen(self._tickpen)
        dc.SetTextForeground(self._textcolour)

        dc.DrawRectangleRect(self.GetClientRect())        

        if self._orientation == wx.HORIZONTAL:
            if self._flip:
                dc.DrawLine(self._left, self._top, self._right+1, self._top)
            else:
                dc.DrawLine(self._left, self._bottom-1, self._right+1, self._bottom-1)
        
        else:
            if self._flip:
                dc.DrawLine(self._left, self._top, self._left, self._bottom+1)
            else:
                dc.DrawLine(self._right-1, self._top, self._right-1, self._bottom+1)

        dc.SetFont(self._majorfont)

        for label in self._majorlabels:
            pos = label.pos
            
            if self._orientation == wx.HORIZONTAL:
                if self._flip:
                    dc.DrawLine(self._left + pos, self._top,
                                self._left + pos, self._top + 5)
                else:
                    dc.DrawLine(self._left + pos, self._bottom - 5,
                                self._left + pos, self._bottom)
            
            else:
                if self._flip:
                    dc.DrawLine(self._left, self._top + pos,
                                self._left + 5, self._top + pos)
                else:
                    dc.DrawLine(self._right - 5, self._top + pos,
                                self._right, self._top + pos)
            
            if label.text != "":
                dc.DrawText(label.text, label.lx, label.ly)
        
        dc.SetFont(self._minorfont)

        for label in self._minorlabels:
            pos = label.pos

            if self._orientation == wx.HORIZONTAL:
                if self._flip:
                    dc.DrawLine(self._left + pos, self._top,
                                self._left + pos, self._top + 3)
                else:
                    dc.DrawLine(self._left + pos, self._bottom - 3,
                                self._left + pos, self._bottom)
            
            else:
                if self._flip:
                    dc.DrawLine(self._left, self._top + pos,
                                self._left + 3, self._top + pos)
                else:
                    dc.DrawLine(self._right - 3, self._top + pos,
                                self._right, self._top + pos)
            
            if label.text != "":
                dc.DrawText(label.text, label.lx, label.ly)

        for indicator in self._indicators:
            indicator.Draw(dc)
            

    def SetDrawingParent(self, dparent):
        """
        Sets the window to which L{RulerCtrl} draws a thin line over.

        :param `dparent`: an instance of `wx.Window`, representing the window to
         which L{RulerCtrl} draws a thin line over.
        """

        self._drawingparent = dparent


    def GetDrawingParent(self):
        """ Returns the window to which L{RulerCtrl} draws a thin line over. """

        return self._drawingparent


    def DrawOnParent(self, indicator):
        """
        Actually draws the thin line over the drawing parent window.

        :param `indicator`: the current indicator, an instance of L{Indicator}.

        :note: This method is currently not available on wxMac as it uses `wx.ScreenDC`.        
        """

        if not self._drawingparent:
            return

        xpos, ypos = indicator.GetPosition()
        parentrect = self._drawingparent.GetClientRect()

        dc = wx.ScreenDC()        
        dc.SetLogicalFunction(wx.INVERT)
        dc.SetPen(self._drawingpen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        imgx, imgy = indicator.GetImageSize()
        
        if self._orientation == wx.HORIZONTAL:

            x1 = xpos+ imgx/2
            y1 = parentrect.y
            x2 = x1
            y2 = parentrect.height
            x1, y1 = self._drawingparent.ClientToScreenXY(x1, y1)
            x2, y2 = self._drawingparent.ClientToScreenXY(x2, y2)
                                         
        elif self._orientation == wx.VERTICAL:

            x1 = parentrect.x
            y1 = ypos + imgy/2
            x2 = parentrect.width
            y2 = y1

            x1, y1 = self._drawingparent.ClientToScreenXY(x1, y1)
            x2, y2 = self._drawingparent.ClientToScreenXY(x2, y2)

        dc.DrawLine(x1, y1, x2, y2)
        dc.SetLogicalFunction(wx.COPY)

    
