"""
Tab art provider code - a tab provider provides all drawing functionality to
the L{AuiNotebook}. This allows the L{AuiNotebook} to have a plugable look-and-feel.

By default, a L{AuiNotebook} uses an instance of this class called L{AuiDefaultTabArt}
which provides bitmap art and a colour scheme that is adapted to the major platforms'
look. You can either derive from that class to alter its behaviour or write a
completely new tab art class. Call L{AuiNotebook.SetArtProvider} to make use this
new tab art.
"""

__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx

if wx.Platform == '__WXMAC__':
    import Carbon.Appearance

from aui_utilities import BitmapFromBits, StepColour, IndentPressedBitmap, ChopText
from aui_utilities import GetBaseColour, DrawMACCloseButton, LightColour, TakeScreenShot
from aui_utilities import CopyAttributes

from aui_constants import *


# -- GUI helper classes and functions --
class AuiCommandCapture(wx.PyEvtHandler):
    """ A class to handle the dropdown window menu. """

    def __init__(self):
        """ Default class constructor. """

        wx.PyEvtHandler.__init__(self)        
        self._last_id = 0


    def GetCommandId(self):
        """ Returns the event command identifier. """

        return self._last_id 


    def ProcessEvent(self, event):
        """
        Processes an event, searching event tables and calling zero or more suitable
        event handler function(s).

        :param `event`: the event to process.

        :note: Normally, your application would not call this function: it is called
         in the wxPython implementation to dispatch incoming user interface events
         to the framework (and application).
         However, you might need to call it if implementing new functionality (such as
         a new control) where you define new event types, as opposed to allowing the
         user to override functions.

         An instance where you might actually override the L{ProcessEvent} function is where
         you want to direct event processing to event handlers not normally noticed by
         wxPython. For example, in the document/view architecture, documents and views
         are potential event handlers. When an event reaches a frame, L{ProcessEvent} will
         need to be called on the associated document and view in case event handler
         functions are associated with these objects. 

         The normal order of event table searching is as follows:

         1. If the object is disabled (via a call to `SetEvtHandlerEnabled`) the function
            skips to step (6).
         2. If the object is a `wx.Window`, L{ProcessEvent} is recursively called on the window's 
            `wx.Validator`. If this returns ``True``, the function exits.
         3. wxWidgets `SearchEventTable` is called for this event handler. If this fails, the
            base class table is tried, and so on until no more tables exist or an appropriate
            function was found, in which case the function exits.
         4. The search is applied down the entire chain of event handlers (usually the chain
            has a length of one). If this succeeds, the function exits.
         5. If the object is a `wx.Window` and the event is a `wx.CommandEvent`, L{ProcessEvent} is
            recursively applied to the parent window's event handler. If this returns ``True``,
            the function exits.
         6. Finally, L{ProcessEvent} is called on the `wx.App` object.
        """
        
        if event.GetEventType() == wx.wxEVT_COMMAND_MENU_SELECTED:
            self._last_id = event.GetId()
            return True
        
        if self.GetNextHandler():
            return self.GetNextHandler().ProcessEvent(event)

        return False
    

