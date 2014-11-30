"""
L{RibbonAUIArtProvider} is responsible for drawing all the components of the ribbon
interface using an AUI-compatible appearance.


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

from math import cos
from math import pi as M_PI

from art_msw import RibbonMSWArtProvider
from art_internal import RibbonHSLColour, RibbonShiftLuminance, RibbonInterpolateColour

import bar as BAR, panel as PANEL

from art import *

if wx.Platform == "__WXMAC__":
    import Carbon.Appearance


def FontFromFont(original):

    newFont = wx.Font(original.GetPointSize(), original.GetFamily(),
                      original.GetStyle(), original.GetWeight(), original.GetUnderlined(),
                      original.GetFaceName(), original.GetEncoding())

    return newFont

    
class RibbonAUIArtProvider(RibbonMSWArtProvider):

    def __init__(self):

        RibbonMSWArtProvider.__init__(self)
        
        if wx.Platform == "__WXMAC__":

            if hasattr(wx, 'MacThemeColour'):
                base_colour = wx.MacThemeColour(Carbon.Appearance.kThemeBrushToolbarBackground)
            else:
                brush = wx.Brush(wx.BLACK)
                brush.MacSetTheme(Carbon.Appearance.kThemeBrushToolbarBackground)
                base_colour = brush.GetColour()
        else:
            
            base_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DFACE)

        self.SetColourScheme(base_colour, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT),
                             wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT))

        self._tab_active_label_font = FontFromFont(self._tab_label_font)
        self._tab_active_label_font.SetWeight(wx.FONTWEIGHT_BOLD)

        self._page_border_left = 1
        self._page_border_right = 1
        self._page_border_top = 1
        self._page_border_bottom = 2
        self._tab_separation_size = 0

        self._gallery_bitmap_padding_left_size = 3
        self._gallery_bitmap_padding_right_size = 3
        self._gallery_bitmap_padding_top_size = 3
        self._gallery_bitmap_padding_bottom_size = 3


    def Clone(self):
        """
        Create a new art provider which is a clone of this one.
        """

        copy = RibbonAUIArtProvider()
        self.CloneTo(copy)

        copy._tab_ctrl_background_colour = self._tab_ctrl_background_colour
        copy._tab_ctrl_background_gradient_colour = self._tab_ctrl_background_gradient_colour
        copy._panel_label_background_colour = self._panel_label_background_colour
        copy._panel_label_background_gradient_colour = self._panel_label_background_gradient_colour
        copy._panel_hover_label_background_colour = self._panel_hover_label_background_colour
        copy._panel_hover_label_background_gradient_colour = self._panel_hover_label_background_gradient_colour

        copy._background_brush = self._background_brush
        copy._tab_active_top_background_brush = self._tab_active_top_background_brush
        copy._tab_hover_background_brush = self._tab_hover_background_brush
        copy._button_bar_hover_background_brush = self._button_bar_hover_background_brush
        copy._button_bar_active_background_brush = self._button_bar_active_background_brush
        copy._gallery_button_active_background_brush = self._gallery_button_active_background_brush
        copy._gallery_button_hover_background_brush = self._gallery_button_hover_background_brush
        copy._gallery_button_disabled_background_brush = self._gallery_button_disabled_background_brush

        copy._toolbar_hover_borden_pen = self._toolbar_hover_borden_pen
        copy._tool_hover_background_brush = self._tool_hover_background_brush
        copy._tool_active_background_brush = self._tool_active_background_brush

        return copy


    def SetFont(self, id, font):
        """
        Set the value of a certain font setting to the value.

        can be one of the font values of `RibbonArtSetting`.

        :param `id`: the font id;
        :param `font`: MISSING DESCRIPTION.

        """

        RibbonMSWArtProvider.SetFont(self, id, font)
        
        if id == RIBBON_ART_TAB_LABEL_FONT:        
            self._tab_active_label_font = FontFromFont(self._tab_label_font)
            self._tab_active_label_font.SetWeight(wx.FONTWEIGHT_BOLD)
    

    def GetColour(self, id):
        """
        Get the value of a certain colour setting.

        can be one of the colour values of `RibbonArtSetting`.

        :param `id`: the colour id.

        """

        if id in [RIBBON_ART_PAGE_BACKGROUND_COLOUR, RIBBON_ART_PAGE_BACKGROUND_GRADIENT_COLOUR]:
            return self._background_brush.GetColour()
        elif id == RIBBON_ART_TAB_CTRL_BACKGROUND_COLOUR:
            return self._tab_ctrl_background_colour
        elif id == RIBBON_ART_TAB_CTRL_BACKGROUND_GRADIENT_COLOUR:
            return self._tab_ctrl_background_gradient_colour
        elif id in [RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_COLOUR, RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR]:
            return self._tab_active_top_background_brush.GetColour()
        elif id in [RIBBON_ART_TAB_HOVER_BACKGROUND_COLOUR, RIBBON_ART_TAB_HOVER_BACKGROUND_GRADIENT_COLOUR]:
            return self._tab_hover_background_brush.GetColour()
        elif id == RIBBON_ART_PANEL_LABEL_BACKGROUND_COLOUR:
            return self._panel_label_background_colour
        elif id == RIBBON_ART_PANEL_LABEL_BACKGROUND_GRADIENT_COLOUR:
            return self._panel_label_background_gradient_colour
        elif id == RIBBON_ART_PANEL_HOVER_LABEL_BACKGROUND_COLOUR:
            return self._panel_hover_label_background_colour
        elif id == RIBBON_ART_PANEL_HOVER_LABEL_BACKGROUND_GRADIENT_COLOUR:
            return self._panel_hover_label_background_gradient_colour
        elif id in [RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_COLOUR, RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_GRADIENT_COLOUR]:
            return self._button_bar_hover_background_brush.GetColour()
        elif id in [RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_COLOUR, RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_GRADIENT_COLOUR]:
            return self._gallery_button_hover_background_brush.GetColour()
        elif id in [RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_COLOUR, RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_GRADIENT_COLOUR]:
            return self._gallery_button_active_background_brush.GetColour()
        elif id in [RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_COLOUR, RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_GRADIENT_COLOUR]:
            return self._gallery_button_disabled_background_brush.GetColour()
        else:
            return RibbonMSWArtProvider.GetColour(self, id)
    

    def SetColour(self, id, colour):
        """
        Set the value of a certain colour setting to the value.

        can be one of the colour values of `RibbonArtSetting`, though not all colour
        settings will have an affect on every art provider.

        :param `id`: the colour id;
        :param `colour`: MISSING DESCRIPTION.

        :see: L{SetColourScheme}
        """

        if id in [RIBBON_ART_PAGE_BACKGROUND_COLOUR, RIBBON_ART_PAGE_BACKGROUND_GRADIENT_COLOUR]:
            self._background_brush.SetColour(colour)
        elif id == RIBBON_ART_TAB_CTRL_BACKGROUND_COLOUR:
            self._tab_ctrl_background_colour = colour
        elif id == RIBBON_ART_TAB_CTRL_BACKGROUND_GRADIENT_COLOUR:
            self._tab_ctrl_background_gradient_colour = colour
        elif id in [RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_COLOUR, RIBBON_ART_TAB_ACTIVE_BACKGROUND_TOP_GRADIENT_COLOUR]:
            self._tab_active_top_background_brush.SetColour(colour)
        elif id in [RIBBON_ART_TAB_HOVER_BACKGROUND_COLOUR, RIBBON_ART_TAB_HOVER_BACKGROUND_GRADIENT_COLOUR]:
            self._tab_hover_background_brush.SetColour(colour)
        elif id == RIBBON_ART_PANEL_LABEL_BACKGROUND_COLOUR:
            self._panel_label_background_colour = colour
        elif id == RIBBON_ART_PANEL_LABEL_BACKGROUND_GRADIENT_COLOUR:
            self._panel_label_background_gradient_colour = colour
        elif id in [RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_COLOUR, RIBBON_ART_BUTTON_BAR_HOVER_BACKGROUND_GRADIENT_COLOUR]:
            self._button_bar_hover_background_brush.SetColour(colour)
        elif id in [RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_COLOUR, RIBBON_ART_GALLERY_BUTTON_HOVER_BACKGROUND_GRADIENT_COLOUR]:
            self._gallery_button_hover_background_brush.SetColour(colour)
        elif id in [RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_COLOUR, RIBBON_ART_GALLERY_BUTTON_ACTIVE_BACKGROUND_GRADIENT_COLOUR]:
            self._gallery_button_active_background_brush.SetColour(colour)
        elif id in [RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_COLOUR, RIBBON_ART_GALLERY_BUTTON_DISABLED_BACKGROUND_GRADIENT_COLOUR]:
            self._gallery_button_disabled_background_brush.SetColour(colour)
        else:
            RibbonMSWArtProvider.SetColour(self, id, colour)
    

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

        :see: L{SetColour}, L{RibbonMSWArtProvider.GetColourScheme}
        """

        primary_hsl = RibbonHSLColour(primary)
        secondary_hsl = RibbonHSLColour(secondary)
        tertiary_hsl = RibbonHSLColour(tertiary)

        # Map primary & secondary luminance from [0, 1] to [0.15, 0.85]
        primary_hsl.luminance = cos(primary_hsl.luminance * M_PI) * -0.35 + 0.5
        secondary_hsl.luminance = cos(secondary_hsl.luminance * M_PI) * -0.35 + 0.5

        # TODO: Remove next line once this provider stops piggybacking MSW
        RibbonMSWArtProvider.SetColourScheme(self, primary, secondary, tertiary)

        self._tab_ctrl_background_colour = RibbonShiftLuminance(primary_hsl, 0.9).ToRGB()
        self._tab_ctrl_background_gradient_colour = RibbonShiftLuminance(primary_hsl, 1.7).ToRGB()
        self._tab_border_pen = wx.Pen(RibbonShiftLuminance(primary_hsl, 0.75).ToRGB())
        self._tab_label_colour = RibbonShiftLuminance(primary_hsl, 0.1).ToRGB()
        self._tab_hover_background_top_colour =  primary_hsl.ToRGB()
        self._tab_hover_background_top_gradient_colour = RibbonShiftLuminance(primary_hsl, 1.6).ToRGB()
        self._tab_hover_background_brush = wx.Brush(self._tab_hover_background_top_colour)
        self._tab_active_background_colour = self._tab_ctrl_background_gradient_colour
        self._tab_active_background_gradient_colour = primary_hsl.ToRGB()
        self._tab_active_top_background_brush = wx.Brush(self._tab_active_background_colour)
        self._panel_label_colour = self._tab_label_colour
        self._panel_minimised_label_colour = self._panel_label_colour
        self._panel_hover_label_colour = tertiary_hsl.ToRGB()
        self._page_border_pen = self._tab_border_pen
        self._panel_border_pen = self._tab_border_pen
        self._background_brush = wx.Brush(primary_hsl.ToRGB())
        self._page_hover_background_colour = RibbonShiftLuminance(primary_hsl, 1.5).ToRGB()
        self._page_hover_background_gradient_colour = RibbonShiftLuminance(primary_hsl, 0.9).ToRGB()
        self._panel_label_background_colour = RibbonShiftLuminance(primary_hsl, 0.85).ToRGB()
        self._panel_label_background_gradient_colour = RibbonShiftLuminance(primary_hsl, 0.97).ToRGB()
        self._panel_hover_label_background_gradient_colour = secondary_hsl.ToRGB()
        self._panel_hover_label_background_colour = secondary_hsl.Lighter(0.2).ToRGB()
        self._button_bar_hover_border_pen = wx.Pen(secondary_hsl.ToRGB())
        self._button_bar_hover_background_brush = wx.Brush(RibbonShiftLuminance(secondary_hsl, 1.7).ToRGB())
        self._button_bar_active_background_brush = wx.Brush(RibbonShiftLuminance(secondary_hsl, 1.4).ToRGB())
        self._button_bar_label_colour = self._tab_label_colour
        self._gallery_border_pen = self._tab_border_pen
        self._gallery_item_border_pen = self._button_bar_hover_border_pen
        self._gallery_hover_background_brush = wx.Brush(RibbonShiftLuminance(primary_hsl, 1.2).ToRGB())
        self._gallery_button_background_colour = self._page_hover_background_colour
        self._gallery_button_background_gradient_colour = self._page_hover_background_gradient_colour
        self._gallery_button_hover_background_brush = self._button_bar_hover_background_brush
        self._gallery_button_active_background_brush = self._button_bar_active_background_brush
        self._gallery_button_disabled_background_brush = wx.Brush(primary_hsl.Desaturated(0.15).ToRGB())
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_FACE_COLOUR, RibbonShiftLuminance(primary_hsl, 0.1).ToRGB())
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_DISABLED_FACE_COLOUR, wx.Colour(128, 128, 128))
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_ACTIVE_FACE_COLOUR, RibbonShiftLuminance(secondary_hsl, 0.1).ToRGB())
        self.SetColour(RIBBON_ART_GALLERY_BUTTON_HOVER_FACE_COLOUR, RibbonShiftLuminance(secondary_hsl, 0.1).ToRGB())
        self._toolbar_border_pen = self._tab_border_pen
        self.SetColour(RIBBON_ART_TOOLBAR_FACE_COLOUR, RibbonShiftLuminance(primary_hsl, 0.1).ToRGB())
        self._tool_background_colour = self._page_hover_background_colour
        self._tool_background_gradient_colour = self._page_hover_background_gradient_colour
        self._toolbar_hover_borden_pen = self._button_bar_hover_border_pen
        self._tool_hover_background_brush = self._button_bar_hover_background_brush
        self._tool_active_background_brush = self._button_bar_active_background_brush


    def DrawTabCtrlBackground(self, dc, wnd, rect):
        """
        Draw the background of the tab region of a ribbon bar.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw.

        """

        gradient_rect = wx.Rect(*rect)
        gradient_rect.height -= 1
        dc.GradientFillLinear(gradient_rect, self._tab_ctrl_background_colour, self._tab_ctrl_background_gradient_colour, wx.SOUTH)
        dc.SetPen(self._tab_border_pen)
        dc.DrawLine(rect.x, rect.GetBottom(), rect.GetRight()+1, rect.GetBottom())


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
            # one pixel of border though.
            return 1        

        if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS:        
            dc.SetFont(self._tab_active_label_font)
            text_height = dc.GetTextExtent("ABCDEFXj")[1]
        
        if self._flags & RIBBON_BAR_SHOW_PAGE_ICONS:
            for info in pages:
                if info.page.GetIcon().IsOk():                
                    icon_height = max(icon_height, info.page.GetIcon().GetHeight())

        return max(text_height, icon_height) + 10


    def DrawTab(self, dc, wnd, tab):
        """
        Draw a single tab in the tab region of a ribbon bar.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto (not the L{RibbonPage}
         associated with the tab being drawn);
        :param `tab`: The rectangle within which to draw, and also the tab label,
         icon, and state (active and/or hovered). The drawing rectangle will be
         entirely within a rectangle on the same device context previously painted
         with L{DrawTabCtrlBackground}. The rectangle's width will be at least the
         minimum value returned by L{GetBarTabWidth}, and height will be the value
         returned by L{GetTabCtrlHeight}.

        """

        if tab.rect.height <= 1:
            return

        dc.SetFont(self._tab_label_font)
        dc.SetPen(wx.TRANSPARENT_PEN)
        
        if tab.active or tab.hovered:        
            if tab.active:            
                dc.SetFont(self._tab_active_label_font)
                dc.SetBrush(self._background_brush)
                dc.DrawRectangle(tab.rect.x, tab.rect.y + tab.rect.height - 1, tab.rect.width - 1, 1)
            
            grad_rect = wx.Rect(*tab.rect)
            grad_rect.height -= 4
            grad_rect.width -= 1
            grad_rect.height /= 2
            grad_rect.y = grad_rect.y + tab.rect.height - grad_rect.height - 1
            dc.SetBrush(self._tab_active_top_background_brush)
            dc.DrawRectangle(tab.rect.x, tab.rect.y + 3, tab.rect.width - 1, grad_rect.y - tab.rect.y - 3)
            dc.GradientFillLinear(grad_rect, self._tab_active_background_colour, self._tab_active_background_gradient_colour, wx.SOUTH)
        
        else:
        
            btm_rect = wx.Rect(*tab.rect)
            btm_rect.height -= 4
            btm_rect.width -= 1
            btm_rect.height /= 2
            btm_rect.y = btm_rect.y + tab.rect.height - btm_rect.height - 1
            dc.SetBrush(self._tab_hover_background_brush)
            dc.DrawRectangle(btm_rect.x, btm_rect.y, btm_rect.width, btm_rect.height)
            grad_rect = wx.Rect(*tab.rect)
            grad_rect.width -= 1
            grad_rect.y += 3
            grad_rect.height = btm_rect.y - grad_rect.y
            dc.GradientFillLinear(grad_rect, self._tab_hover_background_top_colour, self._tab_hover_background_top_gradient_colour, wx.SOUTH)
        
        border_points = [wx.Point() for i in xrange(5)]
        border_points[0] = wx.Point(0, 3)
        border_points[1] = wx.Point(1, 2)
        border_points[2] = wx.Point(tab.rect.width - 3, 2)
        border_points[3] = wx.Point(tab.rect.width - 1, 4)
        border_points[4] = wx.Point(tab.rect.width - 1, tab.rect.height - 1)

        dc.SetPen(self._tab_border_pen)
        dc.DrawLines(border_points, tab.rect.x, tab.rect.y)

        old_clip = dc.GetClippingRect()
        is_first_tab = False

        bar = tab.page.GetParent()
        icon = wx.NullBitmap
        
        if isinstance(bar, BAR.RibbonBar) and bar.GetPage(0) == tab.page:
            is_first_tab = True

        if self._flags & RIBBON_BAR_SHOW_PAGE_ICONS:        
            icon = tab.page.GetIcon()
            if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS == 0:            
                x = tab.rect.x + (tab.rect.width - icon.GetWidth()) / 2
                dc.DrawBitmap(icon, x, tab.rect.y + 1 + (tab.rect.height - 1 - icon.GetHeight()) / 2, True)
            
        if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS:
            label = tab.page.GetLabel()
            
            if label.strip():            
                dc.SetTextForeground(self._tab_label_colour)
                dc.SetBackgroundMode(wx.TRANSPARENT)

                offset = 0
                
                if icon.IsOk():
                    offset += icon.GetWidth() + 2

                text_width, text_height = dc.GetTextExtent(label)
                x = (tab.rect.width - 2 - text_width - offset) / 2
                if x > 8:
                    x = 8
                elif x < 1:
                    x = 1
                    
                width = tab.rect.width - x - 2
                x += tab.rect.x + offset
                y = tab.rect.y + (tab.rect.height - text_height) / 2
                
                if icon.IsOk():                
                    dc.DrawBitmap(icon, x - offset, tab.rect.y + (tab.rect.height - icon.GetHeight()) / 2, True)
                
                dc.SetClippingRegion(x, tab.rect.y, width, tab.rect.height)
                dc.DrawText(label, x, y)
            
        # Draw the left hand edge of the tab only for the first tab (subsequent
        # tabs use the right edge of the prior tab as their left edge). As this is
        # outside the rectangle for the tab, only draw it if the leftmost part of
        # the tab is within the clip rectangle (the clip region has to be cleared
        # to draw outside the tab).
        if is_first_tab and old_clip.x <= tab.rect.x and tab.rect.x < old_clip.x + old_clip.width:        
            dc.DestroyClippingRegion()
            dc.DrawLine(tab.rect.x - 1, tab.rect.y + 4, tab.rect.x - 1, tab.rect.y + tab.rect.height - 1)
    

    def GetBarTabWidth(self, dc, wnd, label, bitmap, ideal=None, small_begin_need_separator=None,
                       small_must_have_separator=None, minimum=None):
        """
        Calculate the ideal and minimum width (in pixels) of a tab in a ribbon bar.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the tab will eventually be drawn;
        :param `label`: The tab's label (or wx.EmptyString if it has none);
        :param `bitmap`: The tab's icon (or wx.NullBitmap if it has none);
        :param `ideal`: The ideal width (in pixels) of the tab;
        :param `small_begin_need_separator`: A size less than the size, at which a tab
         separator should begin to be drawn (i.e. drawn, but still fairly transparent);
        :param `small_must_have_separator`: A size less than the size, at which a tab
         separator must be drawn (i.e. drawn at full opacity);
        :param `minimum`: A size less than the size, and greater than or equal to zero,
         which is the minimum pixel width for the tab.

        """

        width = mini = 0
        
        if self._flags & RIBBON_BAR_SHOW_PAGE_LABELS and label.strip():        
            dc.SetFont(self._tab_active_label_font)
            width += dc.GetTextExtent(label)[0]
            mini += min(30, width) # enough for a few chars
            
            if bitmap.IsOk():            
                # gap between label and bitmap
                width += 4
                mini += 2
            
        if self._flags & RIBBON_BAR_SHOW_PAGE_ICONS and bitmap.IsOk():
            width += bitmap.GetWidth()
            mini += bitmap.GetWidth()

        ideal = width + 16        
        small_begin_need_separator = mini        
        small_must_have_separator = mini        
        minimum = mini

        return ideal, small_begin_need_separator, small_must_have_separator, minimum


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

        # No explicit separators between tabs
        pass


    def DrawPageBackground(self, dc, wnd, rect):
        """
        Draw the background of a ribbon page.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto (which is commonly the
         L{RibbonPage} whose background is being drawn, but doesn't have to be);
        :param `rect`: The rectangle within which to draw.

        :see: L{RibbonMSWArtProvider.GetPageBackgroundRedrawArea}
        """

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._background_brush)
        dc.DrawRectangle(rect.x + 1, rect.y, rect.width - 2, rect.height - 1)

        dc.SetPen(self._page_border_pen)
        dc.DrawLine(rect.x, rect.y, rect.x, rect.y + rect.height)
        dc.DrawLine(rect.GetRight(), rect.y, rect.GetRight(), rect.y +rect.height)
        dc.DrawLine(rect.x, rect.GetBottom(), rect.GetRight()+1, rect.GetBottom())


    def GetScrollButtonMinimumSize(self, dc, wnd, style):
        """
        Calculate the minimum size (in pixels) of a scroll button.

        :param `dc`: A device context to use when one is required for size calculations;
        :param `wnd`: The window onto which the scroll button will eventually be drawn;
        :param `style`: A combination of flags from `RibbonScrollButtonStyle`, including
         a direction, and a for flag (state flags may be given too, but should be ignored,
         as a button should retain a constant size, regardless of its state).

        """

        return wx.Size(11, 11)


    def DrawScrollButton(self, dc, wnd, rect, style):
        """
        Draw a ribbon-style scroll button.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will be at least the size returned by L{GetScrollButtonMinimumSize} for a
         scroll button with the same style. For tab scroll buttons, this rectangle
         will be entirely within a rectangle on the same device context previously
         painted with L{DrawTabCtrlBackground}, but this is not guaranteed for other
         types of button (for example, page scroll buttons will not be painted on
         an area previously painted with L{DrawPageBackground});
        :param `style`: A combination of flags from `RibbonScrollButtonStyle`,
         including a direction, a for flag, and one or more states.

        """

        true_rect = wx.Rect(*rect)
        arrow_points = [wx.Point() for i in xrange(3)]

        if style & RIBBON_SCROLL_BTN_FOR_MASK == RIBBON_SCROLL_BTN_FOR_TABS:        
            true_rect.y += 2
            true_rect.height -= 2
            dc.SetPen(self._tab_border_pen)
        else:
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(self._background_brush)
            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)
            dc.SetPen(self._page_border_pen)

        result = style & RIBBON_SCROLL_BTN_DIRECTION_MASK
        
        if result == RIBBON_SCROLL_BTN_LEFT:
            dc.DrawLine(true_rect.GetRight(), true_rect.y, true_rect.GetRight(), true_rect.y + true_rect.height)
            arrow_points[0] = wx.Point(rect.width / 2 - 2, rect.height / 2)
            arrow_points[1] = arrow_points[0] + wx.Point(5, -5)
            arrow_points[2] = arrow_points[0] + wx.Point(5,  5)

        elif result == RIBBON_SCROLL_BTN_RIGHT:
            dc.DrawLine(true_rect.x, true_rect.y, true_rect.x, true_rect.y + true_rect.height)
            arrow_points[0] = wx.Point(rect.width / 2 + 3, rect.height / 2)
            arrow_points[1] = arrow_points[0] - wx.Point(5, -5)
            arrow_points[2] = arrow_points[0] - wx.Point(5,  5)

        elif result == RIBBON_SCROLL_BTN_DOWN:
            dc.DrawLine(true_rect.x, true_rect.y, true_rect.x + true_rect.width, true_rect.y)
            arrow_points[0] = wx.Point(rect.width / 2, rect.height / 2 + 3)
            arrow_points[1] = arrow_points[0] - wx.Point( 5, 5)
            arrow_points[2] = arrow_points[0] - wx.Point(-5, 5)

        elif result == RIBBON_SCROLL_BTN_UP:
            dc.DrawLine(true_rect.x, true_rect.GetBottom(), true_rect.x + true_rect.width, true_rect.GetBottom())
            arrow_points[0] = wx.Point(rect.width / 2, rect.height / 2 - 2)
            arrow_points[1] = arrow_points[0] + wx.Point( 5, 5)
            arrow_points[2] = arrow_points[0] + wx.Point(-5, 5)

        else:
            return
        
        x = rect.x
        y = rect.y

        if style & RIBBON_SCROLL_BTN_ACTIVE:        
            x += 1
            y += 1

        dc.SetPen(wx.TRANSPARENT_PEN)
        B = wx.Brush(self._tab_label_colour)
        dc.SetBrush(B)
        dc.DrawPolygon(arrow_points, x, y)


    def GetPanelSize(self, dc, wnd, client_size, client_offset=None):
        """
        Calculate the size of a panel for a given client size.

        This should increment the given size by enough to fit the panel label and other
        chrome.

        :param `dc`: A device context to use if one is required for size calculations;
        :param `wnd`: The ribbon panel in question;
        :param `client_size`: The client size;
        :param `client_offset`: The offset where the client rectangle begins within
         the panel (may be ``None``).

        :see: L{GetPanelClientSize}
        """

        dc.SetFont(self._panel_label_font)
        label_size = wx.Size(*dc.GetTextExtent(wnd.GetLabel()))
        label_height = label_size.GetHeight() + 5
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            client_size.IncBy(4, label_height + 6)
            if client_offset is not None:
                client_offset = wx.Point(2, label_height + 3)
        
        else:        
            client_size.IncBy(6, label_height + 4)
            if client_offset is not None:
                client_offset = wx.Point(3, label_height + 2)
        
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
        label_height = label_size.GetHeight() + 5

        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            size.DecBy(4, label_height + 6)
            if client_offset is not None:
                client_offset = wx.Point(2, label_height + 3)
        
        else:        
            size.DecBy(6, label_height + 4)
            if client_offset is not None:
                client_offset = wx.Point(3, label_height + 2)
        
        return size, client_offset


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

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._background_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        true_rect = wx.Rect(*rect)
        true_rect = self.RemovePanelPadding(true_rect)

        dc.SetPen(self._panel_border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRectangle(true_rect.x, true_rect.y, true_rect.width, true_rect.height)

        true_rect.x += 1
        true_rect.width -= 2
        true_rect.y += 1

        dc.SetFont(self._panel_label_font)
        label_size = wx.Size(*dc.GetTextExtent(wnd.GetLabel()))
        label_height = label_size.GetHeight() + 5
        label_rect = wx.Rect(*true_rect)
        label_rect.height = label_height - 1
        
        dc.DrawLine(label_rect.x, label_rect.y + label_rect.height, label_rect.x + label_rect.width, label_rect.y + label_rect.height)

        label_bg_colour = self._panel_label_background_colour
        label_bg_grad_colour = self._panel_label_background_gradient_colour
        
        if wnd.IsHovered():        
            label_bg_colour = self._panel_hover_label_background_colour
            label_bg_grad_colour = self._panel_hover_label_background_gradient_colour
            dc.SetTextForeground(self._panel_hover_label_colour)
        else:        
            dc.SetTextForeground(self._panel_label_colour)

        if wx.Platform == "__WXMAC__":
            dc.GradientFillLinear(label_rect, label_bg_grad_colour, label_bg_colour, wx.SOUTH)
        else:
            dc.GradientFillLinear(label_rect, label_bg_colour, label_bg_grad_colour, wx.SOUTH)

        dc.SetFont(self._panel_label_font)
        dc.DrawText(wnd.GetLabel(), label_rect.x + 3, label_rect.y + 2)

        if wnd.IsHovered():
        
            gradient_rect = wx.Rect(*true_rect)
            gradient_rect.y += label_rect.height + 1
            gradient_rect.height = true_rect.height - label_rect.height - 3
            if wx.Platform == "__WXMAC__":
                colour = self._page_hover_background_gradient_colour
                gradient = self._page_hover_background_colour
            else:
                colour = self._page_hover_background_colour
                gradient = self._page_hover_background_gradient_colour

            dc.GradientFillLinear(gradient_rect, colour, gradient, wx.SOUTH)
        

    def DrawMinimisedPanel(self, dc, wnd, rect, bitmap):
        """
        Draw a minimised ribbon panel.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto, which is always the panel
         which is minimised. The panel label can be obtained from this window. The
         minimised icon obtained from querying the window may not be the size requested
         by L{RibbonMSWArtProvider.GetMinimisedPanelMinimumSize} - the argument contains the icon in the
         requested size;
        :param `rect`: The rectangle within which to draw. The size of the rectangle
         will be at least the size returned by L{RibbonMSWArtProvider.GetMinimisedPanelMinimumSize};
        :param `bitmap`: A copy of the panel's minimised bitmap rescaled to the size
         returned by L{RibbonMSWArtProvider.GetMinimisedPanelMinimumSize}.

        """

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._background_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        true_rect = wx.Rect(*rect)
        true_rect = self.RemovePanelPadding(true_rect)

        dc.SetPen(self._panel_border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRectangle(true_rect.x, true_rect.y, true_rect.width, true_rect.height)
        true_rect.Deflate(1, 1)

        if wnd.IsHovered() or wnd.GetExpandedPanel():
            colour = self._page_hover_background_colour
            gradient = self._page_hover_background_gradient_colour
            if (wx.Platform == "__WXMAC__" and not wnd.GetExpandedPanel()) or \
               (wx.Platform != "__WXMAC__" and wnd.GetExpandedPanel()):
                temp = colour
                colour = gradient
                gradient = temp
            
            dc.GradientFillLinear(true_rect, colour, gradient, wx.SOUTH)
        
        preview = self.DrawMinimisedPanelCommon(dc, wnd, true_rect)

        dc.SetPen(self._panel_border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRectangle(preview.x, preview.y, preview.width, preview.height)
        preview.Deflate(1, 1)

        preview_caption_rect = wx.Rect(*preview)
        preview_caption_rect.height = 7
        preview.y += preview_caption_rect.height
        preview.height -= preview_caption_rect.height
        
        if wx.Platform == "__WXMAC__":
            dc.GradientFillLinear(preview_caption_rect, self._panel_hover_label_background_gradient_colour,
                                  self._panel_hover_label_background_colour, wx.SOUTH)
            dc.GradientFillLinear(preview, self._page_hover_background_gradient_colour,
                                  self._page_hover_background_colour, wx.SOUTH)
        else:
            dc.GradientFillLinear(preview_caption_rect, self._panel_hover_label_background_colour,
                                  self._panel_hover_label_background_gradient_colour, wx.SOUTH)
            dc.GradientFillLinear(preview, self._page_hover_background_colour,
                                  self._page_hover_background_gradient_colour, wx.SOUTH)

        if bitmap.IsOk():        
            dc.DrawBitmap(bitmap, preview.x + (preview.width - bitmap.GetWidth()) / 2,
                          preview.y + (preview.height - bitmap.GetHeight()) / 2, True)
        

    def DrawPartialPanelBackground(self, dc, wnd, rect):

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(self._background_brush)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        offset = wx.Point(*wnd.GetPosition())
        parent = wnd.GetParent()
        panel = None

        while 1:
            panel = parent
            if isinstance(panel, PANEL.RibbonPanel):
                if not panel.IsHovered():
                    return
                break
            
            offset += parent.GetPosition()
            parent = panel.GetParent()
        
        if panel is None:
            return

        background = wx.Rect(0, 0, *panel.GetSize())
        background = self.RemovePanelPadding(background)
        
        background.x += 1
        background.width -= 2
        dc.SetFont(self._panel_label_font)
        caption_height = dc.GetTextExtent(panel.GetLabel())[1] + 7
        background.y += caption_height - 1
        background.height -= caption_height

        paint_rect = wx.Rect(*rect)
        paint_rect.x += offset.x
        paint_rect.y += offset.y

        if wx.Platform == "__WXMAC__":
            bg_grad_clr = self._page_hover_background_colour
            bg_clr = self._page_hover_background_gradient_colour
        else:
            bg_clr = self._page_hover_background_colour
            bg_grad_clr = self._page_hover_background_gradient_colour

        paint_rect.Intersect(background)
        
        if not paint_rect.IsEmpty():
            starting_colour = RibbonInterpolateColour(bg_clr, bg_grad_clr, paint_rect.y, background.y, background.y + background.height)
            ending_colour = RibbonInterpolateColour(bg_clr, bg_grad_clr, paint_rect.y + paint_rect.height, background.y, background.y + background.height)
            paint_rect.x -= offset.x
            paint_rect.y -= offset.y
            dc.GradientFillLinear(paint_rect, starting_colour, ending_colour, wx.SOUTH)

    
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

        self.DrawPartialPanelBackground(dc, wnd, rect)

        if wnd.IsHovered():        
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(self._gallery_hover_background_brush)
            
            if self._flags & RIBBON_BAR_FLOW_VERTICAL:            
                dc.DrawRectangle(rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 16)            
            else:            
                dc.DrawRectangle(rect.x + 1, rect.y + 1, rect.width - 16, rect.height - 2)
            
        dc.SetPen(self._gallery_border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)
        
        self.DrawGalleryBackgroundCommon(dc, wnd, rect)


    def DrawGalleryButton(self, dc, rect, state, bitmaps):

        extra_height = 0
        extra_width = 0
        reduced_rect = wx.Rect(*rect)
        reduced_rect.Deflate(1, 1)
        
        if self._flags & RIBBON_BAR_FLOW_VERTICAL:        
            reduced_rect.width += 1
            extra_width = 1
        else:        
            reduced_rect.height += 1
            extra_height = 1
        
        if state == RIBBON_GALLERY_BUTTON_NORMAL:
            dc.GradientFillLinear(reduced_rect, self._gallery_button_background_colour, self._gallery_button_background_gradient_colour, wx.SOUTH)
            btn_bitmap = bitmaps[0]

        elif state == RIBBON_GALLERY_BUTTON_HOVERED:
            dc.SetPen(self._gallery_item_border_pen)
            dc.SetBrush(self._gallery_button_hover_background_brush)
            dc.DrawRectangle(rect.x, rect.y, rect.width + extra_width, rect.height + extra_height)
            btn_bitmap = bitmaps[1]

        elif state == RIBBON_GALLERY_BUTTON_ACTIVE:
            dc.SetPen(self._gallery_item_border_pen)
            dc.SetBrush(self._gallery_button_active_background_brush)
            dc.DrawRectangle(rect.x, rect.y, rect.width + extra_width, rect.height + extra_height)
            btn_bitmap = bitmaps[2]

        elif state == RIBBON_GALLERY_BUTTON_DISABLED:
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(self._gallery_button_disabled_background_brush)
            dc.DrawRectangle(reduced_rect.x, reduced_rect.y, reduced_rect.width, reduced_rect.height)
            btn_bitmap = bitmaps[3]        

        dc.DrawBitmap(btn_bitmap, reduced_rect.x + reduced_rect.width / 2 - 2, (rect.y + rect.height / 2) - 2, True)


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
        :param `item`: The item whose background is being painted. Typically the
         background will vary if the item is hovered, active, or selected;
         L{RibbonGallery.GetSelection}, L{RibbonGallery.GetActiveItem}, and
         L{RibbonGallery.GetHoveredItem} can be called to test if the given item is in one of these states.

        """

        if wnd.GetHoveredItem() != item and wnd.GetActiveItem() != item and wnd.GetSelection() != item:
            return

        dc.SetPen(self._gallery_item_border_pen)
        
        if wnd.GetActiveItem() == item or wnd.GetSelection() == item:
            dc.SetBrush(self._gallery_button_active_background_brush)
        else:
            dc.SetBrush(self._gallery_button_hover_background_brush)

        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)


    def DrawButtonBarBackground(self, dc, wnd, rect):
        """
        Draw the background for a L{bar.RibbonButtonBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto (which will typically
         be the button bar itself, though this is not guaranteed);
        :param `rect`: The rectangle within which to draw.

        """

        self.DrawPartialPanelBackground(dc, wnd, rect)


    def DrawButtonBarButton(self, dc, wnd, rect, kind, state, label, bitmap_large, bitmap_small):
        """
        Draw a single button for a L{bar.RibbonButtonBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will be a size previously returned by L{RibbonMSWArtProvider.GetButtonBarButtonSize}, and the
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
            dc.SetPen(self._button_bar_hover_border_pen)
            bg_rect = wx.Rect(*rect)
            bg_rect.Deflate(1, 1)

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

                elif result == RIBBON_BUTTONBAR_BUTTON_MEDIUM:                    
                    iArrowWidth = 9
                    
                    if state & RIBBON_BUTTONBAR_BUTTON_NORMAL_HOVERED:                    
                        bg_rect.width -= iArrowWidth
                        dc.DrawLine(bg_rect.x + bg_rect.width, rect.y, bg_rect.x + bg_rect.width, rect.y + rect.height)                    
                    else:                   
                        iArrowWidth -= 1
                        bg_rect.x += bg_rect.width - iArrowWidth
                        bg_rect.width = iArrowWidth
                        dc.DrawLine(bg_rect.x - 1, rect.y, bg_rect.x - 1, rect.y + rect.height)

            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

            dc.SetPen(wx.TRANSPARENT_PEN)
            
            if state & RIBBON_BUTTONBAR_BUTTON_ACTIVE_MASK:
                dc.SetBrush(self._button_bar_active_background_brush)
            else:
                dc.SetBrush(self._button_bar_hover_background_brush)
                
            dc.DrawRectangle(bg_rect.x, bg_rect.y, bg_rect.width, bg_rect.height)
        
        dc.SetFont(self._button_bar_label_font)
        dc.SetTextForeground(self._button_bar_label_colour)
        self.DrawButtonBarButtonForeground(dc, rect, kind, state, label, bitmap_large, bitmap_small)


    def DrawToolBarBackground(self, dc, wnd, rect):
        """
        Draw the background for a L{RibbonToolBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The which is being drawn onto. In most cases this will be
         a L{RibbonToolBar}, but it doesn't have to be;
        :param `rect`: The rectangle within which to draw. Some of this rectangle
         will later be drawn over using L{DrawToolGroupBackground} and L{DrawTool},
         but not all of it will (unless there is only a single group of tools).

        """

        self.DrawPartialPanelBackground(dc, wnd, rect)


    def DrawToolGroupBackground(self, dc, wnd, rect):
        """
        Draw the background for a group of tools on a L{RibbonToolBar} control.

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto. In most cases this will
         be a L{RibbonToolBar}, but it doesn't have to be;
        :param `rect`: The rectangle within which to draw. This rectangle is a union
         of the individual tools' rectangles. As there are no gaps between tools, this
         rectangle will be painted over exactly once by calls to L{DrawTool}. The
         group background could therefore be painted by L{DrawTool}, though it can be
         conceptually easier and more efficient to draw it all at once here. The
         rectangle will be entirely within a rectangle on the same device context
         previously painted with L{DrawToolBarBackground}.

        """

        dc.SetPen(self._toolbar_border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)
        bg_rect = wx.Rect(*rect)
        bg_rect.Deflate(1, 1)
        dc.GradientFillLinear(bg_rect, self._tool_background_colour, self._tool_background_gradient_colour, wx.SOUTH)


    def DrawTool(self, dc, wnd, rect, bitmap, kind, state):
        """
        Draw a single tool (for a L{RibbonToolBar} control).

        :param `dc`: The device context to draw onto;
        :param `wnd`: The window which is being drawn onto. In most cases this will
         be a L{RibbonToolBar}, but it doesn't have to be;
        :param `rect`: The rectangle within which to draw. The size of this rectangle
         will at least the size returned by L{RibbonMSWArtProvider.GetToolSize}, and the height of it will
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
            
        is_custom_bg = (state & (RIBBON_TOOLBAR_TOOL_HOVER_MASK | RIBBON_TOOLBAR_TOOL_ACTIVE_MASK)) != 0
        is_split_hybrid = kind == RIBBON_BUTTON_HYBRID and is_custom_bg

        # Background
        if is_custom_bg:        
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(self._tool_hover_background_brush)
            dc.DrawRectangle(bg_rect.x, bg_rect.y, bg_rect.width, bg_rect.height)
            
            if state & RIBBON_TOOLBAR_TOOL_ACTIVE_MASK:            
                active_rect = wx.Rect(*bg_rect)
                
                if kind == RIBBON_BUTTON_HYBRID:                
                    active_rect.width -= 8
                    
                    if state & RIBBON_TOOLBAR_TOOL_DROPDOWN_ACTIVE:                    
                        active_rect.x += active_rect.width
                        active_rect.width = 8
                    
                dc.SetBrush(self._tool_active_background_brush)
                dc.DrawRectangle(active_rect.x, active_rect.y, active_rect.width, active_rect.height)

        # Border
        if is_custom_bg:
            dc.SetPen(self._toolbar_hover_borden_pen)
        else:
            dc.SetPen(self._toolbar_border_pen)
            
        if state & RIBBON_TOOLBAR_TOOL_FIRST == 0:
            existing = dc.GetPixel(rect.x, rect.y + 1)
            
            if existing == wx.NullColour or existing != self._toolbar_hover_borden_pen.GetColour():            
                dc.DrawLine(rect.x, rect.y + 1, rect.x, rect.y + rect.height - 1)
                    
        if is_custom_bg:
            border_rect = wx.Rect(*bg_rect)
            border_rect.Inflate(1, 1)
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.DrawRectangle(border_rect.x, border_rect.y, border_rect.width, border_rect.height)
        
        # Foreground
        avail_width = bg_rect.GetWidth()

        if kind != RIBBON_BUTTON_NORMAL:        
            avail_width -= 8
            if is_split_hybrid:            
                dc.DrawLine(rect.x + avail_width + 1, rect.y, rect.x + avail_width + 1, rect.y + rect.height)
            
            dc.DrawBitmap(self._toolbar_drop_bitmap, bg_rect.x + avail_width + 2, bg_rect.y + (bg_rect.height / 2) - 2, True)
        
        dc.DrawBitmap(bitmap, bg_rect.x + (avail_width - bitmap.GetWidth()) / 2, bg_rect.y + (bg_rect.height - bitmap.GetHeight()) / 2, True)

