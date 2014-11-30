# --------------------------------------------------------------------------------- #
# PYPROGRESS wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 03 Nov 2006
# Latest Revision: 14 Apr 2010, 12.00 GMT
#
#
# TODO List
#
# 1. Do we support all the styles of wx.ProgressDialog in indeterminated mode?
#
# 2. Other ideas?
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
PyProgress is similar to `wx.ProgressDialog` in indeterminated mode, but with a
different gauge appearance and a different spinning behavior.


Description
===========

PyProgress is similar to `wx.ProgressDialog` in indeterminated mode, but with a
different gauge appearance and a different spinning behavior. The moving gauge
can be drawn with a single solid colour or with a shading gradient foreground.
The gauge background colour is user customizable.

The bar does not move always from the beginning to the end as in `wx.ProgressDialog`
in indeterminated mode, but spins cyclically forward and backward.

Other options include:

- Possibility to change the proportion between the spinning bar and the
  entire gauge, so that the bar can be longer or shorter (the default is 20%);
- Modifying the number of steps the spinning bar performs before a forward
  (or backward) loop reverses.
    
PyProgress can optionally display a ``Cancel`` button, and a `wx.StaticText` which
outputs the elapsed time from the starting of the process.


Supported Platforms
===================

PyProgress has been tested on the following platforms:
  * Windows (Windows XP);
  * Linux Ubuntu (Dapper 6.06)


Window Styles
=============

This class supports the following window styles:

=================== =========== ==================================================
Window Styles       Hex Value   Description
=================== =========== ==================================================
``PD_CAN_ABORT``            0x1 This flag tells the dialog that it should have a ``Cancel`` button which the user may press. If this happens, the next call to `Update` will return ``False``.
``PD_APP_MODAL``            0x2 Make the progress dialog modal. If this flag is not given, it is only 'locally' modal - that is the input to the parent window is disabled, but not to the other ones.
``PD_AUTO_HIDE``            0x4 Causes the progress dialog to disappear from screen as soon as the maximum value of the progress meter has been reached.
``PD_ELAPSED_TIME``         0x8 This flag tells the dialog that it should show elapsed time (since creating the dialog).
=================== =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

PyProgress is distributed under the wxPython license. 

Latest Revision: Andrea Gavana @ 14 Apr 2010, 12.00 GMT

Version 0.4

