###############################################################################
# Name: latex.py                                                              #
# Purpose: Define TeX/LateX syntax for highlighting and other features        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: latex.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Tex/LaTex.
@todo: Fairly poor needs lots of work.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: latex.py 58334 2009-01-23 16:32:44Z CJP $"
__revision__ = "$Revision: 58334 $"

#-----------------------------------------------------------------------------#
# Dependancies
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Tex Keywords
TEX_KW = (0, "Downarrow backslash lceil rceil Uparrow downarrow lfloor rfloor "
             "Updownarrow langle rangle Vert")
# ConTeXt Dutch
DUTCH = (1, "")
# ConTeXt English
ENG = (2, "")
# ConTeXt German
GERMAN = (3, "")
# ConTeXt Czech
CZECH = (4, "")
# ConTeXt Italian
ITALIAN = (5, "")
# ConTeXt Romanian
ROMAINIAN = (6, "")

# LaTeXt
# There are no keyword settings available for LaTeX

#---- Syntax Style Specs ----#
# TeX
SYNTAX_ITEMS1 = [ ('STC_TEX_DEFAULT', 'default_style'),
                 ('STC_TEX_COMMAND', 'keyword_style'),
                 ('STC_TEX_GROUP', 'scalar_style'),
                 ('STC_TEX_SPECIAL', 'operator_style'),
                 ('STC_TEX_SYMBOL', 'number_style'),
                 ('STC_TEX_TEXT', 'default_style') ]

# LaTeX
SYNTAX_ITEMS2 = [ ('STC_L_DEFAULT', 'default_style'),
                 ('STC_L_COMMAND', 'pre_style'),
                 ('STC_L_COMMENT', 'comment_style'),
                 ('STC_L_MATH', 'operator_style'),
                 ('STC_L_TAG', 'keyword_style')]

#---- Extra Properties ----#
# None
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id in [synglob.ID_LANG_LATEX, synglob.ID_LANG_TEX]:
        return [TEX_KW]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id in [synglob.ID_LANG_LATEX, synglob.ID_LANG_TEX]:
        return SYNTAX_ITEMS1
    else:
        return SYNTAX_ITEMS2

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id in [synglob.ID_LANG_LATEX, synglob.ID_LANG_TEX]:
        return list()
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id in [synglob.ID_LANG_LATEX, synglob.ID_LANG_TEX]:
        return [u'%']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
