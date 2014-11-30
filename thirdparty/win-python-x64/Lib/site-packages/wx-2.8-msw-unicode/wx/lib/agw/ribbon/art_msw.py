"""
L{RibbonMSWArtProvider} is responsible for drawing all the components of the ribbon
interface using a Windows appearance.


Description
===========

This allows a ribbon bar to have a pluggable look-and-feel, while retaining the same
underlying behaviour. As a single art provider is used for all ribbon components, a
ribbon bar usually has a consistent (though unique) appearance.

By default, a L{RibbonBar} uses an instance of a class called `RibbonDefaultArtProvider`,
which resolves to `RibbonAUIArtProvider`, `RibbonMSWArtProvider`, or `RibbonOSXArtProvider`
- whichever is most appropriate to the current platform. These art providers are all
slightly configurable with regard to colours and fonts, but for larger modifications,
you can derive from one of these classes, or write a completely new art provider class.

Call L{RibbonBar.SetArtProvider} to change the art provider being used.


See Also
========

L{RibbonBar}
"""

import wx
import types

from math import cos
from math import pi as M_PI

import panel as PANEL
import page as PAGE

from art_internal import RibbonLoadPixmap, RibbonInterpolateColour, RibbonDrawParallelGradientLines
from art_internal import RibbonCanLabelBreakAtPosition
from art_internal import RibbonHSLColour

from art import *


gallery_up_xpm = ["5 5 2 1", "  c None", "x c #FF00FF", "     ", "  x  ", " xxx ", "xxxxx", "     "]
gallery_down_xpm = ["5 5 2 1", "  c None", "x c #FF00FF", "     ", "xxxxx", " xxx ", "  x  ", "     "]
gallery_left_xpm = ["5 5 2 1", "  c None", "x c #FF00FF", "   x ", "  xx ", " xxx ", "  xx ", "   x "]
gallery_right_xpm = ["5 5 2 1", "  c None", "x c #FF00FF", " x   ", " xx  ", " xxx ", " xx  ", " x   "]
gallery_extension_xpm = ["5 5 2 1", "  c None", "x c #FF00FF", "xxxxx", "     ", "xxxxx", " xxx ", "  x  "]


def LikePrimary(primary_hsl, h, s, l):

    return primary_hsl.ShiftHue(h).Saturated(s).Lighter(l).ToRGB()


def LikeSecondary(secondary_hsl, h, s, l):
    
    return secondary_hsl.ShiftHue(h).Saturated(s).Lighter(l).ToRGB()


def SingleLine(dc, rect, start, finish):

    dc.DrawLine(start.x + rect.x, start.y + rect.y, finish.x + rect.x, finish.y + rect.y)

                               
