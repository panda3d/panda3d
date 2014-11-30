###############################################################################
# Name: outbuff.py                                                            #
# Purpose: Gui and helper classes for running processes and displaying output #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: OutputBuffer

This module contains classes that are useful for displaying output from running
tasks and processes. The classes are divided into three main categories, gui
classes, mixins, and thread classes. All the classes can be used together to
easily create multithreaded gui display classes without needing to worry about
the details and thread safety of the gui.

For example usage of these classes see ed_log and the Editra's Launch plugin

Class OutputBuffer:
This is the main class exported by this module. It provides a readonly output
display buffer that when used with the other classes in this module provides an
easy way to display continuous output from other processes and threads. It
provides two methods for subclasses to override if they wish to perform custom
handling.

  - Override the ApplyStyles method to do any processing and coloring of the
    text as it is put in the buffer.
  - Override the DoHotSpotClicked method to handle any actions to take when a
    hotspot has been clicked in the buffer.
  - Override the DoUpdatesEmpty method to perform any idle processing when no
    new text is waiting to be processed.

Class ProcessBufferMixin:
Mixin class for the L{OutputBuffer} class that provides handling for when an
OutputBuffer is used with a L{ProcessThread}. It provides three methods that can
be overridden in subclasses to perform extra processing.

  - DoProcessStart: Called as the process is being started in the ProcessThread,
                    it receives the process command string as an argument.
  - DoFilterInput: Called as each chunk of output comes from the running process
                   use it to filter the results before displaying them in the
                   buffer.
  - DoProcessExit: Called when the running process has exited. It receives the
                   processes exit code as a parameter.

Class ProcessThread:
Thread class for running subprocesses and posting the output to an
L{OutputBuffer} via events.

Class TaskThread:
Thread class for running a callable. For optimal performance and responsiveness
the callable should be a generator object. All results are directed to an
L{OutputBuffer} through its AppendUpdate method.

Requirements:
  * wxPython 2.8
  * Macintosh/Linux/Unix Python 2.4+
  * Windows Python 2.5+ (ctypes is needed)

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: outbuff.py 67636 2011-04-28 00:32:43Z CJP $"
__revision__ = "$Revision: 67636 $"

__all__ = ["OutputBuffer", "OutputBufferEvent", "ProcessBufferMixin",
           "ProcessThreadBase", "ProcessThread", "TaskThread", "TaskObject",
           "OPB_STYLE_DEFAULT", "OPB_STYLE_INFO",
           "OPB_STYLE_WARN", "OPB_STYLE_ERROR", "OPB_STYLE_MAX",

           "OPB_ERROR_NONE", "OPB_ERROR_INVALID_COMMAND",

           "edEVT_PROCESS_START", "EVT_PROCESS_START", "edEVT_TASK_START",
           "EVT_TASK_START", "edEVT_UPDATE_TEXT", "EVT_UPDATE_TEXT",
           "edEVT_PROCESS_EXIT", "EVT_PROCESS_EXIT", "edEVT_TASK_COMPLETE",
           "EVT_TASK_COMPLETE", "edEVT_PROCESS_ERROR", "EVT_PROCESS_ERROR"]

#--------------------------------------------------------------------------#
# Imports
import os
import sys
import time
import errno
import signal
import threading
import types
import subprocess
import wx
import wx.stc

# Platform specific modules needed for killing processes
if subprocess.mswindows:
    import msvcrt
    import ctypes
else:
    import shlex
    import select
    import fcntl

#--------------------------------------------------------------------------#
# Globals
OUTPUTBUFF_NAME_STR = u'EditraOutputBuffer'
THREADEDBUFF_NAME_STR = u'EditraThreadedBuffer'

# Style Codes
OPB_STYLE_DEFAULT = 0  # Default Black text styling
OPB_STYLE_INFO    = 1  # Default Blue text styling
OPB_STYLE_WARN    = 2  # Default Red text styling
OPB_STYLE_ERROR   = 3  # Default Red/Hotspot text styling
OPB_STYLE_MAX     = 3  # Highest style byte used by outputbuffer

