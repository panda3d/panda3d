###############################################################################
# Name: pike.py                                                               #
# Purpose: Define highlighting/syntax for Pike programming language           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: pike.py
@summary: Defines syntax and highlighting settings for the Pike programming
          language. Pike is very similar in form to C/CPP so the Cpp lexer is
          used to provide the highlighting settings.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: pike.py 55174 2008-08-22 15:12:27Z CJP $"
__revision__ = "$Revision: 55174 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
import cpp

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
PIKE_KW = (0, "goto break return continue case default if else switch while "
              "foreach do gauge destruct lambda inherit import typeof catch "
              "for inline nomask")

PIKE_TYPE = (1, "private protected public static "
                "int string void float mapping array multiset mixed program "
                "object function")

PIKE_DOC = tuple(cpp.DOC_KEYWORDS)

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = list(cpp.SYNTAX_ITEMS)

#---- Extra Properties ----#
# Fetched from cpp module on request

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_PIKE:
        return [PIKE_KW, PIKE_TYPE, PIKE_DOC]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_PIKE:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_PIKE:
        return cpp.Properties(synglob.ID_LANG_CPP)
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_PIKE:
        return cpp.CommentPattern(synglob.ID_LANG_CPP)
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