class AuiDefaultTabArt(object):
    """
    Tab art provider code - a tab provider provides all drawing functionality to
    the L{AuiNotebook}. This allows the L{AuiNotebook} to have a plugable look-and-feel.

    By default, a L{AuiNotebook} uses an instance of this class called L{AuiDefaultTabArt}
    which provides bitmap art and a colour scheme that is adapted to the major platforms'
    look. You can either derive from that class to alter its behaviour or write a
    completely new tab art class. Call L{AuiNotebook.SetArtProvider} to make use this
    new tab art.
    """
    
    def __init__(self):
        """ Default class constructor. """

        self._normal_font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self._selected_font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self._selected_font.SetWeight(wx.BOLD)
        self._measuring_font = self._selected_font

        self._fixed_tab_width = 100
        self._tab_ctrl_height = 0
        self._buttonRect = wx.Rect()

        self.SetDefaultColours()

        if wx.Platform == "__WXMAC__":
            bmp_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DDKSHADOW)
            self._active_close_bmp = DrawMACCloseButton(bmp_colour)
            self._disabled_close_bmp = DrawMACCloseButton(wx.Colour(128, 128, 128))
        else:
            self._active_close_bmp = BitmapFromBits(nb_close_bits, 16, 16, wx.BLACK)
            self._disabled_close_bmp = BitmapFromBits(nb_close_bits, 16, 16, wx.Colour(128, 128, 128))

        self._hover_close_bmp = self._active_close_bmp
        self._pressed_close_bmp = self._active_close_bmp

        self._active_left_bmp = BitmapFromBits(nb_left_bits, 16, 16, wx.BLACK)
        self._disabled_left_bmp = BitmapFromBits(nb_left_bits, 16, 16, wx.Colour(128, 128, 128))

        self._active_right_bmp = BitmapFromBits(nb_right_bits, 16, 16, wx.BLACK)
        self._disabled_right_bmp = BitmapFromBits(nb_right_bits, 16, 16, wx.Colour(128, 128, 128))

        self._active_windowlist_bmp = BitmapFromBits(nb_list_bits, 16, 16, wx.BLACK)
        self._disabled_windowlist_bmp = BitmapFromBits(nb_list_bits, 16, 16, wx.Colour(128, 128, 128))

        if wx.Platform == "__WXMAC__":
            # Get proper highlight colour for focus rectangle from the
            # current Mac theme.  kThemeBrushFocusHighlight is
            # available on Mac OS 8.5 and higher
            if hasattr(wx, 'MacThemeColour'):
                c = wx.MacThemeColour(Carbon.Appearance.kThemeBrushFocusHighlight)
            else:
                brush = wx.Brush(wx.BLACK)
                brush.MacSetTheme(Carbon.Appearance.kThemeBrushFocusHighlight)
                c = brush.GetColour()
            self._focusPen = wx.Pen(c, 2, wx.SOLID)
        else:
            self._focusPen = wx.Pen(wx.BLACK, 1, wx.USER_DASH)
            self._focusPen.SetDashes([1, 1])
            self._focusPen.SetCap(wx.CAP_BUTT)
            
            
    def SetBaseColour(self, base_colour):
        """
        Sets a new base colour.

        :param `base_colour`: an instance of `wx.Colour`.
        """
        
        self._base_colour = base_colour
        self._base_colour_pen = wx.Pen(self._base_colour)
        self._base_colour_brush = wx.Brush(self._base_colour)


    def SetDefaultColours(self, base_colour=None):
        """
        Sets the default colours, which are calculated from the given base colour.

        :param `base_colour`: an instance of `wx.Colour`. If defaulted to ``None``, a colour
         is generated accordingly to the platform and theme.
        """

        if base_colour is None:
            base_colour = GetBaseColour()

        self.SetBaseColour( base_colour )
        self._border_colour = StepColour(base_colour, 75)
        self._border_pen = wx.Pen(self._border_colour)

        self._background_top_colour = StepColour(self._base_colour, 90)
        self._background_bottom_colour = StepColour(self._base_colour, 170)
        
        self._tab_top_colour = self._base_colour
        self._tab_bottom_colour = wx.WHITE
        self._tab_gradient_highlight_colour = wx.WHITE

        self._tab_inactive_top_colour = self._base_colour
        self._tab_inactive_bottom_colour = StepColour(self._tab_inactive_top_colour, 160)
        
        self._tab_text_colour = lambda page: page.text_colour
        self._tab_disabled_text_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT)


    def Clone(self):
        """ Clones the art object. """

        art = type(self)()
        art.SetNormalFont(self.GetNormalFont())
        art.SetSelectedFont(self.GetSelectedFont())
        art.SetMeasuringFont(self.GetMeasuringFont())

        art = CopyAttributes(art, self)
        return art


    def SetAGWFlags(self, agwFlags):
        """
        Sets the tab art flags.

        :param `agwFlags`: a combination of the following values:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet.
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet.
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by L{AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ``AUI_NB_ORDER_BY_ACCESS``           Tab navigation order by last access time for the tabs
         ``AUI_NB_NO_TAB_FOCUS``              Don't draw tab focus rectangle
         ==================================== ==================================
        
        """

        self._agwFlags = agwFlags


    def GetAGWFlags(self):
        """
        Returns the tab art flags.

        :see: L{SetAGWFlags} for a list of possible return values.
        """

        return self._agwFlags
    
            
    def SetSizingInfo(self, tab_ctrl_size, tab_count, minMaxTabWidth):
        """
        Sets the tab sizing information.
        
        :param `tab_ctrl_size`: the size of the tab control area;
        :param `tab_count`: the number of tabs;
        :param `minMaxTabWidth`: a tuple containing the minimum and maximum tab widths
         to be used when the ``AUI_NB_TAB_FIXED_WIDTH`` style is active.
        """
        
        self._fixed_tab_width = 100
        minTabWidth, maxTabWidth = minMaxTabWidth

        tot_width = tab_ctrl_size.x - self.GetIndentSize() - 4
        agwFlags = self.GetAGWFlags()
        
        if agwFlags & AUI_NB_CLOSE_BUTTON:
            tot_width -= self._active_close_bmp.GetWidth()
        if agwFlags & AUI_NB_WINDOWLIST_BUTTON:
            tot_width -= self._active_windowlist_bmp.GetWidth()

        if tab_count > 0:
            self._fixed_tab_width = tot_width/tab_count

        if self._fixed_tab_width < 100:
            self._fixed_tab_width = 100

        if self._fixed_tab_width > tot_width/2:
            self._fixed_tab_width = tot_width/2

        if self._fixed_tab_width > 220:
            self._fixed_tab_width = 220

        if minTabWidth > -1:
            self._fixed_tab_width = max(self._fixed_tab_width, minTabWidth)
        if maxTabWidth > -1:
            self._fixed_tab_width = min(self._fixed_tab_width, maxTabWidth)

        self._tab_ctrl_height = tab_ctrl_size.y
    

    def DrawBackground(self, dc, wnd, rect):
        """
        Draws the tab area background.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `rect`: the tab control rectangle.
        """

        self._buttonRect = wx.Rect()

        # draw background
        agwFlags = self.GetAGWFlags()
        if agwFlags & AUI_NB_BOTTOM:
            r = wx.Rect(rect.x, rect.y, rect.width+2, rect.height)

        # TODO: else if (agwFlags & AUI_NB_LEFT) 
        # TODO: else if (agwFlags & AUI_NB_RIGHT) 
        else: #for AUI_NB_TOP
            r = wx.Rect(rect.x, rect.y, rect.width+2, rect.height-3)

        dc.GradientFillLinear(r, self._background_top_colour, self._background_bottom_colour, wx.SOUTH)

        # draw base lines

        dc.SetPen(self._border_pen)
        y = rect.GetHeight()
        w = rect.GetWidth()

        if agwFlags & AUI_NB_BOTTOM:
            dc.SetBrush(wx.Brush(self._background_bottom_colour))
            dc.DrawRectangle(-1, 0, w+2, 4)

        # TODO: else if (agwFlags & AUI_NB_LEFT) 
        # TODO: else if (agwFlags & AUI_NB_RIGHT)
        
        else: # for AUI_NB_TOP
            dc.SetBrush(self._base_colour_brush)
            dc.DrawRectangle(-1, y-4, w+2, 4)


    def DrawTab(self, dc, wnd, page, in_rect, close_button_state, paint_control=False):
        """
        Draws a single tab.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `page`: the tab control page associated with the tab;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `close_button_state`: the state of the close button on the tab;
        :param `paint_control`: whether to draw the control inside a tab (if any) on a `wx.MemoryDC`.
        """

        # if the caption is empty, measure some temporary text
        caption = page.caption
        if not caption:
            caption = "Xj"

        dc.SetFont(self._selected_font)
        selected_textx, selected_texty, dummy = dc.GetMultiLineTextExtent(caption)

        dc.SetFont(self._normal_font)
        normal_textx, normal_texty, dummy = dc.GetMultiLineTextExtent(caption)

        control = page.control

        # figure out the size of the tab
        tab_size, x_extent = self.GetTabSize(dc, wnd, page.caption, page.bitmap,
                                             page.active, close_button_state, control)

        tab_height = self._tab_ctrl_height - 3
        tab_width = tab_size[0]
        tab_x = in_rect.x
        tab_y = in_rect.y + in_rect.height - tab_height

        caption = page.caption

        # select pen, brush and font for the tab to be drawn

        if page.active:
        
            dc.SetFont(self._selected_font)
            textx, texty = selected_textx, selected_texty
        
        else:
        
            dc.SetFont(self._normal_font)
            textx, texty = normal_textx, normal_texty

        if not page.enabled:
            dc.SetTextForeground(self._tab_disabled_text_colour)
            pagebitmap = page.dis_bitmap
        else:
            dc.SetTextForeground(self._tab_text_colour(page))
            pagebitmap = page.bitmap
            
        # create points that will make the tab outline

        clip_width = tab_width
        if tab_x + clip_width > in_rect.x + in_rect.width:
            clip_width = in_rect.x + in_rect.width - tab_x

        # since the above code above doesn't play well with WXDFB or WXCOCOA,
        # we'll just use a rectangle for the clipping region for now --
        dc.SetClippingRegion(tab_x, tab_y, clip_width+1, tab_height-3)

        border_points = [wx.Point() for i in xrange(6)]
        agwFlags = self.GetAGWFlags()
        
        if agwFlags & AUI_NB_BOTTOM:
        
            border_points[0] = wx.Point(tab_x,             tab_y)
            border_points[1] = wx.Point(tab_x,             tab_y+tab_height-6)
            border_points[2] = wx.Point(tab_x+2,           tab_y+tab_height-4)
            border_points[3] = wx.Point(tab_x+tab_width-2, tab_y+tab_height-4)
            border_points[4] = wx.Point(tab_x+tab_width,   tab_y+tab_height-6)
            border_points[5] = wx.Point(tab_x+tab_width,   tab_y)
        
        else: #if (agwFlags & AUI_NB_TOP) 
        
            border_points[0] = wx.Point(tab_x,             tab_y+tab_height-4)
            border_points[1] = wx.Point(tab_x,             tab_y+2)
            border_points[2] = wx.Point(tab_x+2,           tab_y)
            border_points[3] = wx.Point(tab_x+tab_width-2, tab_y)
            border_points[4] = wx.Point(tab_x+tab_width,   tab_y+2)
            border_points[5] = wx.Point(tab_x+tab_width,   tab_y+tab_height-4)
        
        # TODO: else if (agwFlags & AUI_NB_LEFT) 
        # TODO: else if (agwFlags & AUI_NB_RIGHT) 

        drawn_tab_yoff = border_points[1].y
        drawn_tab_height = border_points[0].y - border_points[1].y

        if page.active:
        
            # draw active tab

            # draw base background colour
            r = wx.Rect(tab_x, tab_y, tab_width, tab_height)
            dc.SetPen(self._base_colour_pen)
            dc.SetBrush(self._base_colour_brush)
            dc.DrawRectangle(r.x+1, r.y+1, r.width-1, r.height-4)

            # this white helps fill out the gradient at the top of the tab
            dc.SetPen( wx.Pen(self._tab_gradient_highlight_colour) )
            dc.SetBrush( wx.Brush(self._tab_gradient_highlight_colour) )
            dc.DrawRectangle(r.x+2, r.y+1, r.width-3, r.height-4)

            # these two points help the rounded corners appear more antialiased
            dc.SetPen(self._base_colour_pen)
            dc.DrawPoint(r.x+2, r.y+1)
            dc.DrawPoint(r.x+r.width-2, r.y+1)

            # set rectangle down a bit for gradient drawing
            r.SetHeight(r.GetHeight()/2)
            r.x += 2
            r.width -= 2
            r.y += r.height
            r.y -= 2

            # draw gradient background
            top_colour = self._tab_bottom_colour
            bottom_colour = self._tab_top_colour
            dc.GradientFillLinear(r, bottom_colour, top_colour, wx.NORTH)
        
        else:
        
            # draw inactive tab

            r = wx.Rect(tab_x, tab_y+1, tab_width, tab_height-3)

            # start the gradent up a bit and leave the inside border inset
            # by a pixel for a 3D look.  Only the top half of the inactive
            # tab will have a slight gradient
            r.x += 3
            r.y += 1
            r.width -= 4
            r.height /= 2
            r.height -= 1

            # -- draw top gradient fill for glossy look
            top_colour = self._tab_inactive_top_colour
            bottom_colour = self._tab_inactive_bottom_colour
            dc.GradientFillLinear(r, bottom_colour, top_colour, wx.NORTH)

            r.y += r.height
            r.y -= 1

            # -- draw bottom fill for glossy look
            top_colour = self._tab_inactive_bottom_colour
            bottom_colour = self._tab_inactive_bottom_colour
            dc.GradientFillLinear(r, top_colour, bottom_colour, wx.SOUTH)
        
        # draw tab outline
        dc.SetPen(self._border_pen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawPolygon(border_points)

        # there are two horizontal grey lines at the bottom of the tab control,
        # this gets rid of the top one of those lines in the tab control
        if page.active:
        
            if agwFlags & AUI_NB_BOTTOM:
                dc.SetPen(wx.Pen(self._background_bottom_colour))
                
            # TODO: else if (agwFlags & AUI_NB_LEFT) 
            # TODO: else if (agwFlags & AUI_NB_RIGHT) 
            else: # for AUI_NB_TOP
                dc.SetPen(self._base_colour_pen)
                
            dc.DrawLine(border_points[0].x+1,
                        border_points[0].y,
                        border_points[5].x,
                        border_points[5].y)
        
        text_offset = tab_x + 8
        close_button_width = 0

        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            close_button_width = self._active_close_bmp.GetWidth()

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                text_offset += close_button_width - 5
                
        bitmap_offset = 0
        
        if pagebitmap.IsOk():
        
            bitmap_offset = tab_x + 8
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT and close_button_width:
                bitmap_offset += close_button_width - 5

            # draw bitmap
            dc.DrawBitmap(pagebitmap,
                          bitmap_offset,
                          drawn_tab_yoff + (drawn_tab_height/2) - (pagebitmap.GetHeight()/2),
                          True)

            text_offset = bitmap_offset + pagebitmap.GetWidth()
            text_offset += 3 # bitmap padding

        else:

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT == 0 or not close_button_width:
                text_offset = tab_x + 8
        
        draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - close_button_width)

        ypos = drawn_tab_yoff + (drawn_tab_height)/2 - (texty/2) - 1

        offset_focus = text_offset     
        if control is not None:
            if control.GetPosition() != wx.Point(text_offset+1, ypos):
                control.SetPosition(wx.Point(text_offset+1, ypos))

            if not control.IsShown():
                control.Show()

            if paint_control:
                bmp = TakeScreenShot(control.GetScreenRect())
                dc.DrawBitmap(bmp, text_offset+1, ypos, True)
                
            controlW, controlH = control.GetSize()
            text_offset += controlW + 4
            textx += controlW + 4
            
        # draw tab text
        rectx, recty, dummy = dc.GetMultiLineTextExtent(draw_text)
        dc.DrawLabel(draw_text, wx.Rect(text_offset, ypos, rectx, recty))

        # draw focus rectangle
        if (agwFlags & AUI_NB_NO_TAB_FOCUS) == 0:
            self.DrawFocusRectangle(dc, page, wnd, draw_text, offset_focus, bitmap_offset, drawn_tab_yoff, drawn_tab_height, rectx, recty)
        
        out_button_rect = wx.Rect()
        
        # draw close button if necessary
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
        
            bmp = self._disabled_close_bmp

            if close_button_state == AUI_BUTTON_STATE_HOVER:
                bmp = self._hover_close_bmp
            elif close_button_state == AUI_BUTTON_STATE_PRESSED:
                bmp = self._pressed_close_bmp

            shift = (agwFlags & AUI_NB_BOTTOM and [1] or [0])[0]

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                rect = wx.Rect(tab_x + 4, tab_y + (tab_height - bmp.GetHeight())/2 - shift,
                               close_button_width, tab_height)
            else:
                rect = wx.Rect(tab_x + tab_width - close_button_width - 1,
                               tab_y + (tab_height - bmp.GetHeight())/2 - shift,
                               close_button_width, tab_height)

            rect = IndentPressedBitmap(rect, close_button_state)
            dc.DrawBitmap(bmp, rect.x, rect.y, True)

            out_button_rect = rect
        
        out_tab_rect = wx.Rect(tab_x, tab_y, tab_width, tab_height)

        dc.DestroyClippingRegion()

        return out_tab_rect, out_button_rect, x_extent
    

    def SetCustomButton(self, bitmap_id, button_state, bmp):
        """
        Sets a custom bitmap for the close, left, right and window list
        buttons.
        
        :param `bitmap_id`: the button identifier;
        :param `button_state`: the button state;
        :param `bmp`: the custom bitmap to use for the button.
        """

        if bitmap_id == AUI_BUTTON_CLOSE:
            if button_state == AUI_BUTTON_STATE_NORMAL:
                self._active_close_bmp = bmp
                self._hover_close_bmp = self._active_close_bmp
                self._pressed_close_bmp = self._active_close_bmp
                self._disabled_close_bmp = self._active_close_bmp
                    
            elif button_state == AUI_BUTTON_STATE_HOVER:
                self._hover_close_bmp = bmp
            elif button_state == AUI_BUTTON_STATE_PRESSED:
                self._pressed_close_bmp = bmp
            else:
                self._disabled_close_bmp = bmp

        elif bitmap_id == AUI_BUTTON_LEFT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                self._disabled_left_bmp = bmp
            else:
                self._active_left_bmp = bmp

        elif bitmap_id == AUI_BUTTON_RIGHT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                self._disabled_right_bmp = bmp
            else:
                self._active_right_bmp = bmp

        elif bitmap_id == AUI_BUTTON_WINDOWLIST:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                self._disabled_windowlist_bmp = bmp
            else:
                self._active_windowlist_bmp = bmp
        

    def GetIndentSize(self):
        """ Returns the tabs indent size. """

        return 5


    def GetTabSize(self, dc, wnd, caption, bitmap, active, close_button_state, control=None):
        """
        Returns the tab size for the given caption, bitmap and button state.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `caption`: the tab text caption;
        :param `bitmap`: the bitmap displayed on the tab;
        :param `active`: whether the tab is selected or not;
        :param `close_button_state`: the state of the close button on the tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """

        dc.SetFont(self._measuring_font)
        measured_textx, measured_texty, dummy = dc.GetMultiLineTextExtent(caption)

        # add padding around the text
        tab_width = measured_textx
        tab_height = measured_texty

        # if the close button is showing, add space for it
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            tab_width += self._active_close_bmp.GetWidth() + 3

        # if there's a bitmap, add space for it
        if bitmap.IsOk():
            tab_width += bitmap.GetWidth()
            tab_width += 3 # right side bitmap padding
            tab_height = max(tab_height, bitmap.GetHeight())
        
        # add padding
        tab_width += 16
        tab_height += 10

        agwFlags = self.GetAGWFlags()
        if agwFlags & AUI_NB_TAB_FIXED_WIDTH:
            tab_width = self._fixed_tab_width

        if control is not None:
            tab_width += control.GetSize().GetWidth() + 4
            
        x_extent = tab_width

        return (tab_width, tab_height), x_extent


    def DrawButton(self, dc, wnd, in_rect, button, orientation):
        """
        Draws a button on the tab or on the tab area, depending on the button identifier. 

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `button`: an instance of the button class;
        :param `orientation`: the tab orientation.
        """

        bitmap_id, button_state = button.id, button.cur_state
        
        if bitmap_id == AUI_BUTTON_CLOSE:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_close_bmp
            elif button_state & AUI_BUTTON_STATE_HOVER:
                bmp = self._hover_close_bmp
            elif button_state & AUI_BUTTON_STATE_PRESSED:
                bmp = self._pressed_close_bmp
            else:
                bmp = self._active_close_bmp

        elif bitmap_id == AUI_BUTTON_LEFT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_left_bmp
            else:
                bmp = self._active_left_bmp

        elif bitmap_id == AUI_BUTTON_RIGHT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_right_bmp
            else:
                bmp = self._active_right_bmp

        elif bitmap_id == AUI_BUTTON_WINDOWLIST:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_windowlist_bmp
            else:
                bmp = self._active_windowlist_bmp

        else:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = button.dis_bitmap
            else:
                bmp = button.bitmap
                
        if not bmp.IsOk():
            return

        rect = wx.Rect(*in_rect)

        if orientation == wx.LEFT:
        
            rect.SetX(in_rect.x)
            rect.SetY(((in_rect.y + in_rect.height)/2) - (bmp.GetHeight()/2))
            rect.SetWidth(bmp.GetWidth())
            rect.SetHeight(bmp.GetHeight())
        
        else:
        
            rect = wx.Rect(in_rect.x + in_rect.width - bmp.GetWidth(),
                           ((in_rect.y + in_rect.height)/2) - (bmp.GetHeight()/2),
                           bmp.GetWidth(), bmp.GetHeight())
        
        rect = IndentPressedBitmap(rect, button_state)
        dc.DrawBitmap(bmp, rect.x, rect.y, True)

        out_rect = rect

        if bitmap_id == AUI_BUTTON_RIGHT:
            self._buttonRect = wx.Rect(rect.x, rect.y, 30, rect.height)
        
        return out_rect


    def DrawFocusRectangle(self, dc, page, wnd, draw_text, text_offset, bitmap_offset, drawn_tab_yoff, drawn_tab_height, textx, texty):
        """
        Draws the focus rectangle on a tab.

        :param `dc`: a `wx.DC` device context;
        :param `page`: the page associated with the tab;
        :param `wnd`: a `wx.Window` instance object;
        :param `draw_text`: the text that has been drawn on the tab;
        :param `text_offset`: the text offset on the tab;
        :param `bitmap_offset`: the bitmap offset on the tab;
        :param `drawn_tab_yoff`: the y offset of the tab text;
        :param `drawn_tab_height`: the height of the tab;
        :param `textx`: the x text extent;
        :param `texty`: the y text extent.
        """

        if self.GetAGWFlags() & AUI_NB_NO_TAB_FOCUS:
            return
        
        if page.active and wx.Window.FindFocus() == wnd:
        
            focusRectText = wx.Rect(text_offset, (drawn_tab_yoff + (drawn_tab_height)/2 - (texty/2)),
                                    textx, texty)

            if page.bitmap.IsOk():
                focusRectBitmap = wx.Rect(bitmap_offset, drawn_tab_yoff + (drawn_tab_height/2) - (page.bitmap.GetHeight()/2),
                                          page.bitmap.GetWidth(), page.bitmap.GetHeight())

            if page.bitmap.IsOk() and draw_text == "":
                focusRect = wx.Rect(*focusRectBitmap)
            elif not page.bitmap.IsOk() and draw_text != "":
                focusRect = wx.Rect(*focusRectText)
            elif page.bitmap.IsOk() and draw_text != "":
                focusRect = focusRectText.Union(focusRectBitmap)

            focusRect.Inflate(2, 2)

            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetPen(self._focusPen)
            dc.DrawRoundedRectangleRect(focusRect, 2)
        

    def GetBestTabCtrlSize(self, wnd, pages, required_bmp_size):
        """
        Returns the best tab control size.

        :param `wnd`: a `wx.Window` instance object;
        :param `pages`: the pages associated with the tabs;
        :param `required_bmp_size`: the size of the bitmap on the tabs.
        """

        dc = wx.ClientDC(wnd)
        dc.SetFont(self._measuring_font)

        # sometimes a standard bitmap size needs to be enforced, especially
        # if some tabs have bitmaps and others don't.  This is important because
        # it prevents the tab control from resizing when tabs are added.

        measure_bmp = wx.NullBitmap
        
        if required_bmp_size.IsFullySpecified():
            measure_bmp = wx.EmptyBitmap(required_bmp_size.x,
                                         required_bmp_size.y)
        
        max_y = 0
        
        for page in pages:
        
            if measure_bmp.IsOk():
                bmp = measure_bmp
            else:
                bmp = page.bitmap

            # we don't use the caption text because we don't
            # want tab heights to be different in the case
            # of a very short piece of text on one tab and a very
            # tall piece of text on another tab
            s, x_ext = self.GetTabSize(dc, wnd, page.caption, bmp, True, AUI_BUTTON_STATE_HIDDEN, None)
            max_y = max(max_y, s[1])

            if page.control:
                controlW, controlH = page.control.GetSize()
                max_y = max(max_y, controlH+4)

        return max_y + 2


    def SetNormalFont(self, font):
        """
        Sets the normal font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """

        self._normal_font = font


    def SetSelectedFont(self, font):
        """
        Sets the selected tab font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """

        self._selected_font = font


    def SetMeasuringFont(self, font):
        """
        Sets the font for calculating text measurements.

        :param `font`: a `wx.Font` object.
        """

        self._measuring_font = font


    def GetNormalFont(self):
        """ Returns the normal font for drawing tab labels. """

        return self._normal_font


    def GetSelectedFont(self):
        """ Returns the selected tab font for drawing tab labels. """

        return self._selected_font


    def GetMeasuringFont(self):
        """ Returns the font for calculating text measurements. """

        return self._measuring_font
    

    def ShowDropDown(self, wnd, pages, active_idx):
        """
        Shows the drop-down window menu on the tab area.

        :param `wnd`: a `wx.Window` derived window instance;
        :param `pages`: the pages associated with the tabs;
        :param `active_idx`: the active tab index.
        """
        
        useImages = self.GetAGWFlags() & AUI_NB_USE_IMAGES_DROPDOWN
        menuPopup = wx.Menu()

        longest = 0
        for i, page in enumerate(pages):
        
            caption = page.caption

            # if there is no caption, make it a space.  This will prevent
            # an assert in the menu code.
            if caption == "":
                caption = " "

            # Save longest caption width for calculating menu width with
            width = wnd.GetTextExtent(caption)[0]
            if width > longest:
                longest = width

            if useImages:
                menuItem = wx.MenuItem(menuPopup, 1000+i, caption)
                if page.bitmap:
                    menuItem.SetBitmap(page.bitmap)

                menuPopup.AppendItem(menuItem)
                
            else:
                
                menuPopup.AppendCheckItem(1000+i, caption)
                
            menuPopup.Enable(1000+i, page.enabled)

        if active_idx != -1 and not useImages:
        
            menuPopup.Check(1000+active_idx, True)
        
        # find out the screen coordinate at the bottom of the tab ctrl
        cli_rect = wnd.GetClientRect()

        # Calculate the approximate size of the popupmenu for setting the
        # position of the menu when its shown.
        # Account for extra padding on left/right of text on mac menus
        if wx.Platform in ['__WXMAC__', '__WXMSW__']:
            longest += 32

        # Bitmap/Checkmark width + padding
        longest += 20

        if self.GetAGWFlags() & AUI_NB_CLOSE_BUTTON:
            longest += 16

        pt = wx.Point(cli_rect.x + cli_rect.GetWidth() - longest,
                     cli_rect.y + cli_rect.height)

        cc = AuiCommandCapture()
        wnd.PushEventHandler(cc)
        wnd.PopupMenu(menuPopup, pt)
        command = cc.GetCommandId()
        wnd.PopEventHandler(True)

        if command >= 1000:
            return command - 1000

        return -1


