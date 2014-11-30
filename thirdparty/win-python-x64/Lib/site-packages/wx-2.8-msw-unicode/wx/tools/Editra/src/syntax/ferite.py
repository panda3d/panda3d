###############################################################################
# Name: ferite.py                                                             #
# Purpose: Syntax Definitions for the Ferite Scripting Language               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
@summary: Lexer configuration module for Ferite Scripting Language

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ferite.py 55174 2008-08-22 15:12:27Z CJP $"
__revision__ = "$Revision: 55174 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
import cpp

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
FERITE_KW = (0, "false null self super true abstract alias and arguments "
                "attribute_missing break case class closure conformsToProtocol "
                "constructor continue default deliver destructor diliver "
                "directive do else extends eval final fix for function global "
                "handle if iferr implements include instanceof isa "
                "method_missing modifies monitor namespace new or private "
                "protected protocol public raise recipient rename return "
                "static switch uses using while")

FERITE_TYPES = (1, "boolean string number array object void XML Unix Sys "
                   "String Stream Serialize RMI Posix Number Network Math "
                   "FileSystem Console Array Regexp XSLT")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
# Same as CPP
#SYNTAX_ITEMS = 

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_FERITE:
        return [FERITE_KW, FERITE_TYPES, cpp.DOC_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_FERITE:
        return cpp.SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_FERITE:
        return [cpp.FOLD,]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_FERITE:
        return ['//',]
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
