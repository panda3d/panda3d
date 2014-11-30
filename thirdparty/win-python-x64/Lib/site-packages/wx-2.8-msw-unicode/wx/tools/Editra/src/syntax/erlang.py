###############################################################################
# Name: erlang.py                                                             #
# Purpose: Define Erlang syntax for highlighting and other features           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: erlang.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for the Erlang Programming Language
@todo: better styling

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: erlang.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
KEYWORDS = (0, "compile define else endif export file ifdef ifndef import "
               "include include_lib module record undef author copyright doc "
               "after begin case catch cond end fun if let of query receive "
               "when define record export import include include_lib else "
               "endif undef apply attribute call do in letrec module primop "
               "try")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_ERLANG_ATOM', 'default_style'),       # need new tag
                ('STC_ERLANG_CHARACTER', 'char_style'),
                ('STC_ERLANG_COMMENT', 'comment_style'),
                ('STC_ERLANG_DEFAULT', 'default_style'),
                ('STC_ERLANG_FUNCTION_NAME', 'funct_style'),
                ('STC_ERLANG_KEYWORD', 'keyword_style'),
                ('STC_ERLANG_MACRO', 'pre_style'),
                ('STC_ERLANG_NODE_NAME', 'string_style'),   # maybe change
                ('STC_ERLANG_NUMBER', 'number_style'),
                ('STC_ERLANG_OPERATOR', 'operator_style'),
                ('STC_ERLANG_RECORD', 'keyword2_style'),
                ('STC_ERLANG_SEPARATOR', 'default_style'),  # need style?
                ('STC_ERLANG_STRING', 'string_style'),
                ('STC_ERLANG_UNKNOWN', 'unknown_style'),
                ('STC_ERLANG_VARIABLE', 'default_style')]   # need custom?

#---- Extra Properties ----#
FOLD = ('fold', '1')
FOLD_CMT = ('fold.comments', '1')
FOLD_KW = ('fold.keywords', '1')
FOLD_BRACE = ('fold.braces', '1')

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_ERLANG:
        return [KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_ERLANG:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_ERLANG:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_ERLANG:
        return [u'%%']
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
