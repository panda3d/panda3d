###############################################################################
# Name: syntax.py                                                             #
# Purpose: Manage and dynamically load/provide syntax on request from editor. #
#          Also manages the mappings of file extensions to lexers.            #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# Licence: wxWindows Licence                                                  #
###############################################################################

"""
FILE: syntax.py
AUTHOR: Cody Precord

SUMMARY:

Toolkit for managing the importing of syntax modules and providing the data
to the requesting text control. It is meant to be the main access point to the
resources and api provided by this package.

DETAIL:

The main item of this module is the L{SyntaxMgr} it is a singleton object that
manages the dynamic importing of syntax data and configurations for all of the
editors supported languages. It allows only the needed data to be loaded into
memory when requested. The loading is only done once per session and all
subsequent requests share the same object.

In addition to the L{SyntaxMgr} there are also a number of other utility and
convienience functions in this module for accessing data from other related
objects such as the Extension Register.

@summary: Main api access point for the syntax package.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: syntax.py 63846 2010-04-03 22:57:21Z CJP $"
__revision__ = "$Revision: 63846 $"

#-----------------------------------------------------------------------------#
# Dependencies
import wx
import os

#-----------------------------------------------------------------------------#
# Data Objects / Constants

# Used to index the tuple returned by getting data from EXT_REG
LANG_ID    = 0
MODULE     = 1

_ = wx.GetTranslation
#-----------------------------------------------------------------------------#
# Imports
import synglob
import syndata
import synxml

# Needed by other modules that use this api
from synextreg import ExtensionRegister, GetFileExtensions, RegisterNewLangId

#-----------------------------------------------------------------------------#

class SyntaxMgr(object):
    """Class Object for managing loaded syntax data. The manager
    is only created once as a singleton and shared amongst all
    editor windows

    """
    instance = None
    first = True
    def __init__(self, config=None):
        """Initialize a syntax manager. If the optional
        value config is set the mapping of extensions to
        lexers will be loaded from a config file.
        @keyword config: path of config file to load file extension config from

        """
        if SyntaxMgr.first:
            object.__init__(self)
            SyntaxMgr.first = False
            self._extreg = ExtensionRegister()
            self._config = config
            self._loaded = dict()

            # Syntax mode extensions
            self._extensions = dict()   # loaded extensions "py" : PythonMode()

            self.InitConfig()

    def __new__(cls, config=None):
        """Ensure only a single instance is shared amongst
        all objects.
        @return: class instance

        """
        if cls.instance is None:
            cls.instance = object.__new__(cls)
        return cls.instance

    def _ExtToMod(self, ext):
        """Gets the name of the module that is is associated
        with the given extension or None in the event that there
        is no association or that the association is plain text.
        @param ext: extension string to lookup module for

        """
        ftype = self._extreg.FileTypeFromExt(ext)
        lexdat = synglob.LANG_MAP.get(ftype)
        mod = None
        if lexdat:
            mod = lexdat[MODULE]
        return mod

    def GetLangId(self, ext):
        """Gets the language Id that is associated with the file
        extension.
        @param ext: extension to get lang id for

        """
        ftype = self._extreg.FileTypeFromExt(ext)
        return synglob.LANG_MAP[ftype][LANG_ID]

    def InitConfig(self):
        """Initialize the SyntaxMgr's configuration state"""
        if self._config:
            self._extreg.LoadFromConfig(self._config)
        else:
            self._extreg.LoadDefault()

        if self._config:
            self.LoadExtensions(self._config)

    def IsModLoaded(self, modname):
        """Checks if a module has already been loaded
        @param modname: name of module to lookup

        """
        if modname in self._loaded:
            return True
        else:
            return False

    def LoadModule(self, modname):
        """Dynamically loads a module by name. The loading is only
        done if the modules data set is not already being managed
        @param modname: name of syntax module to load

        """
        if modname == None:
            return False
        if not self.IsModLoaded(modname):
            try:
                self._loaded[modname] = __import__(modname, globals(), 
                                                   locals(), [''])
            except ImportError, msg:
                return False
        return True

    def SaveState(self):
        """Saves the current configuration state of the manager to
        disk for use in other sessions.
        @return: whether save was successful or not

        """
        if not self._config or not os.path.exists(self._config):
            return False
        path = os.path.join(self._config, self._extreg.config)
        try:
            file_h = open(path, "wb")
            file_h.write(str(self._extreg))
            file_h.close()
        except IOError:
            return False
        return True

    def GetSyntaxData(self, ext):
        """Fetches the language data based on a file extension string. The file
        extension is used to look up the default lexer actions from the EXT_REG
        dictionary.
        @see: L{synglob}
        @param ext: a string representing the file extension
        @return: SyntaxData object

        """
        # The Return Value
        lang = self._extreg.FileTypeFromExt(ext)
        if lang in self._extensions:
            syn_data = self._extensions[lang]
            return syn_data

        # Check for extensions that may have been removed
        if lang not in synglob.LANG_MAP:
            self._extreg.Remove(lang)

        lex_cfg = synglob.LANG_MAP.get(lang, synglob.LANG_MAP[synglob.LANG_TXT])

        # Check if module is loaded and load if necessary
        if not self.LoadModule(lex_cfg[MODULE]):
            # Bail out and return a default plaintext config as
            # nothing else can be done at this point
            return syndata.SyntaxDataBase()

        # This little bit of code fetches the keyword/syntax 
        # spec set(s) from the specified module
        mod = self._loaded[lex_cfg[MODULE]]
        syn_data = mod.SyntaxData(lex_cfg[LANG_ID])
        return syn_data

    def LoadExtensions(self, path):
        """Load all extensions found at the extension path
        @param path: path to look for extension on

        """
        for fname in os.listdir(path):
            if fname.endswith(u".edxml"):
                fpath = os.path.join(path, fname)
                modeh = synxml.LoadHandler(fpath)

                if modeh.IsOk():
                    sdata = SynExtensionDelegate(modeh)
                    self._extensions[sdata.GetXmlObject().GetLanguage()] = sdata
                else:
                    pass
                    #TODO: report error

    def SetConfigDir(self, path):
        """Set the path to locate config information at. The SyntaxMgr will
        look for file type associations in a file called synmap and will load
        syntax extensions from .edxml files found at this path.
        @param path: string

        """
        self._config = path