class AuiSimpleTabArt(object):
    """ A simple-looking implementation of a tab art. """

    def __init__(self):
        """ Default class constructor. """

        self._normal_font = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self._selected_font = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)
        self._selected_font.SetWeight(wx.BOLD)
        self._measuring_font = self._selected_font

        self._agwFlags = 0
        self._fixed_tab_width = 100

        base_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DFACE)

        background_colour = base_colour
        normaltab_colour = base_colour
        selectedtab_colour = wx.WHITE

        self._bkbrush = wx.Brush(background_colour)
        self._normal_bkbrush = wx.Brush(normaltab_colour)
        self._normal_bkpen = wx.Pen(normaltab_colour)
        self._selected_bkbrush = wx.Brush(selectedtab_colour)
        self._selected_bkpen = wx.Pen(selectedtab_colour)

        self._active_close_bmp = BitmapFromBits(nb_close_bits, 16, 16, wx.BLACK)
        self._disabled_close_bmp = BitmapFromBits(nb_close_bits, 16, 16, wx.Colour(128, 128, 128))

        self._active_left_bmp = BitmapFromBits(nb_left_bits, 16, 16, wx.BLACK)
        self._disabled_left_bmp = BitmapFromBits(nb_left_bits, 16, 16, wx.Colour(128, 128, 128))

        self._active_right_bmp = BitmapFromBits(nb_right_bits, 16, 16, wx.BLACK)
        self._disabled_right_bmp = BitmapFromBits(nb_right_bits, 16, 16, wx.Colour(128, 128, 128))

        self._active_windowlist_bmp = BitmapFromBits(nb_list_bits, 16, 16, wx.BLACK)
        self._disabled_windowlist_bmp = BitmapFromBits(nb_list_bits, 16, 16, wx.Colour(128, 128, 128))


    def Clone(self):
        """ Clones the art object. """

        art = type(self)()
        art.SetNormalFont(self.GetNormalFont())
        art.SetSelectedFont(self.GetSelectedFont())
        art.SetMeasuringFont(self.GetMeasuringFont())

        art = CopyAttributes(art, self)
        return art


    def SetAGWFlags(self, agwFlags):
        """
        Sets the tab art flags.

        :param `agwFlags`: a combination of the following values:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet.
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet.
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by L{AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ``AUI_NB_ORDER_BY_ACCESS``           Tab navigation order by last access time for the tabs
         ``AUI_NB_NO_TAB_FOCUS``              Don't draw tab focus rectangle
         ==================================== ==================================
        
        """

        self._agwFlags = agwFlags


    def GetAGWFlags(self):
        """
        Returns the tab art flags.

        :see: L{SetAGWFlags} for a list of possible return values.
        """

        return self._agwFlags
    

    def SetSizingInfo(self, tab_ctrl_size, tab_count, minMaxTabWidth):
        """
        Sets the tab sizing information.
        
        :param `tab_ctrl_size`: the size of the tab control area;
        :param `tab_count`: the number of tabs;
        :param `minMaxTabWidth`: a tuple containing the minimum and maximum tab widths
         to be used when the ``AUI_NB_TAB_FIXED_WIDTH`` style is active.
        """
        
        self._fixed_tab_width = 100
        minTabWidth, maxTabWidth = minMaxTabWidth

        tot_width = tab_ctrl_size.x - self.GetIndentSize() - 4

        if self._agwFlags & AUI_NB_CLOSE_BUTTON:
            tot_width -= self._active_close_bmp.GetWidth()
        if self._agwFlags & AUI_NB_WINDOWLIST_BUTTON:
            tot_width -= self._active_windowlist_bmp.GetWidth()

        if tab_count > 0:
            self._fixed_tab_width = tot_width/tab_count
        
        if self._fixed_tab_width < 100:
            self._fixed_tab_width = 100

        if self._fixed_tab_width > tot_width/2:
            self._fixed_tab_width = tot_width/2

        if self._fixed_tab_width > 220:
            self._fixed_tab_width = 220

        if minTabWidth > -1:
            self._fixed_tab_width = max(self._fixed_tab_width, minTabWidth)
        if maxTabWidth > -1:
            self._fixed_tab_width = min(self._fixed_tab_width, maxTabWidth)

        self._tab_ctrl_height = tab_ctrl_size.y
        

    def DrawBackground(self, dc, wnd, rect):
        """
        Draws the tab area background.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `rect`: the tab control rectangle.
        """
        
        # draw background
        dc.SetBrush(self._bkbrush)
        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.DrawRectangle(-1, -1, rect.GetWidth()+2, rect.GetHeight()+2)

        # draw base line
        dc.SetPen(wx.GREY_PEN)
        dc.DrawLine(0, rect.GetHeight()-1, rect.GetWidth(), rect.GetHeight()-1)


    def DrawTab(self, dc, wnd, page, in_rect, close_button_state, paint_control=False):
        """
        Draws a single tab.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `page`: the tab control page associated with the tab;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `close_button_state`: the state of the close button on the tab;
        :param `paint_control`: whether to draw the control inside a tab (if any) on a `wx.MemoryDC`.
        """
        
        # if the caption is empty, measure some temporary text
        caption = page.caption
        if caption == "":
            caption = "Xj"

        agwFlags = self.GetAGWFlags()
        
        dc.SetFont(self._selected_font)
        selected_textx, selected_texty, dummy = dc.GetMultiLineTextExtent(caption)

        dc.SetFont(self._normal_font)
        normal_textx, normal_texty, dummy = dc.GetMultiLineTextExtent(caption)

        control = page.control

        # figure out the size of the tab
        tab_size, x_extent = self.GetTabSize(dc, wnd, page.caption, page.bitmap,
                                             page.active, close_button_state, control)

        tab_height = tab_size[1]
        tab_width = tab_size[0]
        tab_x = in_rect.x
        tab_y = in_rect.y + in_rect.height - tab_height

        caption = page.caption
        # select pen, brush and font for the tab to be drawn

        if page.active:
        
            dc.SetPen(self._selected_bkpen)
            dc.SetBrush(self._selected_bkbrush)
            dc.SetFont(self._selected_font)
            textx = selected_textx
            texty = selected_texty
        
        else:
        
            dc.SetPen(self._normal_bkpen)
            dc.SetBrush(self._normal_bkbrush)
            dc.SetFont(self._normal_font)
            textx = normal_textx
            texty = normal_texty

        if not page.enabled:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))
        else:
            dc.SetTextForeground(page.text_colour)
        
        # -- draw line --

        points = [wx.Point() for i in xrange(7)]
        points[0].x = tab_x
        points[0].y = tab_y + tab_height - 1
        points[1].x = tab_x + tab_height - 3
        points[1].y = tab_y + 2
        points[2].x = tab_x + tab_height + 3
        points[2].y = tab_y
        points[3].x = tab_x + tab_width - 2
        points[3].y = tab_y
        points[4].x = tab_x + tab_width
        points[4].y = tab_y + 2
        points[5].x = tab_x + tab_width
        points[5].y = tab_y + tab_height - 1
        points[6] = points[0]

        dc.SetClippingRect(in_rect)
        dc.DrawPolygon(points)

        dc.SetPen(wx.GREY_PEN)
        dc.DrawLines(points)

        close_button_width = 0
        
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
        
            close_button_width = self._active_close_bmp.GetWidth()
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                if control:
                    text_offset = tab_x + (tab_height/2) + close_button_width - (textx/2) - 2
                else:
                    text_offset = tab_x + (tab_height/2) + ((tab_width+close_button_width)/2) - (textx/2) - 2
            else:
                if control:
                    text_offset = tab_x + (tab_height/2) + close_button_width - (textx/2)
                else:
                    text_offset = tab_x + (tab_height/2) + ((tab_width-close_button_width)/2) - (textx/2)
        
        else:
        
            text_offset = tab_x + (tab_height/3) + (tab_width/2) - (textx/2)
            if control:
                if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                    text_offset = tab_x + (tab_height/3) - (textx/2) + close_button_width + 2
                else:
                    text_offset = tab_x + (tab_height/3) - (textx/2)
        
        # set minimum text offset
        if text_offset < tab_x + tab_height:
            text_offset = tab_x + tab_height

        # chop text if necessary
        if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x))
        else:
            draw_text = ChopText(dc, caption,
                                 tab_width - (text_offset-tab_x) - close_button_width)

        ypos = (tab_y + tab_height)/2 - (texty/2) + 1

        if control is not None:
            if control.GetPosition() != wx.Point(text_offset+1, ypos):
                control.SetPosition(wx.Point(text_offset+1, ypos))

            if not control.IsShown():
                control.Show()

            if paint_control:
                bmp = TakeScreenShot(control.GetScreenRect())
                dc.DrawBitmap(bmp, text_offset+1, ypos, True)
                
            controlW, controlH = control.GetSize()
            text_offset += controlW + 4

        # draw tab text
        rectx, recty, dummy = dc.GetMultiLineTextExtent(draw_text)
        dc.DrawLabel(draw_text, wx.Rect(text_offset, ypos, rectx, recty))

        # draw focus rectangle
        if page.active and wx.Window.FindFocus() == wnd and (agwFlags & AUI_NB_NO_TAB_FOCUS) == 0:
        
            focusRect = wx.Rect(text_offset, ((tab_y + tab_height)/2 - (texty/2) + 1),
                                selected_textx, selected_texty)

            focusRect.Inflate(2, 2)
            # TODO:
            # This should be uncommented when DrawFocusRect will become
            # available in wxPython
            # wx.RendererNative.Get().DrawFocusRect(wnd, dc, focusRect, 0)

        out_button_rect = wx.Rect()        
        # draw close button if necessary
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
        
            if page.active:
                bmp = self._active_close_bmp
            else:
                bmp = self._disabled_close_bmp

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                rect = wx.Rect(tab_x + tab_height - 2,
                               tab_y + (tab_height/2) - (bmp.GetHeight()/2) + 1,
                               close_button_width, tab_height - 1)
            else:                
                rect = wx.Rect(tab_x + tab_width - close_button_width - 1,
                               tab_y + (tab_height/2) - (bmp.GetHeight()/2) + 1,
                               close_button_width, tab_height - 1)
            
            self.DrawButtons(dc, rect, bmp, wx.WHITE, close_button_state)
            out_button_rect = wx.Rect(*rect)
        
        out_tab_rect = wx.Rect(tab_x, tab_y, tab_width, tab_height)
        dc.DestroyClippingRegion()

        return out_tab_rect, out_button_rect, x_extent  


    def DrawButtons(self, dc, _rect, bmp, bkcolour, button_state):
        """
        Convenience method to draw tab buttons.

        :param `dc`: a `wx.DC` device context;
        :param `_rect`: the tab rectangle;
        :param `bmp`: the tab bitmap;
        :param `bkcolour`: the tab background colour;
        :param `button_state`: the state of the tab button.
        """

        rect = wx.Rect(*_rect)

        if button_state == AUI_BUTTON_STATE_PRESSED:
            rect.x += 1
            rect.y += 1

        if button_state in [AUI_BUTTON_STATE_HOVER, AUI_BUTTON_STATE_PRESSED]:
            dc.SetBrush(wx.Brush(StepColour(bkcolour, 120)))
            dc.SetPen(wx.Pen(StepColour(bkcolour, 75)))

            # draw the background behind the button
            dc.DrawRectangle(rect.x, rect.y, 15, 15)

        # draw the button itself
        dc.DrawBitmap(bmp, rect.x, rect.y, True)

    
    def GetIndentSize(self):
        """ Returns the tabs indent size. """
        
        return 0


    def GetTabSize(self, dc, wnd, caption, bitmap, active, close_button_state, control=None):
        """
        Returns the tab size for the given caption, bitmap and button state.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `caption`: the tab text caption;
        :param `bitmap`: the bitmap displayed on the tab;
        :param `active`: whether the tab is selected or not;
        :param `close_button_state`: the state of the close button on the tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """
        
        dc.SetFont(self._measuring_font)
        measured_textx, measured_texty, dummy = dc.GetMultiLineTextExtent(caption)

        tab_height = measured_texty + 4
        tab_width = measured_textx + tab_height + 5

        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            tab_width += self._active_close_bmp.GetWidth()

        if self._agwFlags & AUI_NB_TAB_FIXED_WIDTH:
            tab_width = self._fixed_tab_width

        if control is not None:
            controlW, controlH = control.GetSize()
            tab_width += controlW + 4

        x_extent = tab_width - (tab_height/2) - 1

        return (tab_width, tab_height), x_extent


    def DrawButton(self, dc, wnd, in_rect, button, orientation):
        """
        Draws a button on the tab or on the tab area, depending on the button identifier. 

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `button`: an instance of the button class;
        :param `orientation`: the tab orientation.
        """

        bitmap_id, button_state = button.id, button.cur_state
        
        if bitmap_id == AUI_BUTTON_CLOSE:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_close_bmp
            else:
                bmp = self._active_close_bmp

        elif bitmap_id == AUI_BUTTON_LEFT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_left_bmp
            else:
                bmp = self._active_left_bmp

        elif bitmap_id == AUI_BUTTON_RIGHT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_right_bmp
            else:
                bmp = self._active_right_bmp

        elif bitmap_id == AUI_BUTTON_WINDOWLIST:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = self._disabled_windowlist_bmp
            else:
                bmp = self._active_windowlist_bmp

        else:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                bmp = button.dis_bitmap
            else:
                bmp = button.bitmap
            
        if not bmp.IsOk():
            return

        rect = wx.Rect(*in_rect)

        if orientation == wx.LEFT:
        
            rect.SetX(in_rect.x)
            rect.SetY(((in_rect.y + in_rect.height)/2) - (bmp.GetHeight()/2))
            rect.SetWidth(bmp.GetWidth())
            rect.SetHeight(bmp.GetHeight())
        
        else:
        
            rect = wx.Rect(in_rect.x + in_rect.width - bmp.GetWidth(),
                           ((in_rect.y + in_rect.height)/2) - (bmp.GetHeight()/2),
                           bmp.GetWidth(), bmp.GetHeight())

        self.DrawButtons(dc, rect, bmp, wx.WHITE, button_state)

        out_rect = wx.Rect(*rect)
        return out_rect


    def ShowDropDown(self, wnd, pages, active_idx):
        """
        Shows the drop-down window menu on the tab area.

        :param `wnd`: a `wx.Window` derived window instance;
        :param `pages`: the pages associated with the tabs;
        :param `active_idx`: the active tab index.
        """
        
        menuPopup = wx.Menu()
        useImages = self.GetAGWFlags() & AUI_NB_USE_IMAGES_DROPDOWN
        
        for i, page in enumerate(pages):

            if useImages:
                menuItem = wx.MenuItem(menuPopup, 1000+i, page.caption)
                if page.bitmap:
                    menuItem.SetBitmap(page.bitmap)

                menuPopup.AppendItem(menuItem)
                
            else:
                
                menuPopup.AppendCheckItem(1000+i, page.caption)
                
            menuPopup.Enable(1000+i, page.enabled)
        
        if active_idx != -1 and not useImages:
            menuPopup.Check(1000+active_idx, True)
        
        # find out where to put the popup menu of window
        # items.  Subtract 100 for now to center the menu
        # a bit, until a better mechanism can be implemented
        pt = wx.GetMousePosition()
        pt = wnd.ScreenToClient(pt)
        
        if pt.x < 100:
            pt.x = 0
        else:
            pt.x -= 100

        # find out the screen coordinate at the bottom of the tab ctrl
        cli_rect = wnd.GetClientRect()
        pt.y = cli_rect.y + cli_rect.height

        cc = AuiCommandCapture()
        wnd.PushEventHandler(cc)
        wnd.PopupMenu(menuPopup, pt)
        command = cc.GetCommandId()
        wnd.PopEventHandler(True)

        if command >= 1000:
            return command-1000

        return -1


    def GetBestTabCtrlSize(self, wnd, pages, required_bmp_size):
        """
        Returns the best tab control size.

        :param `wnd`: a `wx.Window` instance object;
        :param `pages`: the pages associated with the tabs;
        :param `required_bmp_size`: the size of the bitmap on the tabs.
        """
        
        dc = wx.ClientDC(wnd)
        dc.SetFont(self._measuring_font)
        s, x_extent = self.GetTabSize(dc, wnd, "ABCDEFGHIj", wx.NullBitmap, True,
                                      AUI_BUTTON_STATE_HIDDEN, None)

        max_y = s[1]

        for page in pages:
            if page.control:
                controlW, controlH = page.control.GetSize()
                max_y = max(max_y, controlH+4)
                
            textx, texty, dummy = dc.GetMultiLineTextExtent(page.caption)
            max_y = max(max_y, texty)
        
        return max_y + 3


    def SetNormalFont(self, font):
        """
        Sets the normal font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """
        
        self._normal_font = font


    def SetSelectedFont(self, font):
        """
        Sets the selected tab font for drawing tab labels.

        :param `font`: a `wx.Font` object.
        """
        
        self._selected_font = font


    def SetMeasuringFont(self, font):
        """
        Sets the font for calculating text measurements.

        :param `font`: a `wx.Font` object.
        """
        
        self._measuring_font = font


    def GetNormalFont(self):
        """ Returns the normal font for drawing tab labels. """

        return self._normal_font


    def GetSelectedFont(self):
        """ Returns the selected tab font for drawing tab labels. """

        return self._selected_font


    def GetMeasuringFont(self):
        """ Returns the font for calculating text measurements. """

        return self._measuring_font


    def SetCustomButton(self, bitmap_id, button_state, bmp):
        """
        Sets a custom bitmap for the close, left, right and window list
        buttons.
        
        :param `bitmap_id`: the button identifier;
        :param `button_state`: the button state;
        :param `bmp`: the custom bitmap to use for the button.
        """
        
        if bitmap_id == AUI_BUTTON_CLOSE:
            if button_state == AUI_BUTTON_STATE_NORMAL:
                self._active_close_bmp = bmp
                self._hover_close_bmp = self._active_close_bmp
                self._pressed_close_bmp = self._active_close_bmp
                self._disabled_close_bmp = self._active_close_bmp
                    
            elif button_state == AUI_BUTTON_STATE_HOVER:
                self._hover_close_bmp = bmp
            elif button_state == AUI_BUTTON_STATE_PRESSED:
                self._pressed_close_bmp = bmp
            else:
                self._disabled_close_bmp = bmp

        elif bitmap_id == AUI_BUTTON_LEFT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                self._disabled_left_bmp = bmp
            else:
                self._active_left_bmp = bmp

        elif bitmap_id == AUI_BUTTON_RIGHT:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                self._disabled_right_bmp = bmp
            else:
                self._active_right_bmp = bmp

        elif bitmap_id == AUI_BUTTON_WINDOWLIST:
            if button_state & AUI_BUTTON_STATE_DISABLED:
                self._disabled_windowlist_bmp = bmp
            else:
                self._active_windowlist_bmp = bmp
    

