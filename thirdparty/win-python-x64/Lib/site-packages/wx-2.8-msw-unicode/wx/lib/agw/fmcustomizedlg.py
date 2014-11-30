"""
This module contains a custom dialog class used to personalize the appearance of a
L{FlatMenu} on the fly, allowing also the user of your application to do the same.
"""

import wx
from UserDict import UserDict

from artmanager import ArtManager
from fmresources import *
from labelbook import LabelBook

_ = wx.GetTranslation

# ---------------------------------------------------------------------------- #
# Class OrderedDict
# ---------------------------------------------------------------------------- #

class OrderedDict(UserDict):
    """
    An ordered dictionary implementation.
    """

    def __init__(self, dict = None):
        self._keys = []
        UserDict.__init__(self, dict)

    def __delitem__(self, key):
        UserDict.__delitem__(self, key)
        self._keys.remove(key)

    def __setitem__(self, key, item):
        UserDict.__setitem__(self, key, item)
        if key not in self._keys: self._keys.append(key)

    def clear(self):
        UserDict.clear(self)
        self._keys = []

    def copy(self):
        dict = UserDict.copy(self)
        dict._keys = self._keys[:]
        return dict

    def items(self):
        return zip(self._keys, self.values())

    def keys(self):
        return self._keys

    def popitem(self):
        try:
            key = self._keys[-1]
        except IndexError:
            raise KeyError('dictionary is empty')

        val = self[key]
        del self[key]

        return (key, val)

    def setdefault(self, key, failobj = None):
        UserDict.setdefault(self, key, failobj)
        if key not in self._keys: self._keys.append(key)

    def update(self, dict):
        UserDict.update(self, dict)
        for key in dict.keys():
            if key not in self._keys: self._keys.append(key)

    def values(self):
        return map(self.get, self._keys)


# ---------------------------------------------------------------------------- #
# Class FMTitlePanel
# ---------------------------------------------------------------------------- #

class FMTitlePanel(wx.Panel):
    """
    Helper class to draw gradient shadings on the dialog.
    """

    def __init__(self, parent, title):
        """
        Default class constructor.

        :param `parent`: the L{FMTitlePanel} parent;
        :param `title`: the string to use as a dialog title.
        """

        wx.Panel.__init__(self, parent)
        self._title = title

        # Set the panel size
        dc = wx.MemoryDC()
        dc.SelectObject(wx.EmptyBitmap(1, 1))
        dc.SetFont(wx.SystemSettings_GetFont( wx.SYS_DEFAULT_GUI_FONT ))
        
        ww, hh = dc.GetTextExtent("Tp")
        dc.SelectObject(wx.NullBitmap)
        
        # Set minimum panel size
        if ww < 250:
            ww = 250

        self.SetSize(wx.Size(ww, hh + 10))

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)


    def OnEraseBackground(self, event):
        """
        Handles the ``wx.EVT_ERASE_BACKGROUND`` event for L{FMTitlePanel}.

        :param `event`: a `wx.EraseEvent` event to be processed.

        :note: This method is intentionally empty to reduce flicker.        
        """

        pass        


    def OnPaint(self, event):
        """
        Handles the ``wx.EVT_PAINT`` event for L{FMTitlePanel}.

        :param `event`: a `wx.PaintEvent` event to be processed.
        """
        
        dc = wx.BufferedPaintDC(self)

        # Draw the background
        colour1 = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
        colour2 = ArtManager.Get().LightColour(colour1, 70)
        ArtManager.Get().PaintStraightGradientBox(dc, self.GetClientRect(), colour1, colour2, False)

        # Draw the text
        font = wx.SystemSettings_GetFont(wx.SYS_DEFAULT_GUI_FONT)
        font.SetWeight(wx.BOLD)
        dc.SetFont(font)
        dc.SetTextForeground(wx.BLACK)
        dc.DrawText(self._title, 5, 5)


# ---------------------------------------------------------------------------- #
# Class FMCustomizeDlg
# ---------------------------------------------------------------------------- #

