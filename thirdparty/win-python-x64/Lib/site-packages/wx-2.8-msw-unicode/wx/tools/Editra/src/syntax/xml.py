###############################################################################
# Name: xml.py                                                                #
# Purpose: Define XML syntax for highlighting and other features              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: xml.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for XML Files.
@todo: Almost Everything

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: xml.py 59543 2009-03-15 00:28:39Z CJP $"
__revision__ = "$Revision: 59543 $"

#-----------------------------------------------------------------------------#
# Dependencies
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Xml Keywords
XML_KEYWORDS = ("rss atom pubDate channel version title link description "
                "language generator item")

# SGML Keywords
import html
SGML_KEYWORDS = html.KeywordString(synglob.ID_LANG_SGML)

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = html.SYNTAX_ITEMS

#---- Extra Properties ----#
# See html.py
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    return [(5, XML_KEYWORDS + u" " + SGML_KEYWORDS)]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return [html.FOLD, html.FLD_HTML]

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'<!--', u'-->']

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
