"""
auibar contains an implementation of L{AuiToolBar}, which is a completely owner-drawn
toolbar perfectly integrated with the AUI layout system. This allows drag and drop of
toolbars, docking/floating behaviour and the possibility to define "overflow" items
in the toolbar itself.

The default theme that is used is L{AuiDefaultToolBarArt}, which provides a modern,
glossy look and feel. The theme can be changed by calling L{AuiToolBar.SetArtProvider}.
"""

__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx
import types

from aui_utilities import BitmapFromBits, StepColour, GetLabelSize
from aui_utilities import GetBaseColour, MakeDisabledBitmap

import framemanager
from aui_constants import *

# wxPython version string
_VERSION_STRING = wx.VERSION_STRING

# AuiToolBar events
wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN = wx.NewEventType()
wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK = wx.NewEventType()
wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK = wx.NewEventType()
wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK = wx.NewEventType()
wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG = wx.NewEventType()

EVT_AUITOOLBAR_TOOL_DROPDOWN = wx.PyEventBinder(wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN, 1)
""" A dropdown `AuiToolBarItem` is being shown. """
EVT_AUITOOLBAR_OVERFLOW_CLICK = wx.PyEventBinder(wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK, 1)
""" The user left-clicked on the overflow button in `AuiToolBar`. """
EVT_AUITOOLBAR_RIGHT_CLICK = wx.PyEventBinder(wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK, 1)
""" Fires an event when the user right-clicks on a `AuiToolBarItem`. """
EVT_AUITOOLBAR_MIDDLE_CLICK = wx.PyEventBinder(wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK, 1)
""" Fires an event when the user middle-clicks on a `AuiToolBarItem`. """
EVT_AUITOOLBAR_BEGIN_DRAG = wx.PyEventBinder(wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG, 1)
""" A drag operation involving a toolbar item has started. """

# ----------------------------------------------------------------------

class CommandToolBarEvent(wx.PyCommandEvent):
    """ A specialized command event class for events sent by L{AuiToolBar}. """
    
    def __init__(self, command_type, win_id):
        """
        Default class constructor.

        :param `command_type`: the event kind or an instance of `wx.PyCommandEvent`.
        :param `win_id`: the window identification number.
        """
        
        if type(command_type) == types.IntType:    
            wx.PyCommandEvent.__init__(self, command_type, win_id)
        else:
            wx.PyCommandEvent.__init__(self, command_type.GetEventType(), command_type.GetId())
            
        self.is_dropdown_clicked = False
        self.click_pt = wx.Point(-1, -1)
        self.rect = wx.Rect(-1, -1, 0, 0)
        self.tool_id = -1


    def IsDropDownClicked(self):
        """ Returns whether the drop down menu has been clicked. """

        return self.is_dropdown_clicked
    

    def SetDropDownClicked(self, c):
        """
        Sets whether the drop down menu has been clicked.

        :param `c`: ``True`` to set the drop down as clicked, ``False`` otherwise.
        """

        self.is_dropdown_clicked = c    


    def GetClickPoint(self):
        """ Returns the point where the user clicked with the mouse. """

        return self.click_pt

    
    def SetClickPoint(self, p):
        """
        Sets the clicking point.

        :param `p`: a `wx.Point` object.
        """
        
        self.click_pt = p    


    def GetItemRect(self):
        """ Returns the L{AuiToolBarItem} rectangle. """

        return self.rect

    
    def SetItemRect(self, r):
        """
        Sets the L{AuiToolBarItem} rectangle.

        :param `r`: an instance of `wx.Rect`.
        """

        self.rect = r    


    def GetToolId(self):
        """ Returns the L{AuiToolBarItem} identifier. """

        return self.tool_id

    
    def SetToolId(self, id):
        """
        Sets the L{AuiToolBarItem} identifier.

        :param `id`: the toolbar item identifier.
        """

        self.tool_id = id   


# ----------------------------------------------------------------------

class AuiToolBarEvent(CommandToolBarEvent):
    """ A specialized command event class for events sent by L{AuiToolBar}. """
    
    def __init__(self, command_type=None, win_id=0):
        """
        Default class constructor.

        :param `command_type`: the event kind or an instance of `wx.PyCommandEvent`.
        :param `win_id`: the window identification number.
        """

        CommandToolBarEvent.__init__(self, command_type, win_id)

        if type(command_type) == types.IntType:
            self.notify = wx.NotifyEvent(command_type, win_id)
        else:
            self.notify = wx.NotifyEvent(command_type.GetEventType(), command_type.GetId())

        
    def GetNotifyEvent(self):
        """ Returns the actual `wx.NotifyEvent`. """
        
        return self.notify


    def IsAllowed(self):
        """ Returns whether the event is allowed or not. """

        return self.notify.IsAllowed()


    def Veto(self):
        """
        Prevents the change announced by this event from happening.

        It is in general a good idea to notify the user about the reasons for
        vetoing the change because otherwise the applications behaviour (which
        just refuses to do what the user wants) might be quite surprising.
        """

        self.notify.Veto()


    def Allow(self):
        """
        This is the opposite of L{Veto}: it explicitly allows the event to be
        processed. For most events it is not necessary to call this method as the
        events are allowed anyhow but some are forbidden by default (this will
        be mentioned in the corresponding event description).
        """

        self.notify.Allow()


# ----------------------------------------------------------------------

class ToolbarCommandCapture(wx.PyEvtHandler):
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


# ----------------------------------------------------------------------

class AuiToolBarItem(object):
    """
    AuiToolBarItem is a toolbar element.
    
    It has a unique id (except for the separators which always have id = -1), the
    style (telling whether it is a normal button, separator or a control), the
    state (toggled or not, enabled or not) and short and long help strings. The
    default implementations use the short help string for the tooltip text which
    is popped up when the mouse pointer enters the tool and the long help string
    for the applications status bar.
    """

    def __init__(self, item=None):
        """
        Default class constructor.

        :param `item`: another instance of L{AuiToolBarItem}.
        """

        if item:
            self.Assign(item)
            return
        
        self.window = None
        self.clockwisebmp = wx.NullBitmap
        self.counterclockwisebmp = wx.NullBitmap
        self.clockwisedisbmp = wx.NullBitmap
        self.counterclockwisedisbmp = wx.NullBitmap
        self.sizer_item = None
        self.spacer_pixels = 0
        self.id = 0
        self.kind = ITEM_NORMAL
        self.state = 0   # normal, enabled
        self.proportion = 0
        self.active = True
        self.dropdown = True
        self.sticky = True
        self.user_data = 0

        self.label = ""
        self.bitmap = wx.NullBitmap
        self.disabled_bitmap = wx.NullBitmap
        self.hover_bitmap = wx.NullBitmap
        self.short_help = ""
        self.long_help = ""
        self.min_size = wx.Size(-1, -1)
        self.alignment = wx.ALIGN_CENTER
        self.orientation = AUI_TBTOOL_HORIZONTAL
        

    def Assign(self, c):
        """
        Assigns the properties of the L{AuiToolBarItem} `c` to `self`.

        :param `c`: another instance of L{AuiToolBarItem}.
        """

        self.window = c.window
        self.label = c.label
        self.bitmap = c.bitmap
        self.disabled_bitmap = c.disabled_bitmap
        self.hover_bitmap = c.hover_bitmap
        self.short_help = c.short_help
        self.long_help = c.long_help
        self.sizer_item = c.sizer_item
        self.min_size = c.min_size
        self.spacer_pixels = c.spacer_pixels
        self.id = c.id
        self.kind = c.kind
        self.state = c.state
        self.proportion = c.proportion
        self.active = c.active
        self.dropdown = c.dropdown
        self.sticky = c.sticky
        self.user_data = c.user_data
        self.alignment = c.alignment
        self.orientation = c.orientation


    def SetWindow(self, w):
        """
        Assigns a window to the toolbar item.

        :param `w`: an instance of `wx.Window`.
        """

        self.window = w

        
    def GetWindow(self):
        """ Returns window associated to the toolbar item. """

        return self.window        


    def SetId(self, new_id):
        """
        Sets the toolbar item identifier.

        :param `new_id`: the new tool id.
        """

        self.id = new_id

        
    def GetId(self):
        """ Returns the toolbar item identifier. """

        return self.id 


    def SetKind(self, new_kind):
        """
        Sets the L{AuiToolBarItem} kind.

        :param `new_kind`: can be one of the following items:

         ========================  =============================
         Item Kind                 Description
         ========================  =============================
         ``ITEM_CONTROL``          The item in the `AuiToolBar` is a control
         ``ITEM_LABEL``            The item in the `AuiToolBar` is a text label
         ``ITEM_SPACER``           The item in the `AuiToolBar` is a spacer
         ``ITEM_SEPARATOR``        The item in the `AuiToolBar` is a separator
         ``ITEM_CHECK``            The item in the `AuiToolBar` is a toolbar check item
         ``ITEM_NORMAL``           The item in the `AuiToolBar` is a standard toolbar item
         ``ITEM_RADIO``            The item in the `AuiToolBar` is a toolbar radio item
         ========================  =============================
        """

        self.kind = new_kind


    def GetKind(self):
        """ Returns the toolbar item kind. See L{SetKind} for more details. """

        return self.kind
        

    def SetState(self, new_state):
        """
        Sets the toolbar item state.

        :param `new_state`: can be one of the following states:

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
    
        """

        self.state = new_state

        
    def GetState(self):
        """
        Returns the toolbar item state. See L{SetState} for more details.

        :see: L{SetState}
        """
        
        return self.state 


    def SetSizerItem(self, s):
        """
        Associates a sizer item to this toolbar item.

        :param `s`: an instance of `wx.SizerItem`.
        """

        self.sizer_item = s

        
    def GetSizerItem(self):
        """ Returns the associated sizer item. """

        return self.sizer_item 


    def SetLabel(self, s):
        """
        Sets the toolbar item label.

        :param `s`: a string specifying the toolbar item label.
        """

        self.label = s

        
    def GetLabel(self):
        """ Returns the toolbar item label. """

        return self.label 


    def SetBitmap(self, bmp):
        """
        Sets the toolbar item bitmap.

        :param `bmp`: an instance of `wx.Bitmap`.
        """
        
        self.bitmap = bmp

        
    def GetBitmap(self):
        """ Returns the toolbar item bitmap. """

        return self.GetRotatedBitmap(False)


    def SetDisabledBitmap(self, bmp):
        """
        Sets the toolbar item disabled bitmap.

        :param `bmp`: an instance of `wx.Bitmap`.
        """
        
        self.disabled_bitmap = bmp

        
    def GetDisabledBitmap(self):
        """ Returns the toolbar item disabled bitmap. """
        
        return self.GetRotatedBitmap(True)


    def SetHoverBitmap(self, bmp):
        """
        Sets the toolbar item hover bitmap.

        :param `bmp`: an instance of `wx.Bitmap`.
        """
        
        self.hover_bitmap = bmp


    def SetOrientation(self, a):
        """
        Sets the toolbar tool orientation.

        :param `a`: one of ``AUI_TBTOOL_HORIZONTAL``, ``AUI_TBTOOL_VERT_CLOCKWISE`` or
         ``AUI_TBTOOL_VERT_COUNTERCLOCKWISE``.
        """

        self.orientation = a


    def GetOrientation(self):
        """ Returns the toolbar tool orientation. """

        return self.orientation
    
        
    def GetHoverBitmap(self):
        """ Returns the toolbar item hover bitmap. """
        
        return self.hover_bitmap 


    def GetRotatedBitmap(self, disabled):
        """
        Returns the correct bitmap depending on the tool orientation.

        :param `disabled`: whether to return the disabled bitmap or not.
        """
        
        bitmap_to_rotate = (disabled and [self.disabled_bitmap] or [self.bitmap])[0]
        if not bitmap_to_rotate.IsOk() or self.orientation == AUI_TBTOOL_HORIZONTAL:
            return bitmap_to_rotate

        rotated_bitmap = wx.NullBitmap
        clockwise = True
        if self.orientation == AUI_TBTOOL_VERT_CLOCKWISE:
            rotated_bitmap = (disabled and [self.clockwisedisbmp] or [self.clockwisebmp])[0]

        elif self.orientation == AUI_TBTOOL_VERT_COUNTERCLOCKWISE:
            rotated_bitmap = (disabled and [self.counterclockwisedisbmp] or [self.counterclockwisebmp])[0]
            clockwise = False

        if not rotated_bitmap.IsOk():
            rotated_bitmap = wx.BitmapFromImage(bitmap_to_rotate.ConvertToImage().Rotate90(clockwise))

        return rotated_bitmap


    def SetShortHelp(self, s):
        """
        Sets the short help string for the L{AuiToolBarItem}, to be displayed in a
        `wx.ToolTip` when the mouse hover over the toolbar item.

        :param `s`: the tool short help string.
        """

        self.short_help = s

        
    def GetShortHelp(self):
        """ Returns the short help string for the L{AuiToolBarItem}. """

        return self.short_help 


    def SetLongHelp(self, s):
        """
        Sets the long help string for the toolbar item. This string is shown in the
        statusbar (if any) of the parent frame when the mouse pointer is inside the
        tool.

        :param `s`: the tool long help string.
        """

        self.long_help = s

        
    def GetLongHelp(self):
        """ Returns the long help string for the L{AuiToolBarItem}. """

        return self.long_help 


    def SetMinSize(self, s):
        """
        Sets the toolbar item minimum size.

        :param `s`: an instance of `wx.Size`.
        """

        self.min_size = wx.Size(*s)

        
    def GetMinSize(self):
        """ Returns the toolbar item minimum size. """

        return self.min_size 


    def SetSpacerPixels(self, s):
        """
        Sets the number of pixels for a toolbar item with kind = ``ITEM_SEPARATOR``.

        :param `s`: number of pixels.
        """

        self.spacer_pixels = s

        
    def GetSpacerPixels(self):
        """ Returns the number of pixels for a toolbar item with kind = ``ITEM_SEPARATOR``. """

        return self.spacer_pixels 


    def SetProportion(self, p):
        """
        Sets the L{AuiToolBarItem} proportion in the toolbar.

        :param `p`: the item proportion.
        """

        self.proportion = p

        
    def GetProportion(self):
        """ Returns the L{AuiToolBarItem} proportion in the toolbar. """

        return self.proportion 


    def SetActive(self, b):
        """
        Activates/deactivates the toolbar item.

        :param `b`: ``True`` to activate the item, ``False`` to deactivate it.
        """

        self.active = b

        
    def IsActive(self):
        """ Returns whether the toolbar item is active or not. """

        return self.active
    

    def SetHasDropDown(self, b):
        """
        Sets whether the toolbar item has an associated dropdown menu.

        :param `b`: ``True`` to set a dropdown menu, ``False`` otherwise.
        """

        self.dropdown = b

        
    def HasDropDown(self):
        """ Returns whether the toolbar item has an associated dropdown menu or not. """

        return self.dropdown 


    def SetSticky(self, b):
        """
        Sets whether the toolbar item is sticky (permanent highlight after mouse enter)
        or not.

        :param `b`: ``True`` to set the item as sticky, ``False`` otherwise.
        """

        self.sticky = b

        
    def IsSticky(self):
        """ Returns whether the toolbar item has a sticky behaviour or not. """

        return self.sticky 


    def SetUserData(self, l):
        """
        Associates some kind of user data to the toolbar item.
        
        :param `l`: a Python object.

        :note: The user data can be any Python object.
        """

        self.user_data = l

        
    def GetUserData(self):
        """ Returns the associated user data. """

        return self.user_data
    

    def SetAlignment(self, l):
        """
        Sets the toolbar item alignment.

        :param `l`: the item alignment, which can be one of the available `wx.Sizer`
         alignments.
        """

        self.alignment = l

        
    def GetAlignment(self):
        """ Returns the toolbar item alignment. """

        return self.alignment        


