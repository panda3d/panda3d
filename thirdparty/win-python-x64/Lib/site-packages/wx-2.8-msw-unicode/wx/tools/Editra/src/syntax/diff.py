###############################################################################
# Name: diff.py                                                               #
# Purpose: Define Diff/Patch file syntax for highlighting and other features  #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: diff.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Diff/Patch files
@todo:

"""

__author__ = "Cody Precord <cprecord@editra.org"
__svnid__ = "$Id: diff.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
# None
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_DIFF_ADDED', 'global_style'),
                ('STC_DIFF_COMMAND', 'pre_style'),
                ('STC_DIFF_COMMENT', 'comment_style'),
                ('STC_DIFF_DEFAULT', 'default_style'),
                ('STC_DIFF_DELETED', 'error_style'),
                ('STC_DIFF_HEADER', 'comment_style'),
                ('STC_DIFF_POSITION', 'pre_style')]

#---- Extra Properties ----#
FOLD = ('fold', '1')
FOLD_COMPACT = ('fold.compact', '1')
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_DIFF:
        return list()
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_DIFF:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_DIFF:
        return [FOLD, FOLD_COMPACT]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_DIFF:
        return ['--- ']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
