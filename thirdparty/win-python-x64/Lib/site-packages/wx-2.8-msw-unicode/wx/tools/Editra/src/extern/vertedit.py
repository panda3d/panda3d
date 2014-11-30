#!/usr/bin/env python
# -*- coding: latin1 -*-
#
# Created in 2008 by Don Quijote
# Licence: wxWindows
#
# Updated 02/09/2011 Cody Precord

#-----------------------------------------------------------------------------#

import wx
from wx import stc
from collections import deque
from functools import partial

#-----------------------------------------------------------------------------#
# Globals
STATE_OFF = 0
STATE_SELECTION = 1
STATE_ACTIVE = 2

#-----------------------------------------------------------------------------#

class VertEdit(object):
    """
    Used instance variables:
      - e             STC Editor passed via class constructor
      - enabled       To deactivate this plugin after been loaded
      - state         0: mode off, 1: mode just activated but still in dynamic line selection, 2: mode active, static block of lines
      - stack         Holds insertions and deletions, to replicate on all lines of the block
      - oldCol        Used by vertCaret() to remember last column of vertical caret
      - markedLines   Used by newMarkers() to remember highlighted lines
      - orig          Holds position of anchor when first switching from state 0 to state 1.
      - origCol       Holds column of anchor when first switching from state 0 to state 1.
      - origLine      Holds line of anchor when first switching from state 0 to state 1.
      - gotDeletes    An action caused STC to try to delete some text while in state 1.
      - insCol        Remembers column of cursor before STC deleted some text (see gotDeletes)
      - delCol2       Remembers last column enclosing text to be deleted
   
    """
    INS = stc.STC_MOD_INSERTTEXT|stc.STC_PERFORMED_USER
    DEL = stc.STC_MOD_DELETETEXT|stc.STC_PERFORMED_USER
    BDEL = stc.STC_MOD_BEFOREDELETE|stc.STC_PERFORMED_USER

    def __init__(self, editor,
                 blockBackColour="light blue",
                 markerNumber=1):
        super(VertEdit, self).__init__()

        # Attributes
        self.e = editor
        self.marker = markerNumber
        self.enabled = False            # Disable by default
        self.state = STATE_OFF
        self.stack = deque()
        self.jitter = None
        self.SetBlockColor(blockBackColour)
        self.modmask = long(self.e.ModEventMask)

        # Event Handlers
