###############################################################################
# Name: ed_vim.py                                                             #
# Purpose: Vim emulation                                                      #
# Author: Hasan Aljudy                                                        #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Vim emulation class and helper methods to implement vim emulation in Editra's
text buffer.

"""

__author__ = "Hasan Aljudy"
__cvsid__ = "$Id: ed_vim.py 66867 2011-02-08 23:52:17Z CJP $"
__revision__ = "$Revision: 66867 $"

# ---------------------------------------------------------------------------- #
# Imports
import wx
import re
import string

# Local Imports
import ebmlib

# ---------------------------------------------------------------------------- #

# TODO:CJP maybe replace all repeatable methods with this decorator to reduce
#          repeated code. NOTE: may need to name different as the word repeat
#          is used in this module in many places.
#def repeat(funct, count=1):
#    def rwrap(*args, **kwargs):
#        for i in range(count):
#            funct()
#    return rwrap

# ---------------------------------------------------------------------------- #

def Parse(cmd, editor):
    """Parses a command and (if applicable) executes it on an editor
    @param cmd: the command string
    @param editor: a proxy object used to issue commands to the editor
    @return: boolean as a signal to the caller as to whether it should
             clear its command buffer.
             i.e. True if command is handled or invalid
             False if we need more input before we can execute the command

    """
    handled = True
    handler = GetHandler(cmd)
    if handler is None:
        if cmd.isdigit():
            handled = False
    else:
        need_more = DoHandle(handler, cmd, editor)
        if need_more:
            handled = False
    return handled

# ---------------------------------------------------------------------------- #

class EditraCommander(object):
    """Proxy object that sends commands to the editor

    The purpose of this class is to make vim support more abstract and portable
    so in theory one could add Vim Emulation to another editor using this module
    by just creating a commander object for it and re-implementing all of these
    methods defined below.

    The methods are based on turtle geometry. Everything is relative to the
    position of the cursor. Commands shouldn't take a position argument; they
    should all work on the current position of the cursor.

    """
    def __init__(self, keyprocessor):
        super(EditraCommander, self).__init__()

        # Attributes
        self.keyprocessor = keyprocessor
        self.stc = keyprocessor.stc
        self.LastChangeCommand = None
        self.LastInsertedText = None
        self.InsertRepeat = 1
        self.CaretStack = []
        self.ColumnStack = []
        self.ScrollStack = []
        self.LastFindChar = None
        self._Bookmarks = {}

    @property
    def STC(self):
        return self.stc

    def InsertMode(self):
        """Put editor in insert mode"""
        self.keyprocessor.InsertMode()

    def ReplaceMode(self):
        """Put editor in replace mode"""
        self.keyprocessor.ReplaceMode()

    def VisualMode(self):
        """Put editor in visual (selection) mode"""
        self.keyprocessor.VisualMode()

    def NormalMode(self):
        """Put editor in normal (command) mode"""
        self.keyprocessor.NormalMode()

    def IsNormalMode(self):
        return self.keyprocessor.IsNormalMode()

    def IsInsertMode(self):
        return self.keyprocessor.IsInsertMode()

    def InsertRepetition(self):
        """Repeat last inserted text according to the value that was set
        in SetInsertRepeat

        """
        if self.InsertRepeat > 1:
            self.RepeatChangeCommand(self.InsertRepeat - 1)
            self.InsertRepeat = 1

    def IsAtLineEnd(self):
        lnum = self.stc.GetCurrentLine()
        epos = self.stc.GetLineEndPosition(lnum)
        return epos == self._GetPos()

    def IsAtLineStart(self):
        """Is the cursor currently at the start of a line
        @return: bool

        """
        return self._GetCol() == 0

    def Undo(self, repeat):
        """Undo actions in the buffer
        @param repeat: int

        """
        for i in range(repeat):
            self.stc.Undo()

    def Redo(self, repeat):
        """Redo actions in the buffer
        @param repeat: int

        """
        for i in range(repeat):
            self.stc.Redo()

    def _GetPos(self):
        """Get caret position"""
        return self.stc.GetCurrentPos()

    def _SetPos(self, pos):
        """Set caret position"""
        self.stc.GotoPos(pos)

    def SelectLines(self, repeat):
        """Select specified number of lines starting with current line
        and going down
        @param repeat: int

        """
        cline = self.stc.GetCurrentLine()
        lline = self.stc.GetLineCount() - 1
        self.GotoLineStart()
        if cline == lline:
            cpos = self.stc.GetCurrentPos() - len(self.stc.GetEOLChar())
            cpos = max(0, cpos)
            self.stc.GotoPos(cpos)
        self.PushCaret()
        self.MoveDown(repeat)
        self.StartSelection()
        self.PopCaret()
        self.EndSelection()

    def PushCaret(self):
        """Pushes caret position for later restoration"""
        self.CaretStack += [self._GetPos()]

    def PopCaret(self, restore=True):
        """If no parameters are sent, restores caret position. If on the
        other hand, the restore keyword is set to false, the caret is not
        restored, but the last pushed value is discarded
        @keyword restore: bool

        """
        pos = self.CaretStack[-1]
        self.CaretStack = self.CaretStack[:-1]
        if restore:
            self._SetPos(pos)

    def PushColumn(self):
        """Push column position for later restoration"""
        self.ColumnStack += [self._GetCol()]

    def PopColumn(self, restore=True):
        """Pop caret position, and optionally discard it
        @keyword restore: if set to False, column will be discarded

        """
        column = self.ColumnStack[-1]
        self.ColumnStack = self.ColumnStack[:-1]
        if restore:
            self.GotoColumn(column)

    def PushScroll(self):
        """Push current scrolling state for later restoration"""
        self.ScrollStack += [self.stc.GetFirstVisibleLine()]

    def PopScroll(self, restore=True):
        """If not parameters are sent, restores the scrolling state. If on the
        other hadn, the restore keywords it set to false, then the last pushed
        scroll state is discarded
        @keyword restore: bool

        """
        line = self.ScrollStack[-1]
        self.ScrollStack[:-1]
        if restore:
            self.stc.ScrollToLine(line)

    def _IsLine(self, text):
        """Check if text ends with EOL
        @param text: string

        """
        eol = self.stc.GetEOLChar()
        eol_len = len(eol)
        if len(text) > eol_len:
            return text[-len(eol):] == eol
        else:
            return False

    def _GetCol(self):
        """Get the X position of the caret, aka column"""
        return self.stc.GetColumn(self._GetPos())

    def _SetCol(self, col):
        """Set the X position of the caret
        @param col: int

        """
        self.stc.GotoColumn(col)

    def GotoColumn(self, column):
        """Goto the specified column number on the current line"""
        self._SetCol(column)
        self.stc.ChooseCaretX()

    def StartSelection(self):
        """Record the starting place for selection
        Nothing is sent to the actual text control, just some internal
        state is recorded.
        @see: EndSelection

        """
        self._Anchor = self._GetPos()

    def EndSelection(self):
        """Set the selection to start from the starting position as it was set
        by StartSelection to the current caret position

        It doesn't matter whether the starting position is before or after the
        ending position

        """
        self.stc.SetSelection(self._Anchor, self._GetPos())

    def ExtendSelection(self):
        """If the selection was already ended, and the caret moved, this method
        will extend the selection to the new caret position.

        """
        self.EndSelection() # happens to be the same implementation ..

    def Deselect(self):
        """Unselect. Puts caret to the (visual) start of selection"""
        start, end = self._GetSelectionRange()
        self._SetPos(start)
        self.StartSelection()
        self.EndSelection()

    def GotoLine(self, line):
        """Goto the start of indentation of given line"""
        # PostionFromLine returns the position past the last character on the
        # line. So need to look at previous line to get the correct position.
        pos = self.stc.PositionFromLine(max(0,line-1))
        self._SetPos(pos)
        self.GotoIndentStart()

    def GotoLastLine(self):
        self.GotoLine(self.stc.GetLineCount())

    def GotoLineStart(self):
        """Goto the beginning of the current line"""
        self.stc.Home()

    def GotoIndentStart(self):
        """Goto to the first non-space character on the current line"""
        self.stc.GotoIndentPos()

    def GotoLineEnd(self):
        """Goto the end of the current line"""
        self.stc.LineEnd()

    def OpenLine(self):
        """Open a new line below the current line"""
        # TODO: watch out if AutoIndent gets "fixed" so that it doesn't
        #       open a new line by itself!
        self.GotoLineEnd()
        self.stc.AutoIndent()

    def OpenLineUp(self):
        """Open a new line above the current line"""
        self.MoveUp()
        self.OpenLine()

    def BeginUndoAction(self):
        self.stc.BeginUndoAction()

    def EndUndoAction(self):
        self.stc.EndUndoAction()

    def MoveUp(self, repeat=1):
        """Move caret up
        @keyword repeat: int

        """
        for i in range(repeat):
            self.stc.LineUp()

    def MoveDown(self, repeat=1):
        """Move caret down
        @keyword repeat: int

        """
        for i in range(repeat):
            self.stc.LineDown()

    def MoveRight(self, repeat=1):
        """Move position to the right
        @keyword repeat: int

        """
        self.stc.MoveCaretPos(repeat)

    def MoveLeft(self, repeat=1):
        """Move the caret postion to the left
        @keyword repeat: int

        """
        self.stc.MoveCaretPos(-repeat)

    def MoveForward(self, repeat=1):
        """Move the caret position to the right"""
        self.MoveRight(repeat)

    def MoveBack(self, repeat=1):
        """Move the caret position to the left"""
        self.MoveLeft(repeat)

    def NextWord(self, repeat=1):
        """Move to beginning of next word"""
        for i in range(repeat):
            self.stc.WordRight()

    def NextWordBig(self, repeat=1):
        """Move to beginning of next Word (words are separated by space)"""
        # TODO:CJP Test on empty document, possible error condition
        for i in range(repeat):
            self.stc.WordRight()
            while self.GetChar(-1) and not self.GetChar(-1).isspace():
                self.stc.WordRight()

    def WordEnd(self, repeat=1):
        """Move to end of this word"""
        for i in range(repeat):
            self.stc.WordRightEnd()

    def WordEndBig(self, repeat=1):
        """Move to end of this Word (words are separated by space)"""
        # TODO:CJP Test on empty document, possible error condition
        for i in range(repeat):
            self.stc.WordRightEnd()
            while self.GetChar() and not self.GetChar().isspace():
                self.stc.WordRightEnd()

    def BackWord(self, repeat=1):
        """Move back to start of word"""
        for i in range(repeat):
            self.stc.WordLeft()

    def BackWordBig(self, repeat=1):
        """Move back to start of Word (words are separated by space)"""
        # TODO:CJP Test on empty document, possible error condition
        for i in range(repeat):
            self.stc.WordLeft()
            while self.GetChar(-1) and not self.GetChar(-1).isspace():
                self.stc.WordLeft()

    def BackWordPart(self, repeat=1):
        """Move back to start of word part"""
        for i in range(repeat):
            self.stc.WordPartLeft()

    def WordPartEnd(self, repeat=1):
        """Move to the end of the word part"""
        for i in range(repeat):
            self.stc.WordPartRight()

    def ParaUp(self, repeat=1):
        """Move the caret one paragraph up"""
        for i in range(repeat):
            self.stc.ParaUp()

    def ParaDown(self, repeat=1):
        """Move the caret one paragraph down"""
        for i in range(repeat):
            self.stc.ParaDown()

    def RegexSearch(self, text, back=False):
        """Search for a regex expression
        @param text: search string
        @keyword back: seach in reverse

        """
        # TODO:CJP using the StyledTextCtrl's regex searching is not as
        #          powerful as using Editras SearchEngine.
        self.stc.SearchText(text, regex=True, back=back)
        if back and self.HasSelection():
            # Let the caret pos be at the end of the selection
            start, end = self._GetSelectionRange()
            self.stc.SetSelection(start, end)

    def GotoScreenTop(self):
        """Goto first visible line"""
        self.stc.GotoIndentPos(self.stc.GetFirstVisibleLine())

    def GotoScreenMiddle(self):
        """Goto to the middle line on the screen"""
        self.stc.GotoIndentPos(self.stc.GetMiddleVisibleLine())

    def GotoScreenLast(self):
        """Goto last visible line"""
        self.stc.GotoIndentPos(self.stc.GetLastVisibleLine())

    def _EnsureCaretVisible(self):
        """Smartly scroll view to make caret visible if it's out of view"""
        first = self.stc.GetFirstVisibleLine()
        lines = self.stc.LinesOnScreen()
        current = self.stc.GetCurrentLine()
        if current in range(first, first + lines):
            # supposedly visible! XXX: doesn't account for folding
            # TODO:CJP to ensure caret is visible call EnsureCaretVisible
            return None

        if current < first:
            amt = 0.7
        else:
            amt = 0.3
        self._Scroll(amt)

    def _Scroll(self, amt):
        """Scroll to the current line
        @param amt: number between 0 and 1 signifying percentage
        @example: _Scroll(0.5) scrolls the current line to the middle
                 of the screen

        """
        lines = self.stc.LinesOnScreen() - 1
        current = self.stc.GetCurrentLine()
        diff = int(lines * amt)
        self.stc.ScrollToLine(current - diff)

    def ScrollTop(self):
        """Scroll such that the current line is the first visible line"""
        self._Scroll(0)

    def ScrollMiddle(self):
        """Scroll such that the current line is in the middle of the screen"""
        self._Scroll(0.5)

    def ScrollBottom(self):
        """Scroll such that the current line is the last visible line"""
        self._Scroll(1)

    def _SelectIdentifierUnderCaret(self):
        """If nothing is selected, select the identifier under caret, otherwise
        keeps selection as is.
        @return: selected text

        """
        ident = self.GetSelectedText()
        if ident:
            return ident

        self.WordEnd()
        self.StartSelection()
        self.BackWord()
        self.EndSelection()
        ident = self.GetSelectedText()
        return ident

    def _NextIdent(self, repeat=1, back=False):
        """Jump to the next (or previous) occurance of identifier
        under caret or selected text.

        @note: Holds common code between NextIdent and PrevIdent
        @see: NextIdent
        @see: PrevIdent

        """
        ident = self._SelectIdentifierUnderCaret()
        self.PushCaret()
        self.PushScroll()
        if not ident:
            return

        for i in range(repeat):
            if back:
                self._GotoSelectionStart()
            else:
                self._GotoSelectionEnd()

            result = self.stc.SearchText(ident, back=back)
            if result == -1:
                break

        self.PopCaret(result == -1)
        self.PopScroll()
        self._EnsureCaretVisible()

    def NextIdent(self, repeat=1):
        """Find next occurance of identifier under cursor"""
        self._NextIdent(repeat)

    def PrevIdent(self, repeat=1):
        """Find the previous occurrence of identifier under cursor"""
        self._NextIdent(repeat, back=True)

    def InsertText(self, text):
        """Insert some text at the current caret position and advance caret
        The inserted text is remembered so it can be repeated.
        @param text: string

        """
        self.keyprocessor.InsertText(self._GetPos(), text)
        self.MoveForward(len(text))

    def SetInsertRepeat(self, repeat):
        """Set how many times to repeat the last inserted text when
        InsertRepetition is called.

        """
        self.InsertRepeat = repeat

    def SetLastChangeCommand(self, cmd):
        """Rememer this command as the last change cmmand so that it gets
        repeated when RepeatChangeCommand is called.

        """
        self.LastChangeCommand = cmd

    def SetLastInsertedText(self, text):
        """Set this as the last inserted text so it gets repeated when
        InsertRepetition is called.

        """
        self.LastInsertedText = text

    def RepeatChangeCommand(self, repeat=1):
        """Repeat the last change command as set by SetLastChangeCommand"""
        if not self.LastChangeCommand:
            return

        self.BeginUndoAction()
        for i in range(repeat):
            self.LastChangeCommand()
            self.InsertText(self.LastInsertedText)
        self.EndUndoAction()

    def GetChar(self, dist=1):
        """Get the character (or text) under caret
        @param dist: length of text to get, negative values can be used to get
            the text before the caret

        """
        pos = self._GetPos()
        start, end = minmax(pos, pos + dist)
        start = max(0, start)
        end = min(end, self.stc.GetLength())
        return self.stc.GetTextRange(start, end)

    def GetSelectedText(self):
        """Get the selected text
        @rtype: string

        """
        return self.stc.GetSelectedText()

    def HasSelection(self):
        """Detects if there's anything selected
        @rtype: bool

        """
        return len(self.GetSelectedText()) > 0

    def SetRegister(self, reg='"'):
        """Set the current working clipboard (aka register) to given name"""
        ebmlib.Clipboard.Switch(reg)

    def YankSelection(self):
        """Copy the current selection to the clipboard"""
        ebmlib.Clipboard.Set(self.GetSelectedText())

    def DeleteSelection(self):
        """Yank selection and delete it"""
        start, end = self._GetSelectionRange()
        self.stc.BeginUndoAction()
        self.YankSelection()
        self.stc.Clear()
        self._SetPos(start)
        self.stc.EndUndoAction()

    def ChangeSelection(self):
        """Yank selection, delete it, and enter insert mode"""
        # HACK: need to adjust selection behavior for change command
        #       to better match vi behavior by not including trailing
        #       whitespace in the selection from the motion.
        stext = self.STC.GetSelectedText()
        clen = len(stext)
        slen = len(stext.rstrip())
        if slen < clen and slen > 0:
            start, end = self._GetSelectionRange()
            new_end = end - (clen - slen)
            self.STC.SetSelectionStart(start)
            self.STC.SetSelectionEnd(new_end)
        self.DeleteSelection()
        self.InsertMode()

    def Put(self, before=False, repeat=1):
        """Paste text

        If there's selection, the pasted text replaces it

        If text to be pasted ends with EOL it's assumed to be a line, and is
        pasted on a new line below the current line, or if the before keyword
        is set to True, then it's pasted on a new line above the current line

        """
        text = ebmlib.Clipboard.Get()
        if not text:
            return

        bIsLine = self._IsLine(text)
        if bIsLine:
            self.GotoLineStart()
            if not before:
                self.MoveDown()

        self.BeginUndoAction()
        if self.HasSelection():
            # paste over selection, if any
            self.stc.Clear()
        elif not bIsLine:
            self.stc.CharRight()

        for i in range(repeat):
            self.InsertText(text)
        self.EndUndoAction()

    def InvertCase(self, repeat):
        """Invert the case of the following characters"""
        if not self.HasSelection():
            for i in range(repeat):
                self.StartSelection()
                self.MoveForward()
                self.EndSelection()
        self.stc.InvertCase()
        self.Deselect()

    def JoinLines(self, repeat):
        """Join lines into a single line.
        @param repeat: number of lines below the current line to join with

        """
        self.SelectLines(repeat)
        self.stc.LinesJoinSelected()

    def _DeleteChars(self, repeat, back=False):
        """Delete characters under caret.
        @keyword back: If set to true, delete character before the caret

        """
        if back:
            move = self.MoveBack
        else:
            move = self.MoveForward

        self.BeginUndoAction()
        self.StartSelection()
        move(repeat)
        self.EndSelection()
        self.DeleteSelection()
        self.EndUndoAction()

    def DeleteForward(self, repeat):
        """Delete characters after the caret"""
        self._DeleteChars(repeat)

    def DeleteBackward(self, repeat):
        """Delete characters before the caret"""
        self._DeleteChars(repeat, back=True)

    def ReplaceChar(self, char, repeat):
        """Replace the next character under cursor with the given char"""
        self.BeginUndoAction()
        self.DeleteForward(repeat)
        for i in range(repeat):
            self.InsertText(char)
        self.EndUndoAction()

    def FindNextChar(self, char, repeat):
        """Find next occurance of char on the current line"""
        self.stc.FindChar(char, repeat)

    def FindPrevChar(self, char, repeat):
        """Find the previos occurance of char on the current line"""
        self.stc.FindChar(char, repeat, reverse=True)

    def FindTillNextChar(self, char, repeat):
        """Similar to FindNextChar, but stop one character short"""
        self.stc.FindChar(char, repeat, extra_offset=-1)

    def FindTillPrevChar(self, char, repeat):
        """Similar to FindPrevChar, but stop one character short"""
        self.stc.FindChar(char, repeat, reverse=True, extra_offset=1)

    def SetFindCharCmd(self, *args):
        """Remember last find-char command (raw unparsed command, e.g. 'f')
        for repeating later on.

        """
        self.LastFindChar = args

    def GetFindCharCmd(self):
        """@see SetFindCharCmd"""
        return self.LastFindChar

    def Mark(self, mark):
        """Create a bookmark on current line and give it a char label
        @param mark: the character label of the bookmark.

        """
        #TODO: handle global bookmarks
        bm_handle, column = self.stc.AddBookmark(), self._GetCol()
        if mark in string.ascii_lowercase:
             # Local bookmarks
            self._Bookmarks[mark] = (bm_handle, column)
        elif mark in string.ascii_uppercase:
            # Global bookmarks
            pass

    def GotoMark(self, mark):
        """Goto the position of the bookmark associated with the given
        character label.
        @param mark: the character label of the book mark
        @todo: hook into Bookmark Manager ed_bookmark?

        """
        if not mark in self._Bookmarks:
            return

        bm_handle, col = self._Bookmarks[mark]
        #TODO: handle global bookmarks
        line = self.stc.MarkerLineFromHandle(bm_handle)
        if line != -1:
            pos = self.stc.FindColumn(line, col)
            self.stc.GotoPos(pos)
        else: #line == -1 means the bookmark was deleted by the user
            del self._Bookmarks[mark]

    def _GetSelectionRange(self):
        """Get the range of selection such that the start is the visual start
        of the selection, not the logical start.

        """
        start, end = minmax(self.stc.GetSelectionStart(),
                            self.stc.GetSelectionEnd())
        return start, end

    def _GotoSelectionStart(self):
        """Goto the visual start of the current selection and deselect"""
        start, end = self._GetSelectionRange()
        self._SetPos(start)
        self.StartSelection()
        self.EndSelection()

    def _GotoSelectionEnd(self):
        """Goto the visual end of the current selection and deselect"""
        start, end = self._GetSelectionRange()
        self._SetPos(end)
        self.StartSelection()
        self.EndSelection()

    def _GetSelectedLines(self):
        """Get the first and last line (exclusive) of selection"""
        start, end = self._GetSelectionRange()
        start_line, end_line = (self.stc.LineFromPosition(start),
                                self.stc.LineFromPosition(end - 1) + 1)
        return start_line, end_line

    def SelectFullLines(self):
        """Extends selection so it covers entire lines"""
        start, end = self._GetSelectedLines()
        self.GotoLine(start)
        self.StartSelection()
        self.GotoLine(end)
        self.EndSelection()

    def IndentSelection(self, forward=True):
        """Indent the lines of the current selection forward (or backward)"""
        if not self.GetSelectedText():
            return

        self.PushScroll()
        self.PushCaret()
        self.BeginUndoAction()
        if forward:
            indent = self.stc.Tab
        else:
            indent = self.stc.BackTab

        for line in range(*self._GetSelectedLines()):
            self.stc.GotoLine(line)
            indent()
        self.EndUndoAction()
        self.PopCaret()
        self.PopScroll()

    def DedentSelection(self):
        """Indent lines of current selection backward"""
        self.IndentSelection(forward=False)

    def ShowCommandBar(self):
        """Open the command bar for the user to enter commands"""
        self.stc.ShowCommandBar()

    def ShowFindBar(self):
        """Open the find bar"""
        self.stc.ShowFindBar()

