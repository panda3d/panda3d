###############################################################################
# Name: synglob.py                                                            #
# Purpose: Acts as a registration point for all supported languages.          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: synglob.py
AUTHOR: Cody Precord
@summary: Provides configuration and basic API functionality to all the syntax
          modules. It also acts  as a configuration file for the syntax
          management code. When support for a new languages is added it must
          have a registration entry in the below L{LANG_MAP} dictionary in
          order to be loadable by the syntax module.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: synglob.py 66605 2011-01-06 00:55:52Z CJP $"
__revision__ = "$Revision: 66605 $"

#-----------------------------------------------------------------------------#
# Dependencies
import wx.stc as stc

# The language identifiers and the EXT_MAP have been moved out of this
# module in order to be independent of Editra and wx, but they are
# still needed here...
from synextreg import *

#-----------------------------------------------------------------------------#
# Feature Identifiers
FEATURE_AUTOINDENT = u"AutoIndenter"
FEATURE_STYLETEXT = u"StyleText"

#-----------------------------------------------------------------------------#

# Maps file types to syntax definitions
LANG_MAP = {LANG_4GL    : (ID_LANG_4GL,       '_progress'),
            LANG_DSP56K : (ID_LANG_DSP56K,    '_asm68k'),
            LANG_68K    : (ID_LANG_68K,       '_asm68k'),
            LANG_ADA    : (ID_LANG_ADA,       '_ada'),
            LANG_APACHE : (ID_LANG_APACHE,    '_apache'),
            LANG_AS     : (ID_LANG_AS,        '_actionscript'),
            LANG_ASM    : (ID_LANG_ASM,       '_asm'),
            LANG_BASH   : (ID_LANG_BASH,      '_sh'),
            LANG_BATCH  : (ID_LANG_BATCH,     '_batch'),
            LANG_BOO    : (ID_LANG_BOO,       '_boo'),
            LANG_C      : (ID_LANG_C,         '_cpp'),
            LANG_CAML   : (ID_LANG_CAML,      '_caml'),
            LANG_CILK   : (ID_LANG_CILK,      '_cpp'),
            LANG_COBRA  : (ID_LANG_COBRA,     '_cobra'),
            LANG_COLDFUSION : (ID_LANG_COLDFUSION, '_html'),
            LANG_CPP    : (ID_LANG_CPP,    '_cpp'),
            LANG_CSH    : (ID_LANG_CSH,    '_sh'),
            LANG_CSHARP : (ID_LANG_CSHARP, '_cpp'),
            LANG_CSS    : (ID_LANG_CSS,    '_css'),
            LANG_D      : (ID_LANG_D,      '_d'),
            LANG_DIFF   : (ID_LANG_DIFF,   '_diff'),
            LANG_DJANGO : (ID_LANG_DJANGO, '_django'),
            LANG_DOT    : (ID_LANG_DOT,    '_dot'),
            LANG_EDJE   : (ID_LANG_EDJE,   '_edje'),
            LANG_EIFFEL : (ID_LANG_EIFFEL, '_eiffel'),
            LANG_ERLANG : (ID_LANG_ERLANG, '_erlang'),
            LANG_ESS    : (ID_LANG_ESS,    '_editra_ss'),
            LANG_F77    : (ID_LANG_F77,    '_fortran'),
            LANG_F95    : (ID_LANG_F95,    '_fortran'),
            LANG_FERITE : (ID_LANG_FERITE, '_ferite'),
            LANG_FLAGSHIP: (ID_LANG_FLAGSHIP, '_flagship'),
            LANG_FORTH  : (ID_LANG_FORTH, '_forth'),
            LANG_GUI4CLI : (ID_LANG_GUI4CLI,  '_gui4cli'),
            LANG_HASKELL : (ID_LANG_HASKELL,  '_haskell'),
            LANG_HAXE   : (ID_LANG_HAXE,      '_haxe'),
            LANG_HTML   : (ID_LANG_HTML,   '_html'),
            LANG_INNO   : (ID_LANG_INNO,   '_inno'),
            LANG_ISSL   : (ID_LANG_ISSL,   '_issuelist'),
            LANG_JAVA   : (ID_LANG_JAVA,   '_java'),
            LANG_JS     : (ID_LANG_JS,     '_javascript'),
            LANG_KIX    : (ID_LANG_KIX,    '_kix'),
            LANG_KSH    : (ID_LANG_KSH,    '_sh'),
            LANG_LATEX  : (ID_LANG_LATEX,  '_latex'),
            LANG_LISP   : (ID_LANG_LISP,   '_lisp'),
            LANG_LOUT   : (ID_LANG_LOUT,   '_lout'),
            LANG_LUA    : (ID_LANG_LUA,    '_lua'),
            LANG_MAKE   : (ID_LANG_MAKE,   '_make'),
            LANG_MAKO   : (ID_LANG_MAKO,    '_mako'),
            LANG_MASM   : (ID_LANG_MASM,   '_masm'),
            LANG_MATLAB : (ID_LANG_MATLAB, '_matlab'),
            LANG_MSSQL  : (ID_LANG_MSSQL,  '_mssql'),
            LANG_NASM   : (ID_LANG_NASM,   '_nasm'),
            LANG_NEWLISP: (ID_LANG_NEWLISP,'_lisp'),
            LANG_NONMEM : (ID_LANG_NONMEM, '_nonmem'),
            LANG_NSIS   : (ID_LANG_NSIS,   '_nsis'),
            LANG_OBJC   : (ID_LANG_OBJC,   '_cpp'),
            LANG_OCTAVE : (ID_LANG_OCTAVE, '_matlab'),
            LANG_OOC    : (ID_LANG_OOC,    '_ooc'),
            LANG_PASCAL : (ID_LANG_PASCAL, '_pascal'),
            LANG_PERL   : (ID_LANG_PERL,   '_perl'),
            LANG_PHP    : (ID_LANG_PHP,    '_php'),
            LANG_PIKE   : (ID_LANG_PIKE,   '_pike'),
            LANG_PLSQL  : (ID_LANG_PLSQL,  '_sql'),
            LANG_PROPS  : (ID_LANG_PROPS,  '_props'),
            LANG_PS     : (ID_LANG_PS,     '_postscript'),
            LANG_PYTHON : (ID_LANG_PYTHON, '_python'),
            LANG_R      : (ID_LANG_R,      '_s'),
            LANG_RUBY   : (ID_LANG_RUBY,   '_ruby'),
            LANG_S      : (ID_LANG_S,      '_s'),
            LANG_SCHEME : (ID_LANG_SCHEME, '_lisp'),
            LANG_SQL    : (ID_LANG_SQL,    '_sql'),
            LANG_SQUIRREL : (ID_LANG_SQUIRREL, '_squirrel'),
            LANG_ST     : (ID_LANG_ST,         '_smalltalk'),
            LANG_STATA : (ID_LANG_STATA,       '_stata'),
            LANG_SYSVERILOG : (ID_LANG_SYSVERILOG,  '_verilog'),
            LANG_TCL    : (ID_LANG_TCL,        '_tcl'),
            LANG_TXT    : (ID_LANG_TXT,        None),
            LANG_VALA   : (ID_LANG_VALA,       '_cpp'),
            LANG_VB     : (ID_LANG_VB,         '_visualbasic'),
            LANG_VBSCRIPT : (ID_LANG_VBSCRIPT, '_vbscript'),
            LANG_VERILOG: (ID_LANG_VERILOG,    '_verilog'),
            LANG_VHDL   : (ID_LANG_VHDL,    '_vhdl'),
            LANG_XML    : (ID_LANG_XML,     '_xml'),
            LANG_YAML   : (ID_LANG_YAML,    '_yaml'),
            LANG_GROOVY : (ID_LANG_GROOVY,  '_groovy'),
            LANG_XTEXT  : (ID_LANG_XTEXT,   '_xtext')
            }


