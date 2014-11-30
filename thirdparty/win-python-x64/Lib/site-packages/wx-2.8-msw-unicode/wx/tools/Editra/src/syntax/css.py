###############################################################################
# Name: css.py                                                                #
# Purpose: Define CSS syntax for highlighting and other features              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: css.py
@author: Cody Precord
@summary: Lexer configuration file for Cascading Style Sheets.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: css.py 55178 2008-08-22 15:39:56Z CJP $"
__revision__ = "$Revision: 55178 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# CSS1 Keywords (Idenifiers)
CSS1_KEYWORDS = (0, "font-family font-style font-variant font-weight font-size "
                    "font color background-color background-image "
                    "background-repeat background-position background "
                    "word-spacing letter-spacing text-decoration "
                    "vertical-align text-transform text-align text-indent "
                    "line-height margin-top margin-right margin-left margin "
                    "padding-top padding-right padding-bottom padding-left "
                    "padding border-top-width border-right-width "
                    "border-bottom-width border-left-width border-width "
                    "border-color border-style border-top border-right "
                    "border-bottom border-left border width height float clear "
                    "display white-space list-style-type list-style-image "
                    "list-style-position list-style margin-bottom "
                    "text-decoration min-width min-height")

# CSS Psuedo Classes
CSS_PSUEDO_CLASS = (1, "link visited active hover focus before after left "
                       "right lang first-letter first-line first-child")

# CSS2 Keywords (Identifers2)
# This is meant for css2 specific keywords, but in order to get a better
# coloring effect this will contain special css properties as well.
CSS2_KEYWORDS = (2, "src stemv stemh slope ascent descent widths bbox baseline "
                    "centerline mathline topline all aqua black blue fuchsia "
                    "gray green lime maroon navy olive purple red silver teal "
                    "yellow ActiveBorder ActiveCaption AppWorkspace ButtonFace "
                    "ButtonHighlight ButtonShadow ButtonText CaptionText "
                    "GrayText Highlight HighlightText InactiveBorder "
                    "InactiveCaption InactiveCaptionText InfoBackground "
                    "InfoText Menu MenuText Scrollbar ThreeDDarkShadow "
                    "ThreeDFace ThreeDHighlight ThreeDLightShadow ThreeDShadow "
                    "Window WindowFrame WindowText Background auto none "
                    "inherit top bottom medium normal cursive fantasy "
                    "monospace italic oblique bold bolder lighter larger "
                    "smaller icon menu narrower wider color center scroll "
                    "fixed underline overline blink sub super middle "
                    "capitalize uppercase lowercase center justify baseline "
                    "width height float clear overflow clip visibility thin "
                    "thick both dotted dashed solid double groove ridge inset "
                    "outset hidden visible scroll collapse content quotes disc "
                    "circle square hebrew armenian georgian inside outside "
                    "size marks inside orphans widows landscape portrait crop "
                    "cross always avoid cursor default crosshair pointer move "
                    "wait help invert position below level above higher block "
                    "inline compact static relative absolute fixed ltr rtl "
                    "embed bidi-override pre nowrap volume during azimuth "
                    "elevation stress richness silent non mix leftwards "
                    "rightwards behind faster slower male female child code "
                    "digits continuous separate show hide once ")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_CSS_DEFAULT', 'default_style'),
                 ('STC_CSS_ATTRIBUTE', 'funct_style'),
                 ('STC_CSS_CLASS', 'global_style'),
                 ('STC_CSS_COMMENT', 'comment_style'),
                 ('STC_CSS_DIRECTIVE', 'directive_style'),
                 ('STC_CSS_DOUBLESTRING', 'string_style'),
                 ('STC_CSS_ID', 'scalar_style'),
                 ('STC_CSS_IDENTIFIER', 'keyword_style'),
                 ('STC_CSS_IDENTIFIER2', 'keyword3_style'),
                 ('STC_CSS_IMPORTANT', 'error_style'),
                 ('STC_CSS_OPERATOR', 'operator_style'),
                 ('STC_CSS_PSEUDOCLASS', 'scalar_style'),
                 ('STC_CSS_SINGLESTRING', 'string_style'),
                 ('STC_CSS_TAG', 'keyword_style'),
                 ('STC_CSS_UNKNOWN_IDENTIFIER', 'unknown_style'),
                 ('STC_CSS_UNKNOWN_PSEUDOCLASS', 'unknown_style'),
                 ('STC_CSS_VALUE', 'char_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_CSS:
        return [CSS1_KEYWORDS , CSS_PSUEDO_CLASS, CSS2_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_CSS:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_CSS:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_CSS:
        return [u'/*', u'*/']
    else:
        return list()
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

    if text.endswith('{'):
        rtxt += ichar

    return rtxt

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
