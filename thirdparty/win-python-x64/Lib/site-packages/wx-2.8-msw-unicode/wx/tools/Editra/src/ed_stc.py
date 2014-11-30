###############################################################################
# Name: ed_stc.py                                                             #
# Purpose: Editra's styled editing buffer                                     #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This is the main component of the editor that manages all the information
of the on disk file that it represents in memory. It works with the StyleManager
and SyntaxManager to provide an editing pane that auto detects and configures
itself for type of file that is in buffer to do highlighting and other language
specific options such as commenting code.

@summary: Editra's main text buffer class

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_stc.py 68139 2011-07-02 20:35:04Z CJP $"
__revision__ = "$Revision: 68139 $"

#-------------------------------------------------------------------------#
# Imports

import os
import wx, wx.stc

# Local Imports
import ed_event
import ed_glob
from profiler import Profile_Get as _PGET
from syntax import syntax
import util
import ed_basestc
import ed_marker
import ed_msg
import ed_mdlg
import ed_txt
from ed_keyh import KeyHandler, ViKeyHandler
import ebmlib
import ed_thread

#-------------------------------------------------------------------------#
# Globals
_ = wx.GetTranslation

# EOL Constants
EDSTC_EOL_CR   = ed_glob.EOL_MODE_CR
EDSTC_EOL_LF   = ed_glob.EOL_MODE_LF
EDSTC_EOL_CRLF = ed_glob.EOL_MODE_CRLF

# Character sets
SPACECHARS = " \t\r\n"
NONSPACE = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
OPERATORS = "./\?[]{}<>!@#$%^&*():=-+\"';,"

#-------------------------------------------------------------------------#

def jumpaction(func):
    """Decorator method to notify clients about jump actions"""
    def WrapJump(*args, **kwargs):
        """Wrapper for capturing before/after pos of a jump action"""
        stc = args[0]
        pos = stc.GetCurrentPos()
        line = stc.GetCurrentLine()
        func(*args, **kwargs)
        cpos = stc.GetCurrentPos()
        cline = stc.GetCurrentLine()
        fname = stc.GetFileName()

        mdata = dict(fname=fname,
                     prepos=pos, preline=line,
                     lnum=cline, pos=cpos)
        tlw = stc.GetTopLevelParent()
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_POS_JUMPED, mdata, tlw.GetId()) 

    WrapJump.__name__ = func.__name__
    WrapJump.__doc__ = func.__doc__
    return WrapJump


#-------------------------------------------------------------------------#

class EditraStc(ed_basestc.EditraBaseStc):
    """Defines a styled text control for editing text
    @summary: Subclass of wx.stc.StyledTextCtrl and L{ed_style.StyleMgr}.
              Manages the documents display and input.

    """
    def __init__(self, parent, id_,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, use_dt=True):
        """Initializes a control and sets the default objects for
        Tracking events that occur in the control.
        @keyword use_dt: whether to use a drop target or not

        """
        super(EditraStc, self).__init__(parent, id_, pos, size, style)

        self.SetModEventMask(wx.stc.STC_PERFORMED_UNDO | \
                             wx.stc.STC_PERFORMED_REDO | \
                             wx.stc.STC_MOD_DELETETEXT | \
                             wx.stc.STC_MOD_INSERTTEXT)

        self.CmdKeyAssign(ord('-'), wx.stc.STC_SCMOD_CTRL, \
                          wx.stc.STC_CMD_ZOOMOUT)
        self.CmdKeyAssign(ord('+'), wx.stc.STC_SCMOD_CTRL | \
                          wx.stc.STC_SCMOD_SHIFT, wx.stc.STC_CMD_ZOOMIN)

        #---- Drop Target ----#
        if use_dt and hasattr(parent, 'OnDrop'):
            self.SetDropTarget(util.DropTargetFT(self, None, parent.OnDrop))

        # Attributes
        self.LOG = wx.GetApp().GetLog()
        self._loading = None
        self.key_handler = KeyHandler(self)
        self._backup_done = True
        self._bktimer = wx.Timer(self)
        self._dwellsent = False

        # Macro Attributes
        self._macro = list()
        self.recording = False

        # Command/Settings Attributes
        self._config = dict(autocomp=_PGET('AUTO_COMP'),
                            autoindent=_PGET('AUTO_INDENT'),
                            brackethl=_PGET('BRACKETHL'),
                            folding=_PGET('CODE_FOLD'),
                            highlight=_PGET('SYNTAX'),
                            autobkup=_PGET('AUTOBACKUP'))

        # Set Default Styles used by all documents
        self.Configure()
        self.UpdateBaseStyles()

        # Other Settings
        self.SetMouseDwellTime(900)
        self.UsePopUp(False)

        #self.Bind(wx.stc.EVT_STC_MACRORECORD, self.OnRecordMacro)
        self.Bind(wx.stc.EVT_STC_MARGINCLICK, self.OnMarginClick)
        self.Bind(wx.stc.EVT_STC_UPDATEUI, self.OnUpdateUI)
        self.Bind(wx.stc.EVT_STC_USERLISTSELECTION, self.OnUserListSel)
        self.Bind(wx.stc.EVT_STC_DWELLSTART, self.OnDwellStart)
        self.Bind(wx.stc.EVT_STC_DWELLEND, self.OnDwellEnd)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_TIMER, self.OnBackupTimer)

        # Async file load events
        self.Bind(ed_txt.EVT_FILE_LOAD, self.OnLoadProgress)

       #---- End Init ----#

    __name__ = u"EditraTextCtrl"

    #---- Protected Member Functions ----#

    def _BuildMacro(self):
        """Constructs a macro script from items in the macro
        record list.
        @status: in limbo

        """
        if not len(self._macro):
            return

        # Get command mappings
        cmds = list()
        for x in dir(wx.stc):
            if x.startswith('STC_CMD_'):
                cmds.append(x)
        cmdvals = [getattr(wx.stc, x) for x in cmds]
        cmds = [x.replace('STC_CMD_', u'') for x in cmds]

        # Get the commands names used in the macro
        named = list()
        for x in self._macro:
            if x[0] in cmdvals:
                named.append(cmds[cmdvals.index(x[0])])
        code = list()

        stc_dict = wx.stc.StyledTextCtrl.__dict__
        for cmd in named:
            for attr in stc_dict:
                if attr.upper() == cmd:
                    code.append(attr)
                    break

        code_txt = u''
        for fun in code:
            code_txt += "    ctrl.%s()\n" % fun
        code_txt += "    print \"Executed\""    #TEST
        code_txt = "def macro(ctrl):\n" + code_txt
        self.GetParent().NewPage()
        self.GetParent().GetCurrentPage().SetText(code_txt)
        self.GetParent().GetCurrentPage().FindLexer('py')