class FMCustomizeDlg(wx.Dialog):
    """
    Class used to customize the appearance of L{FlatMenu} and L{FlatMenuBar}.
    """

    def __init__(self, parent=None):
        """
        Default class constructor.

        :param `parent`: the L{FMCustomizeDlg} parent window.
        """
        
        self._book = None

        if not parent:
            wx.Dialog.__init__(self)
            return
    
        wx.Dialog.__init__(self, parent, wx.ID_ANY, _("Customize"), wx.DefaultPosition,
                           wx.DefaultSize, wx.DEFAULT_DIALOG_STYLE)

        self._visibleMenus = OrderedDict()
        self._hiddenMenus = OrderedDict()

        self.CreateDialog()
        self.ConnectEvents()
        self.GetSizer().Fit(self)
        self.GetSizer().SetSizeHints(self)
        self.GetSizer().Layout()
        self.Centre()


    def CreateDialog(self):
        """ Actually creates the dialog. """

        sz = wx.BoxSizer(wx.VERTICAL)
        self.SetSizer(sz)

        # Create the main book and add some pages into it
        style = INB_NO_RESIZE | INB_LEFT | INB_DRAW_SHADOW | INB_BORDER
        self._book = LabelBook(self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, style)
        sz.Add(self._book, 1, wx.EXPAND)

        self._book.SetColour(INB_TAB_AREA_BACKGROUND_COLOUR, ArtManager.Get().GetMenuFaceColour())

        colour = wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE)
        self._book.SetColour(INB_ACTIVE_TAB_COLOUR, colour)

        self.created = False
        self.Initialise()
        hsizer = wx.BoxSizer(wx.HORIZONTAL)

        # add a separator between the book & the buttons area
        hsizer.Add(wx.Button(self, wx.ID_OK, _("&Close")), 0, wx.EXPAND | wx.ALIGN_RIGHT)
        sz.Add(wx.StaticLine(self), 0, wx.EXPAND | wx.TOP | wx.BOTTOM, 3)
        sz.Add(hsizer, 0, wx.ALIGN_RIGHT | wx.ALL, 2)


    def Initialise(self):
        """ Initialzes the L{LabelBook} pages. """
    
        self._book.DeleteAllPages()
        self._book.AddPage(self.CreateMenusPage(), _("Menus"), True)
        self._book.AddPage(self.CreateOptionsPage(), _("Options"), False)
    

    def CloseDialog(self):
        """ Closes the dialog. """
    
        self.EndModal(wx.ID_OK)
    

    def ConnectEvents(self):
        """ Does nothing at the moment. """

        pass        
    
    
    def CreateMenusPage(self):
        """ Creates the L{LabelBook} pages with L{FlatMenu} information. """
    
        menus = wx.Panel(self._book, wx.ID_ANY, wx.DefaultPosition, wx.Size(300, 300))
        sz = wx.BoxSizer(wx.VERTICAL)
        menus.SetSizer(sz)

        choices = []
        
        mb = self.GetParent()

        if not self.created:
            self.order = []
        
        # Add all the menu items that are currently visible to the list
        for i in xrange(len(mb._items)):
        
            dummy, lableOnly = ArtManager.Get().GetAccelIndex(mb._items[i].GetTitle())
            choices.append(lableOnly)

            # Add the menu to the visible menus map
            self._visibleMenus.update({lableOnly: mb._items[i].GetMenu()})
            if not self.created:
                self.order.append(lableOnly)
        
        # Add all hidden menus to the menu bar

        for key in self._hiddenMenus.keys():
            choices.append(key)

        if self.created:
            visible = OrderedDict()
            hidden = OrderedDict()
            for items in self.order:
                if items in self._visibleMenus:
                    visible[items] = self._visibleMenus[items]
                elif items in self._hiddenMenus:
                    hidden[items] = self._hiddenMenus[items]

            self._visibleMenus = visible
            self._hiddenMenus = hidden

        self._menuListId = wx.NewId()
        self._checkListMenus = wx.CheckListBox(menus, self._menuListId, pos=wx.DefaultPosition, size=wx.Size(250, 250),
                                               choices=self.order, style=wx.BORDER_SIMPLE)
        self._checkListMenus.Bind(wx.EVT_CHECKLISTBOX, self.OnMenuChecked)

        # check all visible items
        for indx, item in enumerate(self.order):
            if item in self._visibleMenus:
                self._checkListMenus.Check(indx)

        # Add title panel
        title = FMTitlePanel(menus, _("Select Menu To Add/Remove:"))
        sz.Add(title, 0, wx.EXPAND | wx.ALL, 2)
        sz.Add(self._checkListMenus, 1, wx.EXPAND | wx.TOP | wx.RIGHT | wx.LEFT, 2)

        self.created = True
        
        return menus
    

    def CreateShortcutsPage(self):
        """ Creates the L{LabelBook} shorcuts page. """
    
        shorcuts = wx.Panel(self._book, wx.ID_ANY, wx.DefaultPosition, wx.Size(300, 300))
        return shorcuts
    

    def CreateOptionsPage(self):
        """ Creates the L{LabelBook} option page which holds the L{FlatMenu} styles. """
    
        options = wx.Panel(self._book, wx.ID_ANY, wx.DefaultPosition, wx.Size(300, 300))

        # Create some options here
        vsizer = wx.BoxSizer(wx.VERTICAL)
        options.SetSizer(vsizer)

        #-----------------------------------------------------------
        # options page layout 
        # - Menu Style: Default or 2007 (radio group)
        #
        # - Default Style Settings:     (static box)
        #     + Draw vertical gradient  (check box)
        #     + Draw border             (check box)
        #     + Drop toolbar shadow     (check box)
        #
        # - Colour Scheme                   (static box)
        #     + Menu bar background colour  (combo button)
        #-----------------------------------------------------------

        self._menuStyleID = wx.NewId()
        choices =  [_("Default Style"), _("Metallic")]
        self._menuStyle = wx.RadioBox(options, self._menuStyleID, _("Menu bar style"),
                                      wx.DefaultPosition, wx.DefaultSize, choices)

        # update the selection
        theme = ArtManager.Get().GetMenuTheme()

        if theme == Style2007:
            self._menuStyle.SetSelection(1)
        else:
            self._menuStyle.SetSelection(0)

        # connect event to the control
        self._menuStyle.Bind(wx.EVT_RADIOBOX, self.OnChangeStyle)
        
        vsizer.Add(self._menuStyle, 0, wx.EXPAND | wx.ALL, 5)

        self._sbStyle = wx.StaticBoxSizer(wx.StaticBox(options, -1, _("Default style settings")), wx.VERTICAL)
        self._drawVertGradID = wx.NewId()
        self._verticalGradient = wx.CheckBox(options, self._drawVertGradID, _("Draw vertical gradient"))
        self._verticalGradient.Bind(wx.EVT_CHECKBOX, self.OnChangeStyle)
        self._sbStyle.Add(self._verticalGradient, 0, wx.EXPAND | wx.ALL, 3)
        self._verticalGradient.SetValue(ArtManager.Get().GetMBVerticalGradient())

        self._drawBorderID = wx.NewId()
        self._drawBorder = wx.CheckBox(options, self._drawBorderID, _("Draw border around menu bar"))
        self._drawBorder.Bind(wx.EVT_CHECKBOX, self.OnChangeStyle)
        self._sbStyle.Add(self._drawBorder, 0, wx.EXPAND | wx.ALL, 3)
        self._drawBorder.SetValue(ArtManager.Get().GetMenuBarBorder())
        
        self._shadowUnderTBID = wx.NewId()
        self._shadowUnderTB =  wx.CheckBox(options, self._shadowUnderTBID, _("Toolbar float over menu bar"))
        self._shadowUnderTB.Bind(wx.EVT_CHECKBOX, self.OnChangeStyle)
        self._sbStyle.Add(self._shadowUnderTB, 0, wx.EXPAND | wx.ALL, 3)
        self._shadowUnderTB.SetValue(ArtManager.Get().GetRaiseToolbar())

        vsizer.Add(self._sbStyle, 0, wx.EXPAND | wx.ALL, 5)

        # Misc 
        sb = wx.StaticBoxSizer(wx.StaticBox(options, -1, _("Colour Scheme")), wx.VERTICAL)
        self._colourID = wx.NewId()

        colourChoices = ArtManager.Get().GetColourSchemes()
        colourChoices.sort()

        self._colour = wx.ComboBox(options, self._colourID, ArtManager.Get().GetMenuBarColourScheme(), choices=colourChoices,
                                   style=wx.CB_DROPDOWN | wx.CB_READONLY)
        sb.Add(self._colour, 0, wx.EXPAND)
        vsizer.Add(sb, 0, wx.EXPAND | wx.ALL, 5)
        self._colour.Bind(wx.EVT_COMBOBOX, self.OnChangeStyle)

        # update the dialog by sending all possible events to us
        event = wx.CommandEvent(wx.wxEVT_COMMAND_RADIOBOX_SELECTED, self._menuStyleID)
        event.SetEventObject(self)
        event.SetInt(self._menuStyle.GetSelection())
        self._menuStyle.ProcessEvent(event)

        event.SetEventType(wx.wxEVT_COMMAND_CHECKBOX_CLICKED)
        event.SetId(self._drawVertGradID)
        event.SetInt(ArtManager.Get().GetMBVerticalGradient())
        self._verticalGradient.ProcessEvent(event)

        event.SetEventType(wx.wxEVT_COMMAND_CHECKBOX_CLICKED)
        event.SetId(self._shadowUnderTBID)
        event.SetInt(ArtManager.Get().GetRaiseToolbar())
        self._shadowUnderTB.ProcessEvent(event)

        event.SetEventType(wx.wxEVT_COMMAND_CHECKBOX_CLICKED)
        event.SetId(self._drawBorderID)
        event.SetInt(ArtManager.Get().GetMenuBarBorder())
        self._drawBorder.ProcessEvent(event)

        event.SetEventType(wx.wxEVT_COMMAND_COMBOBOX_SELECTED)
        event.SetId(self._colourID)
        self._colour.ProcessEvent(event)

        return options
    

    def OnMenuChecked(self, event):
        """
        Handles the ``wx.EVT_CHECKBOX`` event for L{FMCustomizeDlg}.

        :param `event`: a `wx.CommandEvent` event to be processed.

        :note: This method handles the L{FlatMenu} menus visibility.
        """
    
        id = event.GetInt()
        checked = self._checkListMenus.IsChecked(id)
        menuName = self._checkListMenus.GetString(id)
        menu = None
        mb = self.GetParent()

        if checked:
        
            # remove the item from the hidden map
            if self._hiddenMenus.has_key(menuName):
                menu = self._hiddenMenus.pop(menuName)
            
            # add it to the visible map
            if menu:
                self._visibleMenus.update({menuName: menu})

            indx = self._checkListMenus.GetItems().index(menuName)
            # update the menubar
            mb.Insert(indx, menu, menu._menuBarFullTitle)
            mb.Refresh()

        else:
        
            # remove the item from the visible items
            if self._visibleMenus.has_key(menuName):
                menu = self._visibleMenus.pop(menuName)

            # add it to the hidden map
            if menu:
                self._hiddenMenus.update({menuName: menu})
            
            # update the menubar
            pos = mb.FindMenu(menuName)
            if pos != wx.NOT_FOUND:
                mb.Remove(pos)
            
            mb.Refresh()

        if self.created:
            visible = OrderedDict()
            hidden = OrderedDict()
            for items in self.order:
                if items in self._visibleMenus:
                    visible[items] = self._visibleMenus[items]
                elif items in self._hiddenMenus:
                    hidden[items] = self._hiddenMenus[items]

            self._visibleMenus = visible
            self._hiddenMenus = hidden
        
    
    def OnChangeStyle(self, event):
        """
        Handles the ``wx.EVT_CHECKBOX`` event for L{FMCustomizeDlg}.

        :param `event`: a `wx.CommandEvent` event to be processed.

        :note: This method handles the L{FlatMenu} styles.        
        """

        mb = self.GetParent()
        
        if event.GetId() == self._menuStyleID:
        
            if event.GetSelection() == 0:
            
                # Default style
                ArtManager.Get().SetMenuTheme(StyleXP)
                self._drawBorder.Enable()
                self._verticalGradient.Enable()
                mb.Refresh()
            
            else:
            
                ArtManager.Get().SetMenuTheme(Style2007)
                self._drawBorder.Enable(False)
                self._verticalGradient.Enable(False)            
                mb.Refresh()
            
            return
        
        if event.GetId() == self._drawBorderID:
        
            ArtManager.Get().DrawMenuBarBorder(event.IsChecked())
            mb.Refresh()
            return
        
        if event.GetId() == self._drawVertGradID:
        
            ArtManager.Get().SetMBVerticalGradient(event.IsChecked())
            mb.Refresh()
            return

        if event.GetId() == self._colourID:

            selection = _("Default")
            sel = self._colour.GetSelection()
            if sel != wx.NOT_FOUND:
                # select new colour scheme
                selection = self._colour.GetStringSelection()

            ArtManager.Get().SetMenuBarColour(selection)
            mb.Refresh()
            return

        if event.GetId() == self._shadowUnderTBID:
            ArtManager.Get().SetRaiseToolbar(event.IsChecked())
            mb.Refresh()
            return 


