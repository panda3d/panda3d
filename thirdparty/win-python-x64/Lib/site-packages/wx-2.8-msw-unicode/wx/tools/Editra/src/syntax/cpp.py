###############################################################################
# Name: cpp.py                                                                #
# Purpose: Define C/CPP/ObjC/Vala syntax for highlighting and other features  #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: cpp.py                                                                
@author: Cody Precord                                                       
@summary: Lexer configuration file for C/C++/C#/Objective C/Vala source files.
                                                                         
"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: cpp.py 58027 2009-01-11 17:58:04Z CJP $"
__revision__ = "$Revision: 58027 $"

#-----------------------------------------------------------------------------#
# Imports
import re

# Local imports
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# C Keywords
C_KEYWORDS = ("asm break case const continue default do else for goto return "
              "if sizeof static switch typeof while")

# C Types/Structures/Storage Classes
C_TYPES = ("auto bool char clock_t complex div_t double enum extern float "
           "fpos_t inline int int_least8_t int_least16_t int_least32_t "
           "int_least64_t int8_t int16_t int32_t int64_t intmax_t intptr_t "
           "jmp_buf ldiv_t long mbstate_t ptrdiff_t register sig_atomic_t "
           "size_t ssize_t short signed struct typedef union time_t "
           "uint_fast8_t uint_fast16_t uint_fast32_t uint_fast64_t uint8_t "
           "uint16_t uint32_t uint64_t uintptr_t uintmax_t unsigned va_list "
           "void volatile wchar_t wctrans_t wctype_t wint_t FILE DIR __label__ "
           "__complex__ __volatile__ __attribute__")

# C/CPP Documentation Keywords (includes Doxygen keywords)
DOC_KEYWORDS = (2, "TODO FIXME XXX author brief bug callgraph category class "
                   "code date def depreciated dir dot dotfile else elseif em "
                   "endcode enddot endif endverbatim example exception file if "
                   "ifnot image include link mainpage name namespace page par "
                   "paragraph param pre post return retval section struct "
                   "subpage subsection subsubsection test todo typedef union "
                   "var verbatim version warning $ @ ~ < > # % HACK")

# CPP Keyword Extensions
CPP_KEYWORDS = ("and and_eq bitand bitor catch class compl const_cast delete "
                "dynamic_cast false friend new not not_eq operator or or_eq "
                "private protected public reinterpret_cast static_cast this "
                "throw try true typeid using xor xor_eq")

# CPP Type/Structure/Storage Class Extensions
CPP_TYPES = ("bool inline explicit export mutable namespace template typename "
             "virtual wchar_t")

# C# Keywords
CSHARP_KW = ("abstract as base break case catch checked class const continue  "
             "default delegate do else event explicit extern false finally "
             "fixed for foreach goto if implicit in interface internal is lock "
             "new null operator out override params readonly ref return sealed "
             "sizeof stackalloc static switch this throw true try typeof "
             "unchecked unsafe using while")

# C# Types
CSHARP_TYPES = ("bool byte char decimal double enum float int long "
                "namespace object private protected public sbyte short string "
                "struct uint ulong ushort virtual void volatile")

# Objective C
OBJC_KEYWORDS = ("@catch @interface @implementation @end @finally @private "
                 "@protected @protocol @public @throw @try self super false "
                 "true")

OBJC_TYPES = ("id")

# Vala Keywords
VALA_KEYWORDS = ("abstract as base break case catch checked construct continue "
                 "default delegate do else event false finally for foreach get "
                 "goto if implicit interface internal is lock new operator out "
                 "override params readonly ref return sealed set sizeof "
                 "stackalloc this throw true try typeof unchecked using while")