#         code = compile(code_txt, self.__module__, 'exec')
#         exec code in self.__dict__ # Inject new code into this namespace

    def _MacHandleKey(self, k_code, shift_down, alt_down, ctrl_down, cmd_down):
        """Handler for mac specific actions"""
        if alt_down:
            return False

        if (k_code == wx.WXK_BACK and shift_down) and \
           not (ctrl_down or cmd_down):
            self.DeleteForward()
        elif cmd_down and not ctrl_down:
            line = self.GetCurrentLine()
            if k_code == wx.WXK_RIGHT:
                pos = self.GetLineStartPosition(line)
                txt = self.GetLine(line)
                diff = len(txt.rstrip())
                self.GotoPos(pos + diff)
                if shift_down:
                    self.SetSelection(pos, pos + diff)
            elif k_code == wx.WXK_LEFT:
                cpos = self.GetCurrentPos()
                self.GotoIndentPos(line)
                if shift_down:
                    self.SetSelection(cpos, self.GetCurrentPos())
            else:
                return False
        else:
            return False

        return True

    #---- Public Member Functions ----#

    def AddBookmark(self, line=-1):
        """Add a bookmark and return its handle
        Sends notifications for bookmark added
        @keyword line: if < 0 bookmark will be added to current line

        """
        rval = self.AddMarker(ed_marker.Bookmark(), line)
        mdata = dict(stc=self, added=True, line=line, handle=rval)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_BOOKMARK, mdata)
        return rval

    def RemoveBookmark(self, line):
        """Remove the book mark from the given line
        Sends notifications for bookmark removal.
        @param line: int

        """
        self.RemoveMarker(ed_marker.Bookmark(), line)
        mdata = dict(stc=self, added=False, line=line)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_BOOKMARK, mdata)

    def RemoveAllBookmarks(self):
        """Remove all the bookmarks in the buffer
        Sends notifications for bookmark removal.

        """
        self.RemoveAllMarkers(ed_marker.Bookmark())
        mdata = dict(stc=self, added=False, line=-1)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_BOOKMARK, mdata)

    def PlayMacro(self):
        """Send the list of built up macro messages to the editor
        to be played back.
        @postcondition: the macro of this control has been played back

        """
        self.BeginUndoAction()
        for msg in self._macro:
            if msg[0] == 2170:
                self.AddText(msg[2])
            elif msg[0] == 2001:
                self.AddText(self.GetEOLChar() + u' ' * (msg[1] - 1))
            else:
                self.SendMsg(msg[0], msg[1], msg[2])
        self.EndUndoAction()

    #---- Begin Function Definitions ----#

    def Bookmark(self, action):
        """Handles bookmark actions
        @param action: An event ID that describes what is to be done
        @return: None

        """
        lnum = self.GetCurrentLine()
        mark = -1
        if action == ed_glob.ID_ADD_BM:
            if self.MarkerGet(lnum):
                self.RemoveBookmark(lnum)
            else:
                self.AddBookmark(lnum)
        elif action == ed_glob.ID_DEL_ALL_BM:
            self.RemoveAllBookmarks()
        elif action == ed_glob.ID_NEXT_MARK:
            if self.MarkerGet(lnum):
                lnum += 1
            mark = self.MarkerNext(lnum, 1)
            if mark == -1:
                mark = self.MarkerNext(0, 1)
        elif action == ed_glob.ID_PRE_MARK:
            if self.MarkerGet(lnum):
                lnum -= 1
            mark = self.MarkerPrevious(lnum, 1)
            if mark == -1:
                mark = self.MarkerPrevious(self.GetLineCount(), 1)

        if mark != -1:
            self.GotoLine(mark)

    # TODO: DO NOT use these methods anywhere new they will be removed soon
    def ShowCommandBar(self):
        """Open the command bar"""
        self.GetTopLevelParent().GetEditPane().ShowCommandControl(ed_glob.ID_COMMAND)

    def ShowFindBar(self):
        """Open the quick-find bar"""
        self.GetTopLevelParent().GetEditPane().ShowCommandControl(ed_glob.ID_QUICK_FIND)
    # END TODO

    def GetBookmarks(self):
        """Gets a list of all lines containing bookmarks
        @return: list of line numbers

        """
        MarkIsSet = ed_marker.Bookmark.IsSet
        return [line for line in range(self.GetLineCount())
                if MarkIsSet(self, line)]

    def DoBraceHighlight(self):
        """Perform a brace matching highlight
        @note: intended for internal use only
        """
        brace_at_caret, brace_opposite = self.GetBracePair()
        # CallAfter necessary to reduce CG warnings on Mac
        if brace_at_caret != -1  and brace_opposite == -1:
            wx.CallAfter(self.BraceBadLight, brace_at_caret)
        else:
            wx.CallAfter(self.BraceHighlight, brace_at_caret, brace_opposite)

    def GetBracePair(self, pos=-1):
        """Get a tuple of the positions in the buffer where the brace at the
        current caret position and its match are. if a brace doesn't have a
        match it will return -1 for the missing brace.
        @keyword pos: -1 to use current cursor pos, else use to specify brace pos
        @return: tuple (brace_at_caret, brace_opposite)

        """
        brace_at_caret = -1
        brace_opposite = -1
        char_before = None
        if pos < 0:
            # use current position
            caret_pos = self.GetCurrentPos()
        else:
            caret_pos = pos

        if caret_pos > 0:
            char_before = self.GetCharAt(caret_pos - 1)

        # check before
        if char_before and unichr(char_before) in "[]{}()<>":
            brace_at_caret = caret_pos - 1

        # check after
        if brace_at_caret < 0:
            char_after = self.GetCharAt(caret_pos)
            if char_after and chr(char_after) in "[]{}()<>":
                brace_at_caret = caret_pos

        if brace_at_caret >= 0:
            brace_opposite = self.BraceMatch(brace_at_caret)

        return (brace_at_caret, brace_opposite)

    def Configure(self):
        """Configures the editors settings by using profile values
        @postcondition: all profile dependent attributes are configured

        """
#        self.SetControlCharSymbol(172)
        self.SetWrapMode(_PGET('WRAP', 'bool'))
        self.SetViewWhiteSpace(_PGET('SHOW_WS', 'bool'))
        self.SetUseAntiAliasing(_PGET('AALIASING'))
        self.SetUseTabs(_PGET('USETABS'))
        self.SetBackSpaceUnIndents(_PGET('BSUNINDENT'))
        self.SetCaretLineVisible(_PGET('HLCARETLINE'))
        self.SetIndent(_PGET('INDENTWIDTH', 'int'))
        self.SetTabWidth(_PGET('TABWIDTH', 'int'))
