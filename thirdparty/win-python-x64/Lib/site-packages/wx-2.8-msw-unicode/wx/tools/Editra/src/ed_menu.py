###############################################################################
# Name: ed_menu.py                                                            #
# Purpose: Editra's Menubar and Menu related classes                          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007-2008 Cody Precord <staff@editra.org>                    #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides an advanced menu class for easily creating menus and setting their
related bitmaps when available from Editra's ArtProvider. The Keybinder class
for managing keybindings and profiles is also provided by this module.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_menu.py 68038 2011-06-24 17:18:05Z CJP $"
__revision__ = "$Revision: 68038 $"

#--------------------------------------------------------------------------#
# Dependancies
import os
import wx

# Editra Libraries
import ed_glob
import ed_msg
import profiler
import util
from syntax import syntax
from syntax import synglob

#--------------------------------------------------------------------------#
# Globals
_ = wx.GetTranslation

#--------------------------------------------------------------------------#

class EdMenu(wx.Menu):
    """Custom wxMenu class that makes it easier to customize and access items.

    """
    def __init__(self, title=wx.EmptyString, style=0):
        """Initialize a Menu Object
        @param title: menu title string
        @param style: type of menu to create

        """
        super(EdMenu, self).__init__(title, style)

    def Append(self, id_, text=u'', helpstr=u'', \
               kind=wx.ITEM_NORMAL, use_bmp=True):
        """Append a MenuItem
        @keyword use_bmp: try and set a bitmap if an appropriate one is
                          available in the ArtProvider

        """
        item = wx.MenuItem(self, id_, text, helpstr, kind)
        self.AppendItem(item, use_bmp)
        return item

    def AppendEx(self, id_, text=u'', helpstr=u'',
                 kind=wx.ITEM_NORMAL, use_bmp=True):
        """Like L{Append} but automatically applies keybindings to text
        based on item id.

        """
        binding = EdMenuBar.keybinder.GetBinding(id_)
        item = self.Append(id_, text+binding, helpstr, kind, use_bmp)
        return item

    def AppendItem(self, item, use_bmp=True):
        """Appends a MenuItem to the menu and adds an associated
        bitmap if one is available, unless use_bmp is set to false.
        @keyword use_bmp: try and set a bitmap if an appropriate one is
                          available in the ArtProvider

        """
        if use_bmp and item.GetKind() == wx.ITEM_NORMAL:
            self.SetItemBitmap(item)
        super(EdMenu, self).AppendItem(item)

    def Insert(self, pos, id_, text=u'', helpstr=u'', \
               kind=wx.ITEM_NORMAL, use_bmp=True):
        """Insert an item at position and attach a bitmap
        if one is available.
        @keyword use_bmp: try and set a bitmap if an appropriate one is
                          available in the ArtProvider

        """
        item = super(EdMenu, self).Insert(pos, id_, text, helpstr, kind)
        if use_bmp and kind == wx.ITEM_NORMAL:
            self.SetItemBitmap(item)
        return item

    def InsertAfter(self, item_id, id_, label=u'', helpstr=u'',
                    kind=wx.ITEM_NORMAL, use_bmp=True):
        """Inserts the given item after the specified item id in
        the menu. If the id cannot be found then the item will appended
        to the end of the menu.
        @keyword use_bmp: try and set a bitmap if an appropriate one is
                          available in the ArtProvider
        @return: the inserted menu item

        """
        pos = None
        for item in xrange(self.GetMenuItemCount()):
            mitem = self.FindItemByPosition(item)
            if mitem.GetId() == item_id:
                pos = item
                break
        if pos:
            mitem = self.Insert(pos + 1, id_, label, helpstr, kind, use_bmp)
        else:
            mitem = self.Append(id_, label, helpstr, kind, use_bmp)
        return mitem

    def InsertBefore(self, item_id, id_, label=u'', helpstr=u'',
                    kind=wx.ITEM_NORMAL, use_bmp=True):
        """Inserts the given item before the specified item id in
        the menu. If the id cannot be found then the item will appended
        to the end of the menu.
        @keyword use_bmp: try and set a bitmap if an appropriate one is
                          available in the ArtProvider
        @return: menu item that was inserted

        """
        pos = None
        for item in xrange(self.GetMenuItemCount()):
            mitem = self.FindItemByPosition(item)
            if mitem.GetId() == item_id:
                pos = item
                break
        if pos:
            mitem = self.Insert(pos, id_, label, helpstr, kind, use_bmp)
        else:
            mitem = self.Append(id_, label, helpstr, kind, use_bmp)
        return mitem

    def InsertAlpha(self, id_, label=u'', helpstr=u'',
                    kind=wx.ITEM_NORMAL, after=0, use_bmp=True):
        """Attempts to insert the new menuitem into the menu
        alphabetically. The optional parameter 'after' is used
        specify an item id to start the alphabetical lookup after.
        Otherwise the lookup begins from the first item in the menu.
        @keyword after: id of item to start alpha lookup after
        @keyword use_bmp: try and set a bitmap if an appropriate one is
                          available in the ArtProvider
        @return: menu item that was inserted

        """
        if after:
            start = False
        else:
            start = True
        last_ind = self.GetMenuItemCount() - 1
        pos = last_ind
        for item in range(self.GetMenuItemCount()):
            mitem = self.FindItemByPosition(item)
            if mitem.IsSeparator():
                continue

            mlabel = mitem.GetItemLabel()
            if after and mitem.GetId() == after:
                start = True
                continue
            if after and not start:
                continue
            if label < mlabel:
                pos = item
                break

        l_item = self.FindItemByPosition(last_ind)
        if pos == last_ind and (l_item.IsSeparator() or label > mlabel):
            mitem = self.Append(id_, label, helpstr, kind, use_bmp)
        else:
            mitem = self.Insert(pos, id_, label, helpstr, kind, use_bmp)
        return mitem

    def RemoveItemByName(self, name):
        """Removes an item by the label. It will remove the first
        item matching the given name in the menu, the matching is
        case sensitive. The return value is the either the id of the
        removed item or None if the item was not found.
        @param name: name of item to remove
        @return: id of removed item or None if not found

        """
        menu_id = None
        for pos in range(self.GetMenuItemCount()):
            item = self.FindItemByPosition(pos)
            if name == item.GetLabel():
                menu_id = item.GetId()
                self.Remove(menu_id)
                break
        return menu_id

    def SetItemBitmap(self, item):
        """Sets the MenuItems bitmap by getting the id from the
        artprovider if one exists.
        @param item: item to set bitmap for

        """
        bmp = wx.ArtProvider.GetBitmap(str(item.GetId()), wx.ART_MENU)
        if not bmp.IsNull():
            item.SetBitmap(bmp)

