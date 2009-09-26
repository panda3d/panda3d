!include "MUI.nsh"
!include LogicLib.nsh
!include FileFunc.nsh

; Several variables are assumed to be pre-defined by the caller.  See
; make_installer.py in this directory.

!define UNINSTALL_SUCCESS "$(^Name) was successfully removed from your computer."
!define UNINSTALL_CONFIRM "Are you sure you want to completely remove $(^Name) and all of its components?"
!define UNINSTALL_LINK_NAME "Uninstall"
!define WEBSITE_LINK_NAME "Website"

!insertmacro GetOptions

; HM NIS Edit Wizard helper defines
!define APP_INTERNAL_NAME "Panda3D"

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${OCX}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PROG_GROUPNAME "${PRODUCT_NAME}"

!define FIREFOX_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\firefox.exe"

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
;!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_NOAUTOCLOSE ;un-comment to put a pause after the file installation screen
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
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  SetOverwrite ifdiff

  File "${OCX_PATH}"
  File "${NPAPI_PATH}"
  File "${PANDA3D_PATH}"

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

; Start->Programs links
  CreateDirectory "$SMPROGRAMS\${PROG_GROUPNAME}"
;  CreateShortCut "$SMPROGRAMS\${PROG_GROUPNAME}\${PRODUCT_NAME_SHORT}.lnk" "$INSTDIR\${LAUNCHER}"

; Desktop Icon...commented out for now
;  CreateShortCut "$DESKTOP\${PRODUCT_NAME_SHORT}.lnk" "$INSTDIR\${OCX}"

  # Make the directory "$INSTDIR" read write accessible by all users
  AccessControl::GrantOnFile "$INSTDIR" "(BU)" "FullAccess"

;  File "..\..\..\path\to\file\Example.file"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${PROG_GROUPNAME}\${WEBSITE_LINK_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PROG_GROUPNAME}\${UNINSTALL_LINK_NAME}.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\${OCX}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${OCX}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" ""
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
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

 # Install Firefox Plugin
 ReadRegStr $0 HKLM "${FIREFOX_DIR_REGKEY}" Path
 ${If} $0 != ""
     CopyFiles $INSTDIR\${NPAPI} $0\plugins
!ifdef NPAPI_DEP0
     CopyFiles $INSTDIR\${NPAPI_DEP0} $0\plugins
!endif
!ifdef NPAPI_DEP1
     CopyFiles $INSTDIR\${NPAPI_DEP1} $0\plugins
!endif
!ifdef NPAPI_DEP2
     CopyFiles $INSTDIR\${NPAPI_DEP2} $0\plugins
!endif
!ifdef NPAPI_DEP3
     CopyFiles $INSTDIR\${NPAPI_DEP3} $0\plugins
!endif
!ifdef NPAPI_DEP4
     CopyFiles $INSTDIR\${NPAPI_DEP4} $0\plugins
!endif
 ${EndIf}
FunctionEnd

Section Uninstall
  SetShellVarContext all

  ExecWait 'regsvr32 /u /s "$INSTDIR/${OCX}"'

  Delete "$INSTDIR\${OCX}"
  Delete "$INSTDIR\${NPAPI}"
  Delete "$INSTDIR\${PANDA3D}"
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

  ReadRegStr $0 HKLM "${FIREFOX_DIR_REGKEY}" Path
  ${If} $0 != ""
      Delete "$0\plugins\${NPAPI}"
  ${EndIf}

  ReadRegDWORD $0 HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System EnableLUA
  ${If} $0 != ""
      RmDir /r "$LOCALAPPDATALow\${APP_INTERNAL_NAME}"
  ${Else}
      RmDir /r "$LOCALAPPDATA\${APP_INTERNAL_NAME}"
  ${Endif}

  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\${PRODUCT_NAME}.url"

  Delete "$SMPROGRAMS\${PROG_GROUPNAME}\${UNINSTALL_LINK_NAME}.lnk"
  Delete "$SMPROGRAMS\${PROG_GROUPNAME}\${WEBSITE_LINK_NAME}.lnk"
;  Delete "$DESKTOP\${PRODUCT_NAME_SHORT}${PRODUCT_RELEASE}.lnk"
;  Delete "$SMPROGRAMS\${PROG_GROUPNAME}\${PRODUCT_NAME_SHORT}.lnk"
  RMDir "$SMPROGRAMS\${PROG_GROUPNAME}"

  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