# ---------------------------------------------------------------------------- #

def minmax(a,b):
    return min(a,b), max(a,b)
# Internal functions, other modules should not bother with any of this

REPEAT_RE = re.compile("([1-9][0-9]*)*(.*)")

def SplitRepeat(cmd):
    """Split the command strings into a pair (repeat, rest)

    >>>SplitRepeat( '3ab' )
    (3, 'ab')
    >>>SplitRepeat( '13ab' )
    (13, 'ab')
    >>>SplitRepeat( 'abc' )
    (None, 'abc')
    >>>SplitRepeat( 'ab2' )
    (None, 'ab2')
    >>>SplitRepeat( '0' )
    (None, '0')
    >>>SplitRepeat( '0ab' )
    (None, '0ab')
    >>>SplitRepeat('8')
    (8, '')

    """
    repeat, rest = re.match( REPEAT_RE, cmd ).groups()
    if repeat:
        repeat = int(repeat)
    return (repeat, rest)

def GetHandler(cmd, h_list=None):
    """Finds the function that handles command cmd
    @param cmd: the command string
    @keyword h_list: alternative list of handler functions

    """
    if h_list is None:
        h_list = HANDLERS

    repeat, cmd = SplitRepeat(cmd)
    if not cmd:
        return None

    for handler in h_list:
        if cmd[0] in handler.start_chars:
            return handler

