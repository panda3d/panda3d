!include "MUI.nsh"
!include LogicLib.nsh

; Several variables are assumed to be pre-defined by the caller.  See
; make_installer.py in this directory.

!define UNINSTALL_SUCCESS "$(^Name) was successfully removed from your computer."
!define UNINSTALL_CONFIRM "Are you sure you want to completely remove $(^Name) and all of its components?"
!define UNINSTALL_LINK_NAME "Uninstall"
!define WEBSITE_LINK_NAME "Website"
!define PLID "@panda3d.org/Panda3D Runtime"

; HM NIS Edit Wizard helper defines
!define APP_INTERNAL_NAME "Panda3D"

!define PRODUCT_DIR_REGKEY_PANDA3D "Software\Microsoft\Windows\CurrentVersion\App Paths\${PANDA3D}"
!define PRODUCT_DIR_REGKEY_PANDA3DW "Software\Microsoft\Windows\CurrentVersion\App Paths\${PANDA3DW}"
!define PRODUCT_DIR_REGKEY_OCX "Software\Microsoft\Windows\CurrentVersion\App Paths\${OCX}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PROG_GROUPNAME "${PRODUCT_NAME}"

SetCompressor lzma

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}" ; EULA
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
;!define MUI_FINISHPAGE_NOAUTOCLOSE ;un-comment to put a pause after the file installation screen
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME}"
OutFile p3d-setup.exe
InstallDir "${INSTALL_DIR}"
!ifdef INSTALL_ICON
  Icon "${INSTALL_ICON}"
  UninstallIcon "${INSTALL_ICON}"
!endif
WindowIcon on

ShowInstDetails show
ShowUnInstDetails show

Function .onInit
!ifdef REGVIEW
  SetRegView ${REGVIEW}
!endif

  ClearErrors

  ReadRegStr $0 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation"

  IfErrors +2 0
  StrCpy $INSTDIR $0

FunctionEnd

Section "MainSection" SEC01
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  SetOverwrite ifdiff

!ifdef OCX_PATH
  File "${OCX_PATH}"
!endif
  File "${NPAPI_PATH}"
  File "${PANDA3D_PATH}"
  File "${PANDA3DW_PATH}"

; Auto-detected dependencies on the above executables.  Python
; computes these values for us.
!ifdef DEP0P
  File "${DEP0P}"
!endif
!ifdef DEP1P
  File "${DEP1P}"
!endif
!ifdef DEP2P
  File "${DEP2P}"
!endif
!ifdef DEP3P
  File "${DEP3P}"
!endif
!ifdef DEP4P
  File "${DEP4P}"
!endif
!ifdef DEP5P
  File "${DEP5P}"
!endif
!ifdef DEP6P
  File "${DEP6P}"
!endif
!ifdef DEP7P
  File "${DEP7P}"
!endif
 
!ifdef ADD_START_MENU
; Start->Programs links
  CreateDirectory "$SMPROGRAMS\${PROG_GROUPNAME}"
;  CreateShortCut "$SMPROGRAMS\${PROG_GROUPNAME}\${PRODUCT_NAME_SHORT}.lnk" "$INSTDIR\${LAUNCHER}"
!endif

; Desktop Icon...commented out for now
;  CreateShortCut "$DESKTOP\${PRODUCT_NAME_SHORT}.lnk" "$INSTDIR\${OCX}"

  # Make the directory "$INSTDIR" read write accessible by all users
  AccessControl::GrantOnFile "$INSTDIR" "(BU)" "FullAccess"

;  File "..\..\..\path\to\file\Example.file"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
!ifdef ADD_START_MENU
  CreateShortCut "$SMPROGRAMS\${PROG_GROUPNAME}\${WEBSITE_LINK_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PROG_GROUPNAME}\${UNINSTALL_LINK_NAME}.lnk" "$INSTDIR\uninst.exe"
!endif
SectionEnd

Section -Post
!ifdef REGVIEW
  SetRegView ${REGVIEW}
!endif

  WriteUninstaller "$INSTDIR\uninst.exe"

  WriteRegStr HKCR ".p3d" "" "Panda3D applet"
  WriteRegStr HKCR ".p3d" "Content Type" "application/x-panda3d"
  WriteRegStr HKCR ".p3d" "PerceivedType" "application"
  WriteRegStr HKCR "Panda3D applet" "" "Panda3D applet"
  WriteRegStr HKCR "Panda3D applet\DefaultIcon" "" "$INSTDIR\${PANDA3DW}"
  WriteRegStr HKCR "Panda3D applet\shell" "" "open"
  WriteRegStr HKCR "Panda3D applet\shell\open\command" "" '"$INSTDIR\${PANDA3DW}" "%1"'
  WriteRegExpandStr HKCR "Panda3D applet\shell\open2" "" "Open &with Command Prompt"
  ;WriteRegExpandStr HKCR "Panda3D applet\shell\open2" "MUIVerb" "@%SystemRoot%\System32\wshext.dll,-4511"
  WriteRegExpandStr HKCR "Panda3D applet\shell\open2\command" "" '"$INSTDIR\${PANDA3D}" "%1"'

  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY_PANDA3D}" "" "$INSTDIR\${PANDA3D}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY_PANDA3DW}" "" "$INSTDIR\${PANDA3DW}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PANDA3D}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1

  SectionGetSize SEC01 $0
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" $0

  # Delete keys we used in older versions
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY_OCX}"
  DeleteRegKey HKCR "Panda3D applet\shell\edit"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "${UNINSTALL_SUCCESS}"
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "${UNINSTALL_CONFIRM}" IDYES +2
  Abort
