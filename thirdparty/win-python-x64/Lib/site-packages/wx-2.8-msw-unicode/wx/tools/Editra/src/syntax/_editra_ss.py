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
__svnid__ = "$Id: _editra_ss.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata
from _css import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Editra Style Sheet Keywords
ESS_KEYWORDS = (0, "fore back face size eol bold italic modifiers")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_CSS_DEFAULT, 'default_style'),
                 (stc.STC_CSS_CLASS, 'global_style'),
                 (stc.STC_CSS_COMMENT, 'comment_style'),
                 (stc.STC_CSS_DIRECTIVE, 'directive_style'),
                 (stc.STC_CSS_DOUBLESTRING, 'string_style'),
                 (stc.STC_CSS_ID, 'scalar_style'),
                 (stc.STC_CSS_IDENTIFIER, 'keyword4_style'),
                 (stc.STC_CSS_IDENTIFIER2, 'keyword3_style'),
                 (stc.STC_CSS_IMPORTANT, 'error_style'),
                 (stc.STC_CSS_OPERATOR, 'operator_style'),
                 (stc.STC_CSS_PSEUDOCLASS, 'scalar_style'),
                 (stc.STC_CSS_SINGLESTRING, 'string_style'),
                 (stc.STC_CSS_TAG, 'keyword_style'),
                 (stc.STC_CSS_UNKNOWN_IDENTIFIER, 'unknown_style'),
                 (stc.STC_CSS_UNKNOWN_PSEUDOCLASS, 'unknown_style'),
                 (stc.STC_CSS_VALUE, 'char_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
#------------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Editra Style Sheets""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CSS)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [ESS_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'/*', '*/']