def DoHandle(handler, cmd, editor):
    """Call handler for command
    It's necessary to use this function instead of calling the handler
    directly.

    """
    repeat, cmd = SplitRepeat(cmd)
    if handler.generic_repeat:   # generic_repeat means that if no repeat
        repeat = repeat or 1     # parameter is supplied, use 1
    return handler(editor, repeat, cmd)

def GetMotion(editor, cmd):
    """Move cursor of editor to a new location according to motion
    Returns a method that handles this motion, or None

    """
    if not cmd:
        return None

    handler = GetHandler(cmd, MOTION_HANDLERS)
    if handler is None:
        return None

    def motion_function():
        return DoHandle(handler, cmd, editor)
    return motion_function

# ---------------------------------------------------------------------------- #
# Vim commands

def vim_parser(start_chars, generic_repeat=True, is_motion=False):
    """Decorator for function that handle vim commands

    Command Handling functions must follow the signature:

        function( editor, repeat, cmd )

    These functions should never be called directly, but instead
    be called through the GetHandler and DoHandle functions.

    The handler functions should always decorate themselves with this
    decorator.

    The purpose of this decorator is to indicate:
        * What is the first char of commands that the handler can handle
            - So that GetHandler can find the correct handler for a vim command
        * Whether it's a motion
            - Some commands operate on motions
        * How to handle the repeat parameter
            - For most command, no repeat is the same as 1 repeat, but for
              some commands, this is not the case

    """
    def decorator(handler):
        handler.is_parser = True
        handler.start_chars = start_chars
        handler.generic_repeat = generic_repeat
        handler.is_motion = is_motion
        return handler
    return decorator