#-----------------------------------------------------------------------------#

class KeyBinder(object):
    """Class for managing keybinding configurations"""
    cprofile = None # Current Profile Name String
    keyprofile = dict() # Active Profile (dict)

    def __init__(self):
        """Create the KeyBinder object"""
        super(KeyBinder, self).__init__()

        # Attributes
        self.cache = ed_glob.CONFIG['CACHE_DIR'] # Resource Directory

    def GetBinding(self, item_id):
        """Get the keybinding string for use in a menu
        @param item_id: Menu Item Id
        @return: string

        """
        rbind = self.GetRawBinding(item_id)
        shortcut = u''
        if rbind is not None:
            shortcut = u"+".join(rbind)
            if len(shortcut):
                shortcut = u"\t" + shortcut
        return unicode(shortcut)

    @classmethod
    def GetCurrentProfile(cls):
        """Get the name of the currently set key profile if one exists
        @return: string or None

        """
        return cls.cprofile

    @classmethod
    def GetCurrentProfileDict(cls):
        """Get the dictionary of keybindings
        @return: dict

        """
        return cls.keyprofile

    @staticmethod
    def GetKeyProfiles():
        """Get the list of available key profiles
        @return: list of strings

        """
        recs = util.GetResourceFiles(u'cache', trim=True, get_all=False,
                                     suffix='.ekeys', title=False)
        if recs == -1:
            recs = list()

        tmp = util.GetResourceFiles(u'ekeys', True, True, '.ekeys', False)
        if tmp != -1:
            recs.extend(tmp)

        return recs

    def GetProfilePath(self, pname):
        """Get the full path to the given keyprofile
        @param pname: profile name
        @return: string or None
        @note: expects unique name for each profile in the case that
               a name exists in both the user and system paths the one
               found on the user path will be returned.

        """
        if pname is None:
            return None

        rname = None
        for rec in self.GetKeyProfiles():
            if rec.lower() == pname.lower():
                rname = rec
                break

        # Must be a new profile
        if rname is None:
            rname = pname

        kprof = u"%s%s.ekeys" % (ed_glob.CONFIG['CACHE_DIR'], rname)
        if not os.path.exists(kprof):
            # Must be a system supplied keyprofile
            rname = u"%s%s.ekeys" % (ed_glob.CONFIG['KEYPROF_DIR'], rname)
            if not os.path.exists(rname):
                # Doesn't exist at syspath either so instead assume it is a new
                # custom user defined key profile.
                rname = kprof
        else:
            rname = kprof

        return rname

    @classmethod
    def GetRawBinding(cls, item_id):
        """Get the raw key binding tuple
        @param item_id: MenuItem Id
        @return: tuple

        """
        return cls.keyprofile.get(item_id, None)

    @classmethod
    def FindMenuId(cls, keyb):
        """Find the menu item ID that the
        keybinding is currently associated with.
        @param keyb: tuple of unicode (u'Ctrl', u'C')
        @return: int (-1 if not found)

        """
        menu_id = -1
        for key, val in cls.keyprofile.iteritems():
            if val == keyb:
                menu_id = key
                break
        return menu_id

    @classmethod
    def LoadDefaults(cls):
        """Load the default key profile"""
        cls.keyprofile = dict(_DEFAULT_BINDING)
        cls.cprofile = None

    def LoadKeyProfile(self, pname):
        """Load a key profile into the binder
        @param pname: name of key profile to load

        """
        if pname is None:
            ppath = None
        else:
            ppath = self.GetProfilePath(pname)

        keydict = dict()
        if ppath is not None and os.path.exists(ppath):
            reader = util.GetFileReader(ppath)
            if reader != -1:
                util.Log("[keybinder][info] Loading KeyProfile: %s" % ppath)
                for line in reader:
                    parts = line.split(u'=', 1)
                    # Check that the line was formatted properly
                    if len(parts) == 2:
                        # Try to find the ID value
                        item_id = _GetValueFromStr(parts[0])
                        if item_id is not None:
                            tmp = [ part.strip()
                                    for part in parts[1].split(u'+')
                                    if len(part.strip()) ]

                            # Do some checking if the binding is valid
                            nctrl = len([key for key in tmp
                                         if key not in (u'Ctrl', u'Alt', u'Shift')])
                            if nctrl:
                                keydict[item_id] = tmp
                                if parts[1].strip().endswith(u'++'):
                                    keydict[item_id].append(u'+')
                            else:
                                # Invalid key binding
                                continue

                reader.close()
                KeyBinder.keyprofile = keydict
                KeyBinder.cprofile = pname
                return
            else:
                util.Log("[keybinder][err] Couldn't read %s" % ppath)
        elif pname is not None:
            # Fallback to default keybindings
            util.Log("[keybinder][err] Failed to load bindings from %s" % pname)

        util.Log("[keybinder][info] Loading Default Keybindings")
        KeyBinder.LoadDefaults()

    def SaveKeyProfile(self):
        """Save the current key profile to disk"""
        if KeyBinder.cprofile is None:
            util.Log("[keybinder][warn] No keyprofile is set, cant save")
        else:
            ppath = self.GetProfilePath(KeyBinder.cprofile)
            writer = util.GetFileWriter(ppath)
            if writer != -1:
                itemlst = list()
                for item in KeyBinder.keyprofile.keys():
                    itemlst.append(u"%s=%s%s" % (_FindStringRep(item),
                                                self.GetBinding(item).lstrip(),
                                                os.linesep))
                writer.writelines(sorted(itemlst))
                writer.close()
            else:
                util.Log("[keybinder][err] Failed to open %s for writing" % ppath)

    @classmethod
    def SetBinding(cls, item_id, keys):
        """Set the keybinding of a menu id
        @param item_id: item to set
        @param keys: string or list of key strings ['Ctrl', 'S']

        """
        if isinstance(keys, basestring):
            keys = [ key.strip() for key in keys.split(u'+')
                     if len(key.strip())]
            keys = tuple(keys)

        if len(keys):
            # Check for an existing binding
            menu_id = cls.FindMenuId(keys)
            if menu_id != -1:
                del cls.keyprofile[menu_id]
            # Set the binding
            cls.keyprofile[item_id] = keys
        elif item_id in cls.keyprofile:
            # Clear the binding
            del cls.keyprofile[item_id]
        else:
            pass

    @classmethod
    def SetProfileName(cls, pname):
        """Set the name of the current profile
        @param pname: name to set profile to

        """
        cls.cprofile = pname

    @classmethod
    def SetProfileDict(cls, keyprofile):
        """Set the keyprofile using a dictionary of id => bindings
        @param keyprofile: { menu_id : (u'Ctrl', u'C'), }

        """
        cls.keyprofile = keyprofile

