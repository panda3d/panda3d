; Panda3D installation script for the Nullsoft Installation System (NSIS).
; Jon Parise <jparise@cmu.edu>
; with Ben Johnson <bkj@andrew.cmu.edu>
; with Jason Pratt <pratt@andrew.cmu.edu>
; mangled by Josh Yelon <jyelon@andrew.cmu.edu>

; Caller needs to define these variables:
;   COMPRESSOR    - either zlib or lzma
;   FULLNAME      - full name of what we're building            (ie, "Panda3D")
;   SMDIRECTORY   - where to put this in the start menu         (ie, "Panda3D VERSION")
;   UNINSTALLKEY  - what registry key to use for uninstaller    (ie, "Panda3D VERSION")
;   INSTALLDIR    - where to install the program                (ie, "C:\Program Files\Panda3D-VERSION")
;   PPGAME        - only if making a prepackaged game.          (ie, "Airblade")

!define PANDA ..\built
!define PSOURCE ..

; Use the Modern UI
!include "MUI.nsh"
; Windows system messaging support
!include "${NSISDIR}\Include\WinMessages.nsh"

Name "${FULLNAME}"
InstallDir "${INSTALLDIR}"
OutFile "..\nsis-output.exe"

SetCompress auto
SetCompressor ${COMPRESSOR}

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${PSOURCE}\doc\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "Panda3DEnglish"

ShowInstDetails show
ShowUninstDetails show
LicenseData ${PSOURCE}\doc\LICENSE

InstType "Typical"

!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

var READABLE

Section "${SMDIRECTORY}" SecCore
        SectionIn 1 2 3 RO

        SetOutPath $INSTDIR
        SetOverwrite try

        SetOutPath $INSTDIR
        File ${PSOURCE}\doc\LICENSE
        SetOutPath $INSTDIR\bin
        File /r ${PANDA}\bin\*.dll
        SetOutPath $INSTDIR\etc
        File /r ${PANDA}\etc\*
        SetOutPath $INSTDIR\direct
        File /r /x CVS /x Opt?-Win32 ${PSOURCE}\direct\*.py
        File ${PANDA}\direct\__init__.py
        SetOutPath $INSTDIR\pandac
        File /r ${PANDA}\pandac\*.py
        File /r ${PANDA}\pandac\*.pyz
        SetOutPath $INSTDIR\python
        File /r /x CVS /x *.pyc ${PANDA}\python\*

        RMDir /r "$SMPROGRAMS\${SMDIRECTORY}"
        CreateDirectory "$SMPROGRAMS\${SMDIRECTORY}"

        !ifdef PPGAME

            SetOutPath $INSTDIR\bin
            File /r ${PANDA}\bin\ppython.exe
            SetOutpath $INSTDIR\${PPGAME}
            File /r ${PSOURCE}\${PPGAME}\*
            SetOutPath $INSTDIR\${PPGAME}
            CreateShortCut "$SMPROGRAMS\${SMDIRECTORY}\Play ${FULLNAME}.lnk" "$INSTDIR\bin\ppython.exe" "${PPGAME}.py" "$INSTDIR\bin\ppython.exe" 0 SW_SHOWMINIMIZED "" "Play ${FULLNAME}"

        !else

            SetOutPath $INSTDIR\bin
            File /nonfatal /r ${PANDA}\bin\*.dle
            File /nonfatal /r ${PANDA}\bin\*.dlo
            File /nonfatal /r ${PANDA}\bin\*.dlm
            SetOutPath $INSTDIR\pandac\input
            File /r ${PANDA}\pandac\input\*
            SetOutPath $INSTDIR\bin
            File /r ${PANDA}\bin\*.exe
            SetOutPath $INSTDIR\lib
            File /r /x *.exp ${PANDA}\lib\*
            SetOutPath $INSTDIR\include
            File /r /x *.exp ${PANDA}\include\*
            SetOutPath $INSTDIR\SceneEditor
            File /r /x CVS ${PSOURCE}\SceneEditor\*.*
            SetOutPath $INSTDIR\Pmw
            File /r /x CVS ${PANDA}\Pmw\*.*
            SetOutPath $INSTDIR
            File /r /x CVS ${PSOURCE}\doc\ReleaseNotes
            SetOutPath $INSTDIR\python\lib
            File /nonfatal /r ${PSOURCE}\thirdparty\win-extras\*

            DetailPrint "Extracting models ..."
            SetDetailsPrint textonly
            SetOutPath $INSTDIR\models
            File /r /x CVS ${PANDA}\models\*

            DetailPrint "Extracting samples ..."
            SetDetailsPrint textonly
            SetOutPath $INSTDIR\samples
            File /r /x CVS ${PSOURCE}\samples\*

            SetOutPath $INSTDIR
            WriteINIStr $INSTDIR\Website.url "InternetShortcut" "URL" "http://panda3d.etc.cmu.edu/"
            WriteINIStr $INSTDIR\Manual.url "InternetShortcut" "URL" "http://panda3d.etc.cmu.edu/manual/"
            CreateShortCut "$SMPROGRAMS\${SMDIRECTORY}\Panda Manual.lnk" "$INSTDIR\Manual.url" "" "$INSTDIR\bin\ppython.exe" 0 "" "" "Panda Manual"
            CreateShortCut "$SMPROGRAMS\${SMDIRECTORY}\Panda Website.lnk" "$INSTDIR\Website.url" "" "$INSTDIR\bin\ppython.exe" 0 "" "" "Panda Website"
            SetOutPath $INSTDIR\samples\RubiksCube
            CreateShortCut "$SMPROGRAMS\${SMDIRECTORY}\Panda Test - Rubiks Cube.lnk" "$INSTDIR\bin\ppython.exe" "rubiksCube.py" "$INSTDIR\bin\ppython.exe" 0 SW_SHOWMINIMIZED "" "Panda Test"
            CreateDirectory "$SMPROGRAMS\${SMDIRECTORY}\Tutorials"

            FindFirst $0 $1 $INSTDIR\samples\*--*
            loop:
                DetailPrint "Sample: $1"
                StrCmp $1 "" done
		Push $1
		Push "\"
		Push "--"
	        Call StrRep
	        Push " "
                Push "-"
                Call StrRep
                Pop $R0
                StrCpy $READABLE $R0
                CreateDirectory "$SMPROGRAMS\${SMDIRECTORY}\$READABLE"
                SetOutPath $INSTDIR\samples\$1
                CreateShortCut "$SMPROGRAMS\${SMDIRECTORY}\$READABLE\Introduction.lnk" "$INSTDIR\samples\$1\Intro.html"
                CreateShortCut "$SMPROGRAMS\${SMDIRECTORY}\$READABLE\Browse Tutorial.lnk" "$INSTDIR\samples\$1"
            next:
                FindNext $0 $1
                Goto loop
            done:

        !endif


