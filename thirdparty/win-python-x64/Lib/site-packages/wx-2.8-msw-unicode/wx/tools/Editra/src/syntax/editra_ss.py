###############################################################################
# Name: editra_ss.py                                                          #
# Purpose: Define Editra Style Sheet syntax for highlighting and other        #
#          features.                                                          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: editra_ss.py                                                          
AUTHOR: Cody Precord                                                        
@summary: Lexer configuration file for Editra Syntax Highlighter Style Sheets.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: editra_ss.py 55178 2008-08-22 15:39:56Z CJP $"
__revision__ = "$Revision: 55178 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
from css import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Editra Style Sheet Keywords
ESS_KEYWORDS = (0, "fore back face size eol bold italic modifiers")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_CSS_DEFAULT', 'default_style'),
                 ('STC_CSS_CLASS', 'global_style'),
                 ('STC_CSS_COMMENT', 'comment_style'),
                 ('STC_CSS_DIRECTIVE', 'directive_style'),
                 ('STC_CSS_DOUBLESTRING', 'string_style'),
                 ('STC_CSS_ID', 'scalar_style'),
                 ('STC_CSS_IDENTIFIER', 'keyword4_style'),
                 ('STC_CSS_IDENTIFIER2', 'keyword3_style'),
                 ('STC_CSS_IMPORTANT', 'error_style'),
                 ('STC_CSS_OPERATOR', 'operator_style'),
                 ('STC_CSS_PSEUDOCLASS', 'scalar_style'),
                 ('STC_CSS_SINGLESTRING', 'string_style'),
                 ('STC_CSS_TAG', 'keyword_style'),
                 ('STC_CSS_UNKNOWN_IDENTIFIER', 'unknown_style'),
                 ('STC_CSS_UNKNOWN_PSEUDOCLASS', 'unknown_style'),
                 ('STC_CSS_VALUE', 'char_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_ESS:
        return [ESS_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_ESS:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_ESS:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_ESS:
        return [u'/*', '*/']
    else:
        return list()
#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
