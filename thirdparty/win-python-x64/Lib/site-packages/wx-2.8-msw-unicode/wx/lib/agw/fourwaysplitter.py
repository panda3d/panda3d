# --------------------------------------------------------------------------------- #
# FOURWAYSPLITTER wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 03 Nov 2006
# Latest Revision: 14 Apr 2010, 12.00 GMT
#
#
# TODO List
#
# 1. Any idea?
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
FourWaySplitter is a layout manager which manages 4 children like 4 panes in a
window.


Description
===========

The FourWaySplitter is a layout manager which manages four children like four
panes in a window. You can use a four-way splitter for example in a CAD program
where you may want to maintain three orthographic views, and one oblique view of
a model.

The FourWaySplitter allows interactive repartitioning of the panes by
means of moving the central splitter bars. When the FourWaySplitter is itself
resized, each child is proportionally resized, maintaining the same split-percentage.

The main characteristics of FourWaySplitter are:

- Handles horizontal, vertical or four way sizing via the sashes;
- Delayed or live update when resizing;
- Possibility to swap windows;
- Setting the vertical and horizontal split fractions;
- Possibility to expand a window by hiding the onther 3.

And a lot more. See the demo for a complete review of the functionalities.


Supported Platforms
===================

FourWaySplitter has been tested on the following platforms:
  * Windows (Windows XP);
  * Linux Ubuntu (Dapper 6.06)


Window Styles
=============

This class supports the following window styles:

================== =========== ==================================================
Window Styles      Hex Value   Description
================== =========== ==================================================
``SP_NOSASH``             0x10 No sash will be drawn on `FourWaySplitter`.
``SP_LIVE_UPDATE``        0x80 Don't draw XOR line but resize the child windows immediately.
``SP_3DBORDER``          0x200 Draws a 3D effect border.
================== =========== ==================================================


Events Processing
=================

This class processes the following events:

================================== ==================================================
Event Name                         Description
================================== ==================================================
``EVT_SPLITTER_SASH_POS_CHANGED``  The sash position was changed. This event is generated after the user releases the mouse after dragging the splitter. Processes a `wx.wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED` event.
``EVT_SPLITTER_SASH_POS_CHANGING`` The sash position is in the process of being changed. You may prevent this change from happening by calling `Veto` or you may also modify the position of the tracking bar to properly reflect the position that would be set if the drag were to be completed at this point. Processes a `wx.wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGING` event.
================================== ==================================================


License And Version
===================

FourWaySplitter is distributed under the wxPython license. 

Latest Revision: Andrea Gavana @ 14 Apr 2010, 12.00 GMT

Version 0.4

"""

__docformat__ = "epytext"


import wx

_RENDER_VER = (2,6,1,1)

# Tolerance for mouse shape and sizing
_TOLERANCE = 5

# Modes
NOWHERE = 0
FLAG_CHANGED = 1
FLAG_PRESSED = 2

# FourWaySplitter styles
SP_NOSASH = wx.SP_NOSASH
""" No sash will be drawn on `FourWaySplitter`. """
SP_LIVE_UPDATE = wx.SP_LIVE_UPDATE
""" Don't draw XOR line but resize the child windows immediately. """
SP_3DBORDER = wx.SP_3DBORDER
""" Draws a 3D effect border. """

# FourWaySplitter events
EVT_SPLITTER_SASH_POS_CHANGING = wx.EVT_SPLITTER_SASH_POS_CHANGING
""" The sash position is in the process of being changed. You may prevent this change""" \
""" from happening by calling `Veto` or you may also modify the position of the tracking""" \
""" bar to properly reflect the position that would be set if the drag were to be""" \
""" completed at this point. Processes a `wx.wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGING` event."""
EVT_SPLITTER_SASH_POS_CHANGED = wx.EVT_SPLITTER_SASH_POS_CHANGED
""" The sash position was changed. This event is generated after the user releases the""" \
""" mouse after dragging the splitter. Processes a `wx.wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED` event. """

# ---------------------------------------------------------------------------- #
# Class FourWaySplitterEvent
# ---------------------------------------------------------------------------- #

