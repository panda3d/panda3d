###############################################################################
# Name: vhdl.py                                                               #
# Purpose: Define VHDL syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: vhdl.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for VHDL. Very High Scale Integrated
          Circuit Hardware Description Language
@todo: Maybe add highlighting for values S0S, S1S, ect..

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: vhdl.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# VHDL Keywords
VHDL_KW = (0, "access after alias all assert architecture array attribute for "
              "begin block body buffer bus case component configuration inout "
              "constant if else disconnect downto elsif end entity exit file "
              "function generate generic group guarded impure in inertial is "
              "label library linkage literal loop map new next null of on open "
              "others out package port postponed procedure process pure range "
              "record register reject report return select severity signal use "
              "shared subtype then to transport type unaffected units until "
              "variable wait when while with note warning error failure true "
              "false")

# VHDL Operators
VHDL_OP = (1, "and nand or nor xor xnor rol ror sla sll sra srl mod rem abs "
              "not ")

# VHDL Attributes
VHDL_AT = (2, "'high 'left 'length 'low 'range 'reverse_range 'right 'foreign "
              "'ascending 'behavior 'structure 'simple_name 'instance_name "
              "'path_name 'active 'delayed 'event 'last_active 'last_event "
              "'last_value 'quiet 'stable 'transaction 'driving 'driving_value "
              "'base 'high 'left 'leftof 'low 'pos 'pred 'rightof 'succ 'val "
              "'image 'value")

# Standard Functions
VHDL_STDF = (3, "now readline read writeline write endfile to_stdulogicvector "
                "to_bitvector to_stdulogic to_stdlogicvector resolved to_bit  "
                "to_x01 to_x01z to_UX01 rising_edge falling_edge is_x "
                "shift_right rotate_left rotate_right resize to_integer "
                "to_unsigned to_signed std_match to_01 shift_left ")

# Standard Packages
VHDL_STDP = (4, "std ieee work standard textio std_logic_1164 std_logic_arith "
                "std_logic_misc std_logic_signed std_logic_textio "
                "numeric_bit numeric_std math_complex math_real "
                "vital_timing std_logic_unsigned vital_primitives ")

# Standard Types
VHDL_STDT = (5, "bit bit_vector character boolean integer real time string "
                "severity_level positive natural signed unsigned line text "
                "std_logic std_logic_vector std_ulogic std_ulogic_vector "
                "qsim_state qsim_state_vector qsim_12state qsim_12state_vector "
                "qsim_strength mux_bit mux_vector reg_bit reg_vector wor_bit "
                "wor_vector")

# User Words
VHDL_UKW = (6, "")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_VHDL_DEFAULT', 'default_style'),
                 ('STC_VHDL_STRINGEOL', 'stringeol_style'),
                 ('STC_VHDL_COMMENT', 'comment_style'),
                 ('STC_VHDL_COMMENTLINEBANG', 'comment_style'),
                 ('STC_VHDL_IDENTIFIER', 'default_style'),
                 ('STC_VHDL_KEYWORD', 'keyword_style'),
                 ('STC_VHDL_NUMBER', 'default_style'),
                 ('STC_VHDL_OPERATOR', 'operator_style'),
                 ('STC_VHDL_STDFUNCTION', 'funct_style'),
                 ('STC_VHDL_STDOPERATOR', 'operator_style'),
                 ('STC_VHDL_STDPACKAGE', 'pre_style'),
                 ('STC_VHDL_STDTYPE', 'class_style'),
                 ('STC_VHDL_STRING', 'string_style'),
                 ('STC_VHDL_STRINGEOL', 'stringeol_style'),
                 ('STC_VHDL_USERWORD', 'default_style') ]

#---- Extra Property Specifications ----#
FOLD = ("fold", "1")
FLD_COMMENT = ("fold.comment", "1")
FLD_COMMPACT = ("fold.compact", "1")
FLD_ATELSE  = ("fold.at.else", "1")
FLD_ATBEGIN = ("fold.at.Begin", "1")
FLD_ATPAREN = ("fold.at.Parenthese", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    return [VHDL_KW, VHDL_AT, VHDL_STDF, VHDL_STDP, VHDL_STDT]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return [FOLD]

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'--']
#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
