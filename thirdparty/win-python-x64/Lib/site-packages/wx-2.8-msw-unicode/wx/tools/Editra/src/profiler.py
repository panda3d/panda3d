###############################################################################
# Name: profiler.py                                                           #
# Purpose: Editra's user profile services                                     #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This module provides the profile object and support functions for loading and
saving user preferences between sessions. The preferences are saved on disk as
a cPickle, because of this no objects that cannot be resolved in the namespace
of this module prior to starting the mainloop must not be put in the Profile as
it will cause errors on load. Ths means that only builtin python types should
be used and that a translation from that type to the required type should
happen during run time.

@summary: Editra's user profile management

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: profiler.py 67855 2011-06-04 20:11:21Z CJP $"
__revision__ = "$Revision: 67855 $"

#--------------------------------------------------------------------------#
# Imports
import os
import sys
import cPickle
import wx

# Editra Imports
from ed_glob import CONFIG, PROG_NAME, VERSION, PRINT_BLACK_WHITE, EOL_MODE_LF
import util
import dev_tool
import ed_msg

_ = wx.GetTranslation
#--------------------------------------------------------------------------#
# Globals
_DEFAULTS = {
           'ALPHA'      : 255,              # Transparency level (0-255)
           'AALIASING'  : False,            # Use Anti-Aliasing if availble
           'APPSPLASH'  : True,             # Show splash at startup
           'AUTOBACKUP' : False,            # Automatically backup files
           'AUTOBACKUP_PATH' : '',          # Backup path
           'AUTO_COMP'  : True,             # Use Auto-comp if available
           'AUTO_COMP_EX' : False,          # Use extended autocompletion
           'AUTO_INDENT': True,             # Use Auto Indent
           'AUTO_TRIM_WS' : False,          # Trim whitespace on save
           'AUTO_RELOAD' : False,           # Automatically reload files?
           'BRACKETHL'  : True,             # Use bracket highlighting
           'BSUNINDENT' : True,             # Backspace Unindents
           'CTRLBAR'    : dict(),           # ControlBar layouts
           'CHECKMOD'   : True,             # Auto check file for file mod
           'CHECKUPDATE': True,             # Check for updates on start
           'CODE_FOLD'  : True,             # Use code folding
           'DEFAULT_LEX': 'Plain Text',     # Default lexer for new documents
           'DEFAULT'    : False,            # No longer used I believe
           'DEFAULT_VIEW' : 'Automatic',    # Default Perspective
           'EDGE'       : 80,               # Edge guide column
           'ENCODING'   : None,             # Prefered text encoding
           'EOL_MODE'   : EOL_MODE_LF,      # EOL mode 1 == LF, 2 == CRLF
           'FHIST'      : list(),           # List of history files
           'FHIST_LVL'  : 9,                # Filehistory length (9 is max)
           'FFILTER'    : 0,                # Last file filter used
           'GUIDES'     : False,            # Use Indentation guides
           'HLCARETLINE': False,            # Highlight Caret Line
           'ICONS'      : 'Tango',          # Icon Theme
           'ICON_SZ'    : (16, 16),         # Toolbar Icon Size
           'INDENTWIDTH': 4,                # Default indent width
           'ISBINARY'   : False,            # Is this instance a binary
           'KEY_PROFILE': None,             # Keybinding profile
           'LANG'       : 'Default',        # UI language
           'LASTCHECK'  : 0,                # Last time update check was done
           #'LEXERMENU'  : [lang_name,]     # Created on an as needed basis
           'MAXIMIZED'  : False,            # Was window maximized on exit
           'MODE'       : 'CODE',           # Overall editor mode
           'MYPROFILE'  : 'default.ppb',    # Path to profile file
           'OPEN_NW'    : False,            # Open files in new windows
           'PRINT_MODE' : PRINT_BLACK_WHITE,# Printer rendering mode
           'PROXY_SETTINGS' : dict(),       # Proxy Server Settings
           'REPORTER'   : True,             # Error Reporter is Active
           'SAVE_POS'   : True,             # Remember Carat positions
           'SAVE_SESSION' : False,          # Load previous session on startup
           'SEARCH_LOC' : list(),           # Recent Search Locations
           'SEARCH_FILTER' : '',            # Last used search filter
           'SESSION_KEY' : '',              # Ipc Session Server Key
           'SET_WPOS'   : True,             # Remember window position
           'SET_WSIZE'  : True,             # Remember mainwindow size on exit
           'SHOW_EDGE'  : True,             # Show Edge Guide
           'SHOW_EOL'   : False,            # Show EOL markers
           'SHOW_LN'    : True,             # Show Line Numbers
           'SHOW_WS'    : False,            # Show whitespace markers
           'SPELLCHECK' : dict(auto=False,
                               dict='en_US',
                               epath=None), # Spell checking preferences
           'STATBAR'    : True,             # Show Status Bar
           'SYNTAX'     : True,             # Use Syntax Highlighting
           'SYNTHEME'   : 'Default',        # Syntax Highlight color scheme
           'TABICONS'   : True,             # Show Tab Icons
           'TABWIDTH'   : 8,                # Tab width
           'THEME'      : 'DEFAULT',        # For future use
           'TOOLBAR'    : True,             # Show Toolbar
           'USETABS'    : False,            # Use tabs instead of spaces
           'USE_PROXY'  : False,            # Use Proxy server settings?
           'VIEWVERTSPACE' : False,         # Allow extra virtual space in buffer
           'VI_EMU'     : False,            # Use Vi emulation mode
           'VI_NORMAL_DEFAULT' : False,     # Use Normal mode by default
           'WARN_EOL'   : True,             # Warn about mixed eol characters
           'WRAP'       : False,            # Use Wordwrap
           'WSIZE'      : (700, 450)        # Mainwindow size
          #FONT1 created at runtime by ed_styles as primary font
          #FONT2 created at runtime by ed_styles as secondary font
          #FONT3 Standard Font by UI, added to profile when customized
}