# TODO replace with a special objects
# TODO:CJP ok if these become classes but, if they remain as flags then they
#          need to be uppercase.
NeedMore = True
InvalidOp = False

@vim_parser( u'iIaAoO' )
def InsertMode(editor, repeat, cmd):
    """Handler for basic commands that put vim in INSERT mode"""
    cmd_map = {
            u'i': (lambda:None),
            u'I': editor.GotoIndentStart,
            u'A': editor.GotoLineEnd,
            u'a': editor.MoveForward,
            u'o': editor.OpenLine,
            u'O': editor.OpenLineUp
            }
    func = cmd_map[cmd]
    func()
    editor.SetLastChangeCommand(func)  # Let the editor remember to repeat
    editor.SetInsertRepeat(repeat)     # insertion if and when needed
    editor.InsertMode()

#TODO:CJP Documentation for below methods please
@vim_parser( u'v' )
def VisualMode(editor, repeat, cmd):
    """Enter visual mode
    @see: vim_parser

    """
    editor.VisualMode()

@vim_parser('.')
def Dot(editor, repeat, cmd):
    """Repeat last insert command
    @see: vim_parser

    """
    editor.RepeatChangeCommand(repeat)
    if editor.IsInsertMode():
        editor.NormalMode() # in case it was a 'c' command

