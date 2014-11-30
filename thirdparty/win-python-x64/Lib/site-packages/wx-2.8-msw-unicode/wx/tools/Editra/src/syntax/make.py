###############################################################################
# Name: make.py                                                               #
# Purpose: Define Makefile syntax for highlighting and other features         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: make.py                                                               
AUTHOR: Cody Precord                                                        
@summary: Lexer configuration module for Makefiles.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: make.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# No keywords

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_MAKE_DEFAULT', 'default_style'),
                 ('STC_MAKE_COMMENT', 'comment_style'),
                 ('STC_MAKE_IDENTIFIER', "scalar_style"),
                 ('STC_MAKE_IDEOL', 'ideol_style'),
                 ('STC_MAKE_OPERATOR', 'operator_style'),
                 ('STC_MAKE_PREPROCESSOR', "pre2_style"),
                 ('STC_MAKE_TARGET', 'keyword_style') ]

#--- Extra Properties ----#
# None
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'#']
#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#

#-----------------------------------------------------------------------------#
