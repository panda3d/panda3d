###############################################################################
# Name: issuelist.py                                                          #
# Purpose: Define IssueList syntax for highlighting and other features        #
# Author: Torsten Mohr <none_yet>                                             #
# Copyright: (c) 2008 Cody Precord <staff>                                    #
#            (c) 2008 Torsten Mohr <none_yet>                                 #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: issuelist.py
AUTHOR: Cody Precord, Torsten Mohr
@summary: Lexer configuration module for Issue Lists.

"""

__author__ = "Cody Precord <cprecord>, Torsten Mohr <none_yet>"
__svnid__ = "$Id: _issuelist.py 62364 2009-10-11 01:02:12Z CJP $"
__revision__ = "$Revision: 62364 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

#Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#
# Style Id's

(STC_ISSL_DEFAULT,
STC_ISSL_COMMENT,
STC_ISSL_GREEN,
STC_ISSL_RED,
STC_ISSL_ORANGE,
STC_ISSL_BLUE,
STC_ISSL_PURPLE,
STC_ISSL_PINK) = range(8)

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [
    (STC_ISSL_DEFAULT, 'default_style'),
    (STC_ISSL_COMMENT, 'comment_style'),
    (STC_ISSL_GREEN,   'regex_style'),
    (STC_ISSL_RED,     'number_style'),
    (STC_ISSL_ORANGE,  'keyword4_style'),
    (STC_ISSL_BLUE,    'dockey_style'),
    (STC_ISSL_PURPLE,  'scalar2_style'),
    (STC_ISSL_PINK,    'char_style'),
    ]
   
issl_table = ';+-?.#~'

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for IssueLists
    This class is primarly intended as an example to creating a custom
    lexer

    """ 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CONTAINER)
        self.RegisterFeature(synglob.FEATURE_STYLETEXT, StyleText)

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

#---- End Required Module Functions ----#

# ; comment
# + done
# - open
# ? question
# . info
# # strong
# ~
# * 

def StyleText(stc, start, end):
    """Style the text
    @param stc: Styled text control instance
    @param start: Start position
    @param end: end position

    """

    # First, figure out the line based on the position.
    line = stc.LineFromPosition(start)

    # Find the start of the line that's been styled before this one.
    while line > 0 and stc.GetLineState(line) == 0:
        line -= 1

    eline = stc.LineFromPosition(end)

    state = stc.GetLineState(line) - 1
    if state < 0:
        state = 0

    for ln in range(line, eline + 1):
        text = stc.GetLine(ln).encode('utf-8')
        len_text = len(text)
        text = text.strip()

        if len(text) == 0:
            state = 0
        else:
            if len(text) > 0:
                ch = text[0]
                ix = issl_table.find(ch)
                if ix >= 0:
                    state = ix + 1

        stc.StartStyling(stc.PositionFromLine(ln), 0xFF)
        stc.SetStyling(len_text, state)

        stc.SetLineState(ln, state + 1)
