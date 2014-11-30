# --------------------------------------------------------------------------------- #
# GRADIENTBUTTON wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 07 October 2008
# Latest Revision: 27 Nov 2009, 17.00 GMT
#
#
# TODO List
#
# 1) Anything to do?
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
GradientButton is another custom-drawn button class which mimics Windows CE mobile
gradient buttons.


Description
===========

GradientButton is another custom-drawn button class which mimics Windows CE mobile
gradient buttons, using a tri-vertex blended gradient plus some ClearType bold
font (best effect with Tahoma Bold). GradientButton supports:

* Triple blended gradient background, with customizable colours;
* Custom colours for the "pressed" state;
* Rounded-corners buttons;
* Text-only or image+text buttons.

And a lot more. Check the demo for an almost complete review of the functionalities.


Supported Platforms
===================

GradientButton has been tested on the following platforms:
  * Windows (Windows XP).


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

GradientButton is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 27 Nov 2009, 17.00 GMT

Version 0.3

"""

import wx


HOVER = 1
CLICK = 2

class GradientButtonEvent(wx.PyCommandEvent):
    """ Event sent from L{GradientButton} when the button is activated. """
    
    def __init__(self, eventType, eventId):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier.
        """
        
        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self.isDown = False
        self.theButton = None


    def SetButtonObj(self, btn):
        """
        Sets the event object for the event.

        :param `btn`: the button object, an instance of L{GradientButton}.
        """
        
        self.theButton = btn


    def GetButtonObj(self):
        """ Returns the object associated with this event. """
        
        return self.theButton

    
