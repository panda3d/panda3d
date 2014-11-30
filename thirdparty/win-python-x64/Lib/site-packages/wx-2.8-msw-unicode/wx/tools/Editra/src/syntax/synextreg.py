###############################################################################
# Name: synextreg.py                                                          #
# Purpose: IDs and descriptions for supported file types, and also the        #
#          ExtensionRegister.  These items are here in this module inorder to #
#          be usable external to Editra and where wx may not be available.    #
#                                                                             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: synextreg.py
LANGUAGE: Python
@summary: This module defines all supported language/filetype identifiers and
          an extension register for mapping file extensions to filetypes.
@see: synglob.py for more details on how this data is used

@note: Don't use this module directly for internal use only

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: synextreg.py 67571 2011-04-22 01:10:57Z CJP $"
__revision__ = "$Revision: 67571 $"

#-----------------------------------------------------------------------------#
import os

#-----------------------------------------------------------------------------#

def _NewId():
    global _idCounter
    _idCounter += 1
    return _idCounter

_idCounter = 32100

#-----------------------------------------------------------------------------#

#---- Language Identifiers Keys ----#
# Used for specifying what dialect/keyword set to load for a specific lexer

#---- Use LEX_NULL ----#
ID_LANG_TXT  = _NewId()
LANG_TXT = u'Plain Text'

#---- Use LEX_ADA ----#
ID_LANG_ADA = _NewId()
LANG_ADA = u'Ada'

#---- Use LEX_ASM ----#
ID_LANG_ASM  = _NewId()
LANG_ASM = u'GNU Assembly'
ID_LANG_DSP56K = _NewId()
LANG_DSP56K = u'DSP56K Assembly'
ID_LANG_68K  = _NewId()
LANG_68K = u'68k Assembly'
ID_LANG_MASM = _NewId()
LANG_MASM = u'MASM'
ID_LANG_NASM = _NewId()
LANG_NASM = u'Netwide Assembler'

# Use LEX_BASH
ID_LANG_BOURNE = _NewId()
LANG_BOURNE = u'Bourne Shell Script'
ID_LANG_BASH   = _NewId()
LANG_BASH = u'Bash Shell Script'
ID_LANG_CSH    = _NewId()
LANG_CSH = u'C-Shell Script'
ID_LANG_KSH    = _NewId()
LANG_KSH = u'Korn Shell Script'

# Use LEX_CAML
ID_LANG_CAML = _NewId()
LANG_CAML = u'Caml'

# Use LEX_CONF
ID_LANG_APACHE = _NewId()
LANG_APACHE = u'Apache Conf'

# Use LEX_CPP
ID_LANG_AS = _NewId()
LANG_AS = u'ActionScript'
ID_LANG_C = _NewId()
LANG_C = u'C'
ID_LANG_CILK = _NewId()
LANG_CILK = u'Cilk'
ID_LANG_CPP = _NewId()
LANG_CPP = u'CPP'
ID_LANG_CSHARP = _NewId()
LANG_CSHARP = u'C#'
ID_LANG_D = _NewId()
LANG_D = u'D'
ID_LANG_DOT = _NewId()
LANG_DOT = u'DOT'
ID_LANG_EDJE = _NewId()
LANG_EDJE = u'Edje'
ID_LANG_FERITE = _NewId()
LANG_FERITE = u'Ferite'
ID_LANG_HAXE = _NewId()
LANG_HAXE = u'HaXe'
ID_LANG_JAVA = _NewId()
LANG_JAVA = u'Java'
ID_LANG_OBJC = _NewId()
LANG_OBJC = u'Objective C'
ID_LANG_OOC = _NewId()
LANG_OOC = u'OOC'
ID_LANG_PIKE = _NewId()
LANG_PIKE = u'Pike'
ID_LANG_SQUIRREL = _NewId()
LANG_SQUIRREL = u'Squirrel'
ID_LANG_STATA = _NewId()
LANG_STATA = u'Stata'
ID_LANG_VALA = _NewId()
LANG_VALA = u'Vala'

