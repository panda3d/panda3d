###############################################################################
# Name: tcl.py                                                                #
# Purpose: Define TCL/TK syntax for highlighting and other features           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: tcl.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for tcl

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: tcl.py 55180 2008-08-22 17:35:29Z CJP $"
__revision__ = "$Revision: 55180 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
from cpp import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
TCL_KW = (0, "after append array auto_execok auto_import auto_load catch cd "
             "auto_load_index auto_qualify beep bgerror binary break case file "
             "clock close concat continue dde default echo else elseif eof "
             "encoding error eval exec exit expr fblocked fconfigure fcopy "
             "fileevent flush for foreach format gets glob global history http "
             "if incr info interp join lappend lindex linsert list llength "
             "load loadTk lrange lreplace lsearch lset lsort memory msgcat "
             "namespace open package pid pkg::create pkg_mkIndex regsub rename "
             "Platform-specific proc puts pwd re_syntax read regexp registry "
             "resource return scan seek set socket source split string subst "
             "switch tclLog tclMacPkgSearch tclPkgSetup tclPkgUnknown tell "
             "time trace unknown unset update uplevel upvar variable vwait "
             "while")

TK_KW = (1, "bell bind bindtags bitmap button canvas checkbutton clipboard "
             "colors console cursors destroy entry event focus font frame grab "
             "grid image Inter-client keysyms label labelframe listbox lower "
             "menu menubutton message option options pack panedwindow photo "
             "radiobutton raise scale scrollbar selection send spinbox text tk "
             "tk_chooseColor tk_chooseDirectory tk_dialog tk_focusNext tkvars "
             "tk_messageBox tk_optionMenu tk_popup tk_setPalette tkerror "
             "tkwait toplevel winfo wish wm place tk_getOpenFile ")

ITCL_KW = (2, "@scope body class code common component configbody constructor "
              "define destructor hull import inherit itcl itk itk_component "
              "itk_initialize itk_interior itk_option iwidgets keep method "
              "private protected public")

TK_COMMANDS = (3, "tk_bisque tk_chooseColor tk_dialog tk_focusFollowsMouse "
                  "tk_focusNext tk_focusPrev tk_getOpenFile tk_getSaveFile "
                  "tk_messageBox tk_optionMenu tk_popup tk_setPalette "
                  "tk_textCut tk_textPaste tkButtonAutoInvoke tkButtonDown "
                  "tkButtonEnter tkButtonInvoke tkButtonLeave tkButtonUp "
                  "tkCancelRepeat tkCheckRadioDown tkCheckRadioEnter "
                  "tkCheckRadioInvoke tkColorDialog tkColorDialog_BuildDialog "
                  "tkColorDialog_CancelCmd tkColorDialog_Config "
                  "tkColorDialog_CreateSelector tkColorDialog_DrawColorScale "
                  "tkColorDialog_EnterColorBar tkColorDialog_HandleRGBEntry "
                  "tkColorDialog_HandleSelEntry tkColorDialog_InitValues "
                  "tkColorDialog_LeaveColorBar tkColorDialog_MoveSelector "
                  "tkColorDialog_OkCmd tk_textCopy tkColorDialog_StartMove "
                  "tkColorDialog_RedrawColorBars tkColorDialog_ResizeColorBars "
                  "tkColorDialog_RedrawFinalColor tkColorDialog_ReleaseMouse "
                  "tkColorDialog_RgbToX tkColorDialog_SetRGBValue "
                  "tkColorDialog_XToRgb tkConsoleAbout tkConsoleBind "
                  "tkConsoleExit tkConsoleHistory tkConsoleInit tkFocusOK "
                  "tkConsoleInsert tkConsoleInvoke tkConsoleOutput "
                  "tkConsolePrompt tkConsoleSource tkDarken tkEntryAutoScan "
                  "tkEntryBackspace tkEntryButton1 tkEntryClosestGap "
                  "tkEntryGetSelection tkEntryInsert tkEntryKeySelect "
                  "tkEntryMouseSelect tkEntryNextWord tkEntryPaste tkFirstMenu "
                  "tkEntryPreviousWord tkEntrySeeInsert tkEntrySetCursor "
                  "tkEntryTranspose tkEventMotifBindings tkFDGetFileTypes "
                  "tkFocusGroup_BindIn tkFocusGroup_BindOut tkFocusGroup_Out"
                  "tkFocusGroup_Create tkFocusGroup_Destroy tkFocusGroup_In "
                  "tkGenerateMenuSelect tkIconList tkIconList_Add "
                  "tkIconList_Arrange tkIconList_AutoScan tkIconList_Btn1 "
                  "tkIconList_Config tkIconList_Create tkIconList_CtrlBtn1 "
                  "tkIconList_Curselection tkIconList_DeleteAll "
                  "tkIconList_Double1 tkIconList_DrawSelection tkIconList_See "
                  "tkIconList_FocusIn tkIconList_FocusOut tkIconList_Get "
                  "tkIconList_Goto tkIconList_Index tkIconList_Invoke "
                  "tkIconList_KeyPress tkIconList_Leave1 tkIconList_LeftRight "
                  "tkIconList_Motion1 tkIconList_Reset tkIconList_ReturnKey "
                  "tkIconList_Select tkIconList_Selection tkIconList_ShiftBtn1 "
                  "tkIconList_UpDown tkListbox tkListboxKeyAccel_Key "
                  "tkListboxAutoScan tkListboxBeginExtend tkListboxBeginSelect "
                  "tkListboxBeginToggle tkListboxCancel tkListboxDataExtend "
                  "tkListboxExtendUpDown tkListboxKeyAccel_Goto tkMbButtonUp "
                  "tkListboxKeyAccel_Reset tkListboxKeyAccel_Set "
                  "tkListboxKeyAccel_Unset tkListboxMotion tkListboxSelectAll "
                  "tkMbEnter tkMbLeave tkMbMotion tkMbPost tkMenuButtonDown "
                  "tkMenuDownArrow tkMenuDup tkMenuEscape tkMenuFind "
                  "tkMenuFindName tkMenuFirstEntry tkMenuInvoke tkMenuLeave "
                  "tkMenuLeftArrow tkMenuMotion tkMenuNextEntry tkMenuNextMenu "
                  "tkMenuRightArrow tkMenuUnpost tkMenuUpArrow tkMessageBox "
                  "tkMotifFDialog tkMotifFDialog_ActivateDList tkScaleDrag "
                  "tkMotifFDialog_ActivateFEnt tkMotifFDialog_ActivateFList "
                  "tkMotifFDialog_ActivateSEnt tkMotifFDialog_BrowseDList "
                  "tkMotifFDialog_BrowseFList tkMotifFDialog_BuildUI "
                  "tkMotifFDialog_CancelCmd tkMotifFDialog_Config "
                  "tkMotifFDialog_Create tkMotifFDialog_FileTypes "
                  "tkMotifFDialog_FilterCmd tkMotifFDialog_InterpFilter "
                  "tkMotifFDialog_LoadFiles tkMotifFDialog_MakeSList "
                  "tkMotifFDialog_OkCmd tkMotifFDialog_SetFilter "
                  "tkMotifFDialog_SetListMode tkMotifFDialog_Update "
                  "tkPostOverPoint tkRecolorTree tkRestoreOldGrab "
                  "tkSaveGrabInfo tkScaleActivate tkScaleButton2Down "
                  "tkScaleButtonDown tkScaleControlPress  tkScaleEndDrag "
                  "tkScaleIncrement tkScreenChanged tkScrollButton2Down "
                  "tkScrollButtonDown tkScrollButtonDrag tkScrollButtonUp "
                  "tkScrollByPages tkScrollByUnits tkScrollDrag tkTextNextPos "
                  "tkScrollEndDrag tkScrollSelect tkScrollStartDrag "
                  "tkScrollTopBottom tkScrollToPos tkTabToWindow tkTearOffMenu "
                  "tkTextAutoScan tkTextButton1 tkTextClosestGap tkTextInsert "
                  "tkTextKeyExtend tkTextKeySelect tkTextNextPara "
                  "tkTextNextWord tkTextPaste tkTextPrevPara tkTextPrevPos "
                  "tkTextPrevWord tkTextResetAnchor tkTextScrollPages "
                  "tkTextSelectTo tkTextSetCursor tkTextTranspose "
                  "tkTextUpDownLine tkTraverseToMenu tkTraverseWithinMenu "
                  "tkListboxUpDown ")