class FourWaySplitterEvent(wx.PyCommandEvent):
    """
    This event class is almost the same as `wx.SplitterEvent` except
    it adds an accessor for the sash index that is being changed.  The
    same event type IDs and event binders are used as with
    `wx.SplitterEvent`.
    """
    
    def __init__(self, evtType=wx.wxEVT_NULL, splitter=None):
        """
        Default class constructor.

        :param `evtType`: the event type;
        :param `splitter`: the associated L{FourWaySplitter} window.
        """
        
        wx.PyCommandEvent.__init__(self, evtType)

        if splitter:
            self.SetEventObject(splitter)
            self.SetId(splitter.GetId())

        self.sashIdx = -1
        self.sashPos = -1
        self.isAllowed = True


    def SetSashIdx(self, idx):
        """
        Sets the index of the sash currently involved in the event.

        :param `idx`: an integer between 0 and 3, representing the index of the
         sash involved in the event.
        """
        
        self.sashIdx = idx


    def SetSashPosition(self, pos):
        """
        In the case of ``EVT_SPLITTER_SASH_POS_CHANGED`` events, sets the new sash
        position. In the case of ``EVT_SPLITTER_SASH_POS_CHANGING`` events, sets
        the new tracking bar position so visual feedback during dragging will represent
        that change that will actually take place. Set to -1 from the event handler
        code to prevent repositioning.

        :param `pos`: the new sash position.        

        :note: May only be called while processing ``EVT_SPLITTER_SASH_POS_CHANGING``
         and ``EVT_SPLITTER_SASH_POS_CHANGED`` events.
        """
        
        self.sashPos = pos


    def GetSashIdx(self):
        """ Returns the index of the sash currently involved in the event. """

        return self.sashIdx


    def GetSashPosition(self):
        """
        Returns the new sash position.

        :note: May only be called while processing ``EVT_SPLITTER_SASH_POS_CHANGING``
         and ``EVT_SPLITTER_SASH_POS_CHANGED`` events.
        """
        
        return self.sashPos


    # methods from wx.NotifyEvent
    def Veto(self):
        """
        Prevents the change announced by this event from happening.

        :note: It is in general a good idea to notify the user about the reasons
         for vetoing the change because otherwise the applications behaviour (which
         just refuses to do what the user wants) might be quite surprising.
        """

        self.isAllowed = False


    def Allow(self):
        """
        This is the opposite of L{Veto}: it explicitly allows the event to be processed.
        For most events it is not necessary to call this method as the events are
        allowed anyhow but some are forbidden by default (this will be mentioned
        in the corresponding event description).
        """

        self.isAllowed = True


    def IsAllowed(self):
        """
        Returns ``True`` if the change is allowed (L{Veto} hasn't been called) or
        ``False`` otherwise (if it was).
        """

        return self.isAllowed
        

# ---------------------------------------------------------------------------- #
# Class FourWaySplitter
# ---------------------------------------------------------------------------- #

