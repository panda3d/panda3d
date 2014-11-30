###############################################################################
# Name: finddlg.py                                                            #
# Purpose: Custom advanced find dialog                                        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Editra Control Library: Advanced Find Replace Dialog

The AdvancedFindReplaceDialog is a custom FindReplaceDlg that functions
similarly to the standard wx.FindReplaceDialog but provides more search
configuration and presentation options. 

The following items are the options that the AdvancedFindReplaceDialog offers
over the basic FindReplaceDialog.

  * Hide/Show each option or section individually (basic dialog only disables them)
  * Multi-Find/Replace event action for Find All / Replace All actions
  * Switch dialog from Find mode to Replace mode or visa-versa once its already
    been created.
  * Options for specifying the location to look in.
  * Regular Expression option
  * Use standard dialog or a floating MiniFrame (default)

Requirements:
python 2.4+
wxPython 2.8+
eclib.platebtn
eclib.ctrlbox

@todo: Make Look In location strings configurable

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: finddlg.py 68232 2011-07-12 02:08:53Z CJP $"
__revision__ = "$Revision: 68232 $"

__all__ = ["FindBox", "FindEvent", "FindPanel", "FindReplaceDlg",
           "MiniFindReplaceDlg", "AdvFindReplaceDlg",

           "AFR_STYLE_FINDDIALOG", "AFR_STYLE_REPLACEDIALOG",
           "AFR_STYLE_NON_FLOATING", "AFR_STYLE_NO_MODE_SELECT",
           
           "AFR_UP", "AFR_WHOLEWORD", 
           "AFR_MATCHCASE", "AFR_REGEX", "AFR_RECURSIVE", "AFR_NOLOOKIN",
           "AFR_NOUPDOWN", "AFR_NOWHOLEWORD", "AFR_NOMATCHCASE", "AFR_NOREGEX",
           "AFR_NOFILTER", "AFR_NOOPTIONS", "AFR_NO_COUNT", "AFR_NO_ALL_BTN",
           "AFR_SIMPLE",

           "LOCATION_CURRENT_DOC", "LOCATION_IN_SELECTION",
           "LOCATION_OPEN_DOCS", "LOCATION_IN_CURRENT_DIR",
           "LOCATION_IN_FILES", "LOCATION_MAX",

           "edEVT_FIND_CLOSE", "EVT_FIND_CLOSE", "edEVT_FIND", "EVT_FIND",
           "edEVT_FIND_NEXT", "EVT_FIND_NEXT", "edEVT_FIND_ALL", "EVT_FIND_ALL",
           "edEVT_REPLACE", "EVT_REPLACE", "edEVT_REPLACE_ALL",
           "EVT_REPLACE_ALL", "EVT_OPTION_CHANGED", "edEVT_OPTION_CHANGED",
           "EVT_COUNT", "edEVT_COUNT"]

#--------------------------------------------------------------------------#
# Imports
import os
import wx

# Local Imports
import ctrlbox
import platebtn

#--------------------------------------------------------------------------#
# Globals

# Style Flags
AFR_STYLE_FINDDIALOG        = 0 # Start dialog in Find mode
AFR_STYLE_REPLACEDIALOG     = 1 # Start dialog in Replace mode
AFR_STYLE_NON_FLOATING      = 2 # Use a standard dialog as the tlw
AFR_STYLE_NO_MODE_SELECT    = 4 # Hide the mode selector buttons

# FindReplaceData Flags
AFR_UP          = 1             # Set dialogs Search Up flag
AFR_WHOLEWORD   = 2             # Set dialogs whole word search flag
AFR_MATCHCASE   = 4             # Set dialogs match case search flag
AFR_REGEX       = 8             # Set dialogs use regular expression flag
AFR_RECURSIVE   = 16            # Set dialogs recursive directory search flag
AFR_NOLOOKIN    = 32            # Hide the Looking combobox in the dialog
AFR_NOUPDOWN    = 64            # Hide the Direction options in the dialog
AFR_NOWHOLEWORD = 128           # Hide the Whole Word option in the dialog
AFR_NOMATCHCASE = 256           # Hide the Match Case option in the dialog
AFR_NOREGEX     = 512           # Hide the Regular Expression option
AFR_NOFILTER    = 1024          # Hide the File Filter option
AFR_NOOPTIONS   = 2048          # Hide all options in the dialog
AFR_NO_COUNT    = 4096          # Hide Count Button
AFR_NO_ALL_BTN  = 8192          # Hide Find All / Replace All buttons

# Convenience Flags
AFR_SIMPLE = (AFR_NOLOOKIN | AFR_NOOPTIONS | AFR_NOUPDOWN | \
              AFR_NO_COUNT | AFR_NO_ALL_BTN)

# Search Location Parameters (NOTE: must be kept in sync with Lookin List)
LOCATION_CURRENT_DOC    = 0
LOCATION_IN_SELECTION   = 1
LOCATION_OPEN_DOCS      = 2
LOCATION_IN_CURRENT_DIR = 3
LOCATION_IN_FILES       = 4
LOCATION_MAX            = LOCATION_IN_CURRENT_DIR

# Control Names
FindBoxName = "EdFindBox"
FindPanelName = "EdFindPanel"

# Find Panel Control Ids
ID_LOOKIN = wx.NewId()
ID_FIND_LBL = wx.NewId()
ID_REPLACE_LBL = wx.NewId()
ID_MATCH_CASE = wx.NewId()
ID_WHOLE_WORD = wx.NewId()
ID_REGEX = wx.NewId()
ID_RECURSE = wx.NewId()
ID_COUNT = wx.NewId()
ID_FIND_ALL = wx.NewId()
ID_REPLACE_ALL = wx.NewId()
ID_OPTION_CHANGE = wx.NewId()
ID_CHOOSE_DIR = wx.NewId()
ID_DIR_BOX = wx.NewId()