#--------------------------------------------------------------------------#

class Profile(dict):
    """Class for managing profile data. All data is stored as builtin
    python objects (i.e. str, tuple, list, ect...) however on a request
    for data the object can be transformed in to a requested type where
    applicable. The profile saves itself to disk using the cPickle module
    to preserve data types and allow for easy loading.

    """
    _instance = None
    _created = False

    def __init__(self):
        """Initialize the profile"""
        if not self._created:
            dict.__init__(self)
        else:
            pass

    def __new__(cls, *args, **kargs):
        """Maintain only a single instance of this object
        @return: instance of this class

        """
        if cls._instance is None:
            cls._instance = dict.__new__(cls, *args, **kargs)
        return cls._instance

    #---- End Private Members ----#

    #---- Begin Public Members ----#
    def DeleteItem(self, item):
        """Removes an entry from the profile

        @param item: items name
        @type item: string

        """
        if item in self:
            del self[item]
        else:
            pass

    def Get(self, index, fmt=None, default=None):
        """Gets the specified item from the data set

        @param index: index of item to get
        @keyword fmt: format the item should be in
        @keyword default: Default value to return if index is
                          not in profile.

        """
        if index in self:
            val = self.__getitem__(index)
        else:
            return default

        if fmt is None:
            return val
        else:
            return _ToObject(index, val, fmt)

    def Load(self, path):
        """Load the profiles data set with data from the given file
        @param path: path to file to load data from
        @note: The files data must have been written with a pickler

        """
        if os.path.exists(path):
            try:
                fhandle = open(path, 'rb')
                val = cPickle.load(fhandle)
                fhandle.close()
            except (IOError, SystemError, OSError,
                    cPickle.UnpicklingError, EOFError), msg:
                dev_tool.DEBUGP("[profile][err] %s" % str(msg))
            else:
                if isinstance(val, dict):
                    self.update(val)
                    self.Set('MYPROFILE', path)
                    dev_tool.DEBUGP("[profile][info] Loaded %s" % path)
        else:
            dev_tool.DEBUGP("[profile][err] %s does not exist" % path)
            dev_tool.DEBUGP("[profile][info] Loading defaults")
            self.LoadDefaults()
            self.Set('MYPROFILE', path)
            return False

        # Update profile to any keys that are missing
        for key in _DEFAULTS:
            if key not in self:
                self.Set(key, _DEFAULTS[key])
        return True

    def LoadDefaults(self):
        """Loads the default values into the profile
        @return: None

        """
        self.clear()
        self.update(_DEFAULTS)

    def Set(self, index, val, fmt=None):
        """Set the value of the given index
        @param index: Index to set
        @param val: Value to set
        @keyword fmt: Format to convert to string from

        """
        if fmt is None:
            self.__setitem__(index, val)
        else:
            self.__setitem__(index, _FromObject(val, fmt))

        # Notify all clients with the configuration change message
        ed_msg.PostMessage(ed_msg.EDMSG_PROFILE_CHANGE + (index,), val) 

    def Write(self, path):
        """Write the dataset of this profile as a pickle
        @param path: path to where to write the pickle
        @return: True on success / False on failure

        """
        try:
            # Only write if given an absolute path
            if not os.path.isabs(path):
                return False
            self.Set('MYPROFILE', path)
            fhandle = open(path, 'wb')
            cPickle.dump(self.copy(), fhandle, cPickle.HIGHEST_PROTOCOL)
            fhandle.close()
            UpdateProfileLoader()
        except (IOError, cPickle.PickleError), msg:
            dev_tool.DEBUGP(u"[profile][err] %s" % msg)
            return False
        else:
            return True

    def Update(self, update=None):
        """Update the profile using data from provided dictionary
        or the default set if none is given.
        @keyword update: dictionary of values to update from or None
        @postcondition: All profile values from the update set are set
                        in this profile. If update is None then the current
                        set is only updated to include values from the
                        DEFAULTS that are not currently present.

        """
        if update is None:
            for key, val in _DEFAULTS.iteritems():
                if key not in self:
                    self.Set(key, val)
        else:
            self.update(update)

    #---- End Public Members ----#

