###############################################################################
# Name: latex.py                                                              #
# Purpose: Define TeX/LateX syntax for highlighting and other features        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: latex.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Tex/LaTex.
@todo: Fairly poor needs lots of work.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _latex.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Tex Keywords
TEX_KW = (0, "Downarrow backslash lceil rceil Uparrow downarrow lfloor rfloor "
             "Updownarrow langle rangle Vert")
# ConTeXt Dutch
DUTCH = (1, "")
# ConTeXt English
ENG = (2, "")
# ConTeXt German
GERMAN = (3, "")
# ConTeXt Czech
CZECH = (4, "")
# ConTeXt Italian
ITALIAN = (5, "")
# ConTeXt Romanian
ROMAINIAN = (6, "")

# LaTeXt
# There are no keyword settings available for LaTeX

#---- Syntax Style Specs ----#
# TeX
SYNTAX_ITEMS1 = [(stc.STC_TEX_DEFAULT, 'default_style'),
                 (stc.STC_TEX_COMMAND, 'keyword_style'),
                 (stc.STC_TEX_GROUP, 'scalar_style'),
                 (stc.STC_TEX_SPECIAL, 'operator_style'),
                 (stc.STC_TEX_SYMBOL, 'number_style'),
                 (stc.STC_TEX_TEXT, 'default_style') ]

# LaTeX
SYNTAX_ITEMS2 = [(stc.STC_L_DEFAULT, 'default_style'),
                 (stc.STC_L_COMMAND, 'pre_style'),
                 (stc.STC_L_COMMENT, 'comment_style'),
                 (stc.STC_L_MATH, 'operator_style'),
                 (stc.STC_L_TAG, 'keyword_style')]

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for LaTeX/TeX""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        # TODO: change to LEX_TEX for TeX?
        if self.LangId == synglob.ID_LANG_LATEX:
            self.SetLexer(stc.STC_LEX_LATEX)
        else:
            self.SetLexer(stc.STC_LEX_TEX)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [TEX_KW]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        if self.LangId == synglob.ID_LANG_TEX:
            return SYNTAX_ITEMS1
        else:
            return SYNTAX_ITEMS2

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'%']
