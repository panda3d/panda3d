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
__svnid__ = "$Id: ruby.py 60049 2009-04-07 04:28:52Z CJP $"
__revision__ = "$Revision: 60049 $"

#-----------------------------------------------------------------------------#
# Imports
import re

# Local Imports
import synglob

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
SYNTAX_ITEMS = [ ('STC_RB_BACKTICKS', 'scalar_style'),
                 ('STC_RB_CHARACTER', 'char_style'),
                 ('STC_RB_CLASSNAME', 'class_style'),
                 ('STC_RB_CLASS_VAR', 'default_style'), # STYLE ME
                 ('STC_RB_COMMENTLINE', 'comment_style'),
                 ('STC_RB_DATASECTION', 'default_style'), # STYLE ME
                 ('STC_RB_DEFAULT', 'default_style'),
                 ('STC_RB_DEFNAME', 'keyword3_style'), # STYLE ME
                 ('STC_RB_ERROR', 'error_style'),
                 ('STC_RB_GLOBAL', 'global_style'),
                 ('STC_RB_HERE_DELIM', 'default_style'), # STYLE ME
                 ('STC_RB_HERE_Q', 'here_style'),
                 ('STC_RB_HERE_QQ', 'here_style'),
                 ('STC_RB_HERE_QX', 'here_style'),
                 ('STC_RB_IDENTIFIER', 'default_style'),
                 ('STC_RB_INSTANCE_VAR', 'scalar2_style'),
                 ('STC_RB_MODULE_NAME', 'global_style'), # STYLE ME
                 ('STC_RB_NUMBER', 'number_style'),
                 ('STC_RB_OPERATOR', 'operator_style'),
                 ('STC_RB_POD', 'default_style'), # STYLE ME
                 ('STC_RB_REGEX', 'regex_style'), # STYLE ME
                 ('STC_RB_STDIN', 'default_style'), # STYLE ME
                 ('STC_RB_STDOUT', 'default_style'), # STYLE ME
                 ('STC_RB_STRING', 'string_style'),
                 ('STC_RB_STRING_Q', 'default_style'), # STYLE ME
                 ('STC_RB_STRING_QQ', 'default_style'), # STYLE ME
                 ('STC_RB_STRING_QR', 'default_style'), # STYLE ME
                 ('STC_RB_STRING_QW', 'default_style'), # STYLE ME
                 ('STC_RB_STRING_QX', 'default_style'), # STYLE ME
                 ('STC_RB_SYMBOL', 'default_style'), # STYLE ME
                 ('STC_RB_UPPER_BOUND', 'default_style'), # STYLE ME
                 ('STC_RB_WORD', 'keyword_style'),
                 ('STC_RB_WORD_DEMOTED', 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
TIMMY = ("fold.timmy.whinge.level", "1")
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_RUBY:
        return [RUBY_KW]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_RUBY:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_RUBY:
        return [FOLD, TIMMY]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_RUBY:
        return [u'#']
    else:
        return list()

#---- End Required Module Functions ----#

def AutoIndenter(stc, pos, ichar):
    """Auto indent cpp code. uses \n the text buffer will handle any
    eol character formatting.

    @param stc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character
    @return: string

    """
    rtxt = u''
    line = stc.GetCurrentLine()
    text = stc.GetTextRange(stc.PositionFromLine(line), pos)

    indent = stc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = stc.GetTabWidth()
    else:
        tabw = stc.GetIndent()

    i_space = indent / tabw
    ndent = u"\n" + ichar * i_space
    rtxt = ndent + ((indent - (tabw * i_space)) * u' ')

    def_pat = re.compile('\s*(class|def)\s+[a-zA-Z_][a-zA-Z0-9_]*')
    text = text.strip()
    if text.endswith('{') or def_pat.match(text):
        rtxt += ichar

    return rtxt

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return RUBY_KW[1]

#---- End Syntax Modules Internal Functions ----#
