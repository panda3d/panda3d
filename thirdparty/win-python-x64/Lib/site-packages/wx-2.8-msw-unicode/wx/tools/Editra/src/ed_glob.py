###############################################################################
# Name: ed_glob.py                                                            #
# Purpose: Global IDs/objects used throughout Editra                          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
This file contains variables that are or may be used in multiple files and
libraries within the project. Its purpose is to create a globally accessible
access point for all common variables in the project.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_glob.py 67594 2011-04-24 15:24:40Z CJP $"
__revision__ = "$Revision: 67594 $"

__all__ = [ 'CONFIG', 'SB_INFO', 'VERSION', 'PROG_NAME', 'ID_NEW', 'ID_OPEN',
            'ID_CLOSE', 'ID_CLOSEALL', 'ID_SAVE', 'ID_SAVEAS', 'ID_SAVEALL',
            'ID_SAVE_PROFILE', 'ID_LOAD_PROFILE', 'ID_PRINT', 'ID_PRINT_PRE',
            'ID_PRINT_SU', 'ID_EXIT', 'ID_UNDO', 'ID_REDO', 'ID_CUT',
            'ID_COPY', 'ID_PASTE', 'ID_SELECTALL', 'ID_ADD_BM',
            'ID_DEL_ALL_BM', 'ID_LINE_AFTER', 'ID_LINE_BEFORE', 'ID_CUT_LINE',
            'ID_COPY_LINE', 'ID_JOIN_LINES', 'ID_TRANSPOSE', 'ID_DELETE_LINE',
            'ID_LINE_MOVE_UP', 'ID_LINE_MOVE_DOWN',
            'ID_QUICK_FIND', 'ID_PREF', 'ID_ZOOM_OUT',
            'HOME_PAGE', 'CONTACT_MAIL', 'ID_ZOOM_IN', 'ID_ZOOM_NORMAL',
            'ID_SHOW_EDGE', 'ID_SHOW_EOL', 'ID_SHOW_LN', 'ID_SHOW_WS',
            'ID_PERSPECTIVES', 'ID_INDENT_GUIDES', 'ID_VIEW_TOOL',
            'ID_GOTO_LINE', 'ID_NEXT_MARK', 'ID_PRE_MARK', 'ID_FONT',
            'ID_EOL_MAC', 'ID_EOL_UNIX', 'ID_EOL_WIN', 'ID_WORD_WRAP',
            'ID_INDENT', 'ID_UNINDENT', 'ID_TO_UPPER', 'ID_TO_LOWER',
            'ID_SPACE_TO_TAB', 'ID_TAB_TO_SPACE', 'ID_TRIM_WS',
            'ID_TOGGLECOMMENT', 'ID_AUTOCOMP', 'ID_AUTOINDENT', 'ID_SYNTAX',
            'ID_FOLDING', 'ID_BRACKETHL', 'ID_LEXER',
            'ID_PLUGMGR', 'ID_STYLE_EDIT', 'ID_MACRO_START', 'ID_MACRO_STOP',
            'ID_MACRO_PLAY', 'ID_ABOUT', 'ID_HOMEPAGE', 'ID_CONTACT',
            'ID_BUG_TRACKER', 'ID_DOCUMENTATION', 'ID_COMMAND',
            'ID_USE_SOFTTABS', 'ID_DUP_LINE', 'ID_TRANSLATE',
            'I18N_PAGE', 'ID_GOTO_MBRACE', 'ID_HLCARET_LINE', 'ID_SHOW_SB',
            'ID_REVERT_FILE', 'ID_RELOAD_ENC', 'ID_DOCPROP', 'ID_PASTE_AFTER',
            'ID_COLUMN_MODE', 'ID_PANELIST', 'ID_MAXIMIZE_EDITOR',
            'ID_NEW_WINDOW', 'ID_TOGGLE_FOLD', 'ID_TOGGLE_ALL_FOLDS',
            'ID_SAVE_SESSION', 'ID_LOAD_SESSION', 'ID_NEXT_POS', 'ID_PRE_POS',
            'ID_CYCLE_CLIPBOARD', 'ID_LEXER_CUSTOM', 'ID_SHOW_AUTOCOMP',
            'ID_SHOW_CALLTIP' ]

