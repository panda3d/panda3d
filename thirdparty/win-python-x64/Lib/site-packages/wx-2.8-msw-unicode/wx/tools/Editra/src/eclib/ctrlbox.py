###############################################################################
# Name: ctrlbox.py                                                            #
# Purpose: Container Window helper class                                      #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: ControlBox

Sizer managed panel class with support for a toolbar like control that can be
placed on the top/bottom of the main window area, multiple control bars are also
possible.

Class ControlBar:

Toolbar like control with automatic item spacing and layout.

Styles:
  - CTRLBAR_STYLE_DEFAULT: Plain background
  - CTRLBAR_STYLE_GRADIENT: Draw the bar with a vertical gradient.
  - CTRLBAR_STYLE_BORDER_BOTTOM: add a border to the bottom
  - CTRLBAR_STYLE_BORDER: add a border to the top
  - CTRLBAR_STYLE_VERTICAL = Vertical ControlBar tool layout

Class ControlBox:

The ControlBox is a sizer managed panel that supports easy creation of windows
that require a sandwich like layout.

+---------------------------------------+
| ControlBar                            |
+---------------------------------------+
|                                       |
|                                       |
|          MainWindow Area              |
|                                       |
|                                       |
|                                       |
+---------------------------------------+
| ControlBar                            |
+---------------------------------------+

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ctrlbox.py 67325 2011-03-27 20:24:20Z CJP $"
__revision__ = "$Revision: 67325 $"

__all__ = ["ControlBox", "CTRLBOX_NAME_STR",

           "ControlBar", "ControlBarEvent",
           "CTRLBAR_STYLE_DEFAULT", "CTRLBAR_STYLE_GRADIENT",
           "CTRLBAR_STYLE_BORDER_TOP", "CTRLBAR_STYLE_BORDER_BOTTOM",
           "CTRLBAR_STYLE_VERTICAL",
           "EVT_CTRLBAR", "edEVT_CTRLBAR", "CTRLBAR_NAME_STR",

           "SegmentBar", "SegmentBarEvent",
           "EVT_SEGMENT_SELECTED", "edEVT_SEGMENT_SELECTED",
           "EVT_SEGMENT_CLOSE", "edEVT_SEGMENT_CLOSE",
           "CTRLBAR_STYLE_LABELS", "CTRLBAR_STYLE_NO_DIVIDERS",
           "SEGBTN_OPT_CLOSEBTNL", "SEGBTN_OPT_CLOSEBTNR",
           "SEGBAR_NAME_STR", "SEGMENT_HT_NOWHERE",
           "SEGMENT_HT_SEG", "SEGMENT_HT_X_BTN"
]

#--------------------------------------------------------------------------#
# Dependencies
import wx

# Local Imports
from eclutil import AdjustColour, DrawCircleCloseBmp

#--------------------------------------------------------------------------#
# Globals

#-- Control Name Strings --#
CTRLBAR_NAME_STR = u'EditraControlBar'
CTRLBOX_NAME_STR = u'EditraControlBox'
SEGBAR_NAME_STR = u'EditraSegmentBar'

#-- Control Style Flags --#

# ControlBar / SegmentBar Style Flags
CTRLBAR_STYLE_DEFAULT       = 0
CTRLBAR_STYLE_GRADIENT      = 1     # Paint the bar with a gradient
CTRLBAR_STYLE_BORDER_BOTTOM = 2     # Add a border to the bottom
CTRLBAR_STYLE_BORDER_TOP    = 4     # Add a border to the top
CTRLBAR_STYLE_LABELS        = 8     # Draw labels under the icons (SegmentBar)
CTRLBAR_STYLE_NO_DIVIDERS   = 16    # Don't draw dividers between segments
CTRLBAR_STYLE_VERTICAL      = 32    # Control bar in vertical orientation

# Segment Button Options
SEGBTN_OPT_NONE          = 1     # No options set.
SEGBTN_OPT_CLOSEBTNL     = 2     # Close button on the segments left side.
SEGBTN_OPT_CLOSEBTNR     = 4     # Close button on the segment right side.

# Hit test locations
SEGMENT_HT_NOWHERE = 0
SEGMENT_HT_SEG     = 1
SEGMENT_HT_X_BTN   = 2

# Segment States
SEGMENT_STATE_NONE = 0  # Hover no where
SEGMENT_STATE_SEG  = 1  # Hover on segment
SEGMENT_STATE_X    = 2  # Hover on segment x button

# ControlBar event for items added by AddTool
edEVT_CTRLBAR = wx.NewEventType()
EVT_CTRLBAR = wx.PyEventBinder(edEVT_CTRLBAR, 1)
class ControlBarEvent(wx.PyCommandEvent):
    """ControlBar Button Event"""