#-----------------------------------------------------------------------------#

class EdMenuBar(wx.MenuBar):
    """Custom menubar to allow for easier access and updating
    of menu components.
    @todo: redo all of this

    """
    keybinder = KeyBinder()

    def __init__(self, style=0):
        """Initializes the Menubar
        @keyword style: style to set for menu bar

        """
        super(EdMenuBar, self).__init__(style)

        # Setup
        if EdMenuBar.keybinder.GetCurrentProfile() is None:
            kprof = profiler.Profile_Get('KEY_PROFILE', default='default')
            EdMenuBar.keybinder.LoadKeyProfile(kprof)

        # Attributes
        self._menus = dict()
        self.GenFileMenu()
        self.GenEditMenu()
        self.GenViewMenu()
        self.GenFormatMenu()
        self.GenSettingsMenu()
        self.GenToolsMenu()
        self.GenHelpMenu()

        # Message handlers
        ed_msg.Subscribe(self.OnRebind, ed_msg.EDMSG_MENU_REBIND)
        ed_msg.Subscribe(self.OnLoadProfile, ed_msg.EDMSG_MENU_LOADPROFILE)
        ed_msg.Subscribe(self.OnCreateLexerMenu, ed_msg.EDMSG_CREATE_LEXER_MENU)

    def CreateLexerMenu(self):
        """Create the Lexer menu"""
        settingsmenu = self._menus['settings']
        item = settingsmenu.FindItemById(ed_glob.ID_LEXER)
        if item:
            settingsmenu.Remove(ed_glob.ID_LEXER)
        mconfig = profiler.Profile_Get('LEXERMENU', default=list())
        mconfig.sort()

        # Create the menu
        langmenu = wx.Menu()
        langmenu.Append(ed_glob.ID_LEXER_CUSTOM, _("Customize..."),
                        _("Customize the items shown in this menu."))
        langmenu.AppendSeparator()

        for label in mconfig:
            lid = synglob.GetIdFromDescription(label)
            langmenu.Append(lid, label,
                            _("Switch Lexer to %s") % label, wx.ITEM_CHECK)

        settingsmenu.AppendMenu(ed_glob.ID_LEXER, _("Lexers"),
                                langmenu,
                                _("Manually Set a Lexer/Syntax"))

    @classmethod
    def DeleteKeyProfile(cls, pname):
        """Remove named keyprofile
        @param pname: keyprofile name
        @return: True if removed, False otherwise

        """
        ppath = cls.keybinder.GetProfilePath(pname)
        if ppath is not None and os.path.exists(ppath):
            try:
                os.remove(ppath)
            except:
                return False
            else:
                return True
        else:
            return False

    # TODO these Gen* functions should be broken up to the components
    #      that supply the functionality and inserted in the menus on
    #      init when the editor loads an associated widget.
    def GenFileMenu(self):
        """Makes and attaches the file menu
        @return: None

        """
        filemenu = EdMenu()
        filehist = self._menus['filehistory'] = EdMenu()
        filemenu.AppendEx(ed_glob.ID_NEW, _("&New Tab"),
                          _("Start a new file in a new tab"))
        filemenu.AppendEx(ed_glob.ID_NEW_WINDOW, _("New &Window"),
                        _("Start a new file in a new window"))
        filemenu.AppendSeparator()
        filemenu.AppendEx(ed_glob.ID_OPEN, _("&Open"), _("Open"))
        ## Setup File History in the File Menu
        filemenu.AppendMenu(ed_glob.ID_FHIST, _("Open &Recent"),
                            filehist, _("Recently Opened Files"))
        filemenu.AppendSeparator()
        filemenu.AppendEx(ed_glob.ID_CLOSE, _("&Close Tab"),
                        _("Close Current Tab"))
        filemenu.AppendEx(ed_glob.ID_CLOSE_WINDOW,
                        _("Close Window") , _("Close the current window"))
        filemenu.AppendEx(ed_glob.ID_CLOSEALL, _("Close All Tabs"),
                        _("Close all open tabs"))
        filemenu.AppendSeparator()
        filemenu.AppendEx(ed_glob.ID_SAVE, _("&Save"), _("Save Current File"))
        filemenu.AppendEx(ed_glob.ID_SAVEAS, _("Save &As"), _("Save As"))
        filemenu.AppendEx(ed_glob.ID_SAVEALL, _("Save All"),
                        _("Save all open pages"))
        filemenu.AppendSeparator()
        filemenu.AppendEx(ed_glob.ID_REVERT_FILE, _("Revert to Saved"),
                        _("Revert file to last save point"))
        filemenu.AppendEx(ed_glob.ID_RELOAD_ENC, _("Reload with Encoding..."),
                        _("Reload the file with a specified encoding"))
        filemenu.AppendSeparator()

        # Profile
        pmenu = EdMenu()
        pmenu.AppendEx(ed_glob.ID_SAVE_PROFILE, _("Save Profile"),
                     _("Save Current Settings to a New Profile"))
        pmenu.AppendEx(ed_glob.ID_LOAD_PROFILE, _("Load Profile"),
                     _("Load a Custom Profile"))
        filemenu.AppendSubMenu(pmenu, _("Profile"),
                               _("Load and save custom Profiles"))

        # Sessions
        smenu = EdMenu()
        smenu.AppendEx(ed_glob.ID_SAVE_SESSION, _("Save Session"),
                     _("Save the current session."))
        smenu.AppendEx(ed_glob.ID_LOAD_SESSION, _("Load Session"),
                     _("Load a saved session."))
        filemenu.AppendSubMenu(smenu, _("Sessions"),
                               _("Load and save custom sessions."))

        filemenu.AppendSeparator()
        filemenu.AppendEx(ed_glob.ID_PRINT_SU, _("Page Set&up"),
                        _("Configure Printer"))
        filemenu.AppendEx(ed_glob.ID_PRINT_PRE, _("Print Pre&view"),
                        _("Preview Printout"))
        filemenu.AppendEx(ed_glob.ID_PRINT, _("&Print"), _("Print Current File"))
        filemenu.AppendSeparator()
        filemenu.AppendEx(ed_glob.ID_EXIT, _("E&xit"), _("Exit the Program"))

        # Attach to menubar and save reference
        self.Append(filemenu, _("&File"))
        self._menus['file'] = filemenu

    def GenEditMenu(self):
        """Makes and attaches the edit menu
        @return: None

        """
        editmenu = EdMenu()
        editmenu.AppendEx(ed_glob.ID_UNDO, _("&Undo"), _("Undo Last Action"))
        editmenu.AppendEx(ed_glob.ID_REDO, _("Redo"), _("Redo Last Undo"))
        editmenu.AppendSeparator()
        editmenu.AppendEx(ed_glob.ID_CUT, _("Cu&t"),
                          _("Cut Selected Text from File"))
        editmenu.AppendEx(ed_glob.ID_COPY, _("&Copy"),
                        _("Copy Selected Text to Clipboard"))
        editmenu.AppendEx(ed_glob.ID_PASTE, _("&Paste"),
                        _("Paste Text from Clipboard to File"))
        editmenu.AppendEx(ed_glob.ID_PASTE_AFTER, _("P&aste After"),
                        _("Paste Text from Clipboard to File after the cursor"))
        editmenu.AppendEx(ed_glob.ID_CYCLE_CLIPBOARD, _("Cycle Clipboard"),
                        _("Cycle through recent clipboard text"))
        editmenu.AppendSeparator()
        editmenu.AppendEx(ed_glob.ID_SELECTALL, _("Select &All"),
                        _("Select All Text in Document"))
        editmenu.AppendEx(ed_glob.ID_COLUMN_MODE, _("Column Edit"),
                        _("Enable column edit mode."), wx.ITEM_CHECK)
        editmenu.AppendSeparator()
        linemenu = EdMenu()
        linemenu.AppendEx(ed_glob.ID_LINE_AFTER, _("New Line After"),
                         _("Add a new line after the current line"))
        linemenu.AppendEx(ed_glob.ID_LINE_BEFORE, _("New Line Before"),
                        _("Add a new line before the current line"))
        linemenu.AppendSeparator()
        linemenu.AppendEx(ed_glob.ID_CUT_LINE, _("Cut Line"),
                        _("Cut Current Line"))
        linemenu.AppendEx(ed_glob.ID_DELETE_LINE, _("Delete Line"),
                        _("Delete the selected line(s)"))
        linemenu.AppendEx(ed_glob.ID_COPY_LINE, _("Copy Line"),
                        _("Copy Current Line"))
        linemenu.AppendEx(ed_glob.ID_DUP_LINE, _("Duplicate Line"),
                        _("Duplicate the current line"))
        linemenu.AppendSeparator()
        linemenu.AppendEx(ed_glob.ID_JOIN_LINES, _("Join Lines"),
                        _("Join the Selected Lines"))
        linemenu.AppendEx(ed_glob.ID_TRANSPOSE, _("Transpose Line"),
                        _("Transpose the current line with the previous one"))
        linemenu.AppendEx(ed_glob.ID_LINE_MOVE_UP, _("Move Current Line Up"),
                        _("Move the current line up"))
        linemenu.AppendEx(ed_glob.ID_LINE_MOVE_DOWN,
                        _("Move Current Line Down"),
                        _("Move the current line down"))
        editmenu.AppendMenu(ed_glob.ID_LINE_EDIT, _("Line Edit"), linemenu,
                            _("Commands that affect an entire line"))
        bookmenu = EdMenu()
        bookmenu.AppendEx(ed_glob.ID_ADD_BM, _("Toggle Bookmark"),
                        _("Toggle bookmark of the current line"))
        bookmenu.AppendEx(ed_glob.ID_DEL_ALL_BM, _("Remove All Bookmarks"),
                        _("Remove all bookmarks from the current document"))
        editmenu.AppendMenu(ed_glob.ID_BOOKMARK, _("Bookmarks"),  bookmenu,
                            _("Add and remove bookmarks"))
        editmenu.AppendSeparator()
        # Autocompletion shortcuts
        editmenu.AppendEx(ed_glob.ID_SHOW_AUTOCOMP, _("Word Completion"),
                          _("Show autocompletion hints."))
        editmenu.AppendEx(ed_glob.ID_SHOW_CALLTIP, _("Show Calltip"),
                          _("Show a calltip for the current word."))
        editmenu.AppendSeparator()
        editmenu.AppendEx(ed_glob.ID_FIND, _("&Find"), _("Find Text"))
        editmenu.AppendEx(ed_glob.ID_FIND_REPLACE, _("Find/R&eplace"),
                        _("Find and Replace Text"))
        editmenu.AppendEx(ed_glob.ID_QUICK_FIND, _("&Quick Find"),
                        _("Open the Quick Find Bar"))
        editmenu.AppendEx(ed_glob.ID_FIND_PREVIOUS, _("Find Previous"),
                        _("Goto previous match"))
        editmenu.AppendEx(ed_glob.ID_FIND_NEXT, _("Find Next"),
                        _("Goto the next match"))
        editmenu.AppendEx(ed_glob.ID_FIND_SELECTED, _("Find Selected"),
                          _("Search for the currently selected phrase"))
        editmenu.AppendSeparator()
        editmenu.AppendEx(ed_glob.ID_PREF, _("Pr&eferences"),
                        _("Edit Preferences / Settings"))

        # Attach to menubar and save ref
        self.Append(editmenu, _("&Edit"))
        self._menus['edit'] = editmenu

    def GenViewMenu(self):
        """Makes and attaches the view menu
        @return: None

        """
        viewmenu = EdMenu()
        viewmenu.AppendEx(ed_glob.ID_ZOOM_OUT, _("Zoom Out"), _("Zoom Out"))
        viewmenu.AppendEx(ed_glob.ID_ZOOM_IN, _("Zoom In"), _("Zoom In"))
        viewmenu.AppendEx(ed_glob.ID_ZOOM_NORMAL, _("Zoom Default"),
                        _("Zoom Default"))
        viewmenu.AppendSeparator()
        viewedit = self._menus['viewedit'] = EdMenu()
        viewedit.AppendEx(ed_glob.ID_HLCARET_LINE, _("Highlight Caret Line"),
                        _("Highlight the background of the current line"),
                        wx.ITEM_CHECK)
        viewedit.AppendEx(ed_glob.ID_INDENT_GUIDES, _("Indentation Guides"),
                        _("Show Indentation Guides"), wx.ITEM_CHECK)
        viewedit.AppendEx(ed_glob.ID_SHOW_EDGE, _("Show Edge Guide"),
                        _("Show the edge column guide"), wx.ITEM_CHECK)
        viewedit.AppendEx(ed_glob.ID_SHOW_EOL, _("Show EOL Markers"),
                        _("Show EOL Markers"), wx.ITEM_CHECK)
        viewedit.AppendEx(ed_glob.ID_SHOW_LN, _("Show Line Numbers"),
                        _("Show Line Number Margin"), wx.ITEM_CHECK)
        viewedit.AppendEx(ed_glob.ID_SHOW_WS, _("Show Whitespace"),
                        _("Show Whitespace Markers"), wx.ITEM_CHECK)
        viewmenu.AppendSubMenu(self._menus['viewedit'], _("Editor"), \
                               _("Toggle Editor View Options"))
        viewfold = self._menus['viewfold'] = EdMenu()
        viewfold.AppendEx(ed_glob.ID_TOGGLE_FOLD, _("Toggle fold"),
                        _("Toggle current fold"))
        viewfold.AppendEx(ed_glob.ID_TOGGLE_ALL_FOLDS, _("Toggle all folds"),
                        _("Toggle all folds"))
        viewmenu.AppendSubMenu(self._menus['viewfold'], _("Code Folding"), \
                               _("Code folding toggle actions"))

        viewmenu.AppendSeparator()
        viewmenu.AppendEx(ed_glob.ID_PANELIST, _("Pane Navigator"),
                        _("View pane selection list"))
        viewmenu.AppendEx(ed_glob.ID_MAXIMIZE_EDITOR, _("Maximize Editor"),
                        _("Toggle Editor Maximization"))
        viewmenu.AppendSeparator()
        viewmenu.AppendEx(ed_glob.ID_GOTO_LINE, _("&Goto Line"),
                        _("Goto Line Number"))
        viewmenu.AppendEx(ed_glob.ID_GOTO_MBRACE, _("Goto Matching Brace"),
                        _("Move caret matching brace"))
        viewmenu.AppendSeparator()
        viewmenu.AppendEx(ed_glob.ID_NEXT_POS, _("Next Position"),
                        _("Goto next position in history."))
        viewmenu.AppendEx(ed_glob.ID_PRE_POS, _("Previous Position"),
                        _("Goto previous position in history."))
        viewmenu.AppendSeparator()
        viewmenu.AppendEx(ed_glob.ID_NEXT_MARK, _("Next Bookmark"),
                        _("View Line of Next Bookmark"))
        viewmenu.AppendEx(ed_glob.ID_PRE_MARK, _("Previous Bookmark"),
                        _("View Line of Previous Bookmark"))
        viewmenu.AppendSeparator()
        viewmenu.AppendEx(ed_glob.ID_SHOW_SB, ("Status &Bar"),
                        _("Show Status Bar"), wx.ITEM_CHECK)
        viewmenu.AppendEx(ed_glob.ID_VIEW_TOOL, _("&Toolbar"),
                        _("Show Toolbar"), wx.ITEM_CHECK)

        # Attach to menubar
        self.Append(viewmenu, _("&View"))
        self._menus['view'] = viewmenu

    def GenFormatMenu(self):
        """Makes and attaches the format menu
        @return: None

        """
        formatmenu = EdMenu()
        formatmenu.AppendEx(ed_glob.ID_FONT, _("&Font"),
                          _("Change Font Settings"))
        formatmenu.AppendSeparator()
        formatmenu.AppendEx(ed_glob.ID_TOGGLECOMMENT, _("Toggle Comment"),
                          _("Toggle comment on the selected line(s)"))
        formatmenu.AppendSeparator()

        formatmenu.AppendEx(ed_glob.ID_INDENT, _("Indent Lines"),
                          _("Indent the selected lines"))
        formatmenu.AppendEx(ed_glob.ID_UNINDENT, _("Unindent Lines"),
                          _("Unindent the selected lines"))
        formatmenu.AppendSeparator()
        formatmenu.AppendEx(ed_glob.ID_TO_UPPER,  _("Uppercase"),
                          _("Convert selected text to all uppercase letters"))
        formatmenu.AppendEx(ed_glob.ID_TO_LOWER,  _("Lowercase"),
                          _("Convert selected text to all lowercase letters"))
        formatmenu.AppendSeparator()
        formatmenu.AppendEx(ed_glob.ID_USE_SOFTTABS, _("Use Soft Tabs"),
                          _("Insert spaces instead of tab "
                            "characters with tab key"), wx.ITEM_CHECK)
        formatmenu.AppendEx(ed_glob.ID_WORD_WRAP, _("Word Wrap"),
                          _("Wrap Text Horizontally"), wx.ITEM_CHECK)
        formatmenu.AppendSeparator()

        # Whitespace submenu
        whitespace = self._menus['whitespaceformat'] = EdMenu()
        whitespace.AppendEx(ed_glob.ID_SPACE_TO_TAB, _("Spaces to Tabs"),
                          _("Convert spaces to tabs in selected/all text"))
        whitespace.AppendEx(ed_glob.ID_TAB_TO_SPACE, _("Tabs to Spaces"),
                          _("Convert tabs to spaces in selected/all text"))
        whitespace.AppendEx(ed_glob.ID_TRIM_WS, _("Trim Trailing Whitespace"),
                          _("Remove trailing whitespace"))
        formatmenu.AppendMenu(ed_glob.ID_WS_FORMAT, _("Whitespace"), whitespace,
                              _("Whitespace formating commands"))

        # Line EOL formatting submenu
        lineformat = self._menus['lineformat'] = EdMenu()
        lineformat.AppendEx(ed_glob.ID_EOL_MAC, _("Old Macintosh (\\r)"),
                          _("Format all EOL characters to %s Mode") % \
                          _(u"Old Macintosh (\\r)"), wx.ITEM_CHECK)
        lineformat.AppendEx(ed_glob.ID_EOL_UNIX, _("Unix (\\n)"),
                          _("Format all EOL characters to %s Mode") % \
                          _(u"Unix (\\n)"), wx.ITEM_CHECK)
        lineformat.AppendEx(ed_glob.ID_EOL_WIN, _("Windows (\\r\\n)"),
                          _("Format all EOL characters to %s Mode") % \
                          _("Windows (\\r\\n)"), wx.ITEM_CHECK)
        formatmenu.AppendMenu(ed_glob.ID_EOL_MODE, _("EOL Mode"), lineformat,
                              _("End of line character formatting"))

        # Attach to menubar
        self.Append(formatmenu, _("F&ormat"))
        self._menus['format'] = formatmenu

    def GenSettingsMenu(self):
        """Makes and attaches the settings menu
        @return: None

        """
        settingsmenu = EdMenu()
        settingsmenu.AppendEx(ed_glob.ID_AUTOCOMP, _("Auto-Completion"),
                            _("Use Auto Completion when available"), wx.ITEM_CHECK)
        settingsmenu.AppendEx(ed_glob.ID_AUTOINDENT, _("Auto-Indent"),
                            _("Toggle Auto-Indentation functionality"),
                            wx.ITEM_CHECK)
        settingsmenu.AppendEx(ed_glob.ID_BRACKETHL, _("Bracket Highlighting"),
                            _("Highlight Brackets/Braces"), wx.ITEM_CHECK)
        settingsmenu.AppendEx(ed_glob.ID_FOLDING, _("Code Folding"),
                            _("Toggle Code Folding"), wx.ITEM_CHECK)
        settingsmenu.AppendEx(ed_glob.ID_SYNTAX, _("Syntax Highlighting"),
                            _("Color Highlight Code Syntax"), wx.ITEM_CHECK)

        settingsmenu.AppendSeparator()

        # Lexer Menu Appended later by main frame
        self.Append(settingsmenu, _("&Settings"))
        self._menus['settings'] = settingsmenu

        self.CreateLexerMenu()

    def GenToolsMenu(self):
        """Makes and attaches the tools menu
        @return: None

        """
        toolsmenu = EdMenu()
        toolsmenu.AppendEx(ed_glob.ID_COMMAND, _("Editor Command"),
                         _("Goto command buffer"))
        toolsmenu.AppendEx(ed_glob.ID_PLUGMGR, _("Plugin Manager"),
                         _("Manage, Download, and Install plugins"))
        toolsmenu.AppendEx(ed_glob.ID_STYLE_EDIT, _("Style Editor"),
                         _("Edit the way syntax is highlighted"))
        toolsmenu.AppendSeparator()