class VC71TabArt(AuiDefaultTabArt):
    """ A class to draw tabs using the Visual Studio 2003 (VC71) style. """

    def __init__(self):
        """ Default class constructor. """

        AuiDefaultTabArt.__init__(self)


    def Clone(self):
        """ Clones the art object. """

        art = type(self)()
        art.SetNormalFont(self.GetNormalFont())
        art.SetSelectedFont(self.GetSelectedFont())
        art.SetMeasuringFont(self.GetMeasuringFont())

        art = CopyAttributes(art, self)
        return art


    def DrawTab(self, dc, wnd, page, in_rect, close_button_state, paint_control=False):
        """
        Draws a single tab.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `page`: the tab control page associated with the tab;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `close_button_state`: the state of the close button on the tab;
        :param `paint_control`: whether to draw the control inside a tab (if any) on a `wx.MemoryDC`.
        """
        
        # Visual studio 7.1 style
        # This code is based on the renderer included in FlatNotebook

        # figure out the size of the tab

        control = page.control
        tab_size, x_extent = self.GetTabSize(dc, wnd, page.caption, page.bitmap, page.active,
                                             close_button_state, control)

        tab_height = self._tab_ctrl_height - 3
        tab_width = tab_size[0]
        tab_x = in_rect.x
        tab_y = in_rect.y + in_rect.height - tab_height
        clip_width = tab_width

        if tab_x + clip_width > in_rect.x + in_rect.width - 4:
            clip_width = (in_rect.x + in_rect.width) - tab_x - 4
            
        dc.SetClippingRegion(tab_x, tab_y, clip_width + 1, tab_height - 3)
        agwFlags = self.GetAGWFlags()

        if agwFlags & AUI_NB_BOTTOM:
            tab_y -= 1

        dc.SetPen((page.active and [wx.Pen(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DHIGHLIGHT))] or \
                   [wx.Pen(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DSHADOW))])[0])
        dc.SetBrush((page.active and [wx.Brush(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DFACE))] or \
                     [wx.TRANSPARENT_BRUSH])[0])

        if page.active:

            tabH = tab_height - 2
            dc.DrawRectangle(tab_x, tab_y, tab_width, tabH)

            rightLineY1 = (agwFlags & AUI_NB_BOTTOM and [vertical_border_padding - 2] or \
                           [vertical_border_padding - 1])[0]
            rightLineY2 = tabH + 3
            dc.SetPen(wx.Pen(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DSHADOW)))
            dc.DrawLine(tab_x + tab_width - 1, rightLineY1 + 1, tab_x + tab_width - 1, rightLineY2)
            
            if agwFlags & AUI_NB_BOTTOM:
                dc.DrawLine(tab_x + 1, rightLineY2 - 3 , tab_x + tab_width - 1, rightLineY2 - 3)
                
            dc.SetPen(wx.Pen(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DDKSHADOW)))
            dc.DrawLine(tab_x + tab_width, rightLineY1, tab_x + tab_width, rightLineY2)
            
            if agwFlags & AUI_NB_BOTTOM:
                dc.DrawLine(tab_x, rightLineY2 - 2, tab_x + tab_width, rightLineY2 - 2)

        else:
        
            # We dont draw a rectangle for non selected tabs, but only
            # vertical line on the right
            blackLineY1 = (agwFlags & AUI_NB_BOTTOM and [vertical_border_padding + 2] or \
                           [vertical_border_padding + 1])[0]
            blackLineY2 = tab_height - 5
            dc.DrawLine(tab_x + tab_width, blackLineY1, tab_x + tab_width, blackLineY2)
        
        border_points = [0, 0]
        
        if agwFlags & AUI_NB_BOTTOM:
        
            border_points[0] = wx.Point(tab_x, tab_y)
            border_points[1] = wx.Point(tab_x, tab_y + tab_height - 6)
        
        else: # if (agwFlags & AUI_NB_TOP)
        
            border_points[0] = wx.Point(tab_x, tab_y + tab_height - 4)
            border_points[1] = wx.Point(tab_x, tab_y + 2)

        drawn_tab_yoff = border_points[1].y
        drawn_tab_height = border_points[0].y - border_points[1].y

        text_offset = tab_x + 8
        close_button_width = 0

        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            close_button_width = self._active_close_bmp.GetWidth()
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                text_offset += close_button_width - 5

        if not page.enabled:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))
            pagebitmap = page.dis_bitmap
        else:
            dc.SetTextForeground(page.text_colour)
            pagebitmap = page.bitmap

        shift = 0
        if agwFlags & AUI_NB_BOTTOM:
            shift = (page.active and [1] or [2])[0]
            
        bitmap_offset = 0
        if pagebitmap.IsOk():
            bitmap_offset = tab_x + 8
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT and close_button_width:
                bitmap_offset += close_button_width - 5

            # draw bitmap
            dc.DrawBitmap(pagebitmap, bitmap_offset,
                          drawn_tab_yoff + (drawn_tab_height/2) - (pagebitmap.GetHeight()/2) + shift,
                          True)

            text_offset = bitmap_offset + pagebitmap.GetWidth()
            text_offset += 3 # bitmap padding
        
        else:
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT == 0 or not close_button_width:
                text_offset = tab_x + 8
        
        # if the caption is empty, measure some temporary text
        caption = page.caption

        if caption == "":
            caption = "Xj"

        if page.active:
            dc.SetFont(self._selected_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)
        else:
            dc.SetFont(self._normal_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)

        draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - close_button_width)

        ypos = drawn_tab_yoff + (drawn_tab_height)/2 - (texty/2) - 1 + shift

        offset_focus = text_offset
        
        if control is not None:
            if control.GetPosition() != wx.Point(text_offset+1, ypos):
                control.SetPosition(wx.Point(text_offset+1, ypos))

            if not control.IsShown():
                control.Show()

            if paint_control:
                bmp = TakeScreenShot(control.GetScreenRect())
                dc.DrawBitmap(bmp, text_offset+1, ypos, True)
                
            controlW, controlH = control.GetSize()
            text_offset += controlW + 4
            textx += controlW + 4

        # draw tab text
        rectx, recty, dummy = dc.GetMultiLineTextExtent(draw_text)
        dc.DrawLabel(draw_text, wx.Rect(text_offset, ypos, rectx, recty))

        out_button_rect = wx.Rect()

        # draw focus rectangle
        if (agwFlags & AUI_NB_NO_TAB_FOCUS) == 0:
            self.DrawFocusRectangle(dc, page, wnd, draw_text, offset_focus, bitmap_offset, drawn_tab_yoff+shift,
                                    drawn_tab_height+shift, rectx, recty)
                
        # draw 'x' on tab (if enabled)
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            close_button_width = self._active_close_bmp.GetWidth()

            bmp = self._disabled_close_bmp

            if close_button_state == AUI_BUTTON_STATE_HOVER:
                bmp = self._hover_close_bmp
            elif close_button_state == AUI_BUTTON_STATE_PRESSED:
                bmp = self._pressed_close_bmp

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                rect = wx.Rect(tab_x + 4,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + shift,
                               close_button_width, tab_height)
            else:
                rect = wx.Rect(tab_x + tab_width - close_button_width - 3,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + shift,
                               close_button_width, tab_height)

            # Indent the button if it is pressed down:
            rect = IndentPressedBitmap(rect, close_button_state)
            dc.DrawBitmap(bmp, rect.x, rect.y, True)

            out_button_rect = rect        

        out_tab_rect = wx.Rect(tab_x, tab_y, tab_width, tab_height)
        dc.DestroyClippingRegion()

        return out_tab_rect, out_button_rect, x_extent


