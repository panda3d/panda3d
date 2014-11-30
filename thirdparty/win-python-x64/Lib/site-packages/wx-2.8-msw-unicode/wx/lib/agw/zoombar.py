"""
ZoomBar is a class that *appoximatively* mimics the behaviour of the Mac Dock,
inside a `wx.Panel`.


Description
===========

ZoomBar is a class that *appoximatively* mimics the behaviour of the Mac Dock,
inside a `wx.Panel`.

Once you hover mouse over the ZoomBar correct button will bubble up, grow to
predefined size, so you can easily see the button you are about to click and
have larger area to click. Difference this makes is amazing.

The buttons are able to be zoomed from the bottom up (bottom fixed) as well
as to zoom from a central point. Also the container is able to move the
remaining buttons around the zoomed button.


Example Usage
=============

Sample usage for ZoomBar::

    bar = ZB.ZoomBar(self, -1)

    normalImages = glob.glob(bitmapDir + "/*96.png")
    reflectionImages = glob.glob(bitmapDir + "/*96Flip40.png")
        
    for std, ref in zip(normalImages, reflectionImages):
    
        bname = os.path.split(std)[1][0:-6]

        normalBmp = wx.Bitmap(std, wx.BITMAP_TYPE_PNG)
        reflectionBmp = wx.Bitmap(ref, wx.BITMAP_TYPE_PNG)
        bar.AddButton(normalBmp, reflectionBmp, bname)

    bar.ResetSize()


Supported Platforms
===================

ZoomBar has been tested on the following platforms:
  * Windows (Windows XP).


Window Styles
=============

`No particular window styles are available for this class.`


Events Processing
=================

This class processes the following events:

============================== ==================================================
Event Name                     Description
============================== ==================================================
``EVT_ZOOMBAR``                Process a `wxEVT_ZOOMBAR` event, when a `ZoomBar` button is clicked.
============================== ==================================================


License And Version
===================

ZoomBar is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 03 Dec 2009, 09.00 GMT

Version 0.1

"""

import wx
import sys

from wx.lib.embeddedimage import PyEmbeddedImage

zoombackgrey = PyEmbeddedImage(
    "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"
    "kIck2UJrAAAAAElFTkSuQmCC")


# ----------------------------------------------------------------------------
# ZoomBar events and binding for handling them
# ----------------------------------------------------------------------------

wxEVT_ZOOMBAR = wx.NewEventType()
EVT_ZOOMBAR = wx.PyEventBinder(wxEVT_ZOOMBAR, 1)
""" Process a `wxEVT_ZOOMBAR` event, when a `ZoomBar` button is clicked. """

# ----------------------------------------------------------------------------

def MakeDisabledBitmap(original):
    """
    Creates a disabled-looking bitmap starting from the input one.

    :param `original`: an instance of `wx.Bitmap` to be greyed-out.
    """
    
    img = original.ConvertToImage()
    return wx.BitmapFromImage(img.ConvertToGreyscale())

# ----------------------------------------------------------------------------