SectionEnd


Section -post

        !ifndef PPGAME
        # Add the "bin" directory to the PATH.
        DetailPrint "Adding 'bin' directory to the PATH..."
        Push "$INSTDIR\bin"
        Call AddToPath
        !endif

        DetailPrint "Registering Helix Preferences..."
        WriteRegStr HKLM "Software\Helix\HelixSDK\10.0\Preferences\UseOverlay" "" "0"
        WriteRegStr HKLM "Software\Helix\HelixSDK\10.0\Preferences\UseWinDraw" "" "0"
        WriteRegStr HKCU "Software\Helix\HelixSDK\10.0\Preferences\UseOverlay" "" "0"
        WriteRegStr HKCU "Software\Helix\HelixSDK\10.0\Preferences\UseWinDraw" "" "0"

        DetailPrint "Adding the uninstaller ..."
        Delete "$INSTDIR\uninst.exe"
        WriteUninstaller "$INSTDIR\uninst.exe"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALLKEY}" "DisplayName" "${SMDIRECTORY}"
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALLKEY}" "UninstallString" '"$INSTDIR\uninst.exe"'
        CreateShortcut "$SMPROGRAMS\${SMDIRECTORY}\Uninstall ${FULLNAME}.lnk" "$INSTDIR\uninst.exe" ""
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
        !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "The core files required to use ${FULLNAME}."
        !ifndef PPGAME
          !insertmacro MUI_DESCRIPTION_TEXT ${SecSamples} "Sample worlds, scripts, models and tutorials."
        !endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section Uninstall

!ifndef PPGAME
        DetailPrint "Removing ${SMDIRECTORY} from PATH ..."
        Push "$INSTDIR\bin"
        Call un.RemoveFromPath
