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
__svnid__ = "$Id: _edje.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

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
SYNTAX_ITEMS = [ (stc.STC_C_DEFAULT, 'default_style'),
                 (stc.STC_C_COMMENT, 'comment_style'),
                 (stc.STC_C_COMMENTLINE, 'comment_style'),
                 (stc.STC_C_COMMENTDOC, 'comment_style'),
                 (stc.STC_C_COMMENTDOCKEYWORD, 'dockey_style'),
                 (stc.STC_C_COMMENTDOCKEYWORDERROR, 'error_style'),
                 (stc.STC_C_COMMENTLINE, 'comment_style'),
                 (stc.STC_C_COMMENTLINEDOC, 'comment_style'),
                 (stc.STC_C_CHARACTER, 'char_style'),
                 (stc.STC_C_GLOBALCLASS, 'global_style'),
                 (stc.STC_C_IDENTIFIER, 'default_style'),
                 (stc.STC_C_NUMBER, 'number_style'),
                 (stc.STC_C_OPERATOR, 'operator_style'),
                 (stc.STC_C_PREPROCESSOR, 'pre_style'),
                 (stc.STC_C_REGEX, 'pre_style'),
                 (stc.STC_C_STRING, 'string_style'),
                 (stc.STC_C_STRINGEOL, 'stringeol_style'),
                 (stc.STC_C_UUID, 'pre_style'),
                 (stc.STC_C_VERBATIM, 'number2_style'),
                 (stc.STC_C_WORD, 'keyword_style'),
                 (stc.STC_C_WORD2, 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Edje""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CPP)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [EDJE_KW, EDJE_SEC]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD, FOLD_PRE]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'//']
