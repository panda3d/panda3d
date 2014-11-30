###############################################################################
# Name: mako.py                                                               #
# Purpose: Define Mako syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: mako.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Mako Templates.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: mako.py 55737 2008-09-19 16:12:31Z CJP $"
__revision__ = "$Revision: 55737 $"

#-----------------------------------------------------------------------------#
# Imports
from pygments.token import Token
from pygments.lexers import get_lexer_by_name

#Local Imports
import synglob

#-----------------------------------------------------------------------------#
# Style Id's

STC_MAKO_DEFAULT, \
STC_MAKO_COMMENT, \
STC_MAKO_NUMBER, \
STC_MAKO_STRING, \
STC_MAKO_STRINGEOL, \
STC_MAKO_SCALAR, \
STC_MAKO_OPERATOR, \
STC_MAKO_PREPROCESSOR, \
STC_MAKO_ATTRIBUTE, \
STC_MAKO_TAG, \
STC_MAKO_BUILTIN, \
STC_MAKO_KEYWORD = range(12)

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Python Keywords
KEYWORDS = "include inherit namespace page"

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (STC_MAKO_DEFAULT, 'default_style'),
                 (STC_MAKO_COMMENT, 'comment_style'),
                 (STC_MAKO_NUMBER, 'number_style'),
                 (STC_MAKO_STRING, 'string_style'),
                 (STC_MAKO_STRINGEOL, 'stringeol_style'),
                 (STC_MAKO_SCALAR, 'scalar_style'),
                 (STC_MAKO_OPERATOR, 'operator_style'),
                 (STC_MAKO_PREPROCESSOR, 'pre_style'),
                 (STC_MAKO_ATTRIBUTE, 'keyword2_style'),
                 (STC_MAKO_TAG, 'keyword_style'),       # Need new tag
                 (STC_MAKO_BUILTIN, 'keyword4_style'),
                 (STC_MAKO_KEYWORD, 'keyword_style'), ]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_MAKO:
        return [(1, KEYWORDS)]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_MAKO:
        return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_MAKO:
        return []

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_MAKO:
        return [u"#",]

#---- End Required Module Functions ----#

def StyleText(stc, start, end):
    """Style the text
    @param stc: Styled text control instance
    @param start: Start position
    @param end: end position

    """
    cpos = 0
    stc.StartStyling(cpos, 0x1f)
    lexer = get_lexer_by_name("html+mako")
    for token, txt in lexer.get_tokens(stc.GetTextRange(0, end)):
#        print token, txt
        style = TOKEN_MAP.get(token, STC_MAKO_DEFAULT)
        if style == STC_MAKO_PREPROCESSOR and txt.startswith(u'#'):
            style = STC_MAKO_COMMENT
#        elif style == STC_MAKO_STRING and txt[-1] not in '"\'':
#            style = STC_MAKO_STRINGEOL

        tlen = len(txt)
        if tlen:
            stc.SetStyling(tlen, style)
        cpos += tlen
        stc.StartStyling(cpos, 0x1f)

#-----------------------------------------------------------------------------#

TOKEN_MAP = { Token.Literal.String : STC_MAKO_STRING,
              Token.Comment.Preproc : STC_MAKO_PREPROCESSOR,
              Token.Comment : STC_MAKO_COMMENT,
              Token.Name.Builtin : STC_MAKO_BUILTIN,
              Token.Operator : STC_MAKO_OPERATOR,
              Token.Punctuation : STC_MAKO_OPERATOR,
              Token.Number : STC_MAKO_NUMBER,
              Token.Keyword : STC_MAKO_KEYWORD,
              Token.Name.Attribute : STC_MAKO_ATTRIBUTE,
              Token.String.Interpol : STC_MAKO_SCALAR,
              Token.Name.Tag : STC_MAKO_TAG }
              