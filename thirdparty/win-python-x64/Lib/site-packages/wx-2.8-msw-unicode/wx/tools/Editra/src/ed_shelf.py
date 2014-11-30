###############################################################################
# Name: ed_shelf.py                                                           #
# Purpose: Editra Shelf container                                             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2009 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Shelf plugin and control implementation

@summary: Shelf Implementation

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_shelf.py 68233 2011-07-12 03:01:16Z CJP $"
__revision__ = "$Revision: 68233 $"

#-----------------------------------------------------------------------------#
# Imports
import re
import wx

# Editra Imports
import ed_menu
import ed_glob
from profiler import Profile_Get
import ed_msg
import plugin
import iface
from extern import aui
import ed_book
import ebmlib

#--------------------------------------------------------------------------#
# Globals
PGNUM_PAT = re.compile(' - [0-9]+')
_ = wx.GetTranslation

#-----------------------------------------------------------------------------#

class Shelf(plugin.Plugin):
    """Plugin that creates a notebook for holding the various Shelf items
    implemented by L{ShelfI}.

    """
    SHELF_NAME = u'Shelf'
    observers = plugin.ExtensionPoint(iface.ShelfI)

    def GetUiHandlers(self, delegate):
        """Gets the update ui handlers for the shelf's menu
        @param delegate: L{EdShelfDelegate} instance
        @return: [(ID, handler),]

        """
        handlers = [ (item.GetId(), delegate.UpdateShelfMenuUI)
                     for item in self.observers ]
        return handlers

    def Init(self, parent):
        """Mixes the shelf into the parent window
        @param parent: Reference to MainWindow
        @return: L{EdShelfDelegate}

        """
        # First check if the parent has an instance already
        parent = parent
        mgr = parent.GetFrameManager()
        if mgr.GetPane(Shelf.SHELF_NAME).IsOk():
            return

        shelf = EdShelfBook(parent)
        mgr.AddPane(shelf,
                    wx.aui.AuiPaneInfo().Name(Shelf.SHELF_NAME).\
                            Caption(_("Shelf")).Bottom().Layer(0).\
                            CloseButton(True).MaximizeButton(True).\
                            BestSize(wx.Size(500,250)))

        # Hide the pane and let the perspective manager take care of it
        mgr.GetPane(Shelf.SHELF_NAME).Hide()
        mgr.Update()

        # Create the delegate
        delegate = EdShelfDelegate(shelf, self)

        # Install Shelf menu under View and bind event handlers
        view = parent.GetMenuBar().GetMenuByName("view")
        menu = delegate.GetMenu()
        pos = 0
        for pos in range(view.GetMenuItemCount()):
            mitem = view.FindItemByPosition(pos)
            if mitem.GetId() == ed_glob.ID_PERSPECTIVES:
                break

        view.InsertMenu(pos + 1, ed_glob.ID_SHELF, _("Shelf"), 
                        menu, _("Put an item on the Shelf"))

        for item in menu.GetMenuItems():
            if item.IsSeparator():
                continue
            parent.Bind(wx.EVT_MENU, delegate.OnGetShelfItem, item)

        if menu.GetMenuItemCount() < 3:
            view.Enable(ed_glob.ID_SHELF, False)

        # Check for any other plugin specific install needs
        for observer in self.observers:
            if not observer.IsInstalled() and \
               hasattr(observer, 'InstallComponents'):
                observer.InstallComponents(parent)

        # Only Load Perspective if all items are loaded
        if delegate.StockShelf(Profile_Get('SHELF_ITEMS', 'list', [])):
            delegate.SetPerspective(Profile_Get('SHELF_LAYOUT', 'str', u""))
            delegate.SetSelection(Profile_Get('SHELF_SELECTION', 'int', -1))
        return delegate

#--------------------------------------------------------------------------#