#-----------------------------------------------------------------------------#

class SynExtensionDelegate(syndata.SyntaxDataBase):
    """Delegate SyntaxData class for SynXml Extension class instances"""
    def __init__(self, xml_obj):
        """Initialize the data class
        @param xml_obj: SynXml

        """
        langId = _RegisterExtensionHandler(xml_obj)
        syndata.SyntaxDataBase.__init__(self, langId)

        # Attributes
        self._xml = xml_obj

        # Setup
        self.SetLexer(self._xml.GetLexer())
        #TODO: Load and register features specified by the xml object

    #---- Syntax Data Implementation ----#

    def GetCommentPattern(self):
        """Get the comment pattern
        @return: list of strings ['/*', '*/']

        """
        return self._xml.GetCommentPattern()

    def GetKeywords(self):
        """Get the Keyword List(s)
        @return: list of tuples [(1, ['kw1', kw2']),]

        """
        keywords = self._xml.GetKeywords()
        rwords = list()
        for sid, words in keywords:
            rwords.append((sid, u" ".join(words)))
        return rwords

    def GetProperties(self):
        """Get the Properties List
        @return: list of tuples [('fold', '1'),]

        """
        return self._xml.GetProperties()

    def GetSyntaxSpec(self):
        """Get the the syntax specification list
        @return: list of tuples [(int, 'style_tag'),]

        """
        return self._xml.GetSyntaxSpec()

    #---- End Syntax Data Implementation ----#

    def GetXmlObject(self):
        """Get the xml object
        @return: EditraXml instance

        """
        return self._xml
    
#-----------------------------------------------------------------------------#