!endif

        Delete "$INSTDIR\uninst.exe"
        DetailPrint "Removing Start Menu folder ..."
        RMDir /r "$SMPROGRAMS\${SMDIRECTORY}"
        DetailPrint "Completely removing the directory ($INSTDIR)..."
        RMDir /r "$INSTDIR"
        DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTALLKEY}"

SectionEnd

# --[ Utility Functions ]------------------------------------------------------

; From: http://nsis.sourceforge.net/archive/viewpage.php?pageid=91
Function IsNT
        Push $0
        ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
        StrCmp $0 "" 0 IsNT_yes
        ; we are not NT.
        Pop $0
        Push 0
        Return
        IsNT_yes:
                ; NT!!!
                Pop $0
                Push 1
FunctionEnd

; From: http://nsis.sourceforge.net/archive/viewpage.php?pageid=91
Function un.IsNT
        Push $0
        ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
        StrCmp $0 "" 0 unIsNT_yes
        ; we are not NT.
        Pop $0
        Push 0
        Return
        unIsNT_yes:
                ; NT!!!
                Pop $0
                Push 1
FunctionEnd

; From: http://nsis.sourceforge.net/archive/viewpage.php?pageid=91
Function un.StrStr
        Push $0
        Exch
        Pop $0 ; $0 now have the string to find
        Push $1
        Exch 2
        Pop $1 ; $1 now have the string to find in
        Exch
        Push $2
        Push $3
        Push $4
        Push $5
        StrCpy $2 -1
        StrLen $3 $0
        StrLen $4 $1
        IntOp $4 $4 - $3
        unStrStr_loop:
                IntOp $2 $2 + 1
                IntCmp $2 $4 0 0 unStrStrReturn_notFound
                StrCpy $5 $1 $3 $2
                StrCmp $5 $0 unStrStr_done unStrStr_loop
        unStrStrReturn_notFound:
                StrCpy $2 -1
        unStrStr_done:
                Pop $5
                Pop $4
                Pop $3
                Exch $2
                Exch 2
                Pop $0
                Pop $1
FunctionEnd

; From: http://nsis.sourceforge.net/archive/viewpage.php?pageid=91
; Commentary and smarter ';' checking by Jon Parise <jparise@cmu.edu>
Function AddToPath
        Exch $0
        Push $1
        Push $2
        Push $3
        Call IsNT
        Pop $1

        StrCmp $1 1 AddToPath_NT
                ; We're not on NT, so modify the AUTOEXEC.BAT file.
                StrCpy $1 $WINDIR 2
                FileOpen $1 "$1\autoexec.bat" a
                FileSeek $1 0 END
                GetFullPathName /SHORT $0 $0
                FileWrite $1 "$\r$\nSET PATH=%PATH%;$0$\r$\n"
                FileClose $1
                Goto AddToPath_done
        
        AddToPath_NT:
                ReadRegStr $1 HKCU "Environment" "PATH"
                Call IsUserAdmin
                Pop $3
                ; If this is an Admin user, use the System env. variable instead of the user's env. variable
                StrCmp $3 1 0 +2
                        ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"

                ; If the PATH string is empty, jump over the mangling routines.
                StrCmp $1 "" AddToPath_NTdoIt

                ; Pull off the last character of the PATH string.  If it's a semicolon,
                ; we don't need to add another one, so jump to the section where we
                ; append the new PATH component(s).
                StrCpy $2 $1 1 -1
                StrCmp $2 ";" AddToPath_NTAddPath AddToPath_NTAddSemi

                AddToPath_NTAddSemi:
                        StrCpy $1 "$1;"
                        Goto AddToPath_NTAddPath
                AddToPath_NTAddPath:
                        StrCpy $0 "$1$0"
                        Goto AddToPath_NTdoIt
                AddToPath_NTdoIt:
                        Call IsUserAdmin
                        Pop $3
                        StrCmp $3 1 0 NotAdmin
                                WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $0
                                Goto AddToPath_done
                        
                        NotAdmin:
                                WriteRegExpandStr HKCU "Environment" "PATH" $0
        AddToPath_done:
                SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
                Pop $3
                Pop $2
                Pop $1
                Pop $0
FunctionEnd