edEVT_SEGMENT_SELECTED = wx.NewEventType()
EVT_SEGMENT_SELECTED = wx.PyEventBinder(edEVT_SEGMENT_SELECTED, 1)
edEVT_SEGMENT_CLOSE = wx.NewEventType()
EVT_SEGMENT_CLOSE = wx.PyEventBinder(edEVT_SEGMENT_CLOSE, 1)
class SegmentBarEvent(wx.PyCommandEvent):
    """SegmentBar Button Event"""
    def __init__(self, etype, id=0):
        super(SegmentBarEvent, self).__init__(etype, id)

        # Attributes
        self.notify = wx.NotifyEvent(etype, id)
        self._pre = -1
        self._cur = -1

    def GetPreviousSelection(self):
        """Get the previously selected segment
        @return: int

        """
        return self._pre

    def GetCurrentSelection(self):
        """Get the currently selected segment
        @return: int

        """
        return self._cur

    def IsAllowed(self):
        """Is the event allowed to propagate
        @return: bool

        """
        return self.notify.IsAllowed()

    def SetSelections(self, previous=-1, current=-1):
        """Set the events selection
        @keyword previous: previously selected button index (int)
        @keyword previous: currently selected button index (int)

        """
        self._pre = previous
        self._cur = current

    def Veto(self):
        """Veto the event"""
        self.notify.Veto()

#--------------------------------------------------------------------------#

class ControlBox(wx.PyPanel):
    """Simple managed panel helper class that allows for adding and
    managing the position of a small toolbar like panel.
    @see: L{ControlBar}

    """
    def __init__(self, parent, id=wx.ID_ANY,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.TAB_TRAVERSAL|wx.NO_BORDER,
                 name=CTRLBOX_NAME_STR):
        super(ControlBox, self).__init__(parent, id, pos, size, style, name)

        # Attributes
        self._vsizer = wx.BoxSizer(wx.VERTICAL)
        self._hsizer = wx.BoxSizer(wx.HORIZONTAL)
        self._cbars = dict()
        self._main = None

        # Layout
        self._hsizer.Add(self._vsizer, 1, wx.EXPAND)
        self.SetSizer(self._hsizer)

    #---- Properties ----#
    Window = property(lambda self: self.GetWindow())

    def _GetCtrlBarSizer(self, pos):
        """Get the correct sizer for the ControlBar at the given pos
        @param pos: wx.TOP/LEFT/RIGHT/BOTTOM

        """
        if pos in (wx.TOP, wx.BOTTOM):
            sizer = self._vsizer
        else:
            sizer = self._hsizer
        return sizer

    def ChangeWindow(self, window):
        """Change the main window area, and return the current window
        @param window: Any window/panel like object
        @return: the old window or None

        """
        rwindow = None
        if self.GetWindow() is None or not isinstance(self._main, wx.Window):
            del self._main
            topb = self.GetControlBar(wx.TOP)
            if topb is None:
                self._vsizer.Add(window, 1, wx.EXPAND)
            else:
                self._vsizer.Insert(1, window, 1, wx.EXPAND)
        else:
            self._vsizer.Replace(self._main, window)
            rwindow = self._main

        self._main = window
        return rwindow

    def CreateControlBar(self, pos=wx.TOP):
        """Create a ControlBar at the given position if one does not
        already exist.
        @keyword pos: wx.TOP (default), BOTTOM, LEFT, RIGHT
        @postcondition: A top aligned L{ControlBar} is created.
        @return: ControlBar

        """
        cbar = self.GetControlBar(pos)
        if cbar is None:
            dsize = (-1, 24)
            style=CTRLBAR_STYLE_GRADIENT
            if pos in (wx.LEFT, wx.RIGHT):
                dsize = (24, -1)
                style |= CTRLBAR_STYLE_VERTICAL
            cbar = ControlBar(self, size=dsize, style=style)
            self.SetControlBar(cbar, pos)
        return cbar

    def GetControlBar(self, pos=wx.TOP):
        """Get the L{ControlBar} used by this window
        @param pos: wx.TOP, BOTTOM, LEFT, RIGHT
        @return: ControlBar or None

        """
        assert pos in (wx.TOP, wx.LEFT, wx.BOTTOM, wx.RIGHT)
        cbar = self._cbars.get(pos, None)
        return cbar

    def GetWindow(self):
        """Get the main display window
        @return: Window or None

        """
        return self._main

    def ReplaceControlBar(self, ctrlbar, pos=wx.TOP):
        """Replace the L{ControlBar} at the given position
        with the given ctrlbar and return the bar that was
        replaced or None.
        @param ctrlbar: L{ControlBar}
        @keyword pos: Position
        @return: L{ControlBar} or None

        """
        assert isinstance(ctrlbar, ControlBar)
        assert pos in (wx.TOP, wx.BOTTOM, wx.LEFT, wx.RIGHT)
        tbar = self.GetControlBar(pos)
        rbar = None
        sizer = self._GetCtrlBarSizer(pos)
        if tbar is None and pos in (wx.TOP, wx.LEFT):
            sizer.Insert(0, ctrlbar, 0, wx.EXPAND)
        elif tbar is None and pos in (wx.BOTTOM, wx.RIGHT):
            sizer.Add(ctrlbar, 0, wx.EXPAND)
        else:
            sizer.Replace(tbar, ctrlbar)
            rbar = tbar

        self._cbars[pos] = ctrlbar

        return rbar

    def SetControlBar(self, ctrlbar, pos=wx.TOP):
        """Set the ControlBar used by this ControlBox
        @param ctrlbar: L{ControlBar}
        @keyword pos: wx.TOP/wx.BOTTOM/wx.LEFT/wx.RIGHT

        """
        assert isinstance(ctrlbar, ControlBar)
        assert pos in (wx.TOP, wx.BOTTOM, wx.LEFT, wx.RIGHT)
        tbar = self.GetControlBar(pos)
        if tbar is ctrlbar:
            return # ignore setting same bar again
        sizer = self._GetCtrlBarSizer(pos)
        if tbar is None and pos in (wx.TOP, wx.LEFT):
            sizer.Insert(0, ctrlbar, 0, wx.EXPAND)
        elif tbar is None and pos in (wx.BOTTOM, wx.RIGHT):
            sizer.Add(ctrlbar, 0, wx.EXPAND)
        else:
            sizer.Replace(tbar, ctrlbar)

            try:
                tbar.Destroy()
            except wx.PyDeadObjectError:
                pass

        self._cbars[pos] = ctrlbar

    def SetWindow(self, window):
        """Set the main window control portion of the box. This will be the
        main central item shown in the box
        @param window: Any window/panel like object

        """
        if self.GetWindow() is None:
            topb = self.GetControlBar(wx.TOP)
            botb = self.GetControlBar(wx.BOTTOM)
            if (topb and botb is None) or (topb is None and botb is None):
                self._vsizer.Add(window, 1, wx.EXPAND)
            elif botb and topb is None:
                self._vsizer.Insert(0, window, 1, wx.EXPAND)
            else:
                self._vsizer.Insert(1, window, 1, wx.EXPAND)
        else:
            self._vsizer.Replace(self._main, window)

            try:
                self._main.Destroy()
            except wx.PyDeadObjectError:
                pass

        self._main = window

