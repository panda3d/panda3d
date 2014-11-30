###############################################################################
# Name: clipboard.py                                                          #
# Purpose: Vim like clipboard                                                 #
# Author: Hasan Aljudy                                                        #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: Clipboard

Clipboard helper class

"""

__author__ = "Hasan Aljudy"
__cvsid__ = "$Id: clipboard.py 67123 2011-03-04 00:02:35Z CJP $"
__revision__ = "$Revision: 67123 $"

__all__ = [ 'Clipboard', 'ClipboardException']

#-----------------------------------------------------------------------------#
# Imports
import wx

#-----------------------------------------------------------------------------#

class ClipboardException(Exception):
    """Thrown for errors in the Clipboard class"""
    pass

#-----------------------------------------------------------------------------#

class Clipboard(object):
    """Multiple clipboards as named registers (as per vim)

    " is an alias for system clipboard and is also the default clipboard.

    @note: The only way to access multiple clipboards right now is through
           Normal mode when Vi(m) emulation is enabled.

    """
    NAMES = list(u'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_')
    registers = {}
    current = u'"'

    @classmethod
    def ClearAll(cls):
        """Clear all registers"""
        for reg in cls.registers:
            cls.registers[reg] = u''

    @classmethod
    def DeleteAll(cls):
        """Delete all registers"""
        cls.registers.clear()

    @classmethod
    def Switch(cls, reg):
        """Switch to register
        @param reg: char

        """
        if reg in cls.NAMES or reg == u'"':
            cls.current = reg
        else:
            raise ClipboardException(u"Switched to invalid register name")

    @classmethod
    def NextFree(cls):
        """Switch to the next free register. If current register is free, no
        switching happens.

        A free register is one that's either unused or has no content

        @note: This is not used yet.

        """
        if cls.Get() == u'':
            return

        for name in cls.NAMES:
            if cls.registers.get(name, u'') == u'':
                cls.Switch(name)
                break

    @classmethod
    def AllUsed(cls):
        """Get a dictionary mapping all used clipboards (plus the system
        clipboard) to their content.
        @note: This is not used yet.
        @return: dict

        """
        cmd_map = { u'"': cls.SystemGet() }
        for name in cls.NAMES:
            if cls.registers.get(name, u''):
                cmd_map[name] = cls.registers[name]
        return cmd_map

    @classmethod
    def Get(cls):
        """Get the content of the current register. Used for pasting"""
        if cls.current == u'"':
            return cls.SystemGet()
        else:
            return cls.registers.get(cls.current, u'')

    @classmethod
    def Set(cls, text):
        """Set the content of the current register
        @param text: string

        """
        if cls.current == u'"':
            return cls.SystemSet(text)
        else:
            cls.registers[cls.current] = text

    @classmethod
    def SystemGet(cls):
        """Get text from the system clipboard
        @return: string

        """
        text = None
        if wx.TheClipboard.Open():
            if wx.TheClipboard.IsSupported(wx.DataFormat(wx.DF_TEXT)):
                text = wx.TextDataObject()
                wx.TheClipboard.GetData(text)

            wx.TheClipboard.Close()

        if text is not None:
            return text.GetText()
        else:
            return u''

    @classmethod
    def SystemSet(cls, text):
        """Set text into the system clipboard
        @param text: string
        @return: bool

        """
        ok = False
        if wx.TheClipboard.Open():
            wx.TheClipboard.SetData(wx.TextDataObject(text))
            wx.TheClipboard.Close()
            ok = True
        return ok
