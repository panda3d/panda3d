"""
This module contains drawing routines and customizations for the AGW widgets
L{LabelBook} and L{FlatMenu}.
"""

import wx
import cStringIO
import random

from fmresources import *

# ---------------------------------------------------------------------------- #
# Class DCSaver
# ---------------------------------------------------------------------------- #

_ = wx.GetTranslation

_libimported = None

if wx.Platform == "__WXMSW__":
    osVersion = wx.GetOsVersion()
    # Shadows behind menus are supported only in XP
    if osVersion[1] == 5 and osVersion[2] == 1:
        try:
            import win32api
            import win32con
            import winxpgui
            _libimported = "MH"
        except:
            try:
                import ctypes
                _libimported = "ctypes"
            except:
                pass
    else:
        _libimported = None


class DCSaver(object):
    """
    Construct a DC saver. The dc is copied as-is.
    """

    def __init__(self, pdc):
        """
        Default class constructor.

        :param `pdc`: an instance of `wx.DC`.        
        """

        self._pdc = pdc
        self._pen = pdc.GetPen()
        self._brush = pdc.GetBrush()


    def __del__(self):
        """ While destructing, restores the dc pen and brush. """

        if self._pdc:
            self._pdc.SetPen(self._pen)
            self._pdc.SetBrush(self._brush)


# ---------------------------------------------------------------------------- #
# Class RendererBase
# ---------------------------------------------------------------------------- #

class RendererBase(object):
    """ Base class for all theme renderers. """
    
    def __init__(self):
        """ Default class constructor. Intentionally empty. """
        
        pass


    def DrawButtonBorders(self, dc, rect, penColour, brushColour):
        """
        Draws borders for buttons.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `penColour`: a valid `wx.Colour` for the pen border;
        :param `brushColour`: a valid `wx.Colour` for the brush.
        """

        # Keep old pen and brush
        dcsaver = DCSaver(dc)
        dc.SetPen(wx.Pen(penColour))
        dc.SetBrush(wx.Brush(brushColour))
        dc.DrawRectangleRect(rect)


    def DrawBitmapArea(self, dc, xpm_name, rect, baseColour, flipSide):
        """
        Draws the area below a bitmap and the bitmap itself using a gradient shading.

        :param `dc`: an instance of `wx.DC`;
        :param `xpm_name`: a name of a XPM bitmap;
        :param `rect`: the bitmap client rectangle;
        :param `baseColour`: a valid `wx.Colour` for the bitmap background;
        :param `flipSide`: ``True`` to flip the gradient direction, ``False`` otherwise.
        """

        # draw the gradient area
        if not flipSide:
            ArtManager.Get().PaintDiagonalGradientBox(dc, rect, wx.WHITE,
                                                      ArtManager.Get().LightColour(baseColour, 20),
                                                      True, False)
        else:
            ArtManager.Get().PaintDiagonalGradientBox(dc, rect, ArtManager.Get().LightColour(baseColour, 20),
                                                      wx.WHITE, True, False)

        # draw arrow
        arrowDown = wx.BitmapFromXPMData(xpm_name)
        arrowDown.SetMask(wx.Mask(arrowDown, wx.WHITE))
        dc.DrawBitmap(arrowDown, rect.x + 1 , rect.y + 1, True)


    def DrawBitmapBorders(self, dc, rect, penColour, bitmapBorderUpperLeftPen):
        """
        Draws borders for a bitmap.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `penColour`: a valid `wx.Colour` for the pen border;
        :param `bitmapBorderUpperLeftPen`: a valid `wx.Colour` for the pen upper
         left border.
        """

        # Keep old pen and brush
        dcsaver = DCSaver(dc)

        # lower right size
        dc.SetPen(wx.Pen(penColour))
        dc.DrawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width, rect.y + rect.height - 1)
        dc.DrawLine(rect.x + rect.width - 1, rect.y, rect.x + rect.width - 1, rect.y + rect.height)
        
        # upper left side
        dc.SetPen(wx.Pen(bitmapBorderUpperLeftPen))
        dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y)
        dc.DrawLine(rect.x, rect.y, rect.x, rect.y + rect.height)


    def GetMenuFaceColour(self):
        """ Returns the foreground colour for the menu. """
        
        return ArtManager.Get().LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE), 80)


    def GetTextColourEnable(self):
        """ Returns the colour used for text colour when enabled. """

        return wx.BLACK


    def GetTextColourDisable(self):
        """ Returns the colour used for text colour when disabled. """

        return ArtManager.Get().LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_GRAYTEXT), 30)


    def GetFont(self):
        """ Returns the font used for text. """

        return wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)

                
# ---------------------------------------------------------------------------- #
# Class RendererXP
# ---------------------------------------------------------------------------- #

class RendererXP(RendererBase):
    """ Xp-Style renderer. """
    
    def __init__(self):
        """ Default class constructor. """

        RendererBase.__init__(self)


    def DrawButton(self, dc, rect, state, input=None):
        """
        Draws a button using the XP theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `input`: a flag used to call the right method.
        """

        if input is None or type(input) == type(False):
            self.DrawButtonTheme(dc, rect, state, input)
        else:
            self.DrawButtonColour(dc, rect, state, input)

            
    def DrawButtonTheme(self, dc, rect, state, useLightColours=None):
        """
        Draws a button using the XP theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `useLightColours`: ``True`` to use light colours, ``False`` otherwise.
        """

        # switch according to the status
        if state == ControlFocus:
            penColour = ArtManager.Get().FrameColour()
            brushColour = ArtManager.Get().BackgroundColour()
        elif state == ControlPressed:
            penColour = ArtManager.Get().FrameColour()
            brushColour = ArtManager.Get().HighlightBackgroundColour()
        else:
            penColour = ArtManager.Get().FrameColour()
            brushColour = ArtManager.Get().BackgroundColour()        

        # Draw the button borders
        self.DrawButtonBorders(dc, rect, penColour, brushColour)


    def DrawButtonColour(self, dc, rect, state, colour):
        """
        Draws a button using the XP theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `colour`: a valid `wx.Colour` instance.
        """

        # switch according to the status        
        if statet == ControlFocus:
            penColour = colour
            brushColour = ArtManager.Get().LightColour(colour, 75)
        elif state == ControlPressed:
            penColour = colour
            brushColour = ArtManager.Get().LightColour(colour, 60)
        else:
            penColour = colour
            brushColour = ArtManager.Get().LightColour(colour, 75)

        # Draw the button borders
        self.DrawButtonBorders(dc, rect, penColour, brushColour)


    def DrawMenuBarBg(self, dc, rect):
        """
        Draws the menu bar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the menu bar's client rectangle.
        """

        # For office style, we simple draw a rectangle with a gradient colouring
        artMgr = ArtManager.Get()
        vertical = artMgr.GetMBVerticalGradient()

        dcsaver = DCSaver(dc)

        # fill with gradient
        startColour = artMgr.GetMenuBarFaceColour()
        if artMgr.IsDark(startColour):
            startColour = artMgr.LightColour(startColour, 50)

        endColour = artMgr.LightColour(startColour, 90)
        artMgr.PaintStraightGradientBox(dc, rect, startColour, endColour, vertical)

        # Draw the border
        if artMgr.GetMenuBarBorder():

            dc.SetPen(wx.Pen(startColour))
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.DrawRectangleRect(rect)


    def DrawToolBarBg(self, dc, rect):
        """
        Draws the toolbar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the toolbar's client rectangle.
        """

        artMgr = ArtManager.Get()
        
        if not artMgr.GetRaiseToolbar():
            return

        # For office style, we simple draw a rectangle with a gradient colouring
        vertical = artMgr.GetMBVerticalGradient()

        dcsaver = DCSaver(dc)

        # fill with gradient
        startColour = artMgr.GetMenuBarFaceColour()
        if artMgr.IsDark(startColour):
            startColour = artMgr.LightColour(startColour, 50)
    
        startColour = artMgr.LightColour(startColour, 20)

        endColour   = artMgr.LightColour(startColour, 90)
        artMgr.PaintStraightGradientBox(dc, rect, startColour, endColour, vertical)
        artMgr.DrawBitmapShadow(dc, rect)


    def GetTextColourEnable(self):
        """ Returns the colour used for text colour when enabled. """

        return wx.BLACK

    
