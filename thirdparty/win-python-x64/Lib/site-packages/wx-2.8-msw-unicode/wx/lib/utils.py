#----------------------------------------------------------------------
# Name:        wx.lib.utils
# Purpose:     Miscelaneous utility functions
#
# Author:      Robin Dunn
#
# Created:     18-Jan-2009
# RCS-ID:      $Id: utils.py 58205 2009-01-18 20:37:52Z RD $
# Copyright:   (c) 2009 Total Control Software
# Licence:     wxWidgets license
#----------------------------------------------------------------------

"""
A few useful functions.  (Ok, only one so far...)
"""

import wx

#---------------------------------------------------------------------------

def AdjustRectToScreen(rect, adjust=(0,0)):
        """
        Compare the rect with the dinmensions of the display that the rect's
        upper left corner is positioned on. If it doesn't fit entirely on
        screen then attempt to make it do so either by repositioning the
        rectangle, resizing it, or both.  Returns the adjusted rectangle.
        
        If the adjustment value is given then it will be used to ensure that
        the rectangle is at least that much smaller than the display's client
        area.
        """
        assert isinstance(rect, wx.Rect)
        if -1 in rect.Get():
            # bail out if there are any -1's in the dimensions
            return rect
        
        dispidx = wx.Display.GetFromPoint(rect.Position)
        if dispidx == wx.NOT_FOUND:
            dispidx = 0
        ca = wx.Display(dispidx).GetClientArea()
        assert isinstance(ca, wx.Rect)

        # is it already fully visible?
        if ca.ContainsRect(rect):
            return rect
        
        # if not then try adjusting the position
        if not ca.Contains(rect.Position):
            rect.Position = ca.Position
            if ca.ContainsRect(rect):
                return rect
        dx = dy = 0
        if rect.right > ca.right:
            dx = ca.right - rect.right
        if rect.bottom > ca.bottom:
            dy = ca.bottom - rect.bottom
        rect.OffsetXY(dx, dy)
        
        # if the rectangle has been moved too far, then readjust the position
        # and also adjust the size
        if rect.left < ca.left:
            rect.width -= (ca.left - rect.left)
            rect.left = ca.left
        if rect.top < ca.top:
            rect.height -= (ca.top - rect.top)
            rect.top = ca.top
            
        # make final adjustments if needed
        adjust = wx.Size(*adjust)
        if rect.width > (ca.width - adjust.width):
            rect.width = ca.width - adjust.width
        if rect.height > (ca.height - adjust.height):
            rect.height = ca.height - adjust.height

        # return the result
        return rect


#---------------------------------------------------------------------------
