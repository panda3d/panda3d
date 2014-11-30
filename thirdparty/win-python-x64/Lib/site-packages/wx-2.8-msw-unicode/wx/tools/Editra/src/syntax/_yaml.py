###############################################################################
# Name: yaml.py                                                               #
# Purpose: Define YAML syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: yaml.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for YAML
@todo: Maybe new custom style for text regions

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _yaml.py 66106 2010-11-10 20:23:27Z CJP $"
__revision__ = "$Revision: 66106 $"

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.stc as stc

# Local Imports
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
YAML_KW = [(0, "true false yes no")]

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_YAML_COMMENT, 'comment_style'),
                (stc.STC_YAML_DEFAULT, 'default_style'),
                (stc.STC_YAML_DOCUMENT, 'scalar_style'),
                (stc.STC_YAML_ERROR, 'error_style'),
                (stc.STC_YAML_IDENTIFIER, 'keyword2_style'),
                (stc.STC_YAML_KEYWORD, 'keyword_style'),
                (stc.STC_YAML_NUMBER, 'number_style'),
                (stc.STC_YAML_REFERENCE, 'global_style'),
                (stc.STC_YAML_TEXT, 'default_style')] # Different style maybe

if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS.append((stc.STC_YAML_OPERATOR, 'operator_style'))

#---- Extra Properties ----#
FOLD_COMMENT = ("fold.comment.yaml", "1")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for YAML""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_YAML)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return YAML_KW

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD_COMMENT]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'#']
