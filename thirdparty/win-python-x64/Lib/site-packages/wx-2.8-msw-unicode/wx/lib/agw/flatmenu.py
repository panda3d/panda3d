# --------------------------------------------------------------------------------- #
# FLATMENU wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 03 Nov 2006
# Latest Revision: 21 Sep 2010, 23.00 GMT
#
# TODO List
#
# 1. Work is still in progress, so other functionalities may be added in the future;
# 2. No shadows under MAC, but it may be possible to create them using Carbon.
#
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# gavana@kpo.kz
# andrea.gavana@gmail.com
#
# Or, Obviously, To The wxPython Mailing List!!!
#
#
# End Of Comments
# --------------------------------------------------------------------------------- #

"""
FlatMenu is a generic menu implementation.


Description
===========

FlatMenu, like the name implies, it is a generic menu implementation. 
I tried to provide a full functionality for menus, menubar and toolbar.


FlatMenu supports the following features:

- Fires all the events (UI & Cmd);
- Check items;
- Separators;
- Enabled / Disabled menu items;
- Images on items;
- Toolbar support, with images and separators;
- Controls in toolbar (work in progress);
- Toolbar tools tooltips (done: thanks to Peter Kort);
- Accelerators for menus;
- Accelerators for menubar;
- Radio items in menus;
- Integration with AUI;
- Scrolling when menu is too big to fit the screen;
- Menu navigation with keyboard;
- Drop down arrow button to the right of the menu, it always contains the
  "Customize" option, which will popup an options dialog. The dialog has the
  following abilities:

  (a) Ability to add/remove menus;
  (b) Select different colour schemes for the menu bar / toolbar;
  (c) Control various options, such as: colour for highlight menu item, draw
      border around menus (classic look only);
  (d) Toolbar floating appearance.
  
- Allows user to specify grey bitmap for disabled menus/toolbar tools;
- If no grey bitmap is provided, it generates one from the existing bitmap;
- Hidden toolbar items / menu bar items - will appear in a small popmenu
  to the right if they are hidden;
- 4 different colour schemes for the menu bar (more can easily added);
- Scrolling is available if the menu height is greater than the screen height;
- Context menus for menu items;
- Show/hide the drop down arrow which allows the customization of FlatMenu;
- Multiple columns menu window;
- Tooltips for menus and toolbar items on a `wx.StatusBar` (if present);
- Transparency (alpha channel) for menu windows (for platforms supporting it);
- First attempt in adding controls to FlatToolbar;
- Added a MiniBar (thanks to Vladiuz);
- Added `wx.ToolBar` methods AddCheckTool/AddRadioTool (thanks to Vladiuz).
  

Supported Platforms
===================

FlatMenu v0.8 has been tested on the following platforms:
  * Windows (Windows XP);
  * Linux Ubuntu (Dapper 6.06)
v0.9.* has been tested on
  * Windows (Windows XP, Vista);


Window Styles
=============

This class supports the following window styles:

========================= =========== ==================================================
Window Styles             Hex Value   Description
========================= =========== ==================================================
``FM_OPT_IS_LCD``                 0x1 Use this style if your computer uses a LCD screen.
``FM_OPT_MINIBAR``                0x2 Use this if you plan to use the toolbar only.
``FM_OPT_SHOW_CUSTOMIZE``         0x4 Show "customize link" in the `More` menu, you will need to write your own handler. See demo.
``FM_OPT_SHOW_TOOLBAR``           0x8 Set this option is you are planning to use the toolbar.
========================= =========== ==================================================


Events Processing
=================

This class processes the following events:

================================= ==================================================
Event Name                        Description
================================= ==================================================
``EVT_FLAT_MENU_DISMISSED``       Used internally.
``EVT_FLAT_MENU_ITEM_MOUSE_OUT``  Fires an event when the mouse leaves a `FlatMenuItem`.
``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` Fires an event when the mouse enters a `FlatMenuItem`.
``EVT_FLAT_MENU_SELECTED``        Fires the `wx.EVT_MENU` event for `FlatMenu`.
================================= ==================================================


License And Version
===================

FlatMenu is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 21 Sep 2010, 23.00 GMT

Version 0.9.6

"""

__docformat__ = "epytext"
__version__ = "0.9.6"

import wx
import math
import cStringIO

import wx.lib.colourutils as colourutils

from fmcustomizedlg import FMCustomizeDlg
from artmanager import ArtManager, DCSaver
from fmresources import *
            
# FlatMenu styles
FM_OPT_IS_LCD = 1
""" Use this style if your computer uses a LCD screen. """
FM_OPT_MINIBAR = 2
""" Use this if you plan to use the toolbar only. """
FM_OPT_SHOW_CUSTOMIZE = 4
""" Show "customize link" in the `More` menu, you will need to write your own handler. See demo. """
FM_OPT_SHOW_TOOLBAR = 8
""" Set this option is you are planning to use the toolbar. """

# Some checking to see if we can draw shadows behind the popup menus
# at least on Windows. *REQUIRES* Mark Hammond's win32all extensions
# and ctypes, on Windows obviouly. Mac and GTK have no shadows under
# the menus, and it has been reported that shadows don't work well
# on Windows 2000 and previous.

_libimported = None
_DELAY = 5000

if wx.Platform == "__WXMSW__":
    osVersion = wx.GetOsVersion()
    # Shadows behind menus are supported only in XP
    if osVersion[1] == 5 and osVersion[2] == 1:
        try:
            import win32api
            import win32gui
            _libimported = "MH"
        except:
            try:
                import ctypes
                _libimported = "ctypes"
            except:
                pass
    else:
        _libimported = None

# Simple hack, but I don't know how to make it work on Mac
# I don't have  Mac ;-)
#if wx.Platform == "__WXMAC__":
#    try:
#        import ctypes
#        _carbon_dll = ctypes.cdll.LoadLibrary(r'/System/Frameworks/Carbon.framework/Carbon')
#    except:
#        _carbon_dll = None


# FIXME: No way to get shadows on Windows with the original code...
# May anyone share some suggestion on how to make it work??
# Right now I am using win32api to create shadows behind wx.PopupWindow,
# but this will result in *all* the popup windows in an application
# to have shadows behind them, even the user defined wx.PopupWindow
# that do not derive from FlatMenu.

import wx.aui as AUI
AuiPaneInfo = AUI.AuiPaneInfo

try:
    import aui as PyAUI
    PyAuiPaneInfo = PyAUI.AuiPaneInfo
except ImportError:
    pass

# Check for the new method in 2.7 (not present in 2.6.3.3)
if wx.VERSION_STRING < "2.7":
    wx.Rect.Contains = lambda self, point: wx.Rect.Inside(self, point)


wxEVT_FLAT_MENU_DISMISSED = wx.NewEventType()
wxEVT_FLAT_MENU_SELECTED = wx.wxEVT_COMMAND_MENU_SELECTED
wxEVT_FLAT_MENU_ITEM_MOUSE_OVER = wx.NewEventType()
wxEVT_FLAT_MENU_ITEM_MOUSE_OUT = wx.NewEventType()

EVT_FLAT_MENU_DISMISSED = wx.PyEventBinder(wxEVT_FLAT_MENU_DISMISSED, 1)
""" Used internally. """
EVT_FLAT_MENU_SELECTED = wx.PyEventBinder(wxEVT_FLAT_MENU_SELECTED, 2)
""" Fires the wx.EVT_MENU event for `FlatMenu`. """
EVT_FLAT_MENU_ITEM_MOUSE_OUT = wx.PyEventBinder(wxEVT_FLAT_MENU_ITEM_MOUSE_OUT, 1)
""" Fires an event when the mouse leaves a `FlatMenuItem`. """
EVT_FLAT_MENU_ITEM_MOUSE_OVER = wx.PyEventBinder(wxEVT_FLAT_MENU_ITEM_MOUSE_OVER, 1)
""" Fires an event when the mouse enters a `FlatMenuItem`. """


def GetAccelIndex(label):
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


def ConvertToMonochrome(bmp):
    """
    Converts a bitmap to monochrome colour.

    :param `bmp`: a valid `wx.Bitmap` object.
    """

    mem_dc = wx.MemoryDC()
    shadow = wx.EmptyBitmap(bmp.GetWidth(), bmp.GetHeight())
    mem_dc.SelectObject(shadow)
    mem_dc.DrawBitmap(bmp, 0, 0, True)
    mem_dc.SelectObject(wx.NullBitmap)
    img = shadow.ConvertToImage()
    img = img.ConvertToMono(0, 0, 0)
    
    # we now have black where the original bmp was drawn,
    # white elsewhere
    shadow = wx.BitmapFromImage(img)
    shadow.SetMask(wx.Mask(shadow, wx.BLACK))

    # Convert the black to grey
    tmp = wx.EmptyBitmap(bmp.GetWidth(), bmp.GetHeight())
    col = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)
    mem_dc.SelectObject(tmp)
    mem_dc.SetPen(wx.Pen(col))
    mem_dc.SetBrush(wx.Brush(col))
    mem_dc.DrawRectangle(0, 0, bmp.GetWidth(), bmp.GetHeight())
    mem_dc.DrawBitmap(shadow, 0, 0, True)   # now contains a bitmap with grey where the image was, white elsewhere
    mem_dc.SelectObject(wx.NullBitmap)
    shadow = tmp
    shadow.SetMask(wx.Mask(shadow, wx.WHITE)) 

    return shadow


# ---------------------------------------------------------------------------- #
# Class FMRendererMgr
# ---------------------------------------------------------------------------- #

class FMRendererMgr(object):
    """
    This class represents a manager that handles all the renderers defined. 
    Every instance of this class will share the same state, so everyone can
    instantiate their own and a call to L{FMRendererMgr.SetTheme} anywhere will affect everyone. 
    """

    def __new__(cls, *p, **k):
        if not '_instance' in cls.__dict__:
            cls._instance = object.__new__(cls)
        return cls._instance    


    def __init__(self):
        """ Default class constructor. """
   
        # If we have already initialized don't do it again. There is only one 
        # FMRendererMgr process-wide.

        if hasattr(self, '_alreadyInitialized'):
            return

        self._alreadyInitialized = True
       
        self._currentTheme = StyleDefault
        self._renderers = []
        self._renderers.append(FMRenderer())
        self._renderers.append(FMRendererXP())
        self._renderers.append(FMRendererMSOffice2007())
        self._renderers.append(FMRendererVista())        
        
        
    def GetRenderer(self):
        """ Returns the current theme's renderer. """
        
        return self._renderers[self._currentTheme]
    
    
    def AddRenderer(self, renderer):
        """
        Adds a user defined custom renderer.

        :param `renderer`: a class derived from L{FMRenderer}.
        """
        
        lastRenderer = len(self._renderers)
        self._renderers.append(renderer)
        
        return lastRenderer

    
    def SetTheme(self, theme):
        """
        Sets the current theme.

        :param `theme`: an integer specifying the theme to use.
        """
        
        if theme < 0 or theme > len(self._renderers):
            raise ValueError("Error invalid theme specified.")
        
        self._currentTheme = theme


# ---------------------------------------------------------------------------- #
# Class FMRenderer
# ---------------------------------------------------------------------------- #