VALA_TYPES = ("bool byte char class const decimal double enum explicit extern "
              "fixed float int long namespace private protected public sbyte "
              "short static string struct uint ulong unichar unsafe ushort var "
              "volatile void virtual")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_C_DEFAULT', 'default_style'),
                 ('STC_C_COMMENT', 'comment_style'),
                 ('STC_C_COMMENTLINE', 'comment_style'),
                 ('STC_C_COMMENTDOC', 'comment_style'),
                 ('STC_C_COMMENTDOCKEYWORD', 'dockey_style'),
                 ('STC_C_COMMENTDOCKEYWORDERROR', 'error_style'),
                 ('STC_C_COMMENTLINE', 'comment_style'),
                 ('STC_C_COMMENTLINEDOC', 'comment_style'),
                 ('STC_C_CHARACTER', 'char_style'),
                 ('STC_C_GLOBALCLASS', 'global_style'),
                 ('STC_C_IDENTIFIER', 'default_style'),
                 ('STC_C_NUMBER', 'number_style'),
                 ('STC_C_OPERATOR', 'operator_style'),
                 ('STC_C_PREPROCESSOR', 'pre_style'),
                 ('STC_C_REGEX', 'pre_style'),
                 ('STC_C_STRING', 'string_style'),
                 ('STC_C_STRINGEOL', 'stringeol_style'),
                 ('STC_C_UUID', 'pre_style'),
                 ('STC_C_VERBATIM', 'number2_style'),
                 ('STC_C_WORD', 'keyword_style'),
                 ('STC_C_WORD2', 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")
ALLOW_DOLLARS = ("lexer.cpp.allow.dollars", "1")

#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    keywords = list()
    kw1_str = [C_KEYWORDS]
    kw2_str = [C_TYPES]
    if lang_id == synglob.ID_LANG_CPP:
        kw1_str.append(CPP_KEYWORDS)
        kw2_str.append(CPP_TYPES)
    elif lang_id == synglob.ID_LANG_CSHARP:
        kw1_str = [CSHARP_KW]
        kw2_str = [CSHARP_TYPES]
    elif lang_id == synglob.ID_LANG_OBJC:
        kw1_str.append(OBJC_KEYWORDS)
        kw2_str.append(OBJC_TYPES)
    elif lang_id == synglob.ID_LANG_VALA:
        kw1_str = [VALA_KEYWORDS]
        kw2_str = [VALA_TYPES]
    else:
        pass

    keywords.append((0, " ".join(kw1_str)))
    keywords.append((1, " ".join(kw2_str)))
    keywords.append(DOC_KEYWORDS)
    return keywords

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id in [ synglob.ID_LANG_C,
                    synglob.ID_LANG_CPP,
                    synglob.ID_LANG_CSHARP,
                    synglob.ID_LANG_OBJC,
                    synglob.ID_LANG_VALA ]:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id in [ synglob.ID_LANG_C,
                    synglob.ID_LANG_CPP,
                    synglob.ID_LANG_CSHARP,
                    synglob.ID_LANG_OBJC,
                    synglob.ID_LANG_VALA ]:
        return [FOLD, FOLD_PRE, FOLD_COM]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id in [ synglob.ID_LANG_CPP,
                    synglob.ID_LANG_CSHARP,
                    synglob.ID_LANG_OBJC,
                    synglob.ID_LANG_VALA ]:
        return [u'//']
    else:
        return [u'/*', u'*/']

#---- End Required Functions ----#

def AutoIndenter(stc, pos, ichar):
    """Auto indent cpp code. uses \n the text buffer will
    handle any eol character formatting.
    @param stc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character
    @return: string

    """
    rtxt = u''
    line = stc.GetCurrentLine()
    text = stc.GetTextRange(stc.PositionFromLine(line), pos)

    indent = stc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = stc.GetTabWidth()
    else:
        tabw = stc.GetIndent()

    i_space = indent / tabw
    ndent = u"\n" + ichar * i_space
    rtxt = ndent + ((indent - (tabw * i_space)) * u' ')

    cdef_pat = re.compile('(public|private|protected)\s*\:')
    case_pat = re.compile('(case\s+.+|default)\:')
    text = text.strip()
    if text.endswith('{') or cdef_pat.match(text) or case_pat.match(text):
        rtxt += ichar

    return rtxt

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