; From: http://nsis.sourceforge.net/archive/viewpage.php?pageid=91
Function un.RemoveFromPath
        Exch $0
        Push $1
        Push $2
        Push $3
        Push $4
        Push $5
        Call un.IsNT
        Pop $1
        StrCmp $1 1 unRemoveFromPath_NT
                ; Not on NT
                StrCpy $1 $WINDIR 2
                FileOpen $1 "$1\autoexec.bat" r
                GetTempFileName $4
                FileOpen $2 $4 w
                GetFullPathName /SHORT $0 $0
                StrCpy $0 "SET PATH=%PATH%;$0"
                SetRebootFlag true
                Goto unRemoveFromPath_dosLoop

                unRemoveFromPath_dosLoop:
                        FileRead $1 $3
                        StrCmp $3 "$0$\r$\n" unRemoveFromPath_dosLoop
                        StrCmp $3 "$0$\n" unRemoveFromPath_dosLoop
                        StrCmp $3 "$0" unRemoveFromPath_dosLoop
                        StrCmp $3 "" unRemoveFromPath_dosLoopEnd
                        FileWrite $2 $3
                        Goto unRemoveFromPath_dosLoop

                unRemoveFromPath_dosLoopEnd:
                        FileClose $2
                        FileClose $1
                        StrCpy $1 $WINDIR 2
                        Delete "$1\autoexec.bat"
                        CopyFiles /SILENT $4 "$1\autoexec.bat"
                        Delete $4
                        Goto unRemoveFromPath_done

                unRemoveFromPath_NT:
                        StrLen $2 $0
                        Call un.IsUserAdmin
                        Pop $5
                        StrCmp $5 1 0 NotAdmin
                                ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
                                Push $1
                                Push $0
                                Call un.StrStr ; Find $0 in $1
                                Pop $0 ; pos of our dir
                                IntCmp $0 -1 unRemoveFromPath_done
                                        ; else, it is in path
                                        StrCpy $3 $1 $0 ; $3 now has the part of the path before our dir
                                        IntOp $2 $2 + $0 ; $2 now contains the pos after our dir in the path (';')
                                        IntOp $2 $2 + 1 ; $2 now containts the pos after our dir and the semicolon.
                                        StrLen $0 $1
                                        StrCpy $1 $1 $0 $2
                                        StrCpy $3 "$3$1"
                                        WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH" $3
                                        SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
                                        Goto unRemoveFromPath_done
                        
                        
                        NotAdmin:               
                                ReadRegStr $1 HKCU "Environment" "PATH"
                                Push $1
                                Push $0
                                Call un.StrStr ; Find $0 in $1
                                Pop $0 ; pos of our dir
                                IntCmp $0 -1 unRemoveFromPath_done
                                        ; else, it is in path
                                        StrCpy $3 $1 $0 ; $3 now has the part of the path before our dir
                                        IntOp $2 $2 + $0 ; $2 now contains the pos after our dir in the path (';')
                                        IntOp $2 $2 + 1 ; $2 now containts the pos after our dir and the semicolon.
                                        StrLen $0 $1
                                        StrCpy $1 $1 $0 $2
                                        StrCpy $3 "$3$1"
                                        WriteRegExpandStr HKCU "Environment" "PATH" $3
                                        SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

                unRemoveFromPath_done:
                        Pop $5
                        Pop $4
                        Pop $3
                        Pop $2
                        Pop $1
                        Pop $0
FunctionEnd

; From: http://nsis.sourceforge.net/archive/nsisweb.php?page=329&instances=0,11
; Localized by Ben Johnson (bkj@andrew.cmu.edu)
Function IsUserAdmin
        Push $0
        Push $1
        Push $2
        Push $3
        Call IsNT
        Pop $1

        ClearErrors
        UserInfo::GetName
        ;IfErrors Win9x
        Pop $2
        UserInfo::GetAccountType
        Pop $3

        ; Compare results of IsNT with "1"
        StrCmp $1 1 0 NotNT
                ;This is NT
                

                StrCmp $3 "Admin" 0 NotAdmin
                        ; Observation: I get here when running Win98SE. (Lilla)
                        ; The functions UserInfo.dll looks for are there on Win98 too, 
                        ; but just don't work. So UserInfo.dll, knowing that admin isn't required
                        ; on Win98, returns admin anyway. (per kichik)
                        ; MessageBox MB_OK 'User "$R1" is in the Administrators group'
                        Pop $3
                        Pop $2
                        Pop $1
                        Pop $0

                        Push 1
                        Return

                NOtAdmin:
                        ; You should still check for an empty string because the functions
                        ; UserInfo.dll looks for may not be present on Windows 95. (per kichik)
                
                        #StrCmp $2 "" Win9x
                        #StrCpy $0 0
                        ;MessageBox MB_OK 'User "$2" is in the "$3" group'
                        Pop $3
                        Pop $2
                        Pop $1
                        Pop $0

                        Push 0
                        Return

        ;Because we use IsNT, this is redundant.
        #Win9x:
        #       ; comment/message below is by UserInfo.nsi author:
        #       ; This one means you don't need to care about admin or
        #       ; not admin because Windows 9x doesn't either
        #       ;MessageBox MB_OK "Error! This DLL can't run under Windows 9x!"
        #       StrCpy $0 0

        NotNT:
                ;We are not NT
                ;Win9x doesn't have "admin" users.
                ;Let the user do whatever.
                Pop $3
                Pop $2
                Pop $1
                Pop $0

                Push 1

