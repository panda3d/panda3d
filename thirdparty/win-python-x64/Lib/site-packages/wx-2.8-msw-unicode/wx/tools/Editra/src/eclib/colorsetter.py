###############################################################################
# Name: colorsetter.py                                                        #
# Purpose: Color Picker/Setter Control                                        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: ColorSetter

Color picker control that has a text entry section for entering hex color codes,
there is also a button that previews the color and can be used to open a color
choice dialog.
       
"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: colorsetter.py 65202 2010-08-06 15:49:23Z CJP $"
__revision__ = "$Revision: 65202 $"

__all__ = ["ColorSetter", "ColorSetterEvent",
           "EVT_COLORSETTER", "csEVT_COLORSETTER"]

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.lib.colourselect as csel

from eclutil import HexToRGB

#-----------------------------------------------------------------------------#
# Globals
# NOTE: # is expected at end
HEX_CHARS = "0123456789ABCDEFabcdef#"

_ = wx.GetTranslation
#-----------------------------------------------------------------------------#

csEVT_COLORSETTER = wx.NewEventType()
EVT_COLORSETTER = wx.PyEventBinder(csEVT_COLORSETTER, 1)
class ColorSetterEvent(wx.PyCommandEvent):
    """Event to signal that text needs updating"""
    def __init__(self, etype, eid, value=None):
        """Creates the event object"""
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = value

    def GetValue(self):
        """Returns the value from the event.
        @return: the value of this event

        """
        return self._value

#-----------------------------------------------------------------------------#

