###############################################################################
# Name: dot.py                                                                #
# Purpose: Define DOT graph visualization language syntax for highlighting    #
#          and other features.                                                #
# Author: Rob McMullen <robm@users.sourceforge.net>                           #
# Copyright: (c) 2007 Rob McMullen <robm@users.sourceforge.net                #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: dot.py
AUTHOR: Rob McMullen
@summary: Lexer configuration module for the DOT graph description language

"""

__author__ = "Rob McMullen <robm@users.sourceforge.net>"
__svnid__ = "$Id: _dot.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
DOT_KEYWORDS = (0, "node edge graph digraph subgraph strict")

DOT_TYPES = (1,
"Damping K URL arrowhead arrowsize arrowtail bb bgcolor center charsetil "
"clusterrank color colorscheme comment compound concentrate constraint "
"decorate defaultdist dim dir diredgeconstraints distortion dpi edgeURL "
"edgehref edgetarget edgetooltip epsilon esep fillcolor fixedsize "
"fontcolor fontname fontnames fontpath fontsize group headURL headclip "
"headhref headlabel headport headtarget headtooltip height href label "
"labelURL labelangle labeldistance labelfloat labelfontcolor "
"labelfontname labelfontsize labelhref labeljust labelloc labeltarget "
"labeltooltip landscape layer layers layersep len levelsgap lhead lp "
"ltail margin maxiter mclimit mindist minlen mode model mosek nodesep "
"nojustify normalize nslimit nslimit1 ordering orientation orientation "
"outputorder overlap pack packmode pad page pagedir pencolor "
"peripheries pin pos quantum rank rankdir ranksep ratio rects regular "
"remincross resolution root rotate samehead sametail samplepoints "
"searchsize sep shape shapefile showboxes sides size skew splines start "
"style stylesheet tailURL tailclip tailhref taillabel tailport "
"tailtarget tailtooltip target tooltip truecolor vertices viewport "
"voro_margin weight width z arrowType clusterMode color colorList "
"dirType escString layerList layerRange lblString outputMode packMode "
"pagedir point pointf pointfList portPos rankType rankdir rect shape "
"splineType startType style viewPort "
)

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_C_DEFAULT, 'default_style'),
                 (stc.STC_C_COMMENT, 'comment_style'),
                 (stc.STC_C_COMMENTLINE, 'comment_style'),
                 (stc.STC_C_COMMENTDOC, 'comment_style'),
                 (stc.STC_C_COMMENTDOCKEYWORD, 'dockey_style'),
                 (stc.STC_C_COMMENTDOCKEYWORDERROR, 'error_style'),
                 (stc.STC_C_COMMENTLINE, 'comment_style'),
                 (stc.STC_C_COMMENTLINEDOC, 'comment_style'),
                 (stc.STC_C_CHARACTER, 'char_style'),
                 (stc.STC_C_GLOBALCLASS, 'global_style'),
                 (stc.STC_C_IDENTIFIER, 'default_style'),
                 (stc.STC_C_NUMBER, 'number_style'),
                 (stc.STC_C_OPERATOR, 'operator_style'),
                 (stc.STC_C_PREPROCESSOR, 'pre_style'),
                 (stc.STC_C_REGEX, 'pre_style'),
                 (stc.STC_C_STRING, 'string_style'),
                 (stc.STC_C_STRINGEOL, 'stringeol_style'),
                 (stc.STC_C_UUID, 'pre_style'),
                 (stc.STC_C_VERBATIM, "number2_style"),
                 (stc.STC_C_WORD, 'keyword_style'),
                 (stc.STC_C_WORD2, 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for DOT""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CPP)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [DOT_KEYWORDS, DOT_TYPES]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD, FOLD_PRE]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'//']
