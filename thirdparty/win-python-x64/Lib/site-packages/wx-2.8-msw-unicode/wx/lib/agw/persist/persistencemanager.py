# --------------------------------------------------------------------------- #
# PersistentControls Library wxPython IMPLEMENTATION
#
# Inspired by the wxWidgets implementation by Vadim Zeitlin.
#
# License: wxWidgets license
#
# Python Code By:
#
# Andrea Gavana, @ 16 Nov 2009
# Latest Revision: 28 Jan 2011, 15.00 GMT
#
# For All Kind Of Problems, Requests Of Enhancements And Bug Reports, Please
# Write To Me At:
#
# andrea.gavana@gmail.com
# gavana@kpo.kz
#
# Or, Obviously, To The wxPython Mailing List!!!
#
# End Of Comments
# --------------------------------------------------------------------------- #

"""
This module contains the definitions of `PersistentObject` and `PersistenceManager` objects.
"""

import wx
import os
import warnings
import datetime

import wx.gizmos

from persist_handlers import FindHandler

from persist_constants import BAD_DEFAULT_NAMES, CONFIG_PATH_SEPARATOR
from persist_constants import PM_DEFAULT_STYLE

# ----------------------------------------------------------------------------------- #

class PersistentObject(object):
    """
    PersistentObject: ABC for anything persistent.

    This is the base class for persistent object adapters.
    wxPython persistence framework is non-intrusive, i.e. can work with the
    classes which have no relationship to nor knowledge of it. To allow this,
    an intermediate persistence adapter is used: this is just a simple object
    which provides the methods used by L{PersistenceManager} to save and restore
    the object properties and implements them using the concrete class methods.

    You may derive your own classes from L{PersistentObject} to implement persistence
    support for your common classes, see :ref:`persistent-windows` in the
    `__init__.py` file.
    """
    
    def __init__(self, window, persistenceHandler=None):
        """
        Default class constructor.

        :param `window`: an instance of `wx.Window`;
        :param `persistenceHandler`: if not ``None``, this should a custom handler derived
         from L{persist_handlers.AbstractHandler}.
        """

        self._name = window.GetName()

        if not self._name.strip():
            raise Exception("Persistent windows should be named! (class=%s)"%window.__class__)

        klass = window.__class__
        if issubclass(klass, wx.GenericDirCtrl):
            self._window = window.GetTreeCtrl()
        elif issubclass(klass, wx.gizmos.EditableListBox):
            self._window = window.GetListCtrl()
        else:
            self._window = window

        if persistenceHandler is not None:
            self._persistentHandler = persistenceHandler(self)
        else:
            self._persistentHandler = FindHandler(self)
        
        if self._name in BAD_DEFAULT_NAMES:
            warnings.warn("Window names should not be defaulted! (class=%s, name=%s)"%(window.__class__, window.GetName()))


    def GetName(self):
        """
        Returns the string uniquely identifying the window we're associated with
        among all the other objects of the same type.
        
        :note: This method is used together with L{GetKind} to construct the unique
         full name of the object in e.g. a configuration file.
        """

        return self._name


    def GetWindow(self):
        """ Returns the actual associated window. """

        return self._window


    def GetKind(self):
        """
        Returns the string uniquely identifying the objects supported by this adapter.

        :note: This method is called from L{SaveValue} and L{RestoreValue} and normally
         returns some short (but not too cryptic) strings, e.g. "Checkbox".
        """

        return self._persistentHandler.GetKind()
    

    def Save(self):
        """
        Saves the corresponding window settings. 

        :note: This method shouldn't be used directly as it doesn't respect the
         global L{PersistenceManager.DisableSaving} settings, use L{PersistenceManager}
         methods with the same name instead.
        """

        self._persistentHandler.Save()
    

    def Restore(self):
        """
        Restores the corresponding window settings. 

        :note: This method shouldn't be used directly as it doesn't respect the
         global L{PersistenceManager.DisableRestoring} settings, use L{PersistenceManager}
         methods with the same name instead.
        """

        self._persistentHandler.Restore()


    def SaveValue(self, name, value):
        """
        Save the specified value using the given name.
        
        :param `name`: the name of the value in the configuration file.
        :param `value`: the value to save.

        :returns: ``True`` if the value was saved or ``False`` if an error occurred.
        """
    
        return PersistenceManager.Get().SaveValue(self, name, value)


    def RestoreValue(self, name):
        """
        Restore the value saved by L{Save}.
        
        :param `name`: the same name as was used by L{Save}.

        :returns: ``True`` if the value was successfully read or ``False`` if it was not
         found or an error occurred.
        """

        return PersistenceManager.Get().RestoreValue(self, name)