class ColorSetter(wx.Panel):
    """Control for setting a hex color value or selecting it from a 
    Color Dialog.

    """
    def __init__(self, parent, id_, color=wx.NullColour):
        """Create the control, it is a composite of a colourSelect and
        and a text control.
        @keyword label: the hex string value to go in the text portion

        """
        super(ColorSetter, self).__init__(parent, id_)

        if isinstance(color, tuple):
            color = wx.Colour(*color)

        # Attributes
        self._label = color.GetAsString(wx.C2S_HTML_SYNTAX)
        self._txt = wx.TextCtrl(self,
                                value=self._label,
                                style=wx.TE_CENTER,
                                validator=HexValidator())
        txtheight = self._txt.GetTextExtent('#000000')[1]
        self._txt.SetMaxSize((-1, txtheight + 4))
        self._txt.SetToolTip(wx.ToolTip(_("Enter a hex color value")))
        self._cbtn = csel.ColourSelect(self, colour=color, size=(20, 20))
        self._preval = color
        self._DoLayout()

        # Event Handlers
        self.Bind(csel.EVT_COLOURSELECT, self.OnColour)
        self._txt.Bind(wx.EVT_KEY_UP, self.OnTextChange)
        self._txt.Bind(wx.EVT_TEXT_PASTE, self.OnTextChange)
        self._txt.Bind(wx.EVT_KEY_DOWN, self.OnValidateTxt)

    def __PostEvent(self):
        """Notify the parent window of any value changes to the control"""
        value = self._cbtn.GetValue()
        if not isinstance(value, wx.Colour):
            value = wx.Colour(*value)

        # Don't post update if value hasn't changed
        if value == self._preval:
            return

        self._preval = value
        evt = ColorSetterEvent(csEVT_COLORSETTER, self.GetId(), value)
        evt.SetEventObject(self)
        wx.PostEvent(self.GetParent(), evt)

    def __UpdateValues(self):
        """Update the values based on the current state of the text control"""
        self._txt.Freeze()
        cpos = self._txt.GetInsertionPoint()
        hexstr = self._txt.GetValue().replace('#', '').strip()
        valid = ''
        for char in hexstr:
            if char in HEX_CHARS[:-1]:
                valid = valid + char

        if len(valid) > 6:
            valid = valid[:6]

        valid = '#' + valid
        self._txt.SetValue(valid)
        self._txt.SetInsertionPoint(cpos)
        valid = valid + (u'0' * (6 - len(valid)))
        self._cbtn.SetValue(HexToRGB(valid))
        self._txt.Thaw()

    def _DoLayout(self):
        """Layout the controls"""
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(self._txt, 0, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL)
        sizer.Add((5, 5), 0)
        sizer.Add(self._cbtn, 0, wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL)
        self.SetSizer(sizer)

    def GetColour(self):
        """Returns the colour value of the control
        @return: wxColour object

        """
        return self._cbtn.GetValue()

    def GetLabel(self):
        """Gets the hex value from the text control
        @return: string '#123456'
        @note: ensures a full 6 digit hex value is returned, padding
               with zero's where necessary

        """
        hexstr = self._txt.GetValue()
        hexstr = hexstr.replace('#', '').replace(' ', '')
        hexstr = '#' + hexstr + ('0' * (6 - len(hexstr)))
        return hexstr

    def OnColour(self, evt):
        """Update the button and text control value
        when a choice is made in the colour dialog.
        @param evt: EVT_COLOURSELECT

        """
        e_val = evt.GetValue()[0:3]
        red, green, blue = (hex(val)[2:].upper() for val in e_val)
        hex_str = u"#%s%s%s" % (red.zfill(2), green.zfill(2), blue.zfill(2))
        self._txt.SetValue(hex_str)
        self._cbtn.SetValue(wx.Colour(*e_val))
        self.__PostEvent()

    def OnTextChange(self, evt=None):
        """Catch when text changes in the text control and update
        button accordingly.
        @keyword evt: event that called this handler

        """
        self.__UpdateValues()
        self.__PostEvent()

    def OnValidateTxt(self, evt):
        """Validate text to ensure only valid hex characters are entered
        @param evt: wxEVT_KEY_DOWN

        """
        code = evt.GetKeyCode()
        if code in (wx.WXK_DELETE, wx.WXK_BACK, wx.WXK_LEFT,
                    wx.WXK_RIGHT, wx.WXK_TAB) or evt.CmdDown():
            evt.Skip()
            return

        key = unichr(code)
        if (key.isdigit() and evt.ShiftDown()) or \
           evt.AltDown() or evt.MetaDown():
            return

        if key in HEX_CHARS and \
           (len(self._txt.GetValue().lstrip(u"#")) < 6 or \
            self._txt.GetStringSelection()):
            evt.Skip()

    def SetLabel(self, label):
        """Set the label value of the text control
        @param label: hex string to set label to

        """
        self._txt.SetValue(label)
        self.__UpdateValues()

    def SetValue(self, colour):
        """Set the color value of the button
        @param colour: wxColour or 3 tuple to set color value to

        """
        self._cbtn.SetValue(colour)
        self._preval = colour
        red, green, blue = (hex(val)[2:].zfill(2).upper() for val in colour[0:3])
        hex_str = u"#%s%s%s" % (red, green, blue)
        self._txt.SetValue(hex_str)

#-----------------------------------------------------------------------------#

class HexValidator(wx.PyValidator):
    """Validate Hex strings for the color setter"""
    def __init__(self):
        """Initialize the validator

        """
        super(HexValidator, self).__init__()

        # Event Handlers
        self.Bind(wx.EVT_CHAR, self.OnChar)

    def Clone(self):
        """Clones the current validator
        @return: clone of this object

        """
        return HexValidator()

    def Validate(self, win):
        """Validate an window value
        @param win: window to validate

        """
        for char in val:
            if char not in HEX_CHARS:
                return False
        else:
            return True

    def OnChar(self, event):
        """Process values as they are entered into the control
        @param event: event that called this handler

        """
        key = event.GetKeyCode()
        if event.CmdDown() or key < wx.WXK_SPACE or key == wx.WXK_DELETE or \
           key > 255 or chr(key) in HEX_CHARS[:-1]:
            event.Skip()
            return

        if not wx.Validator_IsSilent():
            wx.Bell()

        return
