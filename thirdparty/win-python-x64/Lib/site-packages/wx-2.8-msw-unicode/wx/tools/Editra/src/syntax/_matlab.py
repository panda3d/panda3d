###############################################################################
# Name: matlab.py                                                             #
# Purpose: Define Matlab and Octave syntax for highlighting and other features#
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: matlab.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Matlab and Octave

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _matlab.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
MATLAB_KW = (0, "break case catch continue else elseif end for function "
                "global if otherwise persistent return switch try while")

OCTAVE_KW = (0, "break case catch continue do else elseif end "
                "end_unwind_protect endfor endif endswitch endwhile for "
                "function endfunction global if otherwise persistent return "
                "switch try until unwind_protect unwind_protect_cleanup while")
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_MATLAB_COMMAND, 'funct_style'),
                (stc.STC_MATLAB_COMMENT, 'comment_style'),
                (stc.STC_MATLAB_DEFAULT, 'default_style'),
                (stc.STC_MATLAB_DOUBLEQUOTESTRING, 'string_style'),
                (stc.STC_MATLAB_IDENTIFIER, 'default_style'),
                (stc.STC_MATLAB_KEYWORD, 'keyword_style'),
                (stc.STC_MATLAB_NUMBER, 'number_style'),
                (stc.STC_MATLAB_OPERATOR, 'operator_style'),
                (stc.STC_MATLAB_STRING, 'string_style')]

#---- Extra Properties ----#
FOLD = ('fold', '1')

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for MatLab and Octave""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        if self.LangId == synglob.ID_LANG_MATLAB:
            self.SetLexer(stc.STC_LEX_MATLAB)
        else:
            self.SetLexer(stc.STC_LEX_OCTAVE)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        if self.LangId == synglob.ID_LANG_MATLAB:
            return [MATLAB_KW]
        elif self.LangId == synglob.ID_LANG_OCTAVE:
            return [OCTAVE_KW]
        else:
            return list()

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        if self.LangId == synglob.ID_LANG_MATLAB:
            return [u'%']
        elif self.LangId == synglob.ID_LANG_OCTAVE:
            return [u'#']
        else:
            return list()