# ----------------------------------------------------------------------

class AuiDefaultToolBarArt(object):
    """
    Toolbar art provider code - a tab provider provides all drawing functionality to
    the L{AuiToolBar}. This allows the L{AuiToolBar} to have a plugable look-and-feel.

    By default, a L{AuiToolBar} uses an instance of this class called L{AuiDefaultToolBarArt}
    which provides bitmap art and a colour scheme that is adapted to the major platforms'
    look. You can either derive from that class to alter its behaviour or write a
    completely new tab art class. Call L{AuiToolBar.SetArtProvider} to make use this
    new tab art.
    """

    def __init__(self):
        """ Default class constructor. """
        
        self._base_colour = GetBaseColour()

        self._agwFlags = 0
        self._text_orientation = AUI_TBTOOL_TEXT_BOTTOM
        self._highlight_colour = wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT)

        self._separator_size = 7
        self._orientation = AUI_TBTOOL_HORIZONTAL
        self._gripper_size = 7
        self._overflow_size = 16

        darker1_colour = StepColour(self._base_colour, 85)
        darker2_colour = StepColour(self._base_colour, 75)
        darker3_colour = StepColour(self._base_colour, 60)
        darker4_colour = StepColour(self._base_colour, 50)
        darker5_colour = StepColour(self._base_colour, 40)

        self._gripper_pen1 = wx.Pen(darker5_colour)
        self._gripper_pen2 = wx.Pen(darker3_colour)
        self._gripper_pen3 = wx.WHITE_PEN

        button_dropdown_bits = "\xe0\xf1\xfb"
        overflow_bits = "\x80\xff\x80\xc1\xe3\xf7"

        self._button_dropdown_bmp = BitmapFromBits(button_dropdown_bits, 5, 3, wx.BLACK)
        self._disabled_button_dropdown_bmp = BitmapFromBits(button_dropdown_bits, 5, 3,
                                                            wx.Colour(128, 128, 128))
        self._overflow_bmp = BitmapFromBits(overflow_bits, 7, 6, wx.BLACK)
        self._disabled_overflow_bmp = BitmapFromBits(overflow_bits, 7, 6, wx.Colour(128, 128, 128))

        self._font = wx.SystemSettings.GetFont(wx.SYS_DEFAULT_GUI_FONT)


    def Clone(self):
        """ Clones the L{AuiToolBar} art. """

        return AuiDefaultToolBarArt()


    def SetAGWFlags(self, agwFlags):
        """
        Sets the toolbar art flags.

        :param `agwFlags`: a combination of the following values:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_TB_TEXT``                      Shows the text in the toolbar buttons; by default only icons are shown
         ``AUI_TB_NO_TOOLTIPS``               Don't show tooltips on `AuiToolBar` items
         ``AUI_TB_NO_AUTORESIZE``             Do not auto-resize the `AuiToolBar`
         ``AUI_TB_GRIPPER``                   Shows a gripper on the `AuiToolBar`
         ``AUI_TB_OVERFLOW``                  The `AuiToolBar` can contain overflow items
         ``AUI_TB_VERTICAL``                  The `AuiToolBar` is vertical
         ``AUI_TB_HORZ_LAYOUT``               Shows the text and the icons alongside, not vertically stacked. This style must be used with ``AUI_TB_TEXT``
         ``AUI_TB_PLAIN_BACKGROUND``          Don't draw a gradient background on the toolbar
         ``AUI_TB_HORZ_TEXT``                 Combination of ``AUI_TB_HORZ_LAYOUT`` and ``AUI_TB_TEXT``
         ==================================== ==================================
        
        """
        
        self._agwFlags = agwFlags


    def GetAGWFlags(self):
        """
        Returns the L{AuiDefaultToolBarArt} flags. See L{SetAGWFlags} for more
        details.

        :see: L{SetAGWFlags}
        """

        return self._agwFlags


    def SetFont(self, font):
        """
        Sets the L{AuiDefaultToolBarArt} font.

        :param `font`: a `wx.Font` object.
        """

        self._font = font


    def SetTextOrientation(self, orientation):
        """
        Sets the text orientation.

        :param `orientation`: can be one of the following constants:

         ==================================== ==================================
         Orientation Switches                 Description
         ==================================== ==================================
         ``AUI_TBTOOL_TEXT_LEFT``             Text in `AuiToolBar` items is aligned left
         ``AUI_TBTOOL_TEXT_RIGHT``            Text in `AuiToolBar` items is aligned right
         ``AUI_TBTOOL_TEXT_TOP``              Text in `AuiToolBar` items is aligned top
         ``AUI_TBTOOL_TEXT_BOTTOM``           Text in `AuiToolBar` items is aligned bottom
         ==================================== ==================================
        
        """

        self._text_orientation = orientation


    def GetFont(self):
        """ Returns the L{AuiDefaultToolBarArt} font. """

        return self._font


    def GetTextOrientation(self):
        """
        Returns the L{AuiDefaultToolBarArt} text orientation. See
        L{SetTextOrientation} for more details.

        :see: L{SetTextOrientation}
        """

        return self._text_orientation


    def SetOrientation(self, orientation):
        """
        Sets the toolbar tool orientation.

        :param `orientation`: one of ``AUI_TBTOOL_HORIZONTAL``, ``AUI_TBTOOL_VERT_CLOCKWISE`` or
         ``AUI_TBTOOL_VERT_COUNTERCLOCKWISE``.
        """

        self._orientation = orientation


    def GetOrientation(self):
        """ Returns the toolbar orientation. """

        return self._orientation        


    def DrawBackground(self, dc, wnd, _rect, horizontal=True):
        """
        Draws a toolbar background with a gradient shading.

        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `_rect`: the L{AuiToolBar} rectangle;
        :param `horizontal`: ``True`` if the toolbar is horizontal, ``False`` if it is vertical.
        """

        rect = wx.Rect(*_rect)

        start_colour = StepColour(self._base_colour, 180)
        end_colour = StepColour(self._base_colour, 85)
        reflex_colour = StepColour(self._base_colour, 95)
        
        dc.GradientFillLinear(rect, start_colour, end_colour,
                              (horizontal and [wx.SOUTH] or [wx.EAST])[0])

        left = rect.GetLeft()
        right = rect.GetRight()
        top = rect.GetTop()
        bottom = rect.GetBottom()

        dc.SetPen(wx.Pen(reflex_colour))
        if horizontal:
            dc.DrawLine(left, bottom, right+1, bottom)
        else:
            dc.DrawLine(right, top, right, bottom+1)
            

    def DrawPlainBackground(self, dc, wnd, _rect):
        """
        Draws a toolbar background with a plain colour.

        This method contrasts with the default behaviour of the L{AuiToolBar} that
        draws a background gradient and this break the window design when putting
        it within a control that has margin between the borders and the toolbar
        (example: put L{AuiToolBar} within a `wx.StaticBoxSizer` that has a plain background).
      
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `_rect`: the L{AuiToolBar} rectangle.
        """
        
        rect = wx.Rect(*_rect)
        rect.height += 1

        dc.SetBrush(wx.Brush(wx.SystemSettings.GetColour(wx.SYS_COLOUR_3DFACE)))
        dc.DrawRectangle(rect.x - 1, rect.y - 1, rect.width + 2, rect.height + 1)


    def DrawLabel(self, dc, wnd, item, rect):
        """
        Draws a toolbar item label.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `item`: an instance of L{AuiToolBarItem};
        :param `rect`: the L{AuiToolBarItem} rectangle.
        """
        
        dc.SetFont(self._font)
        dc.SetTextForeground(wx.BLACK)
        orient = item.GetOrientation()

        horizontal = orient == AUI_TBTOOL_HORIZONTAL
        # we only care about the text height here since the text
        # will get cropped based on the width of the item
        label_size = GetLabelSize(dc, item.GetLabel(), not horizontal)
        text_width = label_size.GetWidth()
        text_height = label_size.GetHeight()

        if orient == AUI_TBTOOL_HORIZONTAL:
            text_x = rect.x
            text_y = rect.y + (rect.height-text_height)/2
            dc.DrawText(item.GetLabel(), text_x, text_y)

        elif orient == AUI_TBTOOL_VERT_CLOCKWISE:
            text_x = rect.x + (rect.width+text_width)/2
            text_y = rect.y
            dc.DrawRotatedText(item.GetLabel(), text_x, text_y, 270)

        elif AUI_TBTOOL_VERT_COUNTERCLOCKWISE:
            text_x = rect.x + (rect.width-text_width)/2
            text_y = rect.y + text_height
            dc.DrawRotatedText(item.GetLabel(), text_x, text_y, 90)


    def DrawButton(self, dc, wnd, item, rect):
        """
        Draws a toolbar item button.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `item`: an instance of L{AuiToolBarItem};
        :param `rect`: the L{AuiToolBarItem} rectangle.
        """

        bmp_rect, text_rect = self.GetToolsPosition(dc, item, rect)
        
        if not item.GetState() & AUI_BUTTON_STATE_DISABLED:
        
            if item.GetState() & AUI_BUTTON_STATE_PRESSED:
            
                dc.SetPen(wx.Pen(self._highlight_colour))
                dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 150)))
                dc.DrawRectangleRect(rect)
            
            elif item.GetState() & AUI_BUTTON_STATE_HOVER or item.IsSticky():
            
                dc.SetPen(wx.Pen(self._highlight_colour))
                dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 170)))

                # draw an even lighter background for checked item hovers (since
                # the hover background is the same colour as the check background)
                if item.GetState() & AUI_BUTTON_STATE_CHECKED:
                    dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 180)))

                dc.DrawRectangleRect(rect)
            
            elif item.GetState() & AUI_BUTTON_STATE_CHECKED:
            
                # it's important to put this code in an else statment after the
                # hover, otherwise hovers won't draw properly for checked items
                dc.SetPen(wx.Pen(self._highlight_colour))
                dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 170)))
                dc.DrawRectangleRect(rect)
            
        if item.GetState() & AUI_BUTTON_STATE_DISABLED:
            bmp = item.GetDisabledBitmap()
        else:
            bmp = item.GetBitmap()

        if bmp.IsOk():
            dc.DrawBitmap(bmp, bmp_rect.x, bmp_rect.y, True)

        # set the item's text colour based on if it is disabled
        dc.SetTextForeground(wx.BLACK)
        if item.GetState() & AUI_BUTTON_STATE_DISABLED:
            dc.SetTextForeground(DISABLED_TEXT_COLOUR)

        if self._agwFlags & AUI_TB_TEXT and item.GetLabel() != "":
            self.DrawLabel(dc, wnd, item, text_rect)
        

    def DrawDropDownButton(self, dc, wnd, item, rect):
        """
        Draws a toolbar dropdown button.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `item`: an instance of L{AuiToolBarItem};
        :param `rect`: the L{AuiToolBarItem} rectangle.
        """
        
        dropbmp_x = dropbmp_y = 0

        button_rect = wx.Rect(rect.x, rect.y, rect.width-BUTTON_DROPDOWN_WIDTH, rect.height)
        dropdown_rect = wx.Rect(rect.x+rect.width-BUTTON_DROPDOWN_WIDTH-1, rect.y, BUTTON_DROPDOWN_WIDTH+1, rect.height)

        horizontal = item.GetOrientation() == AUI_TBTOOL_HORIZONTAL
        
        if horizontal:
            button_rect = wx.Rect(rect.x, rect.y, rect.width-BUTTON_DROPDOWN_WIDTH, rect.height)
            dropdown_rect = wx.Rect(rect.x+rect.width-BUTTON_DROPDOWN_WIDTH-1, rect.y, BUTTON_DROPDOWN_WIDTH+1, rect.height)
        else:
            button_rect = wx.Rect(rect.x, rect.y, rect.width, rect.height-BUTTON_DROPDOWN_WIDTH)
            dropdown_rect = wx.Rect(rect.x, rect.y+rect.height-BUTTON_DROPDOWN_WIDTH-1, rect.width, BUTTON_DROPDOWN_WIDTH+1)

        dropbmp_width = self._button_dropdown_bmp.GetWidth()
        dropbmp_height = self._button_dropdown_bmp.GetHeight()
        if not horizontal:
            tmp = dropbmp_width
            dropbmp_width = dropbmp_height
            dropbmp_height = tmp

        dropbmp_x = dropdown_rect.x + (dropdown_rect.width/2) - dropbmp_width/2
        dropbmp_y = dropdown_rect.y + (dropdown_rect.height/2) - dropbmp_height/2

        bmp_rect, text_rect = self.GetToolsPosition(dc, item, button_rect)
        
        if item.GetState() & AUI_BUTTON_STATE_PRESSED:
        
            dc.SetPen(wx.Pen(self._highlight_colour))
            dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 140)))
            dc.DrawRectangleRect(button_rect)
            dc.DrawRectangleRect(dropdown_rect)
        
        elif item.GetState() & AUI_BUTTON_STATE_HOVER or item.IsSticky():
        
            dc.SetPen(wx.Pen(self._highlight_colour))
            dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 170)))
            dc.DrawRectangleRect(button_rect)
            dc.DrawRectangleRect(dropdown_rect)

        elif item.GetState() & AUI_BUTTON_STATE_CHECKED:
            # it's important to put this code in an else statment after the 
            # hover, otherwise hovers won't draw properly for checked items 
            dc.SetPen(wx.Pen(self._highlight_colour))
            dc.SetBrush(wx.Brush(StepColour(self._highlight_colour, 170)))
            dc.DrawRectangle(button_rect)
            dc.DrawRectangle(dropdown_rect)
            
        if item.GetState() & AUI_BUTTON_STATE_DISABLED:
        
            bmp = item.GetDisabledBitmap()
            dropbmp = self._disabled_button_dropdown_bmp
        
        else:
        
            bmp = item.GetBitmap()
            dropbmp = self._button_dropdown_bmp
        
        if not bmp.IsOk():
            return

        dc.DrawBitmap(bmp, bmp_rect.x, bmp_rect.y, True)
        if horizontal:
            dc.DrawBitmap(dropbmp, dropbmp_x, dropbmp_y, True)
        else:
            dc.DrawBitmap(wx.BitmapFromImage(dropbmp.ConvertToImage().Rotate90(item.GetOrientation() == AUI_TBTOOL_VERT_CLOCKWISE)),
                          dropbmp_x, dropbmp_y, True)
            
        # set the item's text colour based on if it is disabled
        dc.SetTextForeground(wx.BLACK)
        if item.GetState() & AUI_BUTTON_STATE_DISABLED:
            dc.SetTextForeground(DISABLED_TEXT_COLOUR)

        if self._agwFlags & AUI_TB_TEXT and item.GetLabel() != "":  
            self.DrawLabel(dc, wnd, item, text_rect)
        

    def DrawControlLabel(self, dc, wnd, item, rect):
        """
        Draws a label for a toolbar control.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `item`: an instance of L{AuiToolBarItem};
        :param `rect`: the L{AuiToolBarItem} rectangle.
        """

        label_size = GetLabelSize(dc, item.GetLabel(), item.GetOrientation() != AUI_TBTOOL_HORIZONTAL)
        text_height = label_size.GetHeight()
        text_width = label_size.GetWidth()

        dc.SetFont(self._font)

        if self._agwFlags & AUI_TB_TEXT:
        
            tx, text_height = dc.GetTextExtent("ABCDHgj")        

        text_width, ty = dc.GetTextExtent(item.GetLabel())

        # don't draw the label if it is wider than the item width
        if text_width > rect.width:
            return

        # set the label's text colour
        dc.SetTextForeground(wx.BLACK)

        text_x = rect.x + (rect.width/2) - (text_width/2) + 1
        text_y = rect.y + rect.height - text_height - 1

        if self._agwFlags & AUI_TB_TEXT and item.GetLabel() != "": 
            dc.DrawText(item.GetLabel(), text_x, text_y)
    

    def GetLabelSize(self, dc, wnd, item):
        """
        Returns the label size for a toolbar item.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `item`: an instance of L{AuiToolBarItem}.
        """

        dc.SetFont(self._font)
        label_size = GetLabelSize(dc, item.GetLabel(), self._orientation != AUI_TBTOOL_HORIZONTAL)

        return wx.Size(item.GetMinSize().GetWidth(), label_size.GetHeight())


    def GetToolSize(self, dc, wnd, item):
        """
        Returns the toolbar item size.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `item`: an instance of L{AuiToolBarItem}.
        """
        
        if not item.GetBitmap().IsOk() and not self._agwFlags & AUI_TB_TEXT:
            return wx.Size(16, 16)

        width = item.GetBitmap().GetWidth()
        height = item.GetBitmap().GetHeight()

        if self._agwFlags & AUI_TB_TEXT:
        
            dc.SetFont(self._font)
            label_size = GetLabelSize(dc, item.GetLabel(), self.GetOrientation() != AUI_TBTOOL_HORIZONTAL)
            padding = 6
            
            if self._text_orientation == AUI_TBTOOL_TEXT_BOTTOM:
            
                if self.GetOrientation() != AUI_TBTOOL_HORIZONTAL:
                    height += 3   # space between top border and bitmap
                    height += 3   # space between bitmap and text
                    padding = 0

                height += label_size.GetHeight()
            
                if item.GetLabel() != "":
                    width = max(width, label_size.GetWidth()+padding)
                
            elif self._text_orientation == AUI_TBTOOL_TEXT_RIGHT and item.GetLabel() != "":
            
                if self.GetOrientation() == AUI_TBTOOL_HORIZONTAL:
                    
                    width += 3  # space between left border and bitmap
                    width += 3  # space between bitmap and text
                    padding = 0

                width += label_size.GetWidth()
                height = max(height, label_size.GetHeight()+padding)
                
        # if the tool has a dropdown button, add it to the width
        if item.HasDropDown():
            if item.GetOrientation() == AUI_TBTOOL_HORIZONTAL:
                width += BUTTON_DROPDOWN_WIDTH+4
            else:
                height += BUTTON_DROPDOWN_WIDTH+4

        return wx.Size(width, height)


    def DrawSeparator(self, dc, wnd, _rect):
        """
        Draws a toolbar separator.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `_rect`: the L{AuiToolBarItem} rectangle.
        """
        
        horizontal = True
        if self._agwFlags & AUI_TB_VERTICAL:
            horizontal = False

        rect = wx.Rect(*_rect)

        if horizontal:
        
            rect.x += (rect.width/2)
            rect.width = 1
            new_height = (rect.height*3)/4
            rect.y += (rect.height/2) - (new_height/2)
            rect.height = new_height
        
        else:
        
            rect.y += (rect.height/2)
            rect.height = 1
            new_width = (rect.width*3)/4
            rect.x += (rect.width/2) - (new_width/2)
            rect.width = new_width
        
        start_colour = StepColour(self._base_colour, 80)
        end_colour = StepColour(self._base_colour, 80)
        dc.GradientFillLinear(rect, start_colour, end_colour, (horizontal and [wx.SOUTH] or [wx.EAST])[0])


    def DrawGripper(self, dc, wnd, rect):
        """
        Draws the toolbar gripper.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `rect`: the L{AuiToolBar} rectangle.
        """
        
        i = 0
        while 1:
        
            if self._agwFlags & AUI_TB_VERTICAL:
            
                x = rect.x + (i*4) + 4
                y = rect.y + 3
                if x > rect.GetWidth() - 4:
                    break
            
            else:
            
                x = rect.x + 3
                y = rect.y + (i*4) + 4
                if y > rect.GetHeight() - 4:
                    break
            
            dc.SetPen(self._gripper_pen1)
            dc.DrawPoint(x, y)
            dc.SetPen(self._gripper_pen2)
            dc.DrawPoint(x, y+1)
            dc.DrawPoint(x+1, y)
            dc.SetPen(self._gripper_pen3)
            dc.DrawPoint(x+2, y+1)
            dc.DrawPoint(x+2, y+2)
            dc.DrawPoint(x+1, y+2)

            i += 1


    def DrawOverflowButton(self, dc, wnd, rect, state):
        """
        Draws the overflow button for the L{AuiToolBar}.
        
        :param `dc`: a `wx.DC` device context;
        :param `wnd`: a `wx.Window` derived window;
        :param `rect`: the L{AuiToolBar} rectangle;
        :param `state`: the overflow button state.
        """
        
        if state & AUI_BUTTON_STATE_HOVER or  state & AUI_BUTTON_STATE_PRESSED:
        
            cli_rect = wnd.GetClientRect()
            light_gray_bg = StepColour(self._highlight_colour, 170)

            if self._agwFlags & AUI_TB_VERTICAL:
            
                dc.SetPen(wx.Pen(self._highlight_colour))
                dc.DrawLine(rect.x, rect.y, rect.x+rect.width, rect.y)
                dc.SetPen(wx.Pen(light_gray_bg))
                dc.SetBrush(wx.Brush(light_gray_bg))
                dc.DrawRectangle(rect.x, rect.y+1, rect.width, rect.height)
            
            else:
            
                dc.SetPen(wx.Pen(self._highlight_colour))
                dc.DrawLine(rect.x, rect.y, rect.x, rect.y+rect.height)
                dc.SetPen(wx.Pen(light_gray_bg))
                dc.SetBrush(wx.Brush(light_gray_bg))
                dc.DrawRectangle(rect.x+1, rect.y, rect.width, rect.height)
            
        x = rect.x + 1 + (rect.width-self._overflow_bmp.GetWidth())/2
        y = rect.y + 1 + (rect.height-self._overflow_bmp.GetHeight())/2
        dc.DrawBitmap(self._overflow_bmp, x, y, True)


    def GetElementSize(self, element_id):
        """
        Returns the size of a UI element in the L{AuiToolBar}.

        :param `element_id`: can be one of the following:

         ==================================== ==================================
         Element Identifier                   Description
         ==================================== ==================================
         ``AUI_TBART_SEPARATOR_SIZE``         Separator size in `AuiToolBar`
         ``AUI_TBART_GRIPPER_SIZE``           Gripper size in `AuiToolBar`
         ``AUI_TBART_OVERFLOW_SIZE``          Overflow button size in `AuiToolBar`
         ==================================== ==================================        
        """
        
        if element_id == AUI_TBART_SEPARATOR_SIZE:
            return self._separator_size
        elif element_id == AUI_TBART_GRIPPER_SIZE:
            return self._gripper_size
        elif element_id == AUI_TBART_OVERFLOW_SIZE:
            return self._overflow_size

        return 0


    def SetElementSize(self, element_id, size):
        """
        Sets the size of a UI element in the L{AuiToolBar}.

        :param `element_id`: can be one of the following:

         ==================================== ==================================
         Element Identifier                   Description
         ==================================== ==================================
         ``AUI_TBART_SEPARATOR_SIZE``         Separator size in `AuiToolBar`
         ``AUI_TBART_GRIPPER_SIZE``           Gripper size in `AuiToolBar`
         ``AUI_TBART_OVERFLOW_SIZE``          Overflow button size in `AuiToolBar`
         ==================================== ==================================

        :param `size`: the new size of the UI element.        
        """
        
        if element_id == AUI_TBART_SEPARATOR_SIZE:
            self._separator_size = size
        elif element_id == AUI_TBART_GRIPPER_SIZE:
            self._gripper_size = size
        elif element_id == AUI_TBART_OVERFLOW_SIZE:
            self._overflow_size = size


    def ShowDropDown(self, wnd, items):
        """
        Shows the drop down window menu for overflow items.

        :param `wnd`: an instance of `wx.Window`;
        :param `items`: the overflow toolbar items (a Python list).
        """

        menuPopup = wx.Menu()
        items_added = 0

        for item in items:

            if item.GetKind() not in [ITEM_SEPARATOR, ITEM_SPACER, ITEM_CONTROL]:
            
                text = item.GetShortHelp()
                if text == "":
                    text = item.GetLabel()
                if text == "":
                    text = " "

                kind = item.GetKind()
                m = wx.MenuItem(menuPopup, item.GetId(), text, item.GetShortHelp(), kind)
                orientation = item.GetOrientation()
                item.SetOrientation(AUI_TBTOOL_HORIZONTAL)
                
                if kind not in [ITEM_CHECK, ITEM_RADIO]:
                    m.SetBitmap(item.GetBitmap())

                item.SetOrientation(orientation)                    
                    
                menuPopup.AppendItem(m)
                if kind in [ITEM_CHECK, ITEM_RADIO]:            
                    state = (item.state & AUI_BUTTON_STATE_CHECKED and [True] or [False])[0]
                    m.Check(state)

                items_added += 1
            
            else:
            
                if items_added > 0 and item.GetKind() == ITEM_SEPARATOR:
                    menuPopup.AppendSeparator()
            
        # find out where to put the popup menu of window items
        pt = wx.GetMousePosition()
        pt = wnd.ScreenToClient(pt)

        # find out the screen coordinate at the bottom of the tab ctrl
        cli_rect = wnd.GetClientRect()
        pt.y = cli_rect.y + cli_rect.height

        cc = ToolbarCommandCapture()
        wnd.PushEventHandler(cc)

        # Adjustments to get slightly better menu placement
        if wx.Platform == "__WXMAC__":
            pt.y += 5
            pt.x -= 5

        wnd.PopupMenu(menuPopup, pt)
        command = cc.GetCommandId()
        wnd.PopEventHandler(True)

        return command


    def GetToolsPosition(self, dc, item, rect):
        """
        Returns the bitmap and text rectangles for a toolbar item.
        
        :param `dc`: a `wx.DC` device context;
        :param `item`: an instance of L{AuiToolBarItem};
        :param `rect`: the tool rect.
        """
        
        text_width = text_height = 0
        horizontal = self._orientation == AUI_TBTOOL_HORIZONTAL
        text_bottom = self._text_orientation == AUI_TBTOOL_TEXT_BOTTOM
        text_right = self._text_orientation == AUI_TBTOOL_TEXT_RIGHT
        bmp_width = item.GetBitmap().GetWidth()
        bmp_height = item.GetBitmap().GetHeight()
     
        if self._agwFlags & AUI_TB_TEXT:        
            dc.SetFont(self._font)
            label_size = GetLabelSize(dc, item.GetLabel(), not horizontal)
            text_height = label_size.GetHeight()
            text_width = label_size.GetWidth()
        
        bmp_x = bmp_y = text_x = text_y = 0

        if horizontal and text_bottom:
            bmp_x = rect.x + (rect.width/2) - (bmp_width/2)
            bmp_y = rect.y + 3
            text_x = rect.x + (rect.width/2) - (text_width/2)
            text_y = rect.y + ((bmp_y - rect.y) * 2) + bmp_height
        
        elif horizontal and text_right:
            bmp_x = rect.x + 3
            bmp_y = rect.y + (rect.height/2) - (bmp_height / 2)
            text_x = rect.x + ((bmp_x - rect.x) * 2) + bmp_width
            text_y = rect.y + (rect.height/2) - (text_height/2)
        
        elif not horizontal and text_bottom:
            bmp_x = rect.x + (rect.width / 2) - (bmp_width / 2)
            bmp_y = rect.y + 3
            text_x = rect.x + (rect.width / 2) - (text_width / 2)
            text_y = rect.y + ((bmp_y - rect.y) * 2) + bmp_height
        
        bmp_rect = wx.Rect(bmp_x, bmp_y, bmp_width, bmp_height)
        text_rect = wx.Rect(text_x, text_y, text_width, text_height)

        return bmp_rect, text_rect

    