class FMRenderer(object):
    """
    Base class for the L{FlatMenu} renderers. This class implements the common 
    methods of all the renderers.
    """
    
    def __init__(self):
        """ Default class constructor. """

        self.separatorHeight = 5
        self.drawLeftMargin = False
        self.highlightCheckAndRadio = False
        self.scrollBarButtons = False   # Display scrollbar buttons if the menu doesn't fit on the screen
                                        # otherwise default to up and down arrow menu items
        
        self.itemTextColourDisabled = ArtManager.Get().LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_GRAYTEXT), 30)
       
        # Background Colours
        self.menuFaceColour     = wx.WHITE
        self.menuBarFaceColour  = ArtManager.Get().LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE), 80)
        
        self.menuBarFocusFaceColour     = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.menuBarFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.menuBarPressedFaceColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.menuBarPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        
        self.menuFocusFaceColour     = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.menuFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.menuPressedFaceColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.menuPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        
        self.buttonFaceColour          = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.buttonBorderColour        = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.buttonFocusFaceColour     = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.buttonFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.buttonPressedFaceColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.buttonPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        
        # create wxBitmaps from the xpm's
        self._rightBottomCorner = self.ConvertToBitmap(shadow_center_xpm, shadow_center_alpha)
        self._bottom = self.ConvertToBitmap(shadow_bottom_xpm, shadow_bottom_alpha)
        self._bottomLeft = self.ConvertToBitmap(shadow_bottom_left_xpm, shadow_bottom_left_alpha)
        self._rightTop = self.ConvertToBitmap(shadow_right_top_xpm, shadow_right_top_alpha)
        self._right = self.ConvertToBitmap(shadow_right_xpm, shadow_right_alpha)
       
        self._bitmaps = {}
        bmp = self.ConvertToBitmap(arrow_down, alpha=None)
        bmp.SetMask(wx.Mask(bmp, wx.Colour(0, 128, 128)))
        self._bitmaps.update({"arrow_down": bmp})

        bmp = self.ConvertToBitmap(arrow_up, alpha=None)
        bmp.SetMask(wx.Mask(bmp, wx.Colour(0, 128, 128)))
        self._bitmaps.update({"arrow_up": bmp})
        
        self._toolbarSeparatorBitmap = wx.NullBitmap
        self.raiseToolbar = False

        
    def SetMenuBarHighlightColour(self, colour):
        """
        Set the colour to highlight focus on the menu bar.

        :param `colour`: a valid instance of `wx.Colour`.
        """
        
        self.menuBarFocusFaceColour    = colour
        self.menuBarFocusBorderColour  = colour
        self.menuBarPressedFaceColour  = colour
        self.menuBarPressedBorderColour= colour

        
    def SetMenuHighlightColour(self,colour):
        """
        Set the colour to highlight focus on the menu.

        :param `colour`: a valid instance of `wx.Colour`.
        """
        
        self.menuFocusFaceColour    = colour
        self.menuFocusBorderColour  = colour
        self.menuPressedFaceColour     = colour
        self.menuPressedBorderColour   = colour

        
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
    
    
    def DrawLeftMargin(self, item, dc, menuRect):
        """
        Draws the menu left margin.

        :param `dc`: an instance of `wx.DC`;
        :param `menuRect`: the menu client rectangle.
        """

        raise Exception("This style doesn't support Drawing a Left Margin")

    
    def DrawToolbarSeparator(self, dc, rect):
        """
        Draws a separator inside the toolbar in L{FlatMenuBar}.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the bitmap's client rectangle.
        """
        
        # Place a separator bitmap
        bmp = wx.EmptyBitmap(rect.width, rect.height)
        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(bmp)
        mem_dc.SetPen(wx.BLACK_PEN)
        mem_dc.SetBrush(wx.BLACK_BRUSH)
    
        mem_dc.DrawRectangle(0, 0, bmp.GetWidth(), bmp.GetHeight())
    
        col = self.menuBarFaceColour
        col1 = ArtManager.Get().LightColour(col, 40)
        col2 = ArtManager.Get().LightColour(col, 70)
    
        mem_dc.SetPen(wx.Pen(col2))
        mem_dc.DrawLine(5, 0, 5, bmp.GetHeight())
    
        mem_dc.SetPen(wx.Pen(col1))
        mem_dc.DrawLine(6, 0, 6, bmp.GetHeight())
        
        mem_dc.SelectObject(wx.NullBitmap)
        bmp.SetMask(wx.Mask(bmp, wx.BLACK))
            
        dc.DrawBitmap(bmp, rect.x, rect.y, True)
            
    
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
            

    def DrawToolBarBg(self, dc, rect):
        """
        Draws the toolbar background

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the toolbar's client rectangle.
        """

        if not self.raiseToolbar:
            return

        dcsaver = DCSaver(dc)

        # fill with gradient
        colour = self.menuBarFaceColour
        
        dc.SetPen(wx.Pen(colour))
        dc.SetBrush(wx.Brush(colour))
    
        dc.DrawRectangleRect(rect)
        self.DrawBitmapShadow(dc, rect)

        
    def DrawSeparator(self, dc, xCoord, yCoord, textX, sepWidth):
        """
        Draws a separator inside a L{FlatMenu}.

        :param `dc`: an instance of `wx.DC`;
        :param `xCoord`: the current x position where to draw the separator;
        :param `yCoord`: the current y position where to draw the separator;
        :param `textX`: the menu item label x position;
        :param `sepWidth`: the width of the separator, in pixels.
        """
        
        dcsaver = DCSaver(dc)
        sepRect1 = wx.Rect(xCoord + textX, yCoord + 1, sepWidth/2, 1)
        sepRect2 = wx.Rect(xCoord + textX + sepWidth/2, yCoord + 1, sepWidth/2-1, 1)

        artMgr = ArtManager.Get()
        backColour = artMgr.GetMenuFaceColour()
        lightColour = wx.NamedColour("LIGHT GREY")
        
        artMgr.PaintStraightGradientBox(dc, sepRect1, backColour, lightColour, False)
        artMgr.PaintStraightGradientBox(dc, sepRect2, lightColour, backColour, False) 
        

    def DrawMenuItem(self, item, dc, xCoord, yCoord, imageMarginX, markerMarginX, textX, rightMarginX, selected=False):
        """
        Draws the menu item.

        :param `item`: `FlatMenuItem` instance;
        :param `dc`: an instance of `wx.DC`;
        :param `xCoord`: the current x position where to draw the menu;
        :param `yCoord`: the current y position where to draw the menu;
        :param `imageMarginX`: the spacing between the image and the menu border;
        :param `markerMarginX`: the spacing between the checkbox/radio marker and
         the menu border;
        :param `textX`: the menu item label x position;
        :param `rightMarginX`: the right margin between the text and the menu border;
        :param `selected`: ``True`` if this menu item is currentl hovered by the mouse,
         ``False`` otherwise.
        """
 
        borderXSize = item._parentMenu.GetBorderXWidth()
        itemHeight = item._parentMenu.GetItemHeight()
        menuWidth  = item._parentMenu.GetMenuWidth()

        # Define the item actual rectangle area
        itemRect = wx.Rect(xCoord, yCoord, menuWidth, itemHeight)

        # Define the drawing area 
        rect = wx.Rect(xCoord+2, yCoord, menuWidth - 4, itemHeight)

        # Draw the background
        backColour = self.menuFaceColour
        penColour  = backColour
        backBrush = wx.Brush(backColour)
        leftMarginWidth = item._parentMenu.GetLeftMarginWidth()
        
        pen = wx.Pen(penColour)
        dc.SetPen(pen)
        dc.SetBrush(backBrush)
        dc.DrawRectangleRect(rect)

        # Draw the left margin gradient
        if self.drawLeftMargin:
            self.DrawLeftMargin(item, dc, itemRect)

        # check if separator
        if item.IsSeparator():
            # Separator is a small grey line separating between menu items. 
            sepWidth = xCoord + menuWidth - textX - 1
            self.DrawSeparator(dc, xCoord, yCoord, textX, sepWidth)
            return
        
        # Keep the item rect
        item._rect = itemRect

        # Get the bitmap base on the item state (disabled, selected ..)
        bmp = item.GetSuitableBitmap(selected)
        
        # First we draw the selection rectangle
        if selected:
            self.DrawMenuButton(dc, rect.Deflate(1,0), ControlFocus)
            #copy.Inflate(0, menubar._spacer)

        if bmp.Ok():
        
            # Calculate the postion to place the image
            imgHeight = bmp.GetHeight()
            imgWidth  = bmp.GetWidth()

            if imageMarginX == 0:
                xx = rect.x + (leftMarginWidth - imgWidth)/2
            else:
                xx = rect.x + ((leftMarginWidth - rect.height) - imgWidth)/2 + rect.height

            yy = rect.y + (rect.height - imgHeight)/2
            dc.DrawBitmap(bmp, xx, yy, True)
        
        if item.GetKind() == wx.ITEM_CHECK:
        
            # Checkable item
            if item.IsChecked():
            
                # Draw surrounding rectangle around the selection box
                xx = rect.x + 1
                yy = rect.y + 1
                rr = wx.Rect(xx, yy, rect.height-2, rect.height-2)
                
                if not selected and self.highlightCheckAndRadio:
                    self.DrawButton(dc, rr, ControlFocus)

                dc.DrawBitmap(item._checkMarkBmp, rr.x + (rr.width - 16)/2, rr.y + (rr.height - 16)/2, True)

        if item.GetKind() == wx.ITEM_RADIO:
            
            # Checkable item
            if item.IsChecked():
                
                # Draw surrounding rectangle around the selection box
                xx = rect.x + 1
                yy = rect.y + 1
                rr = wx.Rect(xx, yy, rect.height-2, rect.height-2)

                if not selected and self.highlightCheckAndRadio:
                    self.DrawButton(dc, rr, ControlFocus)
                    
                dc.DrawBitmap(item._radioMarkBmp, rr.x + (rr.width - 16)/2, rr.y + (rr.height - 16)/2, True)

        # Draw text - without accelerators
        text = item.GetLabel()
        
        if text:

            font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            if selected:
                enabledTxtColour = colourutils.BestLabelColour(self.menuFocusFaceColour, bw=True)
            else:
                enabledTxtColour = colourutils.BestLabelColour(self.menuFaceColour, bw=True)
            disabledTxtColour = self.itemTextColourDisabled
            textColour = (item.IsEnabled() and [enabledTxtColour] or [disabledTxtColour])[0]

            dc.SetFont(font)
            w, h = dc.GetTextExtent(text)
            dc.SetTextForeground(textColour)

            if item._mnemonicIdx != wx.NOT_FOUND:
            
                # We divide the drawing to 3 parts
                text1 = text[0:item._mnemonicIdx]
                text2 = text[item._mnemonicIdx]
                text3 = text[item._mnemonicIdx+1:]

                w1, dummy = dc.GetTextExtent(text1)
                w2, dummy = dc.GetTextExtent(text2)
                w3, dummy = dc.GetTextExtent(text3)

                posx = xCoord + textX + borderXSize
                posy = (itemHeight - h)/2 + yCoord

                # Draw first part 
                dc.DrawText(text1, posx, posy)

                # mnemonic 
                if "__WXGTK__" not in wx.Platform:
                    font.SetUnderlined(True)
                    dc.SetFont(font)

                posx += w1
                dc.DrawText(text2, posx, posy)

                # last part
                font.SetUnderlined(False)
                dc.SetFont(font)
                posx += w2
                dc.DrawText(text3, posx, posy)
            
            else:
            
                w, h = dc.GetTextExtent(text)
                dc.DrawText(text, xCoord + textX + borderXSize, (itemHeight - h)/2 + yCoord)
            
        
        # Now draw accelerator
        # Accelerators are aligned to the right
        if item.GetAccelString():
        
            accelWidth, accelHeight = dc.GetTextExtent(item.GetAccelString())
            dc.DrawText(item.GetAccelString(), xCoord + rightMarginX - accelWidth, (itemHeight - accelHeight)/2 + yCoord)
        
        # Check if this item has sub-menu - if it does, draw 
        # right arrow on the right margin
        if item.GetSubMenu():
        
            # Draw arrow 
            rightArrowBmp = wx.BitmapFromXPMData(menu_right_arrow_xpm)
            rightArrowBmp.SetMask(wx.Mask(rightArrowBmp, wx.WHITE))

            xx = xCoord + rightMarginX + borderXSize 
            rr = wx.Rect(xx, rect.y + 1, rect.height-2, rect.height-2)
            dc.DrawBitmap(rightArrowBmp, rr.x + 4, rr.y +(rr.height-16)/2, True)

        
    def DrawMenuBarButton(self, dc, rect, state):
        """
        Draws the highlight on a FlatMenuBar

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        """
        
        # switch according to the status
        if state == ControlFocus:
            penColour   = self.menuBarFocusBorderColour
            brushColour = self.menuBarFocusFaceColour
        elif state == ControlPressed: 
            penColour   = self.menuBarPressedBorderColour
            brushColour = self.menuBarPressedFaceColour
            
        dcsaver = DCSaver(dc)
        dc.SetPen(wx.Pen(penColour))
        dc.SetBrush(wx.Brush(brushColour))
        dc.DrawRectangleRect(rect)


    def DrawMenuButton(self, dc, rect, state):
        """
        Draws the highlight on a FlatMenu

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        """
        
        # switch according to the status
        if state == ControlFocus:
            penColour   = self.menuFocusBorderColour
            brushColour = self.menuFocusFaceColour
        elif state == ControlPressed: 
            penColour   = self.menuPressedBorderColour
            brushColour = self.menuPressedFaceColour
            
        dcsaver = DCSaver(dc)
        dc.SetPen(wx.Pen(penColour))
        dc.SetBrush(wx.Brush(brushColour))
        dc.DrawRectangleRect(rect)
        

    def DrawScrollButton(self, dc, rect, state):
        """
        Draws the scroll button

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        """
        
        if not self.scrollBarButtons:
            return

        colour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        colour = ArtManager.Get().LightColour(colour, 30)
        
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

        
    def DrawButton(self, dc, rect, state, colour=None):
        """
        Draws a button

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        """
        
        # switch according to the status
        if state == ControlFocus:
            if colour == None:
                penColour   = self.buttonFocusBorderColour
                brushColour = self.buttonFocusFaceColour
            else:
                penColour   = colour
                brushColour = ArtManager.Get().LightColour(colour, 75)
                
        elif state == ControlPressed: 
            if colour == None:
                penColour   = self.buttonPressedBorderColour
                brushColour = self.buttonPressedFaceColour
            else:
                penColour   = colour
                brushColour = ArtManager.Get().LightColour(colour, 60)
        else:
            if colour == None:
                penColour   = self.buttonBorderColour
                brushColour = self.buttonFaceColour
            else:
                penColour   = colour
                brushColour = ArtManager.Get().LightColour(colour, 75)
            
        dcsaver = DCSaver(dc)
        dc.SetPen(wx.Pen(penColour))
        dc.SetBrush(wx.Brush(brushColour))
        dc.DrawRectangleRect(rect)


    def DrawMenuBarBackground(self, dc, rect):
        """
        Draws the menu bar background colour according to the menubar.GetBackgroundColour

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the menu bar's client rectangle.
        """

        dcsaver = DCSaver(dc)

        # fill with gradient
        colour = self.menuBarFaceColour

        dc.SetPen(wx.Pen(colour))
        dc.SetBrush(wx.Brush(colour))
        dc.DrawRectangleRect(rect)


    def DrawMenuBar(self, menubar, dc):
        """
        Draws everything for L{FlatMenuBar}.

        :param `dc`: an instance of `wx.DC`.
        """

        #artMgr = ArtManager.Get()
        fnt = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        textColour = colourutils.BestLabelColour(menubar.GetBackgroundColour(), bw=True)
        highlightTextColour = colourutils.BestLabelColour(self.menuBarFocusFaceColour, bw=True)

        dc.SetFont(fnt)
        dc.SetTextForeground(textColour)
        
        clientRect = menubar.GetClientRect()

        self.DrawMenuBarBackground(dc, clientRect)

        padding, dummy = dc.GetTextExtent("W") 
        
        posx = 0
        posy = menubar._margin

        # ---------------------------------------------------------------------------
        # Draw as much items as we can if the screen is not wide enough, add all
        # missing items to a drop down menu
        # ---------------------------------------------------------------------------
        menuBarRect = menubar.GetClientRect()

        # mark all items as non-visibles at first
        for item in menubar._items:
            item.SetRect(wx.Rect())

        for item in menubar._items:

            # Handle accelerator ('&')        
            title = item.GetTitle()

            fixedText = title
            location, labelOnly = GetAccelIndex(fixedText)
            
            # Get the menu item rect
            textWidth, textHeight = dc.GetTextExtent(fixedText)
            #rect = wx.Rect(posx+menubar._spacer/2, posy, textWidth, textHeight)
            rect = wx.Rect(posx+padding/2, posy, textWidth, textHeight)

            # Can we draw more??
            # the +DROP_DOWN_ARROW_WIDTH  is the width of the drop down arrow
            if posx + rect.width + DROP_DOWN_ARROW_WIDTH >= menuBarRect.width:
                break
            
            # In this style the button highlight includes the menubar margin
            button_rect = wx.Rect(*rect)
            button_rect.height = menubar._menuBarHeight
            #button_rect.width = rect.width + menubar._spacer
            button_rect.width = rect.width + padding
            button_rect.x = posx 
            button_rect.y = 0
            
            # Keep the item rectangle, will be used later in functions such
            # as 'OnLeftDown', 'OnMouseMove'
            copy = wx.Rect(*button_rect)
            #copy.Inflate(0, menubar._spacer)
            item.SetRect(copy)
           
            selected = False
            if item.GetState() == ControlFocus:
                self.DrawMenuBarButton(dc, button_rect, ControlFocus)
                dc.SetTextForeground(highlightTextColour)
                selected = True
            else:
                dc.SetTextForeground(textColour)

            ww, hh = dc.GetTextExtent(labelOnly)
            textOffset = (rect.width - ww) / 2

            if not menubar._isLCD and item.GetTextBitmap().Ok() and not selected:
                dc.DrawBitmap(item.GetTextBitmap(), rect.x, rect.y, True)
            elif not menubar._isLCD and item.GetSelectedTextBitmap().Ok() and selected:
                dc.DrawBitmap(item.GetSelectedTextBitmap(), rect.x, rect.y, True)
            else:
                if not menubar._isLCD:
                    # Draw the text on a bitmap using memory dc, 
                    # so on following calls we will use this bitmap instead
                    # of calculating everything from scratch
                    bmp = wx.EmptyBitmap(rect.width, rect.height)
                    memDc = wx.MemoryDC()
                    memDc.SelectObject(bmp)
                    if selected:
                        memDc.SetTextForeground(highlightTextColour)
                    else:
                        memDc.SetTextForeground(textColour)

                    # Fill the bitmap with the masking colour
                    memDc.SetPen(wx.Pen(wx.Colour(255, 0, 0)) )
                    memDc.SetBrush(wx.Brush(wx.Colour(255, 0, 0)) )
                    memDc.DrawRectangle(0, 0, rect.width, rect.height)
                    memDc.SetFont(fnt)

                if location == wx.NOT_FOUND or location >= len(fixedText):
                    # draw the text
                    if not menubar._isLCD:
                        memDc.DrawText(title, textOffset, 0)
                    dc.DrawText(title, rect.x + textOffset, rect.y)
                
                else:
                    
                    # underline the first '&'
                    before = labelOnly[0:location]
                    underlineLetter = labelOnly[location] 
                    after = labelOnly[location+1:]

                    # before
                    if not menubar._isLCD:
                        memDc.DrawText(before, textOffset, 0)
                    dc.DrawText(before, rect.x + textOffset, rect.y)

                    # underlineLetter
                    if "__WXGTK__" not in wx.Platform:
                        w1, h = dc.GetTextExtent(before)
                        fnt.SetUnderlined(True)
                        dc.SetFont(fnt)
                        dc.DrawText(underlineLetter, rect.x + w1 + textOffset, rect.y)
                        if not menubar._isLCD:
                            memDc.SetFont(fnt)
                            memDc.DrawText(underlineLetter, textOffset + w1, 0)
                        
                    else:
                        w1, h = dc.GetTextExtent(before)
                        dc.DrawText(underlineLetter, rect.x + w1 + textOffset, rect.y)
                        if not menubar._isLCD:
                            memDc.DrawText(underlineLetter, textOffset + w1, 0)

                        # Draw the underline ourselves since using the Underline in GTK, 
                        # causes the line to be too close to the letter
                        
                        uderlineLetterW, uderlineLetterH = dc.GetTextExtent(underlineLetter)
                        dc.DrawLine(rect.x + w1 + textOffset, rect.y + uderlineLetterH - 2,
                                    rect.x + w1 + textOffset + uderlineLetterW, rect.y + uderlineLetterH - 2)

                    # after
                    w2, h = dc.GetTextExtent(underlineLetter)
                    fnt.SetUnderlined(False)
                    dc.SetFont(fnt)                
                    dc.DrawText(after, rect.x + w1 + w2 + textOffset, rect.y)
                    if not menubar._isLCD:
                        memDc.SetFont(fnt)
                        memDc.DrawText(after,  w1 + w2 + textOffset, 0)

                    if not menubar._isLCD:
                        memDc.SelectObject(wx.NullBitmap)
                        # Set masking colour to the bitmap
                        bmp.SetMask(wx.Mask(bmp, wx.Colour(255, 0, 0)))
                        if selected:
                            item.SetSelectedTextBitmap(bmp)                        
                        else:
                            item.SetTextBitmap(bmp)                        
                    
            posx += rect.width + padding # + menubar._spacer

        # Get a backgroud image of the more menu button
        moreMenubtnBgBmpRect = wx.Rect(*menubar.GetMoreMenuButtonRect())
        if not menubar._moreMenuBgBmp:
            menubar._moreMenuBgBmp = wx.EmptyBitmap(moreMenubtnBgBmpRect.width, moreMenubtnBgBmpRect.height)

        if menubar._showToolbar and len(menubar._tbButtons) > 0:
            rectX      = 0
            rectWidth  = clientRect.width - moreMenubtnBgBmpRect.width 
            if len(menubar._items) == 0:
                rectHeight = clientRect.height
                rectY      = 0
            else:
                rectHeight = clientRect.height - menubar._menuBarHeight
                rectY      = menubar._menuBarHeight
            rr = wx.Rect(rectX, rectY, rectWidth, rectHeight)
            self.DrawToolBarBg(dc, rr)
            menubar.DrawToolbar(dc, rr)

        if menubar._showCustomize or menubar.GetInvisibleMenuItemCount() > 0 or  menubar.GetInvisibleToolbarItemCount() > 0:
            memDc = wx.MemoryDC()
            memDc.SelectObject(menubar._moreMenuBgBmp)
            try:
                memDc.Blit(0, 0, menubar._moreMenuBgBmp.GetWidth(), menubar._moreMenuBgBmp.GetHeight(), dc,
                           moreMenubtnBgBmpRect.x, moreMenubtnBgBmpRect.y)
            except:
                pass
            memDc.SelectObject(wx.NullBitmap)

            # Draw the drop down arrow button
            menubar.DrawMoreButton(dc, 0, menubar._dropDownButtonState)
            # Set the button rect
            menubar._dropDownButtonArea = moreMenubtnBgBmpRect

            
    def DrawMenu(self, flatmenu, dc):
        """
        Draws the menu.

        :param `flatmenu`: the L{FlatMenu} instance we need to paint;
        :param `dc`: an instance of `wx.DC`.
        """
        
        menuRect = flatmenu.GetClientRect()
        menuBmp = wx.EmptyBitmap(menuRect.width, menuRect.height)

        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(menuBmp)

        # colour the menu face with background colour
        backColour = self.menuFaceColour
        penColour  = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)

        backBrush = wx.Brush(backColour)
        pen = wx.Pen(penColour)

        mem_dc.SetPen(pen)
        mem_dc.SetBrush(backBrush)
        mem_dc.DrawRectangleRect(menuRect)
        
        # draw items
        posy = 3
        nItems = len(flatmenu._itemsArr)

        # make all items as non-visible first
        for item in flatmenu._itemsArr:
            item.Show(False)

        visibleItems = 0
        screenHeight = wx.SystemSettings_GetMetric(wx.SYS_SCREEN_Y)

        numCols = flatmenu.GetNumberColumns()
        switch, posx, index = 1e6, 0, 0
        if numCols > 1:
            switch = int(math.ceil((nItems - flatmenu._first)/float(numCols)))
            
        # If we have to scroll and are not using the scroll bar buttons we need to draw
        # the scroll up menu item at the top.
        if not self.scrollBarButtons and flatmenu._showScrollButtons:
            posy += flatmenu.GetItemHeight()
            
        for nCount in xrange(flatmenu._first, nItems):

            visibleItems += 1
            item = flatmenu._itemsArr[nCount]
            self.DrawMenuItem(item, mem_dc,
                          posx,
                          posy,     
                          flatmenu._imgMarginX,
                          flatmenu._markerMarginX,
                          flatmenu._textX, 
                          flatmenu._rightMarginPosX,
                          nCount == flatmenu._selectedItem 
                          )
            posy += item.GetHeight()
            item.Show()
            
            if visibleItems >= switch:
                posy = 2
                index += 1
                posx = flatmenu._menuWidth*index
                visibleItems = 0

            # make sure we draw only visible items
            pp = flatmenu.ClientToScreen(wx.Point(0, posy))
            
            menuBottom = (self.scrollBarButtons and [pp.y] or [pp.y + flatmenu.GetItemHeight()*2])[0]
            
            if menuBottom > screenHeight:
                break

        if flatmenu._showScrollButtons:
            if flatmenu._upButton:
                flatmenu._upButton.Draw(mem_dc)
            if flatmenu._downButton:
                flatmenu._downButton.Draw(mem_dc)

        dc.Blit(0, 0, menuBmp.GetWidth(), menuBmp.GetHeight(), mem_dc, 0, 0)

                
# ---------------------------------------------------------------------------- #
# Class FMRendererMSOffice2007
# ---------------------------------------------------------------------------- #

