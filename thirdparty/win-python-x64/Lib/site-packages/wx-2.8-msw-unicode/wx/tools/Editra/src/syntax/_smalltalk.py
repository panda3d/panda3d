###############################################################################
# Name: smalltalk.py                                                          #
# Purpose: Define Smalltalk syntax for highlighting and other features        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: smalltalk.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Smalltalk
@todo: more keywords, styling fixes

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _smalltalk.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
# Special Selectors
ST_KEYWORDS = (0, "ifTrue: ifFalse: whileTrue: whileFalse: ifNil: ifNotNil: "
                  "whileTrue repeat isNil put to at notNil super self "
                  "true false new not isNil inspect out nil do add for "
                  "methods methodsFor instanceVariableNames classVariableNames "
                  "poolDictionaries subclass")
#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_ST_ASSIGN, 'operator_style'),
                (stc.STC_ST_BINARY, 'operator_style'),
                (stc.STC_ST_BOOL, 'keyword_style'),
                (stc.STC_ST_CHARACTER, 'char_style'),
                (stc.STC_ST_COMMENT, 'comment_style'),
                (stc.STC_ST_DEFAULT, 'default_style'),
                (stc.STC_ST_GLOBAL, 'global_style'),
                (stc.STC_ST_KWSEND, 'keyword_style'),
                (stc.STC_ST_NIL, 'keyword_style'),
                (stc.STC_ST_NUMBER, 'number_style'),
                (stc.STC_ST_RETURN, 'keyword_style'),
                (stc.STC_ST_SELF, 'keyword_style'),
                (stc.STC_ST_SPECIAL, 'pre_style'),
                (stc.STC_ST_SPEC_SEL, 'keyword_style'), # Words in keyword list
                (stc.STC_ST_STRING, 'string_style'),
                (stc.STC_ST_SUPER, 'class_style'),
                (stc.STC_ST_SYMBOL, 'scalar_style')]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Smalltalk""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_SMALLTALK)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [ST_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'\"', u'\"']

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return ST_KEYWORDS[1]

#---- End Syntax Modules Internal Functions ----#
