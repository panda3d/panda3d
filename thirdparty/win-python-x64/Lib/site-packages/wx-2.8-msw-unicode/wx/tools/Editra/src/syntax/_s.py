###############################################################################
# Name: s.py                                                                  #
# Purpose: Define S and R syntax for highlighting and other features          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: s.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for the S and R statistical languages

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _s.py 66108 2010-11-10 21:04:54Z CJP $"
__revision__ = "$Revision: 66108 $"

#-----------------------------------------------------------------------------#
# Imports
from pygments.token import Token
from pygments.lexers import get_lexer_by_name
import wx
import wx.stc as stc

#Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#
# Style Id's

STC_S_DEFAULT, \
STC_S_COMMENT, \
STC_S_NUMBER, \
STC_S_STRING, \
STC_S_STRINGEOL, \
STC_S_OPERATOR, \
STC_S_KEYWORD = range(7)

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

KEYWORDS = "for while if else break return function NULL NA TRUE FALSE"

#---- Syntax Style Specs ----#
if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS = [ (stc.STC_R_BASEKWORD,  'default_style'), #TODO
                     (stc.STC_R_COMMENT,    'comment_style'),
                     (stc.STC_R_DEFAULT,    'default_style'),
                     (stc.STC_R_IDENTIFIER, 'default_style'),
                     (stc.STC_R_INFIX,      'default_style'), #TODO
                     (stc.STC_R_INFIXEOL,   'default_style'), #TODO
                     (stc.STC_R_KWORD,      'keyword_style'),
                     (stc.STC_R_NUMBER,     'number_style'),
                     (stc.STC_R_OPERATOR,   'operator_style'),
                     (stc.STC_R_OTHERKWORD, 'default_style'), #TODO
                     (stc.STC_R_STRING,     'string_style'),
                     (stc.STC_R_STRING2,    'string_style')] #TODO
else:
    SYNTAX_ITEMS = [ (STC_S_DEFAULT,   'default_style'),
                     (STC_S_COMMENT,   'comment_style'),
                     (STC_S_NUMBER,    'number_style'),
                     (STC_S_STRING,    'string_style'),
                     (STC_S_STRINGEOL, 'stringeol_style'),
                     (STC_S_OPERATOR,  'operator_style'),
                     (STC_S_KEYWORD,   'keyword_style') ]

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for R and S""" 
    def __init__(self, langid):
        super(SyntaxData, self).__init__(langid)

        # Setup
        if wx.VERSION >= (2, 9, 0, 0, ''):
            self.SetLexer(stc.STC_LEX_R)
        else:
            self.SetLexer(stc.STC_LEX_CONTAINER)
            self.RegisterFeature(synglob.FEATURE_STYLETEXT, StyleText)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        if wx.VERSION >= (2, 9, 0, 0, ''):
            return [(0, KEYWORDS),]
        else:
            return [(1, KEYWORDS)]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u"#",]

#-----------------------------------------------------------------------------#

def StyleText(_stc, start, end):
    """Style the text
    @param stc: Styled text control instance
    @param start: Start position
    @param end: end position
    @todo: performance improvements
    @todo: style errors caused by unicode characters (related to internal utf8)

    """
    cpos = 0
    _stc.StartStyling(cpos, 0x1f)
    lexer = get_lexer_by_name("s")
    is_wineol = _stc.GetEOLMode() == stc.STC_EOL_CRLF
    for token, txt in lexer.get_tokens(_stc.GetTextRange(0, end)):
        style = TOKEN_MAP.get(token, STC_S_DEFAULT)

        tlen = len(txt)

        # Account for \r\n end of line characters
        if is_wineol and "\n" in txt:
            tlen += txt.count("\n")

        if tlen:
            _stc.SetStyling(tlen, style)
        cpos += tlen
        _stc.StartStyling(cpos, 0x1f)

#-----------------------------------------------------------------------------#

TOKEN_MAP = { Token.Literal.String  : STC_S_STRING,
              Token.Comment         : STC_S_COMMENT,
              Token.Comment.Single  : STC_S_COMMENT,
              Token.Operator        : STC_S_OPERATOR,
              Token.Punctuation     : STC_S_OPERATOR,
              Token.Number          : STC_S_NUMBER,
              Token.Literal.Number  : STC_S_NUMBER,
              Token.Keyword         : STC_S_KEYWORD,
              Token.Keyword.Constant: STC_S_KEYWORD }
              