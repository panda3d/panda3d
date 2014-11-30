###############################################################################
# Name: txtentry.py                                                           #
# Purpose: Text entry control base clases                                     #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: TextEntry

Text entry base and helper classes.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: txtentry.py 67500 2011-04-15 22:51:55Z CJP $"
__revision__ = "$Revision: 67500 $"

__all__ = ["CommandEntryBase",]

#-----------------------------------------------------------------------------#
# Imports
import wx

#-----------------------------------------------------------------------------#

class CommandEntryBase(wx.SearchCtrl):
    """Base single line text control with key event handling callbacks."""
    def __init__(self, parent, id=wx.ID_ANY, value='', pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, validator=wx.DefaultValidator,
                 name="CommandEntryBase"):

        clone = None
        if validator != wx.DefaultValidator:
            clone = validator.Clone()

        super(CommandEntryBase, self).__init__(parent, id, value, pos,
                                               size, style, validator, name)

        # Attributes
        self._txtctrl = None  # For msw/gtk
        self._enterhook = None

        # Hide the search button and text by default
        self.ShowSearchButton(False)
        self.ShowCancelButton(False)
        self.SetDescriptiveText(wx.EmptyString)

        # MSW/GTK HACK need to bind directly to the text control component
        if wx.Platform in ['__WXGTK__', '__WXMSW__']:
            for child in self.GetChildren():
                if isinstance(child, wx.TextCtrl):
                    if clone is not None:
                        child.SetValidator(clone)
                    self._txtctrl = child
                    child.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
                    child.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
                    break
        else:
            self._txtctrl = self
            self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
            self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)

        # Event management
        self.Bind(wx.EVT_TEXT_ENTER, self.OnEnter)

    EnterCallback = property(lambda self: self._enterhook,
                             lambda self, cback: setattr(self, '_enterhook', cback))

    def GetTextControl(self):
        """Get the wx.TextCtrl window.
        @note: only for msw/gtk

        """
        return self._txtctrl

    def OnKeyDown(self, evt):
        """Handle KeyDown events"""
        evt.Skip()

    def OnKeyUp(self, evt):
        """Handle KeyUp events"""
        evt.Skip()

    def OnEnter(self, evt):
        """Handle the Enter key event"""
        if self.EnterCallback:
            self.EnterCallback()
        else:
            evt.Skip()

    def SetFocus(self):
        """Set the focus and select the text in the field"""
        super(CommandEntryBase, self).SetFocus()
        self.SelectAll()
