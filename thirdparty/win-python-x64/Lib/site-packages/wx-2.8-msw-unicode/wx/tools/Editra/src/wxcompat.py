###############################################################################
# Name: wxcompat.py                                                           #
# Purpose: Help with compatibility between wx versions.                       #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
@summary: wx Compatibility helper module

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: wxcompat.py 63517 2010-02-19 02:44:37Z CJP $"
__revision__ = "$Revision: 63517 $"

#-----------------------------------------------------------------------------#
# Imports
import wx

#-----------------------------------------------------------------------------#

if wx.Platform == '__WXMAC__':
    # MacThemeColour is defined in wxPython2.9 but does not exist in 2.8
    # This is a 2.8 version of this method.
    if not hasattr(wx, 'MacThemeColour'):
        def MacThemeColour(theme_id):
            """Get a specified Mac theme colour
            @param theme_id: Carbon theme id
            @return: wx.Colour

            """
            brush = wx.Brush(wx.BLACK)
            brush.MacSetTheme(theme_id)
            return brush.GetColour()

        wx.MacThemeColour = MacThemeColour

    wx.SystemOptions.SetOptionInt("mac.textcontrol-use-spell-checker", 1)

# GetText is not available in 2.9 but GetItemLabel is not available pre 2.8.6
if wx.VERSION < (2, 8, 6, 0, ''):
    wx.MenuItem.GetItemLabel = wx.MenuItem.GetText
    wx.MenuItem.GetItemLabelText = wx.MenuItem.GetLabel
