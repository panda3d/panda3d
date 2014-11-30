###############################################################################
# Name: cmenumgr.py                                                           #
# Purpose: ContextMenu Manager                                                #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2010 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
Editra Business Model Library: ContextMenuManager

Helper class for managing context menu callbacks

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__cvsid__ = "$Id: cmenumgr.py 67348 2011-03-30 17:59:32Z CJP $"
__revision__ = "$Revision: 67348 $"

__all__ = [ 'ContextMenuManager', ]

#-----------------------------------------------------------------------------#
# Imports
import wx

#-----------------------------------------------------------------------------#

class ContextMenuManager(object):
    """Class for registering and managing context menu callbacks"""
    def __init__(self):
        super(ContextMenuManager, self).__init__()

        # Attributes
        self._menu = None       # Context Menu
        self._pos = (0, 0)      # Menu position
        self._handlers = dict() # {ID : callable(event)}
        self._userdata = dict()

    Menu = property(lambda self: self.GetMenu(), 
                    lambda self, menu: self.SetMenu(menu))

    Position = property(lambda self: self.GetPosition(),
                        lambda self, pos: self.SetPosition(pos))

    def AddHandler(self, evt_id, handler):
        """Add an event handler
        @param evt_id: int
        @param handler: callable(event)

        """
        self._handlers[evt_id] = handler

    def Clear(self):
        """Clear all handlers and destroy the menu"""
        self._handlers.clear()
        self._userdata.clear()
        if self._menu:
            self._menu.Destroy()

    def GetHandler(self, evt_id):
        """Get the event handler for the provided ID or None
        @param evt_id: int
        @return: callable or None

        """
        return self._handlers.get(evt_id, None)

    def GetMenu(self):
        """Get the menu that is being managed by this manager
        @return: wxMenu

        """
        return self._menu

    def GetPosition(self):
        """Get the menu position
        @return: tuple (int, int)

        """
        return self._pos

    def GetUserData(self, key):
        """Get user data
        @param key: data id key

        """
        return self._userdata.get(key, None)

    def SetMenu(self, menu):
        """Set the menu that this manager should manage
        @param menu: wxMenu

        """
        assert isinstance(menu, wx.Menu), "menu must be a wxMenu"
        self._menu = menu

    def SetPosition(self, pos):
        """Set the menu position
        @param pos: tuple (int, int)

        """
        self._pos = pos

    def SetUserData(self, key, data):
        """Add custom user data to the manager
        @param key: unique key used to retrieve the data later
        @param data: user data

        """
        self._userdata[key] = data