#        self.e.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
#        self.e.Bind(stc.EVT_STC_UPDATEUI, self.OnUpdateUI)

    def enable(self, enable=True):
        """Enable the column edit mode"""
        if not enable:
            self.endMode()
        self.enabled = enable

    def disable(self):
        """Disable the column edit mode"""
        self.endMode()
        self.enabled = False

    @property
    def Enabled(self):
        """Is the column edit mode enabled"""
        return self.enabled

    def vertCaret(self, col=None, pos=None):
        if col is None:
            if pos is None:
                pos = self.e.CurrentPos
            col = self.e.GetColumn(pos)

        if col != self.oldCol:
            self.e.EdgeColumn = col
            self.oldCol = col

    def newMarkers(self, linesToMark=frozenset()):
        # careful when STC collapses markers due to multiline deletion!
        # for line in linesToMark.difference(self.markedLines):
        for line in linesToMark:
            self.e.MarkerAdd(line, self.marker)

        for line in self.markedLines.difference(linesToMark):
            self.e.MarkerDelete(line, self.marker)

        self.markedLines = linesToMark

    def OnKeyDown(self, evt):
        if self.Enabled and self.state != STATE_OFF:
            k = evt.GetKeyCode()
            if k in (wx.WXK_ESCAPE, wx.WXK_LEFT, wx.WXK_NUMPAD_LEFT,
                     wx.WXK_RIGHT, wx.WXK_NUMPAD_RIGHT) and evt.Modifiers == 0:
                if k == wx.WXK_ESCAPE:
                    self.endMode()
                    self.e.GotoPos(self.e.CurrentPos)
                    return
                elif self.state == STATE_SELECTION:
                    self.e.Anchor = self.e.CurrentPos
        evt.Skip()

    def endMode(self):
        if self.state != STATE_OFF:
            self.e.SetModEventMask(self.modmask)
            self.e.HideSelection(False)
            self.state = STATE_OFF
            self.e.EndUndoAction()
            self.e.EdgeColumn, self.e.EdgeMode = self.edge
            self.newMarkers()

    def fn_ins(self, col, text, line):
        colEnd = self.e.GetColumn(self.e.GetLineEndPosition(line))
        if col > colEnd:
            text = u" " * (col - colEnd) + text
            col = colEnd

        self.e.CurrentPos = pos = self.e.FindColumn(line, col)
        self.e.AddText(text)
        return col == self.e.GetColumn(pos)

    def fn_del(self, col1, col2, line):
        pos1 = self.e.FindColumn(line, col1)
        pos2 = self.e.FindColumn(line, col2)
        self.e.CurrentPos = pos1

        if pos1 == pos2 or col2 != self.e.GetColumn(pos2):
            return False

        self.e.SetTargetStart(pos1)
        self.e.SetTargetEnd(pos2)
        self.e.ReplaceTarget(u'')
        return True

    def OnModified(self, evt):
        if self.Enabled and self.state > STATE_OFF:
            fn = None
            if evt.ModificationType & VertEdit.INS == VertEdit.INS:
                col = self.insCol or self.e.GetColumn(evt.Position)
                fn = partial(self.fn_ins, col, evt.Text)
                self.insCol = None
            elif evt.ModificationType & VertEdit.DEL == VertEdit.DEL:
                if self.state == STATE_ACTIVE:
                    fn = partial(self.fn_del, self.e.GetColumn(evt.Position), self.delCol2)
            elif evt.ModificationType & VertEdit.BDEL == VertEdit.BDEL:
                self.delCol2 = self.e.GetColumn(evt.Position + evt.Length)
                if self.state == STATE_SELECTION and not self.gotDeletes:
                    self.gotDeletes = True
                    self.insCol = self.e.GetColumn(self.e.CurrentPos)

            if fn:
                if evt.LinesAdded:
                    self.endMode()
                else:
                    self.stack.append(fn)

    def SetBlockColor(self, color):
        """Set the block background color used during the highlight
        @param color: wxColor

        """
        self.e.MarkerDefine(self.marker,
                            stc.STC_MARK_BACKGROUND,
                            background=color)

    def startMode(self, singleLine=False):
        if self.state == STATE_OFF:
            self.e.ModEventMask |= VertEdit.INS|VertEdit.DEL|VertEdit.BDEL
            self.e.HideSelection(True)
            orig = self.e.Anchor
            self.origCol = self.e.GetColumn(orig)
            self.origLine = self.e.LineFromPosition(orig)
            self.stack.clear()
            self.gotDeletes = False
            self.e.BeginUndoAction()

            # painting related
            self.oldCol = -1
            self.markedLines = frozenset()
            self.edge = self.e.EdgeColumn, self.e.EdgeMode
            self.e.EdgeMode = stc.STC_EDGE_LINE

        self.state = STATE_SELECTION
        self.insCol = None
        self.curLine = self.e.LineFromPosition(self.e.CurrentPos)
        self.lines = frozenset(range(min(self.origLine, self.curLine),
                                     max(self.origLine, self.curLine) + 1))
        self.jitter = None

        if singleLine:
            newA = self.e.PositionFromLine(self.curLine)
            if newA == self.e.CurrentPos:
                newA = self.e.GetLineEndPosition(self.curLine)
                if newA == self.e.CurrentPos:
                    self.e.CurrentPos -= 1
                    newA += 1
            self.e.Anchor = newA
            self.jitter = newA

        self.e.SetSelectionMode(stc.STC_SEL_LINES)

        # painting related
        self.vertCaret()
        self.newMarkers(self.lines)

    def OnUpdateUI(self, evt):
        """Handle EVT_UPDATEUI"""
        # Check if enabled
        if not self.Enabled:
            return

        curP = self.e.CurrentPos
        if self.state == STATE_OFF:
            anchor = self.e.Anchor
            selection = anchor != curP
            sameCol = selection and self.e.GetColumn(curP) == self.e.GetColumn(anchor)
            if sameCol:
                self.startMode()
            return

        anchor = self.e.Anchor
        selection = anchor != curP
        sameCol = selection and self.e.GetColumn(curP) == self.origCol
        linesOverride = None
        if self.state == STATE_SELECTION:
            if self.jitter == anchor and selection:
                self.jitter = None
                self.e.Anchor = anchor = self.e.FindColumn(self.origLine,self.origCol)
                selection = anchor != curP

            self.state = STATE_ACTIVE
            if self.stack:
                self.e.EndUndoAction()
                self.e.Undo()
                self.e.BeginUndoAction()
                linesOverride = self.lines
                self.newMarkers(self.lines)
                curP = self.e.PositionFromLine(self.curLine)
            elif sameCol:
                self.startMode()
            elif self.gotDeletes or selection:
                if not self.gotDeletes:
                    self.e.Anchor = self.e.FindColumn(self.origLine, self.origCol)
                self.endMode()
            elif curP == self.e.FindColumn(self.origLine, self.origCol):
                self.startMode(True)

        if self.state == STATE_ACTIVE:
            curI = self.e.LineFromPosition(curP)
            if curP == self.e.GetLineEndPosition(curI):
                self.e.GotoPos(curP)

            if curI not in self.lines:
                self.endMode()
            else:
                self.e.HideSelection(False)
                curC = self.e.GetColumn(curP)
                if self.stack:
                    self.state *= -1
                    lines = linesOverride or self.lines.difference((curI,))

                    cont = True
                    while(cont and self.stack):
                        fn = self.stack.popleft()
                        for line in lines:
                            if not fn(line):
                                self.e.EndUndoAction()
                                self.e.Undo()
                                self.e.BeginUndoAction()
                                cont = False
                                break

                    self.stack.clear()
                    self.e.EndUndoAction()
                    self.e.BeginUndoAction()
                    curC = self.e.GetColumn(self.e.CurrentPos)
                    self.e.GotoPos(self.e.FindColumn(curI, curC))
                    self.state *= -1

                self.vertCaret(col = curC)