"""

__docformat__ = "epytext"


import wx

# Some constants, taken straight from wx.ProgressDialog
Uncancelable = -1
Canceled = 0
Continue = 1
Finished = 2

# Margins between gauge and text/button
LAYOUT_MARGIN = 8

# PyProgress styles
PD_CAN_ABORT = wx.PD_CAN_ABORT
""" This flag tells the dialog that it should have a "Cancel" button which the user""" \
""" may press. If this happens, the next call to `Update()` will return ``False``. """
PD_APP_MODAL = wx.PD_APP_MODAL
""" Make the progress dialog modal. If this flag is not given, it is only 'locally'"""\
""" modal - that is the input to the parent window is disabled, but not to the other ones. """
PD_AUTO_HIDE = wx.PD_AUTO_HIDE
""" Causes the progress dialog to disappear from screen as soon as the maximum""" \
""" value of the progress meter has been reached. """
PD_ELAPSED_TIME = wx.PD_ELAPSED_TIME
""" This flag tells the dialog that it should show elapsed time (since creating the dialog). """

# ---------------------------------------------------------------------------- #
# Class ProgressGauge
# ---------------------------------------------------------------------------- #

class ProgressGauge(wx.PyWindow):
    """ This class provides a visual alternative for `wx.Gauge`."""
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=(-1,30)):
        """
        Default class constructor.

        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform.
        """

        wx.PyWindow.__init__(self, parent, id, pos, size, style=wx.SUNKEN_BORDER)

        self._value = 0
        self._steps = 50
        self._pos = 0
        self._current = 0
        self._gaugeproportion = 0.2
        self._firstGradient = wx.WHITE
        self._secondGradient = wx.BLUE
        self._background = wx.Brush(wx.WHITE, wx.SOLID)
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)


    def GetFirstGradientColour(self):
        """ Returns the first gradient colour. """

        return self._firstGradient


    def SetFirstGradientColour(self, colour):
        """
        Sets the first gradient colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        if not isinstance(colour, wx.Colour):
            colour = wx.NamedColour(colour)
            
        self._firstGradient = colour
        self.Refresh()


    def GetSecondGradientColour(self):
        """ Returns the second gradient colour. """

        return self._secondGradient        


    def SetSecondGradientColour(self, colour):
        """
        Sets the second gradient colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        if not isinstance(colour, wx.Colour):
            colour = wx.NamedColour(colour)

        self._secondGradient = colour
        self.Refresh()
        

    def GetGaugeBackground(self):
        """ Returns the gauge background colour. """

        return self._background


    def SetGaugeBackground(self, colour):
        """
        Sets the gauge background colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        if not isinstance(colour, wx.Colour):
            colour = wx.NamedColour(colour)

        self._background = wx.Brush(colour, wx.SOLID)        


    def SetGaugeSteps(self, steps):
        """
        Sets the number of steps the gauge performs before switching from
        forward to backward (or vice-versa) movement.

        :param `steps`: the number of steps the gauge performs before switching from
         forward to backward (or vice-versa) movement.
        """

        if steps <= 0:
            raise Exception("ERROR:\n Gauge steps must be greater than zero. ")

        if steps != self._steps:
            self._steps = steps


    def GetGaugeSteps(self):
        """
        Returns the number of steps the gauge performs before switching from
        forward to backward (or vice-versa) movement.
        """

        return self._steps        


    def GetGaugeProportion(self):
        """
        Returns the relative proportion between the sliding bar and the
        whole gauge.
        """

        return self._gaugeproportion

    
    def SetGaugeProportion(self, proportion):
        """
        Sets the relative proportion between the sliding bar and the
        whole gauge.

        :param `proportion`: a floating point number representing the relative
         proportion between the sliding bar and the whole gauge.
        """

        if proportion <= 0 or proportion >= 1:
            raise Exception("ERROR:\n Gauge proportion must be between 0 and 1. ")

        if proportion != self._gaugeproportion:
            self._gaugeproportion = proportion

        
    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{ProgressGauge}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{ProgressGauge}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.BufferedPaintDC(self)
        dc.SetBackground(self._background)
        
        dc.Clear()

        xsize, ysize = self.GetClientSize()
        interval = xsize/float(self._steps)

        self._pos = interval*self._value
        
        status = self._current/(self._steps - int(self._gaugeproportion*xsize)/int(interval))
        
        if status%2 == 0:
            increment = 1
        else:
            increment = -1
            
        self._value = self._value + increment
        self._current = self._current + 1
        self.DrawProgress(dc, xsize, ysize, increment)
        

    def DrawProgress(self, dc, xsize, ysize, increment):
        """
        Actually draws the sliding bar.

        :param `dc`: an instance of `wx.DC`;
        :param `xsize`: the width of the whole progress bar;
        :param `ysize`: the height of the whole progress bar;
        :param `increment`: a positive value if we are spinning from left to right,
         a negative one if we are spinning from right to left.
        """

        if increment > 0:
            col1 = self.GetFirstGradientColour()
            col2 = self.GetSecondGradientColour()
        else:
            col1 = self.GetSecondGradientColour()
            col2 = self.GetFirstGradientColour()

        interval = self._gaugeproportion*xsize

        r1, g1, b1 = int(col1.Red()), int(col1.Green()), int(col1.Blue())
        r2, g2, b2 = int(col2.Red()), int(col2.Green()), int(col2.Blue())

        rstep = float((r2 - r1)) / interval
        gstep = float((g2 - g1)) / interval
        bstep = float((b2 - b1)) / interval

        rf, gf, bf = 0, 0, 0
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
            
        for ii in xrange(int(self._pos), int(self._pos+interval)):
            currCol = (r1 + rf, g1 + gf, b1 + bf)                
            dc.SetPen(wx.Pen(currCol, 2))
            dc.DrawLine(ii, 1, ii, ysize-2)
            rf = rf + rstep
            gf = gf + gstep
            bf = bf + bstep
        

    def Update(self):
        """ Updates the gauge with a new value. """

        self.Refresh()
        
        
# ---------------------------------------------------------------------------- #
# Class PyProgress
# ---------------------------------------------------------------------------- #