_ = wx.GetTranslation

#--------------------------------------------------------------------------#

# Events Definitions

# Find dialog has been closed
edEVT_FIND_CLOSE = wx.NewEventType()
EVT_FIND_CLOSE = wx.PyEventBinder(edEVT_FIND_CLOSE, 1)

# Find cutton clicked
edEVT_FIND = wx.NewEventType()
EVT_FIND = wx.PyEventBinder(edEVT_FIND, 1)

# Find cutton clicked again with the same search string
edEVT_FIND_NEXT = wx.NewEventType()
EVT_FIND_NEXT = wx.PyEventBinder(edEVT_FIND_NEXT, 1)

# Find All button clicked
edEVT_FIND_ALL = wx.NewEventType()
EVT_FIND_ALL = wx.PyEventBinder(edEVT_FIND_ALL, 1)

# Replace button clicked
edEVT_REPLACE = wx.NewEventType()
EVT_REPLACE = wx.PyEventBinder(edEVT_REPLACE, 1)

# Replace button clicked
edEVT_COUNT = wx.NewEventType()
EVT_COUNT = wx.PyEventBinder(edEVT_COUNT, 1)

# Replace All button clicked
edEVT_REPLACE_ALL = wx.NewEventType()
EVT_REPLACE_ALL = wx.PyEventBinder(edEVT_REPLACE_ALL, 1)

# Find option has changed
edEVT_OPTION_CHANGED = wx.NewEventType()
EVT_OPTION_CHANGED = wx.PyEventBinder(edEVT_OPTION_CHANGED, 1)

# Convenience for generating events
_EVENT_MAP = { wx.ID_FIND : edEVT_FIND,
               wx.ID_REPLACE : edEVT_REPLACE,
               ID_COUNT : edEVT_COUNT,
               ID_FIND_ALL : edEVT_FIND_ALL,
               ID_REPLACE_ALL : edEVT_REPLACE_ALL,
               ID_OPTION_CHANGE : edEVT_OPTION_CHANGED }

class FindEvent(wx.PyCommandEvent):
    """Event sent by the FindReplaceDialog that contains all
    options of the FindReplaceData and requested action of the
    find dialog

    """
    def __init__(self, etype, eid=wx.ID_ANY, flags=0):
        """Creates the event object
        @keyword flags: Find/Replace flags

        """
        super(FindEvent, self).__init__(etype, eid)

        # Attributes
        self._flags = flags
        self._loc = 0
        self._find = u''
        self._replace = u''
        self._dir = u''
        self._filters = None
        self._count = 0

    def GetCount(self):
        """Get the number of matches
        @return: int

        """
        return self._count

    def GetDirectory(self):
        """Get the directory of files to search in
        @return: string

        """
        return self._dir

    def GetFileFilters(self):
        """Get the filter to search with
        @return: list or None

        """
        return self._filters

    def GetFindString(self):
        """Get the find string
        @return: string

        """
        return self._find

    def GetFlags(self):
        """Get the search flags
        @return: long

        """
        return self._flags

    def GetReplaceString(self):
        """Set the find String
        @return: string

        """
        return self._replace

    def GetSearchType(self):
        """Get the type of search (current buffer, open documents, ect...)
        @return: int (see LOCATION_* flags)

        """
        return self._loc

    def SetCount(self, count):
        """Set the count
        @param count: int

        """
        self._count = count

    def SetDirectory(self, directory):
        """Set the directory of files to search in
        @param directory: string

        """
        self._dir = directory

    def SetFileFilters(self, filters):
        """Set the file filters to use for a Search In Files event
        @param filters: string or list of strings

        """
        if isinstance(filters, basestring):
            self._filters = filters.split()
        else:
            self._filters = filters

        if self._filters is not None:
            self._filters = [ f.strip() for f in self._filters ]

    def SetFindString(self, find):
        """Set the find String
        @param find: string

        """
        self._find = find

    def SetFlags(self, flags):
        """Returns the value from the event.
        @return: the value of this event

        """
        self._flags = flags

    def SetReplaceString(self, rstring):
        """Set the find String
        @param rstring: string

        """
        self._replace = rstring

    def SetSearchType(self, stype):
        """Set the type of search (current buffer, open documents, ect...)
        @param stype: int (see LOCATION_* flags)

        """
        self._loc = stype

    def IsMatchCase(self):
        """Is this a match case search
        @return: bool

        """
        return bool(self._flags & AFR_MATCHCASE)

    def IsRecursive(self):
        """Is the search option for recursive directory search enabled
        @return: bool

        """
        return bool(self._flags & AFR_RECURSIVE)

    def IsRegEx(self):
        """Is RegEx enabled in the dialog
        @return: bool

        """
        return bool(self._flags & AFR_REGEX)

    def IsWholeWord(self):
        """Is this a whole word search
        @return: bool

        """
        return bool(self._flags & AFR_WHOLEWORD)

    def IsUp(self):
        """Is the search searching up
        @return: bool

        """
        return bool(self._flags & AFR_UP)

#--------------------------------------------------------------------------#

def AdvFindReplaceDlg(parent, fdata, title, style=AFR_STYLE_FINDDIALOG):
    """Advanced FindReplaceDialog. Create and return the requested dialog type
    @param parent: parent
    @param fdata: FindReplaceData
    @param title: Dialog Title. Pass a single string to set the title for both
                  modes of the dialog or a tuple of two strings to set the title
                  for (find, replace) modes.
    @keyword style: Dialog Style and type
    @note: this is a function not a class

    """
    if style & AFR_STYLE_NON_FLOATING:
        dlg = FindReplaceDlg(parent, fdata, title, style)
    else:
        dlg = MiniFindReplaceDlg(parent, fdata, title, style)

    return dlg

#--------------------------------------------------------------------------#

