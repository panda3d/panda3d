###############################################################################
# Name: matlab.py                                                             #
# Purpose: Define Matlab and Octave syntax for highlighting and other features#
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: matlab.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Matlab and Octave

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: matlab.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
MATLAB_KW = (0, "break case catch continue else elseif end for function "
                "global if otherwise persistent return switch try while")

OCTAVE_KW = (0, "break case catch continue do else elseif end "
                "end_unwind_protect endfor endif endswitch endwhile for "
                "function endfunction global if otherwise persistent return "
                "switch try until unwind_protect unwind_protect_cleanup while")
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_MATLAB_COMMAND', 'funct_style'),
                ('STC_MATLAB_COMMENT', 'comment_style'),
                ('STC_MATLAB_DEFAULT', 'default_style'),
                ('STC_MATLAB_DOUBLEQUOTESTRING', 'string_style'),
                ('STC_MATLAB_IDENTIFIER', 'default_style'),
                ('STC_MATLAB_KEYWORD', 'keyword_style'),
                ('STC_MATLAB_NUMBER', 'number_style'),
                ('STC_MATLAB_OPERATOR', 'operator_style'),
                ('STC_MATLAB_STRING', 'string_style')]

#---- Extra Properties ----#
FOLD = ('fold', '1')
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_MATLAB:
        return [MATLAB_KW]
    elif lang_id == synglob.ID_LANG_OCTAVE:
        return [OCTAVE_KW]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id in [synglob.ID_LANG_MATLAB, synglob.ID_LANG_OCTAVE]:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id in [synglob.ID_LANG_MATLAB, synglob.ID_LANG_OCTAVE]:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_MATLAB:
        return [u'%']
    elif lang_id == synglob.ID_LANG_OCTAVE:
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