#         macro = EdMenu()
#         macro.Append(ed_glob.ID_MACRO_START, _("Record Macro"),
#                          _("Start macro recording"))
#         macro.Append(ed_glob.ID_MACRO_STOP, _("Stop Recording"),
#                          _("Stop macro recording"))
#         macro.Append(ed_glob.ID_MACRO_PLAY, "Play Macro", "Play Macro")
#         toolsmenu.AppendMenu(wx.NewId(), _("Macros"), macro, _("Macro Tools"))

        # Attach to menubar
        self.Append(toolsmenu, _("&Tools"))
        self._menus['tools'] = toolsmenu

    def GenHelpMenu(self):
        """Makes and attaches the help menu
        @return: None

        """
        helpmenu = EdMenu()
        helpmenu.AppendEx(ed_glob.ID_ABOUT, _("&About..."),
                        _("About") + u"...")
        helpmenu.AppendEx(ed_glob.ID_HOMEPAGE, _("Project Homepage..."),
                        _("Visit the project homepage %s") % ed_glob.HOME_PAGE)
        helpmenu.AppendEx(ed_glob.ID_DOCUMENTATION,
                        _("Online Documentation..."),
                        _("Online project documentation and help guides"))
        helpmenu.AppendEx(ed_glob.ID_TRANSLATE, _("Translate Editra..."),
                        _("Editra translations project"))
        helpmenu.AppendEx(ed_glob.ID_BUG_TRACKER, _("Bug Tracker..."))
        helpmenu.AppendEx(ed_glob.ID_CONTACT, _("Feedback"),
                        _("Send bug reports and suggestions"))

        # Attach to menubar
        self.Append(helpmenu, _("&Help"))
        self._menus['help'] = helpmenu

    @classmethod
    def GetKeyBinder(cls):
        """Return the classes keybinder object
        @return: KeyBinder

        """
        return cls.keybinder

    def GetMenuByName(self, namestr):
        """Find and return a menu by name
        @param namestr: menuitems label
        @return: menuitem or None if not found

        """
        return self._menus.get(namestr.lower(), None)

    def GetMenuMap(self):
        """Get a mapping of all menus to (menu id, menu label)
        @return: list of dict

        """
        menumap = list()
        for menu in self.GetMenus():
            menumap.append(WalkMenu(menu[0], menu[1], dict()))
        return menumap

    @classmethod
    def NewKeyProfile(cls, pname):
        """Make a new key profile that is a clone of the current one
        @param pname: Name to give new profile

        """
        cls.keybinder.SetProfileName(pname)
        cls.keybinder.SaveKeyProfile()

    def OnCreateLexerMenu(self, msg):
        """Recreate the lexer menu"""
        self.CreateLexerMenu()

    def OnLoadProfile(self, msg):
        """Load and set the current key profile
        @param msg: ed_msg.EDMSG_MENU_LOADPROFILE
        @note: if message data is None the default bindings will be set

        """
        keyprof = msg.GetData()
        if keyprof is not None:
            self.SetKeyProfile(keyprof)
        else:
            EdMenuBar.keybinder.LoadDefaults()

    def OnRebind(self, msg):
        """Rebind all menu shortcuts when a rebind message is recieved
        @param msg: ed_msg.EDMSG_MENU_REBIND

        """
        self.RebindKeys()

    def RebindKeys(self):
        """Reset all key bindings based on current binder profile"""
        for menu in self.GetMenus():
            for item in IterateMenuItems(menu[0]):
                item_id = item.GetId()
                binding = EdMenuBar.keybinder.GetBinding(item_id)
                empty_binding = not len(binding)
                if not empty_binding:
                    # Verify binding and clear invalid ones from binder
                    tmp = [key.title() for key in binding.strip().split(u'+')]
                    nctrl = len([key for key in tmp
                                if key not in (u'Ctrl', u'Alt', u'Shift')])
                    if len(tmp) > 3 or not nctrl:
                        EdMenuBar.keybinder.SetBinding(item_id, u'')
                        continue

                    # Reset the binding in the binder to ensure it is
                    # correctly formatted.
                    binding = u"\t" + u"+".join(tmp)
                    EdMenuBar.keybinder.SetBinding(item_id, binding)

                clbl = item.GetText()
                # Update the item if the shortcut has changed
                if ('\t' in clbl and (not clbl.endswith(binding) or empty_binding)) or \
                   ('\t' not in clbl and not empty_binding):
                    # wxBug? Getting the text of a menuitem is supposed to
                    # return it with the accelerators but under gtk the string
                    # has underscores '_' where it was supposed to have '&'
                    if wx.Platform == '__WXGTK__':
                        clbl = clbl.replace('_', '&', 1)
                    item.SetText(clbl.split('\t')[0].strip() + binding)

    def ResetIcons(self):
        """Walk through each menu item in all of the bars menu and
        reapply icons where possible.
        @status: Dont use, sort of works on mac, does nothing on gtk, and causes
                 graphical glitches on msw.

        """
        for menu in self.GetMenus():
            WalkAndSetBitmaps(menu[0])

    @classmethod
    def SaveKeyProfile(cls):
        """Save the current key profile"""
        cls.keybinder.SaveKeyProfile()

    def SetKeyProfile(self, pname):
        """Set the current key profile and update the bindings
        @param pname: Name of keyprofile to load

        """
        EdMenuBar.keybinder.LoadKeyProfile(pname)
        self.RebindKeys()

