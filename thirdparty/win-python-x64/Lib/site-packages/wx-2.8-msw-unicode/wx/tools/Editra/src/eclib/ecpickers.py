###############################################################################
# Name: ecpickers.py                                                          #
# Purpose: Custom picker controls                                             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: Editra Control Pickers

Collection of various custom picker controls

Class: PyFontPicker
Custom font picker control

@summary: Various custom picker controls

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ecpickers.py 67672 2011-05-02 00:01:44Z CJP $"
__revision__ = "$Revision: 67672 $"

__all__ = ['PyFontPicker', 'FontChangeEvent',
           'EVT_FONT_CHANGED', 'edEVT_FONT_CHANGED']

#-----------------------------------------------------------------------------#
# Imports
import wx

_ = wx.GetTranslation
#-----------------------------------------------------------------------------#

edEVT_FONT_CHANGED = wx.NewEventType()
EVT_FONT_CHANGED = wx.PyEventBinder(edEVT_FONT_CHANGED, 1)
class FontChangeEvent(wx.PyCommandEvent):
    """General notification event"""
    def __init__(self, etype, eid, value=None, obj=None):
        super(FontChangeEvent, self).__init__(etype, eid)

        # Attributes
        self._value = value

        # Setup
        self.SetEventObject(obj)

    def GetValue(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._value

#-----------------------------------------------------------------------------#

class PyFontPicker(wx.Panel):
    """A slightly enhanced wx.FontPickerCtrl that displays the choosen font in
    the label text using the choosen font as well as the font's size using
    nicer formatting.

    """
    def __init__(self, parent, id_=wx.ID_ANY, default=wx.NullFont):
        """Initializes the PyFontPicker
        @param default: The font to initialize as selected in the control

        """
        super(PyFontPicker, self).__init__(parent, id_, style=wx.NO_BORDER)

        # Attributes
        if default == wx.NullFont:
            self._font = wx.SystemSettings_GetFont(wx.SYS_SYSTEM_FONT)
        else:
            self._font = default

        self._text = wx.StaticText(self)
        self._text.SetFont(default)
        self._text.SetLabel(u"%s - %dpt" % (self._font.GetFaceName(), \
                                            self._font.GetPointSize()))
        self._button = wx.Button(self, label=_("Set Font") + u'...')

        # Layout
        vsizer = wx.BoxSizer(wx.VERTICAL)
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.AddStretchSpacer()
        sizer.Add(self._text, 0, wx.ALIGN_CENTER_VERTICAL)
        sizer.AddStretchSpacer()
        sizer.Add(self._button, 0, wx.ALIGN_CENTER_VERTICAL)
        vsizer.AddMany([((1, 1), 0), (sizer, 0, wx.EXPAND), ((1, 1), 0)])
        self.SetSizer(vsizer)
        self.SetAutoLayout(True)

        # Event Handlers
        self.Bind(wx.EVT_BUTTON, lambda evt: self.ShowFontDlg(), self._button)
        self.Bind(wx.EVT_FONTPICKER_CHANGED, self.OnChange)

    def GetFontValue(self):
        """Gets the currently choosen font
        @return: wx.Font

        """
        return self._font

    def GetTextCtrl(self):
        """Gets the widgets text control
        @return: wx.StaticText

        """
        return self._text

    def OnChange(self, evt):
        """Updates the text control using our custom stylings after
        the font is changed.
        @param evt: The event that called this handler

        """
        font = evt.GetFont()
        if font.IsNull():
            return
        self._font = font
        self._text.SetFont(self._font)
        self._text.SetLabel(u"%s - %dpt" % (font.GetFaceName(), \
                                            font.GetPointSize()))
        self.Layout()
        evt = FontChangeEvent(edEVT_FONT_CHANGED, self.GetId(), self._font, self)
        wx.PostEvent(self.GetParent(), evt)

    def SetButtonLabel(self, label):
        """Sets the buttons label"""
        self._button.SetLabel(label)
        self._button.Refresh()
        self.Layout()

    def SetToolTipString(self, tip):
        """Sets the tooltip of the window
        @param tip: string

        """
        self._text.SetToolTipString(tip)
        self._button.SetToolTipString(tip)
        wx.Panel.SetToolTipString(self, tip)

    def ShowFontDlg(self):
        """Opens the FontDialog and processes the result"""
        fdata = wx.FontData()
        fdata.SetInitialFont(self._font)
        fdlg = wx.FontDialog(self.GetParent(), fdata)
        if fdlg.ShowModal() == wx.ID_OK:
            fdata = fdlg.GetFontData()
            wx.PostEvent(self, wx.FontPickerEvent(self, self.GetId(),
                                                  fdata.GetChosenFont()))
        fdlg.Destroy()