#        self.SetTabIndents(True) # Add option for this too?
        self.SetIndentationGuides(_PGET('GUIDES'))
        self.SetEOLMode(_PGET('EOL_MODE'))
        self.SetViewEOL(_PGET('SHOW_EOL'))
        self.SetAutoComplete(_PGET('AUTO_COMP'))
        self.FoldingOnOff(_PGET('CODE_FOLD'))
        self.ToggleAutoIndent(_PGET('AUTO_INDENT'))
        self.ToggleBracketHL(_PGET('BRACKETHL'))
        self.ToggleLineNumbers(_PGET('SHOW_LN'))
        self.SetViEmulationMode(_PGET('VI_EMU'), _PGET('VI_NORMAL_DEFAULT'))
        self.SetViewEdgeGuide(_PGET('SHOW_EDGE'))
        self.EnableAutoBackup(_PGET('AUTOBACKUP'))
        self.SetEndAtLastLine(not _PGET('VIEWVERTSPACE', default=False))

    def ConvertCase(self, upper=False):
        """Converts the case of the selected text to either all lower
        case(default) or all upper case.
        @keyword upper: Flag whether conversion is to upper case or not.

        """
        sel = self.GetSelectedText()
        if upper:
            sel = sel.upper()
        else:
            sel = sel.lower()
        self.ReplaceSelection(sel)

    def EnableAutoBackup(self, enable):
        """Enable automatic backups
        @param enable: bool

        """
        if enable:
            # TODO: make backup interval configurable
            if not self._bktimer.IsRunning():
                self._bktimer.Start(30000) # every 30 seconds
        else:
            if self._bktimer.IsRunning():
                self._bktimer.Stop()

    def InvertCase(self):
        """Invert the case of the selected text
        @postcondition: all text in selection has case inverted

        """
        text = self.GetSelectedText()
        if len(text):
            self.BeginUndoAction()
            self.ReplaceSelection(text.swapcase())
            self.EndUndoAction()

    def GetAutoIndent(self):
        """Returns whether auto-indent is being used
        @return: whether autoindent is active or not
        @rtype: bool

        """
        return self._config['autoindent']

    def GetLineStartPosition(self, line):
        """Get the starting position of the given line
        @param line: int
        @return: int

        """
        if line > 0:
            spos = self.GetLineEndPosition(line-1)
            if self.GetLine(line).endswith("\r\n"):
                spos += 2
            else:
                spos += 1
        else:
            spos = 0
        return spos

    def GetLastVisibleLine(self):
        """Return what the last visible line is
        @return: int

        """
        return self.GetFirstVisibleLine() + self.LinesOnScreen() - 1

    def GetMiddleVisibleLine(self):
        """Return the number of the line that is in the middle of the display
        @return: int

        """
        fline = self.GetFirstVisibleLine()
        if self.LinesOnScreen() < self.GetLineCount():
            mid = (fline + (self.LinesOnScreen() / 2))
        else:
            mid = (fline + (self.GetLineCount() / 2))
        return mid

    def GotoBraceMatch(self):
        """Jump the caret to the brace opposite of the one the caret is
        currently at. If there is no match or the caret currently is not next
        to a brace no action is taken.
        @return: bool

        """
        cbrace, brace_opposite = self.GetBracePair()
        if -1 in (cbrace, brace_opposite):
            return False
        else:
            self.GotoPos(brace_opposite)
            return True

    def GotoColumn(self, column):
        """Move caret to column of current line
        @param column: Column to move to

        """
        cline = self.GetCurrentLineNum()
        lstart = self.PositionFromLine(cline)
        lend = self.GetLineEndPosition(cline)
        linelen = lend - lstart
        if column > linelen:
            column = linelen
        self.GotoPos(lstart + column)

    @jumpaction
    def GotoLine(self, line):
        """Move caret to beginning given line number
        @param line: line to go to (int)

        """
        if line > self.GetLineCount():
            line = self.GetLineCount()
        elif line < 0:
            line = 0
        else:
            pass

        self.SetYCaretPolicy(wx.stc.STC_CARET_STRICT, 0)
        super(EditraStc, self).GotoLine(line)
        self.SetYCaretPolicy(wx.stc.STC_CARET_EVEN, 0)
        self.PostPositionEvent()

    @jumpaction
    def GotoPos(self, pos):
        """Override StyledTextCtrl.GotoPos
        @param pos: position in buffer to move caret to (int)

        """
        super(EditraStc, self).GotoPos(pos)
        self.PostPositionEvent()

    def SetCaretPos(self, pos):
        """Set the caret position without posting jump events
        @param pos: position to go to

        """
        super(EditraStc, self).GotoPos(pos)
        self.PostPositionEvent()

    def GotoIndentPos(self, line=None):
        """Move the caret to the end of the indentation
        on the given line.
        @param line: line to go to

        """
        if line is None:
            line = self.GetCurrentLine()
        self.GotoPos(self.GetLineIndentPosition(line))

    def SetCurrentCol(self, column):
        """Set the current column position on the currently line
        extending the selection.
        @param column: Column to move to

        """
        cline = self.GetCurrentLineNum()
        lstart = self.PositionFromLine(cline)
        lend = self.GetLineEndPosition(cline)
        linelen = lend - lstart
        if column > linelen:
            column = linelen
        self.SetCurrentPos(lstart + column)

    def DeleteForward(self):
        """Delete the selection, or if there is no selection, then
        delete the character to the right of the cursor.

        """
        if self.GetSelectionStart() == self.GetSelectionEnd():
            self.SetCurrentPos(self.GetCurrentPos() + 1)
        self.DeleteBack()

    def EnableKeyProcessor(self, enable=True):
        """Enable specialized key handling
        @keyword enable: bool

        """
        self.key_handler.EnableProcessing(enable)

    def GetAutoComplete(self):
        """Is Autocomplete being used by this instance
        @return: whether autocomp is active or not

        """
        return self._config['autocomp']

    def OnBackupTimer(self, evt):
        """Backup the buffer to a backup file.
        @param evt: wx.TimerEvent

        """
        fname = self.GetFileName()
        # If the file is loading or is over 5MB don't do automatic backups.
        if self.IsLoading() or ebmlib.GetFileSize(fname) > 5242880:
            return

        # If the file is different than the last save point make the backup.
        bkupmgr = ebmlib.FileBackupMgr(None, u"%s.edbkup")
        path = _PGET('AUTOBACKUP_PATH', default=u"")
        if path and os.path.exists(path):
            bkupmgr.SetBackupDirectory(path)

        if not self._backup_done and \
           (not bkupmgr.HasBackup(fname) or bkupmgr.IsBackupNewer(fname)):
            msg = _("File backup performed: %s") % fname
            idval = self.Id
            target = self.TopLevelParent
            def BackupJob(fobj, text):
                writer = bkupmgr.GetBackupWriter(fobj)
                try:
                    writer(text)
                except Exception, msg:
                    return
                nevt = ed_event.StatusEvent(ed_event.edEVT_STATUS, idval,
                                            msg, ed_glob.SB_INFO)
                wx.PostEvent(target, nevt)
            ed_thread.EdThreadPool().QueueJob(BackupJob, self.File, self.GetText())
            self._backup_done = True

    def OnModified(self, evt):
        """Overrides base modified handler"""
        super(EditraStc, self).OnModified(evt)
        if not self.IsLoading():
            self._backup_done = False

    def OnKeyDown(self, evt):
        """Handles keydown events, currently only deals with
        auto indentation.
        @param evt: event that called this handler
        @type evt: wx.KeyEvent

        """
        k_code = evt.GetKeyCode()
        shift_down = evt.ShiftDown()
        alt_down = evt.AltDown()
        ctrl_down = evt.ControlDown()
        cmd_down = evt.CmdDown()

        if self.key_handler.PreProcessKey(k_code, ctrl_down,
                                          cmd_down, shift_down,
                                          alt_down):
            return

        if wx.Platform == '__WXMAC__' and self._MacHandleKey(k_code, shift_down,
                                                             alt_down, ctrl_down,
                                                             cmd_down):
            pass
        elif k_code == wx.WXK_RETURN:
            if self._config['autoindent'] and not self.AutoCompActive():
                if self.GetSelectedText():
                    self.CmdKeyExecute(wx.stc.STC_CMD_NEWLINE)
                else:
                    self.AutoIndent()
            else:
                evt.Skip()

            self.CallTipCancel()

        elif self.VertEdit.Enabled:
            # XXX: handle column mode
            self.VertEdit.OnKeyDown(evt)
        else:
            evt.Skip()

    def OnChar(self, evt):
        """Handles Char events that aren't caught by the
        KEY_DOWN event.
        @param evt: event that called this handler
        @type evt: wx.EVT_CHAR
        @todo: autocomp/calltip lookup can be very cpu intensive it may
               be better to try and process it on a separate thread to
               prevent a slow down in the input of text into the buffer

        """
        key_code = evt.GetKeyCode()
        cpos = self.GetCurrentPos()
        cmpl = self.GetCompleter()
        if self.key_handler.ProcessKey(key_code, evt.ControlDown(),
                                       evt.CmdDown(), evt.ShiftDown(),
                                       evt.AltDown()):
            # The key handler handled this keypress, we don't need to insert
            # the character into the buffer.
            pass

        elif not self._config['autocomp'] or not cmpl.ShouldCheck(cpos):
            evt.Skip()
            return

        elif key_code in cmpl.GetAutoCompKeys():
            self.HidePopups()

            uchr = unichr(key_code)
            command = self.GetCommandStr() + uchr
            self.PutText(uchr)

            if self._config['autocomp']:
                self.ShowAutoCompOpt(command)

        elif key_code in cmpl.GetCallTipKeys():
            self.HidePopups()
            uchr = unichr(key_code)
            command = self.GetCommandStr() + uchr
            self.PutText(uchr)

            if self._config['autocomp']:
                self.ShowCallTip(command)

        elif key_code in cmpl.GetCallTipCancel():
            evt.Skip()
            self.CallTipCancel()
#        elif key_code == wx.WXK_TAB and \
#             True not in (evt.ControlDown(), evt.CmdDown(), 
#                          evt.ShiftDown(), evt.AltDown()):
#            self.Tab() # <- So action can be overridden
        else:
#            print "IS TAB", key_code, wx.WXK_TAB
            evt.Skip()

    def DoAutoComplete(self):
        """Atempt to perform an autocompletion event."""
        self.HidePopups()
        if self._config['autocomp']:
            command = self.GetCommandStr()
            self.ShowAutoCompOpt(command)

    def DoCallTip(self):
        """Attempt to show a calltip for the current cursor position"""
        self.HidePopups()
        if self._config['autocomp']:
            command = self.GetCommandStr()
            # TODO: GetCommandStr seems to be inadquate under some cases
            self.ShowCallTip(command)

    def OnKeyUp(self, evt):
        """Update status bar of window
        @param evt: wxEVT_KEY_UP

        """
        evt.Skip()
        self.PostPositionEvent()
        tlw = self.GetTopLevelParent()
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_KEYUP,
                           (evt.GetPositionTuple(), evt.GetKeyCode()),
                           tlw.GetId())

    def PostPositionEvent(self):
        """Post an event to update the status of the line/column"""
        line, column = self.GetPos()
        pinfo = dict(lnum=line, cnum=column)
        msg = _("Line: %(lnum)d  Column: %(cnum)d") % pinfo
        nevt = ed_event.StatusEvent(ed_event.edEVT_STATUS, self.GetId(),
                                    msg, ed_glob.SB_ROWCOL)
        tlw = self.GetTopLevelParent()
        wx.PostEvent(tlw, nevt)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_POS_CHANGED, pinfo, tlw.GetId())

    def OnRecordMacro(self, evt):
        """Records macro events
        @param evt: event that called this handler
        @type evt: wx.stc.StyledTextEvent

        """
        if self.IsRecording():
            msg = evt.GetMessage()
            if msg == 2170:
                lparm = self.GetTextRange(self.GetCurrentPos()-1, \
                                          self.GetCurrentPos())
            else:
                lparm = evt.GetLParam()
            mac = (msg, evt.GetWParam(), lparm)
            self._macro.append(mac)
