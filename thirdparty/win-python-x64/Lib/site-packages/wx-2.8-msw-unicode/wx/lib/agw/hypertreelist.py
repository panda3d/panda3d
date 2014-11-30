# --------------------------------------------------------------------------------- #
# HYPERTREELIST wxPython IMPLEMENTATION
# Inspired By And Heavily Based On wx.gizmos.TreeListCtrl.
#
# Andrea Gavana, @ 08 May 2006
# Latest Revision: 21 Jun 2011, 22.00 GMT
#
#
# TODO List
#
# Almost All The Features Of wx.gizmos.TreeListCtrl Are Available, And There Is
# Practically No Limit In What Could Be Added To This Class. The First Things
# That Comes To My Mind Are:
#
# 1. Add Support For 3-State CheckBoxes (Is That Really Useful?).
#
# 2. Try To Implement A More Flicker-Free Background Image In Cases Like
#    Centered Or Stretched Image (Now HyperTreeList Supports Only Tiled
#    Background Images).
#
# 3. Try To Mimic Windows wx.TreeCtrl Expanding/Collapsing behaviour: HyperTreeList
#    Suddenly Expands/Collapses The Nodes On Mouse Click While The Native Control
#    Has Some Kind Of "Smooth" Expanding/Collapsing, Like A Wave. I Don't Even
#    Know Where To Start To Do That.
#
# 4. Speed Up General OnPaint Things? I Have No Idea, Here HyperTreeList Is Quite
#    Fast, But We Should See On Slower Machines.
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
# --------------------------------------------------------------------------------- #


"""
HyperTreeList is a class that mimics the behaviour of `wx.gizmos.TreeListCtrl`, with
some more functionalities.


Description
===========

HyperTreeList is a class that mimics the behaviour of `wx.gizmos.TreeListCtrl`, with
almost the same base functionalities plus some more enhancements. This class does
not rely on the native control, as it is a full owner-drawn tree-list control.

HyperTreeList is somewhat an hybrid between L{CustomTreeCtrl} and `wx.gizmos.TreeListCtrl`.

In addition to the standard `wx.gizmos.TreeListCtrl` behaviour this class supports:

* CheckBox-type items: checkboxes are easy to handle, just selected or unselected
  state with no particular issues in handling the item's children;
* Added support for 3-state value checkbox items;
* RadioButton-type items: since I elected to put radiobuttons in CustomTreeCtrl, I
  needed some way to handle them, that made sense. So, I used the following approach:
  
  - All peer-nodes that are radiobuttons will be mutually exclusive. In other words,
    only one of a set of radiobuttons that share a common parent can be checked at
    once. If a radiobutton node becomes checked, then all of its peer radiobuttons
    must be unchecked.
  - If a radiobutton node becomes unchecked, then all of its child nodes will become
    inactive.

* Hyperlink-type items: they look like an hyperlink, with the proper mouse cursor on
  hovering;
* Multiline text items;
* Enabling/disabling items (together with their plain or grayed out icons);
* Whatever non-toplevel widget can be attached next to a tree item;
* Whatever non-toplevel widget can be attached next to a list item;
* Column headers are fully customizable in terms of icons, colour, font, alignment etc...;
* Default selection style, gradient (horizontal/vertical) selection style and Windows
  Vista selection style;
* Customized drag and drop images built on the fly;
* Setting the HyperTreeList item buttons to a personalized imagelist;
* Setting the HyperTreeList check/radio item icons to a personalized imagelist;
* Changing the style of the lines that connect the items (in terms of `wx.Pen` styles);
* Using an image as a HyperTreeList background (currently only in "tile" mode);

And a lot more. Check the demo for an almost complete review of the functionalities.


Base Functionalities
====================

HyperTreeList supports all the `wx.gizmos.TreeListCtrl` styles, except:

- ``TR_EXTENDED``: supports for this style is on the todo list (Am I sure of this?).

Plus it has 3 more styles to handle checkbox-type items:

- ``TR_AUTO_CHECK_CHILD``: automatically checks/unchecks the item children;
- ``TR_AUTO_CHECK_PARENT``: automatically checks/unchecks the item parent;
- ``TR_AUTO_TOGGLE_CHILD``: automatically toggles the item children.

And a style useful to hide the TreeListCtrl header:

- ``TR_NO_HEADER``: hides the HyperTreeList header.


All the methods available in `wx.gizmos.TreeListCtrl` are also available in HyperTreeList.


Events
======

All the events supported by `wx.gizmos.TreeListCtrl` are also available in HyperTreeList,
with a few exceptions:

- ``EVT_TREE_GET_INFO`` (don't know what this means);
- ``EVT_TREE_SET_INFO`` (don't know what this means);
- ``EVT_TREE_ITEM_MIDDLE_CLICK`` (not implemented, but easy to add);
- ``EVT_TREE_STATE_IMAGE_CLICK`` (no need for that, look at the checking events below).

Plus, HyperTreeList supports the events related to the checkbutton-type items:

- ``EVT_TREE_ITEM_CHECKING``: an item is being checked;
- ``EVT_TREE_ITEM_CHECKED``: an item has been checked.

And to hyperlink-type items:

- ``EVT_TREE_ITEM_HYPERLINK``: an hyperlink item has been clicked (this event is sent
  after the ``EVT_TREE_SEL_CHANGED`` event).


Supported Platforms
===================

HyperTreeList has been tested on the following platforms:
  * Windows (Windows XP);


Window Styles
=============

This class supports the following window styles:

============================== =========== ==================================================
Window Styles                  Hex Value   Description
============================== =========== ==================================================
``TR_NO_BUTTONS``                      0x0 For convenience to document that no buttons are to be drawn.
``TR_SINGLE``                          0x0 For convenience to document that only one item may be selected at a time. Selecting another item causes the current selection, if any, to be deselected. This is the default.
``TR_HAS_BUTTONS``                     0x1 Use this style to show + and - buttons to the left of parent items.
``TR_NO_LINES``                        0x4 Use this style to hide vertical level connectors.
``TR_LINES_AT_ROOT``                   0x8 Use this style to show lines between root nodes. Only applicable if ``TR_HIDE_ROOT`` is set and ``TR_NO_LINES`` is not set.
``TR_DEFAULT_STYLE``                   0x9 No Docs
``TR_TWIST_BUTTONS``                  0x10 Use old Mac-twist style buttons.
``TR_MULTIPLE``                       0x20 Use this style to allow a range of items to be selected. If a second range is selected, the current range, if any, is deselected.
``TR_EXTENDED``                       0x40 Use this style to allow disjoint items to be selected. (Only partially implemented; may not work in all cases).
``TR_HAS_VARIABLE_ROW_HEIGHT``        0x80 Use this style to cause row heights to be just big enough to fit the content. If not set, all rows use the largest row height. The default is that this flag is unset.
``TR_EDIT_LABELS``                   0x200 Use this style if you wish the user to be able to edit labels in the tree control.
``TR_ROW_LINES``                     0x400 Use this style to draw a contrasting border between displayed rows.
``TR_HIDE_ROOT``                     0x800 Use this style to suppress the display of the root node, effectively causing the first-level nodes to appear as a series of root nodes.
``TR_COLUMN_LINES``                 0x1000 No Docs
``TR_FULL_ROW_HIGHLIGHT``           0x2000 Use this style to have the background colour and the selection highlight extend  over the entire horizontal row of the tree control window.
``TR_AUTO_CHECK_CHILD``             0x4000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are checked/unchecked as well.
``TR_AUTO_TOGGLE_CHILD``            0x8000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are toggled accordingly.
``TR_AUTO_CHECK_PARENT``           0x10000 Only meaningful foe checkbox-type items: when a child item is checked/unchecked its parent item is checked/unchecked as well.
``TR_ALIGN_WINDOWS``               0x20000 Flag used to align windows (in items with windows) at the same horizontal position.
``TR_NO_HEADER``                   0x40000 Use this style to hide the columns header.
``TR_VIRTUAL``                     0x80000 `HyperTreeList` will have virtual behaviour.
============================== =========== ==================================================


Events Processing
=================

This class processes the following events:

============================== ==================================================
Event Name                     Description
============================== ==================================================
``EVT_LIST_COL_BEGIN_DRAG``    The user started resizing a column - can be vetoed.
``EVT_LIST_COL_CLICK``         A column has been left-clicked.
``EVT_LIST_COL_DRAGGING``      The divider between columns is being dragged.
``EVT_LIST_COL_END_DRAG``      A column has been resized by the user.
``EVT_LIST_COL_RIGHT_CLICK``   A column has been right-clicked.
``EVT_TREE_BEGIN_DRAG``        Begin dragging with the left mouse button.
``EVT_TREE_BEGIN_LABEL_EDIT``  Begin editing a label. This can be prevented by calling `Veto()`.
``EVT_TREE_BEGIN_RDRAG``       Begin dragging with the right mouse button.
``EVT_TREE_DELETE_ITEM``       Delete an item.
``EVT_TREE_END_DRAG``          End dragging with the left or right mouse button.
``EVT_TREE_END_LABEL_EDIT``    End editing a label. This can be prevented by calling `Veto()`.
``EVT_TREE_GET_INFO``          Request information from the application (not implemented in `CustomTreeCtrl`).
``EVT_TREE_ITEM_ACTIVATED``    The item has been activated, i.e. chosen by double clicking it with mouse or from keyboard.
``EVT_TREE_ITEM_CHECKED``      A checkbox or radiobox type item has been checked.
``EVT_TREE_ITEM_CHECKING``     A checkbox or radiobox type item is being checked.
``EVT_TREE_ITEM_COLLAPSED``    The item has been collapsed.
``EVT_TREE_ITEM_COLLAPSING``   The item is being collapsed. This can be prevented by calling `Veto()`.
``EVT_TREE_ITEM_EXPANDED``     The item has been expanded.
``EVT_TREE_ITEM_EXPANDING``    The item is being expanded. This can be prevented by calling `Veto()`.
``EVT_TREE_ITEM_GETTOOLTIP``   The opportunity to set the item tooltip is being given to the application (call `TreeEvent.SetToolTip`).
``EVT_TREE_ITEM_HYPERLINK``    An hyperlink type item has been clicked.
``EVT_TREE_ITEM_MENU``         The context menu for the selected item has been requested, either by a right click or by using the menu key.
``EVT_TREE_ITEM_MIDDLE_CLICK`` The user has clicked the item with the middle mouse button (not implemented in `CustomTreeCtrl`).
``EVT_TREE_ITEM_RIGHT_CLICK``  The user has clicked the item with the right mouse button.
``EVT_TREE_KEY_DOWN``          A key has been pressed.
``EVT_TREE_SEL_CHANGED``       Selection has changed.
``EVT_TREE_SEL_CHANGING``      Selection is changing. This can be prevented by calling `Veto()`.
``EVT_TREE_SET_INFO``          Information is being supplied to the application (not implemented in `CustomTreeCtrl`).
``EVT_TREE_STATE_IMAGE_CLICK`` The state image has been clicked (not implemented in `CustomTreeCtrl`).
============================== ==================================================


License And Version
===================

HyperTreeList is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 21 Jun 2011, 22.00 GMT

Version 1.2

"""

import wx
import wx.gizmos

from customtreectrl import CustomTreeCtrl
from customtreectrl import DragImage, TreeEvent, GenericTreeItem
from customtreectrl import TreeEditTimer as TreeListEditTimer
from customtreectrl import EVT_TREE_ITEM_CHECKING, EVT_TREE_ITEM_CHECKED, EVT_TREE_ITEM_HYPERLINK

# Version Info
__version__ = "1.2"

# --------------------------------------------------------------------------
# Constants
# --------------------------------------------------------------------------

_NO_IMAGE = -1

_DEFAULT_COL_WIDTH = 100
_LINEHEIGHT = 10
_LINEATROOT = 5
_MARGIN = 2
_MININDENT = 16
_BTNWIDTH = 9
_BTNHEIGHT = 9
_EXTRA_WIDTH = 4
_EXTRA_HEIGHT = 4

_MAX_WIDTH = 30000  # pixels; used by OnPaint to redraw only exposed items

_DRAG_TIMER_TICKS = 250   # minimum drag wait time in ms
_FIND_TIMER_TICKS = 500   # minimum find wait time in ms
_EDIT_TIMER_TICKS = 250 # minimum edit wait time in ms


# --------------------------------------------------------------------------
# Additional HitTest style
# --------------------------------------------------------------------------
TREE_HITTEST_ONITEMCHECKICON  = 0x4000

# HyperTreeList styles
TR_NO_BUTTONS = wx.TR_NO_BUTTONS                               # for convenience
""" For convenience to document that no buttons are to be drawn. """
TR_HAS_BUTTONS = wx.TR_HAS_BUTTONS                             # draw collapsed/expanded btns
""" Use this style to show + and - buttons to the left of parent items. """
TR_NO_LINES = wx.TR_NO_LINES                                   # don't draw lines at all
""" Use this style to hide vertical level connectors. """
TR_LINES_AT_ROOT = wx.TR_LINES_AT_ROOT                         # connect top-level nodes
""" Use this style to show lines between root nodes. Only applicable if ``TR_HIDE_ROOT`` is""" \
""" set and ``TR_NO_LINES`` is not set. """
TR_TWIST_BUTTONS = wx.TR_TWIST_BUTTONS                         # still used by wxTreeListCtrl
""" Use old Mac-twist style buttons. """
TR_SINGLE = wx.TR_SINGLE                                       # for convenience
""" For convenience to document that only one item may be selected at a time. Selecting another""" \
""" item causes the current selection, if any, to be deselected. This is the default. """
TR_MULTIPLE = wx.TR_MULTIPLE                                   # can select multiple items
""" Use this style to allow a range of items to be selected. If a second range is selected,""" \
""" the current range, if any, is deselected. """
TR_EXTENDED = wx.TR_EXTENDED                                   # TODO: allow extended selection
""" Use this style to allow disjoint items to be selected. (Only partially implemented;""" \
""" may not work in all cases). """
TR_HAS_VARIABLE_ROW_HEIGHT = wx.TR_HAS_VARIABLE_ROW_HEIGHT     # what it says
""" Use this style to cause row heights to be just big enough to fit the content.""" \
""" If not set, all rows use the largest row height. The default is that this flag is unset. """
TR_EDIT_LABELS = wx.TR_EDIT_LABELS                             # can edit item labels
""" Use this style if you wish the user to be able to edit labels in the tree control. """
TR_ROW_LINES = wx.TR_ROW_LINES                                 # put border around items
""" Use this style to draw a contrasting border between displayed rows. """
TR_HIDE_ROOT = wx.TR_HIDE_ROOT                                 # don't display root node
""" Use this style to suppress the display of the root node, effectively causing the""" \
""" first-level nodes to appear as a series of root nodes. """
TR_FULL_ROW_HIGHLIGHT = wx.TR_FULL_ROW_HIGHLIGHT               # highlight full horz space
""" Use this style to have the background colour and the selection highlight extend """ \
""" over the entire horizontal row of the tree control window. """

TR_AUTO_CHECK_CHILD = 0x04000                                  # only meaningful for checkboxes
""" Only meaningful foe checkbox-type items: when a parent item is checked/unchecked""" \
""" its children are checked/unchecked as well. """
TR_AUTO_TOGGLE_CHILD = 0x08000                                 # only meaningful for checkboxes
""" Only meaningful foe checkbox-type items: when a parent item is checked/unchecked""" \
""" its children are toggled accordingly. """
TR_AUTO_CHECK_PARENT = 0x10000                                 # only meaningful for checkboxes
""" Only meaningful foe checkbox-type items: when a child item is checked/unchecked""" \
""" its parent item is checked/unchecked as well. """
TR_ALIGN_WINDOWS = 0x20000                                     # to align windows horizontally for items at the same level
""" Flag used to align windows (in items with windows) at the same horizontal position. """
TR_VIRTUAL = 0x80000
""" `HyperTreeList` will have virtual behaviour. """

# --------------------------------------------------------------------------
# Additional HyperTreeList style to hide the header
# --------------------------------------------------------------------------
TR_NO_HEADER = 0x40000
""" Use this style to hide the columns header. """
# --------------------------------------------------------------------------


def IsBufferingSupported():
    """
    Utility function which checks if a platform handles correctly double
    buffering for the header. Currently returns ``False`` for all platforms
    except Windows XP.
    """

    if wx.Platform != "__WXMSW__":
        return False

    if wx.App.GetComCtl32Version() >= 600:
        if wx.GetOsVersion()[1] > 5:
            # Windows Vista
            return False

        return True

    return False    
    

class TreeListColumnInfo(object):
    """
    Class used to store information (width, alignment flags, colours, etc...) about a
    L{HyperTreeList} column header.
    """

    def __init__(self, input="", width=_DEFAULT_COL_WIDTH, flag=wx.ALIGN_LEFT,
                 image=-1, shown=True, colour=None, edit=False):
        """
        Default class constructor.

        :param `input`: can be a string (representing the column header text) or
         another instance of L{TreeListColumnInfo}. In the latter case, all the
         other input parameters are not used;
        :param `width`: the column width in pixels;
        :param `flag`: the column alignment flag, one of ``wx.ALIGN_LEFT``,
         ``wx.ALIGN_RIGHT``, ``wx.ALIGN_CENTER``;
        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column;
        :param `shown`: ``True`` to show the column, ``False`` to hide it;
        :param `colour`: a valid `wx.Colour`, representing the text foreground colour
         for the column;
        :param `edit`: ``True`` to set the column as editable, ``False`` otherwise.
        """

        if isinstance(input, basestring):
            self._text = input
            self._width = width
            self._flag = flag
            self._image = image
            self._selected_image = -1
            self._shown = shown
            self._edit = edit
            self._font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
            if colour is None:
                self._colour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOWTEXT)
            else:
                self._colour = colour
                
        else:
    
            self._text = input._text
            self._width = input._width
            self._flag = input._flag
            self._image = input._image
            self._selected_image = input._selected_image
            self._shown = input._shown
            self._edit = input._edit
            self._colour = input._colour
            self._font = input._font
    

    # get/set
    def GetText(self):
        """ Returns the column header label. """
        
        return self._text

    
    def SetText(self, text):
        """
        Sets the column header label.

        :param `text`: the new column header text.
        """

        self._text = text
        return self


    def GetWidth(self):
        """ Returns the column header width in pixels. """

        return self._width 


    def SetWidth(self, width):
        """
        Sets the column header width.

        :param `width`: the column header width, in pixels.
        """

        self._width = width
        return self


    def GetAlignment(self):
        """ Returns the column text alignment. """

        return self._flag

    
    def SetAlignment(self, flag):
        """
        Sets the column text alignment.

        :param `flag`: the alignment flag, one of ``wx.ALIGN_LEFT``, ``wx.ALIGN_RIGHT``,
         ``wx.ALIGN_CENTER``.
        """

        self._flag = flag
        return self 


    def GetColour(self):
        """ Returns the column text colour. """

        return self._colour


    def SetColour(self, colour):
        """
        Sets the column text colour.

        :param `colour`: a valid `wx.Colour` object.
        """

        self._colour = colour
        return self
        

    def GetImage(self):
        """ Returns the column image index. """

        return self._image 


    def SetImage(self, image):
        """
        Sets the column image index.

        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column.
        """

        self._image = image
        return self 


    def GetSelectedImage(self):
        """ Returns the column image index in the selected state. """

        return self._selected_image
    

    def SetSelectedImage(self, image):
        """
        Sets the column image index in the selected state.

        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column when in
         selected state.
        """

        self._selected_image = image
        return self
    

    def IsEditable(self):
        """ Returns ``True`` if the column is editable, ``False`` otherwise. """

        return self._edit

    
    def SetEditable(self, edit):
        """
        Sets the column as editable or non-editable.

        :param `edit`: ``True`` if the column should be editable, ``False`` otherwise.
        """
        
        self._edit = edit
        return self 


    def IsShown(self):
        """ Returns ``True`` if the column is shown, ``False`` if it is hidden. """

        return self._shown

    
    def SetShown(self, shown):
        """
        Sets the column as shown or hidden.

        :param `shown`: ``True`` if the column should be shown, ``False`` if it
         should be hidden.
        """

        self._shown = shown
        return self 


    def SetFont(self, font):
        """
        Sets the column text font.

        :param `font`: a valid `wx.Font` object.
        """

        self._font = font
        return self


    def GetFont(self):
        """ Returns the column text font. """

        return self._font        


