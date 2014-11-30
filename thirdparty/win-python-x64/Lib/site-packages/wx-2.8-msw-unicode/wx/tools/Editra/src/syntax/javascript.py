###############################################################################
# Name: javascript.py                                                         #
# Purpose: Define JavaScript syntax for highlighting and other features       #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: javascript.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for JavaScript.
@todo: Having trouble with getting html embeded js to highlight

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: javascript.py 59542 2009-03-15 00:23:37Z CJP $"
__revision__ = "$Revision: 59542 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
import cpp

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# JavaScript Keywords # set to 1 for embeded
JS_KEYWORDS = (0, "abstract break boolean byte case const continue catch "
                  "class char debugger default delete do double default "
                  "export false else enum export extend final finally "
                  "float for function goto if implements import in " 
                  "instanceof int interface long native new null "
                  "package private protected public return short static "
                  "synchronized switch super this throw throws transient "
                  "try true typeof var void volatile with while")

#---- Syntax Style Spec ----#
SYNTAX_ITEMS = [ ('STC_HJ_COMMENT', 'comment_style'),
                 ('STC_HJ_COMMENTDOC', 'dockey_style'),
                 ('STC_HJ_COMMENTLINE', 'comment_style'),
                 ('STC_HJ_DEFAULT', 'default_style'),
                 ('STC_HJ_DOUBLESTRING', 'string_style'),
                 ('STC_HJ_KEYWORD', 'keyword_style'),
                 ('STC_HJ_NUMBER', 'number_style'),
                 ('STC_HJ_REGEX', 'scalar_style'), # STYLE ME
                 ('STC_HJ_SINGLESTRING', 'string_style'),
                 ('STC_HJ_START', 'scalar_style'),
                 ('STC_HJ_STRINGEOL', 'stringeol_style'),
                 ('STC_HJ_SYMBOLS', 'array_style'),
                 ('STC_HJ_WORD', 'class_style'),
                 ('STC_HJA_COMMENT', 'comment_style'),
                 ('STC_HJA_COMMENTDOC', 'dockey_style'),
                 ('STC_HJA_COMMENTLINE', 'comment_style'),
                 ('STC_HJA_DEFAULT', 'default_style'),
                 ('STC_HJA_DOUBLESTRING', 'string_style'),
                 ('STC_HJA_KEYWORD', 'keyword_style'),
                 ('STC_HJA_NUMBER', 'number_style'),
                 ('STC_HJA_REGEX', 'scalar_style'), # STYLE ME
                 ('STC_HJA_SINGLESTRING', 'string_style'),
                 ('STC_HJA_START', 'scalar_style'),
                 ('STC_HJA_STRINGEOL', 'stringeol_style'),
                 ('STC_HJA_SYMBOLS', 'array_style'),
                 ('STC_HJA_WORD', 'class_style') ]

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_JS:
        return [JS_KEYWORDS,]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_HTML:
        return SYNTAX_ITEMS
    else:
        return cpp.SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_JS:
        return [("fold", "1")]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_JS:
        return [u'//']
    else:
        return list()

#---- End Required Module Functions ----#

AutoIndenter = cpp.AutoIndenter

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @param option: specific subset of keywords to get

    """
    return JS_KEYWORDS[1]

#---- End Syntax Modules Internal Functions ----#

#-----------------------------------------------------------------------------#