class ZoomBarImage(object):
    """
    This simple class holds information about a L{ZoomBar} button, such as normal
    bitmaps, disabled bitmap, button label, etc...
    """
    
    def __init__(self, parent, bitmap, disabledBmp=wx.NullBitmap, label=""):
        """
        Default class constructor.

        :param `parent`: the main L{ZoomBar} window;
        :param `bitmap`: the button bitmap, an instance of `wx.Bitmap`;
        :param `disabledBmp`: the button bitmap when the button is in a disabled
         state;
        :param `label`: the button label.
        """

        self._parent = parent
        self._bitmap = bitmap

        if not disabledBmp.IsOk():
            disabledBmp = MakeDisabledBitmap(bitmap)

        self._disabledBmp = disabledBmp
        self._label = label
        
        self._height = 36
        self._width = 36

        self._oldnx = 0
        self._oldLeft = 0
        self._oldTop = 0
        self._oldWidth = 0
        self._oldBottom = 0
        self._oldHeight = 0
        self._vCenter = 0
        self._hCenter = 0
        self._oldInc = -sys.maxint
        self._isSeparator = False
        self._enabled = True
        
        # Larger number gives a greater zoom effect
        self._zoomFactor = 3
        
        # Whether this is a reflection or not
        self._isAReflection = False

        self._cachedBitmaps = {}
        self._cachedDisabledBitmaps = {}


    def SetZoomFactor(self, zoom):
        """
        Sets the zoom factor for the button. Larger number gives a greater zoom
        effect.

        :param `zoom`: a floating point number, greater than or equal to 1.0.
        """

        self._zoomFactor = zoom


    def SetCenterZoom(self, center=True):
        """
        Sets to zoom from the center.

        :param `center`: if ``True`` button zooms upwards.
        """
        
        self._centerZoom = center
        

    def ZoomImage(self, nxcoord):
        """
        Zooms the button bitmap depending on the mouse x position.

        :param `nxcoord`: the mouse x position relative to the button center.
        """

        if nxcoord < self._vCenter:
            inc = int(((nxcoord - self._oldLeft)*10.0)/(self._oldWidth/2.0)) + 1
        else:
            inc = int(((self._oldLeft + self._oldWidth - nxcoord)*10.0)/(self._oldWidth/2)) + 1

        if self._isSeparator:
            return False
        
        if abs(inc - self._oldInc) < 2:
            return False

        self._oldInc = inc
        
        if not self._isAReflection:
            # original button
            inc = inc*self._zoomFactor
            self._width = self._oldWidth + inc
            self._left = self._vCenter - self._width/2
            self._height = self._oldHeight + inc
            self._top = self._oldBottom - self._height

        else:
            # the reflection
            self._height = self._oldHeight + inc
            self._width = self._oldWidth + inc
            self._top = self._top + self._height

        self._oldnx = nxcoord
        return True


    def SetSize(self, width, height):
        """
        Sets the button size.

        :param `width`: the button width;
        :param `height`: the button height.
        """

        self._width = width
        self._height = height
        

    def GetPosition(self):
        """ Returns the button position. """

        return wx.Point(self._left, self._top)
    

    def GetSize(self):
        """ Returns the button size. """

        return wx.Size(self._width, self._height)
    

    def GetBitmap(self):
        """
        Returns the button bitmap, which may be a scaled up version of the original
        bitmap is the button is being zoomed.
        """

        if self._isAReflection:
            return self._paintBitmap

        if self._enabled:
            return self._cachedBitmaps[self._width]

        return self._cachedDisabledBitmaps[self._width]        
    
        
    def SetupProps(self, buttonSize):
        """
        Set up the button position and size.

        :param `buttonSize`: the button original size (not zoomed), in pixels.
        """

        self._width = self._oldWidth = buttonSize
        self._height = self._oldHeight = buttonSize
        
        self._oldLeft = self._left
        self._oldTop = self._top
        self._vCenter = self._oldLeft + int(self._oldWidth/2.0)
        self._hCenter = self._oldTop + int(self._oldHeight/2.0)
        self._oldBottom = self._top + self._height

        if self._isAReflection:
            img = self._bitmap.ConvertToImage()
            img = img.Scale(self._width, self._height, wx.IMAGE_QUALITY_HIGH)
            self._paintBitmap = img.ConvertToBitmap()
        

    def GetLabel(self):
        """ Returns the button label (if any). """

        return self._label


    def SetLabel(self, label):
        """
        Sets the button label.

        :param `label`: a string specifying the button label. May be an empty string
         for no label.
        """

        self._label = label


    def IsZoomed(self):
        """ Returns ``True`` if the button is zoomed, ``False`` otherwise. """

        return self._width/float(self._oldWidth) >= 1.25
    

    def LoopScales(self, size):
        """
        Caches the bitmaps at various zoom levels to avoid calling every time
        `image.Scale` on the button bitmap.

        :param `size`: the original button size, in pixels.
        """

        if self._isAReflection:
            return
        
        for scale in range(size-10*self._zoomFactor, size+15*self._zoomFactor, self._zoomFactor):
            if scale in self._cachedBitmaps or scale <= 0:
                continue

            img = self._bitmap.ConvertToImage()
            img = img.Scale(scale, scale, wx.IMAGE_QUALITY_HIGH)
            bmp = img.ConvertToBitmap()
            self._cachedBitmaps[scale] = bmp

            img = self._disabledBmp.ConvertToImage()
            img = img.Scale(scale, scale, wx.IMAGE_QUALITY_HIGH)
            bmp = img.ConvertToBitmap()
            self._cachedDisabledBitmaps[scale] = bmp            


    def Enable(self, enable=True):
        """
        Enables/disables a button.

        :param `enable`: ``True`` to enable a button, ``False`` to disable it.
        """

        self._enabled = enable        


    def IsEnabled(self):
        """ Returns ``True`` if the button is enabled, ``False`` otherwise. """

        return self._enabled
    

