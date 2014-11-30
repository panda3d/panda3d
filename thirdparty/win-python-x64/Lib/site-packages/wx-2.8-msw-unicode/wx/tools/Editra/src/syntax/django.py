###############################################################################
# Name: django.py                                                             #
# Purpose: Define Django syntax for highlighting and other features           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: django.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Django Templates.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: django.py 54460 2008-07-02 01:56:51Z CJP $"
__revision__ = "$Revision: 54460 $"

#-----------------------------------------------------------------------------#
# Imports
from pygments.token import Token
from pygments.lexers import get_lexer_by_name

#Local Imports
import synglob

#-----------------------------------------------------------------------------#
# Style Id's

STC_DJANGO_DEFAULT, \
STC_DJANGO_COMMENT, \
STC_DJANGO_NUMBER, \
STC_DJANGO_STRING, \
STC_DJANGO_STRINGEOL, \
STC_DJANGO_SCALAR, \
STC_DJANGO_OPERATOR, \
STC_DJANGO_PREPROCESSOR, \
STC_DJANGO_ATTRIBUTE, \
STC_DJANGO_TAG, \
STC_DJANGO_BUILTIN, \
STC_DJANGO_KEYWORD = range(12)

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Python Keywords
KEYWORDS = ("true false undefined null in as reversed recursive not and or is "
            "if else import with loop block forloop")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (STC_DJANGO_DEFAULT, 'default_style'),
                 (STC_DJANGO_COMMENT, 'comment_style'),
                 (STC_DJANGO_NUMBER, 'number_style'),
                 (STC_DJANGO_STRING, 'string_style'),
                 (STC_DJANGO_STRINGEOL, 'stringeol_style'),
                 (STC_DJANGO_SCALAR, 'scalar_style'),
                 (STC_DJANGO_OPERATOR, 'operator_style'),
                 (STC_DJANGO_PREPROCESSOR, 'pre_style'),
                 (STC_DJANGO_ATTRIBUTE, 'keyword2_style'),
                 (STC_DJANGO_TAG, 'keyword_style'),       # Need new tag
                 (STC_DJANGO_BUILTIN, 'keyword4_style'),
                 (STC_DJANGO_KEYWORD, 'keyword_style'), ]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_DJANGO:
        return [(1, KEYWORDS)]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_DJANGO:
        return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_DJANGO:
        return []

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_DJANGO:
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
    lexer = get_lexer_by_name("html+django")
    for token, txt in lexer.get_tokens(stc.GetTextRange(0, end)):
#        print token, txt
        style = TOKEN_MAP.get(token, STC_DJANGO_DEFAULT)
        if style == STC_DJANGO_PREPROCESSOR and txt.startswith(u'#'):
            style = STC_DJANGO_COMMENT
#        elif style == STC_DJANGO_STRING and txt[-1] not in '"\'':
#            style = STC_DJANGO_STRINGEOL

        tlen = len(txt)
        if tlen:
            stc.SetStyling(len(txt), style)
        cpos += len(txt)
        stc.StartStyling(cpos, 0x1f)

#-----------------------------------------------------------------------------#

TOKEN_MAP = { Token.Literal.String : STC_DJANGO_STRING,
              Token.Comment.Preproc : STC_DJANGO_PREPROCESSOR,
              Token.Comment : STC_DJANGO_COMMENT,
              Token.Name.Builtin : STC_DJANGO_BUILTIN,
              Token.Operator : STC_DJANGO_OPERATOR,
              Token.Punctuation : STC_DJANGO_OPERATOR,
              Token.Number : STC_DJANGO_NUMBER,
              Token.Keyword : STC_DJANGO_KEYWORD,
              Token.Name.Attribute : STC_DJANGO_ATTRIBUTE,
              Token.String.Interpol : STC_DJANGO_SCALAR,
              Token.Name.Tag : STC_DJANGO_TAG }
              