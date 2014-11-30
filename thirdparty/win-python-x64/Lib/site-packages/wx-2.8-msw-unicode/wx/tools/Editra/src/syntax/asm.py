###############################################################################
# Name: asm.py                                                                #
# Purpose: Define ASM syntax for highlighting and other features              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: asm.py
AUTHOR: Cody Precord
@summary: Lexer configuration file GNU Assembly Code
@todo: Complete Keywords/Registers

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: asm.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

# GNU Assembly CPU Instructions/Storage Types
ASM_CPU_INST = (0, ".long .ascii .asciz .byte .double .float .hword .int .octa "
                   ".quad .short .single .space .string .word")

# GNU FPU Instructions
ASM_MATH_INST = (1, "")

# GNU Registers
ASM_REGISTER = (2, "")

# GNU Assembly Directives/Special statements/Macros
ASM_DIRECTIVES = (3, ".include .macro .endm")

#---- Language Styling Specs ----#
SYNTAX_ITEMS = [ ('STC_ASM_DEFAULT', 'default_style'),
                 ('STC_ASM_CHARACTER', 'char_style'),
                 ('STC_ASM_COMMENT', 'comment_style'),
                 ('STC_ASM_COMMENTBLOCK', 'comment_style'),
                 ('STC_ASM_CPUINSTRUCTION', 'keyword_style'),
                 ('STC_ASM_DIRECTIVE', 'keyword3_style'),
                 ('STC_ASM_DIRECTIVEOPERAND', 'default_style'),
                 ('STC_ASM_EXTINSTRUCTION', 'default_style'),
                 ('STC_ASM_IDENTIFIER', 'default_style'),
                 ('STC_ASM_MATHINSTRUCTION', 'keyword_style'),
                 ('STC_ASM_NUMBER', 'number_style'),
                 ('STC_ASM_OPERATOR', 'operator_style'),
                 ('STC_ASM_REGISTER', 'keyword2_style'),
                 ('STC_ASM_STRING', 'string_style'),
                 ('STC_ASM_STRINGEOL', 'stringeol_style') ]
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns List of Keyword Specifications
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_ASM:
        return [ASM_CPU_INST, ASM_DIRECTIVES]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_ASM:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_ASM:
        return list()
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_ASM:
        return [u';']
    else:
        return list()

#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