#-----------------------------------------------------------------------------#

#---- Private Objects/Functions ----#

_DEFAULT_BINDING = { # File Menu
                     ed_glob.ID_NEW : (u"Ctrl", u"N"),
                     ed_glob.ID_NEW_WINDOW : (u"Ctrl", u"Shift", u"N"),
                     ed_glob.ID_OPEN : (u"Ctrl", u"O"),
                     ed_glob.ID_CLOSE : (u"Ctrl", u"W"),
                     ed_glob.ID_CLOSE_WINDOW : (u"Ctrl", u"Shift", u"W"),
                     ed_glob.ID_SAVE : (u"Ctrl", u"S"),
                     ed_glob.ID_SAVEAS : (u"Ctrl", u"Shift", u"S"),
                     ed_glob.ID_PRINT_SU : (u"Ctrl", u"Shift", u"P"),
                     ed_glob.ID_PRINT : (u"Ctrl", u"P"),
                     ed_glob.ID_EXIT : (u"Ctrl", u"Q"),

                     # Edit Menu
                     ed_glob.ID_UNDO : (u"Ctrl", u"Z"),
                     ed_glob.ID_REDO : (u"Ctrl", u"Shift", u"Z"),
                     ed_glob.ID_CUT : (u"Ctrl", u"X"),
                     ed_glob.ID_COPY : (u"Ctrl", u"C"),
                     ed_glob.ID_PASTE : (u"Ctrl", u"V"),
                     ed_glob.ID_PASTE_AFTER : (u"Ctrl", u"Shift", u"V"),
                     ed_glob.ID_CYCLE_CLIPBOARD : (u"Ctrl", u"I"),
                     ed_glob.ID_SELECTALL : (u"Ctrl", u"A"),
                     ed_glob.ID_COLUMN_MODE : (u"Ctrl", u"Shift", u"|"),
                     ed_glob.ID_LINE_AFTER : (u"Ctrl", u"L"),
                     ed_glob.ID_LINE_BEFORE : (u"Ctrl", u"Shift", u"L"),
                     ed_glob.ID_CUT_LINE : (u"Ctrl", u"D"),
                     ed_glob.ID_DELETE_LINE : (u"Ctrl", u"Shift", "D"),
                     ed_glob.ID_COPY_LINE : (u"Ctrl", u"Y"),
                     ed_glob.ID_DUP_LINE : (u"Ctrl", u"Shift", u"C"),
                     ed_glob.ID_JOIN_LINES : (u"Ctrl", u"J"),
                     ed_glob.ID_TRANSPOSE : (u"Ctrl", u"T"),
                     ed_glob.ID_LINE_MOVE_UP : (u"Ctrl", u"Shift", u"Up"),
                     ed_glob.ID_LINE_MOVE_DOWN : (u"Ctrl", u"Shift", u"Down"),
                     ed_glob.ID_ADD_BM : (u"Ctrl", u"B"),
                     ed_glob.ID_SHOW_AUTOCOMP : (u"Ctrl", u"Space"),
                     ed_glob.ID_SHOW_CALLTIP : (u"Ctrl", u"9"),
                     ed_glob.ID_FIND : (u"Ctrl", u"Shift", u"F"),
                     ed_glob.ID_FIND_PREVIOUS : (u"Shift", u"F3"),
                     ed_glob.ID_FIND_NEXT : (u"F3",),
                     ed_glob.ID_FIND_REPLACE : (u"Ctrl", u"R"),
                     ed_glob.ID_QUICK_FIND : (u"Ctrl", u"F"),
                     ed_glob.ID_FIND_SELECTED : (u"Ctrl", u"F3"),

                     # View Menu
                     ed_glob.ID_ZOOM_IN : (u"Ctrl", u"+"),
                     ed_glob.ID_ZOOM_OUT : (u"Ctrl", u"-"),
                     ed_glob.ID_ZOOM_NORMAL : (u"Ctrl", u"0"),
                     ed_glob.ID_GOTO_LINE : (u"Ctrl", u"G"),
                     ed_glob.ID_GOTO_MBRACE : (u"Ctrl", u"Shift", u"B"),
                     ed_glob.ID_TOGGLE_FOLD : (u"Ctrl", u"Shift", u"T"),
                     ed_glob.ID_NEXT_POS : (u"Ctrl", u"Shift", u">"),
                     ed_glob.ID_PRE_POS : (u"Ctrl", u"Shift", u"<"),
                     ed_glob.ID_NEXT_MARK : (u"Alt", u"Right"), # Win/Linux
                     ed_glob.ID_PRE_MARK : (u"Alt", u"Left"), # Win/Linux
                     ed_glob.ID_SHOW_SHELF : (u"Ctrl", u"Alt", u"S"),
                     ed_glob.ID_PANELIST : (u"Alt", u"1"), # Win/Linux
                     ed_glob.ID_MAXIMIZE_EDITOR : (u"Ctrl", u"M"),

                     # Format Menu
                     ed_glob.ID_TOGGLECOMMENT : (u"Ctrl", u"1"),
                     ed_glob.ID_INDENT : (u"Tab",),
                     ed_glob.ID_UNINDENT : (u"Shift", u"Tab"),
                     ed_glob.ID_USE_SOFTTABS : (u"Ctrl", u"Shift", u"I"),

                     # Tools Menu
                     ed_glob.ID_COMMAND : (u"Ctrl", u"E"),
                     ed_glob.ID_RUN_LAUNCH : (u"F5",),
                     ed_glob.ID_LAUNCH_LAST : (u"Shift", u"F5")
                     }

