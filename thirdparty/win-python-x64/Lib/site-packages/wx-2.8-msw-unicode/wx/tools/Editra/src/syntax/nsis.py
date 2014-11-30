###############################################################################
# Name: nsis.py                                                               #
# Purpose: Define NSIS syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: nsis.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Nullsoft Installer Scripts.
@todo: Add User Defined KW

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: nsis.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# NSIS Functions
NSIS_FUNCT = (0, "!addincludedir !addplugindir MakeNSIS Portions Contributors: "
                 "Abort AddBrandingImage AddSize AutoCloseWindow BGFont "
                 "BrandingText BringToFront Call CallInstDLL Caption ChangeUI "
                 "ClearErrors ComponentText GetDLLVersion GetDLLVersionLocal "
                 "GetFileTime GetFileTimeLocal CopyFiles CRCCheck FileRead "
                 "CreateFont CreateShortCut SetDatablockOptimize DeleteINISec "
                 "DeleteINIStr DeleteRegKey DeleteRegValue Delete DetailPrint "
                 "DirText DirShow DirVar DirVerify GetInstDirError BGGradient"
                 "AllowRootDirInstall CheckBitmap EnableWindow EnumRegKey "
                 "EnumRegValue Exch Exec ExecWait ExecShell ExpandEnvStrings "
                 "FindWindow FindClose FindFirst FindNext File FileBufSize "
                 "FlushINI ReserveFile FileClose FileErrorText FileOpen IntCmp "
                 "FileWrite FileReadByte FileWriteByte FileSeek Function Page "
                 "GetDlgItem GetFullPathName GetTempFileName HideWindow Icon "
                 "IfErrors IfFileExists IfRebootFlag IfSilent InstallDirRegKey "
                 "InstallColors InstallDir InstProgressFlags InstType IntOp "
                 "IntCmpU IntFmt IsWindow Goto LangString LangStringUP Return "
                 "LicenseForceSelection LicenseLangString LicenseText "
                 "LoadLanguageFile LogSet LogText MessageBox Nop Name OutFile "
                 "PageCallbacks PageEx PageExEnd Pop Push Quit ReadINIStr "
                 "ReadRegDWORD ReadRegStr ReadEnvStr Reboot RegDLL Rename "
                 "RMDir Section SectionEnd SectionIn SubSection SectionGroup "
                 "SubSectionEnd SectionGroupEnd SearchPath SectionSetFlags "
                 "SectionGetFlags SectionSetInstTypes SectionGetInstTypes "
                 "SectionGetText SectionSetText SectionGetSize SectionSetSize "
                 "GetCurInstType SetCurInstType InstTypeSetText SetCompress"
                 "SendMessage SetAutoClose SetCtlColors SetBrandingImage  "
                 "SetCompressor SetCompressorDictSize SetCompressionLevel "
                 "SetDateSave SetDetailsView SetDetailsPrint SetErrors "
                 "GetErrorLevel SetFileAttributes SetFont SetOutPath "
                 "SetPluginUnload SetRebootFlag SetShellVarContext SetSilent "
                 "ShowInstDetails ShowUninstDetails ShowWindow SilentInstall "
                 "SilentUnInstall Sleep StrCmp StrCpy StrLen SubCaption "
                 "UninstallExeName UninstallCaption UninstallIcon UninstPage "
                 "UninstallText UninstallSubCaption UnRegDLL WindowIcon "
                 "WriteRegBin WriteRegDWORD WriteRegStr WriteRegExpandStr "
                 "WriteUninstaller XPStyle !packhdr !system !execute !echo "
                 "!include !cd !ifdef !ifndef !endif !define !undef !else "
                 "!warning !error !verbose !macro !macroend !insertmacro "
                 "!ifmacrondef MiscButtonText DetailsButtonText "
                 "InstallButtonText SpaceTexts CompletedText InitPluginsDir "
                 "GetLabelAddress GetCurrentAddress !AddPluginDir LockWindow "
                 "AllowSkipFiles Var VIAddVersionKey VIProductVersion "
                 "ShowUnInstDetails WriteIniStr CreateDirectory FunctionEnd "
                 "IfAbort LicenseData LicenseBkColor InstTypeGetText "
                 "SetErrorLevel SetOverwrite WriteINIStr !AddIncludeDir "
                 "!ifmacrodef UninstallButtonText GetFunctionAddress ")