# All Styles
OPB_ALL_STYLES = (wx.stc.STC_STYLE_DEFAULT, wx.stc.STC_STYLE_CONTROLCHAR,
                  OPB_STYLE_DEFAULT, OPB_STYLE_ERROR, OPB_STYLE_INFO,
                  OPB_STYLE_WARN)

# Error Codes
OPB_ERROR_NONE            = 0
OPB_ERROR_INVALID_COMMAND = -1

#--------------------------------------------------------------------------#

# Event for notifying that the process has started running
# GetValue will return the command line string that started the process
edEVT_PROCESS_START = wx.NewEventType()
EVT_PROCESS_START = wx.PyEventBinder(edEVT_PROCESS_START, 1)

# Event for notifying that a task is starting to run
edEVT_TASK_START = wx.NewEventType()
EVT_TASK_START = wx.PyEventBinder(edEVT_TASK_START, 1)

# Event for passing output data to buffer
# GetValue returns the output text retrieved from the process
edEVT_UPDATE_TEXT = wx.NewEventType()
EVT_UPDATE_TEXT = wx.PyEventBinder(edEVT_UPDATE_TEXT, 1)

# Event for notifying that the the process has finished and no more update
# events will be sent. GetValue will return the processes exit code
edEVT_PROCESS_EXIT = wx.NewEventType()
EVT_PROCESS_EXIT = wx.PyEventBinder(edEVT_PROCESS_EXIT, 1)

# Event to notify that a process has completed
edEVT_TASK_COMPLETE = wx.NewEventType()
EVT_TASK_COMPLETE = wx.PyEventBinder(edEVT_TASK_COMPLETE, 1)

# Event to notify that an error occurred in the process
edEVT_PROCESS_ERROR = wx.NewEventType()
EVT_PROCESS_ERROR = wx.PyEventBinder(edEVT_PROCESS_ERROR, 1)