#---- Project Info ----#
# The project info was moved to another module so it could be accessed
# externally without needing to import anything else.  It's imported
# here with a * until there isn't anyplace left that expects to find
# these values in this module.
from info import *

#---- End Project Info ----#

#---- Imported Libs/Objects ----#
import wx

_ = wx.GetTranslation

#---- WX Compatibility Hacks ----#
import wxcompat

#---- Configuration Locations ----#
# Values set when main loads
CONFIG = {
          'ISLOCAL'     : False, # Using local config (no abs path)
          'CONFIG_BASE' : None, # Set if config base is in nonstandard location
          'INSTALL_DIR' : "",   # Instal directory
          'CONFIG_DIR'  : "",   # Root configration directory
          'CACHE_DIR'   : "",   # Holds temp data about documents
          'KEYPROF_DIR' : "",   # System Keybinding
          'PROFILE_DIR' : "",   # User Profile Directory
          'PLUGIN_DIR'  : "",   # User Plugin Dir
          'SYSPIX_DIR'  : "",   # Editras non user graphics
          'THEME_DIR'   : "",   # Theme Directory
          'LANG_DIR'    : "",   # Locale Data Directory
          'SYS_PLUGIN_DIR' : "", # Editra base plugin dir
          'SYS_STYLES_DIR' : "", # Editra base style sheets
          'TEST_DIR'    : "",   # Test data files dir
}

# Global logging/application variables
DEBUG = False
VDEBUG = False
SINGLE = True

#---- Object ID's ----#
# File Menu IDs
ID_NEW           = wx.ID_NEW
ID_NEW_WINDOW    = wx.NewId()
ID_OPEN          = wx.ID_OPEN
ID_FHIST         = wx.NewId()
ID_CLOSE         = wx.ID_CLOSE
ID_CLOSEALL      = wx.ID_CLOSE_ALL
ID_CLOSE_OTHERS  = wx.NewId()
ID_CLOSE_WINDOW  = wx.NewId()
ID_SAVE          = wx.ID_SAVE
ID_SAVEAS        = wx.ID_SAVEAS
ID_SAVEALL       = wx.NewId()
ID_REVERT_FILE   = wx.ID_REVERT_TO_SAVED
ID_RELOAD_ENC    = wx.NewId()
ID_SAVE_PROFILE  = wx.NewId()
ID_LOAD_PROFILE  = wx.NewId()
ID_SAVE_SESSION  = wx.NewId()
ID_LOAD_SESSION  = wx.NewId()
ID_PRINT         = wx.ID_PRINT
ID_PRINT_PRE     = wx.ID_PREVIEW
ID_PRINT_SU      = wx.NewId()
ID_EXIT          = wx.ID_EXIT

# Edit Menu IDs
ID_UNDO          = wx.ID_UNDO
ID_REDO          = wx.ID_REDO
ID_CUT           = wx.ID_CUT
ID_COPY          = wx.ID_COPY
ID_PASTE         = wx.ID_PASTE
ID_CYCLE_CLIPBOARD = wx.NewId()
ID_PASTE_AFTER   = wx.NewId()
ID_SELECTALL     = wx.ID_SELECTALL
ID_COLUMN_MODE   = wx.NewId()
ID_LINE_EDIT     = wx.NewId()
ID_BOOKMARK      = wx.NewId()
ID_ADD_BM        = wx.NewId()
ID_DEL_BM        = wx.ID_REMOVE # Not used in menu anymore
ID_DEL_ALL_BM    = wx.NewId()
ID_LINE_AFTER    = wx.NewId()
ID_LINE_BEFORE   = wx.NewId()
ID_CUT_LINE      = wx.NewId()
ID_DELETE_LINE   = wx.NewId()
ID_COPY_LINE     = wx.NewId()
ID_DUP_LINE      = wx.NewId()
ID_JOIN_LINES    = wx.NewId()
ID_TRANSPOSE     = wx.NewId()
ID_LINE_MOVE_UP  = wx.NewId()
ID_LINE_MOVE_DOWN= wx.NewId()
ID_SHOW_AUTOCOMP = wx.NewId()
ID_SHOW_CALLTIP  = wx.NewId()
ID_FIND          = wx.ID_FIND
ID_FIND_PREVIOUS = wx.NewId()
ID_FIND_NEXT     = wx.NewId()
ID_FIND_REPLACE  = wx.ID_REPLACE
ID_FIND_SELECTED = wx.NewId()

