###############################################################################
# Name: boo.py                                                                #
# Purpose: Define Boo language syntax and other features                      #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: boo.py
@summary: Defines language and syntax highlighting settings for the Boo
          programming language
@todo: support for C style comment regions
"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _boo.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata
import _python

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
BOO_KW = (0, "abstract and as AST break callable cast char class constructor "
             "continue def destructor do elif else ensure enum event except "
             "failure final false for from get given goto if import in "
             "interface internal is isa not null of or otherwise override "
             "namespace partial pass private protected public raise ref retry "
             "return self set static super struct success transient true try "
             "typeof unless virtual when while yield")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [x for x in _python.SYNTAX_ITEMS if x[0] != 'STC_P_DECORATOR']
SYNTAX_ITEMS.append((stc.STC_P_DECORATOR, 'default_style'))

#---- Extra Properties ----#

FOLD = ("fold", "1")
TIMMY = ("tab.timmy.whinge.level", "1") # Mark Inconsistant indentation

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Boo
    @todo: needs custom highlighting handler

    """ 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_PYTHON)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [BOO_KW]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD, TIMMY]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'#']
