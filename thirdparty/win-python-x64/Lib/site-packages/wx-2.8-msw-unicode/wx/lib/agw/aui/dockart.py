"""
Dock art provider code - a dock provider provides all drawing functionality to
the AUI dock manager. This allows the dock manager to have a plugable look-and-feel.

By default, a L{AuiManager} uses an instance of this class called L{AuiDefaultDockArt}
which provides bitmap art and a colour scheme that is adapted to the major platforms'
look. You can either derive from that class to alter its behaviour or write a
completely new dock art class. Call L{AuiManager.SetArtProvider} to make use this
new dock art.
"""

__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx
import types

from aui_utilities import BitmapFromBits, StepColour, ChopText, GetBaseColour
from aui_utilities import DrawGradientRectangle, DrawMACCloseButton
from aui_utilities import DarkenBitmap, LightContrastColour
from aui_constants import *

optionActive = 2**14

_ctypes = False

# Try to import winxptheme for ModernDockArt
if wx.Platform == "__WXMSW__":
    try:
        import ctypes
        import winxptheme
        _ctypes = True
    except ImportError:
        pass

# -- AuiDefaultDockArt class implementation --

class AuiDefaultDockArt(object):
    """
    Dock art provider code - a dock provider provides all drawing functionality
    to the AUI dock manager. This allows the dock manager to have a plugable
    look-and-feel.

    By default, a L{AuiManager} uses an instance of this class called L{AuiDefaultDockArt}
    which provides bitmap art and a colour scheme that is adapted to the major
    platforms' look. You can either derive from that class to alter its behaviour or
    write a completely new dock art class.
    
    Call L{AuiManager.SetArtProvider} to make use this new dock art.


    **Metric Ordinals**

    These are the possible pane dock art settings for L{AuiManager}:

    ================================================  ======================================
    Metric Ordinal Constant                           Description
    ================================================  ======================================
    ``AUI_DOCKART_SASH_SIZE``                         Customizes the sash size
    ``AUI_DOCKART_CAPTION_SIZE``                      Customizes the caption size
    ``AUI_DOCKART_GRIPPER_SIZE``                      Customizes the gripper size
    ``AUI_DOCKART_PANE_BORDER_SIZE``                  Customizes the pane border size
    ``AUI_DOCKART_PANE_BUTTON_SIZE``                  Customizes the pane button size
    ``AUI_DOCKART_BACKGROUND_COLOUR``                 Customizes the background colour
    ``AUI_DOCKART_BACKGROUND_GRADIENT_COLOUR``        Customizes the background gradient colour
    ``AUI_DOCKART_SASH_COLOUR``                       Customizes the sash colour
    ``AUI_DOCKART_ACTIVE_CAPTION_COLOUR``             Customizes the active caption colour
    ``AUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR``    Customizes the active caption gradient colour
    ``AUI_DOCKART_INACTIVE_CAPTION_COLOUR``           Customizes the inactive caption colour
    ``AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR``  Customizes the inactive gradient caption colour
    ``AUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR``        Customizes the active caption text colour
    ``AUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR``      Customizes the inactive caption text colour
    ``AUI_DOCKART_BORDER_COLOUR``                     Customizes the border colour
    ``AUI_DOCKART_GRIPPER_COLOUR``                    Customizes the gripper colour
    ``AUI_DOCKART_CAPTION_FONT``                      Customizes the caption font
    ``AUI_DOCKART_GRADIENT_TYPE``                     Customizes the gradient type (no gradient, vertical or horizontal)
    ``AUI_DOCKART_DRAW_SASH_GRIP``                    Draw a sash grip on the sash
    ================================================  ======================================


    **Gradient Types**

    These are the possible gradient dock art settings for L{AuiManager}:

    ============================================  ======================================
    Gradient Constant                             Description 
    ============================================  ======================================
    ``AUI_GRADIENT_NONE``                         No gradient on the captions
    ``AUI_GRADIENT_VERTICAL``                     Vertical gradient on the captions
    ``AUI_GRADIENT_HORIZONTAL``                   Horizontal gradient on the captions
    ============================================  ======================================


    **Button States**

    These are the possible pane button / L{AuiNotebook} button / L{AuiToolBar} button states:

    ============================================  ======================================
    Button State Constant                         Description     
    ============================================  ======================================
    ``AUI_BUTTON_STATE_NORMAL``                   Normal button state
    ``AUI_BUTTON_STATE_HOVER``                    Hovered button state
    ``AUI_BUTTON_STATE_PRESSED``                  Pressed button state
    ``AUI_BUTTON_STATE_DISABLED``                 Disabled button state
    ``AUI_BUTTON_STATE_HIDDEN``                   Hidden button state
    ``AUI_BUTTON_STATE_CHECKED``                  Checked button state
    ============================================  ======================================


    **Button Identifiers**

    These are the possible pane button / L{AuiNotebook} button / L{AuiToolBar} button identifiers:

    ============================================  ======================================
    Button Identifier                             Description     
    ============================================  ======================================
    ``AUI_BUTTON_CLOSE``                          Shows a close button on the pane
    ``AUI_BUTTON_MAXIMIZE_RESTORE``               Shows a maximize/restore button on the pane
    ``AUI_BUTTON_MINIMIZE``                       Shows a minimize button on the pane
    ``AUI_BUTTON_PIN``                            Shows a pin button on the pane
    ``AUI_BUTTON_OPTIONS``                        Shows an option button on the pane (not implemented)
    ``AUI_BUTTON_WINDOWLIST``                     Shows a window list button on the pane (for L{AuiNotebook})
    ``AUI_BUTTON_LEFT``                           Shows a left button on the pane (for L{AuiNotebook})
    ``AUI_BUTTON_RIGHT``                          Shows a right button on the pane (for L{AuiNotebook})
    ``AUI_BUTTON_UP``                             Shows an up button on the pane (not implemented)
    ``AUI_BUTTON_DOWN``                           Shows a down button on the pane (not implemented)
    ``AUI_BUTTON_CUSTOM1``                        Shows a custom button on the pane (not implemented)
    ``AUI_BUTTON_CUSTOM2``                        Shows a custom button on the pane (not implemented)
    ``AUI_BUTTON_CUSTOM3``                        Shows a custom button on the pane (not implemented)
    ============================================  ======================================
    
    """

    def __init__(self):
        """ Default class constructor. """

        self.Init()

        isMac = wx.Platform == "__WXMAC__"
        
        if isMac:
            self._caption_font = wx.SMALL_FONT
        else:
            self._caption_font = wx.Font(8, wx.DEFAULT, wx.NORMAL, wx.NORMAL, False)

        self.SetDefaultPaneBitmaps(isMac)
        self._restore_bitmap = wx.BitmapFromXPMData(restore_xpm)
        
        # default metric values
        self._sash_size = 4

        if isMac:
            # This really should be implemented in wx.SystemSettings
            # There is no way to do this that I am aware outside of using
            # the cocoa python bindings. 8 pixels looks correct on my system
            # so hard coding it for now.

            # How do I translate this?!? Not sure of the below implementation...
            # SInt32 height;
            # GetThemeMetric( kThemeMetricSmallPaneSplitterHeight , &height );
            # self._sash_size = height;

            self._sash_size = 8 # Carbon.Appearance.kThemeMetricPaneSplitterHeight            
            
        elif wx.Platform == "__WXGTK__":
            self._sash_size = wx.RendererNative.Get().GetSplitterParams(wx.GetTopLevelWindows()[0]).widthSash

        else:
            self._sash_size = 4
        
        self._caption_size = 19
        self._border_size = 1
        self._button_size = 14
        self._gripper_size = 9
        self._gradient_type = AUI_GRADIENT_VERTICAL
        self._draw_sash = False
        

    def Init(self):
        """ Initializes the dock art. """

        base_colour = GetBaseColour()
        darker1_colour = StepColour(base_colour, 85)
        darker2_colour = StepColour(base_colour, 75)
        darker3_colour = StepColour(base_colour, 60)
        darker4_colour = StepColour(base_colour, 40)

        self._background_colour = base_colour
        self._background_gradient_colour = StepColour(base_colour, 180)

        isMac = wx.Platform == "__WXMAC__"

        if isMac:
            self._active_caption_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        else:
            self._active_caption_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_ACTIVECAPTION)

        self._active_caption_gradient_colour = LightContrastColour(wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT))
        self._active_caption_text_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT)
        self._inactive_caption_colour = darker1_colour
        self._inactive_caption_gradient_colour = StepColour(base_colour, 97)
        self._inactive_caption_text_colour = wx.BLACK
    
        self._sash_brush = wx.Brush(base_colour)
        self._background_brush = wx.Brush(base_colour)
        self._border_pen = wx.Pen(darker2_colour)
        self._gripper_brush = wx.Brush(base_colour)
        self._gripper_pen1 = wx.Pen(darker4_colour)
        self._gripper_pen2 = wx.Pen(darker3_colour)
        self._gripper_pen3 = wx.WHITE_PEN

        
    def GetMetric(self, id):
        """
        Gets the value of a certain setting.

        :param `id`: can be one of the size values in `Metric Ordinals`.
        """


        if id == AUI_DOCKART_SASH_SIZE:
            return self._sash_size
        elif id == AUI_DOCKART_CAPTION_SIZE:
            return self._caption_size
        elif id == AUI_DOCKART_GRIPPER_SIZE:
            return self._gripper_size
        elif id == AUI_DOCKART_PANE_BORDER_SIZE:
            return self._border_size
        elif id == AUI_DOCKART_PANE_BUTTON_SIZE:
            return self._button_size
        elif id == AUI_DOCKART_GRADIENT_TYPE:
            return self._gradient_type
        elif id == AUI_DOCKART_DRAW_SASH_GRIP:
            return self._draw_sash
        else:
            raise Exception("Invalid Metric Ordinal.")


    def SetMetric(self, id, new_val):
        """
        Sets the value of a certain setting using `new_val`

        :param `id`: can be one of the size values in `Metric Ordinals`;
        :param `new_val`: the new value of the setting.
        """

        if id == AUI_DOCKART_SASH_SIZE:
            self._sash_size = new_val
        elif id == AUI_DOCKART_CAPTION_SIZE:
            self._caption_size = new_val
        elif id == AUI_DOCKART_GRIPPER_SIZE:
            self._gripper_size = new_val
        elif id == AUI_DOCKART_PANE_BORDER_SIZE:
            self._border_size = new_val
        elif id == AUI_DOCKART_PANE_BUTTON_SIZE:
            self._button_size = new_val
        elif id == AUI_DOCKART_GRADIENT_TYPE:
            self._gradient_type = new_val
        elif id == AUI_DOCKART_DRAW_SASH_GRIP:
            self._draw_sash = new_val
        else:
            raise Exception("Invalid Metric Ordinal.")


    def GetColor(self, id):
        """
        Gets the colour of a certain setting.

        :param `id`: can be one of the colour values in `Metric Ordinals`.
        """

        if id == AUI_DOCKART_BACKGROUND_COLOUR:
            return self._background_brush.GetColour()
        elif id == AUI_DOCKART_BACKGROUND_GRADIENT_COLOUR:
            return self._background_gradient_colour
        elif id == AUI_DOCKART_SASH_COLOUR:
            return self._sash_brush.GetColour()
        elif id == AUI_DOCKART_INACTIVE_CAPTION_COLOUR:
            return self._inactive_caption_colour
        elif id == AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR:
            return self._inactive_caption_gradient_colour
        elif id == AUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR:
            return self._inactive_caption_text_colour
        elif id == AUI_DOCKART_ACTIVE_CAPTION_COLOUR:
            return self._active_caption_colour
        elif id == AUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR:
            return self._active_caption_gradient_colour
        elif id == AUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR:
            return self._active_caption_text_colour        
        elif id == AUI_DOCKART_BORDER_COLOUR:
            return self._border_pen.GetColour()
        elif id == AUI_DOCKART_GRIPPER_COLOUR:
            return self._gripper_brush.GetColour()
        else:
            raise Exception("Invalid Colour Ordinal.")


    def SetColor(self, id, colour):
        """
        Sets the colour of a certain setting.

        :param `id`: can be one of the colour values in `Metric Ordinals`;
        :param `colour`: the new value of the setting.
        """

        if isinstance(colour, basestring):
            colour = wx.NamedColour(colour)
        elif isinstance(colour, types.TupleType):
            colour = wx.Colour(*colour)
        elif isinstance(colour, types.IntType):
            colour = wx.ColourRGB(colour)
        
        if id == AUI_DOCKART_BACKGROUND_COLOUR:
            self._background_brush.SetColour(colour)
        elif id == AUI_DOCKART_BACKGROUND_GRADIENT_COLOUR:
            self._background_gradient_colour = colour
        elif id == AUI_DOCKART_SASH_COLOUR:
            self._sash_brush.SetColour(colour)
        elif id == AUI_DOCKART_INACTIVE_CAPTION_COLOUR:
            self._inactive_caption_colour = colour
            if not self._custom_pane_bitmaps and wx.Platform == "__WXMAC__":
                # No custom bitmaps for the pane close button
                # Change the MAC close bitmap colour
                self._inactive_close_bitmap = DrawMACCloseButton(wx.WHITE, colour)

        elif id == AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR:
            self._inactive_caption_gradient_colour = colour
        elif id == AUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR:
            self._inactive_caption_text_colour = colour
        elif id == AUI_DOCKART_ACTIVE_CAPTION_COLOUR:
            self._active_caption_colour = colour
            if not self._custom_pane_bitmaps and wx.Platform == "__WXMAC__":
                # No custom bitmaps for the pane close button
                # Change the MAC close bitmap colour
                self._active_close_bitmap = DrawMACCloseButton(wx.WHITE, colour)
                
        elif id == AUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR:
            self._active_caption_gradient_colour = colour
        elif id == AUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR:
            self._active_caption_text_colour = colour
        elif id == AUI_DOCKART_BORDER_COLOUR:
            self._border_pen.SetColour(colour)
        elif id == AUI_DOCKART_GRIPPER_COLOUR:
            self._gripper_brush.SetColour(colour)
            self._gripper_pen1.SetColour(StepColour(colour, 40))
            self._gripper_pen2.SetColour(StepColour(colour, 60))
        else:
            raise Exception("Invalid Colour Ordinal.")
        

    GetColour = GetColor
    SetColour = SetColor

    def SetFont(self, id, font):
        """
        Sets a font setting.
        
        :param `id`: must be ``AUI_DOCKART_CAPTION_FONT``;
        :param `font`: an instance of `wx.Font`.
        """
        
        if id == AUI_DOCKART_CAPTION_FONT:
            self._caption_font = font


    def GetFont(self, id):
        """
        Gets a font setting.
        
        :param `id`: must be ``AUI_DOCKART_CAPTION_FONT``, otherwise `wx.NullFont` is returned.
        """
        
        if id == AUI_DOCKART_CAPTION_FONT:
            return self._caption_font
        
        return wx.NullFont


    def DrawSash(self, dc, window, orient, rect):
        """
        Draws a sash between two windows.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `orient`: the sash orientation;
        :param `rect`: the sash rectangle.
        """                

        # AG: How do we make this work?!?
        # RendererNative does not use the sash_brush chosen by the user
        # and the rect.GetSize() is ignored as the sash is always drawn
        # 3 pixel wide
        # wx.RendererNative.Get().DrawSplitterSash(window, dc, rect.GetSize(), pos, orient)

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._sash_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)        

        draw_sash = self.GetMetric(AUI_DOCKART_DRAW_SASH_GRIP)
        if draw_sash:
            self.DrawSashGripper(dc, orient, rect)


    def DrawBackground(self, dc, window, orient, rect):
        """
        Draws a background.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `orient`: the gradient (if any) orientation;
        :param `rect`: the background rectangle.
        """

        dc.SetPen(wx.TRANSPARENT_PEN)
        if wx.Platform == "__WXMAC__":
            # we have to clear first, otherwise we are drawing a light striped pattern
            # over an already darker striped background
            dc.SetBrush(wx.WHITE_BRUSH)
            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        DrawGradientRectangle(dc, rect, self._background_brush.GetColour(),
                              self._background_gradient_colour,
                              AUI_GRADIENT_HORIZONTAL, rect.x, 700)


    def DrawBorder(self, dc, window, rect, pane):
        """
        Draws the pane border.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `rect`: the border rectangle;
        :param `pane`: the pane for which the border is drawn.
        """        

        drect = wx.Rect(*rect)
        
        dc.SetPen(self._border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        border_width = self.GetMetric(AUI_DOCKART_PANE_BORDER_SIZE)

        if pane.IsToolbar():
        
            for ii in xrange(0, border_width):
            
                dc.SetPen(wx.WHITE_PEN)
                dc.DrawLine(drect.x, drect.y, drect.x+drect.width, drect.y)
                dc.DrawLine(drect.x, drect.y, drect.x, drect.y+drect.height)
                dc.SetPen(self._border_pen)       
                dc.DrawLine(drect.x, drect.y+drect.height-1,
                            drect.x+drect.width, drect.y+drect.height-1)
                dc.DrawLine(drect.x+drect.width-1, drect.y,
                            drect.x+drect.width-1, drect.y+drect.height)
                drect.Deflate(1, 1)
        
        else:
        
            for ii in xrange(0, border_width):
            
                dc.DrawRectangle(drect.x, drect.y, drect.width, drect.height)
                drect.Deflate(1, 1)
            

    def DrawCaptionBackground(self, dc, rect, pane):
        """
        Draws the text caption background in the pane.

        :param `dc`: a `wx.DC` device context;
        :param `rect`: the text caption rectangle;
        :param `pane`: the pane for which the text background is drawn.
        """        

        active = pane.state & optionActive
 
        if self._gradient_type == AUI_GRADIENT_NONE:
            if active:
                dc.SetBrush(wx.Brush(self._active_caption_colour))
            else:
                dc.SetBrush(wx.Brush(self._inactive_caption_colour))

            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)        

        else:

            switch_gradient = pane.HasCaptionLeft()
            gradient_type = self._gradient_type
            if switch_gradient:
                gradient_type = (self._gradient_type == AUI_GRADIENT_HORIZONTAL and [AUI_GRADIENT_VERTICAL] or \
                                 [AUI_GRADIENT_HORIZONTAL])[0]
            
            if active:
                if wx.Platform == "__WXMAC__":
                    DrawGradientRectangle(dc, rect, self._active_caption_colour,
                                          self._active_caption_gradient_colour,
                                          gradient_type)                    
                else:
                    DrawGradientRectangle(dc, rect, self._active_caption_gradient_colour,
                                          self._active_caption_colour,
                                          gradient_type)
            else:
                if wx.Platform == "__WXMAC__":
                    DrawGradientRectangle(dc, rect, self._inactive_caption_gradient_colour,
                                          self._inactive_caption_colour,
                                          gradient_type)
                else:
                    DrawGradientRectangle(dc, rect, self._inactive_caption_colour,
                                          self._inactive_caption_gradient_colour,
                                          gradient_type)


    def DrawIcon(self, dc, rect, pane):
        """
        Draws the icon in the pane caption area.

        :param `dc`: a `wx.DC` device context;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the icon is drawn.
        """        
        
        # Draw the icon centered vertically 
        if pane.icon.Ok():
            if pane.HasCaptionLeft():
                bmp = wx.ImageFromBitmap(pane.icon).Rotate90(clockwise=False)
                dc.DrawBitmap(bmp.ConvertToBitmap(), rect.x+(rect.width-pane.icon.GetWidth())/2, rect.y+rect.height-2-pane.icon.GetHeight(), True)
            else:
                dc.DrawBitmap(pane.icon, rect.x+2, rect.y+(rect.height-pane.icon.GetHeight())/2, True)


    def DrawCaption(self, dc, window, text, rect, pane):
        """
        Draws the text in the pane caption.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `text`: the text to be displayed;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the text is drawn.
        """        

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetFont(self._caption_font)
        
        self.DrawCaptionBackground(dc, rect, pane)

        if pane.state & optionActive:
            dc.SetTextForeground(self._active_caption_text_colour)
        else:
            dc.SetTextForeground(self._inactive_caption_text_colour)

        w, h = dc.GetTextExtent("ABCDEFHXfgkj")

        clip_rect = wx.Rect(*rect)
        btns = pane.CountButtons()

        captionLeft = pane.HasCaptionLeft()
        variable = (captionLeft and [rect.height] or [rect.width])[0]

        variable -= 3      # text offset
        variable -= 2      # button padding

        caption_offset = 0
        if pane.icon:
            if captionLeft:
                caption_offset += pane.icon.GetHeight() + 3
            else:
                caption_offset += pane.icon.GetWidth() + 3
                
            self.DrawIcon(dc, rect, pane)

        variable -= caption_offset
        variable -= btns*(self._button_size + self._border_size)
        draw_text = ChopText(dc, text, variable)

        if captionLeft:
            dc.DrawRotatedText(draw_text, rect.x+(rect.width/2)-(h/2)-1, rect.y+rect.height-3-caption_offset, 90)
        else:
            dc.DrawText(draw_text, rect.x+3+caption_offset, rect.y+(rect.height/2)-(h/2)-1)


    def RequestUserAttention(self, dc, window, text, rect, pane):
        """
        Requests the user attention by intermittently highlighting the pane caption.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `text`: the text to be displayed;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the text is drawn.
        """        

        state = pane.state
        pane.state &= ~optionActive
        
        for indx in xrange(6):
            active = (indx%2 == 0 and [True] or [False])[0]
            if active:
                pane.state |= optionActive
            else:
                pane.state &= ~optionActive
                
            self.DrawCaptionBackground(dc, rect, pane)
            self.DrawCaption(dc, window, text, rect, pane)
            wx.SafeYield()
            wx.MilliSleep(350)

        pane.state = state
        

    def DrawGripper(self, dc, window, rect, pane):
        """
        Draws a gripper on the pane.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the gripper is drawn.
        """        

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._gripper_brush)

        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        if not pane.HasGripperTop():
            y = 4
            while 1:
                dc.SetPen(self._gripper_pen1)
                dc.DrawPoint(rect.x+3, rect.y+y) 
                dc.SetPen(self._gripper_pen2) 
                dc.DrawPoint(rect.x+3, rect.y+y+1) 
                dc.DrawPoint(rect.x+4, rect.y+y) 
                dc.SetPen(self._gripper_pen3) 
                dc.DrawPoint(rect.x+5, rect.y+y+1) 
                dc.DrawPoint(rect.x+5, rect.y+y+2) 
                dc.DrawPoint(rect.x+4, rect.y+y+2) 
                y = y + 4 
                if y > rect.GetHeight() - 4:
                    break
        else:
            x = 4
            while 1:
                dc.SetPen(self._gripper_pen1) 
                dc.DrawPoint(rect.x+x, rect.y+3) 
                dc.SetPen(self._gripper_pen2) 
                dc.DrawPoint(rect.x+x+1, rect.y+3) 
                dc.DrawPoint(rect.x+x, rect.y+4) 
                dc.SetPen(self._gripper_pen3) 
                dc.DrawPoint(rect.x+x+1, rect.y+5) 
                dc.DrawPoint(rect.x+x+2, rect.y+5) 
                dc.DrawPoint(rect.x+x+2, rect.y+4) 
                x = x + 4 
                if x > rect.GetWidth() - 4:
                    break 
        

    def DrawPaneButton(self, dc, window, button, button_state, _rect, pane):
        """
        Draws a pane button in the pane caption area.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `button`: the button to be drawn;
        :param `button_state`: the pane button state;
        :param `_rect`: the pane caption rectangle;
        :param `pane`: the pane for which the button is drawn.
        """        
        
        if not pane:
            return
        
        if button == AUI_BUTTON_CLOSE:
            if pane.state & optionActive:
                bmp = self._active_close_bitmap
            else:
                bmp = self._inactive_close_bitmap

        elif button == AUI_BUTTON_PIN:
            if pane.state & optionActive:
                bmp = self._active_pin_bitmap
            else:
                bmp = self._inactive_pin_bitmap

        elif button == AUI_BUTTON_MAXIMIZE_RESTORE:
            if pane.IsMaximized():
                if pane.state & optionActive:
                    bmp = self._active_restore_bitmap
                else:
                    bmp = self._inactive_restore_bitmap
            else:
                if pane.state & optionActive:
                    bmp = self._active_maximize_bitmap
                else:
                    bmp = self._inactive_maximize_bitmap

        elif button == AUI_BUTTON_MINIMIZE:
            if pane.state & optionActive:
                bmp = self._active_minimize_bitmap
            else:
                bmp = self._inactive_minimize_bitmap

        isVertical = pane.HasCaptionLeft()
        
        rect = wx.Rect(*_rect)

        if isVertical:
            old_x = rect.x
            rect.x = rect.x + (rect.width/2) - (bmp.GetWidth()/2)
            rect.width = old_x + rect.width - rect.x - 1
        else:
            old_y = rect.y
            rect.y = rect.y + (rect.height/2) - (bmp.GetHeight()/2)
            rect.height = old_y + rect.height - rect.y - 1

        if button_state == AUI_BUTTON_STATE_PRESSED:
            rect.x += 1
            rect.y += 1

        if button_state in [AUI_BUTTON_STATE_HOVER, AUI_BUTTON_STATE_PRESSED]:

            if pane.state & optionActive:

                dc.SetBrush(wx.Brush(StepColour(self._active_caption_colour, 120)))
                dc.SetPen(wx.Pen(StepColour(self._active_caption_colour, 70)))

            else:

                dc.SetBrush(wx.Brush(StepColour(self._inactive_caption_colour, 120)))
                dc.SetPen(wx.Pen(StepColour(self._inactive_caption_colour, 70)))

            if wx.Platform != "__WXMAC__":
                # draw the background behind the button
                dc.DrawRectangle(rect.x, rect.y, 15, 15)
            else:
                # Darker the bitmap a bit
                bmp = DarkenBitmap(bmp, self._active_caption_colour, StepColour(self._active_caption_colour, 110))

        if isVertical:
            bmp = wx.ImageFromBitmap(bmp).Rotate90(clockwise=False).ConvertToBitmap()
            
        # draw the button itself
        dc.DrawBitmap(bmp, rect.x, rect.y, True)


    def DrawSashGripper(self, dc, orient, rect):
        """
        Draws a sash gripper on a sash between two windows.

        :param `dc`: a `wx.DC` device context;
        :param `orient`: the sash orientation;
        :param `rect`: the sash rectangle.
        """
        
        dc.SetBrush(self._gripper_brush)

        if orient == wx.HORIZONTAL:  # horizontal sash
            
            x = rect.x + int((1.0/4.0)*rect.width)
            xend = rect.x + int((3.0/4.0)*rect.width)
            y = rect.y + (rect.height/2) - 1

            while 1:
                dc.SetPen(self._gripper_pen3)
                dc.DrawRectangle(x, y, 2, 2)
                dc.SetPen(self._gripper_pen2) 
                dc.DrawPoint(x+1, y+1)
                x = x + 5

                if x >= xend:
                    break

        else:

            y = rect.y + int((1.0/4.0)*rect.height)
            yend = rect.y + int((3.0/4.0)*rect.height)
            x = rect.x + (rect.width/2) - 1

            while 1:
                dc.SetPen(self._gripper_pen3)
                dc.DrawRectangle(x, y, 2, 2)
                dc.SetPen(self._gripper_pen2) 
                dc.DrawPoint(x+1, y+1)
                y = y + 5

                if y >= yend:
                    break


    def SetDefaultPaneBitmaps(self, isMac):
        """
        Assigns the default pane bitmaps.

        :param `isMac`: whether we are on wxMAC or not.
        """

        if isMac:
            self._inactive_close_bitmap = DrawMACCloseButton(wx.WHITE, self._inactive_caption_colour)
            self._active_close_bitmap = DrawMACCloseButton(wx.WHITE, self._active_caption_colour)
        else:
            self._inactive_close_bitmap = BitmapFromBits(close_bits, 16, 16, self._inactive_caption_text_colour)
            self._active_close_bitmap = BitmapFromBits(close_bits, 16, 16, self._active_caption_text_colour)
            
        if isMac:
            self._inactive_maximize_bitmap = BitmapFromBits(max_bits, 16, 16, wx.WHITE)
            self._active_maximize_bitmap = BitmapFromBits(max_bits, 16, 16, wx.WHITE)
        else:
            self._inactive_maximize_bitmap = BitmapFromBits(max_bits, 16, 16, self._inactive_caption_text_colour)
            self._active_maximize_bitmap = BitmapFromBits(max_bits, 16, 16, self._active_caption_text_colour)

        if isMac:
            self._inactive_restore_bitmap = BitmapFromBits(restore_bits, 16, 16, wx.WHITE)
            self._active_restore_bitmap = BitmapFromBits(restore_bits, 16, 16, wx.WHITE)
        else:
            self._inactive_restore_bitmap = BitmapFromBits(restore_bits, 16, 16, self._inactive_caption_text_colour)
            self._active_restore_bitmap = BitmapFromBits(restore_bits, 16, 16, self._active_caption_text_colour)

        if isMac:
            self._inactive_minimize_bitmap = BitmapFromBits(minimize_bits, 16, 16, wx.WHITE)
            self._active_minimize_bitmap = BitmapFromBits(minimize_bits, 16, 16, wx.WHITE)
        else:
            self._inactive_minimize_bitmap = BitmapFromBits(minimize_bits, 16, 16, self._inactive_caption_text_colour)
            self._active_minimize_bitmap = BitmapFromBits(minimize_bits, 16, 16, self._active_caption_text_colour)

        self._inactive_pin_bitmap = BitmapFromBits(pin_bits, 16, 16, self._inactive_caption_text_colour)
        self._active_pin_bitmap = BitmapFromBits(pin_bits, 16, 16, self._active_caption_text_colour)

        self._custom_pane_bitmaps = False
        
        
    def SetCustomPaneBitmap(self, bmp, button, active, maximize=False):
        """
        Sets a custom button bitmap for the pane button.

        :param `bmp`: the actual bitmap to set;
        :param `button`: the button identifier;
        :param `active`: whether it is the bitmap for the active button or not;
        :param `maximize`: used to distinguish between the maximize and restore bitmaps.
        """

        if bmp.GetWidth() > 16 or bmp.GetHeight() > 16:
            raise Exception("The input bitmap is too big")

        if button == AUI_BUTTON_CLOSE:
            if active:
                self._active_close_bitmap = bmp
            else:
                self._inactive_close_bitmap = bmp

            if wx.Platform == "__WXMAC__":
                self._custom_pane_bitmaps = True                

        elif button == AUI_BUTTON_PIN:
            if active:
                self._active_pin_bitmap = bmp
            else:
                self._inactive_pin_bitmap = bmp

        elif button == AUI_BUTTON_MAXIMIZE_RESTORE:
            if maximize:
                if active:
                    self._active_maximize_bitmap = bmp
                else:
                    self._inactive_maximize_bitmap = bmp
            else:
                if active:
                    self._active_restore_bitmap = bmp
                else:
                    self._inactive_restore_bitmap = bmp

        elif button == AUI_BUTTON_MINIMIZE:
            if active:
                self._active_minimize_bitmap = bmp
            else:
                self._inactive_minimize_bitmap = bmp


