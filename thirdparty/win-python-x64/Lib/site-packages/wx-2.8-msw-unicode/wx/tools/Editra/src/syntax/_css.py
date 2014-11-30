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
__svnid__ = "$Id: _css.py 66902 2011-02-16 14:04:01Z CJP $"
__revision__ = "$Revision: 66902 $"

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.stc as stc

# Local Imports
import synglob
import syndata

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
                    "text-decoration min-width min-height "
                    "background-attachment")

# CSS Psuedo Classes
CSS_PSUEDO_CLASS = (1, "link visited active hover focus before after left "
                       "right lang first-letter first-line first-child")

# CSS2 Keywords (Identifers2)
# This is meant for css2 specific keywords, but in order to get a better
# coloring effect this will contain special css properties as well.
CSS2_KEYWORDS = (2, "ActiveBorder ActiveCaption AppWorkspace Background "
                    "ButtonFace ButtonHighlight ButtonShadow ButtonText "
                    "CaptionText GrayText Highlight HighlightText "
                    "InactiveBorder InactiveCaption InactiveCaptionText "
                    "InfoBackground InfoText Menu MenuText Scrollbar "
                    "ThreeDDarkShadow ThreeDFace ThreeDHighlight "
                    "ThreeDLightShadow ThreeDShadow Window WindowFrame "
                    "WindowText above absolute all always aqua armenian ascent "
                    "auto avoid azimuth baseline baseline bbox behind below "
                    "bidi-override black blink block blue bold bolder both "
                    "bottom capitalize center center centerline child circle "
                    "clear clip code collapse color compact content continuous "
                    "crop cross crosshair cursive cursor dashed default "
                    "descent digits disc dotted double during elevation embed "
                    "fantasy faster female fixed fixed float fuchsia georgian "
                    "gray green groove hebrew height help hidden hide higher "
                    "icon inherit inline inset inside inside invert italic "
                    "justify landscape larger leftwards level lighter lime "
                    "lowercase ltr male marks maroon mathline medium menu "
                    "middle mix monospace move narrower navy non none normal "
                    "nowrap oblique olive once orphans outset outside overflow "
                    "overline pointer portrait position pre purple quotes red "
                    "relative richness ridge rightwards rtl scroll scroll "
                    "separate show silent silver size slope slower smaller "
                    "solid square src static stemh stemv stress sub super teal "
                    "thick thin top topline underline uppercase visibility "
                    "visible volume wait wider widows width widths yellow "
                    "z-index outline left")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_CSS_DEFAULT, 'default_style'),
                 (stc.STC_CSS_ATTRIBUTE, 'funct_style'),
                 (stc.STC_CSS_CLASS, 'global_style'),
                 (stc.STC_CSS_COMMENT, 'comment_style'),
                 (stc.STC_CSS_DIRECTIVE, 'directive_style'),
                 (stc.STC_CSS_DOUBLESTRING, 'string_style'),
                 (stc.STC_CSS_ID, 'scalar_style'),
                 (stc.STC_CSS_IDENTIFIER, 'keyword_style'),
                 (stc.STC_CSS_IDENTIFIER2, 'keyword3_style'),
                 (stc.STC_CSS_IMPORTANT, 'error_style'),
                 (stc.STC_CSS_OPERATOR, 'operator_style'),
                 (stc.STC_CSS_PSEUDOCLASS, 'scalar_style'),
                 (stc.STC_CSS_SINGLESTRING, 'string_style'),
                 (stc.STC_CSS_TAG, 'keyword_style'),
                 (stc.STC_CSS_UNKNOWN_IDENTIFIER, 'unknown_style'),
                 (stc.STC_CSS_UNKNOWN_PSEUDOCLASS, 'unknown_style'),
                 (stc.STC_CSS_VALUE, 'char_style') ]

# TODO: add styling and keywords for new style regions in 2.9
if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS.append((stc.STC_CSS_EXTENDED_IDENTIFIER, 'default_style'))
    SYNTAX_ITEMS.append((stc.STC_CSS_EXTENDED_PSEUDOCLASS, 'default_style'))
    SYNTAX_ITEMS.append((stc.STC_CSS_EXTENDED_PSEUDOELEMENT, 'default_style'))
    SYNTAX_ITEMS.append((stc.STC_CSS_IDENTIFIER3, 'default_style'))
    SYNTAX_ITEMS.append((stc.STC_CSS_PSEUDOELEMENT, 'default_style'))

#---- Extra Properties ----#
FOLD = ("fold", "1")

#------------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for CSS""" 
    def __init__(self, langid):
        super(SyntaxData, self).__init__(langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CSS)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [CSS1_KEYWORDS , CSS_PSUEDO_CLASS, CSS2_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'/*', u'*/']

#-----------------------------------------------------------------------------#

def AutoIndenter(estc, pos, ichar):
    """Auto indent cpp code.
    @param estc: EditraStyledTextCtrl
    @param pos: current carat position
    @param ichar: Indentation character

    """
    rtxt = u''
    line = estc.GetCurrentLine()
    text = estc.GetTextRange(estc.PositionFromLine(line), pos)
    eolch = estc.GetEOLChar()

    indent = estc.GetLineIndentation(line)
    if ichar == u"\t":
        tabw = estc.GetTabWidth()
    else:
        tabw = estc.GetIndent()

    i_space = indent / tabw
    ndent = eolch + ichar * i_space
    rtxt = ndent + ((indent - (tabw * i_space)) * u' ')

    if text.endswith('{'):
        rtxt += ichar

    # Put text in the buffer
    estc.AddText(rtxt)
