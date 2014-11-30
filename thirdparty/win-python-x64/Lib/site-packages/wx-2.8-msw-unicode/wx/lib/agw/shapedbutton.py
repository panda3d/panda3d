# --------------------------------------------------------------------------- #
# SHAPEDBUTTON Control wxPython IMPLEMENTATION
# Python Code By:
#
# Andrea Gavana, @ 18 Oct 2005
# Latest Revision: 15 Aug 2010, 15.00 GMT
#
#
# TODO List/Caveats
#
# 1. Elliptic Buttons May Be Handled Better, In My Opinion. They Look Nice
#    But They Are Somewhat More Difficult To Handle When Using Sizers.
#    This Is Probably Due To Some Lack In My Implementation;
#
# 2. I Am Unable To Translate The 2 Files "UpButton.png" And "DownButton.png"
#    Using "img2py" (Under wx.tools) Or With PIL In Order To Embed Them In
#    A Python File. Every Translation I Made, Did Not Preserve The Alpha
#    Channel So I Ended Up With Round Buttons Inside Black Squares. Does
#    Anyone Have A Suggestion Here?
#
# 3. Creating *A Lot* Of ShapedButtons May Require Some Time. In The Demo,
#    I Create 23 Buttons In About 0.4 Seconds On Windows XP, 3 GHz 1 GB RAM.
#
# 4. Creating Buttons With Size Greater Than wx.Size(200, 200) May Display
#    Buttons With Clearly Evident Pixel Edges. This Is Due To The Size Of The
#    Image Files I Load During Initialization. If This Is Not Satisfactory,
#    Please Let Me Know And I Will Upload Bigger Files.
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
ShapedButton tries to fill the lack of "custom shaped" controls in wxPython
and it can be used to build round or elliptic-shaped buttons.


Description
===========

ShapedButton tries to fill the lack of "custom shaped" controls in wxPython
(that depends on the same lack in wxWidgets). It can be used to build round
buttons or elliptic buttons.

I have stolen some code from `wx.lib.buttons` in order to recreate the same
classes (`GenButton`, `GenBitmapButton`, `GenBitmapTextButton`, `GenToggleButton`,
`GenBitmapToggleButton`, `GenBitmapTextToggleButton`). Here you have the same
classes (with "Gen" replaced by "S"), with the same event handling, but they
are rounded/elliptical buttons.

ShapedButton is based on a `wx.Window`, in which 2 images are drawn depending
on the button state (pressed or not pressed). The 2 images have been stolen
from Audacity (written with wxWidgets) and rearranged/reshaped/restyled
using adobe PhotoShop.
Changing the button colour in runtime was more difficult, but using some
intelligent instruction from the PIL library it can be done.

ShapedButton reacts on mouse events *only* if the mouse event occurred inside
the circle/ellipse, even if ShapedButton is built on a rectangular window.
This behavior is a lot different with respect to Audacity round buttons.


Usage
=====

The ShapedButton constructions, excluding wxPython parameter are, for the
6 Classes::

    MyShapedButton = SButton(parent, label)

    MyShapedButton = SBitmapButton(parent, bitmap)

    MyShapedButton = SBitmapTextButton(parent, bitmap, label)

    MyShapedButton = SToggleButton(parent, label)

    MyShapedButton = SBitmapToggleButton(parent, bitmap)

    MyShapedButton = SBitmapTextToggleButton(parent, bitmap, label)


The ShapedButton construction and usage is quite similar to the `wx.lib.buttons`
implementation.


Methods and Settings
====================

With ShapedButton you can:

- create rounded/elliptical buttons/togglebuttons;
- Set images for the enabled/disabled/focused/selected state of the button;
- Draw the focus indicator (or disable it);
- Set label colour and font;
- Apply a rotation to the ShapedButton label;
- Change ShapedButton shape and text orientation in runtime.


:note: ShapedButton **requires** PIL (Python Imaging Library) library to be installed,
 which can be downloaded from http://www.pythonware.com/products/pil/ .


Window Styles
=============

`No particular window styles are available for this class.`


Events Processing
=================

This class processes the following events:

================= ==================================================
Event Name        Description
================= ==================================================
``wx.EVT_BUTTON`` Process a `wx.wxEVT_COMMAND_BUTTON_CLICKED` event, when the button is clicked. 
================= ==================================================


License And Version
===================

ShapedButton is distributed under the wxPython license.

Latest revision: Andrea Gavana @ 15 Aug 2010, 15.00 GMT

Version 0.4