#-----------------------------------------------------------------------------#
# Singleton reference instance

TheProfile = Profile()

#-----------------------------------------------------------------------------#
# Profile convenience functions
Profile_Del  = TheProfile.DeleteItem
Profile_Get = TheProfile.Get
Profile_Set = TheProfile.Set

def _FromObject(val, fmt):
    """Convert the given value to a to a profile compatible value
    @param val: value to convert
    @param fmt: Format to convert to
    @type fmt: string

    """
    if fmt == u'font' and isinstance(val, wx.Font):
        return "%s,%s" % (val.GetFaceName(), val.GetPointSize())
    else:
        return val

def _ToObject(index, val, fmt):
    """Convert the given value to a different object
    @param index: fallback to retrieve item from defaults
    @param val: value to convert
    @param fmt: Format to convert to
    @type fmt: string
    @todo: exception handling,

    """
    tmp = fmt.lower()
    if tmp == u'font':
        fnt = val.split(',')
        rval = wx.FFont(int(fnt[1]), wx.DEFAULT, face=fnt[0])
    elif tmp == u'bool':
        if isinstance(val, bool):
            rval = val
        else:
            rval = _DEFAULTS.get(index, False)
    elif tmp == u'size_tuple':
        if len(val) == 2 and \
           isinstance(val[0], int) and isinstance(val[1], int):
            rval = val
        else:
            rval = _DEFAULTS.get(index, wx.DefaultSize)
    elif tmp == u'str':
        rval = unicode(val)
    elif tmp == u'int':
        if isinstance(val, int):
            rval = val
        elif isinstance(val, basestring) and val.isdigit():
            rval = int(val)
        else:
            rval = _DEFAULTS.get(index)
    else:
        return val
    return rval