class AuiToolBar(wx.PyControl):
    """
    AuiToolBar is a completely owner-drawn toolbar perfectly integrated with the
    AUI layout system. This allows drag and drop of toolbars, docking/floating
    behaviour and the possibility to define "overflow" items in the toolbar itself.

    The default theme that is used is L{AuiDefaultToolBarArt}, which provides a modern,
    glossy look and feel. The theme can be changed by calling L{AuiToolBar.SetArtProvider}.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, agwStyle=AUI_TB_DEFAULT_STYLE):
        """
        Default class constructor.

        :param `parent`: the L{AuiToolBar} parent;
        :param `id`: an identifier for the control: a value of -1 is taken to mean a default;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the control window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:
        
         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_TB_TEXT``                      Shows the text in the toolbar buttons; by default only icons are shown
         ``AUI_TB_NO_TOOLTIPS``               Don't show tooltips on `AuiToolBar` items
         ``AUI_TB_NO_AUTORESIZE``             Do not auto-resize the `AuiToolBar`
         ``AUI_TB_GRIPPER``                   Shows a gripper on the `AuiToolBar`
         ``AUI_TB_OVERFLOW``                  The `AuiToolBar` can contain overflow items
         ``AUI_TB_VERTICAL``                  The `AuiToolBar` is vertical
         ``AUI_TB_HORZ_LAYOUT``               Shows the text and the icons alongside, not vertically stacked. This style must be used with ``AUI_TB_TEXT``
         ``AUI_TB_PLAIN_BACKGROUND``          Don't draw a gradient background on the toolbar
         ``AUI_TB_HORZ_TEXT``                 Combination of ``AUI_TB_HORZ_LAYOUT`` and ``AUI_TB_TEXT``
         ==================================== ==================================

         The default value for `agwStyle` is: ``AUI_TB_DEFAULT_STYLE`` = 0

        """
        
        wx.PyControl.__init__(self, parent, id, pos, size, style|wx.BORDER_NONE)

        self._sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.SetSizer(self._sizer)
        self._button_width = -1
        self._button_height = -1
        self._sizer_element_count = 0
        self._action_pos = wx.Point(-1, -1)
        self._action_item = None
        self._tip_item = None
        self._art = AuiDefaultToolBarArt()
        self._tool_packing = 2
        self._tool_border_padding = 3
        self._tool_text_orientation = AUI_TBTOOL_TEXT_BOTTOM
        self._tool_orientation = AUI_TBTOOL_HORIZONTAL
        self._tool_alignment = wx.EXPAND
        self._gripper_sizer_item = None
        self._overflow_sizer_item = None
        self._dragging = False

        self._agwStyle = self._originalStyle = agwStyle

        self._gripper_visible = (self._agwStyle & AUI_TB_GRIPPER and [True] or [False])[0]
        self._overflow_visible = (self._agwStyle & AUI_TB_OVERFLOW and [True] or [False])[0]
        self._overflow_state = 0
        self._custom_overflow_prepend = []
        self._custom_overflow_append = []

        self._items = []
        
        self.SetMargins(5, 5, 2, 2)
        self.SetFont(wx.NORMAL_FONT)
        self._art.SetAGWFlags(self._agwStyle)
        self.SetExtraStyle(wx.WS_EX_PROCESS_IDLE)
        
        if agwStyle & AUI_TB_HORZ_LAYOUT:
            self.SetToolTextOrientation(AUI_TBTOOL_TEXT_RIGHT)
        elif agwStyle & AUI_TB_VERTICAL:
            if agwStyle & AUI_TB_CLOCKWISE:
                self.SetToolOrientation(AUI_TBTOOL_VERT_CLOCKWISE)
            elif agwStyle & AUI_TB_COUNTERCLOCKWISE:
                self.SetToolOrientation(AUI_TBTOOL_VERT_COUNTERCLOCKWISE)
 
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_IDLE, self.OnIdle)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        self.Bind(wx.EVT_RIGHT_DCLICK, self.OnRightDown)
        self.Bind(wx.EVT_RIGHT_UP, self.OnRightUp)
        self.Bind(wx.EVT_MIDDLE_DOWN, self.OnMiddleDown)
        self.Bind(wx.EVT_MIDDLE_DCLICK, self.OnMiddleDown)
        self.Bind(wx.EVT_MIDDLE_UP, self.OnMiddleUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)
        self.Bind(wx.EVT_SET_CURSOR, self.OnSetCursor)


    def SetWindowStyleFlag(self, style):
        """
        Sets the style of the window.
        
        :param `style`: the new window style. 

        :note: Please note that some styles cannot be changed after the window
         creation and that `Refresh` might need to be be called after changing the
         others for the change to take place immediately.

        :note: Overridden from `wx.PyControl`.
        """

        wx.PyControl.SetWindowStyleFlag(self, style|wx.BORDER_NONE)
        

    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the AGW-specific style of the window.
        
        :param `agwStyle`: the new window style. This can be a combination of the
         following bits:
        
         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_TB_TEXT``                      Shows the text in the toolbar buttons; by default only icons are shown
         ``AUI_TB_NO_TOOLTIPS``               Don't show tooltips on `AuiToolBar` items
         ``AUI_TB_NO_AUTORESIZE``             Do not auto-resize the `AuiToolBar`
         ``AUI_TB_GRIPPER``                   Shows a gripper on the `AuiToolBar`
         ``AUI_TB_OVERFLOW``                  The `AuiToolBar` can contain overflow items
         ``AUI_TB_VERTICAL``                  The `AuiToolBar` is vertical
         ``AUI_TB_HORZ_LAYOUT``               Shows the text and the icons alongside, not vertically stacked. This style must be used with ``AUI_TB_TEXT``
         ``AUI_TB_PLAIN_BACKGROUND``          Don't draw a gradient background on the toolbar
         ``AUI_TB_HORZ_TEXT``                 Combination of ``AUI_TB_HORZ_LAYOUT`` and ``AUI_TB_TEXT``
         ==================================== ==================================

        :note: Please note that some styles cannot be changed after the window
         creation and that `Refresh` might need to be be called after changing the
         others for the change to take place immediately.
        """
        
        self._agwStyle = self._originalStyle = agwStyle

        if self._art:
            self._art.SetAGWFlags(self._agwStyle)
        
        if agwStyle & AUI_TB_GRIPPER:
            self._gripper_visible = True
        else:
            self._gripper_visible = False

        if agwStyle & AUI_TB_OVERFLOW:
            self._overflow_visible = True
        else:
            self._overflow_visible = False

        if agwStyle & AUI_TB_HORZ_LAYOUT:
            self.SetToolTextOrientation(AUI_TBTOOL_TEXT_RIGHT)
        else:
            self.SetToolTextOrientation(AUI_TBTOOL_TEXT_BOTTOM)

        if agwStyle & AUI_TB_VERTICAL:
            if agwStyle & AUI_TB_CLOCKWISE:
                self.SetToolOrientation(AUI_TBTOOL_VERT_CLOCKWISE)
            elif agwStyle & AUI_TB_COUNTERCLOCKWISE:
                self.SetToolOrientation(AUI_TBTOOL_VERT_COUNTERCLOCKWISE)

                
    def GetAGWWindowStyleFlag(self):
        """
        Returns the AGW-specific window style flag.

        :see: L{SetAGWWindowStyleFlag} for an explanation of various AGW-specific style.
        """

        return self._agwStyle
    

    def SetArtProvider(self, art):
        """
        Instructs L{AuiToolBar} to use art provider specified by parameter `art`
        for all drawing calls. This allows plugable look-and-feel features. 

        :param `art`: an art provider.

        :note: The previous art provider object, if any, will be deleted by L{AuiToolBar}.
        """
        
        del self._art
        self._art = art

        if self._art:
            self._art.SetAGWFlags(self._agwStyle)
            self._art.SetTextOrientation(self._tool_text_orientation)
            self._art.SetOrientation(self._tool_orientation)
        

    def GetArtProvider(self):
        """ Returns the current art provider being used. """

        return self._art


    def AddSimpleTool(self, tool_id, label, bitmap, short_help_string="", kind=ITEM_NORMAL):
        """
        Adds a tool to the toolbar. This is the simplest method you can use to
        ass an item to the L{AuiToolBar}.

        :param `tool_id`: an integer by which the tool may be identified in subsequent operations;
        :param `label`: the toolbar tool label;
        :param `bitmap`: the primary tool bitmap;
        :param `short_help_string`: this string is used for the tools tooltip;
        :param `kind`: the item kind. Can be one of the following:

         ========================  =============================
         Item Kind                 Description
         ========================  =============================
         ``ITEM_CONTROL``          The item in the `AuiToolBar` is a control
         ``ITEM_LABEL``            The item in the `AuiToolBar` is a text label
         ``ITEM_SPACER``           The item in the `AuiToolBar` is a spacer
         ``ITEM_SEPARATOR``        The item in the `AuiToolBar` is a separator
         ``ITEM_CHECK``            The item in the `AuiToolBar` is a toolbar check item
         ``ITEM_NORMAL``           The item in the `AuiToolBar` is a standard toolbar item
         ``ITEM_RADIO``            The item in the `AuiToolBar` is a toolbar radio item
         ========================  =============================
        """
        
        return self.AddTool(tool_id, label, bitmap, wx.NullBitmap, kind, short_help_string, "", None)


    def AddToggleTool(self, tool_id, bitmap, disabled_bitmap, toggle=False, client_data=None, short_help_string="", long_help_string=""):
        """
        Adds a toggle tool to the toolbar. 

        :param `tool_id`: an integer by which the tool may be identified in subsequent operations;
        :param `bitmap`: the primary tool bitmap;
        :param `disabled_bitmap`: the bitmap to use when the tool is disabled. If it is equal to
         `wx.NullBitmap`, the disabled bitmap is automatically generated by greing the normal one;
        :param `client_data`: whatever Python object to associate with the toolbar item;
        :param `short_help_string`: this string is used for the tools tooltip;
        :param `long_help_string`: this string is shown in the statusbar (if any) of the parent
         frame when the mouse pointer is inside the tool.
        """

        kind = (toggle and [ITEM_CHECK] or [ITEM_NORMAL])[0]
        return self.AddTool(tool_id, "", bitmap, disabled_bitmap, kind, short_help_string, long_help_string, client_data)


    def AddTool(self, tool_id, label, bitmap, disabled_bitmap, kind, short_help_string, long_help_string, client_data):
        """
        Adds a tool to the toolbar. This is the full feature version of L{AddTool}.

        :param `tool_id`: an integer by which the tool may be identified in subsequent operations;
        :param `label`: the toolbar tool label;
        :param `bitmap`: the primary tool bitmap;
        :param `disabled_bitmap`: the bitmap to use when the tool is disabled. If it is equal to
         `wx.NullBitmap`, the disabled bitmap is automatically generated by greing the normal one;
        :param `kind`: the item kind. Can be one of the following:

         ========================  =============================
         Item Kind                 Description
         ========================  =============================
         ``ITEM_CONTROL``          The item in the `AuiToolBar` is a control
         ``ITEM_LABEL``            The item in the `AuiToolBar` is a text label
         ``ITEM_SPACER``           The item in the `AuiToolBar` is a spacer
         ``ITEM_SEPARATOR``        The item in the `AuiToolBar` is a separator
         ``ITEM_CHECK``            The item in the `AuiToolBar` is a toolbar check item
         ``ITEM_NORMAL``           The item in the `AuiToolBar` is a standard toolbar item
         ``ITEM_RADIO``            The item in the `AuiToolBar` is a toolbar radio item
         ========================  =============================

        :param `short_help_string`: this string is used for the tools tooltip;
        :param `long_help_string`: this string is shown in the statusbar (if any) of the parent
         frame when the mouse pointer is inside the tool.
        :param `client_data`: whatever Python object to associate with the toolbar item.
        """
        
        item = AuiToolBarItem()
        item.window = None
        item.label = label
        item.bitmap = bitmap
        item.disabled_bitmap = disabled_bitmap
        item.short_help = short_help_string
        item.long_help = long_help_string
        item.active = True
        item.dropdown = False
        item.spacer_pixels = 0

        if tool_id == wx.ID_ANY:
            tool_id = wx.NewId()
            
        item.id = tool_id
        item.state = 0
        item.proportion = 0
        item.kind = kind
        item.sizer_item = None
        item.min_size = wx.Size(-1, -1)
        item.user_data = 0
        item.sticky = False
        item.orientation = self._tool_orientation

        if not item.disabled_bitmap.IsOk():
            # no disabled bitmap specified, we need to make one
            if item.bitmap.IsOk():
                item.disabled_bitmap = MakeDisabledBitmap(item.bitmap)
        
        self._items.append(item)
        return self._items[-1]


    def AddCheckTool(self, tool_id, label, bitmap, disabled_bitmap, short_help_string="", long_help_string="", client_data=None):
        """
        Adds a new check (or toggle) tool to the L{AuiToolBar}.
        
        :see: L{AddTool}.
        """

        return self.AddTool(tool_id, label, bitmap, disabled_bitmap, ITEM_CHECK, short_help_string, long_help_string, client_data) 


    def AddRadioTool(self, tool_id, label, bitmap, disabled_bitmap, short_help_string="", long_help_string="", client_data=None):
        """
        Adds a new radio tool to the toolbar.

        Consecutive radio tools form a radio group such that exactly one button
        in the group is pressed at any moment, in other words whenever a button
        in the group is pressed the previously pressed button is automatically
        released. You should avoid having the radio groups of only one element
        as it would be impossible for the user to use such button.

        :note: By default, the first button in the radio group is initially pressed,
         the others are not.

        :see: L{AddTool}.
        """

        return self.AddTool(tool_id, label, bitmap, disabled_bitmap, ITEM_RADIO, short_help_string, long_help_string, client_data)

    
    def AddControl(self, control, label=""):
        """
        Adds any control to the toolbar, typically e.g. a combobox.

        :param `control`: the control to be added;
        :param `label`: the label which appears if the control goes into the
         overflow items in the toolbar.
        """

        item = AuiToolBarItem()
        item.window = control
        item.label = label
        item.bitmap = wx.NullBitmap
        item.disabled_bitmap = wx.NullBitmap
        item.active = True
        item.dropdown = False
        item.spacer_pixels = 0
        item.id = control.GetId()
        item.state = 0
        item.proportion = 0
        item.kind = ITEM_CONTROL
        item.sizer_item = None
        item.min_size = control.GetEffectiveMinSize()
        item.user_data = 0
        item.sticky = False
        item.orientation = self._tool_orientation

        self._items.append(item)
        return self._items[-1]


    def AddLabel(self, tool_id, label="", width=0):
        """
        Adds a label tool to the L{AuiToolBar}.

        :param `tool_id`: an integer by which the tool may be identified in subsequent operations;
        :param `label`: the toolbar tool label;
        :param `width`: the tool width.
        """

        min_size = wx.Size(-1, -1)
        
        if width != -1:
            min_size.x = width

        item = AuiToolBarItem()
        item.window = None
        item.label = label
        item.bitmap = wx.NullBitmap
        item.disabled_bitmap = wx.NullBitmap
        item.active = True
        item.dropdown = False
        item.spacer_pixels = 0

        if tool_id == wx.ID_ANY:
            tool_id = wx.NewId()
        
        item.id = tool_id
        item.state = 0
        item.proportion = 0
        item.kind = ITEM_LABEL
        item.sizer_item = None
        item.min_size = min_size
        item.user_data = 0
        item.sticky = False
        item.orientation = self._tool_orientation

        self._items.append(item)
        return self._items[-1]


    def AddSeparator(self):
        """ Adds a separator for spacing groups of tools. """
        
        item = AuiToolBarItem()
        item.window = None
        item.label = ""
        item.bitmap = wx.NullBitmap
        item.disabled_bitmap = wx.NullBitmap
        item.active = True
        item.dropdown = False
        item.id = -1
        item.state = 0
        item.proportion = 0
        item.kind = ITEM_SEPARATOR
        item.sizer_item = None
        item.min_size = wx.Size(-1, -1)
        item.user_data = 0
        item.sticky = False
        item.orientation = self._tool_orientation

        self._items.append(item)
        return self._items[-1]


    def AddSpacer(self, pixels):
        """
        Adds a spacer for spacing groups of tools.

        :param `pixels`: the width of the spacer.
        """

        item = AuiToolBarItem()
        item.window = None
        item.label = ""
        item.bitmap = wx.NullBitmap
        item.disabled_bitmap = wx.NullBitmap
        item.active = True
        item.dropdown = False
        item.spacer_pixels = pixels
        item.id = -1
        item.state = 0
        item.proportion = 0
        item.kind = ITEM_SPACER
        item.sizer_item = None
        item.min_size = wx.Size(-1, -1)
        item.user_data = 0
        item.sticky = False
        item.orientation = self._tool_orientation

        self._items.append(item)
        return self._items[-1]


    def AddStretchSpacer(self, proportion=1):
        """
        Adds a stretchable spacer for spacing groups of tools.

        :param `proportion`: the stretchable spacer proportion.
        """
        
        item = AuiToolBarItem()
        item.window = None
        item.label = ""
        item.bitmap = wx.NullBitmap
        item.disabled_bitmap = wx.NullBitmap
        item.active = True
        item.dropdown = False
        item.spacer_pixels = 0
        item.id = -1
        item.state = 0
        item.proportion = proportion
        item.kind = ITEM_SPACER
        item.sizer_item = None
        item.min_size = wx.Size(-1, -1)
        item.user_data = 0
        item.sticky = False
        item.orientation = self._tool_orientation

        self._items.append(item)
        return self._items[-1]


    def Clear(self):
        """ Deletes all the tools in the L{AuiToolBar}. """

        self._items = []
        self._sizer_element_count = 0


    def ClearTools(self):
        """ Deletes all the tools in the L{AuiToolBar}. """

        self.Clear()
        

    def DeleteTool(self, tool_id):
        """
        Removes the specified tool from the toolbar and deletes it.

        :param `tool_id`: the L{AuiToolBarItem} identifier.

        :returns: ``True`` if the tool was deleted, ``False`` otherwise.
        
        :note: Note that it is unnecessary to call L{Realize} for the change to
         take place, it will happen immediately.
        """

        idx = self.GetToolIndex(tool_id)
        
        if idx >= 0 and idx < len(self._items):
            self._items.pop(idx)
            self.Realize()
            return True
        
        return False


    def DeleteToolByPos(self, pos):
        """
        This function behaves like L{DeleteTool} but it deletes the tool at the
        specified position and not the one with the given id.

        :param `pos`: the tool position.

        :see: L{DeleteTool}        
        """
        
        if pos >= 0 and pos < len(self._items):
            
            self._items.pop(pos)
            self.Realize()
            return True

        return False


    def FindControl(self, id):
        """
        Returns a pointer to the control identified by `id` or ``None`` if no corresponding
        control is found.

        :param `id`: the control identifier.        
        """
        
        wnd = self.FindWindow(id)
        return wnd


    def FindTool(self, tool_id):
        """
        Finds a tool for the given tool id.

        :param `tool_id`: the L{AuiToolBarItem} identifier.
        """
        
        for item in self._items:
            if item.id == tool_id:
                return item
    
        return None


    def FindToolForPosition(self, x, y):
        """
        Finds a tool for the given mouse position.

        :param `x`: mouse `x` position;
        :param `y`: mouse `y` position.

        :returns: a pointer to a L{AuiToolBarItem} if a tool is found, or ``None`` otherwise.
        """

        for i, item in enumerate(self._items):
            if not item.sizer_item:
                continue

            rect = item.sizer_item.GetRect()
            if rect.Contains((x,y)):
            
                # if the item doesn't fit on the toolbar, return None
                if not self.GetToolFitsByIndex(i):
                    return None

                return item
            
        return None


    def FindToolForPositionWithPacking(self, x, y):
        """
        Finds a tool for the given mouse position, taking into account also the
        tool packing.

        :param `x`: mouse `x` position;
        :param `y`: mouse `y` position.

        :returns: a pointer to a L{AuiToolBarItem} if a tool is found, or ``None`` otherwise.
        """
        
        count = len(self._items)
        
        for i, item in enumerate(self._items):
            if not item.sizer_item:
                continue

            rect = item.sizer_item.GetRect()

            # apply tool packing
            if i+1 < count:
                rect.width += self._tool_packing

            if rect.Contains((x,y)):
            
                # if the item doesn't fit on the toolbar, return None
                if not self.GetToolFitsByIndex(i):
                    return None

                return item

        return None


    def FindToolByIndex(self, pos):
        """
        Finds a tool for the given tool position in the L{AuiToolBar}.

        :param `pos`: the tool position in the toolbar.

        :returns: a pointer to a L{AuiToolBarItem} if a tool is found, or ``None`` otherwise.        
        """
        
        if pos < 0 or pos >= len(self._items):
            return None

        return self._items[pos]


    def SetToolBitmapSize(self, size):
        """
        Sets the default size of each tool bitmap. The default bitmap size is
        16 by 15 pixels.

        :param `size`: the size of the bitmaps in the toolbar.

        :note: This should be called to tell the toolbar what the tool bitmap
         size is. Call it before you add tools.

        :note: Note that this is the size of the bitmap you pass to L{AddTool},
         and not the eventual size of the tool button.

        :todo: Add `wx.ToolBar` compatibility, actually implementing this method.
        """

        # TODO: wx.ToolBar compatibility
        pass


    def GetToolBitmapSize(self):
        """
        Returns the size of bitmap that the toolbar expects to have. The default
        bitmap size is 16 by 15 pixels.

        :note: Note that this is the size of the bitmap you pass to L{AddTool},
         and not the eventual size of the tool button.

        :todo: Add `wx.ToolBar` compatibility, actually implementing this method.
        """
        
        # TODO: wx.ToolBar compatibility
        return wx.Size(16, 15)


    def SetToolProportion(self, tool_id, proportion):
        """
        Sets the tool proportion in the toolbar.

        :param `tool_id`: the L{AuiToolBarItem} identifier;
        :param `proportion`: the tool proportion in the toolbar.
        """

        item = self.FindTool(tool_id)
        if not item:
            return

        item.proportion = proportion


    def GetToolProportion(self, tool_id):
        """
        Returns the tool proportion in the toolbar.

        :param `tool_id`: the L{AuiToolBarItem} identifier.
        """

        item = self.FindTool(tool_id)
        if not item:
            return

        return item.proportion


    def SetToolSeparation(self, separation):
        """
        Sets the separator size for the toolbar.

        :param `separation`: the separator size in pixels.
        """

        if self._art:
            self._art.SetElementSize(AUI_TBART_SEPARATOR_SIZE, separation)


    def GetToolSeparation(self):
        """ Returns the separator size for the toolbar, in pixels. """
        
        if self._art:
            return self._art.GetElementSize(AUI_TBART_SEPARATOR_SIZE)

        return 5


    def SetToolDropDown(self, tool_id, dropdown):
        """
        Assigns a drop down window menu to the toolbar item.

        :param `tool_id`: the L{AuiToolBarItem} identifier;
        :param `dropdown`: whether to assign a drop down menu or not.
        """

        item = self.FindTool(tool_id)
        if not item:
            return

        item.dropdown = dropdown


    def GetToolDropDown(self, tool_id):
        """
        Returns whether the toolbar item identified by `tool_id` has an associated
        drop down window menu or not.

        :param `tool_id`: the L{AuiToolBarItem} identifier.
        """

        item = self.FindTool(tool_id)
        if not item:
            return

        return item.dropdown


    def SetToolSticky(self, tool_id, sticky):
        """
        Sets the toolbar item as sticky or non-sticky.

        :param `tool_id`: the L{AuiToolBarItem} identifier;
        :param `sticky`: whether the tool should be sticky or not.
        """

        # ignore separators
        if tool_id == -1:
            return

        item = self.FindTool(tool_id)
        if not item:
            return

        if item.sticky == sticky:
            return

        item.sticky = sticky

        self.Refresh(False)
        self.Update()


    def GetToolSticky(self, tool_id):
        """
        Returns whether the toolbar item identified by `tool_id` has a sticky
        behaviour or not.

        :param `tool_id`: the L{AuiToolBarItem} identifier.
        """

        item = self.FindTool(tool_id)
        if not item:
            return

        return item.sticky


    def SetToolBorderPadding(self, padding):
        """
        Sets the padding between the tool border and the label.

        :param `padding`: the padding in pixels.
        """

        self._tool_border_padding = padding


    def GetToolBorderPadding(self):
        """ Returns the padding between the tool border and the label, in pixels. """

        return self._tool_border_padding


    def SetToolTextOrientation(self, orientation):
        """
        Sets the label orientation for the toolbar items.

        :param `orientation`: the L{AuiToolBarItem} label orientation.
        """

        self._tool_text_orientation = orientation

        if self._art:
            self._art.SetTextOrientation(orientation)
    

    def GetToolTextOrientation(self):
        """ Returns the label orientation for the toolbar items. """

        return self._tool_text_orientation


    def SetToolOrientation(self, orientation):
        """
        Sets the tool orientation for the toolbar items.

        :param `orientation`: the L{AuiToolBarItem} orientation.
        """

        self._tool_orientation = orientation
        if self._art:
            self._art.SetOrientation(orientation)


    def GetToolOrientation(self):
        """ Returns the orientation for the toolbar items. """

        return self._tool_orientation        


    def SetToolPacking(self, packing):
        """
        Sets the value used for spacing tools. The default value is 1 pixel.

        :param `packing`: the value for packing.
        """

        self._tool_packing = packing


    def GetToolPacking(self):
        """ Returns the value used for spacing tools. The default value is 1 pixel. """

        return self._tool_packing


    def SetOrientation(self, orientation):
        """
        Sets the toolbar orientation.

        :param `orientation`: either ``wx.VERTICAL`` or ``wx.HORIZONTAL``.

        :note: This can be temporarily overridden by L{AuiManager} when floating and
         docking a L{AuiToolBar}.
        """

        pass
    

    def SetMargins(self, left=-1, right=-1, top=-1, bottom=-1):
        """
        Set the values to be used as margins for the toolbar.

        :param `left`: the left toolbar margin;
        :param `right`: the right toolbar margin;
        :param `top`: the top toolbar margin;
        :param `bottom`: the bottom toolbar margin.
        """

        if left != -1:
            self._left_padding = left
        if right != -1:
            self._right_padding = right
        if top != -1:
            self._top_padding = top
        if bottom != -1:
            self._bottom_padding = bottom


    def SetMarginsSize(self, size):
        """
        Set the values to be used as margins for the toolbar.

        :param `size`: the margin size (an instance of `wx.Size`).
        """
        
        self.SetMargins(size.x, size.x, size.y, size.y)


    def SetMarginsXY(self, x, y):
        """
        Set the values to be used as margins for the toolbar.
        
        :param `x`: left margin, right margin and inter-tool separation value;
        :param `y`: top margin, bottom margin and inter-tool separation value.
        """
        
        self.SetMargins(x, x, y, y)        

            
    def GetGripperVisible(self):
        """ Returns whether the toolbar gripper is visible or not. """

        return self._gripper_visible


    def SetGripperVisible(self, visible):
        """
        Sets whether the toolbar gripper is visible or not.

        :param `visible`: ``True`` for a visible gripper, ``False`` otherwise.
        """

        self._gripper_visible = visible
        if visible:
            self._agwStyle |= AUI_TB_GRIPPER
        else:
            self._agwStyle &= ~AUI_TB_GRIPPER
            
        self.Realize()
        self.Refresh(False)


    def GetOverflowVisible(self):
        """ Returns whether the overflow button is visible or not. """

        return self._overflow_visible


    def SetOverflowVisible(self, visible):
        """
        Sets whether the overflow button is visible or not.

        :param `visible`: ``True`` for a visible overflow button, ``False`` otherwise.
        """

        self._overflow_visible = visible
        if visible:
            self._agwStyle |= AUI_TB_OVERFLOW
        else:
            self._agwStyle &= ~AUI_TB_OVERFLOW

        self.Refresh(False)


    def SetFont(self, font):
        """
        Sets the L{AuiToolBar} font.

        :param `font`: a `wx.Font` object.

        :note: Overridden from `wx.PyControl`.
        """        

        res = wx.PyControl.SetFont(self, font)

        if self._art:
            self._art.SetFont(font)
    
        return res


    def SetHoverItem(self, pitem):
        """
        Sets a toolbar item to be currently hovered by the mouse.

        :param `pitem`: an instance of L{AuiToolBarItem}.
        """

        former_hover = None

        for item in self._items:
        
            if item.state & AUI_BUTTON_STATE_HOVER:
                former_hover = item
                
            item.state &= ~AUI_BUTTON_STATE_HOVER

        if pitem:
            pitem.state |= AUI_BUTTON_STATE_HOVER
        
        if former_hover != pitem:
            self.Refresh(False)
            self.Update()
        

    def SetPressedItem(self, pitem):
        """
        Sets a toolbar item to be currently in a "pressed" state.

        :param `pitem`: an instance of L{AuiToolBarItem}.
        """

        former_item = None

        for item in self._items:
        
            if item.state & AUI_BUTTON_STATE_PRESSED:
                former_item = item
                
            item.state &= ~AUI_BUTTON_STATE_PRESSED
        
        if pitem:
            pitem.state &= ~AUI_BUTTON_STATE_HOVER
            pitem.state |= AUI_BUTTON_STATE_PRESSED
        
        if former_item != pitem:
            self.Refresh(False)
            self.Update()
    

    def RefreshOverflowState(self):
        """ Refreshes the overflow button. """

        if not self._overflow_sizer_item:
            self._overflow_state = 0
            return
        
        overflow_state = 0
        overflow_rect = self.GetOverflowRect()

        # find out the mouse's current position
        pt = wx.GetMousePosition()
        pt = self.ScreenToClient(pt)

        # find out if the mouse cursor is inside the dropdown rectangle
        if overflow_rect.Contains((pt.x, pt.y)):

            if _VERSION_STRING < "2.9":
                leftDown = wx.GetMouseState().LeftDown()
            else:
                leftDown = wx.GetMouseState().LeftIsDown()
        
            if leftDown:
                overflow_state = AUI_BUTTON_STATE_PRESSED
            else:
                overflow_state = AUI_BUTTON_STATE_HOVER
        
        if overflow_state != self._overflow_state:
            self._overflow_state = overflow_state
            self.Refresh(False)
            self.Update()
        
        self._overflow_state = overflow_state


    def ToggleTool(self, tool_id, state):
        """
        Toggles a tool on or off. This does not cause any event to get emitted.

        :param `tool_id`: tool in question.
        :param `state`: if ``True``, toggles the tool on, otherwise toggles it off.

        :note: This only applies to a tool that has been specified as a toggle tool.
        """
        
        tool = self.FindTool(tool_id)

        if tool:
            if tool.kind not in [ITEM_CHECK, ITEM_RADIO]:
                return

            if tool.kind == ITEM_RADIO:
                idx = self.GetToolIndex(tool_id)
                if idx >= 0 and idx < len(self._items):
                    for i in xrange(idx, len(self._items)):
                        tool = self.FindToolByIndex(i)
                        if tool.kind != ITEM_RADIO:
                            break
                        tool.state &= ~AUI_BUTTON_STATE_CHECKED

                    for i in xrange(idx, -1, -1):
                        tool = self.FindToolByIndex(i)
                        if tool.kind != ITEM_RADIO:
                            break
                        tool.state &= ~AUI_BUTTON_STATE_CHECKED 

                    tool = self.FindTool(tool_id)
                    tool.state |= AUI_BUTTON_STATE_CHECKED
            else:
                if state == True:
                    tool.state |= AUI_BUTTON_STATE_CHECKED
                else:
                    tool.state &= ~AUI_BUTTON_STATE_CHECKED 


    def GetToolToggled(self, tool_id):
        """
        Returns whether a tool is toggled or not.

        :param `tool_id`: the toolbar item identifier.

        :note: This only applies to a tool that has been specified as a toggle tool.
        """        

        tool = self.FindTool(tool_id)

        if tool:
            if tool.kind not in [ITEM_CHECK, ITEM_RADIO]:
                return False

            return (tool.state & AUI_BUTTON_STATE_CHECKED and [True] or [False])[0]
        
        return False


    def EnableTool(self, tool_id, state):
        """
        Enables or disables the tool.

        :param `tool_id`: identifier for the tool to enable or disable.
        :param `state`: if ``True``, enables the tool, otherwise disables it.
        """

        tool = self.FindTool(tool_id)

        if tool:
        
            if state == True:
                tool.state &= ~AUI_BUTTON_STATE_DISABLED
            else:
                tool.state |= AUI_BUTTON_STATE_DISABLED
        

    def GetToolEnabled(self, tool_id):
        """
        Returns whether the tool identified by `tool_id` is enabled or not.

        :param `tool_id`: the tool identifier.
        """

        tool = self.FindTool(tool_id)

        if tool:
            return (tool.state & AUI_BUTTON_STATE_DISABLED and [False] or [True])[0]

        return False


    def GetToolLabel(self, tool_id):
        """
        Returns the tool label for the tool identified by `tool_id`.

        :param `tool_id`: the tool identifier.
        """

        tool = self.FindTool(tool_id)
        if not tool:
            return ""
        
        return tool.label


    def SetToolLabel(self, tool_id, label):
        """
        Sets the tool label for the tool identified by `tool_id`.

        :param `tool_id`: the tool identifier;
        :param `label`: the new toolbar item label.
        """
        
        tool = self.FindTool(tool_id)
        if tool:    
            tool.label = label
    

    def GetToolBitmap(self, tool_id):
        """
        Returns the tool bitmap for the tool identified by `tool_id`.

        :param `tool_id`: the tool identifier.
        """
        
        tool = self.FindTool(tool_id)
        if not tool:
            return wx.NullBitmap

        return tool.bitmap


    def SetToolBitmap(self, tool_id, bitmap):
        """
        Sets the tool bitmap for the tool identified by `tool_id`.

        :param `tool_id`: the tool identifier;
        :param `bitmap`: the new bitmap for the toolbar item (an instance of `wx.Bitmap`).
        """
        
        tool = self.FindTool(tool_id)
        if tool:
            tool.bitmap = bitmap


    def SetToolNormalBitmap(self, tool_id, bitmap):
        """
        Sets the tool bitmap for the tool identified by `tool_id`.

        :param `tool_id`: the tool identifier;
        :param `bitmap`: the new bitmap for the toolbar item (an instance of `wx.Bitmap`).
        """
        
        self.SetToolBitmap(tool_id, bitmap)


    def SetToolDisabledBitmap(self, tool_id, bitmap):
        """
        Sets the tool disabled bitmap for the tool identified by `tool_id`.

        :param `tool_id`: the tool identifier;
        :param `bitmap`: the new disabled bitmap for the toolbar item (an instance of `wx.Bitmap`).
        """
        
        tool = self.FindTool(tool_id)
        if tool:
            tool.disabled_bitmap = bitmap


    def GetToolShortHelp(self, tool_id):
        """
        Returns the short help for the given tool.

        :param `tool_id`: the tool identifier.
        """

        tool = self.FindTool(tool_id)
        if not tool:
            return ""

        return tool.short_help


    def SetToolShortHelp(self, tool_id, help_string):
        """
        Sets the short help for the given tool.

        :param `tool_id`: the tool identifier;
        :param `help_string`: the string for the short help.
        """
        
        tool = self.FindTool(tool_id)
        if tool:
            tool.short_help = help_string


    def GetToolLongHelp(self, tool_id):
        """
        Returns the long help for the given tool.

        :param `tool_id`: the tool identifier.
        """

        tool = self.FindTool(tool_id)
        if not tool:
            return ""

        return tool.long_help


    def SetToolAlignment(self, alignment=wx.EXPAND):
        """
        This sets the alignment for all of the tools within the
        toolbar (only has an effect when the toolbar is expanded).

        :param `alignment`: `wx.Sizer` alignment value
         (``wx.ALIGN_CENTER_HORIZONTAL`` or ``wx.ALIGN_CENTER_VERTICAL``).
        """

        self._tool_alignment = alignment



    def SetToolLongHelp(self, tool_id, help_string):
        """
        Sets the long help for the given tool.

        :param `tool_id`: the tool identifier;
        :param `help_string`: the string for the long help.
        """
        
        tool = self.FindTool(tool_id)
        if tool:
            tool.long_help = help_string
    

    def SetCustomOverflowItems(self, prepend, append):
        """
        Sets the two lists `prepend` and `append` as custom overflow items.

        :param `prepend`: a list of L{AuiToolBarItem} to be prepended;
        :param `append`: a list of L{AuiToolBarItem} to be appended.
        """

        self._custom_overflow_prepend = prepend
        self._custom_overflow_append = append


    def GetToolCount(self):
        """ Returns the number of tools in the L{AuiToolBar}. """

        return len(self._items)


    def GetToolIndex(self, tool_id):
        """
        Returns the position of the tool in the toolbar given its identifier.

        :param `tool_id`: the toolbar item identifier.
        """

        # this will prevent us from returning the index of the
        # first separator in the toolbar since its id is equal to -1
        if tool_id == -1:
            return wx.NOT_FOUND

        for i, item in enumerate(self._items):
            if item.id == tool_id:
                return i
        
        return wx.NOT_FOUND


    def GetToolPos(self, tool_id):
        """
        Returns the position of the tool in the toolbar given its identifier.

        :param `tool_id`: the toolbar item identifier.
        """
        
        return self.GetToolIndex(tool_id)
                                

    def GetToolFitsByIndex(self, tool_id):
        """
        Returns whether the tool identified by `tool_id` fits into the toolbar or not.

        :param `tool_id`: the toolbar item identifier.
        """
        
        if tool_id < 0 or tool_id >= len(self._items):
            return False

        if not self._items[tool_id].sizer_item:
            return False

        cli_w, cli_h = self.GetClientSize()
        rect = self._items[tool_id].sizer_item.GetRect()

        if self._agwStyle & AUI_TB_VERTICAL:
            # take the dropdown size into account
            if self._overflow_visible:
                cli_h -= self._overflow_sizer_item.GetSize().y

            if rect.y+rect.height < cli_h:
                return True
        
        else:
        
            # take the dropdown size into account
            if self._overflow_visible:
                cli_w -= self._overflow_sizer_item.GetSize().x

            if rect.x+rect.width < cli_w:
                return True
        
        return False


    def GetToolFits(self, tool_id):
        """
        Returns whether the tool identified by `tool_id` fits into the toolbar or not.

        :param `tool_id`: the toolbar item identifier.
        """
        
        return self.GetToolFitsByIndex(self.GetToolIndex(tool_id))


    def GetToolRect(self, tool_id):
        """
        Returns the toolbar item rectangle

        :param `tool_id`: the toolbar item identifier.
        """

        tool = self.FindTool(tool_id)
        if tool and tool.sizer_item:
            return tool.sizer_item.GetRect()

        return wx.Rect()


    def GetToolBarFits(self):
        """ Returns whether the L{AuiToolBar} size fits in a specified size. """

        if len(self._items) == 0:
            # empty toolbar always 'fits'
            return True
        
        # entire toolbar content fits if the last tool fits
        return self.GetToolFitsByIndex(len(self._items) - 1)


    def Realize(self):
        """ Realizes the toolbar. This function should be called after you have added tools. """

        dc = wx.ClientDC(self)
        
        if not dc.IsOk():
            return False

        horizontal = True
        if self._agwStyle & AUI_TB_VERTICAL:
            horizontal = False

        # create the new sizer to add toolbar elements to
        sizer = wx.BoxSizer((horizontal and [wx.HORIZONTAL] or [wx.VERTICAL])[0])

        # add gripper area
        separator_size = self._art.GetElementSize(AUI_TBART_SEPARATOR_SIZE)
        gripper_size = self._art.GetElementSize(AUI_TBART_GRIPPER_SIZE)
        
        if gripper_size > 0 and self._gripper_visible:        
            if horizontal:
                self._gripper_sizer_item = sizer.Add((gripper_size, 1), 0, wx.EXPAND)
            else:
                self._gripper_sizer_item = sizer.Add((1, gripper_size), 0, wx.EXPAND)
        else:
            self._gripper_sizer_item = None
        
        # add "left" padding
        if self._left_padding > 0:
            if horizontal:
                sizer.Add((self._left_padding, 1))
            else:
                sizer.Add((1, self._left_padding))
        
        count = len(self._items)
        for i, item in enumerate(self._items):
        
            sizer_item = None
            kind = item.kind

            if kind == ITEM_LABEL:
                
                size = self._art.GetLabelSize(dc, self, item)
                sizer_item = sizer.Add((size.x + (self._tool_border_padding*2),
                                        size.y + (self._tool_border_padding*2)),
                                       item.proportion,
                                       item.alignment)
                if i+1 < count:
                    sizer.AddSpacer(self._tool_packing)
                

            elif kind in [ITEM_CHECK, ITEM_NORMAL, ITEM_RADIO]:
                
                size = self._art.GetToolSize(dc, self, item)
                sizer_item = sizer.Add((size.x + (self._tool_border_padding*2),
                                        size.y + (self._tool_border_padding*2)),
                                       0,
                                       item.alignment)
                # add tool packing
                if i+1 < count:
                    sizer.AddSpacer(self._tool_packing)

            elif kind == ITEM_SEPARATOR:
                
                if horizontal:
                    sizer_item = sizer.Add((separator_size, 1), 0, wx.EXPAND)
                else:
                    sizer_item = sizer.Add((1, separator_size), 0, wx.EXPAND)

                # add tool packing
                if i+1 < count:
                    sizer.AddSpacer(self._tool_packing)

            elif kind == ITEM_SPACER:
                
                if item.proportion > 0:
                    sizer_item = sizer.AddStretchSpacer(item.proportion)
                else:
                    sizer_item = sizer.Add((item.spacer_pixels, 1))
                    
            elif kind == ITEM_CONTROL:
                
                vert_sizer = wx.BoxSizer(wx.VERTICAL)
                vert_sizer.AddStretchSpacer(1)
                ctrl_sizer_item = vert_sizer.Add(item.window, 0, wx.EXPAND)
                vert_sizer.AddStretchSpacer(1)
                
                if self._agwStyle & AUI_TB_TEXT and \
                    self._tool_text_orientation == AUI_TBTOOL_TEXT_BOTTOM and \
                    item.GetLabel() != "":
                
                    s = self.GetLabelSize(item.GetLabel())
                    vert_sizer.Add((1, s.y))

                sizer_item = sizer.Add(vert_sizer, item.proportion, wx.EXPAND)
                min_size = item.min_size

                # proportional items will disappear from the toolbar if
                # their min width is not set to something really small
                if item.proportion != 0:
                    min_size.x = 1
                
                if min_size.IsFullySpecified():
                    sizer.SetItemMinSize(vert_sizer, min_size)
                    vert_sizer.SetItemMinSize(item.window, min_size)
                
                # add tool packing
                if i+1 < count:
                    sizer.AddSpacer(self._tool_packing)
                
            item.sizer_item = sizer_item
        

        # add "right" padding
        if self._right_padding > 0:
            if horizontal:
                sizer.Add((self._right_padding, 1))
            else:
                sizer.Add((1, self._right_padding))
        
        # add drop down area
        self._overflow_sizer_item = None

        if self._agwStyle & AUI_TB_OVERFLOW:
        
            overflow_size = self._art.GetElementSize(AUI_TBART_OVERFLOW_SIZE)
            if overflow_size > 0 and self._overflow_visible:
            
                if horizontal:
                    self._overflow_sizer_item = sizer.Add((overflow_size, 1), 0, wx.EXPAND)
                else:
                    self._overflow_sizer_item = sizer.Add((1, overflow_size), 0, wx.EXPAND)
            
            else:
            
                self._overflow_sizer_item = None
            
        # the outside sizer helps us apply the "top" and "bottom" padding
        outside_sizer = wx.BoxSizer((horizontal and [wx.VERTICAL] or [wx.HORIZONTAL])[0])

        # add "top" padding
        if self._top_padding > 0:
        
            if horizontal:
                outside_sizer.Add((1, self._top_padding))
            else:
                outside_sizer.Add((self._top_padding, 1))
        
        # add the sizer that contains all of the toolbar elements
        outside_sizer.Add(sizer, 1, self._tool_alignment)

        # add "bottom" padding
        if self._bottom_padding > 0:
        
            if horizontal:
                outside_sizer.Add((1, self._bottom_padding))
            else:
                outside_sizer.Add((self._bottom_padding, 1))

        del self._sizer # remove old sizer
        self._sizer = outside_sizer
        self.SetSizer(outside_sizer)

        # calculate the rock-bottom minimum size
        for item in self._items:
        
            if item.sizer_item and item.proportion > 0 and item.min_size.IsFullySpecified():
                item.sizer_item.SetMinSize((0, 0))
        
        self._absolute_min_size = self._sizer.GetMinSize()

        # reset the min sizes to what they were
        for item in self._items:
        
            if item.sizer_item and item.proportion > 0 and item.min_size.IsFullySpecified():
                item.sizer_item.SetMinSize(item.min_size)
        
        # set control size
        size = self._sizer.GetMinSize()
        self.SetMinSize(size)
        self._minWidth = size.x
        self._minHeight = size.y

        if self._agwStyle & AUI_TB_NO_AUTORESIZE == 0:
        
            cur_size = self.GetClientSize()
            new_size = self.GetMinSize()

            if new_size != cur_size:
            
                self.SetClientSize(new_size)
            
            else:
            
                self._sizer.SetDimension(0, 0, cur_size.x, cur_size.y)
            
        else:
        
            cur_size = self.GetClientSize()
            self._sizer.SetDimension(0, 0, cur_size.x, cur_size.y)
                    
        self.Refresh(False)
        return True


    def GetOverflowState(self):
        """ Returns the state of the overflow button. """

        return self._overflow_state


    def GetOverflowRect(self):
        """ Returns the rectangle of the overflow button. """

        cli_rect = wx.RectPS(wx.Point(0, 0), self.GetClientSize())
        overflow_rect = wx.Rect(*self._overflow_sizer_item.GetRect())
        overflow_size = self._art.GetElementSize(AUI_TBART_OVERFLOW_SIZE)

        if self._agwStyle & AUI_TB_VERTICAL:
        
            overflow_rect.y = cli_rect.height - overflow_size
            overflow_rect.x = 0
            overflow_rect.width = cli_rect.width
            overflow_rect.height = overflow_size
        
        else:
        
            overflow_rect.x = cli_rect.width - overflow_size
            overflow_rect.y = 0
            overflow_rect.width = overflow_size
            overflow_rect.height = cli_rect.height
        
        return overflow_rect


    def GetLabelSize(self, label):
        """
        Returns the standard size of a toolbar item.

        :param `label`: a test label.
        """

        dc = wx.ClientDC(self)
        dc.SetFont(self._font)

        return GetLabelSize(dc, label, self._tool_orientation != AUI_TBTOOL_HORIZONTAL)


    def GetAuiManager(self):
        """ Returns the L{AuiManager} which manages the toolbar. """

        try:
            return self._auiManager
        except AttributeError:
            return False


    def SetAuiManager(self, auiManager):
        """ Sets the L{AuiManager} which manages the toolbar. """
        
        self._auiManager = auiManager        

        
    def DoIdleUpdate(self):
        """ Updates the toolbar during idle times. """

        handler = self.GetEventHandler()
        if not handler:
            return
        
        need_refresh = False

        for item in self._items:
                
            if item.id == -1:
                continue

            evt = wx.UpdateUIEvent(item.id)
            evt.SetEventObject(self)

            if handler.ProcessEvent(evt):
            
                if evt.GetSetEnabled():
                
                    if item.window:
                        is_enabled = item.window.IsEnabled()
                    else:
                        is_enabled = (item.state & AUI_BUTTON_STATE_DISABLED and [False] or [True])[0]

                    new_enabled = evt.GetEnabled()
                    if new_enabled != is_enabled:
                    
                        if item.window:
                            item.window.Enable(new_enabled)
                        else:
                            if new_enabled:
                                item.state &= ~AUI_BUTTON_STATE_DISABLED
                            else:
                                item.state |= AUI_BUTTON_STATE_DISABLED
                        
                        need_refresh = True
                    
                if evt.GetSetChecked():
                
                    # make sure we aren't checking an item that can't be
                    if item.kind != ITEM_CHECK and item.kind != ITEM_RADIO:
                        continue

                    is_checked = (item.state & AUI_BUTTON_STATE_CHECKED and [True] or [False])[0]
                    new_checked = evt.GetChecked()

                    if new_checked != is_checked:
                    
                        if new_checked:
                            item.state |= AUI_BUTTON_STATE_CHECKED
                        else:
                            item.state &= ~AUI_BUTTON_STATE_CHECKED

                        need_refresh = True
                    
        if need_refresh:
            self.Refresh(False)

        
    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{AuiToolBar}.

        :param `event`: a `wx.SizeEvent` event to be processed.        
        """
        
        x, y = self.GetClientSize()
        realize = False

        if x > y:
            self.SetOrientation(wx.HORIZONTAL)
        else:
            self.SetOrientation(wx.VERTICAL)

        if (x >= y and self._absolute_min_size.x > x) or (y > x and self._absolute_min_size.y > y):
        
            # hide all flexible items
            for item in self._items:
                if item.sizer_item and item.proportion > 0 and item.sizer_item.IsShown():
                    item.sizer_item.Show(False)
                    item.sizer_item.SetProportion(0)

            if self._originalStyle & AUI_TB_OVERFLOW:
                if not self.GetOverflowVisible():
                    self.SetOverflowVisible(True)
                    realize = True
                       
        else:

            if self._originalStyle & AUI_TB_OVERFLOW and not self._custom_overflow_append and \
               not self._custom_overflow_prepend:
                if self.GetOverflowVisible():
                    self.SetOverflowVisible(False)
                    realize = True

            # show all flexible items
            for item in self._items:
                if item.sizer_item and item.proportion > 0 and not item.sizer_item.IsShown():
                    item.sizer_item.Show(True)
                    item.sizer_item.SetProportion(item.proportion)
                
        self._sizer.SetDimension(0, 0, x, y)

        if realize:
            self.Realize()
        else:
            self.Refresh(False)
            
        self.Update()

        
    def DoSetSize(self, x, y, width, height, sizeFlags=wx.SIZE_AUTO):
        """        
        Sets the position and size of the window in pixels. The `sizeFlags`
        parameter indicates the interpretation of the other params if they are
        equal to -1.

        :param `x`: the window `x` position;
        :param `y`: the window `y` position;
        :param `width`: the window width;
        :param `height`: the window height;
        :param `sizeFlags`: may have one of this bit set:
   
         ===================================  ======================================
         Size Flags                           Description
         ===================================  ======================================
         ``wx.SIZE_AUTO``                     A -1 indicates that a class-specific default should be used.
         ``wx.SIZE_AUTO_WIDTH``               A -1 indicates that a class-specific default should be used for the width.
         ``wx.SIZE_AUTO_HEIGHT``              A -1 indicates that a class-specific default should be used for the height.
         ``wx.SIZE_USE_EXISTING``             Existing dimensions should be used if -1 values are supplied.
         ``wx.SIZE_ALLOW_MINUS_ONE``          Allow dimensions of -1 and less to be interpreted as real dimensions, not default values.
         ``wx.SIZE_FORCE``                    Normally, if the position and the size of the window are already the same as the parameters of this function, nothing is done. but with this flag a window resize may be forced even in this case (supported in wx 2.6.2 and later and only implemented for MSW and ignored elsewhere currently) 
         ===================================  ======================================

        :note: Overridden from `wx.PyControl`.
        """
        
        parent_size = self.GetParent().GetClientSize()
        if x + width > parent_size.x:
            width = max(0, parent_size.x - x)
        if y + height > parent_size.y:
            height = max(0, parent_size.y - y)

        wx.PyControl.DoSetSize(self, x, y, width, height, sizeFlags)


    def OnIdle(self, event):
        """
        Handles the ``wx.EVT_IDLE`` event for L{AuiToolBar}.

        :param `event`: a `wx.IdleEvent` event to be processed.        
        """
        
        self.DoIdleUpdate()
        event.Skip()


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same
        size as it would have after a call to `Fit()`.
        
        :note: Overridden from `wx.PyControl`.
        """

        return self._absolute_min_size
    

    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{AuiToolBar}.

        :param `event`: a `wx.PaintEvent` event to be processed.        
        """

        dc = wx.AutoBufferedPaintDC(self)
        cli_rect = wx.RectPS(wx.Point(0, 0), self.GetClientSize())

        horizontal = True
        if self._agwStyle & AUI_TB_VERTICAL:
            horizontal = False

        if self._agwStyle & AUI_TB_PLAIN_BACKGROUND:
            self._art.DrawPlainBackground(dc, self, cli_rect)
        else:
            self._art.DrawBackground(dc, self, cli_rect, horizontal)

        gripper_size = self._art.GetElementSize(AUI_TBART_GRIPPER_SIZE)
        dropdown_size = self._art.GetElementSize(AUI_TBART_OVERFLOW_SIZE)

        # paint the gripper
        if gripper_size > 0 and self._gripper_sizer_item:
            gripper_rect = wx.Rect(*self._gripper_sizer_item.GetRect())
            if horizontal:
                gripper_rect.width = gripper_size
            else:
                gripper_rect.height = gripper_size
                
            self._art.DrawGripper(dc, self, gripper_rect)
        
        # calculated how far we can draw items
        if horizontal:
            last_extent = cli_rect.width
        else:
            last_extent = cli_rect.height
            
        if self._overflow_visible:
            last_extent -= dropdown_size

        # paint each individual tool
        for item in self._items:

            if not item.sizer_item:
                continue

            item_rect = wx.Rect(*item.sizer_item.GetRect())

            if (horizontal and item_rect.x + item_rect.width >= last_extent) or \
               (not horizontal and item_rect.y + item_rect.height >= last_extent):

                break
            
            if item.kind == ITEM_SEPARATOR:
                # draw a separator
                self._art.DrawSeparator(dc, self, item_rect)
            
            elif item.kind == ITEM_LABEL:
                # draw a text label only
                self._art.DrawLabel(dc, self, item, item_rect)
            
            elif item.kind == ITEM_NORMAL:
                # draw a regular button or dropdown button
                if not item.dropdown:
                    self._art.DrawButton(dc, self, item, item_rect)
                else:
                    self._art.DrawDropDownButton(dc, self, item, item_rect)
            
            elif item.kind == ITEM_CHECK:
                # draw a regular toggle button or a dropdown one
                if not item.dropdown:
                    self._art.DrawButton(dc, self, item, item_rect)
                else:
                    self._art.DrawDropDownButton(dc, self, item, item_rect)

            elif item.kind == ITEM_RADIO:
                # draw a toggle button
                self._art.DrawButton(dc, self, item, item_rect)
            
            elif item.kind == ITEM_CONTROL:
                # draw the control's label
                self._art.DrawControlLabel(dc, self, item, item_rect)
            
            # fire a signal to see if the item wants to be custom-rendered
            self.OnCustomRender(dc, item, item_rect)
        
        # paint the overflow button
        if dropdown_size > 0 and self._overflow_sizer_item:
            dropdown_rect = self.GetOverflowRect()
            self._art.DrawOverflowButton(dc, self, dropdown_rect, self._overflow_state)

        
    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{AuiToolBar}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This is intentionally empty, to reduce flicker.
        """

        pass
    

    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        cli_rect = wx.RectPS(wx.Point(0, 0), self.GetClientSize())
        self.StopPreviewTimer()

        if self._gripper_sizer_item:
        
            gripper_rect = wx.Rect(*self._gripper_sizer_item.GetRect())
            if gripper_rect.Contains(event.GetPosition()):
            
                # find aui manager
                manager = self.GetAuiManager()
                if not manager:
                    return

                x_drag_offset = event.GetX() - gripper_rect.GetX()
                y_drag_offset = event.GetY() - gripper_rect.GetY()

                clientPt = wx.Point(*event.GetPosition())
                screenPt = self.ClientToScreen(clientPt)
                managedWindow = manager.GetManagedWindow()
                managerClientPt = managedWindow.ScreenToClient(screenPt)

                # gripper was clicked
                manager.OnGripperClicked(self, managerClientPt, wx.Point(x_drag_offset, y_drag_offset))            
                return

        if self._overflow_sizer_item:
            overflow_rect = self.GetOverflowRect()

            if self._art and self._overflow_visible and overflow_rect.Contains(event.GetPosition()):
            
                e = AuiToolBarEvent(wxEVT_COMMAND_AUITOOLBAR_OVERFLOW_CLICK, -1)
                e.SetEventObject(self)
                e.SetToolId(-1)
                e.SetClickPoint(event.GetPosition())
                processed = self.ProcessEvent(e)

                if processed:
                    self.DoIdleUpdate()
                else:                
                    overflow_items = []

                    # add custom overflow prepend items, if any
                    count = len(self._custom_overflow_prepend)
                    for i in xrange(count):
                        overflow_items.append(self._custom_overflow_prepend[i])

                    # only show items that don't fit in the dropdown
                    count = len(self._items)
                    for i in xrange(count):
                    
                        if not self.GetToolFitsByIndex(i):
                            overflow_items.append(self._items[i])
                    
                    # add custom overflow append items, if any
                    count = len(self._custom_overflow_append)
                    for i in xrange(count):
                        overflow_items.append(self._custom_overflow_append[i])

                    res = self._art.ShowDropDown(self, overflow_items)
                    self._overflow_state = 0
                    self.Refresh(False)
                    if res != -1:
                        e = wx.CommandEvent(wx.wxEVT_COMMAND_MENU_SELECTED, res)
                        e.SetEventObject(self)
                        if not self.GetParent().ProcessEvent(e):
                            tool = self.FindTool(res)
                            if tool:
                                state = (tool.state & AUI_BUTTON_STATE_CHECKED and [True] or [False])[0]
                                self.ToggleTool(res, not state)
                    
                return
            
        self._dragging = False
        self._action_pos = wx.Point(*event.GetPosition())
        self._action_item = self.FindToolForPosition(*event.GetPosition())

        if self._action_item:
        
            if self._action_item.state & AUI_BUTTON_STATE_DISABLED:
            
                self._action_pos = wx.Point(-1, -1)
                self._action_item = None
                return
            
            self.SetPressedItem(self._action_item)

            # fire the tool dropdown event
            e = AuiToolBarEvent(wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN, self._action_item.id)
            e.SetEventObject(self)
            e.SetToolId(self._action_item.id)
            e.SetDropDownClicked(False)

            mouse_x, mouse_y = event.GetX(), event.GetY()
            rect = wx.Rect(*self._action_item.sizer_item.GetRect())

            if self._action_item.dropdown:
                if (self._action_item.orientation == AUI_TBTOOL_HORIZONTAL and \
                    mouse_x >= (rect.x+rect.width-BUTTON_DROPDOWN_WIDTH-1) and \
                    mouse_x < (rect.x+rect.width)) or \
                    (self._action_item.orientation != AUI_TBTOOL_HORIZONTAL and \
                     mouse_y >= (rect.y+rect.height-BUTTON_DROPDOWN_WIDTH-1) and \
                     mouse_y < (rect.y+rect.height)):
                    
                    e.SetDropDownClicked(True)            
            
            e.SetClickPoint(event.GetPosition())
            e.SetItemRect(rect)
            self.ProcessEvent(e)
            self.DoIdleUpdate()
        

    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        self.SetPressedItem(None)

        hit_item = self.FindToolForPosition(*event.GetPosition())
        
        if hit_item and not hit_item.state & AUI_BUTTON_STATE_DISABLED:
            self.SetHoverItem(hit_item)

        if self._dragging:
            # reset drag and drop member variables
            self._dragging = False
            self._action_pos = wx.Point(-1, -1)
            self._action_item = None
        
        else:

            if self._action_item and hit_item == self._action_item:
                self.SetToolTipString("")

                if hit_item.kind in [ITEM_CHECK, ITEM_RADIO]:
                    toggle = not (self._action_item.state & AUI_BUTTON_STATE_CHECKED)
                    self.ToggleTool(self._action_item.id, toggle)

                    # repaint immediately
                    self.Refresh(False)
                    self.Update()
                    
                    e = wx.CommandEvent(wx.wxEVT_COMMAND_MENU_SELECTED, self._action_item.id)
                    e.SetEventObject(self)
                    e.SetInt(toggle)
                    self._action_pos = wx.Point(-1, -1)
                    self._action_item = None
                    
                    self.ProcessEvent(e)
                    self.DoIdleUpdate()
                    
                else:

                    if self._action_item.id == ID_RESTORE_FRAME:
                        # find aui manager
                        manager = self.GetAuiManager()

                        if not manager:
                            return

                        pane = manager.GetPane(self)
                        e = framemanager.AuiManagerEvent(framemanager.wxEVT_AUI_PANE_MIN_RESTORE)

                        e.SetManager(manager)
                        e.SetPane(pane)

                        manager.ProcessEvent(e)
                        self.DoIdleUpdate()

                    else:

                        e = wx.CommandEvent(wx.wxEVT_COMMAND_MENU_SELECTED, self._action_item.id)
                        e.SetEventObject(self)
                        self.ProcessEvent(e)
                        self.DoIdleUpdate()
                
        # reset drag and drop member variables
        self._dragging = False
        self._action_pos = wx.Point(-1, -1)
        self._action_item = None


    def OnRightDown(self, event):
        """
        Handles the ``wx.EVT_RIGHT_DOWN`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        cli_rect = wx.RectPS(wx.Point(0, 0), self.GetClientSize())

        if self._gripper_sizer_item:
            gripper_rect = self._gripper_sizer_item.GetRect()
            if gripper_rect.Contains(event.GetPosition()):
                return
        
        if self._overflow_sizer_item:
        
            dropdown_size = self._art.GetElementSize(AUI_TBART_OVERFLOW_SIZE)
            if dropdown_size > 0 and event.GetX() > cli_rect.width - dropdown_size and \
               event.GetY() >= 0 and event.GetY() < cli_rect.height and self._art:
                return
            
        self._action_pos = wx.Point(*event.GetPosition())
        self._action_item = self.FindToolForPosition(*event.GetPosition())

        if self._action_item:
            if self._action_item.state & AUI_BUTTON_STATE_DISABLED:
            
                self._action_pos = wx.Point(-1, -1)
                self._action_item = None
                return


    def OnRightUp(self, event):
        """
        Handles the ``wx.EVT_RIGHT_UP`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        hit_item = self.FindToolForPosition(*event.GetPosition())

        if self._action_item and hit_item == self._action_item:
            
            e = AuiToolBarEvent(wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK, self._action_item.id)
            e.SetEventObject(self)
            e.SetToolId(self._action_item.id)
            e.SetClickPoint(self._action_pos)
            self.ProcessEvent(e)
            self.DoIdleUpdate()
            
        else:
        
            # right-clicked on the invalid area of the toolbar
            e = AuiToolBarEvent(wxEVT_COMMAND_AUITOOLBAR_RIGHT_CLICK, -1)
            e.SetEventObject(self)
            e.SetToolId(-1)
            e.SetClickPoint(self._action_pos)
            self.ProcessEvent(e)
            self.DoIdleUpdate()
        
        # reset member variables
        self._action_pos = wx.Point(-1, -1)
        self._action_item = None


    def OnMiddleDown(self, event):
        """
        Handles the ``wx.EVT_MIDDLE_DOWN`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        cli_rect = wx.RectPS(wx.Point(0, 0), self.GetClientSize())

        if self._gripper_sizer_item:
        
            gripper_rect = self._gripper_sizer_item.GetRect()
            if gripper_rect.Contains(event.GetPosition()):
                return
        
        if self._overflow_sizer_item:
        
            dropdown_size = self._art.GetElementSize(AUI_TBART_OVERFLOW_SIZE)
            if dropdown_size > 0 and event.GetX() > cli_rect.width - dropdown_size and \
               event.GetY() >= 0 and event.GetY() < cli_rect.height and self._art:            
                return
            
        self._action_pos = wx.Point(*event.GetPosition())
        self._action_item = self.FindToolForPosition(*event.GetPosition())

        if self._action_item:
            if self._action_item.state & AUI_BUTTON_STATE_DISABLED:
            
                self._action_pos = wx.Point(-1, -1)
                self._action_item = None
                return


    def OnMiddleUp(self, event):
        """
        Handles the ``wx.EVT_MIDDLE_UP`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        hit_item = self.FindToolForPosition(*event.GetPosition())

        if self._action_item and hit_item == self._action_item:        
            if hit_item.kind == ITEM_NORMAL:
            
                e = AuiToolBarEvent(wxEVT_COMMAND_AUITOOLBAR_MIDDLE_CLICK, self._action_item.id)
                e.SetEventObject(self)
                e.SetToolId(self._action_item.id)
                e.SetClickPoint(self._action_pos)
                self.ProcessEvent(e)
                self.DoIdleUpdate()
            
        # reset member variables
        self._action_pos = wx.Point(-1, -1)
        self._action_item = None


    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """
        
        # start a drag event
        if not self._dragging and self._action_item != None and self._action_pos != wx.Point(-1, -1) and \
           abs(event.GetX() - self._action_pos.x) + abs(event.GetY() - self._action_pos.y) > 5:
        
            self.SetToolTipString("")
            self._dragging = True

            e = AuiToolBarEvent(wxEVT_COMMAND_AUITOOLBAR_BEGIN_DRAG, self.GetId())
            e.SetEventObject(self)
            e.SetToolId(self._action_item.id)
            self.ProcessEvent(e)
            self.DoIdleUpdate()
            return
        
        hit_item = self.FindToolForPosition(*event.GetPosition())
        
        if hit_item:        
            if not hit_item.state & AUI_BUTTON_STATE_DISABLED:
                self.SetHoverItem(hit_item)
            else:
                self.SetHoverItem(None)
        
        else:        
            # no hit item, remove any hit item
            self.SetHoverItem(hit_item)
        
        # figure out tooltips
        packing_hit_item = self.FindToolForPositionWithPacking(*event.GetPosition())
        
        if packing_hit_item:
        
            if packing_hit_item != self._tip_item:
                self._tip_item = packing_hit_item

                if packing_hit_item.short_help != "":
                    self.StartPreviewTimer()
                    self.SetToolTipString(packing_hit_item.short_help)
                else:
                    self.SetToolTipString("")
                    self.StopPreviewTimer()
            
        else:
        
            self.SetToolTipString("")
            self._tip_item = None
            self.StopPreviewTimer()
        
        # if we've pressed down an item and we're hovering
        # over it, make sure it's state is set to pressed
        if self._action_item:
        
            if self._action_item == hit_item:
                self.SetPressedItem(self._action_item)
            else:
                self.SetPressedItem(None)
        
        # figure out the dropdown button state (are we hovering or pressing it?)
        self.RefreshOverflowState()


    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{AuiToolBar}.

        :param `event`: a `wx.MouseEvent` event to be processed.        
        """

        self.RefreshOverflowState()
        self.SetHoverItem(None)
        self.SetPressedItem(None)

        self._tip_item = None
        self.StopPreviewTimer()


    def OnSetCursor(self, event):
        """
        Handles the ``wx.EVT_SET_CURSOR`` event for L{AuiToolBar}.

        :param `event`: a `wx.SetCursorEvent` event to be processed.        
        """
        
        cursor = wx.NullCursor

        if self._gripper_sizer_item:
        
            gripper_rect = self._gripper_sizer_item.GetRect()
            if gripper_rect.Contains((event.GetX(), event.GetY())):
                cursor = wx.StockCursor(wx.CURSOR_SIZING)
            
        event.SetCursor(cursor)


    def OnCustomRender(self, dc, item, rect):
        """
        Handles custom render for single L{AuiToolBar} items.
        
        :param `dc`: a `wx.DC` device context;
        :param `item`: an instance of L{AuiToolBarItem};
        :param `rect`: the toolbar item rect.

        :note: This method must be overridden to provide custom rendering of items.
        """
        
        pass


    def IsPaneMinimized(self):
        """ Returns whether this L{AuiToolBar} contains a minimized pane tool. """
        
        manager = self.GetAuiManager()
        if not manager:
            return False
        
        if manager.GetAGWFlags() & AUI_MGR_PREVIEW_MINIMIZED_PANES == 0:
            # No previews here
            return False

        self_name = manager.GetPane(self).name
        
        if not self_name.endswith("_min"):
            # Wrong tool name
            return False

        return self_name[0:-4]
    
        
    def StartPreviewTimer(self):
        """ Starts a timer in L{AuiManager} to slide-in/slide-out the minimized pane. """

        self_name = self.IsPaneMinimized()
        if not self_name:
            return

        manager = self.GetAuiManager()        
        manager.StartPreviewTimer(self)


    def StopPreviewTimer(self):
        """ Stops a timer in L{AuiManager} to slide-in/slide-out the minimized pane. """

        self_name = self.IsPaneMinimized()
        if not self_name:
            return

        manager = self.GetAuiManager()        
        manager.StopPreviewTimer()
            