class PyProgress(wx.Dialog):
    """
    PyProgress is similar to `wx.ProgressDialog` in indeterminated mode, but with a
    different gauge appearance and a different spinning behavior. The moving gauge
    can be drawn with a single solid colour or with a shading gradient foreground.
    The gauge background colour is user customizable.
    The bar does not move always from the beginning to the end as in `wx.ProgressDialog`
    in indeterminated mode, but spins cyclically forward and backward.
    """

    def __init__(self, parent=None, id=-1, title="", message="",
                 agwStyle=wx.PD_APP_MODAL|wx.PD_AUTO_HIDE):
        """
        Default class constructor.

        :param `parent`: parent window;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `title`: dialog title to show in titlebar;
        :param `message`: message displayed above the progress bar;
        :param `style`: the dialog style. This can be a combination of the following bits:

         =================== =========== ==================================================
         Window Styles       Hex Value   Description
         =================== =========== ==================================================
         ``PD_CAN_ABORT``            0x1 This flag tells the dialog that it should have a ``Cancel`` button which the user may press. If this happens, the next call to `UpdatePulse` will return ``False``.
         ``PD_APP_MODAL``            0x2 Make the progress dialog modal. If this flag is not given, it is only 'locally' modal - that is the input to the parent window is disabled, but not to the other ones.
         ``PD_AUTO_HIDE``            0x4 Causes the progress dialog to disappear from screen as soon as the maximum value of the progress meter has been reached.
         ``PD_ELAPSED_TIME``         0x8 This flag tells the dialog that it should show elapsed time (since creating the dialog).
         =================== =========== ==================================================
         
        """

        wx.Dialog.__init__(self, parent, id, title)
        
        self._delay = 3
        self._hasAbortButton = False

        # we may disappear at any moment, let the others know about it
        self.SetExtraStyle(self.GetExtraStyle()|wx.WS_EX_TRANSIENT)

        self._hasAbortButton = (agwStyle & wx.PD_CAN_ABORT)

        if wx.Platform == "__WXMSW__":
            # we have to remove the "Close" button from the title bar then as it is
            # confusing to have it - it doesn't work anyhow
            # FIXME: should probably have a (extended?) window style for this
            if not self._hasAbortButton:
                self.EnableClose(False)
    
        self._state = (self._hasAbortButton and [Continue] or [Uncancelable])[0]
        self._parentTop = wx.GetTopLevelParent(parent)

        dc = wx.ClientDC(self)
        dc.SetFont(wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT))
        widthText, dummy = dc.GetTextExtent(message)

        sizer = wx.BoxSizer(wx.VERTICAL)

        self._msg = wx.StaticText(self, wx.ID_ANY, message)
        sizer.Add(self._msg, 0, wx.LEFT|wx.TOP, 2*LAYOUT_MARGIN)

        sizeDlg = wx.Size()
        sizeLabel = self._msg.GetSize()
        sizeDlg.y = 2*LAYOUT_MARGIN + sizeLabel.y

        self._gauge = ProgressGauge(self, -1)

        sizer.Add(self._gauge, 0, wx.LEFT|wx.RIGHT|wx.TOP|wx.EXPAND, 2*LAYOUT_MARGIN)

        sizeGauge = self._gauge.GetSize()
        sizeDlg.y += 2*LAYOUT_MARGIN + sizeGauge.y
        
        # create the estimated/remaining/total time zones if requested
        self._elapsed = None
        self._display_estimated = self._last_timeupdate = self._break = 0
        self._ctdelay = 0

        label = None

        nTimeLabels = 0

        if agwStyle & wx.PD_ELAPSED_TIME:
        
            nTimeLabels += 1
            self._elapsed = self.CreateLabel("Elapsed time : ", sizer)
        
        if nTimeLabels > 0:

            label = wx.StaticText(self, -1, "")    
            # set it to the current time
            self._timeStart = wx.GetCurrentTime()
            sizeDlg.y += nTimeLabels*(label.GetSize().y + LAYOUT_MARGIN)
            label.Destroy()

        sizeDlgModified = False
        
        if wx.Platform == "__WXMSW__":
            sizerFlags = wx.ALIGN_RIGHT|wx.ALL
        else:
            sizerFlags = wx.ALIGN_CENTER_HORIZONTAL|wx.BOTTOM|wx.TOP

        if self._hasAbortButton:
            buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
    
            self._btnAbort = wx.Button(self, -1, "Cancel")
            self._btnAbort.Bind(wx.EVT_BUTTON, self.OnCancel)
            
            # Windows dialogs usually have buttons in the lower right corner
            buttonSizer.Add(self._btnAbort, 0, sizerFlags, LAYOUT_MARGIN)

            if not sizeDlgModified:
                sizeDlg.y += 2*LAYOUT_MARGIN + wx.Button.GetDefaultSize().y

        if self._hasAbortButton:
            sizer.Add(buttonSizer, 0, sizerFlags, LAYOUT_MARGIN )

        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy)
        
        self._agwStyle = agwStyle
        
        self.SetSizerAndFit(sizer)
    
        sizeDlg.y += 2*LAYOUT_MARGIN

        # try to make the dialog not square but rectangular of reasonable width
        sizeDlg.x = max(widthText, 4*sizeDlg.y/3)
        sizeDlg.x *= 3
        sizeDlg.x /= 2
        self.SetClientSize(sizeDlg)
    
        self.Centre(wx.CENTER_FRAME|wx.BOTH)

        if agwStyle & wx.PD_APP_MODAL:
            self._winDisabler = wx.WindowDisabler(self)
        else:
            if self._parentTop:
                self._parentTop.Disable()
            self._winDisabler = None
    
        self.ShowDialog()
        self.Enable()

        # this one can be initialized even if the others are unknown for now
        # NB: do it after calling Layout() to keep the labels correctly aligned
        if self._elapsed:
            self.SetTimeLabel(0, self._elapsed)

        if not wx.EventLoop().GetActive():
            self.evtloop = wx.EventLoop()
            wx.EventLoop.SetActive(self.evtloop)
        
        self.Update()


    def CreateLabel(self, text, sizer):
        """
        Creates the `wx.StaticText` that holds the elapsed time label.

        :param `text`: the dialog message to be displayed above the gauge;
        :param `sizer`: the main `wx.BoxSizer` for L{PyProgress}.
        """

        locsizer = wx.BoxSizer(wx.HORIZONTAL)
        dummy = wx.StaticText(self, wx.ID_ANY, text)
        label = wx.StaticText(self, wx.ID_ANY, "unknown")

        if wx.Platform in ["__WXMSW__", "__WXMAC__"]:
            # label and time centered in one row
            locsizer.Add(dummy, 1, wx.ALIGN_LEFT)
            locsizer.Add(label, 1, wx.ALIGN_LEFT|wx.LEFT, LAYOUT_MARGIN)
            sizer.Add(locsizer, 0, wx.ALIGN_CENTER_HORIZONTAL|wx.TOP, LAYOUT_MARGIN)
        else:
            # label and time to the right in one row
            sizer.Add(locsizer, 0, wx.ALIGN_RIGHT|wx.RIGHT|wx.TOP, LAYOUT_MARGIN)
            locsizer.Add(dummy)
            locsizer.Add(label, 0, wx.LEFT, LAYOUT_MARGIN)

        return label


    # ----------------------------------------------------------------------------
    # wxProgressDialog operations
    # ----------------------------------------------------------------------------

    def UpdatePulse(self, newmsg=""):
        """
        Updates the dialog, setting the progress bar to the new value and, if given
        changes the message above it. Returns ``True`` unless the ``Cancel`` button
        has been pressed.

        If ``False`` is returned, the application can either immediately destroy the
        dialog or ask the user for the confirmation.

        :param `newmsg`: The new messages for the progress dialog text, if it is
         empty (which is the default) the message is not changed.
        """
       
        self._gauge.Update()
        
        if newmsg and newmsg != self._msg.GetLabel():
            self._msg.SetLabel(newmsg)
            wx.YieldIfNeeded() 
        
        if self._elapsed:        
            elapsed = wx.GetCurrentTime() - self._timeStart
            if self._last_timeupdate < elapsed:
                self._last_timeupdate = elapsed
                
            self.SetTimeLabel(elapsed, self._elapsed)                            

        if self._state == Finished:

            if not self._agwStyle & wx.PD_AUTO_HIDE:
                
                self.EnableClose()
                
                if newmsg == "":
                    # also provide the finishing message if the application didn't
                    self._msg.SetLabel("Done.")
                
                wx.YieldIfNeeded()
                self.ShowModal()
                return False
            
            else:
                # reenable other windows before hiding this one because otherwise
                # Windows wouldn't give the focus back to the window which had
                # been previously focused because it would still be disabled
                self.ReenableOtherWindows()
                self.Hide()
            
        # we have to yield because not only we want to update the display but
        # also to process the clicks on the cancel and skip buttons
        wx.YieldIfNeeded()

        return self._state != Canceled


    def GetFirstGradientColour(self):
        """ Returns the gauge first gradient colour. """

        return self._gauge.GetFirstGradientColour()


    def SetFirstGradientColour(self, colour):
        """
        Sets the gauge first gradient colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._gauge.SetFirstGradientColour(colour)


    def GetSecondGradientColour(self):
        """ Returns the gauge second gradient colour. """

        return self._gauge.GetSecondGradientColour()


    def SetSecondGradientColour(self, colour):
        """
        Sets the gauge second gradient colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._gauge.SetSecondGradientColour(colour)
        

    def GetGaugeBackground(self):
        """ Returns the gauge background colour. """

        return self._gauge.GetGaugeBackground()


    def SetGaugeBackground(self, colour):
        """
        Sets the gauge background colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._gauge.SetGaugeBackground(colour)


    def SetGaugeSteps(self, steps):
        """
        Sets the number of steps the gauge performs before switching from
        forward to backward (or vice-versa) movement.

        :param `steps`: the number of steps the gauge performs before switching from
         forward to backward (or vice-versa) movement.
        """
        
        self._gauge.SetGaugeSteps(steps)


    def GetGaugeSteps(self):
        """
        Returns the number of steps the gauge performs before switching from
        forward to backward (or vice-versa) movement.
        """

        return self._gauge.GetGaugeSteps()


    def GetGaugeProportion(self):
        """
        Returns the relative proportion between the sliding bar and the
        whole gauge.
        """
        
        return self._gauge.GetGaugeProportion()

    
    def SetGaugeProportion(self, proportion):
        """
        Sets the relative proportion between the sliding bar and the
        whole gauge.

        :param `proportion`: a floating point number representing the relative
         proportion between the sliding bar and the whole gauge.
        """

        self._gauge.SetGaugeProportion(proportion)


    def ShowDialog(self, show=True):
        """
        Show the dialog.

        :param `show`: ``True`` to show the dialog and re-enable all the other windows,
         ``False`` otherwise.
        """

        # reenable other windows before hiding this one because otherwise
        # Windows wouldn't give the focus back to the window which had
        # been previously focused because it would still be disabled
        if not show:
            self.ReenableOtherWindows()

        return self.Show()


    # ----------------------------------------------------------------------------
    # event handlers
    # ----------------------------------------------------------------------------

    def OnCancel(self, event):
        """
        Handles the ``wx.EVT_BUTTON`` event for the dialog.

        :param `event`: a `wx.CommandEvent` event to be processed.

        :note: This method handles the ``Cancel`` button press. 
        """

        if self._state == Finished:
        
            # this means that the count down is already finished and we're being
            # shown as a modal dialog - so just let the default handler do the job
            event.Skip()
        
        else:
        
            # request to cancel was received, the next time Update() is called we
            # will handle it
            self._state = Canceled

            # update the buttons state immediately so that the user knows that the
            # request has been noticed
            self.DisableAbort()

            # save the time when the dialog was stopped
            self._timeStop = wx.GetCurrentTime()

        self.ReenableOtherWindows()


    def OnDestroy(self, event):
        """
        Handles the ``wx.EVT_WINDOW_DESTROY`` event for L{PyProgress}.

        :param `event`: a `wx.WindowDestroyEvent` event to be processed.
        """
        
        self.ReenableOtherWindows()
        event.Skip()

        
    def OnClose(self, event):
        """
        Handles the ``wx.EVT_CLOSE`` event for L{PyProgress}.

        :param `event`: a `wx.CloseEvent` event to be processed.
        """

        if self._state == Uncancelable:
        
            # can't close this dialog
            event.Veto()
        
        elif self._state == Finished:

            # let the default handler close the window as we already terminated
            self.Hide()
            event.Skip()
        
        else:
        
            # next Update() will notice it
            self._state = Canceled
            self.DisableAbort()
    
            self._timeStop = wx.GetCurrentTime()
    

    def ReenableOtherWindows(self):
        """ Re-enables the other windows if using `wx.WindowDisabler`. """

        if self._agwStyle & wx.PD_APP_MODAL:
            if hasattr(self, "_winDisabler"):
                del self._winDisabler
        
        else:
        
            if self._parentTop:
                self._parentTop.Enable()
    

    def SetTimeLabel(self, val, label=None):
        """
        Sets the elapsed time label.

        :param `val`: the elapsed time since the dialog was shown, in seconds;
        :param `label`: the new message to display in the elapsed time text.
        """

        if label:
        
            hours = val/3600
            minutes = (val%3600)/60
            seconds = val%60
            strs = ("%lu:%02lu:%02lu")%(hours, minutes, seconds)

            if strs != label.GetLabel():
                label.SetLabel(strs)

    
    def EnableAbort(self, enable=True):
        """
        Enables or disables the ``Cancel`` button.

        :param `enable`: ``True`` to enable the ``Cancel`` button, ``False`` to disable it.
        """

        if self._hasAbortButton:
            if self._btnAbort:
                self._btnAbort.Enable(enable)


    def EnableClose(self, enable=True):
        """
        Enables or disables the ``Close`` button.

        :param `enable`: ``True`` to enable the ``Close`` button, ``False`` to disable it.
        """
        
        if self._hasAbortButton:
            if self._btnAbort:        
                self._btnAbort.Enable(enable)
                self._btnAbort.SetLabel("Close")
                self._btnAbort.Bind(wx.EVT_BUTTON, self.OnClose)


    def DisableAbort(self):
        """ Disables the ``Cancel`` button. """

        self.EnableAbort(False)

