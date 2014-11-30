###############################################################################
# Name: edje.py                                                               #
# Purpose: Syntax provider module for the Enlightenment Foundation Libraries  #
#          interface language Edje and Edje Data Collection files.            #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: edje.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Edje. Edje is a interface definition
          language that is part of the Enlightenment Foundation Libraries for
          E17.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: edje.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
EDJE_KW = (0, "action after align aspect border clip_to color color2 color3 "
              "color_class effect fit font image inherit name normal max min "
              "mouse_events offset relative repeat_events signal size smooth "
              "source state step target text_class to to_x to_y transition "
              "tween type visible")

EDJE_SEC = (1, "collections description fill group images part parts program "
               "programs rel1 rel2 text")

DOC_KEYWORDS = (2, "TODO FIXME XXX HACK")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_C_DEFAULT', 'default_style'),
                 ('STC_C_COMMENT', 'comment_style'),
                 ('STC_C_COMMENTLINE', 'comment_style'),
                 ('STC_C_COMMENTDOC', 'comment_style'),
                 ('STC_C_COMMENTDOCKEYWORD', 'dockey_style'),
                 ('STC_C_COMMENTDOCKEYWORDERROR', 'error_style'),
                 ('STC_C_COMMENTLINE', 'comment_style'),
                 ('STC_C_COMMENTLINEDOC', 'comment_style'),
                 ('STC_C_CHARACTER', 'char_style'),
                 ('STC_C_GLOBALCLASS', 'global_style'),
                 ('STC_C_IDENTIFIER', 'default_style'),
                 ('STC_C_NUMBER', 'number_style'),
                 ('STC_C_OPERATOR', 'operator_style'),
                 ('STC_C_PREPROCESSOR', 'pre_style'),
                 ('STC_C_REGEX', 'pre_style'),
                 ('STC_C_STRING', 'string_style'),
                 ('STC_C_STRINGEOL', 'stringeol_style'),
                 ('STC_C_UUID', 'pre_style'),
                 ('STC_C_VERBATIM', "number2_style"),
                 ('STC_C_WORD', 'keyword_style'),
                 ('STC_C_WORD2', 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_EDJE:
        return [EDJE_KW, EDJE_SEC]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_EDJE:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_EDJE:
        return [FOLD, FOLD_PRE]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_EDJE:
        return [u'//']
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