# Use LEX_CSS
ID_LANG_CSS = _NewId()
LANG_CSS = u'Cascading Style Sheet'
ID_LANG_ESS = _NewId()
LANG_ESS = u'Editra Style Sheet'

# Use LEX_EIFFEL
ID_LANG_EIFFEL = _NewId()
LANG_EIFFEL = u'Eiffel'

# Use LEX_ERLANG
ID_LANG_ERLANG = _NewId()
LANG_ERLANG = u'Erlang'

# Use LEX_FLAGSHIP
ID_LANG_FLAGSHIP = _NewId()
LANG_FLAGSHIP = u'FlagShip'

# Use LEX_F77
ID_LANG_F77 = _NewId()
LANG_F77 = u'Fortran 77'

# Use LEX_FORTH
ID_LANG_FORTH = _NewId()
LANG_FORTH = u"Forth"

# Use LEX_FORTRAN
ID_LANG_F95 = _NewId()
LANG_F95 = u'Fortran 95'

# Use LEX_GUI4CLI
ID_LANG_GUI4CLI = _NewId()
LANG_GUI4CLI = u'Gui4Cli'

# Use LEX_HASKELL
ID_LANG_HASKELL = _NewId()
LANG_HASKELL = u'Haskell'

# Use LEX_HTML
ID_LANG_COLDFUSION = _NewId()
LANG_COLDFUSION = u'ColdFusion'
ID_LANG_HTML = _NewId()
LANG_HTML = u'HTML'
ID_LANG_JS   = _NewId()
LANG_JS = u'JavaScript'
ID_LANG_PHP  = _NewId()
LANG_PHP = u'PHP'
ID_LANG_XML  = _NewId()
LANG_XML = u'XML'
ID_LANG_SGML = _NewId()

# Use LEX_INNOSETUP
ID_LANG_INNO = _NewId()
LANG_INNO = u'Inno Setup Script'

# Use LEX_KIX
ID_LANG_KIX = _NewId()
LANG_KIX = u'Kix'

# Use LEX_LISP
ID_LANG_LISP = _NewId()
LANG_LISP = u'Lisp'
ID_LANG_SCHEME = _NewId()
LANG_SCHEME = u'Scheme'
ID_LANG_NEWLISP = _NewId()
LANG_NEWLISP = u'newLISP'

# Use LEX_LOUT
ID_LANG_LOUT = _NewId()
LANG_LOUT = u'Lout'

# Use LEX_LUA
ID_LANG_LUA = _NewId()
LANG_LUA = u'Lua'

# Use LEX_MSSQL (Microsoft SQL)
ID_LANG_MSSQL = _NewId()
LANG_MSSQL = u'Microsoft SQL'

# Use LEX_NONMEM
ID_LANG_NONMEM = _NewId()
LANG_NONMEM = u'NONMEM Control Stream'

# Use LEX_NSIS
ID_LANG_NSIS = _NewId()
LANG_NSIS = u'Nullsoft Installer Script'

# Use LEX_PASCAL
ID_LANG_PASCAL = _NewId()
LANG_PASCAL = u'Pascal'

# Use LEX_PERL
ID_LANG_PERL = _NewId()
LANG_PERL = u'Perl'

# Use LEX_PS
ID_LANG_PS = _NewId()
LANG_PS = u'Postscript'

# Use LEX_PYTHON 
ID_LANG_BOO = _NewId()
LANG_BOO = u'Boo'
ID_LANG_PYTHON = _NewId()
LANG_PYTHON = u'Python'
ID_LANG_COBRA = _NewId()
LANG_COBRA = u'Cobra'

# Use LEX_MATLAB
ID_LANG_MATLAB = _NewId()
LANG_MATLAB = u'Matlab'

# Use LEX_RUBY
ID_LANG_RUBY = _NewId()
LANG_RUBY = u'Ruby'

# Use LEX_SMALLTALK
ID_LANG_ST = _NewId()
LANG_ST = u'Smalltalk'

