###############################################################################
# Name: platebtn.py                                                           #
# Purpose: PlateButton is a flat label button with support for bitmaps and    #
#          drop menu.                                                         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Control Library: PlateButton

The PlateButton is a custom owner drawn flat button, that in many ways emulates
the buttons found the bookmark bar of the Safari browser. It can be used as a 
drop in replacement for wx.Button/wx.BitmapButton under most circumstances. It 
also offers a wide range of options for customizing its appearance, a 
description of each of the main style settings is listed below.

Main Button Styles:
Any combination of the following values may be passed to the constructor's style
keyword parameter.

PB_STYLE_DEFAULT:
Creates a flat label button with rounded corners, the highlight for mouse over
and press states is based off of the hightlight color from the systems current
theme.

PB_STYLE_GRADIENT:
The highlight and press states are drawn with gradient using the current
highlight color.

PB_STYLE_SQUARE:
Instead of the default rounded shape use a rectangular shaped button with
square edges.

PB_STYLE_NOBG:
This style only has an effect on Windows but does not cause harm to use on the
platforms. It should only be used when the control is shown on a panel or other
window that has a non solid color for a background. i.e a gradient or image is
painted on the background of the parent window. If used on a background with
a solid color it may cause the control to loose its transparent appearance.

PB_STYLE_DROPARROW:
Add a drop button arrow to the button that will send a separate event when
clicked on.

Other attributes can be configured after the control has been created. The
settings that are currently available are as follows:

  - SetBitmap: Change/Add the bitmap at any time and the control will resize and
               refresh to display it.
  - SetLabelColor: Explicitly set text colors
  - SetMenu: Set the button to have a popupmenu. When a menu is set a small drop
             arrow will be drawn on the button that can then be clicked to show
             a menu.
  - SetPressColor: Use a custom highlight color


Overridden Methods Inherited from PyControl:

  - SetFont: Changing the font is one way to set the size of the button, by
             default the control will inherit its font from its parent.

  - SetWindowVariant: Setting the window variant will cause the control to
                      resize to the corresponding variant size. However if the
                      button is using a bitmap the bitmap will remain unchanged
                      and only the font will be adjusted.

Requirements:
  - python2.4 or higher
  - wxPython2.8 or higher

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: platebtn.py 63348 2010-02-01 22:01:17Z CJP $"
__revision__ = "$Revision: 63348 $"

__all__ = ["PlateButton",
           "PLATE_NORMAL", "PLATE_PRESSED", "PLATE_HIGHLIGHT", 

           "PB_STYLE_DEFAULT", "PB_STYLE_GRADIENT", "PB_STYLE_SQUARE",
           "PB_STYLE_NOBG", "PB_STYLE_DROPARROW", "PB_STYLE_TOGGLE",

           "EVT_PLATEBTN_DROPARROW_PRESSED"]

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.lib.newevent

# Used on OSX to get access to carbon api constants
if wx.Platform == '__WXMAC__':
    import Carbon.Appearance

#-----------------------------------------------------------------------------#
# Button States
PLATE_NORMAL = 0
PLATE_PRESSED = 1
PLATE_HIGHLIGHT = 2

# Button Styles
PB_STYLE_DEFAULT  = 1   # Normal Flat Background
PB_STYLE_GRADIENT = 2   # Gradient Filled Background
PB_STYLE_SQUARE   = 4   # Use square corners instead of rounded
PB_STYLE_NOBG     = 8   # Usefull on Windows to get a transparent appearance
                        # when the control is shown on a non solid background
PB_STYLE_DROPARROW = 16 # Draw drop arrow and fire EVT_PLATEBTN_DROPRROW_PRESSED event
PB_STYLE_TOGGLE   = 32  # Stay pressed untill clicked again

#-----------------------------------------------------------------------------#

