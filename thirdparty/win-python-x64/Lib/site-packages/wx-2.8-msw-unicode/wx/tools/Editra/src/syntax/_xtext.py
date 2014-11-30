###############################################################################
# Name: xtext.py                                                             #
# Purpose: Define xtext syntax for highlighting and other features            #
# Author: Igor Dejanovic <igor.dejanovic@gmail.com>                           #
# Copyright: (c) 2009 Igor Dejanovic <igor.dejanovic@gmail.com>               #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: xtext.py
AUTHOR: Igor Dejanovic
@summary: Lexer module for Xtext language.
          For more information see <http://www.eclipse.org/modeling/tmf/> or 
          <http://www.openarchitectureware.org/>.
"""

__author__ = "Igor Dejanovic <igor.dejanovic@gmail.com>"
__svnid__ = "$Id: _xtext.py 64561 2010-06-12 01:49:05Z CJP $"
__revision__ = "$Revision: 64561 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc
from pygments.lexer import RegexLexer, include, bygroups
from pygments.token import Token, Text, Comment, Operator, \
                            Keyword, Name, String, Number, Punctuation
import re

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#
# Style Id's

STC_XTEXT_DEFAULT, \
STC_XTEXT_COMMENT, \
STC_XTEXT_NUMBER, \
STC_XTEXT_STRING, \
STC_XTEXT_STRINGEOL, \
STC_XTEXT_OPERATOR, \
STC_XTEXT_NAME, \
STC_XTEXT_ABSTRACTRULE, \
STC_XTEXT_FEATURE, \
STC_XTEXT_CROSSREF, \
STC_XTEXT_PACKAGE, \
STC_XTEXT_KEYWORD, \
STC_XTEXT_KEYWORD_PSEUDO = range(13)

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Xtext Keywords
KEYWORDS = ("grammar generate import returns enum terminal hidden with as current")
TERMINALS = ("ID INT STRING")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (STC_XTEXT_DEFAULT, 'default_style'),
                 (STC_XTEXT_COMMENT, 'comment_style'),
                 (STC_XTEXT_NUMBER, 'number_style'),
                 (STC_XTEXT_STRING, 'string_style'),
                 (STC_XTEXT_STRINGEOL, 'stringeol_style'),
                 (STC_XTEXT_OPERATOR, 'operator_style'),
                 (STC_XTEXT_NAME, 'default_style'),
                 (STC_XTEXT_ABSTRACTRULE, 'keyword3_style'),
                 (STC_XTEXT_FEATURE, 'default_style'),
                 (STC_XTEXT_CROSSREF, 'class_style'),
                 (STC_XTEXT_PACKAGE, 'class_style'),
                 (STC_XTEXT_KEYWORD, 'keyword_style'),
                 (STC_XTEXT_KEYWORD_PSEUDO, 'keyword2_style'), ]


#-------- Xtext grammar rules ---------------

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for XText""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CONTAINER)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)
        self.RegisterFeature(synglob.FEATURE_STYLETEXT, StyleText)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [(1, KEYWORDS)]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u"//"]

#-----------------------------------------------------------------------------#
# Features

def StyleText(stc, start, end):
    """Style the text
    @param stc: Styled text control instance
    @param start: Start position
    @param end: end position

    """

    for index, token, txt in lexer.get_tokens_unprocessed(stc.GetTextRange(0, end)):
#        print index, token, txt
        style = TOKEN_MAP.get(token, STC_XTEXT_DEFAULT)

#        print "Text=%s, len=%s" % (txt, len(txt))
        stc.StartStyling(index, 0x1f)
        tlen = len(txt)
        if tlen:
            stc.SetStyling(len(txt), style)

def AutoIndenter(estc, pos, ichar):
    """Auto indent xtext code.
    This code is based on python AutoIndenter.
    @param estc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character
    @return: string

    """
    rtxt = u''
    line = estc.GetCurrentLine()
    spos = estc.PositionFromLine(line)
    text = estc.GetTextRange(spos, pos)
    eolch = estc.GetEOLChar()
    inspace = text.isspace()

    # Cursor is in the indent area somewhere or in the column 0.
    if inspace or not len(text):
        estc.AddText(eolch + text)
        return

    text = text.strip()
    if text.endswith(";"):
        estc.AddText(eolch)
        return

    indent = estc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = estc.GetTabWidth()
    else:
        tabw = estc.GetIndent()

    i_space = indent / tabw
    end_spaces = ((indent - (tabw * i_space)) * u" ")

    if text.endswith(u":"):
        i_space += 1

    rtxt = eolch + ichar * i_space + end_spaces

    # Put text in the buffer
    estc.AddText(rtxt)

#-----------------------------------------------------------------------------#

TOKEN_MAP = { Token.String : STC_XTEXT_STRING,
              Token.Comment.Multiline : STC_XTEXT_COMMENT,
              Token.Comment.Single : STC_XTEXT_COMMENT,
              Token.Operator : STC_XTEXT_OPERATOR,
              Token.Punctuation : STC_XTEXT_OPERATOR,
              Token.Number.Integer : STC_XTEXT_NUMBER,
              Token.Keyword : STC_XTEXT_KEYWORD,
              Token.Keyword.Pseudo: STC_XTEXT_KEYWORD_PSEUDO,
              Token.Name : STC_XTEXT_NAME,
              Token.Name.AbstractRule : STC_XTEXT_ABSTRACTRULE,
              Token.Name.Feature : STC_XTEXT_FEATURE,
              Token.Name.CrossRef : STC_XTEXT_CROSSREF,
              Token.Name.Package : STC_XTEXT_PACKAGE,
              Token.Name.Package.EMF : STC_XTEXT_PACKAGE}


class XTextLexer(RegexLexer):
    """
    Xtext lexer based on statefull RegexLexer from pygments library.
    """

    name = 'Xtext'
    aliases = ['xtext']
    filenames = ['*.xtxt']
    mimetypes = ['text/x-xtext']

    flags = re.MULTILINE | re.DOTALL # | re.UNICODE

    #: optional Comment or Whitespace
    #_ws = r'(?:\s|//.*?\n|/[*].*?[*]/)+'

    def AltWords(words):
        """Makes lexer rule for alternative words from the given words list.
        @param words: string consisting of space separated words
        @return: string in the form \bword1\b|\bword2\b|\bword3\b...
        """
        return "|".join([ "\\b%s\\b" % w for w in words.split()])

    _ident = r'\^?[a-zA-Z_\$][a-zA-Z0-9_]*'

    tokens = {
        'root': [
            (include('first')),
            (_ident + r'(\.' + _ident + r')+', Name.Package),
            ('(' + _ident + r')(\s*)(returns)',
                bygroups(Name.AbstractRule, Text.Whitespace, Keyword), 'parserrule'),
            ('(' + _ident + r')(\s*)(:)',
                bygroups(Name.AbstractRule, Text.Whitespace, Punctuation), 'parserrule'),
            (_ident, Name),
        ],
        'first': [
            (r'/\*', Comment.Multiline, 'comment'),
            (r'\n', Token.EndOfLine),
            (r'//[^\n]*$', Comment.Single),
            (r'[ \t]+', Text.Whitespace),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'(\\\\|\\'|[^'])*'", String),
            (r'\*|\?|\+|!|\||=|\?=|\+=|\.\.|->', Operator),
            (r'[()\[\]{}:]', Punctuation),
            (r'[0-9]+', Number.Integer),
            (AltWords(KEYWORDS), Keyword),
            (AltWords(TERMINALS), Keyword.Pseudo),
            (_ident + r'(::' + _ident + r')+', Name.Package.EMF),
        ],
        'parserrule': [
            (include('first')),
            ('(' + _ident + r'(\.' + _ident + r')?)([ \t]*)(=|\?=|\+=)',
                bygroups(Name.Feature, Text.Whitespace, Operator)),
            (_ident + r'(\.' + _ident + r')+', Name.Package),
            (_ident, Name.CrossRef),
            (r';', Punctuation, "#pop"),
        ],
        'comment': [
            # Nested and multiline comments
            (r'/\*', Comment.Multiline, "#push"),
            (r'\*/', Comment.Multiline, "#pop"),
            (r'\n', Token.EndOfLine),
            (r'[^/*\n]+', Comment.Multiline),
            (r'\*|\/', Comment.Multiline),
        ],
    }

lexer = XTextLexer()

if __name__=='__main__':
    import codecs, sys
    ftext = codecs.open(sys.argv[1], "r", "utf-8")
    text = ftext.read()
    ftext.close()
    line=1
    for index, token, txt in lexer.get_tokens_unprocessed(text):
        if token is Token.EndOfLine:
            line += 1
        print line, token, txt