# Use LEX_SQL (PL/SQL, SQL*Plus)
ID_LANG_SQL = _NewId()
LANG_SQL = u'SQL'
ID_LANG_PLSQL = _NewId()
LANG_PLSQL = u'PL/SQL'
ID_LANG_4GL = _NewId()
LANG_4GL = u"Progress 4GL"

# Use LEX_TCL
ID_LANG_TCL  = _NewId()
LANG_TCL = u'Tcl/Tk'

# Use LEX_TEX
ID_LANG_TEX = _NewId()
LANG_TEX = u'Tex'
ID_LANG_LATEX = _NewId()
LANG_LATEX = u'LaTeX'

# Use LEX_VB
ID_LANG_VB = _NewId()
LANG_VB = u'Visual Basic'

# Use LEX_VBSCRIPT
ID_LANG_VBSCRIPT = _NewId()
LANG_VBSCRIPT = u'VBScript'

# Use LEX_VERILOG
ID_LANG_VERILOG = _NewId()
LANG_VERILOG = u'Verilog'
ID_LANG_SYSVERILOG = _NewId()
LANG_SYSVERILOG = u'System Verilog'

# Use LEX_VHDL
ID_LANG_VHDL = _NewId()
LANG_VHDL = u'VHDL'

# Use LEX_OCTAVE
ID_LANG_OCTAVE = _NewId()
LANG_OCTAVE = u'Octave'

# Use LEX_OTHER (Batch, Diff, Makefile)
ID_LANG_BATCH = _NewId()
LANG_BATCH = u'DOS Batch Script'
ID_LANG_DIFF = _NewId()
LANG_DIFF = u'Diff File'
ID_LANG_MAKE  = _NewId()
LANG_MAKE = u'Makefile'
ID_LANG_PROPS = _NewId()
LANG_PROPS = u'Properties'

# Use LEX_YAML
ID_LANG_YAML = _NewId()
LANG_YAML = u'YAML'

# Use LEX_CONTAINER
ID_LANG_DJANGO = _NewId()
LANG_DJANGO = u'Django'
ID_LANG_ISSL = _NewId()
LANG_ISSL = u'IssueList'
ID_LANG_MAKO = _NewId()
LANG_MAKO = u'Mako'
ID_LANG_R = _NewId()
LANG_R = u'R'
ID_LANG_S = _NewId()
LANG_S = u'S'
ID_LANG_GROOVY = _NewId()
LANG_GROOVY = u'Groovy'
ID_LANG_XTEXT = _NewId()
LANG_XTEXT = u'Xtext'

#---- End Language Identifier Keys ----#

