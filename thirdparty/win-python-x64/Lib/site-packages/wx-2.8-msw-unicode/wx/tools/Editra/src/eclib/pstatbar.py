###############################################################################
# Name: pstatbar.py                                                           #
# Purpose: Custom statusbar with builtin progress indicator                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: ProgressStatusBar

Custom StatusBar that has a builtin progress gauge to indicate busy status and
progress of long running tasks in a window.

The Progress Gauge is only shown when it is active. When shown it is shown in 
the far rightmost field of the StatusBar. The size of the progress Guage is 
also determined by the size of the right most field.When created the StatusBar 
will creates two fields by default, field 0 is expanding, field 1 is set as a
small fixed field on the right. To change this behavior simply call SetFields 
after creating the bar to change it.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: pstatbar.py 66840 2011-02-03 21:05:28Z CJP $"
__revision__ = "$Revision: 66840 $"

__all__ = ["ProgressStatusBar",]

#--------------------------------------------------------------------------#
# Dependancies
import wx

#--------------------------------------------------------------------------#
# Globals

#--------------------------------------------------------------------------#
class ProgressStatusBar(wx.StatusBar):
    """Custom StatusBar with a built-in progress bar"""
    def __init__(self, parent, id_=wx.ID_ANY,
                 style=wx.SB_FLAT,
                 name="ProgressStatusBar"):
        """Creates a status bar that can hide and show a progressbar
        in the far right section. The size of the progressbar is also
        determined by the size of the right most section.
        @param parent: Frame this status bar belongs to

        """
        super(ProgressStatusBar, self).__init__(parent, id_, style, name)
  
        # Attributes
        self._changed = False   # position has changed ?
        self.busy = False       # Bar in busy mode ?
        self.stop = False       # Stop flag to stop progress from other threads
        self.progress = 0       # Current progress value of the bar
        self.range = 0          # Range of progress indicator
        self.tmp = None         # Temp for text that may be pushed when busy
        self.timer = wx.Timer(self)
        self.prog = wx.Gauge(self, style=wx.GA_HORIZONTAL)
        self.prog.Hide()

        # Layout
        self.SetFieldsCount(2)
        self.SetStatusWidths([-1, 155])

        # Event Handlers
        self.Bind(wx.EVT_IDLE, lambda evt: self.__Reposition())
        self.Bind(wx.EVT_TIMER, self.OnTimer)
        self.Bind(wx.EVT_SIZE, self.OnSize)

    def __del__(self):
        """Make sure the timer is stopped
        @postcondition: timer is cleaned up

        """
        if self.timer.IsRunning():
            self.timer.Stop()

    def __Reposition(self):
        """Does the actual repositioning of progress bar
        @postcondition: Progress bar is repostioned inside right most field

        """
        if self._changed:
            rect = self.GetFieldRect(self.GetFieldsCount() - 1)
            self.prog.SetPosition((rect.x + 2, rect.y + 2))
            self.prog.SetSize((rect.width - 8, rect.height - 4))
        self._changed = False

    def _UpdateRange(self, range):
        """Update the internal progress gauges range
        @param range: int

        """
        self.range = range
        try:
            self.prog.SetRange(range)
        except OverflowError:
            # range too large, scale everything to 100
            self.prog.SetRange(100)

    def _UpdateValue(self, value):
        """Update the internal progress gauges value
        @param range: int

        """
        # Ensure value is within range
        range = self.prog.GetRange()
        if range != self.range: # need to scale value
            value = int((float(value) / float(range)) * 100)
        self.progress = value
        self.prog.SetValue(value)

    #---- Public Methods ----#

    def Destroy(self):
        """Destroy the control"""
        if self.timer.IsRunning():
            self.timer.Stop()
        del self.timer
        super(ProgressStatusBar, self).Destroy()

    def DoStop(self):
        """Stop any progress indication action and hide the bar"""
        self.timer.Stop()
        self.ShowProgress(False)
        self.prog.SetValue(0)   # Reset progress value
        self.busy = False
        self.stop = False

        # Restore any status text that was sent while busy
        if self.tmp is not None:
            self.SetStatusText(self.tmp, self.GetFieldsCount() - 1)
            self.tmp = None

    def GetGauge(self):
        """Return the wx.Gauge used by this window
        @return: wx.Gauge

        """
        return self.prog

    def GetProgress(self):
        """Get the progress of the progress bar
        @return: int

        """
        return self.prog.GetValue()

    def GetRange(self):
        """Get the what the range of the progress bar is
        @return: int

        """
        return self.prog.GetRange()

    def IsBusy(self):
        """Is the progress indicator busy or not
        @return: bool

        """
        return self.timer.IsRunning()

    def OnSize(self, evt):
        """Reposition progress bar on resize
        @param evt: wx.EVT_SIZE

        """
        self.__Reposition()
        self._changed = True
        evt.Skip()

    def OnTimer(self, evt):
        """Update the progress bar while the timer is running
        @param evt: wx.EVT_TIMER

        """
        # Check stop flag that can be set from non main thread
        if self.stop:
            self.DoStop()
            return

        if not self.prog.IsShown():
            self.Stop()

        if self.busy or self.progress < 0:
            self.prog.Pulse()
        else:
            # Update the Range if it has changed
            if self.range >= 0 and self.range != self.prog.GetRange():
                self._UpdateRange(self.range)

            # Update the progress value if it is less than the range
            if self.progress <= self.range:
                self._UpdateValue(self.progress)

    def Run(self, rate=100):
        """Start the bar's timer to check for updates to progress
        @keyword rate: rate at which to check for updates in msec

        """
        if not self.timer.IsRunning():
            self.timer.Start(rate)

    def SetProgress(self, val):
        """Set the controls internal progress value that is reflected in the
        progress bar when the timer next updates. Be sure to call Start before
        calling this method if you want the changes to be visible. This method
        can be called from non gui threads.
        @param val: int

        """
        self.progress = val
        if val > 0 and wx.Thread_IsMain():
            self._UpdateValue(val)

    def SetRange(self, val):
        """Set the what the range of the progress bar is. This method can safely
        be called from non gui threads.
        @param val: int

        """
        self.range = val
        if val > 0 and wx.Thread_IsMain():
            self._UpdateRange(val)

    def ShowProgress(self, show=True):
        """Manually show or hide the progress bar
        @keyword show: bool

        """
        # If showing make sure bar is positioned properly
        if show:
            self.__Reposition()
        self.prog.Show(show)
        wx.GetApp().ProcessPendingEvents()

    def SetStatusText(self, txt, number=0):
        """Override wx.StatusBar method to prevent text from being
        put in when the progress indicator is running. Any text that
        comes when it is running is buffered to be displayed afterwords.
        @param txt: Text to put on status bar
        @keyword number: Section number to put text in

        """
        if number == self.GetFieldsCount() - 1 and self.IsBusy():
            if self.tmp is None:
                self.tmp = txt
        else:
            try:
                super(ProgressStatusBar, self).SetStatusText(txt, number)
            except wx.PyAssertionError:
                pass

    # Alias for SetStatusText
    PushStatusText = SetStatusText

    def Start(self, rate=100):
        """Show and the progress indicator and start the timer
        @keyword rate: rate to update progress bar in msec

        """
        self.__Reposition()
        bfield = self.GetFieldsCount() - 1
        self.tmp = self.GetStatusText(bfield)
        # Clear the progress field so the text doesn't show behind
        # the progress indicator.
        super(ProgressStatusBar, self).SetStatusText(u'', bfield)
        self.stop = False
        self.ShowProgress(True)
        self.Run(rate)

    def StartBusy(self, rate=100):
        """Show and start the progress indicator in pulse mode
        @keyword rate: interval to pulse indicator at in msec

        """
        self.busy = True
        self.Start(rate)

    def Stop(self):
        """Stop and hide the progress bar. This method may safely be called
        from background threads.
        @precondition: Bar is already running

        """
        if wx.Thread_IsMain():
            self.DoStop()
        else:
            self.stop = True # Set flag from non main thread
        self.progress = 0

    def StopBusy(self):
        """Stop and hide the progress indicator
        @postcondition: Progress bar is hidden from view

        """
        self.busy = False
        self.Stop()
