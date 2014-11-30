###############################################################################
# Name: yaml.py                                                               #
# Purpose: Define YAML syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: yaml.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for YAML
@todo: Maybe new custom style for text regions

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: yaml.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
YAML_KW = [(0, "true false yes no")]

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_YAML_COMMENT', 'comment_style'),
                ('STC_YAML_DEFAULT', 'default_style'),
                ('STC_YAML_DOCUMENT', 'scalar_style'),
                ('STC_YAML_ERROR', 'error_style'),
                ('STC_YAML_IDENTIFIER', 'keyword2_style'),
                ('STC_YAML_KEYWORD', 'keyword_style'),
                ('STC_YAML_NUMBER', 'number_style'),
                ('STC_YAML_REFERENCE', 'global_style'),
                ('STC_YAML_TEXT', 'default_style')] # Different style maybe

#---- Extra Properties ----#
FOLD_COMMENT = ("fold.comment.yaml", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_YAML:
        return YAML_KW
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_YAML:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_YAML:
        return [FOLD_COMMENT]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_YAML:
        return [u'#']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