# Default extensions to file type mapping
EXT_MAP = {
           '4gl'                : LANG_4GL,
           '56k'                : LANG_DSP56K,
           '68k'                : LANG_68K,
           'ada adb ads a'      : LANG_ADA,
           'conf htaccess'      : LANG_APACHE,
           'as asc mx'          : LANG_AS,
           'gasm'               : LANG_ASM,
           'bsh sh configure'   : LANG_BASH,
           'bat cmd'            : LANG_BATCH,
           'boo'                : LANG_BOO,
           'c h'                : LANG_C,
           'ml mli'             : LANG_CAML,
           'cilk cilkh'         : LANG_CILK,
           'cobra'              : LANG_COBRA,
           'cfm cfc cfml dbm'   : LANG_COLDFUSION,
           'cc c++ cpp cxx hh h++ hpp hxx' : LANG_CPP,
           'csh'                : LANG_CSH,
           'cs'                 : LANG_CSHARP,
           'css'                : LANG_CSS,
           'd'                  : LANG_D,
           'patch diff'         : LANG_DIFF,
           'django'             : LANG_DJANGO,
           'dot'                : LANG_DOT,
           'edc'                : LANG_EDJE,
           'e'                  : LANG_EIFFEL,
           'erl'                : LANG_ERLANG,
           'ess'                : LANG_ESS,
           'f for'              : LANG_F77,
           'f90 f95 f2k fpp'    : LANG_F95,
           'fe'                 : LANG_FERITE,
           'fth 4th fs seq'     : LANG_FORTH,
           'prg'                : LANG_FLAGSHIP,
           'gc gui'             : LANG_GUI4CLI,
           'hs'                 : LANG_HASKELL,
           'hx hxml'            : LANG_HAXE,
           'htm html shtm shtml xhtml' : LANG_HTML,
           'isl'                : LANG_ISSL,
           'iss'                : LANG_INNO,
           'java'               : LANG_JAVA,
           'js'                 : LANG_JS,
           'kix'                : LANG_KIX,
           'ksh'                : LANG_KSH,
           'aux tex sty'        : LANG_LATEX,
           'cl lisp'            : LANG_LISP,
           'lsp'                : LANG_NEWLISP,
           'lt'                 : LANG_LOUT,
           'lua'                : LANG_LUA,
           'mak makefile mk'    : LANG_MAKE,
           'mao mako'           : LANG_MAKO,
           'asm masm'           : LANG_MASM,
           'matlab'             : LANG_MATLAB,
           'mssql'              : LANG_MSSQL,
           'nasm'               : LANG_NASM,
           'ctl nonmem'         : LANG_NONMEM,
           'nsi nsh'            : LANG_NSIS,
           'mm m'               : LANG_OBJC,
           'oct octave'         : LANG_OCTAVE,
           'ooc'                : LANG_OOC,
           'dfm dpk dpr inc p pas pp' : LANG_PASCAL,
           'cgi pl pm pod'      : LANG_PERL,
           'php php3 phtml phtm' : LANG_PHP,
           'pike'                : LANG_PIKE,
           'plsql'              : LANG_PLSQL,
           'ini inf reg url cfg cnf' : LANG_PROPS,
           'ai ps'              : LANG_PS,
           'py pyw python'      : LANG_PYTHON,
           'r'                  : LANG_R,
           'do ado'             : LANG_STATA,
           'rake rb rbw rbx gemspec' : LANG_RUBY,
           's'                  : LANG_S,
           'scm smd ss'         : LANG_SCHEME,
           'sql'                : LANG_SQL,
           'nut'                : LANG_SQUIRREL,
           'st'                 : LANG_ST,
           'sv svh'             : LANG_SYSVERILOG,
           'itcl tcl tk'        : LANG_TCL,
           'txt'                : LANG_TXT,
           'vala'               : LANG_VALA,
           'bas cls frm vb'     : LANG_VB,
           'vbs dsm'            : LANG_VBSCRIPT,
           'v'                  : LANG_VERILOG,
           'vh vhdl vhd'        : LANG_VHDL,
           'axl dtd plist rdf svg xml xrc xsd xsl xslt xul' : LANG_XML,
           'yaml yml'           : LANG_YAML,
           'groovy'             : LANG_GROOVY,
           'xtext'               : LANG_XTEXT,
          }


#-----------------------------------------------------------------------------#

