###############################################################################
# Name: eiffel.py                                                             #
# Purpose: Define Eiffel syntax for highlighting and other features           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: eiffel.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Eiffel
@todo: look into why io.anything is highlighted as a number

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: eiffel.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
EIFFEL_KW = (0, "alias all and any as bit boolean check class character clone "
                "cluster create creation current debug deferred div do double "
                "else elseif end ensure equal expanded export external false "
                "feature forget from frozen general if implies indexing infix "
                "inherit inspect integer invariant is language like local loop "
                "mod name nochange none not obsolete old once or platform "
                "pointer prefix precursor program real redefine rename require "
                "rescue result retry root select separate string strip then "
                "true undefine unique until variant void when xor")
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_EIFFEL_CHARACTER', 'char_style'),
                ('STC_EIFFEL_COMMENTLINE', 'comment_style'),
                ('STC_EIFFEL_DEFAULT', 'default_style'),
                ('STC_EIFFEL_IDENTIFIER', 'default_style'),
                ('STC_EIFFEL_NUMBER', 'number_style'),
                ('STC_EIFFEL_OPERATOR', 'operator_style'),
                ('STC_EIFFEL_STRING', 'string_style'),
                ('STC_EIFFEL_STRINGEOL', 'stringeol_style'),
                ('STC_EIFFEL_WORD', 'keyword_style')]

#---- Extra Properties ----#
FOLD = ("fold", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_EIFFEL:
        return [EIFFEL_KW]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_EIFFEL:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_EIFFEL:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_EIFFEL:
        return [u'--']
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
