# --------------------------------------------------------------------------- #
# TOASTERBOX wxPython IMPLEMENTATION
# Ported And Enhanced From wxWidgets Contribution (Aj Bommarito) By:
#
# Andrea Gavana, @ 16 September 2005
# Latest Revision: 28 Nov 2010, 16.00 GMT
#
#
# TODO/Caveats List
#
# 1. Any Idea?
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
ToasterBox is a cross-platform widget to make the creation of MSN style "toaster"
popups easier.


Description
===========

ToasterBox is a cross-platform widget to make the creation of MSN style "toaster"
popups easier. The syntax is really easy especially if you are familiar with the
syntax of wxPython.

It has 2 main styles:

- ``TB_SIMPLE``: using this style, you will be able to specify a background image for
  ToasterBox, text properties as text colour, font and label;

- ``TB_COMPLEX``: this style will allow you to put almost any control inside a
  ToasterBox. You can add a panel in which you can put all the controls you like.

Both styles support the setting of ToasterBox position (on screen coordinates),
size, the time after which the ToasterBox is destroyed (linger), and the scroll
speed of ToasterBox.


Supported Platforms
===================

ToasterBox has been tested on the following platforms:

- Windows (verified on Windows XP, 2000)
- Linux
- Mac


Window Styles
=============

This class supports the following window styles:

==================== =========== ==================================================
Window Styles        Hex Value   Description
==================== =========== ==================================================
``TB_SIMPLE``                0x1 A simple `ToasterBox`, with background image and text customization can be created.
``TB_ONTIME``                0x1 `ToasterBox` will close after a specified amount of time.
``TB_COMPLEX``               0x2 ToasterBoxes with different degree of complexity can be created. You can add as  many controls as you want, provided that you call the L{AddPanel} method and pass  to it a dummy frame and a `wx.Panel`. See the demo for details.
``TB_ONCLICK``               0x2 `ToasterBox` can be closed by clicking anywhere on the `ToasterBox` frame.
``TB_DEFAULT_STYLE``   0x2008002 Default window style for `ToasterBox`, with no caption nor close box.
``TB_CAPTION``        0x22009806 `ToasterBox` will have a caption, with the possibility to set a title for the `ToasterBox` frame, and a close box.
==================== =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

ToasterBox is distributed under the wxPython license.

Latest revision: Andrea Gavana @ 28 Nov 2010, 16.00 GMT

Version 0.3