class RibbonMSWArtProvider(object):

    def __init__(self, set_colour_scheme=True):

        self._flags = 0
        self._tab_label_font = wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL, False)
        self._button_bar_label_font = wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL, False)
        self._panel_label_font = wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL, False)

        self._gallery_up_bitmap = [wx.NullBitmap for i in xrange(4)]
        self._gallery_down_bitmap = [wx.NullBitmap for i in xrange(4)]
        self._gallery_extension_bitmap = [wx.NullBitmap for i in xrange(4)]

        if set_colour_scheme:
            self.SetColourScheme(wx.Colour(194, 216, 241), wx.Colour(255, 223, 114), wx.Colour(0, 0, 0))
        
        self._cached_tab_separator_visibility = -10.0 # valid visibilities are in range [0, 1]
        self._tab_separation_size = 3
        self._page_border_left = 2
        self._page_border_top = 1
        self._page_border_right = 2
        self._page_border_bottom = 3
        self._panel_x_separation_size = 1
        self._panel_y_separation_size = 1
        self._tool_group_separation_size = 3
        self._gallery_bitmap_padding_left_size = 4
        self._gallery_bitmap_padding_right_size = 4
        self._gallery_bitmap_padding_top_size = 4
        self._gallery_bitmap_padding_bottom__size = 4
        self._cached_tab_separator = wx.NullBitmap

            
    def GetColourScheme(self, primary, secondary, tertiary):
        """
        Get the current colour scheme.

        Returns three colours such that if L{SetColourScheme} were called with them, the
        colour scheme would be restored to what it was when L{SetColourScheme} was last
        called. In practice, this usually means that the returned values are the three
        colours given in the last call to L{SetColourScheme}, however if
        L{SetColourScheme} performs an idempotent operation upon the colours it is given
        (like clamping a component of the colour), then the returned values may not be
        the three colours given in the last call to L{SetColourScheme}.

        If L{SetColourScheme} has not been called, then the returned values should result
        in a colour scheme similar to, if not identical to, the default colours of the
        art provider. Note that if L{SetColour} is called, then L{GetColourScheme} does
        not try and return a colour scheme similar to colours being used - it's return
        values are dependant upon the last values given to L{SetColourScheme}, as
        described above.

        :param `primary`: Pointer to a location to store the primary colour, or ``None``;
        :param `secondary`: Pointer to a location to store the secondary colour, or ``None``;
        :param `tertiary`: Pointer to a location to store the tertiary colour, or ``None``.

        """

        if primary != None:
            primary = self._primary_scheme_colour
        if secondary != None:
            secondary = self._secondary_scheme_colour
        if tertiary != None:
            tertiary = self._tertiary_scheme_colour

        return primary, secondary, tertiary
    

    def SetColourScheme(self, primary, secondary, tertiary):
        """
        Set all applicable colour settings from a few base colours.

        Uses any or all of the three given colours to create a colour scheme, and then
        sets all colour settings which are relevant to the art provider using that
        scheme. Note that some art providers may not use the tertiary colour for
        anything, and some may not use the secondary colour either.

        :param `primary`: MISSING DESCRIPTION;
        :param `secondary`: MISSING DESCRIPTION;
        :param `tertiary`: MISSING DESCRIPTION.

        :see: L{SetColour}, L{GetColourScheme}
        """

        self._primary_scheme_colour = primary
        self._secondary_scheme_colour = secondary
        self._tertiary_scheme_colour = tertiary

        primary_hsl = RibbonHSLColour(primary)
        secondary_hsl = RibbonHSLColour(secondary)
        # tertiary not used for anything

        # Map primary saturation from [0, 1] to [.25, .75]
        primary_hsl.saturation = cos(primary_hsl.saturation * M_PI) * -0.25 + 0.5

        # Map primary luminance from [0, 1] to [.23, .83]
        primary_hsl.luminance = cos(primary_hsl.luminance * M_PI) * -0.3 + 0.53

        # Map secondary saturation from [0, 1] to [0.16, 0.84]
        secondary_hsl.saturation = cos(secondary_hsl.saturation * M_PI) * -0.34 + 0.5

        # Map secondary luminance from [0, 1] to [0.1, 0.9]
        secondary_hsl.luminance = cos(secondary_hsl.luminance * M_PI) * -0.4 + 0.5

        self._page_border_pen = wx.Pen(LikePrimary(primary_hsl, 1.4, 0.00, -0.08))

        self._page_background_top_colour = LikePrimary(primary_hsl, -0.1, -0.03, 0.12)
        self._page_hover_background_top_colour = LikePrimary(primary_hsl, -2.8, 0.27, 0.17)
        self._page_background_top_gradient_colour = LikePrimary(primary_hsl, 0.1, -0.10, 0.08)
        self._page_hover_background_top_gradient_colour = LikePrimary(primary_hsl, 3.2, 0.16, 0.13)
        self._page_background_colour = LikePrimary(primary_hsl, 0.4, -0.09, 0.05)
        self._page_hover_background_colour = LikePrimary(primary_hsl, 0.1, 0.19, 0.10)
        self._page_background_gradient_colour = LikePrimary(primary_hsl, -3.2, 0.27, 0.10)
        self._page_hover_background_gradient_colour = LikePrimary(primary_hsl, 1.8, 0.01, 0.15)

        self._tab_active_background_colour = LikePrimary(primary_hsl, -0.1, -0.31, 0.16)
        self._tab_active_background_gradient_colour = LikePrimary(primary_hsl, -0.1, -0.03, 0.12)
        self._tab_separator_colour = LikePrimary(primary_hsl, 0.9, 0.24, 0.05)
        self._tab_ctrl_background_brush = wx.Brush(LikePrimary(primary_hsl, 1.0, 0.39, 0.07))
        self._tab_hover_background_colour = LikePrimary(primary_hsl, 1.3, 0.15, 0.10)
        self._tab_hover_background_top_colour = LikePrimary(primary_hsl, 1.4, 0.36, 0.08)
        self._tab_border_pen = wx.Pen(LikePrimary(primary_hsl, 1.4, 0.03, -0.05)  )
        self._tab_separator_gradient_colour = LikePrimary(primary_hsl, 1.7, -0.15, -0.18)
        self._tab_hover_background_top_gradient_colour = LikePrimary(primary_hsl, 1.8, 0.34, 0.13)   
        self._tab_label_colour = LikePrimary(primary_hsl, 4.3, 0.13, -0.49)
        self._tab_hover_background_gradient_colour = LikeSecondary(primary_hsl, -1.5, -0.34, 0.01)

        self._panel_minimised_border_gradient_pen = wx.Pen(LikePrimary(primary_hsl, -6.9, -0.17, -0.09))
        self._panel_minimised_border_pen = wx.Pen(LikePrimary(primary_hsl, -5.3, -0.24, -0.06))
        self._panel_border_gradient_pen = wx.Pen(LikePrimary(primary_hsl, -5.2, -0.15, -0.06))
        self._panel_border_pen = wx.Pen(LikePrimary(primary_hsl, -2.8, -0.32, 0.02))
        self._panel_label_background_brush = wx.Brush(LikePrimary(primary_hsl, -1.5, 0.03, 0.05))
        self._panel_active_background_gradient_colour = LikePrimary(primary_hsl, 0.5, 0.34, 0.05)
        self._panel_hover_label_background_brush = wx.Brush(LikePrimary(primary_hsl, 1.0, 0.30, 0.09))
        self._panel_active_background_top_gradient_colour = LikePrimary(primary_hsl, 1.4, -0.17, -0.13)
        self._panel_active_background_colour = LikePrimary(primary_hsl, 1.6, -0.18, -0.18)
        self._panel_active_background_top_colour = LikePrimary(primary_hsl, 1.7, -0.20, -0.03)
        self._panel_label_colour = LikePrimary(primary_hsl, 2.8, -0.14, -0.35)
        self._panel_hover_label_colour = self._panel_label_colour
        self._panel_minimised_label_colour = self._tab_label_colour

        self._gallery_button_disabled_background_colour = LikePrimary(primary_hsl, -2.8, -0.46, 0.09)
        self._gallery_button_disabled_background_top_brush = wx.Brush(LikePrimary(primary_hsl, -2.8, -0.36, 0.15))
        self._gallery_hover_background_brush = wx.Brush(LikePrimary(primary_hsl, -0.8, 0.05, 0.15))
        self._gallery_border_pen = wx.Pen(LikePrimary(primary_hsl, 0.7, -0.02, 0.03))
        self._gallery_button_background_top_brush = wx.Brush(LikePrimary(primary_hsl, 0.8, 0.34, 0.13))
        self._gallery_button_background_colour = LikePrimary(primary_hsl, 1.3, 0.10, 0.08)
        # SetColour used so that the relevant bitmaps are generated
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_FACE_COLOUR, LikePrimary(primary_hsl, 1.4, -0.21, -0.23))
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_HOVER_FACE_COLOUR, LikePrimary(primary_hsl, 1.5, -0.24, -0.29))
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_ACTIVE_FACE_COLOUR, LikePrimary(primary_hsl, 1.5, -0.24, -0.29))
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_DISABLED_FACE_COLOUR, LikePrimary(primary_hsl, 0.0, -1.0, 0.0))
        self._gallery_button_disabled_background_gradient_colour = LikePrimary(primary_hsl, 1.5, -0.43, 0.12)
        self._gallery_button_background_gradient_colour = LikePrimary(primary_hsl, 1.7, 0.11, 0.09)
        self._gallery_item_border_pen = wx.Pen(LikeSecondary(secondary_hsl, -3.9, -0.16, -0.14))
        self._gallery_button_hover_background_colour = LikeSecondary(secondary_hsl, -0.9, 0.16, -0.07)
        self._gallery_button_hover_background_gradient_colour = LikeSecondary(secondary_hsl, 0.1, 0.12, 0.03)
        self._gallery_button_hover_background_top_brush = wx.Brush(LikeSecondary(secondary_hsl, 4.3, 0.16, 0.17))

        self._gallery_button_active_background_colour = LikeSecondary(secondary_hsl, -9.9, 0.03, -0.22)
        self._gallery_button_active_background_gradient_colour = LikeSecondary(secondary_hsl, -9.5, 0.14, -0.11)
        self._gallery_button_active_background_top_brush = wx.Brush(LikeSecondary(secondary_hsl, -9.0, 0.15, -0.08))
        
        self._button_bar_label_colour = self._tab_label_colour
        self._button_bar_hover_border_pen = wx.Pen(LikeSecondary(secondary_hsl, -6.2, -0.47, -0.14))
        self._button_bar_hover_background_gradient_colour = LikeSecondary(secondary_hsl, -0.6, 0.16, 0.04)
        self._button_bar_hover_background_colour = LikeSecondary(secondary_hsl, -0.2, 0.16, -0.10)
        self._button_bar_hover_background_top_gradient_colour = LikeSecondary(secondary_hsl, 0.2, 0.16, 0.03)
        self._button_bar_hover_background_top_colour = LikeSecondary(secondary_hsl, 8.8, 0.16, 0.17)
        self._button_bar_active_border_pen = wx.Pen(LikeSecondary(secondary_hsl, -6.2, -0.47, -0.25))
        self._button_bar_active_background_top_colour = LikeSecondary(secondary_hsl, -8.4, 0.08, 0.06)
        self._button_bar_active_background_top_gradient_colour = LikeSecondary(secondary_hsl, -9.7, 0.13, -0.07)
        self._button_bar_active_background_colour = LikeSecondary(secondary_hsl, -9.9, 0.14, -0.14)
        self._button_bar_active_background_gradient_colour = LikeSecondary(secondary_hsl, -8.7, 0.17, -0.03)

        self._toolbar_border_pen = wx.Pen(LikePrimary(primary_hsl, 1.4, -0.21, -0.16))
        self.SetColour(RIBBON_ART_TOOLBAR_FACE_COLOUR, LikePrimary(primary_hsl, 1.4, -0.17, -0.22))
        self._tool_background_top_colour = LikePrimary(primary_hsl, -1.9, -0.07, 0.06)
        self._tool_background_top_gradient_colour = LikePrimary(primary_hsl, 1.4, 0.12, 0.08)
        self._tool_background_colour = LikePrimary(primary_hsl, 1.4, -0.09, 0.03)
        self._tool_background_gradient_colour = LikePrimary(primary_hsl, 1.9, 0.11, 0.09)
        self._tool_hover_background_top_colour = LikeSecondary(secondary_hsl, 3.4, 0.11, 0.16)
        self._tool_hover_background_top_gradient_colour = LikeSecondary(secondary_hsl, -1.4, 0.04, 0.08)
        self._tool_hover_background_colour = LikeSecondary(secondary_hsl, -1.8, 0.16, -0.12)
        self._tool_hover_background_gradient_colour = LikeSecondary(secondary_hsl, -2.6, 0.16, 0.05)
        self._tool_active_background_top_colour = LikeSecondary(secondary_hsl, -9.9, -0.12, -0.09)
        self._tool_active_background_top_gradient_colour = LikeSecondary(secondary_hsl, -8.5, 0.16, -0.12)
        self._tool_active_background_colour = LikeSecondary(secondary_hsl, -7.9, 0.16, -0.20)
        self._tool_active_background_gradient_colour = LikeSecondary(secondary_hsl, -6.6, 0.16, -0.10)

        # Invalidate cached tab separator
        self._cached_tab_separator_visibility = -1.0


    def Clone(self):
        """
        Create a new art provider which is a clone of this one.
        """

        copy = RibbonMSWArtProvider()
        self.CloneTo(copy)
        return copy


    def CloneTo(self, copy):

        for i in xrange(4):    
            copy._gallery_up_bitmap[i] = self._gallery_up_bitmap[i]
            copy._gallery_down_bitmap[i] = self._gallery_down_bitmap[i]
            copy._gallery_extension_bitmap[i] = self._gallery_extension_bitmap[i]
    
        copy._toolbar_drop_bitmap = self._toolbar_drop_bitmap

        copy._primary_scheme_colour = self._primary_scheme_colour
        copy._secondary_scheme_colour = self._secondary_scheme_colour
        copy._tertiary_scheme_colour = self._tertiary_scheme_colour

        copy._button_bar_label_colour = self._button_bar_label_colour
        copy._tab_label_colour = self._tab_label_colour
        copy._tab_separator_colour = self._tab_separator_colour
        copy._tab_separator_gradient_colour = self._tab_separator_gradient_colour
        copy._tab_active_background_colour = self._tab_hover_background_colour
        copy._tab_active_background_gradient_colour = self._tab_hover_background_gradient_colour
        copy._tab_hover_background_colour = self._tab_hover_background_colour
        copy._tab_hover_background_gradient_colour = self._tab_hover_background_gradient_colour
        copy._tab_hover_background_top_colour = self._tab_hover_background_top_colour
        copy._tab_hover_background_top_gradient_colour = self._tab_hover_background_top_gradient_colour
        copy._panel_label_colour = self._panel_label_colour
        copy._panel_hover_label_colour = self._panel_hover_label_colour
        copy._panel_minimised_label_colour = self._panel_minimised_label_colour
        copy._panel_active_background_colour = self._panel_active_background_colour
        copy._panel_active_background_gradient_colour = self._panel_active_background_gradient_colour
        copy._panel_active_background_top_colour = self._panel_active_background_top_colour
        copy._panel_active_background_top_gradient_colour = self._panel_active_background_top_gradient_colour
        copy._page_background_colour = self._page_background_colour
        copy._page_background_gradient_colour = self._page_background_gradient_colour
        copy._page_background_top_colour = self._page_background_top_colour
        copy._page_background_top_gradient_colour = self._page_background_top_gradient_colour
        copy._page_hover_background_colour = self._page_hover_background_colour
        copy._page_hover_background_gradient_colour = self._page_hover_background_gradient_colour
        copy._page_hover_background_top_colour = self._page_hover_background_top_colour
        copy._page_hover_background_top_gradient_colour = self._page_hover_background_top_gradient_colour
        copy._button_bar_hover_background_colour = self._button_bar_hover_background_colour
        copy._button_bar_hover_background_gradient_colour = self._button_bar_hover_background_gradient_colour
        copy._button_bar_hover_background_top_colour = self._button_bar_hover_background_top_colour
        copy._button_bar_hover_background_top_gradient_colour = self._button_bar_hover_background_top_gradient_colour
        copy._button_bar_active_background_colour = self._button_bar_active_background_colour
        copy._button_bar_active_background_gradient_colour = self._button_bar_active_background_gradient_colour
        copy._button_bar_active_background_top_colour = self._button_bar_active_background_top_colour
        copy._button_bar_active_background_top_gradient_colour = self._button_bar_active_background_top_gradient_colour
        copy._gallery_button_background_colour = self._gallery_button_background_colour
        copy._gallery_button_background_gradient_colour = self._gallery_button_background_gradient_colour    
        copy._gallery_button_hover_background_colour = self._gallery_button_hover_background_colour
        copy._gallery_button_hover_background_gradient_colour = self._gallery_button_hover_background_gradient_colour
        copy._gallery_button_active_background_colour = self._gallery_button_active_background_colour
        copy._gallery_button_active_background_gradient_colour = self._gallery_button_active_background_gradient_colour
        copy._gallery_button_disabled_background_colour = self._gallery_button_disabled_background_colour
        copy._gallery_button_disabled_background_gradient_colour = self._gallery_button_disabled_background_gradient_colour
        copy._gallery_button_face_colour = self._gallery_button_face_colour
        copy._gallery_button_hover_face_colour = self._gallery_button_hover_face_colour
        copy._gallery_button_active_face_colour = self._gallery_button_active_face_colour
        copy._gallery_button_disabled_face_colour = self._gallery_button_disabled_face_colour

        copy._tab_ctrl_background_brush = self._tab_ctrl_background_brush
        copy._panel_label_background_brush = self._panel_label_background_brush
        copy._panel_hover_label_background_brush = self._panel_hover_label_background_brush
        copy._gallery_hover_background_brush = self._gallery_hover_background_brush
        copy._gallery_button_background_top_brush = self._gallery_button_background_top_brush
        copy._gallery_button_hover_background_top_brush = self._gallery_button_hover_background_top_brush
        copy._gallery_button_active_background_top_brush = self._gallery_button_active_background_top_brush
        copy._gallery_button_disabled_background_top_brush = self._gallery_button_disabled_background_top_brush

        copy._tab_label_font = self._tab_label_font
        copy._button_bar_label_font = self._button_bar_label_font
        copy._panel_label_font = self._panel_label_font

        copy._page_border_pen = self._page_border_pen
        copy._panel_border_pen = self._panel_border_pen
        copy._panel_border_gradient_pen = self._panel_border_gradient_pen
        copy._panel_minimised_border_pen = self._panel_minimised_border_pen
        copy._panel_minimised_border_gradient_pen = self._panel_minimised_border_gradient_pen
        copy._tab_border_pen = self._tab_border_pen
        copy._gallery_border_pen = self._gallery_border_pen
        copy._button_bar_hover_border_pen = self._button_bar_hover_border_pen
        copy._button_bar_active_border_pen = self._button_bar_active_border_pen
        copy._gallery_item_border_pen = self._gallery_item_border_pen
        copy._toolbar_border_pen = self._toolbar_border_pen

        copy._flags = self._flags
        copy._tab_separation_size = self._tab_separation_size
        copy._page_border_left = self._page_border_left
        copy._page_border_top = self._page_border_top
        copy._page_border_right = self._page_border_right
        copy._page_border_bottom = self._page_border_bottom
        copy._panel_x_separation_size = self._panel_x_separation_size
        copy._panel_y_separation_size = self._panel_y_separation_size
        copy._gallery_bitmap_padding_left_size = self._gallery_bitmap_padding_left_size
        copy._gallery_bitmap_padding_right_size = self._gallery_bitmap_padding_right_size
        copy._gallery_bitmap_padding_top_size = self._gallery_bitmap_padding_top_size
        copy._gallery_bitmap_padding_bottom__size = self._gallery_bitmap_padding_bottom__size


    def GetFlags(self):
        """
        Get the previously set style flags.
        """

        return self._flags


    def SetFlags(self, flags):
        """
        Set the style flags.

        Normally called automatically by L{RibbonBar.SetArtProvider} with the ribbon
        bar's style flags, so that the art provider has the same flags as the bar which
        it is serving.

        :param `flags`: MISSING DESCRIPTION.

        """

        if (flags ^ self._flags) & RIBBON_BAR_FLOW_VERTICAL:        
            if flags & RIBBON_BAR_FLOW_VERTICAL:            
                self._page_border_left += 1
                self._page_border_right += 1
                self._page_border_top -= 1
                self._page_border_bottom -= 1
            else:            
                self._page_border_left -= 1
                self._page_border_right -= 1
                self._page_border_top += 1
                self._page_border_bottom += 1
            
        self._flags = flags

        # Need to reload some bitmaps when flags change
        self.Reload(RIBBON_ART_GALLERY_BUTTON_FACE_COLOUR)
        self.Reload(RIBBON_ART_GALLERY_BUTTON_HOVER_FACE_COLOUR)
        self.Reload(RIBBON_ART_GALLERY_BUTTON_ACTIVE_FACE_COLOUR)
        self.Reload(RIBBON_ART_GALLERY_BUTTON_DISABLED_FACE_COLOUR)


    def Reload(self, setting):

        self.SetColour(setting, self.GetColour(setting))


    def GetMetric(self, id):
        """
        Get the value of a certain integer setting.

        can be one of the size values of `RibbonArtSetting`.

        :param `id`: a metric id.

        """

        if id == RIBBON_ART_TAB_SEPARATION_SIZE:
            return self._tab_separation_size
        elif id == RIBBON_ART_PAGE_BORDER_LEFT_SIZE:
            return self._page_border_left
        elif id == RIBBON_ART_PAGE_BORDER_TOP_SIZE:
            return self._page_border_top
        elif id == RIBBON_ART_PAGE_BORDER_RIGHT_SIZE:
            return self._page_border_right
        elif id == RIBBON_ART_PAGE_BORDER_BOTTOM_SIZE:
            return self._page_border_bottom
        elif id == RIBBON_ART_PANEL_X_SEPARATION_SIZE:
            return self._panel_x_separation_size
        elif id == RIBBON_ART_PANEL_Y_SEPARATION_SIZE:
            return self._panel_y_separation_size
        elif id == RIBBON_ART_TOOL_GROUP_SEPARATION_SIZE:
            return self._tool_group_separation_size
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE:
            return self._gallery_bitmap_padding_left_size
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_RIGHT_SIZE:
            return self._gallery_bitmap_padding_right_size
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE:
            return self._gallery_bitmap_padding_top_size
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_BOTTOM_SIZE:
            return self._gallery_bitmap_padding_bottom__size
        else:
            raise Exception("Invalid Metric Ordinal")


    def SetMetric(self, id, new_val):
        """
        Set the value of a certain integer setting to the value.

        can be one of the size values of `RibbonArtSetting`.

        :param `id`: a metric id;
        :param `new_val`: the new value of the metric setting.

        """

        if id == RIBBON_ART_TAB_SEPARATION_SIZE:
            self._tab_separation_size = new_val
        elif id == RIBBON_ART_PAGE_BORDER_LEFT_SIZE:
            self._page_border_left = new_val
        elif id == RIBBON_ART_PAGE_BORDER_TOP_SIZE:
            self._page_border_top = new_val
        elif id == RIBBON_ART_PAGE_BORDER_RIGHT_SIZE:
            self._page_border_right = new_val
        elif id == RIBBON_ART_PAGE_BORDER_BOTTOM_SIZE:
            self._page_border_bottom = new_val
        elif id == RIBBON_ART_PANEL_X_SEPARATION_SIZE:
            self._panel_x_separation_size = new_val
        elif id == RIBBON_ART_PANEL_Y_SEPARATION_SIZE:
            self._panel_y_separation_size = new_val
        elif id == RIBBON_ART_TOOL_GROUP_SEPARATION_SIZE:
            self._tool_group_separation_size = new_val
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE:
            self._gallery_bitmap_padding_left_size = new_val
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_RIGHT_SIZE:
            self._gallery_bitmap_padding_right_size = new_val
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE:
            self._gallery_bitmap_padding_top_size = new_val
        elif id == RIBBON_ART_GALLERY_BITMAP_PADDING_BOTTOM_SIZE:
            self._gallery_bitmap_padding_bottom__size = new_val
        else:
            raise Exception("Invalid Metric Ordinal")
    

    def SetFont(self, id, font):
        """
        Set the value of a certain font setting to the value.

        can be one of the font values of `RibbonArtSetting`.

        :param `id`: a font id;
        :param `font`: the new font.

        """
        
        if id == RIBBON_ART_TAB_LABEL_FONT:
            self._tab_label_font = font
        elif id == RIBBON_ART_BUTTON_BAR_LABEL_FONT:
            self._button_bar_label_font = font
        elif id == RIBBON_ART_PANEL_LABEL_FONT:
            self._panel_label_font = font
        else:
            raise Exception("Invalid Font Ordinal")


    def GetFont(self, id):
        """
        Get the value of a certain font setting.

        can be one of the font values of `RibbonArtSetting`.

        :param `id`: the font id.

        """

        if id == RIBBON_ART_TAB_LABEL_FONT:
            return self._tab_label_font
        elif id == RIBBON_ART_BUTTON_BAR_LABEL_FONT:
            return self._button_bar_label_font
        elif id == RIBBON_ART_PANEL_LABEL_FONT:
            return self._panel_label_font
        else:
            raise Exception("Invalid Font Ordinal")


    def GetColour(self, id):
        """
        Get the value of a certain colour setting.

        can be one of the colour values of `RibbonArtSetting`.

        :param `id`: the colour id.

        """

        if id == RIBBON_ART_BUTTON_BAR_LABEL_COLOUR:
            return self._button_bar_label_colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BORDER_COLOUR:
            return self._button_bar_hover_border_pen.GetColour()
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_TOP_COLOUR:
            return self._button_bar_hover_background_top_colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_TOP_GRADIENT_COLOUR:
            return self._button_bar_hover_background_top_gradient_colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_COLOUR:
            return self._button_bar_hover_background_colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_GRADIENT_COLOUR:
            return self._button_bar_hover_background_gradient_colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BORDER_COLOUR:
            return self._button_bar_active_border_pen.GetColour()
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_TOP_COLOUR:
            return self._button_bar_active_background_top_colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR:
            return self._button_bar_active_background_top_gradient_colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_COLOUR:
            return self._button_bar_active_background_colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            return self._button_bar_active_background_gradient_colour
        elif id == RIBBON_ART_GALLERY_BORDER_COLOUR:
            return self._gallery_border_pen.GetColour()
        elif id == RIBBON_ART_GALLERY_HOVER_BACKGROUND_COLOUR:
            return self._gallery_hover_background_brush.GetColour()
        elif id == RIBBON_ART_GALLERY_BUTTON_BACKGROUND_COLOUR:
            return self._gallery_button_background_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_BACKGROUND_GRADIENT_COLOUR:
            return self._gallery_button_background_gradient_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_BACKGROUND_TOP_COLOUR:
            return self._gallery_button_background_top_brush.GetColour()
        elif id == RIBBON_ART_GALLERY_BUTTON_FACE_COLOUR:
            return self._gallery_button_face_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_COLOUR:
            return self._gallery_button_hover_background_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_GRADIENT_COLOUR:
            return self._gallery_button_hover_background_gradient_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_TOP_COLOUR:
            return self._gallery_button_hover_background_top_brush.GetColour()
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_FACE_COLOUR:
            return self._gallery_button_face_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_COLOUR:
            return self._gallery_button_active_background_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            return self._gallery_button_active_background_gradient_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_TOP_COLOUR:
            return self._gallery_button_background_top_brush.GetColour()
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_FACE_COLOUR:
            return self._gallery_button_active_face_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_COLOUR:
            return self._gallery_button_disabled_background_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_GRADIENT_COLOUR:
            return self._gallery_button_disabled_background_gradient_colour
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_TOP_COLOUR:
            return self._gallery_button_disabled_background_top_brush.GetColour()
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_FACE_COLOUR:
            return self._gallery_button_disabled_face_colour
        elif id == RIBBON_ART_GALLERY_ITEM_BORDER_COLOUR:
            return self._gallery_item_border_pen.GetColour()
        elif id in [RIBBON_ART_TAB_CTRL_BACKGROUND_COLOUR, RIBBON_ART_TAB_CTRL_BACKGROUND_GRADIENT_COLOUR]:
            return self._tab_ctrl_background_brush.GetColour()
        elif id == RIBBON_ART_TAB_LABEL_COLOUR:
            return self._tab_label_colour
        elif id == RIBBON_ART_TAB_SEPARATOR_COLOUR:
            return self._tab_separator_colour
        elif id == RIBBON_ART_TAB_SEPARATOR_GRADIENT_COLOUR:
            return self._tab_separator_gradient_colour
        elif id in [RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_COLOUR, RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR]:
            return wx.Colour(0, 0, 0)
        elif id == RIBBON_ART_TAB_ACTIVE_BACKGROUND_COLOUR:
            return self._tab_active_background_colour
        elif id == RIBBON_ART_TAB_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            return self._tab_active_background_gradient_colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_TOP_COLOUR:
            return self._tab_hover_background_top_colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_TOP_GRADIENT_COLOUR:
            return self._tab_hover_background_top_gradient_colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_COLOUR:
            return self._tab_hover_background_colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_GRADIENT_COLOUR:
            return self._tab_hover_background_gradient_colour
        elif id == RIBBON_ART_TAB_BORDER_COLOUR:
            return self._tab_border_pen.GetColour()
        elif id == RIBBON_ART_PANEL_BORDER_COLOUR:
            return self._panel_border_pen.GetColour()
        elif id == RIBBON_ART_PANEL_BORDER_GRADIENT_COLOUR:
            return self._panel_border_gradient_pen.GetColour()
        elif id == RIBBON_ART_PANEL_MINIMISED_BORDER_COLOUR:
            return self._panel_minimised_border_pen.GetColour()
        elif id == RIBBON_ART_PANEL_MINIMISED_BORDER_GRADIENT_COLOUR:
            return self._panel_minimised_border_gradient_pen.GetColour()
        elif id in [RIBBON_ART_PANEL_LABEL_BACKGROUND_COLOUR, RIBBON_ART_PANEL_LABEL_BACKGROUND_GRADIENT_COLOUR]:
            return self._panel_label_background_brush.GetColour()
        elif id == RIBBON_ART_PANEL_LABEL_COLOUR:
            return self._panel_label_colour
        elif id == RIBBON_ART_PANEL_MINIMISED_LABEL_COLOUR:
            return self._panel_minimised_label_colour
        elif id in [RIBBON_ART_PANEL_HOVER_LABEL_BACKGROUND_COLOUR, RIBBON_ART_PANEL_HOVER_LABEL_BACKGROUND_GRADIENT_COLOUR]:
            return self._panel_hover_label_background_brush.GetColour()
        elif id == RIBBON_ART_PANEL_HOVER_LABEL_COLOUR:
            return self._panel_hover_label_colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_TOP_COLOUR:
            return self._panel_active_background_top_colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR:
            return self._panel_active_background_top_gradient_colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_COLOUR:
            return self._panel_active_background_colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            return self._panel_active_background_gradient_colour
        elif id == RIBBON_ART_PAGE_BORDER_COLOUR:
            return self._page_border_pen.GetColour()
        elif id == RIBBON_ART_PAGE_BACKGROUND_TOP_COLOUR:
            return self._page_background_top_colour
        elif id == RIBBON_ART_PAGE_BACKGROUND_TOP_GRADIENT_COLOUR:
            return self._page_background_top_gradient_colour
        elif id == RIBBON_ART_PAGE_BACKGROUND_COLOUR:
            return self._page_background_colour
        elif id == RIBBON_ART_PAGE_BACKGROUND_GRADIENT_COLOUR:
            return self._page_background_gradient_colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_TOP_COLOUR:
            return self._page_hover_background_top_colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_TOP_GRADIENT_COLOUR:
            return self._page_hover_background_top_gradient_colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_COLOUR:
            return self._page_hover_background_colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_GRADIENT_COLOUR:
            return self._page_hover_background_gradient_colour
        elif id in [RIBBON_ART_TOOLBAR_BORDER_COLOUR, RIBBON_ART_TOOLBAR_HOVER_BORDER_COLOUR]:
            return self._toolbar_border_pen.GetColour()
        elif id == RIBBON_ART_TOOLBAR_FACE_COLOUR:
            return self._tool_face_colour
        else:
            raise Exception("Invalid Colour Ordinal")


    def SetColour(self, id, colour):
        """
        Set the value of a certain colour setting to the value.

        can be one of the colour values of `RibbonArtSetting`, though not all colour
        settings will have an affect on every art provider.

        :param `id`: the colour id;
        :param `colour`: the colour.

        :see: L{SetColourScheme}
        """
    
        if id == RIBBON_ART_BUTTON_BAR_LABEL_COLOUR:
            self._button_bar_label_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BORDER_COLOUR:
            self._button_bar_hover_border_pen.SetColour(colour)
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_TOP_COLOUR:
            self._button_bar_hover_background_top_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_TOP_GRADIENT_COLOUR:
            self._button_bar_hover_background_top_gradient_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_COLOUR:
            self._button_bar_hover_background_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_GRADIENT_COLOUR:
            self._button_bar_hover_background_gradient_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BORDER_COLOUR:
            self._button_bar_active_border_pen.SetColour(colour)
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_TOP_COLOUR:
            self._button_bar_active_background_top_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR:
            self._button_bar_active_background_top_gradient_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_COLOUR:
            self._button_bar_active_background_colour = colour
        elif id == RIBBON_ART_BUTTON_BAR_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            self._button_bar_active_background_gradient_colour = colour
        elif id == RIBBON_ART_GALLERY_BORDER_COLOUR:
            self._gallery_border_pen.SetColour(colour)
        elif id == RIBBON_ART_GALLERY_HOVER_BACKGROUND_COLOUR:
            self._gallery_hover_background_brush.SetColour(colour)
        elif id == RIBBON_ART_GALLERY_BUTTON_BACKGROUND_COLOUR:
            self._gallery_button_background_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_BACKGROUND_GRADIENT_COLOUR:
            self._gallery_button_background_gradient_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_BACKGROUND_TOP_COLOUR:
            self._gallery_button_background_top_brush.SetColour(colour)
        elif id == RIBBON_ART_GALLERY_BUTTON_FACE_COLOUR:
            self._gallery_button_face_colour = colour
            
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:            
                self._gallery_up_bitmap[0] = RibbonLoadPixmap(gallery_left_xpm, colour)
                self._gallery_down_bitmap[0] = RibbonLoadPixmap(gallery_right_xpm, colour)
            else:            
                self._gallery_up_bitmap[0] = RibbonLoadPixmap(gallery_up_xpm, colour)
                self._gallery_down_bitmap[0] = RibbonLoadPixmap(gallery_down_xpm, colour)
            
            self._gallery_extension_bitmap[0] = RibbonLoadPixmap(gallery_extension_xpm, colour)
            
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_COLOUR:
            self._gallery_button_hover_background_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_GRADIENT_COLOUR:
            self._gallery_button_hover_background_gradient_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_TOP_COLOUR:
            self._gallery_button_hover_background_top_brush.SetColour(colour)
        elif id == RIBBON_ART_GALLERY_BUTTON_HOVER_FACE_COLOUR:
            self._gallery_button_hover_face_colour = colour
            
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:
                self._gallery_up_bitmap[1] = RibbonLoadPixmap(gallery_left_xpm, colour)
                self._gallery_down_bitmap[1] = RibbonLoadPixmap(gallery_right_xpm, colour)
            else:            
                self._gallery_up_bitmap[1] = RibbonLoadPixmap(gallery_up_xpm, colour)
                self._gallery_down_bitmap[1] = RibbonLoadPixmap(gallery_down_xpm, colour)
            
            self._gallery_extension_bitmap[1] = RibbonLoadPixmap(gallery_extension_xpm, colour)
            
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_COLOUR:
            self._gallery_button_active_background_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            self._gallery_button_active_background_gradient_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_TOP_COLOUR:
            self._gallery_button_background_top_brush.SetColour(colour)
        elif id == RIBBON_ART_GALLERY_BUTTON_ACTIVE_FACE_COLOUR:
            self._gallery_button_active_face_colour = colour

            if self._flags & RIBBON_BAR_FLOW_VERTICAL:            
                self._gallery_up_bitmap[2] = RibbonLoadPixmap(gallery_left_xpm, colour)
                self._gallery_down_bitmap[2] = RibbonLoadPixmap(gallery_right_xpm, colour)            
            else:            
                self._gallery_up_bitmap[2] = RibbonLoadPixmap(gallery_up_xpm, colour)
                self._gallery_down_bitmap[2] = RibbonLoadPixmap(gallery_down_xpm, colour)
            
            self._gallery_extension_bitmap[2] = RibbonLoadPixmap(gallery_extension_xpm, colour)

        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_COLOUR:
            self._gallery_button_disabled_background_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_GRADIENT_COLOUR:
            self._gallery_button_disabled_background_gradient_colour = colour
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_TOP_COLOUR:
            self._gallery_button_disabled_background_top_brush.SetColour(colour)
        elif id == RIBBON_ART_GALLERY_BUTTON_DISABLED_FACE_COLOUR:
            self._gallery_button_disabled_face_colour = colour
            
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:
                self._gallery_up_bitmap[3] = RibbonLoadPixmap(gallery_left_xpm, colour)
                self._gallery_down_bitmap[3] = RibbonLoadPixmap(gallery_right_xpm, colour)            
            else:            
                self._gallery_up_bitmap[3] = RibbonLoadPixmap(gallery_up_xpm, colour)
                self._gallery_down_bitmap[3] = RibbonLoadPixmap(gallery_down_xpm, colour)
            
            self._gallery_extension_bitmap[3] = RibbonLoadPixmap(gallery_extension_xpm, colour)

        elif id == RIBBON_ART_GALLERY_ITEM_BORDER_COLOUR:
            self._gallery_item_border_pen.SetColour(colour)

        elif id in [RIBBON_ART_TAB_CTRL_BACKGROUND_COLOUR, RIBBON_ART_TAB_CTRL_BACKGROUND_GRADIENT_COLOUR]:
            self._tab_ctrl_background_brush.SetColour(colour)
            self._cached_tab_separator_visibility = -1.0
        elif id == RIBBON_ART_TAB_LABEL_COLOUR:
            self._tab_label_colour = colour
        elif id == RIBBON_ART_TAB_SEPARATOR_COLOUR:
            self._tab_separator_colour = colour
            self._cached_tab_separator_visibility = -1.0
        elif id == RIBBON_ART_TAB_SEPARATOR_GRADIENT_COLOUR:
            self._tab_separator_gradient_colour = colour
            self._cached_tab_separator_visibility = -1.0
        elif id in [RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_COLOUR, RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR]:
            pass
        elif id == RIBBON_ART_TAB_ACTIVE_BACKGROUND_COLOUR:
            self._tab_active_background_colour = colour
        elif id == RIBBON_ART_TAB_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            self._tab_active_background_gradient_colour = colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_TOP_COLOUR:
            self._tab_hover_background_top_colour = colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_TOP_GRADIENT_COLOUR:
            self._tab_hover_background_top_gradient_colour = colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_COLOUR:
            self._tab_hover_background_colour = colour
        elif id == RIBBON_ART_TAB_HOVER_BACKGROUND_GRADIENT_COLOUR:
            self._tab_hover_background_gradient_colour = colour
        elif id == RIBBON_ART_TAB_BORDER_COLOUR:
            self._tab_border_pen.SetColour(colour)
        elif id == RIBBON_ART_PANEL_BORDER_COLOUR:
            self._panel_border_pen.SetColour(colour)
        elif id == RIBBON_ART_PANEL_BORDER_GRADIENT_COLOUR:
            self._panel_border_gradient_pen.SetColour(colour)
        elif id == RIBBON_ART_PANEL_MINIMISED_BORDER_COLOUR:
            self._panel_minimised_border_pen.SetColour(colour)
        elif id == RIBBON_ART_PANEL_MINIMISED_BORDER_GRADIENT_COLOUR:
            self._panel_minimised_border_gradient_pen.SetColour(colour)
        elif id in [RIBBON_ART_PANEL_LABEL_BACKGROUND_COLOUR, RIBBON_ART_PANEL_LABEL_BACKGROUND_GRADIENT_COLOUR]:
            self._panel_label_background_brush.SetColour(colour)
        elif id == RIBBON_ART_PANEL_LABEL_COLOUR:
            self._panel_label_colour = colour
        elif id in [RIBBON_ART_PANEL_HOVER_LABEL_BACKGROUND_COLOUR, RIBBON_ART_PANEL_HOVER_LABEL_BACKGROUND_GRADIENT_COLOUR]:
            self._panel_hover_label_background_brush.SetColour(colour)
        elif id == RIBBON_ART_PANEL_HOVER_LABEL_COLOUR:
            self._panel_hover_label_colour = colour
        elif id == RIBBON_ART_PANEL_MINIMISED_LABEL_COLOUR:
            self._panel_minimised_label_colour = colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_TOP_COLOUR:
            self._panel_active_background_top_colour = colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR:
            self._panel_active_background_top_gradient_colour = colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_COLOUR:
            self._panel_active_background_colour = colour
        elif id == RIBBON_ART_PANEL_ACTIVE_BACKGROUND_GRADIENT_COLOUR:
            self._panel_active_background_gradient_colour = colour
        elif id == RIBBON_ART_PAGE_BORDER_COLOUR:
            self._page_border_pen.SetColour(colour)
        elif id == RIBBON_ART_PAGE_BACKGROUND_TOP_COLOUR:
            self._page_background_top_colour = colour
        elif id == RIBBON_ART_PAGE_BACKGROUND_TOP_GRADIENT_COLOUR:
            self._page_background_top_gradient_colour = colour
        elif id == RIBBON_ART_PAGE_BACKGROUND_COLOUR:
            self._page_background_colour = colour
        elif id == RIBBON_ART_PAGE_BACKGROUND_GRADIENT_COLOUR:
            self._page_background_gradient_colour = colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_TOP_COLOUR:
            self._page_hover_background_top_colour = colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_TOP_GRADIENT_COLOUR:
            self._page_hover_background_top_gradient_colour = colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_COLOUR:
            self._page_hover_background_colour = colour
        elif id == RIBBON_ART_PAGE_HOVER_BACKGROUND_GRADIENT_COLOUR:
            self._page_hover_background_gradient_colour = colour
        elif id in [RIBBON_ART_TOOLBAR_BORDER_COLOUR, RIBBON_ART_TOOLBAR_HOVER_BORDER_COLOUR]:
            self._toolbar_border_pen.SetColour(colour)
        elif id == RIBBON_ART_TOOLBAR_FACE_COLOUR:
            self._tool_face_colour = colour
            self._toolbar_drop_bitmap = RibbonLoadPixmap(gallery_down_xpm, colour)
        else:
            raise Exception("Invalid Colour Ordinal")
    

    def DrawTabCtrlBackground(self, dc, wnd, rect):
        """
        Draw the background of the tab region of a ribbon bar.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw.

        """

        dc.SetPen(wx.TRANSPARENT_PEN)

        dc.SetBrush(self._tab_ctrl_background_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        dc.SetPen(self._page_border_pen)
        
        if rect.width > 6:        
            dc.DrawLine(rect.x + 3, rect.y + rect.height - 1, rect.x + rect.width - 3, rect.y + rect.height - 1)        
        else:        
            dc.DrawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width, rect.y + rect.height - 1)
    

    def DrawTab(self, dc, wnd, tab):
        """
        Draw a single tab in the tab region of a ribbon bar.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto (not the L{RibbonPage} associated
         with the tab being drawn);
        :param `tab`: The rectangle within which to draw, and also the tab label, icon, and
         state (active and/or hovered). The drawing rectangle will be entirely within a
         rectangle on the same device context previously painted with L{DrawTabCtrlBackground}.
         The rectangle's width will be at least the minimum value returned by L{GetBarTabWidth},
         and height will be the value returned by L{GetTabCtrlHeight}.

        """

        if tab.rect.height <= 2:
            return

        if tab.active or tab.hovered:        
            if tab.active:            
                background = wx.Rect(*tab.rect)
                background.x += 2
                background.y += 2
                background.width -= 4
                background.height -= 2

                dc.GradientFillLinear(background, self._tab_active_background_colour,
                                      self._tab_active_background_gradient_colour, wx.SOUTH)

                # TODO: active and hovered
            
            elif tab.hovered:            
                background = wx.Rect(*tab.rect)
                background.x += 2
                background.y += 2
                background.width -= 4
                background.height -= 3
                h = background.height
                background.height /= 2
                dc.GradientFillLinear(background, self._tab_hover_background_top_colour,
                                      self._tab_hover_background_top_gradient_colour, wx.SOUTH)

                background.y += background.height
                background.height = h - background.height
                dc.GradientFillLinear(background, self._tab_hover_background_colour,
                                      self._tab_hover_background_gradient_colour, wx.SOUTH)
           
            border_points = [wx.Point() for i in xrange(6)]
            border_points[0] = wx.Point(1, tab.rect.height - 2)
            border_points[1] = wx.Point(1, 3)
            border_points[2] = wx.Point(3, 1)
            border_points[3] = wx.Point(tab.rect.width - 4, 1)
            border_points[4] = wx.Point(tab.rect.width - 2, 3)
            border_points[5] = wx.Point(tab.rect.width - 2, tab.rect.height - 1)

            dc.SetPen(self._tab_border_pen)
            dc.DrawLines(border_points, tab.rect.x, tab.rect.y)

            if tab.active:            
                # Give the tab a curved outward border at the bottom
                dc.DrawPoint(tab.rect.x, tab.rect.y + tab.rect.height - 2)
                dc.DrawPoint(tab.rect.x + tab.rect.width - 1, tab.rect.y + tab.rect.height - 2)

                p = wx.Pen(self._tab_active_background_gradient_colour)
                dc.SetPen(p)

                # Technically the first two points are the wrong colour, but they're near enough
                dc.DrawPoint(tab.rect.x + 1, tab.rect.y + tab.rect.height - 2)
                dc.DrawPoint(tab.rect.x + tab.rect.width - 2, tab.rect.y + tab.rect.height - 2)
                dc.DrawPoint(tab.rect.x + 1, tab.rect.y + tab.rect.height - 1)
                dc.DrawPoint(tab.rect.x, tab.rect.y + tab.rect.height - 1)
                dc.DrawPoint(tab.rect.x + tab.rect.width - 2, tab.rect.y + tab.rect.height - 1)
                dc.DrawPoint(tab.rect.x + tab.rect.width - 1, tab.rect.y + tab.rect.height - 1)
            
        if self._flags & RIBBON_BAR_SHOW_PAGE_ICONS:
            icon = tab.page.GetIcon()
            x = tab.rect.x + 4
            if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS == 0:
                x = tab.rect.x + (tab.rect.width - icon.GetWidth()) / 2
                
            dc.DrawBitmap(icon, x, tab.rect.y + 1 + (tab.rect.height - 1 - icon.GetHeight()) / 2, True)
        
        if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS:
            label = tab.page.GetLabel()
            if label.strip():            
                dc.SetFont(self._tab_label_font)
                dc.SetTextForeground(self._tab_label_colour)
                dc.SetBackgroundMode(wx.TRANSPARENT)

                text_width, text_height = dc.GetTextExtent(label)
                width = tab.rect.width - 5
                x = tab.rect.x + 3
                
                if self._flags & RIBBON_BAR_SHOW_PAGE_ICONS:                
                    x += 3 + tab.page.GetIcon().GetWidth()
                    width -= 3 + tab.page.GetIcon().GetWidth()
                
                y = tab.rect.y + (tab.rect.height - text_height) / 2

                if width <= text_width:                
                    dc.SetClippingRegion(x, tab.rect.y, width, tab.rect.height)
                    dc.DrawText(label, x, y)
                else:                
                    dc.DrawText(label, x + (width - text_width) / 2 + 1, y)

                
    def DrawTabSeparator(self, dc, wnd, rect, visibility):
        """
        Draw a separator between two tabs in a ribbon bar.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw, which will be entirely
         within a rectangle on the same device context previously painted with
         L{DrawTabCtrlBackground};
        :param `visibility`: The opacity with which to draw the separator. Values
         are in the range [0, 1], with 0 being totally transparent, and 1 being totally
         opaque.

        """

        if visibility <= 0.0:        
            return
        
        if visibility > 1.0:        
            visibility = 1.0
        
        # The tab separator is relatively expensive to draw (for its size), and is
        # usually drawn multiple times sequentially (in different positions), so it
        # makes sense to draw it once and cache it.
        if not self._cached_tab_separator.IsOk() or self._cached_tab_separator.GetSize() != rect.GetSize() or \
           visibility != self._cached_tab_separator_visibility:
        
            size = wx.Rect(0, 0, *rect.GetSize())
            self.ReallyDrawTabSeparator(wnd, size, visibility)
        
        dc.DrawBitmap(self._cached_tab_separator, rect.x, rect.y, False)


    def ReallyDrawTabSeparator(self, wnd, rect, visibility):

        if not self._cached_tab_separator.IsOk() or self._cached_tab_separator.GetSize() != rect.GetSize():
            self._cached_tab_separator = wx.EmptyBitmap(*rect.GetSize())
    
        dc = wx.MemoryDC(self._cached_tab_separator)
        self.DrawTabCtrlBackground(dc, wnd, rect)

        x = rect.x + rect.width / 2
        h = float(rect.height - 1)

        r1 = self._tab_ctrl_background_brush.GetColour().Red() * (1.0 - visibility) + 0.5
        g1 = self._tab_ctrl_background_brush.GetColour().Green() * (1.0 - visibility) + 0.5
        b1 = self._tab_ctrl_background_brush.GetColour().Blue() * (1.0 - visibility) + 0.5
        r2 = self._tab_separator_colour.Red()
        g2 = self._tab_separator_colour.Green()
        b2 = self._tab_separator_colour.Blue()
        r3 = self._tab_separator_gradient_colour.Red()
        g3 = self._tab_separator_gradient_colour.Green()
        b3 = self._tab_separator_gradient_colour.Blue()

        for i in xrange(rect.height-1):
        
            p = float(i)/h

            r = (p * r3 + (1.0 - p) * r2) * visibility + r1
            g = (p * g3 + (1.0 - p) * g2) * visibility + g1
            b = (p * b3 + (1.0 - p) * b2) * visibility + b1

            P = wx.Pen(wx.Colour(r, g, b))
            dc.SetPen(P)
            dc.DrawPoint(x, rect.y + i)
        
        self._cached_tab_separator_visibility = visibility


    def DrawPartialPageBackground(self, dc, wnd, rect, allow_hovered_or_page=True, offset=None, hovered=False):

        if isinstance(allow_hovered_or_page, types.BooleanType):
            self.DrawPartialPageBackground2(dc, wnd, rect, allow_hovered_or_page)
        else:
            self.DrawPartialPageBackground1(dc, wnd, rect, allow_hovered_or_page, offset, hovered)
            

    def DrawPartialPageBackground1(self, dc, wnd, rect, page, offset, hovered=False):

        background = wx.Rect(0, 0, *page.GetSize())
        background = page.AdjustRectToIncludeScrollButtons(background)
        background.height -= 2
        
        # Page background isn't dependant upon the width of the page
        # (at least not the part of it intended to be painted by this
        # function). Set to wider than the page itself for when externally
        # expanded panels need a background - the expanded panel can be wider
        # than the bar.

        background.x = 0
        background.width = 10000

        # upper_rect, lower_rect, paint_rect are all in page co-ordinates
        upper_rect = wx.Rect(*background)
        upper_rect.height /= 5

        lower_rect = wx.Rect(*background)
        lower_rect.y += upper_rect.height
        lower_rect.height -= upper_rect.height

        paint_rect = wx.Rect(*rect)
        paint_rect.x += offset.x
        paint_rect.y += offset.y

        if hovered:        
            bg_top = self._page_hover_background_top_colour
            bg_top_grad = self._page_hover_background_top_gradient_colour
            bg_btm = self._page_hover_background_colour
            bg_btm_grad = self._page_hover_background_gradient_colour        
        else:        
            bg_top = self._page_background_top_colour
            bg_top_grad = self._page_background_top_gradient_colour
            bg_btm = self._page_background_colour
            bg_btm_grad = self._page_background_gradient_colour
        
        if paint_rect.Intersects(upper_rect):        
            rect = wx.Rect(*upper_rect)
            rect.Intersect(paint_rect)
            rect.x -= offset.x
            rect.y -= offset.y
            starting_colour = RibbonInterpolateColour(bg_top, bg_top_grad,
                                                      paint_rect.y, upper_rect.y,
                                                      upper_rect.y + upper_rect.height)
            ending_colour = RibbonInterpolateColour(bg_top, bg_top_grad,
                                                    paint_rect.y + paint_rect.height, upper_rect.y,
                                                    upper_rect.y + upper_rect.height)
            dc.GradientFillLinear(rect, starting_colour, ending_colour, wx.SOUTH)
        

        if paint_rect.Intersects(lower_rect):        
            rect = wx.Rect(*lower_rect)
            rect.Intersect(paint_rect)
            rect.x -= offset.x
            rect.y -= offset.y
            starting_colour = RibbonInterpolateColour(bg_btm, bg_btm_grad,
                                                      paint_rect.y, lower_rect.y,
                                                      lower_rect.y + lower_rect.height)
            ending_colour = RibbonInterpolateColour(bg_btm, bg_btm_grad,
                                                    paint_rect.y + paint_rect.height,
                                                    lower_rect.y, lower_rect.y + lower_rect.height)
            
            dc.GradientFillLinear(rect, starting_colour, ending_colour, wx.SOUTH)

        
    def DrawPageBackground(self, dc, wnd, rect):
        """
        Draw the background of a ribbon page.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto (which is commonly the
         L{RibbonPage} whose background is being drawn, but doesn't have to be);
        :param `rect`: The rectangle within which to draw.

        :see: L{GetPageBackgroundRedrawArea}
        """

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._tab_ctrl_background_brush)
        
        edge = wx.Rect(*rect)

        edge.width = 2
        dc.DrawRectangle(edge.x, edge.y, edge.width, edge.height)

        edge.x += rect.width - 2
        dc.DrawRectangle(edge.x, edge.y, edge.width, edge.height)

        edge = wx.Rect(*rect)
        edge.height = 2
        edge.y += (rect.height - edge.height)
        dc.DrawRectangle(edge.x, edge.y, edge.width, edge.height)
    
        background = wx.Rect(*rect)
        background.x += 2
        background.width -= 4
        background.height -= 2

        background.height /= 5
        dc.GradientFillLinear(background, self._page_background_top_colour,
                              self._page_background_top_gradient_colour, wx.SOUTH)

        background.y += background.height
        background.height = rect.height - 2 - background.height
        dc.GradientFillLinear(background, self._page_background_colour,
                              self._page_background_gradient_colour, wx.SOUTH)
    
        border_points = [wx.Point() for i in xrange(8)]
        border_points[0] = wx.Point(2, 0)
        border_points[1] = wx.Point(1, 1)
        border_points[2] = wx.Point(1, rect.height - 4)
        border_points[3] = wx.Point(3, rect.height - 2)
        border_points[4] = wx.Point(rect.width - 4, rect.height - 2)
        border_points[5] = wx.Point(rect.width - 2, rect.height - 4)
        border_points[6] = wx.Point(rect.width - 2, 1)
        border_points[7] = wx.Point(rect.width - 4, -1)

        dc.SetPen(self._page_border_pen)
        dc.DrawLines(border_points, rect.x, rect.y)
        

    def DrawScrollButton(self, dc, wnd, rect_, style):
        """
        Draw a ribbon-style scroll button.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will be at least the size returned by L{GetScrollButtonMinimumSize} for a
         scroll button with the same style. For tab scroll buttons, this rectangle
         will be entirely within a rectangle on the same device context previously
         painted with L{DrawTabCtrlBackground}, but this is not guaranteed for other
         types of button (for example, page scroll buttons will not be painted on an
         area previously painted with L{DrawPageBackground} );
        :param `style`: A combination of flags from `RibbonScrollButtonStyle`,
         including a direction, a for flag, and one or more states.

        """

        rect = wx.Rect(*rect_)

        if (style & RIBBON_SCROLL_BTN_FOR_MASK) == RIBBON_SCROLL_BTN_FOR_PAGE:

            # Page scroll buttons do not have the luxury of rendering on top of anything
            # else, and their size includes some padding, hence the background painting
            # and size adjustment.
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(self._tab_ctrl_background_brush)
            dc.DrawRectangleRect(rect)
            dc.SetClippingRect(rect)
            
            result = style & RIBBON_SCROLL_BTN_DIRECTION_MASK
            
            if result == RIBBON_SCROLL_BTN_LEFT:
                rect.x += 1
            elif result == RIBBON_SCROLL_BTN_RIGHT:
                rect.y -= 1
                rect.width -= 1
            elif result == RIBBON_SCROLL_BTN_UP:
                rect.x += 1
                rect.y -= 1
                rect.width -= 2
                rect.height += 1
            elif result == RIBBON_SCROLL_BTN_DOWN:
                rect.x += 1
                rect.width -= 2
                rect.height -= 1            
        
        background = wx.Rect(*rect)
        background.x += 1
        background.y += 1
        background.width -= 2
        background.height -= 2

        if style & RIBBON_SCROLL_BTN_UP:
            background.height /= 2
        else:
            background.height /= 5
            
        dc.GradientFillLinear(background, self._page_background_top_colour,
                              self._page_background_top_gradient_colour, wx.SOUTH)

        background.y += background.height
        background.height = rect.height - 2 - background.height
        dc.GradientFillLinear(background, self._page_background_colour,
                              self._page_background_gradient_colour, wx.SOUTH)
    
        border_points = [wx.Point() for i in xrange(7)]
        result = style & RIBBON_SCROLL_BTN_DIRECTION_MASK
        
        if result == RIBBON_SCROLL_BTN_LEFT:
            border_points[0] = wx.Point(2, 0)
            border_points[1] = wx.Point(rect.width - 1, 0)
            border_points[2] = wx.Point(rect.width - 1, rect.height - 1)
            border_points[3] = wx.Point(2, rect.height - 1)
            border_points[4] = wx.Point(0, rect.height - 3)
            border_points[5] = wx.Point(0, 2)

        elif result == RIBBON_SCROLL_BTN_RIGHT:
            border_points[0] = wx.Point(0, 0)
            border_points[1] = wx.Point(rect.width - 3, 0)
            border_points[2] = wx.Point(rect.width - 1, 2)
            border_points[3] = wx.Point(rect.width - 1, rect.height - 3)
            border_points[4] = wx.Point(rect.width - 3, rect.height - 1)
            border_points[5] = wx.Point(0, rect.height - 1)

        elif result == RIBBON_SCROLL_BTN_UP:
            border_points[0] = wx.Point(2, 0)
            border_points[1] = wx.Point(rect.width - 3, 0)
            border_points[2] = wx.Point(rect.width - 1, 2)
            border_points[3] = wx.Point(rect.width - 1, rect.height - 1)
            border_points[4] = wx.Point(0, rect.height - 1)
            border_points[5] = wx.Point(0, 2)

        elif result == RIBBON_SCROLL_BTN_DOWN:
            border_points[0] = wx.Point(0, 0)
            border_points[1] = wx.Point(rect.width - 1, 0)
            border_points[2] = wx.Point(rect.width - 1, rect.height - 3)
            border_points[3] = wx.Point(rect.width - 3, rect.height - 1)
            border_points[4] = wx.Point(2, rect.height - 1)
            border_points[5] = wx.Point(0, rect.height - 3)
        
        border_points[6] = border_points[0]

        dc.SetPen(self._page_border_pen)
        dc.DrawLines(border_points, rect.x, rect.y)
    
        # NB: Code for handling hovered/active state is temporary
        arrow_points = [wx.Point() for i in xrange(3)]
        result = style & RIBBON_SCROLL_BTN_DIRECTION_MASK
        
        if result == RIBBON_SCROLL_BTN_LEFT:
            arrow_points[0] = wx.Point(rect.width / 2 - 2, rect.height / 2)
            if style & RIBBON_SCROLL_BTN_ACTIVE:
                arrow_points[0].y += 1
            arrow_points[1] = arrow_points[0] + wx.Point(3, -3)
            arrow_points[2] = arrow_points[0] + wx.Point(3,  3)

        elif result == RIBBON_SCROLL_BTN_RIGHT:
            arrow_points[0] = wx.Point(rect.width / 2 + 2, rect.height / 2)
            if style & RIBBON_SCROLL_BTN_ACTIVE:
                arrow_points[0].y += 1
            arrow_points[1] = arrow_points[0] - wx.Point(3,  3)
            arrow_points[2] = arrow_points[0] - wx.Point(3, -3)

        elif result == RIBBON_SCROLL_BTN_UP:
            arrow_points[0] = wx.Point(rect.width / 2, rect.height / 2 - 2)
            if style & RIBBON_SCROLL_BTN_ACTIVE:
                arrow_points[0].y += 1
            arrow_points[1] = arrow_points[0] + wx.Point( 3, 3)
            arrow_points[2] = arrow_points[0] + wx.Point(-3, 3)

        elif result == RIBBON_SCROLL_BTN_DOWN:
            arrow_points[0] = wx.Point(rect.width / 2, rect.height / 2 + 2)
            if style & RIBBON_SCROLL_BTN_ACTIVE:
                arrow_points[0].y += 1
            arrow_points[1] = arrow_points[0] - wx.Point( 3, 3)
            arrow_points[2] = arrow_points[0] - wx.Point(-3, 3)
        
        dc.SetPen(wx.TRANSPARENT_PEN)
        B = wx.Brush((style & RIBBON_SCROLL_BTN_HOVERED and [self._tab_active_background_colour] or [self._tab_label_colour])[0])
        dc.SetBrush(B)
        dc.DrawPolygon(arrow_points, rect.x, rect.y)
    

    def DrawDropdownArrow(self, dc, x, y, colour):

        arrow_points = [wx.Point() for i in xrange(3)]
        brush = wx.Brush(colour)
        arrow_points[0] = wx.Point(1, 2)
        arrow_points[1] = arrow_points[0] + wx.Point(-3, -3)
        arrow_points[2] = arrow_points[0] + wx.Point( 3, -3)
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(brush)
        dc.DrawPolygon(arrow_points, x, y)


    def RemovePanelPadding(self, rect):

        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            rect.y += 1
            rect.height -= 2        
        else:        
            rect.x += 1
            rect.width -= 2
        
        return rect


    def DrawPanelBackground(self, dc, wnd, rect):
        """
        Draw the background and chrome for a ribbon panel.

        This should draw the border, background, label, and any other items of a panel
        which are outside the client area of a panel. Note that when a panel is
        minimised, this function is not called - only L{DrawMinimisedPanel} is called,
        so a background should be explicitly painted by that if required.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto, which is always the panel
         whose background and chrome is being drawn. The panel label and other panel
         attributes can be obtained by querying this;
        :param `rect`: The rectangle within which to draw.

        """

        self.DrawPartialPageBackground(dc, wnd, rect, False)

        true_rect = wx.Rect(*rect)
        true_rect = self.RemovePanelPadding(true_rect)

        dc.SetFont(self._panel_label_font)
        dc.SetPen(wx.TRANSPARENT_PEN)

        if wnd.IsHovered():
            dc.SetBrush(self._panel_hover_label_background_brush)
            dc.SetTextForeground(self._panel_hover_label_colour)        
        else:        
            dc.SetBrush(self._panel_label_background_brush)
            dc.SetTextForeground(self._panel_label_colour)
        
        label_rect = wx.Rect(*true_rect)
        label = wnd.GetLabel().strip()
        clip_label = False
        label_size = wx.Size(*dc.GetTextExtent(label))

        label_rect.SetX(label_rect.GetX() + 1)
        label_rect.SetWidth(label_rect.GetWidth() - 2)
        label_rect.SetHeight(label_size.GetHeight() + 2)
        label_rect.SetY(true_rect.GetBottom() - label_rect.GetHeight())
        label_height = label_rect.GetHeight()

        if label_size.GetWidth() > label_rect.GetWidth():        
            # Test if there is enough length for 3 letters and ...
            new_label = label[0:3] + "..."
            label_size = wx.Size(*dc.GetTextExtent(new_label))
            
            if label_size.GetWidth() > label_rect.GetWidth():            
                # Not enough room for three characters and ...
                # Display the entire label and just crop it
                clip_label = True
            else:            
                # Room for some characters and ...
                # Display as many characters as possible and append ...
                for l in xrange(len(label)-1, 3, -1):                
                    new_label = label[0:l] + "..."
                    label_size = wx.Size(*dc.GetTextExtent(new_label))
                    if label_size.GetWidth() <= label_rect.GetWidth():                    
                        label = new_label
                        break
                    
        dc.DrawRectangleRect(label_rect)
        
        if clip_label:        
            clip = wx.DCClipper(dc, label_rect)
            dc.DrawText(label, label_rect.x, label_rect.y + (label_rect.GetHeight() - label_size.GetHeight()) / 2)        
        else:        
            dc.DrawText(label, label_rect.x + (label_rect.GetWidth() - label_size.GetWidth()) / 2,
                        label_rect.y + (label_rect.GetHeight() - label_size.GetHeight()) / 2)
        
        if wnd.IsHovered():        
            client_rect = wx.Rect(*true_rect)
            client_rect.x += 1
            client_rect.width -= 2
            client_rect.y += 1
            client_rect.height -= 2 + label_height
            self.DrawPartialPageBackground(dc, wnd, client_rect, True)
        
        self.DrawPanelBorder(dc, true_rect, self._panel_border_pen, self._panel_border_gradient_pen)


    def DrawGalleryBackground(self, dc, wnd, rect):
        """
        Draw the background and chrome for a L{RibbonGallery} control.

        This should draw the border, brackground, scroll buttons, extension button, and
        any other UI elements which are not attached to a specific gallery item.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto, which is always the gallery
         whose background and chrome is being drawn. Attributes used during drawing like
         the gallery hover state and individual button states can be queried from this
         parameter by L{RibbonGallery.IsHovered}, L{RibbonGallery.GetExtensionButtonState},
         L{RibbonGallery.GetUpButtonState}, and L{RibbonGallery.GetDownButtonState};
        :param `rect`: The rectangle within which to draw. This rectangle is the entire
         area of the gallery control, not just the client rectangle.

        """

        self.DrawPartialPageBackground(dc, wnd, rect)

        if wnd.IsHovered():        
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(self._gallery_hover_background_brush)
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:            
                dc.DrawRectangle(rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 16)            
            else:            
                dc.DrawRectangle(rect.x + 1, rect.y + 1, rect.width - 16, rect.height - 2)
            
        dc.SetPen(self._gallery_border_pen)
        # Outline
        dc.DrawLine(rect.x + 1, rect.y, rect.x + rect.width - 1, rect.y)
        dc.DrawLine(rect.x, rect.y + 1, rect.x, rect.y + rect.height - 1)
        dc.DrawLine(rect.x + 1, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1)
        dc.DrawLine(rect.x + rect.width - 1, rect.y + 1, rect.x + rect.width - 1, rect.y + rect.height - 1)

        self.DrawGalleryBackgroundCommon(dc, wnd, rect)


    def DrawGalleryBackgroundCommon(self, dc, wnd, rect):

        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            # Divider between items and buttons
            dc.DrawLine(rect.x, rect.y + rect.height - 15, rect.x + rect.width, rect.y + rect.height - 15)

            up_btn = wx.Rect(rect.x, rect.y + rect.height - 15, rect.width / 3, 15)
            down_btn = wx.Rect(up_btn.GetRight() + 1, up_btn.GetTop(), up_btn.GetWidth(), up_btn.GetHeight())
            dc.DrawLine(down_btn.GetLeft(), down_btn.GetTop(), down_btn.GetLeft(), down_btn.GetBottom())
            ext_btn = wx.Rect(down_btn.GetRight() + 1, up_btn.GetTop(), rect.width - up_btn.GetWidth() - down_btn.GetWidth() - 1, up_btn.GetHeight())
            dc.DrawLine(ext_btn.GetLeft(), ext_btn.GetTop(), ext_btn.GetLeft(), ext_btn.GetBottom())
        
        else:        
            # Divider between items and buttons
            dc.DrawLine(rect.x + rect.width - 15, rect.y, rect.x + rect.width - 15, rect.y + rect.height)
            
            up_btn = wx.Rect(rect.x + rect.width - 15, rect.y, 15, rect.height / 3)
            down_btn = wx.Rect(up_btn.GetLeft(), up_btn.GetBottom() + 1, up_btn.GetWidth(), up_btn.GetHeight())
            dc.DrawLine(down_btn.GetLeft(), down_btn.GetTop(), down_btn.GetRight(), down_btn.GetTop())
            ext_btn = wx.Rect(up_btn.GetLeft(), down_btn.GetBottom() + 1, up_btn.GetWidth(), rect.height - up_btn.GetHeight() - down_btn.GetHeight() - 1)
            dc.DrawLine(ext_btn.GetLeft(), ext_btn.GetTop(), ext_btn.GetRight(), ext_btn.GetTop())
        
        self.DrawGalleryButton(dc, up_btn, wnd.GetUpButtonState(), self._gallery_up_bitmap)
        self.DrawGalleryButton(dc, down_btn, wnd.GetDownButtonState(), self._gallery_down_bitmap)
        self.DrawGalleryButton(dc, ext_btn, wnd.GetExtensionButtonState(), self._gallery_extension_bitmap)


    def DrawGalleryButton(self, dc, rect, state, bitmaps):

        if state == RIBBON_GALLERY_BUTTON_NORMAL:
            btn_top_brush = self._gallery_button_background_top_brush
            btn_colour = self._gallery_button_background_colour
            btn_grad_colour = self._gallery_button_background_gradient_colour
            btn_bitmap = bitmaps[0]
        elif state == RIBBON_GALLERY_BUTTON_HOVERED:
            btn_top_brush = self._gallery_button_hover_background_top_brush
            btn_colour = self._gallery_button_hover_background_colour
            btn_grad_colour = self._gallery_button_hover_background_gradient_colour
            btn_bitmap = bitmaps[1]
        elif state == RIBBON_GALLERY_BUTTON_ACTIVE:
            btn_top_brush = self._gallery_button_active_background_top_brush
            btn_colour = self._gallery_button_active_background_colour
            btn_grad_colour = self._gallery_button_active_background_gradient_colour
            btn_bitmap = bitmaps[2]
        elif state == RIBBON_GALLERY_BUTTON_DISABLED:
            btn_top_brush = self._gallery_button_disabled_background_top_brush
            btn_colour = self._gallery_button_disabled_background_colour
            btn_grad_colour = self._gallery_button_disabled_background_gradient_colour
            btn_bitmap = bitmaps[3]

        rect.x += 1
        rect.y += 1
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            rect.width -= 1
            rect.height -= 2
        else:        
            rect.width -= 2
            rect.height -= 1
        
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(btn_top_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height / 2)

        lower = wx.Rect(*rect)
        lower.height = (lower.height + 1) / 2
        lower.y += rect.height - lower.height
        dc.GradientFillLinear(lower, btn_colour, btn_grad_colour, wx.SOUTH)

        dc.DrawBitmap(btn_bitmap, rect.x + rect.width / 2 - 2, lower.y - 2, True)


    def DrawGalleryItemBackground(self, dc, wnd, rect, item):
        """
        Draw the background of a single item in a L{RibbonGallery} control.

        This is painted on top of a gallery background, and behind the items bitmap.
        Unlike L{DrawButtonBarButton} and L{DrawTool}, it is not expected to draw the
        item bitmap - that is done by the gallery control itself.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto, which is always the gallery
         which contains the item being drawn;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will be the size of the item's bitmap, expanded by gallery item padding values
         (``RIBBON_ART_GALLERY_BITMAP_PADDING_LEFT_SIZE``, ``RIBBON_ART_GALLERY_BITMAP_PADDING_RIGHT_SIZE``,
         ``RIBBON_ART_GALLERY_BITMAP_PADDING_TOP_SIZE``, and ``RIBBON_ART_GALLERY_BITMAP_PADDING_BOTTOM_SIZE``).
         The drawing rectangle will be entirely within a rectangle on the same device
         context previously painted with L{DrawGalleryBackground};
        :param `item`: The item whose background is being painted. Typically the background
         will vary if the item is hovered, active, or selected; L{RibbonGallery.GetSelection},
         L{RibbonGallery.GetActiveItem}, and L{RibbonGallery.GetHoveredItem} can be
         called to test if the given item is in one of these states.

        """

        if wnd.GetHoveredItem() != item and wnd.GetActiveItem() != item and \
           wnd.GetSelection() != item:
            return

        dc.SetPen(self._gallery_item_border_pen)
        dc.DrawLine(rect.x + 1, rect.y, rect.x + rect.width - 1, rect.y)
        dc.DrawLine(rect.x, rect.y + 1, rect.x, rect.y + rect.height - 1)
        dc.DrawLine(rect.x + 1, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1)
        dc.DrawLine(rect.x + rect.width - 1, rect.y + 1, rect.x + rect.width - 1, rect.y + rect.height - 1)

        if wnd.GetActiveItem() == item or wnd.GetSelection() == item:        
            top_brush = self._gallery_button_active_background_top_brush
            bg_colour = self._gallery_button_active_background_colour
            bg_gradient_colour = self._gallery_button_active_background_gradient_colour
        else:        
            top_brush = self._gallery_button_hover_background_top_brush
            bg_colour = self._gallery_button_hover_background_colour
            bg_gradient_colour = self._gallery_button_hover_background_gradient_colour
        
        upper = wx.Rect(*rect)
        upper.x += 1
        upper.width -= 2
        upper.y += 1
        upper.height /= 3
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(top_brush)
        dc.DrawRectangle(upper.x, upper.y, upper.width, upper.height)

        lower = wx.Rect(*upper)
        lower.y += lower.height
        lower.height = rect.height - 2 - lower.height
        dc.GradientFillLinear(lower, bg_colour, bg_gradient_colour, wx.SOUTH)


    def DrawPanelBorder(self, dc, rect, primary_colour, secondary_colour):

        border_points = [wx.Point() for i in xrange(9)]
        border_points[0] = wx.Point(2, 0)
        border_points[1] = wx.Point(rect.width - 3, 0)
        border_points[2] = wx.Point(rect.width - 1, 2)
        border_points[3] = wx.Point(rect.width - 1, rect.height - 3)
        border_points[4] = wx.Point(rect.width - 3, rect.height - 1)
        border_points[5] = wx.Point(2, rect.height - 1)
        border_points[6] = wx.Point(0, rect.height - 3)
        border_points[7] = wx.Point(0, 2)

        if primary_colour.GetColour() == secondary_colour.GetColour():        
            border_points[8] = border_points[0]
            dc.SetPen(primary_colour)
            dc.DrawLines(border_points, rect.x, rect.y)
        else:        
            dc.SetPen(primary_colour)
            dc.DrawLines(border_points[0:3], rect.x, rect.y)

            SingleLine(dc, rect, border_points[0], border_points[7])
            dc.SetPen(secondary_colour)
            dc.DrawLines(border_points[4:7], rect.x, rect.y)
            SingleLine(dc, rect, border_points[4], border_points[3])

            border_points[6] = border_points[2]
            RibbonDrawParallelGradientLines(dc, 2, border_points[6:8], 0, 1,
                                            border_points[3].y - border_points[2].y + 1, rect.x, rect.y,
                                            primary_colour.GetColour(), secondary_colour.GetColour())


    def DrawMinimisedPanel(self, dc, wnd, rect, bitmap):
        """
        Draw a minimised ribbon panel.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto, which is always the panel
         which is minimised. The panel label can be obtained from this window. The
         minimised icon obtained from querying the window may not be the size requested
         by L{GetMinimisedPanelMinimumSize} - the argument contains the icon in the
         requested size;
        :param `rect`: The rectangle within which to draw. The size of the rectangle
         will be at least the size returned by L{GetMinimisedPanelMinimumSize};
        :param `bitmap`: A copy of the panel's minimised bitmap rescaled to the size
         returned by L{GetMinimisedPanelMinimumSize}.

        """

        self.DrawPartialPageBackground(dc, wnd, rect, False)

        true_rect = wx.Rect(*rect)
        true_rect = self.RemovePanelPadding(true_rect)

        if wnd.GetExpandedPanel() != None:        
            client_rect = wx.Rect(*true_rect)
            client_rect.x += 1
            client_rect.width -= 2
            client_rect.y += 1
            client_rect.height = (rect.y + rect.height / 5) - client_rect.x
            dc.GradientFillLinear(client_rect,
                                  self._panel_active_background_top_colour,
                                  self._panel_active_background_top_gradient_colour, wx.SOUTH)

            client_rect.y += client_rect.height
            client_rect.height = (true_rect.y + true_rect.height) - client_rect.y
            dc.GradientFillLinear(client_rect,
                                  self._panel_active_background_colour,
                                  self._panel_active_background_gradient_colour, wx.SOUTH)
        
        elif wnd.IsHovered():
            client_rect = wx.Rect(*true_rect)
            client_rect.x += 1
            client_rect.width -= 2
            client_rect.y += 1
            client_rect.height -= 2
            self.DrawPartialPageBackground(dc, wnd, client_rect, True)

        preview = self.DrawMinimisedPanelCommon(dc, wnd, true_rect)

        dc.SetBrush(self._panel_hover_label_background_brush)
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.DrawRectangle(preview.x + 1, preview.y + preview.height - 8, preview.width - 2, 7)

        mid_pos = rect.y + rect.height / 5 - preview.y
        
        if mid_pos < 0 or mid_pos >= preview.height:        
            full_rect = wx.Rect(*preview)
            full_rect.x += 1
            full_rect.y += 1
            full_rect.width -= 2
            full_rect.height -= 9
            if mid_pos < 0:
                dc.GradientFillLinear(full_rect, self._page_hover_background_colour,
                                      self._page_hover_background_gradient_colour, wx.SOUTH)            
            else:            
                dc.GradientFillLinear(full_rect, self._page_hover_background_top_colour,
                                      self._page_hover_background_top_gradient_colour, wx.SOUTH)
                    
        else:
            top_rect = wx.Rect(*preview)
            top_rect.x += 1
            top_rect.y += 1
            top_rect.width -= 2
            top_rect.height = mid_pos
            dc.GradientFillLinear(top_rect, self._page_hover_background_top_colour,
                                  self._page_hover_background_top_gradient_colour, wx.SOUTH)

            btm_rect = wx.Rect(*top_rect)
            btm_rect.y = preview.y + mid_pos
            btm_rect.height = preview.y + preview.height - 7 - btm_rect.y
            dc.GradientFillLinear(btm_rect, self._page_hover_background_colour,
                                  self._page_hover_background_gradient_colour, wx.SOUTH)
        
        if bitmap.IsOk():        
            dc.DrawBitmap(bitmap, preview.x + (preview.width - bitmap.GetWidth()) / 2,
                          preview.y + (preview.height - 7 - bitmap.GetHeight()) / 2, True)
        
        self.DrawPanelBorder(dc, preview, self._panel_border_pen, self._panel_border_gradient_pen)
        self.DrawPanelBorder(dc, true_rect, self._panel_minimised_border_pen, self._panel_minimised_border_gradient_pen)


    def DrawMinimisedPanelCommon(self, dc, wnd, true_rect):

        preview = wx.Rect(0, 0, 32, 32)
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            preview.x = true_rect.x + 4
            preview.y = true_rect.y + (true_rect.height - preview.height) / 2
        else:        
            preview.x = true_rect.x + (true_rect.width - preview.width) / 2
            preview.y = true_rect.y + 4
        
        dc.SetFont(self._panel_label_font)
        label_width, label_height = dc.GetTextExtent(wnd.GetLabel())

        xpos = true_rect.x + (true_rect.width - label_width + 1) / 2
        ypos = preview.y + preview.height + 5

        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            xpos = preview.x + preview.width + 5
            ypos = true_rect.y + (true_rect.height - label_height) / 2
        
        dc.SetTextForeground(self._panel_minimised_label_colour)
        dc.DrawText(wnd.GetLabel(), xpos, ypos)
        
        arrow_points = [wx.Point() for i in xrange(3)]
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:
            xpos += label_width
            arrow_points[0] = wx.Point(xpos + 5, ypos + label_height / 2)
            arrow_points[1] = arrow_points[0] + wx.Point(-3,  3)
            arrow_points[2] = arrow_points[0] + wx.Point(-3, -3)
        else:        
            ypos += label_height
            arrow_points[0] = wx.Point(true_rect.width / 2, ypos + 5)
            arrow_points[1] = arrow_points[0] + wx.Point(-3, -3)
            arrow_points[2] = arrow_points[0] + wx.Point( 3, -3)
        
        dc.SetPen(wx.TRANSPARENT_PEN)
        B = wx.Brush(self._panel_minimised_label_colour)
        dc.SetBrush(B)
        dc.DrawPolygon(arrow_points, true_rect.x, true_rect.y)

        return preview


    def DrawButtonBarBackground(self, dc, wnd, rect):
        """
        Draw the background for a L{RibbonButtonBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto (which will typically be
         the button bar itself, though this is not guaranteed);
        :param `rect`: The rectangle within which to draw.

        """

        self.DrawPartialPageBackground(dc, wnd, rect, True)


    def DrawPartialPageBackground2(self, dc, wnd, rect, allow_hovered=True):

        # Assume the window is a child of a ribbon page, and also check for a
        # hovered panel somewhere between the window and the page, as it causes
        # the background to change.
        offset = wx.Point(*wnd.GetPosition())
        page = None
        parent = wnd.GetParent()
        hovered = False
        panel = None

        if isinstance(wnd, PANEL.RibbonPanel):
            panel = wnd
            hovered = allow_hovered and panel.IsHovered()
            if panel.GetExpandedDummy() != None:            
                offset = panel.GetExpandedDummy().GetPosition()
                parent = panel.GetExpandedDummy().GetParent()
            
        while 1:

            if panel is None:
                panel = parent
                if isinstance(panel, PANEL.RibbonPanel):
                    hovered = allow_hovered and panel.IsHovered()
                    if panel.GetExpandedDummy() != None:
                        parent = panel.GetExpandedDummy()

            page = parent                    
            if isinstance(page, PAGE.RibbonPage):
                break
            
            offset += parent.GetPosition()
            parent = parent.GetParent()
            if parent is None:
                break

        if page != None:
            self.DrawPartialPageBackground(dc, wnd, rect, page, offset, hovered)
            return
        
        # No page found - fallback to painting with a stock brush
        dc.SetBrush(wx.WHITE_BRUSH)
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)


    def DrawButtonBarButton(self, dc, wnd, rect, kind, state, label, bitmap_large, bitmap_small):
        """
        Draw a single button for a L{RibbonButtonBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will be a size previously returned by L{GetButtonBarButtonSize}, and the
         rectangle will be entirely within a rectangle on the same device context
         previously painted with L{DrawButtonBarBackground};
        :param `kind`: The kind of button to draw (normal, dropdown or hybrid);
        :param `state`: Combination of a size flag and state flags from the
         `RibbonButtonBarButtonState` enumeration;
        :param `label`: The label of the button;
        :param `bitmap_large`: The large bitmap of the button (or the large disabled
         bitmap when ``RIBBON_BUTTONBAR_BUTTON_DISABLED`` is set in );
        :param `bitmap_small`: The small bitmap of the button (or the small disabled
         bitmap when ``RIBBON_BUTTONBAR_BUTTON_DISABLED`` is set in ).
         
        """

        if state & (RIBBON_BUTTONBAR_BUTTON_HOVER_MASK | RIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK):        
            if state & RIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK:
                dc.SetPen(self._button_bar_active_border_pen)
            else:
                dc.SetPen(self._button_bar_hover_border_pen)

            bg_rect = wx.Rect(*rect)
            bg_rect.x += 1
            bg_rect.y += 1
            bg_rect.width -= 2
            bg_rect.height -= 2

            bg_rect_top = wx.Rect(*bg_rect)
            bg_rect_top.height /= 3
            bg_rect.y += bg_rect_top.height
            bg_rect.height -= bg_rect_top.height

            if kind == RIBBON_BUTTON_HYBRID:
            
                result = state & RIBBON_BUTTONBAR_BUTTON_SIZE_MASK
                
                if result == RIBBON_BUTTONBAR_BUTTON_LARGE:
                    iYBorder = rect.y + bitmap_large.GetHeight() + 4
                    partial_bg = wx.Rect(*rect)
                    
                    if state & RIBBON_BUTTONBAR_BUTTON_NORMAL_HOVERED:                        
                        partial_bg.SetBottom(iYBorder - 1)
                    else:
                        partial_bg.height -= (iYBorder - partial_bg.y + 1)
                        partial_bg.y = iYBorder + 1
                        
                    dc.DrawLine(rect.x, iYBorder, rect.x + rect.width, iYBorder)
                    bg_rect.Intersect(partial_bg)
                    bg_rect_top.Intersect(partial_bg)
                    
                elif result == RIBBON_BUTTONBAR_BUTTON_MEDIUM:
                    iArrowWidth = 9
                    
                    if state & RIBBON_BUTTONBAR_BUTTON_NORMAL_HOVERED:                    
                        bg_rect.width -= iArrowWidth
                        bg_rect_top.width -= iArrowWidth
                        dc.DrawLine(bg_rect_top.x + bg_rect_top.width, rect.y, bg_rect_top.x + bg_rect_top.width,
                                    rect.y + rect.height)                    
                    else:                    
                        iArrowWidth -= 1
                        bg_rect.x += bg_rect.width - iArrowWidth
                        bg_rect_top.x += bg_rect_top.width - iArrowWidth
                        bg_rect.width = iArrowWidth
                        bg_rect_top.width = iArrowWidth
                        dc.DrawLine(bg_rect_top.x - 1, rect.y, bg_rect_top.x - 1, rect.y + rect.height)
                        
            if state & RIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK:
            
                dc.GradientFillLinear(bg_rect_top, self._button_bar_active_background_top_colour,
                                      self._button_bar_active_background_top_gradient_colour, wx.SOUTH)
                dc.GradientFillLinear(bg_rect, self._button_bar_active_background_colour,
                                      self._button_bar_active_background_gradient_colour, wx.SOUTH)
            
            else:            
                dc.GradientFillLinear(bg_rect_top, self._button_bar_hover_background_top_colour,
                                      self._button_bar_hover_background_top_gradient_colour, wx.SOUTH)
                dc.GradientFillLinear(bg_rect, self._button_bar_hover_background_colour,
                                      self._button_bar_hover_background_gradient_colour, wx.SOUTH)

            border_points = [wx.Point() for i in xrange(9)]
            border_points[0] = wx.Point(2, 0)
            border_points[1] = wx.Point(rect.width - 3, 0)
            border_points[2] = wx.Point(rect.width - 1, 2)
            border_points[3] = wx.Point(rect.width - 1, rect.height - 3)
            border_points[4] = wx.Point(rect.width - 3, rect.height - 1)
            border_points[5] = wx.Point(2, rect.height - 1)
            border_points[6] = wx.Point(0, rect.height - 3)
            border_points[7] = wx.Point(0, 2)
            border_points[8] = border_points[0]

            dc.DrawLines(border_points, rect.x, rect.y)
        
        dc.SetFont(self._button_bar_label_font)
        dc.SetTextForeground(self._button_bar_label_colour)
        self.DrawButtonBarButtonForeground(dc, rect, kind, state, label, bitmap_large, bitmap_small)


    def DrawButtonBarButtonForeground(self, dc, rect, kind, state, label, bitmap_large, bitmap_small):

        result = state & RIBBON_BUTTONBAR_BUTTON_SIZE_MASK
        
        if result == RIBBON_BUTTONBAR_BUTTON_LARGE:
            
            padding = 2
            dc.DrawBitmap(bitmap_large, rect.x + (rect.width - bitmap_large.GetWidth()) / 2,
                          rect.y + padding, True)
            ypos = rect.y + padding + bitmap_large.GetHeight() + padding
            arrow_width = (kind == RIBBON_BUTTON_NORMAL and [0] or [8])[0]

            label_w, label_h = dc.GetTextExtent(label)
            
            if label_w + 2 * padding <= rect.width:
            
                dc.DrawText(label, rect.x + (rect.width - label_w) / 2, ypos)
                if arrow_width != 0:                
                    self.DrawDropdownArrow(dc, rect.x + rect.width / 2,
                                           ypos + (label_h * 3) / 2,
                                           self._button_bar_label_colour)
            else:
                breaki = len(label)
                
                while breaki > 0:                
                    breaki -= 1
                    if RibbonCanLabelBreakAtPosition(label, breaki):                    
                        label_top = label[0:breaki]
                        label_w, label_h = dc.GetTextExtent(label_top)

                        if label_w + 2 * padding <= rect.width:                        
                            dc.DrawText(label_top, rect.x + (rect.width - label_w) / 2, ypos)
                            ypos += label_h
                            label_bottom = label[breaki:]
                            label_w, label_h = dc.GetTextExtent(label_bottom)
                            label_w += arrow_width
                            iX = rect.x + (rect.width - label_w) / 2
                            dc.DrawText(label_bottom, iX, ypos)
                            
                            if arrow_width != 0:                            
                                self.DrawDropdownArrow(dc, iX + 2 +label_w - arrow_width,
                                                       ypos + label_h / 2 + 1,
                                                       self._button_bar_label_colour)
                            
                            break

        elif result == RIBBON_BUTTONBAR_BUTTON_MEDIUM:
        
            x_cursor = rect.x + 2
            dc.DrawBitmap(bitmap_small, x_cursor, rect.y + (rect.height - bitmap_small.GetHeight())/2, True)
            x_cursor += bitmap_small.GetWidth() + 2
            label_w, label_h = dc.GetTextExtent(label)
            dc.DrawText(label, x_cursor, rect.y + (rect.height - label_h) / 2)
            x_cursor += label_w + 3
            
            if kind != RIBBON_BUTTON_NORMAL:
                self.DrawDropdownArrow(dc, x_cursor, rect.y + rect.height / 2,
                                       self._button_bar_label_colour)
        
        else:
            # TODO
            pass
    

    def DrawToolBarBackground(self, dc, wnd, rect):
        """
        Draw the background for a L{RibbonToolBar} control.


        :param `dc`: The device context to draw onto;
        :param `wnd`: The which is being drawn onto. In most cases this will be a
         L{RibbonToolBar}, but it doesn't have to be;
        :param `rect`: The rectangle within which to draw. Some of this rectangle
         will later be drawn over using L{DrawToolGroupBackground} and L{DrawTool},
         but not all of it will (unless there is only a single group of tools).

        """

        self.DrawPartialPageBackground(dc, wnd, rect)


    def DrawToolGroupBackground(self, dc, wnd, rect):
        """
        Draw the background for a group of tools on a L{RibbonToolBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto. In most cases this will
         be a L{RibbonToolBar}, but it doesn't have to be;
        :param `rect`: The rectangle within which to draw. This rectangle is a union
         of the individual tools' rectangles. As there are no gaps between tools,
         this rectangle will be painted over exactly once by calls to L{DrawTool}.
         The group background could therefore be painted by L{DrawTool}, though it
         can be conceptually easier and more efficient to draw it all at once here.
         The rectangle will be entirely within a rectangle on the same device context
         previously painted with L{DrawToolBarBackground}.

        """

        dc.SetPen(self._toolbar_border_pen)
        outline = [wx.Point() for i in xrange(9)]
        outline[0] = wx.Point(2, 0)
        outline[1] = wx.Point(rect.width - 3, 0)
        outline[2] = wx.Point(rect.width - 1, 2)
        outline[3] = wx.Point(rect.width - 1, rect.height - 3)
        outline[4] = wx.Point(rect.width - 3, rect.height - 1)
        outline[5] = wx.Point(2, rect.height - 1)
        outline[6] = wx.Point(0, rect.height - 3)
        outline[7] = wx.Point(0, 2)
        outline[8] = outline[0]

        dc.DrawLines(outline, rect.x, rect.y)


    def DrawTool(self, dc, wnd, rect, bitmap, kind, state):
        """
        Draw a single tool (for a L{RibbonToolBar} control).

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto. In most cases this will
         be a L{RibbonToolBar}, but it doesn't have to be;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will at least the size returned by L{GetToolSize}, and the height of it will
         be equal for all tools within the same group. The rectangle will be entirely
         within a rectangle on the same device context previously painted with
         L{DrawToolGroupBackground};
        :param `bitmap`: The bitmap to use as the tool's foreground. If the tool is a
         hybrid or dropdown tool, then the foreground should also contain a standard
         dropdown button;
        :param `kind`: The kind of tool to draw (normal, dropdown, or hybrid);
        :param `state`: A combination of wx.RibbonToolBarToolState flags giving the
         state of the tool and it's relative position within a tool group.

        """

        bg_rect = wx.Rect(*rect)
        bg_rect.Deflate(1, 1)
        
        if state & RIBBON_TOOLBAR_TOOL_LAST == 0:
            bg_rect.width += 1
            
        is_split_hybrid = (kind == RIBBON_BUTTON_HYBRID and (state & (RIBBON_TOOLBAR_TOOL_HOVER_MASK | RIBBON_TOOLBAR_TOOL_ACTIVE_MASK)))

        # Background
        bg_rect_top = wx.Rect(*bg_rect)
        bg_rect_top.height = (bg_rect_top.height * 2) / 5
        bg_rect_btm = wx.Rect(*bg_rect)
        bg_rect_btm.y += bg_rect_top.height
        bg_rect_btm.height -= bg_rect_top.height

        bg_top_colour = self._tool_background_top_colour
        bg_top_grad_colour = self._tool_background_top_gradient_colour
        bg_colour = self._tool_background_colour
        bg_grad_colour = self._tool_background_gradient_colour
        
        if state & RIBBON_TOOLBAR_TOOL_ACTIVE_MASK:
            bg_top_colour = self._tool_active_background_top_colour
            bg_top_grad_colour = self._tool_active_background_top_gradient_colour
            bg_colour = self._tool_active_background_colour
            bg_grad_colour = self._tool_active_background_gradient_colour
        
        elif state & RIBBON_TOOLBAR_TOOL_HOVER_MASK:        
            bg_top_colour = self._tool_hover_background_top_colour
            bg_top_grad_colour = self._tool_hover_background_top_gradient_colour
            bg_colour = self._tool_hover_background_colour
            bg_grad_colour = self._tool_hover_background_gradient_colour
        
        dc.GradientFillLinear(bg_rect_top, bg_top_colour, bg_top_grad_colour, wx.SOUTH)
        dc.GradientFillLinear(bg_rect_btm, bg_colour, bg_grad_colour, wx.SOUTH)
        
        if is_split_hybrid:        
            nonrect = wx.Rect(*bg_rect)
            if state & (RIBBON_TOOLBAR_TOOL_DROPDOWN_HOVERED | RIBBON_TOOLBAR_TOOL_DROPDOWN_ACTIVE):            
                nonrect.width -= 8
            else:            
                nonrect.x += nonrect.width - 8
                nonrect.width = 8
            
            B = wx.Brush(self._tool_hover_background_top_colour)
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(B)
            dc.DrawRectangle(nonrect.x, nonrect.y, nonrect.width, nonrect.height)
        
        # Border
        dc.SetPen(self._toolbar_border_pen)
        
        if state & RIBBON_TOOLBAR_TOOL_FIRST:        
            dc.DrawPoint(rect.x + 1, rect.y + 1)
            dc.DrawPoint(rect.x + 1, rect.y + rect.height - 2)        
        else:
            dc.DrawLine(rect.x, rect.y + 1, rect.x, rect.y + rect.height - 1)   

        if state & RIBBON_TOOLBAR_TOOL_LAST:        
            dc.DrawPoint(rect.x + rect.width - 2, rect.y + 1)
            dc.DrawPoint(rect.x + rect.width - 2, rect.y + rect.height - 2)
        
        # Foreground
        avail_width = bg_rect.GetWidth()
        
        if kind != RIBBON_BUTTON_NORMAL:        
            avail_width -= 8
            if is_split_hybrid:            
                dc.DrawLine(rect.x + avail_width + 1, rect.y, rect.x + avail_width + 1, rect.y + rect.height)
            
            dc.DrawBitmap(self._toolbar_drop_bitmap, bg_rect.x + avail_width + 2,
                          bg_rect.y + (bg_rect.height / 2) - 2, True)
        
        dc.DrawBitmap(bitmap, bg_rect.x + (avail_width - bitmap.GetWidth()) / 2,
                      bg_rect.y + (bg_rect.height - bitmap.GetHeight()) / 2, True)


    def GetBarTabWidth(self, dc, wnd, label, bitmap, ideal=None, small_begin_need_separator=None,
                       small_must_have_separator=None, minimum=None):

        """
        Calculate the ideal and minimum width (in pixels) of a tab in a ribbon bar.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the tab will eventually be drawn;
        :param `label`: The tab's label (or "" if it has none);
        :param `bitmap`: The tab's icon (or wx.NullBitmap if it has none);
        :param `ideal`: The ideal width (in pixels) of the tab;
        :param `small_begin_need_separator`: A size less than the size, at which a
         tab separator should begin to be drawn (i.e. drawn, but still fairly transparent);
        :param `small_must_have_separator`: A size less than the size, at which a
         tab separator must be drawn (i.e. drawn at full opacity);
        :param `minimum`: A size less than the size, and greater than or equal to
         zero, which is the minimum pixel width for the tab.

        """

        width = 0
        mini = 0
        
        if (self._flags & RIBBON_BAR_SHOW_PAGE_LABELS) and label.strip():        
            dc.SetFont(self._tab_label_font)
            width += dc.GetTextExtent(label)[0]
            mini += min(25, width) # enough for a few chars
            
            if bitmap.IsOk():            
                # gap between label and bitmap
                width += 4
                mini += 2
            
        if (self._flags & RIBBON_BAR_SHOW_PAGE_ICONS) and bitmap.IsOk():
            width += bitmap.GetWidth()
            mini += bitmap.GetWidth()
        
        ideal = width + 30        
        small_begin_need_separator = width + 20        
        small_must_have_separator = width + 10        
        minimum = mini
        
        return ideal, small_begin_need_separator, small_must_have_separator, minimum


    def GetTabCtrlHeight(self, dc, wnd, pages):
        """
        Calculate the height (in pixels) of the tab region of a ribbon bar.

        Note that as the tab region can contain scroll buttons, the height should be
        greater than or equal to the minimum height for a tab scroll button.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the tabs will eventually be drawn;
        :param `pages`: The tabs which will acquire the returned height.

        """

        text_height = 0
        icon_height = 0

        if len(pages) <= 1 and (self._flags & RIBBON_BAR_ALWAYS_SHOW_TABS) == 0:
            # To preserve space, a single tab need not be displayed. We still need
            # two pixels of border / padding though.
            return 2
        
        if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS:        
            dc.SetFont(self._tab_label_font)
            text_height = dc.GetTextExtent("ABCDEFXj")[1] + 10
        
        if self._flags & RIBBON_BAR_SHOW_PAGE_ICONS:
            for info in pages:
                if info.page.GetIcon().IsOk():                
                    icon_height = max(icon_height, info.page.GetIcon().GetHeight() + 4)

        return max(text_height, icon_height)


    def GetScrollButtonMinimumSize(self, dc, wnd, style):
        """
        Calculate the minimum size (in pixels) of a scroll button.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the scroll button will eventually be drawn;
        :param `style`: A combination of flags from `RibbonScrollButtonStyle`, including
         a direction, and a for flag (state flags may be given too, but should be ignored,
         as a button should retain a constant size, regardless of its state).

        """

        return wx.Size(12, 12)


    def GetPanelSize(self, dc, wnd, client_size, client_offset=None):
        """
        Calculate the size of a panel for a given client size.

        This should increment the given size by enough to fit the panel label and other
        chrome.

        :param `dc`: A device context to use if one is required for size calculations;
        :param `wnd`: The ribbon panel in question;
        :param `client_size`: The client size;
        :param `client_offset`: The offset where the client rectangle begins within the
         panel (may be ``None``).

        :see: L{GetPanelClientSize}
        """

        dc.SetFont(self._panel_label_font)
        label_size = wx.Size(*dc.GetTextExtent(wnd.GetLabel()))

        client_size.IncBy(0, label_size.GetHeight())
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:
            client_size.IncBy(4, 8)
        else:
            client_size.IncBy(6, 6)

        if client_offset != None:        
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:
                client_offset = wx.Point(2, 3)
            else:
                client_offset = wx.Point(3, 2)
        
        return client_size


    def GetPanelClientSize(self, dc, wnd, size, client_offset=None):
        """
        Calculate the client size of a panel for a given overall size.

        This should act as the inverse to L{GetPanelSize}, and decrement the given size
        by enough to fit the panel label and other chrome.

        :param `dc`: A device context to use if one is required for size calculations;
        :param `wnd`: The ribbon panel in question;
        :param `size`: The overall size to calculate client size for;
        :param `client_offset`: The offset where the returned client size begins within
         the given (may be ``None``).

        :see: L{GetPanelSize}
        """

        dc.SetFont(self._panel_label_font)
        label_size = wx.Size(*dc.GetTextExtent(wnd.GetLabel()))

        size.DecBy(0, label_size.GetHeight())
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:
            size.DecBy(4, 8)
        else:
            size.DecBy(6, 6)

        if client_offset != None:        
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:
                client_offset = wx.Point(2, 3)
            else:
                client_offset = wx.Point(3, 2)
        
        return size, client_offset


    def GetGallerySize(self, dc, wnd, client_size):
        """
        Calculate the size of a L{RibbonGallery} control for a given client size.

        This should increment the given size by enough to fit the gallery border,
        buttons, and any other chrome.

        :param `dc`: A device context to use if one is required for size calculations;
        :param `wnd`: The gallery in question;
        :param `client_size`: The client size.

        :see: L{GetGalleryClientSize}
        """

        client_size.IncBy(2, 1) # Left / top padding

        if self._flags & RIBBON_BAR_FLOW_VERTICAL:
            client_size.IncBy(1, 16) # Right / bottom padding
        else:
            client_size.IncBy(16, 1) # Right / bottom padding

        return client_size


    def GetGalleryClientSize(self, dc, wnd, size, client_offset=None, scroll_up_button=None,
                             scroll_down_button=None, extension_button=None):

        """
        Calculate the client size of a L{RibbonGallery} control for a given size.

        This should act as the inverse to L{GetGallerySize}, and decrement the given
        size by enough to fir the gallery border, buttons, and other chrome.

        :param `dc`: A device context to use if one is required for size calculations;
        :param `wnd`: The gallery in question;
        :param `size`: The overall size to calculate the client size for;
        :param `client_offset`: The position within the given size at which the
         returned client size begins;
        :param `scroll_up_button`: The rectangle within the given size which the
         scroll up button occupies;
        :param `scroll_down_button`: The rectangle within the given size which the
         scroll down button occupies;
        :param `extension_button`: The rectangle within the given size which the
         extension button occupies.

        """

        scroll_up = wx.Rect()
        scroll_down = wx.Rect()
        extension = wx.Rect()
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:
            # Flow is vertical - put buttons on bottom
            scroll_up.y = size.GetHeight() - 15
            scroll_up.height = 15
            scroll_up.x = 0
            scroll_up.width = (size.GetWidth() + 2) / 3
            scroll_down.y = scroll_up.y
            scroll_down.height = scroll_up.height
            scroll_down.x = scroll_up.x + scroll_up.width
            scroll_down.width = scroll_up.width        
            extension.y = scroll_down.y
            extension.height = scroll_down.height
            extension.x = scroll_down.x + scroll_down.width
            extension.width = size.GetWidth() - scroll_up.width - scroll_down.width
            size.DecBy(1, 16)
            size.DecBy(2, 1)
        
        else:
            # Flow is horizontal - put buttons on right
            scroll_up.x = size.GetWidth() - 15
            scroll_up.width = 15
            scroll_up.y = 0
            scroll_up.height = (size.GetHeight() + 2) / 3
            scroll_down.x = scroll_up.x
            scroll_down.width = scroll_up.width
            scroll_down.y = scroll_up.y + scroll_up.height
            scroll_down.height = scroll_up.height        
            extension.x = scroll_down.x
            extension.width = scroll_down.width
            extension.y = scroll_down.y + scroll_down.height
            extension.height = size.GetHeight() - scroll_up.height - scroll_down.height
            size.DecBy(16, 1)
            size.DecBy( 2, 1)
        
        client_offset = wx.Point(2, 1)
        scroll_up_button = scroll_up
        scroll_down_button = scroll_down
        extension_button = extension

        return size, client_offset, scroll_up_button, scroll_down_button, extension_button


    def GetPageBackgroundRedrawArea(self, dc, wnd, page_old_size, page_new_size):
        """
        Calculate the portion of a page background which needs to be redrawn when a page
        is resized.

        To optimise the drawing of page backgrounds, as small an area as possible should
        be returned. Of couse, if the way in which a background is drawn means that the
        entire background needs to be repainted on resize, then the entire new size
        should be returned.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The page which is being resized;
        :param `page_old_size`: The size of the page prior to the resize (which has
         already been painted);
        :param `page_new_size`: The size of the page after the resize.

        """

        if page_new_size.GetWidth() != page_old_size.GetWidth():
            if page_new_size.GetHeight() != page_old_size.GetHeight():
                # Width and height both changed - redraw everything
                return wx.Rect(0, 0, *page_new_size)
            else:            
                # Only width changed - redraw right hand side
                right_edge_width = 4
                new_rect = wx.Rect(page_new_size.GetWidth() - right_edge_width, 0, right_edge_width, page_new_size.GetHeight())
                old_rect = wx.Rect(page_old_size.GetWidth() - right_edge_width, 0, right_edge_width, page_old_size.GetHeight())
            
        else:        
            if page_new_size.GetHeight() == page_old_size.GetHeight():            
                # Nothing changed (should never happen) - redraw nothing
                return wx.Rect(0, 0, 0, 0)
            else:            
                # Height changed - need to redraw everything (as the background
                # gradient is done vertically).
                return wx.Rect(0, 0, *page_new_size)
            
        new_rect.Union(old_rect)
        new_rect.Intersect(wx.Rect(0, 0, *page_new_size))
        return new_rect


    def GetButtonBarButtonSize(self, dc, wnd, kind, size, label, bitmap_size_large, bitmap_size_small,
                               button_size=None, normal_region=None, dropdown_region=None):
        """
        Calculate the size of a button within a L{RibbonButtonBar}.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the button will eventually be drawn
         (which is normally a L{RibbonButtonBar}, though this is not guaranteed);
        :param `kind`: The kind of button;
        :param `size`: The size-class to calculate the size for. Buttons on a button
         bar can have three distinct sizes: ``RIBBON_BUTTONBAR_BUTTON_SMALL``,
         ``RIBBON_BUTTONBAR_BUTTON_MEDIUM``, and ``RIBBON_BUTTONBAR_BUTTON_LARGE``.
         If the requested size-class is not applicable, then ``False`` should be returned;
        :param `label`: The label of the button;
        :param `bitmap_size_large`: The size of all "large" bitmaps on the button bar;
        :param `bitmap_size_small`: The size of all "small" bitmaps on the button bar;
        :param `button_size`: The size, in pixels, of the button;
        :param `normal_region`: The region of the button which constitutes the normal button;
        :param `dropdown_region`: The region of the button which constitutes the dropdown button.

        :returns: ``True`` if a size exists for the button, ``False`` otherwise.
        """

        drop_button_width = 8

        normal_region = wx.Rect()
        dropdown_region = wx.Rect()
        
        dc.SetFont(self._button_bar_label_font)
        result = size & RIBBON_BUTTONBAR_BUTTON_SIZE_MASK
        
        if result == RIBBON_BUTTONBAR_BUTTON_SMALL:
            # Small bitmap, no label
            button_size = bitmap_size_small + wx.Size(6, 4)
            
            if kind == RIBBON_BUTTON_NORMAL:
                normal_region = wx.Rect(0, 0, *button_size)
                dropdown_region = wx.Rect(0, 0, 0, 0)

            elif kind == RIBBON_BUTTON_DROPDOWN:
                button_size += wx.Size(drop_button_width, 0)
                dropdown_region = wx.Rect(0, 0, *button_size)
                normal_region = wx.Rect(0, 0, 0, 0)

            elif kind == RIBBON_BUTTON_HYBRID:
                normal_region = wx.Rect(0, 0, *button_size)
                dropdown_region = wx.Rect(button_size.GetWidth(), 0, drop_button_width, button_size.GetHeight())
                button_size += wx.Size(drop_button_width, 0)

        elif result == RIBBON_BUTTONBAR_BUTTON_MEDIUM:
            # Small bitmap, with label to the right
            is_supported, button_size, normal_region, dropdown_region = self.GetButtonBarButtonSize(dc, wnd, kind,
                                                                                                    RIBBON_BUTTONBAR_BUTTON_SMALL,
                                                                                                    label, bitmap_size_large,
                                                                                                    bitmap_size_small)
            text_size = dc.GetTextExtent(label)[0]
            button_size.SetWidth(button_size.GetWidth() + text_size)

            if kind == RIBBON_BUTTON_DROPDOWN:
                dropdown_region.SetWidth(dropdown_region.GetWidth() + text_size)

            elif kind == RIBBON_BUTTON_HYBRID:
                dropdown_region.SetX(dropdown_region.GetX() + text_size)
                normal_region.SetWidth(normal_region.GetWidth() + text_size)
                # no break
            elif kind == RIBBON_BUTTON_NORMAL:
                normal_region.SetWidth(normal_region.GetWidth() + text_size)
            
        elif result == RIBBON_BUTTONBAR_BUTTON_LARGE:
            # Large bitmap, with label below (possibly split over 2 lines)
            
            icon_size = wx.Size(*bitmap_size_large)
            icon_size += wx.Size(4, 4)
            best_width, label_height = dc.GetTextExtent(label)