def GenLexerMenu():
    """Generates a menu of available syntax configurations
    @return: alphabetically ordered menu of of all lexer settings
    @rtype: wx.Menu

    """
    lex_menu = wx.Menu()
    f_types = dict()
    for key in synglob.LANG_MAP:
        f_types[key] = synglob.LANG_MAP[key][LANG_ID]
    f_order = list(f_types)
    f_order.sort(key=unicode.lower)

    for lang in f_order:
        lex_menu.Append(f_types[lang], lang, 
                         _("Switch Lexer to %s") % lang, wx.ITEM_CHECK)
    return lex_menu

def GenFileFilters():
    """Generates a list of file filters
    @return: list of all file filters based on extension associations

    """
    extreg = ExtensionRegister()
    # Convert extension list into a formatted string
    f_dict = dict()
    for key, val in extreg.iteritems():
        val.sort()
        if key.lower() == 'makefile':
            continue

        f_dict[key] = u";*." + u";*.".join(val)

    # Build the final list of properly formatted strings
    filters = list()
    for key in f_dict:
        tmp = u" (%s)|%s|" % (f_dict[key][1:], f_dict[key][1:])
        filters.append(key + tmp)
    filters.sort(key=unicode.lower)
    filters.insert(0, u"All Files (*)|*|")
    filters[-1] = filters[-1][:-1] # IMPORTANT trim last '|' from item in list
    return filters

def GetLexerList():
    """Gets the list of all supported file types
    @return: list of strings

    """ 
    f_types = synglob.LANG_MAP.keys()
    f_types.sort(key=unicode.lower)
    return f_types

#---- Syntax id set ----#
SYNTAX_IDS = None

def SyntaxIds():
    """Gets a list of all Syntax Ids and returns it
    @return: list of all syntax language ids

    """
    # Use the cached list if its available
    if SYNTAX_IDS is not None:
        return SYNTAX_IDS

    syn_ids = list()
    for item in dir(synglob):
        if item.startswith("ID_LANG"):
            syn_ids.append(getattr(synglob, item))

    return syn_ids

SYNTAX_IDS = SyntaxIds()

def SyntaxNames():
    """Gets a list of all Syntax Labels
    @return: list of strings

    """
    syn_list = list()
    for item in dir(synglob):
        if item.startswith("LANG_"):
            val = getattr(synglob, item)
            if isinstance(val, basestring):
                syn_list.append(val)
    return syn_list

#---- End Syntax ids ----#

def GetExtFromId(ext_id):
    """Takes a language ID and fetches an appropriate file extension string
    @param ext_id: language id to get extension for
    @return: file extension
    @rtype: string

    """
    extreg = ExtensionRegister()
    ftype = synglob.GetDescriptionFromId(ext_id)
    rval = u''
    if len(extreg[ftype]):
        rval = extreg[ftype][0]
    return rval

def GetIdFromExt(ext):
    """Get the language id from the given file extension
    @param ext: file extension (no dot)
    @return: language identifier id from extension register

    """
    ftype = ExtensionRegister().FileTypeFromExt(ext)
    for val in dir(synglob):
        if val.startswith('LANG_') and getattr(synglob, val) == ftype:
            return getattr(synglob, 'ID_' + val, synglob.ID_LANG_TXT)

    return synglob.ID_LANG_TXT

def GetTypeFromExt(ext):
    """Get the filetype description string from the given extension.
    The return value defaults to synglob.LANG_TXT if nothing is found.
    @param ext: file extension string (no dot)
    @return: String

    """
    return ExtensionRegister().FileTypeFromExt(ext)

def _RegisterExtensionHandler(xml_obj):
    """Register an ExtensionHandler with this module.
    @todo: this is a temporary hack till what to do with the language id's
           is decided.

    """
    # Create an ID value for the lang id string
    langId = xml_obj.GetLangId() 
    rid = RegisterNewLangId(langId, xml_obj.GetLanguage())
    setattr(synglob, langId, rid)
    setattr(synglob, langId[3:], xml_obj.GetLanguage())

    # Register file extensions with extension register
    ExtensionRegister().Associate(xml_obj.GetLanguage(),
                                  u" ".join(xml_obj.FileExtensions))

    # Update static syntax id list
    if rid not in SYNTAX_IDS:
        SYNTAX_IDS.append(rid)

    return rid
