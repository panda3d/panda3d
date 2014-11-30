###############################################################################
# Name: pike.py                                                               #
# Purpose: Define highlighting/syntax for Pike programming language           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: pike.py
@summary: Defines syntax and highlighting settings for the Pike programming
          language. Pike is very similar in form to C/CPP so the Cpp lexer is
          used to provide the highlighting settings.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _pike.py 62364 2009-10-11 01:02:12Z CJP $"
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
PIKE_KW = (0, "goto break return continue case default if else switch while "
              "foreach do gauge destruct lambda inherit import typeof catch "
              "for inline nomask")

PIKE_TYPE = (1, "private protected public static "
                "int string void float mapping array multiset mixed program "
                "object function")
#---- End Keyword Definitions ----#

#-----------------------------------------------------------------------------#

class SyntaxData(_cpp.SyntaxData):
    """SyntaxData object for Pike""" 
    def __init__(self, langid):
        _cpp.SyntaxData.__init__(self, langid)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [PIKE_KW, PIKE_TYPE, _cpp.DOC_KEYWORDS]

    def GetCommentPattern(self):
        """Get the comment pattern"""
        return [u"//"]
