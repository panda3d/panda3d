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
__svnid__ = "$Id: kix.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob

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
SYNTAX_ITEMS = [('STC_KIX_COMMENT', 'comment_style'),
                ('STC_KIX_DEFAULT', 'default_style'),
                ('STC_KIX_FUNCTIONS', 'funct_style'),
                ('STC_KIX_IDENTIFIER', 'default_style'),
                ('STC_KIX_KEYWORD', 'keyword_style'),
                ('STC_KIX_MACRO', 'pre_style'),
                ('STC_KIX_NUMBER', 'number_style'),
                ('STC_KIX_OPERATOR', 'operator_style'),
                ('STC_KIX_STRING1', 'char_style'),
                ('STC_KIX_STRING2', 'string_style'),
                ('STC_KIX_VAR', 'scalar_style')]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_KIX:
        return [COMMANDS, FUNCTIONS, MACROS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_KIX:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_KIX:
        return list()
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_KIX:
        return [u';']
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