FunctionEnd

Function un.IsUserAdmin
        Push $0
        Push $1
        Push $2
        Push $3
        Call un.IsNT
        Pop $1

        ClearErrors
        UserInfo::GetName
        ;IfErrors Win9x
        Pop $2
        UserInfo::GetAccountType
        Pop $3

        ; Compare results of IsNT with "1"
        StrCmp $1 1 0 NotNT
                ;This is NT
                

                StrCmp $3 "Admin" 0 NotAdmin
                        ; Observation: I get here when running Win98SE. (Lilla)
                        ; The functions UserInfo.dll looks for are there on Win98 too, 
                        ; but just don't work. So UserInfo.dll, knowing that admin isn't required
                        ; on Win98, returns admin anyway. (per kichik)
                        ; MessageBox MB_OK 'User "$R1" is in the Administrators group'
                        Pop $3
                        Pop $2
                        Pop $1
                        Pop $0

                        Push 1
                        Return

                NOtAdmin:
                        ; You should still check for an empty string because the functions
                        ; UserInfo.dll looks for may not be present on Windows 95. (per kichik)
                
                        #StrCmp $2 "" Win9x
                        #StrCpy $0 0
                        ;MessageBox MB_OK 'User "$2" is in the "$3" group'
                        Pop $3
                        Pop $2
                        Pop $1
                        Pop $0

                        Push 0
                        Return

        ;Because we use IsNT, this is redundant.
        #Win9x:
        #       ; comment/message below is by UserInfo.nsi author:
        #       ; This one means you don't need to care about admin or
        #       ; not admin because Windows 9x doesn't either
        #       ;MessageBox MB_OK "Error! This DLL can't run under Windows 9x!"
        #       StrCpy $0 0

        NotNT:
                ;We are not NT
                ;Win9x doesn't have "admin" users.
                ;Let the user do whatever.
                Pop $3
                Pop $2
                Pop $1
                Pop $0

                Push 1

FunctionEnd

Function StrRep

  ;Written by dirtydingus 2003-02-20 04:30:09 
  ; USAGE
  ;Push String to do replacement in (haystack)
  ;Push String to replace (needle)
  ;Push Replacement
  ;Call StrRep
  ;Pop $R0 result
  ;StrCpy $Result STR $R0

  Exch $R4 ; $R4 = Replacement String
  Exch
  Exch $R3 ; $R3 = String to replace (needle)
  Exch 2
  Exch $R1 ; $R1 = String to do replacement in (haystack)
  Push $R2 ; Replaced haystack
  Push $R5 ; Len (needle)
  Push $R6 ; len (haystack)
  Push $R7 ; Scratch reg
  StrCpy $R2 ""
  StrLen $R5 $R3
  StrLen $R6 $R1
loop:
  StrCpy $R7 $R1 $R5
  StrCmp $R7 $R3 found
  StrCpy $R7 $R1 1 ; - optimization can be removed if U know len needle=1
  StrCpy $R2 "$R2$R7"
  StrCpy $R1 $R1 $R6 1
  StrCmp $R1 "" done loop
found:
  StrCpy $R2 "$R2$R4"
  StrCpy $R1 $R1 $R6 $R5
  StrCmp $R1 "" done loop
done:
  StrCpy $R3 $R2
  Pop $R7
  Pop $R6
  Pop $R5
  Pop $R2
  Pop $R1
  Pop $R4
  Exch $R3
	
FunctionEnd