"""


#----------------------------------------------------------------------
# Beginning Of SHAPEDBUTTON wxPython Code
#----------------------------------------------------------------------

import wx
from wx.lib import imageutils

# First Check If PIL Is Installed Properly
try:

    import PIL.Image as Image

except ImportError:

    errstr = ("\nShapedButton *Requires* PIL (Python Imaging Library).\n"
             "You Can Get It At:\n\n"
             "http://www.pythonware.com/products/pil/\n\n"
             "ShapedButton Can Not Continue. Exiting...\n")

    raise Exception(errstr)

import os

folder = os.path.split(__file__)[0]

# Import Some Stuff For The Annoying Ellipsis... ;-)
from math import sin, cos, pi

#-----------------------------------------------------------------------------
# PATH & FILE FILLING FUNCTION (OS INDEPENDENT)
# This Is Required To Load The Pressed And Non-Pressed Images From The
# "images" Directory.
#-----------------------------------------------------------------------------

def opj(path):
    """
    Convert paths to the platform-specific separator.

    :param `path`: the path to convert.
    """

    strs = apply(os.path.join, tuple(path.split('/')))
    # HACK: on Linux, a leading / gets lost...
    if path.startswith('/'):
        strs = '/' + strs

    return strs

#-----------------------------------------------------------------------------


#----------------------------------------------------------------------
# Class SButtonEvent
# Code Stolen From wx.lib.buttons. This Class Handles All The Button
# And ToggleButton Events.
#----------------------------------------------------------------------

class SButtonEvent(wx.PyCommandEvent):
    """ Event sent from the generic buttons when the button is activated. """

    def __init__(self, eventType, eventId):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self.isDown = False
        self.theButton = None
        

    def SetIsDown(self, isDown):
        """
        Sets the button event as pressed.

        :param `isDown`: ``True`` to set the event as "pressed", ``False`` otherwise.
        """
        
        self.isDown = isDown


    def GetIsDown(self):
        """ Returns ``True`` if the button event is "pressed". """
        
        return self.isDown
    

    def SetButtonObj(self, btn):
        """
        Sets the event object for the event.

        :param `btn`: the button object.
        """

        self.theButton = btn


    def GetButtonObj(self):
        """ Returns the object associated with this event. """

        return self.theButton


#----------------------------------------------------------------------
# SBUTTON Class
# This Is The Main Class Implementation. See __init__() Method For
# Details. All The Other Button Types Depend On This Class For Event
# Handling And Property Settings.
#----------------------------------------------------------------------

class SButton(wx.Window):
    """ This is the main implementation of ShapedButton. """
    
    _labeldelta = 1

    def __init__(self, parent, id=wx.ID_ANY, label="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize):
        """
        Default class constructor.

        :param `parent`: the L{SButton} parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `label`: the button text label;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform.
        """

        wx.Window.__init__(self, parent, id, pos, size)

        if label is None:
            label = ""

        self._enabled = True
        self._isup = True
        self._hasfocus = False
        self._usefocusind = True

        # Initialize Button Properties
        self.SetButtonColour()
        self.SetLabel(label)
        self.SetLabelColour()
        self.InitColours()
        self.SetAngleOfRotation()
        self.SetEllipseAxis()

        if size == wx.DefaultSize:
            self.SetInitialSize(self.DoGetBestSize())

        # Event Binding
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)

        if wx.Platform == '__WXMSW__':
            self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDown)

        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_SET_FOCUS, self.OnGainFocus)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnLoseFocus)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)

        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_ERASE_BACKGROUND, lambda x: None)
        self.Bind(wx.EVT_PAINT, self.OnPaint)


    def SetButtonColour(self, colour=None):
        """
        Sets the button colour, for all button states.

        :param `colour`: an instance of `wx.Colour`.
        
        :note: The original button images are greyscale with a lot of pixels with
         different colours. Changing smoothly the button colour in order to
         give the same 3D effect can be efficiently done only with PIL.
        """

        if colour is None:
            colour = wx.WHITE

        palette = colour.Red(), colour.Green(), colour.Blue()

        self._buttoncolour = colour

        self._mainbuttondown = DownButton.GetImage()
        self._mainbuttonup = UpButton.GetImage()
        


    def GetButtonColour(self):
        """ Returns the button colour. """

        return self._buttoncolour


    def SetLabelColour(self, colour=None):
        """
        Sets the button label colour.

        :param `colour`: an instance of `wx.Colour`.
        """

        if colour is None:
            colour = wx.BLACK

        self._labelcolour = colour


    def GetLabelColour(self):
        """ Returns the button label colour. """

        return self._labelcolour


    def SetLabel(self, label=None):
        """
        Sets the button label.

        :param `label`: the new button label.
        """

        if label is None:
            label = ""

        self._buttonlabel = label


    def GetLabel(self):
        """ Returns the button label. """

        return self._buttonlabel


    def SetBestSize(self, size=None):
        """
        Given the current font settings, calculate and set a good size.

        :param `size`: if not ``None``, an instance of `wx.Size` to pass to
         `SetInitialSize`.
        """

        if size is None:
            size = wx.DefaultSize

        self.SetInitialSize(size)


    def DoGetBestSize(self):
        """
        Overridden base class virtual. Determines the best size of the button
        based on the label size.
        """

        w, h, usemin = self._GetLabelSize()
        defsize = wx.Button.GetDefaultSize()
        width = 12 + w

        if usemin and width < defsize.width:
           width = defsize.width

        height = 11 + h

        if usemin and height < defsize.height:
            height = defsize.height

        return (width, height)


    def AcceptsFocus(self):
        """
        Can this window be given focus by mouse click?

        :note: Overridden from `wx.Window`.
        """

        return self.IsShown() and self.IsEnabled()


    def ShouldInheritColours(self):
        """
        Overridden base class virtual. Buttons usually do not inherit
        parent's colours.
        """

        return False


    def Enable(self, enable=True):
        """
        Enables/disables the button.

        :param `enable`: ``True`` to enable the button, ``False`` to disable it.
        
        :note: Overridden from `wx.Window`.
        """

        self._enabled = enable
        self.Refresh()


    def IsEnabled(self):
        """ Returns wheter the button is enabled or not. """

        return self._enabled


    def SetUseFocusIndicator(self, flag):
        """
        Specifies if a focus indicator (dotted line) should be used.

        :param `flag`: ``True`` to use the focus indicator, ``False`` otherwise.
        """

        self._usefocusind = flag


    def GetUseFocusIndicator(self):
        """ Returns focus indicator flag. """

        return self._usefocusind


    def InitColours(self):
        """
        Calculates a new set of focus indicator colour and indicator pen
        based on button colour and label colour.
        """

        textclr = self.GetLabelColour()
        faceclr = self.GetButtonColour()

        r, g, b = faceclr.Get()
        hr, hg, hb = min(255,r+64), min(255,g+64), min(255,b+64)
        self._focusclr = wx.Colour(hr, hg, hb)

        if wx.Platform == "__WXMAC__":
            self._focusindpen = wx.Pen(textclr, 1, wx.SOLID)
        else:
            self._focusindpen = wx.Pen(textclr, 1, wx.USER_DASH)
            self._focusindpen.SetDashes([1,1])
            self._focusindpen.SetCap(wx.CAP_BUTT)


    def SetDefault(self):
        """ Sets the button as default item. """

        self.GetParent().SetDefaultItem(self)


    def _GetLabelSize(self):
        """ Used internally """

        w, h = self.GetTextExtent(self.GetLabel())
        return w, h, True


    def Notify(self):
        """ Notifies an event and let it be processed. """

        evt = SButtonEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, self.GetId())
        evt.SetIsDown(not self._isup)
        evt.SetButtonObj(self)
        evt.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(evt)


    def DrawMainButton(self, dc, width, height):
        """
        Draws the main button, in whichever state it is.

        :param `dc`: an instance of `wx.DC`;
        :param `width`: the button width;
        :param `height`: the button height.
        """

        w = min(width, height)

        if w <= 2:
            return

        position = self.GetPosition()

        main, secondary = self.GetEllipseAxis()
        xc = width/2
        yc = height/2

        if abs(main - secondary) < 1e-6:
            # In This Case The Button Is A Circle
            if self._isup:
                img = self._mainbuttonup.Scale(w, w)
            else:
                img = self._mainbuttondown.Scale(w, w)
        else:
            # In This Case The Button Is An Ellipse... Some Math To Do
            rect = self.GetRect()

            if main > secondary:
                # This Is An Ellipse With Main Axis Aligned With X Axis
                rect2 = w
                rect3 = w*secondary/main

            else:
                # This Is An Ellipse With Main Axis Aligned With Y Axis
                rect3 = w
                rect2 = w*main/secondary

            if self._isup:
                img = self._mainbuttonup.Scale(rect2, rect3)
            else:
                img = self._mainbuttondown.Scale(rect2, rect3)

        bmp = img.ConvertToBitmap()

        if abs(main - secondary) < 1e-6:
            if height > width:
                xpos = 0
                ypos = (height - width)/2
            else:
                xpos = (width - height)/2
                ypos = 0
        else:
            if height > width:
                if main > secondary:
                    xpos = 0
                    ypos = (height - rect3)/2
                else:
                    xpos = (width - rect2)/2
                    ypos = (height - rect3)/2
            else:
                if main > secondary:
                    xpos = (width - rect2)/2
                    ypos = (height - rect3)/2
                else:
                    xpos = (width - rect2)/2
                    ypos = 0

        # Draw Finally The Bitmap
        dc.DrawBitmap(bmp, xpos, ypos, True)

        # Store Bitmap Position And Size To Draw An Elliptical Focus Indicator
        self._xpos = xpos
        self._ypos = ypos
        self._imgx = img.GetWidth()
        self._imgy = img.GetHeight()


    def DrawLabel(self, dc, width, height, dw=0, dh=0):
        """
        Draws the label on the button.

        :param `dc`: an instance of `wx.DC`;
        :param `width`: the button width;
        :param `height`: the button height;
        :param `dw`: width differential, to show a 3D effect;
        :param `dh`: height differential, to show a 3D effect.        
        """

        dc.SetFont(self.GetFont())

        if self.IsEnabled():
            dc.SetTextForeground(self.GetLabelColour())
        else:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))

        label = self.GetLabel()
        tw, th = dc.GetTextExtent(label)

        w = min(width, height)

        # labeldelta Is Used To Give The Impression Of A "3D" Click
        if not self._isup:
            dw = dh = self._labeldelta

        angle = self.GetAngleOfRotation()*pi/180.0

        # Check If There Is Any Rotation Chosen By The User
        if angle == 0:
            dc.DrawText(label, (width-tw)/2+dw, (height-th)/2+dh)
        else:
            xc, yc = (width/2, height/2)

            xp = xc - (tw/2)* cos(angle) - (th/2)*sin(angle)
            yp = yc + (tw/2)*sin(angle) - (th/2)*cos(angle)

            dc.DrawRotatedText(label, xp + dw, yp + dh , angle*180/pi)


    def DrawFocusIndicator(self, dc, width, height):
        """
        Draws the focus indicator. This is a circle/ellipse inside the button
        drawn with a dotted-style pen, to let the user know which button has
        the focus.

        :param `dc`: an instance of `wx.DC`;
        :param `width`: the button width;
        :param `height`: the button height.        
        """

        self._focusindpen.SetColour(self._focusclr)
        dc.SetLogicalFunction(wx.INVERT)
        dc.SetPen(self._focusindpen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        main, secondary = self.GetEllipseAxis()

        if abs(main - secondary) < 1e-6:
            # Ah, That Is A Round Button
            if height > width:
                dc.DrawCircle(width/2, height/2, width/2-4)
            else:
                dc.DrawCircle(width/2, height/2, height/2-4)
        else:
            # This Is An Ellipse
            if hasattr(self, "_xpos"):
                dc.DrawEllipse(self._xpos + 2, self._ypos + 2, self._imgx - 4, self._imgy - 4)

        dc.SetLogicalFunction(wx.COPY)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{SButton}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.Refresh()
        event.Skip()


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{SButton}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        (width, height) = self.GetClientSizeTuple()

        # Use A Double Buffered DC (Good Speed Up)
        dc = wx.BufferedPaintDC(self)

        # The DC Background *Must* Be The Same As The Parent Background Colour,
        # In Order To Hide The Fact That Our "Shaped" Button Is Still Constructed
        # Over A Rectangular Window
        brush = wx.Brush(self.GetParent().GetBackgroundColour(), wx.SOLID)

        dc.SetBackground(brush)
        dc.Clear()

        self.DrawMainButton(dc, width, height)
        self.DrawLabel(dc, width, height)

        if self._hasfocus and self._usefocusind:
            self.DrawFocusIndicator(dc, width, height)


    def IsOutside(self, x, y):
        """
        Checks if a mouse events occurred inside the circle/ellipse or not.

        :param `x`: the mouse x position;
        :param `y`: the mouse y position.
        """

        (width, height) = self.GetClientSizeTuple()
        diam = min(width, height)
        xc, yc = (width/2, height/2)

        main, secondary = self.GetEllipseAxis()

        if abs(main - secondary) < 1e-6:
            # This Is A Circle
            if ((x - xc)**2.0 + (y - yc)**2.0) > (diam/2.0)**2.0:
                return True
        else:
            # This Is An Ellipse
            mn = max(main, secondary)
            main = self._imgx/2.0
            secondary = self._imgy/2.0
            if (((x-xc)/main)**2.0 + ((y-yc)/secondary)**2.0) > 1:
                return True

        return False


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{SButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled():
            return

        x, y = (event.GetX(), event.GetY())

        if self.IsOutside(x,y):
            return

        self._isup = False

        if not self.HasCapture():
            self.CaptureMouse()

        self.SetFocus()

        self.Refresh()
        event.Skip()


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{SButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled() or not self.HasCapture():
            return

        if self.HasCapture():
            self.ReleaseMouse()

            if not self._isup:
                self.Notify()

            self._isup = True
            self.Refresh()
            event.Skip()


    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{SButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled() or not self.HasCapture():
            return

        if event.LeftIsDown() and self.HasCapture():
            x, y = event.GetPositionTuple()

            if self._isup and not self.IsOutside(x, y):
                self._isup = False
                self.Refresh()
                return

            if not self._isup and self.IsOutside(x,y):
                self._isup = True
                self.Refresh()
                return

        event.Skip()


    def OnGainFocus(self, event):
        """
        Handles the ``wx.EVT_SET_FOCUS`` event for L{SButton}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self._hasfocus = True
        dc = wx.ClientDC(self)
        w, h = self.GetClientSizeTuple()

        if self._usefocusind:
            self.DrawFocusIndicator(dc, w, h)


    def OnLoseFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{SButton}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self._hasfocus = False
        dc = wx.ClientDC(self)
        w, h = self.GetClientSizeTuple()

        if self._usefocusind:
            self.DrawFocusIndicator(dc, w, h)

        self.Refresh()


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{SButton}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        if self._hasfocus and event.KeyCode() == ord(" "):

            self._isup = False
            self.Refresh()

        event.Skip()


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` event for L{SButton}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        if self._hasfocus and event.GetKeyCode() == ord(" "):

            self._isup = True
            self.Notify()
            self.Refresh()

        event.Skip()


    def MakePalette(self, tr, tg, tb):
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


    def ConvertWXToPIL(self, bmp):
        """
        Converts a `wx.Image` into a PIL image.

        :param `bmp`: an instance of `wx.Image`.    
        """

        width = bmp.GetWidth()
        height = bmp.GetHeight()
        img = Image.fromstring("RGBA", (width, height), bmp.GetData())

        return img


    def ConvertPILToWX(self, pil, alpha=True):
        """
        Converts a PIL image into a `wx.Image`.

        :param `pil`: a PIL image;
        :param `alpha`: ``True`` if the image contains alpha transparency, ``False``
         otherwise.
        """

        if alpha:
            image = apply(wx.EmptyImage, pil.size)
            image.SetData( pil.convert("RGB").tostring() )
            image.SetAlphaData(pil.convert("RGBA").tostring()[3::4])
        else:
            image = wx.EmptyImage(pil.size[0], pil.size[1])
            new_image = pil.convert('RGB')
            data = new_image.tostring()
            image.SetData(data)

        return image


    def SetAngleOfRotation(self, angle=None):
        """
        Sets angle of button label rotation.

        :param `angle`: the label rotation, in degrees.
        """

        if angle is None:
            angle = 0

        self._rotation = angle*pi/180


    def GetAngleOfRotation(self):
        """ Returns angle of button label rotation, in degrees. """

        return self._rotation*180.0/pi


    def SetEllipseAxis(self, main=None, secondary=None):
        """
        Sets the ellipse axis. What it is important is not their absolute values
        but their ratio.

        :param `main`: a floating point number representing the absolute dimension
         of the main ellipse axis;
        :param `secondary`: a floating point number representing the absolute dimension
         of the secondary ellipse axis.         
        """

        if main is None:
            main = 1.0
            secondary = 1.0

        self._ellipseaxis = (main, secondary)


    def GetEllipseAxis(self):
        """ Returns the ellipse axes. """

        return self._ellipseaxis


#----------------------------------------------------------------------
# SBITMAPBUTTON Class
# It Is Derived From SButton, And It Is A Class Useful To Draw A
# ShapedButton With An Image In The Middle. The Button Can Have 4
# Different Bitmaps Assigned To Its Different States (Pressed, Non
# Pressed, With Focus, Disabled).
#----------------------------------------------------------------------


class SBitmapButton(SButton):
    """
    Subclass of L{SButton} which displays a bitmap, acting like a
    `wx.BitmapButton`.
    """

    def __init__(self, parent, id, bitmap, pos=wx.DefaultPosition, size=wx.DefaultSize):
        """
        Default class constructor.

        :param `parent`: the L{SBitmapButton} parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `bitmap`: the button bitmap (if any);
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform.
        """

        self._bmpdisabled = None
        self._bmpfocus = None
        self._bmpselected = None

        self.SetBitmapLabel(bitmap)

        SButton.__init__(self, parent, id, "", pos, size)


    def GetBitmapLabel(self):
        """ Returns the bitmap associated with the button in the normal state. """
        
        return self._bmplabel


    def GetBitmapDisabled(self):
        """ Returns the bitmap displayed when the button is disabled. """

        return self._bmpdisabled


    def GetBitmapFocus(self):
        """ Returns the bitmap displayed when the button has the focus. """

        return self._bmpfocus


    def GetBitmapSelected(self):
        """ Returns the bitmap displayed when the button is selected (pressed). """

        return self._bmpselected


    def SetBitmapDisabled(self, bitmap):
        """
        Sets the bitmap to display when the button is disabled.

        :param `bitmap`: a valid `wx.Bitmap` object.
        """

        self._bmpdisabled = bitmap


    def SetBitmapFocus(self, bitmap):
        """
        Sets the bitmap to display when the button has the focus.

        :param `bitmap`: a valid `wx.Bitmap` object.
        """

        self._bmpfocus = bitmap
        self.SetUseFocusIndicator(False)


    def SetBitmapSelected(self, bitmap):
        """
        Sets the bitmap to display when the button is selected (pressed).

        :param `bitmap`: a valid `wx.Bitmap` object.
        """

        self._bmpselected = bitmap


    def SetBitmapLabel(self, bitmap, createothers=True):
        """
        Sets the bitmap to display normally. This is the only one that is
        required.

        :param `bitmap`: a valid `wx.Bitmap` object;
        :param `createothers`: if set to ``True``, then the other bitmaps will be
         generated on the fly. Currently, only the disabled bitmap is generated.
        """

        self._bmplabel = bitmap

        if bitmap is not None and createothers:
            image = wx.ImageFromBitmap(bitmap)
            imageutils.grayOut(image)
            self.SetBitmapDisabled(wx.BitmapFromImage(image))


    def _GetLabelSize(self):
        """ Used internally. """

        if not self._bmplabel:
            return -1, -1, False

        return self._bmplabel.GetWidth() + 2, self._bmplabel.GetHeight() + 2, False


    def DrawLabel(self, dc, width, height, dw=0, dh=0):
        """
        Draws the bitmap in the middle of the button.

        :param `dc`: an instance of `wx.DC`;
        :param `width`: the button width;
        :param `height`: the button height;
        :param `dw`: width differential, to show a 3D effect;
        :param `dh`: height differential, to show a 3D effect.        
        """

        bmp = self._bmplabel

        if self._bmpdisabled and not self.IsEnabled():
            bmp = self._bmpdisabled

        if self._bmpfocus and self._hasfocus:
            bmp = self._bmpfocus

        if self._bmpselected and not self._isup:
            bmp = self._bmpselected

        bw, bh = bmp.GetWidth(), bmp.GetHeight()

        if not self._isup:
            dw = dh = self._labeldelta

        hasMask = bmp.GetMask() != None
        dc.DrawBitmap(bmp, (width - bw)/2 + dw, (height - bh)/2 + dh, hasMask)


#----------------------------------------------------------------------
# SBITMAPTEXTBUTTON Class
# It Is Derived From SButton, And It Is A Class Useful To Draw A
# ShapedButton With An Image And Some Text Ceneterd In The Middle.
# The Button Can Have 4 Different Bitmaps Assigned To Its Different
# States (Pressed, Non Pressed, With Focus, Disabled).
#----------------------------------------------------------------------

class SBitmapTextButton(SBitmapButton):
    """
    Subclass of L{SButton} which displays a bitmap and a label.
    """

    def __init__(self, parent, id, bitmap, label,
                 pos=wx.DefaultPosition, size=wx.DefaultSize):
        """
        Default class constructor.

        :param `parent`: the L{SBitmapTextButton} parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `bitmap`: the button bitmap (if any);
        :param `label`: the button text label;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform.
        """

        SBitmapButton.__init__(self, parent, id, bitmap, pos, size)
        self.SetLabel(label)


    def _GetLabelSize(self):
        """ Used internally. """

        w, h = self.GetTextExtent(self.GetLabel())

        if not self._bmplabel:
            return w, h, True       # if there isn't a bitmap use the size of the text

        w_bmp = self._bmplabel.GetWidth() + 2
        h_bmp = self._bmplabel.GetHeight() + 2
        width = w + w_bmp

        if h_bmp > h:
            height = h_bmp
        else:
            height = h

        return width, height, True


    def DrawLabel(self, dc, width, height, dw=0, dh=0):
        """
        Draws the bitmap and the text label.

        :param `dc`: an instance of `wx.DC`;
        :param `width`: the button width;
        :param `height`: the button height;
        :param `dw`: width differential, to show a 3D effect;
        :param `dh`: height differential, to show a 3D effect.
        """

        bmp = self._bmplabel

        if bmp != None:     # if the bitmap is used

            if self._bmpdisabled and not self.IsEnabled():
                bmp = self._bmpdisabled

            if self._bmpfocus and self._hasfocus:
                bmp = self._bmpfocus

            if self._bmpselected and not self._isup:
                bmp = self._bmpselected

            bw, bh = bmp.GetWidth(), bmp.GetHeight()

            if not self._isup:
                dw = dh = self._labeldelta

            hasMask = bmp.GetMask() != None

        else:

            bw = bh = 0     # no bitmap -> size is zero

        dc.SetFont(self.GetFont())

        if self.IsEnabled():
            dc.SetTextForeground(self.GetLabelColour())
        else:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))

        label = self.GetLabel()
        tw, th = dc.GetTextExtent(label)  # size of text

        if not self._isup:
            dw = dh = self._labeldelta

        w = min(width, height)

        pos_x = (width - bw - tw)/2 + dw      # adjust for bitmap and text to centre

        rotangle = self.GetAngleOfRotation()*pi/180.0

        if bmp != None:
            if rotangle < 1.0/180.0:
                dc.DrawBitmap(bmp, pos_x, (height - bh)/2 + dh, hasMask) # draw bitmap if available
                pos_x = pos_x + 4   # extra spacing from bitmap
            else:
                pass

        if rotangle < 1.0/180.0:
            dc.DrawText(label, pos_x + dw + bw, (height-th)/2+dh)      # draw the text
        else:
            xc, yc = (width/2, height/2)
            xp = xc - (tw/2)* cos(rotangle) - (th/2)*sin(rotangle)
            yp = yc + (tw/2)*sin(rotangle) - (th/2)*cos(rotangle)
            dc.DrawRotatedText(label, xp, yp , rotangle*180.0/pi)


#----------------------------------------------------------------------
# __STOGGLEMIXIN Class
# A Mixin That Allows To Transform Any Of SButton, SBitmapButton And
# SBitmapTextButton In The Corresponding ToggleButtons.
#----------------------------------------------------------------------

class __SToggleMixin(object):
    """
    A mixin that allows to transform any of L{SButton}, L{SBitmapButton} and
    L{SBitmapTextButton} in the corresponding toggle buttons.
    """

    def SetToggle(self, flag):
        """
        Sets the button as toggled/not toggled.

        :param `flag`: ``True`` to set the button as toggled, ``False`` otherwise.
        """

        self._isup = not flag
        self.Refresh()

    SetValue = SetToggle


    def GetToggle(self):
        """ Returns the toggled state of a button. """

        return not self._isup

    GetValue = GetToggle


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for the button.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled():
            return

        x, y = (event.GetX(), event.GetY())

        if self.IsOutside(x,y):
            return

        self._saveup = self._isup
        self._isup = not self._isup

        if not self.HasCapture():
            self.CaptureMouse()

        self.SetFocus()
        self.Refresh()


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for the button.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """


        if not self.IsEnabled() or not self.HasCapture():
            return

        if self.HasCapture():
            if self._isup != self._saveup:
                self.Notify()

            self.ReleaseMouse()
            self.Refresh()


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for the button.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        event.Skip()


    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for the button.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled():
            return

        if event.LeftIsDown() and self.HasCapture():

            x,y = event.GetPositionTuple()
            w,h = self.GetClientSizeTuple()

            if not self.IsOutside(x, y):
                self._isup = not self._saveup
                self.Refresh()
                return

            if self.IsOutside(x,y):
                self._isup = self._saveup
                self.Refresh()
                return

        event.Skip()


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` event for the button.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        if self._hasfocus and event.KeyCode() == ord(" "):

            self._isup = not self._isup
            self.Notify()
            self.Refresh()

        event.Skip()


