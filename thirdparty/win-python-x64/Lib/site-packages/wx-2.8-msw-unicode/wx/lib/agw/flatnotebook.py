# --------------------------------------------------------------------------- #
# FLATNOTEBOOK Widget wxPython IMPLEMENTATION
#
# Original C++ Code From Eran. You Can Find It At:
#
# http://wxforum.shadonet.com/viewtopic.php?t=5761&start=0
#
# License: wxWidgets license
#
#
# Python Code By:
#
# Andrea Gavana, @ 02 Oct 2006
# Latest Revision: 12 Sep 2010, 10.00 GMT
#
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# andrea.gavana@gmail.com
# gavana@kpo.kz
#
# Or, Obviously, To The wxPython Mailing List!!!
#
#
# End Of Comments
# --------------------------------------------------------------------------- #

"""
FlatNotebook is a full, generic and owner-drawn implementation of `wx.Notebook`.


Description
===========

The FlatNotebook is a full implementation of the `wx.Notebook`, and designed to be
a drop-in replacement for `wx.Notebook`. The API functions are similar so one can
expect the function to behave in the same way. 

Some features:

- The buttons are highlighted a la Firefox style;
- The scrolling is done for bulks of tabs (so, the scrolling is faster and better);
- The buttons area is never overdrawn by tabs (unlike many other implementations I saw);
- It is a generic control;
- Currently there are 6 different styles - VC8, VC 71, Standard, Fancy, Firefox 2 and Ribbon;
- Mouse middle click can be used to close tabs;
- A function to add right click menu for tabs (simple as L{SetRightClickMenu});
- All styles has bottom style as well (they can be drawn in the bottom of screen);
- An option to hide 'X' button or navigation buttons (separately);
- Gradient colouring of the selected tabs and border;
- Support for drag 'n' drop of tabs, both in the same notebook or to another notebook;
- Possibility to have closing button on the active tab directly;
- Support for disabled tabs;
- Colours for active/inactive tabs, and captions;
- Background of tab area can be painted in gradient (VC8 style only);
- Colourful tabs - a random gentle colour is generated for each new tab (very cool, VC8 style only);
- Try setting the tab area colour for the Ribbon Style.


And much more.


Window Styles
=============

This class supports the following window styles:

================================ =========== ==================================================
Window Styles                    Hex Value   Description
================================ =========== ==================================================
``FNB_VC71``                             0x1 Use Visual Studio 2003 (VC7.1) style for tabs.
``FNB_FANCY_TABS``                       0x2 Use fancy style - square tabs filled with gradient colouring.
``FNB_TABS_BORDER_SIMPLE``               0x4 Draw thin border around the page.
``FNB_NO_X_BUTTON``                      0x8 Do not display the 'X' button.
``FNB_NO_NAV_BUTTONS``                  0x10 Do not display the right/left arrows.
``FNB_MOUSE_MIDDLE_CLOSES_TABS``        0x20 Use the mouse middle button for cloing tabs.
``FNB_BOTTOM``                          0x40 Place tabs at bottom - the default is to place them at top.
``FNB_NODRAG``                          0x80 Disable dragging of tabs.
``FNB_VC8``                            0x100 Use Visual Studio 2005 (VC8) style for tabs.
``FNB_X_ON_TAB``                       0x200 Place 'X' close button on the active tab.
``FNB_BACKGROUND_GRADIENT``            0x400 Use gradients to paint the tabs background.
``FNB_COLOURFUL_TABS``                 0x800 Use colourful tabs (VC8 style only).
``FNB_DCLICK_CLOSES_TABS``            0x1000 Style to close tab using double click.
``FNB_SMART_TABS``                    0x2000 Use `Smart Tabbing`, like ``Alt`` + ``Tab`` on Windows.
``FNB_DROPDOWN_TABS_LIST``            0x4000 Use a dropdown menu on the left in place of the arrows.
``FNB_ALLOW_FOREIGN_DND``             0x8000 Allows drag 'n' drop operations between different FlatNotebooks.
``FNB_HIDE_ON_SINGLE_TAB``           0x10000 Hides the Page Container when there is one or fewer tabs.
``FNB_DEFAULT_STYLE``                0x10020 FlatNotebook default style.
``FNB_FF2``                          0x20000 Use Firefox 2 style for tabs.
``FNB_NO_TAB_FOCUS``                 0x40000 Does not allow tabs to have focus.
``FNB_RIBBON_TABS``                  0x80000 Use the Ribbon Tabs style 
``FNB_HIDE_TABS``                   0x100000 Hides the Page Container allowing only keyboard navigation
================================ =========== ==================================================


Events Processing
=================

This class processes the following events:

========================================= ==================================================
Event Name                                Description
========================================= ==================================================
``EVT_FLATNOTEBOOK_PAGE_CHANGED``         Notify client objects when the active page in `FlatNotebook` has changed.
``EVT_FLATNOTEBOOK_PAGE_CHANGING``        Notify client objects when the active page in `FlatNotebook` is about to change.
``EVT_FLATNOTEBOOK_PAGE_CLOSED``          Notify client objects when a page in `FlatNotebook` has been closed.
``EVT_FLATNOTEBOOK_PAGE_CLOSING``         Notify client objects when a page in `FlatNotebook` is closing.
``EVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU``    Notify client objects when a pop-up menu should appear next to a tab.
``EVT_FLATNOTEBOOK_PAGE_DROPPED``         Notify client objects when a tab has been dropped and re-arranged (on the *same* notebook)
``EVT_FLATNOTEBOOK_PAGE_DROPPED_FOREIGN`` Notify client objects when a tab has been dropped and re-arranged (from a foreign notebook)
========================================= ==================================================


License And Version
===================

FlatNotebook is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 12 Sep 2010, 10.00 GMT

Version 3.1
"""

__docformat__ = "epytext"


#----------------------------------------------------------------------
# Beginning Of FLATNOTEBOOK wxPython Code
#----------------------------------------------------------------------

import wx
import random
import math
import weakref
import cPickle

# Used on OSX to get access to carbon api constants
if wx.Platform == '__WXMAC__':
    import Carbon.Appearance

# Check for the new method in 2.7 (not present in 2.6.3.3)
if wx.VERSION_STRING < "2.7":
    wx.Rect.Contains = lambda self, point: wx.Rect.Inside(self, point)

FNB_HEIGHT_SPACER = 10

# Use Visual Studio 2003 (VC7.1) style for tabs
FNB_VC71 = 1
"""Use Visual Studio 2003 (VC7.1) style for tabs"""

# Use fancy style - square tabs filled with gradient colouring
FNB_FANCY_TABS = 2
"""Use fancy style - square tabs filled with gradient colouring"""

# Draw thin border around the page
FNB_TABS_BORDER_SIMPLE = 4
"""Draw thin border around the page"""

# Do not display the 'X' button
FNB_NO_X_BUTTON = 8
"""Do not display the 'X' button"""

# Do not display the Right / Left arrows
FNB_NO_NAV_BUTTONS = 16
"""Do not display the right/left arrows"""

# Use the mouse middle button for cloing tabs
FNB_MOUSE_MIDDLE_CLOSES_TABS = 32
"""Use the mouse middle button for cloing tabs"""

# Place tabs at bottom - the default is to place them
# at top
FNB_BOTTOM = 64
"""Place tabs at bottom - the default is to place them at top"""

# Disable dragging of tabs
FNB_NODRAG = 128
"""Disable dragging of tabs"""

# Use Visual Studio 2005 (VC8) style for tabs
FNB_VC8 = 256
"""Use Visual Studio 2005 (VC8) style for tabs"""

# Firefox 2 tabs style
FNB_FF2 = 131072
"""Use Firefox 2 style for tabs"""

# Place 'X' on a tab
FNB_X_ON_TAB = 512
"""Place 'X' close button on the active tab"""

FNB_BACKGROUND_GRADIENT = 1024
"""Use gradients to paint the tabs background"""

FNB_COLOURFUL_TABS = 2048
"""Use colourful tabs (VC8 style only)"""

# Style to close tab using double click - styles 1024, 2048 are reserved
FNB_DCLICK_CLOSES_TABS = 4096
"""Style to close tab using double click"""

FNB_SMART_TABS = 8192
"""Use `Smart Tabbing`, like ``Alt`` + ``Tab`` on Windows"""

FNB_DROPDOWN_TABS_LIST = 16384
"""Use a dropdown menu on the left in place of the arrows"""

FNB_ALLOW_FOREIGN_DND = 32768
"""Allows drag 'n' drop operations between different L{FlatNotebook}s"""

FNB_HIDE_ON_SINGLE_TAB = 65536
"""Hides the Page Container when there is one or fewer tabs"""

FNB_NO_TAB_FOCUS = 262144
""" Does not allow tabs to have focus"""

FNB_RIBBON_TABS = 0x80000
"""Use Ribbon style for tabs"""

FNB_HIDE_TABS = 0x100000
"""Hides the tabs allowing only keyboard navigation between pages"""

VERTICAL_BORDER_PADDING = 4

# Button size is a 16x16 xpm bitmap
BUTTON_SPACE = 16
"""Button size is a 16x16 xpm bitmap"""

VC8_SHAPE_LEN = 16

MASK_COLOUR  = wx.Colour(0, 128, 128)
"""Mask colour for the arrow bitmaps"""

# Button status
FNB_BTN_PRESSED = 2
"""Navigation button is pressed"""
FNB_BTN_HOVER = 1
"""Navigation button is hovered"""
FNB_BTN_NONE = 0
"""No navigation"""

# Hit Test results
FNB_TAB = 1             # On a tab
"""Indicates mouse coordinates inside a tab"""
FNB_X = 2               # On the X button
"""Indicates mouse coordinates inside the X region"""
FNB_TAB_X = 3           # On the 'X' button (tab's X button)
"""Indicates mouse coordinates inside the X region in a tab"""
FNB_LEFT_ARROW = 4      # On the rotate left arrow button
"""Indicates mouse coordinates inside the left arrow region"""
FNB_RIGHT_ARROW = 5     # On the rotate right arrow button
"""Indicates mouse coordinates inside the right arrow region"""
FNB_DROP_DOWN_ARROW = 6 # On the drop down arrow button
"""Indicates mouse coordinates inside the drop down arrow region"""
FNB_NOWHERE = 0         # Anywhere else
"""Indicates mouse coordinates not on any tab of the notebook"""

FNB_DEFAULT_STYLE = FNB_MOUSE_MIDDLE_CLOSES_TABS | FNB_HIDE_ON_SINGLE_TAB
"""L{FlatNotebook} default style"""

# FlatNotebook Events:
# wxEVT_FLATNOTEBOOK_PAGE_CHANGED: Event Fired When You Switch Page;
# wxEVT_FLATNOTEBOOK_PAGE_CHANGING: Event Fired When You Are About To Switch
# Pages, But You Can Still "Veto" The Page Changing By Avoiding To Call
# event.Skip() In Your Event Handler;
# wxEVT_FLATNOTEBOOK_PAGE_CLOSING: Event Fired When A Page Is Closing, But
# You Can Still "Veto" The Page Changing By Avoiding To Call event.Skip()
# In Your Event Handler;
# wxEVT_FLATNOTEBOOK_PAGE_CLOSED: Event Fired When A Page Is Closed.
# wxEVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU: Event Fired When A Menu Pops-up In A Tab.
# wxEVT_FLATNOTEBOOK_PAGE_DROPPED: Event Fired When A Tab Is Dropped On The Same Notebook

wxEVT_FLATNOTEBOOK_PAGE_CHANGED = wx.wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED
wxEVT_FLATNOTEBOOK_PAGE_CHANGING = wx.wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING
wxEVT_FLATNOTEBOOK_PAGE_CLOSING = wx.NewEventType()
wxEVT_FLATNOTEBOOK_PAGE_CLOSED = wx.NewEventType()
wxEVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU = wx.NewEventType()
wxEVT_FLATNOTEBOOK_PAGE_DROPPED = wx.NewEventType()
wxEVT_FLATNOTEBOOK_PAGE_DROPPED_FOREIGN = wx.NewEventType()

#-----------------------------------#
#        FlatNotebookEvent
#-----------------------------------#

EVT_FLATNOTEBOOK_PAGE_CHANGED = wx.EVT_NOTEBOOK_PAGE_CHANGED
""" Notify client objects when the active page in `FlatNotebook` has changed."""
EVT_FLATNOTEBOOK_PAGE_CHANGING = wx.EVT_NOTEBOOK_PAGE_CHANGING
""" Notify client objects when the active page in `FlatNotebook` is about to change."""
EVT_FLATNOTEBOOK_PAGE_CLOSING = wx.PyEventBinder(wxEVT_FLATNOTEBOOK_PAGE_CLOSING, 1)
""" Notify client objects when a page in `FlatNotebook` is closing."""
EVT_FLATNOTEBOOK_PAGE_CLOSED = wx.PyEventBinder(wxEVT_FLATNOTEBOOK_PAGE_CLOSED, 1)
""" Notify client objects when a page in `FlatNotebook` has been closed."""
EVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU = wx.PyEventBinder(wxEVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU, 1)
""" Notify client objects when a pop-up menu should appear next to a tab."""
EVT_FLATNOTEBOOK_PAGE_DROPPED = wx.PyEventBinder(wxEVT_FLATNOTEBOOK_PAGE_DROPPED, 1)
""" Notify client objects when a tab has been dropped and re-arranged (on the *same* notebook)."""
EVT_FLATNOTEBOOK_PAGE_DROPPED_FOREIGN = wx.PyEventBinder(wxEVT_FLATNOTEBOOK_PAGE_DROPPED_FOREIGN, 1)
""" Notify client objects when a tab has been dropped and re-arranged (from a foreign notebook)."""

# Some icons in XPM format

left_arrow_disabled_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #555555",
    "# c #000000",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````.```````",
    "```````..```````",
    "``````.`.```````",
    "`````.``.```````",
    "````.```.```````",
    "`````.``.```````",
    "``````.`.```````",
    "```````..```````",
    "````````.```````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````"
    ]

x_button_pressed_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #9e9ede",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "`..............`",
    "`.############.`",
    "`.############.`",
    "`.############.`",
    "`.###aa####aa#.`",
    "`.####aa##aa##.`",
    "`.#####aaaa###.`",
    "`.######aa####.`",
    "`.#####aaaa###.`",
    "`.####aa##aa##.`",
    "`.###aa####aa#.`",
    "`.############.`",
    "`..............`",
    "````````````````",
    "````````````````"
    ]


left_arrow_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #555555",
    "# c #000000",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````.```````",
    "```````..```````",
    "``````...```````",
    "`````....```````",
    "````.....```````",
    "`````....```````",
    "``````...```````",
    "```````..```````",
    "````````.```````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````"
    ]

x_button_hilite_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #c9dafb",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "`..............`",
    "`.############.`",
    "`.############.`",
    "`.##aa####aa##.`",
    "`.###aa##aa###.`",
    "`.####aaaa####.`",
    "`.#####aa#####.`",
    "`.####aaaa####.`",
    "`.###aa##aa###.`",
    "`.##aa####aa##.`",
    "`.############.`",
    "`.############.`",
    "`..............`",
    "````````````````",
    "````````````````"
    ]

x_button_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #555555",
    "# c #000000",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````..````..````",
    "`````..``..`````",
    "``````....``````",
    "```````..```````",
    "``````....``````",
    "`````..``..`````",
    "````..````..````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````"
    ]

left_arrow_pressed_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #9e9ede",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "`..............`",
    "`.############.`",
    "`.############.`",
    "`.#######a####.`",
    "`.######aa####.`",
    "`.#####aaa####.`",
    "`.####aaaa####.`",
    "`.###aaaaa####.`",
    "`.####aaaa####.`",
    "`.#####aaa####.`",
    "`.######aa####.`",
    "`.#######a####.`",
    "`..............`",
    "````````````````",
    "````````````````"
    ]

left_arrow_hilite_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #c9dafb",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "`..............`",
    "`.############.`",
    "`.######a#####.`",
    "`.#####aa#####.`",
    "`.####aaa#####.`",
    "`.###aaaa#####.`",
    "`.##aaaaa#####.`",
    "`.###aaaa#####.`",
    "`.####aaa#####.`",
    "`.#####aa#####.`",
    "`.######a#####.`",
    "`.############.`",
    "`..............`",
    "````````````````",
    "````````````````"
    ]

right_arrow_disabled_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #555555",
    "# c #000000",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "````````````````",
    "````````````````",
    "```````.````````",
    "```````..```````",
    "```````.`.``````",
    "```````.``.`````",
    "```````.```.````",
    "```````.``.`````",
    "```````.`.``````",
    "```````..```````",
    "```````.````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````"
    ]

right_arrow_hilite_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #c9dafb",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "`..............`",
    "`.############.`",
    "`.####a#######.`",
    "`.####aa######.`",
    "`.####aaa#####.`",
    "`.####aaaa####.`",
    "`.####aaaaa###.`",
    "`.####aaaa####.`",
    "`.####aaa#####.`",
    "`.####aa######.`",
    "`.####a#######.`",
    "`.############.`",
    "`..............`",
    "````````````````",
    "````````````````"
    ]

right_arrow_pressed_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #9e9ede",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "`..............`",
    "`.############.`",
    "`.############.`",
    "`.#####a######.`",
    "`.#####aa#####.`",
    "`.#####aaa####.`",
    "`.#####aaaa###.`",
    "`.#####aaaaa##.`",
    "`.#####aaaa###.`",
    "`.#####aaa####.`",
    "`.#####aa#####.`",
    "`.#####a######.`",
    "`..............`",
    "````````````````",
    "````````````````"
    ]


right_arrow_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #555555",
    "# c #000000",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "````````````````",
    "````````````````",
    "```````.````````",
    "```````..```````",
    "```````...``````",
    "```````....`````",
    "```````.....````",
    "```````....`````",
    "```````...``````",
    "```````..```````",
    "```````.````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````"
    ]

down_arrow_hilite_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #c9dafb",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "``.............`",
    "``.###########.`",
    "``.###########.`",
    "``.###########.`",
    "``.#aaaaaaaaa#.`",
    "``.##aaaaaaa##.`",
    "``.###aaaaa###.`",
    "``.####aaa####.`",
    "``.#####a#####.`",
    "``.###########.`",
    "``.###########.`",
    "``.###########.`",
    "``.............`",
    "````````````````",
    "````````````````"
    ]

down_arrow_pressed_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #4766e0",
    "# c #9e9ede",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "``.............`",
    "``.###########.`",
    "``.###########.`",
    "``.###########.`",
    "``.###########.`",
    "``.###########.`",
    "``.#aaaaaaaaa#.`",
    "``.##aaaaaaa##.`",
    "``.###aaaaa###.`",
    "``.####aaa####.`",
    "``.#####a#####.`",
    "``.###########.`",
    "``.............`",
    "````````````````",
    "````````````````"
    ]


down_arrow_xpm = [
    "    16    16        8            1",
    "` c #008080",
    ". c #000000",
    "# c #000000",
    "a c #000000",
    "b c #000000",
    "c c #000000",
    "d c #000000",
    "e c #000000",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````.........```",
    "`````.......````",
    "``````.....`````",
    "```````...``````",
    "````````.```````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````",
    "````````````````"
    ]


#----------------------------------------------------------------------
from wx.lib.embeddedimage import PyEmbeddedImage

Mondrian = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAAHFJ"
    "REFUWIXt1jsKgDAQRdF7xY25cpcWC60kioI6Fm/ahHBCMh+BRmGMnAgEWnvPpzK8dvrFCCCA"
    "coD8og4c5Lr6WB3Q3l1TBwLYPuF3YS1gn1HphgEEEABcKERrGy0E3B0HFJg7C1N/f/kTBBBA"
    "+Vi+AMkgFEvBPD17AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------


def LightColour(colour, percent):
    """
    Brighten the input colour by a percentage.

    :param `colour`: a valid `wx.Colour` instance;
    :param `percent`: the percentage by which the input colour should be brightened.
    """
    
    end_colour = wx.WHITE
    
    rd = end_colour.Red() - colour.Red()
    gd = end_colour.Green() - colour.Green()
    bd = end_colour.Blue() - colour.Blue()

    high = 100

    # We take the percent way of the colour from colour -. white
    i = percent
    r = colour.Red() + ((i*rd*100)/high)/100
    g = colour.Green() + ((i*gd*100)/high)/100
    b = colour.Blue() + ((i*bd*100)/high)/100
    return wx.Colour(r, g, b)


def RandomColour(): 
    """ Creates a random colour. """
    
    r = random.randint(0, 255) # Random value betweem 0-255
    g = random.randint(0, 255) # Random value betweem 0-255
    b = random.randint(0, 255) # Random value betweem 0-255

    return wx.Colour(r, g, b)


def PaintStraightGradientBox(dc, rect, startColour, endColour, vertical=True):
    """
    Draws a gradient coloured box from `startColour` to `endColour`.

    :param `dc`: an instance of `wx.DC`;
    :param `rect`: the rectangle to fill with the gradient shading;
    :param `startColour`: the first colour in the gradient shading;
    :param `endColour`: the last colour in the gradient shading;
    :param `vertical`: ``True`` if the gradient shading is north to south, ``False``
     if it is east to west.
    """

    rd = endColour.Red() - startColour.Red()
    gd = endColour.Green() - startColour.Green()
    bd = endColour.Blue() - startColour.Blue()

    # Save the current pen and brush
    savedPen = dc.GetPen()
    savedBrush = dc.GetBrush()

    if vertical:
        high = rect.GetHeight()-1
    else:
        high = rect.GetWidth()-1

    if high < 1:
        return

    for i in xrange(high+1):
    
        r = startColour.Red() + ((i*rd*100)/high)/100
        g = startColour.Green() + ((i*gd*100)/high)/100
        b = startColour.Blue() + ((i*bd*100)/high)/100

        p = wx.Pen(wx.Colour(r, g, b))
        dc.SetPen(p)

        if vertical:
            dc.DrawLine(rect.x, rect.y+i, rect.x+rect.width, rect.y+i)
        else:
            dc.DrawLine(rect.x+i, rect.y, rect.x+i, rect.y+rect.height)
    
    # Restore the pen and brush
    dc.SetPen(savedPen)
    dc.SetBrush(savedBrush)


def AdjustColour(colour, percent, alpha=wx.ALPHA_OPAQUE):
    """
    Brighten/darken input colour by `percent` and adjust `alpha` channel if needed. 

    :param `colour`: colour object to adjust, an instance of `wx.Colour`;
    :param `percent`: percent to adjust ``+`` (brighten) or ``-`` (darken);
    :param `alpha`: amount to adjust the alpha channel.

    :return: The modified colour.    

    """
    
    radj, gadj, badj = [int(val * (abs(percent) / 100.)) for val in colour.Get()]

    if percent < 0:
        radj, gadj, badj = [val * -1 for val in [radj, gadj, badj]]
    else:
        radj, gadj, badj = [val or 255 for val in [radj, gadj, badj]]

    red = min(colour.Red() + radj, 255)
    green = min(colour.Green() + gadj, 255)
    blue = min(colour.Blue() + badj, 255)
    return wx.Colour(red, green, blue, alpha)


if wx.VERSION_STRING < "2.8.9.2":
    adjust_colour = AdjustColour
else:
    from wx.lib.colourutils import AdjustColour as adjust_colour


# -----------------------------------------------------------------------------
# Util functions
# -----------------------------------------------------------------------------