@vim_parser(u'hjkl\r \x08\u013c\u013a', is_motion=True)
def Arrows(editor, repeat, cmd):
    """Basic arrow movement in vim.
    @see: vim_parser

    """
    cmd_map = {
            u'h': editor.MoveLeft,
            u'j': editor.MoveDown,
            u'k': editor.MoveUp,
            u'l': editor.MoveRight,
            u'\r': editor.MoveDown,
            u' ' : editor.MoveRight,
            u'\x08' : editor.MoveLeft
          }
    if cmd in cmd_map:
        cmd_map[cmd](repeat)
    else:
        # Handle motion for actual arrow keys
        if cmd == u'\u013c' and not editor.IsAtLineEnd():
            # Right arrow
            editor.MoveRight()
        elif cmd == u'\u013a' and not editor.IsAtLineStart():
            # Left Arrow
            editor.MoveLeft()

@vim_parser('wbeWBE[]', is_motion=True)
def Words(editor,repeat, cmd):
    """Word motions.
    @note: [] is based on what scintilla calls "word parts", these motions
           are not in vim
    @see: vim_parser

    """
    cmd_map = {
            u'w': editor.NextWord,
            u'e': editor.WordEnd,
            u'b': editor.BackWord,
            u'W': editor.NextWordBig,
            u'E': editor.WordEndBig,
            u'B': editor.BackWordBig,
            u'[': editor.BackWordPart,
            u']': editor.WordPartEnd,
          }
    cmd_map[cmd](repeat)