#--------------------------------------------------------------------------#

class ControlBar(wx.PyPanel):
    """Toolbar like control container for use with a L{ControlBox}. It
    uses a panel with a managed sizer as a convenient way to add a small
    bar with various controls in it to any window.

    """
    def __init__(self, parent, id=wx.ID_ANY,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=CTRLBAR_STYLE_DEFAULT,
                 name=CTRLBAR_NAME_STR):
        super(ControlBar, self).__init__(parent, id, pos, size,
                                         wx.TAB_TRAVERSAL|wx.NO_BORDER, name)

        tsz_orient = wx.HORIZONTAL
        msz_orient = wx.VERTICAL
        if style & CTRLBAR_STYLE_VERTICAL:
            tsz_orient = wx.VERTICAL
            msz_orient = wx.HORIZONTAL

        # Attributes
        self._style = style
        self._sizer = wx.BoxSizer(tsz_orient)
        self._tools = dict(simple=list())
        self._spacing = (5, 5)

        # Drawing related
        color = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
        if wx.Platform != '__WXMAC__':
            self._color2 = AdjustColour(color, 15)
            self._color = AdjustColour(color, -10)
        else:
            self._color2 = AdjustColour(color, 15)
            self._color = AdjustColour(color, -20)

        pcolor = tuple([min(190, x) for x in AdjustColour(self._color, -25)])
        self._pen = wx.Pen(pcolor, 1)

        # Setup
        msizer = wx.BoxSizer(msz_orient)
        spacer = (0, 0)
        msizer.Add(spacer, 0)
        msizer.Add(self._sizer, 1, wx.EXPAND)
        msizer.Add(spacer, 0)
        self.SetSizer(msizer)

        # Event Handlers
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_BUTTON, self._DispatchEvent)

    def _DispatchEvent(self, evt):
        """Translate the button events generated by the controls added by
        L{AddTool} to L{ControlBarEvent}'s.

        """
        e_id = evt.GetId()
        if e_id in self._tools['simple']:
            cb_evt = ControlBarEvent(edEVT_CTRLBAR, e_id)
            self.GetEventHandler().ProcessEvent(cb_evt)
        else:
            # Allow to propagate
            evt.Skip()

    def _GetAlignment(self):
        """Verify and get the proper secondary alignment based on the
        control bar alignment.

        """
        if not self.IsVerticalMode():
            align2 = wx.ALIGN_CENTER_VERTICAL
        else:
            align2 = wx.ALIGN_CENTER_HORIZONTAL
        return align2

    def AddControl(self, control, align=-1, stretch=0):
        """Add a control to the bar
        @param control: The control to add to the bar
        @keyword align: wx.ALIGN_**
        @keyword stretch: The controls proportions 0 for normal, 1 for expand

        """
        if wx.Platform == '__WXMAC__':
            if hasattr(control, 'SetWindowVariant'):
                control.SetWindowVariant(wx.WINDOW_VARIANT_SMALL)

        # Default to proper alignment when -1 specified
        if align not in (wx.ALIGN_LEFT, wx.ALIGN_RIGHT,
                         wx.ALIGN_BOTTOM, wx.ALIGN_TOP):
            if self.IsVerticalMode():
                align = wx.ALIGN_TOP
            else:
                align = wx.ALIGN_LEFT

        align2 = self._GetAlignment()
        if align in (wx.ALIGN_LEFT, wx.ALIGN_TOP):
            self._sizer.Add(self._spacing, 0)
            self._sizer.Add(control, stretch, align|align2)
        else:
            self._sizer.Add(control, stretch, align|align2)
            self._sizer.Add(self._spacing, 0)

        self.Layout()

    def AddSpacer(self, width, height):
        """Add a fixed size spacer to the control bar
        @param width: width of the spacer
        @param height: height of the spacer

        """
        self._sizer.Add((width, height), 0)

    def AddStretchSpacer(self):
        """Add an expanding spacer to the bar that will stretch and
        contract when the window changes size.

        """
        self._sizer.AddStretchSpacer(2)

    def AddTool(self, tid, bmp, help=u'', align=-1):
        """Add a simple bitmap button tool to the control bar
        @param tid: Tool Id
        @param bmp: Tool bitmap
        @keyword help: Short help string
        @keyword align: wx.ALIGN_**

        """
        tool = wx.BitmapButton(self, tid, bmp, style=wx.NO_BORDER)
        if wx.Platform == '__WXGTK__':
            # SetMargins not available in wxPython 2.9+
            getattr(tool, 'SetMargins', lambda x,y: False)(0, 0)
            spacer = (0, 0)
        else:
            spacer = self._spacing
        tool.SetToolTipString(help)

        # Default to proper alignment when unknown is specified
        if align not in (wx.ALIGN_LEFT, wx.ALIGN_RIGHT,
                         wx.ALIGN_BOTTOM, wx.ALIGN_TOP):
            if self.IsVerticalMode():
                align = wx.ALIGN_TOP
            else:
                align = wx.ALIGN_LEFT

        align2 = self._GetAlignment()
        self._tools['simple'].append(tool.GetId())
        if align in (wx.ALIGN_TOP, wx.ALIGN_LEFT):
            self._sizer.Add(spacer, 0)
            self._sizer.Add(tool, 0, align|align2)
        else:
            self._sizer.Add(spacer, 0)
            self._sizer.Add(tool, 0, align|align2)

    def GetControlSizer(self):
        """Get the sizer that is used to layout the contols (horizontal sizer)
        @return: wx.BoxSizer

        """
        return self._sizer

    def GetControlSpacing(self):
        """Get the spacing used between controls
        @return: size tuple

        """
        return self._spacing

    def IsVerticalMode(self):
        """Is the ControlBar in vertical orientation
        @return: bool

        """
        return self._style & CTRLBAR_STYLE_VERTICAL

    def DoPaintBackground(self, dc, rect, color, color2):
        """Paint the background of the given rect based on the style of
        the control bar.
        @param dc: DC to draw on
        @param rect: wx.Rect
        @param color: Pen/Base gradient color
        @param color2: Gradient end color

        """
        # Paint the gradient
        if self._style & CTRLBAR_STYLE_GRADIENT:
            if isinstance(dc, wx.GCDC):
                gc = dc.GetGraphicsContext()
            else:
                gc = wx.GraphicsContext.Create(dc)

            if not self.IsVerticalMode():
                grad = gc.CreateLinearGradientBrush(rect.x, rect.y, rect.x,
                                                    rect.x+rect.height,
                                                    color2, color)
            else:
                grad = gc.CreateLinearGradientBrush(rect.x, rect.y,
                                                    rect.x+rect.width,
                                                    rect.y,
                                                    color2, color)

            gc.SetPen(gc.CreatePen(self._pen))
            gc.SetBrush(grad)
            gc.DrawRectangle(rect.x, rect.y, rect.Width - 0.5, rect.Height - 0.5)

        dc.SetPen(wx.Pen(color, 1))

        # TODO: handle vertical mode
        if not self.IsVerticalMode():
            # Add a border to the bottom
            if self._style & CTRLBAR_STYLE_BORDER_BOTTOM:
                dc.DrawLine(rect.x, rect.GetHeight() - 1,
                            rect.GetWidth(), rect.GetHeight() - 1)

            # Add a border to the top
            if self._style & CTRLBAR_STYLE_BORDER_TOP:
                dc.DrawLine(rect.x, 1, rect.GetWidth(), 1)

    def OnPaint(self, evt):
        """Paint the background to match the current style
        @param evt: wx.PaintEvent

        """
        dc = wx.AutoBufferedPaintDCFactory(self)
        gc = wx.GCDC(dc)
        rect = self.GetClientRect()

        self.DoPaintBackground(gc, rect, self._color, self._color2)

        evt.Skip()

    def SetToolSpacing(self, px):
        """Set the spacing to use between tools/controls.
        @param px: int (number of pixels)
        @todo: dynamically update existing layouts

        """
        self._spacing = (px, px)

    def SetVMargin(self, top, bottom):
        """WARNING this method is Deprecated use SetMargins instead!!
        @param top: Top margin in pixels
        @param bottom: Bottom margin in pixels

        """
        # TODO: Remove all usage of this method
        self.SetMargins(top, bottom)

    def SetMargins(self, param1, param2):
        """Setup the margins on the edges of the ControlBar
        @param param1: left/top margin depending on orientation
        @param param2: right/bottom margin depending on orientation

        """
        sizer = self.GetSizer()
        if wx.VERSION < (2, 9, 0, 0, ''):
            sizer.GetItem(0).SetSpacer((param1, param1))
            sizer.GetItem(2).SetSpacer((param2, param2))
        else:
            sizer.GetItem(0).AssignSpacer((param1, param1))
            sizer.GetItem(2).AssignSpacer((param2, param2))
        sizer.Layout()

    def SetWindowStyle(self, style):
        """Set the style flags of this window
        @param style: long

        """
        if self.IsVerticalMode() and not (CTRLBAR_STYLE_VERTICAL & style):
            # Switching from vertical to HORIZONTAL
            self._sizer.SetOrientation(wx.HORIZONTAL)
        elif not self.IsVerticalMode() and (CTRLBAR_STYLE_VERTICAL & style):
            # Switching from horizontal to vertical
            self._sizer.SetOrientation(wx.VERTICAL)
        self._style = style
        self.Layout()
        self.Refresh()

