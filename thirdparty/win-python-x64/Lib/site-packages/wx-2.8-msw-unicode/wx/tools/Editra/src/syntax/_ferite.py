###############################################################################
# Name: ferite.py                                                             #
# Purpose: Syntax Definitions for the Ferite Scripting Language               #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
@summary: Lexer configuration module for Ferite Scripting Language

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _ferite.py 62364 2009-10-11 01:02:12Z CJP $"
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
FERITE_KW = (0, "false null self super true abstract alias and arguments "
                "attribute_missing break case class closure conformsToProtocol "
                "constructor continue default deliver destructor diliver "
                "directive do else extends eval final fix for function global "
                "handle if iferr implements include instanceof isa "
                "method_missing modifies monitor namespace new or private "
                "protected protocol public raise recipient rename return "
                "static switch uses using while")

FERITE_TYPES = (1, "boolean string number array object void XML Unix Sys "
                   "String Stream Serialize RMI Posix Number Network Math "
                   "FileSystem Console Array Regexp XSLT")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Ferite""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CPP)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, _cpp.AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [FERITE_KW, FERITE_TYPES, _cpp.DOC_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return _cpp.SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [_cpp.FOLD,]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return ['//',]