class FindReplaceDlgBase:
    """Delegate mixin base class for deriving FindReplaceDialogs, delegates
    calls to the widgets and containers that make up the dialog.
    @note: The mixin must be initialized after the class its being mixed into

    """
    def __init__(self, parent, fdata, title, style=AFR_STYLE_FINDDIALOG):
        """Create the base object
        @param title: string or tuple (findstr, replacestr)

        """
        # Attributes
        if isinstance(title, basestring):
            self._ftitle = title
            self._rtitle = title
        else:
            self._ftitle, self._rtitle = title[:2]

        self._box = FindBox(self, fdata, style=style)
        self._panel = self._box.GetWindow()
        self._accl = wx.AcceleratorTable([(wx.ACCEL_NORMAL, wx.WXK_ESCAPE, wx.ID_CLOSE),])
        self.SetAcceleratorTable(self._accl)
        
        # Layout
        self.__DoLayout()
        tmp_title = self._ftitle
        if style & AFR_STYLE_REPLACEDIALOG:
            tmp_title = self._rtitle
        self.SetTitle(tmp_title)

        # Event handlers
        self.Bind(_EVT_MODE_CHANGE, self._OnModeChange)
        self.Bind(_EVT_DO_CLOSE_DLG, lambda evt: self._SendCloseEvent())
        self.Bind(wx.EVT_MENU, lambda evt: self._SendCloseEvent(), id=wx.ID_CLOSE)
        self.Bind(wx.EVT_SET_FOCUS,
                  lambda evt: self._panel.SetFocus() and evt.Skip())
        self.Bind(wx.EVT_SHOW, lambda evt: self._panel.SetFocus())

    def __DoLayout(self):
        """Layout the dialog"""
        vsizer = wx.BoxSizer(wx.VERTICAL)
        vsizer.Add(self._box, 1, wx.EXPAND)
        self.SetSizer(vsizer)
        self.SetAutoLayout(True)
        self.Fit()

    def _SendCloseEvent(self):
        """Send a dialog close event and hide the dialog"""
        data = self._panel.GetData()
        data.SetFindString(self._panel._ftxt.GetValue())
        data.SetReplaceString(self._panel._rtxt.GetValue())
        evt = FindEvent(edEVT_FIND_CLOSE, self.GetId())
        self.Hide()
        wx.PostEvent(self.GetParent(), evt)

    def _OnModeChange(self, evt):
        """Update the the dialog when the mode changes"""
        self.Fit()
        title = self._ftitle
        if self.GetDialogMode() != AFR_STYLE_FINDDIALOG:
            title = self._rtitle
        self.SetTitle(title)

    def GetData(self):
        """Get the FindReplaceData used by this dialog"""
        return self._panel.GetData()

    def GetDialogMode(self):
        """Get the current mode of the dialog
        @return: AFR_STYLE_FINDDIALOG or AFR_STYLE_REPLACEDIALOG

        """
        return self._panel.GetPanelMode()

    def GetFileFilters(self):
        """Get the file filters field value
        @return: string

        """
        return self._panel.GetFileFilters()

    def GetLookinChoices(self):
        """Get the set choices from the looking choice list
        @return: list of strings

        """
        return self._panel.GetLookinChoices()

    def GetLookinSelection(self):
        """Get the index of the selected item in the lookin choice list
        @return: int

        """
        return self._panel.GetLookinSelection()

    def RefreshFindReplaceFields(self):
        """Refresh the values of the Find and Replace fields with the
        values that are currently in the FindReplaceData.

        """
        data = self.GetData()
        self._panel.SetFindString(data.GetFindString())
        self._panel.SetReplaceString(data.GetReplaceString())

    def RefreshFindOptions(self):
        """Refresh the find options controls based on the current
        values of the FindData owned by this window.

        """
        self._panel.RefreshControls()

    def SetData(self, data):
        """Set the dialogs FindReplaceData
        @param data: FindReplaceData
        @note: causes updates in dialog

        """
        self._panel.SetData(data)

    def SetDialogMode(self, mode):
        """Set the dialog mode between find and find/replace
        @param mode: AFR_STYLE_FINDDIALOG or AFR_STYLE_REPLACEDIALOG

        """
        self._box.SetBoxMode(mode == AFR_STYLE_FINDDIALOG)

    def SetFileFilters(self, filters):
        """Set the file filters field value
        @param filters: string

        """
        self._panel.SetFileFilters(filters)

    def SetFindBitmap(self, bmp):
        """Set the find Bitmap
        @param bmp: wx.Bitmap

        """
        self._box.SetFindBitmap(bmp)

    def SetFlag(self, flags):
        """Set a search dialog flag.
        @param flags: AFR_*

        """
        self._panel.SetFlag(flags)

    def SetFlags(self, flags):
        """Set the search dialog flags.
        @param flags: bitmask of AFR_ values

        """
        self._panel.SetFlags(flags)

    def SetLookinChoices(self, paths):
        """Set the looking choices
        @param paths: list of strings

        """
        self._panel.SetLookinChoices(paths)

    def SetLookinPath(self, path):
        """Set the lookin path, adding it to the collection if it is
        not in there.
        @param path: string (path of directory)

        """
        idx = self._panel.AddLookinPath(path)
        self.SetLookinSelection(idx)

    def SetLookinSelection(self, sel):
        """Set the looking choices selection
        @param sel: int

        """
        self._panel.SetLookinSelection(sel)

    def SetReplaceBitmap(self, bmp):
        """Set the replace bitmap
        @param bmp: wx.Bitmap

        """
        self._box.SetReplaceBitmap(bmp)

    def SetFindDirectory(self, path):
        """Set the directory selection for find in files
        @param path: path to set for lookin data

        """
        self._panel.SetLookinSelection(path)

    def SetFindString(self, query):
        """Set the find controls search string
        @param query: string

        """
        self._panel.SetFindString(query)

    def SetReplaceString(self, rstring):
        """Set the replace controls search string
        @param rstring: string

        """
        self._panel.SetReplaceString(rstring)