class FF2TabArt(AuiDefaultTabArt):
    """ A class to draw tabs using the Firefox 2 (FF2) style. """

    def __init__(self):
        """ Default class constructor. """

        AuiDefaultTabArt.__init__(self)


    def Clone(self):
        """ Clones the art object. """

        art = type(self)()
        art.SetNormalFont(self.GetNormalFont())
        art.SetSelectedFont(self.GetSelectedFont())
        art.SetMeasuringFont(self.GetMeasuringFont())

        art = CopyAttributes(art, self)
        return art


    def GetTabSize(self, dc, wnd, caption, bitmap, active, close_button_state, control):
        """
        Returns the tab size for the given caption, bitmap and button state.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `caption`: the tab text caption;
        :param `bitmap`: the bitmap displayed on the tab;
        :param `active`: whether the tab is selected or not;
        :param `close_button_state`: the state of the close button on the tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """
        
        tab_size, x_extent = AuiDefaultTabArt.GetTabSize(self, dc, wnd, caption, bitmap,
                                                         active, close_button_state, control)

        tab_width, tab_height = tab_size        

        # add some vertical padding
        tab_height += 2
        
        return (tab_width, tab_height), x_extent


    def DrawTab(self, dc, wnd, page, in_rect, close_button_state, paint_control=False):
        """
        Draws a single tab.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `page`: the tab control page associated with the tab;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `close_button_state`: the state of the close button on the tab;
        :param `paint_control`: whether to draw the control inside a tab (if any) on a `wx.MemoryDC`.
        """
        
        # Firefox 2 style

        control = page.control

        # figure out the size of the tab
        tab_size, x_extent = self.GetTabSize(dc, wnd, page.caption, page.bitmap,
                                             page.active, close_button_state, control)

        tab_height = self._tab_ctrl_height - 2
        tab_width = tab_size[0]
        tab_x = in_rect.x
        tab_y = in_rect.y + in_rect.height - tab_height

        clip_width = tab_width
        if tab_x + clip_width > in_rect.x + in_rect.width - 4:
            clip_width = (in_rect.x + in_rect.width) - tab_x - 4
            
        dc.SetClippingRegion(tab_x, tab_y, clip_width + 1, tab_height - 3)

        tabPoints = [wx.Point() for i in xrange(7)]
        
        adjust = 0
        if not page.active:
            adjust = 1

        agwFlags = self.GetAGWFlags()
        
        tabPoints[0].x = tab_x + 3
        tabPoints[0].y = (agwFlags & AUI_NB_BOTTOM and [3] or [tab_height - 2])[0]

        tabPoints[1].x = tabPoints[0].x
        tabPoints[1].y = (agwFlags & AUI_NB_BOTTOM and [tab_height - (vertical_border_padding + 2) - adjust] or \
                          [(vertical_border_padding + 2) + adjust])[0]

        tabPoints[2].x = tabPoints[1].x+2
        tabPoints[2].y = (agwFlags & AUI_NB_BOTTOM and [tab_height - vertical_border_padding - adjust] or \
                          [vertical_border_padding + adjust])[0]

        tabPoints[3].x = tab_x + tab_width - 2
        tabPoints[3].y = tabPoints[2].y

        tabPoints[4].x = tabPoints[3].x + 2
        tabPoints[4].y = tabPoints[1].y

        tabPoints[5].x = tabPoints[4].x
        tabPoints[5].y = tabPoints[0].y

        tabPoints[6].x = tabPoints[0].x
        tabPoints[6].y = tabPoints[0].y

        rr = wx.RectPP(tabPoints[2], tabPoints[5])
        self.DrawTabBackground(dc, rr, page.active, (agwFlags & AUI_NB_BOTTOM) == 0)

        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)))

        # Draw the tab as rounded rectangle
        dc.DrawPolygon(tabPoints)

        if page.active:
            dc.DrawLine(tabPoints[0].x + 1, tabPoints[0].y, tabPoints[5].x , tabPoints[0].y)
        
        drawn_tab_yoff = tabPoints[1].y
        drawn_tab_height = tabPoints[0].y - tabPoints[2].y

        text_offset = tab_x + 8
        close_button_width = 0
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            close_button_width = self._active_close_bmp.GetWidth()
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                text_offset += close_button_width - 4

        if not page.enabled:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))
            pagebitmap = page.dis_bitmap
        else:
            dc.SetTextForeground(page.text_colour)
            pagebitmap = page.bitmap

        shift = -1
        if agwFlags & AUI_NB_BOTTOM:
            shift = 2
        
        bitmap_offset = 0
        if pagebitmap.IsOk():
            bitmap_offset = tab_x + 8
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT and close_button_width:
                bitmap_offset += close_button_width - 4

            # draw bitmap
            dc.DrawBitmap(pagebitmap, bitmap_offset,
                          drawn_tab_yoff + (drawn_tab_height/2) - (pagebitmap.GetHeight()/2) + shift,
                          True)

            text_offset = bitmap_offset + pagebitmap.GetWidth()
            text_offset += 3 # bitmap padding
        
        else:
        
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT == 0 or not close_button_width:
                text_offset = tab_x + 8
        
        # if the caption is empty, measure some temporary text
        caption = page.caption
        if caption == "":
            caption = "Xj"

        if page.active:
            dc.SetFont(self._selected_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)
        else:
            dc.SetFont(self._normal_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)

        if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - close_button_width + 1)
        else:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - close_button_width)

        ypos = drawn_tab_yoff + drawn_tab_height/2 - texty/2 - 1 + shift

        offset_focus = text_offset
        
        if control is not None:
            if control.GetPosition() != wx.Point(text_offset+1, ypos):
                control.SetPosition(wx.Point(text_offset+1, ypos))

            if not control.IsShown():
                control.Show()

            if paint_control:
                bmp = TakeScreenShot(control.GetScreenRect())
                dc.DrawBitmap(bmp, text_offset+1, ypos, True)
                
            controlW, controlH = control.GetSize()
            text_offset += controlW + 4
            textx += controlW + 4
        
        # draw tab text
        rectx, recty, dummy = dc.GetMultiLineTextExtent(draw_text)
        dc.DrawLabel(draw_text, wx.Rect(text_offset, ypos, rectx, recty))

        # draw focus rectangle
        if (agwFlags & AUI_NB_NO_TAB_FOCUS) == 0:
            self.DrawFocusRectangle(dc, page, wnd, draw_text, offset_focus, bitmap_offset, drawn_tab_yoff+shift,
                                    drawn_tab_height, rectx, recty)
        
        out_button_rect = wx.Rect()
        # draw 'x' on tab (if enabled)
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
        
            close_button_width = self._active_close_bmp.GetWidth()
            bmp = self._disabled_close_bmp

            if close_button_state == AUI_BUTTON_STATE_HOVER:
                bmp = self._hover_close_bmp
            elif close_button_state == AUI_BUTTON_STATE_PRESSED:
                bmp = self._pressed_close_bmp

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                rect = wx.Rect(tab_x + 5,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + shift,
                               close_button_width, tab_height)
            else:
                rect = wx.Rect(tab_x + tab_width - close_button_width - 3,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + shift,
                               close_button_width, tab_height)

            # Indent the button if it is pressed down:
            rect = IndentPressedBitmap(rect, close_button_state)
            dc.DrawBitmap(bmp, rect.x, rect.y, True)
            out_button_rect = rect
        
        out_tab_rect = wx.Rect(tab_x, tab_y, tab_width, tab_height)
        dc.DestroyClippingRegion()
    
        return out_tab_rect, out_button_rect, x_extent


    def DrawTabBackground(self, dc, rect, focus, upperTabs):
        """
        Draws the tab background for the Firefox 2 style.
        This is more consistent with L{FlatNotebook} than before.

        :param `dc`: a `wx.DC` device context;
        :param `rect`: rectangle the tab should be confined to;
        :param `focus`: whether the tab has focus or not;
        :param `upperTabs`: whether the style is ``AUI_NB_TOP`` or ``AUI_NB_BOTTOM``.
        """

        # Define the rounded rectangle base on the given rect
        # we need an array of 9 points for it
        regPts = [wx.Point() for indx in xrange(9)]

        if focus:
            if upperTabs:
                leftPt = wx.Point(rect.x, rect.y + (rect.height / 10)*8)
                rightPt = wx.Point(rect.x + rect.width - 2, rect.y + (rect.height / 10)*8)
            else:
                leftPt = wx.Point(rect.x, rect.y + (rect.height / 10)*5)
                rightPt = wx.Point(rect.x + rect.width - 2, rect.y + (rect.height / 10)*5)
        else:
            leftPt = wx.Point(rect.x, rect.y + (rect.height / 2))
            rightPt = wx.Point(rect.x + rect.width - 2, rect.y + (rect.height / 2))

        # Define the top region
        top = wx.RectPP(rect.GetTopLeft(), rightPt)
        bottom = wx.RectPP(leftPt, rect.GetBottomRight())

        topStartColour = wx.WHITE

        if not focus:
            topStartColour = LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE), 50)

        topEndColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
        bottomStartColour = topEndColour
        bottomEndColour = topEndColour

        # Incase we use bottom tabs, switch the colours
        if upperTabs:
            if focus:
                dc.GradientFillLinear(top, topStartColour, topEndColour, wx.SOUTH)
                dc.GradientFillLinear(bottom, bottomStartColour, bottomEndColour, wx.SOUTH)
            else:
                dc.GradientFillLinear(top, topEndColour , topStartColour, wx.SOUTH)
                dc.GradientFillLinear(bottom, bottomStartColour, bottomEndColour, wx.SOUTH)

        else:
            if focus:
                dc.GradientFillLinear(bottom, topEndColour, bottomEndColour, wx.SOUTH)
                dc.GradientFillLinear(top, topStartColour, topStartColour, wx.SOUTH)
            else:
                dc.GradientFillLinear(bottom, bottomStartColour, bottomEndColour, wx.SOUTH)
                dc.GradientFillLinear(top, topEndColour, topStartColour, wx.SOUTH)
        
        dc.SetBrush(wx.TRANSPARENT_BRUSH)