class EdShelfBook(ed_book.EdBaseBook):
    """Shelf notebook control"""
    def __init__(self, parent):
        style = aui.AUI_NB_BOTTOM | \
                aui.AUI_NB_TAB_SPLIT | \
                aui.AUI_NB_SCROLL_BUTTONS | \
                aui.AUI_NB_CLOSE_ON_ACTIVE_TAB | \
                aui.AUI_NB_TAB_MOVE | \
                aui.AUI_NB_DRAW_DND_TAB
        super(EdShelfBook, self).__init__(parent, style=style)

        # Attributes
        self._parent = parent
        self._open = dict()
        self._name2idx = dict() # For settings maintenance
        self._menu = ebmlib.ContextMenuManager()
        self._mcback = None

        # Setup
        self.SetSashDClickUnsplit(True)

        # Event Handlers
        self.Bind(aui.EVT_AUINOTEBOOK_TAB_RIGHT_UP, self.OnRightUp)
        self.Bind(aui.EVT_AUINOTEBOOK_BG_RIGHT_UP, self.OnRightUp)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.OnDestroy, self)

        # Message handlers
        ed_msg.Subscribe(self.OnUpdateTabs, ed_msg.EDMSG_THEME_NOTEBOOK)
        ed_msg.Subscribe(self.OnUpdateTabs, ed_msg.EDMSG_THEME_CHANGED)

    def OnDestroy(self, evt):
        if evt.GetId() == self.GetId():
            self._menu.Clear()
            ed_msg.Unsubscribe(self.OnUpdateTabs)
        evt.Skip()

    def OnRightUp(self, evt):
        """Show context menu"""
        self._menu.Clear()
        if self.MenuCallback:
            self._menu.Menu = self.MenuCallback()
            self.PopupMenu(self._menu.Menu)

    BitmapCallbacks = property(lambda self: self._name2idx)
    MenuCallback = property(lambda self: self._mcback,
                            lambda self, funct: setattr(self, '_mcback', funct))

    def AddItem(self, item, name, bmp=wx.NullBitmap):
        """Add an item to the shelf's notebook. This is useful for interacting
        with the Shelf from outside its interface. It may be necessary to
        call L{EnsureShelfVisible} before or after adding an item if you wish
        the shelf to be shown when the item is added.
        @param item: A panel like instance to add to the shelf's notebook
        @param name: Items name used for page text in notebook
        @keyword bmp: Tab bitmap to display

        """
        self.AddPage(item, 
                     u"%s - %d" % (name, self._open.get(name, 0)),
                     select=True)

        # Set the tab icon
        self.SetPageBitmap(self.GetPageCount()-1, bmp)
        self._open[name] = self._open.get(name, 0) + 1

    def EnsureShelfVisible(self):
        """Make sure the Shelf is visible
        @precondition: Shelf.Init has been called
        @postcondition: Shelf is shown

        """
        mgr = self._parent.GetFrameManager()
        pane = mgr.GetPane(Shelf.SHELF_NAME)
        if not pane.IsShown():
            pane.Show()
            mgr.Update()

    def GetCount(self, item_name):
        """Get the number of open instances of a given Shelf Item
        @param item_name: Name of the Shelf item
        @return: number of instances on the Shelf

        """
        count = 0
        for page in range(self.GetPageCount()):
            if self.GetPageText(page).startswith(item_name):
                count = count + 1
        return count

    def GetMainWindow(self):
        """Get the main window that this shelf instance was created for
        @return: ed_main.MainWindow

        """
        return self._parent

    def GetOpen(self):
        """Get the list of open shelf items
        @return: list

        """
        return self._open

    def Hide(self):
        """Hide the shelf
        @postcondition: Shelf is hidden by aui manager

        """
        mgr = self._parent.GetFrameManager()
        pane = mgr.GetPane(Shelf.SHELF_NAME)
        if pane.IsOk():
            pane.Hide()
            mgr.Update()

    def ItemIsOnShelf(self, item_name):
        """Check if at least one instance of a given item
        is currently on the Shelf.
        @param item_name: name of Item to look for

        """
        for page in xrange(self.GetPageCount()):
            if self.GetPageText(page).startswith(item_name):
                return True
        return False

    def IsShown(self):
        """Is the shelf visible?
        @return: bool

        """
        mgr = self._parent.GetFrameManager()
        pane = mgr.GetPane(Shelf.SHELF_NAME)
        if pane.IsOk():
            return pane.IsShown()
        else:
            return False

    def OnUpdateTabs(self, msg):
        """Update all tab images depending upon current settings"""
        if not Profile_Get('TABICONS', default=True):
            for page in range(self.GetPageCount()):
                self.SetPageBitmap(page, wx.NullBitmap)
        else:
            # Show the icons
            for pnum in range(self.GetPageCount()):
                page = self.GetPage(pnum)
                bmp = self.BitmapCallbacks.get(repr(page.__class__), lambda:wx.NullBitmap)()
                self.SetPageBitmap(pnum, bmp)

#--------------------------------------------------------------------------#