class ExtensionRegister(dict):
    """A data storage class for managing mappings of
    file types to file extensions. The register is created
    as a singleton.
    @status: initial implementation

    """
    instance = None
    config = u'synmap'
    def __init__(self):
        """Initializes the register"""
        if not ExtensionRegister.instance:
            self.LoadDefault()

    def __new__(cls, *args, **kargs):
        """Maintain only a single instance of this object
        @return: instance of this class

        """
        if not cls.instance:
            cls.instance = dict.__new__(cls, *args, **kargs)
        return cls.instance

    def __missing__(self, key):
        """Return the default value if an item is not found
        @return: txt extension for plain text

        """
        return u'txt'

    def __setitem__(self, i, y):
        """Ensures that only one filetype is associated with an extension
        at one time. The behavior is that more recent settings override
        and remove associations from older settings.
        @param i: key to set
        @param y: value to set
        @throws: TypeError Only accepts list() objects

        """
        if not isinstance(y, list):
            raise TypeError, "Extension Register Expects a List"
        for key, val in self.iteritems():
            for item in y:
                if item in val:
                    val.pop(val.index(item))
        y.sort()
        dict.__setitem__(self, i, [x.strip() for x in y])

    def __str__(self):
        """Converts the Register to a string that is formatted
        for output to a config file.
        @return: the register as a string

        """
        keys = self.keys()
        keys.sort()
        tmp = list()
        for key in keys:
            tmp.append("%s=%s" % (key, u':'.join(self.__getitem__(key))))
        return os.linesep.join(tmp)

    def Associate(self, ftype, ext):
        """Associate a given file type with the given file extension(s).
        The ext parameter can be a string of space separated extensions
        to allow for multiple associations at once.
        @param ftype: file type description string
        @param ext: file extension to associate
        
        """
        assoc = self.get(ftype, None)
        exts = ext.strip().split()
        if assoc:
            for x in exts:
                if x not in assoc:
                    assoc.append(x)
        else:
            assoc = list(set(exts))
        assoc.sort()
        super(ExtensionRegister, self).__setitem__(ftype, assoc)

    def Disassociate(self, ftype, ext):
        """Disassociate a file type with a given extension or space
        separated list of extensions.
        @param ftype: filetype description string
        @param ext: extension to disassociate

        """
        to_drop = ext.strip().split()
        assoc = self.get(ftype, None)
        if assoc:
            for item in to_drop:
                if item in assoc:
                    assoc.remove(item)
            super(ExtensionRegister, self).__setitem__(ftype, assoc)
        else:
            pass

    def FileTypeFromExt(self, ext):
        """Returns the file type that is associated with
        the extension. If no association is found Plain Text
        will be returned by default.
        @param ext: extension to lookup

        """
        for key, val in self.iteritems():
            if ext in val:
                return key
        return LANG_TXT

    def GetAllExtensions(self):
        """Returns a sorted list of all extensions registered
        @return: list of all registered extensions

        """
        ext = list()
        for extension in self.values():
            ext.extend(extension) 
        ext.sort()
        return ext

    def LoadDefault(self):
        """Loads the default settings
        @postcondition: sets dictionary back to default installation state

        """
        self.clear()
        for key in EXT_MAP:
            self.__setitem__(EXT_MAP[key], key.split())

    def LoadFromConfig(self, config):
        """Load the extension register with values from a config file
        @param config: path to config file to load settings from

        """
        path = os.path.join(config, self.config)
        if not os.path.exists(path):
            self.LoadDefault()
        else:
            file_h = file(path, "rb")
            lines = file_h.readlines()
            file_h.close()
            for line in lines:
                tmp = line.split(u'=')
                if len(tmp) != 2:
                    continue
                ftype = tmp[0].strip()
                exts = tmp[1].split(u':')
                self.__setitem__(ftype, exts)

    def Remove(self, ftype):
        """Remove a filetype from the register
        @param ftype: File type description string
        @return: bool removed or not

        """
        if ftype in self:
            del self[ftype]
            return True
        return False

    def SetAssociation(self, ftype, ext):
        """Like Associate but overrides any current settings instead of
        just adding to them.
        @param ftype: File type description string
        @param ext: space separated list of file extensions to set

        """
        self.__setitem__(ftype, list(set(ext.split())))

#-----------------------------------------------------------------------------#

def GetFileExtensions():
    """Gets a sorted list of all file extensions the editor is configured
    to handle.
    @return: all registered file extensions

    """
    extreg = ExtensionRegister()
    return extreg.GetAllExtensions()

def RegisterNewLangId(langId, langName):
    """Register a new language identifier
    @param langId: "ID_LANG_FOO"
    @param langName: "Foo"
    @return: int

    """
    gdict = globals()
    if langId not in gdict:
        gdict[langId] = _NewId()
        gdict[langId[3:]] = langName
    return gdict[langId]

#-----------------------------------------------------------------------------#