class FMRendererMSOffice2007(FMRenderer):
    """ Windows Office 2007 style. """
    
    def __init__(self):
        """ Default class constructor. """

        FMRenderer.__init__(self)
       
        self.drawLeftMargin = True
        self.separatorHeight = 3
        self.highlightCheckAndRadio = True
        self.scrollBarButtons = True   # Display scrollbar buttons if the menu doesn't fit on the screen
        
        self.menuBarFaceColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
        
        self.buttonBorderColour        = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.buttonFaceColour          = ArtManager.Get().LightColour(self.buttonBorderColour, 75)
        self.buttonFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.buttonFocusFaceColour     = ArtManager.Get().LightColour(self.buttonFocusBorderColour, 75)
        self.buttonPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.buttonPressedFaceColour   = ArtManager.Get().LightColour(self.buttonPressedBorderColour, 60)
        
        self.menuFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuFocusFaceColour     = ArtManager.Get().LightColour(self.buttonFocusBorderColour, 75)
        self.menuPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuPressedFaceColour   = ArtManager.Get().LightColour(self.buttonPressedBorderColour, 60)
        
        self.menuBarFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuBarFocusFaceColour     = ArtManager.Get().LightColour(self.buttonFocusBorderColour, 75)
        self.menuBarPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuBarPressedFaceColour   = ArtManager.Get().LightColour(self.buttonPressedBorderColour, 60)


    def DrawLeftMargin(self, item, dc, menuRect):
        """
        Draws the menu left margin.

        :param `dc`: an instance of `wx.DC`;
        :param `menuRect`: the menu client rectangle.
        """

        # Construct the margin rectangle
        marginRect = wx.Rect(menuRect.x+1, menuRect.y, item._parentMenu.GetLeftMarginWidth(), menuRect.height)

        # Set the gradient colours
        artMgr = ArtManager.Get()
        faceColour = self.menuFaceColour
        
        dcsaver = DCSaver(dc)
        marginColour = artMgr.DarkColour(faceColour, 5)
        dc.SetPen(wx.Pen(marginColour))
        dc.SetBrush(wx.Brush(marginColour))
        dc.DrawRectangleRect(marginRect)

        dc.SetPen(wx.WHITE_PEN)
        dc.DrawLine(marginRect.x + marginRect.width, marginRect.y, marginRect.x + marginRect.width, marginRect.y + marginRect.height)

        borderColour = artMgr.DarkColour(faceColour, 10)
        dc.SetPen(wx.Pen(borderColour))
        dc.DrawLine(marginRect.x + marginRect.width-1, marginRect.y, marginRect.x + marginRect.width-1, marginRect.y + marginRect.height)


    def DrawMenuButton(self, dc, rect, state):
        """
        Draws the highlight on a L{FlatMenu}.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state.
        """
        
        self.DrawButton(dc, rect, state)

        
    def DrawMenuBarButton(self, dc, rect, state):
        """
        Draws the highlight on a L{FlatMenuBar}.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state.
        """
        
        self.DrawButton(dc, rect, state)

        
    def DrawButton(self, dc, rect, state, colour=None):
        """
        Draws a button using the Office 2007 theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `colour`: a valid `wx.Colour` instance.
        """

        colour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        colour = ArtManager.Get().LightColour(colour, 30)
        self.DrawButtonColour(dc, rect, state, colour)


    def DrawButtonColour(self, dc, rect, state, colour):
        """
        Draws a button using the Office 2007 theme.

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


    def DrawMenuBarBackground(self, dc, rect):
        """
        Draws the menu bar background according to the active theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the menu bar's client rectangle.
        """

        # Keep old pen and brush
        dcsaver = DCSaver(dc)
        artMgr = ArtManager.Get()
        baseColour = self.menuBarFaceColour

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

        topStartColour    = artMgr.LightColour(baseColour, 90)
        topEndColour      = artMgr.LightColour(baseColour, 60)
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
        
        baseColour = self.menuBarFaceColour
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
# Class FMRendererVista
# ---------------------------------------------------------------------------- #

class FMRendererVista(FMRendererMSOffice2007):
    """ Windows Vista-like style. """
    
    def __init__(self):
        """ Default class constructor. """

        FMRendererMSOffice2007.__init__(self)


    def DrawButtonColour(self, dc, rect, state, colour):
        """
        Draws a button using the Vista theme.

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the button's client rectangle;
        :param `state`: the button state;
        :param `colour`: a valid `wx.Colour` instance.
        """

        artMgr = ArtManager.Get()
        
        # Keep old pen and brush
        dcsaver = DCSaver(dc)

        outer = rgbSelectOuter
        inner = rgbSelectInner
        top = rgbSelectTop
        bottom = rgbSelectBottom

        bdrRect = wx.Rect(*rect)
        filRect = wx.Rect(*rect)
        filRect.Deflate(1,1)
        
        r1, g1, b1 = int(top.Red()), int(top.Green()), int(top.Blue())
        r2, g2, b2 = int(bottom.Red()), int(bottom.Green()), int(bottom.Blue())
        dc.GradientFillLinear(filRect, top, bottom, wx.SOUTH)
        
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(wx.Pen(outer))
        dc.DrawRoundedRectangleRect(bdrRect, 3)
        bdrRect.Deflate(1, 1)
        dc.SetPen(wx.Pen(inner))
        dc.DrawRoundedRectangleRect(bdrRect, 2)

        
# ---------------------------------------------------------------------------- #
# Class FMRendererXP
# ---------------------------------------------------------------------------- #

class FMRendererXP(FMRenderer):
    """ Xp-Style renderer. """
    
    def __init__(self):
        """ Default class constructor. """

        FMRenderer.__init__(self)
        
        self.drawLeftMargin = True
        self.separatorHeight = 3
        self.highlightCheckAndRadio = True
        self.scrollBarButtons = True   # Display scrollbar buttons if the menu doesn't fit on the screen
        
        self.buttonBorderColour        = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.buttonFaceColour          = ArtManager.Get().LightColour(self.buttonBorderColour, 75)
        self.buttonFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.buttonFocusFaceColour     = ArtManager.Get().LightColour(self.buttonFocusBorderColour, 75)
        self.buttonPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.buttonPressedFaceColour   = ArtManager.Get().LightColour(self.buttonPressedBorderColour, 60)
        
        self.menuFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuFocusFaceColour     = ArtManager.Get().LightColour(self.buttonFocusBorderColour, 75)
        self.menuPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuPressedFaceColour   = ArtManager.Get().LightColour(self.buttonPressedBorderColour, 60)
        
        self.menuBarFocusBorderColour   = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuBarFocusFaceColour     = ArtManager.Get().LightColour(self.buttonFocusBorderColour, 75)
        self.menuBarPressedBorderColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self.menuBarPressedFaceColour   = ArtManager.Get().LightColour(self.buttonPressedBorderColour, 60)


    def DrawLeftMargin(self, item, dc, menuRect):
        """
        Draws the menu left margin.

        :param `dc`: an instance of `wx.DC`;
        :param `menuRect`: the menu client rectangle.
        """

        # Construct the margin rectangle
        marginRect = wx.Rect(menuRect.x+1, menuRect.y, item._parentMenu.GetLeftMarginWidth(), menuRect.height)

        # Set the gradient colours
        artMgr = ArtManager.Get()
        faceColour = self.menuFaceColour
        
        startColour = artMgr.DarkColour(faceColour, 20)
        endColour   = faceColour
        artMgr.PaintStraightGradientBox(dc, marginRect, startColour, endColour, False)


    def DrawMenuBarBackground(self, dc, rect):
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
# Class FlatMenuEvent
# ---------------------------------------------------------------------------- #

class FlatMenuEvent(wx.PyCommandEvent):
    """
    Event class that supports the L{FlatMenu}-compatible event called
    ``EVT_FLAT_MENU_SELECTED``.
    """
        
    def __init__(self, eventType, eventId=1, nSel=-1, nOldSel=-1):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier;
        :param `nSel`: the current selection;
        :param `nOldSel`: the old selection.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self._eventType = eventType


# ---------------------------------------------------------------------------- #
# Class MenuEntryInfo
# ---------------------------------------------------------------------------- #

class MenuEntryInfo(object):
    """
    Internal class which holds information about a menu.
    """

    def __init__(self, titleOrMenu="", menu=None, state=ControlNormal, cmd=wx.ID_ANY):
        """
        Default class constructor.

        Used internally. Do not call it in your code!

        :param `titleOrMenu`: if it is a string, it represents the new menu label,
         otherwise it is another instance of L{MenuEntryInfo} from which the attributes
         are copied;
        :param `menu`: the associated L{FlatMenu} object;
        :param `state`: the menu item state. This can be one of the following:

         ==================== ======= ==========================
         Item State            Value  Description
         ==================== ======= ==========================         
         ``ControlPressed``         0 The item is pressed
         ``ControlFocus``           1 The item is focused
         ``ControlDisabled``        2 The item is disabled
         ``ControlNormal``          3 Normal state
         ==================== ======= ==========================

        :param `cmd`: the menu accelerator identifier.
        """

        if isinstance(titleOrMenu, basestring):

            self._title = titleOrMenu
            self._menu = menu

            self._rect = wx.Rect()
            self._state = state
            if cmd == wx.ID_ANY:
                cmd = wx.NewId()
                
            self._cmd = cmd             # the menu itself accelerator id

        else:
            
            self._title = titleOrMenu._title
            self._menu = titleOrMenu._menu
            self._rect = titleOrMenu._rect
            self._state = titleOrMenu._state
            self._cmd = titleOrMenu._cmd

        self._textBmp = wx.NullBitmap
        self._textSelectedBmp = wx.NullBitmap
        

    def GetTitle(self):
        """ Returns the associated menu title. """

        return self._title 
    

    def GetMenu(self):
        """ Returns the associated menu. """
        
        return self._menu 
    

    def SetRect(self, rect):
        """
        Sets the associated menu client rectangle.

        :param `rect`: an instance of `wx.Rect`, representing the menu client rectangle.
        """

        self._rect = rect


    def GetRect(self):
        """ Returns the associated menu client rectangle. """

        return self._rect
    

    def SetState(self, state):
        """
        Sets the associated menu state.

        :param `state`: the menu item state. This can be one of the following:

         ==================== ======= ==========================
         Item State            Value  Description
         ==================== ======= ==========================         
         ``ControlPressed``         0 The item is pressed
         ``ControlFocus``           1 The item is focused
         ``ControlDisabled``        2 The item is disabled
         ``ControlNormal``          3 Normal state
         ==================== ======= ==========================
        """

        self._state = state


    def GetState(self):
        """
        Returns the associated menu state.

        :see: L{SetState} for a list of valid menu states.
        """

        return self._state
    

    def SetTextBitmap(self, bmp):
        """
        Sets the associated menu bitmap.

        :param `bmp`: a valid `wx.Bitmap` object.
        """

        self._textBmp = bmp
        
    def SetSelectedTextBitmap(self, bmp):
        """
        Sets the associated selected menu bitmap.

        :param `bmp`: a valid `wx.Bitmap` object.
        """

        self._textSelectedBmp = bmp
    

    def GetTextBitmap(self):
        """ Returns the associated menu bitmap. """

        return self._textBmp
    
    def GetSelectedTextBitmap(self):
        """ Returns the associated selected menu bitmap. """

        return self._textSelectedBmp

  
    def GetCmdId(self):
        """ Returns the associated menu accelerator identifier. """

        return self._cmd


# ---------------------------------------------------------------------------- #
# Class StatusBarTimer
# ---------------------------------------------------------------------------- #

class StatusBarTimer(wx.Timer):
    """ Timer used for deleting `wx.StatusBar` long help after ``_DELAY`` seconds. """

    def __init__(self, owner):
        """
        Default class constructor.
        For internal use: do not call it in your code!

        :param `owner`: the `wx.Timer` owner (L{FlatMenuBar}).
        """
        
        wx.Timer.__init__(self)
        self._owner = owner        


    def Notify(self):
        """ The timer has expired. """

        self._owner.OnStatusBarTimer()


# ---------------------------------------------------------------------------- #
# Class FlatMenuBar
# ---------------------------------------------------------------------------- #

class FlatMenuBar(wx.Panel):
    """
    Implements the generic owner-drawn menu bar for L{FlatMenu}.
    """

    def __init__(self, parent, id=wx.ID_ANY, iconSize=SmallIcons,
                 spacer=SPACER, options=FM_OPT_SHOW_CUSTOMIZE|FM_OPT_IS_LCD):
        """
        Default class constructor.

        :param `parent`: the menu bar parent
        :param `id`: the window identifier. If ``wx.ID_ANY``, will automatically create an identifier;
        :param `iconSize`: size of the icons in the toolbar. This can be one of the
         following values (in pixels):

         ==================== ======= =============================
         `iconSize` Bit        Value  Description
         ==================== ======= =============================
         ``LargeIcons``            32 Use large 32x32 icons
         ``SmallIcons``            16 Use standard 16x16 icons
         ==================== ======= =============================
         
        :param `spacer`: the space between the menu bar text and the menu bar border;
        :param `options`: a combination of the following bits:

         ========================= ========= =============================
         `options` Bit             Hex Value  Description
         ========================= ========= =============================
         ``FM_OPT_IS_LCD``               0x1 Use this style if your computer uses a LCD screen
         ``FM_OPT_MINIBAR``              0x2 Use this if you plan to use toolbar only
         ``FM_OPT_SHOW_CUSTOMIZE``       0x4 Show "customize link" in more menus, you will need to write your own handler. See demo.
         ``FM_OPT_SHOW_TOOLBAR``         0x8 Set this option is you are planing to use the toolbar
         ========================= ========= =============================
         
        """

        self._rendererMgr = FMRendererMgr()
        self._parent = parent
        self._curretHiliteItem = -1

        self._items = []
        self._dropDownButtonArea = wx.Rect()
        self._tbIconSize = iconSize
        self._tbButtons = []
        self._interval = 20      # 20 milliseconds
        self._showTooltip = -1
        
        self._haveTip = False
        self._statusTimer = None
        self._spacer = SPACER
        self._margin = spacer
        self._toolbarSpacer = TOOLBAR_SPACER
        self._toolbarMargin = TOOLBAR_MARGIN
        
        self._showToolbar = options & FM_OPT_SHOW_TOOLBAR
        self._showCustomize = options & FM_OPT_SHOW_CUSTOMIZE
        self._isLCD = options & FM_OPT_IS_LCD
        self._isMinibar = options & FM_OPT_MINIBAR
        self._options = options
        
        self._dropDownButtonState = ControlNormal
        self._moreMenu = None
        self._dlg = None
        self._tbMenu = None
        self._moreMenuBgBmp = None
        self._lastRadioGroup = 0
        self._mgr = None

        self._barHeight = 0
        self._menuBarHeight = 0
        self.SetBarHeight()

        wx.Panel.__init__(self, parent, id, size=(-1, self._barHeight), style=wx.WANTS_CHARS)

        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(EVT_FLAT_MENU_DISMISSED, self.OnMenuDismissed)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveMenuBar)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_IDLE, self.OnIdle)
        
        if "__WXGTK__" in wx.Platform:
            self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)

        self.SetFocus()
        
        # start the stop watch
        self._watch = wx.StopWatch()
        self._watch.Start()


    def Append(self, menu, title):
        """
        Adds the item to the end of the menu bar.

        :param `menu`: the menu to which we are appending a new item;
        :param `title`: the menu item label.
        """

        menu._menuBarFullTitle = title
        position, label = GetAccelIndex(title)
        menu._menuBarLabelOnly = label

        return self.Insert(len(self._items), menu, title)


    def OnIdle(self, event):
        """
        Handles the ``wx.EVT_IDLE`` event for L{FlatMenuBar}.

        :param `event`: a `wx.IdleEvent` event to be processed.
        """

        refresh = False
        
        if self._watch.Time() > self._interval:
        
            # it is time to process UpdateUIEvents
            for but in self._tbButtons:
                event = wx.UpdateUIEvent(but._tbItem.GetId())
                event.Enable(but._tbItem.IsEnabled())
                event.SetText(but._tbItem.GetLabel())
                event.SetEventObject(self)

                self.GetEventHandler().ProcessEvent(event)

                if but._tbItem.GetLabel() != event.GetText() or but._tbItem.IsEnabled() != event.GetEnabled():
                    refresh = True

                but._tbItem.SetLabel(event.GetText())
                but._tbItem.Enable(event.GetEnabled())
            
            self._watch.Start() # Reset the timer
        
        # we need to update the menu bar
        if refresh:
            self.Refresh()


    def SetBarHeight(self):
        """ Recalculates the L{FlatMenuBar} height when its settings change. """

        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(wx.EmptyBitmap(1, 1))
        dummy, self._barHeight = mem_dc.GetTextExtent("Tp")
        mem_dc.SelectObject(wx.NullBitmap)
       
        if not self._isMinibar:
            self._barHeight += 2*self._margin # The menu bar margin
        else:
            self._barHeight  = 0
        
        self._menuBarHeight = self._barHeight

        if self._showToolbar :
            # add the toolbar height to the menubar height
            self._barHeight += self._tbIconSize + 2*self._toolbarMargin

        if self._mgr is None:
            return

        pn = self._mgr.GetPane("flat_menu_bar")
        pn.MinSize(wx.Size(-1, self._barHeight))
        self._mgr.Update()
        self.Refresh()

        
    def SetOptions(self, options):
        """
        Sets the L{FlatMenuBar} options, whether to show a toolbar, to use LCD screen settings etc...

        :param `options`: a combination of the following bits:
        
         ========================= ========= =============================
         `options` Bit             Hex Value  Description
         ========================= ========= =============================
         ``FM_OPT_IS_LCD``               0x1 Use this style if your computer uses a LCD screen
         ``FM_OPT_MINIBAR``              0x2 Use this if you plan to use toolbar only
         ``FM_OPT_SHOW_CUSTOMIZE``       0x4 Show "customize link" in more menus, you will need to write your own handler. See demo.
         ``FM_OPT_SHOW_TOOLBAR``         0x8 Set this option is you are planing to use the toolbar
         ========================= ========= =============================

        """

        self._options = options

        self._showToolbar = options & FM_OPT_SHOW_TOOLBAR
        self._showCustomize = options & FM_OPT_SHOW_CUSTOMIZE
        self._isLCD = options & FM_OPT_IS_LCD
        self._isMinibar = options & FM_OPT_MINIBAR

        self.SetBarHeight()
        
        self.Refresh()
        self.Update()
                    

    def GetOptions(self):
        """
        Returns the L{FlatMenuBar} options, whether to show a toolbar, to use LCD screen settings etc...

        :see: L{SetOptions} for a list of valid options.        
        """

        return self._options

    
    def GetRendererManager(self):
        """
        Returns the L{FlatMenuBar} renderer manager.
        """
        
        return self._rendererMgr

        
    def GetRenderer(self):
        """
        Returns the renderer associated with this instance.
        """
        
        return self._rendererMgr.GetRenderer()
        

    def UpdateItem(self, item):
        """
        An item was modified. This function is called by L{FlatMenu} in case
        an item was modified directly and not via a `wx.UpdateUIEvent` event.

        :param `item`: an instance of L{FlatMenu}.        
        """

        if not self._showToolbar:
            return

        # search for a tool bar with id
        refresh = False

        for but in self._tbButtons:
            if but._tbItem.GetId() == item.GetId():
                if but._tbItem.IsEnabled() != item.IsEnabled():
                    refresh = True
                    
                but._tbItem.Enable(item.IsEnabled())
                break
            
        if refresh:        
            self.Refresh()
        

    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{FlatMenuBar}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        # on GTK, dont use the bitmap for drawing, 
        # draw directly on the DC

        if "__WXGTK__" in wx.Platform and not self._isLCD:
            self.ClearBitmaps(0)

        dc = wx.BufferedPaintDC(self)
        self.GetRenderer().DrawMenuBar(self, dc)

        
    def DrawToolbar(self, dc, rect):
        """
        Draws the toolbar (if present).

        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the toolbar client rectangle.
        """

        highlight_width = self._tbIconSize + self._toolbarSpacer
        highlight_height = self._tbIconSize + self._toolbarMargin
        
        xx = rect.x + self._toolbarMargin
        #yy = rect.y #+ self._toolbarMargin #+ (rect.height - height)/2

        # by default set all toolbar items as invisible
        for but in self._tbButtons:
            but._visible = False

        counter = 0
        # Get all the toolbar items
        for i in xrange(len(self._tbButtons)):
            
            xx += self._toolbarSpacer

            tbItem = self._tbButtons[i]._tbItem
            # the button width depends on its type
            if tbItem.IsSeparator():
                hightlight_width = SEPARATOR_WIDTH
            elif tbItem.IsCustomControl():
                control = tbItem.GetCustomControl()
                hightlight_width = control.GetSize().x + self._toolbarSpacer
            else:
                hightlight_width = self._tbIconSize + self._toolbarSpacer   # normal bitmap's width

            # can we keep drawing?
            if xx + highlight_width >= rect.width:
                break

            counter += 1

            # mark this item as visible
            self._tbButtons[i]._visible = True

            bmp = wx.NullBitmap

            #------------------------------------------
            # special handling for separator
            #------------------------------------------
            if tbItem.IsSeparator():
            
                # draw the separator
                buttonRect = wx.Rect(xx, rect.y+1, SEPARATOR_WIDTH, rect.height-2)
                self.GetRenderer().DrawToolbarSeparator(dc, buttonRect)
                
                xx += buttonRect.width
                self._tbButtons[i]._rect = buttonRect
                continue

            elif tbItem.IsCustomControl():
                control = tbItem.GetCustomControl()
                ctrlSize = control.GetSize()
                ctrlPos = wx.Point(xx, rect.y + (rect.height - ctrlSize.y)/2)
                if control.GetPosition() != ctrlPos:
                    control.SetPosition(ctrlPos)

                if not control.IsShown():
                    control.Show()
                    
                buttonRect = wx.RectPS(ctrlPos, ctrlSize)
                xx += buttonRect.width
                self._tbButtons[i]._rect = buttonRect
                continue            
            else:
                if tbItem.IsEnabled():
                    bmp = tbItem.GetBitmap()
                else:
                    bmp = tbItem.GetDisabledBitmap()

            # Draw the toolbar image
            if bmp.Ok():

                x = xx - self._toolbarSpacer/2
                #y = rect.y + (rect.height - bmp.GetHeight())/2 - 1
                y = rect.y + self._toolbarMargin/2
                
                buttonRect = wx.Rect(x, y, highlight_width, highlight_height)
                
                if i < len(self._tbButtons) and i >= 0:

                    if self._tbButtons[i]._tbItem.IsSelected():
                        tmpState = ControlPressed
                    else:
                        tmpState = ControlFocus

                    if self._tbButtons[i]._state == ControlFocus or self._tbButtons[i]._tbItem.IsSelected():
                        self.GetRenderer().DrawMenuBarButton(dc, buttonRect, tmpState) # TODO DrawToolbarButton? With separate toolbar colors
                    else:
                        self._tbButtons[i]._state = ControlNormal

                imgx = buttonRect.x + (buttonRect.width - bmp.GetWidth())/2
                imgy = buttonRect.y + (buttonRect.height - bmp.GetHeight())/2

                if self._tbButtons[i]._state == ControlFocus and not self._tbButtons[i]._tbItem.IsSelected():
                
                    # in case we the button is in focus, place it 
                    # once pixle up and left
                    # place a dark image under the original image to provide it
                    # with some shadow
                    # shadow = ConvertToMonochrome(bmp)
                    # dc.DrawBitmap(shadow, imgx, imgy, True)

                    imgx -= 1
                    imgy -= 1
                    
                dc.DrawBitmap(bmp, imgx, imgy, True)
                xx += buttonRect.width
                
                self._tbButtons[i]._rect = buttonRect
                #Edited by P.Kort  
                
                if self._showTooltip == -1:
                    self.RemoveHelp()
                else:
                    try:
                        self.DoGiveHelp(self._tbButtons[self._showTooltip]._tbItem)
                    except:
                        if _debug:
                            print "FlatMenu.py; fn : DrawToolbar; Can't create Tooltip "
                        pass

        for j in xrange(counter, len(self._tbButtons)):
            if self._tbButtons[j]._tbItem.IsCustomControl():
                control = self._tbButtons[j]._tbItem.GetCustomControl()
                control.Hide()


    def GetMoreMenuButtonRect(self):
        """ Returns a rectangle surrounding the menu button. """

        clientRect = self.GetClientRect()
        rect = wx.Rect(*clientRect)
        rect.SetWidth(DROP_DOWN_ARROW_WIDTH)
        rect.SetX(clientRect.GetWidth() + rect.GetX() - DROP_DOWN_ARROW_WIDTH - 3)
        rect.SetY(2)
        rect.SetHeight(rect.GetHeight() - self._spacer)
        
        return rect

            
    def DrawMoreButton(self, dc, fr, state):
        """
        Draws 'more' button to the right side of the menu bar.

        :param `dc`: an instance of `wx.DC`;
        :param `fr`: unused at present;
        :param `state`: the 'more' button state.

        :see: L{MenuEntryInfo.SetState} for a list of valid menu states.
        """

        if (not self._showCustomize) and self.GetInvisibleMenuItemCount() < 1 and  self.GetInvisibleToolbarItemCount() < 1:
            return
        
        # Draw a drop down menu at the right position of the menu bar
        # we use xpm file with 16x16 size, another 4 pixels we take as spacer
        # from the right side of the frame, this will create a DROP_DOWN_ARROW_WIDTH  pixels width
        # of unwanted zone on the right side

        rect = self.GetMoreMenuButtonRect()

        # Draw the bitmap
        if state != ControlNormal:
            # Draw background according to state
            self.GetRenderer().DrawButton(dc, rect, state)
        else:
            # Delete current image
            if self._moreMenuBgBmp.Ok():
                dc.DrawBitmap(self._moreMenuBgBmp, rect.x, rect.y, True)

        dropArrowBmp = self.GetRenderer()._bitmaps["arrow_down"]

        # Calc the image coordinates
        xx = rect.x + (DROP_DOWN_ARROW_WIDTH - dropArrowBmp.GetWidth())/2
        yy = rect.y + (rect.height - dropArrowBmp.GetHeight())/2
        
        dc.DrawBitmap(dropArrowBmp, xx, yy + self._spacer, True)        
        self._dropDownButtonState = state


    def HitTest(self, pt):
        """
        HitTest method for L{FlatMenuBar}.

        :param `pt`: an instance of `wx.Point`, specifying the hit test position.
        """

        if self._dropDownButtonArea.Contains(pt):
            return -1, DropDownArrowButton

        for ii, item in enumerate(self._items):
            if item.GetRect().Contains(pt):
                return ii, MenuItem

        # check for tool bar items
        if self._showToolbar:
            for ii, but in enumerate(self._tbButtons):
                if but._rect.Contains(pt):
                    # locate the corresponded menu item
                    enabled  = but._tbItem.IsEnabled()
                    separator = but._tbItem.IsSeparator()
                    visible  = but._visible
                    if enabled and not separator and visible:
                        self._showTooltip = ii
                        return ii, ToolbarItem

        self._showTooltip = -1
        return -1, NoWhere


    def FindMenuItem(self, id):
        """
        Returns a L{FlatMenuItem} according to its `id`.

        :param `id`: the identifier for the sought L{FlatMenuItem}.
        """

        for item in self._items:
            mi = item.GetMenu().FindItem(id)
            if mi:
                return mi
        return None


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{FlatMenuBar}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.ClearBitmaps(0)
        self.Refresh()


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{FlatMenuBar}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass    


    def ShowCustomize(self, show=True):
        """
        Shows/hides the drop-down arrow which allows customization of L{FlatMenu}.

        :param `show`: ``True`` to show the customize menu, ``False`` to hide it.
        """

        if self._showCustomize == show:
            return

        self._showCustomize = show
        self.Refresh()


    def SetMargin(self, margin):
        """
        Sets the margin above and below the menu bar text
        
        :param `margin`: Height in pixels of the margin 
        """
        self._margin = margin

        
    def SetSpacing(self, spacer):
        """
        Sets the spacing between the menubar items
        
        :param `spacer`: number of pixels between each menu item
        """
        self._spacer = spacer
        

    def SetToolbarMargin(self, margin):
        """
        Sets the margin around the toolbar
        
        :param `margin`: width in pixels of the margin around the tools in the toolbar
        """
        self._toolbarMargin = margin

        
    def SetToolbarSpacing(self, spacer):
        """
        Sets the spacing between the toolbar tools
        
        :param `spacer`: number of pixels between each tool in the toolbar
        """
        self._toolbarSpacer = spacer

        
    def SetLCDMonitor(self, lcd=True):
        """
        Sets whether the PC monitor is an LCD or not.

        :param `lcd`: ``True`` to use the settings appropriate for a LCD monitor,
         ``False`` otherwise.
        """

        if self._isLCD == lcd:
            return

        self._isLCD = lcd
        self.Refresh()
    

    def ProcessMouseMoveFromMenu(self, pt):
        """
        This function is called from child menus, this allow a child menu to
        pass the mouse movement event to the menu bar.

        :param `pt`: an instance of `wx.Point`.
        """

        idx, where = self.HitTest(pt)
        if where == MenuItem:
            self.ActivateMenu(self._items[idx])


    def DoMouseMove(self, pt, leftIsDown):
        """
        Handles mouse move event.

        :param `pt`: an instance of `wx.Point`;
        :param `leftIsDown`: ``True`` is the left mouse button is down, ``False`` otherwise.
        """
        
        # Reset items state
        for item in self._items:
            item.SetState(ControlNormal)

        idx, where = self.HitTest(pt)

        if where == DropDownArrowButton:
            self.RemoveHelp()
            if self._dropDownButtonState != ControlFocus and not leftIsDown:
                dc = wx.ClientDC(self)
                self.DrawMoreButton(dc, -1, ControlFocus)

        elif where == MenuItem:
            self._dropDownButtonState = ControlNormal
            # On Item
            self._items[idx].SetState(ControlFocus)

            # If this item is already selected, dont draw it again
            if self._curretHiliteItem == idx:
                return

            self._curretHiliteItem = idx
            if self._showToolbar:

                # mark all toolbar items as non-hilited
                for but in self._tbButtons:
                    but._state = ControlNormal

            self.Refresh()

        elif where == ToolbarItem:

            if self._showToolbar:
                if idx < len(self._tbButtons) and idx >= 0:
                    if self._tbButtons[idx]._state == ControlFocus:
                        return

                    # we need to refresh the toolbar
                    active = self.GetActiveToolbarItem()
                    if active != wx.NOT_FOUND:
                        self._tbButtons[active]._state = ControlNormal

                    for but in self._tbButtons:
                        but._state = ControlNormal

                    self._tbButtons[idx]._state = ControlFocus
                    self.DoGiveHelp(self._tbButtons[idx]._tbItem)
                    self.Refresh()

        elif where == NoWhere:

            refresh = False
            self.RemoveHelp()

            if self._dropDownButtonState != ControlNormal:
                refresh = True
                self._dropDownButtonState = ControlNormal

            if self._showToolbar:
                tbActiveItem = self.GetActiveToolbarItem()
                if tbActiveItem != wx.NOT_FOUND:
                    self._tbButtons[tbActiveItem]._state = ControlNormal
                    refresh = True

            if self._curretHiliteItem != -1:
            
                self._items[self._curretHiliteItem].SetState(ControlNormal)
                self._curretHiliteItem = -1
                self.Refresh()

            if refresh:
                self.Refresh()


    def OnMouseMove(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{FlatMenuBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        pt = event.GetPosition()
        self.DoMouseMove(pt, event.LeftIsDown())


    def OnLeaveMenuBar(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{FlatMenuBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.

        :note: This method is for MSW only.
        """
        
        pt = event.GetPosition()
        self.DoMouseMove(pt, event.LeftIsDown())


    def ResetToolbarItems(self):
        """ Used internally. """

        for but in self._tbButtons:
            but._state = ControlNormal


    def GetActiveToolbarItem(self):
        """ Returns the active toolbar item. """

        for but in self._tbButtons:
        
            if but._state == ControlFocus or but._state == ControlPressed:
                return self._tbButtons.index(but)
        
        return wx.NOT_FOUND
    
    def GetBackgroundColour(self):
        """ Returns the menu bar background colour. """
        
        return self.GetRenderer().menuBarFaceColour
    
    def SetBackgroundColour(self, colour):
        """
        Sets the menu bar background colour.

        :param `colour`: a valid `wx.Colour`.
        """
        
        self.GetRenderer().menuBarFaceColour = colour


    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{FlatMenuBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.

        :note: This method is for GTK only.
        """

        self._curretHiliteItem = -1
        self._dropDownButtonState = ControlNormal

        # Reset items state
        for item in self._items:
            item.SetState(ControlNormal)

        for but in self._tbButtons:
            but._state = ControlNormal

        self.Refresh()


    def OnMenuDismissed(self, event):
        """
        Handles the ``EVT_FLAT_MENU_DISMISSED`` event for L{FlatMenuBar}.

        :param `event`: a L{FlatMenuEvent} event to be processed.
        """

        pt = wx.GetMousePosition()
        pt = self.ScreenToClient(pt)

        idx, where = self.HitTest(pt)
        self.RemoveHelp()

        if where not in [MenuItem, DropDownArrowButton]:
            self._dropDownButtonState = ControlNormal
            self._curretHiliteItem = -1
            for item in self._items:
                item.SetState(ControlNormal)
                
            self.Refresh()


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{FlatMenuBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        pt = event.GetPosition()
        idx, where = self.HitTest(pt)

        if where == DropDownArrowButton:
            dc = wx.ClientDC(self)
            self.DrawMoreButton(dc, -1, ControlPressed)
            self.PopupMoreMenu()

        elif where == MenuItem:
            # Position the menu, the GetPosition() return the coords
            # of the button relative to its parent, we need to translate
            # them into the screen coords
            self.ActivateMenu(self._items[idx])
            
        elif where == ToolbarItem:
            redrawAll = False
            item = self._tbButtons[idx]._tbItem
            # try to toggle if its a check item:
            item.Toggle()
            # switch is if its a unselected radio item
            if not item.IsSelected() and item.IsRadioItem():
                group = item.GetGroup()
                for i in xrange(len(self._tbButtons)):
                    if self._tbButtons[i]._tbItem.GetGroup() == group and \
                      i != idx and self._tbButtons[i]._tbItem.IsSelected():
                        self._tbButtons[i]._state = ControlNormal
                        self._tbButtons[i]._tbItem.Select(False)
                        redrawAll = True
                item.Select(True)
            # Over a toolbar item
            if redrawAll:
                self.Refresh()
                if "__WXMSW__" in wx.Platform:
                    dc = wx.BufferedDC(wx.ClientDC(self))
                else:
                    dc = wx.ClientDC(self)
            else:
                dc = wx.ClientDC(self)
                self.DrawToolbarItem(dc, idx, ControlPressed)

            # TODO:: Do the action specified in this button
            self.DoToolbarAction(idx)


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{FlatMenuBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        pt = event.GetPosition()
        idx, where = self.HitTest(pt)
        
        if where == ToolbarItem:
            # Over a toolbar item
            dc = wx.ClientDC(self)
            self.DrawToolbarItem(dc, idx, ControlFocus)


    def DrawToolbarItem(self, dc, idx, state):
        """
        Draws a toolbar item button.

        :param `dc`: an instance of `wx.DC`;
        :param `idx`: the tool index in the toolbar;
        :param `state`: the button state.

        :see: L{MenuEntryInfo.SetState} for a list of valid menu states.        
        """

        if idx >= len(self._tbButtons) or idx < 0:
            return
        
        if self._tbButtons[idx]._tbItem.IsSelected():
            state = ControlPressed
        rect = self._tbButtons[idx]._rect
        self.GetRenderer().DrawButton(dc, rect, state)
        
        # draw the bitmap over the highlight 
        buttonRect = wx.Rect(*rect)
        x = rect.x + (buttonRect.width - self._tbButtons[idx]._tbItem.GetBitmap().GetWidth())/2
        y = rect.y + (buttonRect.height - self._tbButtons[idx]._tbItem.GetBitmap().GetHeight())/2

        if state == ControlFocus:
        
            # place a dark image under the original image to provide it
            # with some shadow
            # shadow = ConvertToMonochrome(self._tbButtons[idx]._tbItem.GetBitmap())
            # dc.DrawBitmap(shadow, x, y, True)

            # in case we the button is in focus, place it 
            # once pixle up and left
            x -= 1
            y -= 1
        dc.DrawBitmap(self._tbButtons[idx]._tbItem.GetBitmap(), x, y, True)


    def ActivateMenu(self, menuInfo):
        """
        Activates a menu.

        :param `menuInfo`: an instance of L{MenuEntryInfo}.                
        """

        # first make sure all other menus are not popedup
        if menuInfo.GetMenu().IsShown():
            return

        idx = wx.NOT_FOUND
        
        for item in self._items:
            item.GetMenu().Dismiss(False, True)
            if item.GetMenu() == menuInfo.GetMenu():
                idx = self._items.index(item)

        # Remove the popup menu as well
        if self._moreMenu and self._moreMenu.IsShown():
            self._moreMenu.Dismiss(False, True)

        # make sure that the menu item button is highlited
        if idx != wx.NOT_FOUND:
            self._dropDownButtonState = ControlNormal
            self._curretHiliteItem = idx
            for item in self._items:
                item.SetState(ControlNormal)

            self._items[idx].SetState(ControlFocus)
            self.Refresh()

        rect = menuInfo.GetRect()
        menuPt = self.ClientToScreen(wx.Point(rect.x, rect.y))
        menuInfo.GetMenu().SetOwnerHeight(rect.height)
        menuInfo.GetMenu().Popup(wx.Point(menuPt.x, menuPt.y), self)


    def DoToolbarAction(self, idx):
        """
        Performs a toolbar button pressed action.

        :param `idx`: the tool index in the toolbar.
        """
        
        # we handle only button clicks
        if self._tbButtons[idx]._tbItem.IsRegularItem() or \
            self._tbButtons[idx]._tbItem.IsCheckItem():

            # Create the event
            event = wx.CommandEvent(wxEVT_FLAT_MENU_SELECTED, self._tbButtons[idx]._tbItem.GetId())
            event.SetEventObject(self)

            # all events are handled by this control and its parents
            self.GetEventHandler().ProcessEvent(event)


    def FindMenu(self, title):
        """
        Returns the index of the menu with the given title or ``wx.NOT_FOUND`` if
        no such menu exists in this menubar.

        :param `title`: may specify either the menu title (with accelerator characters
         , i.e. "&File") or just the menu label ("File") indifferently.
        """

        for ii, item in enumerate(self._items):
            accelIdx, labelOnly = GetAccelIndex(item.GetTitle())

            if labelOnly == title or item.GetTitle() == title:
                return ii
        
        return wx.NOT_FOUND


    def GetMenu(self, menuIdx):
        """
        Returns the menu at the specified index (zero-based).

        :param `menuIdx`: the index of the sought menu.
        """

        if menuIdx >= len(self._items) or menuIdx < 0:
            return None
        
        return self._items[menuIdx].GetMenu()


    def GetMenuCount(self):
        """ Returns the number of menus in the menubar. """

        return len(self._items)        


    def Insert(self, pos, menu, title):
        """
        Inserts the menu at the given position into the menu bar.

        :param `pos`: the position of the new menu in the menu bar;
        :param `menu`: the menu to add. L{FlatMenuBar} owns the menu and will free it;
        :param `title`: the title of the menu.

        :note: Inserting menu at position 0 will insert it in the very beginning of it,
         inserting at position L{GetMenuCount} is the same as calling L{Append}.
        """

        menu.SetMenuBar(self)
        self._items.insert(pos, MenuEntryInfo(title, menu))
        self.UpdateAcceleratorTable()

        self.ClearBitmaps(pos)
        self.Refresh()        
        return True


    def Remove(self, pos):
        """
        Removes the menu from the menu bar and returns the menu object - the
        caller is responsible for deleting it.

        :param `pos`: the position of the menu in the menu bar.
        
        :note: This function may be used together with L{Insert} to change the menubar
         dynamically.
        """

        if pos >= len(self._items):
            return None

        menu = self._items[pos].GetMenu()
        self._items.pop(pos)
        self.UpdateAcceleratorTable()

        # Since we use bitmaps to optimize our drawings, we need
        # to reset all bitmaps from pos and until end of vector
        # to force size/position changes to the menu bar
        self.ClearBitmaps(pos)
        self.Refresh()

        # remove the connection to this menubar
        menu.SetMenuBar(None)
        return menu


    def UpdateAcceleratorTable(self):
        """ Updates the parent accelerator table. """
        
        # first get the number of items we have
        updatedTable = []
        parent = self.GetParent()

        for item in self._items:
        
            updatedTable = item.GetMenu().GetAccelArray() + updatedTable 

            # create accelerator for every menu (if it exist)
            title = item.GetTitle()
            mnemonic, labelOnly = GetAccelIndex(title)
            
            if mnemonic != wx.NOT_FOUND:
            
                # Get the accelrator character
                accelChar = labelOnly[mnemonic] 
                accelString = "\tAlt+" + accelChar
                title += accelString

                accel = wx.GetAccelFromString(title)
                itemId = item.GetCmdId()
                
                if accel:
                
                    # connect an event to this cmd
                    parent.Connect(itemId, -1, wxEVT_FLAT_MENU_SELECTED, self.OnAccelCmd)
                    accel.Set(accel.GetFlags(), accel.GetKeyCode(), itemId)
                    updatedTable.append(accel)
                
        entries = [wx.AcceleratorEntry() for ii in xrange(len(updatedTable))]
                    
        # Add the new menu items
        for i in xrange(len(updatedTable)):
            entries[i] = updatedTable[i]

        table = wx.AcceleratorTable(entries)
        del entries

        parent.SetAcceleratorTable(table)


    def ClearBitmaps(self, start=0):
        """
        Restores a `wx.NullBitmap` for the menu.

        :param `start`: the index at which to start resetting the bitmaps.
        """

        if self._isLCD:
            return
        
        for item in self._items[start:]:
            item.SetTextBitmap(wx.NullBitmap)
            item.SetSelectedTextBitmap(wx.NullBitmap)


    def OnAccelCmd(self, event):
        """
        Single function to handle any accelerator key used inside the menubar.

        :param `event`: a L{FlatMenuEvent} event to be processed.
        """

        for item in self._items:
            if item.GetCmdId() == event.GetId():
                self.ActivateMenu(item)
        

    def ActivateNextMenu(self):
        """ Activates next menu and make sure all others are non-active. """
        
        last_item = self.GetLastVisibleMenu()
        # find the current active menu
        for i in xrange(last_item+1):
            if self._items[i].GetMenu().IsShown():
                nextMenu = i + 1
                if nextMenu >= last_item:
                    nextMenu = 0
                self.ActivateMenu(self._items[nextMenu])
                return
            

    def GetLastVisibleMenu(self):
        """ Returns the index of the last visible menu on the menu bar. """

        last_item = 0

        # find the last visible item
        rect = wx.Rect()
        
        for item in self._items:

            if item.GetRect() == rect:
                break

            last_item += 1

        return last_item


    def ActivatePreviousMenu(self):
        """ Activates previous menu and make sure all others are non-active. """

        # find the current active menu
        last_item = self.GetLastVisibleMenu()

        for i in xrange(last_item):
            if self._items[i].GetMenu().IsShown():
                prevMenu = i - 1
                if prevMenu < 0:
                    prevMenu = last_item - 1

                if prevMenu < 0:
                    return

                self.ActivateMenu(self._items[prevMenu])
                return


    def CreateMoreMenu(self):
        """ Creates the drop down menu and populate it. """
        
        if not self._moreMenu: 
            # first time
            self._moreMenu = FlatMenu(self)
            self._popupDlgCmdId = wx.NewId()

            # Connect an event handler for this event
            self.Connect(self._popupDlgCmdId, -1, wxEVT_FLAT_MENU_SELECTED, self.OnCustomizeDlg)
        
        # Remove all items from the popup menu
        self._moreMenu.Clear()
        
        invM = self.GetInvisibleMenuItemCount()
        
        for i in xrange(len(self._items) - invM, len(self._items)):
            item = FlatMenuItem(self._moreMenu, wx.ID_ANY, self._items[i].GetTitle(),
                                "", wx.ITEM_NORMAL, self._items[i].GetMenu())
            self._moreMenu.AppendItem(item)

        # Add invisible toolbar items
        invT = self.GetInvisibleToolbarItemCount()
        
        if self._showToolbar and invT > 0:
            if self.GetInvisibleMenuItemCount() > 0:
                self._moreMenu.AppendSeparator()

            for i in xrange(len(self._tbButtons) - invT, len(self._tbButtons)):
                if self._tbButtons[i]._tbItem.IsSeparator():
                    self._moreMenu.AppendSeparator()
                elif not self._tbButtons[i]._tbItem.IsCustomControl():
                    tbitem = self._tbButtons[i]._tbItem
                    item = FlatMenuItem(self._tbMenu, tbitem.GetId(), tbitem.GetLabel(), "", wx.ITEM_NORMAL, None, tbitem.GetBitmap(), tbitem.GetDisabledBitmap())
                    item.Enable(tbitem.IsEnabled())
                    self._moreMenu.AppendItem(item)
            

        if self._showCustomize:
            if invT + invM > 0:
                self._moreMenu.AppendSeparator()
            item = FlatMenuItem(self._moreMenu, self._popupDlgCmdId, "Customize ...")
            self._moreMenu.AppendItem(item)


    def GetInvisibleMenuItemCount(self):
        """
        Returns the number of invisible menu items.

        :note: Valid only after the `wx.PaintEvent` has been processed after a resize.
        """
        
        return len(self._items) - self.GetLastVisibleMenu()

    
    def GetInvisibleToolbarItemCount(self):
        """
        Returns the number of invisible toolbar items.

        :note: Valid only after the `wx.PaintEvent` has been processed after a resize.
        """
        
        count = 0
        for i in xrange(len(self._tbButtons)):
            if self._tbButtons[i]._visible == False:
                break
            count = i

        return len(self._tbButtons) - count - 1

    
    def PopupMoreMenu(self):
        """ Popups the 'more' menu. """

        if (not self._showCustomize) and self.GetInvisibleMenuItemCount() + self.GetInvisibleToolbarItemCount() < 1:
            return
        
        self.CreateMoreMenu()

        pt = self._dropDownButtonArea.GetTopLeft()
        pt = self.ClientToScreen(pt)
        pt.y += self._dropDownButtonArea.GetHeight()
        self._moreMenu.Popup(pt, self)


    def OnCustomizeDlg(self, event):
        """
        Handles the customize dialog here.

        :param `event`: a L{FlatMenuEvent} event to be processed.
        """

        if not self._dlg:
            self._dlg = FMCustomizeDlg(self)
        else:
            # intialize the dialog
            self._dlg.Initialise()
        
        if self._dlg.ShowModal() == wx.ID_OK:
            # Handle customize requests here
            pass
        
        if "__WXGTK__" in wx.Platform:
            # Reset the more button
            dc = wx.ClientDC(self)
            self.DrawMoreButton(dc, -1, ControlNormal)


    def AppendToolbarItem(self, item):
        """
        Appends a tool to the L{FlatMenuBar}.
        
        :warning: This method is now deprecated.

        :see: L{AddTool}        
        """

        newItem = ToolBarItem(item, wx.Rect(), ControlNormal)
        self._tbButtons.append(newItem)


    def AddTool(self, toolId, label="", bitmap1=wx.NullBitmap, bitmap2=wx.NullBitmap,
                kind=wx.ITEM_NORMAL, shortHelp="", longHelp=""):
        """
        Adds a tool to the toolbar.
        
        :param `toolId`: an integer by which the tool may be identified in subsequent
         operations;
        :param `label`: the tool label string;
        :param `kind`: may be ``wx.ITEM_NORMAL`` for a normal button (default),
         ``wx.ITEM_CHECK`` for a checkable tool (such tool stays pressed after it had been
         toggled) or ``wx.ITEM_RADIO`` for a checkable tool which makes part of a radio
         group of tools each of which is automatically unchecked whenever another button
         in the group is checked;
        :param `bitmap1`: the primary tool bitmap;
        :param `bitmap2`: the bitmap used when the tool is disabled. If it is equal to
         `wx.NullBitmap`, the disabled bitmap is automatically generated by greing out
         the normal one;
        :param `shortHelp`: a string used for the tools tooltip;
        :param `longHelp`: this string is shown in the `wx.StatusBar` (if any) of the
         parent frame when the mouse pointer is inside the tool.
        """
        
        self._tbButtons.append(ToolBarItem(FlatToolbarItem(bitmap1, toolId, label, bitmap2, kind, shortHelp, longHelp), wx.Rect(), ControlNormal))


    def AddSeparator(self):
        """ Adds a separator for spacing groups of tools in toolbar. """

        if len(self._tbButtons) > 0 and not self._tbButtons[len(self._tbButtons)-1]._tbItem.IsSeparator():
            self._tbButtons.append(ToolBarItem(FlatToolbarItem(), wx.Rect(), ControlNormal))

        
    def AddControl(self, control):
        """
        Adds any control to the toolbar, typically e.g. a combobox.
        
        :param `control`: the control to be added.
        """
        
        self._tbButtons.append(ToolBarItem(FlatToolbarItem(control), wx.Rect(), ControlNormal))


    def AddCheckTool(self, toolId, label="", bitmap1=wx.NullBitmap, bitmap2=wx.NullBitmap, shortHelp="", longHelp=""):
        """
        Adds a new check (or toggle) tool to the toolbar.

        :see: L{AddTool} for parameter descriptions.
        """
        
        self.AddTool(toolId, label, bitmap1, bitmap2, kind=wx.ITEM_CHECK, shortHelp=shortHelp, longHelp=longHelp)

        
    def AddRadioTool(self, toolId, label= "", bitmap1=wx.NullBitmap, bitmap2=wx.NullBitmap, shortHelp="", longHelp=""):
        """
        Adds a new radio tool to the toolbar.

        Consecutive radio tools form a radio group
        such that exactly one button in the group is pressed at any moment, in other
        words whenever a button in the group is pressed the previously pressed button
        is automatically released.

        You should avoid having the radio groups of only one element as it would be
        impossible for the user to use such button.

        By default, the first button in the radio group is initially pressed, the others are not.
        
        :see: L{AddTool} for parameter descriptions.
        """
        
        self.AddTool(toolId, label, bitmap1, bitmap2, kind=wx.ITEM_RADIO, shortHelp=shortHelp, longHelp=longHelp)
        
        if len(self._tbButtons)<1 or not self._tbButtons[len(self._tbButtons)-2]._tbItem.IsRadioItem():
            self._tbButtons[len(self._tbButtons)-1]._tbItem.Select(True)
            self._lastRadioGroup += 1
            
        self._tbButtons[len(self._tbButtons)-1]._tbItem.SetGroup(self._lastRadioGroup)


    def SetUpdateInterval(self, interval):
        """
        Sets the updateUI interval for toolbar items. All UpdateUI events are
        sent from within L{OnIdle} handler, the default is 20 milliseconds.

        :param `interval`: the updateUI interval in milliseconds.        
        """

        self._interval = interval


    def PositionAUI(self, mgr, fixToolbar=True):
        """
        Positions the control inside a wxAUI / PyAUI frame manager.

        :param `mgr`: an instance of `wx.aui.AuiManager` or L{AuiManager};
        :param `fixToolbar`: ``True`` if L{FlatMenuBar} can not be floated.
        """

        if isinstance(mgr, wx.aui.AuiManager):
            pn = AuiPaneInfo()
        else:
            pn = PyAuiPaneInfo()
            
        xx = wx.SystemSettings_GetMetric(wx.SYS_SCREEN_X)

        # We add our menu bar as a toolbar, with the following settings

        pn.Name("flat_menu_bar")
        pn.Caption("Menu Bar")
        pn.Top()
        pn.MinSize(wx.Size(xx/2, self._barHeight))
        pn.LeftDockable(False)
        pn.RightDockable(False)
        pn.ToolbarPane()
        
        if not fixToolbar:
            # We add our menu bar as a toolbar, with the following settings
            pn.BestSize(wx.Size(xx, self._barHeight))
            pn.FloatingSize(wx.Size(300, self._barHeight))
            pn.Floatable(True)
            pn.MaxSize(wx.Size(xx, self._barHeight))
            pn.Gripper(True)
            
        else:
            pn.BestSize(wx.Size(xx, self._barHeight))
            pn.Gripper(False)

        pn.Resizable(False)
        pn.PaneBorder(False)
        mgr.AddPane(self, pn)

        self._mgr = mgr        


    def DoGiveHelp(self, hit):
        """
        Gives tooltips and help in `wx.StatusBar`.

        :param `hit`: the toolbar tool currently hovered by the mouse.
        """

        shortHelp = hit.GetShortHelp()
        if shortHelp:
            self.SetToolTipString(shortHelp)
            self._haveTip = True

        longHelp = hit.GetLongHelp()
        if not longHelp:
            return
        
        topLevel = wx.GetTopLevelParent(self)
        
        if isinstance(topLevel, wx.Frame) and topLevel.GetStatusBar():
            statusBar = topLevel.GetStatusBar()

            if self._statusTimer and self._statusTimer.IsRunning():
                self._statusTimer.Stop()
                statusBar.PopStatusText(0)
                
            statusBar.PushStatusText(longHelp, 0)
            self._statusTimer = StatusBarTimer(self)
            self._statusTimer.Start(_DELAY, wx.TIMER_ONE_SHOT)


    def RemoveHelp(self):
        """ Removes the tooltips and statusbar help (if any) for a button. """

        if self._haveTip:
            self.SetToolTipString("")
            self._haveTip = False

        if self._statusTimer and self._statusTimer.IsRunning():
            topLevel = wx.GetTopLevelParent(self)
            statusBar = topLevel.GetStatusBar()
            self._statusTimer.Stop()
            statusBar.PopStatusText(0)
            self._statusTimer = None


    def OnStatusBarTimer(self):
        """ Handles the timer expiring to delete the `longHelp` string in the `wx.StatusBar`. """

        topLevel = wx.GetTopLevelParent(self)
        statusBar = topLevel.GetStatusBar()        
        statusBar.PopStatusText(0)



class mcPopupWindow(wx.MiniFrame):
    """ Since Max OS does not support `wx.PopupWindow`, this is an alternative."""

    def __init__(self, parent):
        """
        Default class constructor.

        :param `parent`: the L{mcPopupWindow} parent window.
        """

        wx.MiniFrame.__init__(self, parent, style = wx.POPUP_WINDOW)
        self.SetExtraStyle(wx.WS_EX_TRANSIENT)
        self._parent = parent
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)
        

    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{mcPopupWindow}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        event.Skip()


havePopupWindow = 1

if wx.Platform == '__WXMAC__':
    havePopupWindow = 0
    wx.PopupWindow = mcPopupWindow


# ---------------------------------------------------------------------------- #
# Class ShadowPopupWindow
# ---------------------------------------------------------------------------- #
    
class ShadowPopupWindow(wx.PopupWindow):
    """ Base class for generic L{FlatMenu} derived from `wx.PopupWindow`. """
    
    def __init__(self, parent=None):
        """
        Default class constructor.

        :param `parent`: the L{ShadowPopupWindow} parent (tipically your main frame).
        """
        
        if not parent:
            parent = wx.GetApp().GetTopWindow()

        if not parent:
            raise Exception("Can't create menu without parent!")

        wx.PopupWindow.__init__(self, parent)

        if "__WXMSW__" in wx.Platform and _libimported == "MH":

            GCL_STYLE= -26
            cstyle= win32gui.GetClassLong(self.GetHandle(), GCL_STYLE)
            if cstyle & CS_DROPSHADOW == 0:
                win32api.SetClassLong(self.GetHandle(),
                                      GCL_STYLE, cstyle | CS_DROPSHADOW)

        # popup windows are created hidden by default
        self.Hide()

    
#--------------------------------------------------------
# Class FlatMenuButton
#--------------------------------------------------------

class FlatMenuButton(object):
    """
    A nice small class that functions like `wx.BitmapButton`, the reason I did
    not used `wx.BitmapButton` is that on Linux, it has some extra margins that
    I can't seem to be able to remove.
    """

    def __init__(self, menu, up, normalBmp, disabledBmp=wx.NullBitmap, scrollOnHover=False):
        """
        Default class constructor.

        :param `menu`: the parent menu associated with this button;
        :param `up`: ``True`` for up arrow or ``False`` for down arrow;
        :param `normalBmp`: normal state bitmap;
        :param `disabledBmp`: disabled state bitmap.
        """

        self._normalBmp = normalBmp
        self._up = up
        self._parent = menu
        self._pos = wx.Point()
        self._size = wx.Size()
        self._timerID = wx.NewId()
        self._scrollOnHover = scrollOnHover

        if not disabledBmp.Ok():
            self._disabledBmp = wx.BitmapFromImage(self._normalBmp.ConvertToImage().ConvertToGreyscale())
        else: 
            self._disabledBmp = disabledBmp
        
        self._state = ControlNormal
        self._timer = wx.Timer(self._parent, self._timerID)
        self._timer.Stop()


    def __del__(self):
        """ Used internally. """

        if self._timer:
            if self._timer.IsRunning():
                self._timer.Stop()
                
            del self._timer


    def Contains(self, pt):
        """ Used internally. """
    
        rect = wx.RectPS(self._pos, self._size)
        if not rect.Contains(pt):
            return False

        return True
    

    def Draw(self, dc):
        """
        Draws self at rect using dc.

        :param `dc`: an instance of `wx.DC`.
        """

        rect = wx.RectPS(self._pos, self._size)
        xx = rect.x + (rect.width - self._normalBmp.GetWidth())/2
        yy = rect.y + (rect.height - self._normalBmp.GetHeight())/2

        self._parent.GetRenderer().DrawScrollButton(dc, rect, self._state)
        dc.DrawBitmap(self._normalBmp, xx, yy, True)


    def ProcessLeftDown(self, pt):
        """
        Handles left down mouse events.

        :param `pt`: an instance of `wx.Point` where the left mouse button was pressed.
        """

        if not self.Contains(pt):
            return False

        self._state = ControlPressed
        self._parent.Refresh()
        
        if self._up:
            self._parent.ScrollUp()
        else:
            self._parent.ScrollDown()
           
        self._timer.Start(100)
        return True


    def ProcessLeftUp(self, pt):
        """
        Handles left up mouse events.

        :param `pt`: an instance of `wx.Point` where the left mouse button was released.
        """

        # always stop the timer
        self._timer.Stop()

        if not self.Contains(pt):
            return False

        self._state = ControlFocus
        self._parent.Refresh()

        return True


    def ProcessMouseMove(self, pt):
        """
        Handles mouse motion events. This is called any time the mouse moves in the parent menu, 
        so we must check to see if the mouse is over the button.

        :param `pt`: an instance of `wx.Point` where the mouse pointer was moved.
        """

        if not self.Contains(pt):
        
            self._timer.Stop()
            if self._state != ControlNormal:
            
                self._state = ControlNormal
                self._parent.Refresh()
            
            return False
        
        if self._scrollOnHover and not self._timer.IsRunning():
            self._timer.Start(100)
        
        # Process mouse move event
        if self._state != ControlFocus:
            if self._state != ControlPressed:
                self._state = ControlFocus
                self._parent.Refresh()
        
        return True


    def GetTimerId(self):
        """ Returns the timer object Ientifier. """

        return self._timerID


    def GetTimer(self):
        """ Returns the timer object. """

        return self._timer


    def Move(self, input1, input2=None):
        """
        Moves L{FlatMenuButton} to the specified position.

        :param `input1`: if it is an instance of `wx.Point`, it represents the L{FlatMenuButton}
         position and the `input2` parameter is not used. Otherwise it is an integer representing
         the button `x` position;
        :param `input2`: if not ``None``, it is an integer representing the button `y` position.
        """

        if type(input) == type(1):
            self._pos = wx.Point(input1, input2)
        else:
            self._pos = input1
            

    def SetSize(self, input1, input2=None):
        """
        Sets the size for L{FlatMenuButton}.

        :param `input1`: if it is an instance of `wx.Size`, it represents the L{FlatMenuButton}
         size and the `input2` parameter is not used. Otherwise it is an integer representing
         the button width;
        :param `input2`: if not ``None``, it is an integer representing the button height.
        """

        if type(input) == type(1):
            self._size = wx.Size(input1, input2)
        else:
            self._size = input1
                

    def GetClientRect(self):
        """ Returns the client rectangle for L{FlatMenuButton}. """

        return wx.RectPS(self._pos, self._size)


#--------------------------------------------------------
# Class FlatMenuItemGroup
#--------------------------------------------------------

class FlatMenuItemGroup(object):
    """
    A class that manages a group of radio menu items.
    """
 
    def __init__(self):
        """ Default class constructor. """

        self._items = []
        

    def GetSelectedItem(self):
        """ Returns the selected item. """

        for item in self._items:
            if item.IsChecked():
                return item
        
        return None


    def Add(self, item):
        """
        Adds a new item to the group.

        :param `item`: an instance of L{FlatMenu}.
        """

        if item.IsChecked():
            # uncheck all other items
            for exitem in self._items:
                exitem._bIsChecked = False
        
        self._items.append(item)


    def Exist(self, item):
        """
        Checks if an item is in the group.

        :param `item`: an instance of L{FlatMenu}.
        """

        if item in self._items:
            return True
        
        return False


    def SetSelection(self, item):
        """
        Selects a particular item.

        :param `item`: an instance of L{FlatMenu}.
        """

        # make sure this item exist in our group
        if not self.Exist(item):
            return

        # uncheck all other items
        for exitem in self._items:
            exitem._bIsChecked = False
        
        item._bIsChecked = True


    def Remove(self, item):
        """
        Removes a particular item.

        :param `item`: an instance of L{FlatMenu}.
        """

        if item not in self._items:
            return

        self._items.remove(item)

        if item.IsChecked() and len(self._items) > 0:
            #if the removed item was the selected one,
            # select the first one in the group
            self._items[0]._bIsChecked = True


#--------------------------------------------------------
# Class FlatMenuBase
#--------------------------------------------------------

class FlatMenuBase(ShadowPopupWindow):
    """
    Base class for generic flat menu derived from `wx.PopupWindow`.
    """

    def __init__(self, parent=None):
        """
        Default class constructor.

        :param `parent`: the L{ShadowPopupWindow} parent window.
        """

        self._rendererMgr = FMRendererMgr()
        self._parentMenu = parent
        self._openedSubMenu = None
        self._owner = None
        self._popupPtOffset = 0
        self._showScrollButtons = False
        self._upButton = None
        self._downButton = None
        self._is_dismiss = False

        ShadowPopupWindow.__init__(self, parent)
                    

    def OnDismiss(self):
        """ Fires an event ``EVT_FLAT_MENU_DISMISSED`` and handle menu dismiss. """

        # Release mouse capture if needed
        if self.HasCapture():
            self.ReleaseMouse()

        self._is_dismiss = True
        
        # send an event about our dismissal to the parent (unless we are a sub menu)
        if self.IsShown() and not self._parentMenu:

            event = FlatMenuEvent(wxEVT_FLAT_MENU_DISMISSED, self.GetId())
            event.SetEventObject(self)

            # Send it
            if self.GetMenuOwner():
                self.GetMenuOwner().GetEventHandler().ProcessEvent(event)
            else:
                self.GetEventHandler().ProcessEvent(event)
        

    def Popup(self, pt, parent):
        """
        Popups menu at the specified point.

        :param `pt`: an instance of `wx.Point`, assumed to be in screen coordinates. However,
         if `parent` is not ``None``, `pt` is translated into the screen coordinates using
         `parent.ClientToScreen()`;
        :param `parent`: if not ``None``, an instance of `wx.Window`.
        """

        # some controls update themselves from OnIdle() call - let them do it
        wx.GetApp().ProcessIdle()

        # The mouse was pressed in the parent coordinates, 
        # e.g. pressing on the left top of a text ctrl
        # will result in (1, 1), these coordinates needs
        # to be converted into screen coords
        self._parentMenu = parent

        # If we are topmost menu, we use the given pt
        # else we use the logical 
        # parent (second argument provided to this function)

        if self._parentMenu:
            pos = self._parentMenu.ClientToScreen(pt)
        else:
            pos = pt

        # Fit the menu into screen
        pos = self.AdjustPosition(pos)
        if self._showScrollButtons:
            
            sz = self.GetSize()
            # Get the screen height
            scrHeight = wx.SystemSettings_GetMetric(wx.SYS_SCREEN_Y)
            

            # position the scrollbar - If we are doing scroll bar buttons put them in the top right and 
            # bottom right or else place them as menu items at the top and bottom.
            if self.GetRenderer().scrollBarButtons:
                if not self._upButton:
                    self._upButton = FlatMenuButton(self, True, ArtManager.Get().GetStockBitmap("arrow_up"))

                if not self._downButton:
                    self._downButton = FlatMenuButton(self, False, ArtManager.Get().GetStockBitmap("arrow_down"))
                    
                self._upButton.SetSize((SCROLL_BTN_HEIGHT, SCROLL_BTN_HEIGHT))
                self._downButton.SetSize((SCROLL_BTN_HEIGHT, SCROLL_BTN_HEIGHT))

                self._upButton.Move((sz.x - SCROLL_BTN_HEIGHT - 4, 4))
                self._downButton.Move((sz.x - SCROLL_BTN_HEIGHT - 4, scrHeight - pos.y - 2 - SCROLL_BTN_HEIGHT))
            else:
                if not self._upButton:
                    self._upButton = FlatMenuButton(self, True, getMenuUpArrowBitmap(), scrollOnHover=True)

                if not self._downButton:
                    self._downButton = FlatMenuButton(self, False, getMenuDownArrowBitmap(), scrollOnHover=True)
                    
                self._upButton.SetSize((sz.x-2, self.GetItemHeight()))
                self._downButton.SetSize((sz.x-2, self.GetItemHeight()))

                self._upButton.Move((1, 3))
                self._downButton.Move((1, scrHeight - pos.y - 3 - self.GetItemHeight()))

        self.Move(pos)        
        self.Show()

        # Capture mouse event and direct them to us
        if not self.HasCapture():
            self.CaptureMouse()
            
        self._is_dismiss = False
        

    def AdjustPosition(self, pos):
        """
        Adjusts position so the menu will be fully visible on screen.

        :param `pos`: an instance of `wx.Point` specifying the menu position.
        """

        # Check that the menu can fully appear in the screen
        scrWidth  = wx.SystemSettings_GetMetric(wx.SYS_SCREEN_X)
        scrHeight = wx.SystemSettings_GetMetric(wx.SYS_SCREEN_Y)
        
        scrollBarButtons = self.GetRenderer().scrollBarButtons
        scrollBarMenuItems = not scrollBarButtons

        size = self.GetSize()
        if scrollBarMenuItems:
            size.y += self.GetItemHeight()*2

        # always assume that we have scrollbuttons on
        self._showScrollButtons = False
        pos.y += self._popupPtOffset
        
        if size.y + pos.y > scrHeight:
            # the menu will be truncated
            if self._parentMenu is None:
                # try to flip the menu
                flippedPosy = pos.y - size.y
                flippedPosy -= self._popupPtOffset

                if flippedPosy >= 0 and flippedPosy + size.y < scrHeight:
                    pos.y = flippedPosy
                    return pos
                else: 
                    # We need to popup scrollbuttons!
                    self._showScrollButtons = True
                
            else: 
                # we are a submenu
                # try to decrease the y value of the menu position
                newy = pos.y
                newy -= (size.y + pos.y) - scrHeight
                
                if newy + size.y > scrHeight:
                    # probably the menu size is too high to fit
                    # the screen, we need scrollbuttons
                    self._showScrollButtons = True
                else:
                    pos.y = newy

        menuMaxX = pos.x + size.x

        if menuMaxX > scrWidth and pos.x < scrWidth:

            if self._parentMenu:
            
                # We are submenu
                self._shiftePos = (size.x + self._parentMenu.GetSize().x)
                pos.x -= self._shiftePos
                pos.x += 10
                            
            else:

                self._shiftePos  = ((size.x + pos.x) - scrWidth)            
                pos.x -= self._shiftePos

        else:

            if self._parentMenu:
                pos.x += 5
                
        return pos
    

    def Dismiss(self, dismissParent, resetOwner):
        """
        Dismisses the popup window.

        :param `dismissParent`: whether to dismiss the parent menu or not;
        :param `resetOwner`: ``True`` to delete the link between this menu and the
         owner menu, ``False`` otherwise.        
        """

        # Check if child menu is poped, if so, dismiss it
        if self._openedSubMenu:
            self._openedSubMenu.Dismiss(False, resetOwner)

        self.OnDismiss()

        # Reset menu owner
        if resetOwner:
            self._owner = None

        self.Show(False)

        if self._parentMenu and dismissParent:
        
            self._parentMenu.OnChildDismiss()
            self._parentMenu.Dismiss(dismissParent, resetOwner)
        
        self._parentMenu = None


    def OnChildDismiss(self):
        """ Handles children dismiss. """

        self._openedSubMenu = None

    def GetRenderer(self):
        """ Gets the renderer for this class. """
        
        return self._rendererMgr.GetRenderer()


    def GetRootMenu(self):
        """ Gets the top level menu. """

        root = self
        while root._parentMenu:
            root = root._parentMenu
            
        return root


    def SetOwnerHeight(self, height):
        """
        Sets the menu owner height, this will be used to position the menu below
        or above the owner.

        :param `height`: an integer representing the menu owner height.        
        """

        self._popupPtOffset = height
        
    
    # by default do nothing
    def ScrollDown(self):
        """
        Scroll one unit down.
        By default this function is empty, let derived class do something.
        """
        
        pass


    # by default do nothing
    def ScrollUp(self):
        """
        Scroll one unit up.
        By default this function is empty, let derived class do something.
        """
        
        pass


    def GetMenuOwner(self):
        """
        Returns the menu logical owner, the owner does not necessarly mean the
        menu parent, it can also be the window that popped up it.
        """

        return self._owner
        

#--------------------------------------------------------
# Class ToolBarItem
#--------------------------------------------------------

class ToolBarItem(object):
    """
    A simple class that holds information about a toolbar item.
    """
    
    def __init__(self, tbItem, rect, state):
        """
        Default class constructor.

        :param `tbItem`: an instance of L{FlatToolbarItem};
        :param `rect`: the client rectangle for the toolbar item;
        :param `state`: the toolbar item state.

        :see: L{MenuEntryInfo.SetState} for a list of valid item states.        
        """

        self._tbItem = tbItem
        self._rect = rect
        self._state = state
        self._visible = True


#--------------------------------------------------------
# Class FlatToolBarItem
#--------------------------------------------------------

class FlatToolbarItem(object):
    """
    This class represents a toolbar item.
    """

    def __init__(self, controlType=None, id=wx.ID_ANY, label="", disabledBmp=wx.NullBitmap, kind=wx.ITEM_NORMAL,
                 shortHelp="", longHelp=""):
        """
        Default class constructor.

        :param `controlType`: can be ``None`` for a toolbar separator, an instance
         of `wx.Window` for a control or an instance of `wx.Bitmap` for a standard
         toolbar tool;
        :param `id`: the toolbar tool id. If set to ``wx.ID_ANY``, a new id is
         automatically assigned;
        :param `label`: the toolbar tool label;
        :param `disabledBmp`: the bitmap used when the tool is disabled. If the tool
         is a standard one (i.e., not a control or a separator), and `disabledBmp`
         is equal to `wx.NullBitmap`, the disabled bitmap is automatically generated
         by greing the normal one;
        :param `kind`: may be ``wx.ITEM_NORMAL`` for a normal button (default),
         ``wx.ITEM_CHECK`` for a checkable tool (such tool stays pressed after it had been
         toggled) or ``wx.ITEM_RADIO`` for a checkable tool which makes part of a radio
         group of tools each of which is automatically unchecked whenever another button
         in the group is checked;
        :param `shortHelp`: a string used for the tool's tooltip;
        :param `longHelp`: this string is shown in the `wx.StatusBar` (if any) of the
         parent frame when the mouse pointer is inside the tool.
        """
        
        if id == wx.ID_ANY:
            id = wx.NewId()

        if controlType is None:    # Is a separator
            self._normalBmp = wx.NullBitmap
            self._id = wx.NewId()
            self._label = ""
            self._disabledImg = wx.NullBitmap
            self._customCtrl = None
            kind = wx.ITEM_SEPARATOR

        elif isinstance(controlType, wx.Window): # is a wxControl
            self._normalBmp = wx.NullBitmap
            self._id = id
            self._label = ""
            self._disabledImg = wx.NullBitmap
            self._customCtrl = controlType
            kind = FTB_ITEM_CUSTOM
            
        elif isinstance(controlType, wx.Bitmap):   # Bitmap construction, simple tool
            self._normalBmp = controlType
            self._id = id
            self._label = label
            self._disabledImg = disabledBmp
            self._customCtrl = None
            
            if not self._disabledImg.Ok():
                # Create a grey bitmap from the normal bitmap
                self._disabledImg = wx.BitmapFromImage(self._normalBmp.ConvertToImage().ConvertToGreyscale())

        self._kind = kind
        self._enabled = True
        self._selected = False
        self._group = -1 # group id for radio items

        if not shortHelp:
            shortHelp = label
            
        self._shortHelp = shortHelp
        self._longHelp = longHelp

    def GetLabel(self):
        """ Returns the tool label. """

        return self._label


    def SetLabel(self, label):
        """
        Sets the tool label.

        :param `label`: the new tool string.
        """

        self._label = label


    def GetBitmap(self):
        """ Returns the tool bitmap. """

        return self._normalBmp


    def SetBitmap(self, bmp):
        """
        Sets the tool bitmap.

        :param `bmp`: the new tool bitmap, a valid `wx.Bitmap` object.
        """

        self._normalBmp = bmp
        

    def GetDisabledBitmap(self):
        """ Returns the tool disabled bitmap. """

        return self._disabledImg


    def SetDisabledBitmap(self, bmp):
        """
        Sets the tool disabled bitmap.

        :param `bmp`: the new tool disabled bitmap, a valid `wx.Bitmap` object.
        """

        self._disabledImg = bmp
        

    def GetId(self):
        """ Gets the tool id. """

        return self._id


    def IsSeparator(self):
        """ Returns whether the tool is a separator or not. """

        return self._kind == wx.ITEM_SEPARATOR


    def IsRadioItem(self):
        """ Returns True if the item is a radio item. """
        
        return self._kind == wx.ITEM_RADIO


    def IsCheckItem(self):
        """ Returns True if the item is a radio item. """
        
        return self._kind == wx.ITEM_CHECK

    
    def IsCustomControl(self):
        """ Returns whether the tool is a custom control or not. """

        return self._kind == FTB_ITEM_CUSTOM


    def IsRegularItem(self):
        """ Returns whether the tool is a standard tool or not. """

        return self._kind == wx.ITEM_NORMAL


    def GetCustomControl(self):
        """ Returns the associated custom control. """

        return self._customCtrl


    def IsSelected(self):
        """ Returns whether the tool is selected or checked."""
        
        return self._selected


    def IsChecked(self):
        """ Same as L{IsSelected}. More intuitive for check items though. """
        
        return self._selected


    def Select(self, select=True):
        """
        Selects or checks a radio or check item.

        :param `select`: ``True`` to select or check a tool, ``False`` to unselect
         or uncheck it.
        """
        
        self._selected = select

        
    def Toggle(self):
        """ Toggles a check item. """
        
        if self.IsCheckItem():
            self._selected = not self._selected

            
    def SetGroup(self, group):
        """
        Sets group id for a radio item, for other items does nothing.

        :param `group`: an instance of L{FlatMenuItemGroup}.
        """
        
        if self.IsRadioItem():
            self._group = group

            
    def GetGroup(self):
        """ Returns group id for radio item, or -1 for other item types. """
        
        return self._group
    
    
    def IsEnabled(self):
        """ Returns whether the tool is enabled or not. """

        return self._enabled
    
    
    def Enable(self, enable=True):
        """
        Enables or disables the tool.

        :param `enable`: ``True`` to enable the tool, ``False`` to disable it.
        """

        self._enabled = enable


    def GetShortHelp(self):
        """ Returns the tool short help string (displayed in the tool's tooltip). """

        if self._kind == wx.ITEM_NORMAL:
            return self._shortHelp

        return ""


    def SetShortHelp(self, help):
        """
        Sets the tool short help string (displayed in the tool's tooltip).

        :param `help`: the new tool short help string.
        """

        if self._kind == wx.ITEM_NORMAL:
            self._shortHelp = help


    def SetLongHelp(self, help):
        """
        Sets the tool long help string (displayed in the parent frame `wx.StatusBar`).

        :param `help`: the new tool long help string.
        """

        if self._kind == wx.ITEM_NORMAL:
            self._longHelp = help


    def GetLongHelp(self):
        """ Returns the tool long help string (displayed in the parent frame `wx.StatusBar`). """

        if self._kind == wx.ITEM_NORMAL:
            return self._longHelp

        return ""


#--------------------------------------------------------
# Class FlatMenuItem
#--------------------------------------------------------

class FlatMenuItem(object):
    """
    A class that represents an item in a menu.
    """

    def __init__(self, parent, id=wx.ID_SEPARATOR, label="", helpString="",
                 kind=wx.ITEM_NORMAL, subMenu=None, normalBmp=wx.NullBitmap,
                 disabledBmp=wx.NullBitmap,
                 hotBmp=wx.NullBitmap):
        """
        Default class constructor.

        :param `parent`: menu that the menu item belongs to;
        :param `id`: the menu item identifier;
        :param `label`: text for the menu item, as shown on the menu. An accelerator
         key can be specified using the ampersand '&' character. In order to embed
         an ampersand character in the menu item text, the ampersand must be doubled;
        :param `helpString`: optional help string that will be shown on the status bar;
        :param `kind`: may be ``wx.ITEM_SEPARATOR``, ``wx.ITEM_NORMAL``, ``wx.ITEM_CHECK``
         or ``wx.ITEM_RADIO``;
        :param `subMenu`: if not ``None``, the subMenu this item belongs to;
        :param `normalBmp`: normal bitmap to draw to the side of the text, this bitmap
         is used when the menu is enabled;
        :param `disabledBmp`: 'greyed' bitmap to draw to the side of the text, this
         bitmap is used when the menu is disabled, if none supplied normal is used;
        :param `hotBmp`: hot bitmap to draw to the side of the text, this bitmap is
         used when the menu is hovered, if non supplied, normal is used.
        """

        self._text = label
        self._kind = kind
        self._helpString = helpString

        if id == wx.ID_ANY:
            id = wx.NewId()

        self._id = id
        self._parentMenu = parent
        self._subMenu = subMenu
        self._normalBmp = normalBmp
        self._disabledBmp = disabledBmp
        self._hotBmp = hotBmp
        self._bIsChecked = False
        self._bIsEnabled = True
        self._mnemonicIdx = wx.NOT_FOUND
        self._isAttachedToMenu = False
        self._accelStr = ""
        self._rect = wx.Rect()
        self._groupPtr = None
        self._visible = False
        self._contextMenu = None

        self.SetLabel(self._text)
        self.SetMenuBar()

        self._checkMarkBmp = wx.BitmapFromXPMData(check_mark_xpm)
        self._checkMarkBmp.SetMask(wx.Mask(self._checkMarkBmp, wx.WHITE))
        self._radioMarkBmp = wx.BitmapFromXPMData(radio_item_xpm)
        self._radioMarkBmp.SetMask(wx.Mask(self._radioMarkBmp, wx.WHITE))

                
    def SetLongHelp(self, help):
        """
        Sets the item long help string (displayed in the parent frame `wx.StatusBar`).

        :param `help`: the new item long help string.
        """

        self._helpString = help
        

    def GetLongHelp(self):
        """ Returns the item long help string (displayed in the parent frame `wx.StatusBar`). """

        return self._helpString


    def GetShortHelp(self):
        """ Returns the item short help string (displayed in the tool's tooltip). """

        return ""
    

    def Enable(self, enable=True):
        """
        Enables or disables a menu item.

        :param `enable`: ``True`` to enable the menu item, ``False`` to disable it.
        """

        self._bIsEnabled = enable
        if self._parentMenu:
            self._parentMenu.UpdateItem(self)


    def GetBitmap(self):
        """
        Returns the normal bitmap associated to the menu item or `wx.NullBitmap` if
        none has been supplied.
        """

        return self._normalBmp 


    def GetDisabledBitmap(self):
        """
        Returns the disabled bitmap associated to the menu item or `wx.NullBitmap`
        if none has been supplied.
        """

        return self._disabledBmp 


    def GetHotBitmap(self):
        """
        Returns the hot bitmap associated to the menu item or `wx.NullBitmap` if
        none has been supplied.
        """

        return self._hotBmp 

    
    def GetHelp(self):
        """ Returns the item help string. """

        return self._helpString 


    def GetId(self):
        """ Returns the item id. """

        return self._id 


    def GetKind(self):
        """
        Returns the menu item kind, can be one of ``wx.ITEM_SEPARATOR``, ``wx.ITEM_NORMAL``,
        ``wx.ITEM_CHECK`` or ``wx.ITEM_RADIO``.
        """

        return self._kind 


    def GetLabel(self):
        """ Returns the menu item label (without the accelerator if it is part of the string). """

        return self._label 


    def GetMenu(self):
        """ Returns the parent menu. """

        return self._parentMenu 


    def GetContextMenu(self):
        """ Returns the context menu associated with this item (if any). """

        return self._contextMenu


    def SetContextMenu(self, context_menu):
        """
        Assigns a context menu to this item.

        :param `context_menu`: an instance of L{FlatMenu}.
        """

        self._contextMenu = context_menu
        

    def GetText(self):
        """ Returns the text associated with the menu item including the accelerator. """

        return self._text 


    def GetSubMenu(self):
        """ Returns the sub-menu of this menu item (if any). """

        return self._subMenu


    def IsCheckable(self):
        """ Returns ``True`` if this item is of type ``wx.ITEM_CHECK``, ``False`` otherwise. """

        return self._kind == wx.ITEM_CHECK


    def IsChecked(self):
        """
        Returns whether an item is checked or not.

        :note: This method is meaningful only for items of kind ``wx.ITEM_CHECK`` or
         ``wx.ITEM_RADIO``.
        """

        return self._bIsChecked 


    def IsRadioItem(self):
        """ Returns ``True`` if this item is of type ``wx.ITEM_RADIO``, ``False`` otherwise. """

        return self._kind == wx.ITEM_RADIO


    def IsEnabled(self):
        """ Returns whether an item is enabled or not. """

        return self._bIsEnabled 


    def IsSeparator(self):
        """ Returns ``True`` if this item is of type ``wx.ITEM_SEPARATOR``, ``False`` otherwise. """

        return self._id == wx.ID_SEPARATOR
    

    def IsSubMenu(self):
        """ Returns whether an item is a sub-menu or not. """

        return self._subMenu != None
    

    def SetNormalBitmap(self, bmp):
        """
        Sets the menu item normal bitmap.

        :param `bmp`: an instance of `wx.Bitmap`.
        """

        self._normalBmp = bmp 


    def SetDisabledBitmap(self, bmp):
        """
        Sets the menu item disabled bitmap.

        :param `bmp`: an instance of `wx.Bitmap`.
        """

        self._disabledBmp = bmp 


    def SetHotBitmap(self, bmp):
        """
        Sets the menu item hot bitmap.

        :param `bmp`: an instance of `wx.Bitmap`.
        """

        self._hotBmp = bmp 


    def SetHelp(self, helpString):
        """
        Sets the menu item help string.

        :param `helpString`: the new menu item help string.
        """

        self._helpString = helpString 


    def SetMenu(self, menu):
        """
        Sets the menu item parent menu.

        :param `menu`: an instance of L{FlatMenu}.
        """

        self._parentMenu = menu 


    def SetSubMenu(self, menu):
        """
        Sets the menu item sub-menu.

        :param `menu`: an instance of L{FlatMenu}.
        """

        self._subMenu = menu
        
        # Fix toolbar update
        self.SetMenuBar()


    def GetAccelString(self):
        """ Returns the accelerator string. """

        return self._accelStr 


    def SetRect(self, rect):
        """
        Sets the menu item client rectangle.

        :param `rect`: the menu item client rectangle.
        """

        self._rect = rect 


    def GetRect(self):
        """ Returns the menu item client rectangle. """

        return self._rect 


    def IsShown(self):
        """ Returns whether an item is shown or not. """

        return self._visible


    def Show(self, show=True):
        """
        Actually shows/hides the menu item.

        :param `show`: ``True`` to show the menu item, ``False`` to hide it.
        """

        self._visible = show

    def GetHeight(self):
        """ Returns the menu item height. """

        if self.IsSeparator():
            return self._parentMenu.GetRenderer().separatorHeight
        else:
            return self._parentMenu._itemHeight


    def GetSuitableBitmap(self, selected):
        """
        Gets the bitmap that should be used based on the item state.

        :param `selected`: ``True`` if this menu item is currentl hovered by the mouse,
         ``False`` otherwise.
        """

        normalBmp = self._normalBmp
        gBmp = (self._disabledBmp.Ok() and [self._disabledBmp] or [self._normalBmp])[0]
        hotBmp = (self._hotBmp.Ok() and [self._hotBmp] or [self._normalBmp])[0]

        if not self.IsEnabled():
            return gBmp
        elif selected:
            return hotBmp
        else:
            return normalBmp


    def SetLabel(self, text):
        """
        Sets the label text for this item from the text (excluding the accelerator).

        :param `text`: the new item label (excluding the accelerator).
        """
 
        if text:

            indx = text.find("\t")
            if indx >= 0:
                self._accelStr = text[indx+1:]
                label = text[0:indx]
            else:
                self._accelStr = ""
                label = text

            self._mnemonicIdx, self._label = GetAccelIndex(label)
            
        else:
        
            self._mnemonicIdx = wx.NOT_FOUND
            self._label = ""

        if self._parentMenu:
            self._parentMenu.UpdateItem(self)


    def SetText(self, text):
        """
        Sets the text for this menu item (including accelerators).

        :param `text`: the new item label (including the accelerator).
        """
     
        self._text = text
        self.SetLabel(self._text)


    def SetMenuBar(self):
        """ Links the current menu item with the main L{FlatMenuBar}. """

        # Fix toolbar update
        if self._subMenu and self._parentMenu:
            self._subMenu.SetSubMenuBar(self._parentMenu.GetMenuBarForSubMenu())


    def GetAcceleratorEntry(self):
        """ Returns the accelerator entry associated to this menu item. """

        return wx.GetAccelFromString(self.GetText())


    def GetMnemonicChar(self):
        """ Returns the shortcut char for this menu item. """

        if self._mnemonicIdx == wx.NOT_FOUND:
            return 0

        mnemonic = self._label[self._mnemonicIdx]
        return mnemonic.lower()


    def Check(self, check=True):
        """
        Checks or unchecks the menu item.

        :param `check`: ``True`` to check the menu item, ``False`` to uncheck it.

        :note: This method is meaningful only for menu items of ``wx.ITEM_CHECK``
         or ``wx.ITEM_RADIO`` kind.
        """
 
        if self.IsRadioItem() and not self._isAttachedToMenu:
        
            # radio items can be checked only after they are attached to menu
            return
        
        self._bIsChecked = check
        
        # update group
        if self.IsRadioItem() and check:
            self._groupPtr.SetSelection(self)

        # Our parent menu might want to do something with this change
        if self._parentMenu:
            self._parentMenu.UpdateItem(self)


#--------------------------------------------------------
# Class FlatMenu
#--------------------------------------------------------

class FlatMenu(FlatMenuBase):
    """
    A Flat popup menu generic implementation.
    """
    
    def __init__(self, parent=None):
        """
        Default class constructor.

        :param `parent`: the L{FlatMenu} parent window (used to initialize the
         underlying L{ShadowPopupWindow}).
        """
                        
        self._menuWidth = 2*26
        self._leftMarginWidth = 26
        self._rightMarginWidth = 30
        self._borderXWidth = 1
        self._borderYWidth = 2
        self._activeWin = None
        self._focusWin = None
        self._imgMarginX = 0
        self._markerMarginX = 0
        self._textX = 26
        self._rightMarginPosX = -1
        self._itemHeight = 20
        self._selectedItem = -1
        self._clearCurrentSelection = True
        self._textPadding = 8
        self._marginHeight = 20
        self._marginWidth = 26
        self._accelWidth = 0
        self._mb = None
        self._itemsArr = []
        self._accelArray = []
        self._ptLast = wx.Point()
        self._resizeMenu = True
        self._shiftePos = 0
        self._first = 0
        self._mb_submenu = 0
        self._is_dismiss = False
        self._numCols = 1

        FlatMenuBase.__init__(self, parent)        

        self.SetSize(wx.Size(self._menuWidth, self._itemHeight+4))

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnMouseEnterWindow)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeaveWindow)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnMouseLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnMouseLeftUp)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnMouseLeftDown)
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnMouseRightDown)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)
        self.Bind(wx.EVT_TIMER, self.OnTimer)


    def SetMenuBar(self, mb):
        """
        Attaches this menu to a menubar.

        :param `mb`: an instance of L{FlatMenuBar}.
        """

        self._mb = mb


    def SetSubMenuBar(self, mb):
        """
        Attaches this menu to a menubar.

        :param `mb`: an instance of L{FlatMenuBar}.
        """

        self._mb_submenu = mb


    def GetMenuBar(self):
        """ Returns the menubar associated with this menu item. """

        if self._mb_submenu:
            return self._mb_submenu

        return self._mb


    def GetMenuBarForSubMenu(self):
        """ Returns the menubar associated with this menu item. """

        return self._mb
   
    
    def Popup(self, pt, owner=None, parent=None):
        """
        Pops up the menu.

        :param `pt`: the point at which the menu should be popped up (an instance
         of `wx.Point`);
        :param `owner`: the owner of the menu. The owner does not necessarly mean the
         menu parent, it can also be the window that popped up it;
        :param `parent`: the menu parent window.
        """

        if "__WXMSW__" in wx.Platform:
            self._mousePtAtStartup = wx.GetMousePosition()

        # each time we popup, need to reset the starting index
        self._first = 0
        
        # Loop over self menu and send update UI event for
        # every item in the menu
        numEvents = len(self._itemsArr)
        cc = 0
        self._shiftePos = 0

        # Set the owner of the menu. All events will be directed to it.
        # If owner is None, the Default GetParent() is used as the owner
        self._owner = owner

        for cc in xrange(numEvents):
            self.SendUIEvent(cc)

        # Adjust menu position and show it
        FlatMenuBase.Popup(self, pt, parent)

        artMgr = ArtManager.Get()
        artMgr.MakeWindowTransparent(self, artMgr.GetTransparency())

        # Replace the event handler of the active window to direct
        # all keyboard events to us and the focused window to direct char events to us
        self._activeWin = wx.GetActiveWindow()
        if self._activeWin:
        
            oldHandler = self._activeWin.GetEventHandler()
            newEvtHandler = MenuKbdRedirector(self, oldHandler)
            self._activeWin.PushEventHandler(newEvtHandler)
        
        if "__WXMSW__" in wx.Platform:
            self._focusWin = wx.Window.FindFocus()
        elif "__WXGTK__" in wx.Platform:
            self._focusWin = self
        else:
            self._focusWin = None

        if self._focusWin:
            newEvtHandler = FocusHandler(self)
            self._focusWin.PushEventHandler(newEvtHandler)

        
    def Append(self, id, item, helpString="", kind=wx.ITEM_NORMAL):
        """
        Appends an item to this menu.

        :param `id`: the menu item identifier;
        :param `item`: the string to appear on the menu item;
        :param `helpString`: an optional help string associated with the item. By default,
         the handler for the ``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` event displays this string
         in the status line;
        :param `kind`: may be ``wx.ITEM_NORMAL`` for a normal button (default),
         ``wx.ITEM_CHECK`` for a checkable tool (such tool stays pressed after it had been
         toggled) or ``wx.ITEM_RADIO`` for a checkable tool which makes part of a radio
         group of tools each of which is automatically unchecked whenever another button
         in the group is checked;
        """

        newItem = FlatMenuItem(self, id, item, helpString, kind)
        return self.AppendItem(newItem)
    
    
    def Prepend(self, id, item, helpString="", kind=wx.ITEM_NORMAL):
        """
        Prepends an item to this menu.

        :param `id`: the menu item identifier;
        :param `item`: the string to appear on the menu item;
        :param `helpString`: an optional help string associated with the item. By default,
         the handler for the ``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` event displays this string
         in the status line;
        :param `kind`: may be ``wx.ITEM_NORMAL`` for a normal button (default),
         ``wx.ITEM_CHECK`` for a checkable tool (such tool stays pressed after it had been
         toggled) or ``wx.ITEM_RADIO`` for a checkable tool which makes part of a radio
         group of tools each of which is automatically unchecked whenever another button
         in the group is checked;
        """

        newItem = FlatMenuItem(self, id, item, helpString, kind)
        return self.PrependItem(newItem)


    def AppendSubMenu(self, subMenu, item, helpString=""):
        """
        Adds a pull-right submenu to the end of the menu.
        
        This function is added to duplicate the API of `wx.Menu`.

        :see: L{AppendMenu} for an explanation of the input parameters.        
        """
        
        return self.AppendMenu(wx.ID_ANY, item, subMenu, helpString)
        
        
    def AppendMenu(self, id, item, subMenu, helpString=""):
        """
        Adds a pull-right submenu to the end of the menu.

        :param `id`: the menu item identifier;
        :param `item`: the string to appear on the menu item;
        :param `subMenu`: an instance of L{FlatMenu}, the submenu to append;
        :param `helpString`: an optional help string associated with the item. By default,
         the handler for the ``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` event displays this string
         in the status line.
        """

        newItem = FlatMenuItem(self, id, item, helpString, wx.ITEM_NORMAL, subMenu)
        return self.AppendItem(newItem)


    def AppendItem(self, menuItem):
        """
        Appends an item to this menu.

        :param `menuItem`: an instance of L{FlatMenuItem}.
        """
        
        self._itemsArr.append(menuItem)
        return self.AddItem(menuItem)
   
        
    def PrependItem(self, menuItem):
        """
        Prepends an item to this menu.

        :param `menuItem`: an instance of L{FlatMenuItem}.
        """
        
        self._itemsArr.insert(0,menuItem)
        return self.AddItem(menuItem)
        

    def AddItem(self, menuItem):
        """
        Internal function to add the item to this menu. The item must
        already be in self._itemsArr.

        :param `menuItem`: an instance of L{FlatMenuItem}.
        """

        if not menuItem:
            raise Exception("Adding None item?")
        
        # Reparent to us
        menuItem.SetMenu(self) 
        menuItem._isAttachedToMenu = True

        # Update the menu width if necessary
        menuItemWidth = self.GetMenuItemWidth(menuItem)
        self._menuWidth = (self._menuWidth > menuItemWidth + self._accelWidth and \
                           [self._menuWidth] or [menuItemWidth + self._accelWidth])[0]

        menuHeight = 0
        switch = 1e6
        
        if self._numCols > 1:
            nItems = len(self._itemsArr)
            switch = int(math.ceil((nItems - self._first)/float(self._numCols)))
            
        for indx, item in enumerate(self._itemsArr):

            if indx >= switch:
                break
            
            if item.IsSeparator():
                menuHeight += self.GetRenderer().separatorHeight
            else:
                menuHeight += self._itemHeight
                    
        self.SetSize(wx.Size(self._menuWidth*self._numCols, menuHeight+4))

        # Add accelerator entry to the menu if needed
        accel = menuItem.GetAcceleratorEntry()
        
        if accel:
            accel.Set(accel.GetFlags(), accel.GetKeyCode(), menuItem.GetId())
            self._accelArray.append(accel)        

        self.UpdateRadioGroup(menuItem)

        return menuItem


    def GetMenuItems(self):
        """ Returns the list of menu items in the menu. """

        return self._itemsArr
    

    def GetMenuItemWidth(self, menuItem):
        """
        Returns the width of a particular item.

        :param `menuItem`: an instance of L{FlatMenuItem}.
        """

        menuItemWidth = 0
        text = menuItem.GetLabel() # Without accelerator
        accel = menuItem.GetAccelString()

        dc = wx.ClientDC(self)

        font = ArtManager.Get().GetFont()
        dc.SetFont(font)

        accelFiller = "XXXX"     # 4 spaces betweem text and accel column

        # Calc text length/height
        dummy, itemHeight = dc.GetTextExtent("Tp")
        width, height = dc.GetTextExtent(text)
        accelWidth, accelHeight = dc.GetTextExtent(accel)
        filler, dummy = dc.GetTextExtent(accelFiller)
        
        bmpHeight = bmpWidth = 0
        
        if menuItem.GetBitmap().Ok():
            bmpHeight = menuItem.GetBitmap().GetHeight()
            bmpWidth  = menuItem.GetBitmap().GetWidth()
        
        if itemHeight < self._marginHeight:
            itemHeight = self._marginHeight

        itemHeight = (bmpHeight > self._itemHeight and [bmpHeight] or [itemHeight])[0]
        itemHeight += 2*self._borderYWidth

        # Update the global menu item height if needed
        self._itemHeight = (self._itemHeight > itemHeight and [self._itemHeight] or [itemHeight])[0]
        self._marginWidth = (self._marginWidth > bmpWidth and [self._marginWidth] or [bmpWidth])[0]

        # Update the accel width
        accelWidth += filler
        if accel:
            self._accelWidth = (self._accelWidth > accelWidth and [self._accelWidth] or [accelWidth])[0]

        # In case the item has image & is type radio or check, we need double size
        # left margin
        factor = (((menuItem.GetBitmap() != wx.NullBitmap) and \
                   (menuItem.IsCheckable() or (menuItem.GetKind() == wx.ITEM_RADIO))) and [2] or [1])[0]
        
        if factor == 2:
        
            self._imgMarginX = self._marginWidth + 2*self._borderXWidth
            self._leftMarginWidth = 2 * self._marginWidth + 2*self._borderXWidth
        
        else:
            
            self._leftMarginWidth = ((self._leftMarginWidth > self._marginWidth + 2*self._borderXWidth) and \
                                    [self._leftMarginWidth] or [self._marginWidth + 2*self._borderXWidth])[0]
        
        menuItemWidth = self.GetLeftMarginWidth() + 2*self.GetBorderXWidth() + width + self.GetRightMarginWidth()
        self._textX = self._imgMarginX + self._marginWidth + self._textPadding

        # update the rightMargin X position
        self._rightMarginPosX = ((self._textX + width + self._accelWidth> self._rightMarginPosX) and \
                                 [self._textX + width + self._accelWidth] or [self._rightMarginPosX])[0]
        
        return menuItemWidth


    def GetMenuWidth(self):
        """ Returns the menu width. """

        return self._menuWidth
    
    
    def GetLeftMarginWidth(self):
        """ Returns the menu left margin width. """

        return self._leftMarginWidth

    
    def GetRightMarginWidth(self):
        """ Returns the menu right margin width. """

        return self._rightMarginWidth

    
    def GetBorderXWidth(self):
        """ Returns the menu border x-width. """

        return self._borderXWidth


    def GetBorderYWidth(self):
        """ Returns the menu border y-width. """

        return self._borderYWidth

    
    def GetItemHeight(self):
        """ Returns the height of a particular item. """

        return self._itemHeight


    def AppendCheckItem(self, id, item, helpString=""):
        """
        Adds a checkable item to the end of the menu.

        :see: L{Append} for the explanation of the input parameters.        
        """

        newItem = FlatMenuItem(self, id, item, helpString, wx.ITEM_CHECK)
        return self.AppendItem(newItem)


    def AppendRadioItem(self, id, item, helpString=""):
        """
        Adds a radio item to the end of the menu.
        All consequent radio items form a group and when an item in the group is
        checked, all the others are automatically unchecked.

        :see: L{Append} for the explanation of the input parameters.        
        """
        
        newItem = FlatMenuItem(self, id, item, helpString, wx.ITEM_RADIO)
        return self.AppendItem(newItem)


    def AppendSeparator(self):
        """ Appends a separator item to teh end of this menu. """
        
        newItem = FlatMenuItem(self)
        return self.AppendItem(newItem)


    def InsertSeparator(self, pos):
        """
        Inserts a separator at the given position.

        :param `pos`: the index at which we want to insert the separator.    
        """

        newItem = FlatMenuItem(self)
        return self.Insert(pos, newItem)


    def Dismiss(self, dismissParent, resetOwner):
        """
        Dismisses the popup window.

        :param `dismissParent`: whether to dismiss the parent menu or not;
        :param `resetOwner`: ``True`` to delete the link between this menu and the
         owner menu, ``False`` otherwise.        
        """

        if self._activeWin:
        
            self._activeWin.PopEventHandler(True)
            self._activeWin = None

        if self._focusWin:
        
            self._focusWin.PopEventHandler(True)
            self._focusWin = None
        
        self._selectedItem = -1
        
        if self._mb:
            self._mb.RemoveHelp()   

        FlatMenuBase.Dismiss(self, dismissParent, resetOwner)

    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{FlatMenu}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """
        
        dc = wx.PaintDC(self)
        self.GetRenderer().DrawMenu(self, dc)

        # We need to redraw all our child menus
        self.RefreshChilds()


    def UpdateItem(self, item):
        """
        Updates an item.

        :param `item`: an instance of L{FlatMenuItem}.
        """

        # notify menu bar that an item was modified directly
        if item and self._mb:
            self._mb.UpdateItem(item)


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{FlatMenu}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to avoid flicker.        
        """

        pass

    
    def DrawSelection(self, dc, oldSelection=-1):
        """
        Redraws the menu.

        :param `dc`: an instance of `wx.DC`;
        :param `oldSelection`: if >= 0, the index representing the previous selected
         menu item.
        """

        self.Refresh()
        

    def RefreshChilds(self):
        """
        In some cases, we need to perform a recursive refresh for all opened submenu
        from this.
        """

        # Draw all childs menus of self menu as well
        child = self._openedSubMenu
        while child:
            dc = wx.ClientDC(child)
            self.GetRenderer().DrawMenu(child, dc)
            child = child._openedSubMenu


    def GetMenuRect(self):
        """ Returns the menu client rectangle. """

        clientRect = self.GetClientRect()
        return wx.Rect(clientRect.x, clientRect.y, clientRect.width, clientRect.height)


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{FlatMenu}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        self.OnChar(event.GetKeyCode())


    def OnChar(self, key):
        """
        Handles key events for L{FlatMenu}.

        :param `key`: the keyboard key integer code.
        """
        
        processed = True

        if key == wx.WXK_ESCAPE:

            if self._parentMenu:
                self._parentMenu.CloseSubMenu(-1)
            else:
                self.Dismiss(True, True)

        elif key == wx.WXK_LEFT:

            if self._parentMenu:
                # We are a submenu, dismiss us.
                self._parentMenu.CloseSubMenu(-1)
            else:               
                # try to find our root menu, if we are attached to menubar,
                # let it try and open the previous menu
                root = self.GetRootMenu()
                if root:
                    if root._mb:
                        root._mb.ActivatePreviousMenu()
                    
        elif key == wx.WXK_RIGHT:

            if not self.TryOpenSubMenu(self._selectedItem, True):
                # try to find our root menu, if we are attached to menubar,
                # let it try and open the previous menu
                root = self.GetRootMenu()
                if root:
                    if root._mb:
                        root._mb.ActivateNextMenu()
                            
        elif key == wx.WXK_UP:
            self.AdvanceSelection(False)
            
        elif key == wx.WXK_DOWN:
            
            self.AdvanceSelection()

        elif key in [wx.WXK_RETURN, wx.WXK_NUMPAD_ENTER]:
            self.DoAction(self._selectedItem)
            
        elif key == wx.WXK_HOME:
            
            # Select first item of the menu
            if self._selectedItem != 0:
                oldSel = self._selectedItem
                self._selectedItem = 0
                dc = wx.ClientDC(self)
                self.DrawSelection(dc, oldSel)

        elif key == wx.WXK_END:
            
            # Select last item of the menu
            if self._selectedItem != len(self._itemsArr)-1:
                oldSel = self._selectedItem
                self._selectedItem = len(self._itemsArr)-1
                dc = wx.ClientDC(self)
                self.DrawSelection(dc, oldSel)
            
        elif key in [wx.WXK_CONTROL, wx.WXK_ALT]:
            # Alt was pressed
            root = self.GetRootMenu()
            root.Dismiss(False, True)
            
        else:
            try:
                chrkey = chr(key)
            except:
                return processed
            
            if chrkey.isalnum():
            
                ch = chrkey.lower()

                # Iterate over all the menu items 
                itemIdx = -1
                occur = 0
                
                for i in xrange(len(self._itemsArr)):
                
                    item = self._itemsArr[i]
                    mnemonic = item.GetMnemonicChar()

                    if mnemonic == ch:
                    
                        if itemIdx == -1:
                        
                            itemIdx = i
                            # We keep the index of only 
                            # the first occurence
                        
                        occur += 1

                        # Keep on looping until no more items for self menu
                    
                if itemIdx != -1:
                
                    if occur > 1:
                    
                        # We select the first item
                        if self._selectedItem == itemIdx:
                            return processed

                        oldSel = self._selectedItem
                        self._selectedItem = itemIdx
                        dc = wx.ClientDC(self)
                        self.DrawSelection(dc, oldSel)
                    
                    elif occur == 1:
                    
                        # Activate the item, if self is a submenu item we first select it
                        item = self._itemsArr[itemIdx]
                        if item.IsSubMenu() and self._selectedItem != itemIdx:
                        
                            oldSel = self._selectedItem
                            self._selectedItem = itemIdx
                            dc = wx.ClientDC(self)
                            self.DrawSelection(dc, oldSel)
                        
                        self.DoAction(itemIdx)
                    
                else:
                
                    processed = False
        
        return processed


    def AdvanceSelection(self, down=True):
        """
        Advance forward or backward the current selection.

        :param `down`: ``True`` to advance the selection forward, ``False`` otherwise.
        """

        # make sure we have at least two items in the menu (which are not 
        # separators)
        num=0
        singleItemIdx = -1

        for i in xrange(len(self._itemsArr)):
        
            item = self._itemsArr[i]
            if item.IsSeparator():
                continue
            num += 1
            singleItemIdx = i
        
        if num < 1:
            return

        if num == 1: 
            # Select the current one
            self._selectedItem = singleItemIdx
            dc = wx.ClientDC(self)
            self.DrawSelection(dc, -1)
            return

        oldSelection = self._selectedItem
        
        if not down:
        
            # find the next valid item
            while 1:
            
                self._selectedItem -= 1
                if self._selectedItem < 0:
                    self._selectedItem = len(self._itemsArr)-1
                if not self._itemsArr[self._selectedItem].IsSeparator():
                    break
            
        else:
        
            # find the next valid item
            while 1:
            
                self._selectedItem += 1
                if self._selectedItem > len(self._itemsArr)-1:
                    self._selectedItem = 0
                if not self._itemsArr[self._selectedItem].IsSeparator():
                    break
            
        dc = wx.ClientDC(self)
        self.DrawSelection(dc, oldSelection)


    def HitTest(self, pos):
        """
        HitTest method for L{FlatMenu}.

        :param `pos`: an instance of `wx.Point`, a point to test for hits.
        """

        if self._showScrollButtons:

            if self._upButton and self._upButton.GetClientRect().Contains(pos):
                return MENU_HT_SCROLL_UP, -1
        
            if self._downButton and self._downButton.GetClientRect().Contains(pos):
                return MENU_HT_SCROLL_DOWN, -1

        for ii, item in enumerate(self._itemsArr):
                    
            if item.GetRect().Contains(pos) and item.IsEnabled() and item.IsShown():
                return MENU_HT_ITEM, ii
        
        return MENU_HT_NONE, -1


    def OnMouseMove(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{FlatMenu}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if "__WXMSW__" in wx.Platform:
            # Ignore dummy mouse move events
            pt = wx.GetMousePosition()
            if self._mousePtAtStartup == pt:
                return
        
        pos = event.GetPosition()

        # we need to ignore extra mouse events: example when this happens is when
        # the mouse is on the menu and we open a submenu from keyboard - Windows
        # then sends us a dummy mouse move event, we (correctly) determine that it
        # happens in the parent menu and so immediately close the just opened
        # submenunot
        
        if "__WXMSW__" in wx.Platform:

            ptCur = self.ClientToScreen(pos)
            if ptCur == self._ptLast:
                return
        
            self._ptLast = ptCur

        # first let the scrollbar handle it
        self.TryScrollButtons(event)
        self.ProcessMouseMove(pos)


    def OnMouseLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{FlatMenu}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.TryScrollButtons(event):
            return

        pos = event.GetPosition()
        self.ProcessMouseLClick(pos)


    def OnMouseLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{FlatMenu}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.TryScrollButtons(event):
            return

        pos = event.GetPosition()
        rect = self.GetClientRect()
        
        if not rect.Contains(pos):
        
            # The event is not in our coords, 
            # so we try our parent
            win = self._parentMenu

            while win:
            
                # we need to translate our client coords to the client coords of the
                # window we forward this event to
                ptScreen = self.ClientToScreen(pos)
                p = win.ScreenToClient(ptScreen)
                
                if win.GetClientRect().Contains(p):
                
                    event.m_x = p.x
                    event.m_y = p.y
                    win.OnMouseLeftUp(event)
                    return
                
                else:
                    # try the grandparent
                    win = win._parentMenu

        else:
            self.ProcessMouseLClickEnd(pos)
            
        if self._showScrollButtons:

            if self._upButton:
                self._upButton.ProcessLeftUp(pos)
            if self._downButton:
                self._downButton.ProcessLeftUp(pos)
            

    def OnMouseRightDown(self, event):
        """
        Handles the ``wx.EVT_RIGHT_DOWN`` event for L{FlatMenu}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.TryScrollButtons(event):
            return
        
        pos = event.GetPosition()
        self.ProcessMouseRClick(pos)


    def ProcessMouseRClick(self, pos):
        """
        Processes mouse right clicks.

        :param `pos`: the position at which the mouse right button was pressed.
        """

        rect = self.GetClientRect()
        
        if not rect.Contains(pos):
        
            # The event is not in our coords, 
            # so we try our parent

            win = self._parentMenu
            while win:
            
                # we need to translate our client coords to the client coords of the
                # window we forward self event to
                ptScreen = self.ClientToScreen(pos)
                p = win.ScreenToClient(ptScreen)
                
                if win.GetClientRect().Contains(p):
                    win.ProcessMouseRClick(p)
                    return
                
                else:
                    # try the grandparent
                    win = win._parentMenu
            
            # At this point we can assume that the event was not 
            # processed, so we dismiss the menu and its children
            self.Dismiss(True, True)
            return
        
        # test if we are on a menu item
        res, itemIdx = self.HitTest(pos)
        if res == MENU_HT_ITEM:
            self.OpenItemContextMenu(itemIdx)


    def OpenItemContextMenu(self, itemIdx):
        """
        Open an item's context menu (if any).

        :param `itemIdx`: the index of the item for which we want to open the context menu.
        """

        item = self._itemsArr[itemIdx]
        context_menu = item.GetContextMenu()

        # If we have a context menu, close any opened submenu
        if context_menu:
            self.CloseSubMenu(itemIdx, True)

        if context_menu and not context_menu.IsShown():
            # Popup child menu
            pos = wx.Point()
            pos.x = item.GetRect().GetWidth() + item.GetRect().GetX() - 5
            pos.y = item.GetRect().GetY()
            self._clearCurrentSelection = False
            self._openedSubMenu = context_menu
            context_menu.Popup(self.ScreenToClient(wx.GetMousePosition()), self._owner, self)
            return True

        return False
    

    def ProcessMouseLClick(self, pos):
        """
        Processes mouse left clicks.

        :param `pos`: the position at which the mouse left button was pressed.
        """
        
        rect = self.GetClientRect()
        
        if not rect.Contains(pos):
        
            # The event is not in our coords, 
            # so we try our parent

            win = self._parentMenu
            while win:
            
                # we need to translate our client coords to the client coords of the
                # window we forward self event to
                ptScreen = self.ClientToScreen(pos)
                p = win.ScreenToClient(ptScreen)
                
                if win.GetClientRect().Contains(p):                
                    win.ProcessMouseLClick(p)
                    return
                
                else:
                    # try the grandparent
                    win = win._parentMenu
            
            # At this point we can assume that the event was not 
            # processed, so we dismiss the menu and its children
            self.Dismiss(True, True)
            return


    def ProcessMouseLClickEnd(self, pos):
        """
        Processes mouse left clicks.

        :param `pos`: the position at which the mouse left button was pressed.
        """

        self.ProcessMouseLClick(pos)

        # test if we are on a menu item        
        res, itemIdx = self.HitTest(pos)
        
        if res == MENU_HT_ITEM:
            self.DoAction(itemIdx)

        elif res == MENU_HT_SCROLL_UP:
            if self._upButton:
                self._upButton.ProcessLeftDown(pos)

        elif res == MENU_HT_SCROLL_DOWN:
            if self._downButton:
                self._downButton.ProcessLeftDown(pos)

        else:
            self._selectedItem = -1


    def ProcessMouseMove(self, pos):
        """
        Processes mouse movements.

        :param `pos`: the position at which the mouse was moved.
        """

        rect = self.GetClientRect()
        
        if not rect.Contains(pos):
        
            # The event is not in our coords, 
            # so we try our parent

            win = self._parentMenu
            while win:
            
                # we need to translate our client coords to the client coords of the
                # window we forward self event to
                ptScreen = self.ClientToScreen(pos)
                p = win.ScreenToClient(ptScreen)

                if win.GetClientRect().Contains(p):
                    win.ProcessMouseMove(p)
                    return
                
                else:
                    # try the grandparent
                    win = win._parentMenu
            
            # If we are attached to a menu bar, 
            # let him process the event as well
            if self._mb:
            
                ptScreen = self.ClientToScreen(pos)
                p = self._mb.ScreenToClient(ptScreen)
                
                if self._mb.GetClientRect().Contains(p):
                
                    # let the menu bar process it
                    self._mb.ProcessMouseMoveFromMenu(p)
                    return

            if self._mb_submenu:
                ptScreen = self.ClientToScreen(pos)
                p = self._mb_submenu.ScreenToClient(ptScreen)
                if self._mb_submenu.GetClientRect().Contains(p):
                    # let the menu bar process it
                    self._mb_submenu.ProcessMouseMoveFromMenu(p)
                    return

            return
        
        # test if we are on a menu item
        res, itemIdx = self.HitTest(pos)

        if res == MENU_HT_SCROLL_DOWN:

            if self._downButton:
                self._downButton.ProcessMouseMove(pos)

        elif res == MENU_HT_SCROLL_UP:

            if self._upButton:
                self._upButton.ProcessMouseMove(pos)

        elif res == MENU_HT_ITEM:

            if self._downButton:
                self._downButton.ProcessMouseMove(pos)

            if self._upButton:
                self._upButton.ProcessMouseMove(pos)

            if self._selectedItem == itemIdx:
                return

            # Message to send when out of last selected item
            if self._selectedItem != -1:
                self.SendOverItem(self._selectedItem, False)
            self.SendOverItem(itemIdx, True)   # Message to send when over an item
            
            oldSelection = self._selectedItem
            self._selectedItem = itemIdx
            self.CloseSubMenu(self._selectedItem)

            dc = wx.ClientDC(self)
            self.DrawSelection(dc, oldSelection)

            self.TryOpenSubMenu(self._selectedItem)

            if self._mb:
                self._mb.RemoveHelp()
                if itemIdx >= 0:
                    self._mb.DoGiveHelp(self._itemsArr[itemIdx])

        else:

            # Message to send when out of last selected item
            if self._selectedItem != -1:
                item = self._itemsArr[self._selectedItem]
                if item.IsSubMenu() and item.GetSubMenu().IsShown():
                    return

                # Message to send when out of last selected item
                if self._selectedItem != -1:
                    self.SendOverItem(self._selectedItem, False)
                    
            oldSelection = self._selectedItem
            self._selectedItem = -1
            dc = wx.ClientDC(self)
            self.DrawSelection(dc, oldSelection)
            

    def OnMouseLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{FlatMenu}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self._mb:
            self._mb.RemoveHelp()
            
        if self._clearCurrentSelection:

            # Message to send when out of last selected item
            if self._selectedItem != -1:
                item = self._itemsArr[self._selectedItem]
                if item.IsSubMenu() and item.GetSubMenu().IsShown():
                    return
                
                # Message to send when out of last selected item
                if self._selectedItem != -1:
                    self.SendOverItem(self._selectedItem, False)

            oldSelection = self._selectedItem
            self._selectedItem = -1
            dc = wx.ClientDC(self)
            self.DrawSelection(dc, oldSelection)
        
        self._clearCurrentSelection = True

        if "__WXMSW__" in wx.Platform:
            self.SetCursor(self._oldCur)


    def OnMouseEnterWindow(self, event):
        """
        Handles the ``wx.EVT_ENTER_WINDOW`` event for L{FlatMenu}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if "__WXMSW__" in wx.Platform:
            self._oldCur = self.GetCursor()
            self.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))

        event.Skip()


    def OnKillFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{FlatMenu}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """
        
        self.Dismiss(True, True)


    def CloseSubMenu(self, itemIdx, alwaysClose=False):
        """
        Closes a child sub-menu.

        :param `itemIdx`: the index of the item for which we want to close the submenu;
        :param `alwaysClose`: if ``True``, always close the submenu irrespectively of
         other conditions.
        """

        item = None
        subMenu = None

        if itemIdx >= 0 and itemIdx < len(self._itemsArr):
            item = self._itemsArr[itemIdx]

        # Close sub-menu first
        if item:
            subMenu = item.GetSubMenu()

        if self._openedSubMenu:
            if self._openedSubMenu != subMenu or alwaysClose:
                # We have another sub-menu open, close it 
                self._openedSubMenu.Dismiss(False, True)
                self._openedSubMenu = None
        

    def DoAction(self, itemIdx):
        """
        Performs an action based on user selection.

        :param `itemIdx`: the index of the item for which we want to perform the action.
        """

        if itemIdx < 0 or itemIdx >= len(self._itemsArr):
            raise Exception("Invalid menu item")
            return

        item = self._itemsArr[itemIdx]
        
        if not item.IsEnabled() or item.IsSeparator():
            return

        # Close sub-menu if needed
        self.CloseSubMenu(itemIdx)
        
        if item.IsSubMenu() and not item.GetSubMenu().IsShown():
        
            # Popup child menu
            self.TryOpenSubMenu(itemIdx)
            return

        if item.IsRadioItem():
            # if the radio item is already checked, 
            # just send command event. Else, check it, uncheck the current
            # checked item in the radio item group, and send command event
            if not item.IsChecked():
                item._groupPtr.SetSelection(item)
            
        elif item.IsCheckable():
            
            item.Check(not item.IsChecked())
            dc = wx.ClientDC(self)
            self.DrawSelection(dc)
        
        if not item.IsSubMenu():
        
            self.Dismiss(True, False)

            # Send command event
            self.SendCmdEvent(itemIdx)


    def TryOpenSubMenu(self, itemIdx, selectFirst=False):
        """
        If `itemIdx` is an item with submenu, open it.

        :param `itemIdx`: the index of the item for which we want to open the submenu;
        :param `selectFirst`: if ``True``, the first item of the submenu will be shown
         as selected.
        """
        
        if itemIdx < 0 or itemIdx >= len(self._itemsArr):
            return False

        item = self._itemsArr[itemIdx]
        if item.IsSubMenu() and not item.GetSubMenu().IsShown():
            
            pos = wx.Point()

            # Popup child menu
            pos.x = item.GetRect().GetWidth()+ item.GetRect().GetX()-5
            pos.y = item.GetRect().GetY()
            self._clearCurrentSelection = False
            self._openedSubMenu = item.GetSubMenu()
            item.GetSubMenu().Popup(pos, self._owner, self)
            
            # Select the first child
            if selectFirst:
            
                dc = wx.ClientDC(item.GetSubMenu())
                item.GetSubMenu()._selectedItem = 0
                item.GetSubMenu().DrawSelection(dc)
            
            return True
        
        return False


    def _RemoveById(self, id):
        """ Used internally. """
        
        # First we search for the menu item (recursively)
        menuParent = None
        item = None
        idx = wx.NOT_FOUND
        idx, menuParent = self.FindMenuItemPos(id)
        
        if idx != wx.NOT_FOUND:
        
            # Remove the menu item
            item = menuParent._itemsArr[idx]
            menuParent._itemsArr.pop(idx)

            # update group
            if item._groupPtr and item.IsRadioItem():
                item._groupPtr.Remove(item)
            
            # Resize the menu
            menuParent.ResizeMenu()
        
        return item


    def Remove(self, item):
        """
        Removes the menu item from the menu but doesn't delete the associated menu
        object. This allows to reuse the same item later by adding it back to the
        menu (especially useful with submenus).

        :param `item`: can be either a menu item identifier or a plain L{FlatMenuItem}.
        """

        if type(item) != type(1):
            item = item.GetId()
            
        return self._RemoveById(item)


    def _DestroyById(self, id):
        """ Used internally. """

        item = None
        item = self.Remove(id)
        
        if item:
            del item


    def Destroy(self, item):
        """
        Deletes the menu item from the menu. If the item is a submenu, it will be
        deleted. Use L{Remove} if you want to keep the submenu (for example, to reuse
        it later).

        :param `item`: can be either a menu item identifier or a plain L{FlatMenuItem}.        
        """

        if type(item) != type(1):
            item = item.GetId()

        self._DestroyById(item)
        

    def Insert(self, pos, id, item, helpString="", kind=wx.ITEM_NORMAL):
        """
        Inserts the given `item` before the position `pos`.

        :param `pos`: the position at which to insert the new menu item;
        :param `id`: the menu item identifier;
        :param `item`: the string to appear on the menu item;
        :param `helpString`: an optional help string associated with the item. By default,
         the handler for the ``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` event displays this string
         in the status line;
        :param `kind`: may be ``wx.ITEM_NORMAL`` for a normal button (default),
         ``wx.ITEM_CHECK`` for a checkable tool (such tool stays pressed after it had been
         toggled) or ``wx.ITEM_RADIO`` for a checkable tool which makes part of a radio
         group of tools each of which is automatically unchecked whenever another button
         in the group is checked;
        """
        
        newitem = FlatMenuItem(self, id, item, helpString, kind)
        return self.InsertItem(pos, newitem)


    def InsertItem(self, pos, item):
        """
        Inserts an item into the menu.

        :param `pos`: the position at which to insert the new menu item;
        :param `item`: an instance of L{FlatMenuItem}.
        """

        if pos == len(self._itemsArr):
            # Append it
            return self.AppendItem(item)

        # Insert the menu item 
        self._itemsArr.insert(pos, item)
        item._isAttachedToMenu = True

        # Recalculate the menu geometry
        self.ResizeMenu()
        
        # Update radio groups
        self.UpdateRadioGroup(item)

        return item


    def UpdateRadioGroup(self, item):
        """
        Updates a group of radio items.

        :param `item`: an instance of L{FlatMenuItem}.
        """

        if item.IsRadioItem():
    
            # Udpate radio groups in case this item is a radio item
            sibling = self.GetSiblingGroupItem(item)
            if sibling:
            
                item._groupPtr = sibling._groupPtr
                item._groupPtr.Add(item)

                if item.IsChecked():
                
                    item._groupPtr.SetSelection(item)
                
            else:
            
                # first item in group
                item._groupPtr = FlatMenuItemGroup()
                item._groupPtr.Add(item)
                item._groupPtr.SetSelection(item)

        
    def ResizeMenu(self):
        """ Resizes the menu to the correct size. """

        # can we do the resize?
        if not self._resizeMenu:
            return

        items = self._itemsArr
        self._itemsArr = []

        # Clear accelerator table
        self._accelArray = []

        # Reset parameters and menu size
        self._menuWidth =  2*self._marginWidth
        self._imgMarginX = 0
        self._markerMarginX = 0
        self._textX = self._marginWidth
        self._rightMarginPosX = -1
        self._itemHeight = self._marginHeight
        self.SetSize(wx.Size(self._menuWidth*self._numCols, self._itemHeight+4))

        # Now we simply add the items 
        for item in items:
            self.AppendItem(item)


    def SetNumberColumns(self, numCols):
        """
        Sets the number of columns for a menu window.

        :param `numCols`: the number of columns for this L{FlatMenu} window.
        """

        if self._numCols == numCols:
            return
        
        self._numCols = numCols
        self.ResizeMenu()
        self.Refresh()


    def GetNumberColumns(self):
        """ Returns the number of columns for a menu window. """

        return self._numCols
    

    def FindItem(self, itemId, menu=None):
        """
        Finds the menu item object associated with the given menu item identifier and, 
        optionally, the (sub)menu it belongs to.

        :param `itemId`: menu item identifier;
        :param `menu`: if not ``None``, it will be filled with the item's parent menu
         (if the item was found).
        """
        
        idx = wx.NOT_FOUND
        
        if menu:
        
            idx, menu = self.FindMenuItemPos(itemId, menu)
            if idx != wx.NOT_FOUND:
                return menu._itemsArr[idx]
            else:
                return None
        
        else:
        
            idx, parentMenu = self.FindMenuItemPos(itemId, None)
            if idx != wx.NOT_FOUND:
                return parentMenu._itemsArr[idx]
            else:
                return None
            

    def FindMenuItemPos(self, itemId, menu=None):
        """
        Finds an item and its position inside the menu based on its id.

        :param `itemId`: menu item identifier;
        :param `menu`: if not ``None``, it will be filled with the item's parent menu
         (if the item was found).
        """
        
        menu = None
        item = None

        idx = wx.NOT_FOUND

        for i in xrange(len(self._itemsArr)):
        
            item = self._itemsArr[i]

            if item.GetId() == itemId:
            
                menu = self
                idx = i
                break
            
            elif item.IsSubMenu():
            
                idx, menu = item.GetSubMenu().FindMenuItemPos(itemId, menu)
                if idx != wx.NOT_FOUND:
                    break
            
            else:
                
                item = None
            
        return idx, menu


    def GetAccelTable(self):
        """ Returns the menu accelerator table. """
        
        n = len(self._accelArray)
        if n == 0:
            return wx.NullAcceleratorTable
        
        entries = [wx.AcceleratorEntry() for ii in xrange(n)]

        for counter in len(entries):
            entries[counter] = self._accelArray[counter]
        
        table = wx.AcceleratorTable(entries)
        del entries

        return table


    def GetAccelArray(self):
        """ Returns an array filled with the accelerator entries for the menu. """

        return self._accelArray


    # events 
    def SendCmdEvent(self, itemIdx):
        """
        Actually sends menu command events.

        :param `itemIdx`: the menu item index for which we want to send a command event.
        """
        
        if itemIdx < 0 or itemIdx >= len(self._itemsArr):
            raise Exception("Invalid menu item")
            return
        
        item = self._itemsArr[itemIdx]

        # Create the event
        event = wx.CommandEvent(wxEVT_FLAT_MENU_SELECTED, item.GetId())

        # For checkable item, set the IsChecked() value
        if item.IsCheckable():
            event.SetInt((item.IsChecked() and [1] or [0])[0])
        
        event.SetEventObject(self)

        if self._owner:
            self._owner.GetEventHandler().ProcessEvent(event)
        else:
            self.GetEventHandler().ProcessEvent(event)


    def SendOverItem(self, itemIdx, over):
        """
        Sends the ``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` and ``EVT_FLAT_MENU_ITEM_MOUSE_OUT``
        events.

        :param `itemIdx`: the menu item index for which we want to send an event;
        :param `over`: ``True`` to send a ``EVT_FLAT_MENU_ITEM_MOUSE_OVER`` event, ``False`` to
         send a ``EVT_FLAT_MENU_ITEM_MOUSE_OUT`` event.
        """

        item = self._itemsArr[itemIdx]

        # Create the event
        event = FlatMenuEvent((over and [wxEVT_FLAT_MENU_ITEM_MOUSE_OVER] or [wxEVT_FLAT_MENU_ITEM_MOUSE_OUT])[0], item.GetId())

        # For checkable item, set the IsChecked() value
        if item.IsCheckable():
            event.SetInt((item.IsChecked() and [1] or [0])[0])
            
        event.SetEventObject(self)

        if self._owner:
            self._owner.GetEventHandler().ProcessEvent(event)
        else:
            self.GetEventHandler().ProcessEvent(event)


    def SendUIEvent(self, itemIdx):
        """
        Actually sends menu UI events.

        :param `itemIdx`: the menu item index for which we want to send a UI event.
        """
        
        if itemIdx < 0 or itemIdx >= len(self._itemsArr):
            raise Exception("Invalid menu item")
            return
        
        item = self._itemsArr[itemIdx]
        event = wx.UpdateUIEvent(item.GetId())
        
        event.Check(item.IsChecked())
        event.Enable(item.IsEnabled())
        event.SetText(item.GetText())
        event.SetEventObject(self)

        if self._owner:
            self._owner.GetEventHandler().ProcessEvent(event)
        else:
            self.GetEventHandler().ProcessEvent(event)

        item.Check(event.GetChecked()) 
        item.SetLabel(event.GetText())
        item.Enable(event.GetEnabled())


    def Clear(self):
        """ Clears the menu items. """

        # since Destroy() call ResizeMenu(), we turn this flag on
        # to avoid resizing the menu for every item removed
        self._resizeMenu = False

        lenItems = len(self._itemsArr)
        for ii in xrange(lenItems):
            self.Destroy(self._itemsArr[0].GetId())

        # Now we can resize the menu
        self._resizeMenu = True
        self.ResizeMenu()


    def FindMenuItemPosSimple(self, item):
        """
        Finds an item and its position inside the menu based on its id.

        :param `item`: an instance of L{FlatMenuItem}.
        """

        if item == None or len(self._itemsArr) == 0:
            return wx.NOT_FOUND

        for i in xrange(len(self._itemsArr)):
            if self._itemsArr[i] == item:
                return i
        
        return wx.NOT_FOUND


    def GetAllItems(self, menu=None, items=[]):
        """
        Internal function to help recurse through all the menu items.

        :param `menu`: the menu from which we start accumulating items;
        :param `items`: the array which is recursively filled with menu items.
        """

        # first copy the current menu items
        newitems = [item for item in items]

        if not menu:
            return newitems
        
        # if any item in this menu has sub-menu, copy them as well
        for i in xrange(len(menu._itemsArr)):
            if menu._itemsArr[i].IsSubMenu():
                newitems = self.GetAllItems(menu._itemsArr[i].GetSubMenu(), newitems)

        return newitems
    
    
    def GetSiblingGroupItem(self, item):
        """
        Used internally.

        :param `item`: an instance of L{FlatMenuItem}.
        """

        pos = self.FindMenuItemPosSimple(item)
        if pos in [wx.NOT_FOUND, 0]:
            return None

        if self._itemsArr[pos-1].IsRadioItem():
            return self._itemsArr[pos-1]
        
        return None


    def ScrollDown(self):
        """ Scrolls the menu down (for very tall menus). """

        # increase the self._from index
        if not self._itemsArr[-1].IsShown():        
            self._first += 1
            self.Refresh()
            
            return True
        
        else:
            if self._downButton:
                self._downButton.GetTimer().Stop()
        
            return False


    def ScrollUp(self):
        """ Scrolls the menu up (for very tall menus). """

        if self._first == 0:
            if self._upButton:
                self._upButton.GetTimer().Stop()
                
            return False
            
        else:
        
            self._first -= 1
            self.Refresh()
            return True

        
    # Not used anymore
    def TryScrollButtons(self, event):
        """ Used internally. """
        
        return False


    def OnTimer(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{FlatMenu}.

        :param `event`: a `wx.TimerEvent` event to be processed.        
        """

        if self._upButton and self._upButton.GetTimerId() == event.GetId():

            self.ScrollUp()

        elif self._downButton and self._downButton.GetTimerId() == event.GetId():

            self.ScrollDown()

        else:
            
            event.Skip()


