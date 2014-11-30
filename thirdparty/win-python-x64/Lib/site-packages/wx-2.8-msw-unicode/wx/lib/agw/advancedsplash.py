# --------------------------------------------------------------------------- #
# ADVANCEDSPLASH Control wxPython IMPLEMENTATION
# Python Code By:
#
# Andrea Gavana, @ 10 Oct 2005
# Latest Revision: 27 Jan 2011, 15.00 GMT
#
#
# TODO List/Caveats
#
# 1. Actually, Setting The Shape Of AdvancedSplash Is Done Using "SetShape"
#    Function On A Frame. This Works, AFAIK, On This Following Platforms:
#
#    - MSW
#    - UNIX/Linux
#    - MacOS Carbon
#
#    Obviously I May Be Wrong Here. Could Someone Verify That Lines 139-145
#    Work Correctly On Other Platforms Than Mine (MSW XP/2000)?
#    Moreover, Is There A Way To Avoid The Use Of The "SetShape" Method?
#    I Don't Know.
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
AdvancedSplash tries to reproduce the behavior of `wx.SplashScreen`, with
some enhancements.


Description
===========

AdvancedSplash tries to reproduce the behavior of `wx.SplashScreen`, but with
some enhancements (in my opinion).

AdvancedSplash starts its construction from a simple frame. Then, depending on
the options passed to AdvancedSplash, it sets the frame shape accordingly to
the image passed as input. AdvancedSplash behaves somewhat like `wx.SplashScreen`,
and almost all the methods available in `wx.SplashScreen` are available also In
AdvancedSplash.


Usage
=====

Sample usage::

    SplashScreen = AS.AdvancedSplash(parent, bitmap, timeout, agwStyle,
                                     shadowcolour)


None of the options are strictly required (a part of the `bitmap` parameter).
If you use the defaults you get a very simple AdvancedSplash.


Methods and Settings
====================

AdvancedSplash is customizable, and in particular you can set:

- Whether you want to mask a colour or not in your input bitmap;
- Where to center the splash screen (on screen, on parent or nowhere);
- Whether it is a "timeout" splashscreen or not;
- The time after which AdvancedSplash is destroyed (if it is a timeout splashscreen);
- The (optional) text you wish to display;
- The font, colour and position of the displayed text (optional).


Window Styles
=============

This class supports the following window styles:

======================= =========== ==================================================
Window Styles           Hex Value   Description
======================= =========== ==================================================
``AS_TIMEOUT``                  0x1 `AdvancedSplash` will be destroyed after `timeout` milliseconds.
``AS_NOTIMEOUT``                0x2 `AdvancedSplash` can be destroyed by clicking on it, pressing a key or by explicitly call the `Close()` method.
``AS_CENTER_ON_SCREEN``         0x4 `AdvancedSplash` will be centered on screen.
``AS_CENTER_ON_PARENT``         0x8 `AdvancedSplash` will be centered on parent.
``AS_NO_CENTER``               0x10 `AdvancedSplash` will not be centered.
``AS_SHADOW_BITMAP``           0x20 If the bitmap you pass as input has no transparency, you can choose one colour that will be masked in your bitmap. the final shape of `AdvancedSplash` will be defined only by non-transparent (non-masked) pixels.
======================= =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

AdvancedSplash control is distributed under the wxPython license.

Latest revision: Andrea Gavana @ 27 Jan 2011, 15.00 GMT

Version 0.4

