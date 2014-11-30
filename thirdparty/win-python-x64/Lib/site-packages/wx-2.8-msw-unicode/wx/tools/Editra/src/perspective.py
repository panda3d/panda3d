###############################################################################
# Name: perspectives.py                                                       #
# Purpose: Editra's view management service                                   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides a perspective management class for saving and loading custom
perspectives in the MainWindow.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: perspective.py 67680 2011-05-03 14:37:40Z CJP $"
__revision__ = "$Revision: 67680 $"

#--------------------------------------------------------------------------#
# Dependencies
import os
import wx
import util
import ed_menu
from profiler import Profile_Get, Profile_Set

#--------------------------------------------------------------------------#
# Globals
AUTO_PERSPECTIVE = u'Automatic'
DATA_FILE = u'perspectives'
LAST_KEY = u'**LASTVIEW**'

# ID's
ID_SAVE_PERSPECTIVE = wx.NewId()
ID_DELETE_PERSPECTIVE = wx.NewId()
ID_AUTO_PERSPECTIVE = wx.NewId()

# Aliases
_ = wx.GetTranslation
#--------------------------------------------------------------------------#

class PerspectiveManager(object):
    """Creates a perspective manager for the given aui managed window.
    It supports saving and loading of on disk perspectives as created by
    calling SavePerspective from the AuiManager. Mixin class for a wx.Frame.

    """
    def __init__(self, base):
        """Initializes the perspective manager. The auimgr parameter is
        a reference to the windows AuiManager instance, base is the base
        path to where perspectives should be loaded from and saved to.
        @param base: path to configuration cache

        """
        super(PerspectiveManager, self).__init__()

        hint = wx.aui.AUI_MGR_TRANSPARENT_HINT
        if wx.Platform == '__WXGTK__':
            # Use venetian blinds style as transparent can cause crashes
            # on linux when desktop compositing is used.
            hint = wx.aui.AUI_MGR_VENETIAN_BLINDS_HINT

        self._mgr = wx.aui.AuiManager(flags=wx.aui.AUI_MGR_DEFAULT |
                                      wx.aui.AUI_MGR_TRANSPARENT_DRAG |
                                      hint |
                                      wx.aui.AUI_MGR_ALLOW_ACTIVE_PANE)
        self._mgr.SetManagedWindow(self)

        # Attributes
        self._ids = list()                          # List of menu ids
        self._base = os.path.join(base, DATA_FILE)  # Path to config
        self._viewset = dict()                      # Set of Views
        self.LoadPerspectives()
        self._menu = ed_menu.EdMenu()               # Control menu
        self._currview = Profile_Get('DEFAULT_VIEW')    # Currently used view

        # Setup Menu
        self._menu.Append(ID_SAVE_PERSPECTIVE, _("Save Current View"),
                    _("Save the current window layout"))
        self._menu.Append(ID_DELETE_PERSPECTIVE, _("Delete Saved View"))
        self._menu.AppendSeparator()
        self._menu.Append(ID_AUTO_PERSPECTIVE, _("Automatic"),
                          _("Automatically save/use window state from last session"),
                          wx.ITEM_CHECK)
        self._menu.AppendSeparator()
        for name in self._viewset:
            self.AddPerspectiveMenuEntry(name)

        # Restore the managed windows previous position preference if available.
        pos = Profile_Get('WPOS', "size_tuple", False)
        if Profile_Get('SET_WPOS') and pos:
            # Ensure window is on screen
            if not self.IsPositionOnScreen(pos):
                pos = self.GetPrimaryDisplayOrigin()
            self.SetPosition(pos)

        # Event Handlers
        self.Bind(wx.EVT_MENU, self.OnPerspectiveMenu)

    #---- Properties ----#
    PanelMgr = property(lambda self: self._mgr)

    def AddPerspective(self, name, p_data=None):
        """Add a perspective to the view set. If the p_data parameter
        is not set then the current view will be added with the given name.
        @param name: name for new perspective
        @keyword p_data: perspective data from auimgr
        @return: bool (True == Added, False == Not Added)

        """
        # Don't allow empty keys or ones that override the automatic
        # settings to be added
        name = name.strip()
        if not len(name) or name == AUTO_PERSPECTIVE:
            return False

        domenu = not self.HasPerspective(name)
        if p_data is None:
            self._viewset[name] = self._mgr.SavePerspective()
        else:
            self._viewset[name] = p_data
        self._currview = name

        if name != AUTO_PERSPECTIVE and domenu:
            self.AddPerspectiveMenuEntry(name)

        self.SavePerspectives()
        return True

    def AddPerspectiveMenuEntry(self, name):
        """Adds an entry to list of perspectives in the menu for this manager.
        @param name: name of perspective to add to menu
        @return: bool (added or not)

        """
        name = name.strip()
        if not len(name) or name == AUTO_PERSPECTIVE:
            return False

        per_id = wx.NewId()
        self._ids.append(per_id)
        self._menu.InsertAlpha(per_id, name, _("Change view to \"%s\"") % name,
                               kind=wx.ITEM_CHECK, after=ID_AUTO_PERSPECTIVE)
        return True

    def GetFrameManager(self):
        """Returns the manager for this frame
        @return: Reference to the AuiMgr of this window

        """
        return self._mgr

    def GetPerspectiveControls(self):
        """Returns the control menu for the manager
        @return: menu of this manager

        """
        return self._menu

    def GetPerspective(self):
        """Returns the name of the current perspective used
        @return: name of currently active perspective

        """
        return self._currview

    def GetPerspectiveData(self, name):
        """Returns the given named perspectives data string
        @param name: name of perspective to fetch data from

        """
        return self._viewset.get(name, None)

    def GetPersectiveHandlers(self):
        """Gets a list of ID to UIHandlers for the perspective Menu
        @return: list of [(ID, HandlerFunction)]

        """
        handlers = [(m_id, self.OnUpdatePerspectiveMenu) for m_id in self._ids]
        return handlers + [(ID_AUTO_PERSPECTIVE, self.OnUpdatePerspectiveMenu)]

    def GetPerspectiveList(self):
        """Returns a list of all the loaded perspectives. The
        returned list only provides the names of the perspectives
        and not the actual data.
        @return: list of all managed perspectives

        """
        return sorted(self._viewset.keys())

    def GetPrimaryDisplayOrigin(self):
        """Get the origin on the primary display to use as a default
        window placement position.
        @return: position tuple

        """
        # NOTE: don't default to 0,0 otherwise on osx the frame will be
        #       stuck behind the menubar.
        for idx in range(wx.Display.GetCount()):
            disp = wx.Display(idx)
            if disp.IsPrimary():
                drect = disp.GetClientArea()
                return drect.GetPosition() + (5, 5)
        else:
            return (5, 5)

    def HasPerspective(self, name):
        """Returns True if there is a perspective by the given name
        being managed by this manager, or False otherwise.
        @param name: name of perspective to look for
        @return: whether perspective is managed by this manager or not

        """
        return name in self._viewset

    def InitWindowAlpha(self):
        """Initialize the windows alpha setting"""
        level = max(100, Profile_Get('ALPHA', default=255))
        # Only set the transparency if it is not opaque
        if level != 255:
            self.SetTransparent(level)

    def IsPositionOnScreen(self, pos):
        """Check if the given position is on any of the connected displays
        @param pos: Position Tuple
        @return: bool

        """
        bOnScreen = False
        if len(pos) == 2:
            for idx in range(wx.Display.GetCount()):
                disp = wx.Display(idx)
                drect = disp.GetClientArea()
                bOnScreen = drect.Contains(pos)
                if bOnScreen:
                    break
        return bOnScreen

    def LoadPerspectives(self):
        """Loads the perspectives data into the manager. Returns
        the number of perspectives that were successfully loaded.
        @return: number of perspectives loaded

        """
        reader = util.GetFileReader(self._base)
        if reader == -1:
            util.Log("[perspective][err] Failed to get " +
                     "file reader for %s" % self._base)
            return 0

        try:
            for line in reader.readlines():
                label, val = line.split(u"=", 1)
                label = label.strip()
                if not len(label):
                    continue
                self._viewset[label] = val.strip()
            reader.close()
        finally:
            if LAST_KEY in self._viewset:
                self._currview = self._viewset[LAST_KEY]
                del self._viewset[LAST_KEY]
            return len(self._viewset)

    def OnPerspectiveMenu(self, evt):
        """Handles menu events generated by the managers control menu.
        @param evt: event that called this handler

        """
        e_id = evt.GetId()
        if e_id == ID_SAVE_PERSPECTIVE:
            name = wx.GetTextFromUser(_("Perspective Name"), \
                                      _("Save Perspective"))
            if name:
                self.AddPerspective(name, p_data=None)
                self.SavePerspectives()
                Profile_Set('DEFAULT_VIEW', name)

                # It may make sense to update all windows to use this
                # perspective at this point but it may be an unexpected
                # event to happen when there is many windows open. Will
                # leave this to future consideration.
                for mainw in wx.GetApp().GetMainWindows():
                    mainw.AddPerspective(name, self._viewset[name])

        elif e_id == ID_DELETE_PERSPECTIVE:
            views = [ view for view in self._viewset.keys()
                      if view != AUTO_PERSPECTIVE ]
            name = wx.GetSingleChoice(_("Perspective to Delete"),
                                      _("Delete Perspective"), views)

            if name:
                self.RemovePerspective(name)
                self.SavePerspectives()
                for mainw in wx.GetApp().GetMainWindows():
                    mainw.RemovePerspective(name)
            else:
                pass

            # Update all windows data sets
            for mainw in wx.GetApp().GetMainWindows():
                mainw.LoadPerspectives()

        elif e_id in self._ids + [ID_AUTO_PERSPECTIVE]:
            if e_id == ID_AUTO_PERSPECTIVE:
                Profile_Set('DEFAULT_VIEW', AUTO_PERSPECTIVE)
                self.SetAutoPerspective()
            else:
                self.SetPerspectiveById(e_id)
        else:
            evt.Skip()

    def OnUpdatePerspectiveMenu(self, evt):
        """Update the perspective menu's check mark states
        @param evt: UpdateUI event that called this handler

        """
        e_id = evt.GetId()
        if e_id in self._ids + [ID_AUTO_PERSPECTIVE]:
            evt.Check(self._menu.GetLabel(e_id) == self._currview)
        else:
            evt.Skip()

    def RemovePerspective(self, name):
        """Removes a named perspective from the managed set
        @param name: name of perspective to remove/delete

        """
        if name in self._viewset:
            del self._viewset[name]
            rem_id = self._menu.RemoveItemByName(name)
            if rem_id:
                self._ids.remove(rem_id)

    def SetAutoPerspective(self):
        """Set the current perspective management into automatic mode
        @postcondition: window is set into

        """
        self._currview = AUTO_PERSPECTIVE
        self.UpdateAutoPerspective()

    def SavePerspectives(self):
        """Writes the perspectives out to disk. Returns
        True if all data was written and False if there
        was an error.
        @return: whether save was successful
        @rtype: bool

        """
        writer = util.GetFileWriter(self._base)
        if writer == -1:
            util.Log("[perspective][err] Failed to save %s" % self._base)
            return False

        try:
            self._viewset[LAST_KEY] = self._currview
            for perspect in self._viewset:
                writer.write(u"%s=%s\n" % (perspect, self._viewset[perspect]))
            del self._viewset[LAST_KEY]
        except (IOError, OSError):
            util.Log("[perspective][err] Write error: %s" % self._base)
            return False
        else:
            return True

    def SetPerspective(self, name):
        """Sets the perspective of the managed window, returns
        True on success and False on failure.
        @param name: name of perspective to set
        @return: whether perspective was set or not
        @rtype: bool

        """
        if name in self._viewset:
            self._mgr.LoadPerspective(self._viewset[name])
            self._mgr.Update()

            self._currview = name
            self.SavePerspectives()
            return True
        else:
            # Fall back to automatic mode
            self.SetAutoPerspective()
            return False

    def SetPerspectiveById(self, per_id):
        """Sets the perspective using the given control id
        @param per_id: id of requested perspective
        @return: whether perspective was set or not
        @rtype: bool

        """
        name = None
        for pos in range(self._menu.GetMenuItemCount()):
            item = self._menu.FindItemByPosition(pos)
            if per_id == item.GetId():
                name = item.GetLabel()
                break

        if name is not None:
            return self.SetPerspective(name)
        else:
            return False

    def UpdateAutoPerspective(self):
        """Update the value of the auto-perspectives current saved state
        @postcondition: The perspective data for the Automatic setting is
                        updated to have data for the current state of the
                        window.

        """
        self._viewset[AUTO_PERSPECTIVE] = self._mgr.SavePerspective()
        self.SavePerspectives()
