###############################################################################
# Name: caml.py                                                               #
# Purpose: Define Caml syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: caml.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Caml

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: caml.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
# Objective Caml 3 textual keywords
CAML_KW1 = (0, "and as assert asr begin class constraint do done downto else "
               "end exception external false for fun function functor if in "
               "include inherit initializer land lazy let lor lsl lsr lxor "
               "match method mod module mutable new object of open or private "
               "rec sig struct then to true try type val virtual when while "
               "with")

# Caml optional keywords
CAML_KW2 = (1, "option Some None ignore ref lnot succ pred parser")

# Caml type/library keywords
CAML_KW3 = (2, "array bool char float int list string unit")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_CAML_CHAR', 'char_style'),
                ('STC_CAML_COMMENT', 'comment_style'),
                ('STC_CAML_COMMENT1', 'comment_style'),
                ('STC_CAML_COMMENT2', 'comment_style'),
                ('STC_CAML_COMMENT3', 'comment_style'),
                ('STC_CAML_DEFAULT', 'default_style'),
                ('STC_CAML_IDENTIFIER', 'default_style'),
                ('STC_CAML_KEYWORD', 'keyword_style'),
                ('STC_CAML_KEYWORD2', 'pre_style'),
                ('STC_CAML_KEYWORD3', 'keyword2_style'),
                ('STC_CAML_LINENUM', 'number_style'),
                ('STC_CAML_NUMBER', 'number_style'),
                ('STC_CAML_OPERATOR', 'operator_style'),
                ('STC_CAML_STRING', 'string_style'),
                ('STC_CAML_TAGNAME', 'directive_style')] #STYLE ME

#---- Extra Properties ----#
FOLD = ('fold', '1')

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_CAML:
        return [CAML_KW1, CAML_KW2, CAML_KW3]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_CAML:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_CAML:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_CAML:
        return [u'(*', u'*)']
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