EXPAND = (4, "")

USER1_KW = (5, "")

USER2_KW = (6, "")

USER3_KW = (7, "")

USER4_KW = (8, "")

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_TCL_BLOCK_COMMENT', 'comment_style'),
                 ('STC_TCL_COMMENT', 'comment_style'),
                 ('STC_TCL_COMMENTLINE', 'comment_style'),
                 ('STC_TCL_COMMENT_BOX', 'comment_style'),
                 ('STC_TCL_DEFAULT', 'default_style'),
                 ('STC_TCL_EXPAND', 'default_style'), # STYLE NEEDED
                 ('STC_TCL_IDENTIFIER', 'default_style'),
                 ('STC_TCL_IN_QUOTE', 'string_style'),
                 ('STC_TCL_MODIFIER', 'default_style'), # STYLE NEEDED
                 ('STC_TCL_NUMBER', 'number_style'),
                 ('STC_TCL_OPERATOR', 'operator_style'),
                 ('STC_TCL_SUBSTITUTION', 'scalar_style'),
                 ('STC_TCL_SUB_BRACE', 'string_style'), # STYLE NEEDED
                 ('STC_TCL_WORD', 'keyword_style'),        # tcl_kw
                 ('STC_TCL_WORD2', 'keyword2_style'),      # tk_kw
                 ('STC_TCL_WORD3', 'keyword3_style'),      # itcl_kw
                 ('STC_TCL_WORD4', 'keyword4_style'),      # tkCommands
                 ('STC_TCL_WORD5', 'default_style'),
                 ('STC_TCL_WORD6', 'default_style'),
                 ('STC_TCL_WORD7', 'default_style'),
                 ('STC_TCL_WORD8', 'default_style'),
                 ('STC_TCL_WORD_IN_QUOTE', 'default_style')]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_COMMENT = ("fold.comment", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_TCL:
        return [TCL_KW, TK_KW, ITCL_KW, TK_COMMANDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_TCL:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_TCL:
        return [FOLD, FOLD_COMMENT]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_TCL:
        return [u'#']
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