"""

import textwrap
import wx


# Define Window List, We Use It Globally
winlist = []

TB_SIMPLE = 1
""" A simple ToasterBox, with background image and text customization can be created. """
TB_COMPLEX = 2
""" ToasterBoxes with different degree of complexity can be created. You can add as """ \
""" many controls as you want, provided that you call the AddPanel() method and pass """ \
""" to it a dummy frame and a wx.Panel. See the demo for details. """
TB_DEFAULT_STYLE = wx.SIMPLE_BORDER | wx.STAY_ON_TOP | wx.FRAME_NO_TASKBAR
""" Default window style for `ToasterBox`, with no caption nor close box. """
TB_CAPTION = TB_DEFAULT_STYLE | wx.CAPTION | wx.SYSTEM_MENU | wx.CLOSE_BOX | wx.FRAME_TOOL_WINDOW
""" `ToasterBox` will have a caption, with the possibility to set a title """ \
""" for the `ToasterBox` frame, and a close box. """
TB_ONTIME = 1
""" `ToasterBox` will close after a specified amount of time. """
TB_ONCLICK = 2
""" `ToasterBox` can be closed by clicking anywhere on the `ToasterBox` frame. """

# scroll from up to down
TB_SCR_TYPE_UD = 1
# scroll from down to up
TB_SCR_TYPE_DU = 2
# fade in/out 
TB_SCR_TYPE_FADE = 4


# ------------------------------------------------------------------------------ #
# Class ToasterBox
#    Main Class Implementation. It Is Basically A wx.Timer. It Creates And
#    Displays Popups And Handles The "Stacking".
# ------------------------------------------------------------------------------ #

class ToasterBox(wx.Timer):
    """
    ToasterBox is a cross-platform widget to make the creation of MSN style "toaster"
    popups easier.
    """

    def __init__(self, parent, tbstyle=TB_SIMPLE, windowstyle=TB_DEFAULT_STYLE,
                 closingstyle=TB_ONTIME, scrollType=TB_SCR_TYPE_DU):
        """
        Default class constructor.

        :param `parent`: the window parent;
        :param `tbstyle`: the L{ToasterBox} main style. Can be one of the following
         bits:

         ====================== ======= ================================
         `ToasterBox` Style      Value  Description
         ====================== ======= ================================
         ``TB_SIMPLE``              0x1 A simple L{ToasterBox}, with background image and text customization can be created
         ``TB_COMPLEX``             0x2 `ToasterBoxes` with different degree of complexity can be created. You can add as many controls as you want, provided that you call the L{AddPanel} method and pass to it a dummy frame and a `wx.Panel`.
         ====================== ======= ================================

        :param `windowstyle`: this parameter influences the visual appearance of
         L{ToasterBox}, and can be one of the following styles:

         ====================== ========== ================================
         Window Style           Hex Value  Description
         ====================== ========== ================================
         ``TB_DEFAULT_STYLE``   0x2008002  Default window style for L{ToasterBox}, with no caption nor close box.
         ``TB_CAPTION``         0x22009806 L{ToasterBox} will have a caption, with the possibility to set a title for the L{ToasterBox} frame, and a close box.
         ====================== ========== ================================
       
        :param `closingstyle`: the closing style for L{ToasterBox}. Can be one of the
         following bits:

         ==================== =========== ==================================================
         Closing Styles       Hex Value   Description
         ==================== =========== ==================================================
         ``TB_ONTIME``                0x1 L{ToasterBox} will close after a specified amount of time.
         ``TB_ONCLICK``               0x2 L{ToasterBox} can be closed by clicking anywhere on the L{ToasterBox} frame.
         ==================== =========== ==================================================

        :param `scrollType`: the scrolling direction for L{ToasterBox}. Can be one of the
         following bits:

         ==================== =========== ==================================================
         Scroll Styles        Hex Value   Description
         ==================== =========== ==================================================
         ``TB_SCR_TYPE_UD``           0x1 L{ToasterBox} will scroll from up to down
         ``TB_SCR_TYPE_DU``           0x2 L{ToasterBox} will scroll from down to up
         ``TB_SCR_TYPE_FADE``         0x4 L{ToasterBox} will fade in/out (without scrolling). 
         ==================== =========== ==================================================
         
        """

        self._parent = parent
        self._sleeptime = 10
        self._pausetime = 1700
        self._popuptext = "default"
        self._popupposition = wx.Point(100,100)
        self._popuptop = wx.Point(0,0)
        self._popupsize = wx.Size(150, 170)
        self._usefocus = True
        self._originalfocus = wx.Window.FindFocus()

        self._backgroundcolour = wx.WHITE
        self._foregroundcolour = wx.BLACK
        self._textfont = wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL, False, "Verdana")

        self._bitmap = None

        self._tbstyle = tbstyle
        self._windowstyle = windowstyle
        self._closingstyle = closingstyle
        self._scrollType = scrollType
        
        self._panel = None

        self._bottomright = wx.Point(wx.GetDisplaySize().GetWidth(),
                                     wx.GetDisplaySize().GetHeight())

        if parent is not None:
            parent.Bind(wx.EVT_ICONIZE, lambda evt: [w.Hide() for w in winlist])
            self._moveTimer = wx.Timer(parent, -1)
            parent.Bind(wx.EVT_TIMER, self.OnMoveTimer, self._moveTimer)
 
        self._tb = ToasterBoxWindow(self._parent, self, self._tbstyle, self._windowstyle,
                                    self._closingstyle, scrollType=self._scrollType)


        
    def SetPopupPosition(self, pos):
        """
        Sets the L{ToasterBox} position on screen.

        :param `pos`: the widget position, an instance of `wx.Point`.        
        """

        self._popupposition = pos


    def SetPopupPositionByInt(self, pos):
        """
        Sets the L{ToasterBox} position on screen, at one of the screen corners.
 
        :param `pos`: an integer specifying the screen corner, namely:

         ============= ========================================
         Corner Number Position
         ============= ========================================
               0       Top left screen corner
               1       Top right screen corner
               2       Bottom left screen corner
               3       Bottom right screen corner
         ============= ========================================
         
        """

        w, h = wx.GetDisplaySize()
        self._bottomright = wx.Point(w, h)

        # top left
        if pos == 0:
            popupposition = wx.Point(0,0)
        # top right
        elif pos == 1:
            popupposition = wx.Point(w - self._popupsize[0], 0)
        # bottom left
        elif pos == 2:
            popupposition = wx.Point(0, h - self._popupsize[1])
        # bottom right
        elif pos == 3:
            popupposition = wx.Point(self._bottomright.x - self._popupsize[0],
                                     self._bottomright.y - self._popupsize[1])

        self._bottomright = wx.Point(popupposition.x + self._popupsize[0],
                                     popupposition.y + self._popupsize[1])

        self._popupposition = popupposition


    def CenterOnParent(self, direction=wx.BOTH):
        """
        Centres the window on its parent (if any). If the L{ToasterBox} parent is ``None``,
        it calls L{CenterOnScreen}.

        :param `direction`: specifies the direction for the centering. May be ``wx.HORIZONTAL``,
         ``wx.VERTICAL`` or ``wx.BOTH``.

        :note: This methods provides for a way to center L{ToasterBox} over their parents instead of the
         entire screen. If there is no parent, then behaviour is the same as L{CenterOnScreen}.

        :see: L{CenterOnScreen}.
        """

        if not self._parent:
            self.CenterOnScreen(direction)
            return

        parent = self._parent
        screenrect = parent.GetScreenRect()
        toast_width, toast_height = self._popupsize
        x, y = screenrect.GetX(), screenrect.GetY()
        width, height = screenrect.GetWidth(), screenrect.GetHeight()
        
        if direction == wx.VERTICAL:
            pos = wx.Point(x, (y + (height/2) - (toast_height/2)))
        elif direction == wx.HORIZONTAL:
            pos = wx.Point((x + (width/2) - (toast_width/2)), y)
        else:
            pos = wx.Point((x + (width/2) - (toast_width/2)), (y + (height/2) - (toast_height/2)))

        tb.SetPopupPosition(pos)        
                

    CentreOnParent = CenterOnParent


    def CenterOnScreen(self, direction=wx.BOTH):
        """
        Centres the L{ToasterBox} on screen.

        :param `direction`: specifies the direction for the centering. May be ``wx.HORIZONTAL``,
         ``wx.VERTICAL`` or ``wx.BOTH``.

        :see: L{CenterOnParent}.
        """

        screenSize = wx.GetDisplaySize()
        toast_width, toast_height = self._popupsize
        width, height = screenSize.GetWidth(), screenSize.GetHeight()
        
        if direction == wx.VERTICAL:
            pos = wx.Point(0, (height/2) - (toast_height/2))
        elif direction == wx.HORIZONTAL:
            pos = wx.Point((width/2) - (toast_width/2), 0)
        else:
            pos = wx.Point((width/2) - (toast_width/2), (height/2) - (toast_height/2))

        tb.SetPopupPosition(pos)


    CentreOnScreen = CenterOnScreen        

    
    def SetPopupBackgroundColour(self, colour=None):
        """
        Sets the L{ToasterBox} background colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, then
         the background colour will be white.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        if colour is None:
            colour = wx.WHITE

        if isinstance(colour, basestring):
            colour = wx.NamedColour(colour)
            
        self._backgroundcolour = colour
        self._tb.SetPopupBackgroundColour(self._backgroundcolour)


    def SetPopupTextColour(self, colour=None):
        """
        Sets the L{ToasterBox} foreground colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, then
         the background colour will be black.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        if colour is None:
            colour = wx.BLACK

        if isinstance(colour, basestring):
            colour = wx.NamedColour(colour)
            
        self._foregroundcolour = colour


    def SetPopupTextFont(self, font=None):
        """
        Sets the L{ToasterBox} text font.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, then
         a simple generic font will be generated.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        if font is None:
            font = wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL, False)

        self._textfont = font


    def SetPopupSize(self, size):
        """
        Sets the L{ToasterBox} size.

        :param `size`: the new control size, an instance of `wx.Size`.        
        """

        self._popupsize = size


    def SetPopupPauseTime(self, pausetime):
        """
        Sets the time after which the L{ToasterBox} is destroyed (linger).

        :param `pausetime`: the delay after which the control is destroyed, in seconds.
        """

        self._pausetime = pausetime


    def SetPopupBitmap(self, bitmap=None):
        """
        Sets the L{ToasterBox} background image.

        :param `bitmap`: a valid `wx.Bitmap` object or filename. If defaulted
         to ``None``, then no background bitmap is used.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        if bitmap is not None:
            if isinstance(bitmap, basestring):
                bitmap = wx.Bitmap(bitmap)

        self._bitmap = bitmap


    def SetPopupScrollSpeed(self, speed):
        """
        Sets the L{ToasterBox} scroll speed.

        :param `speed`: it is the pause time (in milliseconds) for every step in the
         `ScrollUp` method.
        """

        self._sleeptime = speed


    def SetPopupText(self, text):
        """
        Sets the L{ToasterBox} text label.

        :param `text`: the widget label.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        self._popuptext = text


    def AddPanel(self, panel):
        """
        Adds a panel to the L{ToasterBox}.

        :param `panel`: an instance of `wx.Window`.
        
        :note: Use this method only for a L{ToasterBox} created with the ``TB_COMPLEX`` style.
        """

        if not self._tbstyle & TB_COMPLEX:
            raise Exception("\nERROR: Panel Can Not Be Added When Using TB_SIMPLE ToasterBox Style")

        self._panel = panel


    def Play(self):
        """ Creates the L{ToasterBoxWindow}, that does all the job. """

        # create new window
        self._tb.SetPopupSize((self._popupsize[0], self._popupsize[1]))
        self._tb.SetPopupPosition((self._popupposition[0], self._popupposition[1]))
        self._tb.SetPopupPauseTime(self._pausetime)
        self._tb.SetPopupScrollSpeed(self._sleeptime)
        self._tb.SetUseFocus(self._usefocus, self._originalfocus)

        if self._tbstyle == TB_SIMPLE:
            self._tb.SetPopupTextColour(self._foregroundcolour)
            self._tb.SetPopupBackgroundColour(self._backgroundcolour)
            self._tb.SetPopupTextFont(self._textfont)

            if self._bitmap is not None:
                self._tb.SetPopupBitmap(self._bitmap)

            self._tb.SetPopupText(self._popuptext)

        if self._tbstyle == TB_COMPLEX:
            if self._panel is not None:
                self._tb.AddPanel(self._panel)

        # clean up the list
        self.CleanList()

        # check to see if there is already a window displayed
        # by looking at the linked list
        if len(winlist) > 0:
            # there ARE other windows displayed already
            # reclac where it should display
            self.MoveAbove(self._tb)

        # shift new window on to the list
        winlist.append(self._tb)

        if not self._tb.Play():
            # if we didn't show the window properly, remove it from the list
            winlist.remove(winlist[-1])
            # delete the object too
            self._tb.Destroy()
            return


    def MoveAbove(self, tb):
        """
        If a L{ToasterBox} already exists, move the new one above the existing one.

        :param `tb`: another instance of L{ToasterBox}.
        """

        # recalc where to place this popup
 
        self._tb.SetPopupPosition((self._popupposition[0], self._popupposition[1] -
                                   self._popupsize[1]*len(winlist)))


    def GetToasterBoxWindow(self):
        """ Returns the L{ToasterBox} frame. """

        return self._tb


    def SetTitle(self, title):
        """
        Sets the L{ToasterBox} title if it was created with ``TB_CAPTION`` window style.

        :param `title`: the L{ToasterBox} caption title.        
        """

        self._tb.SetTitle(title)


    def SetUseFocus(self, focus):
        """
        If `focus` is ``True``, Instructs L{ToasterBox} to steal the focus from the
        parent application, otherwise it returns the focus to the original owner.

        :param `focus`: ``True`` to set the focus on L{ToasterBox}, ``False`` to
         return it to the original owner.
        """

        self._usefocus = focus


    def GetUseFocus(self):
        """ Returns whether L{ToasterBox} will steal the focus from the parent application. """

        return self._usefocus
    

    def Notify(self):
        """ It's time to hide a L{ToasterBox}. """

        if len(winlist) == 0:
            return

        # clean the window list
        self.CleanList()

        # figure out how many blanks we have
        try:
            node = winlist[0]
        except:
            return

        if not node:
            return

        self._startPos = node.GetPosition()[1]
        self._moveTimer.Start(self._sleeptime)


    def OnMoveTimer(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{ToasterBox}, moving the new window
        on top of the last one created.

        :param `event`: a `wx.TimerEvent` event to be processed.
        """

        current = self._startPos
        if current >= self._popupposition[1]:
            self._moveTimer.Stop()

        # move windows to fill in blank space
        
        if current > self._popupposition[1]:
            current = self._popupposition[1]

        # loop through all the windows
        for j in xrange(0, len(winlist)):
            ourNewHeight = current - (j*self._popupsize[1] - 8)
            tmpTb = winlist[j]
            # reset where the object THINKS its supposed to be
            tmpTb.SetPopupPosition((self._popupposition[0], ourNewHeight))
            # actually move it
            tmpTb.SetDimensions(self._popupposition[0], ourNewHeight, tmpTb.GetSize().GetWidth(),
                                tmpTb.GetSize().GetHeight())

        self._startPos += 4
        

    def CleanList(self):
        """ Cleans the window list, erasing the stack of L{ToasterBox} objects. """

        if len(winlist) == 0:
            return

        node = winlist[0]
        while node:
            if not node.IsShown():
                winlist.remove(node)
                node.Close()
                try:
                    node = winlist[0]
                except:
                    node = 0
            else:
                indx = winlist.index(node)
                try:
                    node = winlist[indx+1]
                except:
                    node = 0


# ------------------------------------------------------------------------------ #
# Class ToasterBoxWindow
#    This Class Does All The Job, By Handling Background Images, Text Properties
#    And Panel Adding. Depending On The Style You Choose, ToasterBoxWindow Will
#    Behave Differently In Order To Handle Widgets Inside It.
# ------------------------------------------------------------------------------ #

class ToasterBoxWindow(wx.Frame):
    """
    This class does all the job, by handling background images, text properties
    and panel adding. Depending on the style you choose, L{ToasterBoxWindow} will
    behave differently in order to handle widgets inside it.
    """
    
    def __init__(self, parent, parent2, tbstyle, windowstyle, closingstyle,
                 scrollType=TB_SCR_TYPE_DU):
        """
        Default class constructor.
        Used internally. Do not call directly this class in your application!

        :param `parent`: the window parent;
        :param `parent2`: the L{ToasterBox} calling this window;
        :param `tbstyle`: the L{ToasterBoxWindow} main style. Can be one of the following
         bits:

         ====================== ======= ================================
         `ToasterBox` Style      Value  Description
         ====================== ======= ================================
         ``TB_SIMPLE``              0x1 A simple L{ToasterBox}, with background image and text customization can be created
         ``TB_COMPLEX``             0x2 `ToasterBoxes` with different degree of complexity can be created. You can add as many controls as you want, provided that you call the L{AddPanel} method and pass to it a dummy frame and a `wx.Panel`.
         ====================== ======= ================================

        :param `windowstyle`: this parameter influences the visual appearance of
         L{ToasterBoxWindow}, and can be one of the following styles:

         ====================== ========== ================================
         Window Style           Hex Value  Description
         ====================== ========== ================================
         ``TB_DEFAULT_STYLE``   0x2008002  Default window style for L{ToasterBox}, with no caption nor close box.
         ``TB_CAPTION``         0x22009806 L{ToasterBox} will have a caption, with the possibility to set a title for the L{ToasterBox} frame, and a close box.
         ====================== ========== ================================
       
        :param `closingstyle`: the closing style for L{ToasterBoxWindow}. Can be one of the
         following bits:

         ==================== =========== ==================================================
         Closing Styles       Hex Value   Description
         ==================== =========== ==================================================
         ``TB_ONTIME``                0x1 L{ToasterBox} will close after a specified amount of time.
         ``TB_ONCLICK``               0x2 L{ToasterBox} can be closed by clicking anywhere on the L{ToasterBox} frame.
         ==================== =========== ==================================================

        :param `scrollType`: the scrolling direction for L{ToasterBoxWindow}. Can be one of the
         following bits:

         ==================== =========== ==================================================
         Scroll Styles        Hex Value   Description
         ==================== =========== ==================================================
         ``TB_SCR_TYPE_UD``           0x1 L{ToasterBox} will scroll from up to down
         ``TB_SCR_TYPE_DU``           0x2 L{ToasterBox} will scroll from down to up
         ``TB_SCR_TYPE_FADE``         0x4 L{ToasterBox} will fade in/out (without scrolling).
         ==================== =========== ==================================================

        """

        wx.Frame.__init__(self, parent, wx.ID_ANY, "window", wx.DefaultPosition,
                         wx.DefaultSize, style=windowstyle | wx.CLIP_CHILDREN)

        self._starttime = wx.GetLocalTime()
        self._parent2 = parent2
        self._parent = parent
        self._sleeptime = 10
        self._step = 4
        self._pausetime = 1700
        self._textcolour = wx.BLACK
        self._popuptext = "Change Me!"
        # the size we want the dialog to be
        framesize = wx.Size(150, 170)
        self._count = 1
        self._tbstyle = tbstyle
        self._windowstyle = windowstyle
        self._closingstyle = closingstyle
        self._backgroundcolour = wx.WHITE

        if tbstyle == TB_COMPLEX:
            self.sizer = wx.BoxSizer(wx.VERTICAL)
        else:
            self._staticbitmap = None

        if self._windowstyle == TB_CAPTION:
            self.Bind(wx.EVT_CLOSE, self.OnClose)
            self.SetTitle("")

        if scrollType == TB_SCR_TYPE_FADE and not self.CanSetTransparent():
            import warnings
            warnings.warn("The style ``TB_SCR_TYPE_FADE`` is not supported on this platform.")
            scrollType = TB_SCR_TYPE_DU
            
        self._scrollType = scrollType

        if self._closingstyle & TB_ONCLICK and self._windowstyle != TB_CAPTION:
            self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseDown)

        self._bottomright = wx.Point(wx.GetDisplaySize().GetWidth(),
                                     wx.GetDisplaySize().GetHeight())

        self.SetDimensions(self._bottomright.x, self._bottomright.y,
                           framesize.GetWidth(), framesize.GetHeight())

        self._scrollTimer = wx.Timer(self, -1)
        self._alphaTimer = wx.Timer(self, -1)
        
        self.Bind(wx.EVT_TIMER, self.OnScrollTimer, self._scrollTimer)
        self.Bind(wx.EVT_TIMER, self.AlphaCycle, self._alphaTimer)

        if not self._tbstyle & TB_COMPLEX:
            self.Bind(wx.EVT_PAINT, self.OnPaint)
            self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        

    def OnClose(self, event):
        """
        Handles the ``wx.EVT_CLOSE`` event for L{ToasterBoxWindow}.

        :param `event`: a `wx.CloseEvent` event to be processed.
        """

        self.NotifyTimer(None)
        event.Skip()


    def OnMouseDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{ToasterBoxWindow}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self.NotifyTimer(None)
        event.Skip()


    def SetPopupBitmap(self, bitmap=None):
        """
        Sets the L{ToasterBox} background image.

        :param `bitmap`: a valid `wx.Bitmap` object. If defaulted to ``None``, then
         no background bitmap is used.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        if bitmap is None:
            self._staticbitmap = None
        else:
            bitmap = bitmap.ConvertToImage()
            xsize, ysize = self.GetSize()
            bitmap = bitmap.Scale(xsize, ysize)
            self._staticbitmap = bitmap.ConvertToBitmap()


    def SetPopupSize(self, size):
        """
        Sets the L{ToasterBox} size.

        :param `size`: the new control size, an instance of `wx.Size`.        
        """

        self.SetDimensions(self._bottomright.x, self._bottomright.y, size[0], size[1])


    def SetPopupPosition(self, pos):
        """
        Sets the L{ToasterBox} position on screen.

        :param `pos`: the widget position, an instance of `wx.Point`.        
        """

        self._bottomright = wx.Point(pos[0] + self.GetSize().GetWidth(),
                                     pos[1] + self.GetSize().GetHeight())
        self._dialogtop = pos


    def SetPopupPositionByInt(self, pos):
        """
        Sets the L{ToasterBox} position on screen, at one of the screen corners.
 
        :param `pos`: an integer specifying the screen corner, namely:

         ============= ========================================
         Corner Number Position
         ============= ========================================
               0       Top left screen corner
               1       Top right screen corner
               2       Bottom left screen corner
               3       Bottom right screen corner
         ============= ========================================
         
        """

        w, h = wx.GetDisplaySize()
        self._bottomright = wx.Point(w, h)

        # top left
        if pos == 0:
            popupposition = wx.Point(0, 0)
        # top right
        elif pos == 1:
            popupposition = wx.Point(w - self._popupsize[0], 0)
        # bottom left
        elif pos == 2:
           popupposition = wx.Point(0, h - self._popupsize[1])
         # bottom right
        elif pos == 3:
            popupposition = wx.Point(self._bottomright.x - self._popupsize[0],
                                     self._bottomright.y - self._popupsize[1])

        self._bottomright = wx.Point(popupposition.x + self._popupsize[0],
                                     popupposition.y + self._popupsize[1])

        self._dialogtop = popupposition


    def SetPopupPauseTime(self, pausetime):
        """
        Sets the time after which the L{ToasterBox} is destroyed (linger).

        :param `pausetime`: the delay after which the control is destroyed, in seconds.
        """

        self._pausetime = pausetime


    def SetPopupScrollSpeed(self, speed):
        """
        Sets the L{ToasterBox} scroll speed.

        :param `speed`: it is the pause time (in milliseconds) for every step in the
         L{ScrollUp} method.
        """

        self._sleeptime = speed


    def AddPanel(self, panel):
        """
        Adds a panel to the L{ToasterBox}.

        :param `panel`: an instance of `wx.Window`.
        
        :note: Use this method only for a L{ToasterBox} created with the ``TB_COMPLEX`` style.
        """

        if not self._tbstyle & TB_COMPLEX:
            raise Exception("\nERROR: Panel Can Not Be Added When Using TB_SIMPLE ToasterBox Style")

        self.sizer.Add(panel, 1, wx.EXPAND)
        self.SetSizer(self.sizer)
        self.Layout()
        
        if self._closingstyle & TB_ONCLICK and self._windowstyle != TB_CAPTION:
            panel.Bind(wx.EVT_LEFT_DOWN, self.OnMouseDown)


    def SetPopupText(self, text):
        """
        Sets the L{ToasterBox} text label.

        :param `text`: the widget label.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        self._popuptext = text


    def SetPopupTextFont(self, font):
        """
        Sets the L{ToasterBox} text font.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, then
         a simple generic font will be generated.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        self._textfont = font


    def GetPopupText(self):
        """
        Returns the L{ToasterBox} text.

        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.       
        """

        return self._popuptext


    def Play(self):
        """ Creates the L{ToasterBoxWindow}, that does all the job. """

        # do some checks to make sure this window is valid
        if self._bottomright.x < 1 or self._bottomright.y < 1:
            return False

        if self.GetSize().GetWidth() < 50 or self.GetSize().GetWidth() < 50:
            # toasterbox launches into a endless loop for some reason
            # when you try to make the window too small.
            return False

        self._direction = wx.UP
        self.SetupPositions()
        self.ScrollUp()
        timerid = wx.NewId()
        self.showtime = wx.Timer(self, timerid)
        self.showtime.Start(self._pausetime)
        self.Bind(wx.EVT_TIMER, self.NotifyTimer, id=timerid)

        return True


    def NotifyTimer(self, event):
        """ Hides gradually the L{ToasterBoxWindow}. """

        self.showtime.Stop()
        del self.showtime

        self._direction = wx.DOWN
        self.SetupPositions()

        self.ScrollDown()


    def SetPopupBackgroundColour(self, colour):
        """
        Sets the L{ToasterBox} background colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, then
         the background colour will be white.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        self.SetBackgroundColour(colour)
        self._backgroundcolour = colour


    def SetPopupTextColour(self, colour):
        """
        Sets the L{ToasterBox} foreground colour.

        :param `colour`: a valid `wx.Colour` object. If defaulted to ``None``, then
         the background colour will be black.
         
        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.
        """

        self._textcolour = colour


    def SetUseFocus(self, focus, originalfocus):
        """
        If `focus` is ``True``, Instructs L{ToasterBoxWindow} to steal the focus from the
        parent application, otherwise it returns the focus to the original owner.

        :param `focus`: ``True`` to set the focus on L{ToasterBoxWindow}, ``False`` to
         return it to the original owner;
        :param `originalfocus`: an instance of `wx.Window`, representing a pointer to
         the window which originally had the focus
        """

        self._usefocus = focus
        self._originalfocus = originalfocus


    def OnScrollTimer(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{ToasterBoxWindow} scrolling up/down.

        :param `event`: a `wx.TimerEvent` event to be processed.
        """

        if self._direction == wx.UP:
            self.TearUp()
        else:
            self.TearDown()
            
        
    def TearUp(self):
        """ Scrolls the L{ToasterBox} up, which means gradually showing it. """

        self._windowsize = self._windowsize + self._step
        step = self._currentStep

        if step < self._dialogtop[1]:
            step = self._dialogtop[1]

         # checking the type of the scroll (from up to down or from down to up)
        if self._scrollType == TB_SCR_TYPE_UD:
            dimY = self._dialogtop[1]
        elif self._scrollType == TB_SCR_TYPE_DU:
            dimY = step

        self.SetDimensions(self._dialogtop[0], dimY, self.GetSize().GetWidth(), self._windowsize)

        self.Refresh(False)

        self._currentStep += self._scrollStep

        if self._currentStep not in range(self._start, self._stop, self._scrollStep):
            self._scrollTimer.Stop()
            self.Update()

            if self._tbstyle == TB_SIMPLE:
                self.DrawText()

            if self._usefocus:
                self.SetFocus()
            else:
                self._originalfocus.SetFocus()

        
    def TearDown(self):
        """ Scrolls the L{ToasterBox} down, which means gradually hiding it. """

        self._windowsize = self._windowsize - self._step
        step = self._currentStep

        if step > self._bottomright.y:
            step = self._bottomright.y
        
        if self._windowsize > 0:            
            # checking the type of the scroll (from up to down or from down to up)
            if self._scrollType == TB_SCR_TYPE_UD:
                dimY = self._dialogtop[1]
            elif self._scrollType == TB_SCR_TYPE_DU:
                dimY = step

            self.SetDimensions(self._dialogtop[0], dimY,
                               self.GetSize().GetWidth(), self._windowsize)

            self.Update()
            self.Refresh()

            self._currentStep += self._scrollStep
            
        else:            
            self._scrollTimer.Stop()
            self.Hide()
            if self._parent2:
                self._parent2.Notify()


    def SetupPositions(self):
        """ Sets up the position, size and scrolling step for L{ToasterBoxWindow}. """

        if self._scrollType == TB_SCR_TYPE_FADE:
            self.SetPosition(wx.Point(*self._dialogtop))
            return

        if self._direction == wx.UP:
            # walk the Y value up in a raise motion
            self._xpos = self.GetPosition().x
            self._ypos = self._bottomright[1]
            self._windowsize = 0

            # checking the type of the scroll (from up to down or from down to up)
            if self._scrollType == TB_SCR_TYPE_UD:
                self._start = self._dialogtop[1]
                self._stop = self._ypos
                self._scrollStep = self._step
            elif self._scrollType == TB_SCR_TYPE_DU:
                self._start = self._ypos
                self._stop = self._dialogtop[1]
                self._scrollStep = -self._step

        else:

            # walk down the Y value
            self._windowsize = self.GetSize().GetHeight()

            # checking the type of the scroll (from up to down or from down to up)
            if self._scrollType == TB_SCR_TYPE_UD:
                self._start = self._bottomright.y
                self._stop = self._dialogtop[1]
                self._scrollStep = -self._step
            elif self._scrollType == TB_SCR_TYPE_DU:
                self._start = self._dialogtop[1]
                self._stop = self._bottomright.y
                self._scrollStep = self._step

        self._currentStep = self._start


    def ScrollUp(self):
        """ Scrolls the L{ToasterBox} up, which means gradually showing it. """

        if self._scrollType == TB_SCR_TYPE_FADE:
            self._amount = 0
            self._delta = 5
            self.SetSize(self.GetSize())
            self._alphaTimer.Start(self._sleeptime)
        else:
            self.Show(True)
            self._scrollTimer.Start(self._sleeptime)
    

    def ScrollDown(self):
        """ Scrolls the L{ToasterBox} down, which means gradually hiding it. """

        if self._scrollType == TB_SCR_TYPE_FADE:
            self._amount = 255
            self._delta = -5
            self._alphaTimer.Start(self._sleeptime)
        else:
            self._scrollTimer.Start(self._sleeptime)


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{ToasterBoxWindow}.

        :param `event`: a `wx.PaintEvent` event to be processed.

        :note: This event is handled and processed only if the style ``TB_SIMPLE`` is
         given to L{ToasterBox}.
        """
        
        dc = wx.AutoBufferedPaintDC(self)
        self.DrawText(dc)
        
            
    def DrawText(self, dc=None):
        """
        Draws the text label for a L{ToasterBox} with ``TB_SIMPLE`` style set.

        :param `dc`: an instance of `wx.DC`. If defaulted to ``None``, a `wx.ClientDC`
         will be created on the fly.
        """
      
        if dc is None:
            dc = wx.ClientDC(self)
           
        dc.SetBackground(wx.Brush(self._backgroundcolour))
        dc.Clear()
        
        if self._staticbitmap:
            dc.DrawBitmap(self._staticbitmap, 0, 0)
        dc.SetFont(self._textfont)
        dc.SetTextForeground(self._textcolour)
        
        if not hasattr(self, "text_coords"):
            self._getTextCoords(dc)
        dc.DrawTextList(*self.text_coords)


    def AlphaCycle(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{ToasterBoxWindow}.

        :param `event`: a `wx.TimerEvent` event to be processed.
        """

        # Increase (or decrease) the alpha channel
        self._amount += self._delta

        if self._tbstyle == TB_SIMPLE:
            self.Refresh(False)
            
        if self._amount > 255 or self._amount < 0:
            # We're done, stop the timer
            self._alphaTimer.Stop()

            if self._amount < 0:
                self.Hide()
                if self._parent2:
                    self._parent2.Notify()

            elif self._amount > 255:
                if self._usefocus:
                    self.SetFocus()
                else:
                    self._originalfocus.SetFocus()

            return

        # Make the ToasterBoxWindow more or less transparent
        self.MakeWindowTransparent(self._amount)
        if not self.IsShown():
            self.Show()
            

    def MakeWindowTransparent(self, amount):
        """
        Makes the L{ToasterBoxWindow} window transparent.

        :param `amount`: the alpha channel value.
        """

        if not self.CanSetTransparent():
            return
        
        self.SetTransparent(amount)
        

    def _getTextCoords(self, dc):
        """
        Draw the user specified text.

        :param `dc`: an instance of `wx.DC`.

        :note: Use this method only for a L{ToasterBox} created with the ``TB_SIMPLE`` style.        
        """

        # border from sides and top to text (in pixels)
        border = 7
        # how much space between text lines
        textPadding = 2

        pText = self.GetPopupText()

        max_len = len(pText)

        tw, th = self._parent2._popupsize

        if self._windowstyle == TB_CAPTION:
            th = th - 20

        while 1:
            lines = textwrap.wrap(pText, max_len)

            for line in lines:
                w, h = dc.GetTextExtent(line)
                if w > tw - border * 2:
                    max_len -= 1
                    break
            else:
                break

        fh = 0
        for line in lines:
            w, h = dc.GetTextExtent(line)
            fh += h + textPadding
        y = (th - fh) / 2; coords = []

        for line in lines:
            w, h = dc.GetTextExtent(line)
            x = (tw - w) / 2
            coords.append((x, y))
            y += h + textPadding

        self.text_coords = (lines, coords)
