###############################################################################
# Name: ada.py                                                                #
# Purpose: Define Ada syntax for highlighting and other features              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
 FILE: ada.py
 AUTHOR: Cody Precord
 @summary: Lexer configuration module for ada
 @todo: styles, keywords, testing

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _ada.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
ADA_KEYWORDS = (0, "abort abstract accept access aliased all array at begin "
                    "body case constant declare delay delta digits do else "
                    "elsif end entry exception exit for function generic goto "
                    "if in is limited loop new null of others out package "
                    "pragma private procedure protected raise range record "
                    "renames requeue return reverse select separate subtype "
                    "tagged task terminate then type until use when while with")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_ADA_CHARACTER, 'char_style'),
                 (stc.STC_ADA_CHARACTEREOL, 'stringeol_style'),
                 (stc.STC_ADA_COMMENTLINE, 'comment_style'),
                 (stc.STC_ADA_DEFAULT, 'default_style'),
                 (stc.STC_ADA_DELIMITER, 'operator_style'),
                 (stc.STC_ADA_IDENTIFIER, 'default_style'),
                 (stc.STC_ADA_ILLEGAL, 'error_style'),
                 (stc.STC_ADA_LABEL, 'keyword2_style'),   # Style This
                 (stc.STC_ADA_NUMBER, 'number_style'),
                 (stc.STC_ADA_STRING, 'string_style'),
                 (stc.STC_ADA_STRINGEOL, 'stringeol_style'),
                 (stc.STC_ADA_WORD, 'keyword_style')]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Ada""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_ADA)

    def GetKeywords(self):
        """Returns Specified Keywords List"""
        return [ADA_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications"""
        return SYNTAX_ITEMS

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code"""
        return [ u'--' ]
