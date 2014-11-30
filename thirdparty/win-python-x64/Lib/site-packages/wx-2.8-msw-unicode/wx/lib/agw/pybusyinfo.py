"""
PyBusyInfo constructs a busy info window and displays a message in it.


Description
===========

PyBusyInfo constructs a busy info window and displays a message in it.

This class makes it easy to tell your user that the program is temporarily busy.
Just create a PyBusyInfo object, and within the current scope, a message window
will be shown.

For example::

    busy = PyBusyInfo("Please wait, working...")

    for i in xrange(10000):
        DoACalculation()

    del busy


It works by creating a window in the constructor, and deleting it in the destructor.
You may also want to call `wx.Yield()` to refresh the window periodically (in case
it had been obscured by other windows, for example).


Supported Platforms
===================

PyBusyInfo has been tested on the following platforms:
  * Windows (Windows XP).


Window Styles
=============

`No particular window styles are available for this class.`


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

PyBusyInfo is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 03 Dec 2009, 09.00 GMT

Version 0.1

"""


import wx

_ = wx.GetTranslation


class PyInfoFrame(wx.Frame):
    """ Base class for L{PyBusyInfo}. """

    def __init__(self, parent, message, title, icon):
        """
        Default class constructor.
        
        :param `parent`: the frame parent;
        :param `message`: the message to display in the L{PyBusyInfo};
        :param `title`: the main L{PyBusyInfo} title;
        :param `icon`: an icon to draw as the frame icon, an instance of `wx.Bitmap`.
        """
        
        wx.Frame.__init__(self, parent, wx.ID_ANY, title, wx.DefaultPosition,
                          wx.DefaultSize, wx.NO_BORDER|wx.FRAME_TOOL_WINDOW|wx.FRAME_SHAPED|wx.STAY_ON_TOP)

        panel = wx.Panel(self)
        panel.SetCursor(wx.HOURGLASS_CURSOR)

        self._message = message
        self._title = title
        self._icon = icon

        dc = wx.ClientDC(self)
        textWidth, textHeight, dummy = dc.GetMultiLineTextExtent(self._message)
        sizeText = wx.Size(textWidth, textHeight)

        self.SetClientSize((max(sizeText.x, 340) + 60, max(sizeText.y, 40) + 60))
        # need to size the panel correctly first so that text.Centre() works
        panel.SetSize(self.GetClientSize())

        # Bind the events to draw ourselves
        panel.Bind(wx.EVT_PAINT, self.OnPaint)
        panel.Bind(wx.EVT_ERASE_BACKGROUND, self.OnErase)
            
        self.Centre(wx.BOTH)

        # Create a non-rectangular region to set the frame shape
        size = self.GetSize()
        bmp = wx.EmptyBitmap(size.x, size.y)
        dc = wx.BufferedDC(None, bmp)
        dc.SetBackground(wx.Brush(wx.Colour(0, 0, 0), wx.SOLID))
        dc.Clear()
        dc.SetPen(wx.Pen(wx.Colour(0, 0, 0), 1))
        dc.DrawRoundedRectangle(0, 0, size.x, size.y, 12)                
        r = wx.RegionFromBitmapColour(bmp, wx.Colour(0, 0, 0))
        # Store the non-rectangular region
        self.reg = r

        if wx.Platform == "__WXGTK__":
            self.Bind(wx.EVT_WINDOW_CREATE, self.SetBusyShape)
        else:
            self.SetBusyShape()

        # Add a custom bitmap at the top (if any)


    def SetBusyShape(self, event=None):
        """
        Sets L{PyInfoFrame} shape using the region created from the bitmap.

        :param `event`: a `wx.WindowCreateEvent` event (GTK only, as GTK supports setting
         the window shape only during window creation).
        """

        self.SetShape(self.reg)
        if event:
            # GTK only
            event.Skip()
            

    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{PyInfoFrame}.

        :param `event`: a `wx.PaintEvent` to be processed.
        """

        panel = event.GetEventObject()
        
        dc = wx.BufferedPaintDC(panel)
        dc.Clear()

        # Fill the background with a gradient shading
        startColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        endColour = wx.WHITE

        rect = panel.GetRect()
        dc.GradientFillLinear(rect, startColour, endColour, wx.SOUTH)

        # Draw the label
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        dc.SetFont(font)

        # Draw the message
        rect2 = wx.Rect(*rect)
        rect2.height += 20
        dc.DrawLabel(self._message, rect2, alignment=wx.ALIGN_CENTER|wx.ALIGN_CENTER)

        # Draw the top title
        font.SetWeight(wx.BOLD)
        dc.SetFont(font)
        dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_CAPTIONTEXT)))
        dc.SetTextForeground(wx.SystemSettings_GetColour(wx.SYS_COLOUR_CAPTIONTEXT))

        if self._icon.IsOk():
            iconWidth, iconHeight = self._icon.GetWidth(), self._icon.GetHeight()
            dummy, textHeight = dc.GetTextExtent(self._title)
            textXPos, textYPos = iconWidth + 10, (iconHeight-textHeight)/2
            dc.DrawBitmap(self._icon, 5, 5, True)
        else:
            textXPos, textYPos = 5, 0
        
        dc.DrawText(self._title, textXPos, textYPos+5)
        dc.DrawLine(5, 25, rect.width-5, 25)

        size = self.GetSize()
        dc.SetPen(wx.Pen(startColour, 1))
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRoundedRectangle(0, 0, size.x, size.y-1, 12)
        

    def OnErase(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{PyInfoFrame}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        # This is empty on purpose, to avoid flickering
        pass

                
# -------------------------------------------------------------------- #
# The actual PyBusyInfo implementation
# -------------------------------------------------------------------- #

class PyBusyInfo(object):
    """
    Constructs a busy info window as child of parent and displays a message in it.
    """

    def __init__(self, message, parent=None, title=_("Busy"), icon=wx.NullBitmap):
        """
        Default class constructor.
        
        :param `parent`: the L{PyBusyInfo} parent;
        :param `message`: the message to display in the L{PyBusyInfo};
        :param `title`: the main L{PyBusyInfo} title;
        :param `icon`: an icon to draw as the frame icon, an instance of `wx.Bitmap`.

        :note: If `parent` is not ``None`` you must ensure that it is not closed
         while the busy info is shown.
        """

        self._infoFrame = PyInfoFrame(parent, message, title, icon)

        if parent and parent.HasFlag(wx.STAY_ON_TOP):
            # we must have this flag to be in front of our parent if it has it
            self._infoFrame.SetWindowStyleFlag(wx.STAY_ON_TOP)
            
        self._infoFrame.Show(True)
        self._infoFrame.Refresh()
        self._infoFrame.Update()
        

    def __del__(self):
        """ Overloaded method, for compatibility with wxWidgets. """

        self._infoFrame.Show(False)
        self._infoFrame.Destroy()


        