"""


#----------------------------------------------------------------------
# Beginning Of ADVANCEDSPLASH wxPython Code
#----------------------------------------------------------------------

import wx

# These Are Used To Declare If The AdvancedSplash Should Be Destroyed After The
# Timeout Or Not
                        
AS_TIMEOUT = 1
""" `AdvancedSplash` will be destroyed after `timeout` milliseconds. """
AS_NOTIMEOUT = 2
""" `AdvancedSplash` can be destroyed by clicking on it, pressing a key or by""" \
""" explicitly call the Close() method. """

# These Flags Are Used To Position AdvancedSplash Correctly On Screen
AS_CENTER_ON_SCREEN = 4
""" `AdvancedSplash` will be centered on screen. """
AS_CENTER_ON_PARENT = 8
""" `AdvancedSplash` will be centered on parent. """
AS_NO_CENTER = 16
""" `AdvancedSplash` will not be centered. """

# This Option Allow To Mask A Colour In The Input Bitmap
AS_SHADOW_BITMAP = 32
""" If the bitmap you pass as input has no transparency, you can choose one colour""" \
""" that will be masked in your bitmap. the final shape of `AdvancedSplash` will be""" \
""" defined only by non-transparent (non-masked) pixels. """

#----------------------------------------------------------------------
# ADVANCEDSPLASH Class
# This Is The Main Class Implementation. See __init__() Method For
# Details.
#----------------------------------------------------------------------

class AdvancedSplash(wx.Frame):
    """
    AdvancedSplash tries to reproduce the behavior of `wx.SplashScreen`, with
    some enhancements.

    This is the main class implementation.    
    """
    def __init__(self, parent, id=-1, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.FRAME_NO_TASKBAR | wx.FRAME_SHAPED | wx.STAY_ON_TOP,
                 bitmap=None, timeout=5000,
                 agwStyle=AS_TIMEOUT | AS_CENTER_ON_SCREEN,
                 shadowcolour=wx.NullColour):
        """
        Default class constructor.

        :param `bitmap`: this must be a valid `wx.Bitmap`, that you may construct using
         whatever image file format supported by wxPython. If the file you load
         already supports mask/transparency (like png), the transparent areas
         will not be drawn on screen, and the L{AdvancedSplash} frame will have
         the shape defined only by *non-transparent* pixels.
         If you use other file formats that does not supports transparency, you
         can obtain the same effect as above by masking a specific colour in
         your `wx.Bitmap`.

        :param `timeout`: if you construct L{AdvancedSplash} using the style ``AS_TIMEOUT``,
         L{AdvancedSplash} will be destroyed after `timeout` milliseconds;

        :param `agwStyle`: this value specifies the L{AdvancedSplash} styles:
        
         ======================= =========== ==================================================
         Window Styles           Hex Value   Description
         ======================= =========== ==================================================
         ``AS_TIMEOUT``                  0x1 L{AdvancedSplash} will be destroyed after `timeout` milliseconds.
         ``AS_NOTIMEOUT``                0x2 L{AdvancedSplash} can be destroyed by clicking on it, pressing a key or by explicitly call the `Close()` method.
         ``AS_CENTER_ON_SCREEN``         0x4 L{AdvancedSplash} will be centered on screen.
         ``AS_CENTER_ON_PARENT``         0x8 L{AdvancedSplash} will be centered on parent.
         ``AS_NO_CENTER``               0x10 L{AdvancedSplash} will not be centered.
         ``AS_SHADOW_BITMAP``           0x20 If the bitmap you pass as input has no transparency, you can choose one colour that will be masked in your bitmap. the final shape of L{AdvancedSplash} will be defined only by non-transparent (non-masked) pixels.
         ======================= =========== ==================================================

        :param `shadowcolour`: if you construct L{AdvancedSplash} using the style
         ``AS_SHADOW_BITMAP``, here you can specify the colour that will be masked on
         your input bitmap. This has to be a valid wxPython colour.

        """

        wx.Frame.__init__(self, parent, id, "", pos, size, style)

        # Some Error Checking
        if agwStyle & AS_TIMEOUT and timeout <= 0:
            raise Exception('\nERROR: Style "AS_TIMEOUT" Used With Invalid "timeout" Parameter Value (' \
                            + str(timeout) + ')')

        if agwStyle & AS_SHADOW_BITMAP and not shadowcolour.IsOk():
            raise Exception('\nERROR: Style "AS_SHADOW_BITMAP" Used With Invalid "shadowcolour" Parameter')

        if not bitmap or not bitmap.IsOk():
            raise Exception("\nERROR: Bitmap Passed To AdvancedSplash Is Invalid.")

        if agwStyle & AS_SHADOW_BITMAP:
            # Our Bitmap Is Masked Accordingly To User Input
            self.bmp = self.ShadowBitmap(bitmap, shadowcolour)
        else:
            self.bmp = bitmap

        self._agwStyle = agwStyle

        # Setting Initial Properties
        self.SetText()
        self.SetTextFont()
        self.SetTextPosition()
        self.SetTextColour()

        # Calculate The Shape Of AdvancedSplash Using The Input-Modified Bitmap
        self.reg = wx.RegionFromBitmap(self.bmp)

        # Don't Know If It Works On Other Platforms!!
        # Tested Only In Windows XP/2000

        if wx.Platform == "__WXGTK__":
            self.Bind(wx.EVT_WINDOW_CREATE, self.SetSplashShape)
        else:
            self.SetSplashShape()

        w = self.bmp.GetWidth()
        h = self.bmp.GetHeight()

        # Set The AdvancedSplash Size To The Bitmap Size
        self.SetSize((w, h))

        if agwStyle & AS_CENTER_ON_SCREEN:
            self.CenterOnScreen()
        elif agwStyle & AS_CENTER_ON_PARENT:
            self.CenterOnParent()

        if agwStyle & AS_TIMEOUT:
            # Starts The Timer. Once Expired, AdvancedSplash Is Destroyed
            self._splashtimer = wx.PyTimer(self.OnNotify)
            self._splashtimer.Start(timeout)

        # Catch Some Mouse Events, To Behave Like wx.SplashScreen
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouseEvents)
        self.Bind(wx.EVT_CHAR, self.OnCharEvents)

        self.Show()


    def SetSplashShape(self, event=None):
        """
        Sets L{AdvancedSplash} shape using the region created from the bitmap.

        :param `event`: a `wx.WindowCreateEvent` event (GTK only, as GTK supports setting
         the window shape only during window creation).
        """

        self.SetShape(self.reg)

        if event is not None:
            event.Skip()


    def ShadowBitmap(self, bmp, shadowcolour):
        """
        Applies a mask on the bitmap accordingly to user input.

        :param `bmp`: the bitmap to which we want to apply the mask colour `shadowcolour`;
        :param `shadowcolour`: the mask colour for our bitmap.
        """

        mask = wx.Mask(bmp, shadowcolour)
        bmp.SetMask(mask)

        return bmp


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{AdvancedSplash}.

        :param `event`: a `wx.PaintEvent` to be processed.
        """

        dc = wx.PaintDC(self)

        # Here We Redraw The Bitmap Over The Frame
        dc.DrawBitmap(self.bmp, 0, 0, True)

        # We Draw The Text Anyway, Wheter It Is Empty ("") Or Not
        textcolour = self.GetTextColour()
        textfont = self.GetTextFont()
        textpos = self.GetTextPosition()
        text = self.GetText()

        dc.SetFont(textfont[0])
        dc.SetTextForeground(textcolour)
        dc.DrawText(text, textpos[0], textpos[1])

        # Seems like this only helps on OS X.
        if wx.Platform == "__WXMAC__":
            wx.SafeYield(self, True)


    def OnNotify(self):
        """ Handles the timer expiration, and calls the `Close()` method. """

        self.Close()


    def OnMouseEvents(self, event):
        """
        Handles the ``wx.EVT_MOUSE_EVENTS`` events for L{AdvancedSplash}.

        :param `event`: a `wx.MouseEvent` to be processed.
        
        :note: This reproduces the behavior of `wx.SplashScreen`.
        """

        if event.LeftDown() or event.RightDown():
            self.Close()

        event.Skip()


    def OnCharEvents(self, event):
        """
        Handles the ``wx.EVT_CHAR`` event for L{AdvancedSplash}.

        :param `event`: a `wx.KeyEvent` to be processed.
        
        :note: This reproduces the behavior of `wx.SplashScreen`.
        """

        self.Close()


    def OnCloseWindow(self, event):
        """
        Handles the ``wx.EVT_CLOSE`` event for L{AdvancedSplash}.

        :param `event`: a `wx.CloseEvent` to be processed.
        
        :note: This reproduces the behavior of `wx.SplashScreen`.
        """

        if hasattr(self, "_splashtimer"):
            self._splashtimer.Stop()
            del self._splashtimer

        self.Destroy()


    def SetText(self, text=None):
        """
        Sets the text to be displayed on L{AdvancedSplash}.

        :param `text`: the text we want to display on top of the bitmap. If `text` is
         set to ``None``, nothing will be drawn on top of the bitmap.
        """

        if text is None:
            text = ""

        self._text = text

        self.Refresh()
        self.Update()


    def GetText(self):
        """ Returns the text displayed on L{AdvancedSplash}."""

        return self._text


    def SetTextFont(self, font=None):
        """
        Sets the font for the text in L{AdvancedSplash}.

        :param `font`: the font to use while drawing the text on top of our bitmap. If `font`
         is ``None``, a simple generic font is generated.
        """

        if font is None:
            self._textfont = wx.Font(1, wx.SWISS, wx.NORMAL, wx.BOLD, False)
            self._textsize = 10.0
            self._textfont.SetPointSize(self._textsize)
        else:
            self._textfont = font
            self._textsize = font.GetPointSize()
            self._textfont.SetPointSize(self._textsize)

        self.Refresh()
        self.Update()


    def GetTextFont(self):
        """ Gets the font for the text in L{AdvancedSplash}."""

        return self._textfont, self._textsize


    def SetTextColour(self, colour=None):
        """
        Sets the colour for the text in L{AdvancedSplash}.

        :param `colour`: the text colour to use while drawing the text on top of our
         bitmap. If `colour` is ``None``, then ``wx.BLACK`` is used.
        """

        if colour is None:
            colour = wx.BLACK

        self._textcolour = colour
        self.Refresh()
        self.Update()


    def GetTextColour(self):
        """ Gets the colour for the text in L{AdvancedSplash}."""

        return self._textcolour


    def SetTextPosition(self, position=None):
        """
        Sets the text position inside L{AdvancedSplash} frame.

        :param `position`: the text position inside our bitmap. If `position` is ``None``,
         the text will be placed at the top-left corner.
        """

        if position is None:
            position = (0,0)

        self._textpos = position
        self.Refresh()
        self.Update()


    def GetTextPosition(self):
        """ Returns the text position inside L{AdvancedSplash} frame."""

        return self._textpos


    def GetSplashStyle(self):
        """ Returns a list of strings and a list of integers containing the styles. """

        stringstyle = []
        integerstyle = []

        if self._agwStyle & AS_TIMEOUT:
            stringstyle.append("AS_TIMEOUT")
            integerstyle.append(AS_TIMEOUT)

        if self._agwStyle & AS_NOTIMEOUT:
            stringstyle.append("AS_NOTIMEOUT")
            integerstyle.append(AS_NOTIMEOUT)

        if self._agwStyle & AS_CENTER_ON_SCREEN:
            stringstyle.append("AS_CENTER_ON_SCREEN")
            integerstyle.append(AS_CENTER_ON_SCREEN)

        if self._agwStyle & AS_CENTER_ON_PARENT:
            stringstyle.append("AS_CENTER_ON_PARENT")
            integerstyle.append(AS_CENTER_ON_PARENT)

        if self._agwStyle & AS_NO_CENTER:
            stringstyle.append("AS_NO_CENTER")
            integerstyle.append(AS_NO_CENTER)

        if self._agwStyle & AS_SHADOW_BITMAP:
            stringstyle.append("AS_SHADOW_BITMAP")
            integerstyle.append(AS_SHADOW_BITMAP)

        return stringstyle, integerstyle


