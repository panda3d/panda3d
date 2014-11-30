###############################################################################
# Name: boo.py                                                                #
# Purpose: Define Boo language syntax and other features                      #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: boo.py
@summary: Defines language and syntax highlighting settings for the Boo
          programming language
@todo: support for C style comment regions
"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: boo.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
import python

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
BOO_KW = (0, "abstract and as AST break callable cast char class constructor "
             "continue def destructor do elif else ensure enum event except "
             "failure final false for from get given goto if import in "
             "interface internal is isa not null of or otherwise override "
             "namespace partial pass private protected public raise ref retry "
             "return self set static super struct success transient true try "
             "typeof unless virtual when while yield")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [x for x in python.SYNTAX_ITEMS if x[0] != 'STC_P_DECORATOR']
SYNTAX_ITEMS.append(('STC_P_DECORATOR', 'default_style'))

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_BOO:
        return [BOO_KW]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_BOO:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_BOO:
        return python.Properties(synglob.ID_LANG_PYTHON)
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_BOO:
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
