###############################################################################
# Name: choicedlg.py                                                          #
# Purpose: Generic Choice Dialog                                              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: Choice Dialog

A generic choice dialog that uses a wx.Choice control to display its choices.

@summary: Generic Choice Dialog

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: choicedlg.py 65202 2010-08-06 15:49:23Z CJP $"
__revision__ = "$Revision: 65202 $"

__all__ = ['ChoiceDialog',]

#--------------------------------------------------------------------------#
# Imports
import wx

import ecbasewin

#--------------------------------------------------------------------------#
# Globals
ChoiceDialogNameStr = u"ChoiceDialog"

#--------------------------------------------------------------------------#

class ChoiceDialog(ecbasewin.ECBaseDlg):
    """Dialog with a wx.Choice control for showing a list of choices"""
    def __init__(self, parent, id=wx.ID_ANY,
                 msg=u'', title=u'',
                 choices=None, default=u'',
                 pos=wx.DefaultPosition,
                 size=wx.DefaultSize,
                 style=0,
                 name=ChoiceDialogNameStr):
        """Create the choice dialog
        @keyword msg: Dialog Message
        @keyword title: Dialog Title
        @keyword choices: list of strings
        @keyword default: Default selection

        """
        super(ChoiceDialog, self).__init__(parent, id, title,
                                           style=wx.CAPTION, pos=pos,
                                           size=size, name=name)

        # Attributes
        panel = ChoicePanel(self, msg=msg,
                                  choices=choices,
                                  default=default,
                                  style=style)
        self.SetPanel(panel)
        self.SetInitialSize()

#--------------------------------------------------------------------------#

class ChoicePanel(wx.Panel):
    """Generic Choice dialog panel"""
    def __init__(self, parent, msg=u'', choices=list(),
                 default=u'', style=wx.OK|wx.CANCEL):
        """Create the panel
        @keyword msg: Display message
        @keyword choices: list of strings
        @keyword default: default selection
        @keyword style: dialog style

        """
        super(ChoicePanel, self).__init__(parent)

        # Attributes
        self._msg = msg
        self._choices = wx.Choice(self, wx.ID_ANY)
        self._selection = default
        self._selidx = 0
        self._bmp = None
        self._buttons = list()

        # Setup
        self._choices.SetItems(choices)
        if default in choices:
            self._choices.SetStringSelection(default)
            self._selidx = self._choices.GetSelection()
        else:
            self._choices.SetSelection(0)
            self._selidx = 0
            self._selection = self._choices.GetStringSelection()

        # Setup Buttons
        for btn, id_ in ((wx.OK, wx.ID_OK), (wx.CANCEL, wx.ID_CANCEL),
                         (wx.YES, wx.ID_YES), (wx.NO, wx.ID_NO)):
            if btn & style:
                button = wx.Button(self, id_)
                self._buttons.append(button)

        if not len(self._buttons):
            self._buttons.append(wx.Button(self, wx.ID_OK))
            self._buttons.append(wx.Button(self, wx.ID_CANCEL))

        # Layout
        self.__DoLayout(style)

        # Event Handlers
        self.Bind(wx.EVT_CHOICE, self.OnChoice, self._choices)
        self.Bind(wx.EVT_BUTTON, self.OnButton)

    def __DoLayout(self, style):
        """Layout the panel"""
        hsizer = wx.BoxSizer(wx.HORIZONTAL)
        vsizer = wx.BoxSizer(wx.VERTICAL)
        caption = wx.StaticText(self, label=self._msg)

        # Layout the buttons
        bsizer = wx.StdDialogButtonSizer()
        for button in self._buttons:
            bsizer.AddButton(button)
            bid = button.GetId()
            if bid in (wx.ID_NO, wx.ID_YES):
                if wx.NO_DEFAULT & style:
                    if bid == wx.ID_NO:
                        button.SetDefault()
                else:
                    if bid == wx.ID_YES:
                        button.SetDefault()
            elif bid == wx.ID_OK:
                button.SetDefault()

        bsizer.Realize()

        vsizer.AddMany([((10, 10), 0), (caption, 0), ((20, 20), 0),
                        (self._choices, 1, wx.EXPAND), ((10, 10), 0),
                        (bsizer, 1, wx.EXPAND),
                        ((10, 10), 0)])

        icon_id = wx.ART_INFORMATION
        for i_id, a_id in ((wx.ICON_ERROR, wx.ART_ERROR),
                     (wx.ICON_WARNING, wx.ART_WARNING)):
            if i_id & style:
                icon_id = a_id
                break

        icon = wx.ArtProvider.GetBitmap(icon_id, wx.ART_MESSAGE_BOX, (64, 64))
        self._bmp = wx.StaticBitmap(self, bitmap=icon)
        bmpsz = wx.BoxSizer(wx.VERTICAL)
        bmpsz.AddMany([((10, 10), 0), (self._bmp, 0, wx.ALIGN_CENTER_VERTICAL),
                       ((10, 30), 0, wx.EXPAND)])
        hsizer.AddMany([((10, 10), 0), (bmpsz, 0, wx.ALIGN_TOP),
                        ((10, 10), 0), (vsizer, 1), ((10, 10), 0)])

        self.SetSizer(hsizer)
        self.SetInitialSize()
        self.SetAutoLayout(True)

    def GetChoiceControl(self):
        """Get the dialogs choice control
        @return: wx.Choice

        """
        return self._choices

    @ecbasewin.expose(ChoiceDialog)
    def GetSelection(self):
        """Get the chosen index
        @return: int

        """
        return self._selidx

    @ecbasewin.expose(ChoiceDialog)
    def GetStringSelection(self):
        """Get the chosen string
        @return: string

        """
        return self._selection

    def OnButton(self, evt):
        """Handle button events
        @param evt: wx.EVT_BUTTON
        @type evt: wx.CommandEvent

        """
        self.GetParent().EndModal(evt.GetId())

    def OnChoice(self, evt):
        """Update the selection
        @param evt: wx.EVT_CHOICE
        @type evt: wx.CommandEvent

        """
        if evt.GetEventObject() == self._choices:
            self._selection = self._choices.GetStringSelection()
            self._selidx = self._choices.GetSelection()
        else:
            evt.Skip()

    @ecbasewin.expose(ChoiceDialog)
    def SetBitmap(self, bmp):
        """Set the dialogs bitmap
        @param bmp: wx.Bitmap

        """
        self._bmp.SetBitmap(bmp)
        self.Layout()

    @ecbasewin.expose(ChoiceDialog)
    def SetChoices(self, choices):
        """Set the dialogs choices
        @param choices: list of strings

        """
        self._choices.SetItems(choices)
        self._choices.SetSelection(0)
        self._selection = self._choices.GetStringSelection()

    @ecbasewin.expose(ChoiceDialog)
    def SetSelection(self, sel):
        """Set the selected choice
        @param sel: int

        """
        self._choices.SetSelection(sel)
        self._selection = self._choices.GetStringSelection()
        self._selidx = self._choices.GetSelection()

    @ecbasewin.expose(ChoiceDialog)
    def SetStringSelection(self, sel):
        """Set the selected choice
        @param sel: string

        """
        self._choices.SetStringSelection(sel)
        self._selection = self._choices.GetStringSelection()
        self._selidx = self._choices.GetSelection()

#--------------------------------------------------------------------------#
