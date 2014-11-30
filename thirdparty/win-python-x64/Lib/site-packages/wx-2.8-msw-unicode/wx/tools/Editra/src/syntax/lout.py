###############################################################################
# Name: lout.py                                                               #
# Purpose: Define Lout syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: lout.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Lout
@todo: style refinement

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: lout.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
# @ prefixed keywords
LOUT_KW1 = (0, "@OptGall @FontDef @Family @Face @Name @Metrics @ExtraMetrics "
               "@Mapping @Recode @Filter @FilterIn @FilterOut @FilterErr @AL "
               "@Common @Rump @Meld @Insert @OneOf @Next @Plus @Minus @Wide "
               "@High @HShift @VShift @BeginHeaderComponent @Document @TItle "
               "@SetHeaderComponent @ClearHeaderComponent @OneCol @OneRow @Doc "
               "@HMirror @VMirror @HScale @VScale @HCover @VCover @Scale @Text "
               "@KernShrink @HContract @VContract @HLimited @VLimited @HExpand "
               "@VExpand @StartHVSpan @StartHSpan @StartVSpan @HSpan @VSpan "
               "@PAdjust @HAdjust @VAdjust @Rotate @Background @IncludeGraphic "
               "@SysIncludeGraphic @Graphic @LinkSource @LinkDest @URLLink @BI "
               "@PlainGraphic @Verbatim @RawVerbatim @Case @Yield @BackEnd @BL "
               "@Char @Font @Space @YUnit @ZUnit @Break @Underline @SetColour "
               "@SetColor @SetUnderlineColour @SetUnderlineColor @SetTexture "
               "@Outline @Language @CurrLang @CurrFamily @CurrFace @CurrYUnit "
               "@CurrZUnit @LEnv @@A @@B @@C @@D @@E @LClos @@V @LUse @LEO @PP "
               "@Open @Use @NotRevealed @Tagged @Database @SysDatabase @I @B"
               "@Include @SysInclude @IncludeGraphicRepeated @InitialFont "
               "@SysIncludeGraphicRepeated @PrependGraphic @SysPrependGraphic "
               "@Target @Null @PageLabel @Galley @ForceGalley @LInput @Split "
               "@Tag @Key @Optimize @Merge @Enclose @Begin @End @Moment @Tab "
               "@Second @Minute @Hour @Day @Month @Year @Century @WeekDay "
               "@YearDay @DaylightSaving @SetContext @GetContext @Time @List "
               "@EndHeaderComponent @Section @BeginSections @EndNote @Abstract "
               "@AlphaList @Appendix @Author @Figure @Report @OuterNote "
               "@IndentedList @InitialBreak @InitialLanguage InnerNote "
               "@Heading @FootNote @Date @LeftList @LeftNote @ListItem "
               "@RightDisplay @RightNote @EndSections")

# Symbols
LOUT_KW2 = (1, "&&& && & ^// ^/ ^|| ^| ^& // / || |")

# Non @ keywords
LOUT_KW3 = (2, "def langdef force horizontally into extend import export "
               "precedence associativity left right body macro named "
               "compulsory following preceding foll_or_prec now "
               "Base Slope Bold BoldSlope Times Helvetica Courier Palatino "
               "adjust breakstyle clines lines linesep hyphen nonhyphen ragged "
               )

# Document Classes
LOUT_KW4 = (3, "fx vx aformat bformat doc eq graph slides tab text tbl")
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_LOUT_COMMENT', 'comment_style'),
                ('STC_LOUT_DEFAULT','default_style'),
                ('STC_LOUT_IDENTIFIER', 'default_style'),
                ('STC_LOUT_NUMBER', 'number_style'),
                ('STC_LOUT_OPERATOR', 'operator_style'),
                ('STC_LOUT_STRING', 'string_style'),
                ('STC_LOUT_STRINGEOL', 'stringeol_style'),
                ('STC_LOUT_WORD', 'scalar_style'),
                ('STC_LOUT_WORD2', 'keyword2_style'),
                ('STC_LOUT_WORD3', 'keyword_style'),
                ('STC_LOUT_WORD4', 'class_style')]

#---- Extra Properties ----#
FOLD_COMPACT = ("fold.compact", '1')

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_LOUT:
        return [LOUT_KW1, LOUT_KW2, LOUT_KW3]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_LOUT:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_LOUT:
        return [FOLD_COMPACT]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_LOUT:
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