#-----------------------------------------------------------------------------
#  TreeListHeaderWindow (internal)
#-----------------------------------------------------------------------------

class TreeListHeaderWindow(wx.Window):
    """ A window which holds the header of L{HyperTreeList}. """
    
    def __init__(self, parent, id=wx.ID_ANY, owner=None, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, name="wxtreelistctrlcolumntitles"):
        """
        Default class constructor.

        :param `parent`: the window parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `owner`: the window owner, in this case an instance of L{TreeListMainWindow};
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `name`: the window name.
        """

        wx.Window.__init__(self, parent, id, pos, size, style, name=name)
        
        self._owner = owner
        self._currentCursor = wx.StockCursor(wx.CURSOR_DEFAULT)
        self._resizeCursor = wx.StockCursor(wx.CURSOR_SIZEWE)
        self._isDragging = False
        self._dirty = False
        self._total_col_width = 0
        self._hotTrackCol = -1
        self._columns = []
        self._headerCustomRenderer = None
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)
        self.Bind(wx.EVT_SET_FOCUS, self.OnSetFocus)

        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)


    def SetBuffered(self, buffered):
        """
        Sets/unsets the double buffering for the header.

        :param `buffered`: ``True`` to use double-buffering, ``False`` otherwise.

        :note: Currently we are using double-buffering only on Windows XP.
        """

        self._buffered = buffered


    # total width of all columns
    def GetWidth(self):
        """ Returns the total width of all columns. """

        return self._total_col_width 


    # column manipulation
    def GetColumnCount(self):
        """ Returns the total number of columns. """

        return len(self._columns)


    # column information manipulation
    def GetColumn(self, column):
        """
        Returns a column item, an instance of L{TreeListItem}.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column]


    def GetColumnText(self, column):
        """
        Returns the column text label.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].GetText()

    
    def SetColumnText(self, column, text):
        """
        Sets the column text label.

        :param `column`: an integer specifying the column index;
        :param `text`: the new column label.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].SetText(text)
    

    def GetColumnAlignment(self, column):
        """
        Returns the column text alignment.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].GetAlignment()
    

    def SetColumnAlignment(self, column, flag):
        """
        Sets the column text alignment.

        :param `column`: an integer specifying the column index;
        :param `flag`: the new text alignment flag.

        :see: L{TreeListColumnInfo.SetAlignment} for a list of valid alignment
         flags.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].SetAlignment(flag)
    

    def GetColumnWidth(self, column):
        """
        Returns the column width, in pixels.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].GetWidth()
    

    def GetColumnColour(self, column):
        """
        Returns the column text colour.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].GetColour()


    def SetColumnColour(self, column, colour):
        """
        Sets the column text colour.

        :param `column`: an integer specifying the column index;
        :param `colour`: a valid `wx.Colour` object.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].SetColour(colour)


    def IsColumnEditable(self, column):
        """
        Returns ``True`` if the column is editable, ``False`` otherwise.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        return self._columns[column].IsEditable()
    

    def IsColumnShown(self, column):
        """
        Returns ``True`` if the column is shown, ``False`` if it is hidden.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")

        return self._columns[column].IsShown()
    

    # shift the DC origin to match the position of the main window horz
    # scrollbar: this allows us to always use logical coords
    def AdjustDC(self, dc):
        """
        Shifts the `wx.DC` origin to match the position of the main window horizontal
        scrollbar: this allows us to always use logical coordinates.

        :param `dc`: an instance of `wx.DC`.        
        """
        
        xpix, dummy = self._owner.GetScrollPixelsPerUnit()
        x, dummy = self._owner.GetViewStart()

        # account for the horz scrollbar offset
        dc.SetDeviceOrigin(-x * xpix, 0)


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{TreeListHeaderWindow}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """
        
        if self._buffered:
            dc = wx.BufferedPaintDC(self)
        else:
            dc = wx.PaintDC(self)
            
        self.AdjustDC(dc)

        x = 0

        # width and height of the entire header window
        w, h = self.GetClientSize()
        w, dummy = self._owner.CalcUnscrolledPosition(w, 0)
        dc.SetBackgroundMode(wx.TRANSPARENT)

        numColumns = self.GetColumnCount()
        
        for i in xrange(numColumns):

            if x >= w:
                break
        
            if not self.IsColumnShown(i):
                continue # do next column if not shown

            params = wx.HeaderButtonParams()

            column = self.GetColumn(i)
            params.m_labelColour = column.GetColour()
            params.m_labelFont = column.GetFont()

            wCol = column.GetWidth()
            flags = 0
            rect = wx.Rect(x, 0, wCol, h)
            x += wCol

            if i == self._hotTrackCol:
                flags |= wx.CONTROL_CURRENT
            
            params.m_labelText = column.GetText()
            params.m_labelAlignment = column.GetAlignment()

            image = column.GetImage()
            imageList = self._owner.GetImageList()

            if image != -1 and imageList:
                params.m_labelBitmap = imageList.GetBitmap(image)

            if self._headerCustomRenderer != None:
               self._headerCustomRenderer.DrawHeaderButton(dc, rect, flags, params)
            else:
                wx.RendererNative.Get().DrawHeaderButton(self, dc, rect, flags,
                                                         wx.HDR_SORT_ICON_NONE, params)
       
        # Fill up any unused space to the right of the columns
        if x < w:
            rect = wx.Rect(x, 0, w-x, h)
            if self._headerCustomRenderer != None:
               self._headerCustomRenderer.DrawHeaderButton(dc, rect)
            else:
                wx.RendererNative.Get().DrawHeaderButton(self, dc, rect)
        

    def DrawCurrent(self):
        """ Draws the column resize line on a `wx.ScreenDC`. """
        
        x1, y1 = self._currentX, 0
        x1, y1 = self.ClientToScreen((x1, y1))
        x2 = self._currentX-1
        if wx.Platform == "__WXMSW__":
            x2 += 1 # but why ????

        y2 = 0
        dummy, y2 = self._owner.GetClientSize()
        x2, y2 = self._owner.ClientToScreen((x2, y2))

        dc = wx.ScreenDC()
        dc.SetLogicalFunction(wx.INVERT)
        dc.SetPen(wx.Pen(wx.BLACK, 2, wx.SOLID))
        dc.SetBrush(wx.TRANSPARENT_BRUSH)

        self.AdjustDC(dc)
        dc.DrawLine (x1, y1, x2, y2)
        dc.SetLogicalFunction(wx.COPY)
        
        
    def SetCustomRenderer(self, renderer=None):
        """
        Associate a custom renderer with the header - all columns will use it

        :param `renderer`: a class able to correctly render header buttons

        :note: the renderer class **must** implement the method `DrawHeaderButton`
        """

        self._headerCustomRenderer = renderer


    def XToCol(self, x):
        """
        Returns the column that corresponds to the logical input `x` coordinate.

        :param `x`: the `x` position to evaluate.

        :return: The column that corresponds to the logical input `x` coordinate,
         or ``wx.NOT_FOUND`` if there is no column at the `x` position.
        """
        
        colLeft = 0
        numColumns = self.GetColumnCount()
        for col in xrange(numColumns):
        
            if not self.IsColumnShown(col):
                continue 

            column = self.GetColumn(col)

            if x < (colLeft + column.GetWidth()):
                 return col
            
            colLeft += column.GetWidth()
        
        return wx.NOT_FOUND


    def RefreshColLabel(self, col):
        """
        Redraws the column.

        :param `col`: the index of the column to redraw.
        """

        if col >= self.GetColumnCount():
            return
        
        x = idx = width = 0
        while idx <= col:
            
            if not self.IsColumnShown(idx):
                idx += 1
                continue 

            column = self.GetColumn(idx)
            x += width
            width = column.GetWidth()
            idx += 1

        x, dummy = self._owner.CalcScrolledPosition(x, 0)
        self.RefreshRect(wx.Rect(x, 0, width, self.GetSize().GetHeight()))

        
    def OnMouse(self, event):
        """
        Handles the ``wx.EVT_MOUSE_EVENTS`` event for L{TreeListHeaderWindow}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        # we want to work with logical coords
        x, dummy = self._owner.CalcUnscrolledPosition(event.GetX(), 0)
        y = event.GetY()

        if event.Moving():
        
            col = self.XToCol(x)
            if col != self._hotTrackCol:
            
                # Refresh the col header so it will be painted with hot tracking
                # (if supported by the native renderer.)
                self.RefreshColLabel(col)

                # Also refresh the old hot header
                if self._hotTrackCol >= 0:
                    self.RefreshColLabel(self._hotTrackCol)

                self._hotTrackCol = col
            
        if event.Leaving() and self._hotTrackCol >= 0:
        
            # Leaving the window so clear any hot tracking indicator that may be present
            self.RefreshColLabel(self._hotTrackCol)
            self._hotTrackCol = -1
        
        if self._isDragging:

            self.SendListEvent(wx.wxEVT_COMMAND_LIST_COL_DRAGGING, event.GetPosition())

            # we don't draw the line beyond our window, but we allow dragging it
            # there
            w, dummy = self.GetClientSize()
            w, dummy = self._owner.CalcUnscrolledPosition(w, 0)
            w -= 6

            # erase the line if it was drawn
            if self._currentX < w:
                self.DrawCurrent()

            if event.ButtonUp():
                self._isDragging = False
                if self.HasCapture():
                    self.ReleaseMouse()
                self._dirty = True
                self.SetColumnWidth(self._column, self._currentX - self._minX)
                self.Refresh()
                self.SendListEvent(wx.wxEVT_COMMAND_LIST_COL_END_DRAG, event.GetPosition())
            else:
                self._currentX = max(self._minX + 7, x)

                # draw in the new location
                if self._currentX < w:
                    self.DrawCurrent()
            
        else: # not dragging

            self._minX = 0
            hit_border = False

            # end of the current column
            xpos = 0

            # find the column where this event occured
            countCol = self.GetColumnCount()

            for column in xrange(countCol):

                if not self.IsColumnShown(column):
                    continue # do next if not shown

                xpos += self.GetColumnWidth(column)
                self._column = column
                if abs (x-xpos) < 3 and y < 22:
                    # near the column border
                    hit_border = True
                    break
                
                if x < xpos:
                    # inside the column
                    break
            
                self._minX = xpos
            
            if event.LeftDown() or event.RightUp():
                if hit_border and event.LeftDown():
                    self._isDragging = True
                    self.CaptureMouse()
                    self._currentX = x
                    self.DrawCurrent()
                    self.SendListEvent(wx.wxEVT_COMMAND_LIST_COL_BEGIN_DRAG, event.GetPosition())
                else: # click on a column
                    evt = (event.LeftDown() and [wx.wxEVT_COMMAND_LIST_COL_CLICK] or [wx.wxEVT_COMMAND_LIST_COL_RIGHT_CLICK])[0]
                    self.SendListEvent(evt, event.GetPosition())
                
            elif event.LeftDClick() and hit_border:
                self.SetColumnWidth(self._column, self._owner.GetBestColumnWidth(self._column))
                self.Refresh()

            elif event.Moving():
                
                if hit_border:
                    setCursor = self._currentCursor == wx.STANDARD_CURSOR
                    self._currentCursor = self._resizeCursor
                else:
                    setCursor = self._currentCursor != wx.STANDARD_CURSOR
                    self._currentCursor = wx.STANDARD_CURSOR
                
                if setCursor:
                    self.SetCursor(self._currentCursor)
    

    def OnSetFocus(self, event):
        """
        Handles the ``wx.EVT_SET_FOCUS`` event for L{TreeListHeaderWindow}.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        self._owner.SetFocus()


    def SendListEvent(self, evtType, pos):
        """
        Sends a `wx.ListEvent` for the parent window.

        :param `evtType`: the event type;
        :param `pos`: an instance of `wx.Point`.
        """
        
        parent = self.GetParent()
        le = wx.ListEvent(evtType, parent.GetId())
        le.SetEventObject(parent)
        le.m_pointDrag = pos

        # the position should be relative to the parent window, not
        # this one for compatibility with MSW and common sense: the
        # user code doesn't know anything at all about this header
        # window, so why should it get positions relative to it?
        le.m_pointDrag.y -= self.GetSize().y
        le.m_col = self._column
        parent.GetEventHandler().ProcessEvent(le)


    def AddColumnInfo(self, colInfo):
        """
        Appends a column to the L{TreeListHeaderWindow}.

        :param `colInfo`: an instance of L{TreeListColumnInfo}.
        """
        
        self._columns.append(colInfo)
        self._total_col_width += colInfo.GetWidth()
        self._owner.AdjustMyScrollbars()
        self._owner._dirty = True


    def AddColumn(self, text, width=_DEFAULT_COL_WIDTH, flag=wx.ALIGN_LEFT,
                  image=-1, shown=True, colour=None, edit=False):
        """
        Appends a column to the L{TreeListHeaderWindow}.

        :param `text`: the column text label;
        :param `width`: the column width in pixels;
        :param `flag`: the column alignment flag, one of ``wx.ALIGN_LEFT``,
         ``wx.ALIGN_RIGHT``, ``wx.ALIGN_CENTER``;
        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column;
        :param `shown`: ``True`` to show the column, ``False`` to hide it;
        :param `colour`: a valid `wx.Colour`, representing the text foreground colour
         for the column;
        :param `edit`: ``True`` to set the column as editable, ``False`` otherwise.
        """

        colInfo = TreeListColumnInfo(text, width, flag, image, shown, colour, edit)
        self.AddColumnInfo(colInfo)


    def SetColumnWidth(self, column, width):
        """
        Sets the column width, in pixels.

        :param `column`: an integer specifying the column index;
        :param `width`: the new width for the column, in pixels.
        """
        
        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")

        self._total_col_width -= self._columns[column].GetWidth()
        self._columns[column].SetWidth(width)
        self._total_col_width += width
        self._owner.AdjustMyScrollbars()
        self._owner._dirty = True


    def InsertColumnInfo(self, before, colInfo):
        """
        Inserts a column to the L{TreeListHeaderWindow} at the position specified
        by `before`.

        :param `before`: the index at which we wish to insert the new column;
        :param `colInfo`: an instance of L{TreeListColumnInfo}.
        """

        if before < 0 or before >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        self._columns.insert(before, colInfo)
        self._total_col_width += colInfo.GetWidth()
        self._owner.AdjustMyScrollbars()
        self._owner._dirty = True


    def InsertColumn(self, before, text, width=_DEFAULT_COL_WIDTH,
                     flag=wx.ALIGN_LEFT, image=-1, shown=True, colour=None, 
                     edit=False):
        """
        Inserts a column to the L{TreeListHeaderWindow} at the position specified
        by `before`.

        :param `before`: the index at which we wish to insert the new column;
        :param `text`: the column text label;
        :param `width`: the column width in pixels;
        :param `flag`: the column alignment flag, one of ``wx.ALIGN_LEFT``,
         ``wx.ALIGN_RIGHT``, ``wx.ALIGN_CENTER``;
        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column;
        :param `shown`: ``True`` to show the column, ``False`` to hide it;
        :param `colour`: a valid `wx.Colour`, representing the text foreground colour
         for the column;
        :param `edit`: ``True`` to set the column as editable, ``False`` otherwise.        
        """
        
        colInfo = TreeListColumnInfo(text, width, flag, image, shown, colour, 
                                     edit)
        self.InsertColumnInfo(before, colInfo)


    def RemoveColumn(self, column):
        """
        Removes a column from the L{TreeListHeaderWindow}.

        :param `column`: an integer specifying the column index.
        """

        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        self._total_col_width -= self._columns[column].GetWidth()
        self._columns.pop(column)
        self._owner.AdjustMyScrollbars()
        self._owner._dirty = True


    def SetColumn(self, column, info):
        """
        Sets a column using an instance of L{TreeListColumnInfo}.

        :param `column`: an integer specifying the column index;
        :param `info`: an instance of L{TreeListColumnInfo}.        
        """
        
        if column < 0 or column >= self.GetColumnCount():
            raise Exception("Invalid column")
        
        w = self._columns[column].GetWidth()
        self._columns[column] = info
        
        if w != info.GetWidth():
            self._total_col_width += info.GetWidth() - w
            self._owner.AdjustMyScrollbars()
        
        self._owner._dirty = True
        

# ---------------------------------------------------------------------------
# TreeListItem
# ---------------------------------------------------------------------------
class TreeListItem(GenericTreeItem):
    """
    This class holds all the information and methods for every single item in
    L{HyperTreeList}.

    :note: Subclassed from L{customtreectrl.GenericTreeItem}.    
    """
    
    def __init__(self, mainWin, parent, text=[], ct_type=0, wnd=None, image=-1, selImage=-1, data=None):
        """
        Default class constructor.
        For internal use: do not call it in your code!

        :param `mainWin`: the main L{HyperTreeList} window, in this case an instance
         of L{TreeListMainWindow};
        :param `parent`: the tree item parent (may be ``None`` for root items);
        :param `text`: the tree item text;
        :param `ct_type`: the tree item kind. May be one of the following integers:

         =============== ==========================
         `ct_type` Value Description
         =============== ==========================
                0        A normal item
                1        A checkbox-like item
                2        A radiobutton-type item
         =============== ==========================

        :param `wnd`: if not ``None``, a non-toplevel window to be displayed next to
         the item;
        :param `image`: an index within the normal image list specifying the image to
         use for the item in unselected state;
        :param `selImage`: an index within the normal image list specifying the image to
         use for the item in selected state; if `image` > -1 and `selImage` is -1, the
         same image is used for both selected and unselected items;
        :param `data`: associate the given Python object `data` with the item.

        :note: Regarding radiobutton-type items (with `ct_type` = 2), the following
         approach is used:
         
         - All peer-nodes that are radiobuttons will be mutually exclusive. In other words,
           only one of a set of radiobuttons that share a common parent can be checked at
           once. If a radiobutton node becomes checked, then all of its peer radiobuttons
           must be unchecked.
         - If a radiobutton node becomes unchecked, then all of its child nodes will become
           inactive.        
        """

        self._col_images = []
        self._owner = mainWin

        # We don't know the height here yet.
        self._text_x = 0
        
        GenericTreeItem.__init__(self, parent, text, ct_type, wnd, image, selImage, data)        
 
        self._wnd = [None]             # are we holding a window?
        self._hidden = False
        
        if wnd:
            self.SetWindow(wnd)


    def IsHidden(self):
        """ Returns whether the item is hidden or not. """

        return self._hidden


    def Hide(self, hide):
        """
        Hides/shows the L{TreeListItem}.

        :param `hide`: ``True`` to hide the item, ``False`` to show it.
        """

        self._hidden = hide
        
    
    def DeleteChildren(self, tree):
        """
        Deletes the item children.

        :param `tree`: the main L{TreeListMainWindow} instance.
        """

        for child in self._children:
            if tree:
                tree.SendDeleteEvent(child)

            child.DeleteChildren(tree)
            
            if child == tree._selectItem:
                tree._selectItem = None

            # We have to destroy the associated window
            for wnd in child._wnd:
                if wnd:
                    wnd.Hide()
                    wnd.Destroy()
                    
            child._wnd = []

            if child in tree._itemWithWindow:
                tree._itemWithWindow.remove(child)
                
            del child
        
        self._children = []


    def HitTest(self, point, theCtrl, flags, column, level):
        """
        HitTest method for an item. Called from the main window HitTest.

        :param `point`: the point to test for the hit (an instance of `wx.Point`);
        :param `theCtrl`: the main L{TreeListMainWindow} tree;
        :param `flags`: a bitlist of hit locations;
        :param `column`: an integer specifying the column index;
        :param `level`: the item's level inside the tree hierarchy.
        
        :see: L{TreeListMainWindow.HitTest} method for the flags explanation.
        """

        # for a hidden root node, don't evaluate it, but do evaluate children
        if not theCtrl.HasAGWFlag(wx.TR_HIDE_ROOT) or level > 0:

            # reset any previous hit infos
            flags = 0
            column = -1
            header_win = theCtrl._owner.GetHeaderWindow()

            # check for right of all columns (outside)
            if point.x > header_win.GetWidth():
                return None, flags, wx.NOT_FOUND

            # evaluate if y-pos is okay
            h = theCtrl.GetLineHeight(self)
            
            if point.y >= self._y and point.y <= self._y + h:

                maincol = theCtrl.GetMainColumn()

                # check for above/below middle
                y_mid = self._y + h/2
                if point.y < y_mid:
                    flags |= wx.TREE_HITTEST_ONITEMUPPERPART
                else:
                    flags |= wx.TREE_HITTEST_ONITEMLOWERPART
                
                # check for button hit
                if self.HasPlus() and theCtrl.HasButtons():
                    bntX = self._x - theCtrl._btnWidth2
                    bntY = y_mid - theCtrl._btnHeight2
                    if ((point.x >= bntX) and (point.x <= (bntX + theCtrl._btnWidth)) and
                        (point.y >= bntY) and (point.y <= (bntY + theCtrl._btnHeight))):
                        flags |= wx.TREE_HITTEST_ONITEMBUTTON
                        column = maincol
                        return self, flags, column

                # check for hit on the check icons
                if self.GetType() != 0:
                    imageWidth = 0
                    numberOfMargins = 1
                    if self.GetCurrentImage() != _NO_IMAGE:
                        imageWidth = theCtrl._imgWidth
                        numberOfMargins += 1
                    chkX = self._text_x - imageWidth - numberOfMargins*_MARGIN - theCtrl._checkWidth
                    chkY = y_mid - theCtrl._checkHeight2
                    if ((point.x >= chkX) and (point.x <= (chkX + theCtrl._checkWidth)) and
                        (point.y >= chkY) and (point.y <= (chkY + theCtrl._checkHeight))):                    
                        flags |= TREE_HITTEST_ONITEMCHECKICON
                        return self, flags, maincol
                    
                # check for image hit
                if self.GetCurrentImage() != _NO_IMAGE:
                    imgX = self._text_x - theCtrl._imgWidth - _MARGIN                        
                    imgY = y_mid - theCtrl._imgHeight2
                    if ((point.x >= imgX) and (point.x <= (imgX + theCtrl._imgWidth)) and
                        (point.y >= imgY) and (point.y <= (imgY + theCtrl._imgHeight))):
                        flags |= wx.TREE_HITTEST_ONITEMICON
                        column = maincol
                        return self, flags, column
                    
                # check for label hit
                if ((point.x >= self._text_x) and (point.x <= (self._text_x + self._width))):
                    flags |= wx.TREE_HITTEST_ONITEMLABEL
                    column = maincol
                    return self, flags, column
                
                # check for indent hit after button and image hit
                if point.x < self._x:
                    flags |= wx.TREE_HITTEST_ONITEMINDENT
                    column = -1 # considered not belonging to main column
                    return self, flags, column
                
                # check for right of label
                end = 0
                for i in xrange(maincol):
                    end += header_win.GetColumnWidth(i)
                    if ((point.x > (self._text_x + self._width)) and (point.x <= end)):
                        flags |= wx.TREE_HITTEST_ONITEMRIGHT
                        column = -1 # considered not belonging to main column
                        return self, flags, column
                
                # else check for each column except main
                x = 0
                for j in xrange(theCtrl.GetColumnCount()):
                    if not header_win.IsColumnShown(j):
                        continue
                    w = header_win.GetColumnWidth(j)
                    if ((j != maincol) and (point.x >= x and point.x < x+w)):
                        flags |= wx.TREE_HITTEST_ONITEMCOLUMN
                        column = j
                        return self, flags, column
                    
                    x += w
                
                # no special flag or column found
                return self, flags, column

            # if children not expanded, return no item
            if not self.IsExpanded():
                return None, flags, wx.NOT_FOUND
        
        # in any case evaluate children
        for child in self._children:
            hit, flags, column = child.HitTest(point, theCtrl, flags, column, level+1)
            if hit:
                return hit, flags, column
        
        # not found
        return None, flags, wx.NOT_FOUND


    def GetText(self, column=None):
        """
        Returns the item text label.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]
        
        if len(self._text) > 0:
            if self._owner.IsVirtual():
                return self._owner.GetItemText(self._data, column)
            else:
                return self._text[column]
        
        return ""
    

    def GetImage(self, which=wx.TreeItemIcon_Normal, column=None):
        """
        Returns the item image for a particular item state.

        :param `which`: can be one of the following bits:

         ================================= ========================
         Item State                        Description
         ================================= ========================
         ``TreeItemIcon_Normal``           To get the normal item image
         ``TreeItemIcon_Selected``         To get the selected item image (i.e. the image which is shown when the item is currently selected)
         ``TreeItemIcon_Expanded``         To get the expanded image (this only makes sense for items which have children - then this image is shown when the item is expanded and the normal image is shown when it is collapsed)
         ``TreeItemIcon_SelectedExpanded`` To get the selected expanded image (which is shown when an expanded item is currently selected) 
         ================================= ========================

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]

        if column == self._owner.GetMainColumn():
            return self._images[which]
        
        if column < len(self._col_images):
            return self._col_images[column]

        return _NO_IMAGE


    def GetCurrentImage(self, column=None):
        """
        Returns the current item image.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.        
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]

        if column != self._owner.GetMainColumn():
            return self.GetImage(column=column)
        
        image = GenericTreeItem.GetCurrentImage(self)
        return image
    

    def SetText(self, column, text):
        """
        Sets the item text label.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used;
        :param `text`: a string specifying the new item label.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]
    
        if column < len(self._text):
            self._text[column] = text
        elif column < self._owner.GetColumnCount():
            self._text.extend([""] * (column - len(self._text) + 1))
            self._text[column] = text
        

    def SetImage(self, column, image, which):
        """
        Sets the item image for a particular item state.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used;
        :param `image`: an index within the normal image list specifying the image to use;
        :param `which`: the item state.

        :see: L{GetImage} for a list of valid item states.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]
    
        if column == self._owner.GetMainColumn():
            self._images[which] = image
        elif column < len(self._col_images):
            self._col_images[column] = image
        elif column < self._owner.GetColumnCount():
            self._col_images.extend([_NO_IMAGE] * (column - len(self._col_images) + 1))
            self._col_images[column] = image
        
    
    def GetTextX(self):
        """ Returns the `x` position of the item text. """

        return self._text_x

    
    def SetTextX(self, text_x):
        """
        Sets the `x` position of the item text.

        :param `text_x`: the `x` position of the item text.
        """

        self._text_x = text_x 


    def SetWindow(self, wnd, column=None):
        """
        Sets the window associated to the item.

        :param `wnd`: a non-toplevel window to be displayed next to the item;
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]

        if type(self._wnd) != type([]):
            self._wnd = [self._wnd]

        if column < len(self._wnd):
            self._wnd[column] = wnd
        elif column < self._owner.GetColumnCount():
            self._wnd.extend([None] * (column - len(self._wnd) + 1))
            self._wnd[column] = wnd

        if self not in self._owner._itemWithWindow:
            self._owner._itemWithWindow.append(self)
            
        # We have to bind the wx.EVT_SET_FOCUS for the associated window
        # No other solution to handle the focus changing from an item in
        # HyperTreeList and the window associated to an item
        # Do better strategies exist?
        wnd.Bind(wx.EVT_SET_FOCUS, self.OnSetFocus)
        
        # We don't show the window if the item is collapsed
        if self._isCollapsed:
            wnd.Show(False)

        # The window is enabled only if the item is enabled                
        wnd.Enable(self._enabled)
        

    def OnSetFocus(self, event):
        """
        Handles the ``wx.EVT_SET_FOCUS`` event for a window associated to an item.

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        treectrl = self._owner
        select = treectrl.GetSelection()

        # If the window is associated to an item that currently is selected
        # (has focus) we don't kill the focus. Otherwise we do it.
        if select != self:
            treectrl._hasFocus = False
        else:
            treectrl._hasFocus = True
            
        event.Skip()

        
    def GetWindow(self, column=None):
        """
        Returns the window associated to the item.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.        
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]
        
        if column >= len(self._wnd):
            return None

        return self._wnd[column]        


    def DeleteWindow(self, column=None):
        """
        Deletes the window associated to the item (if any).

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]

        if column >= len(self._wnd):
            return
        
        if self._wnd[column]:
            self._wnd[column].Destroy()
            self._wnd[column] = None
        

    def GetWindowEnabled(self, column=None):
        """
        Returns whether the window associated with an item is enabled or not.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]

        if not self._wnd[column]:
            raise Exception("\nERROR: This Item Has No Window Associated At Column %s"%column)

        return self._wnd[column].IsEnabled()


    def SetWindowEnabled(self, enable=True, column=None):
        """
        Sets whether the window associated with an item is enabled or not.

        :param `enable`: ``True`` to enable the associated window, ``False`` to disable it;
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]

        if not self._wnd[column]:
            raise Exception("\nERROR: This Item Has No Window Associated At Column %s"%column)

        self._wnd[column].Enable(enable)


    def GetWindowSize(self, column=None):
        """
        Returns the associated window size.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]
        
        if not self._wnd[column]:
            raise Exception("\nERROR: This Item Has No Window Associated At Column %s"%column)
        
        return self._wnd[column].GetSize()   


#-----------------------------------------------------------------------------
# EditTextCtrl (internal)
#-----------------------------------------------------------------------------


class EditCtrl(object):
    """
    Base class for controls used for in-place edit.
    """

    def __init__(self, parent, id=wx.ID_ANY, item=None, column=None, owner=None,
                 value="", pos=wx.DefaultPosition, size=wx.DefaultSize, style=0,
                 validator=wx.DefaultValidator, name="editctrl", **kwargs):
        """
        Default class constructor.

        :param `parent`: the window parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used;
        :param `owner`: the window owner, in this case an instance of L{TreeListMainWindow};
        :param `value`: the initial value in the control;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `validator`: the window validator;
        :param `name`: the window name.
        """
        self._owner = owner
        self._startValue = value
        self._itemEdited = item
        self._finished = False
        
        column = (column is not None and [column] or [self._owner.GetMainColumn()])[0]
        
        self._column = column

        w = self._itemEdited.GetWidth()
        h = self._itemEdited.GetHeight()

        wnd = self._itemEdited.GetWindow(column)
        if wnd:
            w = w - self._itemEdited.GetWindowSize(column)[0]
            h = 0

        x = item.GetX()

        if column > 0:
            x = 0
            
        for i in xrange(column):
            if not self._owner.GetParent()._header_win.IsColumnShown(i):
                continue # do next column if not shown
            
            col = self._owner.GetParent()._header_win.GetColumn(i)
            wCol = col.GetWidth()
            x += wCol
        
        x, y = self._owner.CalcScrolledPosition(x+2, item.GetY())

        image_w = image_h = wcheck = hcheck = 0
        image = item.GetCurrentImage(column)

        if image != _NO_IMAGE:
    
            if self._owner._imageListNormal:
                image_w, image_h = self._owner._imageListNormal.GetSize(image)
                image_w += 2*_MARGIN
        
            else:
        
                raise Exception("\n ERROR: You Must Create An Image List To Use Images!")

        if column > 0:
            checkimage = item.GetCurrentCheckedImage()
            if checkimage is not None:
                wcheck, hcheck = self._owner._imageListCheck.GetSize(checkimage)
                wcheck += 2*_MARGIN

        if wnd:
            h = max(hcheck, image_h)
            dc = wx.ClientDC(self._owner)
            h = max(h, dc.GetTextExtent("Aq")[1])
            h = h + 2
            
        # FIXME: what are all these hardcoded 4, 8 and 11s really?
        x += image_w + wcheck
        w -= image_w + 2*_MARGIN + wcheck

        super(EditCtrl, self).__init__(parent, id, value, wx.Point(x,y),
                                       wx.Size(w+15, h), 
                                       style=style|wx.SIMPLE_BORDER, 
                                       name=name, **kwargs)
        
        if wx.Platform == "__WXMAC__":
            self.SetFont(owner.GetFont())
            bs = self.GetBestSize()
            self.SetSize((-1, bs.height))

        self.Bind(wx.EVT_KILL_FOCUS, self.OnKillFocus)


    def item(self):
        """Returns the item currently edited."""

        return self._itemEdited


    def column(self): 
        """Returns the column currently edited.""" 

        return self._column


    def StopEditing(self):
        """Suddenly stops the editing."""

        self._owner.OnCancelEdit()
        self.Finish()


    def Finish(self):
        """Finish editing."""

        if not self._finished:
        
            self._finished = True
            self._owner.SetFocusIgnoringChildren()
            self._owner.ResetEditControl()


    def AcceptChanges(self):
        """Accepts/refuses the changes made by the user."""

        value = self.GetValue()

        if value == self._startValue:
            # nothing changed, always accept
            # when an item remains unchanged, the owner
            # needs to be notified that the user decided
            # not to change the tree item label, and that
            # the edit has been cancelled
            self._owner.OnCancelEdit()
            return True
        else:
            return self._owner.OnAcceptEdit(value)


    def OnKillFocus(self, event):
        """
        Handles the ``wx.EVT_KILL_FOCUS`` event for L{EditCtrl}

        :param `event`: a `wx.FocusEvent` event to be processed.
        """

        # We must let the native control handle focus, too, otherwise
        # it could have problems with the cursor (e.g., in wxGTK).
        event.Skip()



class EditTextCtrl(EditCtrl, wx.TextCtrl):
    """
    Text control used for in-place edit.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, item=None, column=None, owner=None,
                 value="", pos=wx.DefaultPosition, size=wx.DefaultSize, style=0,
                 validator=wx.DefaultValidator, name="edittextctrl", **kwargs):
        """
        Default class constructor.
        For internal use: do not call it in your code!

        :param `parent`: the window parent. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used;
        :param `owner`: the window owner, in this case an instance of L{TreeListMainWindow};
        :param `value`: the initial value in the text control;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the window style;
        :param `validator`: the window validator;
        :param `name`: the window name.
        """

        super(EditTextCtrl, self).__init__(parent, id, item, column, owner, 
                                           value, pos, size, style, validator, 
                                           name, **kwargs)       
        self.SelectAll()

        self.Bind(wx.EVT_CHAR, self.OnChar)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)


    def OnChar(self, event):
        """
        Handles the ``wx.EVT_CHAR`` event for L{EditTextCtrl}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        keycode = event.GetKeyCode()

        if keycode in [wx.WXK_RETURN, wx.WXK_NUMPAD_ENTER] and not event.ShiftDown():
            # Notify the owner about the changes
            self.AcceptChanges()
            # Even if vetoed, close the control (consistent with MSW)
            wx.CallAfter(self.Finish)

        elif keycode == wx.WXK_ESCAPE:
            self.StopEditing()

        else:
            event.Skip()
    

    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_KEY_UP`` event for L{EditTextCtrl}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        if not self._finished:

            # auto-grow the textctrl:
            parentSize = self._owner.GetSize()
            myPos = self.GetPosition()
            mySize = self.GetSize()
            
            sx, sy = self.GetTextExtent(self.GetValue() + "M")
            if myPos.x + sx > parentSize.x:
                sx = parentSize.x - myPos.x
            if mySize.x > sx:
                sx = mySize.x
                
            self.SetSize((sx, -1))

        event.Skip()
        
        
        
# ---------------------------------------------------------------------------
# TreeListMainWindow implementation
# ---------------------------------------------------------------------------

class TreeListMainWindow(CustomTreeCtrl):
    """
    This class represents the main window (and thus the main column) in L{HyperTreeList}.

    :note: This is a subclass of L{CustomTreeCtrl}.
    """

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=wx.TR_DEFAULT_STYLE, validator=wx.DefaultValidator,
                 name="wxtreelistmainwindow"):
        """
        Default class constructor.
        
        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyScrolledWindow` style;
        :param `agwStyle`: the AGW-specific L{TreeListMainWindow} window style. This can be a
         combination of the following bits:
        
         ============================== =========== ==================================================
         Window Styles                  Hex Value   Description
         ============================== =========== ==================================================
         ``TR_NO_BUTTONS``                      0x0 For convenience to document that no buttons are to be drawn.
         ``TR_SINGLE``                          0x0 For convenience to document that only one item may be selected at a time. Selecting another item causes the current selection, if any, to be deselected. This is the default.
         ``TR_HAS_BUTTONS``                     0x1 Use this style to show + and - buttons to the left of parent items.
         ``TR_NO_LINES``                        0x4 Use this style to hide vertical level connectors.
         ``TR_LINES_AT_ROOT``                   0x8 Use this style to show lines between root nodes. Only applicable if ``TR_HIDE_ROOT`` is set and ``TR_NO_LINES`` is not set.
         ``TR_DEFAULT_STYLE``                   0x9 The set of flags that are closest to the defaults for the native control for a particular toolkit.
         ``TR_TWIST_BUTTONS``                  0x10 Use old Mac-twist style buttons.
         ``TR_MULTIPLE``                       0x20 Use this style to allow a range of items to be selected. If a second range is selected, the current range, if any, is deselected.
         ``TR_EXTENDED``                       0x40 Use this style to allow disjoint items to be selected. (Only partially implemented; may not work in all cases).
         ``TR_HAS_VARIABLE_ROW_HEIGHT``        0x80 Use this style to cause row heights to be just big enough to fit the content. If not set, all rows use the largest row height. The default is that this flag is unset.
         ``TR_EDIT_LABELS``                   0x200 Use this style if you wish the user to be able to edit labels in the tree control.
         ``TR_ROW_LINES``                     0x400 Use this style to draw a contrasting border between displayed rows.
         ``TR_HIDE_ROOT``                     0x800 Use this style to suppress the display of the root node, effectively causing the first-level nodes to appear as a series of root nodes.
         ``TR_FULL_ROW_HIGHLIGHT``           0x2000 Use this style to have the background colour and the selection highlight extend  over the entire horizontal row of the tree control window.
         ``TR_AUTO_CHECK_CHILD``             0x4000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are checked/unchecked as well.
         ``TR_AUTO_TOGGLE_CHILD``            0x8000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are toggled accordingly.
         ``TR_AUTO_CHECK_PARENT``           0x10000 Only meaningful foe checkbox-type items: when a child item is checked/unchecked its parent item is checked/unchecked as well.
         ``TR_ALIGN_WINDOWS``               0x20000 Flag used to align windows (in items with windows) at the same horizontal position.
         ``TR_NO_HEADER``                   0x40000 Use this style to hide the columns header.
         ``TR_VIRTUAL``                     0x80000 L{HyperTreeList} will have virtual behaviour.
         ============================== =========== ==================================================

        :param `validator`: window validator;
        :param `name`: window name.
        """

        self._buffered = False
        
        CustomTreeCtrl.__init__(self, parent, id, pos, size, style, agwStyle, validator, name)
        
        self._shiftItem = None
        self._editItem = None
        self._selectItem = None

        self._curColumn = -1 # no current column
        self._owner = parent
        self._main_column = 0
        self._dragItem = None

        self._imgWidth = self._imgWidth2 = 0
        self._imgHeight = self._imgHeight2 = 0
        self._btnWidth = self._btnWidth2 = 0
        self._btnHeight = self._btnHeight2 = 0
        self._checkWidth = self._checkWidth2 = 0
        self._checkHeight = self._checkHeight2 = 0
        self._agwStyle = agwStyle
        self._current = None

        # TextCtrl initial settings for editable items
        self._editTimer = TreeListEditTimer(self)
        self._left_down_selection = False

        self._dragTimer = wx.Timer(self)
        self._findTimer = wx.Timer(self)
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)
        self.Bind(wx.EVT_SCROLLWIN, self.OnScroll)

        # Sets the focus to ourselves: this is useful if you have items
        # with associated widgets.
        self.SetFocus()
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)


    def SetBuffered(self, buffered):
        """
        Sets/unsets the double buffering for the main window.

        :param `buffered`: ``True`` to use double-buffering, ``False`` otherwise.

        :note: Currently we are using double-buffering only on Windows XP.
        """

        self._buffered = buffered
        if buffered:
            self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        else:
            self.SetBackgroundStyle(wx.BG_STYLE_SYSTEM)


    def IsVirtual(self):
        """ Returns ``True`` if L{TreeListMainWindow} has the ``TR_VIRTUAL`` flag set. """
        
        return self.HasAGWFlag(TR_VIRTUAL)


#-----------------------------------------------------------------------------
# functions to work with tree items
#-----------------------------------------------------------------------------

    def GetItemImage(self, item, column=None, which=wx.TreeItemIcon_Normal):
        """
        Returns the item image.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used;
        :param `which`: can be one of the following bits:

         ================================= ========================
         Item State                        Description
         ================================= ========================
         ``TreeItemIcon_Normal``           To get the normal item image
         ``TreeItemIcon_Selected``         To get the selected item image (i.e. the image which is shown when the item is currently selected)
         ``TreeItemIcon_Expanded``         To get the expanded image (this only makes sense for items which have children - then this image is shown when the item is expanded and the normal image is shown when it is collapsed)
         ``TreeItemIcon_SelectedExpanded`` To get the selected expanded image (which is shown when an expanded item is currently selected) 
         ================================= ========================
        """
        
        column = (column is not None and [column] or [self._main_column])[0]

        if column < 0:
            return _NO_IMAGE

        return item.GetImage(which, column)


    def SetItemImage(self, item, image, column=None, which=wx.TreeItemIcon_Normal):
        """
        Sets the item image for a particular item state.

        :param `item`: an instance of L{TreeListItem};
        :param `image`: an index within the normal image list specifying the image to use;
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used;
        :param `which`: the item state.

        :see: L{GetItemImage} for a list of valid item states.
        """
        
        column = (column is not None and [column] or [self._main_column])[0]

        if column < 0:
            return
        
        item.SetImage(column, image, which)
        dc = wx.ClientDC(self)
        self.CalculateSize(item, dc)
        self.RefreshLine(item)


    def GetItemWindowEnabled(self, item, column=None):
        """
        Returns whether the window associated with an item is enabled or not.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        return item.GetWindowEnabled(column)


    def GetItemWindow(self, item, column=None):
        """
        Returns the window associated with an item.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """
        
        return item.GetWindow(column)


    def SetItemWindow(self, item, window, column=None):
        """
        Sets the window associated to an item.

        :param `item`: an instance of L{TreeListItem};
        :param `wnd`: a non-toplevel window to be displayed next to the item;
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.

        :note: The window parent should not be the L{HyperTreeList} itself, but actually
         an instance of L{TreeListMainWindow}. The current solution here is to reparent
         the window to this class.
        """

        # Reparent the window to ourselves
        if window.GetParent() != self:
            window.Reparent(self)
        
        item.SetWindow(window, column)
        if window:
            self._hasWindows = True
        

    def SetItemWindowEnabled(self, item, enable=True, column=None):
        """
        Sets whether the window associated with an item is enabled or not.

        :param `item`: an instance of L{TreeListItem};
        :param `enable`: ``True`` to enable the associated window, ``False`` to disable it;
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        item.SetWindowEnabled(enable, column)


# ----------------------------------------------------------------------------
# navigation
# ----------------------------------------------------------------------------

    def IsItemVisible(self, item):
        """
        Returns whether the item is visible or not.

        :param `item`: an instance of L{TreeListItem};
        """

        # An item is only visible if it's not a descendant of a collapsed item
        parent = item.GetParent()

        while parent:
        
            if not parent.IsExpanded():
                return False
            
            parent = parent.GetParent()
        
        startX, startY = self.GetViewStart()
        clientSize = self.GetClientSize()

        rect = self.GetBoundingRect(item)
        
        if not rect:
            return False
        if rect.GetWidth() == 0 or rect.GetHeight() == 0:
            return False
        if rect.GetBottom() < 0 or rect.GetTop() > clientSize.y:
            return False
        if rect.GetRight() < 0 or rect.GetLeft() > clientSize.x:
            return False

        return True


    def GetPrevChild(self, item, cookie):
        """
        Returns the previous child of an item.

        :param `item`: an instance of L{TreeListItem};
        :param `cookie`: a parameter which is opaque for the application but is necessary
         for the library to make these functions reentrant (i.e. allow more than one
         enumeration on one and the same object simultaneously).

        :note: This method returns ``None`` if there are no further siblings.
        """

        children = item.GetChildren()

        if cookie >= 0:            
            return children[cookie], cookie-1
        else:        
            # there are no more of them
            return None, cookie


    def GetFirstExpandedItem(self):
        """ Returns the first item which is in the expanded state. """

        return self.GetNextExpanded(self.GetRootItem())


    def GetNextExpanded(self, item):
        """
        Returns the next expanded item after the input one.

        :param `item`: an instance of L{TreeListItem}.
        """                

        return self.GetNext(item, False)


    def GetPrevExpanded(self, item):
        """
        Returns the previous expanded item before the input one.

        :param `item`: an instance of L{TreeListItem}.
        """                

        return self.GetPrev(item, False)


    def GetFirstVisibleItem(self):
        """ Returns the first visible item. """

        return self.GetNextVisible(self.GetRootItem())


    def GetPrevVisible(self, item):
        """
        Returns the previous visible item before the input one.

        :param `item`: an instance of L{TreeListItem}.
        """                

        i = self.GetNext(item, False)
        while i:
            if self.IsItemVisible(i):
                return i
            i = self.GetPrev(i, False)
        
        return None


# ----------------------------------------------------------------------------
# operations
# ----------------------------------------------------------------------------

    def DoInsertItem(self, parent, previous, text, ct_type=0, wnd=None, image=-1, selImage=-1, data=None):
        """
        Actually inserts an item in the tree.

        :param `parentId`: an instance of L{TreeListItem} representing the
         item's parent;
        :param `previous`: the index at which we should insert the item;
        :param `text`: the item text label;
        :param `ct_type`: the item type (see L{CustomTreeCtrl.SetItemType} for a list of valid
         item types);
        :param `wnd`: if not ``None``, a non-toplevel window to show next to the item;
        :param `image`: an index within the normal image list specifying the image to
         use for the item in unselected state;
        :param `selImage`: an index within the normal image list specifying the image to
         use for the item in selected state; if `image` > -1 and `selImage` is -1, the
         same image is used for both selected and unselected items;
        :param `data`: associate the given Python object `data` with the item.
        """
        
        self._dirty = True # do this first so stuff below doesn't cause flicker
        arr = [""]*self.GetColumnCount()
        arr[self._main_column] = text
        
        if not parent:
            # should we give a warning here?
            return self.AddRoot(text, ct_type, wnd, image, selImage, data)
        
        self._dirty = True     # do this first so stuff below doesn't cause flicker

        item = TreeListItem(self, parent, arr, ct_type, wnd, image, selImage, data)
        
        if wnd is not None:
            self._hasWindows = True
            self._itemWithWindow.append(item)
        
        parent.Insert(item, previous)

        return item


    def AddRoot(self, text, ct_type=0, wnd=None, image=-1, selImage=-1, data=None):
        """
        Adds a root item to the L{TreeListMainWindow}.

        :param `text`: the item text label;
        :param `ct_type`: the item type (see L{CustomTreeCtrl.SetItemType} for a list of valid
         item types);
        :param `wnd`: if not ``None``, a non-toplevel window to show next to the item;
        :param `image`: an index within the normal image list specifying the image to
         use for the item in unselected state;
        :param `selImage`: an index within the normal image list specifying the image to
         use for the item in selected state; if `image` > -1 and `selImage` is -1, the
         same image is used for both selected and unselected items;
        :param `data`: associate the given Python object `data` with the item.

        :warning: only one root is allowed to exist in any given instance of L{TreeListMainWindow}.
        """        

        if self._anchor:
            raise Exception("\nERROR: Tree Can Have Only One Root")

        if wnd is not None and not (self._agwStyle & wx.TR_HAS_VARIABLE_ROW_HEIGHT):
            raise Exception("\nERROR: In Order To Append/Insert Controls You Have To Use The Style TR_HAS_VARIABLE_ROW_HEIGHT")

        if text.find("\n") >= 0 and not (self._agwStyle & wx.TR_HAS_VARIABLE_ROW_HEIGHT):
            raise Exception("\nERROR: In Order To Append/Insert A MultiLine Text You Have To Use The Style TR_HAS_VARIABLE_ROW_HEIGHT")

        if ct_type < 0 or ct_type > 2:
            raise Exception("\nERROR: Item Type Should Be 0 (Normal), 1 (CheckBox) or 2 (RadioButton). ")

        self._dirty = True     # do this first so stuff below doesn't cause flicker
        arr = [""]*self.GetColumnCount()
        arr[self._main_column] = text
        self._anchor = TreeListItem(self, None, arr, ct_type, wnd, image, selImage, data)
        
        if wnd is not None:
            self._hasWindows = True
            self._itemWithWindow.append(self._anchor)            
        
        if self.HasAGWFlag(wx.TR_HIDE_ROOT):
            # if root is hidden, make sure we can navigate
            # into children
            self._anchor.SetHasPlus()
            self._anchor.Expand()
            self.CalculatePositions()
        
        if not self.HasAGWFlag(wx.TR_MULTIPLE):
            self._current = self._key_current = self._selectItem = self._anchor
            self._current.SetHilight(True)
        
        return self._anchor


    def Delete(self, item):
        """
        Deletes an item.

        :param `item`: an instance of L{TreeListItem}.
        """

        if not item:
            raise Exception("\nERROR: Invalid Tree Item. ")
        
        self._dirty = True     # do this first so stuff below doesn't cause flicker

        if self._editCtrl != None and self.IsDescendantOf(item, self._editCtrl.item()):
            # can't delete the item being edited, cancel editing it first
            self._editCtrl.StopEditing()

        # don't stay with invalid self._shiftItem or we will crash in the next call to OnChar()
        changeKeyCurrent = False
        itemKey = self._shiftItem
        
        while itemKey:
            if itemKey == item:  # self._shiftItem is a descendant of the item being deleted
                changeKeyCurrent = True
                break
            
            itemKey = itemKey.GetParent()
        
        parent = item.GetParent()
        if parent:
            parent.GetChildren().remove(item)  # remove by value
        
        if changeKeyCurrent:
            self._shiftItem = parent

        self.SendDeleteEvent(item)
        if self._selectItem == item:
            self._selectItem = None

        # Remove the item with window
        if item in self._itemWithWindow:
            for wnd in item._wnd:
                if wnd:
                    wnd.Hide()
                    wnd.Destroy()
                
            item._wnd = []
            self._itemWithWindow.remove(item)
            
        item.DeleteChildren(self)
        del item


    # Don't leave edit or selection on a child which is about to disappear
    def ChildrenClosing(self, item):
        """
        We are about to destroy the item's children.

        :param `item`: an instance of L{TreeListItem}.
        """

        if self._editCtrl != None and item != self._editCtrl.item() and self.IsDescendantOf(item, self._editCtrl.item()):
            self._editCtrl.StopEditing()

        if self.IsDescendantOf(item, self._selectItem):
            self._selectItem = item
            
        if item != self._current and self.IsDescendantOf(item, self._current):
            self._current.SetHilight(False)
            self._current = None

            
    def DeleteRoot(self):
        """
        Removes the tree root item (and subsequently all the items in
        L{TreeListMainWindow}.
        """

        if self._anchor:

            self._dirty = True
            self.SendDeleteEvent(self._anchor)
            self._current = None
            self._selectItem = None
            self._anchor.DeleteChildren(self)
            del self._anchor
            self._anchor = None


    def DeleteAllItems(self):
        """ Delete all items in the L{TreeListMainWindow}. """

        self.DeleteRoot()
        

    def HideWindows(self):
        """ Hides the windows associated to the items. Used internally. """
        
        for child in self._itemWithWindow:
            if not self.IsItemVisible(child):
                for column in xrange(self.GetColumnCount()):
                    wnd = child.GetWindow(column)
                    if wnd and wnd.IsShown():
                        wnd.Hide()
                

    def EnableItem(self, item, enable=True, torefresh=True):
        """
        Enables/disables an item.

        :param `item`: an instance of L{TreeListItem};
        :param `enable`: ``True`` to enable the item, ``False`` otherwise;
        :param `torefresh`: whether to redraw the item or not.
        """
        
        if item.IsEnabled() == enable:
            return

        if not enable and item.IsSelected():
            self.DoSelectItem(item, not self.HasAGWFlag(wx.TR_MULTIPLE))

        item.Enable(enable)

        for column in xrange(self.GetColumnCount()):
            wnd = item.GetWindow(column)

            # Handles the eventual window associated to the item        
            if wnd:
                wnd.Enable(enable)
        
        if torefresh:
            # We have to refresh the item line
            dc = wx.ClientDC(self)
            self.CalculateSize(item, dc)
            self.RefreshLine(item)


    def IsItemEnabled(self, item):
        """
        Returns whether an item is enabled or disabled.

        :param `item`: an instance of L{TreeListItem}.
        """

        return item.IsEnabled()
    

    def GetCurrentItem(self):
        """ Returns the current item. """

        return self._current

    
    def GetColumnCount(self):
        """ Returns the total number of columns. """

        return self._owner.GetHeaderWindow().GetColumnCount()


    def SetMainColumn(self, column):
        """
        Sets the L{HyperTreeList} main column (i.e. the position of the underlying
        L{CustomTreeCtrl}.

        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """
        
        if column >= 0 and column < self.GetColumnCount():
            self._main_column = column


    def GetMainColumn(self):
        """
        Returns the L{HyperTreeList} main column (i.e. the position of the underlying
        L{CustomTreeCtrl}.
        """
        
        return self._main_column
    

    def ScrollTo(self, item):
        """
        Scrolls the specified item into view.

        :param `item`: an instance of L{TreeListItem}.
        """

        # ensure that the position of the item it calculated in any case
        if self._dirty:
            self.CalculatePositions()

        # now scroll to the item
        xUnit, yUnit = self.GetScrollPixelsPerUnit()
        start_x, start_y = self.GetViewStart()
        start_y *= yUnit
        client_w, client_h = self.GetClientSize ()

        x, y = self._anchor.GetSize (0, 0, self)
        x = self._owner.GetHeaderWindow().GetWidth()
        y += yUnit + 2 # one more scrollbar unit + 2 pixels
        x_pos = self.GetScrollPos(wx.HORIZONTAL)

        if item._y < start_y+3:
            # going down, item should appear at top
            self.SetScrollbars(xUnit, yUnit, (xUnit and [x/xUnit] or [0])[0], (yUnit and [y/yUnit] or [0])[0],
                               x_pos, (yUnit and [item._y/yUnit] or [0])[0])
            
        elif item._y+self.GetLineHeight(item) > start_y+client_h:
            # going up, item should appear at bottom
            item._y += yUnit + 2
            self.SetScrollbars(xUnit, yUnit, (xUnit and [x/xUnit] or [0])[0], (yUnit and [y/yUnit] or [0])[0],
                               x_pos, (yUnit and [(item._y+self.GetLineHeight(item)-client_h)/yUnit] or [0])[0])
        

    def SetDragItem(self, item):
        """
        Sets the specified item as member of a current drag and drop operation.

        :param `item`: an instance of L{TreeListItem}.
        """

        prevItem = self._dragItem
        self._dragItem = item
        if prevItem:
            self.RefreshLine(prevItem)
        if self._dragItem:
            self.RefreshLine(self._dragItem)


# ----------------------------------------------------------------------------
# helpers
# ----------------------------------------------------------------------------

    def AdjustMyScrollbars(self):
        """ Internal method used to adjust the `wx.PyScrolledWindow` scrollbars. """

        if self._anchor:
            xUnit, yUnit = self.GetScrollPixelsPerUnit()
            if xUnit == 0:
                xUnit = self.GetCharWidth()
            if yUnit == 0:
                yUnit = self._lineHeight

            x, y = self._anchor.GetSize(0, 0, self)
            y += yUnit + 2 # one more scrollbar unit + 2 pixels
            x_pos = self.GetScrollPos(wx.HORIZONTAL)
            y_pos = self.GetScrollPos(wx.VERTICAL)
            x = self._owner.GetHeaderWindow().GetWidth() + 2
            if x < self.GetClientSize().GetWidth():
                x_pos = 0

            self.SetScrollbars(xUnit, yUnit, x/xUnit, y/yUnit, x_pos, y_pos)
        else:
            self.SetScrollbars(0, 0, 0, 0)
    

    def PaintItem(self, item, dc):
        """
        Actually draws an item.

        :param `item`: an instance of L{TreeListItem};
        :param `dc`: an instance of `wx.DC`.
        """

        def _paintText(text, textrect, alignment):
            """
            Sub-function to draw multi-lines text label aligned correctly.

            :param `text`: the item text label (possibly multiline);
            :param `textrect`: the label client rectangle;
            :param `alignment`: the alignment for the text label, one of ``wx.ALIGN_LEFT``,
             ``wx.ALIGN_RIGHT``, ``wx.ALIGN_CENTER``.
            """
            
            txt = text.splitlines()
            if alignment != wx.ALIGN_LEFT and len(txt):
                yorigin = textrect.Y
                for t in txt:
                    w, h = dc.GetTextExtent(t)
                    plus = textrect.Width - w
                    if alignment == wx.ALIGN_CENTER:
                        plus /= 2
                    dc.DrawLabel(t, wx.Rect(textrect.X + plus, yorigin, w, yorigin+h))
                    yorigin += h
                return
            dc.DrawLabel(text, textrect)
        
        attr = item.GetAttributes()
        
        if attr and attr.HasFont():
            dc.SetFont(attr.GetFont())
        elif item.IsBold():
            dc.SetFont(self._boldFont)
        if item.IsHyperText():
            dc.SetFont(self.GetHyperTextFont())
            if item.GetVisited():
                dc.SetTextForeground(self.GetHyperTextVisitedColour())
            else:
                dc.SetTextForeground(self.GetHyperTextNewColour())

        colText = wx.Colour(*dc.GetTextForeground())
        
        if item.IsSelected():
            if (wx.Platform == "__WXMAC__" and self._hasFocus):
                colTextHilight = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT)
            else:
                colTextHilight = wx.SystemSettings_GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT)

        else:
            attr = item.GetAttributes()
            if attr and attr.HasTextColour():
                colText = attr.GetTextColour()
            
        if self._vistaselection:
            colText = colTextHilight = wx.BLACK
                
        total_w = self._owner.GetHeaderWindow().GetWidth()
        total_h = self.GetLineHeight(item)
        off_h = (self.HasAGWFlag(wx.TR_ROW_LINES) and [1] or [0])[0]
        off_w = (self.HasAGWFlag(wx.TR_COLUMN_LINES) and [1] or [0])[0]
##        clipper = wx.DCClipper(dc, 0, item.GetY(), total_w, total_h) # only within line

        text_w, text_h, dummy = dc.GetMultiLineTextExtent(item.GetText(self.GetMainColumn()))

        drawItemBackground = False
        # determine background and show it
        if attr and attr.HasBackgroundColour():
            colBg = attr.GetBackgroundColour()
            drawItemBackground = True
        else:
            colBg = self._backgroundColour
        
        dc.SetBrush(wx.Brush(colBg, wx.SOLID))
        dc.SetPen(wx.TRANSPARENT_PEN)

        if self.HasAGWFlag(wx.TR_FULL_ROW_HIGHLIGHT):

            itemrect = wx.Rect(0, item.GetY() + off_h, total_w-1, total_h - off_h)
            
            if item == self._dragItem:
                dc.SetBrush(self._hilightBrush)
                if wx.Platform == "__WXMAC__":
                    dc.SetPen((item == self._dragItem) and [wx.BLACK_PEN] or [wx.TRANSPARENT_PEN])[0]

                dc.SetTextForeground(colTextHilight)

            elif item.IsSelected():

                wnd = item.GetWindow(self._main_column)
                wndx = 0
                if wnd:
                    wndx, wndy = item.GetWindowSize(self._main_column)

                itemrect = wx.Rect(0, item.GetY() + off_h, total_w-1, total_h - off_h)
                
                if self._usegradients:
                    if self._gradientstyle == 0:   # Horizontal
                        self.DrawHorizontalGradient(dc, itemrect, self._hasFocus)
                    else:                          # Vertical
                        self.DrawVerticalGradient(dc, itemrect, self._hasFocus)
                elif self._vistaselection:
                    self.DrawVistaRectangle(dc, itemrect, self._hasFocus)
                else:
                    if wx.Platform in ["__WXGTK2__", "__WXMAC__"]:
                        flags = wx.CONTROL_SELECTED
                        if self._hasFocus: flags = flags | wx.CONTROL_FOCUSED
                        wx.RendererNative.Get().DrawItemSelectionRect(self._owner, dc, itemrect, flags) 
                    else:
                        dc.SetBrush((self._hasFocus and [self._hilightBrush] or [self._hilightUnfocusedBrush])[0])
                        dc.SetPen((self._hasFocus and [self._borderPen] or [wx.TRANSPARENT_PEN])[0])
                        dc.DrawRectangleRect(itemrect)
                
                dc.SetTextForeground(colTextHilight)

            # On GTK+ 2, drawing a 'normal' background is wrong for themes that
            # don't allow backgrounds to be customized. Not drawing the background,
            # except for custom item backgrounds, works for both kinds of theme.
            elif drawItemBackground:

                itemrect = wx.Rect(0, item.GetY() + off_h, total_w-1, total_h - off_h)
                dc.SetBrush(wx.Brush(colBg, wx.SOLID))
                dc.DrawRectangleRect(itemrect)
                dc.SetTextForeground(colText)
                                                
            else:
                dc.SetTextForeground(colText)

        else:
            
            dc.SetTextForeground(colText)

        text_extraH = (total_h > text_h and [(total_h - text_h)/2] or [0])[0]
        img_extraH = (total_h > self._imgHeight and [(total_h-self._imgHeight)/2] or [0])[0]
        x_colstart = 0
        
        for i in xrange(self.GetColumnCount()):
            if not self._owner.GetHeaderWindow().IsColumnShown(i):
                continue

            col_w = self._owner.GetHeaderWindow().GetColumnWidth(i)
            dc.SetClippingRegion(x_colstart, item.GetY(), col_w, total_h) # only within column

            image = _NO_IMAGE
            x = image_w = wcheck = hcheck = 0

            if i == self.GetMainColumn():
                x = item.GetX() + _MARGIN
                if self.HasButtons():
                    x += (self._btnWidth-self._btnWidth2) + _LINEATROOT
                else:
                    x -= self._indent/2
                
                if self._imageListNormal:
                    image = item.GetCurrentImage(i)
                    
                if item.GetType() != 0 and self._imageListCheck:
                    checkimage = item.GetCurrentCheckedImage()
                    wcheck, hcheck = self._imageListCheck.GetSize(item.GetType())
                else:
                    wcheck, hcheck = 0, 0
            
            else:
                x = x_colstart + _MARGIN
                image = item.GetImage(column=i)
                
            if image != _NO_IMAGE:
                image_w = self._imgWidth + _MARGIN

            # honor text alignment
            text = item.GetText(i)
            alignment = self._owner.GetHeaderWindow().GetColumn(i).GetAlignment()

            text_w, dummy, dummy = dc.GetMultiLineTextExtent(text)

            if alignment == wx.ALIGN_RIGHT:
                w = col_w - (image_w + wcheck + text_w + off_w + _MARGIN + 1)
                x += (w > 0 and [w] or [0])[0]

            elif alignment == wx.ALIGN_CENTER:
                w = (col_w - (image_w + wcheck + text_w + off_w + _MARGIN))/2
                x += (w > 0 and [w] or [0])[0]
            else:
                if not item.HasPlus() and image_w == 0 and wcheck:
                    x += 3*_MARGIN
            
            text_x = x + image_w + wcheck + 1
            
            if i == self.GetMainColumn():
                item.SetTextX(text_x)

            if not self.HasAGWFlag(wx.TR_FULL_ROW_HIGHLIGHT):
                dc.SetBrush((self._hasFocus and [self._hilightBrush] or [self._hilightUnfocusedBrush])[0])
                dc.SetPen((self._hasFocus and [self._borderPen] or [wx.TRANSPARENT_PEN])[0])
                if i == self.GetMainColumn():
                    if item == self._dragItem:
                        if wx.Platform == "__WXMAC__":  # don't draw rect outline if we already have the background colour
                            dc.SetPen((item == self._dragItem and [wx.BLACK_PEN] or [wx.TRANSPARENT_PEN])[0])

                        dc.SetTextForeground(colTextHilight)
                        
                    elif item.IsSelected():

                        itemrect = wx.Rect(text_x-2, item.GetY() + off_h, text_w+2*_MARGIN, total_h - off_h)

                        if self._usegradients:
                            if self._gradientstyle == 0:   # Horizontal
                                self.DrawHorizontalGradient(dc, itemrect, self._hasFocus)
                            else:                          # Vertical
                                self.DrawVerticalGradient(dc, itemrect, self._hasFocus)
                        elif self._vistaselection:
                            self.DrawVistaRectangle(dc, itemrect, self._hasFocus)
                        else:
                            if wx.Platform in ["__WXGTK2__", "__WXMAC__"]:
                                flags = wx.CONTROL_SELECTED
                                if self._hasFocus: flags = flags | wx.CONTROL_FOCUSED
                                wx.RendererNative.Get().DrawItemSelectionRect(self._owner, dc, itemrect, flags) 
                            else:
                                dc.DrawRectangleRect(itemrect)

                        dc.SetTextForeground(colTextHilight)

                    elif item == self._current:
                        dc.SetPen((self._hasFocus and [wx.BLACK_PEN] or [wx.TRANSPARENT_PEN])[0])
                    
                    # On GTK+ 2, drawing a 'normal' background is wrong for themes that
                    # don't allow backgrounds to be customized. Not drawing the background,
                    # except for custom item backgrounds, works for both kinds of theme.
                    elif drawItemBackground:

                        itemrect = wx.Rect(text_x-2, item.GetY() + off_h, text_w+2*_MARGIN, total_h - off_h)
                        dc.SetBrush(wx.Brush(colBg, wx.SOLID))
                        dc.DrawRectangleRect(itemrect)

                    else:
                        dc.SetTextForeground(colText)
    
                else:
                    dc.SetTextForeground(colText)
                
            if self.HasAGWFlag(wx.TR_COLUMN_LINES):  # vertical lines between columns
                pen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DLIGHT), 1, wx.SOLID)
                dc.SetPen((self.GetBackgroundColour() == wx.WHITE and [pen] or [wx.WHITE_PEN])[0])
                dc.DrawLine(x_colstart+col_w-1, item.GetY(), x_colstart+col_w-1, item.GetY()+total_h)
            
            dc.SetBackgroundMode(wx.TRANSPARENT)

            if image != _NO_IMAGE:
                y = item.GetY() + img_extraH
                if wcheck:
                    x += wcheck

                if item.IsEnabled():
                    imglist = self._imageListNormal
                else:
                    imglist = self._grayedImageList
                
                imglist.Draw(image, dc, x, y, wx.IMAGELIST_DRAW_TRANSPARENT)

            if wcheck:
                if item.IsEnabled():
                    imglist = self._imageListCheck
                else:
                    imglist = self._grayedCheckList

                if self.HasButtons():  # should the item show a button?
                    btnWidth = self._btnWidth
                else:
                    btnWidth = -self._btnWidth
                
                imglist.Draw(checkimage, dc,
                             item.GetX() + btnWidth + _MARGIN,
                             item.GetY() + ((total_h > hcheck) and [(total_h-hcheck)/2] or [0])[0]+1,
                             wx.IMAGELIST_DRAW_TRANSPARENT)

            text_w, text_h, dummy = dc.GetMultiLineTextExtent(text)
            text_extraH = (total_h > text_h and [(total_h - text_h)/2] or [0])[0]            
            text_y = item.GetY() + text_extraH
            textrect = wx.Rect(text_x, text_y, text_w, text_h)
        
            if not item.IsEnabled():
                foreground = dc.GetTextForeground()
                dc.SetTextForeground(self._disabledColour)
                _paintText(text, textrect, alignment)
                dc.SetTextForeground(foreground)
            else:
                if wx.Platform == "__WXMAC__" and item.IsSelected() and self._hasFocus:
                    dc.SetTextForeground(wx.WHITE)
                _paintText(text, textrect, alignment)

            wnd = item.GetWindow(i)            
            if wnd:
                if text_w == 0:
                    wndx = text_x
                else:
                    wndx = text_x + text_w + 2*_MARGIN
                xa, ya = self.CalcScrolledPosition((0, item.GetY()))
                wndx += xa
                if item.GetHeight() > item.GetWindowSize(i)[1]:
                    ya += (item.GetHeight() - item.GetWindowSize(i)[1])/2
                    
                if not wnd.IsShown():
                    wnd.Show()
                if wnd.GetPosition() != (wndx, ya):
                    wnd.SetPosition((wndx, ya))                
            
            x_colstart += col_w
            dc.DestroyClippingRegion()
        
        # restore normal font
        dc.SetFont(self._normalFont)


    # Now y stands for the top of the item, whereas it used to stand for middle !
    def PaintLevel(self, item, dc, level, y, x_maincol):
        """
        Paint a level in the hierarchy of L{TreeListMainWindow}.

        :param `item`: an instance of L{TreeListItem};
        :param `dc`: an instance of `wx.DC`;
        :param `level`: the item level in the tree hierarchy;
        :param `y`: the current vertical position in the `wx.PyScrolledWindow`;
        :param `x_maincol`: the horizontal position of the main column.
        """

        if item.IsHidden():
            return y, x_maincol
        
        # Handle hide root (only level 0)
        if self.HasAGWFlag(wx.TR_HIDE_ROOT) and level == 0:
            for child in item.GetChildren():
                y, x_maincol = self.PaintLevel(child, dc, 1, y, x_maincol)
            
            # end after expanding root
            return y, x_maincol
        
        # calculate position of vertical lines
        x = x_maincol + _MARGIN # start of column

        if self.HasAGWFlag(wx.TR_LINES_AT_ROOT):
            x += _LINEATROOT # space for lines at root
            
        if self.HasButtons():
            x += (self._btnWidth-self._btnWidth2) # half button space
        else:
            x += (self._indent-self._indent/2)
        
        if self.HasAGWFlag(wx.TR_HIDE_ROOT):
            x += self._indent*(level-1) # indent but not level 1
        else:
            x += self._indent*level # indent according to level
        
        # set position of vertical line
        item.SetX(x)
        item.SetY(y)

        h = self.GetLineHeight(item)
        y_top = y
        y_mid = y_top + (h/2)
        y += h

        exposed_x = dc.LogicalToDeviceX(0)
        exposed_y = dc.LogicalToDeviceY(y_top)

        # horizontal lines between rows?
        draw_row_lines = self.HasAGWFlag(wx.TR_ROW_LINES)

        if self.IsExposed(exposed_x, exposed_y, _MAX_WIDTH, h + draw_row_lines):
            if draw_row_lines:
                total_width = self._owner.GetHeaderWindow().GetWidth()
                # if the background colour is white, choose a
                # contrasting colour for the lines
                pen = wx.Pen(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DLIGHT), 1, wx.SOLID)
                dc.SetPen((self.GetBackgroundColour() == wx.WHITE and [pen] or [wx.WHITE_PEN])[0])
                dc.DrawLine(0, y_top, total_width, y_top)
                dc.DrawLine(0, y_top+h, total_width, y_top+h)
            
            # draw item
            self.PaintItem(item, dc)

            # restore DC objects
            dc.SetBrush(wx.WHITE_BRUSH)
            dc.SetPen(self._dottedPen)

            # clip to the column width
            clip_width = self._owner.GetHeaderWindow().GetColumn(self._main_column).GetWidth()
##            clipper = wx.DCClipper(dc, x_maincol, y_top, clip_width, 10000)

            if not self.HasAGWFlag(wx.TR_NO_LINES):  # connection lines

                # draw the horizontal line here
                dc.SetPen(self._dottedPen)
                x2 = x - self._indent
                if x2 < (x_maincol + _MARGIN):
                    x2 = x_maincol + _MARGIN
                x3 = x + (self._btnWidth-self._btnWidth2)
                if self.HasButtons():
                    if item.HasPlus():
                        dc.DrawLine(x2, y_mid, x - self._btnWidth2, y_mid)
                        dc.DrawLine(x3, y_mid, x3 + _LINEATROOT, y_mid)
                    else:
                        dc.DrawLine(x2, y_mid, x3 + _LINEATROOT, y_mid)
                else:
                    dc.DrawLine(x2, y_mid, x - self._indent/2, y_mid)
                
            if item.HasPlus() and self.HasButtons():  # should the item show a button?
                
                if self._imageListButtons:

                    # draw the image button here
                    image = wx.TreeItemIcon_Normal
                    if item.IsExpanded():
                        image = wx.TreeItemIcon_Expanded
                    if item.IsSelected():
                        image += wx.TreeItemIcon_Selected - wx.TreeItemIcon_Normal
                    xx = x - self._btnWidth2 + _MARGIN
                    yy = y_mid - self._btnHeight2
                    dc.SetClippingRegion(xx, yy, self._btnWidth, self._btnHeight)
                    self._imageListButtons.Draw(image, dc, xx, yy, wx.IMAGELIST_DRAW_TRANSPARENT)
                    dc.DestroyClippingRegion()

                elif self.HasAGWFlag(wx.TR_TWIST_BUTTONS):

                    # draw the twisty button here
                    dc.SetPen(wx.BLACK_PEN)
                    dc.SetBrush(self._hilightBrush)
                    button = [wx.Point() for j in xrange(3)]
                    if item.IsExpanded():
                        button[0].x = x - (self._btnWidth2+1)
                        button[0].y = y_mid - (self._btnHeight/3)
                        button[1].x = x + (self._btnWidth2+1)
                        button[1].y = button[0].y
                        button[2].x = x
                        button[2].y = button[0].y + (self._btnHeight2+1)
                    else:
                        button[0].x = x - (self._btnWidth/3)
                        button[0].y = y_mid - (self._btnHeight2+1)
                        button[1].x = button[0].x
                        button[1].y = y_mid + (self._btnHeight2+1)
                        button[2].x = button[0].x + (self._btnWidth2+1)
                        button[2].y = y_mid
                    
                    dc.SetClippingRegion(x_maincol + _MARGIN, y_top, clip_width, h)
                    dc.DrawPolygon(button)
                    dc.DestroyClippingRegion()

                else: # if (HasAGWFlag(wxTR_HAS_BUTTONS))

                    rect = wx.Rect(x-self._btnWidth2, y_mid-self._btnHeight2, self._btnWidth, self._btnHeight)
                    flag = (item.IsExpanded() and [wx.CONTROL_EXPANDED] or [0])[0]
                    wx.RendererNative.GetDefault().DrawTreeItemButton(self, dc, rect, flag)        

        # restore DC objects
        dc.SetBrush(wx.WHITE_BRUSH)
        dc.SetPen(self._dottedPen)
        dc.SetTextForeground(wx.BLACK)

        if item.IsExpanded():

            # process lower levels
            if self._imgWidth > 0:
                oldY = y_mid + self._imgHeight2
            else:
                oldY = y_mid + h/2
            
            for child in item.GetChildren():

                y, x_maincol = self.PaintLevel(child, dc, level+1, y, x_maincol)

                # draw vertical line
                if not self.HasAGWFlag(wx.TR_NO_LINES):
                    Y1 = child.GetY() + child.GetHeight()/2
                    dc.DrawLine(x, oldY, x, Y1)

        return y, x_maincol        


# ----------------------------------------------------------------------------
# wxWindows callbacks
# ----------------------------------------------------------------------------

    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{TreeListMainWindow}.

        :param `event`: a `wx.EraseEvent` event to be processed.
        """

        # do not paint the background separately in buffered mode.
        if not self._buffered:
            CustomTreeCtrl.OnEraseBackground(self, event)


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{TreeListMainWindow}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """

        if self._buffered:

            # paint the background
            dc = wx.BufferedPaintDC(self)
            rect = self.GetUpdateRegion().GetBox()
            dc.SetClippingRect(rect)
            dc.SetBackground(wx.Brush(self.GetBackgroundColour()))
            if self._backgroundImage:
                self.TileBackground(dc)
            else:
                dc.Clear()

        else:
            dc = wx.PaintDC(self)

        self.PrepareDC(dc)

        if not self._anchor or self.GetColumnCount() <= 0:
            return

        # calculate button size
        if self._imageListButtons:
            self._btnWidth, self._btnHeight = self._imageListButtons.GetSize(0)
        elif self.HasButtons():
            self._btnWidth = _BTNWIDTH
            self._btnHeight = _BTNHEIGHT
        
        self._btnWidth2 = self._btnWidth/2
        self._btnHeight2 = self._btnHeight/2

        # calculate image size
        if self._imageListNormal:
            self._imgWidth, self._imgHeight = self._imageListNormal.GetSize(0)
        
        self._imgWidth2 = self._imgWidth/2
        self._imgHeight2 = self._imgHeight/2

        if self._imageListCheck:
            self._checkWidth, self._checkHeight = self._imageListCheck.GetSize(0)

        self._checkWidth2 = self._checkWidth/2
        self._checkHeight2 = self._checkHeight/2
            
        # calculate indent size
        if self._imageListButtons:
            self._indent = max(_MININDENT, self._btnWidth + _MARGIN)
        elif self.HasButtons():
            self._indent = max(_MININDENT, self._btnWidth + _LINEATROOT)
        
        # set default values
        dc.SetFont(self._normalFont)
        dc.SetPen(self._dottedPen)

        # calculate column start and paint
        x_maincol = 0
        for i in xrange(self.GetMainColumn()):
            if not self._owner.GetHeaderWindow().IsColumnShown(i):
                continue
            x_maincol += self._owner.GetHeaderWindow().GetColumnWidth(i)
        
        y, x_maincol = self.PaintLevel(self._anchor, dc, 0, 0, x_maincol)


    def HitTest(self, point, flags=0):
        """
        Calculates which (if any) item is under the given point, returning the tree item
        at this point plus extra information flags plus the item's column.

        :param `point`: an instance of `wx.Point`, a point to test for hits;
        :param `flags`: a bitlist of the following values:

         ================================== =============== =================================
         HitTest Flags                      Hex Value       Description
         ================================== =============== =================================
         ``TREE_HITTEST_ABOVE``                         0x1 Above the client area
         ``TREE_HITTEST_BELOW``                         0x2 Below the client area
         ``TREE_HITTEST_NOWHERE``                       0x4 No item has been hit
         ``TREE_HITTEST_ONITEMBUTTON``                  0x8 On the button associated to an item
         ``TREE_HITTEST_ONITEMICON``                   0x10 On the icon associated to an item
         ``TREE_HITTEST_ONITEMINDENT``                 0x20 On the indent associated to an item
         ``TREE_HITTEST_ONITEMLABEL``                  0x40 On the label (string) associated to an item
         ``TREE_HITTEST_ONITEM``                       0x50 Anywhere on the item
         ``TREE_HITTEST_ONITEMRIGHT``                  0x80 On the right of the label associated to an item
         ``TREE_HITTEST_TOLEFT``                      0x200 On the left of the client area
         ``TREE_HITTEST_TORIGHT``                     0x400 On the right of the client area
         ``TREE_HITTEST_ONITEMUPPERPART``             0x800 On the upper part (first half) of the item
         ``TREE_HITTEST_ONITEMLOWERPART``            0x1000 On the lower part (second half) of the item
         ``TREE_HITTEST_ONITEMCHECKICON``            0x2000 On the check/radio icon, if present
         ================================== =============== =================================

        :return: the item (if any, ``None`` otherwise), the `flags` and the column are always
         returned as a tuple.
        """

        w, h = self.GetSize()
        column = -1

        if not isinstance(point, wx.Point):
            point = wx.Point(*point)

        if point.x < 0:
            flags |= wx.TREE_HITTEST_TOLEFT
        if point.x > w:
            flags |= wx.TREE_HITTEST_TORIGHT
        if point.y < 0:
            flags |= wx.TREE_HITTEST_ABOVE
        if point.y > h:
            flags |= wx.TREE_HITTEST_BELOW
        if flags:
            return None, flags, column

        if not self._anchor:
            flags = wx.TREE_HITTEST_NOWHERE
            column = -1
            return None, flags, column
        
        hit, flags, column = self._anchor.HitTest(self.CalcUnscrolledPosition(point), self, flags, column, 0)
        if not hit:
            flags = wx.TREE_HITTEST_NOWHERE
            column = -1
            return None, flags, column
        
        return hit, flags, column


    def EditLabel(self, item, column=None):
        """
        Starts editing an item label.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.        
        """

        if not item:
            return

        column = (column is not None and [column] or [self._main_column])[0]
        
        if column < 0 or column >= self.GetColumnCount():
            return

        self._curColumn = column
        self._editItem = item

        te = TreeEvent(wx.wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, self._owner.GetId())
        te.SetItem(self._editItem)
        te.SetInt(column)
        te.SetEventObject(self._owner)
        self._owner.GetEventHandler().ProcessEvent(te)

        if not te.IsAllowed():
            return

        # ensure that the position of the item it calculated in any case
        if self._dirty:
            self.CalculatePositions()
            
        if self._editCtrl != None and (item != self._editCtrl.item() or column != self._editCtrl.column()):
            self._editCtrl.StopEditing()
            
        self._editCtrl = self._owner.CreateEditCtrl(item, column) 
        self._editCtrl.SetFocus()
        

    def OnEditTimer(self):
        """ The timer for editing has expired. Start editing. """

        self.EditLabel(self._current, self._curColumn)


    def OnAcceptEdit(self, value):
        """
        Called by L{EditTextCtrl}, to accept the changes and to send the
        ``EVT_TREE_END_LABEL_EDIT`` event.

        :param `value`: the new value of the item label.        
        """

        # TODO if the validator fails this causes a crash
        le = TreeEvent(wx.wxEVT_COMMAND_TREE_END_LABEL_EDIT, self._owner.GetId())
        le.SetItem(self._editItem)
        le.SetEventObject(self._owner)
        le.SetLabel(value)
        le.SetInt(self._curColumn if self._curColumn >= 0 else 0)
        le._editCancelled = False
        self._owner.GetEventHandler().ProcessEvent(le)

        if not le.IsAllowed():
            return

        if self._curColumn == -1:
            self._curColumn = 0
           
        self.SetItemText(self._editItem, unicode(value), self._curColumn)


    def OnCancelEdit(self):
        """
        Called by L{EditCtrl}, to cancel the changes and to send the
        ``EVT_TREE_END_LABEL_EDIT`` event.
        """

        # let owner know that the edit was cancelled
        le = TreeEvent(wx.wxEVT_COMMAND_TREE_END_LABEL_EDIT, self._owner.GetId())
        le.SetItem(self._editItem)
        le.SetEventObject(self._owner)
        le.SetLabel("")
        le._editCancelled = True

        self._owner.GetEventHandler().ProcessEvent(le)

    
    def OnMouse(self, event):
        """
        Handles the ``wx.EVT_MOUSE_EVENTS`` event for L{TreeListMainWindow}.

        :param `event`: a `wx.MouseEvent` event to be processed.
        """

        if not self._anchor:
            return

        # we process left mouse up event (enables in-place edit), right down
        # (pass to the user code), left dbl click (activate item) and
        # dragging/moving events for items drag-and-drop
        if not (event.LeftDown() or event.LeftUp() or event.RightDown() or \
                event.RightUp() or event.LeftDClick() or event.Dragging() or \
                event.GetWheelRotation() != 0 or event.Moving()):
            self._owner.GetEventHandler().ProcessEvent(event)
            return
        

        # set focus if window clicked
        if event.LeftDown() or event.RightDown():
            self._hasFocus = True
            self.SetFocusIgnoringChildren()

        # determine event
        p = wx.Point(event.GetX(), event.GetY())
        flags = 0
        item, flags, column = self._anchor.HitTest(self.CalcUnscrolledPosition(p), self, flags, self._curColumn, 0)

        underMouse = item
        underMouseChanged = underMouse != self._underMouse

        if underMouse and (flags & wx.TREE_HITTEST_ONITEM) and not event.LeftIsDown() and \
           not self._isDragging and (not self._editTimer or not self._editTimer.IsRunning()):
            underMouse = underMouse
        else:
            underMouse = None

        if underMouse != self._underMouse:
            if self._underMouse:
                # unhighlight old item
                self._underMouse = None
             
            self._underMouse = underMouse

        # Determines what item we are hovering over and need a tooltip for
        hoverItem = item

        if (event.LeftDown() or event.LeftUp() or event.RightDown() or \
            event.RightUp() or event.LeftDClick() or event.Dragging()):
            if self._editCtrl != None and (item != self._editCtrl.item() or column != self._editCtrl.column()):
                self._editCtrl.StopEditing()

        # We do not want a tooltip if we are dragging, or if the edit timer is running
        if underMouseChanged and not self._isDragging and (not self._editTimer or not self._editTimer.IsRunning()):
            
            if hoverItem is not None:
                # Ask the tree control what tooltip (if any) should be shown
                hevent = TreeEvent(wx.wxEVT_COMMAND_TREE_ITEM_GETTOOLTIP, self.GetId())
                hevent.SetItem(hoverItem)
                hevent.SetEventObject(self)

                if self.GetEventHandler().ProcessEvent(hevent) and hevent.IsAllowed():
                    self.SetToolTip(hevent._label)

                if hoverItem.IsHyperText() and (flags & wx.TREE_HITTEST_ONITEMLABEL) and hoverItem.IsEnabled():
                    self.SetCursor(wx.StockCursor(wx.CURSOR_HAND))
                    self._isonhyperlink = True
                else:
                    if self._isonhyperlink:
                        self.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
                        self._isonhyperlink = False
                        
        # we only process dragging here
        if event.Dragging():
            
            if self._isDragging:
                if not self._dragImage:
                    # Create the custom draw image from the icons and the text of the item                    
                    self._dragImage = DragImage(self, self._current or item)
                    self._dragImage.BeginDrag(wx.Point(0,0), self)
                    self._dragImage.Show()

                self._dragImage.Move(p)

                if self._countDrag == 0 and item:
                    self._oldItem = self._current
                    self._oldSelection = self._current

                if item != self._dropTarget:
                        
                    # unhighlight the previous drop target
                    if self._dropTarget:
                        self._dropTarget.SetHilight(False)
                        self.RefreshLine(self._dropTarget)
                    if item:
                        item.SetHilight(True)
                        self.RefreshLine(item)
                        self._countDrag = self._countDrag + 1
                    self._dropTarget = item

                    self.Update()

                if self._countDrag >= 3 and self._oldItem is not None:
                    # Here I am trying to avoid ugly repainting problems... hope it works
                    self.RefreshLine(self._oldItem)
                    self._countDrag = 0
                    
                return # nothing to do, already done

            if item == None:
                return # we need an item to dragging

            # determine drag start
            if self._dragCount == 0:
                self._dragTimer.Start(_DRAG_TIMER_TICKS, wx.TIMER_ONE_SHOT)
            
            self._dragCount += 1
            if self._dragCount < 3:
                return # minimum drag 3 pixel
            if self._dragTimer.IsRunning():
                return

            # we're going to drag
            self._dragCount = 0

            # send drag start event
            command = (event.LeftIsDown() and [wx.wxEVT_COMMAND_TREE_BEGIN_DRAG] or [wx.wxEVT_COMMAND_TREE_BEGIN_RDRAG])[0]
            nevent = TreeEvent(command, self._owner.GetId())
            nevent.SetEventObject(self._owner)
            nevent.SetItem(self._current) # the dragged item
            nevent.SetPoint(p)
            nevent.Veto()         # dragging must be explicit allowed!
            
            if self.GetEventHandler().ProcessEvent(nevent) and nevent.IsAllowed():
                
                # we're going to drag this item
                self._isDragging = True
                self.CaptureMouse()
                self.RefreshSelected()

                # in a single selection control, hide the selection temporarily
                if not (self._agwStyle & wx.TR_MULTIPLE):
                    if self._oldSelection:
                    
                        self._oldSelection.SetHilight(False)
                        self.RefreshLine(self._oldSelection)
                else:
                    selections = self.GetSelections()
                    if len(selections) == 1:
                        self._oldSelection = selections[0]
                        self._oldSelection.SetHilight(False)
                        self.RefreshLine(self._oldSelection)

        elif self._isDragging:  # any other event but not event.Dragging()

            # end dragging
            self._dragCount = 0
            self._isDragging = False
            if self.HasCapture():
                self.ReleaseMouse()
            self.RefreshSelected()

            # send drag end event event
            nevent = TreeEvent(wx.wxEVT_COMMAND_TREE_END_DRAG, self._owner.GetId())
            nevent.SetEventObject(self._owner)
            nevent.SetItem(item) # the item the drag is started
            nevent.SetPoint(p)
            self._owner.GetEventHandler().ProcessEvent(nevent)
            
            if self._dragImage:
                self._dragImage.EndDrag()

            if self._dropTarget:
                self._dropTarget.SetHilight(False)
                self.RefreshLine(self._dropTarget)
                
            if self._oldSelection:
                self._oldSelection.SetHilight(True)
                self.RefreshLine(self._oldSelection)
                self._oldSelection = None

            self._isDragging = False
            self._dropTarget = None
            if self._dragImage:
                self._dragImage = None
            
            self.Refresh()

        elif self._dragCount > 0:  # just in case dragging is initiated

            # end dragging
            self._dragCount = 0

        # we process only the messages which happen on tree items
        if (item == None or not self.IsItemEnabled(item)) and not event.GetWheelRotation():
            self._owner.GetEventHandler().ProcessEvent(event)
            return
        
        # remember item at shift down
        if event.ShiftDown():
            if not self._shiftItem:
                self._shiftItem = self._current
        else:
            self._shiftItem = None
        
        if event.RightUp():

            self.SetFocus()
            nevent = TreeEvent(wx.wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK, self._owner.GetId())
            nevent.SetEventObject(self._owner)
            nevent.SetItem(item) # the item clicked
            nevent.SetInt(self._curColumn) # the column clicked
            nevent.SetPoint(p)
            self._owner.GetEventHandler().ProcessEvent(nevent)

        elif event.LeftUp():

            if self._lastOnSame:
                if item == self._current and self._curColumn != -1 and \
                   self._owner.GetHeaderWindow().IsColumnEditable(self._curColumn) and \
                   flags & (wx.TREE_HITTEST_ONITEMLABEL | wx.TREE_HITTEST_ONITEMCOLUMN):
                    self._editTimer.Start(_EDIT_TIMER_TICKS, wx.TIMER_ONE_SHOT)
                
                self._lastOnSame = False
            
            if (((flags & wx.TREE_HITTEST_ONITEMBUTTON) or (flags & wx.TREE_HITTEST_ONITEMICON)) and \
                self.HasButtons() and item.HasPlus()):

                # only toggle the item for a single click, double click on
                # the button doesn't do anything (it toggles the item twice)
                if event.LeftDown():
                    self.Toggle(item)

                # don't select the item if the button was clicked
                return         
            
            # determine the selection if not done by left down
            if not self._left_down_selection:
                unselect_others = not ((event.ShiftDown() or event.CmdDown()) and self.HasAGWFlag(wx.TR_MULTIPLE))
                self.DoSelectItem(item, unselect_others, event.ShiftDown())
                self.EnsureVisible (item)
                self._current = self._key_current = item # make the new item the current item
            else:
                self._left_down_selection = False
            
        elif event.LeftDown() or event.RightDown() or event.LeftDClick():

            if column >= 0:
                self._curColumn = column
            
            if event.LeftDown() or event.RightDown():
                self.SetFocus()
                self._lastOnSame = item == self._current
            
            if (((flags & wx.TREE_HITTEST_ONITEMBUTTON) or (flags & wx.TREE_HITTEST_ONITEMICON)) and \
                self.HasButtons() and item.HasPlus()):

                # only toggle the item for a single click, double click on
                # the button doesn't do anything (it toggles the item twice)
                if event.LeftDown():
                    self.Toggle(item)

                # don't select the item if the button was clicked
                return

            if flags & TREE_HITTEST_ONITEMCHECKICON and event.LeftDown():
                if item.GetType() > 0:
                    if self.IsItem3State(item):
                        checked = self.GetItem3StateValue(item)
                        checked = (checked+1)%3
                    else:
                        checked = not self.IsItemChecked(item)

                    self.CheckItem(item, checked)
                    return
                
            # determine the selection if the current item is not selected
            if not item.IsSelected():
                unselect_others = not ((event.ShiftDown() or event.CmdDown()) and self.HasAGWFlag(wx.TR_MULTIPLE))
                self.DoSelectItem(item, unselect_others, event.ShiftDown())
                self.EnsureVisible(item)
                self._current = self._key_current = item # make the new item the current item
                self._left_down_selection = True
            
            # For some reason, Windows isn't recognizing a left double-click,
            # so we need to simulate it here.  Allow 200 milliseconds for now.
            if event.LeftDClick():

                # double clicking should not start editing the item label
                self._editTimer.Stop()
                self._lastOnSame = False

                # send activate event first
                nevent = TreeEvent(wx.wxEVT_COMMAND_TREE_ITEM_ACTIVATED, self._owner.GetId())
                nevent.SetEventObject(self._owner)
                nevent.SetItem(item) # the item clicked
                nevent.SetInt(self._curColumn) # the column clicked
                nevent.SetPoint(p)
                if not self._owner.GetEventHandler().ProcessEvent(nevent):

                    # if the user code didn't process the activate event,
                    # handle it ourselves by toggling the item when it is
                    # double clicked
                    if item.HasPlus():
                        self.Toggle(item)
                
        else: # any other event skip just in case

            event.Skip()

        
    def OnScroll(self, event):
        """
        Handles the ``wx.EVT_SCROLLWIN`` event for L{TreeListMainWindow}.

        :param `event`: a `wx.ScrollEvent` event to be processed.
        """

        def _updateHeaderWindow(header):
            header.Refresh()
            header.Update()
            
        # Update the header window after this scroll event has fully finished
        # processing, and the scoll action is complete.
        if event.GetOrientation() == wx.HORIZONTAL:
            wx.CallAfter(_updateHeaderWindow, self._owner.GetHeaderWindow())
        event.Skip()
            

    def CalculateSize(self, item, dc):
        """
        Calculates overall position and size of an item.

        :param `item`: an instance of L{TreeListItem};
        :param `dc`: an instance of `wx.DC`.
        """

        attr = item.GetAttributes()

        if attr and attr.HasFont():
            dc.SetFont(attr.GetFont())
        elif item.IsBold():
            dc.SetFont(self._boldFont)
        else:
            dc.SetFont(self._normalFont)

        text_w = text_h = wnd_w = wnd_h = 0
        for column in xrange(self.GetColumnCount()):
            w, h, dummy = dc.GetMultiLineTextExtent(item.GetText(column))
            text_w, text_h = max(w, text_w), max(h, text_h)
            
            wnd = item.GetWindow(column)
            if wnd:
                wnd_h = max(wnd_h, item.GetWindowSize(column)[1])
                if column == self._main_column:
                    wnd_w = item.GetWindowSize(column)[0]

        text_w, dummy, dummy = dc.GetMultiLineTextExtent(item.GetText(self._main_column))
        text_h+=2

        # restore normal font
        dc.SetFont(self._normalFont)

        image_w, image_h = 0, 0
        image = item.GetCurrentImage()

        if image != _NO_IMAGE:
        
            if self._imageListNormal:
            
                image_w, image_h = self._imageListNormal.GetSize(image)
                image_w += 2*_MARGIN

        total_h = ((image_h > text_h) and [image_h] or [text_h])[0]

        checkimage = item.GetCurrentCheckedImage()
        if checkimage is not None:
            wcheck, hcheck = self._imageListCheck.GetSize(checkimage)
            wcheck += 2*_MARGIN
        else:
            wcheck = 0

        if total_h < 30:
            total_h += 2            # at least 2 pixels
        else:
            total_h += total_h/10   # otherwise 10% extra spacing

        if total_h > self._lineHeight:
            self._lineHeight = max(total_h, wnd_h+2)

        item.SetWidth(image_w+text_w+wcheck+2+wnd_w)
        item.SetHeight(max(total_h, wnd_h+2))

        
    def CalculateLevel(self, item, dc, level, y, x_colstart):
        """
        Calculates the level of an item inside the tree hierarchy.

        :param `item`: an instance of L{TreeListItem};
        :param `dc`: an instance of `wx.DC`;
        :param `level`: the item level in the tree hierarchy;
        :param `y`: the current vertical position inside the `wx.PyScrolledWindow`;
        :param `x_colstart`: the x coordinate at which the item's column starts.
        """

        # calculate position of vertical lines
        x = x_colstart + _MARGIN # start of column
        if self.HasAGWFlag(wx.TR_LINES_AT_ROOT):
            x += _LINEATROOT # space for lines at root
        if self.HasButtons():
            x += (self._btnWidth-self._btnWidth2) # half button space
        else:
            x += (self._indent-self._indent/2)
        
        if self.HasAGWFlag(wx.TR_HIDE_ROOT):
            x += self._indent * (level-1) # indent but not level 1
        else:
            x += self._indent * level # indent according to level
        
        # a hidden root is not evaluated, but its children are always
        if self.HasAGWFlag(wx.TR_HIDE_ROOT) and (level == 0):
            # a hidden root is not evaluated, but its
            # children are always calculated
            children = item.GetChildren()
            count = len(children)
            level = level + 1
            for n in xrange(count):
                y = self.CalculateLevel(children[n], dc, level, y, x_colstart)  # recurse
                
            return y

        self.CalculateSize(item, dc)

        # set its position
        item.SetX(x)
        item.SetY(y)
        y += self.GetLineHeight(item)

        if not item.IsExpanded():
            # we don't need to calculate collapsed branches
            return y

        children = item.GetChildren()
        count = len(children)
        level = level + 1
        for n in xrange(count):
            y = self.CalculateLevel(children[n], dc, level, y, x_colstart)  # recurse
        
        return y
    

    def CalculatePositions(self):
        """ Recalculates all the items positions. """
        
        if not self._anchor:
            return

        dc = wx.ClientDC(self)
        self.PrepareDC(dc)

        dc.SetFont(self._normalFont)
        dc.SetPen(self._dottedPen)

        y, x_colstart = 2, 0
        for i in xrange(self.GetMainColumn()):
            if not self._owner.GetHeaderWindow().IsColumnShown(i):
                continue
            x_colstart += self._owner.GetHeaderWindow().GetColumnWidth(i)
        
        self.CalculateLevel(self._anchor, dc, 0, y, x_colstart) # start recursion


    def SetItemText(self, item, text, column=None):
        """
        Sets the item text label.

        :param `item`: an instance of L{TreeListItem};
        :param `text`: a string specifying the new item label;
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        dc = wx.ClientDC(self)
        item.SetText(column, text)
        self.CalculateSize(item, dc)
        self.RefreshLine(item)


    def GetItemText(self, item, column=None):
        """
        Returns the item text label.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: if not ``None``, an integer specifying the column index.
         If it is ``None``, the main column index is used.
        """

        if self.IsVirtual():
            return self._owner.OnGetItemText(item, column)
        else:
            return item.GetText(column)
   

    def GetItemWidth(self, item, column):
        """
        Returns the item width.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: an integer specifying the column index.
        """
        
        if not item:
            return 0

        # determine item width
        font = self.GetItemFont(item)
        if not font.IsOk():
            if item.IsBold():
                font = self._boldFont
            elif item.IsItalic():
                font = self._italicFont
            elif item.IsHyperText():
                font = self.GetHyperTextFont()
            else:
                font = self._normalFont
            
        dc = wx.ClientDC(self)
        dc.SetFont(font)
        w, h, dummy = dc.GetMultiLineTextExtent(item.GetText(column))
        w += 2*_MARGIN

        # calculate width
        width = w + 2*_MARGIN
        if column == self.GetMainColumn():
            width += _MARGIN
            if self.HasAGWFlag(wx.TR_LINES_AT_ROOT):
                width += _LINEATROOT
            if self.HasButtons():
                width += self._btnWidth + _LINEATROOT
            if item.GetCurrentImage() != _NO_IMAGE:
                width += self._imgWidth

            # count indent level
            level = 0
            parent = item.GetParent()
            root = self.GetRootItem()
            while (parent and (not self.HasAGWFlag(wx.TR_HIDE_ROOT) or (parent != root))):
                level += 1
                parent = parent.GetParent()
            
            if level:
                width += level*self.GetIndent()

        wnd = item.GetWindow(column)
        if wnd:
            width += wnd.GetSize()[0] + 2*_MARGIN
            
        return width


    def GetBestColumnWidth(self, column, parent=None):
        """
        Returns the best column's width based on the items width in this column.

        :param `column`: an integer specifying the column index;
        :param `parent`: an instance of L{TreeListItem}.
        """

        maxWidth, h = self.GetClientSize()
        width = 0

        # get root if on item
        if not parent:
            parent = self.GetRootItem()

        # add root width
        if not self.HasAGWFlag(wx.TR_HIDE_ROOT):
            w = self.GetItemWidth(parent, column)
            if width < w:
                width = w
            if width > maxWidth:
                return maxWidth

        item, cookie = self.GetFirstChild(parent)
        while item:
            w = self.GetItemWidth(item, column)
            if width < w:
                width = w
            if width > maxWidth:
                return maxWidth

            # check the children of this item
            if item.IsExpanded():
                w = self.GetBestColumnWidth(column, item)
                if width < w:
                    width = w
                if width > maxWidth:
                    return maxWidth

            # next sibling
            item, cookie = self.GetNextChild(parent, cookie)
        
        return max(10, width) # Prevent zero column width


    def HideItem(self, item, hide=True):
        """
        Hides/shows an item.

        :param `item`: an instance of L{TreeListItem};
        :param `hide`: ``True`` to hide the item, ``False`` to show it.
        """

        item.Hide(hide)
        self.Refresh()
        

#----------------------------------------------------------------------------
# TreeListCtrl - the multicolumn tree control
#----------------------------------------------------------------------------

_methods = ["GetIndent", "SetIndent", "GetSpacing", "SetSpacing", "GetImageList", "GetStateImageList",
            "GetButtonsImageList", "AssignImageList", "AssignStateImageList", "AssignButtonsImageList",
            "SetImageList", "SetButtonsImageList", "SetStateImageList",
            "GetItemText", "GetItemImage", "GetItemPyData", "GetPyData", "GetItemTextColour",
            "GetItemBackgroundColour", "GetItemFont", "SetItemText", "SetItemImage", "SetItemPyData", "SetPyData",
            "SetItemHasChildren", "SetItemBackgroundColour", "SetItemFont", "IsItemVisible", "HasChildren",
            "IsExpanded", "IsSelected", "IsBold", "GetChildrenCount", "GetRootItem", "GetSelection", "GetSelections",
            "GetItemParent", "GetFirstChild", "GetNextChild", "GetPrevChild", "GetLastChild", "GetNextSibling",
            "GetPrevSibling", "GetNext", "GetFirstExpandedItem", "GetNextExpanded", "GetPrevExpanded",
            "GetFirstVisibleItem", "GetNextVisible", "GetPrevVisible", "AddRoot", "PrependItem", "InsertItem",
            "AppendItem", "Delete", "DeleteChildren", "DeleteRoot", "Expand", "ExpandAll", "ExpandAllChildren",
            "Collapse", "CollapseAndReset", "Toggle", "Unselect", "UnselectAll", "SelectItem", "SelectAll",
            "EnsureVisible", "ScrollTo", "HitTest", "GetBoundingRect", "EditLabel", "FindItem", "SelectAllChildren",
            "SetDragItem", "GetColumnCount", "SetMainColumn", "GetHyperTextFont", "SetHyperTextFont",
            "SetHyperTextVisitedColour", "GetHyperTextVisitedColour", "SetHyperTextNewColour", "GetHyperTextNewColour",
            "SetItemVisited", "GetItemVisited", "SetHilightFocusColour", "GetHilightFocusColour", "SetHilightNonFocusColour",
            "GetHilightNonFocusColour", "SetFirstGradientColour", "GetFirstGradientColour", "SetSecondGradientColour",
            "GetSecondGradientColour", "EnableSelectionGradient", "SetGradientStyle", "GetGradientStyle",
            "EnableSelectionVista", "SetBorderPen", "GetBorderPen", "SetConnectionPen", "GetConnectionPen",
            "SetBackgroundImage", "GetBackgroundImage", "SetImageListCheck", "GetImageListCheck", "EnableChildren",
            "EnableItem", "IsItemEnabled", "GetDisabledColour", "SetDisabledColour", "IsItemChecked",
            "UnCheckRadioParent", "CheckItem", "CheckItem2", "AutoToggleChild", "AutoCheckChild", "AutoCheckParent",
            "CheckChilds", "CheckSameLevel", "GetItemWindowEnabled", "SetItemWindowEnabled", "GetItemType",
            "IsDescendantOf", "SetItemHyperText", "IsItemHyperText", "SetItemBold", "SetItemDropHighlight", "SetItemItalic",
            "GetEditControl", "ShouldInheritColours", "GetItemWindow", "SetItemWindow", "SetItemTextColour", "HideItem",
            "DeleteAllItems", "ItemHasChildren", "ToggleItemSelection", "SetItemType", "GetCurrentItem",
            "SetItem3State", "SetItem3StateValue", "GetItem3StateValue", "IsItem3State"]


class HyperTreeList(wx.PyControl):
    """
    HyperTreeList is a class that mimics the behaviour of `wx.gizmos.TreeListCtrl`, with
    almost the same base functionalities plus some more enhancements. This class does
    not rely on the native control, as it is a full owner-drawn tree-list control.
    """
    
    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, agwStyle=wx.TR_DEFAULT_STYLE, validator=wx.DefaultValidator,
                 name="HyperTreeList"):
        """
        Default class constructor.
        
        :param `parent`: parent window. Must not be ``None``;
        :param `id`: window identifier. A value of -1 indicates a default value;
        :param `pos`: the control position. A value of (-1, -1) indicates a default position,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `size`: the control size. A value of (-1, -1) indicates a default size,
         chosen by either the windowing system or wxPython, depending on platform;
        :param `style`: the underlying `wx.PyScrolledWindow` style;
        :param `agwStyle`: the AGW-specific L{HyperTreeList} window style. This can be a combination
         of the following bits:
        
         ============================== =========== ==================================================
         Window Styles                  Hex Value   Description
         ============================== =========== ==================================================
         ``TR_NO_BUTTONS``                      0x0 For convenience to document that no buttons are to be drawn.
         ``TR_SINGLE``                          0x0 For convenience to document that only one item may be selected at a time. Selecting another item causes the current selection, if any, to be deselected. This is the default.
         ``TR_HAS_BUTTONS``                     0x1 Use this style to show + and - buttons to the left of parent items.
         ``TR_NO_LINES``                        0x4 Use this style to hide vertical level connectors.
         ``TR_LINES_AT_ROOT``                   0x8 Use this style to show lines between root nodes. Only applicable if ``TR_HIDE_ROOT`` is set and ``TR_NO_LINES`` is not set.
         ``TR_DEFAULT_STYLE``                   0x9 No Docs
         ``TR_TWIST_BUTTONS``                  0x10 Use old Mac-twist style buttons.
         ``TR_MULTIPLE``                       0x20 Use this style to allow a range of items to be selected. If a second range is selected, the current range, if any, is deselected.
         ``TR_EXTENDED``                       0x40 Use this style to allow disjoint items to be selected. (Only partially implemented; may not work in all cases).
         ``TR_HAS_VARIABLE_ROW_HEIGHT``        0x80 Use this style to cause row heights to be just big enough to fit the content. If not set, all rows use the largest row height. The default is that this flag is unset.
         ``TR_EDIT_LABELS``                   0x200 Use this style if you wish the user to be able to edit labels in the tree control.
         ``TR_ROW_LINES``                     0x400 Use this style to draw a contrasting border between displayed rows.
         ``TR_HIDE_ROOT``                     0x800 Use this style to suppress the display of the root node, effectively causing the first-level nodes to appear as a series of root nodes.
         ``TR_COLUMN_LINES``                 0x1000 No Docs
         ``TR_FULL_ROW_HIGHLIGHT``           0x2000 Use this style to have the background colour and the selection highlight extend  over the entire horizontal row of the tree control window.
         ``TR_AUTO_CHECK_CHILD``             0x4000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are checked/unchecked as well.
         ``TR_AUTO_TOGGLE_CHILD``            0x8000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are toggled accordingly.
         ``TR_AUTO_CHECK_PARENT``           0x10000 Only meaningful foe checkbox-type items: when a child item is checked/unchecked its parent item is checked/unchecked as well.
         ``TR_ALIGN_WINDOWS``               0x20000 Flag used to align windows (in items with windows) at the same horizontal position.
         ``TR_NO_HEADER``                   0x40000 Use this style to hide the columns header.
         ``TR_VIRTUAL``                     0x80000 L{HyperTreeList} will have virtual behaviour.
         ============================== =========== ==================================================

        :param `validator`: window validator;
        :param `name`: window name.
        """

        wx.PyControl.__init__(self, parent, id, pos, size, style, validator, name)

        self._header_win = None
        self._main_win = None
        self._headerHeight = 0
        self._attr_set = False
        
        main_style = style & ~(wx.SIMPLE_BORDER|wx.SUNKEN_BORDER|wx.DOUBLE_BORDER|
                               wx.RAISED_BORDER|wx.STATIC_BORDER)

        self._agwStyle = agwStyle
        
        self._main_win = TreeListMainWindow(self, -1, wx.Point(0, 0), size, main_style, agwStyle, validator)
        self._main_win._buffered = False

        self._header_win = TreeListHeaderWindow(self, -1, self._main_win, wx.Point(0, 0),
                                                wx.DefaultSize, wx.TAB_TRAVERSAL)
        self._header_win._buffered = False
        
        self.CalculateAndSetHeaderHeight()
        self.Bind(wx.EVT_SIZE, self.OnSize)

        self.SetBuffered(IsBufferingSupported())
        self._main_win.SetAGWWindowStyleFlag(agwStyle)
        

    def SetBuffered(self, buffered):
        """
        Sets/unsets the double buffering for the header and the main window.

        :param `buffered`: ``True`` to use double-buffering, ``False`` otherwise.

        :note: Currently we are using double-buffering only on Windows XP.
        """

        self._main_win.SetBuffered(buffered)
        self._header_win.SetBuffered(buffered)


    def CalculateAndSetHeaderHeight(self):
        """ Calculates the best header height and stores it. """

        if self._header_win:
            h = wx.RendererNative.Get().GetHeaderButtonHeight(self._header_win)
            # only update if changed
            if h != self._headerHeight:
                self._headerHeight = h
                self.DoHeaderLayout()
            

    def DoHeaderLayout(self):
        """ Layouts the header control. """

        w, h = self.GetClientSize()
        has_header = self._agwStyle & TR_NO_HEADER == 0
        
        if self._header_win and has_header:
            self._header_win.SetDimensions(0, 0, w, self._headerHeight)
            self._header_win.Refresh()
        else:
            self._header_win.SetDimensions(0, 0, 0, 0)
        
        if self._main_win and has_header:
            self._main_win.SetDimensions(0, self._headerHeight + 1, w, h - self._headerHeight - 1)
        else:
            self._main_win.SetDimensions(0, 0, w, h)
    

    def OnSize(self, event):
        """
        Handles the ``wx.EVT_SIZE`` event for L{HyperTreeList}.

        :param `event`: a `wx.SizeEvent` event to be processed.
        """

        self.DoHeaderLayout()


    def SetFont(self, font):
        """
        Sets the default font for the header window and the main window.

        :param `font`: a valid `wx.Font` object.
        """
        
        if self._header_win:
            self._header_win.SetFont(font)
            self.CalculateAndSetHeaderHeight()
            self._header_win.Refresh()
        
        if self._main_win:
            return self._main_win.SetFont(font)
        else:
            return False


    def SetHeaderFont(self, font):
        """
        Sets the default font for the header window..

        :param `font`: a valid `wx.Font` object.
        """

        if not self._header_win:
            return
        
        for column in xrange(self.GetColumnCount()):
            self._header_win.SetColumn(column, self.GetColumn(column).SetFont(font))

        self._header_win.Refresh()

    
    def SetHeaderCustomRenderer(self, renderer=None):
        """
        Associate a custom renderer with the header - all columns will use it

        :param `renderer`: a class able to correctly render header buttons

        :note: the renderer class **must** implement the method `DrawHeaderButton`
        """

        self._header_win.SetCustomRenderer(renderer)
        

    def SetAGWWindowStyleFlag(self, agwStyle):
        """
        Sets the window style for L{HyperTreeList}.

        :param `agwStyle`: can be a combination of the following bits:

         ============================== =========== ==================================================
         Window Styles                  Hex Value   Description
         ============================== =========== ==================================================
         ``TR_NO_BUTTONS``                      0x0 For convenience to document that no buttons are to be drawn.
         ``TR_SINGLE``                          0x0 For convenience to document that only one item may be selected at a time. Selecting another item causes the current selection, if any, to be deselected. This is the default.
         ``TR_HAS_BUTTONS``                     0x1 Use this style to show + and - buttons to the left of parent items.
         ``TR_NO_LINES``                        0x4 Use this style to hide vertical level connectors.
         ``TR_LINES_AT_ROOT``                   0x8 Use this style to show lines between root nodes. Only applicable if ``TR_HIDE_ROOT`` is set and ``TR_NO_LINES`` is not set.
         ``TR_DEFAULT_STYLE``                   0x9 No Docs
         ``TR_TWIST_BUTTONS``                  0x10 Use old Mac-twist style buttons.
         ``TR_MULTIPLE``                       0x20 Use this style to allow a range of items to be selected. If a second range is selected, the current range, if any, is deselected.
         ``TR_EXTENDED``                       0x40 Use this style to allow disjoint items to be selected. (Only partially implemented; may not work in all cases).
         ``TR_HAS_VARIABLE_ROW_HEIGHT``        0x80 Use this style to cause row heights to be just big enough to fit the content. If not set, all rows use the largest row height. The default is that this flag is unset.
         ``TR_EDIT_LABELS``                   0x200 Use this style if you wish the user to be able to edit labels in the tree control.
         ``TR_ROW_LINES``                     0x400 Use this style to draw a contrasting border between displayed rows.
         ``TR_HIDE_ROOT``                     0x800 Use this style to suppress the display of the root node, effectively causing the first-level nodes to appear as a series of root nodes.
         ``TR_COLUMN_LINES``                 0x1000 No Docs
         ``TR_FULL_ROW_HIGHLIGHT``           0x2000 Use this style to have the background colour and the selection highlight extend  over the entire horizontal row of the tree control window.
         ``TR_AUTO_CHECK_CHILD``             0x4000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are checked/unchecked as well.
         ``TR_AUTO_TOGGLE_CHILD``            0x8000 Only meaningful foe checkbox-type items: when a parent item is checked/unchecked its children are toggled accordingly.
         ``TR_AUTO_CHECK_PARENT``           0x10000 Only meaningful foe checkbox-type items: when a child item is checked/unchecked its parent item is checked/unchecked as well.
         ``TR_ALIGN_WINDOWS``               0x20000 Flag used to align windows (in items with windows) at the same horizontal position.
         ``TR_NO_HEADER``                   0x40000 Use this style to hide the columns header.
         ``TR_VIRTUAL``                     0x80000 L{HyperTreeList} will have virtual behaviour.
         ============================== =========== ==================================================
         
        :note: Please note that some styles cannot be changed after the window creation
         and that `Refresh()` might need to be be called after changing the others for
         the change to take place immediately.
        """
        
        if self._main_win:
            self._main_win.SetAGWWindowStyleFlag(agwStyle)

        tmp = self._agwStyle
        self._agwStyle = agwStyle
        if abs(agwStyle - tmp) & TR_NO_HEADER:
            self.DoHeaderLayout()
            

    def GetAGWWindowStyleFlag(self):
        """
        Returns the L{HyperTreeList} window style flag.

        :see: L{SetAGWWindowStyleFlag} for a list of valid window styles.
        """

        agwStyle = self._agwStyle
        if self._main_win:
            agwStyle |= self._main_win.GetAGWWindowStyleFlag()
            
        return agwStyle


    def HasAGWFlag(self, flag):
        """
        Returns whether a flag is present in the L{HyperTreeList} style.

        :param `flag`: one of the possible L{HyperTreeList} window styles.

        :see: L{SetAGWWindowStyleFlag} for a list of possible window style flags.
        """

        agwStyle = self.GetAGWWindowStyleFlag()
        res = (agwStyle & flag and [True] or [False])[0]
        return res


    def SetBackgroundColour(self, colour):
        """
        Changes the background colour of L{HyperTreeList}.

        :param `colour`: the colour to be used as the background colour, pass
         `wx.NullColour` to reset to the default colour.

        :note: The background colour is usually painted by the default `wx.EraseEvent`
         event handler function under Windows and automatically under GTK.

        :note: Setting the background colour does not cause an immediate refresh, so
         you may wish to call `wx.Window.ClearBackground` or `wx.Window.Refresh` after
         calling this function.

        :note: Overridden from `wx.PyControl`.         
        """

        if not self._main_win:
            return False
        
        return self._main_win.SetBackgroundColour(colour)


    def SetForegroundColour(self, colour):
        """
        Changes the foreground colour of L{HyperTreeList}.

        :param `colour`: the colour to be used as the foreground colour, pass
         `wx.NullColour` to reset to the default colour.

        :note: Overridden from `wx.PyControl`.         
        """

        if not self._main_win:
            return False
        
        return self._main_win.SetForegroundColour(colour)


    def SetColumnWidth(self, column, width):
        """
        Sets the column width, in pixels.

        :param `column`: an integer specifying the column index;
        :param `width`: the new column width, in pixels.
        """

        if width == wx.LIST_AUTOSIZE_USEHEADER:
        
            font = self._header_win.GetFont()
            dc = wx.ClientDC(self._header_win)
            width, dummy, dummy = dc.GetMultiLineTextExtent(self._header_win.GetColumnText(column))
            # Search TreeListHeaderWindow.OnPaint to understand this:
            width += 2*_EXTRA_WIDTH + _MARGIN
        
        elif width == wx.LIST_AUTOSIZE:
        
            width = self._main_win.GetBestColumnWidth(column)
        
        self._header_win.SetColumnWidth(column, width)
        self._header_win.Refresh()


    def GetColumnWidth(self, column):
        """
        Returns the column width, in pixels.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumnWidth(column)

        
    def SetColumnText(self, column, text):
        """
        Sets the column text label.

        :param `column`: an integer specifying the column index;
        :param `text`: the new column label.
        """

        self._header_win.SetColumnText(column, text)
        self._header_win.Refresh()


    def GetColumnText(self, column):
        """
        Returns the column text label.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumnText(column)


    def AddColumn(self, text, width=_DEFAULT_COL_WIDTH, flag=wx.ALIGN_LEFT,
                  image=-1, shown=True, colour=None, edit=False):
        """
        Appends a column to the L{HyperTreeList}.

        :param `text`: the column text label;
        :param `width`: the column width in pixels;
        :param `flag`: the column alignment flag, one of ``wx.ALIGN_LEFT``,
         ``wx.ALIGN_RIGHT``, ``wx.ALIGN_CENTER``;
        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column;
        :param `shown`: ``True`` to show the column, ``False`` to hide it;
        :param `colour`: a valid `wx.Colour`, representing the text foreground colour
         for the column;
        :param `edit`: ``True`` to set the column as editable, ``False`` otherwise.
        """

        self._header_win.AddColumn(text, width, flag, image, shown, colour, edit)
        self.DoHeaderLayout()
        

    def AddColumnInfo(self, colInfo):
        """
        Appends a column to the L{HyperTreeList}.

        :param `colInfo`: an instance of L{TreeListColumnInfo}.
        """

        self._header_win.AddColumnInfo(colInfo)
        self.DoHeaderLayout()


    def InsertColumnInfo(self, before, colInfo):
        """
        Inserts a column to the L{HyperTreeList} at the position specified
        by `before`.

        :param `before`: the index at which we wish to insert the new column;
        :param `colInfo`: an instance of L{TreeListColumnInfo}.
        """

        self._header_win.InsertColumnInfo(before, colInfo)
        self._header_win.Refresh()


    def InsertColumn(self, before, text, width=_DEFAULT_COL_WIDTH,
                     flag=wx.ALIGN_LEFT, image=-1, shown=True, colour=None, 
                     edit=False):
        """
        Inserts a column to the L{HyperTreeList} at the position specified
        by `before`.

        :param `before`: the index at which we wish to insert the new column;
        :param `text`: the column text label;
        :param `width`: the column width in pixels;
        :param `flag`: the column alignment flag, one of ``wx.ALIGN_LEFT``,
         ``wx.ALIGN_RIGHT``, ``wx.ALIGN_CENTER``;
        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column;
        :param `shown`: ``True`` to show the column, ``False`` to hide it;
        :param `colour`: a valid `wx.Colour`, representing the text foreground colour
         for the column;
        :param `edit`: ``True`` to set the column as editable, ``False`` otherwise.        
        """

        self._header_win.InsertColumn(before, text, width, flag, image,
                                      shown, colour, edit)
        self._header_win.Refresh()


    def RemoveColumn(self, column):
        """
        Removes a column from the L{HyperTreeList}.

        :param `column`: an integer specifying the column index.
        """

        self._header_win.RemoveColumn(column)
        self._header_win.Refresh()


    def SetColumn(self, column, colInfo):
        """
        Sets a column using an instance of L{TreeListColumnInfo}.

        :param `column`: an integer specifying the column index;
        :param `info`: an instance of L{TreeListColumnInfo}.        
        """

        self._header_win.SetColumn(column, colInfo)
        self._header_win.Refresh()
            

    def GetColumn(self, column):
        """
        Returns an instance of L{TreeListColumnInfo} containing column information.

        :param `column`: an integer specifying the column index.
        """
        
        return self._header_win.GetColumn(column)


    def SetColumnImage(self, column, image):
        """
        Sets an image on the specified column.

        :param `column`: an integer specifying the column index.
        :param `image`: an index within the normal image list assigned to
         L{HyperTreeList} specifying the image to use for the column.
        """                

        self._header_win.SetColumn(column, self.GetColumn(column).SetImage(image))
        self._header_win.Refresh()


    def GetColumnImage(self, column):
        """
        Returns the image assigned to the specified column.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumn(column).GetImage()


    def SetColumnEditable(self, column, edit):
        """
        Sets the column as editable or non-editable.

        :param `column`: an integer specifying the column index;
        :param `edit`: ``True`` if the column should be editable, ``False`` otherwise.
        """

        self._header_win.SetColumn(column, self.GetColumn(column).SetEditable(edit))


    def SetColumnShown(self, column, shown):
        """
        Sets the column as shown or hidden.

        :param `column`: an integer specifying the column index;
        :param `shown`: ``True`` if the column should be shown, ``False`` if it
         should be hidden.
        """

        if self._main_win.GetMainColumn() == column:
            shown = True # Main column cannot be hidden
            
        self.SetColumn(column, self.GetColumn(column).SetShown(shown))


    def IsColumnEditable(self, column):
        """
        Returns ``True`` if the column is editable, ``False`` otherwise.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumn(column).IsEditable()


    def IsColumnShown(self, column):
        """
        Returns ``True`` if the column is shown, ``False`` otherwise.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumn(column).IsShown()


    def SetColumnAlignment(self, column, flag):
        """
        Sets the column text alignment.

        :param `column`: an integer specifying the column index;
        :param `flag`: the alignment flag, one of ``wx.ALIGN_LEFT``, ``wx.ALIGN_RIGHT``,
         ``wx.ALIGN_CENTER``.
        """

        self._header_win.SetColumn(column, self.GetColumn(column).SetAlignment(flag))
        self._header_win.Refresh()


    def GetColumnAlignment(self, column):
        """
        Returns the column text alignment.

        :param `column`: an integer specifying the column index.
        """
        
        return self._header_win.GetColumn(column).GetAlignment()


    def SetColumnColour(self, column, colour):
        """
        Sets the column text colour.

        :param `column`: an integer specifying the column index;
        :param `colour`: a valid `wx.Colour` object.
        """

        self._header_win.SetColumn(column, self.GetColumn(column).SetColour(colour))
        self._header_win.Refresh()


    def GetColumnColour(self, column):
        """
        Returns the column text colour.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumn(column).GetColour()
                

    def SetColumnFont(self, column, font):
        """
        Sets the column text font.

        :param `column`: an integer specifying the column index;
        :param `font`: a valid `wx.Font` object.
        """

        self._header_win.SetColumn(column, self.GetColumn(column).SetFont(font))
        self._header_win.Refresh()


    def GetColumnFont(self, column):
        """
        Returns the column text font.

        :param `column`: an integer specifying the column index.
        """

        return self._header_win.GetColumn(column).GetFont()


    def Refresh(self, erase=True, rect=None):
        """
        Causes this window, and all of its children recursively (except under wxGTK1
        where this is not implemented), to be repainted. 

        :param `erase`: If ``True``, the background will be erased;
        :param `rect`: If not ``None``, only the given rectangle will be treated as damaged.

        :note: Note that repainting doesn't happen immediately but only during the next
         event loop iteration, if you need to update the window immediately you should
         use `Update` instead.

        :note: Overridden from `wx.PyControl`.         
        """

        self._main_win.Refresh(erase, rect)
        self._header_win.Refresh(erase, rect)


    def SetFocus(self):
        """ This sets the window to receive keyboard input. """
        
        self._main_win.SetFocus() 


    def GetHeaderWindow(self):
        """ Returns the header window, an instance of L{TreeListHeaderWindow}. """
        
        return self._header_win
    

    def GetMainWindow(self):
        """ Returns the main window, an instance of L{TreeListMainWindow}. """
        
        return self._main_win


    def DoGetBestSize(self):
        """
        Gets the size which best suits the window: for a control, it would be the
        minimal size which doesn't truncate the control, for a panel - the same size
        as it would have after a call to `Fit()`.
        """

        # something is better than nothing...
        return wx.Size(200, 200) # but it should be specified values! FIXME


    def OnGetItemText(self, item, column):
        """
        This function **must** be overloaded in the derived class for a control
        with ``TR_VIRTUAL`` style. It should return the string containing the
        text of the given column for the specified item.

        :param `item`: an instance of L{TreeListItem};
        :param `column`: an integer specifying the column index.
        """
        
        return ""


    def SortChildren(self, item):
        """
        Sorts the children of the given item using L{OnCompareItems} method of L{HyperTreeList}. 
        You should override that method to change the sort order (the default is ascending
        case-sensitive alphabetical order).

        :param `item`: an instance of L{TreeListItem};
        """

        if not self._attr_set:
            setattr(self._main_win, "OnCompareItems", self.OnCompareItems)
            self._attr_set = True
            
        self._main_win.SortChildren(item)
        

    def OnCompareItems(self, item1, item2):
        """
        Returns whether 2 items have the same text.
        
        Override this function in the derived class to change the sort order of the items
        in the L{HyperTreeList}. The function should return a negative, zero or positive
        value if the first item is less than, equal to or greater than the second one.

        :param `item1`: an instance of L{TreeListItem};
        :param `item2`: another instance of L{TreeListItem}.

        :note: The base class version compares items alphabetically.
        """

        # do the comparison here, and not delegate to self._main_win, in order
        # to let the user override it

        return self.GetItemText(item1) == self.GetItemText(item2)


    def CreateEditCtrl(self, item, column):
        """
        Create an edit control for editing a label of an item. By default, this
        returns a text control.
        
        Override this function in the derived class to return a different type
        of control.
        
        :param `item`: an instance of L{TreeListItem};
        :param `column`: an integer specifying the column index.
        """
        return EditTextCtrl(self.GetMainWindow(), -1, item, column,
                            self.GetMainWindow(), item.GetText(column),
                            style=self.GetTextCtrlStyle(column))

        
    def GetTextCtrlStyle(self, column):
        """
        Return the style to use for the text control that is used to edit
        labels of items. 
        
        Override this function in the derived class to support a different
        style, e.g. wx.TE_MULTILINE.
        
        :param `column`: an integer specifying the column index.
        """
        return self.GetTextCtrlAlignmentStyle(column) | wx.TE_PROCESS_ENTER

        
    def GetTextCtrlAlignmentStyle(self, column):
        """
        Return the alignment style to use for the text control that is used
        to edit labels of items. The alignment style is derived from the 
        column alignment.
        
        :param `column`: an integer specifying the column index.
        """
        header_win = self.GetHeaderWindow()
        alignment = header_win.GetColumnAlignment(column)
        return {wx.ALIGN_LEFT: wx.TE_LEFT, 
                wx.ALIGN_RIGHT: wx.TE_RIGHT, 
                wx.ALIGN_CENTER: wx.TE_CENTER}[alignment]

    
    def GetClassDefaultAttributes(self):
        """
        Returns the default font and colours which are used by the control. This is
        useful if you want to use the same font or colour in your own control as in
        a standard control -- which is a much better idea than hard coding specific
        colours or fonts which might look completely out of place on the users system,
        especially if it uses themes.

        This static method is "overridden'' in many derived classes and so calling,
        for example, `wx.Button.GetClassDefaultAttributes()` will typically return the
        values appropriate for a button which will be normally different from those
        returned by, say, `wx.ListCtrl.GetClassDefaultAttributes()`.

        :note: The `wx.VisualAttributes` structure has at least the fields `font`,
         `colFg` and `colBg`. All of them may be invalid if it was not possible to
         determine the default control appearance or, especially for the background
         colour, if the field doesn't make sense as is the case for `colBg` for the
         controls with themed background.
        """

        attr = wx.VisualAttributes()
        attr.colFg = wx.SystemSettings_GetColour(wx.SYS_COLOUR_WINDOWTEXT)
        attr.colBg = wx.SystemSettings_GetColour(wx.SYS_COLOUR_LISTBOX)
        attr.font  = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        return attr

    GetClassDefaultAttributes = classmethod(GetClassDefaultAttributes)


def create_delegator_for(method):
    """
    Creates a method that forwards calls to `self._main_win` (an instance of L{TreeListMainWindow}).

    :param `method`: one method inside the L{TreeListMainWindow} local scope.
    """
    
    def delegate(self, *args, **kwargs):
        return getattr(self._main_win, method)(*args, **kwargs)
    return delegate

# Create methods that delegate to self._main_win. This approach allows for
# overriding these methods in possible subclasses of HyperTreeList
for method in _methods:
    setattr(HyperTreeList, method, create_delegator_for(method))    