#             if mac[0] != 2170:
#                 self._macro.append(mac)
        else:
            evt.Skip()

    def ParaDown(self): # pylint: disable-msg=W0221
        """Move the caret one paragraph down
        @note: overrides the default function to set caret at end
               of paragraph instead of jumping to start of next

        """
        self.WordPartRight()
        super(EditraStc, self).ParaDown()
        if self.GetCurrentPos() != self.GetLength():
            self.WordPartLeft()
            self.GotoPos(self.GetCurrentPos() + len(self.GetEOLChar()))

    def ParaDownExtend(self): # pylint: disable-msg=W0221
        """Extend the selection a paragraph down
        @note: overrides the default function to set selection at end
               of paragraph instead of jumping to start of next so that
               extra blank lines don't get swallowed.

        """
        self.WordRightExtend()
        super(EditraStc, self).ParaDownExtend()
        if self.GetCurrentPos() != self.GetLength():
            self.WordLeftExtend()
            self.SetCurrentPos(self.GetCurrentPos() + len(self.GetEOLChar()))

    @jumpaction
    def OnLeftUp(self, evt):
        """Set primary selection and inform mainwindow that cursor position
        has changed.
        @param evt: wx.MouseEvent()

        """
        evt.Skip()
        # FIXME: there is problems with using the primary selection. Setting
        #        the primary selection causes anything else on the clipboard
        #        to get killed.
#        stxt = self.GetSelectedText()
#        if len(stxt):
#            util.SetClipboardText(stxt, primary=True)
        self.PostPositionEvent()

    def OnLoadProgress(self, evt):
        """Recieves file loading events from asynchronous file loading"""
        pid = self.GetTopLevelParent().GetId()
        if evt.GetState() == ed_txt.FL_STATE_READING:
            if evt.HasText():
                # TODO: get gauge updates working properly
