###############################################################################
# Name: haxe.py                                                               #
# Purpose: Syntax Definitions for haXe web language                           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
@summary: Lexer configuration module for haXe web programming language

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: haxe.py 55174 2008-08-22 15:12:27Z CJP $"
__revision__ = "$Revision: 55174 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
import cpp

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
HAXE_KW = (0, "abstract break case catch class const continue trace do else "
              "enum extends finally for function goto if implements import in "
              "instanceof int interface new package private public return "
              "static super switch this throw throws transient try typeof var "
              "void volatile while with" )

HAXE_TYPES = (1, "Bool Enum false Float Int null String true Void ")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
# Same as CPP

#---- Extra Properties ----#
# Same as CPP

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_HAXE:
        return [HAXE_KW, HAXE_TYPES, cpp.DOC_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_HAXE:
        return cpp.SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_HAXE:
        return [cpp.FOLD,]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_HAXE:
        return ['//']
    else:
        return list()

#---- End Required Module Functions ----#

AutoIndenter = cpp.AutoIndenter

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