class VC8TabArt(AuiDefaultTabArt):
    """ A class to draw tabs using the Visual Studio 2005 (VC8) style. """

    def __init__(self):
        """ Default class constructor. """

        AuiDefaultTabArt.__init__(self)


    def Clone(self):
        """ Clones the art object. """

        art = type(self)()
        art.SetNormalFont(self.GetNormalFont())
        art.SetSelectedFont(self.GetSelectedFont())
        art.SetMeasuringFont(self.GetMeasuringFont())

        art = CopyAttributes(art, self)
        return art


    def SetSizingInfo(self, tab_ctrl_size, tab_count, minMaxTabWidth):
        """
        Sets the tab sizing information.
        
        :param `tab_ctrl_size`: the size of the tab control area;
        :param `tab_count`: the number of tabs;
        :param `minMaxTabWidth`: a tuple containing the minimum and maximum tab widths
         to be used when the ``AUI_NB_TAB_FIXED_WIDTH`` style is active.
        """
        
        AuiDefaultTabArt.SetSizingInfo(self, tab_ctrl_size, tab_count, minMaxTabWidth)

        minTabWidth, maxTabWidth = minMaxTabWidth
        if minTabWidth > -1:
            self._fixed_tab_width = max(self._fixed_tab_width, minTabWidth)
        if maxTabWidth > -1:
            self._fixed_tab_width = min(self._fixed_tab_width, maxTabWidth)
        
        self._fixed_tab_width -= 5


    def GetTabSize(self, dc, wnd, caption, bitmap, active, close_button_state, control=None):
        """
        Returns the tab size for the given caption, bitmap and button state.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `caption`: the tab text caption;
        :param `bitmap`: the bitmap displayed on the tab;
        :param `active`: whether the tab is selected or not;
        :param `close_button_state`: the state of the close button on the tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """
        
        tab_size, x_extent = AuiDefaultTabArt.GetTabSize(self, dc, wnd, caption, bitmap,
                                                         active, close_button_state, control)

        tab_width, tab_height = tab_size        

        # add some padding
        tab_width += 10
        tab_height += 2

        return (tab_width, tab_height), x_extent


    def DrawTab(self, dc, wnd, page, in_rect, close_button_state, paint_control=False):
        """
        Draws a single tab.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `page`: the tab control page associated with the tab;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `close_button_state`: the state of the close button on the tab;
        :param `paint_control`: whether to draw the control inside a tab (if any) on a `wx.MemoryDC`.
        """
        
        # Visual Studio 8 style

        control = page.control

        # figure out the size of the tab
        tab_size, x_extent = self.GetTabSize(dc, wnd, page.caption, page.bitmap,
                                             page.active, close_button_state, control)

        tab_height = self._tab_ctrl_height - 1
        tab_width = tab_size[0]
        tab_x = in_rect.x
        tab_y = in_rect.y + in_rect.height - tab_height

        clip_width = tab_width + 3
        if tab_x + clip_width > in_rect.x + in_rect.width - 4:
            clip_width = (in_rect.x + in_rect.width) - tab_x - 4
        
        tabPoints = [wx.Point() for i in xrange(8)]

        # If we draw the first tab or the active tab, 
        # we draw a full tab, else we draw a truncated tab
        #
        #             X(2)                  X(3)
        #        X(1)                            X(4)
        #                                          
        #                                           X(5)
        #                                           
        # X(0),(7)                                  X(6)
        #
        #

        adjust = 0
        if not page.active:
            adjust = 1

        agwFlags = self.GetAGWFlags()
        tabPoints[0].x = (agwFlags & AUI_NB_BOTTOM and [tab_x] or [tab_x + adjust])[0]
        tabPoints[0].y = (agwFlags & AUI_NB_BOTTOM and [2] or [tab_height - 3])[0]

        tabPoints[1].x = tabPoints[0].x + tab_height - vertical_border_padding - 3 - adjust
        tabPoints[1].y = (agwFlags & AUI_NB_BOTTOM and [tab_height - (vertical_border_padding+2)] or \
                          [(vertical_border_padding+2)])[0]

        tabPoints[2].x = tabPoints[1].x + 4
        tabPoints[2].y = (agwFlags & AUI_NB_BOTTOM and [tab_height - vertical_border_padding] or \
                          [vertical_border_padding])[0]

        tabPoints[3].x = tabPoints[2].x + tab_width - tab_height + vertical_border_padding
        tabPoints[3].y = (agwFlags & AUI_NB_BOTTOM and [tab_height - vertical_border_padding] or \
                          [vertical_border_padding])[0]

        tabPoints[4].x = tabPoints[3].x + 1
        tabPoints[4].y = (agwFlags & AUI_NB_BOTTOM and [tabPoints[3].y - 1] or [tabPoints[3].y + 1])[0]

        tabPoints[5].x = tabPoints[4].x + 1
        tabPoints[5].y = (agwFlags & AUI_NB_BOTTOM and [(tabPoints[4].y - 1)] or [tabPoints[4].y + 1])[0]

        tabPoints[6].x = tabPoints[2].x + tab_width - tab_height + 2 + vertical_border_padding
        tabPoints[6].y = tabPoints[0].y

        tabPoints[7].x = tabPoints[0].x
        tabPoints[7].y = tabPoints[0].y

        self.FillVC8GradientColour(dc, tabPoints, page.active)        

        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        dc.SetPen(wx.Pen(wx.SystemSettings.GetColour(wx.SYS_COLOUR_BTNSHADOW)))
        dc.DrawPolygon(tabPoints)

        if page.active:
            # Delete the bottom line (or the upper one, incase we use wxBOTTOM) 
            dc.SetPen(wx.WHITE_PEN)
            dc.DrawLine(tabPoints[0].x, tabPoints[0].y, tabPoints[6].x, tabPoints[6].y)

        dc.SetClippingRegion(tab_x, tab_y, clip_width + 2, tab_height - 3)            

        drawn_tab_yoff = tabPoints[1].y
        drawn_tab_height = tabPoints[0].y - tabPoints[2].y

        text_offset = tab_x + 20
        close_button_width = 0
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            close_button_width = self._active_close_bmp.GetWidth()
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                text_offset += close_button_width

        if not page.enabled:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))
            pagebitmap = page.dis_bitmap
        else:
            dc.SetTextForeground(page.text_colour)
            pagebitmap = page.bitmap

        shift = 0
        if agwFlags & AUI_NB_BOTTOM:
            shift = (page.active and [1] or [2])[0]
        
        bitmap_offset = 0
        if pagebitmap.IsOk():
            bitmap_offset = tab_x + 20
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT and close_button_width:
                bitmap_offset += close_button_width

            # draw bitmap
            dc.DrawBitmap(pagebitmap, bitmap_offset,
                          drawn_tab_yoff + (drawn_tab_height/2) - (pagebitmap.GetHeight()/2) + shift,
                          True)

            text_offset = bitmap_offset + pagebitmap.GetWidth()
            text_offset += 3 # bitmap padding
        
        else:
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT == 0 or not close_button_width:
                text_offset = tab_x + tab_height
        
        # if the caption is empty, measure some temporary text
        caption = page.caption
        if caption == "":
            caption = "Xj"

        if page.active:
            dc.SetFont(self._selected_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)
        else:
            dc.SetFont(self._normal_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)

        if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x))
        else:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - close_button_width)

        ypos = drawn_tab_yoff + drawn_tab_height/2 - texty/2 - 1 + shift

        offset_focus = text_offset
        
        if control is not None:
            if control.GetPosition() != wx.Point(text_offset+1, ypos):
                control.SetPosition(wx.Point(text_offset+1, ypos))

            if not control.IsShown():
                control.Show()

            if paint_control:
                bmp = TakeScreenShot(control.GetScreenRect())
                dc.DrawBitmap(bmp, text_offset+1, ypos, True)
                
            controlW, controlH = control.GetSize()
            text_offset += controlW + 4
            textx += controlW + 4

        # draw tab text
        rectx, recty, dummy = dc.GetMultiLineTextExtent(draw_text)
        dc.DrawLabel(draw_text, wx.Rect(text_offset, ypos, rectx, recty))
        
        # draw focus rectangle
        if (agwFlags & AUI_NB_NO_TAB_FOCUS) == 0:
            self.DrawFocusRectangle(dc, page, wnd, draw_text, offset_focus, bitmap_offset, drawn_tab_yoff+shift,
                                    drawn_tab_height+shift, rectx, recty)
        
        out_button_rect = wx.Rect()
        # draw 'x' on tab (if enabled)
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
        
            close_button_width = self._active_close_bmp.GetWidth()
            bmp = self._disabled_close_bmp

            if close_button_state == AUI_BUTTON_STATE_HOVER:
                bmp = self._hover_close_bmp
            elif close_button_state == AUI_BUTTON_STATE_PRESSED:
                bmp = self._pressed_close_bmp
                
            if page.active:
                xpos = tab_x + tab_width - close_button_width + 3
            else:
                xpos = tab_x + tab_width - close_button_width - 5

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                rect = wx.Rect(tab_x + 20,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + shift,
                               close_button_width, tab_height)
            else:
                rect = wx.Rect(xpos,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + shift,
                               close_button_width, tab_height)

            # Indent the button if it is pressed down:
            rect = IndentPressedBitmap(rect, close_button_state)
            dc.DrawBitmap(bmp, rect.x, rect.y, True)
            out_button_rect = rect
        
        out_tab_rect = wx.Rect(tab_x, tab_y, x_extent, tab_height)
        dc.DestroyClippingRegion()

        return out_tab_rect, out_button_rect, x_extent
        

    def FillVC8GradientColour(self, dc, tabPoints, active):
        """
        Fills the tab with the Visual Studio 2005 gradient background.

        :param `dc`: a `wx.DC` device context;
        :param `tabPoints`: a list of `wx.Point` objects describing the tab shape;
        :param `active`: whether the tab is selected or not.
        """

        xList = [pt.x for pt in tabPoints]
        yList = [pt.y for pt in tabPoints]
        
        minx, maxx = min(xList), max(xList)
        miny, maxy = min(yList), max(yList)

        rect = wx.Rect(minx, maxy, maxx-minx, miny-maxy+1)        
        region = wx.RegionFromPoints(tabPoints)

        if self._buttonRect.width > 0:
            buttonRegion = wx.Region(*self._buttonRect)
            region.XorRegion(buttonRegion)
        
        dc.SetClippingRegionAsRegion(region)

        if active:
            bottom_colour = top_colour = wx.WHITE
        else:
            bottom_colour = StepColour(self._base_colour, 90)
            top_colour = StepColour(self._base_colour, 170)

        dc.GradientFillLinear(rect, top_colour, bottom_colour, wx.SOUTH)
        dc.DestroyClippingRegion()
        