#                sb = self.GetTopLevelParent().GetStatusBar()
#                gauge = sb.GetGauge()
#                gauge.SetValue(evt.GetProgress())
#                gauge.Show()
#                gauge.ProcessPendingEvents()
#                sb.ProcessPendingEvents()
                self.SetReadOnly(False)
                self.AppendText(evt.GetValue())
                self.SetSavePoint()
                self.SetReadOnly(True)
                # wx.GetApp().Yield(True) # Too slow on windows...
        elif evt.GetState() == ed_txt.FL_STATE_END:
            self.SetReadOnly(False)
            ed_msg.PostMessage(ed_msg.EDMSG_PROGRESS_STATE, (pid, 0, 0))
            self.SetSavePoint()
            self.SetUndoCollection(True)
            del self._loading
            self._loading = None
            parent = self.GetParent()
            if hasattr(parent, 'DoPostLoad'):
                parent.DoPostLoad()
        elif evt.GetState() == ed_txt.FL_STATE_START:
            ed_msg.PostMessage(ed_msg.EDMSG_PROGRESS_SHOW, (pid, True))
            ed_msg.PostMessage(ed_msg.EDMSG_PROGRESS_STATE, (pid, 0, self.File.GetSize()))
            self.SetReadOnly(True)
            self.SetUndoCollection(False)
        elif evt.GetState() == ed_txt.FL_STATE_ABORTED:
            self.SetReadOnly(False)
            self.ClearAll()

    def OnUpdateUI(self, evt):
        """Check for matching braces
        @param evt: event that called this handler
        @type evt: wx.stc.StyledTextEvent

        """
        # If disabled just skip the event
        if self._config['brackethl']:
            self.DoBraceHighlight()

        # XXX: handle when column mode is enabled
        if self.VertEdit.Enabled:
            self.VertEdit.OnUpdateUI(evt)
        evt.Skip()

    def OnUserListSel(self, evt):
        """Callback hook for userlist selections"""
        mdata = dict(ltype=evt.GetListType(),
                     text=evt.GetText(),
                     stc=self)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_USERLIST_SEL, mdata,
                           context=self.GetTopLevelParent().GetId())
        evt.Skip()

    def OnDwellStart(self, evt):
        """Callback hook for mouse dwell start"""
        # Workaround issue where this event in incorrectly sent
        # when the mouse has not dwelled within the buffer area
        mpoint = wx.GetMousePosition()
        brect = self.GetScreenRect()
        if not brect.Contains(mpoint) or \
           not self.IsShown() or \
           not self.GetTopLevelParent().IsActive():
            return

        position = evt.Position
        if not self._dwellsent and position >= 0:
            dwellword = self.GetWordFromPosition(position)[0]
            line_num = self.LineFromPosition(position) + 1
            mdata = dict(stc=self, pos=position,
                         line=line_num,
                         word=dwellword)
            ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_DWELL_START, mdata)

            tip = mdata.get('rdata', None)
            if tip:
                self.CallTipShow(position, tip)
            else:
                # Clients did not need to make use of the calltip
                # so check if auto-completion provider has anything to display.
                if not self.IsComment(position) and not self.IsString(position):
                    endpos = self.WordEndPosition(position, True)
                    col = self.GetColumn(endpos)
                    line = self.GetLine(line_num-1)
                    command = self.GetCommandStr(line, col)
                    tip = self._code['compsvc'].GetCallTip(command)
                    if len(tip):
                        tip_pos = position - (len(dwellword.split('.')[-1]) + 1)
                        fail_safe = position - self.GetColumn(position)
                        self.CallTipShow(max(tip_pos, fail_safe), tip)
        evt.Skip()

    def OnDwellEnd(self, evt):
        """Callback hook for mouse dwell end"""
        self._dwellsent = False
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_DWELL_END)
        self.CallTipCancel()
        evt.Skip()

    def OnMarginClick(self, evt):
        """Open and Close Folders as Needed
        @param evt: event that called this handler
        @type evt: wx.stc.StyledTextEvent

        """
        margin_num = evt.GetMargin()
        if margin_num == ed_basestc.FOLD_MARGIN:
            if evt.GetShift() and \
               (evt.GetControl() or (wx.Platform == '__WXMAC__' and evt.GetAlt())):
                self.FoldAll()
            else:
                line_clicked = self.LineFromPosition(evt.GetPosition())
                level = self.GetFoldLevel(line_clicked)
                if level & wx.stc.STC_FOLDLEVELHEADERFLAG:

                    # Expand node and all Subnodes
                    if evt.GetShift():
                        self.SetFoldExpanded(line_clicked, True)
                        self.Expand(line_clicked, True, True, 100, level)
                    elif evt.GetControl() or \
                        (wx.Platform == '__WXMAC__' and evt.GetAlt()):
                        # Contract all subnodes of clicked one
                        # Note: using Alt as Ctrl can not be received for
                        # clicks on mac (Scintilla Bug).
                        if self.GetFoldExpanded(line_clicked):
                            self.SetFoldExpanded(line_clicked, False)
                            self.Expand(line_clicked, False, True, 0, level)
                        else:
                            # Expand all subnodes
                            self.SetFoldExpanded(line_clicked, True)
                            self.Expand(line_clicked, True, True, 100, level)
                    else:
                        self.ToggleFold(line_clicked)
        elif margin_num == ed_basestc.MARK_MARGIN:
            # Bookmarks ect...
            line_clicked = self.LineFromPosition(evt.GetPosition())
            # Hook for client code to interact with margin clicks
            data = dict(stc=self, line=line_clicked)
            ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_MARGIN_CLICK, msgdata=data)
            if not data.get('handled', False):
                # Default to internal bookmark handling
                if ed_marker.Bookmark().IsSet(self, line_clicked):
                    self.RemoveBookmark(line_clicked)
                else:
                    self.AddBookmark(line_clicked)

    def FoldAll(self):
        """Fold Tree In or Out
        @postcondition: code tree is folded open or closed

        """
        line_count = self.GetLineCount()
        expanding = True

        # find out if we are folding or unfolding
        for line_num in range(line_count):
            if self.GetFoldLevel(line_num) & wx.stc.STC_FOLDLEVELHEADERFLAG:
                expanding = not self.GetFoldExpanded(line_num)
                break
        line_num = 0

        while line_num < line_count:
            level = self.GetFoldLevel(line_num)

            if level & wx.stc.STC_FOLDLEVELHEADERFLAG and \
               (level & wx.stc.STC_FOLDLEVELNUMBERMASK) == \
               wx.stc.STC_FOLDLEVELBASE:

                if expanding:
                    self.SetFoldExpanded(line_num, True)
                    line_num = self.Expand(line_num, True) - 1
                else:
                    last_child = self.GetLastChild(line_num, -1)
                    self.SetFoldExpanded(line_num, False)

                    if last_child > line_num:
                        self.HideLines(line_num + 1, last_child)
            line_num = line_num + 1

    def Expand(self, line, do_expand, force=False, vis_levels=0, level=-1):
        """Open the Margin Folder
        @postcondition: the selected folder is expanded

        """
        last_child = self.GetLastChild(line, level)
        line = line + 1

        while line <= last_child:
            if force:
                if vis_levels > 0:
                    self.ShowLines(line, line)
                else:
                    self.HideLines(line, line)
            else:
                if do_expand:
                    self.ShowLines(line, line)

            if level == -1:
                level = self.GetFoldLevel(line)

            if level & wx.stc.STC_FOLDLEVELHEADERFLAG:
                if force:
                    self.SetFoldExpanded(line, vis_levels > 1)
                    line = self.Expand(line, do_expand, force, vis_levels - 1)
                else:
                    if do_expand:
                        if self.GetFoldExpanded(line):
                            self.SetFoldExpanded(line, True)
                    line = self.Expand(line, do_expand, force, vis_levels - 1)
            else:
                line = line + 1
        return line

    def ExpandAll(self):
        """Expand all folded code blocks"""
        line_count = self.GetLineCount()
        for line_num in xrange(line_count):
            if self.GetFoldLevel(line_num) & wx.stc.STC_FOLDLEVELHEADERFLAG:
                if not self.GetFoldExpanded(line_num):
                    self.Expand(line_num, True)

    def FindLexer(self, set_ext=u''):
        """Sets Text Controls Lexer Based on File Extension
        @param set_ext: explicit extension to use in search
        @postcondition: lexer is configured for file

        """
        if not self._config['highlight']:
            return 2

        super(EditraStc, self).FindLexer(set_ext)

        # Configure Autocompletion
        # NOTE: must be done after syntax configuration
        if self._config['autocomp']:
            self.ConfigureAutoComp()
        return 0

    def ControlDispatch(self, evt):
        """Dispatches events caught from the mainwindow to the
        proper functions in this module.
        @param evt: event that was posted to this handler

        """
        e_id = evt.GetId()
        e_obj = evt.GetEventObject()
        e_map = { ed_glob.ID_COPY  : self.Copy, ed_glob.ID_CUT  : self.Cut,
                  ed_glob.ID_PASTE : self.Paste, ed_glob.ID_UNDO : self.Undo,
                  ed_glob.ID_REDO  : self.Redo, ed_glob.ID_INDENT : self.Tab,
                  ed_glob.ID_REVERT_FILE : self.RevertToSaved,
                  ed_glob.ID_CUT_LINE : self.LineCut,
                  ed_glob.ID_DELETE_LINE : self.LineDelete,
                  ed_glob.ID_COLUMN_MODE : self.ToggleColumnMode,
                  ed_glob.ID_COPY_LINE : self.LineCopy,
                  ed_glob.ID_DUP_LINE : self.LineDuplicate,
                  ed_glob.ID_BRACKETHL : self.ToggleBracketHL,
                  ed_glob.ID_SYNTAX : self.SyntaxOnOff,
                  ed_glob.ID_UNINDENT : self.BackTab,
                  ed_glob.ID_TRANSPOSE : self.LineTranspose,
                  ed_glob.ID_LINE_MOVE_UP : self.LineMoveUp,
                  ed_glob.ID_LINE_MOVE_DOWN : self.LineMoveDown,
                  ed_glob.ID_SELECTALL: self.SelectAll,
                  ed_glob.ID_FOLDING : self.FoldingOnOff,
                  ed_glob.ID_SHOW_LN : self.ToggleLineNumbers,
                  ed_glob.ID_TOGGLECOMMENT : self.ToggleComment,
                  ed_glob.ID_AUTOINDENT : self.ToggleAutoIndent,
                  ed_glob.ID_LINE_AFTER : self.AddLine,
                  ed_glob.ID_TOGGLE_FOLD : self.ToggleFold,
                  ed_glob.ID_TOGGLE_ALL_FOLDS : self.FoldAll,
                  ed_glob.ID_TRIM_WS : self.TrimWhitespace,
                  ed_glob.ID_MACRO_START : self.StartRecord,
                  ed_glob.ID_MACRO_STOP : self.StopRecord,
                  ed_glob.ID_MACRO_PLAY : self.PlayMacro,
                  ed_glob.ID_GOTO_MBRACE : self.GotoBraceMatch,
                  ed_glob.ID_SHOW_AUTOCOMP : self.DoAutoComplete,
                  ed_glob.ID_SHOW_CALLTIP : self.DoCallTip
        }

        e_idmap = { ed_glob.ID_ZOOM_OUT : self.DoZoom,
                    ed_glob.ID_ZOOM_IN  : self.DoZoom,
                    ed_glob.ID_ZOOM_NORMAL : self.DoZoom,
                    ed_glob.ID_EOL_MAC  : self.ConvertLineMode,
                    ed_glob.ID_EOL_UNIX : self.ConvertLineMode,
                    ed_glob.ID_EOL_WIN  : self.ConvertLineMode,
                    ed_glob.ID_SPACE_TO_TAB : self.ConvertWhitespace,
                    ed_glob.ID_TAB_TO_SPACE : self.ConvertWhitespace,
                    ed_glob.ID_NEXT_MARK : self.Bookmark,
                    ed_glob.ID_PRE_MARK  : self.Bookmark,
                    ed_glob.ID_ADD_BM    : self.Bookmark,
                    ed_glob.ID_DEL_ALL_BM : self.Bookmark}

        # Hide autocomp popups
        self.HidePopups()

        if e_obj.GetClassName() == "wxToolBar" or e_id in e_map:
            if e_id in e_map:
                e_map[e_id]()
            return

        if e_id in e_idmap:
            e_idmap[e_id](e_id)
        elif e_id == ed_glob.ID_SHOW_EDGE:
            self.SetViewEdgeGuide(not self.GetEdgeMode())
        elif e_id == ed_glob.ID_SHOW_EOL:
            self.SetViewEOL(not self.GetViewEOL())
        elif e_id == ed_glob.ID_PASTE_AFTER:
            cpos = self.GetCurrentPos()
            self.Paste()
            self.SetCurrentPos(cpos)
            self.SetSelection(cpos, cpos)
        elif e_id == ed_glob.ID_SHOW_WS:
            self.SetViewWhiteSpace(not self.GetViewWhiteSpace())
        elif e_id == ed_glob.ID_WORD_WRAP:
            self.SetWrapMode(not self.GetWrapMode())
        elif e_id == ed_glob.ID_JOIN_LINES:
            self.LinesJoinSelected()
        elif e_id == ed_glob.ID_INDENT_GUIDES:
            self.SetIndentationGuides(not bool(self.GetIndentationGuides()))
        elif e_id == ed_glob.ID_HLCARET_LINE:
            self.SetCaretLineVisible(not self.GetCaretLineVisible())
        elif e_id in syntax.SYNTAX_IDS:
            f_ext = syntax.GetExtFromId(e_id)
            self.LOG("[ed_stc][evt] Manually Setting Lexer to %s" % str(f_ext))
            self.FindLexer(f_ext)
        elif e_id == ed_glob.ID_AUTOCOMP:
            self.SetAutoComplete(not self.GetAutoComplete())
        elif e_id == ed_glob.ID_LINE_BEFORE:
            self.AddLine(before=True)
        elif e_id in [ed_glob.ID_TO_UPPER, ed_glob.ID_TO_LOWER]:
            self.ConvertCase(e_id == ed_glob.ID_TO_UPPER)
        elif e_id == ed_glob.ID_USE_SOFTTABS:
            self.SetUseTabs(not self.GetUseTabs())
        else:
            evt.Skip()

    def CheckEOL(self):
        """Checks the EOL mode of the opened document. If the mode
        that the document was saved in is different than the editors
        current mode the editor will switch modes to preserve the eol
        type of the file, if the eol chars are mixed then the editor
        will toggle on eol visibility.
        @postcondition: eol mode is configured to best match file
        @todo: Is showing line endings the best way to show mixed?

        """
        mixed = diff = False
        eol_map = {u"\n" : wx.stc.STC_EOL_LF,
                   u"\r\n" : wx.stc.STC_EOL_CRLF,
                   u"\r" : wx.stc.STC_EOL_CR}

        eol = unichr(self.GetCharAt(self.GetLineEndPosition(0)))
        if eol == u"\r":
            tmp = unichr(self.GetCharAt(self.GetLineEndPosition(0) + 1))
            if tmp == u"\n":
                eol += tmp

        # Is the eol used in the document the same as what is currently set.
        if eol != self.GetEOLChar():
            diff = True

        # Check the lines to see if they are all matching or not.
        LEPFunct = self.GetLineEndPosition
        GCAFunct = self.GetCharAt
        for line in range(self.GetLineCount() - 1):
            end = LEPFunct(line)
            tmp = unichr(GCAFunct(end))
            if tmp == u"\r":
                tmp2 = unichr(GCAFunct(LEPFunct(0) + 1))
                if tmp2 == u"\n":
                    tmp += tmp2
            if tmp != eol:
                mixed = True
                break

        if mixed or diff:
            if mixed:
                # Warn about mixed end of line characters and offer to convert
                msg = _("Mixed EOL characters detected.\n\n"
                        "Would you like to format them to all be the same?")
                dlg = ed_mdlg.EdFormatEOLDlg(self.GetTopLevelParent(), msg,
                                             _("Format EOL?"),
                                             eol_map.get(eol, self.GetEOLMode()))

                if dlg.ShowModal() == wx.ID_YES:
                    sel = dlg.GetSelection()
                    self.ConvertEOLs(sel)
                    super(EditraStc, self).SetEOLMode(sel)
                dlg.Destroy()
            else:
                # The end of line character is different from the preferred
                # user setting for end of line. So change our eol mode to
                # preserve that of what the document is using.
                mode = eol_map.get(eol, wx.stc.STC_EOL_LF)
                super(EditraStc, self).SetEOLMode(mode)
        else:
            pass

    def ConvertLineMode(self, mode_id):
        """Converts all line endings in a document to a specified
        format.
        @param mode_id: (menu) id of eol mode to set

        """
        eol_map = { ed_glob.ID_EOL_MAC  : wx.stc.STC_EOL_CR,
                    ed_glob.ID_EOL_UNIX : wx.stc.STC_EOL_LF,
                    ed_glob.ID_EOL_WIN  : wx.stc.STC_EOL_CRLF
                  }
        self.ConvertEOLs(eol_map[mode_id])
        super(EditraStc, self).SetEOLMode(eol_map[mode_id])

    def ConvertWhitespace(self, mode_id):
        """Convert whitespace from using tabs to spaces or visa versa
        @param mode_id: id of conversion mode

        """
        if mode_id not in (ed_glob.ID_TAB_TO_SPACE, ed_glob.ID_SPACE_TO_TAB):
            return
        tabw = self.GetIndent()
        pos = self.GetCurrentPos()
        sel = self.GetSelectedText()
        if mode_id == ed_glob.ID_TAB_TO_SPACE:
            cmd = (u"\t", u" " * tabw)
            tabs = False
        else:
            cmd = (" " * tabw, u"\t")
            tabs = True

        if sel != wx.EmptyString:
            self.ReplaceSelection(sel.replace(cmd[0], cmd[1]))
        else:
            self.BeginUndoAction()
            part1 = self.GetTextRange(0, pos).replace(cmd[0], cmd[1])
            tmptxt = self.GetTextRange(pos, self.GetLength()).replace(cmd[0], \
                                                                      cmd[1])
            self.SetText(part1 + tmptxt)
            self.GotoPos(len(part1))
            self.SetUseTabs(tabs)
            self.EndUndoAction()

    def GetCurrentLineNum(self):
        """Return the number of the line that the caret is currently at
        @return: Line number (int)

        """
        return self.LineFromPosition(self.GetCurrentPos())

    def GetEOLModeId(self):
        """Gets the id of the eol format. Convenience for updating
        menu ui.
        @return: id of the eol mode of this document

        """
        eol_map = { wx.stc.STC_EOL_CR : ed_glob.ID_EOL_MAC,
                    wx.stc.STC_EOL_LF : ed_glob.ID_EOL_UNIX,
                    wx.stc.STC_EOL_CRLF : ed_glob.ID_EOL_WIN
                  }
        return eol_map.get(self.GetEOLMode(), ed_glob.ID_EOL_UNIX)

    def IsBracketHlOn(self):
        """Returns whether bracket highlighting is being used by this
        control or not.
        @return: status of bracket highlight activation

        """
        return self._config['brackethl']

    def IsFoldingOn(self):
        """Returns whether code folding is being used by this
        control or not.
        @return: whether folding is on or not

        """
        return self._config['folding']

    def IsHighlightingOn(self):
        """Returns whether syntax highlighting is being used by this
        control or not.
        @return: whether syntax highlighting is on or not

        """
        return self._config['highlight']

    def IsLoading(self):
        """Is a background thread loading the text into the file
        @return: bool

        """
        # NOTE: keep the getattr check here some cases
        #       are reporting a yet unexplainable AttributeError here
        return getattr(self, '_loading', None) is not None

    def IsRecording(self):
        """Returns whether the control is in the middle of recording
        a macro or not.
        @return: whether recording macro or not

        """
        return self.recording

    def GetSelectionLineStartEnd(self):
        """Get the start and end positions of the lines in the current
        fuzzy selection.
        @return: tuple (int, int)

        """
        sline = self.LineFromPosition(self.GetSelectionStart())
        eline = self.LineFromPosition(self.GetSelectionEnd())
        last_line = self.GetLineCount() - 1
        eol_len = len(self.GetEOLChar())
        if sline < eline:
            tstart = self.GetLineStartPosition(sline)
            tend = self.GetLineEndPosition(eline)
        else:
            tstart = self.GetLineStartPosition(eline)
            tend = self.GetLineEndPosition(sline)

        if eline == last_line and tstart != 0:
            tstart -= eol_len
        else:
            tend += eol_len

        return (max(tstart, 0), min(tend, self.GetLength()))

    def LineCut(self): # pylint: disable-msg=W0221
        """Cut the selected lines into the clipboard"""
        start, end = self.GetSelectionLineStartEnd()
        self.SetSelection(start, end)
        self.BeginUndoAction()
        self.Cut()
        self.EndUndoAction()

    def LineDelete(self): # pylint: disable-msg=W0221
        """Delete the selected lines without modifying the clipboard"""
        start, end = self.GetSelectionLineStartEnd()
        self.SetTargetStart(start)
        self.SetTargetEnd(end)
        self.BeginUndoAction()
        self.ReplaceTarget(u'')
        self.EndUndoAction()

    def LinesJoin(self): # pylint: disable-msg=W0221
        """Join lines in target and compress whitespace
        @note: overrides default function to allow for leading
               whitespace in joined lines to be compressed to 1 space

        """
        sline = self.LineFromPosition(self.GetTargetStart())
        eline = self.LineFromPosition(self.GetTargetEnd())
        if not eline:
            eline = 1
        lines = list()
        for line in xrange(sline, eline + 1):
            if line != sline:
                tmp = self.GetLine(line).strip()
            else:
                tmp = self.GetLine(line)
                if not tmp.isspace():
                    tmp = tmp.rstrip()
                else:
                    tmp = tmp.replace("\n", u'').replace("\r", u'')
            if len(tmp):
                lines.append(tmp)
        self.SetTargetStart(self.PositionFromLine(sline))
        self.SetTargetEnd(self.GetLineEndPosition(eline))
        self.ReplaceTarget(u' '.join(lines))

    def LinesJoinSelected(self):
        """Similar to LinesJoin, but operates on selection
        @see: LinesJoin

        """
        self.SetTargetStart(self.GetSelectionStart())
        self.SetTargetEnd(self.GetSelectionEnd())
        self.LinesJoin()

    def LineMoveUp(self):
        """Move the current line up"""
        linenum = self.GetCurrentLine()
        if linenum > 0 :
            self.BeginUndoAction()
            self.LineTranspose()
            self.LineUp()
            self.EndUndoAction()

    def LineMoveDown(self):
        """Move the current line down"""
        linenum = self.GetCurrentLine()
        col = self.GetColumn(self.GetCurrentPos())
        if linenum < self.GetLineCount() - 1:
            self.BeginUndoAction()
            self.LineDown()
            self.LineTranspose()
            self.GotoColumn(col)
            self.EndUndoAction()

    def LineTranspose(self): # pylint: disable-msg=W0221
        """Switch the current line with the previous one
        @note: overrides base stc method to do transpose in single undo action

        """
        self.BeginUndoAction()
        super(EditraStc, self).LineTranspose()
        self.EndUndoAction()

    def SetAutoComplete(self, value):
        """Turns Autocompletion on and off
        @param value: use autocomp or not
        @type value: bool

        """
        if isinstance(value, bool):
            self._config['autocomp'] = value
            if value:
                self.InitCompleter()

    def SetEOLMode(self, mode):
        """Sets the EOL mode from a string description
        @param mode: eol mode to set
        @note: overrides StyledTextCtrl.SetEOLMode

        """
        mode_map = { EDSTC_EOL_CR   : wx.stc.STC_EOL_CR,
                     EDSTC_EOL_LF   : wx.stc.STC_EOL_LF,
                     EDSTC_EOL_CRLF : wx.stc.STC_EOL_CRLF
                   }

        mode = mode_map.get(mode, wx.stc.STC_EOL_LF)
        super(EditraStc, self).SetEOLMode(mode)

    def SetViEmulationMode(self, use_vi, use_normal=False):
        """Activate/Deactivate Vi emulation mode
        @param use_vi: Turn vi emulation on/off
        @type use_vi: boolean
        @keyword use_normal: Start in normal mode
        @type use_normal: boolean

        """
        self.key_handler.ClearMode()
        if use_vi:
            self.key_handler = ViKeyHandler(self, use_normal)
        else:
            self.key_handler = KeyHandler(self)

    def SetViewEdgeGuide(self, switch=None):
        """Toggles the visibility of the edge guide
        @keyword switch: force a particular setting

        """
        if (switch is None and not self.GetEdgeMode()) or switch:
            self.SetEdgeColumn(_PGET("EDGE", 'int', 80))
            self.SetEdgeMode(wx.stc.STC_EDGE_LINE)
        else:
            self.SetEdgeMode(wx.stc.STC_EDGE_NONE)

    def StartRecord(self): # pylint: disable-msg=W0221
        """Starts recording all events
        @return: None

        """
        self.recording = True
        evt = ed_event.StatusEvent(ed_event.edEVT_STATUS, self.GetId(),
                                   _("Recording Macro") + u"...",
                                   ed_glob.SB_INFO)
        wx.PostEvent(self.GetTopLevelParent(), evt)
        super(EditraStc, self).StartRecord()

    def StopRecord(self): # pylint: disable-msg=W0221
        """Stops the recording and builds the macro script
        @postcondition: macro recording is stopped

        """
        self.recording = False
        super(EditraStc, self).StopRecord()
        evt = ed_event.StatusEvent(ed_event.edEVT_STATUS, self.GetId(),
                                   _("Recording Finished"),
                                   ed_glob.SB_INFO)
        wx.PostEvent(self.GetTopLevelParent(), evt)
        self._BuildMacro()

    def TrimWhitespace(self):
        """Trims trailing whitespace from all lines in the document.
        @postcondition: all trailing whitespace is removed from document

        """
        cpos = self.GetCurrentPos()
        cline = self.GetCurrentLine()
        cline_len = len(self.GetLine(cline))
        epos = cline_len - (self.GetLineEndPosition(cline) - cpos)

        # Begin stripping trailing whitespace
        self.BeginUndoAction()
        for line in xrange(self.GetLineCount()):
            eol = u''
            tmp = self.GetLine(line)

            # Scintilla stores text in utf8 internally so we need to
            # encode to utf8 to get the correct length of the text.
            try:
                tlen = len(tmp.encode('utf-8'))
            except:
                tlen = len(tmp)

            if tlen:
                if "\r\n" in tmp:
                    eol = "\r\n"
                elif "\n" in tmp:
                    eol = "\n"
                else:
                    eol = tmp[-1]

                if not eol.isspace():
                    continue
                elif eol in u' \t':
                    eol = u''
            else:
                continue

            # Strip the extra whitespace from the line
            end = self.GetLineEndPosition(line) + len(eol)
            start = max(end - tlen, 0)
            self.SetTargetStart(start)
            self.SetTargetEnd(end)
            rtxt = tmp.rstrip() + eol
            if rtxt != self.GetTextRange(start, end):
                self.ReplaceTarget(tmp.rstrip() + eol)
        self.EndUndoAction()

        # Restore carat position
        cline_len = len(self.GetLine(cline))
        end = self.GetLineEndPosition(cline)
        if epos >= cline_len:
            epos = end
        else:
            start = max(end - cline_len, 0)
            epos += start

        if epos != cpos:
            self.GotoPos(epos)

    def FoldingOnOff(self, switch=None):
        """Turn code folding on and off
        @keyword switch: force a particular setting

        """
        if (switch is None and not self._config['folding']) or switch:
            self.LOG("[ed_stc][evt] Code Folding Turned On")
            self._config['folding'] = True
            self.SetMarginWidth(ed_basestc.FOLD_MARGIN, 12)
            self.SetProperty("fold", "1")
        else:
            self.LOG("[ed_stc][evt] Code Folding Turned Off")
            self._config['folding'] = False

            # Ensure all code blocks have been expanded
            self.ExpandAll()

            self.SetMarginWidth(ed_basestc.FOLD_MARGIN, 0)
            self.SetProperty("fold", "0")

    def SyntaxOnOff(self, switch=None):
        """Turn Syntax Highlighting on and off
        @keyword switch: force a particular setting

        """
        if (switch is None and not self._config['highlight']) or switch:
            self.LOG("[ed_stc][evt] Syntax Highlighting Turned On")
            self._config['highlight'] = True
            self.FindLexer()
        else:
            self.LOG("[ed_stc][evt] Syntax Highlighting Turned Off")
            self._config['highlight'] = False
            self.SetLexer(wx.stc.STC_LEX_NULL)
            self.ClearDocumentStyle()
            self.UpdateBaseStyles()
        return 0

    def Tab(self): # pylint: disable-msg=W0221
        """Override base method to ensure that folded blocks get unfolded
        prior to changing the indentation.

        """
        # TODO: unfolding of folded blocks during block indent