@vim_parser(u'$^0', is_motion=True)
def Line(editor, repeat, cmd):
    """Motions to beginning/end of a line.
    @see: vim_parser

    """
    cmd_map = { u'0': editor.GotoLineStart,
            u'^': editor.GotoIndentStart,
            u'$': editor.GotoLineEnd,
            }
    cmd_map[cmd]()

@vim_parser(u'{}', is_motion=True)
def Para(editor, repeat, cmd):
    """Paragraph motions.
    @see: vim_parser

    """
    cmd_map = { u'{': editor.ParaUp,
            u'}': editor.ParaDown,
          }
    cmd_map[cmd](repeat)

@vim_parser(u'uU')
def Undo(editor, repeat, cmd):
    """Undo/Redo commands.
    @note: unlike vim, U is used for redo.
    @see: vim_parser

    """
    if cmd == u'u':
        editor.Undo(repeat)
    elif cmd == u'U':
        editor.Redo(repeat)

# TODO:CJP Scintilla lexing bug, check if fixed in 2.9
@vim_parser(u'*#', is_motion=True)
def FindIdent(editor, repeat, cmd):
    """Find the next/previous occurance of identifier under caret
    @note: There's a sublte difference from vim: if some text is already
           selected, it's used as the search term instead of the identifier
           under the caret
    @see: vim_parser

    """
    cmd_map = { u'#': editor.PrevIdent,
            u'*': editor.NextIdent,
          }
    cmd_map[cmd](repeat)