def DrawButton(dc, rect, focus, upperTabs):
    """
    Draws a L{FlatNotebook} tab.

    :param `dc`: an instance of `wx.DC`;
    :param `rect`: the tab's client rectangle;
    :param `focus`: ``True`` if the tab has focus, ``False`` otherwise;
    :param `upperTabs`: ``True`` if the tabs are at the top, ``False`` if they are
     at the bottom.
    """
    
    # Define the rounded rectangle base on the given rect
    # we need an array of 9 points for it
    regPts = [wx.Point() for indx in xrange(9)]

    if focus:
        if upperTabs:
            leftPt = wx.Point(rect.x, rect.y + (rect.height / 10)*8)
            rightPt = wx.Point(rect.x + rect.width - 2, rect.y + (rect.height / 10)*8)
        else:
            leftPt = wx.Point(rect.x, rect.y + (rect.height / 10)*5)
            rightPt = wx.Point(rect.x + rect.width - 2, rect.y + (rect.height / 10)*5)
    else:
        leftPt = wx.Point(rect.x, rect.y + (rect.height / 2))
        rightPt = wx.Point(rect.x + rect.width - 2, rect.y + (rect.height / 2))

    # Define the top region
    top = wx.RectPP(rect.GetTopLeft(), rightPt)
    bottom = wx.RectPP(leftPt, rect.GetBottomRight())

    topStartColour = wx.WHITE

    if not focus:
        topStartColour = LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE), 50)

    topEndColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
    bottomStartColour = topEndColour
    bottomEndColour = topEndColour

    # Incase we use bottom tabs, switch the colours
    if upperTabs:
        if focus:
            PaintStraightGradientBox(dc, top, topStartColour, topEndColour)
            PaintStraightGradientBox(dc, bottom, bottomStartColour, bottomEndColour)
        else:
            PaintStraightGradientBox(dc, top, topEndColour , topStartColour)
            PaintStraightGradientBox(dc, bottom, bottomStartColour, bottomEndColour)

    else:
        if focus:
            PaintStraightGradientBox(dc, bottom, topEndColour, bottomEndColour)
            PaintStraightGradientBox(dc, top,topStartColour,  topStartColour)
        else:
            PaintStraightGradientBox(dc, bottom, bottomStartColour, bottomEndColour)
            PaintStraightGradientBox(dc, top, topEndColour, topStartColour)
    
    dc.SetBrush(wx.TRANSPARENT_BRUSH)


# ---------------------------------------------------------------------------- #
# Class FNBDropSource
# Gives Some Custom UI Feedback during the DnD Operations
# ---------------------------------------------------------------------------- #

class FNBDropSource(wx.DropSource):
    """
    Give some custom UI feedback during the drag and drop operation in this
    function. It is called on each mouse move, so your implementation must
    not be too slow.
    """
    
    def __init__(self, win):
        """
        Default class constructor.
        Used internally.

        :param `win`: the source window for which we wish to provide UI feedback
         during drag and drop operations.
        """
        
        wx.DropSource.__init__(self, win)
        self._win = win


    def GiveFeedback(self, effect):
        """
        You may give some custom UI feedback during the drag and drop operation
        in this function. It is called on each mouse move, so your implementation
        must not be too slow.

        :param `effect`: the effect to implement. One of ``wx.DragCopy``, ``wx.DragMove``,
         ``wx.DragLink`` and ``wx.DragNone``.

        :return: Return ``False`` if you want default feedback, or ``True`` if you
         implement your own feedback. The return values is ignored under GTK.
        
        :note: To show your own custom drag and drop UI feedback, you must override
         this method.
        """

        self._win.DrawDragHint()
        return False


# ---------------------------------------------------------------------------- #
# Class FNBDragInfo
# Stores All The Information To Allow Drag And Drop Between Different
# FlatNotebooks.
# ---------------------------------------------------------------------------- #

class FNBDragInfo(object):
    """
    Stores all the information to allow drag and drop between different
    L{FlatNotebook} instances.
    """
    
    _map = weakref.WeakValueDictionary()

    def __init__(self, container, pageindex):
        """
        Default class constructor.

        :param `container`: the drag and drop container, a page in L{FlatNotebook};
        :param `pageindex`: the index of the tab that is actually being dragged.
        """
        
        self._id = id(container)
        FNBDragInfo._map[self._id] = container
        self._pageindex = pageindex


    def GetContainer(self):
        """ Returns the L{FlatNotebook} page (usually a panel). """
        
        return FNBDragInfo._map.get(self._id, None)


    def GetPageIndex(self):
        """ Returns the page index associated with a page. """

        return self._pageindex        


# ---------------------------------------------------------------------------- #
# Class FNBDropTarget
# Simply Used To Handle The OnDrop() Method When Dragging And Dropping Between
# Different FlatNotebooks.
# ---------------------------------------------------------------------------- #

class FNBDropTarget(wx.DropTarget):
    """
    Class used to handle the L{FlatNotebook.OnDropTarget} method when dragging and
    dropping between different L{FlatNotebook} instances.
    """
    
    def __init__(self, parent):
        """
        Default class constructor.

        :param `parent`: the window handling the drag and drop, an instance of
         L{FlatNotebook}.
        """
        
        wx.DropTarget.__init__(self)

        self._parent = parent
        self._dataobject = wx.CustomDataObject(wx.CustomDataFormat("FlatNotebook"))
        self.SetDataObject(self._dataobject)


    def OnData(self, x, y, dragres):
        """
        Called after `OnDrop` returns ``True``.

        By default this will usually call `GetData` and will return the suggested default value `dragres`.

        :param `x`: the current x position of the mouse while dragging and dropping;
        :param `y`: the current y position of the mouse while dragging and dropping;
        :param `dragres`: an optional default return value.
        """
        
        if not self.GetData():
            return wx.DragNone

        draginfo = self._dataobject.GetData()
        drginfo = cPickle.loads(draginfo)
        
        return self._parent.OnDropTarget(x, y, drginfo.GetPageIndex(), drginfo.GetContainer())


# ---------------------------------------------------------------------------- #
# Class PageInfo
# Contains parameters for every FlatNotebook page
# ---------------------------------------------------------------------------- #