# ---------------------------------------------------------------------------- #
# Class RendererMSOffice2007
# ---------------------------------------------------------------------------- #

class RendererMSOffice2007(RendererBase):
    """ Windows MS Office 2007 style. """
    
    def __init__(self):
        """ Default class constructor. """

        RendererBase.__init__(self)


    def GetColoursAccordingToState(self, state):
        """
        Returns a `wx.Colour` according to the menu item state.

        :param `state`: one of the following bits:

         ==================== ======= ==========================
         Item State            Value  Description
         ==================== ======= ==========================         
         ``ControlPressed``         0 The item is pressed
         ``ControlFocus``           1 The item is focused
         ``ControlDisabled``        2 The item is disabled
         ``ControlNormal``          3 Normal state
         ==================== ======= ==========================
        
        """

        # switch according to the status        
        if state == ControlFocus:
            upperBoxTopPercent = 95
            upperBoxBottomPercent = 50
            lowerBoxTopPercent = 40
            lowerBoxBottomPercent = 90
            concaveUpperBox = True
            concaveLowerBox = True
            
        elif state == ControlPressed:
            upperBoxTopPercent = 75
            upperBoxBottomPercent = 90
            lowerBoxTopPercent = 90
            lowerBoxBottomPercent = 40
            concaveUpperBox = True
            concaveLowerBox = True

        elif state == ControlDisabled:
            upperBoxTopPercent = 100
            upperBoxBottomPercent = 100
            lowerBoxTopPercent = 70
            lowerBoxBottomPercent = 70
            concaveUpperBox = True
            concaveLowerBox = True

        else:
            upperBoxTopPercent = 90
            upperBoxBottomPercent = 50
            lowerBoxTopPercent = 30
            lowerBoxBottomPercent = 75
            concaveUpperBox = True
            concaveLowerBox = True

        return upperBoxTopPercent, upperBoxBottomPercent, lowerBoxTopPercent, lowerBoxBottomPercent, \
               concaveUpperBox, concaveLowerBox

        
    def DrawButton(self, dc, rect, state, useLightColours):
        """
        Draws a button using the MS Office 2007 theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `useLightColours`: ``True`` to use light colours, ``False`` otherwise.
        """

        self.DrawButtonColour(dc, rect, state, ArtManager.Get().GetThemeBaseColour(useLightColours))


    def DrawButtonColour(self, dc, rect, state, colour):
        """
        Draws a button using the MS Office 2007 theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `colour`: a valid `wx.Colour` instance.
        """

        artMgr = ArtManager.Get()
        
        # Keep old pen and brush
        dcsaver = DCSaver(dc)
        
        # Define the rounded rectangle base on the given rect
        # we need an array of 9 points for it        
        baseColour = colour

        # Define the middle points
        leftPt = wx.Point(rect.x, rect.y + (rect.height / 2))
        rightPt = wx.Point(rect.x + rect.width-1, rect.y + (rect.height / 2))

        # Define the top region
        top = wx.RectPP((rect.GetLeft(), rect.GetTop()), rightPt)
        bottom = wx.RectPP(leftPt, (rect.GetRight(), rect.GetBottom()))

        upperBoxTopPercent, upperBoxBottomPercent, lowerBoxTopPercent, lowerBoxBottomPercent, \
                            concaveUpperBox, concaveLowerBox = self.GetColoursAccordingToState(state)

        topStartColour = artMgr.LightColour(baseColour, upperBoxTopPercent)
        topEndColour = artMgr.LightColour(baseColour, upperBoxBottomPercent)
        bottomStartColour = artMgr.LightColour(baseColour, lowerBoxTopPercent)
        bottomEndColour = artMgr.LightColour(baseColour, lowerBoxBottomPercent)

        artMgr.PaintStraightGradientBox(dc, top, topStartColour, topEndColour)
        artMgr.PaintStraightGradientBox(dc, bottom, bottomStartColour, bottomEndColour)

        rr = wx.Rect(rect.x, rect.y, rect.width, rect.height)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        frameColour = artMgr.LightColour(baseColour, 60)
        dc.SetPen(wx.Pen(frameColour))
        dc.DrawRectangleRect(rr)

        wc = artMgr.LightColour(baseColour, 80)
        dc.SetPen(wx.Pen(wc))
        rr.Deflate(1, 1)
        dc.DrawRectangleRect(rr)


    def DrawMenuBarBg(self, dc, rect):
        """
        Draws the menu bar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the menu bar's client rectangle.
        """

        # Keep old pen and brush
        dcsaver = DCSaver(dc)
        artMgr = ArtManager.Get()
        baseColour = artMgr.GetMenuBarFaceColour()

        dc.SetBrush(wx.Brush(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)))
        dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)))
        dc.DrawRectangleRect(rect)

        # Define the rounded rectangle base on the given rect
        # we need an array of 9 points for it
        regPts = [wx.Point() for ii in xrange(9)]
        radius = 2
        
        regPts[0] = wx.Point(rect.x, rect.y + radius)
        regPts[1] = wx.Point(rect.x+radius, rect.y)
        regPts[2] = wx.Point(rect.x+rect.width-radius-1, rect.y)
        regPts[3] = wx.Point(rect.x+rect.width-1, rect.y + radius)
        regPts[4] = wx.Point(rect.x+rect.width-1, rect.y + rect.height - radius - 1)
        regPts[5] = wx.Point(rect.x+rect.width-radius-1, rect.y + rect.height-1)
        regPts[6] = wx.Point(rect.x+radius, rect.y + rect.height-1)
        regPts[7] = wx.Point(rect.x, rect.y + rect.height - radius - 1)
        regPts[8] = regPts[0]

        # Define the middle points

        factor = artMgr.GetMenuBgFactor()
        
        leftPt1 = wx.Point(rect.x, rect.y + (rect.height / factor))
        leftPt2 = wx.Point(rect.x, rect.y + (rect.height / factor)*(factor-1))

        rightPt1 = wx.Point(rect.x + rect.width, rect.y + (rect.height / factor))
        rightPt2 = wx.Point(rect.x + rect.width, rect.y + (rect.height / factor)*(factor-1))

        # Define the top region
        topReg = [wx.Point() for ii in xrange(7)]
        topReg[0] = regPts[0]
        topReg[1] = regPts[1]
        topReg[2] = wx.Point(regPts[2].x+1, regPts[2].y)
        topReg[3] = wx.Point(regPts[3].x + 1, regPts[3].y)
        topReg[4] = wx.Point(rightPt1.x, rightPt1.y+1)
        topReg[5] = wx.Point(leftPt1.x, leftPt1.y+1)
        topReg[6] = topReg[0]

        # Define the middle region
        middle = wx.RectPP(leftPt1, wx.Point(rightPt2.x - 2, rightPt2.y))
            
        # Define the bottom region
        bottom = wx.RectPP(leftPt2, wx.Point(rect.GetRight() - 1, rect.GetBottom()))

        topStartColour   = artMgr.LightColour(baseColour, 90)
        topEndColour = artMgr.LightColour(baseColour, 60)
        bottomStartColour = artMgr.LightColour(baseColour, 40)
        bottomEndColour   = artMgr.LightColour(baseColour, 20)
        
        topRegion = wx.RegionFromPoints(topReg)

        artMgr.PaintGradientRegion(dc, topRegion, topStartColour, topEndColour)
        artMgr.PaintStraightGradientBox(dc, bottom, bottomStartColour, bottomEndColour)
        artMgr.PaintStraightGradientBox(dc, middle, topEndColour, bottomStartColour)
     

    def DrawToolBarBg(self, dc, rect):
        """
        Draws the toolbar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the toolbar's client rectangle.
        """

        artMgr = ArtManager.Get()
        
        if not artMgr.GetRaiseToolbar():
            return

        # Keep old pen and brush
        dcsaver = DCSaver(dc)
        
        baseColour = artMgr.GetMenuBarFaceColour()
        baseColour = artMgr.LightColour(baseColour, 20)

        dc.SetBrush(wx.Brush(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)))
        dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)))
        dc.DrawRectangleRect(rect)

        radius = 2
        
        # Define the rounded rectangle base on the given rect
        # we need an array of 9 points for it
        regPts = [None]*9
        
        regPts[0] = wx.Point(rect.x, rect.y + radius)
        regPts[1] = wx.Point(rect.x+radius, rect.y)
        regPts[2] = wx.Point(rect.x+rect.width-radius-1, rect.y)
        regPts[3] = wx.Point(rect.x+rect.width-1, rect.y + radius)
        regPts[4] = wx.Point(rect.x+rect.width-1, rect.y + rect.height - radius - 1)
        regPts[5] = wx.Point(rect.x+rect.width-radius-1, rect.y + rect.height-1)
        regPts[6] = wx.Point(rect.x+radius, rect.y + rect.height-1)
        regPts[7] = wx.Point(rect.x, rect.y + rect.height - radius - 1)
        regPts[8] = regPts[0]

        # Define the middle points
        factor = artMgr.GetMenuBgFactor()

        leftPt1 = wx.Point(rect.x, rect.y + (rect.height / factor))
        rightPt1 = wx.Point(rect.x + rect.width, rect.y + (rect.height / factor))
        
        leftPt2 = wx.Point(rect.x, rect.y + (rect.height / factor)*(factor-1))
        rightPt2 = wx.Point(rect.x + rect.width, rect.y + (rect.height / factor)*(factor-1))

        # Define the top region
        topReg = [None]*7
        topReg[0] = regPts[0]
        topReg[1] = regPts[1]
        topReg[2] = wx.Point(regPts[2].x+1, regPts[2].y)
        topReg[3] = wx.Point(regPts[3].x + 1, regPts[3].y)
        topReg[4] = wx.Point(rightPt1.x, rightPt1.y+1)
        topReg[5] = wx.Point(leftPt1.x, leftPt1.y+1)
        topReg[6] = topReg[0]

        # Define the middle region
        middle = wx.RectPP(leftPt1, wx.Point(rightPt2.x - 2, rightPt2.y))

        # Define the bottom region
        bottom = wx.RectPP(leftPt2, wx.Point(rect.GetRight() - 1, rect.GetBottom()))
        
        topStartColour   = artMgr.LightColour(baseColour, 90)
        topEndColour = artMgr.LightColour(baseColour, 60)
        bottomStartColour = artMgr.LightColour(baseColour, 40)
        bottomEndColour   = artMgr.LightColour(baseColour, 20)
        
        topRegion = wx.RegionFromPoints(topReg)

        artMgr.PaintGradientRegion(dc, topRegion, topStartColour, topEndColour)
        artMgr.PaintStraightGradientBox(dc, bottom, bottomStartColour, bottomEndColour)
        artMgr.PaintStraightGradientBox(dc, middle, topEndColour, bottomStartColour)

        artMgr.DrawBitmapShadow(dc, rect)


    def GetTextColourEnable(self):
        """ Returns the colour used for text colour when enabled. """

        return wx.NamedColour("MIDNIGHT BLUE")
    
    
