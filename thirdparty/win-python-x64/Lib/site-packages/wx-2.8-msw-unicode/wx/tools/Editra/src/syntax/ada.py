###############################################################################
# Name: ada.py                                                                #
# Purpose: Define Ada syntax for highlighting and other features              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
 FILE: ada.py
 AUTHOR: Cody Precord
 @summary: Lexer configuration module for ada
 @todo: styles, keywords, testing

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ada.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
ADA_KEYWORDS = (0, "abort abstract accept access aliased all array at begin "
                    "body case constant declare delay delta digits do else "
                    "elsif end entry exception exit for function generic goto "
                    "if in is limited loop new null of others out package "
                    "pragma private procedure protected raise range record "
                    "renames requeue return reverse select separate subtype "
                    "tagged task terminate then type until use when while with")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_ADA_CHARACTER', 'char_style'),
                 ('STC_ADA_CHARACTEREOL', 'stringeol_style'),
                 ('STC_ADA_COMMENTLINE', 'comment_style'),
                 ('STC_ADA_DEFAULT', 'default_style'),
                 ('STC_ADA_DELIMITER', 'operator_style'),
                 ('STC_ADA_IDENTIFIER', 'default_style'),
                 ('STC_ADA_ILLEGAL', 'error_style'),
                 ('STC_ADA_LABEL', 'keyword2_style'),   # Style This
                 ('STC_ADA_NUMBER', 'number_style'),
                 ('STC_ADA_STRING', 'string_style'),
                 ('STC_ADA_STRINGEOL', 'stringeol_style'),
                 ('STC_ADA_WORD', 'keyword_style')]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_ADA:
        return [ADA_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_ADA:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_ADA:
        return list()
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_ADA:
        return [ u'--' ]
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    # Unused by this module, stubbed in for consistancy
    return None

#---- End Syntax Modules Internal Functions ----#
