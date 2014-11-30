###############################################################################
# Name: nonmem.py                                                             #
# Purpose: Define NONMEM syntax for highlighting and other features           #
# Author: Robert McLeay <robert@fearthecow.net>                               #
# Copyright: (c) 2008 Cody Precord <staff>                                    #
#            (c) 2008 Torsten Mohr <none_yet>                                 #
#            (c) 2010 Robert McLeay <robert@fearthecow.net>                   #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: nonmem.py
AUTHOR: Cody Precord, Torsten Mohr, Robert McLeay
@summary: Lexer configuration module for NONMEM control streams.

"""

__author__ = "Cody Precord <cprecord>, Torsten Mohr <none_yet>"
__svnid__ = "$Id: _nonmem.py 65461 2010-09-02 04:09:42Z CJP $"
__revision__ = "$Revision: 65461 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc
from pygments.lexer import RegexLexer, include, bygroups
from pygments.token import Token, Text, Comment, Operator, \
                            Keyword, Name, String, Number, Punctuation

import re

#Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#
# Style Id's

# Style Id's

STC_NONMEM_DEFAULT, \
STC_NONMEM_COMMENT, \
STC_NONMEM_NUMBER, \
STC_NONMEM_STRING, \
STC_NONMEM_STRINGEOL, \
STC_NONMEM_OPERATOR, \
STC_NONMEM_NAME, \
STC_NONMEM_ABSTRACTRULE, \
STC_NONMEM_FEATURE, \
STC_NONMEM_CROSSREF, \
STC_NONMEM_PACKAGE, \
STC_NONMEM_KEYWORD, \
STC_NONMEM_KEYWORD_PSEUDO = range(13)

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Xtext Keywords
KEYWORDS = ("grammar generate import returns enum terminal hidden with as current")
TERMINALS = ("ID INT STRING")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (STC_NONMEM_DEFAULT, 'default_style'),
                 (STC_NONMEM_COMMENT, 'comment_style'),
                 (STC_NONMEM_NUMBER, 'number_style'),
                 (STC_NONMEM_STRING, 'string_style'),
                 (STC_NONMEM_STRINGEOL, 'stringeol_style'),
                 (STC_NONMEM_OPERATOR, 'operator_style'),
                 (STC_NONMEM_NAME, 'default_style'),
                 (STC_NONMEM_ABSTRACTRULE, 'keyword3_style'),
                 (STC_NONMEM_FEATURE, 'default_style'),
                 (STC_NONMEM_CROSSREF, 'class_style'),
                 (STC_NONMEM_PACKAGE, 'class_style'),
                 (STC_NONMEM_KEYWORD, 'keyword_style'),
                 (STC_NONMEM_KEYWORD_PSEUDO, 'keyword2_style'), ]
   
NONMEM_KEYWORDS = ("ADVAN\d+ BLOCK COMP COND CONDITIONAL DEFDOSE DEFOBS "
                   "DOWHILE ELSE ENDDO ENDIF EXP FILE FIX FIXED ICALL IF "
                   "IGNORE INTER INTERACTION LOG MATRIX MAX MAXEVAL METHOD "
                   "NEWIND NOABORT NOAPPEND NOPRINT NOHEADER ONEHEADER PRINT "
                   "SIG SIGDIGITS SLOW SUBPROBLEMS THEN TOL TRANS1 TRANS2 "
                   "TRANS3 TRANS4 ONLYSIM ENDIF")
NONMEM_PARAMS = "DADT ERR EPS ETA THETA"
#NONMEM_SPECIAL = "\$COV $DATA $DES $ERROR $EST \$INPUT $MODEL $OMEGA $PRED \\$PK $PROB $PROBLEM $SIGMA $SIM $SUB $TABLE $THETA"

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for IssueLists
    This class is primarly intended as an example to creating a custom
    lexer

    """ 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CONTAINER)
        self.RegisterFeature(synglob.FEATURE_STYLETEXT, StyleText)

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

#---- End Required Module Functions ----#

def StyleText(stc, start, end):
    """Style the text
    @param stc: Styled text control instance
    @param start: Start position
    @param end: end position

    """
    for index, token, txt in lexer.get_tokens_unprocessed(stc.GetTextRange(0, end)):
#        print index, token, txt
        style = TOKEN_MAP.get(token, STC_NONMEM_DEFAULT)

#        print "Text=%s, len=%s" % (txt, len(txt))
        stc.StartStyling(index, 0x1f)
        tlen = len(txt)
        if tlen:
            stc.SetStyling(len(txt), style)

TOKEN_MAP = { Token.String : STC_NONMEM_STRING,
              Token.Comment.Multiline : STC_NONMEM_COMMENT,
              Token.Comment.Single : STC_NONMEM_COMMENT,
              Token.Operator : STC_NONMEM_OPERATOR,
              Token.Punctuation : STC_NONMEM_OPERATOR,
              Token.Number.Integer : STC_NONMEM_NUMBER,
              Token.Keyword : STC_NONMEM_KEYWORD,
              Token.Keyword.Pseudo: STC_NONMEM_KEYWORD_PSEUDO,
              Token.Name : STC_NONMEM_NAME,
              Token.Name.AbstractRule : STC_NONMEM_ABSTRACTRULE,
              Token.Name.Feature : STC_NONMEM_FEATURE,
              Token.Name.CrossRef : STC_NONMEM_CROSSREF,
              Token.Name.Package : STC_NONMEM_PACKAGE,
              Token.Name.Package.EMF : STC_NONMEM_PACKAGE}


class NONMEMLexer(RegexLexer):
    """
    Nonmem lexer based on statefull RegexLexer from pygments library.
    """

    name = 'NONMEM'
    aliases = ['nonmem']
    filenames = ['*.ctl']
    mimetypes = ['text/x-nonmem']

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
            (r';[^\n]*$', Comment.Single),
            (r'\$[A-Z]+', Name.Package),
            (r'[ \t]+', Text.Whitespace),
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'(\\\\|\\'|[^'])*'", String),
            (r'\*|\?|\+|!|\||=|\?=|\+=|\.\.|->', Operator),
            (r'[()\[\]{}:]', Punctuation),
            (r'[0-9]+', Number.Integer),
            (AltWords(NONMEM_KEYWORDS), Keyword),
            (AltWords(NONMEM_PARAMS), Keyword.Pseudo),
        #    (AltWords(NONMEM_SPECIAL), Name.Package),
        ],
        'parserrule': [
            (include('first')),
            ('(' + _ident + r'(\.' + _ident + r')?)([ \t]*)(=|\?=|\+=)',
                bygroups(Name.Feature, Text.Whitespace, Operator)),
            (_ident + r'(\.' + _ident + r')+', Name.Package),
            (_ident, Name.CrossRef),
        ],
    }

lexer = NONMEMLexer()

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
