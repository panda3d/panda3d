###############################################################################
# Name: eclutil.py                                                            #
# Purpose: Common library utilities.                                          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Control Library: Editra Control Library Utility

Miscellaneous utility functions and gui helpers

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: eclutil.py 67596 2011-04-24 20:05:20Z CJP $"
__revision__ = "$Revision: 67596 $"

__all__ = ['AdjustAlpha', 'AdjustColour', 'BestLabelColour', 'HexToRGB',
           'GetHighlightColour', 'EmptyBitmapRGBA',

           'DRAW_CIRCLE_SMALL', 'DRAW_CIRCLE_NORMAL', 'DRAW_CIRCLE_LARGE',
           'DrawCircleCloseBmp' ]

#-----------------------------------------------------------------------------#
# Imports
import wx

if wx.Platform == '__WXMAC__':
    try:
        import Carbon.Appearance
    except ImportError:
        CARBON = False
    else:
        CARBON = True

#-----------------------------------------------------------------------------#

# DrawCircleCloseBmp options
DRAW_CIRCLE_SMALL  = 0
DRAW_CIRCLE_NORMAL = 1
DRAW_CIRCLE_LARGE  = 2

__CircleDefs = { DRAW_CIRCLE_SMALL : dict(size=(8, 8),
                                          xpath=((1.75, 2), (4.75, 5),
                                                 (1.75, 5), (4.75, 2))),
                 DRAW_CIRCLE_NORMAL : dict(size=(16, 16),
                                           xpath=((4.5, 4), (10.5, 10),
                                                  (4.5, 10), (10.5, 4))),
                 DRAW_CIRCLE_LARGE : dict(size=(32, 32),
                                          xpath=((8, 8), (20, 20),
                                                 (8, 20), (20, 8))) }

#-----------------------------------------------------------------------------#
# Colour Utilities

def AdjustAlpha(colour, alpha):
    """Adjust the alpha of a given colour"""
    return wx.Colour(colour.Red(), colour.Green(), colour.Blue(), alpha)

def AdjustColour(color, percent, alpha=wx.ALPHA_OPAQUE):
    """ Brighten/Darken input colour by percent and adjust alpha
    channel if needed. Returns the modified color.
    @param color: color object to adjust
    @type color: wx.Colour
    @param percent: percent to adjust +(brighten) or -(darken)
    @type percent: int
    @keyword alpha: amount to adjust alpha channel

    """
    radj, gadj, badj = [ int(val * (abs(percent) / 100.0))
                         for val in color.Get() ]

    if percent < 0:
        radj, gadj, badj = [ val * -1 for val in [radj, gadj, badj] ]
    else:
        radj, gadj, badj = [ val or 255 for val in [radj, gadj, badj] ]

    red = min(color.Red() + radj, 255)
    green = min(color.Green() + gadj, 255)
    blue = min(color.Blue() + badj, 255)
    return wx.Colour(red, green, blue, alpha)

def BestLabelColour(color):
    """Get the best color to use for the label that will be drawn on
    top of the given color.
    @param color: background color that text will be drawn on

    """
    avg = sum(color.Get()) // 3
    if avg > 192:
        txt_color = wx.BLACK
    elif avg > 128:
        txt_color = AdjustColour(color, -95)
    elif avg < 64:
        txt_color = wx.WHITE
    else:
        txt_color = AdjustColour(color, 95)
    return txt_color

def GetHighlightColour():
    """Get the default highlight color
    @return: wx.Colour

    """
    if wx.Platform == '__WXMAC__':
        if CARBON:
            if hasattr(wx, 'MacThemeColour'):
                color = wx.MacThemeColour(Carbon.Appearance.kThemeBrushFocusHighlight)
                return color
            else:
                # kThemeBrushButtonPressedLightHighlight
                brush = wx.Brush(wx.BLACK)
                brush.MacSetTheme(Carbon.Appearance.kThemeBrushFocusHighlight)
                return brush.GetColour()

    # Fallback to text highlight color
    return wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT)

def HexToRGB(hex_str):
    """Returns a list of red/green/blue values from a
    hex string.
    @param hex_str: hex string to convert to rgb

    """
    hexval = hex_str
    if hexval[0] == u"#":
        hexval = hexval[1:]
    ldiff = 6 - len(hexval)
    hexval += ldiff * u"0"
    # Convert hex values to integer
    red = int(hexval[0:2], 16)
    green = int(hexval[2:4], 16)
    blue = int(hexval[4:], 16)
    return [red, green, blue]

#-----------------------------------------------------------------------------#

def EmptyBitmapRGBA(width, height):
    """Create an empty bitmap with an alpha channel"""
    bmp = wx.EmptyBitmap(width, height, 32)
    if hasattr(bmp, 'UseAlpha'):
        bmp.UseAlpha()
    return bmp

#-----------------------------------------------------------------------------#
# Drawing helpers

def DrawCircleCloseBmp(colour, backColour=None, option=DRAW_CIRCLE_SMALL):
    """
    Draws a small circular close button.
    @param colour: Circle's background colour
    @keyword backColour: pen colour for border and X
    @keyword option: DRAW_CIRCLE_* value
    @return: wxBitmap

    """
    assert option in __CircleDefs, "Invalid DRAW option!"

    defs = __CircleDefs.get(option)
    size = defs['size']
    if option != DRAW_CIRCLE_SMALL:
        # Adjust for border
        diameter = size[0] - 1
    else:
        diameter = size[0]
    radius = float(diameter) / 2.0
    xpath = defs['xpath']

    bmp = EmptyBitmapRGBA(*size)
    dc = wx.MemoryDC()
    dc.SelectObject(bmp)

    gc = wx.GraphicsContext.Create(dc)
    gc.SetBrush(wx.Brush(colour))
    if option > DRAW_CIRCLE_SMALL:
        gc.SetPen(wx.Pen(AdjustColour(colour, -30)))
    else:
        gc.SetPen(wx.TRANSPARENT_PEN)

    path = gc.CreatePath()
    path.AddCircle(radius, radius, radius)
    path.CloseSubpath()
    gc.FillPath(path)
    gc.StrokePath(path)

    path = gc.CreatePath()
    if backColour is not None:
        pen = wx.Pen(backColour, 1)
    else:
        pen = wx.Pen("white", 1)

    pen.SetCap(wx.CAP_BUTT)
    pen.SetJoin(wx.JOIN_BEVEL)
    gc.SetPen(pen)
    path.MoveToPoint(*xpath[0])
    path.AddLineToPoint(*xpath[1])
    path.MoveToPoint(*xpath[2])
    path.AddLineToPoint(*xpath[3])
    path.CloseSubpath()
    gc.DrawPath(path)

    dc.SelectObject(wx.NullBitmap)
    return bmp