#--------------------------------------------------------------------------#

class _SegmentButton(object):
    """Class for managing segment button data"""
    def __init__(self, id_, bmp, label, lbl_size):
        super(_SegmentButton, self).__init__()

        # Attributes
        self._id = id_
        self._bmp = bmp
        self._lbl = label
        self._lbl_size = lbl_size
        self._rect = wx.Rect()
        self._bx1 = 0
        self._bx2 = 0
        self._opts = 0
        self._selected = False
        self._x_button = wx.Rect()
        self._x_state = SEGMENT_STATE_NONE

    Id = property(lambda self: self._id,
                  lambda self, id_: setattr(self, '_id', id_))
    Bitmap = property(lambda self: self._bmp,
                      lambda self, bmp: setattr(self, '_bmp', bmp))
    Label = property(lambda self: self._lbl,
                     lambda self, label: setattr(self, '_lbl', label))
    LabelSize = property(lambda self: self._lbl_size,
                         lambda self, size: setattr(self, '_lbl_size', size))
    Rect = property(lambda self: self._rect,
                    lambda self, rect: setattr(self, '_rect', rect))
    Selected = property(lambda self: self._selected,
                        lambda self, sel: setattr(self, '_selected', sel))
    XState = property(lambda self: self._x_state,
                      lambda self, state: setattr(self, '_x_state', state))
    XButton = property(lambda self: self._x_button,
                       lambda self, rect: setattr(self, '_x_button', rect))
    BX1 = property(lambda self: self._bx1,
                   lambda self, x1: setattr(self, '_bx1', x1))
    BX2 = property(lambda self: self._bx2,
                   lambda self, x2: setattr(self, '_bx2', x2))
    Options = property(lambda self: self._opts,
                       lambda self, opt: setattr(self, '_opts', opt))