# Using the system ids automatically disables the menus items
# when the dialog is open which is not wanted
if wx.Platform == '__WXMAC__':
    ID_FIND = wx.NewId()
    ID_FIND_REPLACE = wx.NewId()
ID_QUICK_FIND    = wx.NewId()
ID_PREF          = wx.ID_PREFERENCES

# Preference Dlg Ids
ID_PREF_LANG     = wx.NewId()
ID_PREF_AALIAS   = wx.NewId()
ID_PREF_AUTOBKUP = wx.NewId()
ID_PREF_AUTO_RELOAD = wx.NewId()
ID_PREF_AUTOCOMPEX = wx.NewId()
ID_PREF_AUTOTRIM = wx.NewId()
ID_PREF_CHKMOD   = wx.NewId()
ID_PREF_CHKUPDATE = wx.NewId()
ID_PREF_DLEXER   = wx.NewId()
ID_PREF_EDGE     = wx.NewId()
ID_PREF_ENCODING = wx.NewId()
ID_PREF_SYNTHEME = wx.NewId()
ID_PREF_TABS     = wx.NewId()
ID_PREF_UNINDENT = wx.NewId()
ID_PREF_TABW     = wx.NewId()
ID_PREF_INDENTW  = wx.NewId()
ID_PREF_FHIST    = wx.NewId()
ID_PREF_WSIZE    = wx.NewId()
ID_PREF_WPOS     = wx.NewId()
ID_PREF_ICON     = wx.NewId()
ID_PREF_ICONSZ   = wx.NewId()
ID_PREF_MODE     = wx.NewId()
ID_PREF_TABICON  = wx.NewId()
ID_PRINT_MODE    = wx.NewId()
ID_TRANSPARENCY  = wx.NewId()
ID_PREF_SPOS     = wx.NewId()
ID_PREF_UPDATE_BAR = wx.NewId()
ID_PREF_VIRT_SPACE = wx.NewId()
ID_PREF_WARN_EOL = wx.NewId()
ID_SESSION       = wx.NewId()

# View Menu IDs
ID_ZOOM_OUT      = wx.ID_ZOOM_OUT
ID_ZOOM_IN       = wx.ID_ZOOM_IN
ID_ZOOM_NORMAL   = wx.ID_ZOOM_100
ID_HLCARET_LINE  = wx.NewId()
ID_SHOW_EDGE     = wx.NewId()
ID_SHOW_EOL      = wx.NewId()
ID_SHOW_LN       = wx.NewId()
ID_SHOW_WS       = wx.NewId()
ID_SHOW_SHELF    = wx.NewId()
ID_PERSPECTIVES  = wx.NewId()
ID_INDENT_GUIDES = wx.NewId()
ID_SHOW_SB       = wx.NewId()
ID_VIEW_TOOL     = wx.NewId()
ID_SHELF         = wx.NewId()
ID_PANELIST      = wx.NewId()
ID_GOTO_LINE     = wx.NewId()
ID_GOTO_MBRACE   = wx.NewId()
ID_TOGGLE_FOLD   = wx.NewId()
ID_TOGGLE_ALL_FOLDS = wx.NewId()
ID_NEXT_POS      = wx.NewId()
ID_PRE_POS       = wx.NewId()
ID_NEXT_MARK     = wx.ID_FORWARD
ID_PRE_MARK      = wx.ID_BACKWARD
ID_MAXIMIZE_EDITOR = wx.NewId()

# Format Menu IDs
ID_FONT          = wx.NewId()
ID_EOL_MODE      = wx.NewId()
ID_EOL_MAC       = wx.NewId()
ID_EOL_UNIX      = wx.NewId()
ID_EOL_WIN       = wx.NewId()
ID_USE_SOFTTABS  = wx.NewId()
ID_WORD_WRAP     = wx.NewId()
ID_INDENT        = wx.ID_INDENT
ID_UNINDENT      = wx.ID_UNINDENT
ID_TO_UPPER      = wx.NewId()
ID_TO_LOWER      = wx.NewId()
ID_WS_FORMAT     = wx.NewId()
ID_SPACE_TO_TAB  = wx.NewId()
ID_TAB_TO_SPACE  = wx.NewId()
ID_TRIM_WS       = wx.NewId()
ID_TOGGLECOMMENT = wx.NewId()

