###############################################################################
# Name: ruby.py                                                               #
# Purpose: Define Ruby syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: ruby.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Ruby.
@todo: Default Style Refinement.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _ruby.py 64561 2010-06-12 01:49:05Z CJP $"
__revision__ = "$Revision: 64561 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc
import re

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Ruby Keywords
# NOTE: putting words with question marks in them causes an assertion to be
#       raised when showing the list in the keyword helper! defined?
RUBY_KW = (0, "__FILE__ and def end in or self unless __LINE__ begin defined "
              "ensure module redo super until BEGIN break do false next "
              "require rescue then when END case else for nil retry true while "
              "alias class elsif if not return undef yieldr puts raise "
              "protected private")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_RB_BACKTICKS, 'scalar_style'),
                 (stc.STC_RB_CHARACTER, 'char_style'),
                 (stc.STC_RB_CLASSNAME, 'class_style'),
                 (stc.STC_RB_CLASS_VAR, 'default_style'), # STYLE ME
                 (stc.STC_RB_COMMENTLINE, 'comment_style'),
                 (stc.STC_RB_DATASECTION, 'default_style'), # STYLE ME
                 (stc.STC_RB_DEFAULT, 'default_style'),
                 (stc.STC_RB_DEFNAME, 'keyword3_style'), # STYLE ME
                 (stc.STC_RB_ERROR, 'error_style'),
                 (stc.STC_RB_GLOBAL, 'global_style'),
                 (stc.STC_RB_HERE_DELIM, 'default_style'), # STYLE ME
                 (stc.STC_RB_HERE_Q, 'here_style'),
                 (stc.STC_RB_HERE_QQ, 'here_style'),
                 (stc.STC_RB_HERE_QX, 'here_style'),
                 (stc.STC_RB_IDENTIFIER, 'default_style'),
                 (stc.STC_RB_INSTANCE_VAR, 'scalar2_style'),
                 (stc.STC_RB_MODULE_NAME, 'global_style'), # STYLE ME
                 (stc.STC_RB_NUMBER, 'number_style'),
                 (stc.STC_RB_OPERATOR, 'operator_style'),
                 (stc.STC_RB_POD, 'default_style'), # STYLE ME
                 (stc.STC_RB_REGEX, 'regex_style'), # STYLE ME
                 (stc.STC_RB_STDIN, 'default_style'), # STYLE ME
                 (stc.STC_RB_STDOUT, 'default_style'), # STYLE ME
                 (stc.STC_RB_STRING, 'string_style'),
                 (stc.STC_RB_STRING_Q, 'default_style'), # STYLE ME
                 (stc.STC_RB_STRING_QQ, 'default_style'), # STYLE ME
                 (stc.STC_RB_STRING_QR, 'default_style'), # STYLE ME
                 (stc.STC_RB_STRING_QW, 'default_style'), # STYLE ME
                 (stc.STC_RB_STRING_QX, 'default_style'), # STYLE ME
                 (stc.STC_RB_SYMBOL, 'default_style'), # STYLE ME
                 (stc.STC_RB_UPPER_BOUND, 'default_style'), # STYLE ME
                 (stc.STC_RB_WORD, 'keyword_style'),
                 (stc.STC_RB_WORD_DEMOTED, 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
TIMMY = ("fold.timmy.whinge.level", "1")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Ruby""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_RUBY)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [RUBY_KW]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD, TIMMY]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'#']

#-----------------------------------------------------------------------------#

def AutoIndenter(estc, pos, ichar):
    """Auto indent cpp code.
    @param estc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character

    """
    rtxt = u''
    line = estc.GetCurrentLine()
    text = estc.GetTextRange(estc.PositionFromLine(line), pos)
    eolch = estc.GetEOLChar()

    indent = estc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = estc.GetTabWidth()
    else:
        tabw = estc.GetIndent()

    i_space = indent / tabw
    ndent = eolch + ichar * i_space
    rtxt = ndent + ((indent - (tabw * i_space)) * u' ')

    def_pat = re.compile('\s*(class|def)\s+[a-zA-Z_][a-zA-Z0-9_]*')
    text = text.strip()
    if text.endswith('{') or def_pat.match(text):
        rtxt += ichar

    # Put text in the buffer
    estc.AddText(rtxt)

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return RUBY_KW[1]

#---- End Syntax Modules Internal Functions ----#
