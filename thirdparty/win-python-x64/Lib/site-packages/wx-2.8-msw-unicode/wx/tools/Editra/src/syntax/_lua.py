###############################################################################
# Name: lua.py                                                                #
# Purpose: Define Lua5 syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: lua.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Lua
@todo: This setup for Lua5, maybe add Lua4 support

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _lua.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
# Keywords
LUA_KEYWORDS = (0, "and break do else elseif end false for function if "
                   "in local nil not or repeat return then true until while")

# Basic Functions
LUA_FUNCT = (1, "_VERSION assert collectgarbage dofile error gcinfo loadfile "
                "loadstring print rawget rawset require tonumber tostring type "
                "unpack "
                # Lua5 Basic functions
                "_G getfenv getmetatable ipairs loadlib next pairs pcall "
                "rawequal setfenv setmetatable xpcall \string table math "
                "coroutine io os debug \load module select")

# String, (table) & math functions Lua5
LUA_STR = (2, "string.byte string.char string.dump string.find string.len "
              "string.lower string.rep string.sub string.upper string.format "
              "string.gfind string.gsub table.concat table.foreach "
              "table.foreachi table.getn table.sort table.insert table.remove "
              "table.setn math.abs math.acos math.asin math.atan math.atan2 "
              "math.ceil math.cos math.deg math.exp math.floor math.frexp "
              "math.ldexp math.log math.log10 math.max math.min math.mod "
              "math.pi math.pow math.rad math.random math.randomseed math.sin "
              "math.sqrt math.tan string.gmatch string.match string.reverse "
              "table.maxn math.cosh math.fmod math.modf math.sinh math.tanh "
              "math.huge")

# (coroutines), I/O & system facilities
LUA_CO = (3, "coroutine.create coroutine.resume coroutine.status coroutine."
             "wrap coroutine.yield io.close io.flush io.input io.lines io.open "
             "io.output io.read io.tmpfile io.type io.write io.stdin io.stdout "
             "io.stderr os.clock os.date os.difftime os.execute os.exit "
             "os.getenv os.remove os.rename os.setlocale os.time os.tmpname "
             "coroutine.running package.cpath package.loaded package.loadlib "
             "package.path package.preload package.seeall io.popen")

# user1
LUA_U1 = (4, "")

# user2
LUA_U2 = (5, "")

# user3
LUA_U3 = (6, "")

# user4
LUA_U4 = (7, "")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_LUA_CHARACTER, 'char_style'),
                (stc.STC_LUA_COMMENT, 'comment_style'),
                (stc.STC_LUA_COMMENTDOC, 'dockey_style'),
                (stc.STC_LUA_COMMENTLINE, 'comment_style'),
                (stc.STC_LUA_DEFAULT, 'default_style'),
                (stc.STC_LUA_IDENTIFIER, 'default_style'), # style maybe
                (stc.STC_LUA_LITERALSTRING, 'string_style'),
                (stc.STC_LUA_NUMBER, 'number_style'),
                (stc.STC_LUA_OPERATOR, 'operator_style'),
                (stc.STC_LUA_PREPROCESSOR, 'pre_style'),
                (stc.STC_LUA_STRING, 'string_style'),
                (stc.STC_LUA_STRINGEOL, 'stringeol_style'),
                (stc.STC_LUA_WORD, 'keyword_style'),
                (stc.STC_LUA_WORD2, 'keyword3_style'),
                (stc.STC_LUA_WORD3, 'funct_style'),
                (stc.STC_LUA_WORD4, 'funct_style'),
                (stc.STC_LUA_WORD5, 'default_style'), # currently unused
                (stc.STC_LUA_WORD6, 'default_style'), # currently unused
                (stc.STC_LUA_WORD7, 'default_style'), # currently unused
                (stc.STC_LUA_WORD8, 'default_style')  # currently unused
               ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_COMP = ("fold.compact", "1")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Lua""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_LUA)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [LUA_KEYWORDS, LUA_FUNCT, LUA_STR, LUA_CO]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set"""
        return [FOLD, FOLD_COMP]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'--']
