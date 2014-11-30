###############################################################################
# Name: vbscript.py                                                           #
# Purpose: Define VBScript syntax for highlighting and other features         #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: vbscript.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for VBScript.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: vbscript.py 56840 2008-11-18 19:32:54Z CJP $"
__revision__ = "$Revision: 56840 $"


#-----------------------------------------------------------------------------#
# Imports
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

VBS_KW = ("addressof alias and as attribute base begin binary boolean byref "
          "byte byval call case cdbl cint clng compare const csng cstr "
          "currency date decimal declare defbool defbyte defcur defdate defdbl "
          "defdec defint deflng defobj defsng defstr defvar dim do double each "
          "else elseif empty end enum eqv erase error event exit explicit "
          "false for friend function get global gosub goto if imp implements "
          "in input integer is len let lib like load lock long loop lset me "
          "mid midb mod new next not nothing null object on option optional "
          "or paramarray preserve print private property public raiseevent "
          "randomize redim rem resume return rset seek select set single "
          "static step stop string sub text then time to true type typeof "
          "unload until variant wend while with withevents xor")

# Syntax specifications
SYNTAX_ITEMS = [ ('STC_B_ASM', 'asm_style'),
                 ('STC_B_BINNUMBER', 'default_style'), # STYLE NEEDED
                 ('STC_B_COMMENT', 'comment_style'),
                 ('STC_B_CONSTANT', 'const_style'),
                 ('STC_B_DATE', 'default_style'), # STYLE NEEDED
                 ('STC_B_DEFAULT', 'default_style'),
                 ('STC_B_ERROR', 'error_style'),
                 ('STC_B_HEXNUMBER', 'number_style'),
                 ('STC_B_IDENTIFIER', 'default_style'),
                 ('STC_B_KEYWORD', 'keyword_style'),
                 ('STC_B_KEYWORD2', 'class_style'),   # STYLE NEEDED
                 ('STC_B_KEYWORD3', 'funct_style'), # STYLE NEEDED
                 ('STC_B_KEYWORD4', 'scalar_style'), # STYLE NEEDED
                 ('STC_B_LABEL', 'directive_style'), # STYLE NEEDED
                 ('STC_B_NUMBER', 'number_style'),
                 ('STC_B_OPERATOR', 'operator_style'),
                 ('STC_B_PREPROCESSOR', 'pre_style'),
                 ('STC_B_STRING', 'string_style'),
                 ('STC_B_STRINGEOL', 'stringeol_style')
               ]

#---- Extra Properties ----#
FOLD = ("fold", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_VBSCRIPT:
        return [(0, VBS_KW),]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_VBSCRIPT:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_VBSCRIPT:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_VBSCRIPT:
        return [u'\'']
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
