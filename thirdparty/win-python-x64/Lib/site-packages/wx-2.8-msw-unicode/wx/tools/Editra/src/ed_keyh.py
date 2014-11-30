###############################################################################
# Name: ed_keyh.py                                                            #
# Purpose: Editra's Vi Emulation Key Handler                                  #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
KeyHandler interface for implementing extended key action handling in Editra's
main text editting buffer.

@summary: Custom keyhandler interface

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_keyh.py 66736 2011-01-23 18:26:50Z CJP $"
__revision__ = "$Revision: 66736 $"

#-------------------------------------------------------------------------#
# Imports
import re
import wx
import wx.stc

# Editra Libraries
import ed_event
import ed_glob
import ed_basestc
import ed_stc
import string
import ed_vim

#-------------------------------------------------------------------------#
# Use this base class to derive any new keyhandlers from. The keyhandler is
# called upon by the active buffer when ever a key press event happens. The
# handler then has the responsibility of deciding what to do with the key.
#
class KeyHandler(object):
    """KeyHandler base class"""
    def __init__(self, stc):
        super(KeyHandler, self).__init__()

        # Attributes
        self.stc = stc

    def ClearMode(self):
        """Clear any key input modes to normal input mode"""
        evt = ed_event.StatusEvent(ed_event.edEVT_STATUS, self.stc.GetId(),
                                   '', ed_glob.SB_BUFF)
        wx.PostEvent(self.stc.GetTopLevelParent(), evt)

    def GetHandlerName(self):
        """Get the name of this handler
        @return: string

        """
        return u'NULL'

    def PreProcessKey(self, key_code, ctrldown=False,
                      cmddown=False, shiftdown=False, altdown=False):
        """Pre process any keys before they get to the char handler
        @param key_code: Raw keycode
        @keyword ctrldown: Is the control key down
        @keyword cmddown: Is the Command key down (Mac osx)
        @keyword shiftdown: Is the Shift key down
        @keyword altdown: Is the Alt key down
        @return: bool

        """
        return False

    def ProcessKey(self, key_code, ctrldown=False,
                   cmddown=False, shiftdown=False, altdown=False):
        """Process the key and return True if it was processed and
        false if it was not. The key is recieved at EVT_CHAR.
        @param key_code: Raw keycode
        @keyword ctrldown: Is the control key down
        @keyword cmddown: Is the Command key down (Mac osx)
        @keyword shiftdown: Is the Shift key down
        @keyword altdown: Is the Alt key down
        @return: bool

        """
        return False

#-------------------------------------------------------------------------#

