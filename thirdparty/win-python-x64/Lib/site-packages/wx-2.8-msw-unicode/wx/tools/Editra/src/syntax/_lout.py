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
__svnid__ = "$Id: _lout.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

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
SYNTAX_ITEMS = [(stc.STC_LOUT_COMMENT, 'comment_style'),
                (stc.STC_LOUT_DEFAULT,'default_style'),
                (stc.STC_LOUT_IDENTIFIER, 'default_style'),
                (stc.STC_LOUT_NUMBER, 'number_style'),
                (stc.STC_LOUT_OPERATOR, 'operator_style'),
                (stc.STC_LOUT_STRING, 'string_style'),
                (stc.STC_LOUT_STRINGEOL, 'stringeol_style'),
                (stc.STC_LOUT_WORD, 'scalar_style'),
                (stc.STC_LOUT_WORD2, 'keyword2_style'),
                (stc.STC_LOUT_WORD3, 'keyword_style'),
                (stc.STC_LOUT_WORD4, 'class_style')]

#---- Extra Properties ----#
FOLD_COMPACT = ("fold.compact", '1')

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for LOUT""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_LOUT)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [LOUT_KW1, LOUT_KW2, LOUT_KW3]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD_COMPACT]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'#']
