###############################################################################
# Name: kix.py                                                                #
# Purpose: Syntax configuration module for KIXtart scripts                    #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: kix.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for KIXtart scripts

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _kix.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
COMMANDS = (0, "? and beep big break call cd cls color cookie1 copy debug del "
               "dim display do until exit flushkb for each next function "
               "endfunction get gets global go gosub goto if else endif md or "
               "password play quit rd redim return run select case endselect "
               "set setl setm settime shell sleep small use while loop")

FUNCTIONS = (1, "abs addkey addprinterconnection addprogramgroup "
                "addprogramitem asc ascan at backupeventlog box cdbl chr cint "
                "cleareventlog close comparefiletimes createobject cstr "
                "dectohex delkey delprinterconnection delprogramgroup "
                "delprogramitem deltree delvalue dir enumgroup enumipinfo "
                "enumkey enumlocalgroup enumvalue execute exist existkey "
                "expandenvironmentvars fix formatnumber freefilehandle "
                "getdiskspace getfileattr getfilesize getfiletime "
                "getfileversion getobject iif ingroup instr instrrev int "
                "isdeclared join kbhit keyexist lcase left len loadhive "
                "loadkey logevent logoff ltrim memorysize messagebox open "
                "readline readprofilestring readtype readvalue redirectoutput "
                "right rnd round rtrim savekey sendkeys sendmessage setascii "
                "setconsole setdefaultprinter setfileattr setfocus setoption "
                "setsystemstate settitle setwallpaper showprogramgroup "
                "shutdown sidtoname split srnd substr trim ubound ucase "
                "unloadhive val vartype vartypename writeline "
                "writeprofilestring writevalue")

MACROS = (2, "address build color comment cpu crlf csd curdir date day domain "
             "dos error fullname homedir homedrive homeshr hostname inwin "
             "ipaddress0 ipaddress1 ipaddress2 ipaddress3 kix lanroot ldomain "
             "ldrive lm logonmode longhomedir lserver maxpwage mdayno mhz "
             "monthno month msecs pid primarygroup priv productsuite "
             "producttype pwage ras result rserver scriptdir scriptexe "
             "scriptname serror sid site startdir syslang ticks time userid "
             "userlang wdayno wksta wuserid ydayno year")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [(stc.STC_KIX_COMMENT, 'comment_style'),
                (stc.STC_KIX_DEFAULT, 'default_style'),
                (stc.STC_KIX_FUNCTIONS, 'funct_style'),
                (stc.STC_KIX_IDENTIFIER, 'default_style'),
                (stc.STC_KIX_KEYWORD, 'keyword_style'),
                (stc.STC_KIX_MACRO, 'pre_style'),
                (stc.STC_KIX_NUMBER, 'number_style'),
                (stc.STC_KIX_OPERATOR, 'operator_style'),
                (stc.STC_KIX_STRING1, 'char_style'),
                (stc.STC_KIX_STRING2, 'string_style'),
                (stc.STC_KIX_VAR, 'scalar_style')]

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Kix""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_KIX)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [COMMANDS, FUNCTIONS, MACROS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u';']