if _ctypes:
    class RECT(ctypes.Structure):
        """ Used to handle L{ModernDockArt} on Windows XP/Vista/7. """
        _fields_ = [('left', ctypes.c_ulong),('top', ctypes.c_ulong),('right', ctypes.c_ulong),('bottom', ctypes.c_ulong)]

        def dump(self):
            """ Dumps `self` as a `wx.Rect`. """
            return map(int, (self.left, self.top, self.right, self.bottom))


    class SIZE(ctypes.Structure):
        """ Used to handle L{ModernDockArt} on Windows XP/Vista/7. """
        _fields_ = [('x', ctypes.c_long),('y', ctypes.c_long)]


class ModernDockArt(AuiDefaultDockArt):
    """
    ModernDockArt is a custom `AuiDockArt` class, that implements a look similar to
    Firefox and other recents applications. 

    Is uses the `winxptheme` module and XP themes whenever possible, so it should
    look good even if the user has a custom theme.

    :note: This dock art is Windows only and will only work if you have installed
     Mark Hammond's `pywin32` module (http://sourceforge.net/projects/pywin32/).
    """

    def __init__(self, win):
        """
        Default class constructor. 

        :param `win`: the window managed by L{AuiManager}. 
        """
        
        AuiDefaultDockArt.__init__(self)
        
        self.win = win

        # Get the size of a small close button (themed)
        hwnd = self.win.GetHandle()
        
        self.hTheme1 = winxptheme.OpenThemeData(hwnd, "Window")
        
        self.usingTheme = True
        
        if not self.hTheme1:
            self.usingTheme = False

        self._button_size = 13

        self._button_border_size = 3
        self._caption_text_indent = 6
        self._caption_size = 22
        
        # We only highlight the active pane with the caption text being in bold.
        # So we do not want a special colour for active elements.        
        self._active_close_bitmap = self._inactive_close_bitmap

        self.Init()
        

    def Init(self):
        """ Initializes the dock art. """

        AuiDefaultDockArt.Init(self)
        
        self._active_caption_colour = self._inactive_caption_colour
        self._active_caption_text_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_CAPTIONTEXT)
        self._inactive_caption_text_colour = self._active_caption_text_colour


    def DrawCaption(self, dc, window, text, rect, pane):
        """
        Draws the text in the pane caption.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `text`: the text to be displayed;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the text is drawn.
        """        

        dc.SetPen(wx.TRANSPARENT_PEN)
        self.DrawCaptionBackground(dc, rect, pane)

        active = ((pane.state & optionActive) and [True] or [False])[0]

        self._caption_font.SetWeight(wx.FONTWEIGHT_BOLD)
        dc.SetFont(self._caption_font)
        
        if active:
            dc.SetTextForeground(self._active_caption_text_colour)
        else:
            dc.SetTextForeground(self._inactive_caption_text_colour)

        w, h = dc.GetTextExtent("ABCDEFHXfgkj")

        clip_rect = wx.Rect(*rect)
        btns = pane.CountButtons()

        captionLeft = pane.HasCaptionLeft()
        variable = (captionLeft and [rect.height] or [rect.width])[0]

        variable -= 3      # text offset
        variable -= 2      # button padding

        caption_offset = 0
        if pane.icon:
            if captionLeft:
                caption_offset += pane.icon.GetHeight() + 3
            else:
                caption_offset += pane.icon.GetWidth() + 3
                
            self.DrawIcon(dc, rect, pane)

        diff = -2
        if self.usingTheme:
            diff = -1

        variable -= caption_offset
        variable -= btns*(self._button_size + self._button_border_size)
        draw_text = ChopText(dc, text, variable)

        if captionLeft:
            dc.DrawRotatedText(draw_text, rect.x+(rect.width/2)-(h/2)-diff, rect.y+rect.height-3-caption_offset, 90)
        else:
            dc.DrawText(draw_text, rect.x+3+caption_offset, rect.y+(rect.height/2)-(h/2)-diff)


    def DrawCaptionBackground(self, dc, rect, pane):
        """
        Draws the text caption background in the pane.

        :param `dc`: a `wx.DC` device context;
        :param `rect`: the text caption rectangle;
        :param `pane`: the pane for which we are drawing the caption background.
        """        

        dc.SetBrush(self._background_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        active = ((pane.state & optionActive) and [True] or [False])[0]

        if self.usingTheme:
            
            rectangle = wx.Rect()

            rc = RECT(rectangle.x, rectangle.y, rectangle.width, rectangle.height)

            # If rect x/y values are negative rc.right/bottom values will overflow and winxptheme.DrawThemeBackground
            # will raise a TypeError. Ensure they are never negative.
            rect.x = max(0, rect.x)
            rect.y = max(0, rect.y)

            rc.top = rect.x
            rc.left = rect.y
            rc.right = rect.x + rect.width
            rc.bottom = rect.y + rect.height

            if active:
                winxptheme.DrawThemeBackground(self.hTheme1, dc.GetHDC(), 5, 1, (rc.top, rc.left, rc.right, rc.bottom), None)
            else:
                winxptheme.DrawThemeBackground(self.hTheme1, dc.GetHDC(), 5, 2, (rc.top, rc.left, rc.right, rc.bottom), None)
            
        else:

            AuiDefaultDockArt.DrawCaptionBackground(self, dc, rect, pane)


    def RequestUserAttention(self, dc, window, text, rect, pane):
        """
        Requests the user attention by intermittently highlighting the pane caption.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `text`: the text to be displayed;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the text is drawn.
        """        
    
        state = pane.state
        pane.state &= ~optionActive
        
        for indx in xrange(6):
            active = (indx%2 == 0 and [True] or [False])[0]
            if active:
                pane.state |= optionActive
            else:
                pane.state &= ~optionActive
                
            self.DrawCaptionBackground(dc, rect, pane)
            self.DrawCaption(dc, window, text, rect, pane)
            wx.SafeYield()
            wx.MilliSleep(350)

        pane.state = state


    def DrawPaneButton(self, dc, window, button, button_state, rect, pane):
        """
        Draws a pane button in the pane caption area.

        :param `dc`: a `wx.DC` device context;
        :param `window`: an instance of `wx.Window`;
        :param `button`: the button to be drawn;
        :param `button_state`: the pane button state;
        :param `rect`: the pane caption rectangle;
        :param `pane`: the pane for which the button is drawn.
        """        

        if self.usingTheme:

            hTheme = self.hTheme1            
                    
            # Get the real button position (compensating for borders)
            drect = wx.Rect(rect.x, rect.y, self._button_size, self._button_size)
            
            # Draw the themed close button
            rc = RECT(0, 0, 0, 0)
            if pane.HasCaptionLeft():
                rc.top = rect.x + self._button_border_size
                rc.left = int(rect.y + 1.5*self._button_border_size)
                rc.right = rect.x + self._button_size + self._button_border_size
                rc.bottom = int(rect.y + self._button_size + 1.5*self._button_border_size)
            else:
                rc.top = rect.x - self._button_border_size
                rc.left = int(rect.y + 1.5*self._button_border_size)
                rc.right = rect.x + self._button_size- self._button_border_size
                rc.bottom = int(rect.y + self._button_size + 1.5*self._button_border_size)

            if button == AUI_BUTTON_CLOSE:
                btntype = 19
                
            elif button == AUI_BUTTON_PIN:
                btntype = 23

            elif button == AUI_BUTTON_MAXIMIZE_RESTORE:
                if not pane.IsMaximized():
                    btntype = 17
                else:
                    btntype = 21
            else:
                btntype = 15

            state = 4 # CBS_DISABLED
            
            if pane.state & optionActive:

                if button_state == AUI_BUTTON_STATE_NORMAL:
                    state = 1 # CBS_NORMAL

                elif button_state == AUI_BUTTON_STATE_HOVER:
                    state = 2 # CBS_HOT

                elif button_state == AUI_BUTTON_STATE_PRESSED:
                    state = 3 # CBS_PUSHED

                else:
                    raise Exception("ERROR: Unknown State.")

            else: # inactive pane

                if button_state == AUI_BUTTON_STATE_NORMAL:
                    state = 5 # CBS_NORMAL

                elif button_state == AUI_BUTTON_STATE_HOVER:
                    state = 6 # CBS_HOT

                elif button_state == AUI_BUTTON_STATE_PRESSED:
                    state = 7 # CBS_PUSHED

                else:
                    raise Exception("ERROR: Unknown State.")

            try:
                winxptheme.DrawThemeBackground(hTheme, dc.GetHDC(), btntype, state, (rc.top, rc.left, rc.right, rc.bottom), None)
            except TypeError:
                return

        else:

            # Fallback to default closebutton if themes are not enabled
            rect2 = wx.Rect(rect.x-4, rect.y+2, rect.width, rect.height)
            AuiDefaultDockArt.DrawPaneButton(self, dc, window, button, button_state, rect2, pane)