#            label_height += 1
            best_num_lines = 1
            last_line_extra_width = 0
            
            if kind != RIBBON_BUTTON_NORMAL:            
                last_line_extra_width += 8
                best_num_lines = 2 # label on top line, button below
            
            for i in xrange(0, len(label)):            
                if RibbonCanLabelBreakAtPosition(label, i):
                    
                    width = max(dc.GetTextExtent(label[0:i])[0],
                                dc.GetTextExtent(label[i+1:])[0] + last_line_extra_width)
                    if width < best_width:
                        best_width = width
                        best_num_lines = 2

            label_height *= 2 # Assume two lines even when only one is used
                              # (to give all buttons a consistent height)
            icon_size.SetWidth(max(icon_size.GetWidth(), best_width) + 6)
            icon_size.SetHeight(icon_size.GetHeight() + label_height)
            button_size = wx.Size(*icon_size)

            if kind == RIBBON_BUTTON_DROPDOWN:
                dropdown_region = wx.Rect(0, 0, *icon_size)
            elif kind == RIBBON_BUTTON_HYBRID:
                normal_region = wx.Rect(0, 0, *icon_size)
                normal_region.height -= 2 + label_height
                dropdown_region.x = 0
                dropdown_region.y = normal_region.height
                dropdown_region.width = icon_size.GetWidth()
                dropdown_region.height = icon_size.GetHeight() - normal_region.height
            elif kind == RIBBON_BUTTON_NORMAL:
                normal_region = wx.Rect(0, 0, *icon_size)
            
        return True, button_size, normal_region, dropdown_region


    def GetMinimisedPanelMinimumSize(self, dc, wnd, desired_bitmap_size=None, expanded_panel_direction=None):
        """
        Calculate the size of a minimised ribbon panel.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The ribbon panel in question. Attributes like the panel label can
         be queried from this;
        :param `desired_bitmap_size`: MISSING DESCRIPTION;
        :param `expanded_panel_direction`: MISSING DESCRIPTION.

        """

        if desired_bitmap_size != None:        
            desired_bitmap_size = wx.Size(16, 16)
        
        if expanded_panel_direction != None:        
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:
                expanded_panel_direction = wx.EAST
            else:
                expanded_panel_direction = wx.SOUTH
        
        base_size = wx.Size(42, 42)

        dc.SetFont(self._panel_label_font)
        label_size = wx.Size(*dc.GetTextExtent(wnd.GetLabel()))
        label_size.IncBy(2, 2) # Allow for differences between this DC and a paint DC
        label_size.IncBy(6, 0) # Padding
        label_size.y *= 2 # Second line for dropdown button

        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            # Label alongside icon
            return wx.Size(base_size.x + label_size.x, max(base_size.y, label_size.y)), \
                   desired_bitmap_size, expanded_panel_direction
        else:        
            # Label beneath icon
            return wx.Size(max(base_size.x, label_size.x), base_size.y + label_size.y), \
                   desired_bitmap_size, expanded_panel_direction
        

    def GetToolSize(self, dc, wnd, bitmap_size, kind, is_first, is_last, dropdown_region=None):
        """
        Calculate the size of a tool within a L{RibbonToolBar}.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the tool will eventually be drawn;
        :param `bitmap_size`: The size of the tool's foreground bitmap;
        :param `kind`: The kind of tool (normal, dropdown, or hybrid);
        :param `is_first`: ``True`` if the tool is the first within its group. ``False``
         otherwise;
        :param `is_last`: ``True`` if the tool is the last within its group. ``False``
         otherwise;
        :param `dropdown_region`: For dropdown and hybrid tools, the region within the
         returned size which counts as the dropdown part.
        """

        size = wx.Size(*bitmap_size)
        size.IncBy(7, 6)

        if is_last:
            size.IncBy(1, 0)

        if kind != RIBBON_BUTTON_NORMAL:
            size.IncBy(8, 0)
            if kind == RIBBON_BUTTON_DROPDOWN:
                dropdown_region = wx.Rect(0, 0, *size)
            else:
                dropdown_region = wx.Rect(size.GetWidth() - 8, 0, 8, size.GetHeight())
        else:        
            dropdown_region = wx.Rect(0, 0, 0, 0)

        return size, dropdown_region