#---- Begin Function Definitions ----#

def CalcVersionValue(ver_str="0.0.0"):
    """Calculates a version value from the provided dot-formated string

    1) SPECIFICATION: Version value calculation AA.BBB.CCC
         - major values: < 1     (i.e 0.0.85 = 0.850)
         - minor values: 1 - 999 (i.e 0.1.85 = 1.850)
         - micro values: >= 1000 (i.e 1.1.85 = 1001.850)

    @keyword ver_str: Version string to calculate value of

    """
    ver_str = ''.join([char for char in ver_str
                       if char.isdigit() or char == '.'])
    ver_lvl = ver_str.split(u".")
    if len(ver_lvl) < 3:
        return 0

    major = int(ver_lvl[0]) * 1000
    minor = int(ver_lvl[1])
    if len(ver_lvl[2]) <= 2:
        ver_lvl[2] += u'0'
    micro = float(ver_lvl[2]) / 1000
    return float(major) + float(minor) + micro

def GetLoader():
    """Finds the loader to use
    @return: path to profile loader
    @note: path may not exist, only returns the path to where the loader
           should be.

    """
    cbase = util.GetUserConfigBase()
    loader = os.path.join(cbase, u"profiles", u".loader2")
    return loader

def GetProfileStr():
    """Reads the profile string from the loader and returns it.
    The profile string must be the first line in the loader file.
    @return: path of profile used in last session

    """
    reader = util.GetFileReader(GetLoader())
    if reader == -1:
        # So return the default
        return CONFIG['PROFILE_DIR'] + u"default.ppb"

    profile = reader.readline()
    profile = profile.split("\n")[0] # strip newline from end
    reader.close()
    if not os.path.isabs(profile):
        profile = CONFIG['PROFILE_DIR'] + profile
    return profile

def ProfileIsCurrent():
    """Checks if profile is compatible with current editor version
    and returns a bool stating if it is or not.
    @return: whether profile on disk was written with current program version

    """
    if CalcVersionValue(ProfileVersionStr()) >= CalcVersionValue(VERSION):
        return True
    else:
        return False

def ProfileVersionStr():
    """Checks the Loader for the profile version string and
    returns the version string. If there is an error or the
    string is not found it returns a zero version string.
    @return: the version string value from the profile loader file

    """
    loader = GetLoader()
    reader = util.GetFileReader(loader, sys.getfilesystemencoding())
    if reader == -1:
        return "0.0.0"

    ret_val = "0.0.0"
    count = 0
    while True:
        count += 1
        value = reader.readline()
        value = value.split()
        if len(value) > 0:
            if value[0] == u'VERSION':
                ret_val = value[1]
                break
        # Give up after 20 lines if version string not found
        if count > 20:
            break
    reader.close()

    return ret_val

def UpdateProfileLoader():
    """Updates Loader File
    @precondition: MYPROFILE has been set
    @postcondition: on disk profile loader is updated
    @return: 0 if no error, non zero for error condition

    """
    writer = util.GetFileWriter(GetLoader())
    if writer == -1:
        return 1

    prof_name = Profile_Get('MYPROFILE')
    if not prof_name or not os.path.isabs(prof_name):
        prof_name = CONFIG['PROFILE_DIR'] + 'default.ppb'

    if not os.path.exists(prof_name):
        prof_name = os.path.join(CONFIG['CONFIG_DIR'],
                                 os.path.basename(prof_name))
        Profile_Set('MYPROFILE', prof_name)

    # Use just the relative profile name for local(portable) config paths
    prof_name = os.path.basename(prof_name)

    writer.write(prof_name)
    writer.write(u"\nVERSION\t" + VERSION)
    writer.close()
    return 0