#        lines = list()
#        if self.HasSelection():
#            sel = self.GetSelection()
#            sline = self.LineFromPosition(sel[0])
#            eline = self.LineFromPosition(sel[1])
#            lines = range(sline, eline+1)
#        else:
#            cline = self.GetCurrentLine()
#            lines = [cline, cline+1]

#        for line_num in lines:
#            if self.GetFoldLevel(line_num) & wx.stc.STC_FOLDLEVELHEADERFLAG:
#                if not self.GetFoldExpanded(line_num):
#                    self.Expand(line_num, True)
        super(EditraStc, self).Tab()

    def ToggleAutoIndent(self, switch=None):
        """Toggles Auto-indent On and Off
        @keyword switch: force a particular setting

        """
        if (switch is None and not self._config['autoindent']) or switch:
            self._config['autoindent'] = True
        else:
            self._config['autoindent'] = False

    def ToggleBracketHL(self, switch=None):
        """Toggle Bracket Highlighting On and Off
        @keyword switch: force a particular setting

        """
        if (switch is None and not self._config['brackethl']) or switch:
            self.LOG("[ed_stc][evt] Bracket Highlighting Turned On")
            self._config['brackethl'] = True
            # Make sure to highlight a brace if next to on when turning it on
            self.DoBraceHighlight()
        else:
            self.LOG("[ed_stc][evt] Bracket Highlighting Turned Off")
            self._config['brackethl'] = False
            # Make sure that if there was a highlighted brace it gets cleared
            wx.CallAfter(self.BraceHighlight, -1, -1)

    def ToggleFold(self, lineNum=None):
        """Toggle the fold at the given line number. If lineNum is
        None then the fold closest cursors current postions.
        @keyword lineNum: int

        """
        if lineNum is None:
            lineNum = self.GetCurrentLine()
        super(EditraStc, self).ToggleFold(lineNum)

    @jumpaction
    def WordLeft(self): # pylint: disable-msg=W0221
        """Move caret to beginning of previous word
        @note: override builtin to include extra characters in word

        """
        self.SetWordChars(NONSPACE)
        super(EditraStc, self).WordLeft()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordLeft()
        self.SetWordChars('')

    def WordLeftExtend(self): # pylint: disable-msg=W0221
        """Extend selection to beginning of previous word
        @note: override builtin to include extra characters in word

        """
        self.SetWordChars(NONSPACE)
        super(EditraStc, self).WordLeftExtend()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordLeftExtend()
        self.SetWordChars('')

    @jumpaction
    def WordPartLeft(self): # pylint: disable-msg=W0221
        """Move the caret left to the next change in capitalization/punctuation
        @note: overrides default function to not count whitespace as words

        """
        super(EditraStc, self).WordPartLeft()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordPartLeft()

    def WordPartLeftExtend(self): # pylint: disable-msg=W0221
        """Extend selection left to the next change in c
        apitalization/punctuation.
        @note: overrides default function to not count whitespace as words

        """
        super(EditraStc, self).WordPartLeftExtend()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordPartLeftExtend()

    @jumpaction
    def WordPartRight(self): # pylint: disable-msg=W0221
        """Move the caret to the start of the next word part to the right
        @note: overrides default function to exclude white space

        """
        super(EditraStc, self).WordPartRight()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordPartRight()

    @jumpaction
    def WordPartRightEnd(self): # pylint: disable-msg=W0221
        """Move caret to end of next change in capitalization/punctuation
        @postcondition: caret is moved

        """
        super(EditraStc, self).WordPartRight()
        super(EditraStc, self).WordPartRight()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos - 1) in SPACECHARS:
            self.CharLeft()

    def WordPartRightEndExtend(self): # pylint: disable-msg=W0221
        """Extend selection to end of next change in capitalization/punctuation
        @postcondition: selection is extended

        """
        super(EditraStc, self).WordPartRightExtend()
        super(EditraStc, self).WordPartRightExtend()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos - 1) in SPACECHARS:
            self.CharLeftExtend()

    def WordPartRightExtend(self): # pylint: disable-msg=W0221
        """Extend selection to start of next change in 
        capitalization/punctuation
        @postcondition: selection is extended

        """
        super(EditraStc, self).WordPartRightExtend()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordPartRightExtend()

    @jumpaction
    def WordRight(self): # pylint: disable-msg=W0221
        """Move caret to beginning of next word
        @note: override builtin to include extra characters in word

        """
        self.SetWordChars(NONSPACE)
        super(EditraStc, self).WordRight()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordRight()
        self.SetWordChars('')

    @jumpaction
    def WordRightEnd(self): # pylint: disable-msg=W0221
        """Move caret to end of next change in word
        @note: override builtin to include extra characters in word

        """
        self.SetWordChars(NONSPACE)
        super(EditraStc, self).WordRightEnd()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos - 1) in SPACECHARS:
            super(EditraStc, self).WordRightEnd()
        self.SetWordChars('')

    def WordRightExtend(self): # pylint: disable-msg=W0221
        """Extend selection to beginning of next word
        @note: override builtin to include extra characters in word

        """
        self.SetWordChars(NONSPACE)
        super(EditraStc, self).WordRightExtend()
        cpos = self.GetCurrentPos()
        if self.GetTextRange(cpos, cpos + 1) in SPACECHARS:
            super(EditraStc, self).WordRightExtend()
        self.SetWordChars('')

    def LoadFile(self, path):
        """Load the file at the given path into the buffer. Returns
        True if no errors and False otherwise. To retrieve the errors
        check the last error that was set in the file object returned by
        L{GetDocument}.
        @param path: path to file

        """
        fsize = ebmlib.GetFileSize(path)
        if fsize < 1048576: # 1MB
            return super(EditraStc, self).LoadFile(path)
        else:
            ed_msg.PostMessage(ed_msg.EDMSG_FILE_OPENING, path)
            self.file.SetPath(path)
            self._loading = wx.BusyCursor()
            self.file.ReadAsync(self)
            return True

    def ReloadFile(self):
        """Reloads the current file, returns True on success and
        False if there is a failure.
        @return: whether file was reloaded or not
        @rtype: bool

        """
        cfile = self.GetFileName()
        if os.path.exists(cfile):
            try:
                self.BeginUndoAction()
                marks = self.GetBookmarks()
                cpos = self.GetCurrentPos()
                # TODO: Handle async re-loads of large files
                txt = self.File.Read()
                self.SetReadOnly(False)
                if txt is not None:
                    if self.File.IsRawBytes() and not ebmlib.IsUnicode(txt):
                        self.AddStyledText(txt)
                        self.SetReadOnly(True) # Don't allow editing of raw bytes
                    else:
                        self.SetText(txt)
                else:
                    return False, _("Failed to reload: %s") % cfile

                self.SetModTime(ebmlib.GetFileModTime(cfile))
                for mark in marks:
                    self.AddBookmark(mark)
                self.EndUndoAction()
                self.SetSavePoint()
            except (UnicodeDecodeError, AttributeError, OSError, IOError), msg:
                self.LOG("[ed_stc][err] Failed to Reload %s" % cfile)
                return False, msg
            else:
                self.GotoPos(cpos)
                context = self.GetTopLevelParent().GetId()
                ed_msg.PostMessage(ed_msg.EDMSG_FILE_OPENED,
                                   self.GetFileName(), context)
                return True, ''
        else:
            self.LOG("[ed_stc][err] %s does not exists, cant reload." % cfile)
            return False, _("%s does not exist") % cfile

    def RevertFile(self):
        """Revert all the changes made to the file since it was opened
        @postcondition: undo history is re-wound to initial state and file
                        is re-saved if it has an on disk file.

        """
        self.Freeze()
        while self.CanUndo():
            self.Undo()
        self.Thaw()

        fname = self.GetFileName()
        if len(fname):
            self.SaveFile(fname)

    def RevertToSaved(self):
        """Revert the current buffer back to the last save point"""
        self.Freeze()
        while self.CanUndo():
            if self.GetModify():
                self.Undo()
            else:
                break
        self.Thaw()

    def SaveFile(self, path):
        """Save buffers contents to disk
        @param path: path of file to save
        @return: whether file was written or not
        @rtype: bool

        """
        result = True
        try:
            tlw_id = self.GetTopLevelParent().GetId()
            ed_msg.PostMessage(ed_msg.EDMSG_FILE_SAVE,
                               (path, self.GetLangId()), tlw_id)
            self.File.SetPath(path)
            self.LOG("[ed_stc][info] Writing file %s, with encoding %s" % \
                     (path, self.File.GetEncoding()))

            if _PGET('AUTO_TRIM_WS', 'bool', False):
                self.TrimWhitespace()

            if self.File.IsReadOnly():
                wx.MessageBox(_("File is Read Only and cannot be saved"),
                              _("Read Only"),
                              style=wx.OK|wx.CENTER|wx.ICON_WARNING)
                return True
            else:
                if not self.File.IsRawBytes():
                    self.File.Write(self.GetText())
                else:
                    nchars = self.GetTextLength()
                    txt = self.GetStyledText(0, nchars)[0:nchars*2:2]
                    self.File.Write(txt)
        except Exception, msg:
            result = False
            self.LOG("[ed_stc][err] There was an error saving %s" % path)
            self.LOG("[ed_stc][err] ERROR: %s" % str(msg))

        if result:
            self.SetSavePoint()
            self.SetModTime(ebmlib.GetFileModTime(path))
            self.File.FireModified()
            self.SetFileName(path)

        wx.CallAfter(ed_msg.PostMessage,
                     ed_msg.EDMSG_FILE_SAVED,
                     (path, self.GetLangId()),
                     tlw_id)

        return result

    def ConfigureLexer(self, file_ext):
        """Sets Lexer and Lexer Keywords for the specified file extension
        @param file_ext: a file extension to configure the lexer from

        """
        super(EditraStc, self).ConfigureLexer(file_ext)

        if not self._config['folding']:
            self.SetProperty("fold", "0")

        # Notify that lexer has changed
        pid = self.TopLevelParent.Id
        self.LOG("[ed_stc][info] Lexer change notification for context %d" % pid)
        ed_msg.PostMessage(ed_msg.EDMSG_UI_STC_LEXER,
                           (self.GetFileName(), self.GetLangId()), pid)
        return True
