###############################################################################
# Name: squirrel.py                                                           #
# Purpose: Syntax Definitions for Squirrel programming language               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
@summary: Lexer configuration module for Squirrel Programming Language

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: squirrel.py 55180 2008-08-22 17:35:29Z CJP $"
__revision__ = "$Revision: 55180 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
import cpp

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
SQUIRREL_KW = (0, "break case catch class clone continue const default "
                  "delegate delete do else enum extends for foreach function "
                  "if in local null resume return switch this throw try typeof "
                  "while parent yield constructor vargc vargv instanceof true "
                  "false static")

SQUIRREL_TYPES = (1, "")

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
    if lang_id == synglob.ID_LANG_SQUIRREL:
        return [SQUIRREL_KW, SQUIRREL_TYPES, cpp.DOC_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_SQUIRREL:
        return cpp.SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_SQUIRREL:
        return [cpp.FOLD,]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_SQUIRREL:
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