# NSIS Variables/Constants
NSIS_VAR = (1, "$0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $R0 $R1 $R2 $R3 $R4 $R5 $R6 $R7 "
               "$R8 $R9 $\t $\" $\' $\` $VARNAME $0, $INSTDIR $OUTDIR $CMDLINE "
               "$LANGUAGE $PROGRAMFILES $COMMONFILES $DESKTOP $EXEDIR "
               "${NSISDIR} $WINDIR $SYSDIR $TEMP $STARTMENU $SMPROGRAMS "
               "$SMSTARTUP $QUICKLAUNCH $DOCUMENTS $SENDTO $RECENT $FAVORITES "
               "$MUSIC $PICTURES $VIDEOS $NETHOOD $FONTS $TEMPLATES $APPDATA "
               "$PRINTHOOD $INTERNET_CACHE $COOKIES $HISTORY $PROFILE "
               "$ADMINTOOLS $RESOURCES $RESOURCES_LOCALIZED $CDBURN_AREA "
               "$HWNDPARENT $PLUGINSDIR $$ $\r $\n")

# NSIS Lables (Attributes)
NSIS_LBL = (2, "ARCHIVE FILE_ATTRIBUTE_ARCHIVE FILE_ATTRIBUTE_HIDDEN LEFT "
               "FILE_ATTRIBUTE_NORMAL FILE_ATTRIBUTE_OFFLINE lastused HKCR "
               "FILE_ATTRIBUTE_SYSTEM FILE_ATTRIBUTE_TEMPORARY HIDDEN HKCC "
               "HKCU HKDD HKEY_CLASSES_ROOT HKEY_CURRENT_CONFIG IDYES SYSTEM "
               "HKEY_DYN_DATA HKEY_LOCAL_MACHINE MB_ICONQUESTION OFFLINE "
               "HKLM HKPD HKU IDABORT IDCANCEL IDIGNORE IDNO IDOK IDRETRY "
               "MB_ABORTRETRYIGNORE MB_DEFBUTTON1 MB_DEFBUTTON2 MB_DEFBUTTON3 "
               "MB_DEFBUTTON4 MB_ICONEXCLAMATION MB_ICONINFORMATION normal off "
               "MB_ICONSTOP MB_OK MB_OKCANCEL MB_RETRYCANCEL MB_RIGHT listonly "
               "MB_SETFOREGROUND MB_TOPMOST MB_YESNO MB_YESNOCANCEL NORMAL "
               "READONLY SW_SHOWMAXIMIZED SW_SHOWMINIMIZED SW_SHOWNORMAL "
               "TEMPORARY auto colored false force hide ifnewer nevershow "
               "on show silent silentlog smooth true try lzma zlib bzip2 none "
               "textonly both top left bottom right license components "
               "instfiles uninstConfirm custom all leave current ifdiff "
               "RIGHT CENTER dlg_id ALT CONTROL EXT SHIFT open print manual "
               "alwaysoff FILE_ATTRIBUTE_READONLY HKEY_CURRENT_USER directory "
               "HKEY_PERFORMANCE_DATA HKEY_USERS ")

# NSIS User Defined (Not sure need help)
NSIS_DEF = (3, "")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_NSIS_DEFAULT', 'default_style'),
                 ('STC_NSIS_COMMENT', 'comment_style'),
                 ('STC_NSIS_FUNCTION', 'funct_style'),
                 ('STC_NSIS_FUNCTIONDEF', 'keyword_style'),
                 ('STC_NSIS_IFDEFINEDEF', 'pre_style'),
                 ('STC_NSIS_LABEL', 'class_style'),
                 ('STC_NSIS_MACRODEF', 'pre_style'),
                 ('STC_NSIS_NUMBER', 'number_style'),
                 ('STC_NSIS_SECTIONDEF', 'keyword_style'),
                 ('STC_NSIS_STRINGDQ', 'string_style'),
                 ('STC_NSIS_STRINGLQ', 'string_style'),
                 ('STC_NSIS_STRINGRQ', 'string_style'),
                 ('STC_NSIS_STRINGVAR', 'string_style'),
                 ('STC_NSIS_SUBSECTIONDEF', 'keyword_style'),
                 ('STC_NSIS_USERDEFINED', 'pre_style'),
                 ('STC_NSIS_VARIABLE', 'scalar_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_NSIS:
        return [NSIS_FUNCT, NSIS_VAR, NSIS_LBL]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_NSIS:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_NSIS:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_NSIS:
        return [u';']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
