#----------------------------------------------------------------------
# Name:        wx.lib.msgpanel
# Purpose:     The MessagePanel class  (Note: this class used to live
#              in the demo's Main module.)
#
# Author:      Robin Dunn
#
# Created:     19-Oct-2009
# RCS-ID:      $Id: $
# Copyright:   (c) 2009 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

"""
MessagePanel is a simple panel class for displaying a message, very
much like how wx.MessageDialog works, including the icon flags.
"""

import wx

#----------------------------------------------------------------------

class MessagePanel(wx.Panel):
    def __init__(self, parent, message, caption='', flags=0):
        wx.Panel.__init__(self, parent)

        # Make widgets
        if flags:
            artid = None
            if flags & wx.ICON_EXCLAMATION:
                artid = wx.ART_WARNING            
            elif flags & wx.ICON_ERROR:
                artid = wx.ART_ERROR
            elif flags & wx.ICON_QUESTION:
                artid = wx.ART_QUESTION
            elif flags & wx.ICON_INFORMATION:
                artid = wx.ART_INFORMATION

            if artid is not None:
                bmp = wx.ArtProvider.GetBitmap(artid, wx.ART_MESSAGE_BOX, (32,32))
                icon = wx.StaticBitmap(self, -1, bmp)
            else:
                icon = (32,32) # make a spacer instead

        if caption:
            caption = wx.StaticText(self, -1, caption)
            caption.SetFont(wx.Font(24, wx.SWISS, wx.NORMAL, wx.BOLD))

        message = wx.StaticText(self, -1, message)

        # add to sizers for layout
        tbox = wx.BoxSizer(wx.VERTICAL)
        if caption:
            tbox.Add(caption)
            tbox.Add((10,10))
        tbox.Add(message)
        
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        hbox.Add((10,10), 1)
        hbox.Add(icon)
        hbox.Add((10,10))
        hbox.Add(tbox)
        hbox.Add((10,10), 1)

        box = wx.BoxSizer(wx.VERTICAL)
        box.Add((10,10), 1)
        box.Add(hbox, 0, wx.EXPAND)
        box.Add((10,10), 2)

        self.SetSizer(box)
        self.Fit()

        
#----------------------------------------------------------------------


if __name__ == '__main__':
    app = wx.App(redirect=False)
    frm = wx.Frame(None, title='MessagePanel Test')
    pnl = MessagePanel(frm, flags=wx.ICON_EXCLAMATION,
                       caption="Please stand by...",
                       message="""\
This is a test.  This is a test of the emergency broadcast
system.  Had this been a real emergency, you would have
already been reduced to a pile of radioactive cinders and
wondering why 'duck and cover' didn't help.

This is only a test...""")
    frm.Sizer = wx.BoxSizer()
    frm.Sizer.Add(pnl, 1, wx.EXPAND)
    frm.Fit()
    frm.Show()
    app.MainLoop()
    