# EVT_BUTTON used for normal event notification
# EVT_TOGGLE_BUTTON used for toggle button mode notification
PlateBtnDropArrowPressed, EVT_PLATEBTN_DROPARROW_PRESSED = wx.lib.newevent.NewEvent()

#-----------------------------------------------------------------------------#
# Utility Functions, moved to their own module

from wx.lib.colourutils import *

#-----------------------------------------------------------------------------#

class PlateButton(wx.PyControl):
    """PlateButton is a custom type of flat button with support for
    displaying bitmaps and having an attached dropdown menu.

    """
    def __init__(self, parent, id_=wx.ID_ANY, label='', bmp=None, 
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=PB_STYLE_DEFAULT, name=wx.ButtonNameStr):
        """Create a PlateButton
        @keyword label: Buttons label text
        @keyword bmp: Buttons bitmap
        @keyword style: Button style

        """
        wx.PyControl.__init__(self, parent, id_, pos, size,
                              wx.BORDER_NONE|wx.TRANSPARENT_WINDOW, name=name)

        # Attributes
        self.InheritAttributes()
        self._bmp = dict(enable=bmp)
        if bmp is not None:
            img = bmp.ConvertToImage()
            img = img.ConvertToGreyscale(.795, .073, .026) #(.634, .224, .143)
            self._bmp['disable'] = img.ConvertToBitmap()
        else:
            self._bmp['disable'] = None

        self._menu = None
        self.SetLabel(label)
        self._style = style
        self._state = dict(pre=PLATE_NORMAL, cur=PLATE_NORMAL)
        self._color = self.__InitColors()
        self._pressed = False

        # Setup Initial Size
        self.SetInitialSize()

        # Event Handlers
        self.Bind(wx.EVT_PAINT, lambda evt: self.__DrawButton())
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnErase)
        self.Bind(wx.EVT_SET_FOCUS, self.OnFocus)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)

        # Mouse Events
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_LEFT_DCLICK, lambda evt: self.ToggleState())
        self.Bind(wx.EVT_ENTER_WINDOW,
                  lambda evt: self.SetState(PLATE_HIGHLIGHT))
        self.Bind(wx.EVT_LEAVE_WINDOW,
                  lambda evt: wx.CallLater(80, self.__LeaveWindow))

        # Other events
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self.Bind(wx.EVT_CONTEXT_MENU, lambda evt: self.ShowMenu())

    def __DrawBitmap(self, gc):
        """Draw the bitmap if one has been set
        @param gc: GCDC to draw with
        @return: x cordinate to draw text at

        """
        if self.IsEnabled():
            bmp = self._bmp['enable']
        else:
            bmp = self._bmp['disable']

        if bmp is not None and bmp.IsOk():
            bw, bh = bmp.GetSize()
            ypos = (self.GetSize()[1] - bh) / 2
            gc.DrawBitmap(bmp, 6, ypos, bmp.GetMask() != None)
            return bw + 6
        else:
            return 6

    def __DrawDropArrow(self, gc, xpos, ypos):
        """Draw a drop arrow if needed and restore pen/brush after finished
        @param gc: GCDC to draw with
        @param xpos: x cord to start at
        @param ypos: y cord to start at

        """
        if self._menu is not None or self._style & PB_STYLE_DROPARROW:
            # Positioning needs a little help on Windows
            if wx.Platform == '__WXMSW__':
                xpos -= 2
            tripoints = [(xpos, ypos), (xpos + 6, ypos), (xpos + 3, ypos + 5)]
            brush_b = gc.GetBrush()
            pen_b = gc.GetPen()
            gc.SetPen(wx.TRANSPARENT_PEN)
            gc.SetBrush(wx.Brush(gc.GetTextForeground()))
            gc.DrawPolygon(tripoints)
            gc.SetBrush(brush_b)
            gc.SetPen(pen_b)
        else:
            pass

    def __DrawHighlight(self, gc, width, height):
        """Draw the main highlight/pressed state
        @param gc: GCDC to draw with
        @param width: width of highlight
        @param height: height of highlight

        """
        if self._state['cur'] == PLATE_PRESSED:
            color = self._color['press']
        else:
            color = self._color['hlight']

        if self._style & PB_STYLE_SQUARE:
            rad = 0
        else:
            rad = (height - 3) / 2

        if self._style & PB_STYLE_GRADIENT:
            gc.SetBrush(wx.TRANSPARENT_BRUSH)
            rgc = gc.GetGraphicsContext()
            brush = rgc.CreateLinearGradientBrush(0, 1, 0, height,
                                                  color, AdjustAlpha(color, 55))
            rgc.SetBrush(brush)
        else:
            gc.SetBrush(wx.Brush(color))

        gc.DrawRoundedRectangle(1, 1, width - 2, height - 2, rad)

    def __PostEvent(self):
        """Post a button event to parent of this control"""
        if self._style & PB_STYLE_TOGGLE:
            etype = wx.wxEVT_COMMAND_TOGGLEBUTTON_CLICKED
        else:
            etype = wx.wxEVT_COMMAND_BUTTON_CLICKED
        bevt = wx.CommandEvent(etype, self.GetId())
        bevt.SetEventObject(self)
        bevt.SetString(self.GetLabel())
        self.GetEventHandler().ProcessEvent(bevt)

    def __DrawButton(self):
        """Draw the button"""
        # TODO using a buffered paintdc on windows with the nobg style
        #      causes lots of weird drawing. So currently the use of a
        #      buffered dc is dissabled for this style.
        if PB_STYLE_NOBG & self._style:
            dc = wx.PaintDC(self)
        else:
            dc = wx.AutoBufferedPaintDCFactory(self)

        gc = wx.GCDC(dc)

        # Setup
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        gc.SetBrush(wx.TRANSPARENT_BRUSH)
        gc.SetFont(self.GetFont())
        gc.SetBackgroundMode(wx.TRANSPARENT)

        # The background needs some help to look transparent on
        # on Gtk and Windows
        if wx.Platform in ['__WXGTK__', '__WXMSW__']:
            gc.SetBackground(self.GetBackgroundBrush(gc))
            gc.Clear()

        # Calc Object Positions
        width, height = self.GetSize()
        tw, th = gc.GetTextExtent(self.GetLabel())
        txt_y = max((height - th) / 2, 1)

        if self._state['cur'] == PLATE_HIGHLIGHT:
            gc.SetTextForeground(self._color['htxt'])
            gc.SetPen(wx.TRANSPARENT_PEN)
            self.__DrawHighlight(gc, width, height)

        elif self._state['cur'] == PLATE_PRESSED:
            gc.SetTextForeground(self._color['htxt'])
            if wx.Platform == '__WXMAC__':
                pen = wx.Pen(GetHighlightColour(), 1, wx.SOLID)
            else:
                pen = wx.Pen(AdjustColour(self._color['press'], -80, 220), 1)
            gc.SetPen(pen)

            self.__DrawHighlight(gc, width, height)
            txt_x = self.__DrawBitmap(gc)
            gc.DrawText(self.GetLabel(), txt_x + 2, txt_y)
            self.__DrawDropArrow(gc, txt_x + tw + 6, (height / 2) - 2)

        else:
            if self.IsEnabled():
                gc.SetTextForeground(self.GetForegroundColour())
            else:
                txt_c = wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT)
                gc.SetTextForeground(txt_c)

        # Draw bitmap and text
        if self._state['cur'] != PLATE_PRESSED:
            txt_x = self.__DrawBitmap(gc)
            gc.DrawText(self.GetLabel(), txt_x + 2, txt_y)
            self.__DrawDropArrow(gc, txt_x + tw + 6, (height / 2) - 2)

    def __InitColors(self):
        """Initialize the default colors"""
        color = GetHighlightColour()
        pcolor = AdjustColour(color, -12)
        colors = dict(default=True,
                      hlight=color, 
                      press=pcolor,
                      htxt=BestLabelColour(self.GetForegroundColour()))
        return colors

    def __LeaveWindow(self):
        """Handle updating the buttons state when the mouse cursor leaves"""
        if (self._style & PB_STYLE_TOGGLE) and self._pressed:
            self.SetState(PLATE_PRESSED) 
        else:
            self.SetState(PLATE_NORMAL)

    #---- End Private Member Function ----#

    #---- Public Member Functions ----#
    def AcceptsFocus(self):
        """Can this window have the focus?"""
        return self.IsEnabled()

    @property
    def BitmapDisabled(self):
        """Property for accessing the bitmap for the disabled state"""
        return self._bmp['disable']

    @property
    def BitmapLabel(self):
        """Property for accessing the default bitmap"""
        return self._bmp['enable']

    # Aliases
    BitmapFocus = BitmapLabel
    BitmapHover = BitmapLabel
    BitmapSelected = BitmapLabel

    def Disable(self):
        """Disable the control"""
        super(PlateButton, self).Disable()
        self.Refresh()

    def DoGetBestSize(self):
        """Calculate the best size of the button
        @return: wx.Size

        """
        width = 4
        height = 6
        if self.GetLabel():
            lsize = self.GetTextExtent(self.GetLabel())
            width += lsize[0]
            height += lsize[1]
            
        if self._bmp['enable'] is not None:
            bsize = self._bmp['enable'].GetSize()
            width += (bsize[0] + 10)
            if height <= bsize[1]:
                height = bsize[1] + 6
            else:
                height += 3
        else:
            width += 10

        if self._menu is not None or self._style & PB_STYLE_DROPARROW:
            width += 12

        best = wx.Size(width, height)
        self.CacheBestSize(best)
        return best

    def Enable(self, enable=True):
        """Enable/Disable the control"""
        super(PlateButton, self).Enable(enable)
        self.Refresh()

    def GetBackgroundBrush(self, dc):
        """Get the brush for drawing the background of the button
        @return: wx.Brush
        @note: used internally when on gtk

        """
        if wx.Platform == '__WXMAC__' or self._style & PB_STYLE_NOBG:
            return wx.TRANSPARENT_BRUSH

        bkgrd = self.GetBackgroundColour()
        brush = wx.Brush(bkgrd, wx.SOLID)
        my_attr = self.GetDefaultAttributes()
        p_attr = self.GetParent().GetDefaultAttributes()
        my_def = bkgrd == my_attr.colBg
        p_def = self.GetParent().GetBackgroundColour() == p_attr.colBg
        if my_def and not p_def:
            bkgrd = self.GetParent().GetBackgroundColour()
            brush = wx.Brush(bkgrd, wx.SOLID)
        return brush

    def GetBitmapDisabled(self):
        """Get the bitmap of the disable state
        @return: wx.Bitmap or None

        """
        return self._bmp['disable']

    def GetBitmapLabel(self):
        """Get the label bitmap
        @return: wx.Bitmap or None

        """
        return self._bmp['enable']

    # GetBitmap Aliases for BitmapButton api
    GetBitmapFocus = GetBitmapLabel
    GetBitmapHover = GetBitmapLabel
    
    # Alias for GetLabel
    GetLabelText = wx.PyControl.GetLabel

    def GetMenu(self):
        """Return the menu associated with this button or None if no
        menu is associated with it.

        """
        return getattr(self, '_menu', None)

    def GetState(self):
        """Get the current state of the button
        @return: int
        @see: PLATE_NORMAL, PLATE_HIGHLIGHT, PLATE_PRESSED

        """
        return self._state['cur']

    def HasTransparentBackground(self):
        """Override setting of background fill"""
        return True

    def IsPressed(self):
        """Return if button is pressed (PB_STYLE_TOGGLE)
        @return: bool

        """
        return self._pressed

    @property
    def LabelText(self):
        """Property for getting the label of the button"""
        return self.GetLabel()

    #---- Event Handlers ----#

    def OnErase(self, evt):
        """Trap the erase event to keep the background transparent
        on windows.
        @param evt: wx.EVT_ERASE_BACKGROUND

        """
        pass

    def OnFocus(self, evt):
        """Set the visual focus state if need be"""
        if self._state['cur'] == PLATE_NORMAL:
            self.SetState(PLATE_HIGHLIGHT)

    def OnKeyUp(self, evt):
        """Execute a single button press action when the Return key is pressed
        and this control has the focus.
        @param evt: wx.EVT_KEY_UP

        """
        if evt.GetKeyCode() == wx.WXK_SPACE:
            self.SetState(PLATE_PRESSED)
            self.__PostEvent()
            wx.CallLater(100, self.SetState, PLATE_HIGHLIGHT)
        else:
            evt.Skip()

    def OnKillFocus(self, evt):
        """Set the visual state back to normal when focus is lost
        unless the control is currently in a pressed state.

        """
        # Note: this delay needs to be at least as much as the on in the KeyUp
        #       handler to prevent ghost highlighting from happening when
        #       quickly changing focus and activating buttons
        if self._state['cur'] != PLATE_PRESSED:
            self.SetState(PLATE_NORMAL)

    def OnLeftDown(self, evt):
        """Sets the pressed state and depending on the click position will
        show the popup menu if one has been set.

        """
        if (self._style & PB_STYLE_TOGGLE):
            self._pressed = not self._pressed

        pos = evt.GetPositionTuple()
        self.SetState(PLATE_PRESSED)
        size = self.GetSizeTuple()
        if pos[0] >= size[0] - 16:
            if self._menu is not None:
                self.ShowMenu()
            elif self._style & PB_STYLE_DROPARROW:
                event = PlateBtnDropArrowPressed()
                event.SetEventObject(self)
                wx.PostEvent(self, event)
        
        self.SetFocus()

    def OnLeftUp(self, evt):
        """Post a button event if the control was previously in a
        pressed state.
        @param evt: wx.MouseEvent

        """
        if self._state['cur'] == PLATE_PRESSED:
            pos = evt.GetPositionTuple()
            size = self.GetSizeTuple()
            if not (self._style & PB_STYLE_DROPARROW and pos[0] >= size[0] - 16):
                self.__PostEvent()

        if self._pressed:
            self.SetState(PLATE_PRESSED)
        else:
            self.SetState(PLATE_HIGHLIGHT)

    def OnMenuClose(self, evt):
        """Refresh the control to a proper state after the menu has been
        dismissed.
        @param evt: wx.EVT_MENU_CLOSE

        """
        mpos = wx.GetMousePosition()
        if self.HitTest(self.ScreenToClient(mpos)) != wx.HT_WINDOW_OUTSIDE:
            self.SetState(PLATE_HIGHLIGHT)
        else:
            self.SetState(PLATE_NORMAL)
        evt.Skip()

    #---- End Event Handlers ----#

    def SetBitmap(self, bmp):
        """Set the bitmap displayed in the button
        @param bmp: wx.Bitmap

        """
        self._bmp['enable'] = bmp
        img = bmp.ConvertToImage()
        img = img.ConvertToGreyscale(.795, .073, .026) #(.634, .224, .143)
        self._bmp['disable'] = img.ConvertToBitmap()
        self.InvalidateBestSize()

    def SetBitmapDisabled(self, bmp):
        """Set the bitmap for the disabled state
        @param bmp: wx.Bitmap

        """
        self._bmp['disable'] = bmp

    # Aliases for SetBitmap* functions from BitmapButton
    SetBitmapFocus = SetBitmap
    SetBitmapHover = SetBitmap
    SetBitmapLabel = SetBitmap
    SetBitmapSelected = SetBitmap

    def SetFocus(self):
        """Set this control to have the focus"""
        if self._state['cur'] != PLATE_PRESSED:
            self.SetState(PLATE_HIGHLIGHT)
        super(PlateButton, self).SetFocus()

    def SetFont(self, font):
        """Adjust size of control when font changes"""
        super(PlateButton, self).SetFont(font)
        self.InvalidateBestSize()

    def SetLabel(self, label):
        """Set the label of the button
        @param label: lable string

        """
        super(PlateButton, self).SetLabel(label)
        self.InvalidateBestSize()

    def SetLabelColor(self, normal, hlight=wx.NullColour):
        """Set the color of the label. The optimal label color is usually
        automatically selected depending on the button color. In some
        cases the colors that are choosen may not be optimal.
        
        The normal state must be specified, if the other two params are left
        Null they will be automatically guessed based on the normal color. To
        prevent this automatic color choices from happening either specify
        a color or None for the other params.

        @param normal: Label color for normal state
        @keyword hlight: Color for when mouse is hovering over

        """
        self._color['default'] = False
        self.SetForegroundColour(normal)

        if hlight is not None:
            if hlight.IsOk():
                self._color['htxt'] = hlight
            else:
                self._color['htxt'] = BestLabelColour(normal)

        if wx.Platform == '__WXMSW__':
            self.GetParent().RefreshRect(self.GetRect(), False)
        else:
            self.Refresh()

    def SetMenu(self, menu):
        """Set the menu that can be shown when clicking on the
        drop arrow of the button.
        @param menu: wxMenu to use as a PopupMenu
        @note: Arrow is not drawn unless a menu is set

        """
        if self._menu is not None:
            self.Unbind(wx.EVT_MENU_CLOSE)

        self._menu = menu
        self.Bind(wx.EVT_MENU_CLOSE, self.OnMenuClose)
        self.InvalidateBestSize()

    def SetPressColor(self, color):
        """Set the color used for highlighting the pressed state
        @param color: wx.Color
        @note: also resets all text colours as necessary

        """
        self._color['default'] = False
        if color.Alpha() == 255:
            self._color['hlight'] = AdjustAlpha(color, 200)
        else:
            self._color['hlight'] = color
        self._color['press'] = AdjustColour(color, -10, 160)
        self._color['htxt'] = BestLabelColour(self._color['hlight'])
        self.Refresh()

    def SetState(self, state):
        """Manually set the state of the button
        @param state: one of the PLATE_* values
        @note: the state may be altered by mouse actions

        """
        self._state['pre'] = self._state['cur']
        self._state['cur'] = state
        if wx.Platform == '__WXMSW__':
            self.GetParent().RefreshRect(self.GetRect(), False)
        else:
            self.Refresh()

    def SetWindowStyle(self, style):
        """Sets the window style bytes, the updates take place
        immediately no need to call refresh afterwards.
        @param style: bitmask of PB_STYLE_* values

        """
        self._style = style
        self.Refresh()

    def SetWindowVariant(self, variant):
        """Set the variant/font size of this control"""
        super(PlateButton, self).SetWindowVariant(variant)
        self.InvalidateBestSize()

    def ShouldInheritColours(self):
        """Overridden base class virtual. If the parent has non-default
        colours then we want this control to inherit them.

        """
        return True

    def ShowMenu(self):
        """Show the dropdown menu if one is associated with this control"""
        if self._menu is not None:
            size = self.GetSizeTuple()
            adj = wx.Platform == '__WXMAC__' and 3 or 0

            if self._style & PB_STYLE_SQUARE:
                xpos = 1
            else:
                xpos = size[1] / 2

            self.PopupMenu(self._menu, (xpos, size[1] + adj))

    def ToggleState(self):
        """Toggle button state"""
        if self._state['cur'] != PLATE_PRESSED:
            self.SetState(PLATE_PRESSED)
        else:
            self.SetState(PLATE_HIGHLIGHT)

    #---- End Public Member Functions ----#
