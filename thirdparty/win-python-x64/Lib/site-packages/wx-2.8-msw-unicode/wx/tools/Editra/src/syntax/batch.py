###############################################################################
# Name: batch.py                                                              #
# Purpose: Define DOS Batch syntax for highlighting and other features        #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: batch.py
AUTHOR: Cody Precord
@summary: Lexer configuration file for dos/windows batch scripts.
@todo: incorportate winbat keywords

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: batch.py 53369 2008-04-26 05:35:41Z CJP $"
__revision__ = "$Revision: 53369 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

DOSBAT_KEYWORDS = (0, "append arp assoc at atmadm attrib bootcfg break cacls "
                      "call cd chcp chdir chkdsk chkntfs cls cmd color command "
                      "comp compact control convert copy date debug defrag del "
                      "delete dir diskcomp diskcopy doskey echo edit edlin "
                      "else endlocal EQU erase errorlevel exists exit expand "
                      "fc find findstr for format ftp ftype GEQ goto graftabl "
                      "GTR help if ipconfig keyb label LEQ loadfix logoff LSS "
                      "md mkdir mode more move nbtstat NEQ net netsh netstat "
                      "nlsfunc not nslookup path pathping pause ping popd "
                      "print prompt pushd rd rem ren rename replace restore "
                      "rmdir route runas set setlocal setver share shift "
                      "shutdown sort start subst time title telnet tracetr "
                      "tree type ver verify vol xcopy ")

# WinBatch Keywords
WINBAT_KEYWORDS = (0, "if then else endif break end return exit next while for "
                      "gosub goto switch select to case endselect endwhile "
                      "endswitch aboveicons acc_attrib acc_chng_nt acc_control "
                      "acc_create acc_delete acc_full_95 acc_full_nt acc_list "
                      "acc_pfull_nt acc_pmang_nt acc_print_nt acc_read "
                      "acc_read_95 acc_read_nt acc_write amc arrange ascending "
                      "attr_a attr_a attr_ci attr_ci attr_dc attr_dc attr_di "
                      "attr_di attr_dm attr_dm attr_h attr_h attr_ic attr_ic "
                      "attr_p attr_p attr_ri attr_ri attr_ro attr_ro attr_sh "
                      "attr_sh attr_sy attr_sy attr_t attr_t attr_x attr_x "
                      "avogadro backscan boltzmann cancel capslock check "
                      "columnscommonformat cr crlf ctrl default default "
                      "deg2rad descending disable drive electric enable eulers "
                      "false faraday float8 fwdscan gftsec globalgroup gmtsec "
                      "goldenratio gravitation hidden icon lbutton lclick "
                      "ldblclick lf lightmps lightmtps localgroup magfield "
                      "major mbokcancel mbutton mbyesno mclick mdblclick minor "
                      "msformat multiple ncsaformat no none none noresize "
                      "normal notify nowait numlock off on open parsec "
                      "parseonly pi planckergs planckjoules printer rad2deg "
                      "rbutton rclick rdblclick regclasses regcurrent "
                      "regmachine regroot regusers rows save scrolllock server "
                      "shift single sorted stack string tab tile true uncheck "
                      "unsorted wait wholesection word1 word2 word4 yes zoomed "
                      "about abs acos addextender appexist appwaitclose asin "
                      "askfilename askfiletext askitemlist askline askpassword "
                      "askyesno atan average beep binaryalloc binarycopy "
                      "binaryeodget binaryeodset binaryfree binaryhashrec "
                      "binaryincr binaryincr2 binaryincr4 binaryincrflt "
                      "binaryindex binaryindexnc binaryoletype binarypeek "
                      "binarypeek2 binarypeek4 binarypeekflt binarypeekstr "
                      "binarypoke binarypoke2 binarypoke4 binarypokeflt "
                      "binarypokestr binaryread binarysort binarystrcnt "
                      "binarywrite boxbuttondraw boxbuttonkill boxbuttonstat "
                      "boxbuttonwait boxcaption boxcolor boxdataclear "
                      "boxdatatag boxdestroy boxdrawcircle boxdrawline "
                      "boxdrawrect boxdrawtext boxesup boxmapmode boxnew "
                      "boxopen boxpen boxshut boxtext boxtextcolor boxtextfont "
                      "boxtitle boxupdates break buttonnames by call callext "
                      "ceiling char2num clipappend clipget clipput continue "
                      "cos cosh datetime ddeexecute ddeinitiate ddepoke "
                      "dderequest ddeterminate ddetimeout debug debugdata "
                      "decimals delay dialog dialogbox dirattrget dirattrset "
                      "dirchange direxist")

#---- Language Styling Specs ----#
SYNTAX_ITEMS = [ ('STC_BAT_DEFAULT', "default_style"),
                 ('STC_BAT_COMMAND', "class_style"),
                 ('STC_BAT_COMMENT', "comment_style"),
                 ('STC_BAT_HIDE', "string_style"),
                 ('STC_BAT_IDENTIFIER', "scalar_style"),
                 ('STC_BAT_LABEL', "class_style"),
                 ('STC_BAT_OPERATOR', "operator_style"),
                 ('STC_BAT_WORD', "keyword_style") ]

#---- Extra Properties ----#
FOLD = ("fold", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_BATCH:
        return [DOSBAT_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_BATCH:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_BATCH:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_BATCH:
        return [u'rem']
    else:
        return list()

#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
