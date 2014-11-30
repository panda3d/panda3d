###############################################################################
# Name: gui4cli.py                                                            #
# Purpose: Syntax configuration for the Gui4Cli programming language          #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: gui4cli.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Gui4Cli

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: gui4cli.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
GLOBALS = (0, "3STATE #ANSI #FIXANSI #FIXOEM #FIXSYS #GUI #SEPARATOR #SYS ABRI "
              "ACTIVE ALL ALTSTART APPDATA APPWIN ARRANGE ARROW ASCEND AUTO "
              "BACK BC BITBUCKET BL BOLD BORDER BOTTOM BR BUFFERS BUSY BUTTON "
              "BUTTONS CAPTION CENTER CHAR CHECK CHILD CLEAN CLEAR CLOSED "
              "COLOR COMM COMMON.ALTSTART COMMON.DESKTOP COMMON.FAVORITES "
              "COMMON.MENU COMMON.PROGRAMS COMMON.STARTUP COOKIES CROSS CURDIR "
              "CURRENT CURSOR DATA DATE DAY DECORATIVE DEF1 DEF2 DEF3 DEF4 "
              "DESCEND DESKTOP DIALOG DIR DISABLE DISABLED DISK DISKS DLBEGIN "
              "DLCOMPLETE DLGFRAME DOUBLE DRAG DROP DROPLIST DTTM EDIT EDITOR "
              "EFFECTS ELLIPSE EMBOSS END ENABLE ENDGUI ENGLISH ENTER ERROR "
              "EXISTS EXPLORE EXT FAIL FAVORITES FIELDS FILE FILES FIND FIRST "
              "FIXED FIXWIDTH FLAT FNUMBER FOCUS FOREGROUND FORMAT FORWARD "
              "FREE FRONT FULL FULLPATH GCDIR GCEXE GCNAME GREEK GRID GUIPATH "
              "HEAVY HEIGHT HELP HEX HIDDEN HIDE HIST HISTORY HOME HORIZONTAL "
              "HOT HOUR IBEAM ICLEFT INDEX INFO INT INVOKE ITALIC ITEM JULIAN "
              "JUSTIFY LARGE LAST LB LBLEFT LC LENGTH LIGHT LINE LINES LMB "
              "LMDC LOAD LOADED LOWER LT MAX MAXI MAXBOX MAXIMIZE MEDIUM MENU "
              "MINBOX MINI MINIMIZE MINUTE MMB MODERN MONTH MOVE MSEC MULTI "
              "NAME NAVCOMPLETE NETCACHE NETHOOD NEW NEWMENU NEXT NO NOADJUST "
              "NOBORDER NOERROR NOEXT NOFACE NOFILES NOFOCUS NOFOLDERS NONE "
              "NOOPEN NOREFRESH NORESET NORMAL NORTH NOSIZE NOVECTOR NOVRT NOW "
              "NOWRAP NUMBER OCTAL OFF OK OKCANCEL ON ONECLICK ONELINE OPEN "
              "OVERLAP OWNED PARENT PATH PCPATH PERIOD PERSONAL POLYGON PREFS "
              "PREV PREVIOUS PRINT PRINTERS PROGRAMS PROP PROPERTY PULSE "
              "QUESTION QUOTE RAGGED RAISED RB RC REC RECENT REFRESH REMOVE "
              "REMSIZE RENAME REPORT RESIZE RET RETRY RIGHT RMB ROMAN ROOT "
              "ROUNDED ROUTINE ROWS RT SAVE SCALABLE SCREEN SCRIPT SCROLL "
              "SEARCH SECOND SELECT SELECTED SELSCRIPT SENDTO SENSITIVE "
              "SENTENCE SHELL SHOW SILENT SIMPLE SINGLE SIZE SMALL SMOOTH "
              "SOLID SORT START STARTGUI STARTUP STAT0 STATIC STATUS STD STOP "
              "STRECH STRIKE SUBCHILD SUBSUB SUNK SUNKEN SWISS SYSMENU TAB "
              "TABS TC TEMPLATES TEXT THIN TIME TITLE TL TOGGLE TOOL TOP "
              "TOPMOST TOTAL TR TRANS TRANSPARENT TTONLY TYPE UNDERLINE "
              "UNFORMAT UNJULIAN UNQUOTE UNSELECT UNSELECTED UPPER USER VALID "
              "VARIABLE VCENTER VERSION VERTICAL VIEW WAIT WARN WHEEL WIDTH "
              "WINEDGE WORD YEAR YES YESNO YESTOALL YNCANCEL")