class PageInfo(object):
    """
    This class holds all the information (caption, image, etc...) belonging to a
    single tab in L{FlatNotebook}.
    """
    
    def __init__(self, caption="", imageindex=-1, tabangle=0, enabled=True):
        """
        Default Class Constructor.

        :param `caption`: the tab caption;
        :param `imageindex`: the tab image index based on the assigned (set)
         `wx.ImageList` (if any);
        :param `tabangle`: the tab angle (only on standard tabs, from 0 to 15
         degrees);
        :param `enabled`: sets the tab as enabled or disabled.
        """

        self._strCaption = caption
        self._TabAngle = tabangle
        self._ImageIndex = imageindex
        self._bEnabled = enabled
        self._pos = wx.Point(-1, -1)
        self._size = wx.Size(-1, -1)
        self._region = wx.Region()
        self._xRect = wx.Rect()
        self._colour = None
        self._hasFocus = False
        self._pageTextColour = None


    def SetCaption(self, value):
        """
        Sets the tab caption.

        :param `value`: the new tab caption string.
        """
        
        self._strCaption = value


    def GetCaption(self):
        """ Returns the tab caption. """

        return self._strCaption


    def SetPosition(self, value):
        """
        Sets the tab position.

        :param `value`: an instance of `wx.Point`.
        """

        self._pos = value


    def GetPosition(self):
        """ Returns the tab position. """

        return self._pos


    def SetSize(self, value):
        """
        Sets the tab size.

        :param `value`: an instance of `wx.Size`.
        """

        self._size = value


    def GetSize(self):
        """ Returns the tab size. """

        return self._size


    def SetTabAngle(self, value):
        """
        Sets the tab header angle.

        :param `value`: the tab header angle (0 <= value <= 15 degrees).
        """

        self._TabAngle = min(45, value)


    def GetTabAngle(self):
        """ Returns the tab angle. """

        return self._TabAngle

    
    def SetImageIndex(self, value):
        """
        Sets the tab image index.

        :param `value`: an index within the L{FlatNotebook} image list specifying
         the image to use for this tab.
        """

        self._ImageIndex = value


    def GetImageIndex(self):
        """ Returns the tab image index. """

        return self._ImageIndex


    def GetPageTextColour(self):
        """
        Returns the tab text colour if it has been set previously, or ``None``
        otherwise.
        """

        return self._pageTextColour
    

    def SetPageTextColour(self, colour):
        """
        Sets the tab text colour for this tab.

        :param `colour`: an instance of `wx.Colour`. You can pass ``None`` or
         `wx.NullColour` to return to the default page text colour.
        """

        if colour is None or not colour.IsOk():
            self._pageTextColour = None
        else:
            self._pageTextColour = colour


    def GetEnabled(self):
        """ Returns whether the tab is enabled or not. """

        return self._bEnabled 


    def EnableTab(self, enabled):
        """
        Sets the tab enabled or disabled.

        :param `enabled`: ``True`` to enable a tab, ``False`` to disable it.
        """

        self._bEnabled = enabled 


    def SetRegion(self, points=[]):
        """
        Sets the tab region.

        :param `points`: a Python list of `wx.Points`
        """
        
        self._region = wx.RegionFromPoints(points) 


    def GetRegion(self):
        """ Returns the tab region. """

        return self._region  


    def SetXRect(self, xrect):
        """
        Sets the button 'X' area rect.

        :param `xrect`: an instance of `wx.Rect`, specifying the client rectangle
         of the 'X' button.
        """

        self._xRect = xrect 


    def GetXRect(self):
        """ Returns the button 'X' area rect. """

        return self._xRect 


    def GetColour(self):
        """ Returns the tab colour. """

        return self._colour 


    def SetColour(self, colour):
        """
        Sets the tab colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._colour = colour 


# ---------------------------------------------------------------------------- #
# Class FlatNotebookEvent
# ---------------------------------------------------------------------------- #

class FlatNotebookEvent(wx.PyCommandEvent):
    """
    This events will be sent when a ``EVT_FLATNOTEBOOK_PAGE_CHANGED``,
    ``EVT_FLATNOTEBOOK_PAGE_CHANGING``, ``EVT_FLATNOTEBOOK_PAGE_CLOSING``,
    ``EVT_FLATNOTEBOOK_PAGE_CLOSED`` and ``EVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU`` is
    mapped in the parent.
    """
        
    def __init__(self, eventType, eventId=1, nSel=-1, nOldSel=-1):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier;
        :param `nSel`: the current selection;
        :param `nOldSel`: the old selection.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self._eventType = eventType

        self.notify = wx.NotifyEvent(eventType, eventId)


    def GetNotifyEvent(self):
        """ Returns the actual `wx.NotifyEvent`. """
        
        return self.notify


    def IsAllowed(self):
        """
        Returns ``True`` if the change is allowed (L{Veto} hasn't been called) or
        ``False`` otherwise (if it was).
        """

        return self.notify.IsAllowed()


    def Veto(self):
        """
        Prevents the change announced by this event from happening.

        :note: It is in general a good idea to notify the user about the reasons
         for vetoing the change because otherwise the applications behaviour (which
         just refuses to do what the user wants) might be quite surprising.
        """

        self.notify.Veto()


    def Allow(self):
        """
        This is the opposite of L{Veto}: it explicitly allows the event to be processed.
        For most events it is not necessary to call this method as the events are
        allowed anyhow but some are forbidden by default (this will be mentioned
        in the corresponding event description).
        """

        self.notify.Allow()


    def SetSelection(self, nSel):
        """
        Sets the event selection.

        :param `nSel`: an integer specifying the new selection.
        """
        
        self._selection = nSel
        

    def SetOldSelection(self, nOldSel):
        """
        Sets the id of the page selected before the change.

        :param `nOldSel`: an integer specifying the old selection.
        """
        
        self._oldselection = nOldSel


    def GetSelection(self):
        """ Returns the currently selected page, or -1 if none was selected. """
        
        return self._selection
        

    def GetOldSelection(self):
        """ Returns the page that was selected before the change, -1 if none was selected. """
        
        return self._oldselection


# ---------------------------------------------------------------------------- #
# Class TabNavigatorWindow
# ---------------------------------------------------------------------------- #

class FlatNotebookDragEvent(FlatNotebookEvent):
    """
    This event will be sent when a ``EVT_FLATNOTEBOOK_PAGE_DRAGGED_FOREIGN`` is
    mapped in the parent.
    """

    def __init__(self, eventType, eventId=1, nSel=-1, nOldSel=-1):
        """
        Default class constructor.

        :param `eventType`: the event type;
        :param `eventId`: the event identifier;
        :param `nSel`: the current selection;
        :param `nOldSel`: the old selection.
        """

        wx.PyCommandEvent.__init__(self, eventType, eventId)
        self._eventType = eventType

        self.notify = wx.NotifyEvent(eventType, eventId)
        self._oldnotebook = -1
        self._newnotebook = -1


    def GetNotebook(self):
        """ Returns the new notebook. """
        
        return self._newnotebook


    def GetOldNotebook(self):
        """ Returns the old notebook. """

        return self._oldnotebook


    def SetNotebook(self, notebook):
        """
        Sets the new notebook.

        :param `notebook`: an instance of L{FlatNotebook}.
        """
        
        self._newnotebook = notebook


    def SetOldNotebook(self, old):
        """
        Sets the old notebook.

        :param `notebook`: an instance of L{FlatNotebook}.
        """

        self._oldnotebook = old

# ---------------------------------------------------------------------------- #
# Class TabNavigatorWindow
# ---------------------------------------------------------------------------- #

class TabNavigatorWindow(wx.Dialog):
    """
    This class is used to create a modal dialog that enables `Smart Tabbing`,
    similar to what you would get by hitting ``Alt`` + ``Tab`` on Windows.
    """

    def __init__(self, parent=None, icon=None):
        """
        Default class constructor.
        Used internally.

        :param `parent`: the L{TabNavigatorWindow} parent window;
        :param `icon`: a valid `wx.Bitmap` object representing the icon to be displayed
         in the L{TabNavigatorWindow}.
        """

        wx.Dialog.__init__(self, parent, wx.ID_ANY, "", style=0)

        self._selectedItem = -1
        self._indexMap = []
        
        if icon is None:
            self._bmp = Mondrian.GetBitmap()
        else:
            self._bmp = icon

        sz = wx.BoxSizer(wx.VERTICAL)
        
        self._listBox = wx.ListBox(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(200, 150), [], wx.LB_SINGLE | wx.NO_BORDER)
        
        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(wx.EmptyBitmap(1,1))
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetWeight(wx.BOLD)
        mem_dc.SetFont(font)

        panelHeight = mem_dc.GetCharHeight()
        panelHeight += 4 # Place a spacer of 2 pixels

        # Out signpost bitmap is 24 pixels
        if panelHeight < 24:
            panelHeight = 24
        
        self._panel = wx.Panel(self, wx.ID_ANY, wx.DefaultPosition, wx.Size(200, panelHeight))

        sz.Add(self._panel)
        sz.Add(self._listBox, 1, wx.EXPAND)
        
        self.SetSizer(sz)

        # Connect events to the list box
        self._listBox.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self._listBox.Bind(wx.EVT_NAVIGATION_KEY, self.OnNavigationKey)
        self._listBox.Bind(wx.EVT_LISTBOX_DCLICK, self.OnItemSelected)
        
        # Connect paint event to the panel
        self._panel.Bind(wx.EVT_PAINT, self.OnPanelPaint)
        self._panel.Bind(wx.EVT_ERASE_BACKGROUND, self.OnPanelEraseBg)

        self.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self._listBox.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self.PopulateListControl(parent)
        
        self.GetSizer().Fit(self)
        self.GetSizer().SetSizeHints(self)
        self.GetSizer().Layout()
        self.Centre()

        # Set focus on the list box to avoid having to click on it to change
        # the tab selection under GTK.
        self._listBox.SetFocus()


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` for the L{TabNavigatorWindow}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """
        
        if event.GetKeyCode() == wx.WXK_CONTROL:
            self.CloseDialog()


    def OnNavigationKey(self, event):
        """
        Handles the ``wx.EVT_NAVIGATION_KEY`` for the L{TabNavigatorWindow}.

        :param `event`: a `wx.NavigationKeyEvent` event to be processed.
        """

        selected = self._listBox.GetSelection()
        bk = self.GetParent()
        maxItems = bk.GetPageCount()
            
        if event.GetDirection():
        
            # Select next page
            if selected == maxItems - 1:
                itemToSelect = 0
            else:
                itemToSelect = selected + 1
        
        else:
        
            # Previous page
            if selected == 0:
                itemToSelect = maxItems - 1
            else:
                itemToSelect = selected - 1
        
        self._listBox.SetSelection(itemToSelect)


    def PopulateListControl(self, book):
        """
        Populates the L{TabNavigatorWindow} listbox with a list of tabs.

        :param `book`: an instance of L{FlatNotebook} containing the tabs to be
         displayed in the listbox.
        """

        selection = book.GetSelection()
        count = book.GetPageCount()
        
        self._listBox.Append(book.GetPageText(selection))
        self._indexMap.append(selection)
        
        prevSel = book.GetPreviousSelection()
        
        if prevSel != wx.NOT_FOUND:
        
            # Insert the previous selection as second entry 
            self._listBox.Append(book.GetPageText(prevSel))
            self._indexMap.append(prevSel)
        
        for c in xrange(count):
        
            # Skip selected page
            if c == selection:
                continue

            # Skip previous selected page as well
            if c == prevSel:
                continue

            self._listBox.Append(book.GetPageText(c))
            self._indexMap.append(c)

        # Select the next entry after the current selection
        self._listBox.SetSelection(0)
        dummy = wx.NavigationKeyEvent()
        dummy.SetDirection(True)
        self.OnNavigationKey(dummy)


    def OnItemSelected(self, event):
        """
        Handles the ``wx.EVT_LISTBOX_DCLICK`` for the L{TabNavigatorWindow}.

        :param `event`: a `wx.ListEvent` event to be processed.
        """

        self.CloseDialog()


    def CloseDialog(self):
        """
        Closes the L{TabNavigatorWindow} dialog, setting the new selection in
        L{FlatNotebook}.
        """

        bk = self.GetParent()
        self._selectedItem = self._listBox.GetSelection()
        iter = self._indexMap[self._selectedItem]
        bk._pages.FireEvent(iter)
        self.EndModal(wx.ID_OK)
        

    def OnPanelPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` for the L{TabNavigatorWindow} top panel.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.PaintDC(self._panel)
        rect = self._panel.GetClientRect()

        bmp = wx.EmptyBitmap(rect.width, rect.height)

        mem_dc = wx.MemoryDC()
        mem_dc.SelectObject(bmp)

        endColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)
        startColour = LightColour(endColour, 50)
        PaintStraightGradientBox(mem_dc, rect, startColour, endColour)

        # Draw the caption title and place the bitmap
        # get the bitmap optimal position, and draw it
        bmpPt, txtPt = wx.Point(), wx.Point()
        bmpPt.y = (rect.height - self._bmp.GetHeight())/2
        bmpPt.x = 3
        mem_dc.DrawBitmap(self._bmp, bmpPt.x, bmpPt.y, True)

        # get the text position, and draw it
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetWeight(wx.BOLD)
        mem_dc.SetFont(font)
        fontHeight = mem_dc.GetCharHeight()
        
        txtPt.x = bmpPt.x + self._bmp.GetWidth() + 4
        txtPt.y = (rect.height - fontHeight)/2
        mem_dc.SetTextForeground(wx.WHITE)
        mem_dc.DrawText("Opened tabs:", txtPt.x, txtPt.y)
        mem_dc.SelectObject(wx.NullBitmap)
        
        dc.DrawBitmap(bmp, 0, 0)


    def OnPanelEraseBg(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` for the L{TabNavigatorWindow} top panel.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass


# ---------------------------------------------------------------------------- #
# Class FNBRenderer
# ---------------------------------------------------------------------------- #

class FNBRenderer(object):
    """
    Parent class for the 6 renderers defined: `Standard`, `VC71`, `Fancy`, `Firefox 2`,
    `VC8` and `Ribbon`. This class implements the common methods of all 6 renderers.
    """

    def __init__(self):
        """ Default class constructor. """
        
        self._tabHeight = None

        if wx.Platform == "__WXMAC__":
            # Get proper highlight colour for focus rectangle from the
            # current Mac theme.  kThemeBrushFocusHighlight is
            # available on Mac OS 8.5 and higher
            if hasattr(wx, 'MacThemeColour'):
                c = wx.MacThemeColour(Carbon.Appearance.kThemeBrushFocusHighlight)
            else:
                brush = wx.Brush(wx.BLACK)
                brush.MacSetTheme(Carbon.Appearance.kThemeBrushFocusHighlight)
                c = brush.GetColour()
            self._focusPen = wx.Pen(c, 2, wx.SOLID)
        else:
            self._focusPen = wx.Pen(wx.BLACK, 1, wx.USER_DASH)
            self._focusPen.SetDashes([1, 1])
            self._focusPen.SetCap(wx.CAP_BUTT)


    def GetLeftButtonPos(self, pageContainer):
        """
        Returns the left button position in the navigation area.

        :param `pageContainer`: an instance of L{FlatNotebook}.
        """

        pc = pageContainer
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        rect = pc.GetClientRect()
        clientWidth = rect.width
        
        if agwStyle & FNB_NO_X_BUTTON:
            return clientWidth - 38
        else:
            return clientWidth - 54


    def GetRightButtonPos(self, pageContainer):
        """
        Returns the right button position in the navigation area.

        :param `pageContainer`: an instance of L{FlatNotebook}.
        """

        pc = pageContainer
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        rect = pc.GetClientRect()
        clientWidth = rect.width
        
        if agwStyle & FNB_NO_X_BUTTON:
            return clientWidth - 22
        else:
            return clientWidth - 38


    def GetDropArrowButtonPos(self, pageContainer):
        """
        Returns the drop down button position in the navigation area.

        :param `pageContainer`: an instance of L{FlatNotebook}.
        """

        return self.GetRightButtonPos(pageContainer)


    def GetXPos(self, pageContainer):
        """
        Returns the 'X' button position in the navigation area.

        :param `pageContainer`: an instance of L{FlatNotebook}.
        """

        pc = pageContainer
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        rect = pc.GetClientRect()
        clientWidth = rect.width
        
        if agwStyle & FNB_NO_X_BUTTON:
            return clientWidth
        else:
            return clientWidth - 22


    def GetButtonsAreaLength(self, pageContainer):
        """
        Returns the navigation area width.

        :param `pageContainer`: an instance of L{FlatNotebook}.
        """

        pc = pageContainer
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()

        # ''
        if agwStyle & FNB_NO_NAV_BUTTONS and agwStyle & FNB_NO_X_BUTTON and not agwStyle & FNB_DROPDOWN_TABS_LIST:
            return 0

        # 'x'        
        elif agwStyle & FNB_NO_NAV_BUTTONS and not agwStyle & FNB_NO_X_BUTTON and not agwStyle & FNB_DROPDOWN_TABS_LIST:
            return 22
        
        # '<>'
        if not agwStyle & FNB_NO_NAV_BUTTONS and agwStyle & FNB_NO_X_BUTTON and not agwStyle & FNB_DROPDOWN_TABS_LIST:
            return 53 - 16
        
        # 'vx'
        if agwStyle & FNB_DROPDOWN_TABS_LIST and not agwStyle & FNB_NO_X_BUTTON:
            return 22 + 16

        # 'v'
        if agwStyle & FNB_DROPDOWN_TABS_LIST and agwStyle & FNB_NO_X_BUTTON:
            return 22

        # '<>x'
        return 53


    def DrawArrowAccordingToState(self, dc, pc, rect):
        """
        Draws the left and right scrolling arrows.

        :param `dc`: an instance of `wx.DC`;
        :param `pc`: an instance of L{FlatNotebook};
        :param `rect`: the client rectangle containing the scrolling arrows.
        """
        
        lightFactor = (pc.HasAGWFlag(FNB_BACKGROUND_GRADIENT) and [70] or [0])[0]
        PaintStraightGradientBox(dc, rect, pc._tabAreaColour, LightColour(pc._tabAreaColour, lightFactor))

    
    def DrawLeftArrow(self, pageContainer, dc):
        """
        Draws the left navigation arrow.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.        
        """

        pc = pageContainer
        
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        if agwStyle & FNB_NO_NAV_BUTTONS:
            return

        # Make sure that there are pages in the container
        if not pc._pagesInfoVec:
            return

        # Set the bitmap according to the button status
        if pc._nLeftButtonStatus == FNB_BTN_HOVER:
            arrowBmp = wx.BitmapFromXPMData(left_arrow_hilite_xpm)
        elif pc._nLeftButtonStatus == FNB_BTN_PRESSED:
            arrowBmp = wx.BitmapFromXPMData(left_arrow_pressed_xpm)
        else:
            arrowBmp = wx.BitmapFromXPMData(left_arrow_xpm)

        if pc._nFrom == 0:
            # Handle disabled arrow
            arrowBmp = wx.BitmapFromXPMData(left_arrow_disabled_xpm)
        
        arrowBmp.SetMask(wx.Mask(arrowBmp, MASK_COLOUR))

        # Erase old bitmap
        posx = self.GetLeftButtonPos(pc)
        self.DrawArrowAccordingToState(dc, pc, wx.Rect(posx, 6, 16, 14))
        
        # Draw the new bitmap
        dc.DrawBitmap(arrowBmp, posx, 6, True)


    def DrawRightArrow(self, pageContainer, dc):
        """
        Draws the right navigation arrow.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.        
        """

        pc = pageContainer
        
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        if agwStyle & FNB_NO_NAV_BUTTONS:
            return

        # Make sure that there are pages in the container
        if not pc._pagesInfoVec:
            return

        # Set the bitmap according to the button status
        if pc._nRightButtonStatus == FNB_BTN_HOVER:        
            arrowBmp = wx.BitmapFromXPMData(right_arrow_hilite_xpm)
        elif pc._nRightButtonStatus == FNB_BTN_PRESSED:
            arrowBmp = wx.BitmapFromXPMData(right_arrow_pressed_xpm)
        else:
            arrowBmp = wx.BitmapFromXPMData(right_arrow_xpm)

        # Check if the right most tab is visible, if it is
        # don't rotate right anymore
        if pc._pagesInfoVec[-1].GetPosition() != wx.Point(-1, -1):
            arrowBmp = wx.BitmapFromXPMData(right_arrow_disabled_xpm)
        
        arrowBmp.SetMask(wx.Mask(arrowBmp, MASK_COLOUR))

        # erase old bitmap
        posx = self.GetRightButtonPos(pc)
        self.DrawArrowAccordingToState(dc, pc, wx.Rect(posx, 6, 16, 14))

        # Draw the new bitmap
        dc.DrawBitmap(arrowBmp, posx, 6, True)


    def DrawDropDownArrow(self, pageContainer, dc):
        """
        Draws the drop-down arrow in the navigation area.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.
        """

        pc = pageContainer
        
        # Check if this style is enabled
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        if not agwStyle & FNB_DROPDOWN_TABS_LIST:
            return

        # Make sure that there are pages in the container
        if not pc._pagesInfoVec:
            return

        if pc._nArrowDownButtonStatus == FNB_BTN_HOVER:
            downBmp = wx.BitmapFromXPMData(down_arrow_hilite_xpm)
        elif pc._nArrowDownButtonStatus == FNB_BTN_PRESSED:
            downBmp = wx.BitmapFromXPMData(down_arrow_pressed_xpm)
        else:
            downBmp = wx.BitmapFromXPMData(down_arrow_xpm)

        downBmp.SetMask(wx.Mask(downBmp, MASK_COLOUR))

        # erase old bitmap
        posx = self.GetDropArrowButtonPos(pc)
        self.DrawArrowAccordingToState(dc, pc, wx.Rect(posx, 6, 16, 14))

        # Draw the new bitmap
        dc.DrawBitmap(downBmp, posx, 6, True)


    def DrawX(self, pageContainer, dc):
        """
        Draw the 'X' navigation button in the navigation area.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.
        """

        pc = pageContainer
        
        # Check if this style is enabled
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()
        if agwStyle & FNB_NO_X_BUTTON:
            return

        # Make sure that there are pages in the container
        if not pc._pagesInfoVec:
            return

        # Set the bitmap according to the button status
        if pc._nXButtonStatus == FNB_BTN_HOVER:
            xbmp = wx.BitmapFromXPMData(x_button_hilite_xpm)
        elif pc._nXButtonStatus == FNB_BTN_PRESSED:
            xbmp = wx.BitmapFromXPMData(x_button_pressed_xpm)
        else:
            xbmp = wx.BitmapFromXPMData(x_button_xpm)

        xbmp.SetMask(wx.Mask(xbmp, MASK_COLOUR))
        
        # erase old bitmap
        posx = self.GetXPos(pc) 
        self.DrawArrowAccordingToState(dc, pc, wx.Rect(posx, 6, 16, 14))

        # Draw the new bitmap
        dc.DrawBitmap(xbmp, posx, 6, True)


    def DrawTabX(self, pageContainer, dc, rect, tabIdx, btnStatus):
        """
        Draws the 'X' in the selected tab.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `rect`: the current tab client rectangle;
        :param `tabIdx`: the index of the current tab;
        :param `btnStatus`: the status of the 'X' button in the current tab.
        """

        pc = pageContainer
        if not pc.HasAGWFlag(FNB_X_ON_TAB):
            return

        # We draw the 'x' on the active tab only
        if tabIdx != pc.GetSelection() or tabIdx < 0:
            return

        # Set the bitmap according to the button status
        
        if btnStatus == FNB_BTN_HOVER:
            xBmp = wx.BitmapFromXPMData(x_button_hilite_xpm)
        elif btnStatus == FNB_BTN_PRESSED:
            xBmp = wx.BitmapFromXPMData(x_button_pressed_xpm)
        else:
            xBmp = wx.BitmapFromXPMData(x_button_xpm)

        # Set the masking
        xBmp.SetMask(wx.Mask(xBmp, MASK_COLOUR))

        # Draw the new bitmap
        dc.DrawBitmap(xBmp, rect.x, rect.y, True)

        # Update the vector
        rr = wx.Rect(rect.x, rect.y, 14, 13)
        pc._pagesInfoVec[tabIdx].SetXRect(rr)


    def DrawTabsLine(self, pageContainer, dc, selTabX1=-1, selTabX2=-1):
        """
        Draws a line over the tabs.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `selTabX1`: first x coordinate of the tab line;
        :param `selTabX2`: second x coordinate of the tab line.        
        """

        pc = pageContainer
        
        clntRect = pc.GetClientRect()
        clientRect3 = wx.Rect(0, 0, clntRect.width, clntRect.height)

        if pc.HasAGWFlag(FNB_FF2):
            if not pc.HasAGWFlag(FNB_BOTTOM):
                fillColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
            else:
                fillColour = wx.WHITE

            dc.SetPen(wx.Pen(fillColour))

            if pc.HasAGWFlag(FNB_BOTTOM):

                dc.DrawLine(1, 0, clntRect.width-1, 0)
                dc.DrawLine(1, 1, clntRect.width-1, 1)

                dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)))
                dc.DrawLine(1, 2, clntRect.width-1, 2)

                dc.SetPen(wx.Pen(fillColour))
                dc.DrawLine(selTabX1 + 2, 2, selTabX2 - 1, 2)
                
            else:
                
                dc.DrawLine(1, clntRect.height, clntRect.width-1, clntRect.height)
                dc.DrawLine(1, clntRect.height-1, clntRect.width-1, clntRect.height-1)

                dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)))
                dc.DrawLine(1, clntRect.height-2, clntRect.width-1, clntRect.height-2)

                dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)))
                dc.DrawLine(selTabX1 + 2, clntRect.height-2, selTabX2-1, clntRect.height-2)

        else:
            
            if pc.HasAGWFlag(FNB_BOTTOM):
            
                clientRect = wx.Rect(0, 2, clntRect.width, clntRect.height - 2)
                clientRect2 = wx.Rect(0, 1, clntRect.width, clntRect.height - 1)
            
            else:
            
                clientRect = wx.Rect(0, 0, clntRect.width, clntRect.height - 2)
                clientRect2 = wx.Rect(0, 0, clntRect.width, clntRect.height - 1)
            
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetPen(wx.Pen(pc.GetSingleLineBorderColour()))
            dc.DrawRectangleRect(clientRect2)
            dc.DrawRectangleRect(clientRect3)

            dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)))
            dc.DrawRectangleRect(clientRect)

            if not pc.HasAGWFlag(FNB_TABS_BORDER_SIMPLE):
            
                dc.SetPen(wx.Pen((pc.HasAGWFlag(FNB_VC71) and [wx.Colour(247, 243, 233)] or [pc._tabAreaColour])[0]))
                dc.DrawLine(0, 0, 0, clientRect.height+1)
                
                if pc.HasAGWFlag(FNB_BOTTOM):
                
                    dc.DrawLine(0, clientRect.height+1, clientRect.width, clientRect.height+1)
                
                else:
                    
                    dc.DrawLine(0, 0, clientRect.width, 0)
                    
                dc.DrawLine(clientRect.width - 1, 0, clientRect.width - 1, clientRect.height+1)


    def CalcTabWidth(self, pageContainer, tabIdx, tabHeight):
        """
        Calculates the width of the input tab.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `tabIdx`: the index of the input tab;
        :param `tabHeight`: the height of the tab.
        """

        pc = pageContainer
        dc = wx.MemoryDC()
        dc.SelectObject(wx.EmptyBitmap(1,1))

        boldFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        boldFont.SetWeight(wx.FONTWEIGHT_BOLD)

        if pc.IsDefaultTabs():
            shapePoints = int(tabHeight*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))

        # Calculate the text length using the bold font, so when selecting a tab
        # its width will not change
        dc.SetFont(boldFont)
        width, pom = dc.GetTextExtent(pc.GetPageText(tabIdx))

        # Set a minimum size to a tab
        if width < 20:
            width = 20

        tabWidth = 2*pc._pParent.GetPadding() + width

        # Style to add a small 'x' button on the top right
        # of the tab
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
            # The xpm image that contains the 'x' button is 9 pixels
            spacer = 9
            if pc.HasAGWFlag(FNB_VC8):
                spacer = 4

            tabWidth += pc._pParent.GetPadding() + spacer
        
        if pc.IsDefaultTabs():
            # Default style
            tabWidth += 2*shapePoints

        hasImage = pc._ImageList != None and pc._pagesInfoVec[tabIdx].GetImageIndex() != -1

        # For VC71 style, we only add the icon size (16 pixels)
        if hasImage:
        
            if not pc.IsDefaultTabs():
                tabWidth += 16 + pc._pParent.GetPadding()
            else:
                # Default style
                tabWidth += 16 + pc._pParent.GetPadding() + shapePoints/2
        
        return tabWidth


    def CalcTabHeight(self, pageContainer):
        """
        Calculates the height of the input tab.

        :param `pageContainer`: an instance of L{FlatNotebook}.
        """

        if self._tabHeight:
            return self._tabHeight

        pc = pageContainer
        dc = wx.MemoryDC()
        dc.SelectObject(wx.EmptyBitmap(1,1))

        # For GTK it seems that we must do this steps in order
        # for the tabs will get the proper height on initialization
        # on MSW, preforming these steps yields wierd results
        normalFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        boldFont = normalFont

        if "__WXGTK__" in wx.PlatformInfo:
            boldFont.SetWeight(wx.FONTWEIGHT_BOLD)
            dc.SetFont(boldFont)

        height = dc.GetCharHeight()
        
        tabHeight = height + FNB_HEIGHT_SPACER # We use 8 pixels as padding
        if "__WXGTK__" in wx.PlatformInfo:
            # On GTK the tabs are should be larger
            tabHeight += 6

        self._tabHeight = tabHeight
        
        return tabHeight


    def DrawTabs(self, pageContainer, dc):
        """
        Actually draws the tabs in L{FlatNotebook}.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.
        """

        pc = pageContainer
        if "__WXMAC__" in wx.PlatformInfo:
            # Works well on MSW & GTK, however this lines should be skipped on MAC
            if not pc._pagesInfoVec or pc._nFrom >= len(pc._pagesInfoVec):
                pc.Hide()
                return
            
        # Get the text hight
        tabHeight = self.CalcTabHeight(pageContainer)
        agwStyle = pc.GetParent().GetAGWWindowStyleFlag()

        # Calculate the number of rows required for drawing the tabs
        rect = pc.GetClientRect()
        clientWidth = rect.width

        # Set the maximum client size
        pc.SetSizeHints(self.GetButtonsAreaLength(pc), tabHeight)
        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))

        if agwStyle & FNB_VC71:
            backBrush = wx.Brush(wx.Colour(247, 243, 233))
        else:
            backBrush = wx.Brush(pc._tabAreaColour)

        noselBrush = wx.Brush(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNFACE))
        selBrush = wx.Brush(pc._activeTabColour)

        size = pc.GetSize()

        # Background
        dc.SetTextBackground((agwStyle & FNB_VC71 and [wx.Colour(247, 243, 233)] or [pc.GetBackgroundColour()])[0])
        dc.SetTextForeground(pc._activeTextColour)
        dc.SetBrush(backBrush)

        # If border style is set, set the pen to be border pen
        if pc.HasAGWFlag(FNB_TABS_BORDER_SIMPLE):
            dc.SetPen(borderPen)
        else:
            colr = (pc.HasAGWFlag(FNB_VC71) and [wx.Colour(247, 243, 233)] or [pc.GetBackgroundColour()])[0]
            dc.SetPen(wx.Pen(colr))

        if pc.HasAGWFlag(FNB_FF2):
            lightFactor = (pc.HasAGWFlag(FNB_BACKGROUND_GRADIENT) and [70] or [0])[0]
            PaintStraightGradientBox(dc, pc.GetClientRect(), pc._tabAreaColour, LightColour(pc._tabAreaColour, lightFactor))
            dc.SetBrush(wx.TRANSPARENT_BRUSH)

        dc.DrawRectangle(0, 0, size.x, size.y)

        # We always draw the bottom/upper line of the tabs
        # regradless the style
        dc.SetPen(borderPen)

        if not pc.HasAGWFlag(FNB_FF2):
            self.DrawTabsLine(pc, dc)

        # Restore the pen
        dc.SetPen(borderPen)

        if pc.HasAGWFlag(FNB_VC71):
        
            greyLineYVal  = (pc.HasAGWFlag(FNB_BOTTOM) and [0] or [size.y - 2])[0]
            whiteLineYVal = (pc.HasAGWFlag(FNB_BOTTOM) and [3] or [size.y - 3])[0]

            pen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
            dc.SetPen(pen)

            # Draw thik grey line between the windows area and
            # the tab area
            for num in xrange(3):
                dc.DrawLine(0, greyLineYVal + num, size.x, greyLineYVal + num)

            wbPen = (pc.HasAGWFlag(FNB_BOTTOM) and [wx.BLACK_PEN] or [wx.WHITE_PEN])[0]
            dc.SetPen(wbPen)
            dc.DrawLine(1, whiteLineYVal, size.x - 1, whiteLineYVal)

            # Restore the pen
            dc.SetPen(borderPen)
        
        # Draw labels
        normalFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        boldFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        boldFont.SetWeight(wx.FONTWEIGHT_BOLD)
        dc.SetFont(boldFont)

        posx = pc._pParent.GetPadding()

        # Update all the tabs from 0 to 'pc._nFrom' to be non visible
        for i in xrange(pc._nFrom):
        
            pc._pagesInfoVec[i].SetPosition(wx.Point(-1, -1))
            pc._pagesInfoVec[i].GetRegion().Clear()

        count = pc._nFrom
        
        #----------------------------------------------------------
        # Go over and draw the visible tabs
        #----------------------------------------------------------
        x1 = x2 = -1
        for i in xrange(pc._nFrom, len(pc._pagesInfoVec)):
        
            dc.SetPen(borderPen)

            if not pc.HasAGWFlag(FNB_FF2):
                dc.SetBrush((i==pc.GetSelection() and [selBrush] or [noselBrush])[0])

            # Now set the font to the correct font
            dc.SetFont((i==pc.GetSelection() and [boldFont] or [normalFont])[0])

            # Add the padding to the tab width
            # Tab width:
            # +-----------------------------------------------------------+
            # | PADDING | IMG | IMG_PADDING | TEXT | PADDING | x |PADDING |
            # +-----------------------------------------------------------+
            tabWidth = self.CalcTabWidth(pageContainer, i, tabHeight)

            # Check if we can draw more
            if posx + tabWidth + self.GetButtonsAreaLength(pc) >= clientWidth:
                break

            count = count + 1
            
            # By default we clean the tab region
            pc._pagesInfoVec[i].GetRegion().Clear()

            # Clean the 'x' buttn on the tab.
            # A 'Clean' rectangle, is a rectangle with width or height
            # with values lower than or equal to 0
            pc._pagesInfoVec[i].GetXRect().SetSize(wx.Size(-1, -1))

            # Draw the tab (border, text, image & 'x' on tab)
            self.DrawTab(pc, dc, posx, i, tabWidth, tabHeight, pc._nTabXButtonStatus)

            if pc.GetSelection() == i:
                x1 = posx
                x2 = posx + tabWidth + 2

            # Restore the text forground
            dc.SetTextForeground(pc._activeTextColour)

            # Update the tab position & size
            posy = (pc.HasAGWFlag(FNB_BOTTOM) and [0] or [VERTICAL_BORDER_PADDING])[0]

            pc._pagesInfoVec[i].SetPosition(wx.Point(posx, posy))
            pc._pagesInfoVec[i].SetSize(wx.Size(tabWidth, tabHeight))
            self.DrawFocusRectangle(dc, pc, pc._pagesInfoVec[i])

            posx += tabWidth
        
        # Update all tabs that can not fit into the screen as non-visible
        for i in xrange(count, len(pc._pagesInfoVec)):
            pc._pagesInfoVec[i].SetPosition(wx.Point(-1, -1))
            pc._pagesInfoVec[i].GetRegion().Clear()
        
        # Draw the left/right/close buttons
        # Left arrow
        self.DrawLeftArrow(pc, dc)
        self.DrawRightArrow(pc, dc)
        self.DrawX(pc, dc)
        self.DrawDropDownArrow(pc, dc)

        if pc.HasAGWFlag(FNB_FF2):
            self.DrawTabsLine(pc, dc, x1, x2)


    def DrawFocusRectangle(self, dc, pageContainer, page):
        """
        Draws a focus rectangle like the native `wx.Notebooks`.

        :param `dc`: an instance of `wx.DC`;
        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `page`: an instance of L{PageInfo}, representing a page in the notebook.
        """
        
        if not page._hasFocus:
            return

        tabPos = wx.Point(*page.GetPosition())
        if pageContainer.GetParent().GetAGWWindowStyleFlag() & FNB_VC8:
            vc8ShapeLen = self.CalcTabHeight(pageContainer) - VERTICAL_BORDER_PADDING - 2
            tabPos.x += vc8ShapeLen
            
        rect = wx.RectPS(tabPos, page.GetSize())
        rect = wx.Rect(rect.x+2, rect.y+2, rect.width-4, rect.height-8)

        if wx.Platform == '__WXMAC__':
            rect.SetWidth(rect.GetWidth() + 1)

        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(self._focusPen)
        dc.DrawRoundedRectangleRect(rect, 2)
        

    def DrawDragHint(self, pc, tabIdx):
        """
        Draws tab drag hint, the default implementation is to do nothing.
        You can override this function to provide a nice feedback to user.

        :param `pc`: an instance of L{FlatNotebook};
        :param `tabIdx`: the index of the tab we are dragging.

        :note: To show your own custom drag and drop UI feedback, you must override
         this method in your derived class.        
        """
        
        pass


    def NumberTabsCanFit(self, pageContainer, fr=-1):
        """
        Calculates the number of tabs that can fit on the available space on screen.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `fr`: the current first visible tab.
        """

        pc = pageContainer
        
        rect = pc.GetClientRect()
        clientWidth = rect.width

        vTabInfo = []

        tabHeight = self.CalcTabHeight(pageContainer)

        # The drawing starts from posx 
        posx = pc._pParent.GetPadding()

        if fr < 0:
            fr = pc._nFrom

        for i in xrange(fr, len(pc._pagesInfoVec)):

            tabWidth = self.CalcTabWidth(pageContainer, i, tabHeight) 
            if posx + tabWidth + self.GetButtonsAreaLength(pc) >= clientWidth:
                break; 

            # Add a result to the returned vector 
            tabRect = wx.Rect(posx, VERTICAL_BORDER_PADDING, tabWidth , tabHeight)
            vTabInfo.append(tabRect)

            # Advance posx 
            posx += tabWidth + FNB_HEIGHT_SPACER

        return vTabInfo

    
# ---------------------------------------------------------------------------- #
# Class FNBRendererMgr
# A manager that handles all the renderers defined below and calls the
# appropriate one when drawing is needed
# ---------------------------------------------------------------------------- #

class FNBRendererMgr(object):
    """
    This class represents a manager that handles all the 6 renderers defined
    and calls the appropriate one when drawing is needed.
    """

    def __init__(self):
        """ Default class constructor. """
        
        # register renderers

        self._renderers = {}        
        self._renderers.update({-1: FNBRendererDefault()})
        self._renderers.update({FNB_VC71: FNBRendererVC71()})
        self._renderers.update({FNB_FANCY_TABS: FNBRendererFancy()})
        self._renderers.update({FNB_VC8: FNBRendererVC8()})
        self._renderers.update({FNB_RIBBON_TABS: FNBRendererRibbonTabs()})
        self._renderers.update({FNB_FF2: FNBRendererFirefox2()})


    def GetRenderer(self, style):
        """
        Returns the current renderer based on the style selected.

        :param `style`: represents one of the 6 implemented styles for L{FlatNotebook},
         namely one of these bits:

         ===================== ========= ======================
         Tabs style            Hex Value Description
         ===================== ========= ======================
         ``FNB_VC71``                0x1 Use Visual Studio 2003 (VC7.1) style for tabs
         ``FNB_FANCY_TABS``          0x2 Use fancy style - square tabs filled with gradient colouring
         ``FNB_VC8``               0x100 Use Visual Studio 2005 (VC8) style for tabs
         ``FNB_FF2``             0x20000 Use Firefox 2 style for tabs
         ``FNB_RIBBON_TABS``     0x80000 Use the Ribbon Tabs style to render the tabs
         ===================== ========= ======================

        """

        if style & FNB_VC71:
            return self._renderers[FNB_VC71]

        if style & FNB_FANCY_TABS:
            return self._renderers[FNB_FANCY_TABS]

        if style & FNB_VC8:
            return self._renderers[FNB_VC8]

        if style & FNB_FF2:
            return self._renderers[FNB_FF2]
        
        if style & FNB_RIBBON_TABS:
            return self._renderers[FNB_RIBBON_TABS]

        # the default is to return the default renderer
        return self._renderers[-1]


#------------------------------------------
# Default renderer 
#------------------------------------------