#--------------------------------------------------------------------------#

class MiniFindReplaceDlg(wx.MiniFrame, FindReplaceDlgBase):
    """Advanced Find Replace Dialog this version of the dialog uses a 
    MiniFrame that will float on top of its parent

    """
    def __init__(self, parent, fdata, title, style=AFR_STYLE_FINDDIALOG):
        """Create the Dialog
        @param parent: Parent Window
        @param fdata: wx.FindReplaceData
        @param title: Dialog Title. Pass a single string to set the title for
                      both modes of the dialog or a tuple of two strings to set
                      the title for (find, replace) modes.
        @keyword style: Dialog Style

        """
        wx.MiniFrame.__init__(self, parent, wx.ID_ANY, u'',
                              style=wx.DEFAULT_DIALOG_STYLE)
        FindReplaceDlgBase.__init__(self, parent, fdata, title, style)

        # Event handlers
        self.Bind(wx.EVT_CLOSE, lambda evt: self._SendCloseEvent())

#--------------------------------------------------------------------------#

class FindReplaceDlg(wx.Dialog, FindReplaceDlgBase):
    """Advanced Find Replace Dialog this version of the dialog uses a standard
    dialog window.

    """
    def __init__(self, parent, fdata, title, style=AFR_STYLE_FINDDIALOG):
        """Create the Dialog
        @param parent: Parent Window
        @param fdata: wx.FindReplaceData
        @param title: Dialog Title can be a string to set the title for both
                      modes or a tuple of two strings to set the (find, replace)
                      mode titles.
        @keyword style: Dialog Style

        """
        wx.Dialog.__init__(self, parent, wx.ID_ANY, u'',
                           style=wx.DEFAULT_DIALOG_STYLE)
        FindReplaceDlgBase.__init__(self, parent, fdata, title, style)

        # Event handlers
        self.Bind(wx.EVT_CLOSE, lambda evt: self._SendCloseEvent())

#--------------------------------------------------------------------------#

class FindBox(ctrlbox.ControlBox):
    """Container box that allows for switching the L{FindPanel}'s mode
    through the ui. Contains a L{FindPanel} and two PlateButtons for switching
    the mode.

    """
    def __init__(self, parent, fdata, id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=AFR_STYLE_FINDDIALOG,
                 name=FindBoxName):
        """Create the container box
        @param fdata: wx.FindReplaceData

        """
        super(FindBox, self).__init__(parent, id, pos, size,
                                      wx.TAB_TRAVERSAL|wx.NO_BORDER, name)

        # Attributes
        self._fpanel = FindPanel(self, fdata, style=style)
        ctrlbar = ctrlbox.ControlBar(self, style=ctrlbox.CTRLBAR_STYLE_GRADIENT)
        bmp = wx.ArtProvider.GetBitmap(wx.ART_FIND, wx.ART_MENU)
        self.find = platebtn.PlateButton(ctrlbar, label=_("Find"), bmp=bmp,
                                         style=platebtn.PB_STYLE_NOBG)
        bmp = wx.ArtProvider.GetBitmap(wx.ART_FIND_AND_REPLACE, wx.ART_MENU)
        self.replace = platebtn.PlateButton(ctrlbar, label=_("Replace"),
                                            bmp=bmp,
                                            style=platebtn.PB_STYLE_NOBG)

        # Setup
        if wx.Platform == '__WXGTK__':
            ctrlbar.SetWindowStyle(ctrlbox.CTRLBAR_STYLE_BORDER_BOTTOM)

        ctrlbar.SetVMargin(2, 2)
        ctrlbar.AddControl(self.find, wx.ALIGN_LEFT)
        ctrlbar.AddControl(self.replace, wx.ALIGN_LEFT)
        self.SetControlBar(ctrlbar)
        self.SetWindow(self._fpanel)

        if style & AFR_STYLE_NO_MODE_SELECT:
            self.GetControlBar().Hide()

        # Event Handlers
        self.Bind(wx.EVT_BUTTON, self.OnButton)

    def OnButton(self, evt):
        """Change the mode
        @param evt: wx.EVT_BUTTON

        """
        eobj = evt.GetEventObject()
        if eobj in (self.find, self.replace):
            self.SetBoxMode(eobj == self.find)
        else:
            evt.Skip()

    def SetFindBitmap(self, bmp):
        """Set the bitmap of the Find Button
        @param bmp: wx.Bitmap

        """
        self.find.SetBitmap(bmp)
        self.GetControlBar().Layout()

    def SetBoxMode(self, find=True):
        """Set the box to find or find/replace mode
        @keyword find: set to find mode

        """
        self._fpanel.SetFindMode(find)
        self.Layout()
        evt = _AdvFindDlgInternalEvent(self.GetId(), _edEVT_MODE_CHANGE)
        wx.PostEvent(self.GetParent(), evt)

    def SetReplaceBitmap(self, bmp):
        """Set the bitmap of the Replace Button
        @param bmp: wx.Bitmap

        """
        self.replace.SetBitmap(bmp)
        self.GetControlBar().Layout()

#--------------------------------------------------------------------------#