# Set some platform specific keybindings
if wx.Platform == '__WXMAC__':
    _DEFAULT_BINDING[ed_glob.ID_NEXT_MARK] = (u"Ctrl", u"Down")
    _DEFAULT_BINDING[ed_glob.ID_PRE_MARK] = (u"Ctrl", u"Up")
    _DEFAULT_BINDING[ed_glob.ID_FIND_PREVIOUS] = (u"Ctrl", u"Shift", u"G")
    _DEFAULT_BINDING[ed_glob.ID_FIND_NEXT] = (u"Ctrl", u"G")
    _DEFAULT_BINDING[ed_glob.ID_GOTO_LINE] = (u"Ctrl", u"Shift", u"E")
    _DEFAULT_BINDING[ed_glob.ID_PANELIST] = (u"Alt", u"Tab")
    _DEFAULT_BINDING[ed_glob.ID_MAXIMIZE_EDITOR] = (u"Alt", u"M")
    _DEFAULT_BINDING[ed_glob.ID_FIND_SELECTED] = (u"Ctrl", u"3")
elif wx.Platform == '__WXMSW__':
     # FIXME: On Windows if Tab is bound to a menu item it is no longer
     #        usable elsewhere such as in the stc control. On Mac/Gtk there
     #        are not problems with it.
    _DEFAULT_BINDING[ed_glob.ID_INDENT] = (u"",)