# ----------------------------------------------------------------------------------- #

class PersistenceManager(object):
    """
    PersistenceManager: global aspects of persistent windows.

    Provides support for automatically saving and restoring object properties
    to persistent storage.

    This class is the central element of wxPython persistence framework, see
    the :ref:`persistent-overview` in the `__init__.py` file for its overview.

    This is a singleton class and its unique instance can be retrieved using L{PersistenceManager.Get}
    method.
    """
    
    def __init__(self):
        """
        Default class constructor.

        This method should **not** be called directly: you should use the object
        obtained by L{PersistenceManager.Get} and assign manager styles, custom
        configuration files and custom configuration handlers using the appropriate
        methods in this class.

        Interesting attributes you can set for this class are:
        
        - `configFile`: the persistent configuration file for L{PersistenceManager},
          a custom file name to which `wx.FileConfig` will access to store and
          retrieve UI settings;
        - `configKey`: the persistent key name inside the configuration file for
          L{PersistenceManager};
        - `customConfigHandler`: the persistent configuration handler for L{PersistenceManager};
          this attribute is object capable of saving/restoring UI settings. This
          can be a cPickle object or a ConfigObj one, for example.
        - `style`: a combination of the following values:

          ======================================== ==================================
          Flag name                                Description
          ======================================== ==================================
          ``PM_SAVE_RESTORE_AUI_PERSPECTIVES``     If a toplevel window has an AUI manager associated, the manager will save and restore its AUI perspective
          ``PM_SAVE_RESTORE_TREE_LIST_SELECTIONS`` If set, the manager will save items selections in list and tree controls
          ``PM_DEFAULT_STYLE``                     Same as ``PM_SAVE_RESTORE_AUI_PERSPECTIVES``
          ======================================== ==================================
        
        :note: UI settings are stored as dictionaries key <=> tuple: the tuple value
         contains two items. The first is the value *type* (i.e., float, int, bool etc...)
         while the second is the actual key value.
        
        """

        object.__init__(self)

        # Specifies custom wx.Config object to use (i.e., custom file names)
        self._configFile = None

        # Specifies custom key in the wx.Config object to use
        self._configKey = None
        
        # Specifies whether a custom config handler exists, so that we will not use
        # wx.FileConfig (i.e., ConfigObj, ConfigParser etc...)
        self._customConfigHandler = None
        
        # Specifies the PersistenceManager style
        self._style = PM_DEFAULT_STYLE
        
        # Set these values to True if we should restore/save the settings (it doesn't
        # make much sense to use this class when both of them are False but setting
        # one of them to False may make sense in some situations)
        self._doSave = True
        self._doRestore = True

        # map with the registered objects as keys and associated
        # PersistentObjects as values        
        self._persistentObjects = {}
        

    def Get(self):
        """ Accessor to the unique persistence manager object. """

        if not hasattr(self, "_instance"):        
            self._instance = PersistenceManager()

        return self._instance

    Get = classmethod(Get)

    
    def Free(self):
        """ Destructor for the unique persistence manager object. """

        if hasattr(self, "_instance"):        
            del self._instance

    Free = classmethod(Free)


    def GetManagerStyle(self):
        """
        Returns the L{PersistenceManager} style.

        :see: L{SetManagerStyle} for a list of possible styles.
        """
        
        return self._style        


    def SetManagerStyle(self, style):
        """
        Sets the L{PersistenceManager} style.

        :param `style`: a combination of the following values:

        ======================================== ==================================
        Flag name                                Description
        ======================================== ==================================
        ``PM_SAVE_RESTORE_AUI_PERSPECTIVES``     If a toplevel window has an AUI manager associated, the manager will save and restore its AUI perspective
        ``PM_SAVE_RESTORE_TREE_LIST_SELECTIONS`` If set, the manager will save items selections in list and tree controls
        ``PM_DEFAULT_STYLE``                     Same as ``PM_SAVE_RESTORE_AUI_PERSPECTIVES``.        
        ======================================== ==================================
        """

        self._style = style
        
        
    def SetPersistenceKey(self, key):
        """
        Sets the persistent key name inside the configuration file for L{PersistenceManager}.

        :param `key`: a short meaningful name for your unique preferences key.

        :note: Calling this method has no influence if you are using your own
         custom configuration handler (i.e., by using ConfigObj/ConfigParser/cPickle etc...).        
        """

        self._configKey = key
        

    def GetPersistenceKey(self):
        """
        Returns the persistent key name inside the configuration file for L{PersistenceManager}.

        :note: The return value of this method is not used if you are using your own
         custom configuration handler (i.e., by using ConfigObj/ConfigParser/cPickle etc...).
        """

        return self._configKey


    def SetPersistenceFile(self, fileName):
        """
        Sets the persistent configuration file for L{PersistenceManager}.

        :param `fileName`: the file name where to store the persistent options.

        :note: Calling this method has no influence if you are using your own
         custom configuration handler (i.e., by using ConfigObj/ConfigParser/cPickle etc...).
        """

        self._configFile = fileName
        

    def GetPersistenceFile(self):
        """
        Returns the persistent configuration file for L{PersistenceManager}.

        :note: The return value of this method is not used if you are using your own
         custom configuration handler (i.e., by using ConfigObj/ConfigParser/cPickle etc...).
        """

        if self._configFile is not None:
            persistenceDir, fileName = os.path.split(self._configFile)
            fileName = self._configFile
        else:
            persistenceDir = self.GetPersistenceDirectory()
            fileName = "Persistence_Options"
            
        fileName = os.path.join(persistenceDir, fileName)

        if not os.path.exists(persistenceDir):
            # Create the data folder, it still doesn't exist
            os.makedirs(persistenceDir)

        config = wx.FileConfig(localFilename=fileName)
        return config


    def SetConfigurationHandler(self, handler):
        """
        Sets the persistent configuration handler for L{PersistenceManager}.

        :param `handler`: an object capable of saving/restoring UI settings. This
         can be a cPickle object or a ConfigObj one, for example.

        :note: UI settings are stored as dictionaries key <=> tuple: the tuple value
         contains two items. The first is the value *type* (i.e., float, int, bool etc...)
         while the second is the actual key value.
        """

        self._customConfigHandler = handler
        

    def GetConfigurationHandler(self):
        """
        Returns the persistent configuration handler for L{PersistenceManager}.
        """

        return self._customConfigHandler        

        
    def GetPersistenceDirectory(self):
        """
        Returns a default persistent option directory for L{PersistenceManager}.

        :note: The return value of this method is not used if you are using your own
         custom configuration handler (i.e., by using ConfigObj/ConfigParser/cPickle etc...)
         or if you have specified a custom configuration file to use with `wx.FileConfig`.
        """
        
        sp = wx.StandardPaths.Get()
        return sp.GetUserDataDir()


    def Find(self, window):
        """
        Checks if the object is registered and return the associated L{PersistentObject}
        if it is or ``None`` otherwise.

        :param `window`: an instance of `wx.Window`.
        """
    
        if window.GetName() in self._persistentObjects:
            return window
    

    def Register(self, window, persistenceHandler=None):
        """
        Register an object with the manager.

        :param `window`: an instance of `wx.Window`;
        :param `persistenceHandler`: if not ``None``, this should a custom handler derived
         from L{persist_handlers.AbstractHandler}.

        :note: Note that registering the object doesn't do anything except allowing to call
         L{Restore} for it later. If you want to register the object and restore its
         properties, use L{RegisterAndRestore}.

        :note: The manager takes ownership of the L{PersistentObject} and will delete it when
         it is unregistered.         
        """
    
        if self.Find(window):
            raise Exception("Object (class=%s, name=%s) is already registered"%(window.__class__, window.GetName()))

        name = window.GetName()
        self._persistentObjects[name] = PersistentObject(window, persistenceHandler)

        return True
    

    def Unregister(self, window):
        """
        Unregister the object, this is called by L{PersistentObject} itself so there is
        usually no need to do it explicitly.

        :param `window`: an instance of `wx.Window`, which must have been previously
         registered with L{Register}.

        :note: For the persistent windows this is done automatically (via L{SaveAndUnregister})
         when the window is destroyed so you only need to call this function explicitly if you
         are using custom persistent objects or if you want to prevent the object properties
         from being saved.
         
        :note: This deletes the associated L{PersistentObject}.
        """
    
        if not self.Find(window):
            return False

        name = window.GetName()
        self._persistentObjects.pop(name)

        return True
    

    def Save(self, window):
        """
        Saves the state of an object.

        :param `window`: an instance of `wx.Window`.
        
        :note: This methods does nothing if L{DisableSaving} was called.
        """
    
        if not self._doSave:
            return False

        if not self.Find(window):
            return False

        name = window.GetName()
        self._persistentObjects[name].Save()
        
        return True
    
    
    def Restore(self, window):
        """
        Restores the state of an object.

        :param `window`: an instance of `wx.Window`.

        :returns: ``True`` if the object properties were restored or ``False`` if nothing
         was found to restore or the saved settings were invalid.

        :note: This methods does nothing if L{DisableRestoring} was called.
        """

        if not self._doRestore:
            return False

        if not self.Find(window):
            return False

        name = window.GetName()
        self._persistentObjects[name].Restore()

        return True
    

    def DisableSaving(self):
        """
        Globally disables saving the persistent properties (enabled by default).

        :note: By default, saving properties in L{Save} is enabled but the program
         may wish to disable if, for example, it detects that it is running on a
         system which shouldn't be modified in any way and so configuration file
         (or Windows registry) shouldn't be written to.
        """
        
        self._doSave = False


    def DisableRestoring(self):
        """
        Globally disables restoring the persistent properties (enabled by default).

        :note: By default, restoring properties in L{Restore} is enabled but this
         function allows to disable it. This is mostly useful for testing.
        """
        
        self._doRestore = False


    def EnableSaving(self):
        """
        Globally enables saving the persistent properties (enabled by default).

        :note: By default, saving properties in L{Save} is enabled but the program
         may wish to disable if, for example, it detects that it is running on a
         system which shouldn't be modified in any way and so configuration file
         (or Windows registry) shouldn't be written to.        
        """
        
        self._doSave = True


    def EnableRestoring(self):
        """
        Globally enables restoring the persistent properties (enabled by default).

        :note: By default, restoring properties in L{Restore} is enabled but this
         function allows to disable it. This is mostly useful for testing.
        """
        
        self._doRestore = True


    def SaveAndUnregister(self, window=None):
        """
        Combines both L{Save} and L{Unregister} calls.

        :param `window`: an instance of `wx.Window`. If it is ``None``, all the
         windows previously registered are saved and then unregistered.
        """

        if window is None:
            for name, obj in self._persistentObjects.items():
                self.SaveAndUnregister(obj.GetWindow())

            return

        self.Save(window)
        self.Unregister(window)


    def RegisterAndRestore(self, window):
        """
        Combines both L{Register} and L{Restore} calls.

        :param `window`: an instance of `wx.Window`.
        """
        
        return self.Register(window) and self.Restore(window)


    def GetKey(self, obj, keyName):
        """
        Returns a correctly formatted key name for the object `obj` and `keyName` parameters.

        :param `obj`: an instance of L{PersistentObject};
        :param `keyName`: a string specifying the key name.

        """

        key = (self._configKey is None and ["Persistence_Options"] or [self._configKey])[0]       
        
        key += CONFIG_PATH_SEPARATOR + obj.GetKind()
        key += CONFIG_PATH_SEPARATOR + obj.GetName()
        key += CONFIG_PATH_SEPARATOR + keyName
        
        return key

    
    def SaveValue(self, obj, keyName, value):
        """
        Method used by the persistent objects to save the data.

        By default this method simply use `wx.FileConfig` but this behaviour may be
        overridden by passing a custom configuration handler in the L{PersistenceManager}
        constructor.

        :param `obj`: an instance of L{PersistentObject};
        :param `keyName`: a string specifying the key name;
        :param `value`: the value to store in the configuration file.
        
        """

        kind = repr(value.__class__).split("'")[1]
    
        if self._customConfigHandler is not None:
            result = self._customConfigHandler.SaveValue(self.GetKey(obj, keyName), repr((kind, str(value))))
        else:
            config = self.GetPersistenceFile()
            result = config.Write(self.GetKey(obj, keyName), repr((kind, str(value))))
            config.Flush()

        return result        
    

    def RestoreValue(self, obj, keyName):
        """
        Method used by the persistent objects to restore the data.
        
        By default this method simply use `wx.FileConfig` but this behaviour may be
        overridden by passing a custom config handler in the PersistenceManager
        constructor.

        :param `obj`: an instance of L{PersistentObject};
        :param `keyName`: a string specifying the key name.
        """

        if self._customConfigHandler is not None:
            result = self._customConfigHandler.RestoreValue(self.GetKey(obj, keyName))
        else:
            config = self.GetPersistenceFile()
            result = config.Read(self.GetKey(obj, keyName))

        if result:
            kind, result = eval(result)
            if kind in ("unicode", "str"):
                return result
            elif kind == "datetime.date":
                y, m, d = result.split("-")
                result = datetime.date(int(y), int(m), int(d))
                return result
            
            return eval(result)

        