#--------------------------------------------------------
# Class MenuKbdRedirector
#--------------------------------------------------------

class MenuKbdRedirector(wx.EvtHandler):
    """ A keyboard event handler. """
    
    def __init__(self, menu, oldHandler):
        """
        Default class constructor.

        :param `menu`: an instance of L{FlatMenu} for which we want to redirect
         keyboard inputs;
        :param `oldHandler`: a previous (if any) `wx.EvtHandler` associated with
         the menu.
        """

        self._oldHandler = oldHandler
        self.SetMenu(menu)
        wx.EvtHandler.__init__(self)
        

    def SetMenu(self, menu):
        """
        Sets the listener menu.

        :param `menu`: an instance of L{FlatMenu}.
        """

        self._menu = menu


    def ProcessEvent(self, event):
        """
        Processes the inout event.

        :param `event`: any kind of keyboard-generated events.
        """

        if event.GetEventType() in [wx.EVT_KEY_DOWN, wx.EVT_CHAR, wx.EVT_CHAR_HOOK]:
            return self._menu.OnChar(event.GetKeyCode())
        else:
            return self._oldHandler.ProcessEvent(event)


#--------------------------------------------------------
# Class FocusHandler
#--------------------------------------------------------
 
class FocusHandler(wx.EvtHandler):
    """ A focus event handler. """
    
    def __init__(self, menu):
        """
        Default class constructor.

        :param `menu`: an instance of L{FlatMenu} for which we want to redirect
         focus inputs.
        """

        wx.EvtHandler.__init__(self)
        self.SetMenu(menu)

        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)
        

    def SetMenu(self, menu):
        """
        Sets the listener menu.

        :param `menu`: an instance of L{FlatMenu}.
        """

        self._menu = menu


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{FocusHandler}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        # Let parent process it
        self._menu.OnKeyDown(event)


    def OnKillFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{FocusHandler}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """
        
        wx.PostEvent(self._menu, event)