class FNBRendererDefault(FNBRenderer):
    """
    This class handles the drawing of tabs using the standard renderer.
    """
    
    def __init__(self):
        """ Default class constructor. """

        FNBRenderer.__init__(self)
        

    def DrawTab(self, pageContainer, dc, posx, tabIdx, tabWidth, tabHeight, btnStatus):
        """
        Draws a tab using the `Standard` style.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `posx`: the x position of the tab;
        :param `tabIdx`: the index of the tab;
        :param `tabWidth`: the tab's width;
        :param `tabHeight`: the tab's height;
        :param `btnStatus`: the status of the 'X' button inside this tab.
        """

        # Default style
        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))
        pc = pageContainer 

        tabPoints = [wx.Point() for ii in xrange(7)]
        tabPoints[0].x = posx
        tabPoints[0].y = (pc.HasAGWFlag(FNB_BOTTOM) and [2] or [tabHeight - 2])[0]

        tabPoints[1].x = int(posx+(tabHeight-2)*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))
        tabPoints[1].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - (VERTICAL_BORDER_PADDING+2)] or [(VERTICAL_BORDER_PADDING+2)])[0]

        tabPoints[2].x = tabPoints[1].x+2
        tabPoints[2].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - VERTICAL_BORDER_PADDING] or [VERTICAL_BORDER_PADDING])[0]

        tabPoints[3].x = int(posx+tabWidth-(tabHeight-2)*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))-2
        tabPoints[3].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - VERTICAL_BORDER_PADDING] or [VERTICAL_BORDER_PADDING])[0]

        tabPoints[4].x = tabPoints[3].x+2
        tabPoints[4].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - (VERTICAL_BORDER_PADDING+2)] or [(VERTICAL_BORDER_PADDING+2)])[0]

        tabPoints[5].x = int(tabPoints[4].x+(tabHeight-2)*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))
        tabPoints[5].y = (pc.HasAGWFlag(FNB_BOTTOM) and [2] or [tabHeight - 2])[0]

        tabPoints[6].x = tabPoints[0].x
        tabPoints[6].y = tabPoints[0].y
        
        if tabIdx == pc.GetSelection():
        
            # Draw the tab as rounded rectangle
            dc.DrawPolygon(tabPoints)
        
        else:
        
            if tabIdx != pc.GetSelection() - 1:
            
                # Draw a vertical line to the right of the text
                pt1x = tabPoints[5].x
                pt1y = (pc.HasAGWFlag(FNB_BOTTOM) and [4] or [tabHeight - 6])[0]
                pt2x = tabPoints[5].x
                pt2y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - 4] or [4])[0]
                dc.DrawLine(pt1x, pt1y, pt2x, pt2y)

        if tabIdx == pc.GetSelection():
        
            savePen = dc.GetPen()
            whitePen = wx.Pen(wx.WHITE)
            whitePen.SetWidth(1)
            dc.SetPen(whitePen)

            secPt = wx.Point(tabPoints[5].x + 1, tabPoints[5].y)
            dc.DrawLine(tabPoints[0].x, tabPoints[0].y, secPt.x, secPt.y)

            # Restore the pen
            dc.SetPen(savePen)
        
        # -----------------------------------
        # Text and image drawing
        # -----------------------------------

        # Text drawing offset from the left border of the
        # rectangle
        
        # The width of the images are 16 pixels
        padding = pc.GetParent().GetPadding()
        shapePoints = int(tabHeight*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))
        hasImage = pc._pagesInfoVec[tabIdx].GetImageIndex() != -1
        imageYCoord = (pc.HasAGWFlag(FNB_BOTTOM) and [6] or [8])[0]

        if hasImage:
            textOffset = 2*pc._pParent._nPadding + 16 + shapePoints/2 
        else:
            textOffset = pc._pParent._nPadding + shapePoints/2 

        textOffset += 2

        if tabIdx != pc.GetSelection():
        
            # Set the text background to be like the vertical lines
            dc.SetTextForeground(pc._pParent.GetNonActiveTabTextColour())
        
        if hasImage:
        
            imageXOffset = textOffset - 16 - padding
            pc._ImageList.Draw(pc._pagesInfoVec[tabIdx].GetImageIndex(), dc,
                                     posx + imageXOffset, imageYCoord,
                                     wx.IMAGELIST_DRAW_TRANSPARENT, True)

        pageTextColour = pc._pParent.GetPageTextColour(tabIdx)
        if pageTextColour is not None:
            dc.SetTextForeground(pageTextColour)
            
        dc.DrawText(pc.GetPageText(tabIdx), posx + textOffset, imageYCoord)

        # draw 'x' on tab (if enabled)
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
        
            textWidth, textHeight = dc.GetTextExtent(pc.GetPageText(tabIdx))
            tabCloseButtonXCoord = posx + textOffset + textWidth + 1

            # take a bitmap from the position of the 'x' button (the x on tab button)
            # this bitmap will be used later to delete old buttons
            tabCloseButtonYCoord = imageYCoord
            x_rect = wx.Rect(tabCloseButtonXCoord, tabCloseButtonYCoord, 16, 16)

            # Draw the tab
            self.DrawTabX(pc, dc, x_rect, tabIdx, btnStatus)            
        

#------------------------------------------
# Firefox2 renderer 
#------------------------------------------
class FNBRendererFirefox2(FNBRenderer):
    """
    This class handles the drawing of tabs using the `Firefox 2` renderer.
    """
    
    def __init__(self):
        """ Default class constructor. """

        FNBRenderer.__init__(self)

        
    def DrawTab(self, pageContainer, dc, posx, tabIdx, tabWidth, tabHeight, btnStatus):
        """
        Draws a tab using the `Firefox 2` style.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `posx`: the x position of the tab;
        :param `tabIdx`: the index of the tab;
        :param `tabWidth`: the tab's width;
        :param `tabHeight`: the tab's height;
        :param `btnStatus`: the status of the 'X' button inside this tab.
        """

        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))
        pc = pageContainer

        tabPoints = [wx.Point() for indx in xrange(7)]
        tabPoints[0].x = posx + 2
        tabPoints[0].y = (pc.HasAGWFlag(FNB_BOTTOM) and [2] or [tabHeight - 2])[0]

        tabPoints[1].x = tabPoints[0].x
        tabPoints[1].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - (VERTICAL_BORDER_PADDING+2)] or [(VERTICAL_BORDER_PADDING+2)])[0]

        tabPoints[2].x = tabPoints[1].x+2
        tabPoints[2].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - VERTICAL_BORDER_PADDING] or [VERTICAL_BORDER_PADDING])[0]

        tabPoints[3].x = posx + tabWidth - 2
        tabPoints[3].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - VERTICAL_BORDER_PADDING] or [VERTICAL_BORDER_PADDING])[0]

        tabPoints[4].x = tabPoints[3].x + 2
        tabPoints[4].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - (VERTICAL_BORDER_PADDING+2)] or [(VERTICAL_BORDER_PADDING+2)])[0]

        tabPoints[5].x = tabPoints[4].x
        tabPoints[5].y = (pc.HasAGWFlag(FNB_BOTTOM) and [2] or [tabHeight - 2])[0]

        tabPoints[6].x = tabPoints[0].x
        tabPoints[6].y = tabPoints[0].y

        #------------------------------------
        # Paint the tab with gradient
        #------------------------------------
        rr = wx.RectPP(tabPoints[2], tabPoints[5])
        DrawButton(dc, rr, pc.GetSelection() == tabIdx , not pc.HasAGWFlag(FNB_BOTTOM))

        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(borderPen)

        # Draw the tab as rounded rectangle
        dc.DrawPolygon(tabPoints)

        # -----------------------------------
        # Text and image drawing
        # -----------------------------------

        # The width of the images are 16 pixels
        padding = pc.GetParent().GetPadding()
        shapePoints = int(tabHeight*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))
        hasImage = pc._pagesInfoVec[tabIdx].GetImageIndex() != -1
        imageYCoord = (pc.HasAGWFlag(FNB_BOTTOM) and [6] or [8])[0]

        if hasImage:
            textOffset = 2*padding + 16 + shapePoints/2 
        else:
            textOffset = padding + shapePoints/2
            
        textOffset += 2

        if tabIdx != pc.GetSelection():
        
            # Set the text background to be like the vertical lines
            dc.SetTextForeground(pc._pParent.GetNonActiveTabTextColour())

        if hasImage:
            imageXOffset = textOffset - 16 - padding
            pc._ImageList.Draw(pc._pagesInfoVec[tabIdx].GetImageIndex(), dc,
                               posx + imageXOffset, imageYCoord,
                               wx.IMAGELIST_DRAW_TRANSPARENT, True)

        pageTextColour = pc._pParent.GetPageTextColour(tabIdx)
        if pageTextColour is not None:
            dc.SetTextForeground(pageTextColour)
        
        dc.DrawText(pc.GetPageText(tabIdx), posx + textOffset, imageYCoord)

        # draw 'x' on tab (if enabled)
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
        
            textWidth, textHeight = dc.GetTextExtent(pc.GetPageText(tabIdx))
            tabCloseButtonXCoord = posx + textOffset + textWidth + 1

            # take a bitmap from the position of the 'x' button (the x on tab button)
            # this bitmap will be used later to delete old buttons
            tabCloseButtonYCoord = imageYCoord
            x_rect = wx.Rect(tabCloseButtonXCoord, tabCloseButtonYCoord, 16, 16)

            # Draw the tab
            self.DrawTabX(pc, dc, x_rect, tabIdx, btnStatus)
        

#------------------------------------------------------------------
# Visual studio 7.1 
#------------------------------------------------------------------

class FNBRendererVC71(FNBRenderer):
    """
    This class handles the drawing of tabs using the `VC71` renderer.
    """

    def __init__(self):
        """ Default class constructor. """

        FNBRenderer.__init__(self)


    def DrawTab(self, pageContainer, dc, posx, tabIdx, tabWidth, tabHeight, btnStatus):
        """
        Draws a tab using the `VC71` style.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `posx`: the x position of the tab;
        :param `tabIdx`: the index of the tab;
        :param `tabWidth`: the tab's width;
        :param `tabHeight`: the tab's height;
        :param `btnStatus`: the status of the 'X' button inside this tab.
        """

        # Visual studio 7.1 style
        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))
        pc = pageContainer

        dc.SetPen((tabIdx == pc.GetSelection() and [wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))] or [borderPen])[0])
        dc.SetBrush((tabIdx == pc.GetSelection() and [wx.Brush(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))] or [wx.Brush(wx.Colour(247, 243, 233))])[0])

        if tabIdx == pc.GetSelection():
        
            posy = (pc.HasAGWFlag(FNB_BOTTOM) and [0] or [VERTICAL_BORDER_PADDING])[0]
            tabH = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - 5] or [tabHeight - 3])[0]
            dc.DrawRectangle(posx, posy, tabWidth, tabH) 

            # Draw a black line on the left side of the
            # rectangle
            dc.SetPen(wx.BLACK_PEN)

            blackLineY1 = VERTICAL_BORDER_PADDING
            blackLineY2 = tabH
            dc.DrawLine(posx + tabWidth, blackLineY1, posx + tabWidth, blackLineY2)

            # To give the tab more 3D look we do the following
            # Incase the tab is on top,
            # Draw a thik white line on topof the rectangle
            # Otherwise, draw a thin (1 pixel) black line at the bottom

            pen = wx.Pen((pc.HasAGWFlag(FNB_BOTTOM) and [wx.BLACK] or [wx.WHITE])[0])
            dc.SetPen(pen)
            whiteLinePosY = (pc.HasAGWFlag(FNB_BOTTOM) and [blackLineY2] or [VERTICAL_BORDER_PADDING ])[0]
            dc.DrawLine(posx , whiteLinePosY, posx + tabWidth + 1, whiteLinePosY)

            # Draw a white vertical line to the left of the tab
            dc.SetPen(wx.WHITE_PEN)
            if not pc.HasAGWFlag(FNB_BOTTOM):
                blackLineY2 += 1
                
            dc.DrawLine(posx, blackLineY1, posx, blackLineY2)
        
        else:
        
            # We dont draw a rectangle for non selected tabs, but only
            # vertical line on the left

            blackLineY1 = (pc.HasAGWFlag(FNB_BOTTOM) and [VERTICAL_BORDER_PADDING + 2] or [VERTICAL_BORDER_PADDING + 1])[0]
            blackLineY2 = pc.GetSize().y - 5 
            dc.DrawLine(posx + tabWidth, blackLineY1, posx + tabWidth, blackLineY2)
        
        # -----------------------------------
        # Text and image drawing
        # -----------------------------------

        # Text drawing offset from the left border of the
        # rectangle
        
        # The width of the images are 16 pixels
        padding = pc.GetParent().GetPadding()
        hasImage = pc._pagesInfoVec[tabIdx].GetImageIndex() != -1
        imageYCoord = (pc.HasAGWFlag(FNB_BOTTOM) and [5] or [8])[0]

        if hasImage:
            textOffset = 2*pc._pParent._nPadding + 16
        else:
            textOffset = pc._pParent._nPadding

        if tabIdx != pc.GetSelection():
        
            # Set the text background to be like the vertical lines
            dc.SetTextForeground(pc._pParent.GetNonActiveTabTextColour())
        
        if hasImage:
        
            imageXOffset = textOffset - 16 - padding
            pc._ImageList.Draw(pc._pagesInfoVec[tabIdx].GetImageIndex(), dc,
                                     posx + imageXOffset, imageYCoord,
                                     wx.IMAGELIST_DRAW_TRANSPARENT, True)

        pageTextColour = pc._pParent.GetPageTextColour(tabIdx)
        if pageTextColour is not None:
            dc.SetTextForeground(pageTextColour)
        
        dc.DrawText(pc.GetPageText(tabIdx), posx + textOffset, imageYCoord)
        
        # draw 'x' on tab (if enabled)
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
        
            textWidth, textHeight = dc.GetTextExtent(pc.GetPageText(tabIdx))
            tabCloseButtonXCoord = posx + textOffset + textWidth + 1

            # take a bitmap from the position of the 'x' button (the x on tab button)
            # this bitmap will be used later to delete old buttons
            tabCloseButtonYCoord = imageYCoord
            x_rect = wx.Rect(tabCloseButtonXCoord, tabCloseButtonYCoord, 16, 16)

            # Draw the tab
            self.DrawTabX(pc, dc, x_rect, tabIdx, btnStatus)                    


#------------------------------------------------------------------
# Fancy style
#------------------------------------------------------------------

class FNBRendererFancy(FNBRenderer):
    """
    This class handles the drawing of tabs using the `Fancy` renderer.
    """

    def __init__(self):
        """ Default class constructor. """

        FNBRenderer.__init__(self)


    def DrawTab(self, pageContainer, dc, posx, tabIdx, tabWidth, tabHeight, btnStatus):
        """
        Draws a tab using the `Fancy` style, similar to the `VC71` one but with gradients.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `posx`: the x position of the tab;
        :param `tabIdx`: the index of the tab;
        :param `tabWidth`: the tab's width;
        :param `tabHeight`: the tab's height;
        :param `btnStatus`: the status of the 'X' button inside this tab.
        """

        # Fancy tabs - like with VC71 but with the following differences:
        # - The Selected tab is coloured with gradient colour
        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))
        pc = pageContainer

        pen = (tabIdx == pc.GetSelection() and [wx.Pen(pc._pParent.GetBorderColour())] or [wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))])[0]

        if tabIdx == pc.GetSelection():
        
            posy = (pc.HasAGWFlag(FNB_BOTTOM) and [2] or [VERTICAL_BORDER_PADDING])[0]
            th = tabHeight - 5

            rect = wx.Rect(posx, posy, tabWidth, th)

            col2 = (pc.HasAGWFlag(FNB_BOTTOM) and [pc._pParent.GetGradientColourTo()] or [pc._pParent.GetGradientColourFrom()])[0]
            col1 = (pc.HasAGWFlag(FNB_BOTTOM) and [pc._pParent.GetGradientColourFrom()] or [pc._pParent.GetGradientColourTo()])[0]

            PaintStraightGradientBox(dc, rect, col1, col2)
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetPen(pen)
            dc.DrawRectangleRect(rect)

            # erase the bottom/top line of the rectangle
            dc.SetPen(wx.Pen(pc._pParent.GetGradientColourFrom()))
            if pc.HasAGWFlag(FNB_BOTTOM):
                dc.DrawLine(rect.x, 2, rect.x + rect.width, 2)
            else:
                dc.DrawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width, rect.y + rect.height - 1)
        
        else:
        
            # We dont draw a rectangle for non selected tabs, but only
            # vertical line on the left
            dc.SetPen(borderPen)
            dc.DrawLine(posx + tabWidth, VERTICAL_BORDER_PADDING + 3, posx + tabWidth, tabHeight - 4)
        

        # -----------------------------------
        # Text and image drawing
        # -----------------------------------

        # Text drawing offset from the left border of the
        # rectangle
        
        # The width of the images are 16 pixels
        padding = pc.GetParent().GetPadding()
        hasImage = pc._pagesInfoVec[tabIdx].GetImageIndex() != -1
        imageYCoord = (pc.HasAGWFlag(FNB_BOTTOM) and [6] or [8])[0]

        if hasImage:
            textOffset = 2*pc._pParent._nPadding + 16
        else:
            textOffset = pc._pParent._nPadding 

        textOffset += 2

        if tabIdx != pc.GetSelection():
        
            # Set the text background to be like the vertical lines
            dc.SetTextForeground(pc._pParent.GetNonActiveTabTextColour())
        
        if hasImage:
        
            imageXOffset = textOffset - 16 - padding
            pc._ImageList.Draw(pc._pagesInfoVec[tabIdx].GetImageIndex(), dc,
                                     posx + imageXOffset, imageYCoord,
                                     wx.IMAGELIST_DRAW_TRANSPARENT, True)

        pageTextColour = pc._pParent.GetPageTextColour(tabIdx)
        if pageTextColour is not None:
            dc.SetTextForeground(pageTextColour)
        
        dc.DrawText(pc.GetPageText(tabIdx), posx + textOffset, imageYCoord)
        
        # draw 'x' on tab (if enabled)
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
        
            textWidth, textHeight = dc.GetTextExtent(pc.GetPageText(tabIdx))
            tabCloseButtonXCoord = posx + textOffset + textWidth + 1

            # take a bitmap from the position of the 'x' button (the x on tab button)
            # this bitmap will be used later to delete old buttons
            tabCloseButtonYCoord = imageYCoord
            x_rect = wx.Rect(tabCloseButtonXCoord, tabCloseButtonYCoord, 16, 16)

            # Draw the tab
            self.DrawTabX(pc, dc, x_rect, tabIdx, btnStatus)            
        

