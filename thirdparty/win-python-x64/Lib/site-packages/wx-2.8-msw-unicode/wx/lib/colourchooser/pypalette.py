"""
PyColourChooser
Copyright (C) 2002 Michael Gilfix

This file is part of PyColourChooser.

You should have received a file COPYING containing license terms
along with this program; if not, write to Michael Gilfix
(mgilfix@eecs.tufts.edu) for a copy.

This version of PyColourChooser is open source; you can redistribute it and/or
modify it under the terms listed in the file COPYING.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
"""
# 12/14/2003 - Jeff Grimmett (grimmtooth@softhome.net)
#
# o 2.5 compatability update.
#
# 12/21/2003 - Jeff Grimmett (grimmtooth@softhome.net)
#
# o wxPyColorChooser -> PyColorChooser
# o wxPyColourChooser -> PyColourChooser
# o Commented out wx.InitAllImageHandlers() (see comments at that
#   point for explanation
#

import  wx

import  canvas
import  colorsys

from wx.lib.embeddedimage import PyEmbeddedImage

Image = PyEmbeddedImage(
    "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")


class PyPalette(canvas.Canvas):
    """The Pure-Python Palette

    The PyPalette is a pure python implementation of a colour palette. The
    palette implementation here imitates the palette layout used by MS
    Windows and Adobe Photoshop.

    The actual palette image has been embedded as an XPM for speed. The
    actual reverse-engineered drawing algorithm is provided in the
    GeneratePaletteBMP() method. The algorithm is tweakable by supplying
    the granularity factor to improve speed at the cost of display
    beauty. Since the generator isn't used in real time, no one will
    likely care :) But if you need it for some sort of unforeseen realtime
    application, it's there.
    """

    HORIZONTAL_STEP = 2
    VERTICAL_STEP   = 4

    def __init__(self, parent, id):
        """Creates a palette object."""
        # Load the pre-generated palette XPM
        
        # Leaving this in causes warning messages in some cases.
        # It is the responsibility of the app to init the image
        # handlers, IAW RD
        #wx.InitAllImageHandlers()
        
        self.palette = Image.GetBitmap()
        canvas.Canvas.__init__ (self, parent, id, size=(200, 192))

    def GetValue(self, x, y):
        """Returns a colour value at a specific x, y coordinate pair. This
        is useful for determining the colour found a specific mouse click
        in an external event handler."""
        return self.buffer.GetPixelColour(x, y)

    def DrawBuffer(self):
        """Draws the palette XPM into the memory buffer."""
        #self.GeneratePaletteBMP ("foo.bmp")
        self.buffer.DrawBitmap(self.palette, 0, 0, 0)

    def HighlightPoint(self, x, y):
        """Highlights an area of the palette with a little circle around
        the coordinate point"""
        colour = wx.Colour(0, 0, 0)
        self.buffer.SetPen(wx.Pen(colour, 1, wx.SOLID))
        self.buffer.SetBrush(wx.Brush(colour, wx.TRANSPARENT))
        self.buffer.DrawCircle(x, y, 3)
        self.Refresh()

    def GeneratePaletteBMP(self, file_name, granularity=1):
        """The actual palette drawing algorithm.

        This used to be 100% reverse engineered by looking at the
        values on the MS map, but has since been redone Correctly(tm)
        according to the HSV (hue, saturation, value) colour model by
        Charl P. Botha <http://cpbotha.net/>.

        Speed is tweakable by changing the granularity factor, but
        that affects how nice the output looks (makes the vertical
        blocks bigger. This method was used to generate the embedded
        XPM data."""
        self.vertical_step = self.VERTICAL_STEP * granularity
        width, height = self.GetSize ()

        # simply iterate over hue (horizontal) and saturation (vertical)
        value = 1.0
        for y in range(0, height, self.vertical_step):
            saturation = 1.0 - float(y) / float(height)
            for x in range(0, width, self.HORIZONTAL_STEP):
                hue = float(x) / float(width)
                r,g,b = colorsys.hsv_to_rgb(hue, saturation, value)
                colour = wx.Colour(int(r * 255.0), int(g * 255.0), int(b * 255.0))
                self.buffer.SetPen(wx.Pen(colour, 1, wx.SOLID))
                self.buffer.SetBrush(wx.Brush(colour, wx.SOLID))
                self.buffer.DrawRectangle(x, y,
                                          self.HORIZONTAL_STEP, self.vertical_step)

        # this code is now simpler (and works)
        bitmap = self.buffer.GetBitmap()
        image = wx.ImageFromBitmap(bitmap)
        image.SaveFile (file_name, wx.BITMAP_TYPE_XPM)