else:
    pass

def _FindStringRep(item_id):
    """Find the string representation of the given id value
    @param item_id: int
    @return: string or None

    """
    for obj in dir(ed_glob):
        if getattr(ed_glob, obj) == item_id:
            return obj
    else:
        return None

def _GetValueFromStr(item_str):
    """Get the id value from the string representation of the object
    @param item_str: items variable string
    @return: int or None

    """
    return getattr(ed_glob, item_str, None)

#---- Public Functions ----#

def IterateMenuItems(menu):
    """Recursively walk and yield menu items as the are found. Only menu
    items are yielded, not submenus or separators.
    @param menu: menu to iterate

    """
    for item in menu.GetMenuItems():
        if item.IsSubMenu():
            for subitem in IterateMenuItems(item.GetSubMenu()):
                yield subitem
        if not item.IsSeparator():
            yield item
        else:
            continue

def WalkAndSetBitmaps(menu):
    """Recursively walk a menu and its submenus setting bitmaps
    as necessary/available, using the the current theme.

    """
    for item in menu.GetMenuItems():
        if item.IsSubMenu():
            WalkAndSetBitmaps(item.GetSubMenu())
        else:
            bmp = wx.ArtProvider.GetBitmap(str(item.GetId()), wx.ART_MENU)
            if bmp.IsOk():
                item.SetBitmap(bmp)
            elif not item.GetBitmap().IsNull():
                item.SetBitmap(wx.NullBitmap)
            else:
                continue