# Settings Menu IDs
ID_AUTOCOMP      = wx.NewId()
ID_AUTOINDENT    = wx.NewId()
ID_SYNTAX        = wx.NewId()
ID_SYN_ON        = wx.NewId()
ID_SYN_OFF       = wx.NewId()
ID_FOLDING       = wx.NewId()
ID_BRACKETHL     = wx.NewId()
ID_LEXER         = wx.NewId()
ID_LEXER_CUSTOM  = wx.NewId()

# Tool Menu IDs
ID_COMMAND       = wx.NewId()
ID_PLUGMGR       = wx.NewId()
ID_STYLE_EDIT    = wx.ID_EDIT
ID_MACRO_START   = wx.NewId()
ID_MACRO_STOP    = wx.NewId()
ID_MACRO_PLAY    = wx.NewId()
ID_GENERATOR     = wx.NewId()
ID_HTML_GEN      = wx.NewId()
ID_TEX_GEN       = wx.NewId()
ID_RTF_GEN       = wx.NewId()
ID_RUN_LAUNCH    = wx.NewId()
ID_LAUNCH_LAST   = wx.NewId()

# Help Menu IDs
ID_ABOUT         = wx.ID_ABOUT
ID_HOMEPAGE      = wx.ID_HOME
ID_DOCUMENTATION = wx.NewId()
ID_TRANSLATE     = wx.NewId()
ID_CONTACT       = wx.NewId()
ID_BUG_TRACKER   = wx.NewId()

# Misc IDs
ID_ADD               = wx.ID_ADD
ID_ADVANCED          = wx.NewId()
ID_APP_SPLASH        = wx.NewId()
ID_BACKWARD          = wx.ID_BACKWARD
ID_BIN_FILE          = ID_COMMAND
ID_CDROM             = wx.NewId()
ID_COMMAND_LINE_OPEN = wx.NewId()
ID_COMPUTER          = wx.NewId()
ID_COPY_PATH         = wx.NewId()
ID_COPY_FILE         = wx.NewId()
ID_DELETE            = wx.NewId()
ID_DOCPROP           = wx.NewId()
ID_DOWN              = wx.ID_DOWN
ID_DOWNLOAD_DLG      = wx.NewId()
ID_FILE              = wx.ID_FILE
ID_FIND_RESULTS      = wx.NewId()
ID_FLOPPY            = wx.NewId()
ID_FOLDER            = wx.NewId()
ID_FORWARD           = wx.ID_FORWARD
ID_HARDDISK          = wx.NewId()
ID_KEY_PROFILES      = wx.NewId()
ID_LOGGER            = wx.NewId()
ID_BOOKMARK_MGR      = wx.NewId()
ID_MOVE_TAB          = wx.NewId()
ID_PACKAGE           = wx.NewId()
ID_PYSHELL           = wx.NewId()
ID_REFRESH           = wx.ID_REFRESH
ID_REMOVE            = wx.ID_REMOVE
ID_REPORTER          = wx.NewId()
ID_STOP              = wx.ID_STOP
ID_THEME             = wx.NewId()
ID_USB               = wx.NewId()
ID_UP                = wx.ID_UP
ID_VI_MODE           = wx.NewId()
ID_VI_NORMAL_DEFAULT = wx.NewId()
ID_WEB               = wx.NewId()
ID_READONLY          = wx.NewId()

# Code Elements (ids for art provider)
ID_CLASS_TYPE = wx.NewId()
ID_FUNCT_TYPE = wx.NewId()
ID_ELEM_TYPE = wx.NewId()
ID_VARIABLE_TYPE = wx.NewId()
ID_ATTR_TYPE = wx.NewId()
ID_PROPERTY_TYPE = wx.NewId()
ID_METHOD_TYPE = wx.NewId()

# Statusbar IDs
SB_INFO          = 0
SB_BUFF          = 1
SB_LEXER         = 2
SB_ENCODING      = 3
SB_EOL           = 4
SB_ROWCOL        = 5

# Print Mode Identifiers
PRINT_BLACK_WHITE = 0
PRINT_COLOR_WHITE = 1
PRINT_COLOR_DEF   = 2
PRINT_INVERT      = 3
PRINT_NORMAL      = 4