#------------------------------------------------------------------
# Visual studio 2005 (VS8)
#------------------------------------------------------------------
class FNBRendererVC8(FNBRenderer):    
    """
    This class handles the drawing of tabs using the `VC8` renderer.
    """

    def __init__(self):
        """ Default class constructor. """

        FNBRenderer.__init__(self)
        self._first = True
        self._factor = 1

        
    def DrawTabs(self, pageContainer, dc):
        """
        Draws all the tabs using `VC8` style.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.
        """

        pc = pageContainer

        if "__WXMAC__" in wx.PlatformInfo:
            # Works well on MSW & GTK, however this lines should be skipped on MAC
            if not pc._pagesInfoVec or pc._nFrom >= len(pc._pagesInfoVec):
                pc.Hide()
                return
            
        # Get the text hight
        tabHeight = self.CalcTabHeight(pageContainer)

        # Set the font for measuring the tab height
        normalFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        boldFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        boldFont.SetWeight(wx.FONTWEIGHT_BOLD)

        # Calculate the number of rows required for drawing the tabs
        rect = pc.GetClientRect()

        # Set the maximum client size
        pc.SetSizeHints(self.GetButtonsAreaLength(pc), tabHeight)
        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))

        # Create brushes
        backBrush = wx.Brush(pc._tabAreaColour)
        noselBrush = wx.Brush(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNFACE))
        selBrush = wx.Brush(pc._activeTabColour)
        size = pc.GetSize()

        # Background
        dc.SetTextBackground(pc.GetBackgroundColour())
        dc.SetTextForeground(pc._activeTextColour)
        
        # If border style is set, set the pen to be border pen
        if pc.HasAGWFlag(FNB_TABS_BORDER_SIMPLE):
            dc.SetPen(borderPen)
        else:
            dc.SetPen(wx.TRANSPARENT_PEN)

        lightFactor = (pc.HasAGWFlag(FNB_BACKGROUND_GRADIENT) and [70] or [0])[0]
        
        # For VC8 style, we colour the tab area in gradient colouring
        lightcolour = LightColour(pc._tabAreaColour, lightFactor)
        PaintStraightGradientBox(dc, pc.GetClientRect(), pc._tabAreaColour, lightcolour)

        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.DrawRectangle(0, 0, size.x, size.y)
    
        # We always draw the bottom/upper line of the tabs
        # regradless the style
        dc.SetPen(borderPen)
        self.DrawTabsLine(pc, dc)

        # Restore the pen
        dc.SetPen(borderPen)

        # Draw labels
        dc.SetFont(boldFont)

        # Update all the tabs from 0 to 'pc.self._nFrom' to be non visible
        for i in xrange(pc._nFrom):
        
            pc._pagesInfoVec[i].SetPosition(wx.Point(-1, -1))
            pc._pagesInfoVec[i].GetRegion().Clear()
        
        # Draw the visible tabs, in VC8 style, we draw them from right to left
        vTabsInfo = self.NumberTabsCanFit(pc)

        activeTabPosx = 0
        activeTabWidth = 0
        activeTabHeight = 0

        for cur in xrange(len(vTabsInfo)-1, -1, -1):
        
            # 'i' points to the index of the currently drawn tab
            # in pc.GetPageInfoVector() vector
            i = pc._nFrom + cur
            dc.SetPen(borderPen)
            dc.SetBrush((i==pc.GetSelection() and [selBrush] or [noselBrush])[0])

            # Now set the font to the correct font
            dc.SetFont((i==pc.GetSelection() and [boldFont] or [normalFont])[0])

            # Add the padding to the tab width
            # Tab width:
            # +-----------------------------------------------------------+
            # | PADDING | IMG | IMG_PADDING | TEXT | PADDING | x |PADDING |
            # +-----------------------------------------------------------+

            tabWidth = self.CalcTabWidth(pageContainer, i, tabHeight)
            posx = vTabsInfo[cur].x

            # By default we clean the tab region
            # incase we use the VC8 style which requires
            # the region, it will be filled by the function
            # drawVc8Tab
            pc._pagesInfoVec[i].GetRegion().Clear()
            
            # Clean the 'x' buttn on the tab 
            # 'Clean' rectanlge is a rectangle with width or height
            # with values lower than or equal to 0
            pc._pagesInfoVec[i].GetXRect().SetSize(wx.Size(-1, -1))

            # Draw the tab
            # Incase we are drawing the active tab
            # we need to redraw so it will appear on top
            # of all other tabs

            # when using the vc8 style, we keep the position of the active tab so we will draw it again later
            if i == pc.GetSelection() and pc.HasAGWFlag(FNB_VC8):
            
                activeTabPosx = posx
                activeTabWidth = tabWidth
                activeTabHeight = tabHeight
            
            else:
            
                self.DrawTab(pc, dc, posx, i, tabWidth, tabHeight, pc._nTabXButtonStatus)
            
            # Restore the text forground
            dc.SetTextForeground(pc._activeTextColour)

            # Update the tab position & size
            pc._pagesInfoVec[i].SetPosition(wx.Point(posx, VERTICAL_BORDER_PADDING))
            pc._pagesInfoVec[i].SetSize(wx.Size(tabWidth, tabHeight))
        
        # Incase we are in VC8 style, redraw the active tab (incase it is visible)
        if pc.GetSelection() >= pc._nFrom and pc.GetSelection() < pc._nFrom + len(vTabsInfo):
        
            self.DrawTab(pc, dc, activeTabPosx, pc.GetSelection(), activeTabWidth, activeTabHeight, pc._nTabXButtonStatus)
        
        # Update all tabs that can not fit into the screen as non-visible
        for xx in xrange(pc._nFrom + len(vTabsInfo), len(pc._pagesInfoVec)):
        
            pc._pagesInfoVec[xx].SetPosition(wx.Point(-1, -1))
            pc._pagesInfoVec[xx].GetRegion().Clear()
        
        # Draw the left/right/close buttons 
        # Left arrow
        self.DrawLeftArrow(pc, dc)
        self.DrawRightArrow(pc, dc)
        self.DrawX(pc, dc)
        self.DrawDropDownArrow(pc, dc)


    def DrawTab(self, pageContainer, dc, posx, tabIdx, tabWidth, tabHeight, btnStatus):
        """
        Draws a tab using the `VC8` style.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `posx`: the x position of the tab;
        :param `tabIdx`: the index of the tab;
        :param `tabWidth`: the tab's width;
        :param `tabHeight`: the tab's height;
        :param `btnStatus`: the status of the 'X' button inside this tab.
        """

        pc = pageContainer
        borderPen = wx.Pen(pc._pParent.GetBorderColour())
        tabPoints = [wx.Point() for ii in xrange(8)]

        # If we draw the first tab or the active tab, 
        # we draw a full tab, else we draw a truncated tab
        #
        #             X(2)                  X(3)
        #        X(1)                            X(4)
        #                                          
        #                                           X(5)
        #                                           
        # X(0),(7)                                  X(6)
        #
        #

        tabPoints[0].x = (pc.HasAGWFlag(FNB_BOTTOM) and [posx] or [posx+self._factor])[0]
        tabPoints[0].y = (pc.HasAGWFlag(FNB_BOTTOM) and [2] or [tabHeight - 3])[0]

        tabPoints[1].x = tabPoints[0].x + tabHeight - VERTICAL_BORDER_PADDING - 3 - self._factor
        tabPoints[1].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - (VERTICAL_BORDER_PADDING+2)] or [(VERTICAL_BORDER_PADDING+2)])[0]

        tabPoints[2].x = tabPoints[1].x + 4
        tabPoints[2].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - VERTICAL_BORDER_PADDING] or [VERTICAL_BORDER_PADDING])[0]

        tabPoints[3].x = tabPoints[2].x + tabWidth - 2
        tabPoints[3].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabHeight - VERTICAL_BORDER_PADDING] or [VERTICAL_BORDER_PADDING])[0]

        tabPoints[4].x = tabPoints[3].x + 1
        tabPoints[4].y = (pc.HasAGWFlag(FNB_BOTTOM) and [tabPoints[3].y - 1] or [tabPoints[3].y + 1])[0]

        tabPoints[5].x = tabPoints[4].x + 1
        tabPoints[5].y = (pc.HasAGWFlag(FNB_BOTTOM) and [(tabPoints[4].y - 1)] or [tabPoints[4].y + 1])[0]

        tabPoints[6].x = tabPoints[2].x + tabWidth
        tabPoints[6].y = tabPoints[0].y

        tabPoints[7].x = tabPoints[0].x
        tabPoints[7].y = tabPoints[0].y

        pc._pagesInfoVec[tabIdx].SetRegion(tabPoints)

        # Draw the polygon
        br = dc.GetBrush()
        dc.SetBrush(wx.Brush((tabIdx == pc.GetSelection() and [pc._activeTabColour] or [pc._colourTo])[0]))
        dc.SetPen(wx.Pen((tabIdx == pc.GetSelection() and [wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)] or [pc._colourBorder])[0]))
        dc.DrawPolygon(tabPoints)

        # Restore the brush
        dc.SetBrush(br)
        rect = pc.GetClientRect()

        if tabIdx != pc.GetSelection() and not pc.HasAGWFlag(FNB_BOTTOM):
        
            # Top default tabs
            dc.SetPen(wx.Pen(pc._pParent.GetBorderColour()))
            lineY = rect.height
            curPen = dc.GetPen()
            curPen.SetWidth(1)
            dc.SetPen(curPen)
            dc.DrawLine(posx, lineY, posx+rect.width, lineY)
        
        # Incase we are drawing the selected tab, we draw the border of it as well
        # but without the bottom (upper line incase of wxBOTTOM)
        if tabIdx == pc.GetSelection():
        
            borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))
            dc.SetPen(borderPen)
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.DrawPolygon(tabPoints)

            # Delete the bottom line (or the upper one, incase we use wxBOTTOM) 
            dc.SetPen(wx.WHITE_PEN)
            dc.DrawLine(tabPoints[0].x, tabPoints[0].y, tabPoints[6].x, tabPoints[6].y)

        self.FillVC8GradientColour(pc, dc, tabPoints, tabIdx == pc.GetSelection(), tabIdx)

        # Draw a thin line to the right of the non-selected tab
        if tabIdx != pc.GetSelection():
        
            dc.SetPen(wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)))
            dc.DrawLine(tabPoints[4].x-1, tabPoints[4].y, tabPoints[5].x-1, tabPoints[5].y)
            dc.DrawLine(tabPoints[5].x-1, tabPoints[5].y, tabPoints[6].x-1, tabPoints[6].y)
        
        # Text drawing offset from the left border of the 
        # rectangle
        
        # The width of the images are 16 pixels
        vc8ShapeLen = tabHeight - VERTICAL_BORDER_PADDING - 2
        if pc.TabHasImage(tabIdx):
            textOffset = 2*pc._pParent.GetPadding() + 16 + vc8ShapeLen 
        else:
            textOffset = pc._pParent.GetPadding() + vc8ShapeLen

        # Draw the image for the tab if any
        imageYCoord = (pc.HasAGWFlag(FNB_BOTTOM) and [6] or [8])[0]

        if pc.TabHasImage(tabIdx):
        
            imageXOffset = textOffset - 16 - pc._pParent.GetPadding()
            pc._ImageList.Draw(pc._pagesInfoVec[tabIdx].GetImageIndex(), dc,
                                     posx + imageXOffset, imageYCoord,
                                     wx.IMAGELIST_DRAW_TRANSPARENT, True)        

        boldFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            
        # if selected tab, draw text in bold
        if tabIdx == pc.GetSelection():
            boldFont.SetWeight(wx.FONTWEIGHT_BOLD)
        
        dc.SetFont(boldFont)

        pageTextColour = pc._pParent.GetPageTextColour(tabIdx)
        if pageTextColour is not None:
            dc.SetTextForeground(pageTextColour)
        
        dc.DrawText(pc.GetPageText(tabIdx), posx + textOffset, imageYCoord)

        # draw 'x' on tab (if enabled)
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():

            textWidth, textHeight = dc.GetTextExtent(pc.GetPageText(tabIdx))
            tabCloseButtonXCoord = posx + textOffset + textWidth + 1

            # take a bitmap from the position of the 'x' button (the x on tab button)
            # this bitmap will be used later to delete old buttons
            tabCloseButtonYCoord = imageYCoord
            x_rect = wx.Rect(tabCloseButtonXCoord, tabCloseButtonYCoord, 16, 16)
            # Draw the tab
            self.DrawTabX(pc, dc, x_rect, tabIdx, btnStatus)

        self.DrawFocusRectangle(dc, pc, pc._pagesInfoVec[tabIdx])


    def FillVC8GradientColour(self, pageContainer, dc, tabPoints, bSelectedTab, tabIdx):
        """
        Fills a tab with a gradient shading.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `tabPoints`: a Python list of `wx.Points` representing the tab outline;
        :param `bSelectedTab`: ``True`` if the tab is selected, ``False`` otherwise;
        :param `tabIdx`: the index of the tab;
        """

        # calculate gradient coefficients
        pc = pageContainer

        if self._first:
            self._first = False
            pc._colourTo   = LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE), 0) 
            pc._colourFrom = LightColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE), 60)
        
        col2 = pc._pParent.GetGradientColourTo()
        col1 = pc._pParent.GetGradientColourFrom()

        # If colourful tabs style is set, override the tab colour
        if pc.HasAGWFlag(FNB_COLOURFUL_TABS):
        
            if not pc._pagesInfoVec[tabIdx].GetColour():
            
                # First time, generate colour, and keep it in the vector
                tabColour = RandomColour()
                pc._pagesInfoVec[tabIdx].SetColour(tabColour)
            
            if pc.HasAGWFlag(FNB_BOTTOM):
            
                col2 = LightColour(pc._pagesInfoVec[tabIdx].GetColour(), 50)
                col1 = LightColour(pc._pagesInfoVec[tabIdx].GetColour(), 80)
            
            else:
            
                col1 = LightColour(pc._pagesInfoVec[tabIdx].GetColour(), 50)
                col2 = LightColour(pc._pagesInfoVec[tabIdx].GetColour(), 80)
            
        size = abs(tabPoints[2].y - tabPoints[0].y) - 1

        rf, gf, bf = 0, 0, 0
        rstep = float(col2.Red() - col1.Red())/float(size)
        gstep = float(col2.Green() - col1.Green())/float(size)
        bstep = float(col2.Blue() - col1.Blue())/float(size)
        
        y = tabPoints[0].y 

        # If we are drawing the selected tab, we need also to draw a line 
        # from 0.tabPoints[0].x and tabPoints[6].x . end, we achieve this
        # by drawing the rectangle with transparent brush
        # the line under the selected tab will be deleted by the drwaing loop
        if bSelectedTab:
            self.DrawTabsLine(pc, dc)

        while 1:
        
            if pc.HasAGWFlag(FNB_BOTTOM):
            
                if y > tabPoints[0].y + size:
                    break
            
            else:
            
                if y < tabPoints[0].y - size:
                    break
            
            currCol = wx.Colour(col1.Red() + rf, col1.Green() + gf, col1.Blue() + bf)

            dc.SetPen((bSelectedTab and [wx.Pen(pc._activeTabColour)] or [wx.Pen(currCol)])[0])
            startX = self.GetStartX(tabPoints, y, pc.GetParent().GetAGWWindowStyleFlag()) 
            endX = self.GetEndX(tabPoints, y, pc.GetParent().GetAGWWindowStyleFlag())
            dc.DrawLine(startX, y, endX, y)

            # Draw the border using the 'edge' point
            dc.SetPen(wx.Pen((bSelectedTab and [wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)] or [pc._colourBorder])[0]))
            
            dc.DrawPoint(startX, y)
            dc.DrawPoint(endX, y)
            
            # Progress the colour 
            rf += rstep
            gf += gstep
            bf += bstep

            if pc.HasAGWFlag(FNB_BOTTOM):
                y = y + 1
            else:
                y = y - 1


    def GetStartX(self, tabPoints, y, style):
        """
        Returns the `x` start position of a tab.

        :param `tabPoints`: a Python list of `wx.Points` representing the tab outline;
        :param `y`: the y start position of the tab;
        :param `style`: can be ``FNB_BOTTOM`` or the default (tabs at top).
        """

        x1, x2, y1, y2 = 0.0, 0.0, 0.0, 0.0

        # We check the 3 points to the left

        bBottomStyle = (style & FNB_BOTTOM and [True] or [False])[0]
        match = False

        if bBottomStyle:
        
            for i in xrange(3):
                
                if y >= tabPoints[i].y and y < tabPoints[i+1].y:
                
                    x1 = tabPoints[i].x
                    x2 = tabPoints[i+1].x
                    y1 = tabPoints[i].y
                    y2 = tabPoints[i+1].y
                    match = True
                    break
                
        else:
        
            for i in xrange(3):
                
                if y <= tabPoints[i].y and y > tabPoints[i+1].y:
                
                    x1 = tabPoints[i].x
                    x2 = tabPoints[i+1].x
                    y1 = tabPoints[i].y
                    y2 = tabPoints[i+1].y
                    match = True
                    break
                
        if not match:
            return tabPoints[2].x

        # According to the equation y = ax + b => x = (y-b)/a
        # We know the first 2 points

        x1, x2, y1, y2 = map(float, (x1, x2, y1, y2))

        if abs(x2 - x1) < 1e-6:
            return x2
        else:
            a = (y2 - y1)/(x2 - x1)

        b = y1 - ((y2 - y1)/(x2 - x1))*x1

        if a == 0:
            return int(x1)

        x = (y - b)/a
        
        return int(x)


    def GetEndX(self, tabPoints, y, style):
        """
        Returns the `x` end position of a tab.

        :param `tabPoints`: a Python list of `wx.Points` representing the tab outline;
        :param `y`: the y end position of the tab;
        :param `style`: can be ``FNB_BOTTOM`` or the default (tabs at top).
        """

        x1, x2, y1, y2 = 0.0, 0.0, 0.0, 0.0

        # We check the 3 points to the left
        bBottomStyle = (style & FNB_BOTTOM and [True] or [False])[0]
        match = False

        if bBottomStyle:

            for i in xrange(7, 3, -1):
                
                if y >= tabPoints[i].y and y < tabPoints[i-1].y:
                
                    x1 = tabPoints[i].x
                    x2 = tabPoints[i-1].x
                    y1 = tabPoints[i].y
                    y2 = tabPoints[i-1].y
                    match = True
                    break
        
        else:
        
            for i in xrange(7, 3, -1):
                
                if y <= tabPoints[i].y and y > tabPoints[i-1].y:
                
                    x1 = tabPoints[i].x
                    x2 = tabPoints[i-1].x
                    y1 = tabPoints[i].y
                    y2 = tabPoints[i-1].y
                    match = True
                    break

        if not match:
            return tabPoints[3].x

        # According to the equation y = ax + b => x = (y-b)/a
        # We know the first 2 points

        # Vertical line
        if x1 == x2:
            return int(x1)
        
        a = (y2 - y1)/(x2 - x1)
        b = y1 - ((y2 - y1)/(x2 - x1))*x1

        if a == 0:
            return int(x1)

        x = (y - b)/a

        return int(x)


    def NumberTabsCanFit(self, pageContainer, fr=-1):
        """
        Calculates the number of tabs that can fit on the available space on screen.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `fr`: the current first visible tab.
        """

        pc = pageContainer
        
        rect = pc.GetClientRect()
        clientWidth = rect.width

        # Empty results
        vTabInfo = []
        tabHeight = self.CalcTabHeight(pageContainer)

        # The drawing starts from posx
        posx = pc._pParent.GetPadding()
        
        if fr < 0:
            fr = pc._nFrom

        for i in xrange(fr, len(pc._pagesInfoVec)):
        
            vc8glitch = tabHeight + FNB_HEIGHT_SPACER
            tabWidth = self.CalcTabWidth(pageContainer, i, tabHeight)

            if posx + tabWidth + vc8glitch + self.GetButtonsAreaLength(pc) >= clientWidth:
                break

            # Add a result to the returned vector
            tabRect = wx.Rect(posx, VERTICAL_BORDER_PADDING, tabWidth, tabHeight)
            vTabInfo.append(tabRect)

            # Advance posx
            posx += tabWidth + FNB_HEIGHT_SPACER
        
        return vTabInfo
    
#------------------------------------------------------------------
# Ribbon Tabs style
#------------------------------------------------------------------
class FNBRendererRibbonTabs(FNBRenderer):    
    """
    This class handles the drawing of tabs using the `Ribbon Tabs` renderer.
    """

    def __init__(self):
        """ Default class constructor. """

        FNBRenderer.__init__(self)
        self._first = True
        self._factor = 1
       
    # definte this because we don't want to use the bold font 
    def CalcTabWidth(self, pageContainer, tabIdx, tabHeight):
        """
        Calculates the width of the input tab.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `tabIdx`: the index of the input tab;
        :param `tabHeight`: the height of the tab.
        """

        pc = pageContainer
        dc = wx.MemoryDC()
        dc.SelectObject(wx.EmptyBitmap(1,1))

        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)

        if pc.IsDefaultTabs():
            shapePoints = int(tabHeight*math.tan(float(pc._pagesInfoVec[tabIdx].GetTabAngle())/180.0*math.pi))

        dc.SetFont(font)
        width, pom = dc.GetTextExtent(pc.GetPageText(tabIdx))

        # Set a minimum size to a tab
        if width < 20:
            width = 20

        tabWidth = 2*pc._pParent.GetPadding() + width

        # Style to add a small 'x' button on the top right
        # of the tab
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
            # The xpm image that contains the 'x' button is 9 pixels
            spacer = 9
            if pc.HasAGWFlag(FNB_VC8):
                spacer = 4

            tabWidth += pc._pParent.GetPadding() + spacer
        
        if pc.IsDefaultTabs():
            # Default style
            tabWidth += 2*shapePoints

        hasImage = pc._ImageList != None and pc._pagesInfoVec[tabIdx].GetImageIndex() != -1

        # For VC71 style, we only add the icon size (16 pixels)
        if hasImage:
        
            if not pc.IsDefaultTabs():
                tabWidth += 16 + pc._pParent.GetPadding()
            else:
                # Default style
                tabWidth += 16 + pc._pParent.GetPadding() + shapePoints/2
        
        return tabWidth

        
    def DrawTab(self, pageContainer, dc, posx, tabIdx, tabWidth, tabHeight, btnStatus):
        """
        Draws a tab using the `Ribbon Tabs` style.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`;
        :param `posx`: the x position of the tab;
        :param `tabIdx`: the index of the tab;
        :param `tabWidth`: the tab's width;
        :param `tabHeight`: the tab's height;
        :param `btnStatus`: the status of the 'X' button inside this tab.
        """

        pc = pageContainer 
        
        gc = wx.GraphicsContext.Create(dc)
        gc.SetPen(dc.GetPen())
        gc.SetBrush(dc.GetBrush())

        spacer = math.ceil(float(FNB_HEIGHT_SPACER)/2/2)
        gc.DrawRoundedRectangle(posx+1,spacer,tabWidth-1,tabHeight-spacer*2,5)
        
        if tabIdx == pc.GetSelection():
            pass
        else:
            if tabIdx != pc.GetSelection() - 1:
                pass

        # -----------------------------------
        # Text and image drawing
        # -----------------------------------

        # Text drawing offset from the left border of the
        # rectangle
        
        # The width of the images are 16 pixels
        padding = pc.GetParent().GetPadding()
        hasImage = pc._pagesInfoVec[tabIdx].GetImageIndex() != -1
        imageYCoord = FNB_HEIGHT_SPACER/2

        if hasImage:
            textOffset = 2*pc._pParent._nPadding + 16
        else:
            textOffset = pc._pParent._nPadding

        textOffset += 2

        if tabIdx != pc.GetSelection():
        
            # Set the text background to be like the vertical lines
            dc.SetTextForeground(pc._pParent.GetNonActiveTabTextColour())
        
        if hasImage:
        
            imageXOffset = textOffset - 16 - padding
            pc._ImageList.Draw(pc._pagesInfoVec[tabIdx].GetImageIndex(), dc,
                                     posx + imageXOffset, imageYCoord,
                                     wx.IMAGELIST_DRAW_TRANSPARENT, True)

        pageTextColour = pc._pParent.GetPageTextColour(tabIdx)
        if pageTextColour is not None:
            dc.SetTextForeground(pageTextColour)
            
        dc.DrawText(pc.GetPageText(tabIdx), posx + textOffset, imageYCoord)

        # draw 'x' on tab (if enabled)
        if pc.HasAGWFlag(FNB_X_ON_TAB) and tabIdx == pc.GetSelection():
        
            textWidth, textHeight = dc.GetTextExtent(pc.GetPageText(tabIdx))
            tabCloseButtonXCoord = posx + textOffset + textWidth + 1

            # take a bitmap from the position of the 'x' button (the x on tab button)
            # this bitmap will be used later to delete old buttons
            tabCloseButtonYCoord = imageYCoord
            x_rect = wx.Rect(tabCloseButtonXCoord, tabCloseButtonYCoord, 16, 16)

            # Draw the tab
            self.DrawTabX(pc, dc, x_rect, tabIdx, btnStatus)            
        

    def DrawTabs(self, pageContainer, dc):
        """
        Actually draws the tabs in L{FlatNotebook}.

        :param `pageContainer`: an instance of L{FlatNotebook};
        :param `dc`: an instance of `wx.DC`.
        """

        pc = pageContainer
        #style = pc.GetParent().GetWindowStyleFlag()
        
        if "__WXMAC__" in wx.PlatformInfo:
            # Works well on MSW & GTK, however this lines should be skipped on MAC
            if not pc._pagesInfoVec or pc._nFrom >= len(pc._pagesInfoVec):
                pc.Hide()
                return
            
        # Get the text height
        tabHeight = self.CalcTabHeight(pageContainer)

        # Calculate the number of rows required for drawing the tabs
        rect = pc.GetClientRect()
        clientWidth = rect.width

        # Set the maximum client size
        pc.SetSizeHints(self.GetButtonsAreaLength(pc), tabHeight)

        size = pc.GetSize()

        # Background
        dc.SetTextBackground(pc.GetBackgroundColour())
        dc.SetTextForeground(pc._activeTextColour)

        borderPen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW))
        backBrush = wx.Brush(pc._tabAreaColour)

        # If border style is set, set the pen to be border pen
        if pc.HasAGWFlag(FNB_TABS_BORDER_SIMPLE):
            dc.SetPen(borderPen)
        else:
            dc.SetPen(wx.Pen(pc._tabAreaColour))
            
        dc.SetBrush(backBrush)
        dc.DrawRectangle(0, 0, size.x, size.y)

        # Draw labels
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        dc.SetFont(font)

        posx = pc._pParent.GetPadding()

        # Update all the tabs from 0 to 'pc._nFrom' to be non visible
        for i in xrange(pc._nFrom):
            pc._pagesInfoVec[i].SetPosition(wx.Point(-1, -1))

        count = pc._nFrom
        
        #----------------------------------------------------------
        # Go over and draw the visible tabs
        #----------------------------------------------------------
        selPen = wx.Pen(adjust_colour(pc._tabAreaColour, -20))
        noselPen = wx.Pen(pc._tabAreaColour)
        noselBrush = wx.Brush(pc._tabAreaColour)
        selBrush = wx.Brush(LightColour(pc._tabAreaColour,60))
        
        for i in xrange(pc._nFrom, len(pc._pagesInfoVec)):

            # This style highlights the selected tab and the tab the mouse is over
            highlight = (i==pc.GetSelection()) or pc.IsMouseHovering(i)
            dc.SetPen((highlight and [selPen] or [noselPen])[0])
            dc.SetBrush((highlight and [selBrush] or [noselBrush])[0])

            # Add the padding to the tab width
            # Tab width:
            # +-----------------------------------------------------------+
            # | PADDING | IMG | IMG_PADDING | TEXT | PADDING | x |PADDING |
            # +-----------------------------------------------------------+
            tabWidth = self.CalcTabWidth(pageContainer, i, tabHeight)

            # Check if we can draw more
            if posx + tabWidth + self.GetButtonsAreaLength(pc) >= clientWidth:
                break

            count = count + 1
            
            # By default we clean the tab region
            #pc._pagesInfoVec[i].GetRegion().Clear()

            # Clean the 'x' buttn on the tab.
            # A 'Clean' rectangle, is a rectangle with width or height
            # with values lower than or equal to 0
            pc._pagesInfoVec[i].GetXRect().SetSize(wx.Size(-1, -1))

            # Draw the tab (border, text, image & 'x' on tab)
            self.DrawTab(pc, dc, posx, i, tabWidth, tabHeight, pc._nTabXButtonStatus)

            # Restore the text forground
            dc.SetTextForeground(pc._activeTextColour)

            # Update the tab position & size
            posy = (pc.HasAGWFlag(FNB_BOTTOM) and [0] or [VERTICAL_BORDER_PADDING])[0]

            pc._pagesInfoVec[i].SetPosition(wx.Point(posx, posy))
            pc._pagesInfoVec[i].SetSize(wx.Size(tabWidth, tabHeight))

            posx += tabWidth
        
        # Update all tabs that can not fit into the screen as non-visible
        for i in xrange(count, len(pc._pagesInfoVec)):
            pc._pagesInfoVec[i].SetPosition(wx.Point(-1, -1))
            pc._pagesInfoVec[i].GetRegion().Clear()
        
        # Draw the left/right/close buttons
        # Left arrow
        self.DrawLeftArrow(pc, dc)
        self.DrawRightArrow(pc, dc)
        self.DrawX(pc, dc)
        self.DrawDropDownArrow(pc, dc)

# ---------------------------------------------------------------------------- #
# Class FlatNotebook
# ---------------------------------------------------------------------------- #

