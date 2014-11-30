###############################################################################
# Name: iface.py                                                              #
# Purpose: Plugin interface definitions                                       #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module contains numerous plugin interfaces and the Extension points that
they extend. Included below is a list of interfaces available in this module.

Intefaces:
  - ShelfI: Interface into the L{Shelf}
  - MainWindowI: Interface into L{ed_main.MainWindow}
  - AutoCompI: Interface for adding autocompletion helpers

@summary: Main Plugin interface defintions

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: iface.py 63069 2010-01-05 01:24:16Z CJP $"
__revision__ = "$Revision: 63069 $"

#--------------------------------------------------------------------------#
# Imports
import wx

# Local Imports
import plugin

#--------------------------------------------------------------------------#

class AutoCompI(plugin.Interface):
    """The Autocompletion interface.

    """
    def GetCompleter(self, buff):
        """Get the completer object implemented by this plugin
        @param: buff EditraStc instance
        @return: instance of autocomp.BaseCompleter

        """
        raise NotImplementedError

    def GetFileTypeId(self):
        """Get the filetype this completer is associated with
        @return: int

        """
        return 0

#--------------------------------------------------------------------------#

class MainWindowI(plugin.Interface):
    """The MainWindow Interface is intended as a simple general purpose
    interface for adding functionality to the main window. It does little
    managing of how objects that implement it are handled, most is left up to
    the plugin. Some examples of plugins using this interface are the
    FileBrowser and Calculator plugins.

    """
    def PlugIt(self, window):
        """This method is called once and only once per window when it is 
        created. It should typically be used to register menu entries, 
        bind event handlers and other similar actions.

        @param window: The parent window of the plugin
        @postcondition: The plugins controls are installed in the L{MainWindow}

        """
        raise NotImplementedError

    def GetMenuHandlers(self):
        """Get menu event handlers/id pairs. This function should return a
        list of tuples containing menu ids and their handlers. The handlers
        should be not be a member of this class but a member of the ui component
        that they handler acts upon.
        
        
        @return: list [(ID_FOO, foo.OnFoo), (ID_BAR, bar.OnBar)]

        """
        pass

    def GetUIHandlers(self):
        """Get update ui event handlers/id pairs. This function should return a
        list of tuples containing object ids and their handlers. The handlers
        should be not be a member of this class but a member of the ui component
        that they handler acts upon.
        
        
        @return: list [(ID_FOO, foo.OnFoo), (ID_BAR, bar.OnBar)]

        """
        pass

#-----------------------------------------------------------------------------#

class ShelfI(plugin.Interface):
    """Interface into the L{Shelf}. All plugins wanting to be
    placed on the L{Shelf} should implement this interface.

    """
    def AllowMultiple(self):
        """This method is used to check if multiple instances of this
        item are allowed to be open at one time.
        @return: True/False
        @rtype: boolean

        """
        return True

    def CreateItem(self, parent):
        """This is them method used to open the item in the L{Shelf}
        It should return an object that is a Panel or subclass of a Panel.
        @param parent: The would be parent window of this panel
        @return: wx.Panel

        """
        raise NotImplementedError

    def GetBitmap(self):
        """Get the bitmap to show in the shelf for this item
        @return: wx.Bitmap
        @note: this method is optional

        """
        return wx.NullBitmap

    def GetId(self):
        """Return the id that identifies this item (same as the menuid)
        @return: Item ID
        @rtype: int

        """
        raise NotImplementedError

    def GetMenuEntry(self, menu):
        """Returns the menu entry associated with this item
        @param menu: The menu this entry will be added to
        @return: wx.MenuItem or None if no menu entry is needed

        """
        raise NotImplementedError

    def GetName(self):
        """Return the name of this shelf item. This should be the
        same as the MenuEntry's label.
        @return: name of item
        @rtype: string

        """
        raise NotImplementedError

    def InstallComponents(self, mainw):
        """Called by the Shelf when the plugin is created to allow it
        to install any extra components that it may have that fall outside
        the normal interface. This method is optional and does not need
        to be implimented if it is not needed.
        @param mainw: MainWindow Instance

        """
        pass

    def IsStockable(self):
        """Return whether this item type is stockable. The shelf saves
        what pages it had open the last time the program was run and then
        reloads the pages the next time the program starts. If this
        item can be reloaded between sessions return True otherwise return
        False.

        """
        return True

#-----------------------------------------------------------------------------#
