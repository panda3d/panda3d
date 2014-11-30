###############################################################################
# Name: squirrel.py                                                           #
# Purpose: Syntax Definitions for Squirrel programming language               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
@summary: Lexer configuration module for Squirrel Programming Language

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _squirrel.py 62364 2009-10-11 01:02:12Z CJP $"
__revision__ = "$Revision: 62364 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata
import _cpp

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
SQUIRREL_KW = (0, "break case catch class clone continue const default "
                  "delegate delete do else enum extends for foreach function "
                  "if in local null resume return switch this throw try typeof "
                  "while parent yield constructor vargc vargv instanceof true "
                  "false static")

SQUIRREL_TYPES = (1, "")

#---- End Keyword Definitions ----#

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Squirrel""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CPP)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, _cpp.AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [SQUIRREL_KW, SQUIRREL_TYPES, _cpp.DOC_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return _cpp.SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [_cpp.FOLD,]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return ['//']
