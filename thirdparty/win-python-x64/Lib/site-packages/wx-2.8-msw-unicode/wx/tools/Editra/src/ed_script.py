###############################################################################
# Name: ed_script.py                                                          #
# Purpose: Currently Unimplemented                                            #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Incomplete not used

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_script.py 52855 2008-03-27 14:53:06Z CJP $"
__revision__ = "$Revision: 52855 $"

#--------------------------------------------------------------------------#
# Dependancies
import wx
import ed_glob
import plugin
import iface

#--------------------------------------------------------------------------#

class ScriptProcessorI(plugin.Interface):
    """Provides an inteface for plugins that want to extend
    the scripting support or add an entire different type of
    scripting to the editor.
    @todo:

    """
    def GetType(self):
        """Returns the identifier of this script processor. This
        is used to determine what script processor should recieve
        the job request.
        @return: script processor identifier

        """

    def ExecuteScript(self, script_name):
        """Executes the given script. The name should be the name
        of a script that is in the script cache.

        @see: L{ScriptCache}

        """

class ScriptProcessor(plugin.Plugin):
    """Processes script/macro data and executes the scripted actions
    @note: implements the ScriptProcessorI

    """
    observers = plugin.ExtensionPoint(ScriptProcessorI)
    def RunScript(self, script, script_type):
        """Runs the given script
        @param script: script to run
        @param script_type: the scripts type

        """
        res = False
        for ob in observers:
            stype = ob.GetType()
            if stype == script_type:
                res = ob.ExecuteScript(script)
                break
        return res

#-----------------------------------------------------------------------------#

class ScriptBase(list):
    """Represents a series of commands to have the editor execute.
    This is a base class that specific script implementations should
    derive from.

    """
    def __init__(self, name):
        """@param name: name of script object"""
        list.__init__(self)

        self.name = name

    def __str__(self):
        """Returns a string version of the script, usefull for
        showing the script in an editor window/and saving it to
        disk.
        @return: formatted string version of script

        """


    def AppendCommand(self, cmd):
        """Appends a command to the end of the script
        @param cmd: command to add to script

        """
        self.append(cmd)

    def GetName(self):
        """Returns the name of the script
        @return: name of script

        """
        return self.name

    def SetName(self, name):
        """Sets the name of this script
        @param name: what to name the script
        """
        if isinstance(name, basestring):
            self.name = name
        else:
            raise TypeError, "SetName expects a string"

class ScriptCache(object):
    """Storage manager class for Script objects. Manages providing
    scripts on request as well as the actual IO of saving and loading.

    """

#-----------------------------------------------------------------------------#

class ScriptDaemon(object):
    """A daemon like object that runs in the main application
    loop. It is called upon to run macros and scripts that
    are used for automating actions in the editor.

    """
    def __init__(self):
        """Create the scipt processing server"""
        object.__init__(self)

        # Attributes
        self._processor = ScriptProcessor()
        self._cache = ScriptCache()

    def AddScript(self, script):
        """Adds a script to the daemons cache
        @param script: script to cache

        """

    def LoadScriptCache(self):
        """Loads on disk scripts into the cache
        @postcondition: scripts saved on disk have been loaded into cache

        """

    def RunScript(self, script, *args):
        """Runs the specified script passing any arguments
        specified in *args to the script object.
        @param script: script to run

        """

    def SaveScripts(self):
        """Dumps the script cache onto disk
        @postcondition: script cache is written to disk

        """

#-----------------------------------------------------------------------------#

ID_MACRO_MAN = wx.NewId()
class MacroManager(plugin.Plugin):
    """Manage macros"""
    plugin.Implements([iface.ShelfI])

    def AllowMultiple(self):
        """This method is used to check if multiple instances of this
        item are allowed to be open at one time.
        @return: True/False
        @rtype: boolean

        """
        return False

    def CreateItem(self, parent):
        """This is them method used to open the item in the L{Shelf}
        It should return an object that is a Panel or subclass of a Panel.
        @param parent: The would be parent window of this panel
        @return: wx.Panel

        """

    def GetId(self):
        """Return the id that identifies this item (same as the menuid)
        @return: Item ID
        @rtype: int

        """
        return ID_MACRO_MAN

    def GetMenuEntry(self, menu):
        """Returns the menu entry for the Macro Manager
        @param menu: The menu this entry will be added to
        @return: wx.MenuItem

        """
        return wx.MenuItem(menu, ID_MACRO_MAN,  _("Macro Manager"),
                           _("View and Edit Macros"))

    def GetName(self):
        """Return the name of this shelf item. This should be the
        same as the MenuEntry's label.
        @return: name of item
        @rtype: string

        """
        return _(u"Macro Manager")


class MacroPanel(wx.Panel):
    """UI for viewing saved macros"""
    def __init__(self, parent, *args, **kwargs):
        """Create the panel"""