class FlatNotebook(wx.PyPanel):
    """
    The L{FlatNotebook} is a full implementation of the `wx.Notebook`, and designed to be
    a drop-in replacement for `wx.Notebook`. The API functions are similar so one can
    expect the function to behave in the same way. 
    """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="FlatNotebook"):
        """
        Default class constructor.

        :param `parent`: the L{FlatNotebook} parent;
        :param `id`: an identifier for the control: a value of -1 is taken to mean a default;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyPanel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         ================================ =========== ==================================================
         Window Styles                    Hex Value   Description
         ================================ =========== ==================================================
         ``FNB_VC71``                             0x1 Use Visual Studio 2003 (VC7.1) style for tabs.
         ``FNB_FANCY_TABS``                       0x2 Use fancy style - square tabs filled with gradient colouring.
         ``FNB_TABS_BORDER_SIMPLE``               0x4 Draw thin border around the page.
         ``FNB_NO_X_BUTTON``                      0x8 Do not display the 'X' button.
         ``FNB_NO_NAV_BUTTONS``                  0x10 Do not display the right/left arrows.
         ``FNB_MOUSE_MIDDLE_CLOSES_TABS``        0x20 Use the mouse middle button for cloing tabs.
         ``FNB_BOTTOM``                          0x40 Place tabs at bottom - the default is to place them at top.
         ``FNB_NODRAG``                          0x80 Disable dragging of tabs.
         ``FNB_VC8``                            0x100 Use Visual Studio 2005 (VC8) style for tabs.
         ``FNB_X_ON_TAB``                       0x200 Place 'X' close button on the active tab.
         ``FNB_BACKGROUND_GRADIENT``            0x400 Use gradients to paint the tabs background.
         ``FNB_COLOURFUL_TABS``                 0x800 Use colourful tabs (VC8 style only).
         ``FNB_DCLICK_CLOSES_TABS``            0x1000 Style to close tab using double click.
         ``FNB_SMART_TABS``                    0x2000 Use `Smart Tabbing`, like ``Alt`` + ``Tab`` on Windows.
         ``FNB_DROPDOWN_TABS_LIST``            0x4000 Use a dropdown menu on the left in place of the arrows.
         ``FNB_ALLOW_FOREIGN_DND``             0x8000 Allows drag 'n' drop operations between different FlatNotebooks.
         ``FNB_HIDE_ON_SINGLE_TAB``           0x10000 Hides the Page Container when there is one or fewer tabs.
         ``FNB_DEFAULT_STYLE``                0x10020 FlatNotebook default style.
         ``FNB_FF2``                          0x20000 Use Firefox 2 style for tabs.
         ``FNB_NO_TAB_FOCUS``                 0x40000 Does not allow tabs to have focus.
         ``FNB_RIBBON_TABS``                  0x80000 Use the Ribbon Tabs style.
         ================================ =========== ==================================================
        
        :param `name`: the window name. 
        """

        self._bForceSelection = False
        self._nPadding = 6
        self._nFrom = 0
        style |= wx.TAB_TRAVERSAL
        self._pages = None
        self._windows = []
        self._popupWin = None
        self._naviIcon = None
        self._agwStyle = agwStyle

        wx.PyPanel.__init__(self, parent, id, pos, size, style)
        
        self._pages = PageContainer(self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, style)
        
        self.Bind(wx.EVT_NAVIGATION_KEY, self.OnNavigationKey)

        self.Init()


    def Init(self):
        """ Initializes all the class attributes. """
        
        self._pages._colourBorder = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNSHADOW)

        self._mainSizer = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(self._mainSizer)

        # The child panels will inherit this bg colour, so leave it at the default value
        #self.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_APPWORKSPACE))

        # Set default page height
        dc = wx.ClientDC(self)
        
        if "__WXGTK__" in wx.PlatformInfo:
            # For GTK it seems that we must do this steps in order
            # for the tabs will get the proper height on initialization
            # on MSW, preforming these steps yields wierd results
            boldFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            boldFont.SetWeight(wx.FONTWEIGHT_BOLD)
            dc.SetFont(boldFont)
        
        height = dc.GetCharHeight()

        tabHeight = height + FNB_HEIGHT_SPACER         # We use 8 pixels as padding
        
        if "__WXGTK__" in wx.PlatformInfo:
            tabHeight += 6
            
        self._pages.SetSizeHints(-1, tabHeight)
        # Add the tab container to the sizer
        self._mainSizer.Insert(0, self._pages, 0, wx.EXPAND)
        self._mainSizer.Layout()

        self._pages._nFrom = self._nFrom
        self._pDropTarget = FNBDropTarget(self)
        self.SetDropTarget(self._pDropTarget)


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same
        size as it would have after a call to `Fit()`.
        
        :note: Overridden from `wx.PyPanel`.
        """

        if not self._windows:
            # Something is better than nothing... no pages!
            return wx.Size(20, 20)

        maxWidth = maxHeight = 0
        tabHeight = self.GetPageBestSize().height

        for win in self._windows:
            # Loop over all the windows to get their best size
            width, height = win.GetBestSize()
            maxWidth, maxHeight = max(maxWidth, width), max(maxHeight, height)

        return wx.Size(maxWidth, maxHeight+tabHeight)
    

    def SetActiveTabTextColour(self, textColour):
        """
        Sets the text colour for the active tab.

        :param `textColour`: a valid `wx.Colour` object.
        """

        self._pages._activeTextColour = textColour


    def OnDropTarget(self, x, y, nTabPage, wnd_oldContainer):
        """
        Handles the drop action from a drag and drop operation.

        :param `x`: the x position of the drop action;
        :param `y`: the y position of the drop action;
        :param `nTabPage`: the index of the tab being dropped;
        :param `wnd_oldContainer`: the L{FlatNotebook} to which the dropped tab previously
         belonged to.
        """

        return self._pages.OnDropTarget(x, y, nTabPage, wnd_oldContainer)


    def GetPreviousSelection(self):
        """ Returns the previous selection. """

        return self._pages._iPreviousActivePage


    def AddPage(self, page, text, select=False, imageId=-1):
        """
        Adds a page to the L{FlatNotebook}.

        :param `page`: specifies the new page;
        :param `text`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `imageId`: specifies the optional image index for the new page.
        
        :return: ``True`` if successful, ``False`` otherwise.
        """

        # sanity check
        if not page:
            return False

        # reparent the window to us
        page.Reparent(self)

        # Add tab
        bSelected = select or len(self._windows) == 0

        if bSelected:
            
            bSelected = False
            
            # Check for selection and send events
            oldSelection = self._pages._iActivePage
            tabIdx = len(self._windows)
            
            event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CHANGING, self.GetId())
            event.SetSelection(tabIdx)
            event.SetOldSelection(oldSelection)
            event.SetEventObject(self)
            
            if not self.GetEventHandler().ProcessEvent(event) or event.IsAllowed() or len(self._windows) == 0:
                bSelected = True            
        
        curSel = self._pages.GetSelection()

        if not self._pages.IsShown():
            self._pages.Show()

        self._pages.AddPage(text, bSelected, imageId)
        self._windows.append(page)

        self.Freeze()

        # Check if a new selection was made
        if bSelected:
        
            if curSel >= 0:
            
                # Remove the window from the main sizer
                self._mainSizer.Detach(self._windows[curSel])
                self._windows[curSel].Hide()
            
            if self.GetAGWWindowStyleFlag() & FNB_BOTTOM:
            
                self._mainSizer.Insert(0, page, 1, wx.EXPAND)
            
            else:
            
                # We leave a space of 1 pixel around the window
                self._mainSizer.Add(page, 1, wx.EXPAND)

            # Fire a wxEVT_FLATNOTEBOOK_PAGE_CHANGED event
            event.SetEventType(wxEVT_FLATNOTEBOOK_PAGE_CHANGED)
            event.SetOldSelection(oldSelection)
            self.GetEventHandler().ProcessEvent(event)
            
        else:

            # Hide the page
            page.Hide()

        self.Thaw()        
        self._mainSizer.Layout()
        self.Refresh()

        return True        


    def SetImageList(self, imageList):
        """
        Sets the image list for the page control.

        :param `imageList`: an instance of `wx.ImageList`.
        """

        self._pages.SetImageList(imageList)


    def AssignImageList(self, imageList):
        """
        Assigns the image list for the page control.

        :param `imageList`: an instance of `wx.ImageList`.
        """

        self._pages.AssignImageList(imageList)


    def GetImageList(self):
        """ Returns the associated image list. """
        
        return self._pages.GetImageList()


    def InsertPage(self, indx, page, text, select=True, imageId=-1):
        """
        Inserts a new page at the specified position.

        :param `indx`: specifies the position of the new page;
        :param `page`: specifies the new page;
        :param `text`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `imageId`: specifies the optional image index for the new page.
        
        :return: ``True`` if successful, ``False`` otherwise.
        """     

        # sanity check
        if not page:
            return False

        # reparent the window to us
        page.Reparent(self)

        if not self._windows:
        
            self.AddPage(page, text, select, imageId)
            return True

        # Insert tab
        bSelected = select or not self._windows
        curSel = self._pages.GetSelection()
        
        indx = max(0, min(indx, len(self._windows)))

        if indx <= len(self._windows):
        
            self._windows.insert(indx, page)
        
        else:
        
            self._windows.append(page)

        if bSelected:
        
            bSelected = False
            
            # Check for selection and send events
            oldSelection = self._pages._iActivePage
            
            event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CHANGING, self.GetId())
            event.SetSelection(indx)
            event.SetOldSelection(oldSelection)
            event.SetEventObject(self)
            
            if not self.GetEventHandler().ProcessEvent(event) or event.IsAllowed() or len(self._windows) == 0:
                bSelected = True            
        
        self._pages.InsertPage(indx, text, bSelected, imageId)
        
        if indx <= curSel:
            curSel = curSel + 1

        self.Freeze()

        # Check if a new selection was made
        if bSelected:
        
            if curSel >= 0:
            
                # Remove the window from the main sizer
                self._mainSizer.Detach(self._windows[curSel])
                self._windows[curSel].Hide()
            
            self._pages.SetSelection(indx)

            # Fire a wxEVT_FLATNOTEBOOK_PAGE_CHANGED event
            event.SetEventType(wxEVT_FLATNOTEBOOK_PAGE_CHANGED)
            event.SetOldSelection(oldSelection)
            self.GetEventHandler().ProcessEvent(event)
        
        else:
        
            # Hide the page
            page.Hide()

        self.Thaw()
        self._mainSizer.Layout()        
        self.Refresh()

        return True


    def SetSelection(self, page):
        """
        Sets the selection for the given page.

        :param `page`: an integer specifying the new selected page.
        
        :note: The call to this function **does not** generate the page changing events.
        """

        if page >= len(self._windows) or not self._windows:
            return

        # Support for disabed tabs
        if not self._pages.GetEnabled(page) and len(self._windows) > 1 and not self._bForceSelection:
            return

        curSel = self._pages.GetSelection()

        # program allows the page change
        self.Freeze()
        if curSel >= 0:
        
            # Remove the window from the main sizer
            self._mainSizer.Detach(self._windows[curSel])
            self._windows[curSel].Hide()
        
        if self.GetAGWWindowStyleFlag() & FNB_BOTTOM:
        
            self._mainSizer.Insert(0, self._windows[page], 1, wx.EXPAND)
        
        else:
        
            # We leave a space of 1 pixel around the window
            self._mainSizer.Add(self._windows[page], 1, wx.EXPAND)
        
        self._windows[page].Show()
        self.Thaw()
        
        self._mainSizer.Layout()
        
        if page != self._pages._iActivePage:
            # there is a real page changing
            self._pages._iPreviousActivePage = self._pages._iActivePage

        self._pages._iActivePage = page
        self._pages.DoSetSelection(page)


    def DeletePage(self, page):
        """
        Deletes the specified page, and the associated window.

        :param `page`: an integer specifying the new selected page.
        
        :note: The call to this function generates the page changing events.
        """

        if page >= len(self._windows) or page < 0:
            return

        # Fire a closing event
        event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CLOSING, self.GetId())
        event.SetSelection(page)
        event.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(event)

        # The event handler allows it?
        if not event.IsAllowed():
            return

        self.Freeze()

        # Delete the requested page
        pageRemoved = self._windows[page]

        # If the page is the current window, remove it from the sizer
        # as well
        if page == self._pages.GetSelection():
            self._mainSizer.Detach(pageRemoved)
        
        # Remove it from the array as well
        self._windows.pop(page)

        # Now we can destroy it in wxWidgets use Destroy instead of delete
        pageRemoved.Destroy()

        self.Thaw()

        self._pages.DoDeletePage(page)
        self.Refresh()
        self.Update()  

        # Fire a closed event
        closedEvent = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CLOSED, self.GetId())
        closedEvent.SetSelection(page)
        closedEvent.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(closedEvent)


    def DeleteAllPages(self):
        """ Deletes all the pages in the L{FlatNotebook}. """

        if not self._windows:
            return False

        self.Freeze()
        
        for page in self._windows:
            page.Destroy()
        
        self._windows = []
        self.Thaw()

        # Clear the container of the tabs as well
        self._pages.DeleteAllPages()
        return True


    def GetCurrentPage(self):
        """ Returns the currently selected notebook page or ``None`` if none is selected. """
        
        sel = self._pages.GetSelection()
        if sel < 0 or sel >= len(self._windows):
            return None

        return self._windows[sel]


    def GetPage(self, page):
        """ Returns the window at the given page position, or ``None``. """

        if page >= len(self._windows):
            return None

        return self._windows[page]


    def GetPageIndex(self, win):
        """
        Returns the index at which the window is found.

        :param `win`: an instance of `wx.Window`.
        """

        try:
            return self._windows.index(win)
        except:
            return -1


    def GetSelection(self):
        """ Returns the currently selected page, or -1 if none was selected. """
        
        return self._pages.GetSelection()


    def AdvanceSelection(self, forward=True):
        """
        Cycles through the tabs.

        :param `forward`: if ``True``, the selection is advanced in ascending order
         (to the right), otherwise the selection is advanced in descending order.
         
        :note: The call to this function generates the page changing events.
        """

        self._pages.AdvanceSelection(forward)


    def GetPageCount(self):
        """ Returns the number of pages in the L{FlatNotebook} control. """

        return self._pages.GetPageCount()


    def SetNavigatorIcon(self, bmp):
        """
        Set the icon used by the L{TabNavigatorWindow}.

        :param `bmp`: a valid `wx.Bitmap` object.
        """
        
        if isinstance(bmp, wx.Bitmap) and bmp.IsOk():
            # Make sure image is proper size
            if bmp.GetSize() != (16, 16):
                img = bmp.ConvertToImage()
                img.Rescale(16, 16, wx.IMAGE_QUALITY_HIGH)
                bmp = wx.BitmapFromImage(img)
            self._naviIcon = bmp
        else:
            raise TypeError("SetNavigatorIcon requires a valid bitmap")


    def OnNavigationKey(self, event):
        """
        Handles the ``wx.EVT_NAVIGATION_KEY`` event for L{FlatNotebook}.

        :param `event`: a `wx.NavigationKeyEvent` event to be processed.
        """

        if event.IsWindowChange():
            if len(self._windows) == 0:
                return
            # change pages
            if self.HasAGWFlag(FNB_SMART_TABS):
                if not self._popupWin:
                    self._popupWin = TabNavigatorWindow(self, self._naviIcon)
                    self._popupWin.SetReturnCode(wx.ID_OK)
                    self._popupWin.ShowModal()
                    self._popupWin.Destroy()
                    self._popupWin = None
                else:
                    # a dialog is already opened
                    self._popupWin.OnNavigationKey(event)
                    return
            else:
                # change pages
                self.AdvanceSelection(event.GetDirection())

        else:
            event.Skip()
            

    def GetPageShapeAngle(self, page_index):
        """
        Returns the angle associated to a tab.

        :param `page_index`: the index of the tab for which we wish to get the shape angle.
        """

        if page_index < 0 or page_index >= len(self._pages._pagesInfoVec):
            return None, False
        
        result = self._pages._pagesInfoVec[page_index].GetTabAngle()
        return result, True


    def SetPageShapeAngle(self, page_index, angle):
        """
        Sets the angle associated to a tab.

        :param `page_index`: the index of the tab for which we wish to get the shape angle;
        :param `angle`: the new shape angle for the tab (must be less than 15 degrees).
        """

        if page_index < 0 or page_index >= len(self._pages._pagesInfoVec):
            return

        if angle > 15:
            return

        self._pages._pagesInfoVec[page_index].SetTabAngle(angle)


    def SetAllPagesShapeAngle(self, angle):
        """
        Sets the angle associated to all the tab.

        :param `angle`: the new shape angle for the tab (must be less than 15 degrees).
        """

        if angle > 15:
            return

        for ii in xrange(len(self._pages._pagesInfoVec)):
            self._pages._pagesInfoVec[ii].SetTabAngle(angle)
        
        self.Refresh()


    def GetPageBestSize(self):
        """ Return the page best size. """

        return self._pages.GetClientSize()


    def SetPageText(self, page, text):
        """
        Sets the text for the given page.

        :param `page`: an integer specifying the page index;
        :param `text`: the new tab label.
        """

        bVal = self._pages.SetPageText(page, text)
        self._pages.Refresh()

        return bVal


    def SetPadding(self, padding):
        """
        Sets the amount of space around each page's icon and label, in pixels.

        :param `padding`: the amount of space around each page's icon and label,
         in pixels.
         
        :note: Only the horizontal padding is considered.
        """

        self._nPadding = padding.GetWidth()


    def GetTabArea(self):
        """ Returns the associated page. """

        return self._pages


    def GetPadding(self):
        """ Returns the amount of space around each page's icon and label, in pixels. """
        
        return self._nPadding 


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the L{FlatNotebook} window style flags.

        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         ================================ =========== ==================================================
         Window Styles                    Hex Value   Description
         ================================ =========== ==================================================
         ``FNB_VC71``                             0x1 Use Visual Studio 2003 (VC7.1) style for tabs.
         ``FNB_FANCY_TABS``                       0x2 Use fancy style - square tabs filled with gradient colouring.
         ``FNB_TABS_BORDER_SIMPLE``               0x4 Draw thin border around the page.
         ``FNB_NO_X_BUTTON``                      0x8 Do not display the 'X' button.
         ``FNB_NO_NAV_BUTTONS``                  0x10 Do not display the right/left arrows.
         ``FNB_MOUSE_MIDDLE_CLOSES_TABS``        0x20 Use the mouse middle button for cloing tabs.
         ``FNB_BOTTOM``                          0x40 Place tabs at bottom - the default is to place them at top.
         ``FNB_NODRAG``                          0x80 Disable dragging of tabs.
         ``FNB_VC8``                            0x100 Use Visual Studio 2005 (VC8) style for tabs.
         ``FNB_X_ON_TAB``                       0x200 Place 'X' close button on the active tab.
         ``FNB_BACKGROUND_GRADIENT``            0x400 Use gradients to paint the tabs background.
         ``FNB_COLOURFUL_TABS``                 0x800 Use colourful tabs (VC8 style only).
         ``FNB_DCLICK_CLOSES_TABS``            0x1000 Style to close tab using double click.
         ``FNB_SMART_TABS``                    0x2000 Use `Smart Tabbing`, like ``Alt`` + ``Tab`` on Windows.
         ``FNB_DROPDOWN_TABS_LIST``            0x4000 Use a dropdown menu on the left in place of the arrows.
         ``FNB_ALLOW_FOREIGN_DND``             0x8000 Allows drag 'n' drop operations between different FlatNotebooks.
         ``FNB_HIDE_ON_SINGLE_TAB``           0x10000 Hides the Page Container when there is one or fewer tabs.
         ``FNB_DEFAULT_STYLE``                0x10020 FlatNotebook default style.
         ``FNB_FF2``                          0x20000 Use Firefox 2 style for tabs.
         ``FNB_NO_TAB_FOCUS``                 0x40000 Does not allow tabs to have focus.
         ``FNB_RIBBON_TABS``                  0x80000 Use the Ribbon Tabs style.
         ================================ =========== ==================================================

        """

        oldStyle = self._agwStyle
        self._agwStyle = agwStyle            
        renderer = self._pages._mgr.GetRenderer(agwStyle)
        renderer._tabHeight = None

        if self._pages:
            # For changing the tab position (i.e. placing them top/bottom)
            # refreshing the tab container is not enough
            self.SetSelection(self._pages._iActivePage)

        # If we just hid the tabs we must Refresh()
        if (not (oldStyle & FNB_HIDE_TABS) and agwStyle & FNB_HIDE_TABS) or \
           (not (oldStyle & FNB_HIDE_ON_SINGLE_TAB) and agwStyle & FNB_HIDE_ON_SINGLE_TAB):
            self.Refresh()
            
        if (oldStyle & FNB_HIDE_TABS and not (agwStyle & FNB_HIDE_TABS)) or \
           (oldStyle & FNB_HIDE_ON_SINGLE_TAB and not self.HasAGWFlag(FNB_HIDE_ON_SINGLE_TAB)):
            #For Redrawing the Tabs once you remove the Hide tyle
            self._pages._ReShow()


    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{FlatNotebook} window style.

        :see: L{SetAGWWindowStyleFlag} for a list of valid window styles.        
        """

        return self._agwStyle


    def HasAGWFlag(self, flag):
        """
        Returns whether a flag is present in the L{FlatNotebook} style.

        :param `flag`: one of the possible L{FlatNotebook} window styles.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.
        """

        agwStyle = self.GetAGWWindowStyleFlag()
        res = (agwStyle & flag and [True] or [False])[0]
        return res


    def HideTabs(self):
        """ Hides the tabs. """
        
        agwStyle = self.GetAGWWindowStyleFlag()
        agwStyle |= FNB_HIDE_TABS
        self.SetAGWWindowStyleFlag( agwStyle )

        
    def ShowTabs(self):
        """ Shows the tabs if hidden previously. """
        
        agwStyle = self.GetAGWWindowStyleFlag()
        agwStyle &= ~FNB_HIDE_TABS
        self.SetAGWWindowStyleFlag( agwStyle )


    def RemovePage(self, page):
        """
        Deletes the specified page, without deleting the associated window.

        :param `page`: an integer specifying the page index.
        """

        if page >= len(self._windows):
            return False

        # Fire a closing event
        event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CLOSING, self.GetId())
        event.SetSelection(page)
        event.SetEventObject(self)
        self.GetEventHandler().ProcessEvent(event)

        # The event handler allows it?
        if not event.IsAllowed():
            return False

        self.Freeze()

        # Remove the requested page
        pageRemoved = self._windows[page]

        # If the page is the current window, remove it from the sizer
        # as well
        if page == self._pages.GetSelection():
            self._mainSizer.Detach(pageRemoved)
        
        # Remove it from the array as well
        self._windows.pop(page)
        self.Thaw()

        self._pages.DoDeletePage(page)

        return True


    def SetRightClickMenu(self, menu):
        """
        Sets the popup menu associated to a right click on a tab.

        :param `menu`: an instance of `wx.Menu`.
        """

        self._pages._pRightClickMenu = menu


    def GetPageText(self, page):
        """
        Returns the string for the given page.

        :param `page`: an integer specifying the page index.
        """

        return self._pages.GetPageText(page)


    def SetGradientColours(self, fr, to, border):
        """
        Sets the gradient colours for the tab.

        :param `fr`: the first gradient colour, an instance of `wx.Colour`;
        :param `to`: the second gradient colour, an instance of `wx.Colour`;
        :param `border`: the border colour, an instance of `wx.Colour`.
        """

        self._pages._colourFrom = fr
        self._pages._colourTo   = to
        self._pages._colourBorder = border


    def SetGradientColourFrom(self, fr):
        """
        Sets the starting colour for the gradient.

        :param `fr`: the first gradient colour, an instance of `wx.Colour`.
        """

        self._pages._colourFrom = fr


    def SetGradientColourTo(self, to):
        """
        Sets the ending colour for the gradient.

        :param `to`: the second gradient colour, an instance of `wx.Colour`;
        """

        self._pages._colourTo = to


    def SetGradientColourBorder(self, border):
        """
        Sets the tab border colour.

        :param `border`: the border colour, an instance of `wx.Colour`.
        """

        self._pages._colourBorder = border


    def GetGradientColourFrom(self):
        """ Gets first gradient colour. """

        return self._pages._colourFrom


    def GetGradientColourTo(self):
        """ Gets second gradient colour. """

        return self._pages._colourTo


    def GetGradientColourBorder(self):
        """ Gets the tab border colour. """

        return self._pages._colourBorder


    def GetBorderColour(self):
        """ Returns the border colour. """

        return self._pages._colourBorder
    

    def GetActiveTabTextColour(self):
        """ Get the active tab text colour. """

        return self._pages._activeTextColour


    def SetPageImage(self, page, image):
        """
        Sets the image index for the given page.

        :param `page`: an integer specifying the page index;
        :param `image`: an index into the image list which was set with L{SetImageList}.
        """

        self._pages.SetPageImage(page, image)


    def GetPageImage(self, page):
        """
        Returns the image index for the given page.

        :param `page`: an integer specifying the page index.
        """

        return self._pages.GetPageImage(page)


    def GetEnabled(self, page):
        """
        Returns whether a tab is enabled or not.

        :param `page`: an integer specifying the page index.
        """

        return self._pages.GetEnabled(page)


    def EnableTab(self, page, enabled=True):
        """
        Enables or disables a tab.

        :param `page`: an integer specifying the page index;
        :param `enabled`: ``True`` to enable a tab, ``False`` to disable it.
        """

        if page >= len(self._windows):
            return

        self._windows[page].Enable(enabled)
        self._pages.EnableTab(page, enabled)


    def GetNonActiveTabTextColour(self):
        """ Returns the non active tabs text colour. """

        return self._pages._nonActiveTextColour


    def SetNonActiveTabTextColour(self, colour):
        """
        Sets the non active tabs text colour.

        :param `colour`: a valid instance of `wx.Colour`.
        """

        self._pages._nonActiveTextColour = colour


    def GetPageTextColour(self, page):
        """
        Returns the tab text colour if it has been set previously, or ``None`` otherwise.

        :param `page`: an integer specifying the page index.
        """

        return self._pages.GetPageTextColour(page)
        

    def SetPageTextColour(self, page, colour):
        """
        Sets the tab text colour individually.

        :param `page`: an integer specifying the page index;
        :param `colour`: an instance of `wx.Colour`. You can pass ``None`` or
         `wx.NullColour` to return to the default page text colour.
        """

        self._pages.SetPageTextColour(page, colour)
        

    def SetTabAreaColour(self, colour):
        """
        Sets the area behind the tabs colour.

        :param `colour`: a valid instance of `wx.Colour`.
        """

        self._pages._tabAreaColour = colour


    def GetTabAreaColour(self):
        """ Returns the area behind the tabs colour. """

        return self._pages._tabAreaColour


    def SetActiveTabColour(self, colour):
        """
        Sets the active tab colour.

        :param `colour`: a valid instance of `wx.Colour`.
        """

        self._pages._activeTabColour = colour


    def GetActiveTabColour(self):
        """ Returns the active tab colour. """

        return self._pages._activeTabColour


    def EnsureVisible(self, page):
       """
       Ensures that a tab is visible.

       :param `page`: an integer specifying the page index.
       """

       self._pages.DoSetSelection(page)

       
