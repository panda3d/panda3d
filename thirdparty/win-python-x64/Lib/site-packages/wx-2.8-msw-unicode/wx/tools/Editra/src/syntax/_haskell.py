###############################################################################
# Name: haskell.py                                                            #
# Purpose: Define Haskell syntax for highlighting and other features          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
 FILE: haskell.py
 AUTHOR: Cody Precord
 @summary: Lexer configuration module for the Haskell Programming Language
 @todo: More custom highlighting

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _haskell.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
HA_KEYWORDS = (0, "as case class data default deriving do forall foreign "
                  "hiding if import in infix infixl infixr instance else let "
                  "mdo module newtype of qualified then type where")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_HA_CAPITAL, 'default_style'),
                (stc.STC_HA_CHARACTER, 'char_style'),
                (stc.STC_HA_CLASS, 'class_style'),
                (stc.STC_HA_COMMENTBLOCK, 'comment_style'),
                (stc.STC_HA_COMMENTBLOCK2, 'comment_style'),
                (stc.STC_HA_COMMENTBLOCK3, 'comment_style'),
                (stc.STC_HA_COMMENTLINE, 'comment_style'),
                (stc.STC_HA_DATA, 'default_style'),
                (stc.STC_HA_DEFAULT, 'default_style'),
                (stc.STC_HA_IDENTIFIER, 'default_style'),
                (stc.STC_HA_IMPORT, 'default_style'), # possibly use custom style
                (stc.STC_HA_INSTANCE, 'default_style'),
                (stc.STC_HA_KEYWORD, 'keyword_style'),
                (stc.STC_HA_MODULE, 'default_style'),
                (stc.STC_HA_NUMBER, 'number_style'),
                (stc.STC_HA_OPERATOR, 'operator_style'),
                (stc.STC_HA_STRING, 'string_style')]

#---- Extra Properties ----#
FOLD = ('fold', '1')

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Haskell""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_HASKELL)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [HA_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'--']