class SegmentBar(ControlBar):
    """Simple toolbar like control that displays bitmaps and optionally
    labels below each bitmap. The bitmaps are turned into a toggle button
    where only one segment in the bar can be selected at one time.

    """
    HPAD = 5
    VPAD = 3
    def __init__(self, parent, id=wx.ID_ANY,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=CTRLBAR_STYLE_DEFAULT,
                 name=SEGBAR_NAME_STR):
        super(SegmentBar, self).__init__(parent, id, pos, size, style, name)

        # Attributes
        self._buttons = list() # list of _SegmentButtons
        self._segsize = (0, 0)
        self._selected = -1
        self._scolor1 = AdjustColour(self._color, -20)
        self._scolor2 = AdjustColour(self._color2, -20)
        self._spen = wx.Pen(AdjustColour(self._pen.GetColour(), -25))
        self._x_clicked_before = False
        self._tip_timer = wx.Timer(self)

        if wx.Platform == '__WXMAC__':
            self.SetWindowVariant(wx.WINDOW_VARIANT_SMALL)

        # Event Handlers
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnEnter)
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnLeave)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_TIMER, self.OnTipTimer, self._tip_timer)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy)

    def _RestartTimer(self):
        """Reset the tip timer for showing tooltips when
        the segments have their labels hidden.

        """
        if not (self._style & CTRLBAR_STYLE_LABELS):
            if self._tip_timer.IsRunning():
                self._tip_timer.Stop()
            self._tip_timer.Start(1000, True)

    def OnDestroy(self, evt):
        """Cleanup on Destroy"""
        if evt.GetEventObject() is self:
            if self._tip_timer.IsRunning():
                self._tip_timer.Stop()
        evt.Skip()

    def AddSegment(self, id, bmp, label=u''):
        """Add a segment to the bar
        @param id: button id
        @param bmp: wx.Bitmap
        @param label: string

        """
        assert bmp.IsOk()
        lsize = self.GetTextExtent(label)
        segment = _SegmentButton(id, bmp, label, lsize)
        self._buttons.append(segment)
        self.InvalidateBestSize()
        self.Refresh()

    def DoDrawButton(self, dc, pos, bidx, selected=False, draw_label=False):
        """Draw a button
        @param dc: DC to draw on
        @param xpos: X coordinate (horizontal mode) / Y coordinate (vertical mode)
        @param bidx: button dict
        @keyword selected: is this the selected button (bool)
        @keyword draw_label: draw the label (bool)
        return: int (next xpos)

        """
        button = self._buttons[bidx]
        height = self.Rect.height
        bVertical = self.IsVerticalMode()

        # Draw the background of the button
        if not bVertical:
            rside = pos + self._segsize[0]
            brect = wx.Rect(pos, 0, rside - pos, height)
        else:
            rside = pos + self._segsize[1]
            brect = wx.Rect(0, pos, self.Rect.Width, rside - pos)

        button.Rect = brect
        if selected:
            self.DoPaintBackground(dc, brect, self._scolor1, self._scolor2)

        # Draw the bitmap
        bmp = button.Bitmap
        bsize = button.Bitmap.Size
        if not bVertical:
            bxpos = ((self._segsize[0] / 2) - (bsize.width / 2)) + pos
            bpos = (bxpos, SegmentBar.VPAD)
        else:
            bxpos = ((self._segsize[0] / 2) - (bsize.width / 2))
            bpos = (bxpos, pos + SegmentBar.VPAD)
        dc.DrawBitmap(bmp, bpos[0], bpos[1], bmp.Mask != None)

        if draw_label:
            lcolor = wx.SystemSettings.GetColour(wx.SYS_COLOUR_WINDOWTEXT)
            dc.SetTextForeground(lcolor)
            twidth, theight = button.LabelSize
            if not bVertical:
                lxpos = pos
                typos = height - theight - 2
            else:
                lxpos = 0
                typos = pos + (self._segsize[1] - theight - 2)
            trect = wx.Rect(lxpos, typos, self._segsize[0], theight + 3)
            dc.DrawLabel(button.Label, trect, wx.ALIGN_CENTER)

        if not selected:
            if not (self._style & CTRLBAR_STYLE_NO_DIVIDERS):
                dc.SetPen(self._pen)
                if not bVertical:
                    dc.DrawLine(pos, 0, pos, height)
                    dc.DrawLine(rside, 0, rside, height)
                else:
                    dc.DrawLine(0, pos, self.Rect.Width, pos)
                    dc.DrawLine(0, rside, self.Rect.Width, rside)
        else:
            dc.SetPen(self._spen)
            tmpx = pos + 1
            trside = rside - 1
            if not bVertical:
                dc.DrawLine(tmpx, 0, tmpx, height)
                dc.DrawLine(trside, 0, trside, height)
            else:
                dc.DrawLine(0, tmpx, self.Rect.Width, tmpx)
                dc.DrawLine(0, trside, self.Rect.Width, trside)

            tpen = wx.Pen(self._spen.Colour)
            tpen.SetJoin(wx.JOIN_BEVEL)
            if not bVertical:
                mpoint = height / 2
                dc.DrawLine(tmpx + 1, mpoint, tmpx, 0)
                dc.DrawLine(tmpx + 1, mpoint, tmpx, height)
                dc.DrawLine(trside - 1, mpoint, trside, 0)
                dc.DrawLine(trside - 1, mpoint, trside, height)
            else:
                mpoint = self.Rect.Width / 2
                dc.DrawLine(mpoint, tmpx + 1, 0, tmpx)
                dc.DrawLine(mpoint, tmpx + 1, self.Rect.Width, tmpx)
                dc.DrawLine(mpoint, trside - 1, 0, trside)
                dc.DrawLine(mpoint, trside - 1, self.Rect.Width, trside)

        # Update derived button data
        button.BX1 = pos + 1
        button.BX2 = rside - 1
        button.Selected = selected

        # Draw delete button if button has one
        if self.SegmentHasCloseButton(bidx):
            if not bVertical:
                brect = wx.Rect(button.BX1, 0, button.BX2 - (pos - 1), height)
            else:
                brect = wx.Rect(0, button.BX1, self.Rect.Width, 
                                button.BX2 - (pos - 1))
            self.DoDrawCloseBtn(dc, button, brect)
        return rside # right/bottom of button just drawn

    def DoDrawCloseBtn(self, gcdc, button, rect):
        """Draw the close button on the segment
        @param gcdc: Device Context
        @param button: Segment Dict
        @param rect: Segment Rect

        """
        if button.Options & SEGBTN_OPT_CLOSEBTNL:
            x = rect.x + 8
            y = rect.y + 6
        else:
            x = (rect.x + rect.Width) - 8
            y = rect.y + 6

        color = self._scolor2
        if button.Selected:
            color = AdjustColour(color, -25)

        if button.XState == SEGMENT_STATE_X:
            color = AdjustColour(color, -20)

        gcdc.SetPen(wx.Pen(AdjustColour(color, -30)))

        brect = wx.Rect(x-3, y-3, 8, 8)
        bmp = DrawCircleCloseBmp(color, wx.WHITE)
        gcdc.DrawBitmap(bmp, brect.x, brect.y)
        button.XButton = brect
        return
    
        # Square style button
