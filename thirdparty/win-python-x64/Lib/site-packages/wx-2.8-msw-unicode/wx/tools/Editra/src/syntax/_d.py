###############################################################################
# Name: d.py                                                                  #
# Purpose: Define D programming language syntax for highlighting and other    #
#          features.                                                          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: d.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for D programming language
@todo: When 2.9 is out switch to the dedicated D Lexer

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _d.py 66108 2010-11-10 21:04:54Z CJP $"
__revision__ = "$Revision: 66108 $"

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.stc as stc

# Local Imports
import synglob
import syndata
from _cpp import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
D_KEYWORDS = (0, "abstract alias align asm assert auto body break case cast "
                 "catch cent class continue debug default delegate delete "
                 "deprecated do else enum export extern false final finally "
                 "for foreach foreach_reverse function goto if import in inout "
                 "interface invariant is lazy mixin module new null out "
                 "override package pragma private protected public return "
                 "scope short struct super switch synchronized template this "
                 "throw true try union unittest version void while with")

D_TYPES = (1, "bool byte cdouble cfloat char const creal dchar double float "
              "idouble ifloat ireal int real long static typeof typedef typeid "
              "ubyte ucent uint ulong ushort volatile wchar")

DOC_KEYWORDS = (2, "TODO FIXME XXX \\author \\brief \\bug \\callgraph "
                   "\\category \\class \\code \\date \\def \\depreciated \\dir "
                   "\\dot \\dotfile \\else \\elseif \\em \\endcode \\enddot "
                   "\\endif \\endverbatim \\example \\exception \\file \\if "
                   "\\ifnot \\image \\include \\link \\mainpage \\name "
                   "\\namespace \\page \\par \\paragraph \\param \\return "
                   "\\retval \\section \\struct \\subpage \\subsection "
                   "\\subsubsection \\test \\todo \\typedef \\union \\var "
                   "\\verbatim \\version \\warning \\$ \\@ \\~ \\< \\> \\# \\% "
                   "HACK ")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS2 = [ (stc.STC_D_CHARACTER, 'char_style'),
                      (stc.STC_D_COMMENT, 'comment_style'),
                      (stc.STC_D_COMMENTDOC, 'comment_style'),
                      (stc.STC_D_COMMENTDOCKEYWORD, 'dockey_style'),
                      (stc.STC_D_COMMENTDOCKEYWORDERROR, 'error_style'),
                      (stc.STC_D_COMMENTLINE, 'comment_style'),
                      (stc.STC_D_COMMENTLINEDOC, 'comment_style'),
                      (stc.STC_D_COMMENTNESTED, 'comment_style'),
                      (stc.STC_D_DEFAULT, 'default_style'),
                      (stc.STC_D_IDENTIFIER, 'default_style'),
                      (stc.STC_D_NUMBER, 'number_style'),
                      (stc.STC_D_OPERATOR, 'operator_style'),
                      (stc.STC_D_STRING, 'string_style'),
                      (stc.STC_D_STRINGB, 'string_style'), #TODO
                      (stc.STC_D_STRINGEOL, 'stringeol_style'),
                      (stc.STC_D_STRINGR, 'string_style'), #TODO
                      (stc.STC_D_TYPEDEF, 'default_style'), # NEEDS STYLE
                      (stc.STC_D_WORD, 'keyword_style'),
                      (stc.STC_D_WORD2, 'keyword2_style'),
                      (stc.STC_D_WORD3, 'keyword3_style'),
                      (stc.STC_D_WORD5, 'default_style'), #TODO
                      (stc.STC_D_WORD6, 'default_style'), #TODO
                      (stc.STC_D_WORD7, 'default_style')] #TODO
else:
    SYNTAX_ITEMS = [ (stc.STC_C_DEFAULT, 'default_style'),
                     (stc.STC_C_COMMENT, 'comment_style'),
                     (stc.STC_C_COMMENTLINE, 'comment_style'),
                     (stc.STC_C_COMMENTDOC, 'comment_style'),
                     (stc.STC_C_COMMENTDOCKEYWORD, 'dockey_style'),
                     (stc.STC_C_COMMENTDOCKEYWORDERROR, 'error_style'),
                     (stc.STC_C_COMMENTLINE, 'comment_style'),
                     (stc.STC_C_COMMENTLINEDOC, 'comment_style'),
                     (stc.STC_C_CHARACTER, 'char_style'),
                     (stc.STC_C_GLOBALCLASS, 'global_style'),
                     (stc.STC_C_IDENTIFIER, 'default_style'),
                     (stc.STC_C_NUMBER, 'number_style'),
                     (stc.STC_C_OPERATOR, 'operator_style'),
                     (stc.STC_C_PREPROCESSOR, 'pre_style'),
                     (stc.STC_C_REGEX, 'pre_style'),
                     (stc.STC_C_STRING, 'string_style'),
                     (stc.STC_C_STRINGEOL, 'stringeol_style'),
                     (stc.STC_C_UUID, 'pre_style'),
                     (stc.STC_C_VERBATIM, 'number2_style'),
                     (stc.STC_C_WORD, 'keyword_style'),
                     (stc.STC_C_WORD2, 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for D""" 
    def __init__(self, langid):
        super(SyntaxData, self).__init__(langid)

        # Setup
        if wx.VERSION >= (2, 9, 0, 0, ''):
            self.SetLexer(stc.STC_LEX_D)
        else:
            self.SetLexer(stc.STC_LEX_CPP)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [D_KEYWORDS, D_TYPES, DOC_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        if wx.VERSION >= (2, 9, 0, 0, ''):
            return SYNTAX_ITEMS2
        else:
            return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD, FOLD_PRE, FOLD_COM]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'//']