class EdShelfDelegate(object):
    """Delegate class to mediate between the plugin singleton object and the
    UI implementation.

    """
    def __init__(self, shelf, pobject):
        """Create the delegate object
        @param shelf: Ui component instance
        @param pobject: Reference to the plugin object

        """
        super(EdShelfDelegate, self).__init__()

        # Attributes
        self._log = wx.GetApp().GetLog()
        self._shelf = shelf
        self._pin = pobject

        # Setup
        self._shelf.MenuCallback = getattr(self, 'GetShelfObjectMenu')

    @property
    def observers(self):
        return self._pin.observers

    def AddItem(self, item, name, bmp=wx.NullBitmap):
        """Add an item to the shelf"""
        self._shelf.AddItem(item, name, bmp)

    def CanStockItem(self, item_name):
        """See if a named item can be stocked or not, meaning if it
        can be saved and opened in the next session or not.
        @param item_name: name of item to check
        @return: bool whether item can be stocked or not

        """
        for item in self.observers:
            if item_name == item.GetName():
                if hasattr(item, 'IsStockable'):
                    return item.IsStockable()
                else:
                    break
        return False

    def EnsureShelfVisible(self):
        """Ensure the shelf is visible"""
        self._shelf.EnsureShelfVisible()

    def GetItemById(self, itemid):
        """Get the shelf item by its id
        @param itemid: Shelf item id
        @return: reference to a ShelfI object

        """
        for item in self.observers:
            if item.GetId() == itemid:
                return item
        return None

    def GetItemId(self, item_name):
        """Get the id that identifies a given item
        @param item_name: name of item to get ID for
        @return: integer id or None if not found

        """
        for item in self.observers:
            if item_name == item.GetName():
                return item.GetId()
        return None

    def GetItemStack(self):
        """Returns a list of ordered named items that are open in the shelf
        @return: list of strings

        """
        rval = list()
        if self._shelf is not None:
            for page in xrange(self._shelf.GetPageCount()):
                rval.append(re.sub(PGNUM_PAT, u'', 
                            self._shelf.GetPageText(page), 1))
        return rval

    def GetSelection(self):
        """Get the index of the currently selected tab"""
        if self._shelf:
            return self._shelf.GetSelection()
        return -1

    def GetPerspective(self):
        """Get the auinotebook perspective data
        @return: string

        """
        return self._shelf.SavePerspective()

    def SetPerspective(self, pdata):
        """Set the aui notebooks perspective and layout
        @param pdata: perspective data string

        """
        if pdata:
            try:
                self._shelf.LoadPerspective(pdata)
                self._shelf.Update()
            except Exception, msg:
                self._log("[shelf][err] Failed LoadPerspective: %s" % msg)

    def GetShelfObjectMenu(self):
        """Get the minimal menu that lists all Shelf objects
        without the 'Show Shelf' item.
        @return: ed_menu.EdMenu

        """
        menu = ed_menu.EdMenu()
        menu_items = list()
        open_items = self._shelf.GetOpen()
        for observer in self.observers:
            # Register Observers
            open_items[observer.GetName()] = 0
            try:
                menu_i = observer.GetMenuEntry(menu)
                if menu_i is not None:
                    menu_items.append((menu_i.GetItemLabel(), menu_i))
            except Exception, msg:
                self._log("[shelf][err] %s" % str(msg))
        menu_items.sort()

        combo = 0
        for item in menu_items:
            combo += 1
            shortcut = u""
            if combo < 10:
                shortcut = u"\tCtrl+Alt+" + unicode(combo)
            nitem = menu.Append(item[1].Id, item[1].GetText() + shortcut)
            if item[1].Bitmap.IsOk():
                nitem.SetBitmap(item[1].Bitmap)
            item[1].Destroy()
        return menu

    def GetMenu(self):
        """Return the menu of this object
        @return: ed_menu.EdMenu()

        """
        menu = self.GetShelfObjectMenu()
        menu.Insert(0, wx.ID_SEPARATOR)
        menu.Insert(0, ed_glob.ID_SHOW_SHELF, _("Show Shelf") + \
                    ed_menu.EdMenuBar.keybinder.GetBinding(ed_glob.ID_SHOW_SHELF), 
                    _("Show the Shelf"))
        return menu

    def GetOwnerWindow(self):
        """Return the L{ed_main.MainWindow} instance that owns/created
        this Shelf.
        @return: reference to ed_main.MainWindow or None

        """
        return self._shelf.GetMainWindow()

    def GetWindow(self):
        """Return reference to the Shelfs window component
        @return: AuiNotebook

        """
        return self._shelf

    def OnGetShelfItem(self, evt):
        """Handles menu events that have been registered
        by the Shelf Items on the Shelf.
        @param evt: Event that called this handler

        """
        e_id = evt.GetId()
        if e_id == ed_glob.ID_SHOW_SHELF:
            parent = self.GetOwnerWindow()
            if self._shelf.IsShown():
                self._shelf.Hide()
                nb = parent.GetNotebook()
                nb.GetCurrentCtrl().SetFocus()
            else:
                self._shelf.EnsureShelfVisible()
                mgr = parent.GetFrameManager()
                pane = mgr.GetPane(Shelf.SHELF_NAME)
                if pane is not None:
                    page = pane.window.GetCurrentPage()
                    if hasattr(page, 'SetFocus'):
                        page.SetFocus()
        else:
            self.PutItemOnShelf(evt.GetId())

    def OnPutShelfItemAway(self, evt):
        """Handles when an item is closed
        @param evt: event that called this handler
        @todo: is this needed?

        """
        raise NotImplementedError

    def PutItemOnShelf(self, shelfid):
        """Put an item on the shelf by using its unique shelf id.
        This is only for use with loading items implementing the
        L{ShelfI} interface. See L{AddItem} if you wish to pass
        a panel to the shelf to add.
        @param shelfid: id of the ShelfItem to open

        """
        item = None
        for shelfi in self.observers:
            if shelfi.GetId() == shelfid:
                item = shelfi
                break

        if item is None:
            return

        name = item.GetName()
        if self._shelf.ItemIsOnShelf(name) and \
            not item.AllowMultiple() or self._shelf is None:
            return
        else:
            self.EnsureShelfVisible()
            window = item.CreateItem(self._shelf)
            bmp = wx.NullBitmap
            if hasattr(item, 'GetBitmap'):
                self._shelf.BitmapCallbacks[repr(window.__class__)] = item.GetBitmap
                bmp = item.GetBitmap()
            else:
                self._shelf.BitmapCallbacks[repr(window.__class__)] = lambda:wx.NullBitmap
            self.AddItem(window, name, bmp)

    def RaiseItem(self, item_name):
        """Set the selection in the notebook to be the that of the first
        instance of item_name that is found in the shelf.
        @param item_name: ShelfI name
        @return: reference to the selected page or None if no instance is

        """
        for page in xrange(self._shelf.GetPageCount()):
            if self._shelf.GetPageText(page).startswith(item_name):
                self._shelf.SetSelection(page)
                return self._shelf.GetPage(page)
        else:
            return None

    def RaiseWindow(self, window):
        """Set the selection in the notebook to be the that of the given
        window. Mostly used internally by items implementing L{ShelfI}.
        @param window: Window object
        @return: reference to the selected page or None if no instance is

        """
        for page in range(self._shelf.GetPageCount()):
            ctrl = self._shelf.GetPage(page)
            if window == ctrl:
                self._shelf.SetSelection(page)
                return ctrl
        else:
            return None

    def SetSelection(self, index):
        """Select an item in the Shelf window
        @param index: shelf tab index

        """
        if self._shelf and index > 0 and index < self._shelf.GetPageCount():
            try:
                self._shelf.SetSelection(index)
            except Exception, msg:
                self._log("[shelf][err] Failed SetSelection: %s" % msg)

    def StockShelf(self, i_list):
        """Fill the shelf by opening an ordered list of items
        @param i_list: List of named L{ShelfI} instances
        @type i_list: list of strings
        @return: bool (True if all loaded / False otherwise)

        """
        bLoaded = True
        for item in i_list:
            if self.CanStockItem(item):
                itemid = self.GetItemId(item)
                if itemid:
                    self.PutItemOnShelf(itemid)
                else:
                    bLoaded = False
            else:
                bLoaded = False
        return bLoaded

    def UpdateShelfMenuUI(self, evt):
        """Enable/Disable shelf items based on whether they support
        muliple instances or not.
        @param evt: wxEVT_UPDATEUI

        """
        item = self.GetItemById(evt.GetId())
        if item is None:
            evt.Skip()
            return

        count = self._shelf.GetCount(item.GetName())
        if count and not item.AllowMultiple():
            evt.Enable(False)
        else:
            evt.Enable(True)