class ViKeyHandler(KeyHandler):
    """Defines a key handler for Vi emulation
    @summary: Handles key presses according to Vi emulation.

    """

    # Vi Mode declarations
    NORMAL, \
    INSERT, \
    VISUAL, \
        = range(3)

    def __init__(self, stc, use_normal_default=False):
        super(ViKeyHandler, self).__init__(stc)

        # Attributes
        self.mode = 0
        self.last = u''
        self.last_find = u''
        self.commander = ed_vim.EditraCommander(self)
        self.buffer = u''

        # Insert mode by defauly
        if use_normal_default:
            self.NormalMode()
        else:
            self.InsertMode()

    def ClearMode(self):
        """Clear the mode back to default input mode"""
        # TODO:CJP when newer scintilla is available in 2.9 use
        #          blockcaret methods.
        self.stc.SetCaretWidth(1)
        self.last = self.cmdcache = u''
        KeyHandler.ClearMode(self)

    def GetHandlerName(self):
        """Get the name of this handler"""
        return u'VI'

    def _SetMode(self, newmode, msg):
        """Set the keyhandlers mode
        @param newmode: New mode name to change to

        """
        self.buffer = u'' # Clear buffer from last mode
        self.mode = newmode
        # Update status bar
        evt = ed_event.StatusEvent(ed_event.edEVT_STATUS, self.stc.GetId(),
                                   msg, ed_glob.SB_BUFF)
        wx.PostEvent(self.stc.GetTopLevelParent(), evt)

    def InsertMode(self):
        """Change to insert mode"""
        self.stc.SetLineCaret()
        self.stc.SetOvertype(False)
        self._SetMode(ViKeyHandler.INSERT, u"INSERT")

    def ReplaceMode(self):
        """Change to replace mode
        This really just insert mode with overtype set to true

        """
        self.stc.SetLineCaret()
        self.stc.SetOvertype(True)
        self._SetMode(ViKeyHandler.INSERT, u"REPLACE")

    def NormalMode(self):
        """Change to normal (command) mode"""
        if self.IsInsertMode():
            self.commander.SetLastInsertedText(self.buffer)

        self.stc.SetOvertype(False)
        self.stc.SetBlockCaret()
        self.commander.Deselect()
        self.commander.InsertRepetition()
        self._SetMode(ViKeyHandler.NORMAL, u'NORMAL')

    def VisualMode(self):
        """Change to visual (selection) mode"""
        self.stc.SetBlockCaret()
        self.stc.SetOvertype(False)
        self._SetMode(ViKeyHandler.VISUAL, u'VISUAL')
        self.commander.StartSelection()

    def IsInsertMode(self):
        """Test if we are in insert mode"""
        return self.mode == ViKeyHandler.INSERT

    def IsNormalMode(self):
        """Test if we are in normal mode"""
        return self.mode == ViKeyHandler.NORMAL

    def IsVisualMode(self):
        """Test if we are in visual mode"""
        return self.mode == ViKeyHandler.VISUAL

    def PreProcessKey(self, key_code, ctrldown=False,
                      cmddown=False, shiftdown=False, altdown=False):
        """Pre process any keys before they get to the char handler
        @param key_code: Raw keycode
        @keyword ctrldown: Is the control key down
        @keyword cmddown: Is the Command key down (Mac osx)
        @keyword shiftdown: Is the Shift key down
        @keyword altdown: Is the Alt key down
        @return: bool

        """
        if not shiftdown and key_code == wx.WXK_ESCAPE:
            # If Vi emulation is active go into Normal mode and
            # pass the key event to the char handler by not processing
            # the key.
            self.NormalMode()
            return False
        elif (ctrldown or cmddown) and key_code == ord('['):
            self.NormalMode()
            return True
        elif key_code in (wx.WXK_RETURN, wx.WXK_BACK,
                          wx.WXK_RIGHT, wx.WXK_LEFT) and \
             not self.IsInsertMode():
            # swallow enter key in normal and visual modes
            # HACK: we have to do it form here because ProcessKey
            #       is only called on Char events, not Key events,
            #       and the Enter key does not generate a Char event.
            self.ProcessKey(key_code)
            return True
        else:
            return False

    def ProcessKey(self, key_code, ctrldown=False,
                   cmddown=False, shiftdown=False, altdown=False):
        """Processes keys and decided whether to interpret them as vim commands
        or normal insert text

        @param key_code: Raw key code
        @keyword cmddown: Command/Ctrl key is down
        @keyword shiftdown: Shift Key is down
        @keyword altdown : Alt key is down

        """
        if ctrldown or cmddown or altdown:
            return False

        # Mode may change after processing the key, so we need to remember
        # whether this was a command or not
        f_cmd = self.IsNormalMode() or self.IsVisualMode()

        self._ProcessKey(key_code)

        # Update status bar
        if self.IsNormalMode():
            if self.stc.GetTopLevelParent():
                evt = ed_event.StatusEvent(ed_event.edEVT_STATUS,
                                           self.stc.GetId(),
                                           u"NORMAL %s" % self.buffer,
                                           ed_glob.SB_BUFF)
                wx.PostEvent(self.stc.GetTopLevelParent(), evt)

        if f_cmd:
            return True
        else:
            # If we're in insert mode we must return False
            # so the text gets inserted into the editor
            return False

    def _ProcessKey(self, key_code):
        """The real processing of keys"""
        char = unichr(key_code)
        if self.IsNormalMode() or self.IsVisualMode():
            self.buffer += char
            if ed_vim.Parse(self.buffer, self.commander):
                # command was handled (or invalid) so clear buffer
                self.buffer = u''

            if self.IsVisualMode():
                self.commander.ExtendSelection()

        elif self.IsInsertMode():
            self.buffer += char

    def InsertText(self, pos, text):
        """Insert text and store it in the buffer if we're in insert mode
        i.e. as if it was typed in

        """
        self.stc.InsertText(pos, text)
        if self.IsInsertMode():
            self.buffer += text