# --------------------------------------------------------------------------- #
# AUI Library wxPython IMPLEMENTATION
#
# Original C++ Code From Kirix (wxAUI). You Can Find It At:
#
#    License: wxWidgets license
#
# http:#www.kirix.com/en/community/opensource/wxaui/about_wxaui.html
#
# Current wxAUI Version Tracked: wxWidgets 2.9.0 SVN HEAD
#
#
# Python Code By:
#
# Andrea Gavana, @ 23 Dec 2005
# Latest Revision: 10 Mar 2011, 15.00 GMT
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# andrea.gavana@gmail.com
# gavana@kpo.kz
#
# Or, Obviously, To The wxPython Mailing List!!!
#
# End Of Comments
# --------------------------------------------------------------------------- #

"""
Description
===========

framemanager is the central module of the AUI class framework.

L{AuiManager} manages the panes associated with it for a particular `wx.Frame`, using
a pane's L{AuiPaneInfo} information to determine each pane's docking and floating
behavior. AuiManager uses wxPython' sizer mechanism to plan the layout of each frame.
It uses a replaceable dock art class to do all drawing, so all drawing is localized
in one area, and may be customized depending on an application's specific needs.

AuiManager works as follows: the programmer adds panes to the class, or makes
changes to existing pane properties (dock position, floating state, show state, etc...).
To apply these changes, AuiManager's L{AuiManager.Update} function is called. This batch
processing can be used to avoid flicker, by modifying more than one pane at a time,
and then "committing" all of the changes at once by calling `Update()`.

Panes can be added quite easily::

    text1 = wx.TextCtrl(self, -1)
    text2 = wx.TextCtrl(self, -1)
    self._mgr.AddPane(text1, AuiPaneInfo().Left().Caption("Pane Number One"))
    self._mgr.AddPane(text2, AuiPaneInfo().Bottom().Caption("Pane Number Two"))

    self._mgr.Update()


Later on, the positions can be modified easily. The following will float an
existing pane in a tool window::

    self._mgr.GetPane(text1).Float()


Layers, Rows and Directions, Positions
======================================

Inside AUI, the docking layout is figured out by checking several pane parameters.
Four of these are important for determining where a pane will end up.

**Direction** - Each docked pane has a direction, `Top`, `Bottom`, `Left`, `Right`, or `Center`.
This is fairly self-explanatory. The pane will be placed in the location specified
by this variable.

**Position** - More than one pane can be placed inside of a "dock". Imagine two panes
being docked on the left side of a window. One pane can be placed over another.
In proportionally managed docks, the pane position indicates it's sequential position,
starting with zero. So, in our scenario with two panes docked on the left side, the
top pane in the dock would have position 0, and the second one would occupy position 1. 

**Row** - A row can allow for two docks to be placed next to each other. One of the most
common places for this to happen is in the toolbar. Multiple toolbar rows are allowed,
the first row being in row 0, and the second in row 1. Rows can also be used on
vertically docked panes. 

**Layer** - A layer is akin to an onion. Layer 0 is the very center of the managed pane.
Thus, if a pane is in layer 0, it will be closest to the center window (also sometimes
known as the "content window"). Increasing layers "swallow up" all layers of a lower
value. This can look very similar to multiple rows, but is different because all panes
in a lower level yield to panes in higher levels. The best way to understand layers
is by running the AUI sample (`AUI.py`).
"""

__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx
import time
import types
import warnings

import auibar
import auibook
import tabmdi
import dockart
import tabart

from aui_utilities import Clip, PaneCreateStippleBitmap, GetDockingImage, GetSlidingPoints

from aui_constants import *

# Define this as a translation function
_ = wx.GetTranslation

_winxptheme = False
if wx.Platform == "__WXMSW__":
    try:
        import winxptheme
        _winxptheme = True
    except ImportError:
        pass

# wxPython version string
_VERSION_STRING = wx.VERSION_STRING

# AUI Events
wxEVT_AUI_PANE_BUTTON = wx.NewEventType()
wxEVT_AUI_PANE_CLOSE = wx.NewEventType()
wxEVT_AUI_PANE_MAXIMIZE = wx.NewEventType()
wxEVT_AUI_PANE_RESTORE = wx.NewEventType()
wxEVT_AUI_RENDER = wx.NewEventType()
wxEVT_AUI_FIND_MANAGER = wx.NewEventType()
wxEVT_AUI_PANE_MINIMIZE = wx.NewEventType()
wxEVT_AUI_PANE_MIN_RESTORE = wx.NewEventType()
wxEVT_AUI_PANE_FLOATING = wx.NewEventType()
wxEVT_AUI_PANE_FLOATED = wx.NewEventType()
wxEVT_AUI_PANE_DOCKING = wx.NewEventType()
wxEVT_AUI_PANE_DOCKED = wx.NewEventType()
wxEVT_AUI_PANE_ACTIVATED = wx.NewEventType()
wxEVT_AUI_PERSPECTIVE_CHANGED = wx.NewEventType()

EVT_AUI_PANE_BUTTON = wx.PyEventBinder(wxEVT_AUI_PANE_BUTTON, 0)
""" Fires an event when the user left-clicks on a pane button. """
EVT_AUI_PANE_CLOSE = wx.PyEventBinder(wxEVT_AUI_PANE_CLOSE, 0)
""" A pane in `AuiManager` has been closed. """
EVT_AUI_PANE_MAXIMIZE = wx.PyEventBinder(wxEVT_AUI_PANE_MAXIMIZE, 0)
""" A pane in `AuiManager` has been maximized. """
EVT_AUI_PANE_RESTORE = wx.PyEventBinder(wxEVT_AUI_PANE_RESTORE, 0)
""" A pane in `AuiManager` has been restored from a maximized state. """
EVT_AUI_RENDER = wx.PyEventBinder(wxEVT_AUI_RENDER, 0)
""" Fires an event every time the AUI frame is being repainted. """
EVT_AUI_FIND_MANAGER = wx.PyEventBinder(wxEVT_AUI_FIND_MANAGER, 0)
""" Used to find which AUI manager is controlling a certain pane. """
EVT_AUI_PANE_MINIMIZE = wx.PyEventBinder(wxEVT_AUI_PANE_MINIMIZE, 0)
""" A pane in `AuiManager` has been minimized. """
EVT_AUI_PANE_MIN_RESTORE = wx.PyEventBinder(wxEVT_AUI_PANE_MIN_RESTORE, 0)
""" A pane in `AuiManager` has been restored from a minimized state. """
EVT_AUI_PANE_FLOATING = wx.PyEventBinder(wxEVT_AUI_PANE_FLOATING, 0)
""" A pane in `AuiManager` is about to be floated. """
EVT_AUI_PANE_FLOATED = wx.PyEventBinder(wxEVT_AUI_PANE_FLOATED, 0)
""" A pane in `AuiManager` has been floated. """
EVT_AUI_PANE_DOCKING = wx.PyEventBinder(wxEVT_AUI_PANE_DOCKING, 0)
""" A pane in `AuiManager` is about to be docked. """
EVT_AUI_PANE_DOCKED = wx.PyEventBinder(wxEVT_AUI_PANE_DOCKED, 0)
""" A pane in `AuiManager` has been docked. """
EVT_AUI_PANE_ACTIVATED = wx.PyEventBinder(wxEVT_AUI_PANE_ACTIVATED, 0)
""" A pane in `AuiManager` has been activated. """
EVT_AUI_PERSPECTIVE_CHANGED = wx.PyEventBinder(wxEVT_AUI_PERSPECTIVE_CHANGED, 0)
""" The layout in `AuiManager` has been changed. """

# ---------------------------------------------------------------------------- #

class AuiDockInfo(object):
    """ A class to store all properties of a dock. """

    def __init__(self):
        """
        Default class constructor.
        Used internally, do not call it in your code!
        """

        object.__init__(self)
        
        self.dock_direction = 0
        self.dock_layer = 0
        self.dock_row = 0
        self.size = 0
        self.min_size = 0
        self.resizable = True
        self.fixed = False
        self.toolbar = False
        self.rect = wx.Rect()
        self.panes = []


    def IsOk(self):
        """
        Returns whether a dock is valid or not.

        In order to be valid, a dock needs to have a non-zero `dock_direction`.
        """

        return self.dock_direction != 0

    
    def IsHorizontal(self):
        """ Returns whether the dock is horizontal or not. """

        return self.dock_direction in [AUI_DOCK_TOP, AUI_DOCK_BOTTOM]


    def IsVertical(self):
        """ Returns whether the dock is vertical or not. """

        return self.dock_direction in [AUI_DOCK_LEFT, AUI_DOCK_RIGHT, AUI_DOCK_CENTER]
    

# ---------------------------------------------------------------------------- #

class AuiDockingGuideInfo(object):
    """ A class which holds information about VS2005 docking guide windows. """

    def __init__(self, other=None):
        """
        Default class constructor.
        Used internally, do not call it in your code!

        :param `other`: another instance of L{AuiDockingGuideInfo}.
        """

        if other:
            self.Assign(other)
        else:
            # window representing the docking target
            self.host = None
            # dock direction (top, bottom, left, right, center)
            self.dock_direction = AUI_DOCK_NONE


    def Assign(self, other):
        """
        Assigns the properties of the `other` L{AuiDockingGuideInfo} to `self`.

        :param `other`: another instance of L{AuiDockingGuideInfo}.
        """

        self.host = other.host
        self.dock_direction = other.dock_direction


    def Host(self, h):
        """
        Hosts a docking guide window.

        :param `h`: an instance of L{AuiSingleDockingGuide} or L{AuiCenterDockingGuide}.
        """

        self.host = h
        return self
    

    def Left(self):
        """ Sets the guide window to left docking. """

        self.dock_direction = AUI_DOCK_LEFT
        return self

    
    def Right(self):
        """ Sets the guide window to right docking. """

        self.dock_direction = AUI_DOCK_RIGHT
        return self 


    def Top(self):
        """ Sets the guide window to top docking. """

        self.dock_direction = AUI_DOCK_TOP
        return self 


    def Bottom(self):
        """ Sets the guide window to bottom docking. """

        self.dock_direction = AUI_DOCK_BOTTOM
        return self 


    def Center(self):
        """ Sets the guide window to center docking. """

        self.dock_direction = AUI_DOCK_CENTER
        return self 


    def Centre(self):
        """ Sets the guide window to centre docking. """
        
        self.dock_direction = AUI_DOCK_CENTRE
        return self


# ---------------------------------------------------------------------------- #

class AuiDockUIPart(object):
    """ A class which holds attributes for a UI part in the interface. """
    
    typeCaption = 0
    typeGripper = 1
    typeDock = 2
    typeDockSizer = 3
    typePane = 4
    typePaneSizer = 5
    typeBackground = 6
    typePaneBorder = 7
    typePaneButton = 8

    def __init__(self):
        """
        Default class constructor.
        Used internally, do not call it in your code!
        """
        
        self.orientation = wx.VERTICAL
        self.type = 0
        self.rect = wx.Rect()


# ---------------------------------------------------------------------------- #

class AuiPaneButton(object):
    """ A simple class which describes the caption pane button attributes. """

    def __init__(self, button_id):
        """
        Default class constructor.
        Used internally, do not call it in your code!

        :param `button_id`: the pane button identifier.
        """

        self.button_id = button_id


# ---------------------------------------------------------------------------- #

# event declarations/classes

class AuiManagerEvent(wx.PyCommandEvent):
    """ A specialized command event class for events sent by L{AuiManager}. """

    def __init__(self, eventType, id=1):
        """
        Default class constructor.

        :param `eventType`: the event kind;
        :param `id`: the event identification number.
        """

        wx.PyCommandEvent.__init__(self, eventType, id)

        self.manager = None
        self.pane = None
        self.button = 0
        self.veto_flag = False
        self.canveto_flag = True
        self.dc = None


    def SetManager(self, mgr):
        """
        Associates a L{AuiManager} to the current event.

        :param `mgr`: an instance of L{AuiManager}.
        """

        self.manager = mgr


    def SetDC(self, pdc):
        """
        Associates a `wx.DC` device context to this event.

        :param `pdc`: a `wx.DC` device context object. 
        """

        self.dc = pdc


    def SetPane(self, p):
        """
        Associates a L{AuiPaneInfo} instance to this event.

        :param `p`: a L{AuiPaneInfo} instance.
        """
        
        self.pane = p

        
    def SetButton(self, b):
        """
        Associates a L{AuiPaneButton} instance to this event.

        :param `b`: a L{AuiPaneButton} instance.
        """
        
        self.button = b

        
    def GetManager(self):
        """ Returns the associated L{AuiManager} (if any). """

        return self.manager


    def GetDC(self):
        """ Returns the associated `wx.DC` device context (if any). """

        return self.dc
    

    def GetPane(self):
        """ Returns the associated L{AuiPaneInfo} structure (if any). """
        
        return self.pane


    def GetButton(self):
        """ Returns the associated L{AuiPaneButton} instance (if any). """

        return self.button


    def Veto(self, veto=True):
        """
        Prevents the change announced by this event from happening.

        It is in general a good idea to notify the user about the reasons for
        vetoing the change because otherwise the applications behaviour (which
        just refuses to do what the user wants) might be quite surprising.

        :param `veto`: ``True`` to veto the event, ``False`` otherwise.
        """

        self.veto_flag = veto

        
    def GetVeto(self):
        """ Returns whether the event has been vetoed or not. """

        return self.veto_flag

    
    def SetCanVeto(self, can_veto):
        """
        Sets whether the event can be vetoed or not.

        :param `can_veto`: a bool flag. ``True`` if the event can be vetoed, ``False`` otherwise.
        """

        self.canveto_flag = can_veto

        
    def CanVeto(self):
        """ Returns whether the event can be vetoed and has been vetoed. """

        return  self.canveto_flag and self.veto_flag


# ---------------------------------------------------------------------------- #

class AuiPaneInfo(object):
    """
    AuiPaneInfo specifies all the parameters for a pane. These parameters specify where
    the pane is on the screen, whether it is docked or floating, or hidden. In addition,
    these parameters specify the pane's docked position, floating position, preferred
    size, minimum size, caption text among many other parameters.
    """
    
    optionFloating         = 2**0
    optionHidden           = 2**1
    optionLeftDockable     = 2**2
    optionRightDockable    = 2**3
    optionTopDockable      = 2**4
    optionBottomDockable   = 2**5
    optionFloatable        = 2**6
    optionMovable          = 2**7
    optionResizable        = 2**8
    optionPaneBorder       = 2**9
    optionCaption          = 2**10
    optionGripper          = 2**11
    optionDestroyOnClose   = 2**12
    optionToolbar          = 2**13
    optionActive           = 2**14
    optionGripperTop       = 2**15
    optionMaximized        = 2**16
    optionDockFixed        = 2**17
    optionNotebookDockable = 2**18
    optionMinimized        = 2**19
    optionLeftSnapped      = 2**20
    optionRightSnapped     = 2**21
    optionTopSnapped       = 2**22
    optionBottomSnapped    = 2**23
    optionFlyOut           = 2**24
    optionCaptionLeft      = 2**25

    buttonClose            = 2**26
    buttonMaximize         = 2**27
    buttonMinimize         = 2**28
    buttonPin              = 2**29
    
    buttonCustom1          = 2**30
    buttonCustom2          = 2**31
    buttonCustom3          = 2**32

    savedHiddenState       = 2**33    # used internally
    actionPane             = 2**34    # used internally
    wasMaximized           = 2**35    # used internally
    needsRestore           = 2**36    # used internally


    def __init__(self):
        """ Default class constructor. """
        
        self.window = None
        self.frame = None
        self.state = 0
        self.dock_direction = AUI_DOCK_LEFT
        self.dock_layer = 0
        self.dock_row = 0
        self.dock_pos = 0
        self.minimize_mode = AUI_MINIMIZE_POS_SMART
        self.floating_pos = wx.Point(-1, -1)
        self.floating_size = wx.Size(-1, -1)
        self.best_size = wx.Size(-1, -1)
        self.min_size = wx.Size(-1, -1)
        self.max_size = wx.Size(-1, -1)
        self.dock_proportion = 0
        self.caption = ""
        self.buttons = []
        self.name = ""
        self.icon = wx.NullIcon
        self.rect = wx.Rect()
        self.notebook_id = -1
        self.transparent = 255
        self.needsTransparency = False
        self.previousDockPos = None
        self.previousDockSize = 0
        self.snapped = 0
        
        self.DefaultPane()
        
    
    def dock_direction_get(self):
        """
        Getter for the `dock_direction`.

        :see: L{dock_direction_set} for a set of valid docking directions.
        """
        
        if self.IsMaximized():
            return AUI_DOCK_CENTER
        else:
            return self._dock_direction


    def dock_direction_set(self, value):
        """
        Setter for the `dock_direction`.

        :param `value`: the docking direction. This cab ne one of the following bits:

        ============================ ======= =============================================
        Dock Flag                     Value  Description
        ============================ ======= =============================================
        ``AUI_DOCK_NONE``                  0 No docking direction.
        ``AUI_DOCK_TOP``                   1 Top docking direction.
        ``AUI_DOCK_RIGHT``                 2 Right docking direction.
        ``AUI_DOCK_BOTTOM``                3 Bottom docking direction.
        ``AUI_DOCK_LEFT``                  4 Left docking direction.
        ``AUI_DOCK_CENTER``                5 Center docking direction.
        ``AUI_DOCK_CENTRE``                5 Centre docking direction.
        ``AUI_DOCK_NOTEBOOK_PAGE``         6 Automatic AuiNotebooks docking style.
        ============================ ======= =============================================
        
        """
        
        self._dock_direction = value
        
    dock_direction = property(dock_direction_get, dock_direction_set)

    def IsOk(self):
        """
        Returns ``True`` if the L{AuiPaneInfo} structure is valid.

        :note: A pane structure is valid if it has an associated window.
        """
        
        return self.window != None


    def IsMaximized(self):
        """ Returns ``True`` if the pane is maximized. """
        
        return self.HasFlag(self.optionMaximized)


    def IsMinimized(self):
        """ Returns ``True`` if the pane is minimized. """

        return self.HasFlag(self.optionMinimized)


    def IsFixed(self):
        """ Returns ``True`` if the pane cannot be resized. """
        
        return not self.HasFlag(self.optionResizable)

    
    def IsResizeable(self):
        """ Returns ``True`` if the pane can be resized. """
        
        return self.HasFlag(self.optionResizable)

    
    def IsShown(self):
        """ Returns ``True`` if the pane is currently shown. """
        
        return not self.HasFlag(self.optionHidden)

    
    def IsFloating(self):
        """ Returns ``True`` if the pane is floating. """

        return self.HasFlag(self.optionFloating)

    
    def IsDocked(self):
        """ Returns ``True`` if the pane is docked. """
        
        return not self.HasFlag(self.optionFloating)

    
    def IsToolbar(self):
        """ Returns ``True`` if the pane contains a toolbar. """

        return self.HasFlag(self.optionToolbar)

    
    def IsTopDockable(self):
        """
        Returns ``True`` if the pane can be docked at the top
        of the managed frame.
        """
        
        return self.HasFlag(self.optionTopDockable)

    
    def IsBottomDockable(self):
        """
        Returns ``True`` if the pane can be docked at the bottom
        of the managed frame.
        """
        
        return self.HasFlag(self.optionBottomDockable)

    
    def IsLeftDockable(self):
        """
        Returns ``True`` if the pane can be docked at the left
        of the managed frame.
        """
        
        return self.HasFlag(self.optionLeftDockable) 


    def IsRightDockable(self):
        """
        Returns ``True`` if the pane can be docked at the right
        of the managed frame.
        """
        
        return self.HasFlag(self.optionRightDockable)


    def IsDockable(self):
        """ Returns ``True`` if the pane can be docked. """
        
        return self.IsTopDockable() or self.IsBottomDockable() or self.IsLeftDockable() or \
               self.IsRightDockable() or self.IsNotebookDockable()
    
    
    def IsFloatable(self):
        """
        Returns ``True`` if the pane can be undocked and displayed as a
        floating window.
        """

        return self.HasFlag(self.optionFloatable)

    
    def IsMovable(self):
        """
        Returns ``True`` if the docked frame can be undocked or moved to
        another dock position.
        """
        
        return self.HasFlag(self.optionMovable)


    def IsDestroyOnClose(self):
        """
        Returns ``True`` if the pane should be destroyed when it is closed.
        
        Normally a pane is simply hidden when the close button is clicked. Calling L{DestroyOnClose}
        with a ``True`` input parameter will cause the window to be destroyed when the user clicks
        the pane's close button.
        """
        
        return self.HasFlag(self.optionDestroyOnClose)
    

    def IsNotebookDockable(self):
        """
        Returns ``True`` if a pane can be docked on top to another to create a
        L{AuiNotebook}.
        """

        return self.HasFlag(self.optionNotebookDockable)
    

    def IsTopSnappable(self):
        """ Returns ``True`` if the pane can be snapped at the top of the managed frame. """
        
        return self.HasFlag(self.optionTopSnapped)

    
    def IsBottomSnappable(self):
        """ Returns ``True`` if the pane can be snapped at the bottom of the managed frame. """
        
        return self.HasFlag(self.optionBottomSnapped)

    
    def IsLeftSnappable(self):
        """ Returns ``True`` if the pane can be snapped on the left of the managed frame. """
        
        return self.HasFlag(self.optionLeftSnapped) 


    def IsRightSnappable(self):
        """ Returns ``True`` if the pane can be snapped on the right of the managed frame. """
        
        return self.HasFlag(self.optionRightSnapped)


    def IsSnappable(self):
        """ Returns ``True`` if the pane can be snapped. """
        
        return self.IsTopSnappable() or self.IsBottomSnappable() or self.IsLeftSnappable() or \
               self.IsRightSnappable()


    def IsFlyOut(self):
        """ Returns ``True`` if the floating pane has a "fly-out" effect. """

        return self.HasFlag(self.optionFlyOut)        
            

    def HasCaption(self):
        """ Returns ``True`` if the pane displays a caption. """
        
        return self.HasFlag(self.optionCaption)

    
    def HasCaptionLeft(self):
        """ Returns ``True`` if the pane displays a caption on the left (rotated by 90 degrees). """
        
        return self.HasFlag(self.optionCaptionLeft)


    def HasGripper(self):
        """ Returns ``True`` if the pane displays a gripper. """
        
        return self.HasFlag(self.optionGripper) 


    def HasBorder(self):
        """ Returns ``True`` if the pane displays a border. """
        
        return self.HasFlag(self.optionPaneBorder)

    
    def HasCloseButton(self):
        """ Returns ``True`` if the pane displays a button to close the pane. """

        return self.HasFlag(self.buttonClose) 


    def HasMaximizeButton(self):
        """ Returns ``True`` if the pane displays a button to maximize the pane. """
        
        return self.HasFlag(self.buttonMaximize)

    
    def HasMinimizeButton(self):
        """ Returns ``True`` if the pane displays a button to minimize the pane. """
        
        return self.HasFlag(self.buttonMinimize) 


    def GetMinimizeMode(self):
        """
        Returns the minimization style for this pane.

        Possible return values are:

        ============================== ========= ==============================
        Minimize Mode Flag             Hex Value Description
        ============================== ========= ==============================
        ``AUI_MINIMIZE_POS_SMART``          0x01 Minimizes the pane on the closest tool bar
        ``AUI_MINIMIZE_POS_TOP``            0x02 Minimizes the pane on the top tool bar
        ``AUI_MINIMIZE_POS_LEFT``           0x03 Minimizes the pane on its left tool bar
        ``AUI_MINIMIZE_POS_RIGHT``          0x04 Minimizes the pane on its right tool bar
        ``AUI_MINIMIZE_POS_BOTTOM``         0x05 Minimizes the pane on its bottom tool bar
        ``AUI_MINIMIZE_POS_MASK``           0x07 Mask to filter the position flags
        ``AUI_MINIMIZE_CAPT_HIDE``           0x0 Hides the caption of the minimized pane
        ``AUI_MINIMIZE_CAPT_SMART``         0x08 Displays the caption in the best rotation (horizontal or clockwise)
        ``AUI_MINIMIZE_CAPT_HORZ``          0x10 Displays the caption horizontally
        ``AUI_MINIMIZE_CAPT_MASK``          0x18 Mask to filter the caption flags
        ============================== ========= ==============================

        The flags can be filtered with the following masks:

        ============================== ========= ==============================
        Minimize Mask Flag             Hex Value Description
        ============================== ========= ==============================        
        ``AUI_MINIMIZE_POS_MASK``           0x07 Filters the position flags
        ``AUI_MINIMIZE_CAPT_MASK``          0x18 Filters the caption flags
        ============================== ========= ==============================

        """
        
        return self.minimize_mode
    

    def HasPinButton(self):
        """ Returns ``True`` if the pane displays a button to float the pane. """
        
        return self.HasFlag(self.buttonPin) 


    def HasGripperTop(self):
        """ Returns ``True`` if the pane displays a gripper at the top. """

        return self.HasFlag(self.optionGripperTop)


    def Window(self, w):
        """
        Associate a `wx.Window` derived window to this pane.

        This normally does not need to be specified, as the window pointer is
        automatically assigned to the L{AuiPaneInfo} structure as soon as it is
        added to the manager.

        :param `w`: a `wx.Window` derived window.
        """

        self.window = w
        return self

    
    def Name(self, name):
        """
        Sets the name of the pane so it can be referenced in lookup functions.

        If a name is not specified by the user, a random name is assigned to the pane
        when it is added to the manager.

        :param `name`: a string specifying the pane name.

        :warning: If you are using L{AuiManager.SavePerspective} and L{AuiManager.LoadPerspective}, you will have
         to specify a name for your pane using L{Name}, as randomly generated names can
         not be properly restored.
        """

        self.name = name
        return self

    
    def Caption(self, caption):
        """
        Sets the caption of the pane.

        :param `caption`: a string specifying the pane caption.
        """
        
        self.caption = caption
        return self

    
    def Left(self):
        """ 
        Sets the pane dock position to the left side of the frame.

        :note: This is the same thing as calling L{Direction} with ``AUI_DOCK_LEFT`` as
         parameter.
        """
        
        self.dock_direction = AUI_DOCK_LEFT
        return self

    
    def Right(self):
        """
        Sets the pane dock position to the right side of the frame.

        :note: This is the same thing as calling L{Direction} with ``AUI_DOCK_RIGHT`` as
         parameter.
        """
        
        self.dock_direction = AUI_DOCK_RIGHT
        return self

    
    def Top(self):
        """
        Sets the pane dock position to the top of the frame.

        :note: This is the same thing as calling L{Direction} with ``AUI_DOCK_TOP`` as
         parameter.
        """

        self.dock_direction = AUI_DOCK_TOP
        return self

    
    def Bottom(self):
        """
        Sets the pane dock position to the bottom of the frame.

        :note: This is the same thing as calling L{Direction} with ``AUI_DOCK_BOTTOM`` as
         parameter.
        """

        self.dock_direction = AUI_DOCK_BOTTOM
        return self

    
    def Center(self):
        """
        Sets the pane to the center position of the frame.

        The centre pane is the space in the middle after all border panes (left, top,
        right, bottom) are subtracted from the layout.

        :note: This is the same thing as calling L{Direction} with ``AUI_DOCK_CENTER`` as
         parameter.
        """
        
        self.dock_direction = AUI_DOCK_CENTER
        return self

        
    def Centre(self):
        """
        Sets the pane to the center position of the frame.

        The centre pane is the space in the middle after all border panes (left, top,
        right, bottom) are subtracted from the layout.

        :note: This is the same thing as calling L{Direction} with ``AUI_DOCK_CENTRE`` as
         parameter.
        """
        
        self.dock_direction = AUI_DOCK_CENTRE
        return self

    
    def Direction(self, direction):
        """
        Determines the direction of the docked pane. It is functionally the
        same as calling L{Left}, L{Right}, L{Top} or L{Bottom}, except that docking direction
        may be specified programmatically via the parameter `direction`.

        :param `direction`: the direction of the docked pane.

        :see: L{dock_direction_set} for a list of valid docking directions.        
        """
        
        self.dock_direction = direction
        return self

    
    def Layer(self, layer):
        """
        Determines the layer of the docked pane.

        The dock layer is similar to an onion, the inner-most layer being layer 0. Each
        shell moving in the outward direction has a higher layer number. This allows for
        more complex docking layout formation.

        :param `layer`: the layer of the docked pane.
        """
        
        self.dock_layer = layer
        return self

    
    def Row(self, row):
        """
        Determines the row of the docked pane.

        :param `row`: the row of the docked pane.
        """
        
        self.dock_row = row
        return self

    
    def Position(self, pos):
        """
        Determines the position of the docked pane.

        :param `pos`: the position of the docked pane.
        """

        self.dock_pos = pos
        return self


    def MinSize(self, arg1=None, arg2=None):
        """
        Sets the minimum size of the pane.

        This method is split in 2 versions depending on the input type. If `arg1` is
        a `wx.Size` object, then L{MinSize1} is called. Otherwise, L{MinSize2} is called.

        :param `arg1`: a `wx.Size` object, a (x, y) tuple or or a `x` coordinate.
        :param `arg2`: a `y` coordinate (only if `arg1` is a `x` coordinate, otherwise unused).
        """
        
        if isinstance(arg1, wx.Size):
            ret = self.MinSize1(arg1)
        elif isinstance(arg1, types.TupleType):
            ret = self.MinSize1(wx.Size(*arg1))
        else:
            ret = self.MinSize2(arg1, arg2)

        return ret

    
    def MinSize1(self, size):
        """
        Sets the minimum size of the pane.

        :see: L{MinSize} for an explanation of input parameters.
        """
        self.min_size = size
        return self


    def MinSize2(self, x, y):
        """
        Sets the minimum size of the pane.

        :see: L{MinSize} for an explanation of input parameters.
        """

        self.min_size = wx.Size(x, y)
        return self


    def MaxSize(self, arg1=None, arg2=None):
        """
        Sets the maximum size of the pane.

        This method is split in 2 versions depending on the input type. If `arg1` is
        a `wx.Size` object, then L{MaxSize1} is called. Otherwise, L{MaxSize2} is called.

        :param `arg1`: a `wx.Size` object, a (x, y) tuple or a `x` coordinate.
        :param `arg2`: a `y` coordinate (only if `arg1` is a `x` coordinate, otherwise unused).
        """
        
        if isinstance(arg1, wx.Size):
            ret = self.MaxSize1(arg1)
        elif isinstance(arg1, types.TupleType):
            ret = self.MaxSize1(wx.Size(*arg1))
        else:
            ret = self.MaxSize2(arg1, arg2)

        return ret
    
    
    def MaxSize1(self, size):
        """
        Sets the maximum size of the pane.

        :see: L{MaxSize} for an explanation of input parameters.
        """

        self.max_size = size
        return self


    def MaxSize2(self, x, y):
        """
        Sets the maximum size of the pane.

        :see: L{MaxSize} for an explanation of input parameters.
        """

        self.max_size.Set(x,y)
        return self


    def BestSize(self, arg1=None, arg2=None):
        """
        Sets the ideal size for the pane. The docking manager will attempt to use
        this size as much as possible when docking or floating the pane.

        This method is split in 2 versions depending on the input type. If `arg1` is
        a `wx.Size` object, then L{BestSize1} is called. Otherwise, L{BestSize2} is called.

        :param `arg1`: a `wx.Size` object, a (x, y) tuple or a `x` coordinate.
        :param `arg2`: a `y` coordinate (only if `arg1` is a `x` coordinate, otherwise unused).
        """
        
        if isinstance(arg1, wx.Size):
            ret = self.BestSize1(arg1)
        elif isinstance(arg1, types.TupleType):
            ret = self.BestSize1(wx.Size(*arg1))
        else:
            ret = self.BestSize2(arg1, arg2)

        return ret
    
            
    def BestSize1(self, size):
        """
        Sets the best size of the pane.

        :see: L{BestSize} for an explanation of input parameters.
        """

        self.best_size = size
        return self

    
    def BestSize2(self, x, y):
        """
        Sets the best size of the pane.

        :see: L{BestSize} for an explanation of input parameters.
        """

        self.best_size.Set(x,y)
        return self
    
    
    def FloatingPosition(self, pos):
        """
        Sets the position of the floating pane.

        :param `pos`: a `wx.Point` or a tuple indicating the pane floating position.
        """
        
        self.floating_pos = wx.Point(*pos)
        return self

    
    def FloatingSize(self, size):
        """
        Sets the size of the floating pane.

        :param `size`: a `wx.Size` or a tuple indicating the pane floating size.
        """
        
        self.floating_size = wx.Size(*size)
        return self


    def Maximize(self):
        """ Makes the pane take up the full area."""

        return self.SetFlag(self.optionMaximized, True)


    def Minimize(self):
        """
        Makes the pane minimized in a L{AuiToolBar}.

        Clicking on the minimize button causes a new L{AuiToolBar} to be created
        and added to the frame manager, (currently the implementation is such that
        panes at West will have a toolbar at the right, panes at South will have
        toolbars at the bottom etc...) and the pane is hidden in the manager.
        
        Clicking on the restore button on the newly created toolbar will result in the
        toolbar being removed and the original pane being restored.
        """
        
        return self.SetFlag(self.optionMinimized, True)


    def MinimizeMode(self, mode):
        """
        Sets the expected minimized mode if the MinimizeButton() is visible.

        The minimized pane can have a specific position in the work space:

        ============================== ========= ==============================
        Minimize Mode Flag             Hex Value Description
        ============================== ========= ==============================
        ``AUI_MINIMIZE_POS_SMART``          0x01 Minimizes the pane on the closest tool bar
        ``AUI_MINIMIZE_POS_TOP``            0x02 Minimizes the pane on the top tool bar
        ``AUI_MINIMIZE_POS_LEFT``           0x03 Minimizes the pane on its left tool bar
        ``AUI_MINIMIZE_POS_RIGHT``          0x04 Minimizes the pane on its right tool bar
        ``AUI_MINIMIZE_POS_BOTTOM``         0x05 Minimizes the pane on its bottom tool bar
        ============================== ========= ==============================

        The caption of the minimized pane can be displayed in different modes:

        ============================== ========= ==============================
        Caption Mode Flag              Hex Value Description
        ============================== ========= ==============================
        ``AUI_MINIMIZE_CAPT_HIDE``           0x0 Hides the caption of the minimized pane
        ``AUI_MINIMIZE_CAPT_SMART``         0x08 Displays the caption in the best rotation (horizontal in the top and in the bottom tool bar or clockwise in the right and in the left tool bar)
        ``AUI_MINIMIZE_CAPT_HORZ``          0x10 Displays the caption horizontally
        ============================== ========= ==============================
        
        """
        
        self.minimize_mode = mode
        return self
    

    def Restore(self):
        """ Is the reverse of L{Maximize} and L{Minimize}."""
        
        return self.SetFlag(self.optionMaximized or self.optionMinimized, False)

    
    def Fixed(self):
        """
        Forces a pane to be fixed size so that it cannot be resized.
        After calling L{Fixed}, L{IsFixed} will return ``True``.
        """
        
        return self.SetFlag(self.optionResizable, False)

    
    def Resizable(self, resizable=True):
        """
        Allows a pane to be resizable if `resizable` is ``True``, and forces
        it to be a fixed size if `resizeable` is ``False``.

        If `resizable` is ``False``, this is simply an antonym for L{Fixed}.

        :param `resizable`: whether the pane will be resizeable or not.
        """
        
        return self.SetFlag(self.optionResizable, resizable)


    def Transparent(self, alpha):
        """
        Makes the pane transparent when floating.

        :param `alpha`: an integer value between 0 and 255 for pane transparency.
        """

        if alpha < 0 or alpha > 255:
            raise Exception("Invalid transparency value (%s)"%repr(alpha))
                            
        self.transparent = alpha
        self.needsTransparency = True

    
    def Dock(self):
        """
        Indicates that a pane should be docked. It is the opposite of L{Float}.
        """

        if self.IsNotebookPage():
            self.notebook_id = -1
            self.dock_direction = AUI_DOCK_NONE
        
        return self.SetFlag(self.optionFloating, False)

    
    def Float(self):
        """
        Indicates that a pane should be floated. It is the opposite of L{Dock}.
        """

        if self.IsNotebookPage():
            self.notebook_id = -1
            self.dock_direction = AUI_DOCK_NONE

        return self.SetFlag(self.optionFloating, True)

    
    def Hide(self):
        """
        Indicates that a pane should be hidden.

        Calling L{Show} (``False``) achieve the same effect.
        """
        
        return self.SetFlag(self.optionHidden, True)

    
    def Show(self, show=True):
        """
        Indicates that a pane should be shown.

        :param `show`: whether the pane should be shown or not.
        """
        
        return self.SetFlag(self.optionHidden, not show)
    

    # By defaulting to 1000, the tab will get placed at the end
    def NotebookPage(self, id, tab_position=1000):
        """
        Forces a pane to be a notebook page, so that the pane can be
        docked on top to another to create a L{AuiNotebook}.

        :param `id`: the notebook id;
        :param `tab_position`: the tab number of the pane once docked in a notebook.
        """
        
        # Remove any floating frame
        self.Dock()
        self.notebook_id = id
        self.dock_pos = tab_position
        self.dock_row = 0
        self.dock_layer = 0
        self.dock_direction = AUI_DOCK_NOTEBOOK_PAGE

        return self


    def NotebookControl(self, id):
        """
        Forces a pane to be a notebook control (L{AuiNotebook}).

        :param `id`: the notebook id.
        """

        self.notebook_id = id
        self.window = None
        self.buttons = []
        
        if self.dock_direction == AUI_DOCK_NOTEBOOK_PAGE:
            self.dock_direction = AUI_DOCK_NONE
            
        return self
    

    def HasNotebook(self):
        """ Returns whether a pane has a L{AuiNotebook} or not. """

        return self.notebook_id >= 0


    def IsNotebookPage(self):
        """ Returns whether the pane is a notebook page in a L{AuiNotebook}. """

        return self.notebook_id >= 0 and self.dock_direction == AUI_DOCK_NOTEBOOK_PAGE


    def IsNotebookControl(self):
        """ Returns whether the pane is a notebook control (L{AuiNotebook}). """

        return not self.IsNotebookPage() and self.HasNotebook()


    def SetNameFromNotebookId(self):
        """ Sets the pane name once docked in a L{AuiNotebook} using the notebook id. """

        if self.notebook_id >= 0:
            self.name = "__notebook_%d"%self.notebook_id
            
        return self


    def CaptionVisible(self, visible=True, left=False):
        """
        Indicates that a pane caption should be visible. If `visible` is ``False``, no pane
        caption is drawn.

        :param `visible`: whether the caption should be visible or not;
        :param `left`: whether the caption should be drawn on the left (rotated by 90 degrees) or not.
        """

        if left:
            self.SetFlag(self.optionCaption, False)
            return self.SetFlag(self.optionCaptionLeft, visible)

        self.SetFlag(self.optionCaptionLeft, False)
        return self.SetFlag(self.optionCaption, visible)

    
    def PaneBorder(self, visible=True):
        """
        Indicates that a border should be drawn for the pane.

        :param `visible`: whether the pane border should be visible or not.
        """
        
        return self.SetFlag(self.optionPaneBorder, visible)

    
    def Gripper(self, visible=True):
        """
        Indicates that a gripper should be drawn for the pane.

        :param `visible`: whether the gripper should be visible or not.
        """
        
        return self.SetFlag(self.optionGripper, visible)


    def GripperTop(self, attop=True):
        """
        Indicates that a gripper should be drawn at the top of the pane.

        :param `attop`: whether the gripper should be drawn at the top or not.
        """
        
        return self.SetFlag(self.optionGripperTop, attop)

    
    def CloseButton(self, visible=True):
        """
        Indicates that a close button should be drawn for the pane.

        :param `visible`: whether the close button should be visible or not.
        """
        
        return self.SetFlag(self.buttonClose, visible)

    
    def MaximizeButton(self, visible=True):
        """
        Indicates that a maximize button should be drawn for the pane.

        :param `visible`: whether the maximize button should be visible or not.
        """
        
        return self.SetFlag(self.buttonMaximize, visible)

    
    def MinimizeButton(self, visible=True):
        """
        Indicates that a minimize button should be drawn for the pane.

        :param `visible`: whether the minimize button should be visible or not.
        """

        return self.SetFlag(self.buttonMinimize, visible)

    
    def PinButton(self, visible=True):
        """
        Indicates that a pin button should be drawn for the pane.

        :param `visible`: whether the pin button should be visible or not.
        """
        
        return self.SetFlag(self.buttonPin, visible)

    
    def DestroyOnClose(self, b=True):
        """
        Indicates whether a pane should be destroyed when it is closed.

        Normally a pane is simply hidden when the close button is clicked. Setting
        `b` to ``True`` will cause the window to be destroyed when the user clicks
        the pane's close button.

        :param `b`: whether the pane should be destroyed when it is closed or not.
        """
        
        return self.SetFlag(self.optionDestroyOnClose, b)

    
    def TopDockable(self, b=True):
        """
        Indicates whether a pane can be docked at the top of the frame.

        :param `b`: whether the pane can be docked at the top or not.
        """
        
        return self.SetFlag(self.optionTopDockable, b)

    
    def BottomDockable(self, b=True):
        """
        Indicates whether a pane can be docked at the bottom of the frame.

        :param `b`: whether the pane can be docked at the bottom or not.
        """
        
        return self.SetFlag(self.optionBottomDockable, b)

    
    def LeftDockable(self, b=True):
        """
        Indicates whether a pane can be docked on the left of the frame.

        :param `b`: whether the pane can be docked at the left or not.
        """

        return self.SetFlag(self.optionLeftDockable, b)

    
    def RightDockable(self, b=True):
        """
        Indicates whether a pane can be docked on the right of the frame.

        :param `b`: whether the pane can be docked at the right or not.
        """
        
        return self.SetFlag(self.optionRightDockable, b)

    
    def Floatable(self, b=True):
        """
        Sets whether the user will be able to undock a pane and turn it
        into a floating window.

        :param `b`: whether the pane can be floated or not.
        """
        
        return self.SetFlag(self.optionFloatable, b)

    
    def Movable(self, b=True):
        """
        Indicates whether a pane can be moved.

        :param `b`: whether the pane can be moved or not.
        """
        
        return self.SetFlag(self.optionMovable, b)


    def NotebookDockable(self, b=True):
        """
        Indicates whether a pane can be docked in an automatic L{AuiNotebook}.

        :param `b`: whether the pane can be docked in a notebook or not.
        """
        
        return self.SetFlag(self.optionNotebookDockable, b)
                                  

    def DockFixed(self, b=True):
        """
        Causes the containing dock to have no resize sash. This is useful
        for creating panes that span the entire width or height of a dock, but should
        not be resizable in the other direction.

        :param `b`: whether the pane will have a resize sash or not.
        """

        return self.SetFlag(self.optionDockFixed, b)

                                   
    def Dockable(self, b=True):
        """
        Specifies whether a frame can be docked or not. It is the same as specifying
        L{TopDockable} . L{BottomDockable} . L{LeftDockable} . L{RightDockable} .

        :param `b`: whether the frame can be docked or not.
        """

        return self.TopDockable(b).BottomDockable(b).LeftDockable(b).RightDockable(b)
    

    def TopSnappable(self, b=True):
        """
        Indicates whether a pane can be snapped at the top of the main frame.

        :param `b`: whether the pane can be snapped at the top of the main frame or not.
        """
        
        return self.SetFlag(self.optionTopSnapped, b)

    
    def BottomSnappable(self, b=True):
        """
        Indicates whether a pane can be snapped at the bottom of the main frame.

        :param `b`: whether the pane can be snapped at the bottom of the main frame or not.
        """
        
        return self.SetFlag(self.optionBottomSnapped, b)

    
    def LeftSnappable(self, b=True):
        """
        Indicates whether a pane can be snapped on the left of the main frame.

        :param `b`: whether the pane can be snapped at the left of the main frame or not.
        """

        return self.SetFlag(self.optionLeftSnapped, b)

    
    def RightSnappable(self, b=True):
        """
        Indicates whether a pane can be snapped on the right of the main frame.

        :param `b`: whether the pane can be snapped at the right of the main frame or not.
        """
        
        return self.SetFlag(self.optionRightSnapped, b)


    def Snappable(self, b=True):
        """
        Indicates whether a pane can be snapped on the main frame. This is
        equivalent as calling L{TopSnappable} . L{BottomSnappable} . L{LeftSnappable} . L{RightSnappable} .

        :param `b`: whether the pane can be snapped on the main frame or not.
        """
    
        return self.TopSnappable(b).BottomSnappable(b).LeftSnappable(b).RightSnappable(b)


    def FlyOut(self, b=True):
        """
        Indicates whether a pane, when floating, has a "fly-out" effect
        (i.e., floating panes which only show themselves when moused over).

        :param `b`: whether the pane can be snapped on the main frame or not.
        """

        return self.SetFlag(self.optionFlyOut, b)
    
    
    # Copy over the members that pertain to docking position
    def SetDockPos(self, source):
        """
        Copies the `source` pane members that pertain to docking position to `self`.

        :param `source`: the source pane from where to copy the attributes.
        """
        
        self.dock_direction = source.dock_direction
        self.dock_layer = source.dock_layer
        self.dock_row = source.dock_row
        self.dock_pos = source.dock_pos
        self.dock_proportion = source.dock_proportion
        self.floating_pos = wx.Point(*source.floating_pos)
        self.floating_size = wx.Size(*source.floating_size)
        self.rect = wx.Rect(*source.rect)
        
        return self


    def DefaultPane(self):
        """ Specifies that the pane should adopt the default pane settings. """
        
        state = self.state    
        state |= self.optionTopDockable | self.optionBottomDockable | \
                 self.optionLeftDockable | self.optionRightDockable | \
                 self.optionNotebookDockable | \
                 self.optionFloatable | self.optionMovable | self.optionResizable | \
                 self.optionCaption | self.optionPaneBorder | self.buttonClose

        self.state = state
        
        return self
    
    
    def CentrePane(self):
        """
        Specifies that the pane should adopt the default center pane settings.

        Centre panes usually do not have caption bars. This function provides an easy way of
        preparing a pane to be displayed in the center dock position.
        """
        
        return self.CenterPane()

    
    def CenterPane(self):
        """
        Specifies that the pane should adopt the default center pane settings.

        Centre panes usually do not have caption bars. This function provides an easy way of
        preparing a pane to be displayed in the center dock position.
        """
        
        self.state = 0
        return self.Center().PaneBorder().Resizable()
    
     
    def ToolbarPane(self):
        """ Specifies that the pane should adopt the default toolbar pane settings. """
        
        self.DefaultPane()
        state = self.state
        
        state |= (self.optionToolbar | self.optionGripper)
        state &= ~(self.optionResizable | self.optionCaption | self.optionCaptionLeft)
        
        if self.dock_layer == 0:
            self.dock_layer = 10

        self.state = state
        
        return self


    def Icon(self, icon):
        """
        Specifies whether an icon is drawn on the left of the caption text when
        the pane is docked. If `icon` is ``None`` or `wx.NullIcon`, no icon is drawn on
        the caption space.

        :param icon: an icon to draw on the caption space, or ``None``.
        """

        if icon is None:
            icon = wx.NullIcon
            
        self.icon = icon
        return self        
    

    def SetFlag(self, flag, option_state):
        """
        Turns the property given by `flag` on or off with the `option_state`
        parameter.

        :param `flag`: the property to set;
        :param `option_state`: either ``True`` or ``False``.
        """
        
        state = self.state
        
        if option_state:
            state |= flag
        else:
            state &= ~flag

        self.state = state

        if flag in [self.buttonClose, self.buttonMaximize, self.buttonMinimize, self.buttonPin]:
            self.ResetButtons()
            
        return self
    
    
    def HasFlag(self, flag):
        """
        Returns ``True`` if the the property specified by flag is active for the pane.

        :param `flag`: the property to check for activity.
        """
        
        return (self.state & flag and [True] or [False])[0]


    def ResetButtons(self):
        """
        Resets all the buttons and recreates them from scratch depending on the
        L{AuiPaneInfo} flags.
        """

        floating = self.HasFlag(self.optionFloating)
        self.buttons = []

        if not floating and self.HasMinimizeButton():
            button = AuiPaneButton(AUI_BUTTON_MINIMIZE)
            self.buttons.append(button)
    
        if not floating and self.HasMaximizeButton():
            button = AuiPaneButton(AUI_BUTTON_MAXIMIZE_RESTORE)
            self.buttons.append(button)

        if not floating and self.HasPinButton():
            button = AuiPaneButton(AUI_BUTTON_PIN)
            self.buttons.append(button)

        if self.HasCloseButton():
            button = AuiPaneButton(AUI_BUTTON_CLOSE)
            self.buttons.append(button)
        

    def CountButtons(self):
        """ Returns the number of visible buttons in the docked pane. """

        n = 0
        
        if self.HasCaption() or self.HasCaptionLeft():
            if isinstance(wx.GetTopLevelParent(self.window), AuiFloatingFrame):
                return 1
            
            if self.HasCloseButton():
                n += 1
            if self.HasMaximizeButton():
                n += 1
            if self.HasMinimizeButton():
                n += 1
            if self.HasPinButton():
                n += 1

        return n
    

    def IsHorizontal(self):
        """ Returns ``True`` if the pane `dock_direction` is horizontal. """

        return self.dock_direction in [AUI_DOCK_TOP, AUI_DOCK_BOTTOM]

    def IsVertical(self):
        """ Returns ``True`` if the pane `dock_direction` is vertical. """

        return self.dock_direction in [AUI_DOCK_LEFT, AUI_DOCK_RIGHT]


# Null AuiPaneInfo reference
NonePaneInfo = AuiPaneInfo()


# ---------------------------------------------------------------------------- #

class AuiDockingGuide(wx.Frame):
    """ Base class for L{AuiCenterDockingGuide} and L{AuiSingleDockingGuide}."""

    def __init__(self, parent, id=wx.ID_ANY, title="", pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.FRAME_TOOL_WINDOW | wx.STAY_ON_TOP |
                 wx.FRAME_NO_TASKBAR | wx.NO_BORDER, name="AuiDockingGuide"):
        """
        Default class constructor. Used internally, do not call it in your code!

        :param `parent`: the L{AuiDockingGuide} parent;
        :param `id`: the window identifier. It may take a value of -1 to indicate a default value.
        :param `title`: the caption to be displayed on the frame's title bar.
        :param `pos`: the window position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform.
        :param `size`: the window size. A value of (-1, -1) indicates a default size, chosen by
         either the windowing system or wxPython, depending on platform.
        :param `style`: the window style. 
        :param `name`: the name of the window. This parameter is used to associate a name with the
         item, allowing the application user to set Motif resource values for individual windows.
        """

        wx.Frame.__init__(self, parent, id, title, pos, size, style, name=name)


    def HitTest(self, x, y):
        """
        To be overridden by parent classes.

        :param `x`: the `x` mouse position;
        :param `y`: the `y` mouse position.
        """

        return 0

    
    def ValidateNotebookDocking(self, valid):
        """
        To be overridden by parent classes.

        :param `valid`: whether a pane can be docked on top to another to form an automatic
         L{AuiNotebook}.
        """
        
        return 0

# ============================================================================
# implementation
# ============================================================================

# ---------------------------------------------------------------------------
# AuiDockingGuideWindow
# ---------------------------------------------------------------------------

class AuiDockingGuideWindow(wx.Window):
    """ Target class for L{AuiSingleDockingGuide} and L{AuiCenterDockingGuide}. """

    def __init__(self, parent, rect, direction=0, center=False, useAero=False):
        """
        Default class constructor. Used internally, do not call it in your code!

        :param `parent`: the L{AuiDockingGuideWindow} parent;
        :param `rect`: the window rect;
        :param `direction`: one of ``wx.TOP``, ``wx.BOTTOM``, ``wx.LEFT``, ``wx.RIGHT``,
         ``wx.CENTER``;
        :param `center`: whether the calling class is a L{AuiCenterDockingGuide};
        :param `useAero`: whether to use the new Aero-style bitmaps or Whidbey-style bitmaps
         for the docking guide.
        """

        wx.Window.__init__(self, parent, -1, rect.GetPosition(), rect.GetSize(), wx.NO_BORDER)

        self._direction = direction
        self._center = center
        self._valid = True
        self._useAero = useAero
        
        self._bmp_unfocus, self._bmp_focus = GetDockingImage(direction, useAero, center)
        
        self._currentImage = self._bmp_unfocus
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_PAINT, self.OnPaint)


    def SetValid(self, valid):
        """
        Sets the docking direction as valid or invalid.

        :param `valid`: whether the docking direction is allowed or not.
        """

        self._valid = valid


    def IsValid(self):
        """ Returns whether the docking direction is valid. """
        
        return self._valid


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{AuiDockingGuideWindow}.

        :param `event`: a `wx.EraseEvent` to be processed.

        :note: This is intentionally empty to reduce flickering while drawing.
        """

        pass


    def DrawBackground(self, dc):
        """
        Draws the docking guide background.

        :param `dc`: a `wx.DC` device context object.
        """

        rect = self.GetClientRect()

        dc.SetPen(wx.TRANSPARENT_PEN)
        dc.SetBrush(wx.Brush(colourTargetBackground))
        dc.DrawRectangleRect(rect)

        dc.SetPen(wx.Pen(colourTargetBorder))

        left = rect.GetLeft()
        top = rect.GetTop()
        right = rect.GetRight()
        bottom = rect.GetBottom()

        if self._direction != wx.CENTER:
        
            if not self._center or self._direction != wx.BOTTOM:
                dc.DrawLine(left, top, right+1, top)
            if not self._center or self._direction != wx.RIGHT:
                dc.DrawLine(left, top, left, bottom+1)
            if not self._center or self._direction != wx.LEFT:
                dc.DrawLine(right, top, right, bottom+1)
            if not self._center or self._direction != wx.TOP:
                dc.DrawLine(left, bottom, right+1, bottom)

            dc.SetPen(wx.Pen(colourTargetShade))

            if self._direction != wx.RIGHT:
                dc.DrawLine(left + 1, top + 1, left + 1, bottom)
            if self._direction != wx.BOTTOM:
                dc.DrawLine(left + 1, top + 1, right, top + 1)

        
    def DrawDottedLine(self, dc, point, length, vertical):
        """
        Draws a dotted line (not used if the docking guide images are ok).

        :param `dc`: a `wx.DC` device context object;
        :param `point`: a `wx.Point` where to start drawing the dotted line;
        :param `length`: the length of the dotted line;
        :param `vertical`: whether it is a vertical docking guide window or not.
        """

        for i in xrange(0, length, 2):
            dc.DrawPoint(point.x, point.y)
            if vertical:
                point.y += 2
            else:
                point.x += 2
        

    def DrawIcon(self, dc):
        """
        Draws the docking guide icon (not used if the docking guide images are ok).

        :param `dc`: a `wx.DC` device context object.
        """

        rect = wx.Rect(*self.GetClientRect())
        point = wx.Point()
        length = 0

        rect.Deflate(4, 4)
        dc.SetPen(wx.Pen(colourIconBorder))
        dc.SetBrush(wx.Brush(colourIconBackground))
        dc.DrawRectangleRect(rect)

        right1 = rect.GetRight() + 1
        bottom1 = rect.GetBottom() + 1

        dc.SetPen(wx.Pen(colourIconShadow))
        dc.DrawLine(rect.x + 1, bottom1, right1 + 1, bottom1)
        dc.DrawLine(right1, rect.y + 1, right1, bottom1 + 1)

        rect.Deflate(1, 1)

        if self._direction == wx.TOP:
            rect.height -= rect.height / 2
            point = rect.GetBottomLeft()
            length = rect.width

        elif self._direction == wx.LEFT:
            rect.width -= rect.width / 2
            point = rect.GetTopRight()
            length = rect.height

        elif self._direction == wx.RIGHT:
            rect.x += rect.width / 2
            rect.width -= rect.width / 2
            point = rect.GetTopLeft()
            length = rect.height

        elif self._direction == wx.BOTTOM:
            rect.y += rect.height / 2
            rect.height -= rect.height / 2
            point = rect.GetTopLeft()
            length = rect.width

        elif self._direction == wx.CENTER:
            rect.Deflate(1, 1)
            point = rect.GetTopLeft()
            length = rect.width

        dc.GradientFillLinear(rect, colourIconDockingPart1,
                              colourIconDockingPart2, self._direction)

        dc.SetPen(wx.Pen(colourIconBorder))

        if self._direction == wx.CENTER:        
            self.DrawDottedLine(dc, rect.GetTopLeft(), rect.width, False)
            self.DrawDottedLine(dc, rect.GetTopLeft(), rect.height, True)
            self.DrawDottedLine(dc, rect.GetBottomLeft(), rect.width, False)
            self.DrawDottedLine(dc, rect.GetTopRight(), rect.height, True)
        
        elif self._direction in [wx.TOP, wx.BOTTOM]:
            self.DrawDottedLine(dc, point, length, False)
        
        else:
            self.DrawDottedLine(dc, point, length, True)
        

    def DrawArrow(self, dc):
        """
        Draws the docking guide arrow icon (not used if the docking guide images are ok).

        :param `dc`: a `wx.DC` device context object.
        """

        rect = self.GetClientRect()
        point = wx.Point()

        point.x = (rect.GetLeft() + rect.GetRight()) / 2
        point.y = (rect.GetTop() + rect.GetBottom()) / 2
        rx, ry = wx.Size(), wx.Size()
        
        if self._direction == wx.TOP:
            rx = wx.Size(1, 0)
            ry = wx.Size(0, 1)

        elif self._direction == wx.LEFT:
            rx = wx.Size(0, -1)
            ry = wx.Size(1, 0)

        elif self._direction == wx.RIGHT:
            rx = wx.Size(0, 1)
            ry = wx.Size(-1, 0)

        elif self._direction == wx.BOTTOM:
            rx = wx.Size(-1, 0)
            ry = wx.Size(0, -1)        

        point.x += ry.x*3
        point.y += ry.y*3

        dc.SetPen(wx.Pen(colourIconArrow))

        for i in xrange(4):
            pt1 = wx.Point(point.x - rx.x*i, point.y - rx.y*i)
            pt2 = wx.Point(point.x + rx.x*(i+1), point.y + rx.y*(i+1))
            dc.DrawLinePoint(pt1, pt2)
            point.x += ry.x
            point.y += ry.y

    
    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{AuiDockingGuideWindow}.

        :param `event`: a `wx.PaintEvent` to be processed.
        """

        dc = wx.AutoBufferedPaintDC(self)
        if self._currentImage.IsOk() and self._valid:
            dc.DrawBitmap(self._currentImage, 0, 0, True)
        else:
            self.Draw(dc)


    def Draw(self, dc):
        """
        Draws the whole docking guide window (not used if the docking guide images are ok).

        :param `dc`: a `wx.DC` device context object.
        """

        self.DrawBackground(dc)

        if self._valid:
            self.DrawIcon(dc)
            self.DrawArrow(dc)


    def UpdateDockGuide(self, pos):
        """
        Updates the docking guide images depending on the mouse position, using focused
        images if the mouse is inside the docking guide or unfocused images if it is
        outside.

        :param `pos`: a `wx.Point` mouse position.
        """

        inside = self.GetScreenRect().Contains(pos)
        
        if inside:
            image = self._bmp_focus
        else:
            image = self._bmp_unfocus

        if image != self._currentImage:
            self._currentImage = image
            self.Refresh()
            self.Update()


# ---------------------------------------------------------------------------
# AuiSingleDockingGuide
# ---------------------------------------------------------------------------

class AuiSingleDockingGuide(AuiDockingGuide):
    """ A docking guide window for single docking hint (not diamond-shaped HUD). """
    
    def __init__(self, parent, direction=0):
        """
        Default class constructor. Used internally, do not call it in your code!

        :param `parent`: the L{AuiSingleDockingGuide} parent;
        :param `direction`: one of ``wx.TOP``, ``wx.BOTTOM``, ``wx.LEFT``, ``wx.RIGHT``.
        """

        self._direction = direction

        style = wx.FRAME_TOOL_WINDOW | wx.STAY_ON_TOP | \
                wx.FRAME_NO_TASKBAR | wx.NO_BORDER

        # Use of FRAME_SHAPED on wxMac causes the frame to be visible
        # breaking the docking hints.
        if wx.Platform != '__WXMAC__':
            style |= wx.FRAME_SHAPED

        AuiDockingGuide.__init__(self, parent, style=style, name="auiSingleDockTarget")
        
        self.Hide()

        useAero = GetManager(self.GetParent()).GetAGWFlags() & AUI_MGR_AERO_DOCKING_GUIDES
        useWhidbey = GetManager(self.GetParent()).GetAGWFlags() & AUI_MGR_WHIDBEY_DOCKING_GUIDES
        
        self._useAero = useAero or useWhidbey
        self._valid = True
        
        if useAero:
            sizeX, sizeY = aeroguideSizeX, aeroguideSizeY
        elif useWhidbey:
            sizeX, sizeY = whidbeySizeX, whidbeySizeY
        else:
            sizeX, sizeY = guideSizeX, guideSizeY

        if direction not in [wx.TOP, wx.BOTTOM]:
            sizeX, sizeY = sizeY, sizeX

        if self._useAero:
            self.CreateShapesWithStyle(useWhidbey)
            
            if wx.Platform == "__WXGTK__":
                self.Bind(wx.EVT_WINDOW_CREATE, self.SetGuideShape)
            else:
                self.SetGuideShape()
            
            self.SetSize(self.region.GetBox().GetSize())
        else:
            self.SetSize((sizeX, sizeY))
            
        self.rect = wx.Rect(0, 0, sizeX, sizeY)

        if self._useAero:
            useAero = (useWhidbey and [2] or [1])[0]
        else:
            useAero = 0
            
        self.target = AuiDockingGuideWindow(self, self.rect, direction, False, useAero)


    def CreateShapesWithStyle(self, useWhidbey):
        """
        Creates the docking guide window shape based on which docking bitmaps are used.

        :param `useWhidbey`: if ``True``, use Whidbey-style bitmaps; if ``False``, use the
         Aero-style bitmaps.
         """

        sizeX, sizeY = aeroguideSizeX, aeroguideSizeY
        if useWhidbey:
            sizeX, sizeY = whidbeySizeX, whidbeySizeY

        if self._direction not in [wx.TOP, wx.BOTTOM]:
            sizeX, sizeY = sizeY, sizeX

        useAero = (useWhidbey and [2] or [1])[0]      
        bmp, dummy = GetDockingImage(self._direction, useAero, False)
        region = wx.RegionFromBitmap(bmp)
            
        self.region = region
        

    def AeroMove(self, pos):
        """
        Moves the docking window to the new position. Overridden in children classes.

        :param `pos`: the new docking guide position.
        """

        pass
    

    def SetGuideShape(self, event=None):
        """
        Sets the correct shape for the docking guide window.

        :param `event`: on wxGTK, a `wx.WindowCreateEvent` event to process.
        """

        self.SetShape(self.region)        
                
        if event is not None:
            # Skip the event on wxGTK
            event.Skip()
            wx.CallAfter(wx.SafeYield, self, True)


    def SetShape(self, region):
        """
        If the platform supports it, sets the shape of the window to that depicted by `region`.
        The system will not display or respond to any mouse event for the pixels that lie
        outside of the region. To reset the window to the normal rectangular shape simply call
        L{SetShape} again with an empty region. 

        :param `region`: the shape of the frame.

        :note: Overridden for wxMac.        
        """
        
        if wx.Platform == '__WXMAC__':
            # HACK so we don't crash when SetShape is called
            return
        else:
            super(AuiSingleDockingGuide, self).SetShape(region)


    def SetValid(self, valid):
        """
        Sets the docking direction as valid or invalid.

        :param `valid`: whether the docking direction is allowed or not.
        """

        self._valid = valid


    def IsValid(self):
        """ Returns whether the docking direction is valid. """
        
        return self._valid


    def UpdateDockGuide(self, pos):
        """
        Updates the docking guide images depending on the mouse position, using focused
        images if the mouse is inside the docking guide or unfocused images if it is
        outside.

        :param `pos`: a `wx.Point` mouse position.
        """

        self.target.UpdateDockGuide(pos)

        
    def HitTest(self, x, y):
        """
        Checks if the mouse position is inside the target window rect.

        :param `x`: the `x` mouse position;
        :param `y`: the `y` mouse position.
        """

        if self.target.GetScreenRect().Contains((x, y)):
            return wx.ALL

        return -1


# ---------------------------------------------------------------------------
# AuiCenterDockingGuide
# ---------------------------------------------------------------------------

class AuiCenterDockingGuide(AuiDockingGuide):
    """ A docking guide window for multiple docking hint (diamond-shaped HUD). """
    
    def __init__(self, parent):
        """
        Default class constructor.
        Used internally, do not call it in your code!

        :param `parent`: the L{AuiCenterDockingGuide} parent.
        """

        AuiDockingGuide.__init__(self, parent, style=wx.FRAME_TOOL_WINDOW | wx.STAY_ON_TOP |
                                 wx.FRAME_NO_TASKBAR | wx.NO_BORDER | wx.FRAME_SHAPED,
                                 name="auiCenterDockTarget")

        self.Hide()

        self.CreateShapesWithStyle()
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        
        if wx.Platform == "__WXGTK__":
            self.Bind(wx.EVT_WINDOW_CREATE, self.SetGuideShape)
        else:
            self.SetGuideShape()
            
        self.SetSize(self.region.GetBox().GetSize())

        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_PAINT, self.OnPaint)


    def CreateShapesWithStyle(self):
        """ Creates the docking guide window shape based on which docking bitmaps are used. """

        useAero = (GetManager(self.GetParent()).GetAGWFlags() & AUI_MGR_AERO_DOCKING_GUIDES) != 0
        useWhidbey = (GetManager(self.GetParent()).GetAGWFlags() & AUI_MGR_WHIDBEY_DOCKING_GUIDES) != 0

        self._useAero = 0
        if useAero:
            self._useAero = 1
        elif useWhidbey:
            self._useAero = 2
        
        if useAero:
            sizeX, sizeY = aeroguideSizeX, aeroguideSizeY
        elif useWhidbey:
            sizeX, sizeY = whidbeySizeX, whidbeySizeY          
        else:
            sizeX, sizeY = guideSizeX, guideSizeY

        rectLeft = wx.Rect(0, sizeY, sizeY, sizeX)
        rectTop = wx.Rect(sizeY, 0, sizeX, sizeY)
        rectRight = wx.Rect(sizeY+sizeX, sizeY, sizeY, sizeX)
        rectBottom = wx.Rect(sizeY, sizeX + sizeY, sizeX, sizeY)
        rectCenter = wx.Rect(sizeY, sizeY, sizeX, sizeX)
            
        if not self._useAero:

            self.targetLeft = AuiDockingGuideWindow(self, rectLeft, wx.LEFT, True, useAero)
            self.targetTop = AuiDockingGuideWindow(self, rectTop, wx.TOP, True, useAero)
            self.targetRight = AuiDockingGuideWindow(self, rectRight, wx.RIGHT, True, useAero)
            self.targetBottom = AuiDockingGuideWindow(self, rectBottom, wx.BOTTOM, True, useAero)
            self.targetCenter = AuiDockingGuideWindow(self, rectCenter, wx.CENTER, True, useAero)

            
            # top-left diamond
            tld = [wx.Point(rectTop.x, rectTop.y+rectTop.height-8),
                   wx.Point(rectLeft.x+rectLeft.width-8, rectLeft.y),
                   rectTop.GetBottomLeft()]
            # bottom-left diamond
            bld = [wx.Point(rectLeft.x+rectLeft.width-8, rectLeft.y+rectLeft.height),
                   wx.Point(rectBottom.x, rectBottom.y+8),
                   rectBottom.GetTopLeft()]
            # top-right diamond
            trd = [wx.Point(rectTop.x+rectTop.width, rectTop.y+rectTop.height-8),
                   wx.Point(rectRight.x+8, rectRight.y),
                   rectRight.GetTopLeft()]        
            # bottom-right diamond
            brd = [wx.Point(rectRight.x+8, rectRight.y+rectRight.height),
                   wx.Point(rectBottom.x+rectBottom.width, rectBottom.y+8),
                   rectBottom.GetTopRight()]

            self._triangles = [tld[0:2], bld[0:2],
                               [wx.Point(rectTop.x+rectTop.width-1, rectTop.y+rectTop.height-8),
                                wx.Point(rectRight.x+7, rectRight.y)],
                               [wx.Point(rectRight.x+7, rectRight.y+rectRight.height),
                                wx.Point(rectBottom.x+rectBottom.width-1, rectBottom.y+8)]]
            
            region = wx.Region()
            region.UnionRect(rectLeft)
            region.UnionRect(rectTop)
            region.UnionRect(rectRight)
            region.UnionRect(rectBottom)
            region.UnionRect(rectCenter)
            region.UnionRegion(wx.RegionFromPoints(tld))
            region.UnionRegion(wx.RegionFromPoints(bld))
            region.UnionRegion(wx.RegionFromPoints(trd))
            region.UnionRegion(wx.RegionFromPoints(brd))

        elif useAero:

            self._aeroBmp = aero_dock_pane.GetBitmap()
            region = wx.RegionFromBitmap(self._aeroBmp)

            self._allAeroBmps = [aero_dock_pane_left.GetBitmap(), aero_dock_pane_top.GetBitmap(),
                                 aero_dock_pane_right.GetBitmap(), aero_dock_pane_bottom.GetBitmap(),
                                 aero_dock_pane_center.GetBitmap(), aero_dock_pane.GetBitmap()]
            self._deniedBitmap = aero_denied.GetBitmap()
            self._aeroRects = [rectLeft, rectTop, rectRight, rectBottom, rectCenter]
            self._valid = True

        elif useWhidbey:

            self._aeroBmp = whidbey_dock_pane.GetBitmap()
            region = wx.RegionFromBitmap(self._aeroBmp)

            self._allAeroBmps = [whidbey_dock_pane_left.GetBitmap(), whidbey_dock_pane_top.GetBitmap(),
                                 whidbey_dock_pane_right.GetBitmap(), whidbey_dock_pane_bottom.GetBitmap(),
                                 whidbey_dock_pane_center.GetBitmap(), whidbey_dock_pane.GetBitmap()]
            self._deniedBitmap = whidbey_denied.GetBitmap()
            self._aeroRects = [rectLeft, rectTop, rectRight, rectBottom, rectCenter]
            self._valid = True
            
            
        self.region = region
        

    def SetGuideShape(self, event=None):
        """
        Sets the correct shape for the docking guide window.

        :param `event`: on wxGTK, a `wx.WindowCreateEvent` event to process.
        """

        self.SetShape(self.region)        

        if event is not None:
            # Skip the event on wxGTK
            event.Skip()
            wx.CallAfter(wx.SafeYield, self, True)

            
    def UpdateDockGuide(self, pos):
        """
        Updates the docking guides images depending on the mouse position, using focused
        images if the mouse is inside the docking guide or unfocused images if it is
        outside.

        :param `pos`: a `wx.Point` mouse position.
        """

        if not self._useAero:
            for target in self.GetChildren():
                target.UpdateDockGuide(pos)
        else:
            lenRects = len(self._aeroRects)
            for indx, rect in enumerate(self._aeroRects):
                if rect.Contains(pos):
                    if self._allAeroBmps[indx] != self._aeroBmp:
                        if indx < lenRects - 1 or (indx == lenRects - 1 and self._valid):
                            self._aeroBmp = self._allAeroBmps[indx]
                            self.Refresh()
                        else:
                            self._aeroBmp = self._allAeroBmps[-1]
                            self.Refresh()
                            
                    return

            if self._aeroBmp != self._allAeroBmps[-1]:
                self._aeroBmp = self._allAeroBmps[-1]
                self.Refresh()


    def HitTest(self, x, y):
        """
        Checks if the mouse position is inside the target windows rect.

        :param `x`: the `x` mouse position;
        :param `y`: the `y` mouse position.
        """

        if not self._useAero:
            if self.targetLeft.GetScreenRect().Contains((x, y)):
                return wx.LEFT
            if self.targetTop.GetScreenRect().Contains((x, y)):
                return wx.UP
            if self.targetRight.GetScreenRect().Contains((x, y)):
                return wx.RIGHT
            if self.targetBottom.GetScreenRect().Contains((x, y)):
                return wx.DOWN
            if self.targetCenter.IsValid() and self.targetCenter.GetScreenRect().Contains((x, y)):
                return wx.CENTER
        else:
            constants = [wx.LEFT, wx.UP, wx.RIGHT, wx.DOWN, wx.CENTER]
            lenRects = len(self._aeroRects)
            for indx, rect in enumerate(self._aeroRects):
                if rect.Contains((x, y)):
                    if indx < lenRects or (indx == lenRects-1 and self._valid):
                        return constants[indx]

        return -1


    def ValidateNotebookDocking(self, valid):
        """
        Sets whether a pane can be docked on top of another to create an automatic
        L{AuiNotebook}.

        :param `valid`: whether a pane can be docked on top to another to form an automatic
         L{AuiNotebook}.
        """

        if not self._useAero:
            if self.targetCenter.IsValid() != valid:        
                self.targetCenter.SetValid(valid)
                self.targetCenter.Refresh()
        else:
            if self._valid != valid:
                self._valid = valid
                self.Refresh()
    

    def AeroMove(self, pos):
        """
        Moves the docking guide window to the new position.

        :param `pos`: the new docking guide position.
        """

        if not self._useAero:
            return

        useWhidbey = (GetManager(self.GetParent()).GetAGWFlags() & AUI_MGR_WHIDBEY_DOCKING_GUIDES) != 0

        if useWhidbey:
            sizeX, sizeY = whidbeySizeX, whidbeySizeY            
        else:
            sizeX, sizeY = aeroguideSizeX, aeroguideSizeY
            
        size = self.GetSize()
        
        leftRect, topRect, rightRect, bottomRect, centerRect = self._aeroRects
        thePos = pos + wx.Point((size.x-sizeY)/2, (size.y-sizeX)/2)
        
        centerRect.SetPosition(thePos)

        leftRect.SetPosition(thePos + wx.Point(-sizeY, 0))
        topRect.SetPosition(thePos + wx.Point(0, -sizeY))
        rightRect.SetPosition(thePos + wx.Point(sizeX, 0))
        bottomRect.SetPosition(thePos + wx.Point(0, sizeX))
        
        
    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{AuiCenterDockingGuide}.

        :param `event`: `wx.EraseEvent` to be processed.

        :note: This is intentionally empty to reduce flickering while drawing.
        """
        
        pass


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{AuiCenterDockingGuide}.

        :param `event`: a `wx.PaintEvent` to be processed.
        """

        dc = wx.AutoBufferedPaintDC(self)

        if self._useAero:
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
            dc.SetPen(wx.TRANSPARENT_PEN)
        else:
            dc.SetBrush(wx.Brush(colourTargetBackground))
            dc.SetPen(wx.Pen(colourTargetBorder))

        rect = self.GetClientRect()
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height)

        if self._useAero:
            dc.DrawBitmap(self._aeroBmp, 0, 0, True)
            if not self._valid:
                diff = (self._useAero == 2 and [1] or [0])[0]
                bmpX, bmpY = self._deniedBitmap.GetWidth(), self._deniedBitmap.GetHeight()
                xPos, yPos = (rect.x + (rect.width)/2 - bmpX/2), (rect.y + (rect.height)/2 - bmpY/2)
                dc.DrawBitmap(self._deniedBitmap, xPos+1, yPos+diff, True)
                
            return
        
        dc.SetPen(wx.Pen(colourTargetBorder, 2))
        for pts in self._triangles:
            dc.DrawLinePoint(pts[0], pts[1])
            

# ----------------------------------------------------------------------------
# AuiDockingHintWindow
# ----------------------------------------------------------------------------

class AuiDockingHintWindow(wx.Frame):
    """ The original wxAUI docking window hint. """

    def __init__(self, parent, id=wx.ID_ANY, title="", pos=wx.DefaultPosition,
                 size=wx.Size(1, 1), style=wx.FRAME_TOOL_WINDOW | wx.FRAME_FLOAT_ON_PARENT |
                 wx.FRAME_NO_TASKBAR | wx.NO_BORDER | wx.FRAME_SHAPED,
                 name="auiHintWindow"):
        """
        Default class constructor. Used internally, do not call it in your code!

        :param `parent`: the L{AuiDockingGuide} parent;
        :param `id`: the window identifier. It may take a value of -1 to indicate a default value.
        :param `title`: the caption to be displayed on the frame's title bar;
        :param `pos`: the window position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the window size. A value of (-1, -1) indicates a default size, chosen by
         either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `name`: the name of the window. This parameter is used to associate a name with the
         item, allowing the application user to set Motif resource values for individual windows.
        """
        if wx.Platform == '__WXMAC__' and style & wx.FRAME_SHAPED:
            # Having the shaped frame causes the frame to not be visible
            # with the transparent style hints.
            style -= wx.FRAME_SHAPED

        wx.Frame.__init__(self, parent, id, title, pos, size, style, name=name)
        
        self._blindMode = False
        self.SetBackgroundColour(colourHintBackground)
        
        # Can't set background colour on a frame on wxMac
        # so add a panel to set the colour on.
        if wx.Platform == '__WXMAC__':
            sizer = wx.BoxSizer(wx.HORIZONTAL)
            self.panel = wx.Panel(self)
            sizer.Add(self.panel, 1, wx.EXPAND)
            self.SetSizer(sizer)
            self.panel.SetBackgroundColour(colourHintBackground)

        self.Bind(wx.EVT_SIZE, self.OnSize)
        

    def MakeVenetianBlinds(self):
        """
        Creates the "venetian blind" effect if L{AuiManager} has the ``AUI_MGR_VENETIAN_BLINDS_HINT``
        flag set.
        """

        amount = 128
        size = self.GetClientSize()
        region = wx.Region(0, 0, size.x, 1)

        for y in xrange(size.y):

            # Reverse the order of the bottom 4 bits
            j = (y & 8 and [1] or [0])[0] | (y & 4 and [2] or [0])[0] | \
                (y & 2 and [4] or [0])[0] | (y & 1 and [8] or [0])[0]
            
            if 16*j+8 < amount:
                region.Union(0, y, size.x, 1)
                        
        self.SetShape(region)


    def SetBlindMode(self, agwFlags):
        """
        Sets whether venetian blinds or transparent hints will be shown as docking hint.
        This depends on the L{AuiManager} flags.

        :param `agwFlags`: the L{AuiManager} flags.
        """

        self._blindMode = (agwFlags & AUI_MGR_VENETIAN_BLINDS_HINT) != 0

        if self._blindMode or not self.CanSetTransparent():
            self.MakeVenetianBlinds()
            self.SetTransparent(255)
        
        else:
            self.SetShape(wx.Region())
            if agwFlags & AUI_MGR_HINT_FADE == 0:                
                self.SetTransparent(80)
            else:
                self.SetTransparent(0)


    def SetShape(self, region):
        """
        If the platform supports it, sets the shape of the window to that depicted by `region`.
        The system will not display or respond to any mouse event for the pixels that lie
        outside of the region. To reset the window to the normal rectangular shape simply call
        L{SetShape} again with an empty region. 

        :param `region`: the shape of the frame (an instance of `wx.Region`).

        :note: Overridden for wxMac.        
        """
        
        if wx.Platform == '__WXMAC__':
            # HACK so we don't crash when SetShape is called
            return
        else:
            super(AuiDockingHintWindow, self).SetShape(region)


    def Show(self, show=True):
        """
        Show the hint window.

        :param `show`: whether to show or hide the hint docking window.
        """
        
        super(AuiDockingHintWindow, self).Show(show)
        if wx.Platform == '__WXMAC__':
            # Need to manually do layout since its a borderless frame.
            self.Layout()


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{AuiDockingHintWindow}.

        :param `event`: a `wx.SizeEvent` to be processed.
        """

        if self._blindMode or not self.CanSetTransparent():
            self.MakeVenetianBlinds()


# ---------------------------------------------------------------------------- #

# -- AuiFloatingFrame class implementation --            

class AuiFloatingFrame(wx.MiniFrame):
    """ AuiFloatingFrame is the frame class that holds floating panes. """

    def __init__(self, parent, owner_mgr, pane=None, id=wx.ID_ANY, title="",
                 style=wx.FRAME_TOOL_WINDOW | wx.FRAME_FLOAT_ON_PARENT |
                 wx.FRAME_NO_TASKBAR | wx.CLIP_CHILDREN):
        """
        Default class constructor. Used internally, do not call it in your code!

        :param `parent`: the L{AuiFloatingFrame} parent;
        :param `owner_mgr`: the L{AuiManager} that manages the floating pane;
        :param `pane`: the L{AuiPaneInfo} pane that is about to float;
        :param `id`: the window identifier. It may take a value of -1 to indicate a default value.
        :param `title`: the caption to be displayed on the frame's title bar.
        :param `style`: the window style.
        """
            
        if pane and pane.IsResizeable():
            style += wx.RESIZE_BORDER
        if pane:
            self._is_toolbar = pane.IsToolbar()

        self._useNativeMiniframes = False
        if AuiManager_UseNativeMiniframes(owner_mgr):
            # On wxMac we always use native miniframes
            self._useNativeMiniframes = True
            style += wx.CAPTION + wx.SYSTEM_MENU
            if pane.HasCloseButton():
                style += wx.CLOSE_BOX
            if pane.HasMaximizeButton():
                style += wx.MAXIMIZE_BOX
            if pane.HasMinimizeButton():
                style += wx.MINIMIZE_BOX
            
        wx.MiniFrame.__init__(self, parent, id, title, pos=pane.floating_pos,
                              size=pane.floating_size, style=style, name="auiFloatingFrame")

        self._fly_timer = wx.Timer(self, wx.ID_ANY)
        self._check_fly_timer = wx.Timer(self, wx.ID_ANY)
        
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_ACTIVATE, self.OnActivate)
        self.Bind(wx.EVT_TIMER, self.OnCheckFlyTimer, self._check_fly_timer)
        self.Bind(wx.EVT_TIMER, self.OnFlyTimer, self._fly_timer)
        self.Bind(EVT_AUI_FIND_MANAGER, self.OnFindManager)

        if self._useNativeMiniframes:
            self.Bind(wx.EVT_MOVE, self.OnMoveEvent)
            self.Bind(wx.EVT_MOVING, self.OnMoveEvent)
            self.Bind(wx.EVT_IDLE, self.OnIdle)
            self._useNativeMiniframes = True
            self.SetExtraStyle(wx.WS_EX_PROCESS_IDLE)
        else:
            self.Bind(wx.EVT_MOVE, self.OnMove)

        self._fly = False
        self._send_size = True
        self._alpha_amount = 255
        
        self._owner_mgr = owner_mgr
        self._moving = False
        self._lastDirection = None
        self._transparent = 255

        self._last_rect = wx.Rect()
        self._last2_rect = wx.Rect()
        self._last3_rect = wx.Rect()

        self._mgr = AuiManager()
        self._mgr.SetManagedWindow(self)
        self._mgr.SetArtProvider(owner_mgr.GetArtProvider())
        self._mgr.SetAGWFlags(owner_mgr.GetAGWFlags())


    def CopyAttributes(self, pane):
        """
        Copies all the attributes of the input `pane` into another L{AuiPaneInfo}.

        :param `pane`: the source L{AuiPaneInfo} from where to copy attributes.
        """

        contained_pane = AuiPaneInfo()

        contained_pane.name = pane.name
        contained_pane.caption = pane.caption
        contained_pane.window = pane.window
        contained_pane.frame = pane.frame
        contained_pane.state = pane.state
        contained_pane.dock_direction = pane.dock_direction
        contained_pane.dock_layer = pane.dock_layer
        contained_pane.dock_row = pane.dock_row
        contained_pane.dock_pos = pane.dock_pos
        contained_pane.best_size = wx.Size(*pane.best_size)
        contained_pane.min_size = wx.Size(*pane.min_size)
        contained_pane.max_size = wx.Size(*pane.max_size)
        contained_pane.floating_pos = wx.Point(*pane.floating_pos)
        contained_pane.floating_size = wx.Size(*pane.floating_size)
        contained_pane.dock_proportion = pane.dock_proportion
        contained_pane.buttons = pane.buttons
        contained_pane.rect = wx.Rect(*pane.rect)
        contained_pane.icon = pane.icon
        contained_pane.notebook_id = pane.notebook_id
        contained_pane.transparent = pane.transparent
        contained_pane.snapped = pane.snapped
        contained_pane.minimize_mode = pane.minimize_mode

        return contained_pane
    

    def SetPaneWindow(self, pane):
        """
        Sets all the properties of a pane.

        :param `pane`: the L{AuiPaneInfo} to analyze.
        """

        self._is_toolbar = pane.IsToolbar()
        self._pane_window = pane.window

        if isinstance(pane.window, auibar.AuiToolBar):
            pane.window.SetAuiManager(self._mgr)
        
        self._pane_window.Reparent(self)
        
        contained_pane = self.CopyAttributes(pane)
        
        contained_pane.Dock().Center().Show(). \
                       CaptionVisible(False). \
                       PaneBorder(False). \
                       Layer(0).Row(0).Position(0)

        if not contained_pane.HasGripper() and not self._useNativeMiniframes:
            contained_pane.CaptionVisible(True)

        indx = self._owner_mgr._panes.index(pane)

        # Carry over the minimum size
        pane_min_size = pane.window.GetMinSize()

        # if the best size is smaller than the min size
        # then set the min size to the best size as well
        pane_best_size = contained_pane.best_size
        if pane_best_size.IsFullySpecified() and (pane_best_size.x < pane_min_size.x or \
                                                  pane_best_size.y < pane_min_size.y):

            pane_min_size = pane_best_size
            self._pane_window.SetMinSize(pane_min_size)
    
        # if the frame window's max size is greater than the min size
        # then set the max size to the min size as well
        cur_max_size = self.GetMaxSize()
        if cur_max_size.IsFullySpecified() and  (cur_max_size.x < pane_min_size.x or \
                                                 cur_max_size.y < pane_min_size.y):
            self.SetMaxSize(pane_min_size)

        art_provider = self._mgr.GetArtProvider()
        caption_size = art_provider.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        button_size = art_provider.GetMetric(AUI_DOCKART_PANE_BUTTON_SIZE) + \
                      4*art_provider.GetMetric(AUI_DOCKART_PANE_BORDER_SIZE)

        min_size = pane.window.GetMinSize()

        if min_size.y < caption_size or min_size.x < button_size:
            new_x, new_y = min_size.x, min_size.y
            if min_size.y < caption_size:
                new_y = (pane.IsResizeable() and [2*wx.SystemSettings.GetMetric(wx.SYS_EDGE_Y)+caption_size] or [1])[0]
            if min_size.x < button_size:
                new_x = (pane.IsResizeable() and [2*wx.SystemSettings.GetMetric(wx.SYS_EDGE_X)+button_size] or [1])[0]
                
            self.SetMinSize((new_x, new_y))
        else:
            self.SetMinSize(min_size)

        self._mgr.AddPane(self._pane_window, contained_pane)
        self._mgr.Update()           

        if pane.min_size.IsFullySpecified():
            # because SetSizeHints() calls Fit() too (which sets the window
            # size to its minimum allowed), we keep the size before calling
            # SetSizeHints() and reset it afterwards...
            tmp = self.GetSize()
            self.GetSizer().SetSizeHints(self)
            self.SetSize(tmp)
        
        self.SetTitle(pane.caption)

        if pane.floating_size != wx.Size(-1, -1):
            self.SetSize(pane.floating_size)
        else:
            size = pane.best_size
            if size == wx.Size(-1, -1):
                size = pane.min_size
            if size == wx.Size(-1, -1):
                size = self._pane_window.GetSize()
            if self._owner_mgr and pane.HasGripper():
                if pane.HasGripperTop():
                    size.y += self._owner_mgr._art.GetMetric(AUI_DOCKART_GRIPPER_SIZE)
                else:
                    size.x += self._owner_mgr._art.GetMetric(AUI_DOCKART_GRIPPER_SIZE)

            if not self._useNativeMiniframes:
                size.y += self._owner_mgr._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
                
            pane.floating_size = size
            
            self.SetClientSize(size)

        self._owner_mgr._panes[indx] = pane

        self._fly_step = abs(pane.floating_size.y - \
                             (caption_size + 2*wx.SystemSettings.GetMetric(wx.SYS_EDGE_Y)))/10

        self._floating_size = wx.Size(*self.GetSize())

        if pane.IsFlyOut():
            self._check_fly_timer.Start(50)

        
    def GetOwnerManager(self):
        """ Returns the L{AuiManager} that manages the pane. """

        return self._owner_mgr


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.SizeEvent` to be processed.
        """

        if self._owner_mgr and self._send_size:
            self._owner_mgr.OnFloatingPaneResized(self._pane_window, event.GetSize())

    
    def OnClose(self, event):
        """
        Handles the ``wx.EVT_CLOSE`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.CloseEvent` to be processed.
        """

        if self._owner_mgr:
            self._owner_mgr.OnFloatingPaneClosed(self._pane_window, event)

        if not event.GetVeto():
            self._mgr.DetachPane(self._pane_window)

            if isinstance(self._pane_window, auibar.AuiToolBar):
                self._pane_window.SetAuiManager(self._owner_mgr)

            # if we do not do this, then we can crash...
            if self._owner_mgr and self._owner_mgr._action_window == self:
                self._owner_mgr._action_window = None

            self.Destroy()
    

    def OnActivate(self, event):
        """
        Handles the ``wx.EVT_ACTIVATE`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.ActivateEvent` to be processed.
        """

        if self._owner_mgr and event.GetActive():
            self._owner_mgr.OnFloatingPaneActivated(self._pane_window)


    def OnMove(self, event):
        """
        Handles the ``wx.EVT_MOVE`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.MoveEvent` to be processed.

        :note: This event is not processed on wxMAC or if L{AuiManager} is not using the
         ``AUI_MGR_USE_NATIVE_MINIFRAMES`` style.
        """

        if self._owner_mgr:
            self._owner_mgr.OnFloatingPaneMoved(self._pane_window, event)
                

    def OnMoveEvent(self, event):
        """
        Handles the ``wx.EVT_MOVE`` and ``wx.EVT_MOVING`` events for L{AuiFloatingFrame}.

        :param `event`: a `wx.MoveEvent` to be processed.

        :note: This event is only processed on wxMAC or if L{AuiManager} is using the
         ``AUI_MGR_USE_NATIVE_MINIFRAMES`` style.
        """

        win_rect = self.GetRect()

        if win_rect == self._last_rect:
            return

        # skip the first move event
        if self._last_rect.IsEmpty():        
            self._last_rect = wx.Rect(*win_rect)
            return
        
        # skip if moving too fast to avoid massive redraws and
        # jumping hint windows
        if abs(win_rect.x - self._last_rect.x) > 3 or abs(win_rect.y - self._last_rect.y) > 3:
            self._last3_rect = wx.Rect(*self._last2_rect)
            self._last2_rect = wx.Rect(*self._last_rect)
            self._last_rect = wx.Rect(*win_rect)
            return

        # prevent frame redocking during resize
        if self._last_rect.GetSize() != win_rect.GetSize():
            self._last3_rect = wx.Rect(*self._last2_rect)
            self._last2_rect = wx.Rect(*self._last_rect)
            self._last_rect = wx.Rect(*win_rect)
            return

        self._last3_rect = wx.Rect(*self._last2_rect)
        self._last2_rect = wx.Rect(*self._last_rect)
        self._last_rect = wx.Rect(*win_rect)

        if _VERSION_STRING < "2.9":
            leftDown = wx.GetMouseState().LeftDown()
        else:
            leftDown = wx.GetMouseState().LeftIsDown()

        if not leftDown:
            return

        if not self._moving:        
            self.OnMoveStart(event)
            self._moving = True

        if self._last3_rect.IsEmpty():
            return

        self.OnMoving(event)


    def OnIdle(self, event):
        """
        Handles the ``wx.EVT_IDLE`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.IdleEvent` event to be processed.

        :note: This event is only processed on wxMAC or if L{AuiManager} is using the
         ``AUI_MGR_USE_NATIVE_MINIFRAMES`` style.        
        """

        if self._moving:        
            if _VERSION_STRING < "2.9":
                leftDown = wx.GetMouseState().LeftDown()
            else:
                leftDown = wx.GetMouseState().LeftIsDown()

            if not leftDown:
                self._moving = False
                self.OnMoveFinished()
            else:            
                event.RequestMore()

        
    def OnMoveStart(self, event):
        """
        The user has just started moving the floating pane.

        :param `event`: an instance of `wx.MouseEvent`.
    
        :note: This method is used only on wxMAC or if L{AuiManager} is using the
         ``AUI_MGR_USE_NATIVE_MINIFRAMES`` style.
        """

        # notify the owner manager that the pane has started to move
        if self._owner_mgr:
            if self._owner_mgr._from_move:
                return
            self._owner_mgr._action_window = self._pane_window
            point = wx.GetMousePosition()
            action_offset = point - self.GetPosition()

            if self._is_toolbar:
                self._owner_mgr._toolbar_action_offset = action_offset
                self._owner_mgr.OnMotion_DragToolbarPane(point)
            else:
                self._owner_mgr._action_offset = action_offset
                self._owner_mgr.OnMotion_DragFloatingPane(point)

    
    def OnMoving(self, event):
        """
        The user is moving the floating pane.

        :param `event`: an instance of `wx.MouseEvent`.
        
        :note: This method is used only on wxMAC or if L{AuiManager} is using the
         ``AUI_MGR_USE_NATIVE_MINIFRAMES`` style.
        """

        # notify the owner manager that the pane is moving
        self.OnMoveStart(event)
        

    def OnMoveFinished(self):
        """
        The user has just finished moving the floating pane.

        :note: This method is used only on wxMAC or if L{AuiManager} is using the
         ``AUI_MGR_USE_NATIVE_MINIFRAMES`` style.
        """

        # notify the owner manager that the pane has finished moving
        if self._owner_mgr:
            self._owner_mgr._action_window = self._pane_window
            point = wx.GetMousePosition()
            if self._is_toolbar:
                self._owner_mgr.OnLeftUp_DragToolbarPane(point)
            else:
                self._owner_mgr.OnLeftUp_DragFloatingPane(point)

            self._owner_mgr.OnFloatingPaneMoved(self._pane_window, point)
    

    def OnCheckFlyTimer(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.TimerEvent` to be processed.

        :note: This is used solely for "fly-out" panes.        
        """
        
        if self._owner_mgr:
            pane = self._mgr.GetPane(self._pane_window)
            if pane.IsFlyOut():
                if self.IsShownOnScreen():
                    self.FlyOut()
                        

    def OnFindManager(self, event):
        """
        Handles the ``EVT_AUI_FIND_MANAGER`` event for L{AuiFloatingFrame}.

        :param `event`: a L{AuiManagerEvent} event to be processed.
        """
        
        event.SetManager(self._owner_mgr)


    def FlyOut(self):
        """ Starts the flying in and out of a floating pane. """

        if self._fly_timer.IsRunning():
            return

        if _VERSION_STRING < "2.9":
            leftDown = wx.GetMouseState().LeftDown()
        else:
            leftDown = wx.GetMouseState().LeftIsDown()

        if leftDown:
            return
        
        rect = wx.Rect(*self.GetScreenRect())
        rect.Inflate(10, 10)

        if rect.Contains(wx.GetMousePosition()):
            if not self._fly:
                return
            self._send_size = False
            self._fly_timer.Start(5)
        else:
            if self._fly:
                return
            self._send_size = False
            self._fly_timer.Start(5)


    def OnFlyTimer(self, event):            
        """
        Handles the ``wx.EVT_TIMER`` event for L{AuiFloatingFrame}.

        :param `event`: a `wx.TimerEvent` to be processed.
        """

        current_size = self.GetClientSize()
        floating_size = wx.Size(*self._owner_mgr.GetPane(self._pane_window).floating_size)

        if floating_size.y == -1:
            floating_size = self._floating_size
        
        if not self._fly:
            min_size = self._mgr.GetArtProvider().GetMetric(AUI_DOCKART_CAPTION_SIZE)

            if wx.Platform != "__WXMSW__":
                min_size += 2*wx.SystemSettings.GetMetric(wx.SYS_EDGE_Y)

            if current_size.y - self._fly_step <= min_size:
                self.SetClientSize((current_size.x, min_size))
                self._fly = True
                self._fly_timer.Stop()
                self._send_size = True
            else:
                self.SetClientSize((current_size.x, current_size.y-self._fly_step))

        else:
            if current_size.y + self._fly_step >= floating_size.y:
                self.SetClientSize((current_size.x, floating_size.y))
                self._fly = False
                self._fly_timer.Stop()
                self._send_size = True
            else:
                self.SetClientSize((current_size.x, current_size.y+self._fly_step))

        self.Update()
        self.Refresh()


    def FadeOut(self):
        """ Actually starts the fading out of the floating pane. """

        while 1:
            self._alpha_amount -= 10
            if self._alpha_amount <= 0:
                self._alpha_amount = 255
                return

            self.SetTransparent(self._alpha_amount)
            wx.SafeYield()
            wx.MilliSleep(15)

    
# -- static utility functions --

def DrawResizeHint(dc, rect):
    """
    Draws a resize hint while a sash is dragged.

    :param `rect`: a `wx.Rect` rectangle which specifies the sash dimensions.
    """
        
    if wx.Platform == "__WXMSW__" and wx.App.GetComCtl32Version() >= 600:
        if wx.GetOsVersion()[1] > 5:
            # Windows Vista
            dc.SetPen(wx.Pen("black", 2, wx.SOLID))
            dc.SetBrush(wx.TRANSPARENT_BRUSH)
        else:
            # Draw the nice XP style splitter
            dc.SetPen(wx.TRANSPARENT_PEN)
            dc.SetBrush(wx.BLACK_BRUSH)
        dc.SetLogicalFunction(wx.INVERT)
        dc.DrawRectangleRect(rect)
        dc.SetLogicalFunction(wx.COPY)
    else:
        stipple = PaneCreateStippleBitmap()
        brush = wx.BrushFromBitmap(stipple)
        dc.SetBrush(brush)
        dc.SetPen(wx.TRANSPARENT_PEN)

        dc.SetLogicalFunction(wx.XOR)
        dc.DrawRectangleRect(rect)    


def CopyDocksAndPanes(src_docks, src_panes):
    """
    This utility function creates shallow copies of
    the dock and pane info. L{AuiDockInfo} usually contain pointers
    to L{AuiPaneInfo} classes, thus this function is necessary to reliably
    reconstruct that relationship in the new dock info and pane info arrays.

    :param `src_docks`: a list of L{AuiDockInfo} classes;
    :param `src_panes`: a list of L{AuiPaneInfo} classes.
    """
    
    dest_docks = src_docks
    dest_panes = src_panes

    for ii in xrange(len(dest_docks)):
        dock = dest_docks[ii]
        for jj in xrange(len(dock.panes)):
            for kk in xrange(len(src_panes)):
                if dock.panes[jj] == src_panes[kk]:
                    dock.panes[jj] = dest_panes[kk]

    return dest_docks, dest_panes


def CopyDocksAndPanes2(src_docks, src_panes):
    """
    This utility function creates full copies of
    the dock and pane info. L{AuiDockInfo} usually contain pointers
    to L{AuiPaneInfo} classes, thus this function is necessary to reliably
    reconstruct that relationship in the new dock info and pane info arrays.

    :param `src_docks`: a list of L{AuiDockInfo} classes;
    :param `src_panes`: a list of L{AuiPaneInfo} classes.
    """
    
    dest_docks = []

    for ii in xrange(len(src_docks)):
        dest_docks.append(AuiDockInfo())
        dest_docks[ii].dock_direction = src_docks[ii].dock_direction
        dest_docks[ii].dock_layer = src_docks[ii].dock_layer
        dest_docks[ii].dock_row = src_docks[ii].dock_row
        dest_docks[ii].size = src_docks[ii].size
        dest_docks[ii].min_size = src_docks[ii].min_size
        dest_docks[ii].resizable = src_docks[ii].resizable
        dest_docks[ii].fixed = src_docks[ii].fixed
        dest_docks[ii].toolbar = src_docks[ii].toolbar
        dest_docks[ii].panes = src_docks[ii].panes
        dest_docks[ii].rect = wx.Rect(*src_docks[ii].rect)

    dest_panes = []

    for ii in xrange(len(src_panes)):
        dest_panes.append(AuiPaneInfo())
        dest_panes[ii].name = src_panes[ii].name
        dest_panes[ii].caption = src_panes[ii].caption
        dest_panes[ii].window = src_panes[ii].window
        dest_panes[ii].frame = src_panes[ii].frame
        dest_panes[ii].state = src_panes[ii].state
        dest_panes[ii].dock_direction = src_panes[ii].dock_direction
        dest_panes[ii].dock_layer = src_panes[ii].dock_layer
        dest_panes[ii].dock_row = src_panes[ii].dock_row
        dest_panes[ii].dock_pos = src_panes[ii].dock_pos
        dest_panes[ii].best_size = wx.Size(*src_panes[ii].best_size)
        dest_panes[ii].min_size = wx.Size(*src_panes[ii].min_size)
        dest_panes[ii].max_size = wx.Size(*src_panes[ii].max_size)
        dest_panes[ii].floating_pos = wx.Point(*src_panes[ii].floating_pos)
        dest_panes[ii].floating_size = wx.Size(*src_panes[ii].floating_size)
        dest_panes[ii].dock_proportion = src_panes[ii].dock_proportion
        dest_panes[ii].buttons = src_panes[ii].buttons
        dest_panes[ii].rect = wx.Rect(*src_panes[ii].rect)
        dest_panes[ii].icon = src_panes[ii].icon
        dest_panes[ii].notebook_id = src_panes[ii].notebook_id
        dest_panes[ii].transparent = src_panes[ii].transparent
        dest_panes[ii].snapped = src_panes[ii].snapped
        dest_panes[ii].minimize_mode = src_panes[ii].minimize_mode

    for ii in xrange(len(dest_docks)):
        dock = dest_docks[ii]
        for jj in xrange(len(dock.panes)):
            for kk in xrange(len(src_panes)):
                if dock.panes[jj] == src_panes[kk]:
                    dock.panes[jj] = dest_panes[kk]

        dest_docks[ii] = dock
        
    return dest_docks, dest_panes


def GetMaxLayer(docks, dock_direction):
    """
    This is an internal function which returns
    the highest layer inside the specified dock.

    :param `docks`: a list of L{AuiDockInfo};
    :param `dock_direction`: the L{AuiDockInfo} docking direction to analyze.
    """
    
    max_layer = 0

    for dock in docks:
        if dock.dock_direction == dock_direction and dock.dock_layer > max_layer and not dock.fixed:
            max_layer = dock.dock_layer
    
    return max_layer


def GetMaxRow(panes, dock_direction, dock_layer):
    """
    This is an internal function which returns
    the highest layer inside the specified dock.

    :param `panes`: a list of L{AuiPaneInfo};
    :param `dock_direction`: the L{AuiPaneInfo} docking direction to analyze;
    :param `dock_layer`: the L{AuiPaneInfo} layer to analyze.
    """
    
    max_row = 0

    for pane in panes:
        if pane.dock_direction == dock_direction and pane.dock_layer == dock_layer and \
           pane.dock_row > max_row:
            max_row = pane.dock_row
    
    return max_row


def DoInsertDockLayer(panes, dock_direction, dock_layer):
    """
    This is an internal function that inserts a new dock
    layer by incrementing all existing dock layer values by one.
    
    :param `panes`: a list of L{AuiPaneInfo};
    :param `dock_direction`: the L{AuiPaneInfo} docking direction to analyze;
    :param `dock_layer`: the L{AuiPaneInfo} layer to analyze.
    """
    
    for ii in xrange(len(panes)):
        pane = panes[ii]
        if not pane.IsFloating() and pane.dock_direction == dock_direction and pane.dock_layer >= dock_layer:
            pane.dock_layer = pane.dock_layer + 1

        panes[ii] = pane

    return panes


def DoInsertDockRow(panes, dock_direction, dock_layer, dock_row):
    """
    This is an internal function that inserts a new dock
    row by incrementing all existing dock row values by one.
    
    :param `panes`: a list of L{AuiPaneInfo};
    :param `dock_direction`: the L{AuiPaneInfo} docking direction to analyze;
    :param `dock_layer`: the L{AuiPaneInfo} layer to analyze;
    :param `dock_row`: the L{AuiPaneInfo} row to analyze.
    """
    
    for pane in panes:
        if not pane.IsFloating() and pane.dock_direction == dock_direction and \
           pane.dock_layer == dock_layer and pane.dock_row >= dock_row:
            pane.dock_row += 1

    return panes

    
def DoInsertPane(panes, dock_direction, dock_layer, dock_row, dock_pos):
    """
    This is an internal function that inserts a new pane
    by incrementing all existing dock position values by one.
    
    :param `panes`: a list of L{AuiPaneInfo};
    :param `dock_direction`: the L{AuiPaneInfo} docking direction to analyze;
    :param `dock_layer`: the L{AuiPaneInfo} layer to analyze;
    :param `dock_row`: the L{AuiPaneInfo} row to analyze;
    :param `dock_pos`: the L{AuiPaneInfo} row to analyze.
    """

    for ii in xrange(len(panes)):
        pane = panes[ii]
        if not pane.IsFloating() and pane.dock_direction == dock_direction and \
           pane.dock_layer == dock_layer and  pane.dock_row == dock_row and \
           pane.dock_pos >= dock_pos:
            pane.dock_pos = pane.dock_pos + 1

        panes[ii] = pane

    return panes


def FindDocks(docks, dock_direction, dock_layer=-1, dock_row=-1, reverse=False):
    """
    This is an internal function that returns a list of docks which meet
    the specified conditions in the parameters and returns a sorted array
    (sorted by layer and then row).
    
    :param `docks`: a list of L{AuiDockInfo};
    :param `dock_direction`: the L{AuiDockInfo} docking direction to analyze;
    :param `dock_layer`: the L{AuiDockInfo} layer to analyze;
    :param `dock_row`: the L{AuiDockInfo} row to analyze;
    """
    
    matchDocks = [(d.dock_layer, d.dock_row, d.dock_direction, d) for d in docks if \
                  (dock_direction == -1 or dock_direction == d.dock_direction) and \
                  ((dock_layer == -1 or dock_layer == d.dock_layer) and \
                  (dock_row == -1 or dock_row == d.dock_row))]
    
    arr = [x[-1] for x in sorted(matchDocks, reverse=reverse)]
    
    return arr


def FindOppositeDocks(docks, dock_direction):
    """
    This is an internal function that returns a list of docks
    which is related to the opposite direction.

    :param `docks`: a list of L{AuiDockInfo};
    :param `dock_direction`: the L{AuiDockInfo} docking direction to analyze;
    """

    if dock_direction == AUI_DOCK_LEFT:
        arr = FindDocks(docks, AUI_DOCK_RIGHT, -1, -1)
    elif dock_direction == AUI_DOCK_TOP:
        arr = FindDocks(docks, AUI_DOCK_BOTTOM, -1, -1)
    elif dock_direction == AUI_DOCK_RIGHT:
        arr = FindDocks(docks, AUI_DOCK_LEFT, -1, -1)
    elif dock_direction == AUI_DOCK_BOTTOM:
        arr = FindDocks(docks, AUI_DOCK_TOP, -1, -1)

    return arr    


def FindPaneInDock(dock, window):
    """
    This method looks up a specified window pointer inside a dock.
    If found, the corresponding L{AuiPaneInfo} pointer is returned, otherwise ``None``.

    :param `dock`: a L{AuiDockInfo} structure;
    :param `window`: a `wx.Window` derived window (associated to a pane).
    """

    for p in dock.panes:
        if p.window == window:
            return p
    
    return None


def GetToolBarDockOffsets(docks):
    """
    Returns the toolbar dock offsets (top-left and bottom-right).

    :param `docks`: a list of L{AuiDockInfo} to analyze.
    """

    top_left = wx.Size(0, 0)
    bottom_right = wx.Size(0, 0)

    for dock in docks:
        if dock.toolbar:
            dock_direction = dock.dock_direction
            if dock_direction == AUI_DOCK_LEFT:
                top_left.x += dock.rect.width
                bottom_right.x += dock.rect.width

            elif dock_direction == AUI_DOCK_TOP:
                top_left.y += dock.rect.height
                bottom_right.y += dock.rect.height

            elif dock_direction == AUI_DOCK_RIGHT:
                bottom_right.x += dock.rect.width
            
            elif dock_direction == AUI_DOCK_BOTTOM:
                bottom_right.y += dock.rect.height

    return top_left, bottom_right        
    

def GetInternalFrameRect(window, docks):
    """
    Returns the window rectangle excluding toolbars.

    :param `window`: a `wx.Window` derived window;
    :param `docks`: a list of L{AuiDockInfo} structures.
    """

    frameRect = wx.Rect()

    frameRect.SetTopLeft(window.ClientToScreen(window.GetClientAreaOrigin()))
    frameRect.SetSize(window.GetClientSize())

    top_left, bottom_right = GetToolBarDockOffsets(docks)

    # make adjustments for toolbars
    frameRect.x += top_left.x
    frameRect.y += top_left.y
    frameRect.width -= bottom_right.x
    frameRect.height -= bottom_right.y

    return frameRect


def CheckOutOfWindow(window, pt):
    """
    Checks if a point is outside the window rectangle.
    
    :param `window`: a `wx.Window` derived window;
    :param `pt`: a `wx.Point` object.
    """

    auiWindowMargin = 30
    marginRect = wx.Rect(*window.GetClientRect())
    marginRect.Inflate(auiWindowMargin, auiWindowMargin)

    return not marginRect.Contains(pt)


def CheckEdgeDrop(window, docks, pt):
    """
    Checks on which edge of a window the drop action has taken place.

    :param `window`: a `wx.Window` derived window;
    :param `docks`: a list of L{AuiDockInfo} structures;
    :param `pt`: a `wx.Point` object.
    """

    screenPt = window.ClientToScreen(pt)
    clientSize = window.GetClientSize()
    frameRect = GetInternalFrameRect(window, docks)

    if screenPt.y >= frameRect.GetTop() and screenPt.y < frameRect.GetBottom():
        if pt.x < auiLayerInsertOffset and pt.x > auiLayerInsertOffset - auiLayerInsertPixels:
            return wx.LEFT
        
        if pt.x >= clientSize.x - auiLayerInsertOffset and \
           pt.x < clientSize.x - auiLayerInsertOffset + auiLayerInsertPixels:
            return wx.RIGHT
        
    if screenPt.x >= frameRect.GetLeft() and screenPt.x < frameRect.GetRight():
        if pt.y < auiLayerInsertOffset and pt.y > auiLayerInsertOffset - auiLayerInsertPixels:
            return wx.TOP
        
        if pt.y >= clientSize.y - auiLayerInsertOffset and \
           pt.y < clientSize.y - auiLayerInsertOffset + auiLayerInsertPixels:
            return wx.BOTTOM

    return -1


def RemovePaneFromDocks(docks, pane, exc=None):
    """
    Removes a pane window from all docks
    with a possible exception specified by parameter `exc`.

    :param `docks`: a list of L{AuiDockInfo} structures;
    :param `pane`: the L{AuiPaneInfo} pane to be removed;
    :param `exc`: the possible pane exception.
    """
    
    for ii in xrange(len(docks)):
        d = docks[ii]
        if d == exc:
            continue
        pi = FindPaneInDock(d, pane.window)
        if pi:
            d.panes.remove(pi)

        docks[ii] = d            

    return docks


def RenumberDockRows(docks):
    """
    Takes a dock and assigns sequential numbers
    to existing rows.  Basically it takes out the gaps so if a
    dock has rows with numbers 0, 2, 5, they will become 0, 1, 2.

    :param `docks`: a list of L{AuiDockInfo} structures.    
    """
    
    for ii in xrange(len(docks)):
        dock = docks[ii]
        dock.dock_row = ii
        for jj in xrange(len(dock.panes)):
            dock.panes[jj].dock_row = ii

        docks[ii] = dock
        
    return docks


def SetActivePane(panes, active_pane):
    """
    Sets the active pane, as well as cycles through
    every other pane and makes sure that all others' active flags
    are turned off.

    :param `panes`: a list of L{AuiPaneInfo} structures;
    :param `active_pane`: the pane to be made active (if found).
    """

    for pane in panes:
        pane.state &= ~AuiPaneInfo.optionActive

    for pane in panes:
        if pane.window == active_pane and not pane.IsNotebookPage():
            pane.state |= AuiPaneInfo.optionActive
            return True, panes
            
    return False, panes
        

def ShowDockingGuides(guides, show):
    """
    Shows or hide the docking guide windows.

    :param `guides`: a list of L{AuiDockingGuideInfo} classes;
    :param `show`: whether to show or hide the docking guide windows.
    """

    for target in guides:
        
        if show and not target.host.IsShown():
            target.host.Show()
            target.host.Update()
        
        elif not show and target.host.IsShown():        
            target.host.Hide()
        

def RefreshDockingGuides(guides):
    """
    Refreshes the docking guide windows.

    :param `guides`: a list of L{AuiDockingGuideInfo} classes;
    """
    
    for target in guides:
        if target.host.IsShown():
            target.host.Refresh()
        
    
def PaneSortFunc(p1, p2):
    """
    This function is used to sort panes by dock position.

    :param `p1`: a L{AuiPaneInfo} instance;
    :param `p2`: another L{AuiPaneInfo} instance.    
    """
    
    return (p1.dock_pos < p2.dock_pos and [-1] or [1])[0]


def GetNotebookRoot(panes, notebook_id):
    """
    Returns the L{AuiPaneInfo} which has the specified `notebook_id`.

    :param `panes`: a list of L{AuiPaneInfo} instances;
    :param `notebook_id`: the target notebook id.
    """    

    for paneInfo in panes:
        if paneInfo.IsNotebookControl() and paneInfo.notebook_id == notebook_id:
            return paneInfo
        
    return None


def EscapeDelimiters(s):
    """
    Changes ``;`` into ``\`` and ``|`` into ``\|`` in the input string.  

    :param `s`: the string to be analyzed.

    :note: This is an internal functions which is used for saving perspectives.    
    """
    
    result = s.replace(";", "\\")
    result = result.replace("|", "|\\")
    
    return result


def IsDifferentDockingPosition(pane1, pane2):
    """
    Returns whether `pane1` and `pane2` are in a different docking position
    based on pane status, docking direction, docking layer and docking row.

    :param `pane1`: a L{AuiPaneInfo} instance;
    :param `pane2`: another L{AuiPaneInfo} instance.
    """

    return pane1.IsFloating() != pane2.IsFloating() or \
           pane1.dock_direction != pane2.dock_direction or \
           pane1.dock_layer != pane2.dock_layer or \
           pane1.dock_row != pane2.dock_row


# Convenience function
def AuiManager_HasLiveResize(manager):
    """
    Static function which returns if the input `manager` should have "live resize"
    behaviour.

    :param `manager`: an instance of L{AuiManager}.

    :note: This method always returns ``True`` on wxMac as this platform doesn't have
     the ability to use `wx.ScreenDC` to draw sashes.
    """

    # With Core Graphics on Mac, it's not possible to show sash feedback,
    # so we'll always use live update instead.
    
    if wx.Platform == "__WXMAC__":
        return True
    else:
        return (manager.GetAGWFlags() & AUI_MGR_LIVE_RESIZE) == AUI_MGR_LIVE_RESIZE


# Convenience function
def AuiManager_UseNativeMiniframes(manager):
    """
    Static function which returns if the input `manager` should use native `wx.MiniFrame` as
    floating panes.

    :param `manager`: an instance of L{AuiManager}.

    :note: This method always returns ``True`` on wxMac as this platform doesn't have
     the ability to use custom drawn miniframes.
    """

    # With Core Graphics on Mac, it's not possible to show sash feedback,
    # so we'll always use live update instead.
    
    if wx.Platform == "__WXMAC__":
        return True
    else:
        return (manager.GetAGWFlags() & AUI_MGR_USE_NATIVE_MINIFRAMES) == AUI_MGR_USE_NATIVE_MINIFRAMES


def GetManager(window):
    """
    This function will return the aui manager for a given window.
    
    :param `window`: this parameter should be any child window or grand-child
     window (and so on) of the frame/window managed by L{AuiManager}. The window
     does not need to be managed by the manager itself, nor does it even need
     to be a child or sub-child of a managed window. It must however be inside
     the window hierarchy underneath the managed window.
    """
    
    if not isinstance(wx.GetTopLevelParent(window), AuiFloatingFrame):
        if isinstance(window, auibar.AuiToolBar):
            return window.GetAuiManager()
    
    evt = AuiManagerEvent(wxEVT_AUI_FIND_MANAGER)
    evt.SetManager(None)
    evt.ResumePropagation(wx.EVENT_PROPAGATE_MAX)

    if not window.GetEventHandler().ProcessEvent(evt):
        return None

    return evt.GetManager()


# ---------------------------------------------------------------------------- #

class AuiManager(wx.EvtHandler):
    """
    AuiManager manages the panes associated with it for a particular `wx.Frame`,
    using a pane's L{AuiPaneInfo} information to determine each pane's docking and
    floating behavior. L{AuiManager} uses wxPython's sizer mechanism to plan the
    layout of each frame. It uses a replaceable dock art class to do all drawing,
    so all drawing is localized in one area, and may be customized depending on an
    applications' specific needs.

    L{AuiManager} works as follows: the programmer adds panes to the class, or makes
    changes to existing pane properties (dock position, floating state, show state, etc...).
    To apply these changes, the L{AuiManager.Update} function is called. This batch
    processing can be used to avoid flicker, by modifying more than one pane at a time,
    and then "committing" all of the changes at once by calling `Update()`.

    Panes can be added quite easily::

        text1 = wx.TextCtrl(self, -1)
        text2 = wx.TextCtrl(self, -1)
        self._mgr.AddPane(text1, AuiPaneInfo().Left().Caption("Pane Number One"))
        self._mgr.AddPane(text2, AuiPaneInfo().Bottom().Caption("Pane Number Two"))

        self._mgr.Update()


    Later on, the positions can be modified easily. The following will float an
    existing pane in a tool window::

        self._mgr.GetPane(text1).Float()


    **Layers, Rows and Directions, Positions:**
    
    Inside AUI, the docking layout is figured out by checking several pane parameters.
    Four of these are important for determining where a pane will end up.

    **Direction** - Each docked pane has a direction, `Top`, `Bottom`, `Left`, `Right`, or `Center`.
    This is fairly self-explanatory. The pane will be placed in the location specified
    by this variable.

    **Position** - More than one pane can be placed inside of a "dock". Imagine two panes
    being docked on the left side of a window. One pane can be placed over another.
    In proportionally managed docks, the pane position indicates it's sequential position,
    starting with zero. So, in our scenario with two panes docked on the left side, the
    top pane in the dock would have position 0, and the second one would occupy position 1. 

    **Row** - A row can allow for two docks to be placed next to each other. One of the most
    common places for this to happen is in the toolbar. Multiple toolbar rows are allowed,
    the first row being in row 0, and the second in row 1. Rows can also be used on
    vertically docked panes. 

    **Layer** - A layer is akin to an onion. Layer 0 is the very center of the managed pane.
    Thus, if a pane is in layer 0, it will be closest to the center window (also sometimes
    known as the "content window"). Increasing layers "swallow up" all layers of a lower
    value. This can look very similar to multiple rows, but is different because all panes
    in a lower level yield to panes in higher levels. The best way to understand layers
    is by running the AUI sample (`AUI.py`).
    """

    def __init__(self, managed_window=None, agwFlags=None):
        """
        Default class constructor.
        
        :param `managed_window`: specifies the window which should be managed;
        :param `agwFlags`: specifies options which allow the frame management behavior to be
         modified. `agwFlags` can be a combination of the following style bits:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_MGR_ALLOW_FLOATING``           Allow floating of panes
         ``AUI_MGR_ALLOW_ACTIVE_PANE``        If a pane becomes active, "highlight" it in the interface
         ``AUI_MGR_TRANSPARENT_DRAG``         If the platform supports it, set transparency on a floating pane while it is dragged by the user
         ``AUI_MGR_TRANSPARENT_HINT``         If the platform supports it, show a transparent hint window when the user is about to dock a floating pane
         ``AUI_MGR_VENETIAN_BLINDS_HINT``     Show a "venetian blind" effect when the user is about to dock a floating pane
         ``AUI_MGR_RECTANGLE_HINT``           Show a rectangle hint effect when the user is about to dock a floating pane
         ``AUI_MGR_HINT_FADE``                If the platform supports it, the hint window will fade in and out
         ``AUI_MGR_NO_VENETIAN_BLINDS_FADE``  Disables the "venetian blind" fade in and out
         ``AUI_MGR_LIVE_RESIZE``              Live resize when the user drag a sash
         ``AUI_MGR_ANIMATE_FRAMES``           Fade-out floating panes when they are closed (all platforms which support frames transparency) and show a moving rectangle when they are docked (Windows < Vista and GTK only)
         ``AUI_MGR_AERO_DOCKING_GUIDES``      Use the new Aero-style bitmaps as docking guides
         ``AUI_MGR_PREVIEW_MINIMIZED_PANES``  Slide in and out minimized panes to preview them
         ``AUI_MGR_WHIDBEY_DOCKING_GUIDES``   Use the new Whidbey-style bitmaps as docking guides
         ``AUI_MGR_SMOOTH_DOCKING``           Performs a "smooth" docking of panes (a la PyQT)
         ``AUI_MGR_USE_NATIVE_MINIFRAMES``    Use miniframes with native caption bar as floating panes instead or custom drawn caption bars (forced on wxMac)
         ``AUI_MGR_AUTONB_NO_CAPTION``        Panes that merge into an automatic notebook will not have the pane caption visible
         ==================================== ==================================

         Default value for `agwFlags` is:
         ``AUI_MGR_DEFAULT`` = ``AUI_MGR_ALLOW_FLOATING`` | ``AUI_MGR_TRANSPARENT_HINT`` | ``AUI_MGR_HINT_FADE`` | ``AUI_MGR_NO_VENETIAN_BLINDS_FADE``

         :note: If using the ``AUI_MGR_USE_NATIVE_MINIFRAMES``, double-clicking on a
          floating pane caption will not re-dock the pane, but simply maximize it (if
          L{AuiPaneInfo.MaximizeButton} has been set to ``True``) or do nothing.
        """

        wx.EvtHandler.__init__(self)
        
        self._action = actionNone
        self._action_window = None
        self._hover_button = None
        self._art = dockart.AuiDefaultDockArt()
        self._hint_window = None
        self._active_pane = None
        self._has_maximized = False
        self._has_minimized = False

        self._frame = None
        self._dock_constraint_x = 0.3
        self._dock_constraint_y = 0.3
        self._reserved = None
    
        self._panes = []
        self._docks = []
        self._uiparts = []
        
        self._guides = []
        self._notebooks = []

        self._masterManager = None
        self._currentDragItem = -1
        self._lastknowndocks = {}

        self._hint_fadetimer = wx.Timer(self, wx.ID_ANY)
        self._hint_fademax = 50
        self._last_hint = wx.Rect()

        self._from_move = False
        self._last_rect = wx.Rect()
        
        if agwFlags is None:
            agwFlags = AUI_MGR_DEFAULT
            
        self._agwFlags = agwFlags
        self._is_docked = (False, wx.RIGHT, wx.TOP, 0)
        self._snap_limits = (15, 15)

        if wx.Platform == "__WXMSW__":
            self._animation_step = 30.0
        else:
            self._animation_step = 5.0

        self._hint_rect = wx.Rect()

        self._preview_timer = wx.Timer(self, wx.ID_ANY)
        self._sliding_frame = None

        self._autoNBTabArt = tabart.AuiDefaultTabArt()
        self._autoNBStyle = AUI_NB_DEFAULT_STYLE | AUI_NB_BOTTOM | \
                            AUI_NB_SUB_NOTEBOOK | AUI_NB_TAB_EXTERNAL_MOVE
        self._autoNBStyle -= AUI_NB_DRAW_DND_TAB

        if managed_window:
            self.SetManagedWindow(managed_window)

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_SET_CURSOR, self.OnSetCursor)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDClick)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_LEAVE_WINDOW, self.OnLeaveWindow)
        self.Bind(wx.EVT_CHILD_FOCUS, self.OnChildFocus)
        self.Bind(wx.EVT_MOUSE_CAPTURE_LOST, self.OnCaptureLost)
        self.Bind(wx.EVT_TIMER, self.OnHintFadeTimer, self._hint_fadetimer)
        self.Bind(wx.EVT_TIMER, self.SlideIn, self._preview_timer)

        self.Bind(wx.EVT_MOVE, self.OnMove)
        self.Bind(wx.EVT_SYS_COLOUR_CHANGED, self.OnSysColourChanged)
        
        self.Bind(EVT_AUI_PANE_BUTTON, self.OnPaneButton)
        self.Bind(EVT_AUI_RENDER, self.OnRender)
        self.Bind(EVT_AUI_FIND_MANAGER, self.OnFindManager)
        self.Bind(EVT_AUI_PANE_MIN_RESTORE, self.OnRestoreMinimizedPane)
        self.Bind(EVT_AUI_PANE_DOCKED, self.OnPaneDocked)

        self.Bind(auibook.EVT_AUINOTEBOOK_BEGIN_DRAG, self.OnTabBeginDrag)
        self.Bind(auibook.EVT_AUINOTEBOOK_PAGE_CLOSE, self.OnTabPageClose)
        self.Bind(auibook.EVT_AUINOTEBOOK_PAGE_CHANGED, self.OnTabSelected)
        

    def CreateFloatingFrame(self, parent, pane_info):
        """
        Creates a floating frame for the windows.

        :param `parent`: the floating frame parent;
        :param `pane_info`: the L{AuiPaneInfo} class with all the pane's information.
        """

        return AuiFloatingFrame(parent, self, pane_info)


    def CanDockPanel(self, p):
        """
        Returns whether a pane can be docked or not.

        :param `p`: the L{AuiPaneInfo} class with all the pane's information.
        """        

        # is the pane dockable?
        if not p.IsDockable():
            return False

        # if a key modifier is pressed while dragging the frame,
        # don't dock the window
        return not (wx.GetKeyState(wx.WXK_CONTROL) or wx.GetKeyState(wx.WXK_ALT))


    def GetPaneByWidget(self, window):
        """
        This version of L{GetPane} looks up a pane based on a
        'pane window'.

        :param `window`: a `wx.Window` derived window.

        :see: L{GetPane}
        """

        for p in self._panes:
            if p.window == window:
                return p

        return NonePaneInfo


    def GetPaneByName(self, name):
        """
        This version of L{GetPane} looks up a pane based on a
        'pane name'.

        :param `name`: the pane name.

        :see: L{GetPane}        
        """
        
        for p in self._panes:
            if p.name == name:
                return p
        
        return NonePaneInfo


    def GetPane(self, item):
        """
        Looks up a L{AuiPaneInfo} structure based
        on the supplied window pointer. Upon failure, L{GetPane}
        returns an empty L{AuiPaneInfo}, a condition which can be checked
        by calling L{AuiPaneInfo.IsOk}.

        The pane info's structure may then be modified. Once a pane's
        info is modified, L{Update} must be called to
        realize the changes in the UI.

        :param `item`: either a pane name or a `wx.Window`.        
        """

        if isinstance(item, basestring):
            return self.GetPaneByName(item)
        else:
            return self.GetPaneByWidget(item)


    def GetAllPanes(self):
        """ Returns a reference to all the pane info structures. """
        
        return self._panes


    def ShowPane(self, window, show):
        """
        Shows or hides a pane based on the window passed as input.

        :param `window`: a `wx.Window` derived window;
        :param `show`: ``True`` to show the pane, ``False`` otherwise.
        """

        p = self.GetPane(window)
        
        if p.IsOk():
            if p.IsNotebookPage():
                if show:
                
                    notebook = self._notebooks[p.notebook_id]
                    id = notebook.GetPageIndex(p.window)
                    if id >= 0:
                        notebook.SetSelection(id)
                    self.ShowPane(notebook, True)
                
            else:
                p.Show(show)
                
            if p.frame:
                p.frame.Raise()
                
            self.Update()

            
    def HitTest(self, x, y):
        """
        This is an internal function which determines
        which UI item the specified coordinates are over.
        
        :param `x`: specifies a x position in client coordinates;
        :param `y`: specifies a y position in client coordinates.
        """

        result = None

        for item in self._uiparts:
            # we are not interested in typeDock, because this space 
            # isn't used to draw anything, just for measurements
            # besides, the entire dock area is covered with other
            # rectangles, which we are interested in.
            if item.type == AuiDockUIPart.typeDock:
                continue

            # if we already have a hit on a more specific item, we are not
            # interested in a pane hit.  If, however, we don't already have
            # a hit, returning a pane hit is necessary for some operations
            if item.type in [AuiDockUIPart.typePane, AuiDockUIPart.typePaneBorder] and result:
                continue
        
            # if the point is inside the rectangle, we have a hit
            if item.rect.Contains((x, y)):
                result = item
        
        return result


    def PaneHitTest(self, panes, pt):
        """
        Similar to L{HitTest}, but it checks in which L{AuiPaneInfo} rectangle the
        input point belongs to.

        :param `panes`: a list of L{AuiPaneInfo} instances;
        :param `pt`: a `wx.Point` object.
        """

        for paneInfo in panes:
            if paneInfo.IsDocked() and paneInfo.IsShown() and paneInfo.rect.Contains(pt):
                return paneInfo

        return NonePaneInfo


    # SetAGWFlags() and GetAGWFlags() allow the owner to set various
    # options which are global to AuiManager

    def SetAGWFlags(self, agwFlags):
        """
        This method is used to specify L{AuiManager}'s settings flags.

        :param `agwFlags`: specifies options which allow the frame management behavior
         to be modified. `agwFlags` can be one of the following style bits:

         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_MGR_ALLOW_FLOATING``           Allow floating of panes
         ``AUI_MGR_ALLOW_ACTIVE_PANE``        If a pane becomes active, "highlight" it in the interface
         ``AUI_MGR_TRANSPARENT_DRAG``         If the platform supports it, set transparency on a floating pane while it is dragged by the user
         ``AUI_MGR_TRANSPARENT_HINT``         If the platform supports it, show a transparent hint window when the user is about to dock a floating pane
         ``AUI_MGR_VENETIAN_BLINDS_HINT``     Show a "venetian blind" effect when the user is about to dock a floating pane
         ``AUI_MGR_RECTANGLE_HINT``           Show a rectangle hint effect when the user is about to dock a floating pane
         ``AUI_MGR_HINT_FADE``                If the platform supports it, the hint window will fade in and out
         ``AUI_MGR_NO_VENETIAN_BLINDS_FADE``  Disables the "venetian blind" fade in and out
         ``AUI_MGR_LIVE_RESIZE``              Live resize when the user drag a sash
         ``AUI_MGR_ANIMATE_FRAMES``           Fade-out floating panes when they are closed (all platforms which support frames transparency) and show a moving rectangle when they are docked (Windows < Vista and GTK only)
         ``AUI_MGR_AERO_DOCKING_GUIDES``      Use the new Aero-style bitmaps as docking guides
         ``AUI_MGR_PREVIEW_MINIMIZED_PANES``  Slide in and out minimized panes to preview them
         ``AUI_MGR_WHIDBEY_DOCKING_GUIDES``   Use the new Whidbey-style bitmaps as docking guides        
         ``AUI_MGR_SMOOTH_DOCKING``           Performs a "smooth" docking of panes (a la PyQT)
         ``AUI_MGR_USE_NATIVE_MINIFRAMES``    Use miniframes with native caption bar as floating panes instead or custom drawn caption bars (forced on wxMac)
         ``AUI_MGR_AUTONB_NO_CAPTION``        Panes that merge into an automatic notebook will not have the pane caption visible
         ==================================== ==================================

         :note: If using the ``AUI_MGR_USE_NATIVE_MINIFRAMES``, double-clicking on a
          floating pane caption will not re-dock the pane, but simply maximize it (if
          L{AuiPaneInfo.MaximizeButton} has been set to ``True``) or do nothing.
        
        """
        
        self._agwFlags = agwFlags

        if len(self._guides) > 0:
            self.CreateGuideWindows()

        if self._hint_window and agwFlags & AUI_MGR_RECTANGLE_HINT == 0:
            self.CreateHintWindow()


    def GetAGWFlags(self):
        """
        Returns the current manager's flags.

        :see: L{SetAGWFlags} for a list of possible L{AuiManager} flags.
        """
        
        return self._agwFlags
        

    def SetManagedWindow(self, managed_window):
        """
        Called to specify the frame or window which is to be managed by L{AuiManager}.
        Frame management is not restricted to just frames. Child windows or custom
        controls are also allowed.

        :param `managed_window`: specifies the window which should be managed by
         the AUI manager.
        """

        if not managed_window:
            raise Exception("Specified managed window must be non-null. ")
        
        self._frame = managed_window
        self._frame.PushEventHandler(self)

        # if the owner is going to manage an MDI parent frame,
        # we need to add the MDI client window as the default
        # center pane

        if isinstance(self._frame, wx.MDIParentFrame):
            mdi_frame = self._frame
            client_window = mdi_frame.GetClientWindow()

            if not client_window:
                raise Exception("Client window is None!")

            self.AddPane(client_window, AuiPaneInfo().Name("mdiclient").
                         CenterPane().PaneBorder(False))

        elif isinstance(self._frame, tabmdi.AuiMDIParentFrame):

            mdi_frame = self._frame
            client_window = mdi_frame.GetClientWindow()

            if not client_window:
                raise Exception("Client window is None!")

            self.AddPane(client_window, AuiPaneInfo().Name("mdiclient").
                         CenterPane().PaneBorder(False))


    def GetManagedWindow(self):
        """ Returns the window being managed by L{AuiManager}. """
        
        return self._frame


    def SetFrame(self, managed_window):
        """
        Called to specify the frame or window which is to be managed by L{AuiManager}.
        Frame management is not restricted to just frames. Child windows or custom
        controls are also allowed.

        :param `managed_window`: specifies the window which should be managed by
         the AUI manager.

        :warning: This method is now deprecated, use L{SetManagedWindow} instead.
        """

        DeprecationWarning("This method is deprecated, use SetManagedWindow instead.")
        return self.SetManagedWindow(managed_window)
    
        
    def GetFrame(self):
        """
        Returns the window being managed by L{AuiManager}.

        :warning: This method is now deprecated, use L{GetManagedWindow} instead.
        """

        DeprecationWarning("This method is deprecated, use GetManagedWindow instead.")        
        return self._frame


    def CreateGuideWindows(self):
        """ Creates the VS2005 HUD guide windows. """

        self.DestroyGuideWindows()

        self._guides.append(AuiDockingGuideInfo().Left().
                            Host(AuiSingleDockingGuide(self._frame, wx.LEFT)))
        self._guides.append(AuiDockingGuideInfo().Top().
                            Host(AuiSingleDockingGuide(self._frame, wx.TOP)))
        self._guides.append(AuiDockingGuideInfo().Right().
                            Host(AuiSingleDockingGuide(self._frame, wx.RIGHT)))
        self._guides.append(AuiDockingGuideInfo().Bottom().
                            Host(AuiSingleDockingGuide(self._frame, wx.BOTTOM)))
        self._guides.append(AuiDockingGuideInfo().Centre().
                            Host(AuiCenterDockingGuide(self._frame)))


    def DestroyGuideWindows(self):
        """ Destroys the VS2005 HUD guide windows. """

        for guide in self._guides:
            if guide.host:
                guide.host.Destroy()
        
        self._guides = []
    

    def CreateHintWindow(self):
        """ Creates the standard wxAUI hint window. """

        self.DestroyHintWindow()

        self._hint_window = AuiDockingHintWindow(self._frame)
        self._hint_window.SetBlindMode(self._agwFlags)


    def DestroyHintWindow(self):
        """ Destroys the standard wxAUI hint window. """

        if self._hint_window:

            self._hint_window.Destroy()
            self._hint_window = None


    def UnInit(self):
        """
        Uninitializes the framework and should be called before a managed frame or
        window is destroyed. L{UnInit} is usually called in the managed `wx.Frame`/`wx.Window`
        destructor.

        It is necessary to call this function before the managed frame or window is
        destroyed, otherwise the manager cannot remove its custom event handlers
        from a window.
        """

        if self._frame:
            self._frame.RemoveEventHandler(self)


    def GetArtProvider(self):
        """ Returns the current art provider being used. """
        
        return self._art


    def ProcessMgrEvent(self, event):
        """
        Process the AUI events sent to the manager.

        :param `event`: the event to process, an instance of L{AuiManagerEvent}.
        """

        # first, give the owner frame a chance to override
        if self._frame:
            if self._frame.GetEventHandler().ProcessEvent(event):
                return
        
        self.ProcessEvent(event)


    def FireEvent(self, evtType, pane, canVeto=False):
        """
        Fires one of the ``EVT_AUI_PANE_FLOATED``/``FLOATING``/``DOCKING``/``DOCKED``/``ACTIVATED`` event. 

        :param `evtType`: one of the aforementioned events;
        :param `pane`: the L{AuiPaneInfo} instance associated to this event;
        :param `canVeto`: whether the event can be vetoed or not.
        """        

        event = AuiManagerEvent(evtType)
        event.SetPane(pane)
        event.SetCanVeto(canVeto)
        self.ProcessMgrEvent(event)

        return event

    
    def CanUseModernDockArt(self):
        """
        Returns whether L{ModernDockArt} can be used (Windows XP / Vista / 7 only,
        requires Mark Hammonds's `pywin32` package).
        """

        if not _winxptheme:
            return False

        # Get the size of a small close button (themed)
        hwnd = self._frame.GetHandle()
        hTheme = winxptheme.OpenThemeData(hwnd, "Window")

        if not hTheme:
            return False

        return True
            
    
    def SetArtProvider(self, art_provider):
        """
        Instructs L{AuiManager} to use art provider specified by the parameter
        `art_provider` for all drawing calls. This allows plugable look-and-feel
        features.

        :param `art_provider`: a AUI dock art provider.

        :note: The previous art provider object, if any, will be deleted by L{AuiManager}.
        """

        # delete the last art provider, if any
        del self._art
        
        # assign the new art provider
        self._art = art_provider

        for pane in self.GetAllPanes():
            if pane.IsFloating() and pane.frame:                
                pane.frame._mgr.SetArtProvider(art_provider)
                pane.frame._mgr.Update()


    def AddPane(self, window, arg1=None, arg2=None, target=None):
        """
        Tells the frame manager to start managing a child window. There
        are four versions of this function. The first verison allows the full spectrum
        of pane parameter possibilities (L{AddPane1}). The second version is used for
        simpler user interfaces which do not require as much configuration (L{AddPane2}).
        The L{AddPane3} version allows a drop position to be specified, which will determine
        where the pane will be added. The L{AddPane4} version allows to turn the target
        L{AuiPaneInfo} pane into a notebook and the added pane into a page.

        In wxPython, simply call L{AddPane}.

        :param `window`: the child window to manage;
        :param `arg1`: a L{AuiPaneInfo} or an integer value (direction);
        :param `arg2`: a L{AuiPaneInfo} or a `wx.Point` (drop position);
        :param `target`: a L{AuiPaneInfo} to be turned into a notebook
         and new pane added to it as a page. (additionally, target can be any pane in 
         an existing notebook)
         """
 
        if target in self._panes:
            return self.AddPane4(window, arg1, target)

        if type(arg1) == type(1):
            # This Is Addpane2
            if arg1 is None:
                arg1 = wx.LEFT
            if arg2 is None:
                arg2 = ""
            return self.AddPane2(window, arg1, arg2)
        else:
            if isinstance(arg2, wx.Point):
                return self.AddPane3(window, arg1, arg2)
            else:
                return self.AddPane1(window, arg1)
        

    def AddPane1(self, window, pane_info):
        """ See comments on L{AddPane}. """

        # check if the pane has a valid window
        if not window:
            return False

        # check if the pane already exists
        if self.GetPane(pane_info.window).IsOk():
            return False

        # check if the pane name already exists, this could reveal a
        # bug in the library user's application
        already_exists = False
        if pane_info.name != "" and self.GetPane(pane_info.name).IsOk():
            warnings.warn("A pane with the name '%s' already exists in the manager!"%pane_info.name)
            already_exists = True

        # if the new pane is docked then we should undo maximize
        if pane_info.IsDocked():
            self.RestoreMaximizedPane()

        self._panes.append(pane_info)
        pinfo = self._panes[-1]

        # set the pane window
        pinfo.window = window

        # if the pane's name identifier is blank, create a random string
        if pinfo.name == "" or already_exists:
            pinfo.name = ("%s%08x%08x%08x")%(pinfo.window.GetName(), time.time(),
                                             time.clock(), len(self._panes))

        # set initial proportion (if not already set)
        if pinfo.dock_proportion == 0:
            pinfo.dock_proportion = 100000

        floating = isinstance(self._frame, AuiFloatingFrame)

        pinfo.buttons = []

        if not floating and pinfo.HasMinimizeButton():
            button = AuiPaneButton(AUI_BUTTON_MINIMIZE)
            pinfo.buttons.append(button)
    
        if not floating and pinfo.HasMaximizeButton():
            button = AuiPaneButton(AUI_BUTTON_MAXIMIZE_RESTORE)
            pinfo.buttons.append(button)

        if not floating and pinfo.HasPinButton():
            button = AuiPaneButton(AUI_BUTTON_PIN)
            pinfo.buttons.append(button)

        if pinfo.HasCloseButton():
            button = AuiPaneButton(AUI_BUTTON_CLOSE)
            pinfo.buttons.append(button)

        if pinfo.HasGripper():
            if isinstance(pinfo.window, auibar.AuiToolBar):
                # prevent duplicate gripper -- both AuiManager and AuiToolBar
                # have a gripper control.  The toolbar's built-in gripper
                # meshes better with the look and feel of the control than ours,
                # so turn AuiManager's gripper off, and the toolbar's on.

                tb = pinfo.window
                pinfo.SetFlag(AuiPaneInfo.optionGripper, False)
                tb.SetGripperVisible(True)

        if pinfo.window:
            if pinfo.best_size == wx.Size(-1, -1):
                pinfo.best_size = pinfo.window.GetClientSize()

            if isinstance(pinfo.window, wx.ToolBar):
                # GetClientSize() doesn't get the best size for
                # a toolbar under some newer versions of wxWidgets,
                # so use GetBestSize()
                pinfo.best_size = pinfo.window.GetBestSize()

                # this is needed for Win2000 to correctly fill toolbar backround
                # it should probably be repeated once system colour change happens
                if wx.Platform == "__WXMSW__" and pinfo.window.UseBgCol():
                    pinfo.window.SetBackgroundColour(self.GetArtProvider().GetColour(AUI_DOCKART_BACKGROUND_COLOUR))
                
            if pinfo.min_size != wx.Size(-1, -1):
                if pinfo.best_size.x < pinfo.min_size.x:
                    pinfo.best_size.x = pinfo.min_size.x
                if pinfo.best_size.y < pinfo.min_size.y:
                    pinfo.best_size.y = pinfo.min_size.y

        self._panes[-1] = pinfo
        if isinstance(window, auibar.AuiToolBar):
            window.SetAuiManager(self)

        return True


    def AddPane2(self, window, direction, caption):
        """ See comments on L{AddPane}. """
        
        pinfo = AuiPaneInfo()
        pinfo.Caption(caption)
        
        if direction == wx.TOP:
            pinfo.Top()
        elif direction == wx.BOTTOM:
            pinfo.Bottom()
        elif direction == wx.LEFT:
            pinfo.Left()
        elif direction == wx.RIGHT:
            pinfo.Right()
        elif direction == wx.CENTER:
            pinfo.CenterPane()
        
        return self.AddPane(window, pinfo)


    def AddPane3(self, window, pane_info, drop_pos):
        """ See comments on L{AddPane}. """
        
        if not self.AddPane(window, pane_info):
            return False

        pane = self.GetPane(window)
        indx = self._panes.index(pane)

        ret, pane = self.DoDrop(self._docks, self._panes, pane, drop_pos, wx.Point(0, 0))
        self._panes[indx] = pane

        return True


    def AddPane4(self, window, pane_info, target):
        """ See comments on L{AddPane}. """
        
        if not self.AddPane(window, pane_info):
            return False
               
        paneInfo = self.GetPane(window)
        
        if not paneInfo.IsNotebookDockable():
            return self.AddPane1(window, pane_info)
        if not target.IsNotebookDockable() and not target.IsNotebookControl():
            return self.AddPane1(window, pane_info)

        if not target.HasNotebook():
            self.CreateNotebookBase(self._panes, target)
        
        # Add new item to notebook
        paneInfo.NotebookPage(target.notebook_id)

        # we also want to remove our captions sometimes
        self.RemoveAutoNBCaption(paneInfo)
        self.UpdateNotebook()
        
        return True


    def InsertPane(self, window, pane_info, insert_level=AUI_INSERT_PANE):
        """
        This method is used to insert either a previously unmanaged pane window
        into the frame manager, or to insert a currently managed pane somewhere else.
        L{InsertPane} will push all panes, rows, or docks aside and insert the window
        into the position specified by `pane_info`.

        Because `pane_info` can specify either a pane, dock row, or dock layer, the
        `insert_level` parameter is used to disambiguate this. The parameter `insert_level`
        can take a value of ``AUI_INSERT_PANE``, ``AUI_INSERT_ROW`` or ``AUI_INSERT_DOCK``.

        :param `window`: the window to be inserted and managed;
        :param `pane_info`: the insert location for the new window;
        :param `insert_level`: the insertion level of the new pane.
        """

        if not window:
            raise Exception("Invalid window passed to InsertPane.")
                            
        # shift the panes around, depending on the insert level
        if insert_level == AUI_INSERT_PANE:
            self._panes = DoInsertPane(self._panes, pane_info.dock_direction,
                                       pane_info.dock_layer, pane_info.dock_row,
                                       pane_info.dock_pos)

        elif insert_level == AUI_INSERT_ROW:
            self._panes = DoInsertDockRow(self._panes, pane_info.dock_direction,
                                          pane_info.dock_layer, pane_info.dock_row)

        elif insert_level == AUI_INSERT_DOCK:
            self._panes = DoInsertDockLayer(self._panes, pane_info.dock_direction,
                                            pane_info.dock_layer)
        
        # if the window already exists, we are basically just moving/inserting the
        # existing window.  If it doesn't exist, we need to add it and insert it
        existing_pane = self.GetPane(window)
        indx = self._panes.index(existing_pane)
        
        if not existing_pane.IsOk():
        
            return self.AddPane(window, pane_info)
        
        else:
        
            if pane_info.IsFloating():
                existing_pane.Float()
                if pane_info.floating_pos != wx.Point(-1, -1):
                    existing_pane.FloatingPosition(pane_info.floating_pos)
                if pane_info.floating_size != wx.Size(-1, -1):
                    existing_pane.FloatingSize(pane_info.floating_size)
            else:
                # if the new pane is docked then we should undo maximize
                self.RestoreMaximizedPane()

                existing_pane.Direction(pane_info.dock_direction)
                existing_pane.Layer(pane_info.dock_layer)
                existing_pane.Row(pane_info.dock_row)
                existing_pane.Position(pane_info.dock_pos)

            self._panes[indx] = existing_pane                
            
        return True

    
    def DetachPane(self, window):
        """
        Tells the L{AuiManager} to stop managing the pane specified
        by `window`. The window, if in a floated frame, is reparented to the frame
        managed by L{AuiManager}.

        :param `window`: the window to be un-managed.
        """
        
        for p in self._panes:
            if p.window == window:
                if p.frame:
                    # we have a floating frame which is being detached. We need to
                    # reparent it to self._frame and destroy the floating frame

                    # reduce flicker
                    p.window.SetSize((1, 1))
                    if p.frame.IsShown():
                        p.frame.Show(False)

                    if self._action_window == p.frame:
                        self._action_window = None
                        
                    # reparent to self._frame and destroy the pane
                    p.window.Reparent(self._frame)
                    p.frame.SetSizer(None)
                    p.frame.Destroy()
                    p.frame = None

                elif p.IsNotebookPage():
                    notebook = self._notebooks[p.notebook_id]
                    id = notebook.GetPageIndex(p.window)
                    notebook.RemovePage(id)
                
                # make sure there are no references to this pane in our uiparts,
                # just in case the caller doesn't call Update() immediately after
                # the DetachPane() call.  This prevets obscure crashes which would
                # happen at window repaint if the caller forgets to call Update()
                counter = 0
                for pi in xrange(len(self._uiparts)):
                    part = self._uiparts[counter]
                    if part.pane == p:
                        self._uiparts.pop(counter)
                        counter -= 1

                    counter += 1
            
                self._panes.remove(p)
                return True
        
        return False


    def ClosePane(self, pane_info):
        """
        Destroys or hides the pane depending on its flags.

        :param `pane_info`: a L{AuiPaneInfo} instance.
        """

        # if we were maximized, restore
        if pane_info.IsMaximized():
            self.RestorePane(pane_info)

        if pane_info.frame:
            if self._agwFlags & AUI_MGR_ANIMATE_FRAMES:
                pane_info.frame.FadeOut()

        # first, hide the window
        if pane_info.window and pane_info.window.IsShown():
            pane_info.window.Show(False)

        # make sure that we are the parent of this window
        if pane_info.window and pane_info.window.GetParent() != self._frame:
            pane_info.window.Reparent(self._frame)

        # if we have a frame, destroy it
        if pane_info.frame:
            pane_info.frame.Destroy()
            pane_info.frame = None
            
        elif pane_info.IsNotebookPage():
            # if we are a notebook page, remove ourselves...
            # the  code would index out of bounds 
            # if the last page of a sub-notebook was closed
            # because the notebook would be deleted, before this
            # code is executed.
            # This code just prevents an out-of bounds error.
            if self._notebooks:
                nid = pane_info.notebook_id
                if nid >= 0 and nid < len(self._notebooks):
                    notebook = self._notebooks[nid]
                    page_idx = notebook.GetPageIndex(pane_info.window)
                    if page_idx >= 0:
                        notebook.RemovePage(page_idx)
                                
        # now we need to either destroy or hide the pane
        to_destroy = 0
        if pane_info.IsDestroyOnClose():
            to_destroy = pane_info.window
            self.DetachPane(to_destroy)
        else:
            if isinstance(pane_info.window, auibar.AuiToolBar) and pane_info.IsFloating():
                tb = pane_info.window
                if pane_info.dock_direction in [AUI_DOCK_LEFT, AUI_DOCK_RIGHT]:
                    tb.SetAGWWindowStyleFlag(tb.GetAGWWindowStyleFlag() | AUI_TB_VERTICAL)
                
            pane_info.Dock().Hide()

        if pane_info.IsNotebookControl():

            notebook = self._notebooks[pane_info.notebook_id]
            while notebook.GetPageCount():
                window = notebook.GetPage(0)
                notebook.RemovePage(0)
                info = self.GetPane(window)
                if info.IsOk():
                    info.notebook_id = -1
                    info.dock_direction = AUI_DOCK_NONE
                    # Note: this could change our paneInfo reference ...
                    self.ClosePane(info)

        if to_destroy:
            to_destroy.Destroy()


    def MaximizePane(self, pane_info, savesizes=True):
        """
        Maximizes the input pane.

        :param `pane_info`: a L{AuiPaneInfo} instance.
        :param `savesizes`: whether to save previous dock sizes.
        """

        if savesizes:
            self.SavePreviousDockSizes(pane_info)
                
        for p in self._panes:
            
            # save hidden state
            p.SetFlag(p.savedHiddenState, p.HasFlag(p.optionHidden))

            if not p.IsToolbar() and not p.IsFloating():
                p.Restore()
        
                # hide the pane, because only the newly
                # maximized pane should show
                p.Hide()

        pane_info.previousDockPos = pane_info.dock_pos

        # mark ourselves maximized
        pane_info.Maximize()
        pane_info.Show()
        self._has_maximized = True

        # last, show the window
        if pane_info.window and not pane_info.window.IsShown():
            pane_info.window.Show(True)

            
    def SavePreviousDockSizes(self, pane_info):
        """
        Stores the previous dock sizes, to be used in a "restore" action later.

        :param `pane_info`: a L{AuiPaneInfo} instance.
        """

        for d in self._docks:
            if not d.toolbar:
                for p in d.panes:
                    p.previousDockSize = d.size
                    if pane_info is not p:
                        p.SetFlag(p.needsRestore, True)

        
    def RestorePane(self, pane_info):
        """
        Restores the input pane from a previous maximized or minimized state.

        :param `pane_info`: a L{AuiPaneInfo} instance.
        """
        
        # restore all the panes
        for p in self._panes:
            if not p.IsToolbar():
                p.SetFlag(p.optionHidden, p.HasFlag(p.savedHiddenState))

        pane_info.SetFlag(pane_info.needsRestore, True)

        # mark ourselves non-maximized
        pane_info.Restore()
        self._has_maximized = False
        self._has_minimized = False

        # last, show the window
        if pane_info.window and not pane_info.window.IsShown():
            pane_info.window.Show(True)


    def RestoreMaximizedPane(self):
        """ Restores the current maximized pane (if any). """
        
        # restore all the panes
        for p in self._panes:
            if p.IsMaximized():
                self.RestorePane(p)
                break


    def ActivatePane(self, window):
        """
        Activates the pane to which `window` is associated.

        :param `window`: a `wx.Window` derived window.
        """

        if self.GetAGWFlags() & AUI_MGR_ALLOW_ACTIVE_PANE:
            while window:
                ret, self._panes = SetActivePane(self._panes, window)
                if ret:
                    break

                window = window.GetParent()

            self.RefreshCaptions()
            self.FireEvent(wxEVT_AUI_PANE_ACTIVATED, window, canVeto=False)
            

    def CreateNotebook(self):
        """
        Creates an automatic L{AuiNotebook} when a pane is docked on
        top of another pane.
        """

        notebook = auibook.AuiNotebook(self._frame, -1, wx.Point(0, 0), wx.Size(0, 0), agwStyle=self._autoNBStyle)

        # This is so we can get the tab-drag event.
        notebook.GetAuiManager().SetMasterManager(self)
        notebook.SetArtProvider(self._autoNBTabArt.Clone())
        self._notebooks.append(notebook)

        return notebook


    def SetAutoNotebookTabArt(self, art):
        """
        Sets the default tab art provider for automatic notebooks.

        :param `art`: a tab art provider.
        """

        for nb in self._notebooks:
            nb.SetArtProvider(art.Clone())
            nb.Refresh()
            nb.Update()

        self._autoNBTabArt = art


    def GetAutoNotebookTabArt(self):
        """ Returns the default tab art provider for automatic notebooks. """

        return self._autoNBTabArt        
        

    def SetAutoNotebookStyle(self, agwStyle):
        """
        Sets the default AGW-specific window style for automatic notebooks.

        :param `agwStyle`: the underlying L{AuiNotebook} window style.
         This can be a combination of the following bits:
        
         ==================================== ==================================
         Flag name                            Description
         ==================================== ==================================
         ``AUI_NB_TOP``                       With this style, tabs are drawn along the top of the notebook
         ``AUI_NB_LEFT``                      With this style, tabs are drawn along the left of the notebook. Not implemented yet.
         ``AUI_NB_RIGHT``                     With this style, tabs are drawn along the right of the notebook. Not implemented yet.
         ``AUI_NB_BOTTOM``                    With this style, tabs are drawn along the bottom of the notebook
         ``AUI_NB_TAB_SPLIT``                 Allows the tab control to be split by dragging a tab
         ``AUI_NB_TAB_MOVE``                  Allows a tab to be moved horizontally by dragging
         ``AUI_NB_TAB_EXTERNAL_MOVE``         Allows a tab to be moved to another tab control
         ``AUI_NB_TAB_FIXED_WIDTH``           With this style, all tabs have the same width
         ``AUI_NB_SCROLL_BUTTONS``            With this style, left and right scroll buttons are displayed
         ``AUI_NB_WINDOWLIST_BUTTON``         With this style, a drop-down list of windows is available
         ``AUI_NB_CLOSE_BUTTON``              With this style, a close button is available on the tab bar
         ``AUI_NB_CLOSE_ON_ACTIVE_TAB``       With this style, a close button is available on the active tab
         ``AUI_NB_CLOSE_ON_ALL_TABS``         With this style, a close button is available on all tabs
         ``AUI_NB_MIDDLE_CLICK_CLOSE``        Allows to close L{AuiNotebook} tabs by mouse middle button click
         ``AUI_NB_SUB_NOTEBOOK``              This style is used by {AuiManager} to create automatic AuiNotebooks
         ``AUI_NB_HIDE_ON_SINGLE_TAB``        Hides the tab window if only one tab is present
         ``AUI_NB_SMART_TABS``                Use Smart Tabbing, like ``Alt`` + ``Tab`` on Windows
         ``AUI_NB_USE_IMAGES_DROPDOWN``       Uses images on dropdown window list menu instead of check items
         ``AUI_NB_CLOSE_ON_TAB_LEFT``         Draws the tab close button on the left instead of on the right (a la Camino browser)
         ``AUI_NB_TAB_FLOAT``                 Allows the floating of single tabs. Known limitation: when the notebook is more or less full screen, tabs cannot be dragged far enough outside of the notebook to become floating pages
         ``AUI_NB_DRAW_DND_TAB``              Draws an image representation of a tab while dragging (on by default)
         ==================================== ==================================

        """

        for nb in self._notebooks:
            nb.SetAGWWindowStyleFlag(agwStyle)
            nb.Refresh()
            nb.Update()

        self._autoNBStyle = agwStyle


    def GetAutoNotebookStyle(self):
        """
        Returns the default AGW-specific window style for automatic notebooks.

        :see: L{SetAutoNotebookStyle} method for a list of possible styles.
        """

        return self._autoNBStyle


    def SavePaneInfo(self, pane):
        """
        This method is similar to L{SavePerspective}, with the exception
        that it only saves information about a single pane. It is used in
        combination with L{LoadPaneInfo}.

        :param `pane`: a L{AuiPaneInfo} instance to save.        
        """

        result = "name=" + EscapeDelimiters(pane.name) + ";"
        result += "caption=" + EscapeDelimiters(pane.caption) + ";"

        result += "state=%u;"%pane.state
        result += "dir=%d;"%pane.dock_direction
        result += "layer=%d;"%pane.dock_layer
        result += "row=%d;"%pane.dock_row
        result += "pos=%d;"%pane.dock_pos
        result += "prop=%d;"%pane.dock_proportion
        result += "bestw=%d;"%pane.best_size.x
        result += "besth=%d;"%pane.best_size.y
        result += "minw=%d;"%pane.min_size.x
        result += "minh=%d;"%pane.min_size.y
        result += "maxw=%d;"%pane.max_size.x
        result += "maxh=%d;"%pane.max_size.y
        result += "floatx=%d;"%pane.floating_pos.x
        result += "floaty=%d;"%pane.floating_pos.y
        result += "floatw=%d;"%pane.floating_size.x
        result += "floath=%d;"%pane.floating_size.y
        result += "notebookid=%d;"%pane.notebook_id
        result += "transparent=%d"%pane.transparent

        return result


    def LoadPaneInfo(self, pane_part, pane):
        """
        This method is similar to to L{LoadPerspective}, with the exception that
        it only loads information about a single pane. It is used in combination
        with L{SavePaneInfo}.

        :param `pane_part`: the string to analyze;
        :param `pane`: the L{AuiPaneInfo} structure in which to load `pane_part`.
        """

        # replace escaped characters so we can
        # split up the string easily
        pane_part = pane_part.replace("\\|", "\a")
        pane_part = pane_part.replace("\\;", "\b")

        options = pane_part.split(";")
        for items in options:

            val_name, value = items.split("=")
            val_name = val_name.strip()

            if val_name == "name":
                pane.name = value
            elif val_name == "caption":
                pane.caption = value
            elif val_name == "state":
                pane.state = int(value)
            elif val_name == "dir":
                pane.dock_direction = int(value)
            elif val_name == "layer":
                pane.dock_layer = int(value)
            elif val_name == "row":
                pane.dock_row = int(value)
            elif val_name == "pos":
                pane.dock_pos = int(value)
            elif val_name == "prop":
                pane.dock_proportion = int(value)
            elif val_name == "bestw":
                pane.best_size.x = int(value)
            elif val_name == "besth":
                pane.best_size.y = int(value)
                pane.best_size = wx.Size(pane.best_size.x, pane.best_size.y)
            elif val_name == "minw":
                pane.min_size.x = int(value)
            elif val_name == "minh":
                pane.min_size.y = int(value)
                pane.min_size = wx.Size(pane.min_size.x, pane.min_size.y)
            elif val_name == "maxw":
                pane.max_size.x = int(value)
            elif val_name == "maxh":
                pane.max_size.y = int(value)
                pane.max_size = wx.Size(pane.max_size.x, pane.max_size.y)
            elif val_name == "floatx":
                pane.floating_pos.x = int(value)
            elif val_name == "floaty":
                pane.floating_pos.y = int(value)
                pane.floating_pos = wx.Point(pane.floating_pos.x, pane.floating_pos.y)
            elif val_name == "floatw":
                pane.floating_size.x = int(value)
            elif val_name == "floath":
                pane.floating_size.y = int(value)
                pane.floating_size = wx.Size(pane.floating_size.x, pane.floating_size.y)
            elif val_name == "notebookid":
                pane.notebook_id = int(value)
            elif val_name == "transparent":
                pane.transparent = int(value)
            else:
                raise Exception("Bad perspective string")

        # replace escaped characters so we can
        # split up the string easily
        pane.name = pane.name.replace("\a", "|")
        pane.name = pane.name.replace("\b", ";")
        pane.caption = pane.caption.replace("\a", "|")
        pane.caption = pane.caption.replace("\b", ";")
        pane_part = pane_part.replace("\a", "|")
        pane_part = pane_part.replace("\b", ";")

        return pane
    

    def SavePerspective(self):
        """
        Saves the entire user interface layout into an encoded string, which can then
        be stored by the application (probably using `wx.Config`).

        When a perspective is restored using L{LoadPerspective}, the entire user
        interface will return to the state it was when the perspective was saved.
        """

        result = "layout2|"

        for pane in self._panes:
            result += self.SavePaneInfo(pane) + "|"
        
        for dock in self._docks:
            result = result + ("dock_size(%d,%d,%d)=%d|")%(dock.dock_direction,
                                                           dock.dock_layer,
                                                           dock.dock_row,
                                                           dock.size)
        return result


    def LoadPerspective(self, layout, update=True):
        """
        Loads a layout which was saved with L{SavePerspective}.
        
        If the `update` flag parameter is ``True``, L{Update} will be
        automatically invoked, thus realizing the saved perspective on screen.

        :param `layout`: a string which contains a saved AUI layout;
        :param `update`: whether to update immediately the window or not.
        """

        input = layout

        # check layout string version
        #    'layout1' = wxAUI 0.9.0 - wxAUI 0.9.2
        #    'layout2' = wxAUI 0.9.2 (wxWidgets 2.8)
        index = input.find("|")
        part = input[0:index].strip()
        input = input[index+1:]
        
        if part != "layout2":
            return False

        # mark all panes currently managed as docked and hidden
        for pane in self._panes:
            pane.Dock().Hide()

        # clear out the dock array; this will be reconstructed
        self._docks = []

        # replace escaped characters so we can
        # split up the string easily
        input = input.replace("\\|", "\a")
        input = input.replace("\\;", "\b")

        while 1:

            pane = AuiPaneInfo()
            index = input.find("|")
            pane_part = input[0:index].strip()
            input = input[index+1:]

            # if the string is empty, we're done parsing
            if pane_part == "":
                break

            if pane_part[0:9] == "dock_size":
                index = pane_part.find("=")
                val_name = pane_part[0:index]
                value = pane_part[index+1:]

                index = val_name.find("(")
                piece = val_name[index+1:]
                index = piece.find(")")
                piece = piece[0:index]

                vals = piece.split(",")
                dir = int(vals[0])
                layer = int(vals[1])
                row = int(vals[2])
                size = int(value)
                
                dock = AuiDockInfo()
                dock.dock_direction = dir
                dock.dock_layer = layer
                dock.dock_row = row
                dock.size = size
                self._docks.append(dock)
                
                continue

            # Undo our escaping as LoadPaneInfo needs to take an unescaped
            # name so it can be called by external callers
            pane_part = pane_part.replace("\a", "|")
            pane_part = pane_part.replace("\b", ";")

            pane = self.LoadPaneInfo(pane_part, pane)

            p = self.GetPane(pane.name)
                
            if not p.IsOk():
                if pane.IsNotebookControl():
                    # notebook controls - auto add...
                    self._panes.append(pane)
                    indx = self._panes.index(pane)
                else:
                    # the pane window couldn't be found
                    # in the existing layout -- skip it
                    continue

            else:
                indx = self._panes.index(p)
            pane.window = p.window
            pane.frame = p.frame
            pane.buttons = p.buttons
            self._panes[indx] = pane

            if isinstance(pane.window, auibar.AuiToolBar) and (pane.IsFloatable() or pane.IsDockable()):
                pane.window.SetGripperVisible(True)
            
        if update:
            self.Update()

        return True


    def GetPanePositionsAndSizes(self, dock):
        """
        Returns all the panes positions and sizes in a dock.

        :param `dock`: a L{AuiDockInfo} instance.
        """
        
        caption_size = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        pane_border_size = self._art.GetMetric(AUI_DOCKART_PANE_BORDER_SIZE)
        gripper_size = self._art.GetMetric(AUI_DOCKART_GRIPPER_SIZE)

        positions = []
        sizes = []

        action_pane = -1
        pane_count = len(dock.panes)

        # find the pane marked as our action pane
        for pane_i in xrange(pane_count):
            pane = dock.panes[pane_i]
            if pane.HasFlag(AuiPaneInfo.actionPane):
                if action_pane != -1:
                    raise Exception("Too many action panes!")
                action_pane = pane_i
            
        # set up each panes default position, and
        # determine the size (width or height, depending
        # on the dock's orientation) of each pane
        for pane in dock.panes:
            positions.append(pane.dock_pos)
            size = 0
            
            if pane.HasBorder():
                size += pane_border_size*2
                    
            if dock.IsHorizontal():
                if pane.HasGripper() and not pane.HasGripperTop():
                    size += gripper_size

                if pane.HasCaptionLeft():
                    size += caption_size
                    
                size += pane.best_size.x
                 
            else:
                if pane.HasGripper() and pane.HasGripperTop():
                    size += gripper_size

                if pane.HasCaption() and not pane.HasCaptionLeft():
                    size += caption_size
                    
                size += pane.best_size.y
       
            sizes.append(size)

        # if there is no action pane, just return the default
        # positions (as specified in pane.pane_pos)
        if action_pane == -1:
            return positions, sizes

        offset = 0
        for pane_i in xrange(action_pane-1, -1, -1):
            amount = positions[pane_i+1] - (positions[pane_i] + sizes[pane_i])
            if amount >= 0:
                offset += amount
            else:
                positions[pane_i] -= -amount

            offset += sizes[pane_i]
        
        # if the dock mode is fixed, make sure none of the panes
        # overlap we will bump panes that overlap
        offset = 0
        for pane_i in xrange(action_pane, pane_count):
            amount = positions[pane_i] - offset
            if amount >= 0:
                offset += amount
            else:
                positions[pane_i] += -amount

            offset += sizes[pane_i]

        return positions, sizes
    

    def LayoutAddPane(self, cont, dock, pane, uiparts, spacer_only):
        """
        Adds a pane into the existing layout (in an existing dock).

        :param `cont`: a `wx.Sizer` object;
        :param `dock`: the L{AuiDockInfo} structure in which to add the pane;
        :param `pane`: the L{AuiPaneInfo} instance to add to the dock;
        :param `uiparts`: a list of UI parts in the interface;
        :param `spacer_only`: whether to add a simple spacer or a real window.
        """
        
        sizer_item = wx.SizerItem()
        caption_size = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        gripper_size = self._art.GetMetric(AUI_DOCKART_GRIPPER_SIZE)
        pane_border_size = self._art.GetMetric(AUI_DOCKART_PANE_BORDER_SIZE)
        pane_button_size = self._art.GetMetric(AUI_DOCKART_PANE_BUTTON_SIZE)

        # find out the orientation of the item (orientation for panes
        # is the same as the dock's orientation)

        if dock.IsHorizontal():
            orientation = wx.HORIZONTAL
        else:
            orientation = wx.VERTICAL

        # this variable will store the proportion
        # value that the pane will receive
        pane_proportion = pane.dock_proportion

        horz_pane_sizer = wx.BoxSizer(wx.HORIZONTAL)
        vert_pane_sizer = wx.BoxSizer(wx.VERTICAL)

        if pane.HasGripper():
            
            part = AuiDockUIPart()
            if pane.HasGripperTop():
                sizer_item = vert_pane_sizer.Add((1, gripper_size), 0, wx.EXPAND)
            else:
                sizer_item = horz_pane_sizer.Add((gripper_size, 1), 0, wx.EXPAND)

            part.type = AuiDockUIPart.typeGripper
            part.dock = dock
            part.pane = pane
            part.button = None
            part.orientation = orientation
            part.cont_sizer = horz_pane_sizer
            part.sizer_item = sizer_item
            uiparts.append(part)

        button_count = len(pane.buttons)
        button_width_total = button_count*pane_button_size
        if button_count >= 1:
            button_width_total += 3

        caption, captionLeft = pane.HasCaption(), pane.HasCaptionLeft()
        button_count = len(pane.buttons)

        if captionLeft:
            caption_sizer = wx.BoxSizer(wx.VERTICAL)

            # add pane buttons to the caption
            dummy_parts = []
            for btn_id in xrange(len(pane.buttons)-1, -1, -1):
                sizer_item = caption_sizer.Add((caption_size, pane_button_size), 0, wx.EXPAND)
                part = AuiDockUIPart()
                part.type = AuiDockUIPart.typePaneButton
                part.dock = dock
                part.pane = pane
                part.button = pane.buttons[btn_id]
                part.orientation = orientation
                part.cont_sizer = caption_sizer
                part.sizer_item = sizer_item
                dummy_parts.append(part)
            
            sizer_item = caption_sizer.Add((caption_size, 1), 1, wx.EXPAND)
            vert_pane_sizer = wx.BoxSizer(wx.HORIZONTAL)

            # create the caption sizer
            part = AuiDockUIPart()

            part.type = AuiDockUIPart.typeCaption
            part.dock = dock
            part.pane = pane
            part.button = None
            part.orientation = orientation
            part.cont_sizer = vert_pane_sizer
            part.sizer_item = sizer_item
            caption_part_idx = len(uiparts)
            uiparts.append(part)
            uiparts.extend(dummy_parts)

        elif caption:

            caption_sizer = wx.BoxSizer(wx.HORIZONTAL)
            sizer_item = caption_sizer.Add((1, caption_size), 1, wx.EXPAND)

            # create the caption sizer
            part = AuiDockUIPart()

            part.type = AuiDockUIPart.typeCaption
            part.dock = dock
            part.pane = pane
            part.button = None
            part.orientation = orientation
            part.cont_sizer = vert_pane_sizer
            part.sizer_item = sizer_item
            caption_part_idx = len(uiparts)
            uiparts.append(part)

            # add pane buttons to the caption
            for button in pane.buttons:
                sizer_item = caption_sizer.Add((pane_button_size, caption_size), 0, wx.EXPAND)                        
                part = AuiDockUIPart()
                part.type = AuiDockUIPart.typePaneButton
                part.dock = dock
                part.pane = pane
                part.button = button
                part.orientation = orientation
                part.cont_sizer = caption_sizer
                part.sizer_item = sizer_item
                uiparts.append(part)

        if caption or captionLeft:
            # if we have buttons, add a little space to the right
            # of them to ease visual crowding
            if button_count >= 1:
                if captionLeft:
                    caption_sizer.Add((caption_size, 3), 0, wx.EXPAND)
                else:
                    caption_sizer.Add((3, caption_size), 0, wx.EXPAND)

            # add the caption sizer
            sizer_item = vert_pane_sizer.Add(caption_sizer, 0, wx.EXPAND)
            uiparts[caption_part_idx].sizer_item = sizer_item
                    
        # add the pane window itself
        if spacer_only or not pane.window:
            sizer_item = vert_pane_sizer.Add((1, 1), 1, wx.EXPAND)
        else:
            sizer_item = vert_pane_sizer.Add(pane.window, 1, wx.EXPAND)
            vert_pane_sizer.SetItemMinSize(pane.window, (1, 1))

        part = AuiDockUIPart()        
        part.type = AuiDockUIPart.typePane
        part.dock = dock
        part.pane = pane
        part.button = None
        part.orientation = orientation
        part.cont_sizer = vert_pane_sizer
        part.sizer_item = sizer_item
        uiparts.append(part)

        # determine if the pane should have a minimum size if the pane is
        # non-resizable (fixed) then we must set a minimum size. Alternatively,
        # if the pane.min_size is set, we must use that value as well
        
        min_size = pane.min_size
        if pane.IsFixed():
            if min_size == wx.Size(-1, -1):
                min_size = pane.best_size
                pane_proportion = 0

        if min_size != wx.Size(-1, -1):
            vert_pane_sizer.SetItemMinSize(len(vert_pane_sizer.GetChildren())-1, (min_size.x, min_size.y))
        
        # add the vertical/horizontal sizer (caption, pane window) to the
        # horizontal sizer (gripper, vertical sizer)
        horz_pane_sizer.Add(vert_pane_sizer, 1, wx.EXPAND)

        # finally, add the pane sizer to the dock sizer
        if pane.HasBorder():
            # allowing space for the pane's border
            sizer_item = cont.Add(horz_pane_sizer, pane_proportion,
                                  wx.EXPAND | wx.ALL, pane_border_size)
            part = AuiDockUIPart()
            part.type = AuiDockUIPart.typePaneBorder
            part.dock = dock
            part.pane = pane
            part.button = None
            part.orientation = orientation
            part.cont_sizer = cont
            part.sizer_item = sizer_item
            uiparts.append(part)
        else:
            sizer_item = cont.Add(horz_pane_sizer, pane_proportion, wx.EXPAND)
        
        return uiparts
        
        
    def LayoutAddDock(self, cont, dock, uiparts, spacer_only):
        """
        Adds a dock into the existing layout.

        :param `cont`: a `wx.Sizer` object;
        :param `dock`: the L{AuiDockInfo} structure to add to the layout;
        :param `uiparts`: a list of UI parts in the interface;
        :param `spacer_only`: whether to add a simple spacer or a real window.
        """
        
        sizer_item = wx.SizerItem()
        part = AuiDockUIPart()

        sash_size = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)
        orientation = (dock.IsHorizontal() and [wx.HORIZONTAL] or [wx.VERTICAL])[0]

        # resizable bottom and right docks have a sash before them
        if not self._has_maximized and not dock.fixed and \
           dock.dock_direction in [AUI_DOCK_BOTTOM, AUI_DOCK_RIGHT]:
        
            sizer_item = cont.Add((sash_size, sash_size), 0, wx.EXPAND)

            part.type = AuiDockUIPart.typeDockSizer
            part.orientation = orientation
            part.dock = dock
            part.pane = None
            part.button = None
            part.cont_sizer = cont
            part.sizer_item = sizer_item
            uiparts.append(part)
        
        # create the sizer for the dock
        dock_sizer = wx.BoxSizer(orientation)

        # add each pane to the dock
        has_maximized_pane = False
        pane_count = len(dock.panes)

        if dock.fixed:
        
            # figure out the real pane positions we will
            # use, without modifying the each pane's pane_pos member
            pane_positions, pane_sizes = self.GetPanePositionsAndSizes(dock)

            offset = 0
            for pane_i in xrange(pane_count):
            
                pane = dock.panes[pane_i]
                pane_pos = pane_positions[pane_i]

                if pane.IsMaximized():
                    has_maximized_pane = True

                amount = pane_pos - offset
                if amount > 0:
                
                    if dock.IsVertical():
                        sizer_item = dock_sizer.Add((1, amount), 0, wx.EXPAND)
                    else:
                        sizer_item = dock_sizer.Add((amount, 1), 0, wx.EXPAND)

                    part = AuiDockUIPart()
                    part.type = AuiDockUIPart.typeBackground
                    part.dock = dock
                    part.pane = None
                    part.button = None
                    part.orientation = (orientation==wx.HORIZONTAL and \
                                        [wx.VERTICAL] or [wx.HORIZONTAL])[0]
                    part.cont_sizer = dock_sizer
                    part.sizer_item = sizer_item
                    uiparts.append(part)

                    offset = offset + amount
                
                uiparts = self.LayoutAddPane(dock_sizer, dock, pane, uiparts, spacer_only)

                offset = offset + pane_sizes[pane_i]
            
            # at the end add a very small stretchable background area
            sizer_item = dock_sizer.Add((0, 0), 1, wx.EXPAND)
            part = AuiDockUIPart()
            part.type = AuiDockUIPart.typeBackground
            part.dock = dock
            part.pane = None
            part.button = None
            part.orientation = orientation
            part.cont_sizer = dock_sizer
            part.sizer_item = sizer_item
            uiparts.append(part)
        
        else:
        
            for pane_i in xrange(pane_count):
            
                pane = dock.panes[pane_i]

                if pane.IsMaximized():
                    has_maximized_pane = True

                # if this is not the first pane being added,
                # we need to add a pane sizer
                if not self._has_maximized and pane_i > 0:
                    sizer_item = dock_sizer.Add((sash_size, sash_size), 0, wx.EXPAND)
                    part = AuiDockUIPart()
                    part.type = AuiDockUIPart.typePaneSizer
                    part.dock = dock
                    part.pane = dock.panes[pane_i-1]
                    part.button = None
                    part.orientation = (orientation==wx.HORIZONTAL and \
                                        [wx.VERTICAL] or [wx.HORIZONTAL])[0]
                    part.cont_sizer = dock_sizer
                    part.sizer_item = sizer_item
                    uiparts.append(part)
                
                uiparts = self.LayoutAddPane(dock_sizer, dock, pane, uiparts, spacer_only)
            
        if dock.dock_direction == AUI_DOCK_CENTER or has_maximized_pane:
            sizer_item = cont.Add(dock_sizer, 1, wx.EXPAND)
        else:
            sizer_item = cont.Add(dock_sizer, 0, wx.EXPAND)

        part = AuiDockUIPart()
        part.type = AuiDockUIPart.typeDock
        part.dock = dock
        part.pane = None
        part.button = None
        part.orientation = orientation
        part.cont_sizer = cont
        part.sizer_item = sizer_item
        uiparts.append(part)

        if dock.IsHorizontal():
            cont.SetItemMinSize(dock_sizer, (0, dock.size))
        else:
            cont.SetItemMinSize(dock_sizer, (dock.size, 0))

        #  top and left docks have a sash after them
        if not self._has_maximized and not dock.fixed and \
           dock.dock_direction in [AUI_DOCK_TOP, AUI_DOCK_LEFT]:
        
            sizer_item = cont.Add((sash_size, sash_size), 0, wx.EXPAND)

            part = AuiDockUIPart()
            part.type = AuiDockUIPart.typeDockSizer
            part.dock = dock
            part.pane = None
            part.button = None
            part.orientation = orientation
            part.cont_sizer = cont
            part.sizer_item = sizer_item
            uiparts.append(part)
        
        return uiparts
    

    def LayoutAll(self, panes, docks, uiparts, spacer_only=False, oncheck=True):
        """
        Layouts all the UI structures in the interface.

        :param `panes`: a list of L{AuiPaneInfo} instances;
        :param `docks`: a list of L{AuiDockInfo} classes;
        :param `uiparts`: a list of UI parts in the interface;
        :param `spacer_only`: whether to add a simple spacer or a real window;
        :param `oncheck`: whether to store the results in a class member or not.
        """
        
        container = wx.BoxSizer(wx.VERTICAL)

        pane_border_size = self._art.GetMetric(AUI_DOCKART_PANE_BORDER_SIZE)
        caption_size = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        cli_size = self._frame.GetClientSize()
        
        # empty all docks out
        for dock in docks:
            dock.panes = []
            if dock.fixed:
                # always reset fixed docks' sizes, because
                # the contained windows may have been resized
                dock.size = 0
            
        dock_count = len(docks)
        
        # iterate through all known panes, filing each
        # of them into the appropriate dock. If the
        # pane does not exist in the dock, add it
        for p in panes:

            # don't layout hidden panes.
            if p.IsShown():
                
                # find any docks with the same dock direction, dock layer, and
                # dock row as the pane we are working on
                arr = FindDocks(docks, p.dock_direction, p.dock_layer, p.dock_row)

                if arr:
                    dock = arr[0]

                else:
                    # dock was not found, so we need to create a new one
                    d = AuiDockInfo()
                    d.dock_direction = p.dock_direction
                    d.dock_layer = p.dock_layer
                    d.dock_row = p.dock_row
                    docks.append(d)
                    dock = docks[-1]

                    if p.HasFlag(p.needsRestore) and not p.HasFlag(p.wasMaximized):
                   
                        isHor = dock.IsHorizontal()
                        sashSize = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)

                        # get the sizes of any docks that might 
                        # overlap with our restored dock

                        # make list of widths or heights from the size in the dock rects
                        sizes = [d.rect[2:][isHor] for \
                                 d in docks if d.IsOk() and \
                                 (d.IsHorizontal() == isHor) and \
                                 not d.toolbar and \
                                 d.dock_direction != AUI_DOCK_CENTER]
                        
                        frameRect = GetInternalFrameRect(self._frame, self._docks)

                        # set max size allowing for sashes and absolute minimum
                        maxsize = frameRect[2:][isHor] - sum(sizes) - (len(sizes)*10) - (sashSize*len(sizes))
                        dock.size = min(p.previousDockSize,maxsize)

                    else:
                        dock.size = 0

                if p.HasFlag(p.wasMaximized):
                    self.MaximizePane(p, savesizes=False)
                    p.SetFlag(p.wasMaximized, False)

                if p.HasFlag(p.needsRestore):
                    if p.previousDockPos is not None:
                        DoInsertPane(dock.panes, dock.dock_direction, dock.dock_layer, dock.dock_row, p.previousDockPos)
                        p.dock_pos = p.previousDockPos
                        p.previousDockPos = None
                    p.SetFlag(p.needsRestore, False)

                if p.IsDocked():
                    # remove the pane from any existing docks except this one
                    docks = RemovePaneFromDocks(docks, p, dock)

                    # pane needs to be added to the dock,
                    # if it doesn't already exist 
                    if not FindPaneInDock(dock, p.window):
                        dock.panes.append(p)
                else:
                    # remove the pane from any existing docks
                    docks = RemovePaneFromDocks(docks, p)
                
        # remove any empty docks
        docks = [dock for dock in docks if dock.panes]

        dock_count = len(docks)
        # configure the docks further
        for ii, dock in enumerate(docks):
            # sort the dock pane array by the pane's
            # dock position (dock_pos), in ascending order
            dock.panes.sort(PaneSortFunc)
            dock_pane_count = len(dock.panes)
            
            # for newly created docks, set up their initial size
            if dock.size == 0:
                size = 0
                for pane in dock.panes:
                    pane_size = pane.best_size
                    if pane_size == wx.Size(-1, -1):
                        pane_size = pane.min_size
                    if pane_size == wx.Size(-1, -1) and pane.window:
                        pane_size = pane.window.GetSize()
                    if dock.IsHorizontal():
                        size = max(pane_size.y, size)
                    else:
                        size = max(pane_size.x, size)
                
                # add space for the border (two times), but only
                # if at least one pane inside the dock has a pane border
                for pane in dock.panes:
                    if pane.HasBorder():
                        size = size + pane_border_size*2
                        break
                    
                # if pane is on the top or bottom, add the caption height,
                # but only if at least one pane inside the dock has a caption
                if dock.IsHorizontal():
                    for pane in dock.panes:
                        if pane.HasCaption() and not pane.HasCaptionLeft():
                            size = size + caption_size
                            break
                else:
                    for pane in dock.panes:
                        if pane.HasCaptionLeft() and not pane.HasCaption():
                            size = size + caption_size
                            break
                    
                # new dock's size may not be more than the dock constraint
                # parameter specifies.  See SetDockSizeConstraint()
                max_dock_x_size = int(self._dock_constraint_x*float(cli_size.x))
                max_dock_y_size = int(self._dock_constraint_y*float(cli_size.y))
                if cli_size <= wx.Size(20, 20):
                    max_dock_x_size = 10000
                    max_dock_y_size = 10000

                if dock.IsHorizontal():
                    size = min(size, max_dock_y_size)
                else:
                    size = min(size, max_dock_x_size)

                # absolute minimum size for a dock is 10 pixels
                if size < 10:
                    size = 10

                dock.size = size

            # determine the dock's minimum size
            plus_border = False
            plus_caption = False
            plus_caption_left = False
            dock_min_size = 0
            for pane in dock.panes:
                if pane.min_size != wx.Size(-1, -1):
                    if pane.HasBorder():
                        plus_border = True
                    if pane.HasCaption():
                        plus_caption = True
                    if pane.HasCaptionLeft():
                        plus_caption_left = True
                    if dock.IsHorizontal():
                        if pane.min_size.y > dock_min_size:
                            dock_min_size = pane.min_size.y
                    else:
                        if pane.min_size.x > dock_min_size:
                            dock_min_size = pane.min_size.x
                    
            if plus_border:
                dock_min_size += pane_border_size*2
            if plus_caption and dock.IsHorizontal():
                dock_min_size += caption_size
            if plus_caption_left and dock.IsVertical():
                dock_min_size += caption_size
               
            dock.min_size = dock_min_size
            
            # if the pane's current size is less than it's
            # minimum, increase the dock's size to it's minimum
            if dock.size < dock.min_size:
                dock.size = dock.min_size

            # determine the dock's mode (fixed or proportional)
            # determine whether the dock has only toolbars
            action_pane_marked = False
            dock.fixed = True
            dock.toolbar = True
            for pane in dock.panes:
                if not pane.IsFixed():
                    dock.fixed = False
                if not pane.IsToolbar():
                    dock.toolbar = False
                if pane.HasFlag(AuiPaneInfo.optionDockFixed):
                    dock.fixed = True
                if pane.HasFlag(AuiPaneInfo.actionPane):
                    action_pane_marked = True

            # if the dock mode is proportional and not fixed-pixel,
            # reassign the dock_pos to the sequential 0, 1, 2, 3
            # e.g. remove gaps like 1, 2, 30, 500
            if not dock.fixed:
                for jj in xrange(dock_pane_count):
                    pane = dock.panes[jj]
                    pane.dock_pos = jj
                
            # if the dock mode is fixed, and none of the panes
            # are being moved right now, make sure the panes
            # do not overlap each other.  If they do, we will
            # adjust the panes' positions
            if dock.fixed and not action_pane_marked:
                pane_positions, pane_sizes = self.GetPanePositionsAndSizes(dock)
                offset = 0
                for jj in xrange(dock_pane_count):
                    pane = dock.panes[jj]
                    pane.dock_pos = pane_positions[jj]
                    amount = pane.dock_pos - offset
                    if amount >= 0:
                        offset += amount
                    else:
                        pane.dock_pos += -amount

                    offset += pane_sizes[jj]
                    dock.panes[jj] = pane

            if oncheck:
                self._docks[ii] = dock                    

        # shrink docks if needed 
##        docks = self.SmartShrink(docks, AUI_DOCK_TOP)
##        docks = self.SmartShrink(docks, AUI_DOCK_LEFT)

        if oncheck:
            self._docks = docks
            
        # discover the maximum dock layer
        max_layer = 0
        dock_count = len(docks)
        
        for ii in xrange(dock_count):
            max_layer = max(max_layer, docks[ii].dock_layer)

        # clear out uiparts
        uiparts = []

        # create a bunch of box sizers,
        # from the innermost level outwards.
        cont = None
        middle = None

        if oncheck:
            docks = self._docks
        
        for layer in xrange(max_layer+1):
            # find any docks in this layer
            arr = FindDocks(docks, -1, layer, -1)
            # if there aren't any, skip to the next layer
            if not arr:
                continue

            old_cont = cont

            # create a container which will hold this layer's
            # docks (top, bottom, left, right)
            cont = wx.BoxSizer(wx.VERTICAL)

            # find any top docks in this layer
            arr = FindDocks(docks, AUI_DOCK_TOP, layer, -1)
            for row in arr:
                uiparts = self.LayoutAddDock(cont, row, uiparts, spacer_only)
            
            # fill out the middle layer (which consists
            # of left docks, content area and right docks)
            
            middle = wx.BoxSizer(wx.HORIZONTAL)

            # find any left docks in this layer
            arr = FindDocks(docks, AUI_DOCK_LEFT, layer, -1)
            for row in arr:
                uiparts = self.LayoutAddDock(middle, row, uiparts, spacer_only)
            
            # add content dock (or previous layer's sizer
            # to the middle
            if not old_cont:
                # find any center docks
                arr = FindDocks(docks, AUI_DOCK_CENTER, -1, -1)
                if arr:
                    for row in arr:
                       uiparts = self.LayoutAddDock(middle, row, uiparts, spacer_only)
                       
                elif not self._has_maximized:
                    # there are no center docks, add a background area
                    sizer_item = middle.Add((1, 1), 1, wx.EXPAND)
                    part = AuiDockUIPart()
                    part.type = AuiDockUIPart.typeBackground
                    part.pane = None
                    part.dock = None
                    part.button = None
                    part.cont_sizer = middle
                    part.sizer_item = sizer_item
                    uiparts.append(part)
            else:
                middle.Add(old_cont, 1, wx.EXPAND)
            
            # find any right docks in this layer
            arr = FindDocks(docks, AUI_DOCK_RIGHT, layer, -1, reverse=True)
            for row in arr:
                uiparts = self.LayoutAddDock(middle, row, uiparts, spacer_only)
                    
            if len(middle.GetChildren()) > 0:
                cont.Add(middle, 1, wx.EXPAND)

            # find any bottom docks in this layer
            arr = FindDocks(docks, AUI_DOCK_BOTTOM, layer, -1, reverse=True)
            for row in arr:
                    uiparts = self.LayoutAddDock(cont, row, uiparts, spacer_only)

        if not cont:
            # no sizer available, because there are no docks,
            # therefore we will create a simple background area
            cont = wx.BoxSizer(wx.VERTICAL)
            sizer_item = cont.Add((1, 1), 1, wx.EXPAND)
            part = AuiDockUIPart()
            part.type = AuiDockUIPart.typeBackground
            part.pane = None
            part.dock = None
            part.button = None
            part.cont_sizer = middle
            part.sizer_item = sizer_item
            uiparts.append(part)

        if oncheck:
            self._uiparts = uiparts
            self._docks = docks

        container.Add(cont, 1, wx.EXPAND)

        if oncheck:
            return container
        else:
            return container, panes, docks, uiparts


    def SetDockSizeConstraint(self, width_pct, height_pct):
        """
        When a user creates a new dock by dragging a window into a docked position,
        often times the large size of the window will create a dock that is unwieldly
        large.

        L{AuiManager} by default limits the size of any new dock to 1/3 of the window
        size. For horizontal docks, this would be 1/3 of the window height. For vertical
        docks, 1/3 of the width. Calling this function will adjust this constraint value.

        The numbers must be between 0.0 and 1.0. For instance, calling L{SetDockSizeConstraint}
        with (0.5, 0.5) will cause new docks to be limited to half of the size of the entire
        managed window.

        :param `width_pct`: a float number representing the x dock size constraint;
        :param `width_pct`: a float number representing the y dock size constraint.
        """

        self._dock_constraint_x = max(0.0, min(1.0, width_pct))
        self._dock_constraint_y = max(0.0, min(1.0, height_pct))


    def GetDockSizeConstraint(self):
        """
        Returns the current dock constraint values.

        :see: L{SetDockSizeConstraint}
        """

        return self._dock_constraint_x, self._dock_constraint_y


    def Update(self):
        """
        This method is called after any number of changes are made to any of the
        managed panes. L{Update} must be invoked after L{AddPane} or L{InsertPane} are
        called in order to "realize" or "commit" the changes.

        In addition, any number of changes may be made to L{AuiPaneInfo} structures
        (retrieved with L{GetPane}), but to realize the changes, L{Update}
        must be called. This construction allows pane flicker to be avoided by updating
        the whole layout at one time.
        """

        self._hover_button = None
        self._action_part = None
        
        # destroy floating panes which have been
        # redocked or are becoming non-floating
        for p in self._panes:
            if p.IsFloating() or not p.frame:
                continue
            
            # because the pane is no longer in a floating, we need to
            # reparent it to self._frame and destroy the floating frame
            # reduce flicker
            p.window.SetSize((1, 1))

            # the following block is a workaround for bug #1531361
            # (see wxWidgets sourceforge page).  On wxGTK (only), when
            # a frame is shown/hidden, a move event unfortunately
            # also gets fired.  Because we may be dragging around
            # a pane, we need to cancel that action here to prevent
            # a spurious crash.
            if self._action_window == p.frame:
                if self._frame.HasCapture():
                    self._frame.ReleaseMouse()
                self._action = actionNone
                self._action_window = None

            # hide the frame
            if p.frame.IsShown():
                p.frame.Show(False)

            if self._action_window == p.frame:
                self._action_window = None
        
            # reparent to self._frame and destroy the pane
            p.window.Reparent(self._frame)
            if isinstance(p.window, auibar.AuiToolBar):
                p.window.SetAuiManager(self)

            if p.frame:
                p.frame.SetSizer(None)
                p.frame.Destroy()
            p.frame = None

        # Only the master manager should create/destroy notebooks...
        if not self._masterManager:
            self.UpdateNotebook()
        
        # delete old sizer first
        self._frame.SetSizer(None)

        # create a layout for all of the panes
        sizer = self.LayoutAll(self._panes, self._docks, self._uiparts, False)

        # hide or show panes as necessary,
        # and float panes as necessary
        
        pane_count = len(self._panes)
        
        for ii in xrange(pane_count):
            p = self._panes[ii]
            pFrame = p.frame

            if p.IsFloating():
                if pFrame is None:
                    # we need to create a frame for this
                    # pane, which has recently been floated
                    frame = self.CreateFloatingFrame(self._frame, p)

                    # on MSW and Mac, if the owner desires transparent dragging, and
                    # the dragging is happening right now, then the floating
                    # window should have this style by default
                    if self._action in [actionDragFloatingPane, actionDragToolbarPane] and \
                       self._agwFlags & AUI_MGR_TRANSPARENT_DRAG:
                        frame.SetTransparent(150)

                    if p.IsToolbar():
                        bar = p.window
                        if isinstance(bar, auibar.AuiToolBar):
                            bar.SetGripperVisible(False)
                            agwStyle = bar.GetAGWWindowStyleFlag()
                            bar.SetAGWWindowStyleFlag(agwStyle & ~AUI_TB_VERTICAL)
                            bar.Realize()

                        s = p.window.GetMinSize()
                        p.BestSize(s)
                        p.FloatingSize(wx.DefaultSize)

                    frame.SetPaneWindow(p)
                    p.needsTransparency = True
                    p.frame = pFrame = frame
                    if p.IsShown() and not frame.IsShown():
                        frame.Show()
                        frame.Update()
                else:

                    # frame already exists, make sure it's position
                    # and size reflect the information in AuiPaneInfo
                    if pFrame.GetPosition() != p.floating_pos or pFrame.GetSize() != p.floating_size:
                        pFrame.SetDimensions(p.floating_pos.x, p.floating_pos.y,
                                             p.floating_size.x, p.floating_size.y, wx.SIZE_USE_EXISTING)

                    # update whether the pane is resizable or not
                    style = p.frame.GetWindowStyleFlag()
                    if p.IsFixed():
                        style &= ~wx.RESIZE_BORDER
                    else:
                        style |= wx.RESIZE_BORDER

                    p.frame.SetWindowStyleFlag(style)

                    if pFrame.IsShown() != p.IsShown():
                        p.needsTransparency = True
                        pFrame.Show(p.IsShown())

                if pFrame.GetTitle() != p.caption:
                    pFrame.SetTitle(p.caption)
                if p.icon.IsOk():
                    pFrame.SetIcon(wx.IconFromBitmap(p.icon))
                    
            else:

                if p.IsToolbar():
#                    self.SwitchToolBarOrientation(p)
                    p.best_size = p.window.GetBestSize()

                if p.window and not p.IsNotebookPage() and p.window.IsShown() != p.IsShown():
                    p.window.Show(p.IsShown())

            if pFrame and p.needsTransparency:
                if pFrame.IsShown() and pFrame._transparent != p.transparent:
                    pFrame.SetTransparent(p.transparent)
                    pFrame._transparent = p.transparent
                    
                p.needsTransparency = False

            # if "active panes" are no longer allowed, clear
            # any optionActive values from the pane states
            if self._agwFlags & AUI_MGR_ALLOW_ACTIVE_PANE == 0:
                p.state &= ~AuiPaneInfo.optionActive

            self._panes[ii] = p

        old_pane_rects = []
        pane_count = len(self._panes)
        
        for p in self._panes:
            r = wx.Rect()
            if p.window and p.IsShown() and p.IsDocked():
                r = p.rect

            old_pane_rects.append(r)    
            
        # apply the new sizer
        self._frame.SetSizer(sizer)
        self._frame.SetAutoLayout(False)
        self.DoFrameLayout()
        
        # now that the frame layout is done, we need to check
        # the new pane rectangles against the old rectangles that
        # we saved a few lines above here.  If the rectangles have
        # changed, the corresponding panes must also be updated
        for ii in xrange(pane_count):
            p = self._panes[ii]
            if p.window and p.IsShown() and p.IsDocked():
                if p.rect != old_pane_rects[ii]:
                    p.window.Refresh()
                    p.window.Update()

        if wx.Platform == "__WXMAC__":
            self._frame.Refresh()
        else:
            self.Repaint()

        if not self._masterManager:
            e = self.FireEvent(wxEVT_AUI_PERSPECTIVE_CHANGED, None, canVeto=False)    


    def UpdateNotebook(self):
        """ Updates the automatic L{AuiNotebook} in the layout (if any exists). """

        # Workout how many notebooks we need.
        max_notebook = -1

        # destroy floating panes which have been
        # redocked or are becoming non-floating
        for paneInfo in self._panes:
            if max_notebook < paneInfo.notebook_id:
                max_notebook = paneInfo.notebook_id
        
        # We are the master of our domain
        extra_notebook = len(self._notebooks)
        max_notebook += 1
        
        for i in xrange(extra_notebook, max_notebook):
            self.CreateNotebook()

        # Remove pages from notebooks that no-longer belong there ...
        for nb, notebook in enumerate(self._notebooks):
            pages = notebook.GetPageCount()
            pageCounter, allPages = 0, pages

            # Check each tab ...
            for page in xrange(pages):

                if page >= allPages:
                    break
                
                window = notebook.GetPage(pageCounter)
                paneInfo = self.GetPane(window)
                if paneInfo.IsOk() and paneInfo.notebook_id != nb:
                    notebook.RemovePage(pageCounter)
                    window.Hide()
                    window.Reparent(self._frame)
                    pageCounter -= 1
                    allPages -= 1

                pageCounter += 1

            notebook.DoSizing()

        # Add notebook pages that aren't there already...
        for paneInfo in self._panes:
            if paneInfo.IsNotebookPage():
            
                title = (paneInfo.caption == "" and [paneInfo.name] or [paneInfo.caption])[0]

                notebook = self._notebooks[paneInfo.notebook_id]
                page_id = notebook.GetPageIndex(paneInfo.window)

                if page_id < 0:
                
                    paneInfo.window.Reparent(notebook)
                    notebook.AddPage(paneInfo.window, title, True, paneInfo.icon)
                
                # Update title and icon ...
                else:
                
                    notebook.SetPageText(page_id, title)
                    notebook.SetPageBitmap(page_id, paneInfo.icon)

                notebook.DoSizing()
                
            # Wire-up newly created notebooks
            elif paneInfo.IsNotebookControl() and not paneInfo.window:
                paneInfo.window = self._notebooks[paneInfo.notebook_id]
            
        # Delete empty notebooks, and convert notebooks with 1 page to
        # normal panes...
        remap_ids = [-1]*len(self._notebooks)
        nb_idx = 0

        for nb, notebook in enumerate(self._notebooks): 
            if notebook.GetPageCount() == 1:
            
                # Convert notebook page to pane...
                window = notebook.GetPage(0)
                child_pane = self.GetPane(window)
                notebook_pane = self.GetPane(notebook)
                if child_pane.IsOk() and notebook_pane.IsOk():
                
                    child_pane.SetDockPos(notebook_pane)
                    child_pane.window.Hide()
                    child_pane.window.Reparent(self._frame)
                    child_pane.frame = None
                    child_pane.notebook_id = -1
                    if notebook_pane.IsFloating():
                        child_pane.Float()

                    self.DetachPane(notebook)

                    notebook.RemovePage(0)
                    notebook.Destroy()
                
                else:
                
                    raise Exception("Odd notebook docking")
                
            elif notebook.GetPageCount() == 0:
            
                self.DetachPane(notebook)
                notebook.Destroy()
            
            else:
            
                # Correct page ordering. The original wxPython code
                # for this did not work properly, and would misplace 
                # windows causing errors.
                notebook.Freeze()
                self._notebooks[nb_idx] = notebook
                pages = notebook.GetPageCount()
                selected = notebook.GetPage(notebook.GetSelection())

                # Take each page out of the notebook, group it with
                # its current pane, and sort the list by pane.dock_pos
                # order
                pages_and_panes = []
                for idx in reversed(range(pages)):
                    page = notebook.GetPage(idx)
                    pane = self.GetPane(page)
                    pages_and_panes.append((page, pane))
                    notebook.RemovePage(idx)
                sorted_pnp = sorted(pages_and_panes, key=lambda tup: tup[1].dock_pos)

                # Grab the attributes from the panes which are ordered
                # correctly, and copy those attributes to the original
                # panes. (This avoids having to change the ordering
                # of self._panes) Then, add the page back into the notebook
                sorted_attributes = [self.GetAttributes(tup[1])
                                     for tup in sorted_pnp]
                for attrs, tup in zip(sorted_attributes, pages_and_panes):
                    pane = tup[1]
                    self.SetAttributes(pane, attrs)
                    notebook.AddPage(pane.window, pane.caption)

                notebook.SetSelection(notebook.GetPageIndex(selected), True)
                notebook.DoSizing()
                notebook.Thaw()

                # It's a keeper.
                remap_ids[nb] = nb_idx
                nb_idx += 1

        # Apply remap...
        nb_count = len(self._notebooks)
        
        if nb_count != nb_idx:
        
            self._notebooks = self._notebooks[0:nb_idx]
            for p in self._panes:
                if p.notebook_id >= 0:                
                    p.notebook_id = remap_ids[p.notebook_id]
                    if p.IsNotebookControl():
                        p.SetNameFromNotebookId()
                
        # Make sure buttons are correct ...
        for notebook in self._notebooks:
            want_max = True
            want_min = True
            want_close = True

            pages = notebook.GetPageCount()
            for page in xrange(pages):
            
                win = notebook.GetPage(page)
                pane = self.GetPane(win)
                if pane.IsOk():
                
                    if not pane.HasCloseButton():
                        want_close = False
                    if not pane.HasMaximizeButton():
                        want_max = False
                    if not pane.HasMinimizeButton():
                        want_min = False
                
            notebook_pane = self.GetPane(notebook)
            if notebook_pane.IsOk():
                if notebook_pane.HasMinimizeButton() != want_min:
                    if want_min:
                        button = AuiPaneButton(AUI_BUTTON_MINIMIZE)
                        notebook_pane.state |= AuiPaneInfo.buttonMinimize
                        notebook_pane.buttons.append(button)

                    # todo: remove min/max
            
                if notebook_pane.HasMaximizeButton() != want_max:
                    if want_max:
                        button = AuiPaneButton(AUI_BUTTON_MAXIMIZE_RESTORE)
                        notebook_pane.state |= AuiPaneInfo.buttonMaximize
                        notebook_pane.buttons.append(button)
                    
                    # todo: remove min/max
                
                if notebook_pane.HasCloseButton() != want_close:
                    if want_close:
                        button = AuiPaneButton(AUI_BUTTON_CLOSE)
                        notebook_pane.state |= AuiPaneInfo.buttonClose
                        notebook_pane.buttons.append(button)
                    
                    # todo: remove close


    def SmartShrink(self, docks, direction):
        """
        Used to intelligently shrink the docks' size (if needed).

        :param `docks`: a list of L{AuiDockInfo} instances;
        :param `direction`: the direction in which to shrink.
        """

        sashSize = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)
        caption_size = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        clientSize = self._frame.GetClientSize()
        ourDocks = FindDocks(docks, direction, -1, -1)
        oppositeDocks = FindOppositeDocks(docks, direction)
        oppositeSize = self.GetOppositeDockTotalSize(docks, direction)
        ourSize = 0

        for dock in ourDocks:
            ourSize += dock.size

            if not dock.toolbar:
                ourSize += sashSize
        
        shrinkSize = ourSize + oppositeSize

        if direction == AUI_DOCK_TOP or direction == AUI_DOCK_BOTTOM:
            shrinkSize -= clientSize.y
        else:
            shrinkSize -= clientSize.x

        if shrinkSize <= 0:
            return docks

        # Combine arrays
        for dock in oppositeDocks:
            ourDocks.append(dock)
            
        oppositeDocks = []

        for dock in ourDocks:
            if dock.toolbar or not dock.resizable:
                continue

            dockRange = dock.size - dock.min_size

            if dock.min_size == 0:
                dockRange -= sashSize
                if direction == AUI_DOCK_TOP or direction == AUI_DOCK_BOTTOM:
                    dockRange -= caption_size
            
            if dockRange >= shrinkSize:
            
                dock.size -= shrinkSize
                return docks
            
            else:
            
                dock.size -= dockRange
                shrinkSize -= dockRange
            
        return docks
    

    def UpdateDockingGuides(self, paneInfo):
        """
        Updates the docking guide windows positions and appearance.

        :param `paneInfo`: a L{AuiPaneInfo} instance.
        """

        if len(self._guides) == 0:
            self.CreateGuideWindows()

        captionSize = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        frameRect = GetInternalFrameRect(self._frame, self._docks)
        mousePos = wx.GetMousePosition()

        for indx, guide in enumerate(self._guides):
        
            pt = wx.Point()
            guide_size = guide.host.GetSize()
            if not guide.host:
                raise Exception("Invalid docking host")

            direction = guide.dock_direction

            if direction == AUI_DOCK_LEFT:
                pt.x = frameRect.x + guide_size.x / 2 + 16
                pt.y = frameRect.y + frameRect.height / 2

            elif direction == AUI_DOCK_TOP:
                pt.x = frameRect.x + frameRect.width / 2
                pt.y = frameRect.y + guide_size.y / 2 + 16

            elif direction == AUI_DOCK_RIGHT:
                pt.x = frameRect.x + frameRect.width - guide_size.x / 2 - 16
                pt.y = frameRect.y + frameRect.height / 2

            elif direction == AUI_DOCK_BOTTOM:
                pt.x = frameRect.x + frameRect.width / 2
                pt.y = frameRect.y + frameRect.height - guide_size.y / 2 - 16

            elif direction == AUI_DOCK_CENTER:
                rc = paneInfo.window.GetScreenRect()
                pt.x = rc.x + rc.width / 2
                pt.y = rc.y + rc.height / 2
                if paneInfo.HasCaption():
                    pt.y -= captionSize / 2
                elif paneInfo.HasCaptionLeft():
                    pt.x -= captionSize / 2

            # guide will be centered around point 'pt'
            targetPosition = wx.Point(pt.x - guide_size.x / 2, pt.y - guide_size.y / 2)

            if guide.host.GetPosition() != targetPosition:
                guide.host.Move(targetPosition)
                
            guide.host.AeroMove(targetPosition)

            if guide.dock_direction == AUI_DOCK_CENTER:
                guide.host.ValidateNotebookDocking(paneInfo.IsNotebookDockable())

            guide.host.UpdateDockGuide(mousePos)
        
        paneInfo.window.Lower()

                        
    def DoFrameLayout(self):
        """
        This is an internal function which invokes `wx.Sizer.Layout`
        on the frame's main sizer, then measures all the various UI items
        and updates their internal rectangles.

        :note: This should always be called instead of calling
         `self._managed_window.Layout()` directly.
        """

        self._frame.Layout()
        
        for part in self._uiparts:            
            # get the rectangle of the UI part
            # originally, this code looked like this:
            #    part.rect = wx.Rect(part.sizer_item.GetPosition(),
            #                       part.sizer_item.GetSize())
            # this worked quite well, with one exception: the mdi
            # client window had a "deferred" size variable 
            # that returned the wrong size.  It looks like
            # a bug in wx, because the former size of the window
            # was being returned.  So, we will retrieve the part's
            # rectangle via other means

            part.rect = part.sizer_item.GetRect()
            flag = part.sizer_item.GetFlag()
            border = part.sizer_item.GetBorder()
            
            if flag & wx.TOP:
                part.rect.y -= border
                part.rect.height += border
            if flag & wx.LEFT:
                part.rect.x -= border
                part.rect.width += border
            if flag & wx.BOTTOM:
                part.rect.height += border
            if flag & wx.RIGHT:
                part.rect.width += border

            if part.type == AuiDockUIPart.typeDock:
                part.dock.rect = part.rect
            if part.type == AuiDockUIPart.typePane:
                part.pane.rect = part.rect


    def GetPanePart(self, wnd):
        """
        Looks up the pane border UI part of the
        pane specified. This allows the caller to get the exact rectangle
        of the pane in question, including decorations like caption and border.

        :param `wnd`: the window to which the pane border belongs to.        
        """

        for part in self._uiparts:
            if part.type == AuiDockUIPart.typePaneBorder and \
               part.pane and part.pane.window == wnd:
                return part

        for part in self._uiparts:
            if part.type == AuiDockUIPart.typePane and \
               part.pane and part.pane.window == wnd:
                return part
    
        return None


    def GetDockPixelOffset(self, test):
        """
        This is an internal function which returns
        a dock's offset in pixels from the left side of the window
        (for horizontal docks) or from the top of the window (for
        vertical docks).

        This value is necessary for calculating fixed-pane/toolbar offsets
        when they are dragged.

        :param `test`: a fake L{AuiPaneInfo} for testing purposes.
        """

        # the only way to accurately calculate the dock's
        # offset is to actually run a theoretical layout
        docks, panes = CopyDocksAndPanes2(self._docks, self._panes)
        panes.append(test)

        sizer, panes, docks, uiparts = self.LayoutAll(panes, docks, [], True, False)
        client_size = self._frame.GetClientSize()
        sizer.SetDimension(0, 0, client_size.x, client_size.y)
        sizer.Layout()

        for part in uiparts:
            pos = part.sizer_item.GetPosition()
            size = part.sizer_item.GetSize()
            part.rect = wx.RectPS(pos, size)
            if part.type == AuiDockUIPart.typeDock:
                part.dock.rect = part.rect

        sizer.Destroy()

        for dock in docks:
            if test.dock_direction == dock.dock_direction and \
               test.dock_layer == dock.dock_layer and  \
               test.dock_row == dock.dock_row:
            
                if dock.IsVertical():
                    return dock.rect.y
                else:
                    return dock.rect.x
            
        return 0
    

    def GetPartnerDock(self, dock):
        """
        Returns the partner dock for the input dock.

        :param `dock`: a L{AuiDockInfo} instance.
        """

        for layer in xrange(dock.dock_layer, -1, -1):
        
            bestDock = None

            for tmpDock in self._docks:
            
                if tmpDock.dock_layer != layer:
                    continue
                
                if tmpDock.dock_direction != dock.dock_direction:
                    continue

                if tmpDock.dock_layer < dock.dock_layer:
                
                    if not bestDock or tmpDock.dock_row < bestDock.dock_row:
                        bestDock = tmpDock
                
                elif tmpDock.dock_row > dock.dock_row:
                
                    if not bestDock or tmpDock.dock_row > bestDock.dock_row:
                        bestDock = tmpDock
                
            if bestDock:
                return bestDock
        
        return None


    def GetPartnerPane(self, dock, pane):
        """
        Returns the partner pane for the input pane. They both need to live
        in the same L{AuiDockInfo}.

        :param `dock`: a L{AuiDockInfo} instance;
        :param `pane`: a L{AuiPaneInfo} class.
        """
        
        panePosition = -1

        for i, tmpPane in enumerate(dock.panes):        
            if tmpPane.window == pane.window:
                panePosition = i
            elif not tmpPane.IsFixed() and panePosition != -1:
                return tmpPane
        
        return None


    def GetTotalPixSizeAndProportion(self, dock):
        """
        Returns the dimensions and proportion of the input dock.

        :param `dock`: the L{AuiDockInfo} structure to analyze.
        """

        totalPixsize = 0
        totalProportion = 0

        # determine the total proportion of all resizable panes,
        # and the total size of the dock minus the size of all
        # the fixed panes
        for tmpPane in dock.panes:
        
            if tmpPane.IsFixed():
                continue

            totalProportion += tmpPane.dock_proportion

            if dock.IsHorizontal():
                totalPixsize += tmpPane.rect.width
            else:
                totalPixsize += tmpPane.rect.height

##            if tmpPane.min_size.IsFullySpecified():
##            
##                if dock.IsHorizontal():
##                    totalPixsize -= tmpPane.min_size.x
##                else:
##                    totalPixsize -= tmpPane.min_size.y
            
        return totalPixsize, totalProportion


    def GetOppositeDockTotalSize(self, docks, direction):
        """
        Returns the dimensions of the dock which lives opposite of the input dock.

        :param `docks`: a list of L{AuiDockInfo} structures to analyze;
        :param `direction`: the direction in which to look for the opposite dock.
        """
        
        sash_size = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)
        caption_size = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        pane_border_size = self._art.GetMetric(AUI_DOCKART_PANE_BORDER_SIZE)
        minSizeMax = 0
        result = sash_size
        vertical = False

        if direction in [AUI_DOCK_TOP, AUI_DOCK_BOTTOM]:
            vertical = True

        # Get minimum size of the most inner area
        for tmpDock in docks:
        
            if tmpDock.dock_layer != 0:
                continue

            if tmpDock.dock_direction != AUI_DOCK_CENTER and tmpDock.IsVertical() != vertical:
                continue

            for tmpPane in tmpDock.panes:
            
                minSize = pane_border_size*2 - sash_size

                if vertical:
                    minSize += tmpPane.min_size.y + caption_size
                else:
                    minSize += tmpPane.min_size.x

                if minSize > minSizeMax:
                    minSizeMax = minSize
            
        result += minSizeMax

        # Get opposite docks
        oppositeDocks = FindOppositeDocks(docks, direction)

        # Sum size of the opposite docks and their sashes
        for dock in oppositeDocks:
            result += dock.size
            # if it's not a toolbar add the sash_size too
            if not dock.toolbar:
                result += sash_size
        
        return result


    def CalculateDockSizerLimits(self, dock):
        """
        Calculates the minimum and maximum sizes allowed for the input dock.

        :param `dock`: the L{AuiDockInfo} structure to analyze.
        """

        docks, panes = CopyDocksAndPanes2(self._docks, self._panes)

        sash_size = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)
        caption_size = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
        opposite_size = self.GetOppositeDockTotalSize(docks, dock.dock_direction)

        for tmpDock in docks:
        
            if tmpDock.dock_direction == dock.dock_direction and \
               tmpDock.dock_layer == dock.dock_layer and \
               tmpDock.dock_row == dock.dock_row:
        
                tmpDock.size = 1
                break
        
        sizer, panes, docks, uiparts = self.LayoutAll(panes, docks, [], True, False)
        client_size = self._frame.GetClientSize()
        sizer.SetDimension(0, 0, client_size.x, client_size.y)
        sizer.Layout()

        for part in uiparts:
        
            part.rect = wx.RectPS(part.sizer_item.GetPosition(), part.sizer_item.GetSize())
            if part.type == AuiDockUIPart.typeDock:
                part.dock.rect = part.rect
        
        sizer.Destroy()
        new_dock = None

        for tmpDock in docks:
            if tmpDock.dock_direction == dock.dock_direction and \
               tmpDock.dock_layer == dock.dock_layer and \
               tmpDock.dock_row == dock.dock_row:
            
                new_dock = tmpDock
                break
            
        partnerDock = self.GetPartnerDock(dock)

        if partnerDock:
            partnerRange = partnerDock.size - partnerDock.min_size
            if partnerDock.min_size == 0:
                partnerRange -= sash_size
                if dock.IsHorizontal():
                    partnerRange -= caption_size
            
            direction = dock.dock_direction
            
            if direction == AUI_DOCK_LEFT:
                minPix = new_dock.rect.x + new_dock.rect.width
                maxPix = dock.rect.x + dock.rect.width
                maxPix += partnerRange

            elif direction == AUI_DOCK_TOP:
                minPix = new_dock.rect.y + new_dock.rect.height
                maxPix = dock.rect.y + dock.rect.height
                maxPix += partnerRange

            elif direction == AUI_DOCK_RIGHT:
                minPix = dock.rect.x - partnerRange - sash_size
                maxPix = new_dock.rect.x - sash_size

            elif direction == AUI_DOCK_BOTTOM:
                minPix = dock.rect.y - partnerRange - sash_size
                maxPix = new_dock.rect.y - sash_size

            return minPix, maxPix
        
        direction = new_dock.dock_direction
        
        if direction == AUI_DOCK_LEFT:
            minPix = new_dock.rect.x + new_dock.rect.width
            maxPix = client_size.x - opposite_size - sash_size

        elif direction == AUI_DOCK_TOP:
            minPix = new_dock.rect.y + new_dock.rect.height
            maxPix = client_size.y - opposite_size - sash_size

        elif direction == AUI_DOCK_RIGHT:
            minPix = opposite_size
            maxPix = new_dock.rect.x - sash_size

        elif direction == AUI_DOCK_BOTTOM:
            minPix = opposite_size
            maxPix = new_dock.rect.y - sash_size

        return minPix, maxPix


    def CalculatePaneSizerLimits(self, dock, pane):
        """
        Calculates the minimum and maximum sizes allowed for the input pane.

        :param `dock`: the L{AuiDockInfo} structure to which `pane` belongs to;
        :param `pane`: a L{AuiPaneInfo} class for which calculation are requested.
        """
        
        if pane.IsFixed():
            if dock.IsHorizontal():
                minPix = maxPix = pane.rect.x + 1 + pane.rect.width
            else:
                minPix = maxPix = pane.rect.y + 1 + pane.rect.height

            return minPix, maxPix
        
        totalPixsize, totalProportion = self.GetTotalPixSizeAndProportion(dock)
        partnerPane = self.GetPartnerPane(dock, pane)

        if dock.IsHorizontal():
        
            minPix = pane.rect.x + 1
            maxPix = pane.rect.x + 1 + pane.rect.width

            if pane.min_size.IsFullySpecified():
                minPix += pane.min_size.x
            else:
                minPix += 1

            if partnerPane:
                maxPix += partnerPane.rect.width

                if partnerPane.min_size.IsFullySpecified():
                    maxPix -= partnerPane.min_size.x - 1
            
            else:
                minPix = maxPix
        
        else:
        
            minPix = pane.rect.y + 1
            maxPix = pane.rect.y + 1 + pane.rect.height

            if pane.min_size.IsFullySpecified():
                minPix += pane.min_size.y
            else:
                minPix += 1

            if partnerPane:            
                maxPix += partnerPane.rect.height

                if partnerPane.min_size.IsFullySpecified():
                    maxPix -= partnerPane.min_size.y - 1
            
            else:            
                minPix = maxPix
            
        return minPix, maxPix


    def CheckMovableSizer(self, part):
        """
        Checks if a UI part can be actually resized.

        :param `part`: a UI part.
        """

        # a dock may not be resized if it has a single
        # pane which is not resizable
        if part.type == AuiDockUIPart.typeDockSizer and part.dock and \
           len(part.dock.panes) == 1 and part.dock.panes[0].IsFixed():
        
            return False
        
        if part.pane:
        
            # panes that may not be resized should be ignored here
            minPix, maxPix = self.CalculatePaneSizerLimits(part.dock, part.pane)

            if minPix == maxPix:
                return False
        
        return True


    def PaneFromTabEvent(self, event):
        """
        Returns a L{AuiPaneInfo} from a L{AuiNotebookEvent} event.

        :param `event`: a L{AuiNotebookEvent} event.
        """

        obj = event.GetEventObject()

        if obj and isinstance(obj, auibook.AuiTabCtrl):
        
            page_idx = obj.GetActivePage()

            if page_idx >= 0:
                page = obj.GetPage(page_idx)
                window = page.window
                if window:
                    return self.GetPane(window)
            
        elif obj and isinstance(obj, auibook.AuiNotebook):
        
            page_idx = event.GetSelection()
            
            if page_idx >= 0:
                window = obj.GetPage(page_idx)
                if window:
                    return self.GetPane(window)
            
        return NonePaneInfo


    def OnTabBeginDrag(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_BEGIN_DRAG`` event.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        if self._masterManager:
            self._masterManager.OnTabBeginDrag(event)
        
        else:
            paneInfo = self.PaneFromTabEvent(event)
            
            if paneInfo.IsOk():
            
                # It's one of ours!
                self._action = actionDragFloatingPane
                mouse = wx.GetMousePosition()

                # set initial float position - may have to think about this
                # offset a bit more later ...
                self._action_offset = wx.Point(20, 10)
                self._toolbar_action_offset = wx.Point(20, 10)
                
                paneInfo.floating_pos = mouse - self._action_offset
                paneInfo.dock_pos = AUI_DOCK_NONE
                paneInfo.notebook_id = -1

                tab = event.GetEventObject()

                if tab.HasCapture():
                    tab.ReleaseMouse()

                # float the window
                if paneInfo.IsMaximized():
                    self.RestorePane(paneInfo)
                paneInfo.Float()
                self.Update()

                self._action_window = paneInfo.window
                    
                self._frame.CaptureMouse()
                event.SetDispatched(True)
            
            else:
            
                # not our window
                event.Skip()
            

    def OnTabPageClose(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_PAGE_CLOSE`` event.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """

        if self._masterManager:
            self._masterManager.OnTabPageClose(event)
        
        else:
        
            p = self.PaneFromTabEvent(event)
            if p.IsOk():
            
                # veto it because we will call "RemovePage" ourselves
                event.Veto()

                # Now ask the app if they really want to close...
                # fire pane close event
                e = AuiManagerEvent(wxEVT_AUI_PANE_CLOSE)
                e.SetPane(p)
                e.SetCanVeto(True)
                self.ProcessMgrEvent(e)

                if e.GetVeto():
                    return

                self.ClosePane(p)
                self.Update()
            else:
                event.Skip()
            

    def OnTabSelected(self, event):
        """
        Handles the ``EVT_AUINOTEBOOK_PAGE_CHANGED`` event.

        :param `event`: a L{AuiNotebookEvent} event to be processed.
        """
        
        if self._masterManager:
            self._masterManager.OnTabSelected(event)
            return
        
        obj = event.GetEventObject()

        if obj and isinstance(obj, auibook.AuiNotebook):
        
            notebook = obj
            page = notebook.GetPage(event.GetSelection())
            paneInfo = self.GetPane(page)

            if paneInfo.IsOk():
                notebookRoot = GetNotebookRoot(self._panes, paneInfo.notebook_id)
                if notebookRoot:
                
                    notebookRoot.Caption(paneInfo.caption)
                    self.RefreshCaptions()
                
        event.Skip()


    def GetNotebooks(self):
        """ Returns all the automatic L{AuiNotebook} in the L{AuiManager}. """

        if self._masterManager:
            return self._masterManager.GetNotebooks()
        
        return self._notebooks


    def SetMasterManager(self, manager):
        """
        Sets the master manager for an automatic L{AuiNotebook}.

        :param `manager`: an instance of L{AuiManager}.
        """

        self._masterManager = manager

        
    def ProcessDockResult(self, target, new_pos):
        """
        This is a utility function used by L{DoDrop} - it checks
        if a dock operation is allowed, the new dock position is copied into
        the target info. If the operation was allowed, the function returns ``True``.

        :param `target`: the L{AuiPaneInfo} instance to be docked;
        :param `new_pos`: the new docking position if the docking operation is allowed.
        """

        allowed = False
        direction = new_pos.dock_direction
        
        if direction == AUI_DOCK_TOP:
            allowed = target.IsTopDockable()
        elif direction == AUI_DOCK_BOTTOM:
            allowed = target.IsBottomDockable()
        elif direction == AUI_DOCK_LEFT:
            allowed = target.IsLeftDockable()
        elif direction == AUI_DOCK_RIGHT:
            allowed = target.IsRightDockable()

        if allowed:
            target = new_pos

            if target.IsToolbar():
                self.SwitchToolBarOrientation(target)

        return allowed, target


    def SwitchToolBarOrientation(self, pane):
        """
        Switches the toolbar orientation from vertical to horizontal and vice-versa.
        This is especially useful for vertical docked toolbars once they float.

        :param `pane`: an instance of L{AuiPaneInfo}, which may have a L{AuiToolBar}
         window associated with it.
        """

        if not isinstance(pane.window, auibar.AuiToolBar):
            return pane
        
        if pane.IsFloating():
            return pane

        toolBar = pane.window
        direction = pane.dock_direction
        vertical = direction in [AUI_DOCK_LEFT, AUI_DOCK_RIGHT]

        agwStyle = toolBar.GetAGWWindowStyleFlag()
        new_agwStyle = agwStyle

        if vertical:
            new_agwStyle |= AUI_TB_VERTICAL
        else:
            new_agwStyle &= ~(AUI_TB_VERTICAL)

        if agwStyle != new_agwStyle:
            toolBar.SetAGWWindowStyleFlag(new_agwStyle)
        if not toolBar.GetGripperVisible():
            toolBar.SetGripperVisible(True)

        s = pane.window.GetMinSize()
        pane.BestSize(s)

        if new_agwStyle != agwStyle:
            toolBar.Realize()
        
        return pane


    def DoDrop(self, docks, panes, target, pt, offset=wx.Point(0, 0)):
        """
        This is an important function. It basically takes a mouse position,
        and determines where the panes new position would be. If the pane is to be
        dropped, it performs the drop operation using the specified dock and pane
        arrays. By specifying copy dock and pane arrays when calling, a "what-if"
        scenario can be performed, giving precise coordinates for drop hints.

        :param `docks`: a list of L{AuiDockInfo} classes;
        :param `panes`: a list of L{AuiPaneInfo} instances;
        :param `pt`: a mouse position to check for a drop operation;
        :param `offset`: a possible offset from the input point `pt`.
        """

        if target.IsToolbar():
            return self.DoDropToolbar(docks, panes, target, pt, offset)
        elif target.IsFloating():
            return self.DoDropFloatingPane(docks, panes, target, pt)
        else:
            return self.DoDropNonFloatingPane(docks, panes, target, pt)
    

    def CopyTarget(self, target):
        """
        Copies all the attributes of the input `target` into another L{AuiPaneInfo}.

        :param `target`: the source L{AuiPaneInfo} from where to copy attributes.
        """

        drop = AuiPaneInfo()
        drop.name = target.name
        drop.caption = target.caption
        drop.window = target.window
        drop.frame = target.frame
        drop.state = target.state
        drop.dock_direction = target.dock_direction
        drop.dock_layer = target.dock_layer
        drop.dock_row = target.dock_row
        drop.dock_pos = target.dock_pos
        drop.best_size = wx.Size(*target.best_size)
        drop.min_size = wx.Size(*target.min_size)
        drop.max_size = wx.Size(*target.max_size)
        drop.floating_pos = wx.Point(*target.floating_pos)
        drop.floating_size = wx.Size(*target.floating_size)
        drop.dock_proportion = target.dock_proportion
        drop.buttons = target.buttons
        drop.rect = wx.Rect(*target.rect)
        drop.icon = target.icon
        drop.notebook_id = target.notebook_id
        drop.transparent = target.transparent
        drop.snapped = target.snapped
        drop.minimize_mode = target.minimize_mode

        return drop        


    def DoDropToolbar(self, docks, panes, target, pt, offset):
        """
        Handles the situation in which the dropped pane contains a toolbar.

        :param `docks`: a list of L{AuiDockInfo} classes;
        :param `panes`: a list of L{AuiPaneInfo} instances;
        :param `target`: the target pane containing the toolbar;
        :param `pt`: a mouse position to check for a drop operation;
        :param `offset`: a possible offset from the input point `pt`.        
        """
        
        drop = self.CopyTarget(target)

        # The result should always be shown
        drop.Show()

        # Check to see if the toolbar has been dragged out of the window
        if CheckOutOfWindow(self._frame, pt):
            if self._agwFlags & AUI_MGR_ALLOW_FLOATING and drop.IsFloatable():
                drop.Float()

            return self.ProcessDockResult(target, drop)

        # Allow directional change when the cursor leaves this rect
        safeRect = wx.Rect(*target.rect)
        if target.IsHorizontal():
            safeRect.Inflate(100, 50)
        else:
            safeRect.Inflate(50, 100)
        
        # Check to see if the toolbar has been dragged to edge of the frame
        dropDir = CheckEdgeDrop(self._frame, docks, pt)

        if dropDir != -1:
        
            if dropDir == wx.LEFT:
                drop.Dock().Left().Layer(auiToolBarLayer).Row(0). \
                    Position(pt.y - self.GetDockPixelOffset(drop) - offset.y)

            elif dropDir == wx.RIGHT:
                drop.Dock().Right().Layer(auiToolBarLayer).Row(0). \
                    Position(pt.y - self.GetDockPixelOffset(drop) - offset.y)

            elif dropDir == wx.TOP:
                drop.Dock().Top().Layer(auiToolBarLayer).Row(0). \
                    Position(pt.x - self.GetDockPixelOffset(drop) - offset.x)

            elif dropDir == wx.BOTTOM:
                drop.Dock().Bottom().Layer(auiToolBarLayer).Row(0). \
                    Position(pt.x - self.GetDockPixelOffset(drop) - offset.x)

            if not target.IsFloating() and safeRect.Contains(pt) and \
               target.dock_direction != drop.dock_direction:
                return False, target
        
            return self.ProcessDockResult(target, drop)
    
        # If the windows is floating and out of the client area, do nothing
        if drop.IsFloating() and not self._frame.GetClientRect().Contains(pt):
            return False, target
    
        # Ok, can't drop on edge - check internals ...

        clientSize = self._frame.GetClientSize()
        x = Clip(pt.x, 0, clientSize.x - 1)
        y = Clip(pt.y, 0, clientSize.y - 1)
        part = self.HitTest(x, y)

        if not part or not part.dock:
            return False, target
        
        dock = part.dock
        
        # toolbars may only be moved in and to fixed-pane docks,
        # otherwise we will try to float the pane.  Also, the pane
        # should float if being dragged over center pane windows
        if not dock.fixed or dock.dock_direction == AUI_DOCK_CENTER:
        
            if (self._agwFlags & AUI_MGR_ALLOW_FLOATING and drop.IsFloatable()) or \
               dock.dock_direction not in [AUI_DOCK_CENTER, AUI_DOCK_NONE]:
                if drop.IsFloatable():
                    drop.Float()
            
            return self.ProcessDockResult(target, drop)
        
        # calculate the offset from where the dock begins
        # to the point where the user dropped the pane
        dockDropOffset = 0
        if dock.IsHorizontal():
            dockDropOffset = pt.x - dock.rect.x - offset.x
        else:
            dockDropOffset = pt.y - dock.rect.y - offset.y
        
        drop.Dock().Direction(dock.dock_direction).Layer(dock.dock_layer). \
            Row(dock.dock_row).Position(dockDropOffset)

        if (pt.y <= dock.rect.GetTop() + 2 and dock.IsHorizontal()) or \
           (pt.x <= dock.rect.GetLeft() + 2 and dock.IsVertical()):
        
            if dock.dock_direction in [AUI_DOCK_TOP, AUI_DOCK_LEFT]:
                row = drop.dock_row
                panes = DoInsertDockRow(panes, dock.dock_direction, dock.dock_layer, dock.dock_row)
                drop.dock_row = row
            
            else:
                panes = DoInsertDockRow(panes, dock.dock_direction, dock.dock_layer, dock.dock_row+1)
                drop.dock_row = dock.dock_row + 1
            
        if (pt.y >= dock.rect.GetBottom() - 2 and dock.IsHorizontal()) or \
           (pt.x >= dock.rect.GetRight() - 2 and dock.IsVertical()):
        
            if dock.dock_direction in [AUI_DOCK_TOP, AUI_DOCK_LEFT]:
                panes = DoInsertDockRow(panes, dock.dock_direction, dock.dock_layer, dock.dock_row+1)
                drop.dock_row = dock.dock_row+1
            
            else:
                row = drop.dock_row
                panes = DoInsertDockRow(panes, dock.dock_direction, dock.dock_layer, dock.dock_row)
                drop.dock_row = row

        if not target.IsFloating() and safeRect.Contains(pt) and \
           target.dock_direction != drop.dock_direction:
            return False, target
    
        return self.ProcessDockResult(target, drop)


    def DoDropFloatingPane(self, docks, panes, target, pt):
        """
        Handles the situation in which the dropped pane contains a normal window.

        :param `docks`: a list of L{AuiDockInfo} classes;
        :param `panes`: a list of L{AuiPaneInfo} instances;
        :param `target`: the target pane containing the window;
        :param `pt`: a mouse position to check for a drop operation.
        """
        
        screenPt = self._frame.ClientToScreen(pt)
        paneInfo = self.PaneHitTest(panes, pt)

        if paneInfo.IsMaximized():
            return False, target

        if paneInfo.window is None:
            return False, target

        # search the dock guides.
        # reverse order to handle the center first.
        for i in xrange(len(self._guides)-1, -1, -1):
            guide = self._guides[i]

            # do hit testing on the guide
            dir = guide.host.HitTest(screenPt.x, screenPt.y)

            if dir == -1:  # point was outside of the dock guide
                continue

            if dir == wx.ALL:   # target is a single dock guide
                return self.DoDropLayer(docks, target, guide.dock_direction)
            
            elif dir == wx.CENTER:

                if not target.IsNotebookDockable():
                    continue
                if not paneInfo.IsNotebookDockable() and not paneInfo.IsNotebookControl():
                    continue

                if not paneInfo.HasNotebook():
                
                    # Add a new notebook pane with the original as a tab...
                    self.CreateNotebookBase(panes, paneInfo)
                
                # Add new item to notebook
                target.NotebookPage(paneInfo.notebook_id)
            
            else:
            
                drop_pane = False
                drop_row = False

                insert_dir = paneInfo.dock_direction
                insert_layer = paneInfo.dock_layer
                insert_row = paneInfo.dock_row
                insert_pos = paneInfo.dock_pos

                if insert_dir == AUI_DOCK_CENTER:
                
                    insert_layer = 0
                    if dir == wx.LEFT:
                        insert_dir = AUI_DOCK_LEFT
                    elif dir == wx.UP:
                        insert_dir = AUI_DOCK_TOP
                    elif dir == wx.RIGHT:
                        insert_dir = AUI_DOCK_RIGHT
                    elif dir == wx.DOWN:
                        insert_dir = AUI_DOCK_BOTTOM
                
                if insert_dir == AUI_DOCK_LEFT:
                
                    drop_pane = (dir == wx.UP   or dir == wx.DOWN)
                    drop_row  = (dir == wx.LEFT or dir == wx.RIGHT)
                    if dir == wx.RIGHT:
                        insert_row += 1
                    elif dir == wx.DOWN:
                        insert_pos += 1
                
                elif insert_dir == AUI_DOCK_RIGHT:
                
                    drop_pane = (dir == wx.UP   or dir == wx.DOWN)
                    drop_row  = (dir == wx.LEFT or dir == wx.RIGHT)
                    if dir == wx.LEFT:
                        insert_row += 1
                    elif dir == wx.DOWN:
                        insert_pos += 1
                
                elif insert_dir == AUI_DOCK_TOP:
                
                    drop_pane = (dir == wx.LEFT or dir == wx.RIGHT)
                    drop_row  = (dir == wx.UP   or dir == wx.DOWN)
                    if dir == wx.DOWN:
                        insert_row += 1
                    elif dir == wx.RIGHT:
                        insert_pos += 1
                
                elif insert_dir == AUI_DOCK_BOTTOM:
                
                    drop_pane = (dir == wx.LEFT or dir == wx.RIGHT)
                    drop_row  = (dir == wx.UP   or dir == wx.DOWN)
                    if dir == wx.UP:
                        insert_row += 1
                    elif dir == wx.RIGHT:
                        insert_pos += 1
                
                if paneInfo.dock_direction == AUI_DOCK_CENTER:
                    insert_row = GetMaxRow(panes, insert_dir, insert_layer) + 1

                if drop_pane:
                    return self.DoDropPane(panes, target, insert_dir, insert_layer, insert_row, insert_pos)

                if drop_row:
                    return self.DoDropRow(panes, target, insert_dir, insert_layer, insert_row)
            
            return True, target
        
        return False, target


    def DoDropNonFloatingPane(self, docks, panes, target, pt):
        """
        Handles the situation in which the dropped pane is not floating.

        :param `docks`: a list of L{AuiDockInfo} classes;
        :param `panes`: a list of L{AuiPaneInfo} instances;
        :param `target`: the target pane containing the toolbar;
        :param `pt`: a mouse position to check for a drop operation.
        """
        
        screenPt = self._frame.ClientToScreen(pt)
        clientSize = self._frame.GetClientSize()
        frameRect = GetInternalFrameRect(self._frame, self._docks)

        drop = self.CopyTarget(target)

        # The result should always be shown
        drop.Show()

        part = self.HitTest(pt.x, pt.y)

        if not part:
            return False, target

        if part.type == AuiDockUIPart.typeDockSizer:
        
            if len(part.dock.panes) != 1:
                return False, target
            
            part = self.GetPanePart(part.dock.panes[0].window)
            if not part:
                return False, target
        
        if not part.pane:
            return False, target

        part = self.GetPanePart(part.pane.window)
        if not part:
            return False, target

        insert_dock_row = False
        insert_row = part.pane.dock_row
        insert_dir = part.pane.dock_direction
        insert_layer = part.pane.dock_layer

        direction = part.pane.dock_direction
        
        if direction == AUI_DOCK_TOP:
            if pt.y >= part.rect.y and pt.y < part.rect.y+auiInsertRowPixels:
                insert_dock_row = True

        elif direction == AUI_DOCK_BOTTOM:
            if pt.y > part.rect.y+part.rect.height-auiInsertRowPixels and \
               pt.y <= part.rect.y + part.rect.height:
                insert_dock_row = True

        elif direction == AUI_DOCK_LEFT:
            if pt.x >= part.rect.x and pt.x < part.rect.x+auiInsertRowPixels:
                insert_dock_row = True

        elif direction == AUI_DOCK_RIGHT:
            if pt.x > part.rect.x+part.rect.width-auiInsertRowPixels and \
               pt.x <= part.rect.x+part.rect.width:
                insert_dock_row = True

        elif direction == AUI_DOCK_CENTER:
            
                # "new row pixels" will be set to the default, but
                # must never exceed 20% of the window size
                new_row_pixels_x = auiNewRowPixels
                new_row_pixels_y = auiNewRowPixels

                if new_row_pixels_x > (part.rect.width*20)/100:
                    new_row_pixels_x = (part.rect.width*20)/100

                if new_row_pixels_y > (part.rect.height*20)/100:
                    new_row_pixels_y = (part.rect.height*20)/100

                # determine if the mouse pointer is in a location that
                # will cause a new row to be inserted.  The hot spot positions
                # are along the borders of the center pane

                insert_layer = 0
                insert_dock_row = True
                pr = part.rect
                
                if pt.x >= pr.x and pt.x < pr.x + new_row_pixels_x:
                    insert_dir = AUI_DOCK_LEFT
                elif pt.y >= pr.y and pt.y < pr.y + new_row_pixels_y:
                    insert_dir = AUI_DOCK_TOP
                elif pt.x >= pr.x + pr.width - new_row_pixels_x and pt.x < pr.x + pr.width:
                    insert_dir = AUI_DOCK_RIGHT
                elif pt.y >= pr.y+ pr.height - new_row_pixels_y and pt.y < pr.y + pr.height:
                    insert_dir = AUI_DOCK_BOTTOM
                else:
                    return False, target

                insert_row = GetMaxRow(panes, insert_dir, insert_layer) + 1
            
        if insert_dock_row:
        
            panes = DoInsertDockRow(panes, insert_dir, insert_layer, insert_row)
            drop.Dock().Direction(insert_dir).Layer(insert_layer). \
                Row(insert_row).Position(0)
                
            return self.ProcessDockResult(target, drop)

        # determine the mouse offset and the pane size, both in the
        # direction of the dock itself, and perpendicular to the dock

        if part.orientation == wx.VERTICAL:
        
            offset = pt.y - part.rect.y
            size = part.rect.GetHeight()
        
        else:
        
            offset = pt.x - part.rect.x
            size = part.rect.GetWidth()
        
        drop_position = part.pane.dock_pos

        # if we are in the top/left part of the pane,
        # insert the pane before the pane being hovered over
        if offset <= size/2:
        
            drop_position = part.pane.dock_pos
            panes = DoInsertPane(panes,
                                 part.pane.dock_direction,
                                 part.pane.dock_layer,
                                 part.pane.dock_row,
                                 part.pane.dock_pos)

        # if we are in the bottom/right part of the pane,
        # insert the pane before the pane being hovered over
        if offset > size/2:
        
            drop_position = part.pane.dock_pos+1
            panes = DoInsertPane(panes,
                                 part.pane.dock_direction,
                                 part.pane.dock_layer,
                                 part.pane.dock_row,
                                 part.pane.dock_pos+1)
        

        drop.Dock(). \
                     Direction(part.dock.dock_direction). \
                     Layer(part.dock.dock_layer).Row(part.dock.dock_row). \
                     Position(drop_position)
        
        return self.ProcessDockResult(target, drop)


    def DoDropLayer(self, docks, target, dock_direction):
        """
        Handles the situation in which `target` is a single dock guide.

        :param `docks`: a list of L{AuiDockInfo} classes;
        :param `target`: the target pane;
        :param `dock_direction`: the docking direction.
        """

        drop = self.CopyTarget(target)
        
        if dock_direction == AUI_DOCK_LEFT:
            drop.Dock().Left()
            drop_new_layer = max(max(GetMaxLayer(docks, AUI_DOCK_LEFT),
                                     GetMaxLayer(docks, AUI_DOCK_BOTTOM)),
                                 GetMaxLayer(docks, AUI_DOCK_TOP)) + 1

        elif dock_direction == AUI_DOCK_TOP:
            drop.Dock().Top()
            drop_new_layer = max(max(GetMaxLayer(docks, AUI_DOCK_TOP),
                                     GetMaxLayer(docks, AUI_DOCK_LEFT)),
                                 GetMaxLayer(docks, AUI_DOCK_RIGHT)) + 1

        elif dock_direction == AUI_DOCK_RIGHT:
            drop.Dock().Right()
            drop_new_layer = max(max(GetMaxLayer(docks, AUI_DOCK_RIGHT),
                                     GetMaxLayer(docks, AUI_DOCK_TOP)),
                                 GetMaxLayer(docks, AUI_DOCK_BOTTOM)) + 1

        elif dock_direction == AUI_DOCK_BOTTOM:
            drop.Dock().Bottom()
            drop_new_layer = max(max(GetMaxLayer(docks, AUI_DOCK_BOTTOM),
                                     GetMaxLayer(docks, AUI_DOCK_LEFT)),
                                 GetMaxLayer(docks, AUI_DOCK_RIGHT)) + 1

        else:
            return False, target
        

        drop.Dock().Layer(drop_new_layer)
        return self.ProcessDockResult(target, drop)


    def DoDropPane(self, panes, target, dock_direction, dock_layer, dock_row, dock_pos):
        """
        Drop a pane in the interface.

        :param `panes`: a list of L{AuiPaneInfo} classes;
        :param `target`: the target pane;
        :param `dock_direction`: the docking direction;
        :param `dock_layer`: the docking layer;
        :param `dock_row`: the docking row;
        :param `dock_pos`: the docking position.
        """
        
        drop = self.CopyTarget(target)
        panes = DoInsertPane(panes, dock_direction, dock_layer, dock_row, dock_pos)

        drop.Dock().Direction(dock_direction).Layer(dock_layer).Row(dock_row).Position(dock_pos)
        return self.ProcessDockResult(target, drop)


    def DoDropRow(self, panes, target, dock_direction, dock_layer, dock_row):
        """
        Insert a row in the interface before dropping.

        :param `panes`: a list of L{AuiPaneInfo} classes;
        :param `target`: the target pane;
        :param `dock_direction`: the docking direction;
        :param `dock_layer`: the docking layer;
        :param `dock_row`: the docking row.
        """
        
        drop = self.CopyTarget(target)
        panes = DoInsertDockRow(panes, dock_direction, dock_layer, dock_row)

        drop.Dock().Direction(dock_direction).Layer(dock_layer).Row(dock_row).Position(0)
        return self.ProcessDockResult(target, drop)


    def ShowHint(self, rect):
        """
        Shows the AUI hint window.

        :param `rect`: the hint rect calculated in advance.
        """

        if rect == self._last_hint:
            return

        if self._agwFlags & AUI_MGR_RECTANGLE_HINT and wx.Platform != "__WXMAC__":

            if self._last_hint != rect:
                # remove the last hint rectangle
                self._last_hint = wx.Rect(*rect)
                self._frame.Refresh()
                self._frame.Update()
            
            screendc = wx.ScreenDC()
            clip = wx.Region(1, 1, 10000, 10000)

            # clip all floating windows, so we don't draw over them
            for pane in self._panes:            
                if pane.IsFloating() and pane.frame.IsShown():
                
                    rect2 = wx.Rect(*pane.frame.GetRect())
                    if wx.Platform == "__WXGTK__":
                        # wxGTK returns the client size, not the whole frame size
                        rect2.width += 15
                        rect2.height += 35
                        rect2.Inflate(5, 5)

                    clip.SubtractRect(rect2)
                
            # As we can only hide the hint by redrawing the managed window, we
            # need to clip the region to the managed window too or we get
            # nasty redrawn problems.
            clip.IntersectRect(self._frame.GetRect())
            screendc.SetClippingRegionAsRegion(clip)

            stipple = PaneCreateStippleBitmap()
            brush = wx.BrushFromBitmap(stipple)
            screendc.SetBrush(brush)
            screendc.SetPen(wx.TRANSPARENT_PEN)
            screendc.DrawRectangle(rect.x, rect.y, 5, rect.height)
            screendc.DrawRectangle(rect.x+5, rect.y, rect.width-10, 5)
            screendc.DrawRectangle(rect.x+rect.width-5, rect.y, 5, rect.height)
            screendc.DrawRectangle(rect.x+5, rect.y+rect.height-5, rect.width-10, 5)
            RefreshDockingGuides(self._guides)

            return
            
        if not self._hint_window:
            self.CreateHintWindow()

        if self._hint_window:
            self._hint_window.SetRect(rect)
            self._hint_window.Show()

        self._hint_fadeamt = self._hint_fademax

        if self._agwFlags & AUI_MGR_HINT_FADE:
            self._hint_fadeamt = 0
            self._hint_window.SetTransparent(self._hint_fadeamt)

        if self._action == actionDragFloatingPane and self._action_window:
            self._action_window.SetFocus()

        if self._hint_fadeamt != self._hint_fademax: #  Only fade if we need to
            # start fade in timer
            self._hint_fadetimer.Start(5)

        self._last_hint = wx.Rect(*rect)
        

    def HideHint(self):
        """ Hides a transparent window hint if there is one. """

        # hides a transparent window hint if there is one
        if self._hint_window:
            self._hint_window.Hide()

        self._hint_fadetimer.Stop()
        self._last_hint = wx.Rect()
        

    def IsPaneButtonVisible(self, part):
        """
        Returns whether a pane button in the pane caption is visible.

        :param `part`: the UI part to analyze.
        """

        captionRect = wx.Rect()

        for temp_part in self._uiparts:
            if temp_part.pane == part.pane and \
               temp_part.type == AuiDockUIPart.typeCaption:
                captionRect = temp_part.rect
                break

        return captionRect.ContainsRect(part.rect)


    def DrawPaneButton(self, dc, part, pt):
        """
        Draws a pane button in the caption (convenience function).

        :param `dc`: a `wx.DC` device context object;
        :param `part`: the UI part to analyze;
        :param `pt`: a `wx.Point` object, specifying the mouse location.
        """

        if not self.IsPaneButtonVisible(part):
            return

        state = AUI_BUTTON_STATE_NORMAL

        if part.rect.Contains(pt):

            if _VERSION_STRING < "2.9":
                leftDown = wx.GetMouseState().LeftDown()
            else:
                leftDown = wx.GetMouseState().LeftIsDown()

            if leftDown:
                state = AUI_BUTTON_STATE_PRESSED
            else:
                state = AUI_BUTTON_STATE_HOVER

        self._art.DrawPaneButton(dc, self._frame, part.button.button_id,
                                 state, part.rect, part.pane)

    
    def RefreshButton(self, part):
        """
        Refreshes a pane button in the caption.

        :param `part`: the UI part to analyze.
        """
        
        rect = wx.Rect(*part.rect)
        rect.Inflate(2, 2)
        self._frame.Refresh(True, rect)
        self._frame.Update()


    def RefreshCaptions(self):
        """ Refreshes all pane captions. """

        for part in self._uiparts:
            if part.type == AuiDockUIPart.typeCaption:
                self._frame.Refresh(True, part.rect)
                self._frame.Update()


    def CalculateHintRect(self, pane_window, pt, offset):
        """
        Calculates the drop hint rectangle.

        The method first calls L{DoDrop} to determine the exact position the pane would
        be at were if dropped. If the pane would indeed become docked at the
        specified drop point, the the rectangle hint will be returned in
        screen coordinates. Otherwise, an empty rectangle is returned.

        :param `pane_window`: it is the window pointer of the pane being dragged;
        :param `pt`: is the mouse position, in client coordinates;
        :param `offset`: describes the offset that the mouse is from the upper-left
         corner of the item being dragged.
        """
        
        # we need to paint a hint rectangle to find out the exact hint rectangle,
        # we will create a new temporary layout and then measure the resulting
        # rectangle we will create a copy of the docking structures (self._docks)
        # so that we don't modify the real thing on screen

        rect = wx.Rect()
        pane = self.GetPane(pane_window)
        
        attrs = self.GetAttributes(pane)
        hint = AuiPaneInfo()
        hint = self.SetAttributes(hint, attrs)
        
        if hint.name != "__HINT__":
            self._oldname = hint.name
            
        hint.name = "__HINT__"
        hint.PaneBorder(True)
        hint.Show()

        if not hint.IsOk():
            hint.name = self._oldname
            return rect

        docks, panes = CopyDocksAndPanes2(self._docks, self._panes)

        # remove any pane already there which bears the same window
        # this happens when you are moving a pane around in a dock
        for ii in xrange(len(panes)):
            if panes[ii].window == pane_window:
                docks = RemovePaneFromDocks(docks, panes[ii])
                panes.pop(ii)
                break

        # find out where the new pane would be
        allow, hint = self.DoDrop(docks, panes, hint, pt, offset)

        if not allow:
            return rect
        
        panes.append(hint)

        sizer, panes, docks, uiparts = self.LayoutAll(panes, docks, [], True, False)
        
        client_size = self._frame.GetClientSize()
        sizer.SetDimension(0, 0, client_size.x, client_size.y)
        sizer.Layout()

        sought = "__HINT__"
        
        # For a notebook page, actually look for the noteboot itself.
        if hint.IsNotebookPage():
            id = hint.notebook_id
            for pane in panes:
                if pane.IsNotebookControl() and pane.notebook_id==id:
                    sought = pane.name
                    break

        for part in uiparts:
            if part.pane and part.pane.name == sought:    
                rect.Union(wx.RectPS(part.sizer_item.GetPosition(),
                                     part.sizer_item.GetSize()))

        sizer.Destroy()

        # check for floating frame ...
        if rect.IsEmpty():
            for p in panes:
                if p.name == sought and p.IsFloating():
                    return wx.RectPS(p.floating_pos, p.floating_size)
    
        if rect.IsEmpty():
            return rect

        # actually show the hint rectangle on the screen
        rect.x, rect.y = self._frame.ClientToScreen((rect.x, rect.y))
        if self._frame.GetLayoutDirection() == wx.Layout_RightToLeft:
            # Mirror rectangle in RTL mode
            rect.x -= rect.GetWidth()

        return rect


    def DrawHintRect(self, pane_window, pt, offset):
        """
        Calculates the hint rectangle by calling
        L{CalculateHintRect}. If there is a rectangle, it shows it
        by calling L{ShowHint}, otherwise it hides any hint
        rectangle currently shown.

        :param `pane_window`: it is the window pointer of the pane being dragged;
        :param `pt`: is the mouse position, in client coordinates;
        :param `offset`: describes the offset that the mouse is from the upper-left
         corner of the item being dragged.
        """

        rect = self.CalculateHintRect(pane_window, pt, offset)

        if rect.IsEmpty():
            self.HideHint()
            self._hint_rect = wx.Rect()
        else:
            self.ShowHint(rect)
            self._hint_rect = wx.Rect(*rect)


    def GetPartSizerRect(self, uiparts):
        """
        Returns the rectangle surrounding the specified UI parts.

        :param `uiparts`: UI parts.
        """

        rect = wx.Rect()

        for part in self._uiparts:
            if part.pane and part.pane.name == "__HINT__":
                rect.Union(wx.RectPS(part.sizer_item.GetPosition(),
                                     part.sizer_item.GetSize()))

        return rect


    def GetAttributes(self, pane):
        """
        Returns all the attributes of a L{AuiPaneInfo}.

        :param `pane`: a L{AuiPaneInfo} instance.
        """

        attrs = []
        attrs.extend([pane.window, pane.frame, pane.state, pane.dock_direction,
                      pane.dock_layer, pane.dock_pos, pane.dock_row, pane.dock_proportion,
                      pane.floating_pos, pane.floating_size, pane.best_size,
                      pane.min_size, pane.max_size, pane.caption, pane.name,
                      pane.buttons, pane.rect, pane.icon, pane.notebook_id,
                      pane.transparent, pane.snapped, pane.minimize_mode])

        return attrs
    

    def SetAttributes(self, pane, attrs):
        """
        Sets all the attributes contained in `attrs` to a L{AuiPaneInfo}.

        :param `pane`: a L{AuiPaneInfo} instance;
        :param `attrs`: a list of attributes.
        """
        
        pane.window = attrs[0]
        pane.frame = attrs[1]
        pane.state = attrs[2]
        pane.dock_direction = attrs[3]
        pane.dock_layer = attrs[4]
        pane.dock_pos = attrs[5]
        pane.dock_row = attrs[6]
        pane.dock_proportion = attrs[7]
        pane.floating_pos = attrs[8]
        pane.floating_size = attrs[9]
        pane.best_size = attrs[10]
        pane.min_size = attrs[11]
        pane.max_size = attrs[12]
        pane.caption = attrs[13]
        pane.name = attrs[14]
        pane.buttons = attrs[15]
        pane.rect = attrs[16]
        pane.icon = attrs[17]
        pane.notebook_id = attrs[18]
        pane.transparent = attrs[19]
        pane.snapped = attrs[20]
        pane.minimize_mode = attrs[21]

        return pane

     
    def OnFloatingPaneResized(self, wnd, size):
        """
        Handles the resizing of a floating pane.

        :param `wnd`: a `wx.Window` derived window, managed by the pane;
        :param `size`: a `wx.Size` object, specifying the new pane floating size.
        """

        # try to find the pane
        pane = self.GetPane(wnd)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        if pane.frame:
            indx = self._panes.index(pane)
            pane.floating_pos = pane.frame.GetPosition()
            pane.floating_size = size
            self._panes[indx] = pane
            if pane.IsSnappable():
                self.SnapPane(pane, pane.floating_pos, pane.floating_size, True)
            

    def OnFloatingPaneClosed(self, wnd, event):
        """
        Handles the close event of a floating pane.

        :param `wnd`: a `wx.Window` derived window, managed by the pane;
        :param `event`: a `wx.CloseEvent` to be processed.
        """
        
        # try to find the pane
        pane = self.GetPane(wnd)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        # fire pane close event
        e = AuiManagerEvent(wxEVT_AUI_PANE_CLOSE)
        e.SetPane(pane)
        e.SetCanVeto(event.CanVeto())
        self.ProcessMgrEvent(e)

        if e.GetVeto():
            event.Veto()
            return
        else:
            # close the pane, but check that it
            # still exists in our pane array first
            # (the event handler above might have removed it)

            check = self.GetPane(wnd)
            if check.IsOk():
                self.ClosePane(pane)
        

    def OnFloatingPaneActivated(self, wnd):
        """
        Handles the activation event of a floating pane.

        :param `wnd`: a `wx.Window` derived window, managed by the pane.
        """
        
        pane = self.GetPane(wnd)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        if self.GetAGWFlags() & AUI_MGR_ALLOW_ACTIVE_PANE:
            ret, self._panes = SetActivePane(self._panes, wnd)
            self.RefreshCaptions()
            self.FireEvent(wxEVT_AUI_PANE_ACTIVATED, wnd, canVeto=False)


    def OnFloatingPaneMoved(self, wnd, eventOrPt):
        """
        Handles the move event of a floating pane.

        :param `wnd`: a `wx.Window` derived window, managed by the pane;
        :param `eventOrPt`: a `wx.MoveEvent` to be processed or an instance of `wx.Point`.
        """
        
        pane = self.GetPane(wnd)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        if not pane.IsSnappable():
            return

        if isinstance(eventOrPt, wx.Point):
            pane_pos = wx.Point(*eventOrPt)
        else:
            pane_pos = eventOrPt.GetPosition()

        pane_size = pane.floating_size

        self.SnapPane(pane, pane_pos, pane_size, False)


    def SnapPane(self, pane, pane_pos, pane_size, toSnap=False):
        """
        Snaps a floating pane to one of the main frame sides.

        :param `pane`: a L{AuiPaneInfo} instance;
        :param `pane_pos`: the new pane floating position;
        :param `pane_size`: the new pane floating size;
        :param `toSnap`: a bool variable to check if L{SnapPane} was called from
         a move event.
        """

        if self._from_move:
            return
        
        managed_window = self.GetManagedWindow()
        wnd_pos = managed_window.GetPosition()
        wnd_size = managed_window.GetSize()
        snapX, snapY = self._snap_limits

        if not toSnap:
            pane.snapped = 0
            if pane.IsLeftSnappable():
                # Check if we can snap to the left
                diff = wnd_pos.x - (pane_pos.x + pane_size.x)
                if -snapX <= diff <= snapX:
                    pane.snapped = wx.LEFT
                    pane.floating_pos = wx.Point(wnd_pos.x-pane_size.x, pane_pos.y)
            elif pane.IsTopSnappable():
                # Check if we can snap to the top
                diff = wnd_pos.y - (pane_pos.y + pane_size.y)
                if -snapY <= diff <= snapY:
                    pane.snapped = wx.TOP
                    pane.floating_pos = wx.Point(pane_pos.x, wnd_pos.y-pane_size.y)
            elif pane.IsRightSnappable():
                # Check if we can snap to the right
                diff = pane_pos.x - (wnd_pos.x + wnd_size.x)
                if -snapX <= diff <= snapX:
                    pane.snapped = wx.RIGHT
                    pane.floating_pos = wx.Point(wnd_pos.x + wnd_size.x, pane_pos.y)
            elif pane.IsBottomSnappable():
                # Check if we can snap to the bottom
                diff = pane_pos.y - (wnd_pos.y + wnd_size.y)
                if -snapY <= diff <= snapY:
                    pane.snapped = wx.BOTTOM
                    pane.floating_pos = wx.Point(pane_pos.x, wnd_pos.y + wnd_size.y)

        self.RepositionPane(pane, wnd_pos, wnd_size)


    def RepositionPane(self, pane, wnd_pos, wnd_size):
        """
        Repositions a pane after the main frame has been moved/resized.
        
        :param `pane`: a L{AuiPaneInfo} instance;
        :param `wnd_pos`: the main frame position;
        :param `wnd_size`: the main frame size.
        """

        pane_pos = pane.floating_pos
        pane_size = pane.floating_size

        snap = pane.snapped
        if snap == wx.LEFT:
            floating_pos = wx.Point(wnd_pos.x - pane_size.x, pane_pos.y)
        elif snap == wx.TOP:
            floating_pos = wx.Point(pane_pos.x, wnd_pos.y - pane_size.y)
        elif snap == wx.RIGHT:
            floating_pos = wx.Point(wnd_pos.x + wnd_size.x, pane_pos.y)
        elif snap == wx.BOTTOM:
            floating_pos = wx.Point(pane_pos.x, wnd_pos.y + wnd_size.y)

        if snap:
            if pane_pos != floating_pos:
                pane.floating_pos = floating_pos
                self._from_move = True
                pane.frame.SetPosition(pane.floating_pos)
                self._from_move = False

            
    def OnGripperClicked(self, pane_window, start, offset):
        """
        Handles the mouse click on the pane gripper.

        :param `pane_window`: a `wx.Window` derived window, managed by the pane;
        :param `start`: a `wx.Point` object, specifying the clicking position;
        :param `offset`: an offset point from the `start` position.
        """

        # try to find the pane
        paneInfo = self.GetPane(pane_window)

        if not paneInfo.IsOk():
            raise Exception("Pane window not found")

        if self.GetAGWFlags() & AUI_MGR_ALLOW_ACTIVE_PANE:
            # set the caption as active
            ret, self._panes = SetActivePane(self._panes, pane_window)
            self.RefreshCaptions()
            self.FireEvent(wxEVT_AUI_PANE_ACTIVATED, pane_window, canVeto=False)
        
        self._action_part = None
        self._action_pane = paneInfo
        self._action_window = pane_window
        self._action_start = start
        self._action_offset = offset
        self._toolbar_action_offset = wx.Point(*self._action_offset)
        
        self._frame.CaptureMouse()

        if paneInfo.IsDocked():
            self._action = actionClickCaption
        else:
            if paneInfo.IsToolbar():
                self._action = actionDragToolbarPane
            else:
                self._action = actionDragFloatingPane

            if paneInfo.frame:
            
                windowPt = paneInfo.frame.GetRect().GetTopLeft()
                originPt = paneInfo.frame.ClientToScreen(wx.Point())
                self._action_offset += originPt - windowPt
                self._toolbar_action_offset = wx.Point(*self._action_offset)

                if self._agwFlags & AUI_MGR_TRANSPARENT_DRAG:
                    paneInfo.frame.SetTransparent(150)
            
            if paneInfo.IsToolbar():
                self._frame.SetCursor(wx.StockCursor(wx.CURSOR_SIZING))
        

    def OnRender(self, event):        
        """
        Draws all of the pane captions, sashes,
        backgrounds, captions, grippers, pane borders and buttons.
        It renders the entire user interface. It binds the ``EVT_AUI_RENDER`` event.

        :param `event`: an instance of L{AuiManagerEvent}.
        """

        # if the frame is about to be deleted, don't bother
        if not self._frame or self._frame.IsBeingDeleted():
            return
        
        if not self._frame.GetSizer():
            return

        mouse = wx.GetMouseState()
        mousePos = wx.Point(mouse.GetX(), mouse.GetY())
        point = self._frame.ScreenToClient(mousePos)
        art = self._art

        dc = event.GetDC()
        
        for part in self._uiparts:
        
            # don't draw hidden pane items or items that aren't windows
            if part.sizer_item and ((not part.sizer_item.IsWindow() and \
                                     not part.sizer_item.IsSpacer() and \
                                     not part.sizer_item.IsSizer()) or \
                                    not part.sizer_item.IsShown()):
            
                continue
            
            ptype = part.type
                    
            if ptype in [AuiDockUIPart.typeDockSizer, AuiDockUIPart.typePaneSizer]:
                art.DrawSash(dc, self._frame, part.orientation, part.rect)

            elif ptype == AuiDockUIPart.typeBackground:
                art.DrawBackground(dc, self._frame, part.orientation, part.rect)

            elif ptype == AuiDockUIPart.typeCaption:
                art.DrawCaption(dc, self._frame, part.pane.caption, part.rect, part.pane)

            elif ptype == AuiDockUIPart.typeGripper:
                art.DrawGripper(dc, self._frame, part.rect, part.pane)

            elif ptype == AuiDockUIPart.typePaneBorder:
                art.DrawBorder(dc, self._frame, part.rect, part.pane)

            elif ptype == AuiDockUIPart.typePaneButton:                
                self.DrawPaneButton(dc, part, point)


    def Repaint(self, dc=None):
        """
        Repaints the entire frame decorations (sashes, borders, buttons and so on).
        It renders the entire user interface.

        :param `dc`: if not ``None``, an instance of `wx.PaintDC`.
        """
        
        w, h = self._frame.GetClientSize()

        # Figure out which dc to use; if one
        # has been specified, use it, otherwise
        # make a client dc
        if dc is None:
            client_dc = wx.ClientDC(self._frame)
            dc = client_dc

        # If the frame has a toolbar, the client area
        # origin will not be (0, 0).
        pt = self._frame.GetClientAreaOrigin()
        if pt.x != 0 or pt.y != 0:
            dc.SetDeviceOrigin(pt.x, pt.y)

        # Render all the items
        self.Render(dc)

                
    def Render(self, dc):
        """
        Fires a render event, which is normally handled by
        L{OnRender}. This allows the render function to
        be overridden via the render event.

        This can be useful for painting custom graphics in the main window.
        Default behavior can be invoked in the overridden function by calling
        L{OnRender}.

        :param `dc`: a `wx.DC` device context object.        
        """

        e = AuiManagerEvent(wxEVT_AUI_RENDER)
        e.SetManager(self)
        e.SetDC(dc)
        self.ProcessMgrEvent(e)


    def OnCaptionDoubleClicked(self, pane_window):
        """
        Handles the mouse double click on the pane caption.

        :param `pane_window`: a `wx.Window` derived window, managed by the pane.
        """

        # try to find the pane
        paneInfo = self.GetPane(pane_window)
        if not paneInfo.IsOk():
            raise Exception("Pane window not found")

        if not paneInfo.IsFloatable() or not paneInfo.IsDockable() or \
           self._agwFlags & AUI_MGR_ALLOW_FLOATING == 0:
            return

        indx = self._panes.index(paneInfo)
        win_rect = None
        
        if paneInfo.IsFloating():
            if paneInfo.name.startswith("__floating__"):
                # It's a floating tab from a AuiNotebook
                notebook = paneInfo.window.__aui_notebook__
                notebook.ReDockPage(paneInfo)
                self.Update()
                return
            else:

                e = self.FireEvent(wxEVT_AUI_PANE_DOCKING, paneInfo, canVeto=True)
                if e.GetVeto():
                    self.HideHint()
                    ShowDockingGuides(self._guides, False)
                    return
                
                win_rect = paneInfo.frame.GetRect()
                paneInfo.Dock()
                if paneInfo.IsToolbar():
                    paneInfo = self.SwitchToolBarOrientation(paneInfo)

                e = self.FireEvent(wxEVT_AUI_PANE_DOCKED, paneInfo, canVeto=False)

        else:

            e = self.FireEvent(wxEVT_AUI_PANE_FLOATING, paneInfo, canVeto=True)
            if e.GetVeto():
                return

            # float the window
            if paneInfo.IsMaximized():
                self.RestorePane(paneInfo)
            
            if paneInfo.floating_pos == wx.Point(-1, -1):
                captionSize = self._art.GetMetric(AUI_DOCKART_CAPTION_SIZE)
                paneInfo.floating_pos = pane_window.GetScreenPosition()
                paneInfo.floating_pos.y -= captionSize

            paneInfo.Float()
            e = self.FireEvent(wxEVT_AUI_PANE_FLOATED, paneInfo, canVeto=False)

        self._panes[indx] = paneInfo
        self.Update()

        if win_rect and self._agwFlags & AUI_MGR_ANIMATE_FRAMES:
            paneInfo = self.GetPane(pane_window)
            pane_rect = paneInfo.window.GetScreenRect()
            self.AnimateDocking(win_rect, pane_rect)


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{AuiManager}.

        :param `event`: an instance of `wx.PaintEvent` to be processed.
        """
        
        dc = wx.PaintDC(self._frame)
        self.Repaint(dc)
                

    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{AuiManager}.

        :param `event`: `wx.EraseEvent` to be processed.

        :note: This is intentionally empty (excluding wxMAC) to reduce
         flickering while drawing.
        """
        
        if wx.Platform == "__WXMAC__":
            event.Skip()


    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{AuiManager}.

        :param `event`: a `wx.SizeEvent` to be processed.
        """
        
        skipped = False
        if isinstance(self._frame, AuiFloatingFrame) and self._frame.IsShownOnScreen():
            skipped = True
            event.Skip()

        if self._frame:
                
            self.DoFrameLayout()
            if wx.Platform == "__WXMAC__":
                self._frame.Refresh()
            else:
                self.Repaint()
            
            if isinstance(self._frame, wx.MDIParentFrame) or isinstance(self._frame, tabmdi.AuiMDIClientWindow) \
               or isinstance(self._frame, tabmdi.AuiMDIParentFrame):
                # for MDI parent frames, this event must not
                # be "skipped".  In other words, the parent frame
                # must not be allowed to resize the client window
                # after we are finished processing sizing changes
                return

        if not skipped:
            event.Skip()

        # For the snap to screen...
        self.OnMove(None)
        

    def OnFindManager(self, event):
        """
        Handles the ``EVT_AUI_FIND_MANAGER`` event for L{AuiManager}.

        :param `event`: a L{AuiManagerEvent} event to be processed.
        """
        
        # Initialize to None
        event.SetManager(None)
        
        if not self._frame:
            return
        
        # See it this window wants to overwrite
        self._frame.ProcessEvent(event)

        # if no, it must be us
        if not event.GetManager():
           event.SetManager(self)
       

    def OnSetCursor(self, event):
        """
        Handles the ``wx.EVT_SET_CURSOR`` event for L{AuiManager}.

        :param `event`: a `wx.SetCursorEvent` to be processed.
        """
        
        # determine cursor
        part = self.HitTest(event.GetX(), event.GetY())
        cursor = wx.NullCursor

        if part:
            if part.type in [AuiDockUIPart.typeDockSizer, AuiDockUIPart.typePaneSizer]:

                if not self.CheckMovableSizer(part):
                    return
                
                if part.orientation == wx.VERTICAL:
                    cursor = wx.StockCursor(wx.CURSOR_SIZEWE)
                else:
                    cursor = wx.StockCursor(wx.CURSOR_SIZENS)
            
            elif part.type == AuiDockUIPart.typeGripper:
                cursor = wx.StockCursor(wx.CURSOR_SIZING)

        event.SetCursor(cursor)


    def UpdateButtonOnScreen(self, button_ui_part, event):
        """
        Updates/redraws the UI part containing a pane button.

        :param `button_ui_part`: the UI part the button belongs to;
        :param `event`: a `wx.MouseEvent` to be processed.
        """

        hit_test = self.HitTest(*event.GetPosition())

        if not hit_test or not button_ui_part:
            return
    
        state = AUI_BUTTON_STATE_NORMAL
        
        if hit_test == button_ui_part:
            if event.LeftDown():
                state = AUI_BUTTON_STATE_PRESSED
            else:
                state = AUI_BUTTON_STATE_HOVER
        else:
            if event.LeftDown():
                state = AUI_BUTTON_STATE_HOVER
        
        # now repaint the button with hover state
        cdc = wx.ClientDC(self._frame)

        # if the frame has a toolbar, the client area
        # origin will not be (0,0).
        pt = self._frame.GetClientAreaOrigin()
        if pt.x != 0 or pt.y != 0:
            cdc.SetDeviceOrigin(pt.x, pt.y)

        if hit_test.pane:        
            self._art.DrawPaneButton(cdc, self._frame,
                      button_ui_part.button.button_id,
                      state,
                      button_ui_part.rect, hit_test.pane)


    def OnLeftDown(self, event):
        """
        Handles the ``wx.EVT_LEFT_DOWN`` event for L{AuiManager}.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        part = self.HitTest(*event.GetPosition())

        if not part:
            event.Skip()
            return
        
        self._currentDragItem = -1
        
        if part.type in [AuiDockUIPart.typeDockSizer, AuiDockUIPart.typePaneSizer]:
        
            if not self.CheckMovableSizer(part):
                return

            self._action = actionResize
            self._action_part = part
            self._action_pane = None
            self._action_rect = wx.Rect()
            self._action_start = wx.Point(event.GetX(), event.GetY())
            self._action_offset = wx.Point(event.GetX() - part.rect.x,
                                           event.GetY() - part.rect.y)

            # draw the resize hint
            rect = wx.RectPS(self._frame.ClientToScreen(part.rect.GetPosition()),
                             part.rect.GetSize())

            self._action_rect = wx.Rect(*rect)

            if not AuiManager_HasLiveResize(self):
                if wx.Platform == "__WXMAC__":
                    dc = wx.ClientDC(self._frame)
                else:
                    dc = wx.ScreenDC()
                    
                DrawResizeHint(dc, rect)

            self._frame.CaptureMouse()
        
        elif part.type == AuiDockUIPart.typePaneButton:
            if self.IsPaneButtonVisible(part):
                self._action = actionClickButton
                self._action_part = part
                self._action_pane = None
                self._action_start = wx.Point(*event.GetPosition())
                self._frame.CaptureMouse()

                self.RefreshButton(part)
        
        elif part.type in [AuiDockUIPart.typeCaption, AuiDockUIPart.typeGripper]:

            # if we are managing a AuiFloatingFrame window, then
            # we are an embedded AuiManager inside the AuiFloatingFrame.
            # We want to initiate a toolbar drag in our owner manager
            if isinstance(part.pane.window.GetParent(), AuiFloatingFrame):
                rootManager = GetManager(part.pane.window)
            else:
                rootManager = self

            offset = wx.Point(event.GetX() - part.rect.x, event.GetY() - part.rect.y)
            rootManager.OnGripperClicked(part.pane.window, event.GetPosition(), offset)
        
        if wx.Platform != "__WXMAC__":
            event.Skip()


    def OnLeftDClick(self, event):
        """
        Handles the ``wx.EVT_LEFT_DCLICK`` event for L{AuiManager}.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        part = self.HitTest(event.GetX(), event.GetY())

        if part and part.type == AuiDockUIPart.typeCaption:
            if isinstance(part.pane.window.GetParent(), AuiFloatingFrame):
                rootManager = GetManager(part.pane.window)
            else:
                rootManager = self
                
            rootManager.OnCaptionDoubleClicked(part.pane.window)
            
        elif part and part.type in [AuiDockUIPart.typeDockSizer, AuiDockUIPart.typePaneSizer]:
            # Handles double click on AuiNotebook sashes to unsplit
            sash_size = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)
            for child in part.cont_sizer.GetChildren():
                if child.IsSizer():
                    win = child.GetSizer().GetContainingWindow()
                    if isinstance(win, auibook.AuiNotebook):
                        win.UnsplitDClick(part, sash_size, event.GetPosition())
                        break
                
        event.Skip()


    def DoEndResizeAction(self, event):
        """
        Ends a resize action, or for live update, resizes the sash.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        clientPt = event.GetPosition()
        screenPt = self._frame.ClientToScreen(clientPt)

        return self.RestrictResize(clientPt, screenPt, createDC=False)


    def RestrictResize(self, clientPt, screenPt, createDC):
        """ Common method between L{DoEndResizeAction} and L{OnLeftUp_Resize}. """

        dock = self._action_part.dock
        pane = self._action_part.pane

        if createDC:
            if wx.Platform == "__WXMAC__":
                dc = wx.ClientDC(self._frame)
            else:
                dc = wx.ScreenDC()

            DrawResizeHint(dc, self._action_rect)
            self._action_rect = wx.Rect()
        
        newPos = clientPt - self._action_offset

        if self._action_part.type == AuiDockUIPart.typeDockSizer:
            minPix, maxPix = self.CalculateDockSizerLimits(dock)
        else:
            if not self._action_part.pane:
                return
            minPix, maxPix = self.CalculatePaneSizerLimits(dock, pane)

        if self._action_part.orientation == wx.HORIZONTAL:
            newPos.y = Clip(newPos.y, minPix, maxPix)
        else:
            newPos.x = Clip(newPos.x, minPix, maxPix)

        if self._action_part.type == AuiDockUIPart.typeDockSizer:
        
            partnerDock = self.GetPartnerDock(dock)
            sash_size = self._art.GetMetric(AUI_DOCKART_SASH_SIZE)
            new_dock_size = 0
            direction = dock.dock_direction

            if direction == AUI_DOCK_LEFT:
                new_dock_size = newPos.x - dock.rect.x

            elif direction == AUI_DOCK_TOP:
                new_dock_size = newPos.y - dock.rect.y

            elif direction == AUI_DOCK_RIGHT:
                new_dock_size = dock.rect.x + dock.rect.width - newPos.x - sash_size

            elif direction == AUI_DOCK_BOTTOM:
                new_dock_size = dock.rect.y + dock.rect.height - newPos.y - sash_size

            deltaDockSize = new_dock_size - dock.size

            if partnerDock:
                if deltaDockSize > partnerDock.size - sash_size:
                    deltaDockSize = partnerDock.size - sash_size

                partnerDock.size -= deltaDockSize
            
            dock.size += deltaDockSize
            self.Update()
        
        else:
        
            # determine the new pixel size that the user wants
            # this will help us recalculate the pane's proportion
            if dock.IsHorizontal():
                oldPixsize = pane.rect.width
                newPixsize = oldPixsize + newPos.x - self._action_part.rect.x
                    
            else:            
                oldPixsize = pane.rect.height
                newPixsize = oldPixsize + newPos.y - self._action_part.rect.y
                                
            totalPixsize, totalProportion = self.GetTotalPixSizeAndProportion(dock)
            partnerPane = self.GetPartnerPane(dock, pane)

            # prevent division by zero
            if totalPixsize <= 0 or totalProportion <= 0 or not partnerPane:
                return

            # adjust for the surplus
            while (oldPixsize > 0 and totalPixsize > 10 and \
                  oldPixsize*totalProportion/totalPixsize < pane.dock_proportion):
            
                totalPixsize -= 1

            # calculate the new proportion of the pane
            
            newProportion = newPixsize*totalProportion/totalPixsize
            newProportion = Clip(newProportion, 1, totalProportion)
            deltaProp = newProportion - pane.dock_proportion

            if partnerPane.dock_proportion - deltaProp < 1:
                deltaProp = partnerPane.dock_proportion - 1
                newProportion = pane.dock_proportion + deltaProp
            
            # borrow the space from our neighbor pane to the
            # right or bottom (depending on orientation)
            partnerPane.dock_proportion -= deltaProp
            pane.dock_proportion = newProportion

            self.Update()
        
        return True
    

    def OnLeftUp(self, event):
        """
        Handles the ``wx.EVT_LEFT_UP`` event for L{AuiManager}.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        if self._action == actionResize:
##            self._frame.Freeze()
            self.OnLeftUp_Resize(event)
##            self._frame.Thaw()
        
        elif self._action == actionClickButton:
            self.OnLeftUp_ClickButton(event)
        
        elif self._action == actionDragFloatingPane:
            self.OnLeftUp_DragFloatingPane(event)
        
        elif self._action == actionDragToolbarPane:
            self.OnLeftUp_DragToolbarPane(event)
            
        else:
            event.Skip()        

        if self._frame.HasCapture():
            self._frame.ReleaseMouse()
            
        self._action = actionNone


    def OnMotion(self, event):
        """
        Handles the ``wx.EVT_MOTION`` event for L{AuiManager}.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        if self._action == actionResize:
            self.OnMotion_Resize(event)
        
        elif self._action == actionClickCaption:
            self.OnMotion_ClickCaption(event)
        
        elif self._action == actionDragFloatingPane:
            self.OnMotion_DragFloatingPane(event)
        
        elif self._action == actionDragToolbarPane:
            self.OnMotion_DragToolbarPane(event)
        
        else:
            self.OnMotion_Other(event)
                        
    
    def OnLeaveWindow(self, event):
        """
        Handles the ``wx.EVT_LEAVE_WINDOW`` event for L{AuiManager}.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        if self._hover_button:
            self.RefreshButton(self._hover_button)
            self._hover_button = None


    def OnCaptureLost(self, event):
        """
        Handles the ``wx.EVT_MOUSE_CAPTURE_LOST`` event for L{AuiManager}.

        :param `event`: a `wx.MouseCaptureLostEvent` to be processed.
        """
        
        # cancel the operation in progress, if any
        if self._action != actionNone:
            self._action = actionNone
            self.HideHint()


    def OnHintFadeTimer(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{AuiManager}.

        :param `event`: a `wx.TimerEvent` to be processed.
        """

        if not self._hint_window or self._hint_fadeamt >= self._hint_fademax:
            self._hint_fadetimer.Stop()
            return

        self._hint_fadeamt += 4
        self._hint_window.SetTransparent(self._hint_fadeamt)


    def OnMove(self, event):
        """
        Handles the ``wx.EVT_MOVE`` event for L{AuiManager}.

        :param `event`: a `wx.MoveEvent` to be processed.
        """

        if event is not None:
            event.Skip()

        if isinstance(self._frame, AuiFloatingFrame) and self._frame.IsShownOnScreen():
            return

        docked, hAlign, vAlign, monitor = self._is_docked
        if docked:
            self.Snap()

        for pane in self._panes:
            if pane.IsSnappable():
                if pane.IsFloating() and pane.IsShown():
                    self.SnapPane(pane, pane.floating_pos, pane.floating_size, True)
        

    def OnSysColourChanged(self, event):
        """
        Handles the ``wx.EVT_SYS_COLOUR_CHANGED`` event for L{AuiManager}.

        :param `event`: a `wx.SysColourChangedEvent` to be processed.
        """
        
        # This event is probably triggered by a theme change 
        # so we have to re-init the art provider.
        if self._art:
            self._art.Init()

        if self._frame:
            self.Update()
            self._frame.Refresh()
            

    def OnChildFocus(self, event):
        """
        Handles the ``wx.EVT_CHILD_FOCUS`` event for L{AuiManager}.

        :param `event`: a `wx.ChildFocusEvent` to be processed.
        """

        # when a child pane has it's focus set, we should change the 
        # pane's active state to reflect this. (this is only true if 
        # active panes are allowed by the owner)

        window = event.GetWindow()
        if isinstance(window, wx.Dialog):
            # Ignore EVT_CHILD_FOCUS events originating from dialogs not
            # managed by AUI
            rootManager = None
        elif isinstance(window.GetParent(), AuiFloatingFrame):
            rootManager = GetManager(window)
        else:
            rootManager = self
                
        if rootManager:
            rootManager.ActivatePane(window)
            
        event.Skip()


    def OnMotion_ClickCaption(self, event):
        """
        Sub-handler for the L{OnMotion} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        clientPt = event.GetPosition()
        screenPt = self._frame.ClientToScreen(clientPt)

        drag_x_threshold = wx.SystemSettings.GetMetric(wx.SYS_DRAG_X)
        drag_y_threshold = wx.SystemSettings.GetMetric(wx.SYS_DRAG_Y)

        if not self._action_pane:
            return

        # we need to check if the mouse is now being dragged
        if not (abs(clientPt.x - self._action_start.x) > drag_x_threshold or \
                abs(clientPt.y - self._action_start.y) > drag_y_threshold):
        
            return
        
        # dragged -- we need to change the mouse action to 'drag'
        if self._action_pane.IsToolbar():
            self._action = actionDragToolbarPane
            self._action_window = self._action_pane.window
        
        elif self._action_pane.IsFloatable() and self._agwFlags & AUI_MGR_ALLOW_FLOATING:

            e = self.FireEvent(wxEVT_AUI_PANE_FLOATING, self._action_pane, canVeto=True)
            if e.GetVeto():
                return
            
            self._action = actionDragFloatingPane

            # set initial float position
            self._action_pane.floating_pos = screenPt - self._action_offset

            # float the window
            if self._action_pane.IsMaximized():
                self.RestorePane(self._action_pane)
                
            self._action_pane.Hide()
            self._action_pane.Float()
            if wx.Platform == "__WXGTK__":
                self._action_pane.Show()

            e = self.FireEvent(wxEVT_AUI_PANE_FLOATED, self._action_pane, canVeto=False)

            if not self._action_pane.frame:
                self.Update()

            self._action_window = self._action_pane.window

            # adjust action offset for window frame
            windowPt = self._action_pane.frame.GetRect().GetTopLeft()
            originPt = self._action_pane.frame.ClientToScreen(wx.Point())
            self._toolbar_action_offset = originPt - windowPt
            
            if self._agwFlags & AUI_MGR_USE_NATIVE_MINIFRAMES:
                originPt = windowPt + wx.Point(3, 3)
                
            self._action_offset += originPt - windowPt

            # action offset is used here to make it feel "natural" to the user
            # to drag a docked pane and suddenly have it become a floating frame.
            # Sometimes, however, the offset where the user clicked on the docked
            # caption is bigger than the width of the floating frame itself, so
            # in that case we need to set the action offset to a sensible value
            frame_size = self._action_pane.frame.GetSize()
            if self._action_offset.x > frame_size.x * 2 / 3:
                self._action_offset.x = frame_size.x / 2
            if self._action_offset.y > frame_size.y * 2 / 3:
                self._action_offset.y = frame_size.y / 2

            self.OnMotion_DragFloatingPane(event)
            if wx.Platform != "__WXGTK__":
                self._action_pane.Show()
                
            self.Update()


    def OnMotion_Resize(self, event):
        """
        Sub-handler for the L{OnMotion} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        if AuiManager_HasLiveResize(self):
            if self._currentDragItem != -1:
                self._action_part = self._uiparts[self._currentDragItem]
            else:
                self._currentDragItem = self._uiparts.index(self._action_part)

            if self._frame.HasCapture():
                self._frame.ReleaseMouse()
                
            self.DoEndResizeAction(event)
            self._frame.CaptureMouse()
            return

        if not self._action_part or not self._action_part.dock or not self._action_part.orientation:
            return

        clientPt = event.GetPosition()
        screenPt = self._frame.ClientToScreen(clientPt)
                    
        dock = self._action_part.dock
        pos = self._action_part.rect.GetPosition()

        if self._action_part.type == AuiDockUIPart.typeDockSizer:
            minPix, maxPix = self.CalculateDockSizerLimits(dock)
        else:
            if not self._action_part.pane:
                return
            
            pane = self._action_part.pane
            minPix, maxPix = self.CalculatePaneSizerLimits(dock, pane)

        if self._action_part.orientation == wx.HORIZONTAL:
            pos.y = Clip(clientPt.y - self._action_offset.y, minPix, maxPix)
        else:
            pos.x = Clip(clientPt.x - self._action_offset.x, minPix, maxPix)

        hintrect = wx.RectPS(self._frame.ClientToScreen(pos), self._action_part.rect.GetSize())

        if hintrect != self._action_rect:
        
            if wx.Platform == "__WXMAC__":
                dc = wx.ClientDC(self._frame)
            else:
                dc = wx.ScreenDC()

            DrawResizeHint(dc, self._action_rect)
            DrawResizeHint(dc, hintrect)
            self._action_rect = wx.Rect(*hintrect)
                

    def OnLeftUp_Resize(self, event):
        """
        Sub-handler for the L{OnLeftUp} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        if self._currentDragItem != -1 and AuiManager_HasLiveResize(self):
            self._action_part = self._uiparts[self._currentDragItem]

            if self._frame.HasCapture():
                self._frame.ReleaseMouse()
                
            self.DoEndResizeAction(event)
            self._currentDragItem = -1
            return
            
        if not self._action_part or not self._action_part.dock:
            return

        clientPt = event.GetPosition()
        screenPt = self._frame.ClientToScreen(clientPt)

        return self.RestrictResize(clientPt, screenPt, createDC=True)
        

    def OnLeftUp_ClickButton(self, event):
        """
        Sub-handler for the L{OnLeftUp} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        self._hover_button = None

        if self._action_part:
            self.RefreshButton(self._action_part)

            # make sure we're still over the item that was originally clicked
            if self._action_part == self.HitTest(*event.GetPosition()):
            
                # fire button-click event
                e = AuiManagerEvent(wxEVT_AUI_PANE_BUTTON)
                e.SetManager(self)
                e.SetPane(self._action_part.pane)
                e.SetButton(self._action_part.button.button_id)
                self.ProcessMgrEvent(e)
        

    def CheckPaneMove(self, pane):
        """
        Checks if a pane has moved by a visible amount.

        :param `pane`: an instance of L{AuiPaneInfo}.
        """

        win_rect = pane.frame.GetRect()
        win_rect.x, win_rect.y = pane.floating_pos
        
        if win_rect == self._last_rect:
            return False

        # skip the first move event
        if self._last_rect.IsEmpty():
            self._last_rect = wx.Rect(*win_rect)
            return False

        # skip if moving too fast to avoid massive redraws and
        # jumping hint windows
        if abs(win_rect.x - self._last_rect.x) > 10 or \
           abs(win_rect.y - self._last_rect.y) > 10:
            self._last_rect = wx.Rect(*win_rect)
            return False

        return True        
        

    def OnMotion_DragFloatingPane(self, eventOrPt):
        """
        Sub-handler for the L{OnMotion} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        isPoint = False
        if isinstance(eventOrPt, wx.Point):
            clientPt = self._frame.ScreenToClient(eventOrPt)
            screenPt = wx.Point(*eventOrPt)
            isPoint = True
        else:
            clientPt = eventOrPt.GetPosition()
            screenPt = self._frame.ClientToScreen(clientPt)
        
        framePos = wx.Point()
        
        # try to find the pane
        pane = self.GetPane(self._action_window)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        # update floating position
        if pane.IsFloating():
            diff = pane.floating_pos - (screenPt - self._action_offset)
            pane.floating_pos = screenPt - self._action_offset

        framePos = pane.floating_pos

        # Move the pane window
        if pane.frame:

            if diff.x != 0 or diff.y != 0:
                if wx.Platform == "__WXMSW__" and (self._agwFlags & AUI_MGR_TRANSPARENT_DRAG) == 0: # and not self.CheckPaneMove(pane):
                    # return
                    # HACK: Terrible hack on wxMSW (!)
                    pane.frame.SetTransparent(254)
                            
                self._from_move = True
                pane.frame.Move(pane.floating_pos)
                self._from_move = False

            if self._agwFlags & AUI_MGR_TRANSPARENT_DRAG:
                pane.frame.SetTransparent(150)

        # calculate the offset from the upper left-hand corner
        # of the frame to the mouse pointer
        action_offset = screenPt - framePos

        # is the pane dockable?
        if not self.CanDockPanel(pane):
            self.HideHint()
            ShowDockingGuides(self._guides, False)
            return
        
        for paneInfo in self._panes:
        
            if not paneInfo.IsDocked() or not paneInfo.IsShown():
                continue
            if paneInfo.IsToolbar() or paneInfo.IsNotebookControl():
                continue
            if paneInfo.IsMaximized():
                continue

            if paneInfo.IsNotebookPage():
            
                notebookRoot = GetNotebookRoot(self._panes, paneInfo.notebook_id)

                if not notebookRoot or not notebookRoot.IsDocked():
                    continue
            
            rc = paneInfo.window.GetScreenRect()
            if rc.Contains(screenPt):
                if rc.height < 20 or rc.width < 20:
                    return
                
                self.UpdateDockingGuides(paneInfo)
                ShowDockingGuides(self._guides, True)
                break

        self.DrawHintRect(pane.window, clientPt, action_offset)


    def OnLeftUp_DragFloatingPane(self, eventOrPt):
        """
        Sub-handler for the L{OnLeftUp} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """

        if isinstance(eventOrPt, wx.Point):
            clientPt = self._frame.ScreenToClient(eventOrPt)
            screenPt = wx.Point(*eventOrPt)
        else:
            clientPt = eventOrPt.GetPosition()
            screenPt = self._frame.ClientToScreen(clientPt)

        # try to find the pane
        paneInfo = self.GetPane(self._action_window)
        if not paneInfo.IsOk():
            raise Exception("Pane window not found")

        ret = False
        
        if paneInfo.frame:
        
            # calculate the offset from the upper left-hand corner
            # of the frame to the mouse pointer
            framePos = paneInfo.frame.GetPosition()
            action_offset = screenPt - framePos

            # is the pane dockable?
            if self.CanDockPanel(paneInfo):
                # do the drop calculation
                indx = self._panes.index(paneInfo)
                ret, paneInfo = self.DoDrop(self._docks, self._panes, paneInfo, clientPt, action_offset)

                if ret:
                    e = self.FireEvent(wxEVT_AUI_PANE_DOCKING, paneInfo, canVeto=True)
                    if e.GetVeto():
                        self.HideHint()
                        ShowDockingGuides(self._guides, False)
                        return

                    e = self.FireEvent(wxEVT_AUI_PANE_DOCKED, paneInfo, canVeto=False)

                    if self._agwFlags & AUI_MGR_SMOOTH_DOCKING:
                        self.SmoothDock(paneInfo)

                self._panes[indx] = paneInfo
            
        # if the pane is still floating, update it's floating
        # position (that we store)
        if paneInfo.IsFloating():
            paneInfo.floating_pos = paneInfo.frame.GetPosition()
            if paneInfo.frame._transparent != paneInfo.transparent or self._agwFlags & AUI_MGR_TRANSPARENT_DRAG:
                paneInfo.frame.SetTransparent(paneInfo.transparent)
                paneInfo.frame._transparent = paneInfo.transparent
        
        elif self._has_maximized:
            self.RestoreMaximizedPane()
        
        # reorder for dropping to a new notebook
        # (caution: this code breaks the reference!)
        tempPaneInfo = self.CopyTarget(paneInfo)
        self._panes.remove(paneInfo)
        self._panes.append(tempPaneInfo)

        if ret:
            self.Update()

        self.HideHint()
        ShowDockingGuides(self._guides, False)


    def OnMotion_DragToolbarPane(self, eventOrPt):
        """
        Sub-handler for the L{OnMotion} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        isPoint = False
        if isinstance(eventOrPt, wx.Point):
            clientPt = self._frame.ScreenToClient(eventOrPt)
            screenPt = wx.Point(*eventOrPt)
            isPoint = True
        else:
            clientPt = eventOrPt.GetPosition()
            screenPt = self._frame.ClientToScreen(clientPt)

        pane = self.GetPane(self._action_window)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        pane.state |= AuiPaneInfo.actionPane
        indx = self._panes.index(pane)

        ret = False
        wasFloating = pane.IsFloating()
        # is the pane dockable?
        if self.CanDockPanel(pane):
            # do the drop calculation
            ret, pane = self.DoDrop(self._docks, self._panes, pane, clientPt, self._action_offset)
        
        # update floating position
        if pane.IsFloating():
            pane.floating_pos = screenPt - self._toolbar_action_offset

        # move the pane window
        if pane.frame:
            if wx.Platform == "__WXMSW__" and (self._agwFlags & AUI_MGR_TRANSPARENT_DRAG) == 0: # and not self.CheckPaneMove(pane):
                # return
                # HACK: Terrible hack on wxMSW (!)
                pane.frame.SetTransparent(254)

            self._from_move = True
            pane.frame.Move(pane.floating_pos)
            self._from_move = False
                
            if self._agwFlags & AUI_MGR_TRANSPARENT_DRAG:
                pane.frame.SetTransparent(150)

        self._panes[indx] = pane
        if ret and wasFloating != pane.IsFloating() or (ret and not wasFloating):
            wx.CallAfter(self.Update)

        # when release the button out of the window.
        # TODO: a better fix is needed.

        if _VERSION_STRING < "2.9":
            leftDown = wx.GetMouseState().LeftDown()
        else:
            leftDown = wx.GetMouseState().LeftIsDown()
        
        if not leftDown:
            self._action = actionNone
            self.OnLeftUp_DragToolbarPane(eventOrPt)


    def OnMotion_Other(self, event):
        """
        Sub-handler for the L{OnMotion} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        part = self.HitTest(*event.GetPosition())

        if part and part.type == AuiDockUIPart.typePaneButton \
           and self.IsPaneButtonVisible(part):
            if part != self._hover_button:
            
                if self._hover_button:
                    self.RefreshButton(self._hover_button)

                self._hover_button = part
                self.RefreshButton(part)
            
        else:
        
            if self._hover_button:
                self.RefreshButton(self._hover_button)
            else:
                event.Skip()

            self._hover_button = None
        
        
    def OnLeftUp_DragToolbarPane(self, eventOrPt):
        """
        Sub-handler for the L{OnLeftUp} event.

        :param `event`: a `wx.MouseEvent` to be processed.
        """
        
        isPoint = False
        if isinstance(eventOrPt, wx.Point):
            clientPt = self._frame.ScreenToClient(eventOrPt)
            screenPt = wx.Point(*eventOrPt)
            isPoint = True
        else:
            clientPt = eventOrPt.GetPosition()
            screenPt = self._frame.ClientToScreen(clientPt)

        # try to find the pane
        pane = self.GetPane(self._action_window)
        if not pane.IsOk():
            raise Exception("Pane window not found")

        if pane.IsFloating():
            pane.floating_pos = pane.frame.GetPosition()
            if pane.frame._transparent != pane.transparent or self._agwFlags & AUI_MGR_TRANSPARENT_DRAG:
                pane.frame.SetTransparent(pane.transparent)
                pane.frame._transparent = pane.transparent
        
        # save the new positions
        docks = FindDocks(self._docks, pane.dock_direction, pane.dock_layer, pane.dock_row)
        if len(docks) == 1:
            dock = docks[0]
            pane_positions, pane_sizes = self.GetPanePositionsAndSizes(dock)

            for i in xrange(len(dock.panes)):
                dock.panes[i].dock_pos = pane_positions[i]
        
        pane.state &= ~AuiPaneInfo.actionPane
        self.Update()


    def OnPaneButton(self, event):
        """
        Handles the ``EVT_AUI_PANE_BUTTON`` event for L{AuiManager}.

        :param `event`: a L{AuiManagerEvent} event to be processed.
        """

        if not event.pane:
            raise Exception("Pane Info passed to AuiManager.OnPaneButton must be non-null")

        pane = event.pane

        if event.button == AUI_BUTTON_CLOSE:

            if isinstance(pane.window.GetParent(), AuiFloatingFrame):
                rootManager = GetManager(pane.window)
            else:
                rootManager = self
            
            if rootManager != self:
                self._frame.Close()
                return

            # fire pane close event
            e = AuiManagerEvent(wxEVT_AUI_PANE_CLOSE)
            e.SetManager(self)
            e.SetPane(event.pane)
            self.ProcessMgrEvent(e)

            if not e.GetVeto():
            
                # close the pane, but check that it
                # still exists in our pane array first
                # (the event handler above might have removed it)

                check = self.GetPane(pane.window)
                if check.IsOk():                
                    self.ClosePane(pane)
                
                self.Update()

        # mn this performs the minimizing of a pane
        elif event.button == AUI_BUTTON_MINIMIZE:
            e = AuiManagerEvent(wxEVT_AUI_PANE_MINIMIZE)
            e.SetManager(self)
            e.SetPane(event.pane)
            self.ProcessMgrEvent(e)

            if not e.GetVeto():
                self.MinimizePane(pane)
    
        elif event.button == AUI_BUTTON_MAXIMIZE_RESTORE and not pane.IsMaximized():
        
            # fire pane close event
            e = AuiManagerEvent(wxEVT_AUI_PANE_MAXIMIZE)
            e.SetManager(self)
            e.SetPane(event.pane)
            self.ProcessMgrEvent(e)

            if not e.GetVeto():
            
                self.MaximizePane(pane)
                self.Update()
            
        elif event.button == AUI_BUTTON_MAXIMIZE_RESTORE and pane.IsMaximized():
        
            # fire pane close event
            e = AuiManagerEvent(wxEVT_AUI_PANE_RESTORE)
            e.SetManager(self)
            e.SetPane(event.pane)
            self.ProcessMgrEvent(e)

            if not e.GetVeto():
            
                self.RestorePane(pane)
                self.Update()
            
        elif event.button == AUI_BUTTON_PIN:
        
            if self._agwFlags & AUI_MGR_ALLOW_FLOATING and pane.IsFloatable():
                e = self.FireEvent(wxEVT_AUI_PANE_FLOATING, pane, canVeto=True)
                if e.GetVeto():
                    return

                pane.Float()
                e = self.FireEvent(wxEVT_AUI_PANE_FLOATED, pane, canVeto=False)

            self.Update()


    def MinimizePane(self, paneInfo):
        """
        Minimizes a pane in a newly and automatically created L{AuiToolBar}.

        Clicking on the minimize button causes a new L{AuiToolBar} to be created
        and added to the frame manager (currently the implementation is such that
        panes at West will have a toolbar at the right, panes at South will have
        toolbars at the bottom etc...) and the pane is hidden in the manager.
        
        Clicking on the restore button on the newly created toolbar will result in the
        toolbar being removed and the original pane being restored.

        :param `paneInfo`: a L{AuiPaneInfo} instance for the pane to be minimized.
        """
        
        if not paneInfo.IsToolbar():

            if paneInfo.IsMinimized():
                # We are already minimized
                return
            
            # Basically the idea is this.
            #
            # 1) create a toolbar, with a restore button 
            #
            # 2) place the new toolbar in the toolbar area representative of the location of the pane 
            #  (NORTH/SOUTH/EAST/WEST, central area always to the right)
            #
            # 3) Hide the minimizing pane 


            # personalize the toolbar style
            tbStyle = AUI_TB_DEFAULT_STYLE
            posMask = paneInfo.minimize_mode & AUI_MINIMIZE_POS_MASK
            captMask = paneInfo.minimize_mode & AUI_MINIMIZE_CAPT_MASK
            dockDirection = paneInfo.dock_direction
            if captMask != 0:
                tbStyle |= AUI_TB_TEXT
            if posMask == AUI_MINIMIZE_POS_SMART:
                if paneInfo.dock_direction in [AUI_DOCK_TOP, AUI_DOCK_BOTTOM]:
                    tbStyle |= AUI_TB_HORZ_LAYOUT

                elif paneInfo.dock_direction in [AUI_DOCK_LEFT, AUI_DOCK_RIGHT, AUI_DOCK_CENTER]:
                    tbStyle |= AUI_TB_VERTICAL
                    if captMask == AUI_MINIMIZE_CAPT_SMART:
                        tbStyle |= AUI_TB_CLOCKWISE
                    
            elif posMask in [AUI_MINIMIZE_POS_TOP, AUI_MINIMIZE_POS_BOTTOM]:
                tbStyle |= AUI_TB_HORZ_LAYOUT
                if posMask == AUI_MINIMIZE_POS_TOP:
                    dockDirection = AUI_DOCK_TOP
                else:
                    dockDirection = AUI_DOCK_BOTTOM

            else:
                tbStyle |= AUI_TB_VERTICAL
                if captMask == AUI_MINIMIZE_CAPT_SMART:
                    tbStyle |= AUI_TB_CLOCKWISE
                if posMask == AUI_MINIMIZE_POS_LEFT:
                    dockDirection = AUI_DOCK_LEFT
                elif posMask == AUI_MINIMIZE_POS_RIGHT:
                    dockDirection = AUI_DOCK_RIGHT
                elif posMask == AUI_MINIMIZE_POS_BOTTOM:
                    dockDirection = AUI_DOCK_BOTTOM

            # Create a new toolbar
            # give it the same name as the minimized pane with _min appended

            win_rect = paneInfo.window.GetScreenRect()
            
            minimize_toolbar = auibar.AuiToolBar(self.GetManagedWindow(), agwStyle=tbStyle)
            minimize_toolbar.Hide()
            minimize_toolbar.SetToolBitmapSize(wx.Size(16, 16))

            if paneInfo.icon and paneInfo.icon.IsOk():
                restore_bitmap = paneInfo.icon
            else:
                restore_bitmap = self._art._restore_bitmap
                
            minimize_toolbar.AddSimpleTool(ID_RESTORE_FRAME, paneInfo.caption, restore_bitmap, "Restore " + paneInfo.caption)
            minimize_toolbar.SetAuiManager(self)
            minimize_toolbar.Realize()
            toolpanelname = paneInfo.name + "_min"

            if paneInfo.IsMaximized():
                paneInfo.SetFlag(paneInfo.wasMaximized, True)

            if dockDirection == AUI_DOCK_TOP:
                self.AddPane(minimize_toolbar, AuiPaneInfo(). \
                    Name(toolpanelname).Caption(paneInfo.caption). \
                    ToolbarPane().Top().BottomDockable(False). \
                    LeftDockable(False).RightDockable(False).DestroyOnClose())
                
            elif dockDirection == AUI_DOCK_BOTTOM:
                self.AddPane(minimize_toolbar, AuiPaneInfo(). \
                    Name(toolpanelname).Caption(paneInfo.caption). \
                    ToolbarPane().Bottom().TopDockable(False). \
                    LeftDockable(False).RightDockable(False).DestroyOnClose())
                
            elif dockDirection == AUI_DOCK_LEFT:
                self.AddPane(minimize_toolbar, AuiPaneInfo(). \
                    Name(toolpanelname).Caption(paneInfo.caption). \
                    ToolbarPane().Left().TopDockable(False). \
                    BottomDockable(False).RightDockable(False).DestroyOnClose())

            elif dockDirection in [AUI_DOCK_RIGHT, AUI_DOCK_CENTER]:
                self.AddPane(minimize_toolbar, AuiPaneInfo(). \
                    Name(toolpanelname).Caption(paneInfo.caption). \
                    ToolbarPane().Right().TopDockable(False). \
                    LeftDockable(False).BottomDockable(False).DestroyOnClose())

            arr = FindDocks(self._docks, paneInfo.dock_direction, paneInfo.dock_layer, paneInfo.dock_row)

            if arr:
                dock = arr[0]
                paneInfo.previousDockSize = dock.size

            paneInfo.previousDockPos = paneInfo.dock_pos
            
            # mark ourselves minimized
            paneInfo.Minimize()
            paneInfo.Show(False)
            self._has_minimized = True
            # last, hide the window
            if paneInfo.window and paneInfo.window.IsShown():
                paneInfo.window.Show(False)

            minimize_toolbar.Show()
            self.Update()
            if self._agwFlags & AUI_MGR_ANIMATE_FRAMES:
                self.AnimateDocking(win_rect, minimize_toolbar.GetScreenRect())


    def OnRestoreMinimizedPane(self, event):
        """
        Handles the ``EVT_AUI_PANE_MIN_RESTORE`` event for L{AuiManager}.

        :param `event`: an instance of L{AuiManagerEvent} to be processed.
        """

        self.RestoreMinimizedPane(event.pane)


    def OnPaneDocked(self, event):
        """
        Handles the ``EVT_AUI_PANE_DOCKED`` event for L{AuiManager}.

        :param `event`: an instance of L{AuiManagerEvent} to be processed.
        """

        event.Skip()
        self.RemoveAutoNBCaption(event.GetPane())        
    

    def CreateNotebookBase(self, panes, paneInfo):
        """
        Creates an auto-notebook base from a pane, and then add that pane as a page.

        :param `panes`: Set of panes to append new notebook base pane to
        :param `paneInfo`: L{AuiPaneInfo} instance to convert to new notebook.
        """

        # Create base notebook pane ...
        nbid = len(self._notebooks)

        baseInfo = AuiPaneInfo()
        baseInfo.SetDockPos(paneInfo).NotebookControl(nbid). \
            CloseButton(False).SetNameFromNotebookId(). \
            NotebookDockable(False).Floatable(paneInfo.IsFloatable())
        baseInfo.best_size = paneInfo.best_size
        panes.append(baseInfo)

        # add original pane as tab ...
        paneInfo.NotebookPage(nbid)

    def RemoveAutoNBCaption(self, pane):
        """
        Removes the caption on newly created automatic notebooks.

        :param `pane`: an instance of L{AuiPaneInfo} (the target notebook).
        """

        if self._agwFlags & AUI_MGR_AUTONB_NO_CAPTION == 0:
            return False

        def RemoveCaption():
            """ Sub-function used to remove the pane caption on automatic notebooks. """
            
            if pane.HasNotebook(): 
                notebook = self._notebooks[pane.notebook_id] 
                self.GetPane(notebook).CaptionVisible(False).PaneBorder(False)                
                self.Update() 

        # it seems the notebook isnt created by this stage, so remove 
        # the caption a moment later 
        wx.CallAfter(RemoveCaption)
        return True
        
        
    def RestoreMinimizedPane(self, paneInfo):
        """
        Restores a previously minimized pane.

        :param `paneInfo`: a L{AuiPaneInfo} instance for the pane to be restored.
        """

        panename = paneInfo.name
        panename = panename[0:-4]
        pane = self.GetPane(panename)

        pane.SetFlag(pane.needsRestore, True)

        if not pane.IsOk():
            panename = paneInfo.name
            pane = self.GetPane(panename)
            paneInfo = self.GetPane(panename + "_min")
            if not paneInfo.IsOk():
                # Already minimized
                return
        
        if pane.IsOk():
            if not pane.IsMinimized():
                return
            

            if pane.HasFlag(pane.wasMaximized):

                self.SavePreviousDockSizes(pane)
                

            self.ShowPane(pane.window, True)
            pane.Show(True)
            self._has_minimized = False
            pane.SetFlag(pane.optionMinimized, False)
            paneInfo.window.Show(False)
            self.DetachPane(paneInfo.window)
            paneInfo.Show(False)
            paneInfo.Hide()

            self.Update()


    def AnimateDocking(self, win_rect, pane_rect):
        """
        Animates the minimization/docking of a pane a la Eclipse, using a `wx.ScreenDC`
        to draw a "moving docking rectangle" on the screen.

        :param `win_rect`: the original pane screen rectangle;
        :param `pane_rect`: the newly created toolbar/pane screen rectangle.

        :note: This functionality is not available on wxMAC as this platform doesn't have
         the ability to use `wx.ScreenDC` to draw on-screen and on Windows > Vista.
        """

        if wx.Platform == "__WXMAC__":
            # No wx.ScreenDC on the Mac...
            return
        if wx.Platform == "__WXMSW__" and wx.GetOsVersion()[1] > 5:
            # No easy way to handle this on Vista...
            return

        xstart, ystart = win_rect.x, win_rect.y
        xend, yend = pane_rect.x, pane_rect.y

        step = self.GetAnimationStep()
        
        wstep = int(abs(win_rect.width - pane_rect.width)/step)
        hstep = int(abs(win_rect.height - pane_rect.height)/step)
        xstep = int(win_rect.x - pane_rect.x)/step
        ystep = int(win_rect.y - pane_rect.y)/step
        
        dc = wx.ScreenDC()
        dc.SetLogicalFunction(wx.INVERT)
        dc.SetBrush(wx.TRANSPARENT_BRUSH)
        dc.SetPen(wx.LIGHT_GREY_PEN)
        
        for i in xrange(int(step)):
            width, height = win_rect.width - i*wstep, win_rect.height - i*hstep
            x, y = xstart - i*xstep, ystart - i*ystep
            new_rect = wx.Rect(x, y, width, height)
            dc.DrawRoundedRectangleRect(new_rect, 3)
            wx.SafeYield()
            wx.MilliSleep(10)
            dc.DrawRoundedRectangleRect(new_rect, 3)
            

    def SmoothDock(self, paneInfo):
        """
        This method implements a smooth docking effect for floating panes, similar to
        what the PyQT library does with its floating windows.

        :param `paneInfo`: an instance of L{AuiPaneInfo}.

        :note: The smooth docking effect can only be used if you set the ``AUI_MGR_SMOOTH_DOCKING``
         style to L{AuiManager}.
        """

        if paneInfo.IsToolbar():
            return

        if not paneInfo.frame or self._hint_rect.IsEmpty():
            return

        hint_rect = self._hint_rect
        win_rect = paneInfo.frame.GetScreenRect()

        xstart, ystart = win_rect.x, win_rect.y
        xend, yend = hint_rect.x, hint_rect.y

        step = self.GetAnimationStep()/3

        wstep = int((win_rect.width - hint_rect.width)/step)
        hstep = int((win_rect.height - hint_rect.height)/step)
        xstep = int((win_rect.x - hint_rect.x))/step
        ystep = int((win_rect.y - hint_rect.y))/step

        for i in xrange(int(step)):
            width, height = win_rect.width - i*wstep, win_rect.height - i*hstep
            x, y = xstart - i*xstep, ystart - i*ystep
            new_rect = wx.Rect(x, y, width, height)
            paneInfo.frame.SetRect(new_rect)
            wx.MilliSleep(10)            
        
            
    def SetSnapLimits(self, x, y):
        """
        Modifies the snap limits used when snapping the `managed_window` to the screen
        (using L{SnapToScreen}) or when snapping the floating panes to one side of the
        `managed_window` (using L{SnapPane}).

        To change the limit after which the `managed_window` or the floating panes are
        automatically stickled to the screen border (or to the `managed_window` side),
        set these two variables. Default values are 15 pixels.
    
        :param `x`: the minimum horizontal distance below which the snap occurs;
        :param `y`: the minimum vertical distance below which the snap occurs.
        """

        self._snap_limits = (x, y)
        self.Snap()


    def Snap(self):
        """
        Snaps the main frame to specified position on the screen.

        :see: L{SnapToScreen}
        """
        
        snap, hAlign, vAlign, monitor = self._is_docked
        if not snap:
            return

        managed_window = self.GetManagedWindow()
        snap_pos = self.GetSnapPosition()
        wnd_pos = managed_window.GetPosition()
        snapX, snapY = self._snap_limits
        
        if abs(snap_pos.x - wnd_pos.x) < snapX and abs(snap_pos.y - wnd_pos.y) < snapY:
            managed_window.SetPosition(snap_pos)
        

    def SnapToScreen(self, snap=True, monitor=0, hAlign=wx.RIGHT, vAlign=wx.TOP):
        """
        Snaps the main frame to specified position on the screen.

        :param `snap`: whether to snap the main frame or not;
        :param `monitor`: the monitor display in which snapping the window;
        :param `hAlign`: the horizontal alignment of the snapping position;
        :param `vAlign`: the vertical alignment of the snapping position.
        """
        
        if not snap:
            self._is_docked = (False, wx.RIGHT, wx.TOP, 0)
            return

        displayCount = wx.Display.GetCount()
        if monitor > displayCount:
            raise Exception("Invalid monitor selected: you only have %d monitors"%displayCount)

        self._is_docked = (True, hAlign, vAlign, monitor)
        self.GetManagedWindow().SetPosition(self.GetSnapPosition())
        

    def GetSnapPosition(self):
        """ Returns the main frame snapping position. """

        snap, hAlign, vAlign, monitor = self._is_docked
        
        display = wx.Display(monitor)
        area = display.GetClientArea()
        size = self.GetManagedWindow().GetSize()
        
        pos = wx.Point()
        if hAlign == wx.LEFT:
            pos.x = area.x
        elif hAlign == wx.CENTER:
            pos.x = area.x + (area.width - size.x)/2
        else:
            pos.x = area.x + area.width - size.x

        if vAlign == wx.TOP:
            pos.y = area.y
        elif vAlign == wx.CENTER:
            pos.y = area.y + (area.height - size.y)/2
        else:
            pos.y = area.y + area.height - size.y

        return pos            


    def GetAnimationStep(self):
        """ Returns the animation step speed (a float) to use in L{AnimateDocking}. """

        return self._animation_step


    def SetAnimationStep(self, step):
        """
        Sets the animation step speed (a float) to use in L{AnimateDocking}.

        :param `step`: a floating point value for the animation speed.
        """

        self._animation_step = float(step)        

        
    def RequestUserAttention(self, pane_window):
        """
        Requests the user attention by intermittently highlighting the pane caption.

        :param `pane_window`: a `wx.Window` derived window, managed by the pane.
        """
                
        # try to find the pane
        paneInfo = self.GetPane(pane_window)
        if not paneInfo.IsOk():
            raise Exception("Pane window not found")

        dc = wx.ClientDC(self._frame)

        # if the frame is about to be deleted, don't bother
        if not self._frame or self._frame.IsBeingDeleted():
            return
        
        if not self._frame.GetSizer():
            return

        for part in self._uiparts:
            if part.pane == paneInfo:
                self._art.RequestUserAttention(dc, self._frame, part.pane.caption, part.rect, part.pane)
                self._frame.RefreshRect(part.rect, True)
                break


    def StartPreviewTimer(self, toolbar):
        """
        Starts a timer for sliding in and out a minimized pane.

        :param `toolbar`: the L{AuiToolBar} containing the minimized pane tool.
        """

        toolbar_pane = self.GetPane(toolbar)
        toolbar_name = toolbar_pane.name
        
        pane_name = toolbar_name[0:-4]
        
        self._sliding_pane = self.GetPane(pane_name)
        self._sliding_rect = toolbar.GetScreenRect()
        self._sliding_direction = toolbar_pane.dock_direction
        self._sliding_frame = None
        
        self._preview_timer.Start(1000, wx.TIMER_ONE_SHOT)


    def StopPreviewTimer(self):
        """ Stops a timer for sliding in and out a minimized pane. """

        if self._preview_timer.IsRunning():
            self._preview_timer.Stop()

        self.SlideOut()
        self._sliding_pane = None


    def SlideIn(self, event):
        """
        Handles the ``wx.EVT_TIMER`` event for L{AuiManager}.

        :param `event`: a `wx.TimerEvent` to be processed.

        :note: This is used solely for sliding in and out minimized panes.
        """

        window = self._sliding_pane.window
        self._sliding_frame = wx.MiniFrame(None, -1, title=_("Pane Preview"),
                                           style=wx.FRAME_TOOL_WINDOW | wx.STAY_ON_TOP |
                                           wx.FRAME_NO_TASKBAR | wx.CAPTION)
        window.Reparent(self._sliding_frame)
        self._sliding_frame.SetSize((0, 0))
        window.Show()
        self._sliding_frame.Show()
        
        size = window.GetBestSize()

        startX, startY, stopX, stopY = GetSlidingPoints(self._sliding_rect, size, self._sliding_direction)
        
        step = stopX/10
        window_size = 0
        
        for i in xrange(0, stopX, step):
            window_size = i
            self._sliding_frame.SetDimensions(startX, startY, window_size, stopY)
            self._sliding_frame.Refresh()
            self._sliding_frame.Update()
            wx.MilliSleep(10)

        self._sliding_frame.SetDimensions(startX, startY, stopX, stopY)
        self._sliding_frame.Refresh()
        self._sliding_frame.Update()
        

    def SlideOut(self):
        """
        Slides out a preview of a minimized pane.

        :note: This is used solely for sliding in and out minimized panes.
        """

        if not self._sliding_frame:
            return

        window = self._sliding_frame.GetChildren()[0]
        size = window.GetBestSize()
        
        startX, startY, stopX, stopY = GetSlidingPoints(self._sliding_rect, size, self._sliding_direction)

        step = stopX/10
        window_size = 0
        
        for i in xrange(stopX, 0, -step):
            window_size = i
            self._sliding_frame.SetDimensions(startX, startY, window_size, stopY)
            self._sliding_frame.Refresh()
            self._sliding_frame.Update()
            self._frame.RefreshRect(wx.Rect(startX+window_size, startY, step, stopY))
            self._frame.Update()
            wx.MilliSleep(10)

        self._sliding_frame.SetDimensions(startX, startY, 0, stopY)

        window.Hide()
        window.Reparent(self._frame)

        self._sliding_frame.Hide()
        self._sliding_frame.Destroy()
        self._sliding_frame = None
        self._sliding_pane = None
        

class AuiManager_DCP(AuiManager):
    """
    A class similar to L{AuiManager} but with a Dummy Center Pane (**DCP**).
    The code for this class is still flickery due to the call to `wx.CallAfter`
    and the double-update call.
    """
    
    def __init__(self, *args, **keys):

        AuiManager.__init__(self, *args, **keys)
        self.hasDummyPane = False
        

    def _createDummyPane(self):
        """ Creates a Dummy Center Pane (**DCP**). """

        if self.hasDummyPane:
            return

        self.hasDummyPane = True
        dummy = wx.Panel(self.GetManagedWindow())
        info = AuiPaneInfo().CenterPane().NotebookDockable(True).Name('dummyCenterPane').DestroyOnClose(True)
        self.AddPane(dummy, info)


    def _destroyDummyPane(self):
        """ Destroys the Dummy Center Pane (**DCP**). """

        if not self.hasDummyPane:
            return
        
        self.hasDummyPane = False
        self.ClosePane(self.GetPane('dummyCenterPane'))

        
    def Update(self):
        """
        This method is called after any number of changes are made to any of the
        managed panes. L{Update} must be invoked after L{AuiManager.AddPane} or L{AuiManager.InsertPane} are
        called in order to "realize" or "commit" the changes.

        In addition, any number of changes may be made to L{AuiPaneInfo} structures
        (retrieved with L{AuiManager.GetPane}), but to realize the changes, L{Update}
        must be called. This construction allows pane flicker to be avoided by updating
        the whole layout at one time.
        """
        
        AuiManager.Update(self)

        # check if there's already a center pane (except our dummy pane)
        dummyCenterPane = self.GetPane('dummyCenterPane')
        haveCenterPane = any((pane != dummyCenterPane) and (pane.dock_direction == AUI_DOCK_CENTER) and
                             not pane.IsFloating() and pane.IsShown() for pane in self.GetAllPanes())
        if haveCenterPane:
            if self.hasDummyPane:
                # there's our dummy pane and also another center pane, therefor let's remove our dummy
                def do():
                    self._destroyDummyPane()
                    self.Update()
                wx.CallAfter(do)
        else:
            # if we get here, there's no center pane, create our dummy
            if not self.hasDummyPane:
                self._createDummyPane()

                