# ---------------------------------------------------------------------------- #
# Class PageContainer
# Acts as a container for the pages you add to FlatNotebook
# ---------------------------------------------------------------------------- #

class PageContainer(wx.Panel):
    """
    This class acts as a container for the pages you add to L{FlatNotebook}.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0):
        """
        Default class constructor.

        Used internally, do not call it in your code!        

        :param `parent`: the L{PageContainer} parent;
        :param `id`: an identifier for the control: a value of -1 is taken to mean a default;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style.
        """
        
        self._ImageList = None
        self._iActivePage = -1
        self._pDropTarget = None
        self._nLeftClickZone = FNB_NOWHERE
        self._iPreviousActivePage = -1

        self._pRightClickMenu = None
        self._nXButtonStatus = FNB_BTN_NONE
        self._nArrowDownButtonStatus = FNB_BTN_NONE
        self._pParent = parent
        self._nRightButtonStatus = FNB_BTN_NONE
        self._nLeftButtonStatus = FNB_BTN_NONE
        self._nTabXButtonStatus = FNB_BTN_NONE
        
        self._nHoveringOverTabIndex = -1
        self._nHoveringOverLastTabIndex = -1

        self._setCursor = False        

        self._pagesInfoVec = []        

        self._colourTo = wx.SystemSettings_GetColour(wx.SYS_COLOUR_ACTIVECAPTION)
        self._colourFrom = wx.WHITE
        self._activeTabColour = wx.WHITE
        self._activeTextColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNTEXT)
        self._nonActiveTextColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNTEXT)
        self._tabAreaColour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_BTNFACE)

        self._nFrom = 0
        self._isdragging = False

        # Set default page height, this is done according to the system font
        memDc = wx.MemoryDC()
        memDc.SelectObject(wx.EmptyBitmap(1,1))
    
        if "__WXGTK__" in wx.PlatformInfo:
            boldFont = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            boldFont.SetWeight(wx.BOLD)
            memDc.SetFont(boldFont)

        height = memDc.GetCharHeight()
        tabHeight = height + FNB_HEIGHT_SPACER # We use 10 pixels as padding

        wx.Panel.__init__(self, parent, id, pos, wx.Size(size.x, tabHeight),
                          style|wx.NO_BORDER|wx.NO_FULL_REPAINT_ON_RESIZE|wx.WANTS_CHARS)

        self._pDropTarget = FNBDropTarget(self)
        self.SetDropTarget(self._pDropTarget)
        self._mgr = FNBRendererMgr()

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        self.Bind(wx.EVT_MIDDLE_DOWN, self.OnMiddleDown)
        self.Bind(wx.EVT_MOTION, self.OnMouseMove)
        self.Bind(wx.EVT_MOUSEWHEEL, self.OnMouseWheel)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnMouseLeave)
        self.Bind(wx.EVT_ENTER_WINDOW, self.OnMouseEnterWindow)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDClick)
        self.Bind(wx.EVT_SET_FOCUS, self.OnSetFocus)
        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        

    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{PageContainer}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass

    
    def _ReShow(self):
        """
        Handles the redraw of the tabs when the ``FNB_HIDE_ON_SINGLE_TAB`` has been removed.
        """
        
        self.Show()
        self.GetParent()._mainSizer.Layout()
        self.Refresh()


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{PageContainer}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        dc = wx.BufferedPaintDC(self)
        renderer = self._mgr.GetRenderer(self.GetParent().GetAGWWindowStyleFlag())
        renderer.DrawTabs(self, dc)

        if self.HasAGWFlag(FNB_HIDE_ON_SINGLE_TAB) and len(self._pagesInfoVec) <= 1 or\
           self.HasAGWFlag(FNB_HIDE_TABS):
            self.Hide()
            self.GetParent()._mainSizer.Layout()
            self.Refresh()


    def AddPage(self, caption, selected=False, imgindex=-1):
        """
        Adds a page to the L{PageContainer}.

        :param `page`: specifies the new page;
        :param `text`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `imageId`: specifies the optional image index for the new page.
        """

        if selected:

            self._iPreviousActivePage = self._iActivePage        
            self._iActivePage = len(self._pagesInfoVec)
        
        # Create page info and add it to the vector
        pageInfo = PageInfo(caption, imgindex)
        self._pagesInfoVec.append(pageInfo)
        self.Refresh()


    def InsertPage(self, indx, text, selected=True, imgindex=-1):
        """
        Inserts a new page at the specified position.

        :param `indx`: specifies the position of the new page;
        :param `page`: specifies the new page;
        :param `text`: specifies the text for the new page;
        :param `select`: specifies whether the page should be selected;
        :param `imageId`: specifies the optional image index for the new page.        
        """     

        if selected:

            self._iPreviousActivePage = self._iActivePage        
            self._iActivePage = len(self._pagesInfoVec)
        
        self._pagesInfoVec.insert(indx, PageInfo(text, imgindex))
        
        self.Refresh()
        return True


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{PageContainer}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        # When resizing the control, try to fit to screen as many tabs as we can 
        agwStyle = self.GetParent().GetAGWWindowStyleFlag() 
        renderer = self._mgr.GetRenderer(agwStyle)
        
        fr = 0
        page = self.GetSelection()
        
        for fr in xrange(self._nFrom):
            vTabInfo = renderer.NumberTabsCanFit(self, fr)
            if page - fr >= len(vTabInfo):
                continue
            break

        self._nFrom = fr

        self.Refresh() # Call on paint
        event.Skip()


    def OnMiddleDown(self, event):
        """
        Handles the ``wx.EVT_MIDDLE_DOWN`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        # Test if this style is enabled
        agwStyle = self.GetParent().GetAGWWindowStyleFlag()
        
        if not agwStyle & FNB_MOUSE_MIDDLE_CLOSES_TABS:
            return

        where, tabIdx = self.HitTest(event.GetPosition())
        
        if where == FNB_TAB:
            self.DeletePage(tabIdx)
        
        event.Skip()


    def OnMouseWheel(self, event):
        """
        Handles the ``wx.EVT_MOUSEWHEEL`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        rotation = event.GetWheelRotation()
        delta = event.GetWheelDelta()
        steps = rotation/delta

        for tab in xrange(abs(steps)):
            if steps > 0:
                before = self._nLeftButtonStatus
                self._nLeftButtonStatus = FNB_BTN_PRESSED
                self.RotateLeft()
                self._nLeftButtonStatus = before
            else:
                before = self._nRightButtonStatus
                self._nRightButtonStatus = FNB_BTN_PRESSED
                self.RotateRight()
                self._nRightButtonStatus = before

        event.Skip()


    def OnRightDown(self, event):
        """
        Handles the ``wx.EVT_RIGHT_DOWN`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """
        
        where, tabIdx = self.HitTest(event.GetPosition())

        if where in [FNB_TAB, FNB_TAB_X]:

            if self._pagesInfoVec[tabIdx].GetEnabled():
                # This shouldn't really change the selection, so it's commented out
                # Fire events and eventually (if allowed) change selection
                # self.FireEvent(tabIdx)

                # send a message to popup a custom menu
                event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CONTEXT_MENU, self.GetParent().GetId())
                event.SetSelection(tabIdx)
                event.SetOldSelection(self._iActivePage)
                event.SetEventObject(self.GetParent())
                self.GetParent().GetEventHandler().ProcessEvent(event)

                if self._pRightClickMenu:
                    self.PopupMenu(self._pRightClickMenu)
            
        event.Skip()


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        # Reset buttons status
        self._nXButtonStatus     = FNB_BTN_NONE
        self._nLeftButtonStatus  = FNB_BTN_NONE
        self._nRightButtonStatus = FNB_BTN_NONE
        self._nTabXButtonStatus  = FNB_BTN_NONE
        self._nArrowDownButtonStatus = FNB_BTN_NONE

        self._nLeftClickZone, tabIdx = self.HitTest(event.GetPosition())

        if self._nLeftClickZone == FNB_DROP_DOWN_ARROW:
            self._nArrowDownButtonStatus = FNB_BTN_PRESSED
            self.Refresh()
        elif self._nLeftClickZone == FNB_LEFT_ARROW:
            self._nLeftButtonStatus = FNB_BTN_PRESSED
            self.Refresh()
        elif self._nLeftClickZone == FNB_RIGHT_ARROW:
            self._nRightButtonStatus = FNB_BTN_PRESSED
            self.Refresh()
        elif self._nLeftClickZone == FNB_X:
            self._nXButtonStatus = FNB_BTN_PRESSED
            self.Refresh()
        elif self._nLeftClickZone == FNB_TAB_X:
            self._nTabXButtonStatus = FNB_BTN_PRESSED
            self.Refresh()

        elif self._nLeftClickZone == FNB_TAB:
            
            if self._iActivePage != tabIdx:
                
                # In case the tab is disabled, we dont allow to choose it
                if len(self._pagesInfoVec) > tabIdx and \
                   self._pagesInfoVec[tabIdx].GetEnabled():
                    self.FireEvent(tabIdx)


    def RotateLeft(self):
        """ Scrolls tabs to the left by bulk of 5 tabs. """

        if self._nFrom == 0:
            return

        # Make sure that the button was pressed before
        if self._nLeftButtonStatus != FNB_BTN_PRESSED:
            return

        self._nLeftButtonStatus = FNB_BTN_HOVER

        # We scroll left with bulks of 5
        scrollLeft = self.GetNumTabsCanScrollLeft()

        self._nFrom -= scrollLeft
        if self._nFrom < 0:
            self._nFrom = 0

        self.Refresh()


    def RotateRight(self):
        """ Scrolls tabs to the right by bulk of 5 tabs. """

        if self._nFrom >= len(self._pagesInfoVec) - 1:
            return

        # Make sure that the button was pressed before
        if self._nRightButtonStatus != FNB_BTN_PRESSED:
            return

        self._nRightButtonStatus = FNB_BTN_HOVER

        # Check if the right most tab is visible, if it is
        # don't rotate right anymore
        if self._pagesInfoVec[len(self._pagesInfoVec)-1].GetPosition() != wx.Point(-1, -1):
            return

        self._nFrom += 1
        self.Refresh()


    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        # forget the zone that was initially clicked
        self._nLeftClickZone = FNB_NOWHERE

        where, tabIdx = self.HitTest(event.GetPosition())

        if not self.HasAGWFlag(FNB_NO_TAB_FOCUS):
            # Make sure selected tab has focus
            self.SetFocus()

        if where == FNB_LEFT_ARROW:
            self.RotateLeft()
            
        elif where == FNB_RIGHT_ARROW:
            self.RotateRight()
            
        elif where == FNB_X:
            
            # Make sure that the button was pressed before
            if self._nXButtonStatus != FNB_BTN_PRESSED:
                return

            self._nXButtonStatus = FNB_BTN_HOVER

            self.DeletePage(self._iActivePage)
            
        elif where == FNB_TAB_X:
            
            # Make sure that the button was pressed before
            if self._nTabXButtonStatus != FNB_BTN_PRESSED:
                return 

            self._nTabXButtonStatus = FNB_BTN_HOVER

            self.DeletePage(self._iActivePage)

        elif where == FNB_DROP_DOWN_ARROW:

            # Make sure that the button was pressed before
            if self._nArrowDownButtonStatus != FNB_BTN_PRESSED:
                return

            self._nArrowDownButtonStatus = FNB_BTN_NONE

            # Refresh the button status
            renderer = self._mgr.GetRenderer(self.GetParent().GetAGWWindowStyleFlag())
            dc = wx.ClientDC(self)
            renderer.DrawDropDownArrow(self, dc)

            self.PopupTabsMenu()

        event.Skip()
        

    def HitTest(self, pt):
        """
        HitTest method for L{PageContainer}.

        :param `pt`: an instance of `wx.Point`, to test for hits.
        
        :return: The hit test flag (if any) and the hit page index (if any). The return
         value can be one of the following bits:

         ========================= ======= =================================
         HitTest Flag               Value  Description
         ========================= ======= =================================
         ``FNB_NOWHERE``                 0 Indicates mouse coordinates not on any tab of the notebook
         ``FNB_TAB``                     1 Indicates mouse coordinates inside a tab
         ``FNB_X``                       2 Indicates mouse coordinates inside the 'X' button region
         ``FNB_TAB_X``                   3 Indicates mouse coordinates inside the 'X' region in a tab
         ``FNB_LEFT_ARROW``              4 Indicates mouse coordinates inside the left arrow region
         ``FNB_RIGHT_ARROW``             5 Indicates mouse coordinates inside the right arrow region
         ``FNB_DROP_DOWN_ARROW``         6 Indicates mouse coordinates inside the drop down arrow region
         ========================= ======= =================================
         
        """

        agwStyle = self.GetParent().GetAGWWindowStyleFlag()
        render = self._mgr.GetRenderer(agwStyle)

        fullrect = self.GetClientRect()
        btnLeftPos = render.GetLeftButtonPos(self)
        btnRightPos = render.GetRightButtonPos(self)
        btnXPos = render.GetXPos(self)
        
        tabIdx = -1
        
        if len(self._pagesInfoVec) == 0:
            return FNB_NOWHERE, tabIdx

        rect = wx.Rect(btnXPos, 8, 16, 16)
        if rect.Contains(pt):
            return (agwStyle & FNB_NO_X_BUTTON and [FNB_NOWHERE] or [FNB_X])[0], tabIdx

        rect = wx.Rect(btnRightPos, 8, 16, 16)
        if agwStyle & FNB_DROPDOWN_TABS_LIST:
            rect = wx.Rect(render.GetDropArrowButtonPos(self), 8, 16, 16)
            if rect.Contains(pt):
                return FNB_DROP_DOWN_ARROW, tabIdx

        if rect.Contains(pt):
            return (agwStyle & FNB_NO_NAV_BUTTONS and [FNB_NOWHERE] or [FNB_RIGHT_ARROW])[0], tabIdx

        rect = wx.Rect(btnLeftPos, 8, 16, 16)
        if rect.Contains(pt):
            return (agwStyle & FNB_NO_NAV_BUTTONS and [FNB_NOWHERE] or [FNB_LEFT_ARROW])[0], tabIdx

        # Test whether a left click was made on a tab
        bFoundMatch = False
        
        for cur in xrange(self._nFrom, len(self._pagesInfoVec)):

            pgInfo = self._pagesInfoVec[cur]

            if pgInfo.GetPosition() == wx.Point(-1, -1):
                continue

            if agwStyle & FNB_X_ON_TAB and cur == self.GetSelection():
                # 'x' button exists on a tab
                if self._pagesInfoVec[cur].GetXRect().Contains(pt):
                    return FNB_TAB_X, cur
                    
            if agwStyle & FNB_VC8:

                if self._pagesInfoVec[cur].GetRegion().Contains(pt.x, pt.y):
                    if bFoundMatch or cur == self.GetSelection():    
                        return FNB_TAB, cur

                    tabIdx = cur
                    bFoundMatch = True
                    
            else:

                tabRect = wx.Rect(pgInfo.GetPosition().x, pgInfo.GetPosition().y,
                                  pgInfo.GetSize().x, pgInfo.GetSize().y)
                
                if tabRect.Contains(pt):
                    # We have a match
                    return FNB_TAB, cur

        if bFoundMatch:
            return FNB_TAB, tabIdx

        if self._isdragging:
            # We are doing DND, so check also the region outside the tabs
            # try before the first tab
            pgInfo = self._pagesInfoVec[0]
            tabRect = wx.Rect(0, pgInfo.GetPosition().y, pgInfo.GetPosition().x, self.GetParent().GetSize().y)
            if tabRect.Contains(pt):
                return FNB_TAB, 0

            # try after the last tab
            pgInfo = self._pagesInfoVec[-1]
            startpos = pgInfo.GetPosition().x+pgInfo.GetSize().x
            tabRect = wx.Rect(startpos, pgInfo.GetPosition().y, fullrect.width-startpos, self.GetParent().GetSize().y)

            if tabRect.Contains(pt):
                return FNB_TAB, len(self._pagesInfoVec)        

        # Default
        return FNB_NOWHERE, -1


    def SetSelection(self, page):
        """
        Sets the selected page.

        :param `page`: an integer specifying the page index.
        """

        book = self.GetParent()
        book.SetSelection(page)
        self.DoSetSelection(page)


    def DoSetSelection(self, page):
        """
        Does the actual selection of a page.

        :param `page`: an integer specifying the page index.
        """

        if page < len(self._pagesInfoVec):
            #! fix for tabfocus
            da_page = self._pParent.GetPage(page)
    
            if da_page != None:
                da_page.SetFocus()
        
        if not self.IsTabVisible(page):
            # Try to remove one tab from start and try again
            
            if not self.CanFitToScreen(page):

                if self._nFrom > page:
                    self._nFrom = page
                else:
                    while self._nFrom < page:
                        self._nFrom += 1
                        if self.CanFitToScreen(page):
                            break
 
        self.Refresh()


    def DeletePage(self, page):
        """
        Delete the specified page from L{PageContainer}.

        :param `page`: an integer specifying the page index.
        """

        book = self.GetParent()
        book.DeletePage(page)
        book.Refresh()


    def IsMouseHovering(self, page):
        """
        Returns whether or not the mouse is hovering over this page's tab

        :param `page`: an integer specifying the page index.
        """
        return page == self._nHoveringOverTabIndex
    
    def IsTabVisible(self, page):
        """
        Returns whether a tab is visible or not.

        :param `page`: an integer specifying the page index.
        """

        iLastVisiblePage = self.GetLastVisibleTab()
        return page <= iLastVisiblePage and page >= self._nFrom


    def DoDeletePage(self, page):
        """
        Does the actual page deletion.

        :param `page`: an integer specifying the page index.
        """

        # Remove the page from the vector
        book = self.GetParent()
        self._pagesInfoVec.pop(page)

        # Thanks to Yiaanis AKA Mandrav
        if self._iActivePage >= page:
            self._iActivePage = self._iActivePage - 1
            self._iPreviousActivePage = -1

        # The delete page was the last first on the array,
        # but the book still has more pages, so we set the
        # active page to be the first one (0)
        if self._iActivePage < 0 and len(self._pagesInfoVec) > 0:
            self._iActivePage = 0
            self._iPreviousActivePage = -1

        # Refresh the tabs
        if self._iActivePage >= 0:
        
            book._bForceSelection = True

            # Check for selection and send event
            event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CHANGING, self.GetParent().GetId())
            event.SetSelection(self._iActivePage)
            event.SetOldSelection(self._iPreviousActivePage)
            event.SetEventObject(self.GetParent())
            self.GetParent().GetEventHandler().ProcessEvent(event)            

            book.SetSelection(self._iActivePage)
            book._bForceSelection = False

            # Fire a wxEVT_FLATNOTEBOOK_PAGE_CHANGED event
            event.SetEventType(wxEVT_FLATNOTEBOOK_PAGE_CHANGED)
            event.SetOldSelection(self._iPreviousActivePage)
            self.GetParent().GetEventHandler().ProcessEvent(event)            
        
        #if not self._pagesInfoVec:        
        #    # Erase the page container drawings
        #    dc = wx.ClientDC(self)
        #    dc.Clear()
        

    def DeleteAllPages(self):
        """ Deletes all the pages in the L{PageContainer}. """

        self._iActivePage = -1
        self._iPreviousActivePage = -1
        self._nFrom = 0
        self._pagesInfoVec = []

        # Erase the page container drawings
        dc = wx.ClientDC(self)
        dc.Clear()


    def OnMouseMove(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if self._pagesInfoVec and self.IsShown():
        
            xButtonStatus = self._nXButtonStatus
            xTabButtonStatus = self._nTabXButtonStatus
            rightButtonStatus = self._nRightButtonStatus
            leftButtonStatus = self._nLeftButtonStatus
            dropDownButtonStatus = self._nArrowDownButtonStatus
            
            agwStyle = self.GetParent().GetAGWWindowStyleFlag()

            self._nXButtonStatus = FNB_BTN_NONE
            self._nRightButtonStatus = FNB_BTN_NONE
            self._nLeftButtonStatus = FNB_BTN_NONE
            self._nTabXButtonStatus = FNB_BTN_NONE
            self._nArrowDownButtonStatus = FNB_BTN_NONE
            bRedrawTabs = False
            self._nHoveringOverTabIndex = -1
                

            where, tabIdx = self.HitTest(event.GetPosition())
            
            if where == FNB_X:
                if event.LeftIsDown():
                
                    self._nXButtonStatus = (self._nLeftClickZone==FNB_X and [FNB_BTN_PRESSED] or [FNB_BTN_NONE])[0]
                
                else:
                
                    self._nXButtonStatus = FNB_BTN_HOVER

            elif where == FNB_DROP_DOWN_ARROW:
                if event.LeftIsDown():

                    self._nArrowDownButtonStatus = (self._nLeftClickZone==FNB_DROP_DOWN_ARROW and [FNB_BTN_PRESSED] or [FNB_BTN_NONE])[0]

                else:

                    self._nArrowDownButtonStatus = FNB_BTN_HOVER

            elif where == FNB_TAB_X:
                if event.LeftIsDown():
                
                    self._nTabXButtonStatus = (self._nLeftClickZone==FNB_TAB_X and [FNB_BTN_PRESSED] or [FNB_BTN_NONE])[0]
                
                else:

                    self._nTabXButtonStatus = FNB_BTN_HOVER
                
            elif where == FNB_RIGHT_ARROW:
                if event.LeftIsDown():
                
                    self._nRightButtonStatus = (self._nLeftClickZone==FNB_RIGHT_ARROW and [FNB_BTN_PRESSED] or [FNB_BTN_NONE])[0]
                
                else:
                
                    self._nRightButtonStatus = FNB_BTN_HOVER
                
            elif where == FNB_LEFT_ARROW:
                if event.LeftIsDown():
                
                    self._nLeftButtonStatus = (self._nLeftClickZone==FNB_LEFT_ARROW and [FNB_BTN_PRESSED] or [FNB_BTN_NONE])[0]
                
                else:
                
                    self._nLeftButtonStatus = FNB_BTN_HOVER
                
            elif where == FNB_TAB:
                # Call virtual method for showing tooltip
                self.ShowTabTooltip(tabIdx)
                
                if not self.GetEnabled(tabIdx):                
                    # Set the cursor to be 'No-entry'
                    wx.SetCursor(wx.StockCursor(wx.CURSOR_NO_ENTRY))
                    self._setCursor = True
                else:
                    self._nHoveringOverTabIndex = tabIdx
                    if self._setCursor:
                        wx.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
                        self._setCursor = False
                
                # Support for drag and drop
                if event.Dragging() and not (agwStyle & FNB_NODRAG):

                    self._isdragging = True                
                    draginfo = FNBDragInfo(self, tabIdx)
                    drginfo = cPickle.dumps(draginfo)
                    dataobject = wx.CustomDataObject(wx.CustomDataFormat("FlatNotebook"))
                    dataobject.SetData(drginfo)
                    dragSource = FNBDropSource(self)
                    dragSource.SetData(dataobject)
                    dragSource.DoDragDrop(wx.Drag_DefaultMove)
                    
            if self._nHoveringOverTabIndex != self._nHoveringOverLastTabIndex:
                self._nHoveringOverLastTabIndex = self._nHoveringOverTabIndex
                if self._nHoveringOverTabIndex >= 0:
                    bRedrawTabs = True
                    
            bRedrawX = self._nXButtonStatus != xButtonStatus
            bRedrawRight = self._nRightButtonStatus != rightButtonStatus
            bRedrawLeft = self._nLeftButtonStatus != leftButtonStatus
            bRedrawTabX = self._nTabXButtonStatus != xTabButtonStatus
            bRedrawDropArrow = self._nArrowDownButtonStatus != dropDownButtonStatus

            render = self._mgr.GetRenderer(agwStyle)
        
            if (bRedrawX or bRedrawRight or bRedrawLeft or bRedrawTabX or bRedrawDropArrow or bRedrawTabs):

                dc = wx.ClientDC(self)
                
                if bRedrawX:
                                
                    render.DrawX(self, dc)
                
                if bRedrawLeft:
                
                    render.DrawLeftArrow(self, dc)
                
                if bRedrawRight:
                
                    render.DrawRightArrow(self, dc)
                
                if bRedrawTabX or bRedrawTabs:
                
                    self.Refresh()

                if bRedrawDropArrow:

                    render.DrawDropDownArrow(self, dc)
                    
        event.Skip()


    def GetLastVisibleTab(self):
        """ Returns the last visible tab in the tab area. """

        if self._nFrom < 0:
            return -1

        ii = 0
        
        for ii in xrange(self._nFrom, len(self._pagesInfoVec)):
        
            if self._pagesInfoVec[ii].GetPosition() == wx.Point(-1, -1):
                break
        
        return ii-1


    def GetNumTabsCanScrollLeft(self):
        """ Returns the number of tabs than can be scrolled left. """

        if self._nFrom - 1 >= 0:
            return 1

        return 0


    def IsDefaultTabs(self):
        """ Returns whether a tab has a default style. """

        agwStyle = self.GetParent().GetAGWWindowStyleFlag()
        res = (agwStyle & FNB_VC71) or (agwStyle & FNB_FANCY_TABS) or (agwStyle & FNB_VC8)
        return not res


    def AdvanceSelection(self, forward=True):
        """
        Cycles through the tabs.

        :param `forward`: if ``True``, the selection is advanced in ascending order
         (to the right), otherwise the selection is advanced in descending order.
         
        :note: The call to this function generates the page changing events.
        """

        nSel = self.GetSelection()

        if nSel < 0:
            return

        nMax = self.GetPageCount() - 1
        
        if forward:
            newSelection = (nSel == nMax and [0] or [nSel + 1])[0]
        else:
            newSelection = (nSel == 0 and [nMax] or [nSel - 1])[0]

        if not self._pagesInfoVec[newSelection].GetEnabled():
            return

        self.FireEvent(newSelection)


    def OnMouseLeave(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self._nLeftButtonStatus = FNB_BTN_NONE
        self._nXButtonStatus = FNB_BTN_NONE
        self._nRightButtonStatus = FNB_BTN_NONE
        self._nTabXButtonStatus = FNB_BTN_NONE
        self._nArrowDownButtonStatus = FNB_BTN_NONE
        self._nHoveringOverTabIndex = -1
        self._nHoveringOverLastTabIndex = -1

        self.Refresh()
        selection = self.GetSelection()

        if selection == -1:
            event.Skip()
            return
        
        if not self.IsTabVisible(selection):
            if selection == len(self._pagesInfoVec) - 1:
                if not self.CanFitToScreen(selection):
                    event.Skip()
                    return
            else:
                event.Skip()
                return
                    
        agwStyle = self.GetParent().GetAGWWindowStyleFlag()        
        render = self._mgr.GetRenderer(agwStyle)
        dc = wx.ClientDC(self)                    
        render.DrawTabX(self, dc, self._pagesInfoVec[selection].GetXRect(), selection, self._nTabXButtonStatus)
        if not agwStyle & FNB_RIBBON_TABS:
            render.DrawFocusRectangle(dc, self, self._pagesInfoVec[selection])

        event.Skip()


    def OnMouseEnterWindow(self, event):
        """
        Handles the ``wx.EVT_ENTER_WINDOW`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        self._nLeftButtonStatus = FNB_BTN_NONE
        self._nXButtonStatus = FNB_BTN_NONE
        self._nRightButtonStatus = FNB_BTN_NONE
        self._nLeftClickZone = FNB_BTN_NONE
        self._nArrowDownButtonStatus = FNB_BTN_NONE

        event.Skip()


    def ShowTabTooltip(self, tabIdx):
        """
        Shows a tab tooltip.
    
        :param `tabIdx`: an integer specifying the page index.
        """

        pWindow = self._pParent.GetPage(tabIdx)
        
        if pWindow:        
            pToolTip = pWindow.GetToolTip()
            if pToolTip and pToolTip.GetWindow() == pWindow:
                self.SetToolTipString(pToolTip.GetTip())
        

    def SetPageImage(self, page, image):
        """
        Sets the image index for the given page.

        :param `page`: an integer specifying the page index;
        :param `image`: an index into the image list which was set with L{SetImageList}.
        """

        if page < len(self._pagesInfoVec):
            self._pagesInfoVec[page].SetImageIndex(image)
            self.Refresh()


    def GetPageImage(self, page):
        """
        Returns the image index associated to a page.

        :param `page`: an integer specifying the page index.
        """

        if page < len(self._pagesInfoVec):
            return self._pagesInfoVec[page].GetImageIndex()
        
        return -1


    def GetPageTextColour(self, page):
        """
        Returns the tab text colour if it has been set previously, or ``None`` otherwise.

        :param `page`: an integer specifying the page index.
        """

        if page < len(self._pagesInfoVec):    
            return self._pagesInfoVec[page].GetPageTextColour()

        return None
    

    def SetPageTextColour(self, page, colour):
        """
        Sets the tab text colour individually.

        :param `page`: an integer specifying the page index;
        :param `colour`: an instance of `wx.Colour`. You can pass ``None`` or
         `wx.NullColour` to return to the default page text colour.
        """

        if page < len(self._pagesInfoVec):
            self._pagesInfoVec[page].SetPageTextColour(colour)
            self.Refresh()


    def OnDropTarget(self, x, y, nTabPage, wnd_oldContainer):
        """
        Handles the drop action from a drag and drop operation.

        :param `x`: the x position of the drop action;
        :param `y`: the y position of the drop action;
        :param `nTabPage`: the index of the tab being dropped;
        :param `wnd_oldContainer`: the L{FlatNotebook} to which the dropped tab previously
         belonged to.
        """

        # Disable drag'n'drop for disabled tab
        if len(wnd_oldContainer._pagesInfoVec) > nTabPage and \
           not wnd_oldContainer._pagesInfoVec[nTabPage].GetEnabled():
            return wx.DragCancel

        self._isdragging = True
        oldContainer = wnd_oldContainer
        nIndex = -1

        where, nIndex = self.HitTest(wx.Point(x, y))

        oldNotebook = oldContainer.GetParent()
        newNotebook = self.GetParent()

        if oldNotebook == newNotebook:
        
            if nTabPage >= 0:
            
                if where == FNB_TAB:
                    self.MoveTabPage(nTabPage, nIndex)
                           
                event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_DROPPED, self.GetParent().GetId())
                event.SetSelection(nIndex)
                event.SetOldSelection(nTabPage)
                event.SetEventObject(self.GetParent())
                self.GetParent().GetEventHandler().ProcessEvent(event)

        elif self.GetParent().GetAGWWindowStyleFlag() & FNB_ALLOW_FOREIGN_DND:
        
            if wx.Platform in ["__WXMSW__", "__WXGTK__", "__WXMAC__"]:
                if nTabPage >= 0:
                
                    window = oldNotebook.GetPage(nTabPage)

                    if window:
                        where, nIndex = newNotebook._pages.HitTest(wx.Point(x, y))
                        caption = oldContainer.GetPageText(nTabPage)
                        imageindex = oldContainer.GetPageImage(nTabPage)
                        oldNotebook.RemovePage(nTabPage)
                        window.Reparent(newNotebook)

                        if imageindex >= 0:

                            bmp = oldNotebook.GetImageList().GetBitmap(imageindex)
                            newImageList = newNotebook.GetImageList()
    
                            if not newImageList:
                                xbmp, ybmp = bmp.GetWidth(), bmp.GetHeight()
                                newImageList = wx.ImageList(xbmp, ybmp)                                
                                imageindex = 0
                            else:
                                imageindex = newImageList.GetImageCount()

                            newImageList.Add(bmp)
                            newNotebook.SetImageList(newImageList)
                                
                        newNotebook.InsertPage(nIndex, window, caption, True, imageindex)

                    event = FlatNotebookDragEvent(wxEVT_FLATNOTEBOOK_PAGE_DROPPED_FOREIGN, self.GetParent().GetId())
                    event.SetSelection(nIndex)
                    event.SetOldSelection(nTabPage)
                    event.SetNotebook(newNotebook)
                    event.SetOldNotebook(oldNotebook)
                    event.SetEventObject(self.GetParent())
                    self.GetParent().GetEventHandler().ProcessEvent(event)

        self._isdragging = False
        
        return wx.DragMove


    def MoveTabPage(self, nMove, nMoveTo):
        """
        Moves a tab inside the same L{FlatNotebook}.

        :param `nMove`: the start index of the moved tab;
        :param `nMoveTo`: the destination index of the moved tab.
        """

        if nMove == nMoveTo:
            return

        elif nMoveTo < len(self._pParent._windows):
            nMoveTo = nMoveTo + 1

        self._pParent.Freeze()
        
        # Remove the window from the main sizer
        nCurSel = self._pParent._pages.GetSelection()
        self._pParent._mainSizer.Detach(self._pParent._windows[nCurSel])
        self._pParent._windows[nCurSel].Hide()

        pWindow = self._pParent._windows[nMove]
        self._pParent._windows.pop(nMove)
        self._pParent._windows.insert(nMoveTo-1, pWindow)

        pgInfo = self._pagesInfoVec[nMove]

        self._pagesInfoVec.pop(nMove)
        self._pagesInfoVec.insert(nMoveTo - 1, pgInfo)

        # Add the page according to the style
        pSizer = self._pParent._mainSizer
        agwStyle = self.GetParent().GetAGWWindowStyleFlag()

        if agwStyle & FNB_BOTTOM:
        
            pSizer.Insert(0, pWindow, 1, wx.EXPAND)
        
        else:
        
            # We leave a space of 1 pixel around the window
            pSizer.Add(pWindow, 1, wx.EXPAND)
        
        pWindow.Show()

        pSizer.Layout()
        self._iActivePage = nMoveTo - 1
        self._iPreviousActivePage = -1
        self.DoSetSelection(self._iActivePage)
        self.Refresh()
        self._pParent.Thaw()


    def CanFitToScreen(self, page):
        """
        Returns wheter a tab can fit in the left space in the screen or not.

        :param `page`: an integer specifying the page index.
        """

        # Incase the from is greater than page,
        # we need to reset the self._nFrom, so in order
        # to force the caller to do so, we return false
        if self._nFrom > page:
            return False

        agwStyle = self.GetParent().GetAGWWindowStyleFlag()
        render = self._mgr.GetRenderer(agwStyle)

        vTabInfo = render.NumberTabsCanFit(self)

        if page - self._nFrom >= len(vTabInfo):
            return False
        
        return True


    def GetNumOfVisibleTabs(self):
        """ Returns the number of visible tabs. """

        count = 0
        for ii in xrange(self._nFrom, len(self._pagesInfoVec)):
            if self._pagesInfoVec[ii].GetPosition() == wx.Point(-1, -1):
                break
            count = count + 1

        return count


    def GetEnabled(self, page):
        """
        Returns whether a tab is enabled or not.

        :param `page`: an integer specifying the page index.
        """

        if page >= len(self._pagesInfoVec):
            return True # Seems strange, but this is the default
        
        return self._pagesInfoVec[page].GetEnabled()


    def EnableTab(self, page, enabled=True):
        """
        Enables or disables a tab.

        :param `page`: an integer specifying the page index;
        :param `enabled`: ``True`` to enable a tab, ``False`` to disable it.
        """

        if page >= len(self._pagesInfoVec):
            return
        
        self._pagesInfoVec[page].EnableTab(enabled)
        

    def GetSingleLineBorderColour(self):
        """ Returns the colour for the single line border. """

        if self.HasAGWFlag(FNB_FANCY_TABS):
            return self._colourFrom
        
        return wx.WHITE


    def HasAGWFlag(self, flag):
        """
        Returns whether a flag is present in the L{FlatNotebook} style.

        :param `flag`: one of the possible L{FlatNotebook} window styles.

        :see: L{FlatNotebook.SetAGWWindowStyleFlag} for a list of possible window
         style flags.
        """

        agwStyle = self.GetParent().GetAGWWindowStyleFlag()
        res = (agwStyle & flag and [True] or [False])[0]
        return res


    def ClearAGWFlag(self, flag):
        """
        Deletes a flag from the L{FlatNotebook} style.

        :param `flag`: one of the possible L{FlatNotebook} window styles.

        :see: L{FlatNotebook.SetAGWWindowStyleFlag} for a list of possible window
         style flags.
        """

        parent = self.GetParent()
        agwStyle = parent.GetAGWWindowStyleFlag()
        agwStyle &= ~flag 
        parent.SetAGWWindowStyleFlag(agwStyle)


    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the L{FlatNotebook} window style.

        :param `agwStyle`: the new L{FlatNotebook} window style.
        
        :see: The L{FlatNotebook.__init__} method for the `agwStyle` parameter description.
        """

        self.GetParent().SetAGWWindowStyleFlag(agwStyle)


    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{FlatNotebook} window style.

        :see: The L{FlatNotebook.__init__} method for the `agwStyle` parameter description.
        """

        return self.GetParent().GetAGWWindowStyleFlag()
    

    def TabHasImage(self, tabIdx):
        """
        Returns whether a tab has an associated image index or not.

        :param `tabIdx`: an integer specifying the page index.
        """

        if self._ImageList:
            return self._pagesInfoVec[tabIdx].GetImageIndex() != -1
        
        return False


    def OnLeftDClick(self, event):
        """
        Handles the ``wx.EVT_LEFT_DCLICK`` event for L{PageContainer}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        where, tabIdx = self.HitTest(event.GetPosition())
        
        if where == FNB_RIGHT_ARROW:
            self._nRightButtonStatus = FNB_BTN_PRESSED
            self.RotateRight()

        elif where == FNB_LEFT_ARROW:
            self._nLeftButtonStatus = FNB_BTN_PRESSED
            self.RotateLeft()

        elif self.HasAGWFlag(FNB_DCLICK_CLOSES_TABS):
        
            if where == FNB_TAB:
                self.DeletePage(tabIdx)
        
        else:
        
            event.Skip()
        

    def OnSetFocus(self, event):
        """
        Handles the ``wx.EVT_SET_FOCUS`` event for L{PageContainer}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        if self._iActivePage < 0:
            event.Skip()
            return

        self.SetFocusedPage(self._iActivePage)


    def OnKillFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{PageContainer}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self.SetFocusedPage()


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{PageContainer}.

        :param `event`: a `wx.KeyEvent` event to be processed.

        :note: When the L{PageContainer} has the focus tabs can be changed with
         the left/right arrow keys.
        """
        
        key = event.GetKeyCode()
        if key == wx.WXK_LEFT:
            self.GetParent().AdvanceSelection(False)
            self.SetFocus()
        elif key == wx.WXK_RIGHT:
            self.GetParent().AdvanceSelection(True)
            self.SetFocus()
        elif key == wx.WXK_TAB and not event.ControlDown():
            flags = 0
            if not event.ShiftDown(): flags |= wx.NavigationKeyEvent.IsForward
            if event.CmdDown():       flags |= wx.NavigationKeyEvent.WinChange
            self.Navigate(flags)
        else:
            event.Skip()

            
    def SetFocusedPage(self, pageIndex=-1):
        """
        Sets/Unsets the focus on the appropriate page.
        
        :param `pageIndex`: an integer specifying the page index. If `pageIndex`
         is defaulted to -1, we have lost focus and no focus indicator is drawn.
        """

        for indx, page in enumerate(self._pagesInfoVec):
            if indx == pageIndex:
                page._hasFocus = True
            else:
                page._hasFocus = False
            
        self.Refresh()
                

    def PopupTabsMenu(self):
        """ Pops up the menu activated with the drop down arrow in the navigation area. """

        popupMenu = wx.Menu()

        for i in xrange(len(self._pagesInfoVec)):
            pi = self._pagesInfoVec[i]
            item = wx.MenuItem(popupMenu, i+1, pi.GetCaption(), pi.GetCaption(), wx.ITEM_NORMAL)
            self.Bind(wx.EVT_MENU, self.OnTabMenuSelection, item)

            # There is an alignment problem with wx2.6.3 & Menus so only use
            # images for versions above 2.6.3
            if wx.VERSION > (2, 6, 3, 0) and self.TabHasImage(i):
                item.SetBitmap(self.GetImageList().GetBitmap(pi.GetImageIndex()))

            popupMenu.AppendItem(item)
            item.Enable(pi.GetEnabled())
            
        self.PopupMenu(popupMenu)


    def OnTabMenuSelection(self, event):
        """
        Handles the ``wx.EVT_MENU`` event for L{PageContainer}.

        :param `event`: a `wx.MenuEvent` event to be processed.
        """

        selection = event.GetId() - 1
        self.FireEvent(selection)


    def FireEvent(self, selection):
        """
        Fires the ``EVT_FLATNOTEBOOK_PAGE_CHANGING`` and ``EVT_FLATNOTEBOOK_PAGE_CHANGED``
        events called from other methods (from menu selection or `Smart Tabbing`).

        This is an utility function.

        :param `selection`: the new selection inside L{FlatNotebook}.        
        """

        if selection == self._iActivePage:
            # No events for the same selection
            return
        
        oldSelection = self._iActivePage

        event = FlatNotebookEvent(wxEVT_FLATNOTEBOOK_PAGE_CHANGING, self.GetParent().GetId())
        event.SetSelection(selection)
        event.SetOldSelection(oldSelection)
        event.SetEventObject(self.GetParent())
        
        if not self.GetParent().GetEventHandler().ProcessEvent(event) or event.IsAllowed():
        
            self.SetSelection(selection)

            # Fire a wxEVT_FLATNOTEBOOK_PAGE_CHANGED event
            event.SetEventType(wxEVT_FLATNOTEBOOK_PAGE_CHANGED)
            event.SetOldSelection(oldSelection)
            self.GetParent().GetEventHandler().ProcessEvent(event)
            if not self.HasAGWFlag(FNB_NO_TAB_FOCUS):
                self.SetFocus()
            

    def SetImageList(self, imglist):
        """
        Sets the image list for the L{PageContainer}.

        :param `imageList`: an instance of `wx.ImageList`.
        """

        self._ImageList = imglist


    def AssignImageList(self, imglist):
        """
        Assigns the image list for the L{PageContainer}.

        :param `imageList`: an instance of `wx.ImageList`.
        """

        self._ImageList = imglist


    def GetImageList(self):
        """ Returns the image list for the page control. """

        return self._ImageList


    def GetSelection(self):
        """ Returns the current selected page. """

        return self._iActivePage 


    def GetPageCount(self):
        """ Returns the number of tabs in the L{FlatNotebook} control. """

        return len(self._pagesInfoVec)


    def GetPageText(self, page):
        """
        Returns the tab caption of the page.

        :param `page`: an integer specifying the page index.
        """
        
        if page < len(self._pagesInfoVec):
            return self._pagesInfoVec[page].GetCaption() 
        else:
            return u''


    def SetPageText(self, page, text):
        """
        Sets the tab caption of the page.

        :param `page`: an integer specifying the page index;
        :param `text`: the new tab label.
        """
        
        if page < len(self._pagesInfoVec):
            self._pagesInfoVec[page].SetCaption(text)
            return True 
        else:
            return False


    def DrawDragHint(self):
        """ Draws small arrow at the place that the tab will be placed. """

        # get the index of tab that will be replaced with the dragged tab
        pt = wx.GetMousePosition()
        client_pt = self.ScreenToClient(pt)
        where, tabIdx = self.HitTest(client_pt)
        self._mgr.GetRenderer(self.GetParent().GetAGWWindowStyleFlag()).DrawDragHint(self, tabIdx)


# ---------------------------------------------------------------------------- #
# Class FlatNotebookCompatible
# This class is more compatible with the wx.Notebook API.
# ---------------------------------------------------------------------------- #

class FlatNotebookCompatible(FlatNotebook):
    """
    This class is more compatible with the `wx.Notebook` API, especially regarding
    page changing events. Use the L{FlatNotebookCompatible.SetSelection} method if you wish to send page
    changing events, or L{FlatNotebookCompatible.ChangeSelection} otherwise.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=0, name="FlatNotebook"):
        """
        Default class constructor.

        :param `parent`: the L{FlatNotebook} parent;
        :param `id`: an identifier for the control: a value of -1 is taken to mean a default;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyPanel` window style;
        :param `agwStyle`: the AGW-specific window style. This can be a combination of the
         following bits:

         ================================ =========== ==================================================
         Window Styles                    Hex Value   Description
         ================================ =========== ==================================================
         ``FNB_VC71``                             0x1 Use Visual Studio 2003 (VC7.1) style for tabs.
         ``FNB_FANCY_TABS``                       0x2 Use fancy style - square tabs filled with gradient colouring.
         ``FNB_TABS_BORDER_SIMPLE``               0x4 Draw thin border around the page.
         ``FNB_NO_X_BUTTON``                      0x8 Do not display the 'X' button.
         ``FNB_NO_NAV_BUTTONS``                  0x10 Do not display the right/left arrows.
         ``FNB_MOUSE_MIDDLE_CLOSES_TABS``        0x20 Use the mouse middle button for cloing tabs.
         ``FNB_BOTTOM``                          0x40 Place tabs at bottom - the default is to place them at top.
         ``FNB_NODRAG``                          0x80 Disable dragging of tabs.
         ``FNB_VC8``                            0x100 Use Visual Studio 2005 (VC8) style for tabs.
         ``FNB_X_ON_TAB``                       0x200 Place 'X' close button on the active tab.
         ``FNB_BACKGROUND_GRADIENT``            0x400 Use gradients to paint the tabs background.
         ``FNB_COLOURFUL_TABS``                 0x800 Use colourful tabs (VC8 style only).
         ``FNB_DCLICK_CLOSES_TABS``            0x1000 Style to close tab using double click.
         ``FNB_SMART_TABS``                    0x2000 Use `Smart Tabbing`, like ``Alt`` + ``Tab`` on Windows.
         ``FNB_DROPDOWN_TABS_LIST``            0x4000 Use a dropdown menu on the left in place of the arrows.
         ``FNB_ALLOW_FOREIGN_DND``             0x8000 Allows drag 'n' drop operations between different FlatNotebooks.
         ``FNB_HIDE_ON_SINGLE_TAB``           0x10000 Hides the Page Container when there is one or fewer tabs.
         ``FNB_DEFAULT_STYLE``                0x10020 FlatNotebook default style.
         ``FNB_FF2``                          0x20000 Use Firefox 2 style for tabs.
         ``FNB_NO_TAB_FOCUS``                 0x40000 Does not allow tabs to have focus.
         ``FNB_RIBBON_TABS``                  0x80000 Use the Ribbon Tabs style.
         ================================ =========== ==================================================
        
        :param `name`: the window name. 
        """

        FlatNotebook.__init__(self, parent, id, pos, size, style, agwStyle, name)


    def SetSelection(self, page):
        """
        Sets the selection for the given page.

        :param `page`: an integer specifying the new selected page.
        
        :note: The call to this function **generates** the page changing events.
        """

        if page >= len(self._windows) or not self._windows:
            return

        # Support for disabed tabs
        if not self._pages.GetEnabled(page) and len(self._windows) > 1 and not self._bForceSelection:
            return
        
        self.FireEvent(page)


    def ChangeSelection(self, page):
        """
        Sets the selection for the given page.

        :param `page`: an integer specifying the new selected page.
        
        :note: The call to this function **does not** generate the page changing events.
        """

        FlatNotebook.SetSelection(self, page)

        