#----------------------------------------------------------------------
# STOGGLEBUTTON Class
#----------------------------------------------------------------------

class SToggleButton(__SToggleMixin, SButton):
    """ A ShapedButton toggle button. """
    pass


#----------------------------------------------------------------------
# SBITMAPTOGGLEBUTTON Class
#----------------------------------------------------------------------

class SBitmapToggleButton(__SToggleMixin, SBitmapButton):
    """ A ShapedButton toggle bitmap button. """
    pass


#----------------------------------------------------------------------
# SBITMAPTEXTTOGGLEBUTTON Class
#----------------------------------------------------------------------

class SBitmapTextToggleButton(__SToggleMixin, SBitmapTextButton):
    """ A ShapedButton toggle bitmap button with a text label. """
    pass



#----------------------------------------------------------------------
from wx.lib.embeddedimage import PyEmbeddedImage

UpButton = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAMgAAADICAMAAACahl6sAAAACXBIWXMAAA7DAAAOwwHHb6hk"
    "AAAABGdBTUEAALGOfPtRkwAAACBjSFJNAAB6JQAAgIMAAPn/AACA6QAAdTAAAOpgAAA6mAAA"
    "F2+SX8VGAAADAFBMVEX//////8z//5n//2b//zP//wD/zP//zMz/zJn/zGb/zDP/zAD/mf//"
    "mcz/mZn/mWb/mTP/mQD/Zv//Zsz/Zpn/Zmb/ZjP/ZgD/M///M8z/M5n/M2b/MzP/MwD/AP//"
    "AMz/AJn/AGb/ADP/AADM///M/8zM/5nM/2bM/zPM/wDMzP/MzMzMzJnMzGbMzDPMzADMmf/M"
    "mczMmZnMmWbMmTPMmQDMZv/MZszMZpnMZmbMZjPMZgDMM//MM8zMM5nMM2bMMzPMMwDMAP/M"
    "AMzMAJnMAGbMADPMAACZ//+Z/8yZ/5mZ/2aZ/zOZ/wCZzP+ZzMyZzJmZzGaZzDOZzACZmf+Z"
    "mcyZmZmZmWaZmTOZmQCZZv+ZZsyZZpmZZmaZZjOZZgCZM/+ZM8yZM5mZM2aZMzOZMwCZAP+Z"
    "AMyZAJmZAGaZADOZAABm//9m/8xm/5lm/2Zm/zNm/wBmzP9mzMxmzJlmzGZmzDNmzABmmf9m"
    "mcxmmZlmmWZmmTNmmQBmZv9mZsxmZplmZmZmZjNmZgBmM/9mM8xmM5lmM2ZmMzNmMwBmAP9m"
    "AMxmAJlmAGZmADNmAAAz//8z/8wz/5kz/2Yz/zMz/wAzzP8zzMwzzJkzzGYzzDMzzAAzmf8z"
    "mcwzmZkzmWYzmTMzmQAzZv8zZswzZpkzZmYzZjMzZgAzM/8zM8wzM5kzM2YzMzMzMwAzAP8z"
    "AMwzAJkzAGYzADMzAAAA//8A/8wA/5kA/2YA/zMA/wAAzP8AzMwAzJkAzGYAzDMAzAAAmf8A"
    "mcwAmZkAmWYAmTMAmQAAZv8AZswAZpkAZmYAZjMAZgAAM/8AM8wAM5kAM2YAMzMAMwAAAP8A"
    "AMwAAJkAAGYAADMAAADU1NTT09PS0tLR0dHQ0NDNzc3Ly8vKysrJycnIyMjHx8fGxsbFxcXC"
    "wsLBwcH39/f09PTw8PDs7Ozo6Ojj4+Pe3t7Z2dnX19fV1dXOzs7ExMTDw8O+vr66urq1tbWw"
    "sLCsrKyoqKikpKSgoKCcnJyUlJSPj4////+2FWUJAAABAHRSTlP/////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "//////////////////////////////8AU/cHJQAAIcNJREFUeNpi+D9MAEAAMQwXjwAE0LDx"
    "CEAA0cIjL9+8wwreA8GHGzdvvn1FA0sBAoj6HsHhCySP3Lx54y3VrQUIIGp6BOHkt2/fAgkQ"
    "BWKBIIpvQB76cPPmrVtUtBwggBio6o232AHYT9g8cvs21awHCCCqeOQV2LFg8AYI4Yw3EIDq"
    "KUTUfPhw48ONGzdvvaaGGwACiAoeeQF14hv8AN03sJgB+uTWS8pdARBAlHoE2QuvXwMROgAK"
    "Y/UNIqWBY4XiRAYQQJR55DVKVLzGBbDEDCL/gzwCipVbbyhyCkAAUeARhnfIkQEFr5AAVr9g"
    "ZBZY8gLGyh0KXAMQQAwUJyp4enqFCRD+wOsTSO0CKsTukO0cgAAi0yMvoXEBcj/UBy/BAKdP"
    "wKohHkHzDKSeBCYvkFfu3CGz2gcIIAayYwPuESRvQHyC8A5q0nqNvTBG88gd8mIFIIAYyPIG"
    "PF9APIHwBlqkvMYaI2jZHV4Mg3L87Tu373z8SIajAAKIgazoQIqNl+jg1UusKes1IotAPPMO"
    "pUKBe+Q2KFLI8AlAAJHqkVeQDA72BLLzX2CLEWw5nVCMgD1y5+NdUgtjgAAi0SMMkNhAigug"
    "D168ePHyBdwr8Bh5/QqSpJA88vYNlsYKvO2F4pG7JLoMIIAYSM8d0KwB9sQLKECKEeQkhdZI"
    "eYPWknyH3FRB9gfII9raJDkNIIAYSKvHkWID5PQXCIDFH8hewNLcIugR7XckOA4ggBhIiw54"
    "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"
    "gg==")

