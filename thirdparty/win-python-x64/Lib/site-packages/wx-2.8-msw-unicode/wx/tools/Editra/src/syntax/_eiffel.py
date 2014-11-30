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
__svnid__ = "$Id: _eiffel.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

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
SYNTAX_ITEMS = [(stc.STC_EIFFEL_CHARACTER, 'char_style'),
                (stc.STC_EIFFEL_COMMENTLINE, 'comment_style'),
                (stc.STC_EIFFEL_DEFAULT, 'default_style'),
                (stc.STC_EIFFEL_IDENTIFIER, 'default_style'),
                (stc.STC_EIFFEL_NUMBER, 'number_style'),
                (stc.STC_EIFFEL_OPERATOR, 'operator_style'),
                (stc.STC_EIFFEL_STRING, 'string_style'),
                (stc.STC_EIFFEL_STRINGEOL, 'stringeol_style'),
                (stc.STC_EIFFEL_WORD, 'keyword_style')]

#---- Extra Properties ----#
FOLD = ("fold", "1")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Eiffel""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_EIFFEL)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [EIFFEL_KW]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'--']