#        gcdc.DrawRectangleRect(brect)
#        gcdc.SetPen(wx.BLACK_PEN)
#        gcdc.DrawLine(brect.x+1, brect.y+1,
#                      brect.x + brect.GetWidth() - 1, brect.y + brect.GetHeight() - 1)
#        gcdc.DrawLine(brect.x + brect.GetWidth() - 1, brect.y + 1,
#                      brect.x + 1, brect.y + brect.GetHeight() - 1)
#        gcdc.SetBrush(brush)
#        gcdc.SetPen(pen)
#        button['xbtn'] = brect

    def DoGetBestSize(self):
        """Get the best size for the control"""
        mwidth, mheight = 0, 0
        draw_label = self._style & CTRLBAR_STYLE_LABELS
        for btn in self._buttons:
            bwidth, bheight = btn.Bitmap.Size
            twidth = btn.LabelSize[0]
            if bheight > mheight:
                mheight = bheight

            if bwidth > mwidth:
                mwidth = bwidth

            if draw_label:
                if twidth > mwidth:
                    mwidth = twidth

        # Adjust for label text
        if draw_label and len(self._buttons):
            mheight += self._buttons[0].LabelSize[1]

        if self.IsVerticalMode():
            height = (mheight + (SegmentBar.VPAD * 2) * len(self._buttons))
            width = mwidth
        else:
            width = (mwidth + (SegmentBar.HPAD * 2)) * len(self._buttons)
            height = mheight

        size = wx.Size(width + (SegmentBar.HPAD * 2),
                       height + (SegmentBar.VPAD * 2))
        self.CacheBestSize(size)
        self._segsize = (mwidth + (SegmentBar.HPAD * 2),
                         mheight + (SegmentBar.VPAD * 2))
        return size

    def GetIndexFromPosition(self, pos):
        """Get the segment index closest to the given position"""
        if not self.IsVerticalMode():
            cur_x = pos[0]
        else:
            cur_x = pos[1]
        for idx, button in enumerate(self._buttons):
            xpos = button.BX1
            xpos2 = button.BX2
            if cur_x >= xpos and cur_x <= xpos2 + 1:
                return idx
        else:
            return wx.NOT_FOUND

    def GetSegmentCount(self):
        """Get the number segments in the control
        @return: int

        """
        return len(self._buttons)

    def GetSegmentLabel(self, index):
        """Get the label of the given segment
        @param index: segment index
        @return: string

        """
        return self._buttons[index].Label

    def GetSelection(self):
        """Get the currently selected index"""
        return self._selected

    def HitTest(self, pos):
        """Find where the position is in the window
        @param pos: (x, y) in client cords
        @return: int

        """
        index = self.GetIndexFromPosition(pos)
        where = SEGMENT_HT_NOWHERE
        if index != wx.NOT_FOUND:
            button = self._buttons[index]
            if self.SegmentHasCloseButton(index):
                brect = button.XButton
                trect = wx.Rect(brect.x, brect.y, brect.Width+4, brect.Height+4)
                if trect.Contains(pos):
                    where = SEGMENT_HT_X_BTN
                else:
                    where = SEGMENT_HT_SEG
            else:
                where = SEGMENT_HT_SEG

        return where, index

    def OnEraseBackground(self, evt):
        """Handle the erase background event"""
        pass

    def OnLeftDown(self, evt):
        """Handle clicks on the bar
        @param evt: wx.MouseEvent

        """
        epos = evt.GetPosition()
        index = self.GetIndexFromPosition(epos)
        if index != wx.NOT_FOUND:
            button = self._buttons[index]
            pre = self._selected
            self._selected = index

            if self._selected != pre:
                self.Refresh()
                sevt = SegmentBarEvent(edEVT_SEGMENT_SELECTED, button.Id)
                sevt.SetSelections(pre, index)
                sevt.SetEventObject(self)
                self.GetEventHandler().ProcessEvent(sevt)

        self._x_clicked_before = False

        # Check for click on close btn
        if self.SegmentHasCloseButton(index):
            if self.HitTest(epos)[0] == SEGMENT_HT_X_BTN:
                self._x_clicked_before = True

        evt.Skip()

    def OnLeftUp(self, evt):
        """Handle clicks on the bar
        @param evt: wx.MouseEvent

        """
        epos = evt.GetPosition()
        where, index = self.HitTest(epos)

        # Check for click on close btn
        if self.SegmentHasCloseButton(index) and self._x_clicked_before:
            if where == SEGMENT_HT_X_BTN:
                event = SegmentBarEvent(edEVT_SEGMENT_CLOSE, self.GetId())
                event.SetSelections(index, index)
                event.SetEventObject(self)
                self.GetEventHandler().ProcessEvent(event)
                if not event.IsAllowed():
                    return False
                removed = self.RemoveSegment(index)

        evt.Skip()

    def OnEnter(self, evt):
        """Mouse has entered the SegmentBar, update state info"""
        evt.Skip()

    def OnLeave(self, evt):
        """Mouse has left the SegmentBar, update state info"""
        if self._tip_timer.IsRunning():
            self._tip_timer.Stop()
        evt.Skip()

    def OnMouseMove(self, evt):
        """Handle when the mouse moves over the bar"""
        epos = evt.GetPosition()
        where, index = self.HitTest(epos)
        if index == -1:
            return
        if not self.SegmentHasCloseButton(index):
            self._RestartTimer()
            return

        # Update button state
        button = self._buttons[index]
        x_state = button.XState
        button.XState = SEGMENT_STATE_NONE

        if where != SEGMENT_HT_NOWHERE:
            if where == SEGMENT_HT_X_BTN:
                button.XState = SEGMENT_STATE_X
            elif where == SEGMENT_HT_SEG:
                # TODO: add highlight option for hover on segment
                pass
        else:
            self._RestartTimer()
            evt.Skip()
            return

        # If the hover state over a segments close button
        # has changed redraw the close button to reflect the
        # proper state.
        if button.XState != x_state:
            crect = self.GetClientRect()
            if not self.IsVerticalMode():
                brect = wx.Rect(button.BX1, 0,
                                button.BX2 - (button.BX1 - 2),
                                crect.Height)
            else:
                brect = wx.Rect(button.BX1, 0,
                                crect.Width,
                                button.BX2 - (button.BX1 - 2))
            self.Refresh(False, brect)
        self._RestartTimer()
        evt.Skip()

    def OnTipTimer(self, evt):
        """Show the tooltip for the current SegmentButton"""
        pos = self.ScreenToClient(wx.GetMousePosition())
        where, index = self.HitTest(pos)
        if index != -1:
            button = self._buttons[index]
            if button.Label:
                rect = button.Rect
                x,y = self.ClientToScreenXY(rect.x, rect.y)
                rect.x = x
                rect.y = y
                wx.TipWindow(self, button.Label, rectBound=rect) # Transient

    def OnPaint(self, evt):
        """Paint the control"""
        dc = wx.AutoBufferedPaintDCFactory(self)
        gc = wx.GCDC(dc)

        # Setup
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        gc.SetBrush(wx.TRANSPARENT_BRUSH)
        gc.SetFont(self.GetFont())
        gc.SetBackgroundMode(wx.TRANSPARENT)
        gc.Clear()

        # Paint the background
        rect = self.GetClientRect()
        self.DoPaintBackground(gc, rect, self._color, self._color2)

        # Draw the buttons
        # TODO: would be more efficient to just redraw the buttons that
        #       need redrawing.
        npos = SegmentBar.HPAD
        if self.IsVerticalMode():
            npos = SegmentBar.VPAD
        use_labels = self._style & CTRLBAR_STYLE_LABELS
        for idx, button in enumerate(self._buttons):
            npos = self.DoDrawButton(gc, npos, idx,
                                     self._selected == idx,
                                     use_labels)

    def RemoveSegment(self, index):
        """Remove a segment from the bar
        @param index: int
        @return: bool

        """
        button = self._buttons[index]

        # TODO: wxPython 2.8.9.2 this causes a crash...