#----------------------------------------------------------------------
DownButton = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAMgAAADICAMAAACahl6sAAAACXBIWXMAAA7DAAAOwwHHb6hk"
    "AAAABGdBTUEAALGOfPtRkwAAACBjSFJNAAB6JQAAgIMAAPn/AACA6QAAdTAAAOpgAAA6mAAA"
    "F2+SX8VGAAADAFBMVEX//////8z//5n//2b//zP//wD/zP//zMz/zJn/zGb/zDP/zAD/mf//"
    "mcz/mZn/mWb/mTP/mQD/Zv//Zsz/Zpn/Zmb/ZjP/ZgD/M///M8z/M5n/M2b/MzP/MwD/AP//"
    "AMz/AJn/AGb/ADP/AADM///M/8zM/5nM/2bM/zPM/wDMzP/MzMzMzJnMzGbMzDPMzADMmf/M"
    "mczMmZnMmWbMmTPMmQDMZv/MZszMZpnMZmbMZjPMZgDMM//MM8zMM5nMM2bMMzPMMwDMAP/M"
    "AMzMAJnMAGbMADPMAACZ//+Z/8yZ/5mZ/2aZ/zOZ/wCZzP+ZzMyZzJmZzGaZzDOZzACZmf+Z"
    "mcyZmZmZmWaZmTOZmQCZZv+ZZsyZZpmZZmaZZjOZZgCZM/+ZM8yZM5mZM2aZMzOZMwCZAP+Z"
    "AMyZAJmZAGaZADOZAABm//9m/8xm/5lm/2Zm/zNm/wBmzP9mzMxmzJlmzGZmzDNmzABmmf9m"
    "mcxmmZlmmWZmmTNmmQBmZv9mZsxmZplmZmZmZjNmZgBmM/9mM8xmM5lmM2ZmMzNmMwBmAP9m"
    "AMxmAJlmAGZmADNmAAAz//8z/8wz/5kz/2Yz/zMz/wAzzP8zzMwzzJkzzGYzzDMzzAAzmf8z"
    "mcwzmZkzmWYzmTMzmQAzZv8zZswzZpkzZmYzZjMzZgAzM/8zM8wzM5kzM2YzMzMzMwAzAP8z"
    "AMwzAJkzAGYzADMzAAAA//8A/8wA/5kA/2YA/zMA/wAAzP8AzMwAzJkAzGYAzDMAzAAAmf8A"
    "mcwAmZkAmWYAmTMAmQAAZv8AZswAZpkAZmYAZjMAZgAAM/8AM8wAM5kAM2YAMzMAMwAAAP8A"
    "AMwAAJkAAGYAADMAAADV1dXU1NTT09PS0tLR0dHQ0NDOzs7Nzc3Ly8vKysrJycnIyMjGxsbD"
    "w8PCwsL39/f09PTx8fHu7u7p6enk5OTf39/Z2dnW1tbPz8/Hx8fExMS/v7+7u7u3t7e0tLSw"
    "sLCtra2pqamlpaWhoaGdnZ2Tk5OPj4////+ZlMjzAAABAHRSTlP/////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "//////////////////////////////8AU/cHJQAAIMRJREFUeNpi+D9MAEAAMQwXjwAE0LDx"
    "CEAA0cIjb+9/wQE+g8GdNzSwFCCAqO8R7S+4AcQjz549+0B1awECiJoeAbr0KwR8QwNQ4a9f"
    "viK8A/IPFS0HCCAG6nvjGy4A8Qsibp5R0ysAAUQVj7yFeeA7CPzADr5DANRHyL6hSpYBCCAq"
    "eOQ1NBJgfviJDSB7B9Mvryl3BUAAUeoRkCcgPgA59xcY/IYBKPcXlPEL4Smwb6ApjTr5BSCA"
    "KPPI7a+QqEDyxm/sAMUrkJhBRAzEJ+8ocgpAAFHgEQZIbEAiA+aFP1gAmm+Q/PIdyStAvzyl"
    "wDUAAcRAYaKCxQXcD2FQgN0zEJ/8gmYaSLRACmWwTz59Its5AAFEpkfewjwBiQhkH2AArL4B"
    "e+Yn3C9wrzz9RGbGBwgg8jwCLmrh/vjzG+6Pv38x/QH3ye8/6EkMFiuIbP/06acnZDkJIIAY"
    "yEtU4OgA+QIRFX+hANU3KMkLwyPwAgwSJ1/AcUKeVwACiIF8f/z8hUhSf+EewZmuUPMJwiff"
    "kYtisFc+PSHDJwABRKpH3oN9AU5TUE/8hccFqk9gSQqpCEb4AZhHEEUxwi/w4uvTk7ckOgwg"
    "gBhILXLBsQFJVMgJCs0bEF/AowAlIqBegBbDP38gl8RfYXn+yRMSXQYQQAykZnJQdPxE9cY/"
    "NH8g/IDi8p+/sLZdfqK0XJCS10eSnAYQQKR45AHUH6BkhYiMf3CfhCF7A7VVgtrgggHUNhi4"
    "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"
    "fdLmvAAAAABJRU5ErkJggg==")