# ---------------------------------------------------------------------------- #
# Class ArtManager
# ---------------------------------------------------------------------------- #

class ArtManager(wx.EvtHandler):

    """
    This class provides various art utilities, such as creating shadow, providing
    lighter / darker colours for a given colour, etc...
    """
    
    _alignmentBuffer = 7
    _menuTheme = StyleXP
    _verticalGradient = False
    _renderers = {StyleXP: None, Style2007: None}
    _bmpShadowEnabled = False
    _ms2007sunken = False
    _drowMBBorder = True
    _menuBgFactor = 5
    _menuBarColourScheme = _("Default")
    _raiseTB = True
    _bitmaps = {}
    _transparency = 255

    def __init__(self):
        """ Default class constructor. """

        wx.EvtHandler.__init__(self)
        self._menuBarBgColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
        
        # connect an event handler to the system colour change event
        self.Bind(wx.EVT_SYS_COLOUR_CHANGED, self.OnSysColourChange)


    def SetTransparency(self, amount):
        """
        Sets the alpha channel value for transparent windows.

        :param `amount`: the actual transparency value (between 0 and 255).
        """

        if self._transparency == amount:
            return

        if amount < 0 or amount > 255:
            raise Exception("Invalid transparency value")

        self._transparency = amount


    def GetTransparency(self):
        """ Returns the alpha channel value for transparent windows. """

        return self._transparency
    

    def ConvertToBitmap(self, xpm, alpha=None):
        """
        Convert the given image to a bitmap, optionally overlaying an alpha
        channel to it.

        :param `xpm`: a list of strings formatted as XPM;
        :param `alpha`: a list of alpha values, the same size as the xpm bitmap.
        """

        if alpha is not None:

            img = wx.BitmapFromXPMData(xpm)
            img = img.ConvertToImage()
            x, y = img.GetWidth(), img.GetHeight()
            img.InitAlpha()
            for jj in xrange(y):
                for ii in xrange(x):
                    img.SetAlpha(ii, jj, alpha[jj*x+ii])
                    
        else:

            stream = cStringIO.StringIO(xpm)
            img = wx.ImageFromStream(stream)
            
        return wx.BitmapFromImage(img)

                
    def Initialize(self):
        """ Initializes the bitmaps and colours. """

        # create wxBitmaps from the xpm's
        self._rightBottomCorner = self.ConvertToBitmap(shadow_center_xpm, shadow_center_alpha)
        self._bottom = self.ConvertToBitmap(shadow_bottom_xpm, shadow_bottom_alpha)
        self._bottomLeft = self.ConvertToBitmap(shadow_bottom_left_xpm, shadow_bottom_left_alpha)
        self._rightTop = self.ConvertToBitmap(shadow_right_top_xpm, shadow_right_top_alpha)
        self._right = self.ConvertToBitmap(shadow_right_xpm, shadow_right_alpha)

        # initialise the colour map
        self.InitColours()
        self.SetMenuBarColour(self._menuBarColourScheme)
        
        # Create common bitmaps
        self.FillStockBitmaps()


    def FillStockBitmaps(self):
        """ Initializes few standard bitmaps. """

        bmp = self.ConvertToBitmap(arrow_down, alpha=None)
        bmp.SetMask(wx.Mask(bmp, wx.Colour(0, 128, 128)))
        self._bitmaps.update({"arrow_down": bmp})

        bmp = self.ConvertToBitmap(arrow_up, alpha=None)
        bmp.SetMask(wx.Mask(bmp, wx.Colour(0, 128, 128)))
        self._bitmaps.update({"arrow_up": bmp})


    def GetStockBitmap(self, name):
        """
        Returns a bitmap from a stock. 

        :param `name`: the bitmap name.

        :return: The stock bitmap, if `name` was found in the stock bitmap dictionary.
         Othewise, `wx.NullBitmap` is returned.
        """

        if self._bitmaps.has_key(name):
            return self._bitmaps[name]

        return wx.NullBitmap


    def Get(self):
        """ Accessor to the unique art manager object. """

        if not hasattr(self, "_instance"):
        
            self._instance = ArtManager()
            self._instance.Initialize()

            # Initialize the renderers map
            self._renderers[StyleXP] = RendererXP()
            self._renderers[Style2007] = RendererMSOffice2007()

        return self._instance

    Get = classmethod(Get)
    
    def Free(self):
        """ Destructor for the unique art manager object. """

        if hasattr(self, "_instance"):
        
            del self._instance

    Free = classmethod(Free)


    def OnSysColourChange(self, event):
        """
        Handles the ``wx.EVT_SYS_COLOUR_CHANGED`` event for L{ArtManager}.

        :param `event`: a `wx.SysColourChangedEvent` event to be processed.        
        """

        # reinitialise the colour map
        self.InitColours()


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


    def DarkColour(self, colour, percent):
        """
        Like the L{LightColour} function, but create the colour darker by `percent`.

        :param `colour`: the input colour to be darkened;
        :param `percent`: determines how dark the colour will be. `percent` = 100
         returns black, `percent` = 0 returns `colour`.
        """

        end_colour = wx.BLACK
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


    def PaintStraightGradientBox(self, dc, rect, startColour, endColour, vertical=True):
        """
        Paint the rectangle with gradient colouring; the gradient lines are either
        horizontal or vertical.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the rectangle to be filled with gradient shading;
        :param `startColour`: the first colour of the gradient shading;
        :param `endColour`: the second colour of the gradient shading;
        :param `vertical`: ``True`` for gradient colouring in the vertical direction,
         ``False`` for horizontal shading.
        """

        dcsaver = DCSaver(dc)
        
        if vertical:
            high = rect.GetHeight()-1
            direction = wx.SOUTH
        else:
            high = rect.GetWidth()-1
            direction = wx.EAST

        if high < 1:
            return

        dc.GradientFillLinear(rect, startColour, endColour, direction)
        

    def PaintGradientRegion(self, dc, region, startColour, endColour, vertical=True):
        """
        Paint a region with gradient colouring.

        :param `dc`: an instance of `wx.DC`;
        :param `region`: a region to be filled with gradient shading (an instance of
         `wx.Region`);
        :param `startColour`: the first colour of the gradient shading;
        :param `endColour`: the second colour of the gradient shading;
        :param `vertical`: ``True`` for gradient colouring in the vertical direction,
         ``False`` for horizontal shading.
 
        """

        # The way to achieve non-rectangle 
        memDC = wx.MemoryDC()
        rect = region.GetBox()
        bitmap = wx.EmptyBitmap(rect.width, rect.height)
        memDC.SelectObject(bitmap)

        # Colour the whole rectangle with gradient
        rr = wx.Rect(0, 0, rect.width, rect.height)
        self.PaintStraightGradientBox(memDC, rr, startColour, endColour, vertical)

        # Convert the region to a black and white bitmap with the white pixels being inside the region
        # we draw the bitmap over the gradient coloured rectangle, with mask set to white, 
        # this will cause our region to be coloured with the gradient, while area outside the 
        # region will be painted with black. then we simply draw the bitmap to the dc with mask set to 
        # black
        tmpRegion = wx.Region(rect.x, rect.y, rect.width, rect.height)
        tmpRegion.Offset(-rect.x, -rect.y)
        regionBmp = tmpRegion.ConvertToBitmap()
        regionBmp.SetMask(wx.Mask(regionBmp, wx.WHITE))

        # The function ConvertToBitmap() return a rectangle bitmap
        # which is shorter by 1 pixl on the height and width (this is correct behavior, since 
        # DrawLine does not include the second point as part of the line)
        # we fix this issue by drawing our own line at the bottom and left side of the rectangle
        memDC.SetPen(wx.BLACK_PEN)
        memDC.DrawBitmap(regionBmp, 0, 0, True)
        memDC.DrawLine(0, rr.height - 1, rr.width, rr.height - 1)
        memDC.DrawLine(rr.width - 1, 0, rr.width - 1, rr.height)

        memDC.SelectObject(wx.NullBitmap)
        bitmap.SetMask(wx.Mask(bitmap, wx.BLACK))
        dc.DrawBitmap(bitmap, rect.x, rect.y, True)


    def PaintDiagonalGradientBox(self, dc, rect, startColour, endColour,
                                 startAtUpperLeft=True, trimToSquare=True):
        """
        Paint rectangle with gradient colouring; the gradient lines are diagonal
        and may start from the upper left corner or from the upper right corner.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the rectangle to be filled with gradient shading;
        :param `startColour`: the first colour of the gradient shading;
        :param `endColour`: the second colour of the gradient shading;
        :param `startAtUpperLeft`: ``True`` to start the gradient lines at the upper
         left corner of the rectangle, ``False`` to start at the upper right corner;
        :param `trimToSquare`: ``True`` to trim the gradient lines in a square.
        """

        # Save the current pen and brush
        savedPen = dc.GetPen()
        savedBrush = dc.GetBrush()

        # gradient fill from colour 1 to colour 2 with top to bottom
        if rect.height < 1 or rect.width < 1:
            return

        # calculate some basic numbers
        size = rect.width
        sizeX = sizeY = 0
        proportion = 1
        
        if rect.width > rect.height:
        
            if trimToSquare:
            
                size = rect.height
                sizeX = sizeY = rect.height - 1
            
            else:
            
                proportion = float(rect.height)/float(rect.width)
                size = rect.width
                sizeX = rect.width - 1
                sizeY = rect.height -1
            
        else:
        
            if trimToSquare:
            
                size = rect.width
                sizeX = sizeY = rect.width - 1
            
            else:
            
                sizeX = rect.width - 1
                size = rect.height
                sizeY = rect.height - 1
                proportion = float(rect.width)/float(rect.height)

        # calculate gradient coefficients
        col2 = endColour
        col1 = startColour

        rf, gf, bf = 0, 0, 0
        rstep = float(col2.Red() - col1.Red())/float(size)
        gstep = float(col2.Green() - col1.Green())/float(size)
        bstep = float(col2.Blue() - col1.Blue())/float(size)
        
        # draw the upper triangle
        for i in xrange(size):
        
            currCol = wx.Colour(col1.Red() + rf, col1.Green() + gf, col1.Blue() + bf)
            dc.SetBrush(wx.Brush(currCol, wx.SOLID))
            dc.SetPen(wx.Pen(currCol))
            
            if startAtUpperLeft:
            
                if rect.width > rect.height:
                
                    dc.DrawLine(rect.x + i, rect.y, rect.x, int(rect.y + proportion*i))
                    dc.DrawPoint(rect.x, int(rect.y + proportion*i))
                
                else:
                
                    dc.DrawLine(int(rect.x + proportion*i), rect.y, rect.x, rect.y + i)
                    dc.DrawPoint(rect.x, rect.y + i)
                
            else:
            
                if rect.width > rect.height:
                
                    dc.DrawLine(rect.x + sizeX - i, rect.y, rect.x + sizeX, int(rect.y + proportion*i))
                    dc.DrawPoint(rect.x + sizeX, int(rect.y + proportion*i))
                
                else:
                
                    xTo = (int(rect.x + sizeX - proportion * i) > rect.x and [int(rect.x + sizeX - proportion*i)] or [rect.x])[0]
                    dc.DrawLine(xTo, rect.y, rect.x + sizeX, rect.y + i)
                    dc.DrawPoint(rect.x + sizeX, rect.y + i)
                
            rf += rstep/2
            gf += gstep/2
            bf += bstep/2
        
        # draw the lower triangle
        for i in xrange(size):

            currCol = wx.Colour(col1.Red() + rf, col1.Green() + gf, col1.Blue() + bf)        
            dc.SetBrush(wx.Brush(currCol, wx.SOLID))
            dc.SetPen(wx.Pen(currCol))
            
            if startAtUpperLeft:
            
                if rect.width > rect.height:
                
                    dc.DrawLine(rect.x + i, rect.y + sizeY, rect.x + sizeX, int(rect.y + proportion * i))
                    dc.DrawPoint(rect.x + sizeX, int(rect.y + proportion * i))
                
                else:
                
                    dc.DrawLine(int(rect.x + proportion * i), rect.y + sizeY, rect.x + sizeX, rect.y + i)
                    dc.DrawPoint(rect.x + sizeX, rect.y + i)
                
            else:
            
                if rect.width > rect.height:
                
                    dc.DrawLine(rect.x, (int)(rect.y + proportion * i), rect.x + sizeX - i, rect.y + sizeY)
                    dc.DrawPoint(rect.x + sizeX - i, rect.y + sizeY)
                
                else:
                
                    xTo = (int(rect.x + sizeX - proportion*i) > rect.x and [int(rect.x + sizeX - proportion*i)] or [rect.x])[0]
                    dc.DrawLine(rect.x, rect.y + i, xTo, rect.y + sizeY)
                    dc.DrawPoint(xTo, rect.y + sizeY)
                
            rf += rstep/2
            gf += gstep/2
            bf += bstep/2
        

        # Restore the pen and brush
        dc.SetPen( savedPen )
        dc.SetBrush( savedBrush )


    def PaintCrescentGradientBox(self, dc, rect, startColour, endColour, concave=True):
        """
        Paint a region with gradient colouring. The gradient is in crescent shape
        which fits the 2007 style.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the rectangle to be filled with gradient shading;
        :param `startColour`: the first colour of the gradient shading;
        :param `endColour`: the second colour of the gradient shading;
        :param `concave`: ``True`` for a concave effect, ``False`` for a convex one.
        """

        diagonalRectWidth = rect.GetWidth()/4
        spare = rect.width - 4*diagonalRectWidth
        leftRect = wx.Rect(rect.x, rect.y, diagonalRectWidth, rect.GetHeight())
        rightRect = wx.Rect(rect.x + 3 * diagonalRectWidth + spare, rect.y, diagonalRectWidth, rect.GetHeight())
        
        if concave:
        
            self.PaintStraightGradientBox(dc, rect, self.MixColours(startColour, endColour, 50), endColour)
            self.PaintDiagonalGradientBox(dc, leftRect, startColour, endColour, True, False) 
            self.PaintDiagonalGradientBox(dc, rightRect, startColour, endColour, False, False) 
        
        else:
        
            self.PaintStraightGradientBox(dc, rect, endColour, self.MixColours(endColour, startColour, 50))
            self.PaintDiagonalGradientBox(dc, leftRect, endColour, startColour, False, False) 
            self.PaintDiagonalGradientBox(dc, rightRect, endColour, startColour, True, False) 


    def FrameColour(self):
        """ Return the surrounding colour for a control. """

        return wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)


    def BackgroundColour(self):
        """ Returns the background colour of a control when not in focus. """

        return self.LightColour(self.FrameColour(), 75)


    def HighlightBackgroundColour(self):
        """ Returns the background colour of a control when it is in focus. """

        return self.LightColour(self.FrameColour(), 60)


    def MixColours(self, firstColour, secondColour, percent):
        """
        Return mix of input colours.

        :param `firstColour`: the first colour to be mixed, an instance of `wx.Colour`;
        :param `secondColour`: the second colour to be mixed, an instance of `wx.Colour`;
        :param `percent`: the relative percentage of `firstColour` with respect to
         `secondColour`.
        """

        # calculate gradient coefficients
        redOffset = float((secondColour.Red() * (100 - percent) / 100) - (firstColour.Red() * percent / 100))
        greenOffset = float((secondColour.Green() * (100 - percent) / 100) - (firstColour.Green() * percent / 100))
        blueOffset = float((secondColour.Blue() * (100 - percent) / 100) -  (firstColour.Blue() * percent / 100))

        return wx.Colour(firstColour.Red() + redOffset, firstColour.Green() + greenOffset,
                        firstColour.Blue() + blueOffset)


    def RandomColour(): 
        """ Creates a random colour. """
        
        r = random.randint(0, 255) # Random value betweem 0-255
        g = random.randint(0, 255) # Random value betweem 0-255
        b = random.randint(0, 255) # Random value betweem 0-255

        return wx.Colour(r, g, b)


    def IsDark(self, colour):
        """
        Returns whether a colour is dark or light.

        :param `colour`: an instance of `wx.Colour`.
        """

        evg = (colour.Red() + colour.Green() + colour.Blue())/3
        
        if evg < 127:
            return True

        return False


    def TruncateText(self, dc, text, maxWidth):
        """
        Truncates a given string to fit given width size. if the text does not fit
        into the given width it is truncated to fit. the format of the fixed text
        is <truncate text ...>.

        :param `dc`: an instance of `wx.DC`;
        :param `text`: the text to be (eventually) truncated;
        :param `maxWidth`: the maximum width allowed for the text.
        """

        textLen = len(text)
        tempText = text
        rectSize = maxWidth

        fixedText = ""
        
        textW, textH = dc.GetTextExtent(text)

        if rectSize >= textW:        
            return text
        
        # The text does not fit in the designated area, 
        # so we need to truncate it a bit
        suffix = ".."
        w, h = dc.GetTextExtent(suffix)
        rectSize -= w

        for i in xrange(textLen, -1, -1):
        
            textW, textH = dc.GetTextExtent(tempText)
            if rectSize >= textW:
                fixedText = tempText
                fixedText += ".."
                return fixedText
            
            tempText = tempText[:-1]


    def DrawButton(self, dc, rect, theme, state, input=None):
        """
        Colour rectangle according to the theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the rectangle to be filled with gradient shading;
        :param `theme`: the theme to use to draw the button;
        :param `state`: the button state;
        :param `input`: a flag used to call the right method.
        """

        if input is None or type(input) == type(False):
            self.DrawButtonTheme(dc, rect, theme, state, input)
        else:
            self.DrawButtonColour(dc, rect, theme, state, input)
            
                           
    def DrawButtonTheme(self, dc, rect, theme, state, useLightColours=True):
        """
        Draws a button using the appropriate theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `theme`: the theme to use to draw the button;
        :param `state`: the button state;
        :param `useLightColours`: ``True`` to use light colours, ``False`` otherwise.
        """

        renderer = self._renderers[theme]
        
        # Set background colour if non given by caller
        renderer.DrawButton(dc, rect, state, useLightColours)


    def DrawButtonColour(self, dc, rect, theme, state, colour):
        """
        Draws a button using the appropriate theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `theme`: the theme to use to draw the button;
        :param `state`: the button state;
        :param `colour`: a valid `wx.Colour` instance.
        """

        renderer = self._renderers[theme]
        renderer.DrawButton(dc, rect, state, colour)


    def CanMakeWindowsTransparent(self):
        """
        Used internally.
        
        :return: ``True`` if the system supports transparency of toplevel windows,
         otherwise returns ``False``.
        """

        if wx.Platform == "__WXMSW__":

            version = wx.GetOsDescription()
            found = version.find("XP") >= 0 or version.find("2000") >= 0 or version.find("NT") >= 0
            return found

        elif wx.Platform == "__WXMAC__":
            return True
        else:
            return False
        

    # on supported windows systems (Win2000 and greater), this function
    # will make a frame window transparent by a certain amount
    def MakeWindowTransparent(self, wnd, amount):
        """
        Used internally. Makes a toplevel window transparent if the system supports it.

        :param `wnd`: the toplevel window to make transparent;
        :param `amount`: the window transparency to apply.
        """

        if wnd.GetSize() == (0, 0):
            return

        # this API call is not in all SDKs, only the newer ones, so
        # we will runtime bind this
        if wx.Platform == "__WXMSW__":
            hwnd = wnd.GetHandle()
    
            if not hasattr(self, "_winlib"):
                if _libimported == "MH":
                    self._winlib = win32api.LoadLibrary("user32")
                elif _libimported == "ctypes":
                    self._winlib = ctypes.windll.user32
                    
            if _libimported == "MH":
                pSetLayeredWindowAttributes = win32api.GetProcAddress(self._winlib,
                                                                      "SetLayeredWindowAttributes")
                
                if pSetLayeredWindowAttributes == None:
                    return
                    
                exstyle = win32api.GetWindowLong(hwnd, win32con.GWL_EXSTYLE)
                if 0 == (exstyle & 0x80000):
                    win32api.SetWindowLong(hwnd, win32con.GWL_EXSTYLE, exstyle | 0x80000)  
                         
                winxpgui.SetLayeredWindowAttributes(hwnd, 0, amount, 2)
    
            elif _libimported == "ctypes":
                style = self._winlib.GetWindowLongA(hwnd, 0xffffffecL)
                style |= 0x00080000
                self._winlib.SetWindowLongA(hwnd, 0xffffffecL, style)
                self._winlib.SetLayeredWindowAttributes(hwnd, 0, amount, 2)                
        else:
            if not wnd.CanSetTransparent():
                return        
            wnd.SetTransparent(amount)
            return


    # assumption: the background was already drawn on the dc
    def DrawBitmapShadow(self, dc, rect, where=BottomShadow|RightShadow):
        """
        Draws a shadow using background bitmap.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the bitmap's client rectangle;
        :param `where`: where to draw the shadow. This can be any combination of the
         following bits:

         ========================== ======= ================================
         Shadow Settings             Value  Description
         ========================== ======= ================================
         ``RightShadow``                  1 Right side shadow
         ``BottomShadow``                 2 Not full bottom shadow
         ``BottomShadowFull``             4 Full bottom shadow
         ========================== ======= ================================
         
        """
    
        shadowSize = 5

        # the rect must be at least 5x5 pixles
        if rect.height < 2*shadowSize or rect.width < 2*shadowSize:
            return

        # Start by drawing the right bottom corner
        if where & BottomShadow or where & BottomShadowFull:
            dc.DrawBitmap(self._rightBottomCorner, rect.x+rect.width, rect.y+rect.height, True)

        # Draw right side shadow
        xx = rect.x + rect.width
        yy = rect.y + rect.height - shadowSize

        if where & RightShadow:
            while yy - rect.y > 2*shadowSize:
                dc.DrawBitmap(self._right, xx, yy, True)
                yy -= shadowSize
            
            dc.DrawBitmap(self._rightTop, xx, yy - shadowSize, True)

        if where & BottomShadow:
            xx = rect.x + rect.width - shadowSize
            yy = rect.height + rect.y
            while xx - rect.x > 2*shadowSize:
                dc.DrawBitmap(self._bottom, xx, yy, True)
                xx -= shadowSize
                
            dc.DrawBitmap(self._bottomLeft, xx - shadowSize, yy, True)

        if where & BottomShadowFull:
            xx = rect.x + rect.width - shadowSize
            yy = rect.height + rect.y
            while xx - rect.x >= 0:
                dc.DrawBitmap(self._bottom, xx, yy, True)
                xx -= shadowSize
            
            dc.DrawBitmap(self._bottom, xx, yy, True)


    def DropShadow(self, wnd, drop=True):
        """
        Adds a shadow under the window (Windows Only).

        :param `wnd`: the window for which we are dropping a shadow;
        :param `drop`: ``True`` to drop a shadow, ``False`` to remove it.
        """

        if not self.CanMakeWindowsTransparent() or not _libimported:
            return
        
        if "__WXMSW__" in wx.Platform:

            hwnd = wnd.GetHandle()
            
            if not hasattr(self, "_winlib"):
                if _libimported == "MH":
                    self._winlib = win32api.LoadLibrary("user32")
                elif _libimported == "ctypes":
                    self._winlib = ctypes.windll.user32
            
            if _libimported == "MH":
                csstyle = win32api.GetWindowLong(hwnd, win32con.GCL_STYLE)
            else:
                csstyle = self._winlib.GetWindowLongA(hwnd, win32con.GCL_STYLE)
            
            if drop:
                if csstyle & CS_DROPSHADOW:
                    return
                else:
                    csstyle |= CS_DROPSHADOW     #Nothing to be done
                    
            else:

                if csstyle & CS_DROPSHADOW:
                    csstyle &= ~(CS_DROPSHADOW)
                else:
                    return  #Nothing to be done

            win32api.SetWindowLong(hwnd, win32con.GCL_STYLE, csstyle)
            

    def GetBitmapStartLocation(self, dc, rect, bitmap, text="", style=0):
        """
        Returns the top left `x` and `y` cordinates of the bitmap drawing.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the bitmap's client rectangle;
        :param `bitmap`: the bitmap associated with the button;
        :param `text`: the button label;
        :param `style`: the button style. This can be one of the following bits:

         ============================== ======= ================================
         Button style                    Value  Description
         ============================== ======= ================================
         ``BU_EXT_XP_STYLE``               1    A button with a XP style
         ``BU_EXT_2007_STYLE``             2    A button with a MS Office 2007 style
         ``BU_EXT_LEFT_ALIGN_STYLE``       4    A left-aligned button
         ``BU_EXT_CENTER_ALIGN_STYLE``     8    A center-aligned button
         ``BU_EXT_RIGHT_ALIGN_STYLE``      16   A right-aligned button
         ``BU_EXT_RIGHT_TO_LEFT_STYLE``    32   A button suitable for right-to-left languages
         ============================== ======= ================================
        
        """

        alignmentBuffer = self.GetAlignBuffer()

        # get the startLocationY
        fixedTextWidth = fixedTextHeight = 0

        if not text:
            fixedTextHeight = bitmap.GetHeight()
        else:
            fixedTextWidth, fixedTextHeight = dc.GetTextExtent(text)
            
        startLocationY = rect.y + (rect.height - fixedTextHeight)/2

        # get the startLocationX
        if style & BU_EXT_RIGHT_TO_LEFT_STYLE:
        
            startLocationX = rect.x + rect.width - alignmentBuffer - bitmap.GetWidth()
        
        else:
        
            if style & BU_EXT_RIGHT_ALIGN_STYLE:
            
                maxWidth = rect.x + rect.width - (2 * alignmentBuffer) - bitmap.GetWidth() # the alignment is for both sides
                
                # get the truncaed text. The text may stay as is, it is not a must that is will be trancated
                fixedText = self.TruncateText(dc, text, maxWidth)

                # get the fixed text dimentions
                fixedTextWidth, fixedTextHeight = dc.GetTextExtent(fixedText)

                # calculate the start location
                startLocationX = maxWidth - fixedTextWidth
            
            elif style & BU_EXT_LEFT_ALIGN_STYLE:
            
                # calculate the start location
                startLocationX = alignmentBuffer
            
            else: # meaning BU_EXT_CENTER_ALIGN_STYLE
            
                maxWidth = rect.x + rect.width - (2 * alignmentBuffer) - bitmap.GetWidth() # the alignment is for both sides

                # get the truncaed text. The text may stay as is, it is not a must that is will be trancated
                fixedText = self.TruncateText(dc, text, maxWidth)

                # get the fixed text dimentions
                fixedTextWidth, fixedTextHeight = dc.GetTextExtent(fixedText)

                if maxWidth > fixedTextWidth:
                
                    # calculate the start location
                    startLocationX = (maxWidth - fixedTextWidth) / 2
                
                else:
                
                    # calculate the start location
                    startLocationX = maxWidth - fixedTextWidth                    
        
        # it is very important to validate that the start location is not less than the alignment buffer
        if startLocationX < alignmentBuffer:
            startLocationX = alignmentBuffer

        return startLocationX, startLocationY            


    def GetTextStartLocation(self, dc, rect, bitmap, text, style=0):
        """
        Returns the top left `x` and `y` cordinates of the text drawing.
        In case the text is too long, the text is being fixed (the text is cut and
        a '...' mark is added in the end).

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the text's client rectangle;
        :param `bitmap`: the bitmap associated with the button;
        :param `text`: the button label;
        :param `style`: the button style. 

        :see: L{GetBitmapStartLocation} for a list of valid button styles.
        """

        alignmentBuffer = self.GetAlignBuffer()

        # get the bitmap offset
        bitmapOffset = 0
        if bitmap != wx.NullBitmap:
            bitmapOffset = bitmap.GetWidth()

        # get the truncated text. The text may stay as is, it is not a must that is will be trancated
        maxWidth = rect.x + rect.width - (2 * alignmentBuffer) - bitmapOffset # the alignment is for both sides

        fixedText = self.TruncateText(dc, text, maxWidth)

        # get the fixed text dimentions
        fixedTextWidth, fixedTextHeight = dc.GetTextExtent(fixedText)
        startLocationY = (rect.height - fixedTextHeight) / 2 + rect.y

        # get the startLocationX
        if style & BU_EXT_RIGHT_TO_LEFT_STYLE:
        
            startLocationX = maxWidth - fixedTextWidth + alignmentBuffer
        
        else:
        
            if style & BU_EXT_LEFT_ALIGN_STYLE:
            
                # calculate the start location
                startLocationX = bitmapOffset + alignmentBuffer
            
            elif style & BU_EXT_RIGHT_ALIGN_STYLE:
            
                # calculate the start location
                startLocationX = maxWidth - fixedTextWidth + bitmapOffset + alignmentBuffer
            
            else: # meaning wxBU_EXT_CENTER_ALIGN_STYLE
            
                # calculate the start location
                startLocationX = (maxWidth - fixedTextWidth) / 2 + bitmapOffset + alignmentBuffer
            
        
        # it is very important to validate that the start location is not less than the alignment buffer
        if startLocationX < alignmentBuffer:
            startLocationX = alignmentBuffer

        return startLocationX, startLocationY, fixedText
    

    def DrawTextAndBitmap(self, dc, rect, text, enable=True, font=wx.NullFont,
                          fontColour=wx.BLACK, bitmap=wx.NullBitmap,
                          grayBitmap=wx.NullBitmap, style=0):
        """
        Draws the text & bitmap on the input dc.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the text and bitmap client rectangle;
        :param `text`: the button label;
        :param `enable`: ``True`` if the button is enabled, ``False`` otherwise;
        :param `font`: the font to use to draw the text, an instance of `wx.Font`;
        :param `fontColour`: the colour to use to draw the text, an instance of
         `wx.Colour`;
        :param `bitmap`: the bitmap associated with the button;
        :param `grayBitmap`: a greyed-out version of the input `bitmap` representing
         a disabled bitmap;
        :param `style`: the button style. 

        :see: L{GetBitmapStartLocation} for a list of valid button styles.
        """

        # enable colours
        if enable:
            dc.SetTextForeground(fontColour)
        else:
            dc.SetTextForeground(wx.SystemSettings_GetColour(wx.SYS_COLOUR_GRAYTEXT))
        
        # set the font
        
        if font == wx.NullFont:
            font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            
        dc.SetFont(font)
        
        startLocationX = startLocationY = 0
        
        if bitmap != wx.NullBitmap:
        
            # calculate the bitmap start location
            startLocationX, startLocationY = self.GetBitmapStartLocation(dc, rect, bitmap, text, style)

            # draw the bitmap
            if enable:
                dc.DrawBitmap(bitmap, startLocationX, startLocationY, True)
            else:
                dc.DrawBitmap(grayBitmap, startLocationX, startLocationY, True)
   
        # calculate the text start location
        location, labelOnly = self.GetAccelIndex(text)
        startLocationX, startLocationY, fixedText = self.GetTextStartLocation(dc, rect, bitmap, labelOnly, style)

        # after all the caculations are finished, it is time to draw the text
        # underline the first letter that is marked with a '&'
        if location == -1 or font.GetUnderlined() or location >= len(fixedText):
            # draw the text
            dc.DrawText(fixedText, startLocationX, startLocationY)
        
        else:
            
            # underline the first '&'
            before = fixedText[0:location]
            underlineLetter = fixedText[location] 
            after = fixedText[location+1:]

            # before
            dc.DrawText(before, startLocationX, startLocationY)

            # underlineLetter
            if "__WXGTK__" not in wx.Platform:
                w1, h = dc.GetTextExtent(before)
                font.SetUnderlined(True)
                dc.SetFont(font)
                dc.DrawText(underlineLetter, startLocationX + w1, startLocationY)
            else:
                w1, h = dc.GetTextExtent(before)
                dc.DrawText(underlineLetter, startLocationX + w1, startLocationY)

                # Draw the underline ourselves since using the Underline in GTK, 
                # causes the line to be too close to the letter
                uderlineLetterW, uderlineLetterH = dc.GetTextExtent(underlineLetter)

                curPen = dc.GetPen()
                dc.SetPen(wx.BLACK_PEN)

                dc.DrawLine(startLocationX + w1, startLocationY + uderlineLetterH - 2,
                            startLocationX + w1 + uderlineLetterW, startLocationY + uderlineLetterH - 2)
                dc.SetPen(curPen)

            # after
            w2, h = dc.GetTextExtent(underlineLetter)
            font.SetUnderlined(False)
            dc.SetFont(font)
            dc.DrawText(after, startLocationX + w1 + w2, startLocationY)


    def CalcButtonBestSize(self, label, bmp):
        """
        Returns the best fit size for the supplied label & bitmap.

        :param `label`: the button label;
        :param `bmp`: the bitmap associated with the button.
        """

        if "__WXMSW__" in wx.Platform:
            HEIGHT = 22
        else:
            HEIGHT = 26

        dc = wx.MemoryDC()
        dc.SelectBitmap(wx.EmptyBitmap(1, 1))

        dc.SetFont(wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT))
        width, height, dummy = dc.GetMultiLineTextExtent(label)

        width += 2*self.GetAlignBuffer() 

        if bmp.Ok():
        
            # allocate extra space for the bitmap
            heightBmp = bmp.GetHeight() + 2
            if height < heightBmp:
                height = heightBmp

            width += bmp.GetWidth() + 2
        
        if height < HEIGHT:
            height = HEIGHT

        dc.SelectBitmap(wx.NullBitmap)
        
        return wx.Size(width, height)


    def GetMenuFaceColour(self):
        """ Returns the colour used for the menu foreground. """

        renderer = self._renderers[self.GetMenuTheme()]
        return renderer.GetMenuFaceColour()


    def GetTextColourEnable(self):
        """ Returns the colour used for enabled menu items. """

        renderer = self._renderers[self.GetMenuTheme()]
        return renderer.GetTextColourEnable()


    def GetTextColourDisable(self):
        """ Returns the colour used for disabled menu items. """

        renderer = self._renderers[self.GetMenuTheme()]
        return renderer.GetTextColourDisable()


    def GetFont(self):
        """ Returns the font used by this theme. """

        renderer = self._renderers[self.GetMenuTheme()]
        return renderer.GetFont()


    def GetAccelIndex(self, label):
        """
        Returns the mnemonic index of the label and the label stripped of the ampersand mnemonic
        (e.g. 'lab&el' ==> will result in 3 and labelOnly = label).

        :param `label`: a string containining an ampersand.        
        """

        indexAccel = 0
        while True:
            indexAccel = label.find("&", indexAccel)
            if indexAccel == -1:
                return indexAccel, label
            if label[indexAccel:indexAccel+2] == "&&":
                label = label[0:indexAccel] + label[indexAccel+1:]
                indexAccel += 1
            else:
                break

        labelOnly = label[0:indexAccel] + label[indexAccel+1:]

        return indexAccel, labelOnly
        

    def GetThemeBaseColour(self, useLightColours=True):
        """
        Returns the theme (Blue, Silver, Green etc.) base colour, if no theme is active
        it return the active caption colour, lighter in 30%.

        :param `useLightColours`: ``True`` to use light colours, ``False`` otherwise.        
        """

        if not useLightColours and not self.IsDark(self.FrameColour()):
            return wx.NamedColour("GOLD")
        else:
            return self.LightColour(self.FrameColour(), 30)


    def GetAlignBuffer(self):
        """ Return the padding buffer for a text or bitmap. """

        return self._alignmentBuffer


    def SetMenuTheme(self, theme):
        """
        Set the menu theme, possible values (Style2007, StyleXP, StyleVista).

        :param `theme`: a rendering theme class, either `StyleXP`, `Style2007` or `StyleVista`.
        """
        
        self._menuTheme = theme


    def GetMenuTheme(self):
        """ Returns the currently used menu theme. """

        return self._menuTheme


    def AddMenuTheme(self, render):
        """
        Adds a new theme to the stock.

        :param `render`: a rendering theme class, which must be derived from
         L{RendererBase}.
        """

        # Add new theme
        lastRenderer = len(self._renderers)
        self._renderers[lastRenderer] = render
        
        return lastRenderer
    

    def SetMS2007ButtonSunken(self, sunken):
        """
        Sets MS 2007 button style sunken or not.

        :param `sunken`: ``True`` to have a sunken border effect, ``False`` otherwise.
        """
        
        self._ms2007sunken = sunken


    def GetMS2007ButtonSunken(self):
        """ Returns the sunken flag for MS 2007 buttons. """

        return self._ms2007sunken


    def GetMBVerticalGradient(self):
        """ Returns ``True`` if the menu bar should be painted with vertical gradient. """

        return self._verticalGradient


    def SetMBVerticalGradient(self, v):
        """
        Sets the menu bar gradient style.

        :param `v`: ``True`` for a vertical shaded gradient, ``False`` otherwise.
        """

        self._verticalGradient = v


    def DrawMenuBarBorder(self, border):
        """
        Enables menu border drawing (XP style only).

        :param `border`: ``True`` to draw the menubar border, ``False`` otherwise.
        """

        self._drowMBBorder = border
        

    def GetMenuBarBorder(self):
        """ Returns menu bar border drawing flag. """

        return self._drowMBBorder


    def GetMenuBgFactor(self):
        """
        Gets the visibility depth of the menu in Metallic style.
        The higher the value, the menu bar will look more raised
        """

        return self._menuBgFactor        


    def DrawDragSash(self, rect):
        """
        Draws resize sash.

        :param `rect`: the sash client rectangle.
        """
  
        dc = wx.ScreenDC()
        mem_dc = wx.MemoryDC()
        
        bmp = wx.EmptyBitmap(rect.width, rect.height)
        mem_dc.SelectObject(bmp)
        mem_dc.SetBrush(wx.WHITE_BRUSH)
        mem_dc.SetPen(wx.Pen(wx.WHITE, 1))
        mem_dc.DrawRectangle(0, 0, rect.width, rect.height)

        dc.Blit(rect.x, rect.y, rect.width, rect.height, mem_dc, 0, 0, wx.XOR)


    def TakeScreenShot(self, rect, bmp):
        """
        Takes a screenshot of the screen at given position & size (rect).

        :param `rect`: the screen rectangle we wish to capture;
        :param `bmp`: currently unused.
        """

        # Create a DC for the whole screen area
        dcScreen = wx.ScreenDC()

        # Create a Bitmap that will later on hold the screenshot image
        # Note that the Bitmap must have a size big enough to hold the screenshot
        # -1 means using the current default colour depth
        bmp = wx.EmptyBitmap(rect.width, rect.height)

        # Create a memory DC that will be used for actually taking the screenshot
        memDC = wx.MemoryDC()

        # Tell the memory DC to use our Bitmap
        # all drawing action on the memory DC will go to the Bitmap now
        memDC.SelectObject(bmp)

        # Blit (in this case copy) the actual screen on the memory DC
        # and thus the Bitmap
        memDC.Blit( 0,   # Copy to this X coordinate
            0,           # Copy to this Y coordinate
            rect.width,  # Copy this width
            rect.height, # Copy this height
            dcScreen,    # From where do we copy?
            rect.x,      # What's the X offset in the original DC?
            rect.y       # What's the Y offset in the original DC?
            )

        # Select the Bitmap out of the memory DC by selecting a new
        # uninitialized Bitmap
        memDC.SelectObject(wx.NullBitmap)


    def DrawToolBarBg(self, dc, rect):
        """
        Draws the toolbar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the toolbar's client rectangle.
        """

        renderer = self._renderers[self.GetMenuTheme()]
        
        # Set background colour if non given by caller
        renderer.DrawToolBarBg(dc, rect)


    def DrawMenuBarBg(self, dc, rect):
        """
        Draws the menu bar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the menubar's client rectangle.
        """

        renderer = self._renderers[self.GetMenuTheme()]
        # Set background colour if non given by caller
        renderer.DrawMenuBarBg(dc, rect)


    def SetMenuBarColour(self, scheme):
        """
        Sets the menu bar colour scheme to use.

        :param `scheme`: a string representing a colour scheme (i.e., 'Default',
         'Dark', 'Dark Olive Green', 'Generic').
        """

        self._menuBarColourScheme = scheme
        # set default colour
        if scheme in self._colourSchemeMap.keys():
            self._menuBarBgColour = self._colourSchemeMap[scheme]


    def GetMenuBarColourScheme(self):
        """ Returns the current colour scheme. """

        return self._menuBarColourScheme


    def GetMenuBarFaceColour(self):
        """ Returns the menu bar face colour. """

        return self._menuBarBgColour


    def GetMenuBarSelectionColour(self):
        """ Returns the menu bar selection colour. """

        return self._menuBarSelColour


    def InitColours(self):
        """ Initialise the colour map. """

        self._colourSchemeMap = {_("Default"): wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE),
                                _("Dark"): wx.BLACK,
                                _("Dark Olive Green"): wx.NamedColour("DARK OLIVE GREEN"),
                                _("Generic"): wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)}


    def GetColourSchemes(self):
        """ Returns the available colour schemes. """

        return self._colourSchemeMap.keys()     
                

    def CreateGreyBitmap(self, bmp):
        """
        Creates a grey bitmap image from the input bitmap.

        :param `bmp`: a valid `wx.Bitmap` object to be greyed out.
        """

        img = bmp.ConvertToImage()
        return wx.BitmapFromImage(img.ConvertToGreyscale())


    def GetRaiseToolbar(self):
        """ Returns ``True`` if we are dropping a shadow under a toolbar. """

        return self._raiseTB


    def SetRaiseToolbar(self, rais):
        """
        Enables/disables toobar shadow drop.

        :param `rais`: ``True`` to drop a shadow below a toolbar, ``False`` otherwise.
        """

        self._raiseTB = rais