EVENTS = (1, "XAREA XBROWSER XBUTTON XCHECKBOX XCOMBO XEDBOX XGROUPBOX XICON "
             "XIMAGE XLISTVIEW XMENU XPAN XPROGRESS XRADIO XREBAR XSPLITER "
             "XSTATUS XTAB XTEXTBOX XTEXTIN XTOOLBAR XTRACKBAR XTREEVIEW "
             "XUPDOWN XAFTER XATTR XBEFORE XENUM XHOTKEY XNOTIFY XONACTIVE "
             "XONBROWSER XONCLICK XONCLOSE XONDOUBLECLICK XONDROP XONFAIL "
             "XONHELP XONINACTIVE XONKEY XONLMB XONLOAD XONLVCLICK XONLVDIR "
             "XONMARK XONMMB XONOPEN XONQUIT XONRELOAD XONRETURN XONRMB "
             "XONTEXTIN XONWHEELDOWN XONWHEELUP XPIPE XREQFILE XROUTINE "
             "XTBARICON XTIMER XONKEY")

ATTRIBUTES = (2, "ATTR BACKGROUND BUDDY BUFFERS COLORS COLSAVE DATA DBGSTEP DBGVARS "
                 "DEBUG DEEPTRANS EDITOR ENDGUI ESCAPE FAIL FILTER FONT FRAME "
                 "GRID HELP ICON ID IXICON KEY LIKE LOCAL LVCOLUMN MAXDATE "
                 "MAXSIZE MAXTRANS MCINOTIFY MCISIGNAL MENU MINDATE MINSIZE "
                 "NEXT OUTPUT PAGE PARENT PATH PREFS RBSTYLE RESIZE SAY SHAPE "
                 "SHOW STARTGUI STYLE TAB TITLE TRANSLATE VARIABLES")

CONTROL = (3, "AND ANDIFEXISTS BREAK CALL CASE DOCASE ELSE ELSEIF ELSEIFEXISTS "
              "ENDCASE ENDFOR ENDIF ENDWHILE FOR GO GOSUB GOTO IF IFEXISTS "
              "LABEL OR ORIFEXISTS QUIT RETURN STOP WHILE")

COMMANDS = (4, "ADD ADDRESS ADDUNIQUE APPEND APPVAR ASSIGN AUTO BRANCH BROWSER "
               "CD CLOSE COMBO COMMAND COPY CREATE CREATELINK CUT CUTVAR DBSUM "
               "DCKDEBUG DDEXEC DDPUT DDUSE DEC DEL DELAY DELETE DELVAR EDBOX "
               "EMPTYBIN ENUM EXIT EXTRACT FLASH FREEFONT FREEICON GETCLIP "
               "GETCOLOR GETFONT GOSUB GUI GUICLOSE GUILOAD GUIOPEN GUIQUIT "
               "GUIRENAME GUIWINDOW HTMLHELP IMAGE INC JOINFILE LAUNCH "
               "LISTVIEW LOADFONT LOADICON LV LVACTION LVCLIP LVREP LVSEARCH "
               "LVSWITCH MAKEDIR MCI MOVE MSGBOX NEWFILE PARSEVAR POPMENU "
               "QUICKMENU QUIT RANDOM REGCREATE REGDELETE REGGET REGSET RENAME "
               "REPVAR REQFILE RUN SAY SEARCHVAR SEND SET SETATTR SETCLIP "
               "SETEVENT SETGADVALUES SETICON SETPOINTER SETVAR SETWINATTR "
               "SETWINTITLE SHELL STATUS SYSTEM TERMINATEPROC TEXTFILE "
               "TREEVIEW TV UPDATE UPDOWN USE WAIT WINATTR WINDOW")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_GC_ATTRIBUTE', 'keyword3_style'),
                ('STC_GC_COMMAND', 'keyword2_style'),
                ('STC_GC_COMMENTBLOCK', 'comment_style'),
                ('STC_GC_COMMENTLINE', 'comment_style'),
                ('STC_GC_CONTROL', 'keyword_style'),
                ('STC_GC_DEFAULT', 'default_style'),
                ('STC_GC_EVENT', 'keyword4_style'),
                ('STC_GC_GLOBAL', 'global_style'),
                ('STC_GC_OPERATOR', 'operator_style'),
                ('STC_GC_STRING', 'string_style')]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_COMP = ("fold.compact", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_GUI4CLI:
        return [GLOBALS, EVENTS, ATTRIBUTES, CONTROL, COMMANDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_GUI4CLI:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties
    @note: gui4cli supports folding but it seems to be partially broken
    """
    if lang_id == synglob.ID_LANG_GUI4CLI:
        return list() #[FOLD, FOLD_COMP]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_GUI4CLI:
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
