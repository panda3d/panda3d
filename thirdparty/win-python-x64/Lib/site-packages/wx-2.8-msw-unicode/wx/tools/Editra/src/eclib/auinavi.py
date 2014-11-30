###############################################################################
# Name: auinavi.py                                                            #
# Purpose: AuiMgr Pane navigator                                              #
# Author: Giuseppe "Cowo" Corbelli                                            #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Control Library: AuiPaneNavigator

Popup navigation window for quickly navigating through AuiPanes in an AuiMgr.
Activating the dialog will cause a modal popup dialog with a list of all panes
managed by the aui manager. Changing the selection in the dialog will highlight
the pane in the managed frame. Selecting the choice in the list will move the
focus to that pane.

+---------------------+
| bmp title           |
+---------------------+
| pane list           |
|                     |                   
|                     |                   
|                     |                   
|                     |                   
|                     |
+---------------------+                   

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: auinavi.py 65794 2010-10-13 14:10:09Z CJP $"
__revision__ = "$Revision: 65794 $"

__all__ = ['AuiPaneNavigator',]

#-----------------------------------------------------------------------------#
# Imports
import wx

# Editra Control Libray Imports
import ctrlbox

#-----------------------------------------------------------------------------#

class AuiPaneNavigator(wx.Dialog):
    """Navigate through Aui Panes"""
    def __init__(self, parent, auiMgr, icon=None, title=''):
        """Initialize the navigator window
        @param parent: parent window
        @param auiMgr: wx.aui.AuiManager
        @keyword icon: wx.Bitmap or None
        @keyword title: string (dialog title)

        """
        super(AuiPaneNavigator, self).__init__(parent, wx.ID_ANY,
                                               "", style=wx.STAY_ON_TOP)

        # Attributes
        self._auimgr = auiMgr
        self._selectedItem = -1
        self._indexMap = list()
        self._sel = 0
        self._tabed = 0
        self._close_keys = [wx.WXK_ALT, wx.WXK_CONTROL, wx.WXK_RETURN]
        self._navi_keys = [wx.WXK_TAB, ord('1')] # <- TEMP
        self._listBox = None
        self._panel = None

        # Setup
        self.__DoLayout(icon, title)

        # Get the panes
        self.PopulateListControl()

        # Event Handlers
        self._listBox.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self._listBox.Bind(wx.EVT_NAVIGATION_KEY, self.OnNavigationKey)
        self._listBox.Bind(wx.EVT_LISTBOX_DCLICK, self.OnItemSelected)
        self._listBox.Bind(wx.EVT_LISTBOX, lambda evt: self.HighlightPane())

    def __del__(self):
        self._auimgr.HideHint()

    def __DoLayout(self, icon, title):
        """Layout the dialog controls
        @param icon: wx.Bitmap or None
        @param title: string

        """
        sz = wx.BoxSizer(wx.VERTICAL)
        self._listBox = wx.ListBox(self, wx.ID_ANY, wx.DefaultPosition,
                                   wx.Size(200, 150), list(),
                                   wx.LB_SINGLE | wx.NO_BORDER)

        self._panel = ctrlbox.ControlBar(self,
                                         style=ctrlbox.CTRLBAR_STYLE_GRADIENT)
        self._panel.SetVMargin(2, 2)

        if icon is not None:
            bmp = wx.StaticBitmap(self._panel, bitmap=icon)
            self._panel.AddControl(bmp, wx.ALIGN_LEFT)

        txt = wx.StaticText(self._panel, label=title)
        self._panel.AddControl(txt, wx.ALIGN_LEFT)

        sz.Add(self._panel, 0, wx.EXPAND)
        sz.Add(self._listBox, 1, wx.EXPAND)
        sz.Fit(self)
        sz.SetSizeHints(self)
        sz.Layout()
        self.Centre()
        self.SetSizer(sz)
        self.SetAutoLayout(True)

    def OnKeyUp(self, event):
        """Handles wx.EVT_KEY_UP"""
        self._auimgr.HideHint()
        key_code = event.GetKeyCode()
        # TODO: add setter method for setting the navigation key
        if key_code in self._navi_keys:
            self._tabed += 1

            # Don't move selection on initial show
            if self._tabed == 1:
                self.HighlightPane()
                event.Skip()
                return

            selected = self._listBox.GetSelection() + 1
            if selected >= self._listBox.GetCount():
                selected = 0

            self._listBox.SetSelection(selected)
            self.HighlightPane()
            event.Skip()
        elif key_code in self._close_keys:
            self.CloseDialog()
        elif key_code == wx.WXK_ESCAPE:
            self.CloseDialog()
        else:
            event.Skip()

    def OnNavigationKey(self, event):
        """Handles wx.EVT_NAVIGATION_KEY"""
        selected = self._listBox.GetSelection()
        maxItems = self._listBox.GetCount()
            
        if event.GetDirection():
            # Select next pane
            if selected == maxItems - 1:
                itemToSelect = 0
            else:
                itemToSelect = selected + 1
        else:
            # Previous pane
            if selected == 0:
                itemToSelect = maxItems - 1
            else:
                itemToSelect = selected - 1

        self._listBox.SetSelection(itemToSelect)
        self.HighlightPane()

    def PopulateListControl(self):
        """Populates the L{AuiPaneNavigator} with the panes in the AuiMgr"""
        self._panes = self._auimgr.GetAllPanes()
        names = [pane.name for pane in self._panes]
        self._listBox.AppendItems(sorted(names))

    def OnItemSelected(self, event):
        """Handles the wx.EVT_LISTBOX_DCLICK event"""
        self.CloseDialog()

    def CloseDialog(self):
        """Closes the L{AuiPaneNavigator} dialog"""
        self._selectedItem = self._listBox.GetStringSelection()
        self._auimgr.HideHint()
        self.EndModal(wx.ID_OK)

    def GetCloseKeys(self):
        """Get the list of keys that can dismiss the dialog
        @return: list of long (wx.WXK_*)

        """
        return self._close_keys

    def GetNavigationKeys(self):
        """Get the list of navigation key(s)
        @return: list of long (wx.WXK_*)

        """
        return self._navi_keys

    def GetSelection(self):
        """Get the index of the selected page"""
        return self._selectedItem

    def HighlightPane(self):
        """Highlight the currently selected pane"""
        sel = self._listBox.GetStringSelection()
        pane = self._auimgr.GetPane(sel)
        if pane.IsOk():
            self._auimgr.ShowHint(pane.window.GetScreenRect())
            # NOTE: this is odd but it is the only way for the focus to
            #       work correctly on wxMac...
            wx.CallAfter(self._listBox.SetFocus)
            self._listBox.SetFocus()

    def SetCloseKeys(self, keylist):
        """Set the keys that can be used to dismiss the L{AuiPaneNavigator}
        window.
        @param keylist: list of key codes

        """
        self._close_keys = keylist

    def SetNavigationKeys(self, keylist):
        """Set the key(s) to advance the selection in the pane list
        @param keylist: list of key codes

        """
        self._navi_keys = keylist

    def ShowModal(self):
        # Set focus on the list box to avoid having to click on it to change
        # the tab selection under GTK.
        self._listBox.SetFocus()
        self._listBox.SetSelection(0)
        return super(AuiPaneNavigator, self).ShowModal()