class OutputBufferEvent(wx.PyCommandEvent):
    """Event for data transfer and signaling actions in the L{OutputBuffer}"""
    def __init__(self, etype, eid=wx.ID_ANY, value=''):
        """Creates the event object"""
        super(OutputBufferEvent, self).__init__(etype, eid)

        # Attributes
        self._value = value
        self._errmsg = None

    def GetValue(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._value

    def GetErrorMessage(self):
        """Get the error message value
        @return: Exception traceback string or None

        """
        return self._errmsg

    def SetErrorMessage(self, msg):
        """Set the error message value
        @param msg: Exception traceback string

        """
        try:
            tmsg = unicode(msg)
        except:
            tmsg = None
        self._errmsg = msg

#--------------------------------------------------------------------------#

class OutputBuffer(wx.stc.StyledTextCtrl):
    """OutputBuffer is a general purpose output display for showing text. It
    provides an easy interface for the buffer to interact with multiple threads
    that may all be sending updates to the buffer at the same time. Methods for
    styleing and filtering output are also available.

    """
    def __init__(self, parent, id=wx.ID_ANY,
                 pos=wx.DefaultPosition,
                 size=wx.DefaultSize,
                 style=wx.BORDER_SUNKEN,
                 name=OUTPUTBUFF_NAME_STR):
        super(OutputBuffer, self).__init__(parent, id, pos,
                                           size, style, name)

        # Attributes
        self._mutex = threading.Lock()
        self._updating = threading.Condition(self._mutex)
        self._updates = list()
        self._timer = wx.Timer(self)
        self._line_buffer = -1                
        self._colors = dict(defaultb=(255, 255, 255), defaultf=(0, 0, 0),
                            errorb=(255, 255, 255), errorf=(255, 0, 0),
                            infob=(255, 255, 255), infof=(0, 0, 255),
                            warnb=(255, 255, 255), warnf=(255, 0, 0))

        # Setup
        self.__ConfigureSTC()

        # Event Handlers
        self.Bind(wx.EVT_TIMER, self.OnTimer)
        self.Bind(wx.stc.EVT_STC_HOTSPOT_CLICK, self._OnHotSpot)

    def __del__(self):
        """Ensure timer is cleaned up when we are deleted"""
        if self._timer.IsRunning():
            self._timer.Stop()

    def __ConfigureSTC(self):
        """Setup the stc to behave/appear as we want it to
        and define all styles used for giving the output context.
        @todo: make more of this configurable

        """
        self.SetMargins(3, 3)
        self.SetMarginWidth(0, 0)
        self.SetMarginWidth(1, 0)

        # To improve performance at cost of memory cache the document layout
        self.SetLayoutCache(wx.stc.STC_CACHE_DOCUMENT)
        self.SetUndoCollection(False) # Don't keep undo history
        self.SetReadOnly(True)
        self.SetCaretWidth(0)

        if wx.Platform == '__WXMSW__':
            self.SetEOLMode(wx.stc.STC_EOL_CRLF)
        else:
            self.SetEOLMode(wx.stc.STC_EOL_LF)

        #self.SetEndAtLastLine(False)
        self.SetVisiblePolicy(1, wx.stc.STC_VISIBLE_STRICT)

        # Define Styles
        highlight = wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT)
        self.SetSelBackground(True, highlight)
        self.__SetupStyles()

    def FlushBuffer(self):
        """Flush the update buffer
        @postcondition: The update buffer is empty

        """
        self._updating.acquire()
        self.SetReadOnly(False)
        txt = u''.join(self._updates[:])
        start = self.GetLength()
        if u'\0' in txt:
            # HACK: handle displaying NULLs in the STC
            self.AddStyledText('\0'.join(txt.encode('utf-8'))+'\0')
        else:
            self.AppendText(txt)
        self.GotoPos(self.GetLength())
        self._updates = list()
        self.ApplyStyles(start, txt)
        self.SetReadOnly(True)
        self._updating.release()

    def __SetupStyles(self, font=None):
        """Setup the default styles of the text in the buffer
        @keyword font: wx.Font to use or None to use default

        """
        if font is None:
            if wx.Platform == '__WXMAC__':
                fsize = 11
            else:
                fsize = 10

            font = wx.Font(fsize, wx.FONTFAMILY_MODERN,
                           wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL)
        style = (font.GetFaceName(), font.GetPointSize(), "#FFFFFF")
        wx.stc.StyledTextCtrl.SetFont(self, font)

        # Custom Styles
        self.StyleSetSpec(OPB_STYLE_DEFAULT,
                          "face:%s,size:%d,fore:#000000,back:%s" % style)
        self.StyleSetSpec(OPB_STYLE_INFO,
                          "face:%s,size:%d,fore:#0000FF,back:%s" % style)
        self.StyleSetSpec(OPB_STYLE_WARN,
                          "face:%s,size:%d,fore:#FF0000,back:%s" % style)
        self.StyleSetSpec(OPB_STYLE_ERROR,
                          "face:%s,size:%d,fore:#FF0000,back:%s" % style)
        self.StyleSetHotSpot(OPB_STYLE_ERROR, True)

        # Default Styles
        self.StyleSetSpec(wx.stc.STC_STYLE_DEFAULT, \
                          "face:%s,size:%d,fore:#000000,back:%s" % style)
        self.StyleSetSpec(wx.stc.STC_STYLE_CONTROLCHAR, \
                          "face:%s,size:%d,fore:#000000,back:%s" % style)
        self.Colourise(0, -1)

    def _OnHotSpot(self, evt):
        """Handle hotspot clicks"""
        pos = evt.GetPosition()
        self.DoHotSpotClicked(pos, self.LineFromPosition(pos))

    #---- Public Member Functions ----#

    def AppendUpdate(self, value):
        """Buffer output before adding to window. This method can safely be
        called from non gui threads to add updates to the buffer, that will
        be displayed during the next idle period.
        @param value: update string to append to stack

        """
        self._updating.acquire()
        if not (type(value) is types.UnicodeType):
            value = value.decode(sys.getfilesystemencoding())
        self._updates.append(value)
        self._updating.release()

    def ApplyStyles(self, start, txt):
        """Apply coloring to text starting at start position.
        Override this function to do perform any styling that you want
        done on the text.
        @param start: Start position of text that needs styling in the buffer
        @param txt: The string of text that starts at the start position in the
                    buffer.

        """
        pass

    def CanCopy(self):
        """Is it possible to copy text right now
        @return: bool

        """
        sel = self.GetSelection()
        return sel[0] != sel[1]

    def CanCut(self):
        """Is it possible to Cut
        @return: bool

        """
        return not self.GetReadOnly()

    def Clear(self):
        """Clear the Buffer"""
        self.SetReadOnly(False)
        self.ClearAll()
        self.EmptyUndoBuffer()
        self.SetReadOnly(True)

    def DoHotSpotClicked(self, pos, line):
        """Action to perform when a hotspot region is clicked in the buffer.
        Override this function to provide handling of hotspots.
        @param pos: Position in buffer of where the click occurred.
        @param line: Line in which the click occurred (zero based index)

        """
        pass

    def DoUpdatesEmpty(self):
        """Called when update stack is empty
        Override this function to perform actions when there are no updates
        to process. It can be used for things such as temporarily stopping
        the timer or performing idle processing.

        """
        pass

    def GetDefaultBackground(self):
        """Get the default text style background color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['defaultb'])

    def GetDefaultForeground(self):
        """Get the default text style foreground color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['defaultf'])

    def GetErrorBackground(self):
        """Get the error text style background color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['errorb'])

    def GetErrorForeground(self):
        """Get the error text style foreground color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['errorf'])

    def GetInfoBackground(self):
        """Get the info text style background color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['infob'])

    def GetInfoForeground(self):
        """Get the info text style foreground color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['infof'])

    def GetWarningBackground(self):
        """Get the warning text style background color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['warnb'])

    def GetWarningForeground(self):
        """Get the warning text style foreground color
        @return: wx.Colour

        """
        return wx.Colour(*self._colors['warnf'])

    def GetUpdateQueue(self):
        """Gets a copy of the current update queue"""
        self._updating.acquire()
        val = list(self._updates)
        self._updating.release()
        return val

    def IsRunning(self):
        """Return whether the buffer is running and ready for output
        @return: bool

        """
        return self._timer.IsRunning()

    def OnTimer(self, evt):
        """Process and display text from the update buffer
        @note: this gets called many times while running thus needs to
               return quickly to avoid blocking the ui.

        """
        if len(self._updates):
            self.FlushBuffer()
        elif evt is not None:
            self.DoUpdatesEmpty()
        else:
            pass

    def RefreshBufferedLines(self):
        """Refresh and readjust the lines in the buffer to fit the current
        line buffering limits.
        @postcondition: Oldest lines are removed until we are back within the
                        buffer limit bounds.

        """
        if self._line_buffer < 0:
            return

        while self.GetLineCount() > self._line_buffer:
            self.SetCurrentPos(0)
            self.SetReadOnly(False)
            self.LineDelete()
            self.SetReadOnly(True)

        self.SetCurrentPos(self.GetLength())

    def SetDefaultColor(self, fore=None, back=None):
        """Set the colors for the default text style
        @keyword fore: Foreground Color
        @keyword back: Background Color

        """
        if fore is not None:
            self.StyleSetForeground(wx.stc.STC_STYLE_DEFAULT, fore)
            self.StyleSetForeground(wx.stc.STC_STYLE_CONTROLCHAR, fore)
            self.StyleSetForeground(OPB_STYLE_DEFAULT, fore)
            self._colors['defaultf'] = fore.Get()

        if back is not None:
            self.StyleSetBackground(wx.stc.STC_STYLE_DEFAULT, back)
            self.StyleSetBackground(wx.stc.STC_STYLE_CONTROLCHAR, back)
            self.StyleSetBackground(OPB_STYLE_DEFAULT, back)
            self._colors['defaultb'] = back.Get()

    def SetErrorColor(self, fore=None, back=None):
        """Set color for error text
        @keyword fore: Foreground Color
        @keyword back: Background Color

        """
        if fore is not None:
            self.StyleSetForeground(OPB_STYLE_ERROR, fore)
            self._colors['errorf'] = fore.Get()

        if back is not None:
            self.StyleSetBackground(OPB_STYLE_ERROR, back)
            self._colors['errorb'] = back.Get()

    def SetInfoColor(self, fore=None, back=None):
        """Set color for info text
        @keyword fore: Foreground Color
        @keyword back: Background Color

        """
        if fore is not None:
            self.StyleSetForeground(OPB_STYLE_INFO, fore)
            self._colors['infof'] = fore.Get()

        if back is not None:
            self.StyleSetBackground(OPB_STYLE_INFO, back)
            self._colors['infob'] = back.Get()

    def SetWarningColor(self, fore=None, back=None):
        """Set color for warning text
        @keyword fore: Foreground Color
        @keyword back: Background Color

        """
        if fore is not None:
            self.StyleSetForeground(OPB_STYLE_WARN, fore)
            self._colors['warnf'] = fore.Get()

        if back is not None:
            self.StyleSetBackground(OPB_STYLE_WARN, back)
            self._colors['warnb'] = back.Get()

    def SetFont(self, font):
        """Set the font used by all text in the buffer
        @param font: wxFont

        """
        for style in OPB_ALL_STYLES:
            self.StyleSetFont(style, font)

    def SetLineBuffering(self, num):
        """Set how many lines the buffer should keep for display.
        @param num: int (-1 == unlimited)

        """
        self._line_buffer = num
        self.RefreshBufferedLines()

    def SetText(self, text):
        """Set the text that is shown in the buffer
        @param text: text string to set as buffers current value

        """
        self.SetReadOnly(False)
        wx.stc.StyledTextCtrl.SetText(self, text)
        self.SetReadOnly(True)

    def Start(self, interval):
        """Start the window's timer to check for updates
        @param interval: interval in milliseconds to do updates

        """
        self._timer.Start(interval)

    def Stop(self):
        """Stop the update process of the buffer"""
        # Dump any output still left in tmp buffer before stopping
        self.OnTimer(None)
        self._timer.Stop()
        self.SetReadOnly(True)

#-----------------------------------------------------------------------------#

class ProcessBufferMixin:
    """Mixin class for L{OutputBuffer} to handle events
    generated by a L{ProcessThread}.

    """
    def __init__(self, update=100):
        """Initialize the mixin
        @keyword update: The update interval speed in msec

        """
        # Attributes
        self._rate = update

        # Event Handlers
        self.Bind(EVT_PROCESS_START, self._OnProcessStart)
        self.Bind(EVT_UPDATE_TEXT, self._OnProcessUpdate)
        self.Bind(EVT_PROCESS_EXIT, self._OnProcessExit)
        self.Bind(EVT_PROCESS_ERROR, self._OnProcessError)

    def _OnProcessError(self, evt):
        """Handle EVT_PROCESS_ERROR"""
        self.DoProcessError(evt.GetValue(), evt.GetErrorMessage())

    def _OnProcessExit(self, evt):
        """Handles EVT_PROCESS_EXIT"""
        self.DoProcessExit(evt.GetValue())

    def _OnProcessStart(self, evt):
        """Handles EVT_PROCESS_START"""
        self.DoProcessStart(evt.GetValue())
        self.Start(self._rate)

    def _OnProcessUpdate(self, evt):
        """Handles EVT_UPDATE_TEXT"""
        txt = self.DoFilterInput(evt.GetValue())
        self.AppendUpdate(txt)

    def DoFilterInput(self, txt):
        """Override this method to do an filtering on input that is sent to
        the buffer from the process text. The return text is what is put in
        the buffer.
        @param txt: incoming update text
        @return: string

        """
        return txt

    def DoProcessError(self, code, excdata=None):
        """Override this method to do any ui notification of when errors happen
        in running the process.
        @param code: an OBP error code
        @keyword excdata: Exception Data from process error
        @return: None

        """
        pass

    def DoProcessExit(self, code=0):
        """Override this method to do any post processing after the running
        task has exited. Typically this is a good place to call
        L{OutputBuffer.Stop} to stop the buffers timer.
        @keyword code: Exit code of program
        @return: None

        """
        self.Stop()

    def DoProcessStart(self, cmd=''):
        """Override this method to do any pre-processing before starting
        a processes output.
        @keyword cmd: Command used to start program
        @return: None

        """
        pass

    def SetUpdateInterval(self, value):
        """Set the rate at which the buffer outputs update messages. Set to
        a higher number if the process outputs large amounts of text at a very
        high rate.
        @param value: rate in milliseconds to do updates on

        """
        self._rate = value

#-----------------------------------------------------------------------------#

class ProcessThreadBase(threading.Thread):
    """Base Process Thread
    Override DoPopen in subclasses.

    """
    def __init__(self, parent):
        super(ProcessThreadBase, self).__init__()

        # Attributes
        self.abort = False          # Abort Process
        self._proc = None
        self._parent = parent       # Parent Window/Event Handler
        self._sig_abort = signal.SIGTERM    # default signal to kill process
        self._last_cmd = u""        # Last run command

    #---- Properties ----#
    LastCommand = property(lambda self: self._last_cmd,
                           lambda self, val: setattr(self, '_last_cmd', val)) 
    Parent = property(lambda self: self._parent)
    Process = property(lambda self: self._proc)

    def __DoOneRead(self):
        """Read one line of output and post results.
        @return: bool (True if more), (False if not)

        """
        if subprocess.mswindows:
            # Windows nonblocking pipe read implementation
            read = u''
            try:
                handle = msvcrt.get_osfhandle(self._proc.stdout.fileno())
                avail = ctypes.c_long()
                ctypes.windll.kernel32.PeekNamedPipe(handle, None, 0, 0,
                                                     ctypes.byref(avail), None)
                if avail.value > 0:
                    read = self._proc.stdout.read(avail.value)
                    if read.endswith(os.linesep):
                        read = read[:-1 * len(os.linesep)]
                else:
                    if self._proc.poll() is None:
                        time.sleep(1)
                        return True
                    else:
                        # Process has Exited
                        return False
            except ValueError, msg:
                return False
            except (subprocess.pywintypes.error, Exception), msg:
                if msg[0] in (109, errno.ESHUTDOWN):
                    return False
        else:
            # OSX and Unix nonblocking pipe read implementation
            if self._proc.stdout is None:
                return False

            flags = fcntl.fcntl(self._proc.stdout, fcntl.F_GETFL)
            if not self._proc.stdout.closed:
                fcntl.fcntl(self._proc.stdout,
                            fcntl.F_SETFL,
                            flags|os.O_NONBLOCK)

            try:
                try:
                    if not select.select([self._proc.stdout], [], [], 1)[0]:
                        return True

                    read = self._proc.stdout.read(4096)
                    if read == '':
                        return False
                except IOError, msg:
                    return False
            finally:
                if not self._proc.stdout.closed:
                    fcntl.fcntl(self._proc.stdout, fcntl.F_SETFL, flags)

        # Ignore encoding errors and return an empty line instead
        try:
            result = read.decode(sys.getfilesystemencoding())
        except UnicodeDecodeError:
            result = os.linesep

        if self.Parent:
            evt = OutputBufferEvent(edEVT_UPDATE_TEXT, self.Parent.GetId(), result)
            wx.PostEvent(self.Parent, evt)
            return True
        else:
            return False # Parent is dead no need to keep running

    def __KillPid(self, pid):
        """Kill a process by process id, causing the run loop to exit
        @param pid: Id of process to kill

        """
        # Dont kill if the process if it is the same one we
        # are running under (i.e we are running a shell command)
        if pid == os.getpid():
            return

        if wx.Platform != '__WXMSW__':
            # Close output pipe(s)
            try:
                try:
                    self._proc.stdout.close()
                except Exception, msg:
                    pass
            finally:
                self._proc.stdout = None

            # Try to kill the group
            try:
                os.kill(pid, self._sig_abort)
            except OSError, msg:
                pass

            # If still alive shoot it again
            if self._proc.poll() is not None:
                try:
                    os.kill(-pid, signal.SIGKILL)
                except OSError, msg:
                    pass

            # Try and wait for it to cleanup
            try:
                os.waitpid(pid, os.WNOHANG)
            except OSError, msg:
                pass

        else:
            # 1 == PROCESS_TERMINATE
            handle = ctypes.windll.kernel32.OpenProcess(1, False, pid)
            ctypes.windll.kernel32.TerminateProcess(handle, -1)
            ctypes.windll.kernel32.CloseHandle(handle)

    #---- Public Member Functions ----#
    def Abort(self, sig=signal.SIGTERM):
        """Abort the running process and return control to the main thread"""
        self._sig_abort = sig
        self.abort = True

    def DoPopen(self):
        """Open the process
        Override in a subclass to implement custom process opening
        @return: subprocess.Popen instance

        """
        raise NotImplementedError("Must implement DoPopen in subclasses!")

    def run(self):
        """Run the process until finished or aborted. Don't call this
        directly instead call self.start() to start the thread else this will
        run in the context of the current thread.
        @note: overridden from Thread

        """
        err = None
        try:
            self._proc = self.DoPopen()
        except OSError, msg:
            # NOTE: throws WindowsError on Windows which is a subclass of
            #       OSError, so it will still get caught here.
            if self.Parent:
                err =  OutputBufferEvent(edEVT_PROCESS_ERROR,
                                         self.Parent.GetId(),
                                         OPB_ERROR_INVALID_COMMAND)
                err.SetErrorMessage(msg)

        if self.Parent:
            evt = OutputBufferEvent(edEVT_PROCESS_START,
                                    self.Parent.GetId(),
                                    self.LastCommand)
            wx.PostEvent(self.Parent, evt)

        # Read from stdout while there is output from process
        while not err and True:
            if self.abort:
                self.__KillPid(self.Process.pid)
                self.__DoOneRead()
                more = False
                break
            else:
                more = False
                try:
                    more = self.__DoOneRead()
                except wx.PyDeadObjectError:
                    # Our parent window is dead so kill process and return
                    self.__KillPid(self.Process.pid)
                    return

                if not more:
                    break

        # Notify of error in running the process
        if err is not None:
            if self.Parent:
                wx.PostEvent(self.Parent, err)
            result = -1
        else:
            try:
                result = self.Process.wait()
            except OSError:
                result = -1

        # Notify that process has exited
        # Pack the exit code as the events value
        if self.Parent:
            evt = OutputBufferEvent(edEVT_PROCESS_EXIT, self.Parent.GetId(), result)
            wx.PostEvent(self.Parent, evt)

class ProcessThread(ProcessThreadBase):
    """Run a subprocess in a separate thread. Thread posts events back
    to parent object on main thread for processing in the ui.
    @see: EVT_PROCESS_START, EVT_PROCESS_END, EVT_UPDATE_TEXT

    """
    def __init__(self, parent, command, fname='',
                 args=list(), cwd=None, env=dict(),
                 use_shell=True):
        """Initialize the ProcessThread object
        Example:
          >>> myproc = ProcessThread(myframe, '/usr/local/bin/python',
                                     'hello.py', '--version', '/Users/me/home/')
          >>> myproc.start()

        @param parent: Parent Window/EventHandler to receive the events
                       generated by the process.
        @param command: Command string to execute as a subprocess.
        @keyword fname: Filename or path to file to run command on.
        @keyword args: Argument list or string to pass to use with fname arg.
        @keyword cwd: Directory to execute process from or None to use current
        @keyword env: Environment to run the process in (dictionary) or None to
                      use default.
        @keyword use_shell: Specify whether a shell should be used to launch 
                            program or run directly

        """
        super(ProcessThread, self).__init__(parent)

        if isinstance(args, list):
            args = u' '.join([arg.strip() for arg in args])

        # Attributes
        self._cwd = cwd             # Path at which to run from
        self._cmd = dict(cmd=command, file=fname, args=args)
        self._use_shell = use_shell

        # Make sure the environment is sane it must be all strings
        nenv = dict(env) # make a copy to manipulate
        for k, v in env.iteritems():
            if isinstance(v, types.UnicodeType):
                nenv[k] = v.encode(sys.getfilesystemencoding())
            elif not isinstance(v, basestring):
                nenv.pop(k)
        self._env = nenv

        # Setup
        self.setDaemon(True)

    def DoPopen(self):
        """Open the process
        @return: subprocess.Popen instance

        """
        # using shell, Popen will need a string, else it must be a sequence
        # use shlex for complex command line tokenization/parsing
        command = u' '.join([item.strip() for item in [self._cmd['cmd'],
                                                       self._cmd['file'],
                                                       self._cmd['args']]])
        command = command.strip()
        # TODO: exception handling and notification to main thread
        #       when encoding fails.
        command = command.encode(sys.getfilesystemencoding())
        if not self._use_shell and not subprocess.mswindows:
            # Note: shlex does not support Unicode
            command = shlex.split(command)

        # TODO: if a file path to the exe has any spaces in it on Windows
        #       and use_shell is True then the command will fail. Must force
        #       to False under this condition.
        use_shell = self._use_shell
        # TODO: See about supporting use_shell on Windows it causes lots of
        #       issues with gui apps and killing processes when it is True.
        if use_shell and subprocess.mswindows:
            suinfo = subprocess.STARTUPINFO()
            # Don't set this flag if we are not using the shell on
            # Windows as it will cause any gui app to not show on the
            # display!
            #TODO: move this into common library as it is needed
            #      by most code that uses subprocess
            if hasattr(subprocess, 'STARTF_USESHOWWINDOW'):
                suinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
            else:
                try:
                    from win32process import STARTF_USESHOWWINDOW
                    suinfo.dwFlags |= STARTF_USESHOWWINDOW
                except ImportError:
                    # Give up and try hard coded value from Windows.h
                    suinfo.dwFlags |= 0x00000001
        else:
            suinfo = None
        
        proc = subprocess.Popen(command,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT,
                                shell=use_shell,
                                cwd=self._cwd,
                                env=self._env,
                                startupinfo=suinfo)
        self.LastCommand = command # Set last run command
        return proc

    def SetArgs(self, args):
        """Set the args to pass to the command
        @param args: list or string of program arguments

        """
        if isinstance(args, list):
            u' '.join(item.strip() for item in args)
        self._cmd['args'] = args.strip()

    def SetCommand(self, cmd):
        """Set the command to execute
        @param cmd: Command string

        """
        self._cmd['cmd'] = cmd

    def SetFilename(self, fname):
        """Set the filename to run the command on
        @param fname: string or Unicode

        """
        self._cmd['file'] = fname

#-----------------------------------------------------------------------------#

class TaskThread(threading.Thread):
    """Run a task in its own thread."""
    def __init__(self, parent, task, *args, **kwargs):
        """Initialize the TaskThread. All *args and **kwargs are passed
        to the task.

        @param parent: Parent Window/EventHandler to receive the events
                       generated by the process.
        @param task: callable should be a generator object and must be iterable

        """
        super(TaskThread, self).__init__()
        assert isinstance(parent, OutputBuffer)

        self._task = TaskObject(parent, task, *args, **kwargs)

    def run(self):
        self._task.DoTask()

    def Cancel(self):
        self._task.Cancel()

class TaskObject(object):
    """Run a task in its own thread."""
    def __init__(self, parent, task, *args, **kwargs):
        """Initialize the TaskObject. All *args and **kwargs are passed
        to the task.

        @param parent: Parent Window/EventHandler to receive the events
                       generated by the process.
        @param task: callable should be a generator object and must be iterable

        """
        super(TaskObject, self).__init__()

        assert isinstance(parent, OutputBuffer)

        # Attributes
        self.cancel = False         # Abort task
        self._parent = parent       # Parent Window/Event Handler
        self.task = task            # Task method to run
        self._args = args
        self._kwargs = kwargs

    def DoTask(self):
        """Start running the task"""
        # Notify that task is beginning
        evt = OutputBufferEvent(edEVT_TASK_START, self._parent.GetId())
        wx.PostEvent(self._parent, evt)
        time.sleep(.5) # Give the event a chance to be processed

        # Run the task and post the results
        for result in self.task(*self._args, **self._kwargs):
            self._parent.AppendUpdate(result)
            if self.cancel:
                break

        # Notify that the task is finished
        evt = OutputBufferEvent(edEVT_TASK_COMPLETE, self._parent.GetId())
        wx.PostEvent(self._parent, evt)

    def Cancel(self):
        """Cancel the running task"""
        self.cancel = True