class FindPanel(wx.Panel):
    """Find controls panel"""
    def __init__(self, parent, fdata, id=wx.ID_ANY, 
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=AFR_STYLE_FINDDIALOG, name=FindPanelName):
        """Create the panel
        @param fdata: wx.FindReplaceData

        """
        super(FindPanel, self).__init__(parent, id, pos, size,
                                        wx.TAB_TRAVERSAL|wx.NO_BORDER, name)

        # Attributes
        # TODO: change to editable combo box when wxMac has native widget
        #       so that we can set a search history to choose from.       
        self._mode = style
        self._ftxt = wx.TextCtrl(self, value=fdata.GetFindString())
        self._rtxt = wx.TextCtrl(self, value=fdata.GetReplaceString())
        locations = [_("Current Document"), _("Selected Text"),
                     _("Open Documents"), _("Current Directory")]
        self._lookin = wx.Choice(self, ID_LOOKIN, choices=locations)
        self._lookin.SetSelection(0)
        self._filterlbl = wx.StaticText(self, label=_("File Filters:"))
        self._filters = None # Created in __DoLayout
        self._sizers = dict()
        self._paths = dict()
        self._fdata = fdata
        self._lastSearch = u''

        # Layout
        self.__DoLayout()
        self.SetInitialSize()
        self._ConfigureControls()

        # Setup
        self.SetFindMode(not (self._mode & AFR_STYLE_REPLACEDIALOG))

        # Event Handlers
        self.Bind(wx.EVT_BUTTON, self.OnChooseDir, id=ID_CHOOSE_DIR)
        self.Bind(wx.EVT_BUTTON,
                  lambda evt: self.FireEvent(evt.GetId()) or evt.Skip())
        self.Bind(wx.EVT_CHECKBOX, self.OnOption)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnOption)
        self.Bind(wx.EVT_CHOICE, self.OnChoice, id=ID_LOOKIN)
        for bid in (wx.ID_FIND, wx.ID_REPLACE, ID_COUNT,
                    ID_FIND_ALL, ID_REPLACE_ALL):
            self.Bind(wx.EVT_UPDATE_UI, self.OnUpdateUI, id=bid)
        self.Bind(wx.EVT_SET_FOCUS, lambda evt: self.__SetFocus())
        self._ftxt.Bind(wx.EVT_SET_FOCUS, lambda evt: self._ftxt.SelectAll())
        self._rtxt.Bind(wx.EVT_SET_FOCUS, lambda evt: self._rtxt.SelectAll())

        # Key handling HACK for windows
        if wx.Platform == '__WXMSW__':
            for child in self.GetChildren():
                child.Bind(wx.EVT_KEY_UP, self._OnKeyUp)

    def __SetFocus(self):
        """Set the focus to the find box"""
        self._ftxt.SetFocus()
        self._ftxt.SelectAll()

    def __DoLayout(self):
        """Layout the panel"""
        hsizer = wx.BoxSizer(wx.HORIZONTAL)
        vsizer = wx.BoxSizer(wx.VERTICAL)

        # Top Section
        topvsizer = wx.BoxSizer(wx.VERTICAL)

        # Platform dependant labels
        if wx.Platform == '__WXMSW__':
            findlbl = wx.StaticText(self, ID_FIND_LBL, _("Find what") + u":")
        else:
            findlbl = wx.StaticText(self, ID_FIND_LBL, _("Find") + u":")

        # Search Field
        fhsizer = wx.BoxSizer(wx.HORIZONTAL)
        fhsizer.Add(self._ftxt, 1, wx.EXPAND)
        topvsizer.AddMany([(findlbl, 0, wx.ALIGN_LEFT), ((3, 3), 0),
                          (fhsizer, 0, wx.EXPAND)])

        # Replace field
        rhsizer = wx.BoxSizer(wx.HORIZONTAL)
        rhsizer.Add(self._rtxt, 1, wx.EXPAND)
        rlbl = wx.StaticText(self, ID_REPLACE_LBL, _("Replace with") + u":")
        self._sizers[ID_REPLACE_LBL] = wx.BoxSizer(wx.VERTICAL)
        self._sizers[ID_REPLACE_LBL].AddMany([((5, 5), 0),
                                             (rlbl, 0, wx.ALIGN_CENTER_VERTICAL),
                                             ((3, 3), 0), (rhsizer, 0, wx.EXPAND)])
        topvsizer.AddMany([(self._sizers[ID_REPLACE_LBL], 0, wx.EXPAND),
                           ((5, 5), 0)])

        # Look in field
        self._sizers['look'] = wx.BoxSizer(wx.VERTICAL)
        li_sz = wx.BoxSizer(wx.HORIZONTAL)
        dirbtn = wx.Button(self, ID_CHOOSE_DIR, u"...", style=wx.BU_EXACTFIT)
        dirbtn.SetToolTipString(_("Choose Folder"))
        li_sz.AddMany([(self._lookin, 1, wx.ALIGN_CENTER_VERTICAL), ((5, 5), 0),
                       (dirbtn, 0, wx.ALIGN_CENTER_VERTICAL)])
        li_lbl = wx.StaticText(self, label=_("Look in") + u":")
        self._sizers['look'].AddMany([(li_lbl, 0, wx.ALIGN_LEFT),
                                      ((3, 3), 0),
                                      (li_sz, 0, wx.EXPAND),
                                      ((5, 5), 0)])
        topvsizer.Add(self._sizers['look'], 0, wx.EXPAND)

        # Search Direction Box
        self._sizers['dir'] = wx.BoxSizer(wx.VERTICAL)
        dbox = wx.StaticBox(self, id=ID_DIR_BOX, label=_("Direction"))
        dboxsz = wx.StaticBoxSizer(dbox, wx.HORIZONTAL)
        dboxsz.AddMany([(wx.RadioButton(self, wx.ID_UP, _("Up")), 0),
                        ((20, 5), 0),
                        (wx.RadioButton(self, wx.ID_DOWN, _("Down")), 0),
                        ((5, 5), 1)])
        self._sizers['dir'].AddMany([((5, 5), 0), (dboxsz, 0, wx.EXPAND)])

        # Search Options Box
        self._sizers['opt'] = wx.BoxSizer(wx.VERTICAL)
        statbox = wx.StaticBox(self, label=_("Find Options"))
        sboxsz = wx.StaticBoxSizer(statbox, wx.VERTICAL)
        for cid, clbl in [(ID_MATCH_CASE, _("Match case")),
                          (ID_WHOLE_WORD, _("Whole word")),
                          (ID_REGEX, _("Regular expression")),
                          (ID_RECURSE, _("Search Recursively"))]:
            sboxsz.AddMany([((3, 3), 0), (wx.CheckBox(self, cid, clbl), 0)])

        # File Filters
        self._sizers['filter'] = wx.BoxSizer(wx.VERTICAL)
        self._filters = wx.TextCtrl(self)
        tt_txt = _("Enter wildcard shell patterns for matching files (*.txt).")
        self._filters.SetToolTipString(tt_txt)

        # Disable spell checking on mac for this control
        if wx.Platform == '__WXMAC__':
            self._filters.MacCheckSpelling(False)

        f_sz = wx.BoxSizer(wx.HORIZONTAL)
        f_sz.Add(self._filters, 1, wx.EXPAND)
        self._sizers['filter'].AddMany([(self._filterlbl, 0, wx.ALIGN_LEFT),
                                        ((3, 3), 0), (f_sz, 0, wx.EXPAND),
                                        ((5, 5), 0)])
        sboxsz.AddMany([((3, 3), 0), (self._sizers['filter'], 0, wx.EXPAND)])

        self._sizers['opt'].AddMany([((5, 5), 0), (sboxsz, 0, wx.EXPAND)])

        # Buttons
        bsizer = wx.BoxSizer(wx.HORIZONTAL)
        self._sizers['fspacer'] = bsizer.Add((100, 1), 1)
        self._sizers['frspacer'] = bsizer.Add((50, 1), 1)
        for bid, blbl in [(wx.ID_FIND, _("Find")),
                          (wx.ID_REPLACE, _("Replace")),
                          (ID_COUNT, _("Count")),
                          (ID_FIND_ALL, _("Find All")),
                          (ID_REPLACE_ALL, _("Replace All"))]:
            self._sizers[bid] = wx.BoxSizer(wx.HORIZONTAL)
            self._sizers[bid].Add((3, 3), 0)
            self._sizers[bid].Add(wx.Button(self, bid, blbl), 0, wx.ALIGN_RIGHT)
            bsizer.Add(self._sizers[bid], 0)
        self.FindWindowById(wx.ID_FIND).SetDefault()

        # Final Layout
        vsizer.AddMany([((5, 5), 0), (topvsizer, 0, wx.EXPAND),
                        (self._sizers['dir'], 0, wx.EXPAND),
                        (self._sizers['opt'], 0, wx.EXPAND), ((10, 10), 0),
                        (bsizer, 0), ((10, 10), 0)])
        hsizer.AddMany([((10, 10), 0), (vsizer, 0, wx.EXPAND), ((10, 10), 0)])
        self.SetSizer(hsizer)
        self.SetAutoLayout(True)

    def _ConfigureControls(self):
        """Configure the state of the controls based on the FindReplaceData"""
        flags = self._fdata.GetFlags()
        self._ftxt.SelectAll()
        self.FindWindowById(ID_MATCH_CASE).SetValue(flags & AFR_MATCHCASE)
        self.FindWindowById(ID_WHOLE_WORD).SetValue(flags & AFR_WHOLEWORD)
        self.FindWindowById(ID_REGEX).SetValue(flags & AFR_REGEX)
        self.FindWindowById(wx.ID_DOWN).SetValue(not (flags & AFR_UP))
        self.FindWindowById(wx.ID_UP).SetValue(flags & AFR_UP)
        self.ShowLookinCombo(not (flags & AFR_NOLOOKIN))
        self.ShowDirectionBox(not (flags & AFR_NOUPDOWN))
        self.ShowOptionsBox(not (flags & AFR_NOOPTIONS))
        self.FindWindowById(ID_WHOLE_WORD).Enable(not (flags & AFR_NOWHOLEWORD))
        self.FindWindowById(ID_MATCH_CASE).Enable(not (flags & AFR_NOMATCHCASE))
        self.FindWindowById(ID_REGEX).Enable(not (flags & AFR_NOREGEX))
        in_files = bool(self._lookin.GetSelection() >= LOCATION_MAX)
        recurse = self.FindWindowById(ID_RECURSE)
        recurse.SetValue(flags & AFR_RECURSIVE)
        recurse.Enable(in_files)
        self.ShowFileFilters(in_files and not (flags & AFR_NOFILTER))

    def _OnKeyUp(self, evt):
        """Check if the dialog should be closed
        @note: msw only

        """
        if evt.GetKeyCode() == wx.WXK_ESCAPE:
            evt = _AdvFindDlgInternalEvent(self.GetId(), _edEVT_DO_CLOSE_DLG)
            wx.PostEvent(self.GetTopLevelParent(), evt)
        evt.Skip()

    def _ShowButtons(self, find=True):
        """Toggle the visiblity of a button set
        @param find: Show Find Buttons or Show Replace Buttons

        """
        if find:
            show = [wx.ID_FIND, ID_COUNT, ID_FIND_ALL, 'fspacer']
            hide = [wx.ID_REPLACE, ID_REPLACE_ALL, 'frspacer']
        else:
            show = [wx.ID_REPLACE, ID_REPLACE_ALL, 'frspacer']
            hide = [ID_FIND_ALL, ID_COUNT, 'fspacer']

        # Hide extra buttons as per configured preference
        flags = self._fdata.GetFlags()
        if flags & AFR_NO_COUNT and ID_COUNT in show:
            show.remove(ID_COUNT)
            hide.append(ID_COUNT)

        if flags & AFR_NO_ALL_BTN:
            if ID_FIND_ALL in show:
                show.remove(ID_FIND_ALL)
                hide.append(ID_FIND_ALL)
            if ID_REPLACE_ALL in show:
                show.remove(ID_REPLACE_ALL)
                hide.append(ID_REPLACE_ALL)

        for ctrl in show:
            if isinstance(ctrl, basestring):
                self._sizers[ctrl].Show(True)
            else:
                self._sizers[ctrl].ShowItems(True)

        for ctrl in hide:
            if isinstance(ctrl, basestring):
                self._sizers[ctrl].Show(False)
            else:
                self._sizers[ctrl].ShowItems(False)

    def _UpdateContext(self):
        """Update available dialog options based on the selected search context

        """
        self._UpdateDefaultBtn()
        in_files = bool(self._lookin.GetSelection() >= LOCATION_MAX)
        self.FindWindowById(ID_RECURSE).Enable(in_files)
        flags = self._fdata.GetFlags()

        # Disable direction settings when searcing in file since they
        # are ignored anyway.
        for cid in (wx.ID_UP, wx.ID_DOWN, ID_DIR_BOX):
            self.FindWindowById(cid).Enable(not in_files)

        # Only update visibility of file filter field if it is enabled
        if not (flags & AFR_NOFILTER):
            self.ShowFileFilters(in_files)

    def _UpdateDefaultBtn(self):
        """Change the default button depending on what the search context
        has been changed to.

        """
        find = self.FindWindowById(wx.ID_FIND)
        find_all = self.FindWindowById(ID_FIND_ALL)
        replace = self.FindWindowById(wx.ID_REPLACE)
        replace_all = self.FindWindowById(ID_REPLACE_ALL)
        count = self.FindWindowById(ID_COUNT)

        lookin = self._lookin.GetSelection()
        if  lookin > LOCATION_IN_SELECTION:
            if self._mode == AFR_STYLE_FINDDIALOG:
                find_all.SetDefault()
            else:
                replace_all.SetDefault()
            find.Disable()
            replace.Disable()
            count.Disable()
        elif lookin == LOCATION_IN_SELECTION:
            find.Disable()
            replace.Disable()
            replace_all.SetDefault()
        else:
            find.SetDefault()
            find.Enable()
            replace.Enable()
            count.Enable()

    #------------------------------------------------------#

    def AddLookinPath(self, path):
        """Add a path to the lookin path collection
        @param path: string
        @return: index of the items location

        """
        if not len(path):
            return None

        # If its a path we already have then just return its index
        if path in self._paths.values():
            for idx, pname in self._paths.iteritems():
                if path == pname:
                    return idx

        # Get the short directory name
        the_dir = u''
        for dname in reversed(path.split(os.sep)):
            if len(dname):
                the_dir = dname
                break

        self._paths[self._lookin.GetCount()] = path
        self._lookin.Append(the_dir)
        rval = self._lookin.GetCount() - 1

        return rval

    def ClearFlag(self, flag):
        """Clear a search flag
        @param flag: AFR_*

        """
        flags = self._fdata.GetFlags()
        flags &= ~flag
        self.SetFlags(flags)

    def FireEvent(self, eid):
        """Fire an event
        @param eid: Event id

        """
        etype = _EVENT_MAP.get(eid, None)
        query = self._ftxt.GetValue()
        if eid == wx.ID_FIND:
            if self._lastSearch == query:
                etype = edEVT_FIND_NEXT
            self._lastSearch = query
            self._fdata.SetFindString(query)

        if etype is not None:
            evt = FindEvent(etype, eid, self._fdata.GetFlags())
            evt.SetEventObject(self)
            lookin_idx = self._lookin.GetSelection()
            stype = min(LOCATION_IN_FILES, max(LOCATION_CURRENT_DOC, lookin_idx))
            evt.SetSearchType(stype)
            evt.SetFindString(query)

            if self._mode & AFR_STYLE_REPLACEDIALOG:
                rstring = self._rtxt.GetValue()
                self._fdata.SetReplaceString(rstring)
                evt.SetReplaceString(rstring)
            else:
                evt.SetReplaceString(None)

            if stype >= LOCATION_IN_FILES:
                evt.SetDirectory(self._paths.get(lookin_idx, u''))
                evt.SetFileFilters(self._filters.GetValue())

            wx.PostEvent(self.GetParent(), evt)
            return True
        else:
            return False

    def GetData(self):
        """Get the FindReplaceData used by this panel
        @return: wx.FindReplaceData

        """
        return self._fdata

    def GetFileFilters(self):
        """Get the currently set file filters
        @return: string

        """
        return self._filters.GetValue()

    def GetLookinChoices(self):
        """Get the looking choices
        @return: list of strings

        """
        choices = list()
        for key in sorted(self._paths.keys()):
            choices.append(self._paths[key])
        return choices

    def GetLookinSelection(self):
        """Get the index of the currently selected lookin choice
        @return: int

        """
        return self._lookin.GetSelection()

    def GetPanelMode(self):
        """Get the current display mode of the panel
        @return: AFR_STYLE_FINDDIALOG or AFR_STYLE_REPLACEDIALOG

        """
        return self._mode

    def OnChoice(self, evt):
        """Handle choice control events
        @param evt: wx.EVT_CHOICE

        """
        e_id = evt.GetId()
        if e_id == ID_LOOKIN:
            self._UpdateContext()
            choice = self._lookin.GetSelection()
            if choice >= LOCATION_IN_FILES:
                tts = self._paths.get(choice, u'')
                self._lookin.SetToolTipString(tts)
            else:
                self._lookin.SetToolTipString(u'')
        else:
            evt.Skip()

    def OnChooseDir(self, evt):
        """Open the choose directory dialog for selecting what
        path to do a search in files in.
        @param evt: wx.EVT_BUTTON

        """
        if evt.GetId() == ID_CHOOSE_DIR:
            dlg = wx.DirDialog(self, _("Choose Search Folder"))
            if dlg.ShowModal() == wx.ID_OK:
                path = dlg.GetPath()
                if path is not None and len(path):
                    idx = self.AddLookinPath(path)
                    self.SetLookinSelection(idx)
                self._UpdateContext()
            dlg.Destroy()
        else:
            evt.Skip()

    def OnOption(self, evt):
        """Update search flags
        @param evt: wx.EVT_CHECKBOX

        """
        fmap = { ID_MATCH_CASE : AFR_MATCHCASE,
                 ID_WHOLE_WORD : AFR_WHOLEWORD,
                 ID_REGEX : AFR_REGEX,
                 ID_RECURSE : AFR_RECURSIVE,
                 wx.ID_UP : AFR_UP }
        eid = evt.GetId()
        eobj = evt.GetEventObject()
        if eid in fmap:
            if eobj.GetValue():
                self.SetFlag(fmap[eid])
            else:
                self.ClearFlag(fmap[eid])
            self.FireEvent(ID_OPTION_CHANGE)
        elif eid == wx.ID_DOWN:
            self.ClearFlag(fmap[wx.ID_UP])
            self.FireEvent(ID_OPTION_CHANGE)
        else:
            evt.Skip()

    def OnUpdateUI(self, evt):
        """Enable and disable buttons depending on state of find entry box.
        @param evt: wx.UpdateUIEvent

        """
        # TODO: support count in files?
        if evt.GetId() in (wx.ID_FIND, ID_COUNT, wx.ID_REPLACE) and \
           self._lookin.GetSelection() > LOCATION_CURRENT_DOC:
            evt.Enable(False)
        else:
            txt = len(self._ftxt.GetValue())
            evt.Enable(txt)

    def SetFindMode(self, find=True):
        """Set the mode of the dialog Replace
        @param find: Set Find Mode or Replace Mode

        """
        self._rtxt.Show(not find)
        self._sizers[ID_REPLACE_LBL].ShowItems(not find)
        if find:
            self._mode = AFR_STYLE_FINDDIALOG
        else:
            self._mode = AFR_STYLE_REPLACEDIALOG

        self._UpdateDefaultBtn()
        self._ShowButtons(find)
        self.Layout()

    def SetFindString(self, query):
        """Set the find fields string
        @param query: string

        """
        self._ftxt.SetValue(query)
        self._fdata.SetFindString(query)

    def RefreshControls(self):
        """Refresh the state of the controls from the current FindData."""
        self._ConfigureControls()

    def SetData(self, data):
        """Set the FindReplaceData and update the dialog with that data
        @param data: wxFindReplaceData

        """
        self._fdata.Destroy()
        self._fdata = None
        self._fdata = data
        self._ConfigureControls()

    def SetFileFilters(self, filters):
        """Set the file filters field values
        @param filters: string

        """
        self._filters.SetValue(filters)

    def SetFlag(self, flag):
        """Set a search flag
        @param flag: AFR_* flag value

        """
        flags = self._fdata.GetFlags()
        flags |= flag
        self.SetFlags(flags)

    def SetFlags(self, flags):
        """Set the search flags
        @param flags: Bitmask of AFR_* values

        """
        self._fdata.SetFlags(flags)
        self._ConfigureControls()

    def SetLookinChoices(self, paths):
        """Set the looking choices
        @param paths: list of strings

        """
        for path in paths:
            self.AddLookinPath(path)

    def SetLookinSelection(self, idx):
        """Set the selection of the lookin control.
        @param idx: int

        """
        if idx <= self._lookin.GetCount():
            self._lookin.SetSelection(idx)

            # If the selection is a Look in Files make sure the filter
            # field is shown
            flags = self._fdata.GetFlags()
            in_files = bool(idx >= LOCATION_MAX)
            self.ShowFileFilters(in_files and not (flags & AFR_NOFILTER))

    def SetReplaceString(self, rstring):
        """Set the replace fields string
        @param rstring: string

        """
        self._rtxt.SetValue(rstring)
        self._fdata.SetReplaceString(rstring)

    def ShowDirectionBox(self, show=True):
        """Show or hide the Direction group box
        @keyword show: bool

        """
        if 'dir' in self._sizers:
            self._sizers['dir'].ShowItems(show)
            self.Layout()

    def ShowFileFilters(self, show=True):
        """Show or hide the File Filters filed
        @keyword show: bool

        """
        if 'filter' in self._sizers:
            self._sizers['filter'].ShowItems(show)
            self.Layout()

            # Require additional resizing
            evt = _AdvFindDlgInternalEvent(self.GetId(), _edEVT_MODE_CHANGE)
            wx.PostEvent(self.GetTopLevelParent(), evt)

    def ShowLookinCombo(self, show=True):
        """Show the lookin choice and directory chooser control
        @keyword show: bool

        """
        if 'look' in self._sizers:
            self._sizers['look'].ShowItems(show)
            self.Layout()

    def ShowOptionsBox(self, show=True):
        """Show the find options group box
        @keyword show: bool

        """
        if 'opt' in self._sizers:
            self._sizers['opt'].ShowItems(show)
            self.Layout()

#--------------------------------------------------------------------------#
# Private Module Api

_edEVT_MODE_CHANGE = wx.NewEventType()
_EVT_MODE_CHANGE = wx.PyEventBinder(_edEVT_MODE_CHANGE, 1)

_edEVT_DO_CLOSE_DLG = wx.NewEventType()
_EVT_DO_CLOSE_DLG = wx.PyEventBinder(_edEVT_DO_CLOSE_DLG, 1)

class _AdvFindDlgInternalEvent(wx.PyEvent):
    def __init__(self, winid=wx.ID_ANY, etype=wx.wxEVT_NULL):
        """Create the Event"""
        wx.PyEvent.__init__(self, winid, etype)

#--------------------------------------------------------------------------#