class ChromeTabArt(AuiDefaultTabArt):
    """
    A class to draw tabs using the Google Chrome browser style.
    It uses custom bitmap to render the tabs, so that the look and feel is as close
    as possible to the Chrome style.
    """

    def __init__(self):
        """ Default class constructor. """

        AuiDefaultTabArt.__init__(self)

        self.SetBitmaps(mirror=False)
        
        closeBmp = tab_close.GetBitmap()
        closeHBmp = tab_close_h.GetBitmap()
        closePBmp = tab_close_p.GetBitmap()

        self.SetCustomButton(AUI_BUTTON_CLOSE, AUI_BUTTON_STATE_NORMAL, closeBmp)
        self.SetCustomButton(AUI_BUTTON_CLOSE, AUI_BUTTON_STATE_HOVER, closeHBmp)
        self.SetCustomButton(AUI_BUTTON_CLOSE, AUI_BUTTON_STATE_PRESSED, closePBmp)
        

    def SetAGWFlags(self, agwFlags):
        """
        Sets the tab art flags.

        :param `agwFlags`: a combination of the following values:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet.
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet.
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by L{AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ``AUI_NB_ORDER_BY_ACCESS``           Tab navigation order by last access time for the tabs
         ``AUI_NB_NO_TAB_FOCUS``              Don't draw tab focus rectangle
         ==================================== ==================================

        :note: Overridden from L{AuiDefaultTabArt}.
        """

        if agwFlags & AUI_NB_TOP:
            self.SetBitmaps(mirror=False)
        elif agwFlags & AUI_NB_BOTTOM:
            self.SetBitmaps(mirror=True)

        AuiDefaultTabArt.SetAGWFlags(self, agwFlags)            


    def SetBitmaps(self, mirror):
        """
        Assigns the tab custom bitmaps

        :param `mirror`: whether to vertically mirror the bitmap or not.
        """

        bmps = [tab_active_left.GetBitmap(), tab_active_center.GetBitmap(),
                tab_active_right.GetBitmap(), tab_inactive_left.GetBitmap(),
                tab_inactive_center.GetBitmap(), tab_inactive_right.GetBitmap()]

        if mirror:
            for indx, bmp in enumerate(bmps):
                img = bmp.ConvertToImage()
                img = img.Mirror(horizontally=False)
                bmps[indx] = img.ConvertToBitmap()
                
        self._leftActiveBmp = bmps[0]
        self._centerActiveBmp = bmps[1]
        self._rightActiveBmp = bmps[2]
        self._leftInactiveBmp = bmps[3]
        self._centerInactiveBmp = bmps[4]
        self._rightInactiveBmp = bmps[5]
            

    def Clone(self):
        """ Clones the art object. """

        art = type(self)()
        art.SetNormalFont(self.GetNormalFont())
        art.SetSelectedFont(self.GetSelectedFont())
        art.SetMeasuringFont(self.GetMeasuringFont())

        art = CopyAttributes(art, self)
        return art


    def SetSizingInfo(self, tab_ctrl_size, tab_count, minMaxTabWidth):
        """
        Sets the tab sizing information.
        
        :param `tab_ctrl_size`: the size of the tab control area;
        :param `tab_count`: the number of tabs;
        :param `minMaxTabWidth`: a tuple containing the minimum and maximum tab widths
         to be used when the ``AUI_NB_TAB_FIXED_WIDTH`` style is active.
        """
        
        AuiDefaultTabArt.SetSizingInfo(self, tab_ctrl_size, tab_count, minMaxTabWidth)

        minTabWidth, maxTabWidth = minMaxTabWidth
        if minTabWidth > -1:
            self._fixed_tab_width = max(self._fixed_tab_width, minTabWidth)
        if maxTabWidth > -1:
            self._fixed_tab_width = min(self._fixed_tab_width, maxTabWidth)

        self._fixed_tab_width -= 5


    def GetTabSize(self, dc, wnd, caption, bitmap, active, close_button_state, control=None):
        """
        Returns the tab size for the given caption, bitmap and button state.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `caption`: the tab text caption;
        :param `bitmap`: the bitmap displayed on the tab;
        :param `active`: whether the tab is selected or not;
        :param `close_button_state`: the state of the close button on the tab;
        :param `control`: a `wx.Window` instance inside a tab (or ``None``).
        """
        
        tab_size, x_extent = AuiDefaultTabArt.GetTabSize(self, dc, wnd, caption, bitmap,
                                                         active, close_button_state, control)

        tab_width, tab_height = tab_size        

        # add some padding
        tab_width += self._leftActiveBmp.GetWidth()
        tab_height += 2

        tab_height = max(tab_height, self._centerActiveBmp.GetHeight())        

        return (tab_width, tab_height), x_extent


    def DrawTab(self, dc, wnd, page, in_rect, close_button_state, paint_control=False):
        """
        Draws a single tab.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` instance object;
        :param `page`: the tab control page associated with the tab;
        :param `in_rect`: rectangle the tab should be confined to;
        :param `close_button_state`: the state of the close button on the tab;
        :param `paint_control`: whether to draw the control inside a tab (if any) on a `wx.MemoryDC`.
        """
        
        # Chrome tab style

        control = page.control
        # figure out the size of the tab
        tab_size, x_extent = self.GetTabSize(dc, wnd, page.caption, page.bitmap, page.active,
                                             close_button_state, control)

        agwFlags = self.GetAGWFlags()
        
        tab_height = self._tab_ctrl_height - 1
        tab_width = tab_size[0]
        tab_x = in_rect.x
        tab_y = in_rect.y + in_rect.height - tab_height
        clip_width = tab_width

        if tab_x + clip_width > in_rect.x + in_rect.width - 4:
            clip_width = (in_rect.x + in_rect.width) - tab_x - 4
            
        dc.SetClippingRegion(tab_x, tab_y, clip_width + 1, tab_height - 3)
        drawn_tab_yoff = 1

        if page.active:
            left = self._leftActiveBmp
            center = self._centerActiveBmp
            right = self._rightActiveBmp
        else:
            left = self._leftInactiveBmp
            center = self._centerInactiveBmp
            right = self._rightInactiveBmp

        dc.DrawBitmap(left, tab_x, tab_y)
        leftw = left.GetWidth()
        centerw = center.GetWidth()
        rightw = right.GetWidth()

        available = tab_x + tab_width - rightw
        posx = tab_x + leftw
        
        while 1:
            if posx >= available:
                break
            dc.DrawBitmap(center, posx, tab_y)
            posx += centerw

        dc.DrawBitmap(right, posx, tab_y)

        drawn_tab_height = center.GetHeight()
        text_offset = tab_x + leftw
        
        close_button_width = 0
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
            close_button_width = self._active_close_bmp.GetWidth()
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                text_offset += close_button_width

        if not page.enabled:
            dc.SetTextForeground(wx.SystemSettings.GetColour(wx.SYS_COLOUR_GRAYTEXT))
            pagebitmap = page.dis_bitmap
        else:
            dc.SetTextForeground(page.text_colour)
            pagebitmap = page.bitmap
        
        bitmap_offset = 0
        if pagebitmap.IsOk():
            bitmap_offset = tab_x + leftw
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT and close_button_width:
                bitmap_offset += close_button_width

            # draw bitmap
            dc.DrawBitmap(pagebitmap, bitmap_offset,
                          drawn_tab_yoff + (drawn_tab_height/2) - (pagebitmap.GetHeight()/2),
                          True)

            text_offset = bitmap_offset + pagebitmap.GetWidth()
            text_offset += 3 # bitmap padding
        
        else:
        
            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT == 0 or not close_button_width:
                text_offset = tab_x + leftw
        
        # if the caption is empty, measure some temporary text
        caption = page.caption
        if caption == "":
            caption = "Xj"

        if page.active:
            dc.SetFont(self._selected_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)
        else:
            dc.SetFont(self._normal_font)
            textx, texty, dummy = dc.GetMultiLineTextExtent(caption)

        if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - leftw)
        else:
            draw_text = ChopText(dc, caption, tab_width - (text_offset-tab_x) - close_button_width - leftw)

        ypos = drawn_tab_yoff + drawn_tab_height/2 - texty/2 - 1

        if control is not None:
            if control.GetPosition() != wx.Point(text_offset+1, ypos):
                control.SetPosition(wx.Point(text_offset+1, ypos))

            if not control.IsShown():
                control.Show()

            if paint_control:
                bmp = TakeScreenShot(control.GetScreenRect())
                dc.DrawBitmap(bmp, text_offset+1, ypos, True)
                
            controlW, controlH = control.GetSize()
            text_offset += controlW + 4

        # draw tab text
        rectx, recty, dummy = dc.GetMultiLineTextExtent(draw_text)
        dc.DrawLabel(draw_text, wx.Rect(text_offset, ypos, rectx, recty))
                
        out_button_rect = wx.Rect()
        # draw 'x' on tab (if enabled)
        if close_button_state != AUI_BUTTON_STATE_HIDDEN:
        
            close_button_width = self._active_close_bmp.GetWidth()
            bmp = self._disabled_close_bmp

            if close_button_state == AUI_BUTTON_STATE_HOVER:
                bmp = self._hover_close_bmp
            elif close_button_state == AUI_BUTTON_STATE_PRESSED:
                bmp = self._pressed_close_bmp

            if agwFlags & AUI_NB_CLOSE_ON_TAB_LEFT:
                rect = wx.Rect(tab_x + leftw - 2,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + 1,
                               close_button_width, tab_height)
            else:
                rect = wx.Rect(tab_x + tab_width - close_button_width - rightw + 2,
                               drawn_tab_yoff + (drawn_tab_height / 2) - (bmp.GetHeight() / 2) + 1,
                               close_button_width, tab_height)

            if agwFlags & AUI_NB_BOTTOM:
                rect.y -= 1
                
            # Indent the button if it is pressed down:
            rect = IndentPressedBitmap(rect, close_button_state)
            dc.DrawBitmap(bmp, rect.x, rect.y, True)
            out_button_rect = rect
            
        out_tab_rect = wx.Rect(tab_x, tab_y, tab_width, tab_height)
        dc.DestroyClippingRegion()

        return out_tab_rect, out_button_rect, x_extent        


