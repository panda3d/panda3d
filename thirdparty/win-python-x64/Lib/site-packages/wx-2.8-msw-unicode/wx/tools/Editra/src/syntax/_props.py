###############################################################################
# Name: props.py                                                              #
# Purpose: Define Properties/ini syntax for highlighting and other features   #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: props.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for properties/config files
          (ini, cfg, ect..).

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _props.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_PROPS_ASSIGNMENT, 'operator_style'),
                (stc.STC_PROPS_COMMENT,   'comment_style'),
                (stc.STC_PROPS_DEFAULT,   'default_style'),
                (stc.STC_PROPS_DEFVAL,    'string_style'),
                (stc.STC_PROPS_KEY,       'scalar_style'),
                (stc.STC_PROPS_SECTION,   'keyword_style')]

#---- Extra Properties ----#
FOLD = ('fold', '1')

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Properties files""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_PROPERTIES)

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return list(u'#')