#---- Objects ----#

# Dictionary to map object ids to Profile keys
ID_2_PROF = {
             ID_PREF_AALIAS       : 'AALIASING',
             ID_TRANSPARENCY      : 'ALPHA',
             ID_PREF_UNINDENT     : 'BSUNINDENT',
             ID_APP_SPLASH        : 'APPSPLASH',
             ID_PREF_AUTOBKUP     : 'AUTOBACKUP',
             ID_AUTOCOMP          : 'AUTO_COMP',
             ID_PREF_AUTOCOMPEX   : 'AUTO_COMP_EX',
             ID_AUTOINDENT        : 'AUTO_INDENT',
             ID_PREF_AUTO_RELOAD  : 'AUTO_RELOAD',
             ID_PREF_AUTOTRIM     : 'AUTO_TRIM_WS',
             ID_BRACKETHL         : 'BRACKETHL',
             ID_PREF_CHKMOD       : 'CHECKMOD',
             ID_PREF_CHKUPDATE    : 'CHECKUPDATE',
             ID_FOLDING           : 'CODE_FOLD',
             ID_PREF_DLEXER       : 'DEFAULT_LEX',
             ID_PERSPECTIVES      : 'DEFAULT_VIEW',
             ID_PREF_EDGE         : 'EDGE',
             ID_PREF_ENCODING     : 'ENCODING',
             ID_EOL_MODE          : 'EOL_MODE',
             ID_PREF_FHIST        : 'FHIST_LVL',
             ID_INDENT_GUIDES     : 'GUIDES',
             ID_HLCARET_LINE      : 'HLCARETLINE',
             ID_PREF_ICON         : 'ICONS',
             ID_PREF_ICONSZ       : 'ICON_SZ',
             ID_PREF_INDENTW      : 'INDENTWIDTH',
             ID_KEY_PROFILES      : 'KEY_PROFILE',
             ID_PREF_LANG         : 'LANG',
             ID_PREF_MODE         : 'MODE',
             ID_NEW_WINDOW        : 'OPEN_NW',
             ID_PRINT_MODE        : 'PRINT_MODE',
             ID_REPORTER          : 'REPORTER',
             ID_PREF_SPOS         : 'SAVE_POS',
             ID_SESSION           : 'SAVE_SESSION',
             ID_PREF_WPOS         : 'SET_WPOS',
             ID_PREF_WSIZE        : 'SET_WSIZE',
             ID_SHOW_EDGE         : 'SHOW_EDGE',
             ID_SHOW_EOL          : 'SHOW_EOL',
             ID_SHOW_LN           : 'SHOW_LN',
             ID_SHOW_WS           : 'SHOW_WS',
             ID_SHOW_SB           : 'STATBAR',
             ID_SYNTAX            : 'SYNTAX',
             ID_PREF_SYNTHEME     : 'SYNTHEME',
             ID_PREF_TABICON      : 'TABICONS',
             ID_PREF_TABW         : 'TABWIDTH',
             ID_VIEW_TOOL         : 'TOOLBAR',
             ID_PREF_TABS         : 'USETABS',
             ID_PREF_VIRT_SPACE   : 'VIEWVERTSPACE',
             ID_VI_MODE           : 'VI_EMU',
             ID_VI_NORMAL_DEFAULT : 'VI_NORMAL_DEFAULT',
             ID_PREF_WARN_EOL     : 'WARN_EOL',
             ID_WORD_WRAP         : 'WRAP',
}

EOL_MODE_CR   = 0
EOL_MODE_LF   = 1
EOL_MODE_CRLF = 2
def EOLModeMap():
    """Get the eol mode map"""
    # Maintenance Note: ints must be kept in sync with EDSTC_EOL_* in edstc
    return { EOL_MODE_CR : _("Old Machintosh (\\r)"),
             EOL_MODE_LF : _("Unix (\\n)"),
             EOL_MODE_CRLF : _("Windows (\\r\\n)")}

# Default Plugins
DEFAULT_PLUGINS = ("generator.Html", "generator.LaTeX", "generator.Rtf",
                   "iface.Shelf", "ed_theme.TangoTheme", "ed_log.EdLogViewer",
                   "ed_search.EdFindResults", "ed_bookmark.EdBookmarks")
