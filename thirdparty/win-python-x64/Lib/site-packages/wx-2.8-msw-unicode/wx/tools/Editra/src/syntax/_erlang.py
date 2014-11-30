###############################################################################
# Name: erlang.py                                                             #
# Purpose: Define Erlang syntax for highlighting and other features           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: erlang.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for the Erlang Programming Language
@todo: better styling

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _erlang.py 66104 2010-11-10 20:18:29Z CJP $"
__revision__ = "$Revision: 66104 $"

#-----------------------------------------------------------------------------#
# Imports
import wx
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
KEYWORDS = (0, "compile define else endif export file ifdef ifndef import "
               "include include_lib module record undef author copyright doc "
               "after begin case catch cond end fun if let of query receive "
               "when define record export import include include_lib else "
               "endif undef apply attribute call do in letrec module primop "
               "try")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_ERLANG_ATOM, 'default_style'),       # need new tag
                (stc.STC_ERLANG_CHARACTER, 'char_style'),
                (stc.STC_ERLANG_COMMENT, 'comment_style'),
                (stc.STC_ERLANG_DEFAULT, 'default_style'),
                (stc.STC_ERLANG_FUNCTION_NAME, 'funct_style'),
                (stc.STC_ERLANG_KEYWORD, 'keyword_style'),
                (stc.STC_ERLANG_MACRO, 'pre_style'),
                (stc.STC_ERLANG_NODE_NAME, 'string_style'),   # maybe change
                (stc.STC_ERLANG_NUMBER, 'number_style'),
                (stc.STC_ERLANG_OPERATOR, 'operator_style'),
                (stc.STC_ERLANG_RECORD, 'keyword2_style'),
                (stc.STC_ERLANG_STRING, 'string_style'),
                (stc.STC_ERLANG_UNKNOWN, 'unknown_style'),
                (stc.STC_ERLANG_VARIABLE, 'default_style')]   # need custom?

# Version specific 
if wx.VERSION >= (2, 9, 0, 0, ''):
    SYNTAX_ITEMS.append((stc.STC_ERLANG_ATOM_QUOTED, 'default_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_BIFS, 'default_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_COMMENT_DOC, 'dockey_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_COMMENT_DOC_MACRO, 'dockey_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_COMMENT_FUNCTION, 'comment_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_COMMENT_MODULE, 'comment_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_MACRO_QUOTED, 'default_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_MODULES, 'default_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_MODULES_ATT, 'default_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_NODE_NAME_QUOTED, 'default_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_PREPROC, 'pre_style')) # TODO
    SYNTAX_ITEMS.append((stc.STC_ERLANG_RECORD_QUOTED, 'default_style')) # TODO
else:
    SYNTAX_ITEMS.append((stc.STC_ERLANG_SEPARATOR, 'default_style')) # need style?

#---- Extra Properties ----#
FOLD = ('fold', '1')
FOLD_CMT = ('fold.comments', '1')
FOLD_KW = ('fold.keywords', '1')
FOLD_BRACE = ('fold.braces', '1')

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Erlang""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_ERLANG)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'%%']