class ImageBar(object):
    """ This class holds the background button bar on which the buttons float. """

    def __init__(self, bitmap=None):
        """
        Default class constructor.

        :param `bitmap`: if not ``None``, the bitmap to use as a background button
         bar on which the buttons float. It should be an instance of `wx.Image`.
        """

        if bitmap and bitmap.IsOk():
            self._bitmap = bitmap
            self._hasBitmap = bitmap
        else:
            self._bitmap = zoombackgrey.GetImage()
            self._hasBitmap = None
            
        self._left = 23
        self._top = 53
        self._startColour = wx.Colour(97, 97, 97)
        self._endColour = wx.Colour(97, 97, 97)


    def GetPosition(self):
        """ Returns the position of L{ImageBar}, as a `wx.Point`. """

        return wx.Point(self._left, self._top)


    def GetSize(self):
        """ Returns the size of L{ImageBar}, as a `wx.Size`. """

        return wx.Size(self._bitmap.GetWidth(), self._bitmap.GetHeight())
    

    def GetBitmap(self):
        """ Returns the background button bar on which the buttons float. """

        return self._bitmap


    def SetPosition(self, xpos, ypos):
        """
        Sets the position of L{ImageBar}.

        :param `xpos`: the `x` position of the bar;
        :param `ypos`: the `y` position of the bar.
        """

        self._left = xpos
        self._top = ypos
        

    def SetSize(self, xSize, ySize):
        """
        Sets the size of L{ImageBar}.

        :param `xSize`: the width of the bar, in pixels;
        :param `ySize`: the height of the bar, in pixels.
        """

        self.SetBarColour(self._startColour, xSize, ySize)
        

    def SetBarColour(self, colour, xSize=None, ySize=None):
        """
        Sets the background button bar colour.

        :param `colour`: an instance of `wx.Colour`;
        :param `xSize`: if not ``None``, the new L{ImageBar} width;
        :param `ySize`: if not ``None``, the new L{ImageBar} height.
        """        

        if not isinstance(colour, wx.Colour):
            colour = wx.NamedColour(colour)

        if self._hasBitmap:
            bitmap = self._hasBitmap
        else:
            bitmap = zoombackgrey.GetImage()
            
        if xSize is not None:
            self._size = wx.Size(xSize, ySize)
            
        bitmap.Rescale(self._size.width, self._size.height/2)

        r1, g1, b1 = self._startColour.Red(), self._startColour.Green(), self._startColour.Blue()
        r2, g2, b2 = colour.Red(), colour.Green(), colour.Blue()

        fr = (r1 > 0 and [float(r2)/r1] or [0])[0]
        fg = (g1 > 0 and [float(g2)/g1] or [0])[0]
        fb = (b1 > 0 and [float(b2)/b1] or [0])[0]

        bitmap = bitmap.AdjustChannels(fr, fg, fb, 1)
        self._bitmap = bitmap.ConvertToBitmap()
        self._endColour = colour
        

    def GetBarColour(self):
        """ Returns the background button bar colour. """

        return self._endColour
    

class ZoomBarEvent(wx.PyCommandEvent):
    """ Event sent from the L{ZoomBar} when a button is activated. """

    def __init__(self, eventType, eventId=1):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self._eventType = eventType


    def SetSelection(self, selection):
        """
        Sets the index of the selected button.

        :param `selection`: an integer indicating the current selected button.
        """

        self._selection = selection


    def GetSelection(self):
        """ Returns the index of the selected button. """

        return self._selection


    def GetLabel(self):
        """ Returns the text label of the selected button. """

        return self._label


    def SetLabel(self, label):
        """
        Sets the text label of the selected button.

        :param `label`: the text label of the selected button.
        """

        self._label = label