class FourWaySplitter(wx.PyPanel):
    """
    This class is very similar to `wx.SplitterWindow` except that it
    allows for four windows and two sashes.  Many of the same styles,
    constants, and methods behave the same as in `wx.SplitterWindow`.
    However, in addition of the ability to drag the vertical and the
    horizontal sash, by dragging at the intersection between the two
    sashes, it is possible to resize the four windows at the same time.

    :note: These things are not yet supported:
    
     * Minimum pane size (minimum of what? Width? Height?);
     * Using negative sash positions to indicate a position offset from the end;
     * User controlled unsplitting with double clicks on the sash (but supported via the L{FourWaySplitter.SetExpanded} method);
     * Sash gravity.

     
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, agwStyle=0, name="FourWaySplitter"):
        """
        Default class constructor.
        
        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyPanel` window style;
        :param `agwStyle`: the AGW-specific window style. It can be a combination of the
         following bits:

         ================== =========== ==================================================
         Window Styles      Hex Value   Description
         ================== =========== ==================================================
         ``SP_NOSASH``             0x10 No sash will be drawn on L{FourWaySplitter}.
         ``SP_LIVE_UPDATE``        0x80 Don't draw XOR line but resize the child windows immediately.
         ``SP_3DBORDER``          0x200 Draws a 3D effect border.
         ================== =========== ==================================================

        :param `name`: the window name.
        """
        
        # always turn on tab traversal
        style |= wx.TAB_TRAVERSAL

        # and turn off any border styles
        style &= ~wx.BORDER_MASK
        style |= wx.BORDER_NONE

        self._agwStyle = agwStyle        

        # initialize the base class
        wx.PyPanel.__init__(self, parent, id, pos, size, style, name)
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)

        self._windows = []
        
        self._splitx = 0
        self._splity = 0
        self._expanded = -1
        self._fhor = 5000
        self._fver = 5000
        self._offx = 0
        self._offy = 0
        self._mode = NOWHERE
        self._flags = 0
        self._isHot = False

        self._sashTrackerPen = wx.Pen(wx.BLACK, 2, wx.SOLID)
        
        self._sashCursorWE = wx.StockCursor(wx.CURSOR_SIZEWE)
        self._sashCursorNS = wx.StockCursor(wx.CURSOR_SIZENS)
        self._sashCursorSIZING = wx.StockCursor(wx.CURSOR_SIZING)
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnEnterWindow)


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the L{FourWaySplitter} window style flags.

        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         ================== =========== ==================================================
         Window Styles      Hex Value   Description
         ================== =========== ==================================================
         ``SP_NOSASH``             0x10 No sash will be drawn on L{FourWaySplitter}.
         ``SP_LIVE_UPDATE``        0x80 Don't draw XOR line but resize the child windows immediately.
         ``SP_3DBORDER``          0x200 Draws a 3D effect border.
         ================== =========== ==================================================         
        """

        self._agwStyle = agwStyle
        self.Refresh()
        

    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{FourWaySplitter} window style.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window styles.        
        """

        return self._agwStyle


    def AppendWindow(self, window):
        """
        Add a new window to the splitter at the right side or bottom
        of the window stack.

        :param `window`: an instance of `wx.Window`.
        """
        
        self.InsertWindow(len(self._windows), window)


    def InsertWindow(self, idx, window, sashPos=-1):
        """
        Insert a new window into the splitter at the position given in `idx`.

        :param `idx`: the index at which the window will be inserted;
        :param `window`: an instance of `wx.Window`;
        :param `sashPos`: the sash position after the window insertion.
        """
        
        assert window not in self._windows, "A window can only be in the splitter once!"
        
        self._windows.insert(idx, window)
        
        self._SizeWindows()


    def DetachWindow(self, window):
        """
        Removes the window from the stack of windows managed by the splitter. The
        window will still exist so you should `Hide` or `Destroy` it as needed.

        :param `window`: an instance of `wx.Window`.        
        """
        
        assert window in self._windows, "Unknown window!"
        
        idx = self._windows.index(window)
        del self._windows[idx]

        self._SizeWindows()


    def ReplaceWindow(self, oldWindow, newWindow):
        """
        Replaces `oldWindow` (which is currently being managed by the
        splitter) with `newWindow`.  The `oldWindow` window will still
        exist so you should `Hide` or `Destroy` it as needed.

        :param `oldWindow`: an instance of `wx.Window`;
        :param `newWindow`: another instance of `wx.Window`.
        """

        assert oldWindow in self._windows, "Unknown window!"
        
        idx = self._windows.index(oldWindow)
        self._windows[idx] = newWindow
        
        self._SizeWindows()


    def ExchangeWindows(self, window1, window2):
        """
        Trade the positions in the splitter of the two windows.

        :param `window1`: an instance of `wx.Window`;
        :param `window2`: another instance of `wx.Window`.        
        """
        
        assert window1 in self._windows, "Unknown window!"
        assert window2 in self._windows, "Unknown window!"
        
        idx1 = self._windows.index(window1)
        idx2 = self._windows.index(window2)
        self._windows[idx1] = window2
        self._windows[idx2] = window1

        if "__WXMSW__" in wx.Platform:
            self.Freeze()

        self._SizeWindows()

        if "__WXMSW__" in wx.Platform:
            self.Thaw()


    def GetWindow(self, idx):
        """
        Returns the window at the index `idx`.

        :param `idx`: the index at which the window is located.
        """

        if len(self._windows) > idx:
            return self._windows[idx]
        
        return None

    # Get top left child
    def GetTopLeft(self):
        """ Returns the top left window (window index: 0). """

        return self.GetWindow(0)
    

    # Get top right child
    def GetTopRight(self):
        """ Returns the top right window (window index: 1). """

        return self.GetWindow(1)
  

    # Get bottom left child
    def GetBottomLeft(self):
        """ Returns the bottom left window (window index: 2). """

        return self.GetWindow(2)


    # Get bottom right child
    def GetBottomRight(self):
        """ Returns the bottom right window (window index: 3). """

        return self.GetWindow(3)


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same size
        as it would have after a call to `Fit()`.

        :note: Overridden from `wx.PyPanel`.        
        """

        if not self._windows:
            # something is better than nothing...
            return wx.Size(10, 10)
        
        width = height = 0
        border = self._GetBorderSize()
        
        tl = self.GetTopLeft()
        tr = self.GetTopRight()
        bl = self.GetBottomLeft()
        br = self.GetBottomRight()
    
        for win in self._windows:
            w, h = win.GetEffectiveMinSize()
            width += w
            height += h
            
        if tl and tr:
          width += self._GetSashSize()

        if bl and br:
          height += self._GetSashSize()
          
        return wx.Size(width+2*border, height+2*border)
  

    # Recompute layout
    def _SizeWindows(self):
        """
        Recalculate the layout based on split positions and split fractions.

        :see: L{SetHSplit} and L{SetVSplit} for more information about split fractions.
        """
            
        win0 = self.GetTopLeft()
        win1 = self.GetTopRight()
        win2 = self.GetBottomLeft()
        win3 = self.GetBottomRight()

        width, height = self.GetSize()
        barSize = self._GetSashSize()
        border = self._GetBorderSize()
        
        if self._expanded < 0:
            totw = width - barSize - 2*border
            toth = height - barSize - 2*border
            self._splitx = (self._fhor*totw)/10000
            self._splity = (self._fver*toth)/10000
            rightw = totw - self._splitx
            bottomh = toth - self._splity
            if win0:
                win0.SetDimensions(0, 0, self._splitx, self._splity)
                win0.Show() 
            if win1:
                win1.SetDimensions(self._splitx + barSize, 0, rightw, self._splity)
                win1.Show() 
            if win2:
                win2.SetDimensions(0, self._splity + barSize, self._splitx, bottomh)
                win2.Show() 
            if win3:
                win3.SetDimensions(self._splitx + barSize, self._splity + barSize, rightw, bottomh)
                win3.Show() 

        else:

            if self._expanded < len(self._windows):
                for ii, win in enumerate(self._windows):
                    if ii == self._expanded:
                        win.SetDimensions(0, 0, width-2*border, height-2*border)
                        win.Show()
                    else:
                        win.Hide()

        
    # Determine split mode
    def GetMode(self, pt):
        """
        Determines the split mode for L{FourWaySplitter}.

        :param `pt`: the point at which the mouse has been clicked, an instance of
         `wx.Point`.

        :return: One of the following 3 split modes:

         ================= ==============================
         Split Mode        Description
         ================= ==============================
         ``wx.HORIZONTAL`` the user has clicked on the horizontal sash
         ``wx.VERTICAL``   The user has clicked on the vertical sash
         ``wx.BOTH``       The user has clicked at the intersection between the 2 sashes
         ================= ==============================

        """

        barSize = self._GetSashSize()        
        flag = wx.BOTH
        
        if pt.x < self._splitx - _TOLERANCE:
            flag &= ~wx.VERTICAL

        if pt.y < self._splity - _TOLERANCE:
            flag &= ~wx.HORIZONTAL

        if pt.x >= self._splitx + barSize + _TOLERANCE:
            flag &= ~wx.VERTICAL
            
        if pt.y >= self._splity + barSize + _TOLERANCE:
            flag &= ~wx.HORIZONTAL
            
        return flag
  

    # Move the split intelligently
    def MoveSplit(self, x, y):
        """
        Moves the split accordingly to user action.

        :param `x`: the new splitter `x` coordinate;
        :param `y`: the new splitter `y` coordinate.
        """

        width, height = self.GetSize()
        barSize = self._GetSashSize()
        
        if x < 0: x = 0
        if y < 0: y = 0
        if x > width - barSize: x = width - barSize
        if y > height - barSize: y = height - barSize
        
        self._splitx = x
        self._splity = y


    # Adjust layout
    def AdjustLayout(self):
        """
        Adjust layout of L{FourWaySplitter}. Mainly used to recalculate the
        correct values for split fractions.
        """

        width, height = self.GetSize()
        barSize = self._GetSashSize()
        border = self._GetBorderSize()
        
        self._fhor = (width > barSize and \
                      [(10000*self._splitx+(width-barSize-1))/(width-barSize)] \
                      or [0])[0]
        
        self._fver = (height > barSize and \
                      [(10000*self._splity+(height-barSize-1))/(height-barSize)] \
                      or [0])[0]

        self._SizeWindows()
            

    # Button being pressed
    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{FourWaySplitter}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self.IsEnabled():
            return
        
        pt = event.GetPosition()
        self.CaptureMouse()
        self._mode = self.GetMode(pt)

        if self._mode:
            self._offx = pt.x - self._splitx
            self._offy = pt.y - self._splity
            if not self.GetAGWWindowStyleFlag() & wx.SP_LIVE_UPDATE:
                self.DrawSplitter(wx.ClientDC(self))
                self.DrawTrackSplitter(self._splitx, self._splity)

            self._flags |= FLAG_PRESSED
            

    # Button being released
    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{FourWaySplitter}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """
        
        if not self.IsEnabled():
            return

        if self.HasCapture():
            self.ReleaseMouse()

        flgs = self._flags
        
        self._flags &= ~FLAG_CHANGED
        self._flags &= ~FLAG_PRESSED
        
        if flgs & FLAG_PRESSED:
            
            if not self.GetAGWWindowStyleFlag() & wx.SP_LIVE_UPDATE:
                self.DrawTrackSplitter(self._splitx, self._splity)
                self.DrawSplitter(wx.ClientDC(self))
                self.AdjustLayout()
                
            if flgs & FLAG_CHANGED:
                event = FourWaySplitterEvent(wx.wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, self)
                event.SetSashIdx(self._mode)
                event.SetSashPosition(wx.Point(self._splitx, self._splity))
                self.GetEventHandler().ProcessEvent(event)                

        self._mode = NOWHERE
        

    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{FourWaySplitter}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self.SetCursor(wx.STANDARD_CURSOR)
        self._RedrawIfHotSensitive(False)


    def OnEnterWindow(self, event):
        """
        Handles the ``wx.EVT_ENTER_WINDOW`` event for L{FourWaySplitter}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """
        
        self._RedrawIfHotSensitive(True)


    def _RedrawIfHotSensitive(self, isHot):
        """
        Used internally. Redraw the splitter if we are using a hot-sensitive splitter.

        :param `isHot`: ``True`` if the splitter is in a hot state, ``False`` otherwise.
        """

        if not wx.VERSION >= _RENDER_VER:
            return

        if wx.RendererNative.Get().GetSplitterParams(self).isHotSensitive:
            self._isHot = isHot
            dc = wx.ClientDC(self)
            self.DrawSplitter(dc)

        
    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{FourWaySplitter}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self.HasFlag(wx.SP_NOSASH):
            return 

        pt = event.GetPosition()

        # Moving split
        if self._flags & FLAG_PRESSED:
                    
            oldsplitx = self._splitx
            oldsplity = self._splity
            
            if self._mode == wx.BOTH:
                self.MoveSplit(pt.x - self._offx, pt.y - self._offy)
              
            elif self._mode == wx.VERTICAL:
                self.MoveSplit(pt.x - self._offx, self._splity)
              
            elif self._mode == wx.HORIZONTAL:
                self.MoveSplit(self._splitx, pt.y - self._offy)

            # Send a changing event
            if not self.DoSendChangingEvent(wx.Point(self._splitx, self._splity)):
                self._splitx = oldsplitx
                self._splity = oldsplity
                return              

            if oldsplitx != self._splitx or oldsplity != self._splity:
                if not self.GetAGWWindowStyleFlag() & wx.SP_LIVE_UPDATE:
                    self.DrawTrackSplitter(oldsplitx, oldsplity)
                    self.DrawTrackSplitter(self._splitx, self._splity)
                else:
                    self.AdjustLayout()

                self._flags |= FLAG_CHANGED
        
        # Change cursor based on position
        ff = self.GetMode(pt)
        
        if ff == wx.BOTH:
            self.SetCursor(self._sashCursorSIZING)

        elif ff == wx.VERTICAL:
            self.SetCursor(self._sashCursorWE)

        elif ff == wx.HORIZONTAL:
            self.SetCursor(self._sashCursorNS)

        else:
            self.SetCursor(wx.STANDARD_CURSOR)

        event.Skip()


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{FourWaySplitter}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.PaintDC(self)
        self.DrawSplitter(dc)


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{FourWaySplitter}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        parent = wx.GetTopLevelParent(self)
        if parent.IsIconized():
            event.Skip()
            return
    
        self._SizeWindows()


    def DoSendChangingEvent(self, pt):
        """
        Sends a ``EVT_SPLITTER_SASH_POS_CHANGING`` event.

        :param `pt`: the point at which the splitter is being positioned.
        """

        # send the event
        event = FourWaySplitterEvent(wx.wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGING, self)
        event.SetSashIdx(self._mode)
        event.SetSashPosition(pt)
        
        if self.GetEventHandler().ProcessEvent(event) and not event.IsAllowed():
            # the event handler vetoed the change or missing event.Skip()
            return False
        else:
            # or it might have changed the value
            return True


    def _GetSashSize(self):
        """ Used internally. """

        if self.HasFlag(wx.SP_NOSASH):
            return 0

        if wx.VERSION >= _RENDER_VER:
            return wx.RendererNative.Get().GetSplitterParams(self).widthSash
        else:
            return 5


    def _GetBorderSize(self):
        """ Used internally. """

        if wx.VERSION >= _RENDER_VER:
            return wx.RendererNative.Get().GetSplitterParams(self).border
        else:
            return 0

        
    # Draw the horizontal split
    def DrawSplitter(self, dc):
        """
        Actually draws the sashes.

        :param `dc`: an instance of `wx.DC`.
        """

        backColour = self.GetBackgroundColour()        
        dc.SetBrush(wx.Brush(backColour, wx.SOLID))
        dc.SetPen(wx.Pen(backColour))
        dc.Clear()

        if wx.VERSION >= _RENDER_VER:
            if self.HasFlag(wx.SP_3DBORDER):
                wx.RendererNative.Get().DrawSplitterBorder(
                    self, dc, self.GetClientRect())
        else:
            barSize = self._GetSashSize()

        # if we are not supposed to use a sash then we're done.
        if self.HasFlag(wx.SP_NOSASH):
            return

        flag = 0
        if self._isHot:
            flag = wx.CONTROL_CURRENT
        
        width, height = self.GetSize()

        if self._mode & wx.VERTICAL:
            if wx.VERSION >= _RENDER_VER:
                wx.RendererNative.Get().DrawSplitterSash(self, dc,
                                                         self.GetClientSize(),
                                                         self._splitx, wx.VERTICAL, flag)
            else:
                dc.DrawRectangle(self._splitx, 0, barSize, height)

        if self._mode & wx.HORIZONTAL:
            if wx.VERSION >= _RENDER_VER:
                wx.RendererNative.Get().DrawSplitterSash(self, dc,
                                                         self.GetClientSize(),
                                                         self._splity, wx.VERTICAL, flag)
            else:
                dc.DrawRectangle(0, self._splity, width, barSize)


    def DrawTrackSplitter(self, x, y):
        """
        Draws a fake sash in case we don't have ``wx.SP_LIVE_UPDATE`` style.

        :param `x`: the `x` position of the sash;
        :param `y`: the `y` position of the sash.

        :note: This method relies on `wx.ScreenDC` which is currently unavailable on wxMac.        
        """

        # Draw a line to represent the dragging sash, for when not
        # doing live updates            
        w, h = self.GetClientSize()
        dc = wx.ScreenDC()
        
        dc.SetLogicalFunction(wx.INVERT)
        dc.SetPen(self._sashTrackerPen)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        
        if self._mode == wx.VERTICAL:
            x1 = x
            y1 = 2
            x2 = x
            y2 = h-2
            if x1 > w:
                x1 = w
                x2 = w
            elif x1 < 0:
                x1 = 0
                x2 = 0

            x1, y1 = self.ClientToScreenXY(x1, y1)
            x2, y2 = self.ClientToScreenXY(x2, y2)
         
            dc.DrawLine(x1, y1, x2, y2)
            dc.SetLogicalFunction(wx.COPY)
                                
        elif self._mode == wx.HORIZONTAL:

            x1 = 2
            y1 = y
            x2 = w-2
            y2 = y
            if y1 > h:
                y1 = h
                y2 = h
            elif y1 < 0:
                y1 = 0
                y2 = 0

            x1, y1 = self.ClientToScreenXY(x1, y1)
            x2, y2 = self.ClientToScreenXY(x2, y2)

            dc.DrawLine(x1, y1, x2, y2)
            dc.SetLogicalFunction(wx.COPY)
            
        elif self._mode == wx.BOTH:

            x1 = 2
            x2 = w-2
            y1 = y
            y2 = y

            x1, y1 = self.ClientToScreenXY(x1, y1)
            x2, y2 = self.ClientToScreenXY(x2, y2)

            dc.DrawLine(x1, y1, x2, y2)
                        
            x1 = x
            x2 = x
            y1 = 2
            y2 = h-2

            x1, y1 = self.ClientToScreenXY(x1, y1)
            x2, y2 = self.ClientToScreenXY(x2, y2)

            dc.DrawLine(x1, y1, x2, y2)
            dc.SetLogicalFunction(wx.COPY)            

        
    # Change horizontal split [fraction*10000]
    def SetHSplit(self, s):
        """
        Change horizontal split fraction.

        :param `s`: the split fraction, which is an integer value between 0 and
         10000 (inclusive), indicating how much space to allocate to the leftmost
         panes. For example, to split the panes at 35 percent, use::

            fourSplitter.SetHSplit(3500)
          
        """

        if s < 0: s = 0
        if s > 10000: s  =10000
        if s != self._fhor:
            self._fhor = s
            self._SizeWindows()

    
    # Change vertical split [fraction*10000]
    def SetVSplit(self, s):
        """
        Change vertical split fraction.

        :param `s`: the split fraction, which is an integer value between 0 and
         10000 (inclusive), indicating how much space to allocate to the topmost
         panes. For example, to split the panes at 35 percent, use::

            fourSplitter.SetVSplit(3500)
          
        """

        if s < 0: s = 0
        if s > 10000: s  =10000
        if s != self._fver:
            self._fver = s
            self._SizeWindows()


    # Expand one or all of the four panes
    def SetExpanded(self, expanded):
        """
        This method is used to expand one of the four window to fill the
        whole client size (when `expanded` >= 0) or to return to the four-window
        view (when `expanded` < 0).

        :param `expanded`: an integer >= 0 to expand a window to fill the whole
         client size, or an integer < 0 to return to the four-window view.
        """
        
        if expanded >= 4:
            raise Exception("ERROR: SetExpanded: index out of range: %d"%expanded)

        if self._expanded != expanded:
            self._expanded = expanded
            self._SizeWindows()


    