def WalkMenu(menu, label, collection):
    """Recursively walk a menu and collect all its sub items
    @param menu: wxMenu to walk
    @param label: the menu's label
    @param collection: dictionary to collect results in
    @return: dict {menulabel : [menu id, (item1 id, label1),]}

    """
    if label not in collection:
        collection[label] = list()

    for item in menu.GetMenuItems():
        i_id = item.GetId()
        if item.IsSubMenu():
            # Ignore dynamically generated menus
            if i_id not in (ed_glob.ID_FHIST, ed_glob.ID_LEXER,
                             ed_glob.ID_PERSPECTIVES):
                ilbl = item.GetItemLabelText()
                collection[ilbl] = [i_id, ]
                WalkMenu(item.GetSubMenu(), ilbl, collection)
            else:
                continue
        elif item.IsSeparator():
            continue
        elif _FindStringRep(i_id) is not None:
            lbl = item.GetItemLabelText().split('\t')[0].strip()
            # wxBug? Even the methods that are supposed to return the text
            # without mnemonics or accelerators on gtk return the string with
            # underscores where the mnemonics '&' are in the original strings
            if wx.Platform == '__WXGTK__':
                lbl = lbl.replace('_', '', 1)
            collection[label].append((i_id, lbl))
        else:
            continue
    return collection