@vim_parser(u'~')
def Tilde(editor, repeat, cmd):
    """Invert case of character(s) under caret. Also operates on selection.
    @see: vim_parser

    """
    editor.InvertCase(repeat)
    editor.NormalMode() # In case this command was applied in visual mode

@vim_parser(u'J')
def Join(editor, repeat, cmd):
    """Join lines command.
    @see: vim_parser

    """
    editor.PushCaret()
    editor.JoinLines(repeat)
    editor.PopCaret()

@vim_parser(u'xXsSCDY')
def Delete(editor, repeat, cmd):
    """Simple delete/change commands that are implemented in terms of more
    advanced c/d commands.
    @see: vim_parser

    """
    cmd_map = {
        u'x': u'dl', u'X': u'dh',
        u's': u'cl', u'S': u'cc',
        u'C': u'c$', u'D': u'd$', u'Y': u'y$',
        }
    Change(editor, repeat, cmd_map[cmd])

@vim_parser(u'cdy<>')
def Change(editor, repeat, cmd):
    """Implementations for c/d/y commands. Also for <> (indentation) commands.
    @todo: This method is quite larger than other methods in this module,
           needs to be simplified.
    @see: vim_parser

    """
    editor.PushColumn()
    editor.PushCaret()
    def ret(return_value, restore=True):
        """Needed because we have to pop what we pushed before we return"""
        editor.PopCaret(restore)
        editor.PopColumn(restore)
        return return_value
    pre_selected = False
    line_motion = False
    if editor.HasSelection():
        pre_selected = True
        if len(cmd) > 1:
            motion = cmd[1:]
        else:
            motion = ''
        cmd = cmd[0]
    else:
        if len(cmd) == 1:
            return ret(NeedMore, False)

        cmd, motion = cmd[0], cmd[1:]
        if motion.isdigit():
            return ret(NeedMore)

        motion_repeat, motion_cmd = SplitRepeat(motion)
        if motion_repeat:
            repeat = repeat * motion_repeat

        if motion_cmd == cmd:
            # Operate on whole line
            line_motion = True
            editor.PushColumn()
            editor.SelectLines(repeat)
        else:
            motion_function = GetMotion(editor, motion_cmd)
            if motion_function is None:
                # Invalid motion; cancel operation
                return ret(InvalidOp)

            editor.PushCaret()
            editor.PushColumn()
            for i in range(repeat):
                if motion_function() == NeedMore:
                    # This motion is incomplete .. (catch on first iteration)
                    editor.Deselect()
                    return ret(NeedMore)
            editor.StartSelection()
            editor.PopCaret()
            editor.EndSelection()

            if motion_cmd[0] in LINE_MOTION_PREFIXES:
                line_motion = True
                editor.SelectFullLines()

    cmd_map = {
            u'y': editor.YankSelection,
            u'd': editor.DeleteSelection,
            u'c': editor.ChangeSelection,
            u'<': editor.DedentSelection,
            u'>': editor.IndentSelection,
          }

    cmd_map[cmd]()

    restore_x = cmd in (u'y', u'<', u'>') or cmd == u'd' and line_motion
    editor.PopColumn(restore_x)

    # XXX: Some special case handling
    #      Not the most elegant way though ..
    if cmd == u'c':
        pass
    else:
        # Repeating delete/yank/indent commands doesn't insert any text
        editor.SetLastInsertedText(u'')
        # Applying them in visual mode ends visual mode
        editor.NormalMode()

    if not pre_selected:
        # Remember this command as last change
        # However, if we're operating on a selection, then remembering
        # doesn't make much sense
        editor.SetLastChangeCommand(lambda : Change(editor, repeat, cmd+motion))

@vim_parser(u'pP')
def Put(editor, repeat, cmd):
    """Paste commands.
    @see: vim_parser

    """
    before = cmd == u'P'
    editor.Put(before, repeat)
    editor.NormalMode() # for pasting in visual mode