class ZoomBar(wx.PyControl):
    """
    ZoomBar is a class that *appoximatively* mimics the behaviour of the Mac Dock,
    inside a `wx.Panel`.

    This is the main class implementation.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 name="ZoomBar"):
        """
        Default class constructor.

        :param `parent`: the L{ZoomBar} parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `name`: the window name.
        """

        wx.PyControl.__init__(self, parent, id, pos, size, style=wx.BORDER_THEME)

        # Zoom from the center. If True button zooms upwards.
        self._centerZoom = False
        # Whether you want reflections or not
        self._showReflections = True
        # Allows us to nudge a reflection closer to original
        self._nudgeReflection = 0
        # Extension of the reflection. BMP or PNG etc.
        # Initial size of the buttons
        self._buttonSize = 48
        # Show labels on hovering
        self._showLabels = True
        # used internally
        self._noResize = False
        self._buttons = []
        self._reflectionButtons = []

        self._imgBar = ImageBar()        
        self._previousHit = -1
        self._currentHit = -1

        wx.CallLater(200, self.OnLeaveWindow, None)

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)

        if wx.Platform == "__WXMSW__":
            self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDown)
            
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        

    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same size
        as it would have after a call to `Fit()`.
        """

        xSize = self._buttonSize*len(self._buttons) + len(self._buttons) + self._buttonSize
        ySize = self._buttonSize*2
        if self._showLabels:
            dc = wx.ClientDC(self)
            dummy, yextent = dc.GetTextExtent("Ajgt")
            ySize += yextent
        
        return wx.Size(xSize+50, ySize+20)
    

    # reposition the buttons
    def Reposition(self, toButton):
        """
        Repositions all the buttons inside the L{ZoomBar}.

        :param `toButton`: the button currently hovered by the mouse (and hence
         zoomed).
        """

        nLeft = toButton._left
        nRight = toButton._left + toButton._width + 1
        nButton = self._buttons.index(toButton)
        
        # do any buttons on the right 
        for n in xrange(nButton + 1, len(self._buttons)):
            oButton = self._buttons[n]
            oButton._left = nRight

            if self._showReflections:
                oButtonR = self._reflectionButtons[n]
                oButtonR._left = nRight

            nRight = nRight + oButton._width + 1

        # Reset
        nLeft = toButton._left

        # now to the left
        if nButton > 0:
            # only for 2nd and more
            for n in xrange(nButton-1, -1, -1):
                oButton = self._buttons[n]
                oButton._left = nLeft - (oButton._width + 1)
                if self._showReflections:
                    oButtonR = self._reflectionButtons[n]
                    oButtonR._left = oButton._left

                nLeft = oButton._left


    # method to add required buttons
    def AddButton(self, normalBmp, reflectionBmp=wx.NullBitmap, label="", disabledBmp=wx.NullBitmap, 
                  disabledReflectionBmp=wx.NullBitmap):
        """
        Adds a button to L{ZoomBar}.

        :param `normalBmp`: the button main bitmap, an instance of `wx.Bitmap`;
        :param `reflectionBmp`: a bitmap representing a reflection of the main bitmap,
         an instance of `wx.Bitmap`;
        :param `label`: the button label;
        :param `disabledBmp`: the button main bitmap when the button is in a disabled
         state, an instance of `wx.Bitmap`;
        :param `disabledReflectionBmp`: a bitmap representing a reflection of the main bitmap,
         when the button is in a disabled state, an instance of `wx.Bitmap`.
        """

        button = ZoomBarImage(self, normalBmp, disabledBmp, label)

        button.SetSize(self._buttonSize, self._buttonSize)
        button._centerZoom = (self._showReflections and [False] or [self._centerZoom])[0]
        self._buttons.append(button)

        self.InitialReposition()

        if self._showReflections and reflectionBmp.IsOk():

            rbutton = ZoomBarImage(self, reflectionBmp, disabledReflectionBmp)
            rbutton.SetSize(self._buttonSize, self._buttonSize)        
            rbutton._centerzoom = False
            rbutton._isAReflection = True
            self._reflectionButtons.append(rbutton)

        return button
    

    def AddSeparator(self, normalBmp, reflectionBmp=wx.NullBitmap):
        """
        Adds a separator to L{ZoomBar}.

        :param `normalBmp`: the separator main bitmap, an instance of `wx.Bitmap`;
        :param `reflectionBmp`: a bitmap representing a reflection of the main bitmap,
         an instance of `wx.Bitmap`.
        """

        button = self.AddButton(normalBmp, reflectionBmp)
        button._isSeparator = True


    def SetZoomFactor(self, zoom):
        """
        Sets the zoom factor for all the buttons. Larger number gives a greater zoom
        effect.

        :param `zoom`: a floating point number, greater than or equal to 1.0.
        """

        if zoom < 1:
            raise Exception("The zoom factor must be greater or equal to 1")

        for button in self._buttons:
            button._zoomFactor = zoom

        self._zoomFactor = zoom
        self.DoLayout()


    def GetZoomFactor(self):
        """ Returns the current zoom factor. """

        return self._zoomFactor
    

    def SetCenterZoom(self, center=True):
        """
        Sets to zoom from the center.

        :param `center`: if ``True`` button zooms upwards.
        """

        self._centerZoom = center

        for button in self._buttons:
            button._centerZoom = (self._showReflections and [False] or [self._centerZoom])[0]

        self.DoLayout()
        

    def GetCenterZoom(self):
        """ Returns ``True`` if buttons zoom upwards. """

        return self._centerZoom
    

    def SetShowReflections(self, show):
        """
        Sets whether to show reflections or not.

        :param `show`: ``True`` to show reflections, ``False`` otherwise.
        """

        self._showReflections = show
        self.DoLayout()
        

    def GetShowReflections(self):
        """ Returns ``True`` if reflections bitmap are currently shown. """

        return self._showReflections
    

    def SetShowLabels(self, show):
        """
        Sets whether to show button labels or not.

        :param `show`: ``True`` to show button labels, ``False`` otherwise.
        """

        self._showLabels = show
        self.DoLayout()
        

    def GetShowLabels(self):
        """ Returns ``True`` if button labels are currently shown. """
        
        return self._showLabels


    def SetBarColour(self, colour):
        """
        Sets the background button bar colour.

        :param `colour`: an instance of `wx.Colour`;
        """        

        self._imgBar.SetBarColour(colour)
        self.Refresh()


    def GetBarColour(self):
        """ Returns the background button bar colour. """

        return self._imgBar.GetBarColour()


    def SetButtonSize(self, size):
        """
        Sets the original button size.

        :param `size`: the new (not-zoomed) button size, in pixels.
        """
        
        self._buttonSize = size
        self.DoLayout()


    def GetButtonSize(self):
        """ Returns the original (not zoomed) button size, in pixels. """

        return self._buttonSize


    def EnableButton(self, index, enable=True):
        """
        Enables/disables the button at position `index`.

        :param `index`: the index of the button to enable/disable;
        :param `enable`: ``True`` to enable the button, ``False`` to disable it.
        """

        if index < 0 or index >= len(self._buttons):
            return False

        self._buttons[index].Enable(enable)
        self.Refresh()

        return True
    

    def IsButtonEnabled(self, index):
        """
        Returns ``True`` if the button at position `index` is enabled, ``False``
        otherwise.

        :param `index`: the index of the button to check.
        """

        if index < 0 or index >= len(self._buttons):
            return False

        return self._buttons[index].IsEnabled()                


    def DoLayout(self):
        """ Common method to re-layout L{ZoomBar}. """

        self.ResetSize()
        self.GetContainingSizer().Layout()
        self.Refresh()
        

    def ResetSize(self):
        """
        Resets all the button sizes and positions, recalculating the optimal L{ZoomBar}
        size.
        """

        xSize = self._buttonSize*len(self._buttons) + len(self._buttons) + self._buttonSize
        ySize = self._buttonSize*2

        self._imgBar.SetSize(xSize+self._buttonSize, ySize)

        for button in self._buttons:
            button.LoopScales(self._buttonSize)

        if self._showLabels:
            dc = wx.ClientDC(self)
            dummy, yextent = dc.GetTextExtent("Ajgt")
            ySize += yextent

        if self._showReflections:
            ySize += self._buttonSize/2
        if self._centerZoom:
            ySize += self._buttonSize

        size = wx.Size(xSize+50, ySize)            
        self.SetInitialSize(size)
        self.SnapToBottom(size)

        
    # Sets up the initial buttons and sizes them from the center
    def InitialReposition(self):
        """
        Sets up the initial buttons and sizes them from the center.
        """
        
        # repositions the button centrally
        # odd buttons one is central - even, half by half

        if not self._buttons:
            return

        size = self.GetSize()
        oButton = self._buttons[0]
        totalWidth = oButton._width*len(self._buttons) + len(self._buttons)
        center = size.GetWidth()/2
        nbCenter = totalWidth/2
        nLeft = center - nbCenter

        if self._showReflections:
            nTop = self._imgBar._top - (oButton._height/2)
        else:
            if self._centerZoom:
                nTop = size.height/2 - oButton._height/2
            else:
                nTop = size.height - oButton._height - 5

        for oButton in self._buttons:
            oButton._left = nLeft
            oButton._top = nTop
            oButton.SetupProps(self._buttonSize)
            nLeft = nLeft + oButton._width+1
            
        # And the reflection if any
        if self._showReflections:
            nLeft = center - nbCenter
            nudge = self._nudgeReflection
            for oButton in self._reflectionButtons:
                oButton._left = nLeft
                oButton._top = (nTop + oButton._height - 2) - nudge 
                oButton.SetupProps(self._buttonSize)
                nLeft = nLeft + oButton._width + 1


    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{ZoomBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.GetScreenRect().Contains(wx.GetMousePosition()):
            return

        for button in self._buttons:
            button.SetSize(self._buttonSize, self._buttonSize)

        self.InitialReposition()
        self.Refresh()
        

    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{ZoomBar}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.SnapToBottom(event.GetSize())
        self.Refresh()
        event.Skip()


    def SnapToBottom(self, size):
        """
        Snaps the background button bar bitmap and all the buttons to the bottom
        of L{ZoomBar}.

        :param `size`: the current L{ZoomBar} size.
        """
        
        backgroundSize = self._imgBar.GetSize()
        xPos = (size.width - backgroundSize.width)/2
        yPos = (size.height - backgroundSize.height)

        self._imgBar.SetPosition(xPos, yPos)
        self.InitialReposition()
        

    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{ZoomBar}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.AutoBufferedPaintDC(self)

        dc.SetBackground(wx.WHITE_BRUSH)
        dc.Clear()

        background = self._imgBar.GetBitmap()
        pos = self._imgBar.GetPosition()

        dc.DrawBitmap(background, pos.x, pos.y, True)        
        
        if not self._buttons:
            return

        self.DrawButtons(dc)
        self.DrawReflections(dc)
        self.DrawLabels(dc)
        

    def DrawButtons(self, dc):
        """
        Draws all the main button bitmaps on the L{ZoomBar} client window.

        :param `dc`: an instance of `wx.DC`.
        """
        
        for button in self._buttons:
            pos = button.GetPosition()
            bitmap = button.GetBitmap()

            dc.DrawBitmap(bitmap, pos.x, pos.y, True)


    def DrawReflections(self, dc):            
        """
        Draws all the reflection button bitmaps on the L{ZoomBar} client window.

        :param `dc`: an instance of `wx.DC`.
        """

        if self._showReflections:
            for button in self._reflectionButtons:
                pos = button.GetPosition()
                bitmap = button.GetBitmap()

                dc.DrawBitmap(bitmap, pos.x, pos.y, True)


    def DrawLabels(self, dc):
        """
        Draws all the button labels on the L{ZoomBar} client window.

        :param `dc`: an instance of `wx.DC`.
        """

        if not self._showLabels:
            return
        
        dc.SetBrush(wx.WHITE_BRUSH)
        dc.SetPen(wx.BLACK_PEN)
        
        for button in self._buttons:
            if not button.IsZoomed():
                continue
            label = button.GetLabel()
            if not label:
                continue

            textWidth, textHeight = dc.GetTextExtent(label)
            buttonPos = button.GetPosition()
            buttonSize = button.GetSize()
            xpos = buttonPos.x + (buttonSize.width - textWidth)/2
            ypos = buttonPos.y - textHeight - 2

            dc.DrawRectangle(xpos-2, ypos-1, textWidth+4, textHeight+2)            
            dc.DrawText(label, xpos, ypos)


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{ZoomBar}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to avoid flicker.        
        """

        pass


    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{ZoomBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        pos = event.GetPosition()
        hit = self.HitTest(pos)

        if hit >= 0:
            button = self._buttons[hit]
            if not button.ZoomImage(pos.x):
                return
            
            self.Reposition(button)
        else:

            if self._previousHit < 0:
                return
            
            for button in self._buttons:
                button.SetSize(self._buttonSize, self._buttonSize)

            self.InitialReposition()

        self._previousHit = hit
        self.Refresh()

            
    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` and ``wx.EVT_LEFT_DCLICK`` events for L{ZoomBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self._currentHit = self.HitTest(event.GetPosition())


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{ZoomBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.HitTest(event.GetPosition()) != self._currentHit:
            return

        if not self.IsButtonEnabled(self._currentHit):
            return
        
        eventOut = ZoomBarEvent(wxEVT_ZOOMBAR, self.GetId())
        eventOut.SetSelection(self._currentHit)
        eventOut.SetLabel(self._buttons[self._currentHit].GetLabel())
        eventOut.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(eventOut)


    def HitTest(self, pos):
        """
        HitTest method for L{ZoomBar}.

        :param `pos`: the current mouse position.

        :return: an index representing the button on which the mouse is hovering,
         or ``wx.NOT_FOUND`` if no button has been hit.
        """

        for index, button in enumerate(self._buttons):
            buttonPos = button.GetPosition()
            if pos.x >= buttonPos.x and pos.x <= buttonPos.x + button._width:
                return index

        return wx.NOT_FOUND
    