### TODO: Profiling on the following methods to see if caching is necessary ###

# Dynamically finds the language description string that matches the given
# language id.
# Used when manually setting lexer from a menu/dialog
def GetDescriptionFromId(lang_id):
    """Get the programming languages description string from the given
    language id. If no corresponding language is found the plain text
    description is returned.
    @param lang_id: Language Identifier ID
    @note: requires that all languages are defined in ID_LANG_NAME, LANG_NAME
           pairs to work properly.

    """
    rval = LANG_TXT
    # Guard against async code that may be modifying globals
    globs = dict(globals())
    for key, val in globs.iteritems():
        if val == lang_id and key.startswith('ID_LANG'):
            rval = globs.get(key[3:], LANG_TXT)
            break
    return rval

def GetIdFromDescription(desc):
    """Get the language identifier for the given file type string. The search
    is case insensitive.
    @param desc: string (i.e "Python")
    @note: if lookup fails ID_LANG_TXT is returned

    """
    rval = ID_LANG_TXT
    desc = desc.lower()
    # Guard against async code that may be modifying globals
    globs = dict(globals())
    for key, val in globs.iteritems():
        if isinstance(val, basestring) and \
           val.lower() == desc and key.startswith('LANG_'):
            rval = globs.get(u"ID_" + key, ID_LANG_TXT)
            break
    return rval