FunctionEnd

Function .onInstSuccess
  # Register ActiveX
  ExecWait 'regsvr32 /s "$INSTDIR/${OCX}"'

!ifdef REGVIEW
  SetRegView ${REGVIEW}
!endif

  # Register Mozilla Plugin
  WriteRegStr HKLM "SOFTWARE\MozillaPlugins\${PLID}" "Description" "Runs 3-D games and interactive applets"
  WriteRegStr HKLM "SOFTWARE\MozillaPlugins\${PLID}" "Path" "$INSTDIR\${NPAPI}"
  WriteRegStr HKLM "SOFTWARE\MozillaPlugins\${PLID}" "ProductName" "${PRODUCT_NAME_SHORT}"
  WriteRegStr HKLM "SOFTWARE\MozillaPlugins\${PLID}" "Vendor" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "SOFTWARE\MozillaPlugins\${PLID}" "Version" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "SOFTWARE\MozillaPlugins\${PLID}\MimeTypes\application/x-panda3d" "Description" "Panda3D applet"

  # Remove old stuff
  DeleteRegKey HKLM "SOFTWARE\MozillaPlugins\${PLID},version=0.0"

FunctionEnd

Section Uninstall
  SetShellVarContext all

  ExecWait 'regsvr32 /u /s "$INSTDIR/${OCX}"'

  Delete "$INSTDIR\${OCX}"
  Delete "$INSTDIR\${NPAPI}"
  Delete "$INSTDIR\${PANDA3D}"
  Delete "$INSTDIR\${PANDA3DW}"
!ifdef DEP0
  Delete "$INSTDIR\${DEP0}"
!endif
!ifdef DEP1
  Delete "$INSTDIR\${DEP1}"
!endif
!ifdef DEP2
  Delete "$INSTDIR\${DEP2}"
!endif
!ifdef DEP3
  Delete "$INSTDIR\${DEP3}"
!endif
!ifdef DEP4
  Delete "$INSTDIR\${DEP4}"
!endif
!ifdef DEP5
  Delete "$INSTDIR\${DEP5}"
!endif
!ifdef DEP6
  Delete "$INSTDIR\${DEP6}"
!endif
!ifdef DEP7
  Delete "$INSTDIR\${DEP7}"
!endif

!ifdef REGVIEW
  SetRegView ${REGVIEW}
!endif

# The following loop uninstalls the plugin where it may have been
# copied into one of the Mozilla Extensions dirs.  Older versions of
# the installer would have done this, but now we just update the
# registry to point to $INSTDIR.
StrCpy $1 "0"
Mozilla-Uninstall-Loop:
  EnumRegKey $0 HKLM "SOFTWARE\Mozilla" "$1"
  StrCmp $0 "" Mozilla-Uninstall-End
  IntOp $1 $1 + 1
  ReadRegStr $2 HKLM "SOFTWARE\Mozilla\$0\Extensions" "Plugins"
  ${If} $2 != ""
     Delete "$2\${NPAPI}"
     # We can't delete the dependency files, because who knows--maybe
     # some other plugins are also using the same files.
  ${EndIf}
  goto Mozilla-Uninstall-Loop
Mozilla-Uninstall-End:

  DeleteRegKey HKLM "SOFTWARE\MozillaPlugins\${PLID}"
  DeleteRegKey HKLM "SOFTWARE\MozillaPlugins\${PLID},version=0.0"

  DeleteRegKey HKCR ".p3d"
  DeleteRegKey HKCR "Panda3D applet"

  # Remove the user's "Panda3D" directory, where all of the downloaded
  # contents are installed.  Too bad we can't do this for every system
  # user, not just the current user.

  RmDir /r "$LOCALAPPDATALow\${APP_INTERNAL_NAME}"
  RmDir /r "$LOCALAPPDATA\${APP_INTERNAL_NAME}"

  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\${PRODUCT_NAME}.url"

!ifdef ADD_START_MENU
  Delete "$SMPROGRAMS\${PROG_GROUPNAME}\${UNINSTALL_LINK_NAME}.lnk"
  Delete "$SMPROGRAMS\${PROG_GROUPNAME}\${WEBSITE_LINK_NAME}.lnk"
;  Delete "$DESKTOP\${PRODUCT_NAME_SHORT}${PRODUCT_RELEASE}.lnk"
;  Delete "$SMPROGRAMS\${PROG_GROUPNAME}\${PRODUCT_NAME_SHORT}.lnk"
  RMDir "$SMPROGRAMS\${PROG_GROUPNAME}"
!endif

  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY_PANDA3D}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY_PANDA3DW}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY_OCX}"
  SetAutoClose true
SectionEnd