class GradientButton(wx.PyControl):
    """ This is the main class implementation of L{GradientButton}. """
    
    def __init__(self, parent, id=wx.ID_ANY, bitmap=None, label="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.NO_BORDER, validator=wx.DefaultValidator,
                 name="gradientbutton"):
        """
        Default class constructor.

        :param `parent`: the L{GradientButton} parent;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `bitmap`: the button bitmap (if any);
        :param `label`: the button text label;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the button style (unused);
        :param `validator`: the validator associated to the button;
        :param `name`: the button name.
        """
        
        wx.PyControl.__init__(self, parent, id, pos, size, style, validator, name)

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, lambda event: None)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeave)
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnMouseEnter)
        self.Bind(wx.EVT_SET_FOCUS, self.OnGainFocus)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnLoseFocus)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)

        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDown)

        self._mouseAction = None
        self._bitmap = bitmap
        self._hasFocus = False
        
        self.SetLabel(label)
        self.InheritAttributes()
        self.SetInitialSize(size)

        # The following defaults are better suited to draw the text outline
        self._bottomStartColour = wx.BLACK
        rgba = self._bottomStartColour.Red(), self._bottomStartColour.Green(), \
               self._bottomStartColour.Blue(), self._bottomStartColour.Alpha()
        self._bottomEndColour = self.LightColour(self._bottomStartColour, 20)
        self._topStartColour = self.LightColour(self._bottomStartColour, 40)
        self._topEndColour = self.LightColour(self._bottomStartColour, 25)
        self._pressedTopColour = self.LightColour(self._bottomStartColour, 20)
        self._pressedBottomColour = wx.Colour(*rgba)
        self.SetForegroundColour(wx.WHITE)

        for method in dir(self):
            if method.endswith("Colour"):
                newMethod = method[0:-6] + "Colour"
                if not hasattr(self, newMethod):
                    setattr(self, newMethod, method)
        

    def LightColour(self, colour, percent):
        """
        Return light contrast of `colour`. The colour returned is from the scale of
        `colour` ==> white.

        :param `colour`: the input colour to be brightened;
        :param `percent`: determines how light the colour will be. `percent` = 100
         returns white, `percent` = 0 returns `colour`.
        """

        end_colour = wx.WHITE
        rd = end_colour.Red() - colour.Red()
        gd = end_colour.Green() - colour.Green()
        bd = end_colour.Blue() - colour.Blue()
        high = 100

        # We take the percent way of the colour from colour -. white
        i = percent
        r = colour.Red() + ((i*rd*100)/high)/100
        g = colour.Green() + ((i*gd*100)/high)/100
        b = colour.Blue() + ((i*bd*100)/high)/100

        return wx.Colour(r, g, b)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{GradientButton}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """
        
        event.Skip()
        self.Refresh()


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{GradientButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled():
            return
        
        self._mouseAction = CLICK
        self.CaptureMouse()
        self.Refresh()
        event.Skip()


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{GradientButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled() or not self.HasCapture():
            return
        
        pos = event.GetPosition()
        rect = self.GetClientRect()

        if self.HasCapture():
            self.ReleaseMouse()
            
        if rect.Contains(pos):
            self._mouseAction = HOVER
            self.Notify()
        else:
            self._mouseAction = None

        self.Refresh()
        event.Skip()


    def OnMouseEnter(self, event):
        """
        Handles the ``wx.EVT_ENTER_WINDOW`` event for L{GradientButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled():
            return
        
        self._mouseAction = HOVER
        self.Refresh()
        event.Skip()


    def OnMouseLeave(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{GradientButton}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self._mouseAction = None
        self.Refresh()
        event.Skip()


    def OnGainFocus(self, event):
        """
        Handles the ``wx.EVT_SET_FOCUS`` event for L{GradientButton}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """
        
        self._hasFocus = True
        self.Refresh()
        self.Update()


    def OnLoseFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{GradientButton}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self._hasFocus = False
        self.Refresh()
        self.Update()


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{GradientButton}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """
        
        if self._hasFocus and event.GetKeyCode() == ord(" "):
            self._mouseAction = HOVER
            self.Refresh()
        event.Skip()


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` event for L{GradientButton}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """
        
        if self._hasFocus and event.GetKeyCode() == ord(" "):
            self._mouseAction = HOVER
            self.Notify()
            self.Refresh()
        event.Skip()


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{GradientButton}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.BufferedPaintDC(self)
        gc = wx.GraphicsContext.Create(dc)
        dc.SetBackground(wx.Brush(self.GetParent().GetBackgroundColour()))        
        dc.Clear()
        
        clientRect = self.GetClientRect()
        gradientRect = wx.Rect(*clientRect)
        capture = wx.Window.GetCapture()

        x, y, width, height = clientRect        
        
        gradientRect.SetHeight(gradientRect.GetHeight()/2 + ((capture==self and [1] or [0])[0]))
        if capture != self:
            if self._mouseAction == HOVER:
                topStart, topEnd = self.LightColour(self._topStartColour, 10), self.LightColour(self._topEndColour, 10)
            else:
                topStart, topEnd = self._topStartColour, self._topEndColour

            rc1 = wx.Rect(x, y, width, height/2)
            path1 = self.GetPath(gc, rc1, 8)
            br1 = gc.CreateLinearGradientBrush(x, y, x, y+height/2, topStart, topEnd)
            gc.SetBrush(br1)
            gc.FillPath(path1) #draw main

            path4 = gc.CreatePath()
            path4.AddRectangle(x, y+height/2-8, width, 8)
            path4.CloseSubpath()
            gc.SetBrush(br1)
            gc.FillPath(path4)            
        
        else:
            
            rc1 = wx.Rect(x, y, width, height)
            path1 = self.GetPath(gc, rc1, 8)
            gc.SetPen(wx.Pen(self._pressedTopColour))
            gc.SetBrush(wx.Brush(self._pressedTopColour))
            gc.FillPath(path1)
        
        gradientRect.Offset((0, gradientRect.GetHeight()))

        if capture != self:

            if self._mouseAction == HOVER:
                bottomStart, bottomEnd = self.LightColour(self._bottomStartColour, 10), self.LightColour(self._bottomEndColour, 10)
            else:
                bottomStart, bottomEnd = self._bottomStartColour, self._bottomEndColour

            rc3 = wx.Rect(x, y+height/2, width, height/2)
            path3 = self.GetPath(gc, rc3, 8)
            br3 = gc.CreateLinearGradientBrush(x, y+height/2, x, y+height, bottomStart, bottomEnd)
            gc.SetBrush(br3)
            gc.FillPath(path3) #draw main

            path4 = gc.CreatePath()
            path4.AddRectangle(x, y+height/2, width, 8)
            path4.CloseSubpath()
            gc.SetBrush(br3)
            gc.FillPath(path4)
            
            shadowOffset = 0
        else:
        
            rc2 = wx.Rect(x+1, gradientRect.height/2, gradientRect.width, gradientRect.height)
            path2 = self.GetPath(gc, rc2, 8)
            gc.SetPen(wx.Pen(self._pressedBottomColour))
            gc.SetBrush(wx.Brush(self._pressedBottomColour))
            gc.FillPath(path2)
            shadowOffset = 1

        font = gc.CreateFont(self.GetFont(), self.GetForegroundColour())
        gc.SetFont(font)
        label = self.GetLabel()
        tw, th = gc.GetTextExtent(label)

        if self._bitmap:
            bw, bh = self._bitmap.GetWidth(), self._bitmap.GetHeight()
        else:
            bw = bh = 0
            
        pos_x = (width-bw-tw)/2+shadowOffset      # adjust for bitmap and text to centre        
        if self._bitmap:
            pos_y =  (height-bh)/2+shadowOffset
            gc.DrawBitmap(self._bitmap, pos_x, pos_y, bw, bh) # draw bitmap if available
            pos_x = pos_x + 2   # extra spacing from bitmap

        gc.DrawText(label, pos_x + bw + shadowOffset, (height-th)/2+shadowOffset) 

        
    def GetPath(self, gc, rc, r):
        """
        Returns a rounded `wx.GraphicsPath` rectangle.

        :param `gc`: an instance of `wx.GraphicsContext`;
        :param `rc`: a client rectangle;
        :param `r`: the radious of the rounded part of the rectangle.
        """
    
        x, y, w, h = rc
        path = gc.CreatePath()
        path.AddRoundedRectangle(x, y, w, h, r)
        path.CloseSubpath()
        return path

    
    def SetInitialSize(self, size=None):
        """
        Given the current font and bezel width settings, calculate
        and set a good size.

        :param `size`: an instance of `wx.Size`.        
        """
        
        if size is None:
            size = wx.DefaultSize            
        wx.PyControl.SetInitialSize(self, size)

    SetBestSize = SetInitialSize
    

    def AcceptsFocus(self):
        """
        Can this window be given focus by mouse click?

        :note: Overridden from `wx.PyControl`.
        """
        
        return self.IsShown() and self.IsEnabled()


    def GetDefaultAttributes(self):
        """
        Overridden base class virtual. By default we should use
        the same font/colour attributes as the native `wx.Button`.
        """
        
        return wx.Button.GetClassDefaultAttributes()


    def ShouldInheritColours(self):
        """
        Overridden base class virtual. Buttons usually don't inherit
        the parent's colours.

        :note: Overridden from `wx.PyControl`.
        """
        
        return False
    

    def Enable(self, enable=True):
        """
        Enables/disables the button.

        :param `enable`: ``True`` to enable the button, ``False`` to disable it.
        
        :note: Overridden from `wx.PyControl`.
        """
        
        wx.PyControl.Enable(self, enable)
        self.Refresh()


    def SetTopStartColour(self, colour):
        """
        Sets the top start colour for the gradient shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._topStartColour = colour
        self.Refresh()


    def GetTopStartColour(self):
        """ Returns the top start colour for the gradient shading. """

        return self._topStartColour
    

    def SetTopEndColour(self, colour):
        """
        Sets the top end colour for the gradient shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._topEndColour = colour
        self.Refresh()


    def GetTopEndColour(self):
        """ Returns the top end colour for the gradient shading. """

        return self._topEndColour
        

    def SetBottomStartColour(self, colour):
        """
        Sets the top bottom colour for the gradient shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._bottomStartColour = colour
        self.Refresh()


    def GetBottomStartColour(self):
        """ Returns the bottom start colour for the gradient shading. """

        return self._bottomStartColour
    
        
    def SetBottomEndColour(self, colour):
        """
        Sets the bottom end colour for the gradient shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._bottomEndColour = colour
        self.Refresh()


    def GetBottomEndColour(self):
        """ Returns the bottom end colour for the gradient shading. """

        return self._bottomEndColour
    

    def SetPressedTopColour(self, colour):
        """
        Sets the pressed top start colour for the gradient shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._pressedTopColour = colour
        self.Refresh()


    def GetPressedTopColour(self):
        """ Returns the pressed top start colour for the gradient shading. """

        return self._pressedTopColour
    
        
    def SetPressedBottomColour(self, colour):
        """
        Sets the pressed bottom start colour for the gradient shading.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._pressedBottomColour = colour
        self.Refresh()


    def GetPressedBottomColour(self):
        """ Returns the pressed bottom start colour for the gradient shading. """

        return self._pressedBottomColour
    

    def SetForegroundColour(self, colour):
        """
        Sets the L{GradientButton} foreground (text) colour.

        :param `colour`: a valid `wx.Colour` object.

        :note: Overridden from `wx.PyControl`.        
        """

        wx.PyControl.SetForegroundColour(self, colour)
        self.Refresh()
        
        
    def DoGetBestSize(self):
        """
        Overridden base class virtual. Determines the best size of the
        button based on the label and bezel size.
        """

        label = self.GetLabel()
        if not label:
            return wx.Size(112, 48)
        
        dc = wx.ClientDC(self)
        dc.SetFont(self.GetFont())
        retWidth, retHeight = dc.GetTextExtent(label)
        
        bmpWidth = bmpHeight = 0
        constant = 15
        if self._bitmap:
            bmpWidth, bmpHeight = self._bitmap.GetWidth()+10, self._bitmap.GetHeight()
            retWidth += bmpWidth
            retHeight = max(bmpHeight, retHeight)
            constant = 15

        return wx.Size(retWidth+constant, retHeight+constant) 


    def SetDefault(self):
        """ Sets the default button. """
        
        tlw = wx.GetTopLevelParent(self)
        if hasattr(tlw, 'SetDefaultItem'):
            tlw.SetDefaultItem(self)
        

    def Notify(self):
        """ Actually sends a ``wx.EVT_BUTTON`` event to the listener (if any). """
        
        evt = GradientButtonEvent(wx.wxEVT_COMMAND_BUTTON_CLICKED, self.GetId())
        evt.SetButtonObj(self)
        evt.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(evt)