#        if button['bmp']:
#            button['bmp'].Destroy()
        del self._buttons[index]

        if self.GetSelection() == index:
            count = self.GetSegmentCount()
            if index >= count:
                self.SetSelection(count-1)

        self.Refresh()
        return True

    def SegmentHasCloseButton(self, index):
        """Does the segment at index have a close button
        @param index: int

        """
        button = self._buttons[index]
        if button.Options & SEGBTN_OPT_CLOSEBTNL or \
           button.Options & SEGBTN_OPT_CLOSEBTNR:
            return True
        return False

    def SetSegmentImage(self, index, bmp):
        """Set the image to use on the given segment
        @param index: int
        @param bmp: Bitmap

        """
        assert bmp.IsOk()
        segment = self._buttons[index]
        if segment.Bitmap.IsOk():
            segment.Bitmap.Destroy()
            segment.Bitmap = None
        segment.Bitmap = bmp
        self.InvalidateBestSize()
        self.Refresh()

    def SetSegmentLabel(self, index, label):
        """Set the label for a given segment
        @param index: segment index
        @param label: string

        """
        segment = self._buttons[index]
        lsize = self.GetTextExtent(label)
        segment.Label = label
        segment.LabelSize = lsize
        self.InvalidateBestSize()
        self.Refresh()

    def SetSegmentOption(self, index, option):
        """Set an option on a given segment
        @param index: segment index
        @param option: option to set

        """
        button = self._buttons[index]
        button.Options = button.Options|option
        self.Refresh()

    def SetSelection(self, index):
        """Set the selection
        @param index: int

        """
        self._selected = index
        self.Refresh()

# Cleanup namespace
#del SegmentBar.__dict__['AddControl']
#del SegmentBar.__dict__['AddSpacer']
#del SegmentBar.__dict__['AddTool']
#del SegmentBar.__dict__['SetToolSpacing']
#del SegmentBar.__dict__['SetVMargin']

#--------------------------------------------------------------------------#
