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
__svnid__ = "$Id: dot.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

import synglob
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
                 ('STC_C_VERBATIM', "number2_style"),
                 ('STC_C_WORD', 'keyword_style'),
                 ('STC_C_WORD2', 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_DOT:
        return [DOT_KEYWORDS, DOT_TYPES]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_DOT:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_DOT:
        return [FOLD, FOLD_PRE]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_DOT:
        return [u'//']
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