@vim_parser(u'"')
def Reg(editor, repeat, cmd):
    """Switch register (clipboard) command
    @note: This command is stand-alone, not prefix to other commands.
    @see: vim_parser

    """
    if len(cmd) == 1:
        return NeedMore

    cmd, char = cmd
    try:
        editor.SetRegister(char)
    except ebmlib.ClipboardException:
        # Attempted to switch to invalid register
        wx.Bell()

@vim_parser(u'r')
def ReplaceChar(editor, repeat, cmd):
    """Replace character under caret with another one.
    @note: Does not enter into Insert mode.
    @see: vim_parser

    """
    if len(cmd) == 1:
        return NeedMore

    r, char = cmd
    f = lambda: editor.ReplaceChar(char, repeat)
    f()
    editor.SetLastChangeCommand(f)
    editor.SetLastInsertedText(u'')

@vim_parser(u'R')
def ReplaceMode(editor, repeat, cmd):
    """Enter into Replace Mode.
    @see: vim_parser

    """
    # TODO handle repetition
    editor.ReplaceMode()

@vim_parser(u'fFtT', is_motion=True)
def FindChar(editor, repeat, cmd):
    """Find character on current line and move the caret to it (if found).
    @see: vim_parser

    """
    if len(cmd) == 1:
        return NeedMore

    cmd, char = cmd
    cmd_map = {
            u'f' : editor.FindNextChar,
            u'F' : editor.FindPrevChar,
            u't' : editor.FindTillNextChar,
            u'T' : editor.FindTillPrevChar,
          }
    cmd_map[cmd](char, repeat)
    editor.SetFindCharCmd(cmd, char)

@vim_parser(u',;', is_motion=True)
def RepeatFindChar(editor, repeat, cmd):
    """Repeat the last FindChar motion.
    @see: vim_parser

    """
    prev_cmd = editor.GetFindCharCmd()
    if not prev_cmd:
        return

    pcmd, char = prev_cmd
    if cmd == u',':
        pcmd = pcmd.swapcase()

    FindChar(editor, repeat, pcmd+char)
    editor.SetFindCharCmd(*prev_cmd)

@vim_parser(u'm')
def Mark(editor, repeat, cmd):
    """Create a bookmark and associate it with a character label.
    @see: vim_parser

    """
    if len(cmd) == 1:
        return NeedMore

    cmd, mark = cmd
    editor.Mark(mark)

@vim_parser(u"`'", is_motion=True)
def Jump(editor, repeat, cmd):
    """Jump to a bookmark specified by a character label.
    @see: vim_parser

    """
    if len(cmd) == 1:
        return NeedMore

    cmd, mark = cmd
    editor.GotoMark(mark)
    if cmd == u"'":
        editor.GotoIndentStart()

@vim_parser(u'HLMGgz', is_motion=True, generic_repeat=False)
def NavExtra(editor, repeat, cmd):
    """Commands for navigating visible lines or scrolling.
    @see: vim_parser

    """
    if cmd == u'H':
        editor.GotoScreenTop()
        if repeat:
            editor.MoveDown(repeat)
    elif cmd == u'M':
        editor.GotoScreenMiddle()
    elif cmd == u'L':
        editor.GotoScreenLast()
        if repeat:
            editor.MoveUp(repeat)
    elif cmd == u'G':
        if repeat:
            editor.GotoLine(repeat)
        else:
            editor.GotoLastLine()
    else: # g or z command
        if len(cmd) < 2:
            return NeedMore

        if cmd == u'gg':
            editor.GotoLine(0)
        elif cmd == u'zz':
            editor.ScrollMiddle()
        elif cmd == u'zt':
            editor.ScrollTop()
        elif cmd == u'zb':
            editor.ScrollBottom()

@vim_parser(u'/?', is_motion=True)
def RegexSearch(editor, repeat, cmd):
    """Incremental search commands.
    @note: Uses the find bar in editra.
    @see: vim_parser

    """
    # XXX: findbar is not regex!
    # TODO:CJP it is if you set the regex search flag or the user's previous
    #          search was done using regular expressions.
    editor.ShowFindBar()

@vim_parser(u'|', is_motion=True)
def Column(editor, repeat, cmd):
    """Goto specified column, with 1 being the default.
    @see: vim_parser

    """
    editor.GotoColumn(repeat)

@vim_parser(u':')
def Ex(editor, repeat, cmd):
    """Command for opening the command bar.
    @see: vim_parser

    """
    editor.ShowCommandBar()

# Cannonical list of handlers
HANDLERS = (
    InsertMode, ReplaceMode, VisualMode, Dot, Undo,
    Arrows, Words, Line, Para, Column, FindChar, RepeatFindChar,
    FindIdent, Change, Delete, Tilde, ReplaceChar, Put, Join,
    Mark, Jump, Ex, Reg, NavExtra, RegexSearch,
    )
MOTION_HANDLERS = [h for h in HANDLERS if h.is_motion]
# HACK: The following is a sign of improper design! but doing it properly
#       would probably require yet another huge rewrite of this module.
#       So watch out if things get out of control with many such hacks.
# These are motions that, if operated on, the operation should happen
# on whole lines.
LINE_MOTION_PREFIXES = [u'\'', u'G', u'{', u'}', u'H', u'L', u'M']