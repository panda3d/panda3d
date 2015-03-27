; Panda3D installation script for the Nullsoft Installation System (NSIS).
; Jon Parise <jparise@cmu.edu>
; with Ben Johnson <bkj@andrew.cmu.edu>
; with Jason Pratt <pratt@andrew.cmu.edu>
; mangled by Josh Yelon <jyelon@andrew.cmu.edu>
; Heavily restructured by rdb

; Caller needs to define these variables:
;
;   COMPRESSOR    - either zlib or lzma
;   TITLE         - title                         (eg. "Panda3D SDK 1.9.0")
;   INSTALLDIR    - default install location      (eg. "C:\Panda3D-1.9.0-x64")
;   OUTFILE       - where to put the output file  (eg. "..\nsis-output.exe")
;
;   BUILT         - location of panda install tree.
;   SOURCE        - location of the panda source-tree if available, OR location of panda install tree.
;   PYVER         - version of Python that Panda was built with (ie, "2.7")
;   PYEXTRAS      - directory containing python extras, if any.
;   REGVIEW       - either 32 or 64, depending on the build architecture.
;

Name "${TITLE}"
InstallDir "${INSTALLDIR}"
OutFile "${OUTFILE}"

RequestExecutionLevel user

SetCompress auto
SetCompressor ${COMPRESSOR}

!include "MUI2.nsh"
!include "Sections.nsh"
!include "WinMessages.nsh"
!include "WordFunc.nsh"

!define MUI_WELCOMEFINISHPAGE_BITMAP "panda-install.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "panda-install.bmp"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION runFunction
!define MUI_FINISHPAGE_RUN_TEXT "Visit the Panda3D Manual"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "../doc/LICENSE"
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_PAGE_CUSTOMFUNCTION_LEAVE ConfirmPythonSelection
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

ShowInstDetails hide
ShowUninstDetails hide

LicenseData "${LICENSE}"

InstType "Full (Recommended)"
InstType "Minimal"

LangString DESC_SecCore ${LANG_ENGLISH} "The Panda3D core libraries, configuration files and models/textures that are needed to use Panda3D."
LangString DESC_SecOpenGL ${LANG_ENGLISH} "The OpenGL graphics back-end is the most well-supported renderer."
LangString DESC_SecDirect3D9 ${LANG_ENGLISH} "The optional Direct3D 9 renderer."
LangString DESC_SecOpenAL ${LANG_ENGLISH} "Support for playing audio via the OpenAL library.  You need either OpenAL or FMOD to be able to play audio."
LangString DESC_SecFMOD ${LANG_ENGLISH} "Support for decoding and playing audio via the FMOD Ex library.  You need either OpenAL or FMOD to be able to play audio."
LangString DESC_SecFFMpeg ${LANG_ENGLISH} "Support for decoding video and audio via the FFMpeg library.  Without this option, Panda3D will only be able to play .wav and .ogg audio files."
LangString DESC_SecBullet ${LANG_ENGLISH} "Support for the Bullet physics engine."
LangString DESC_SecODE ${LANG_ENGLISH} "Support for the Open Dynamics Engine to implement physics."
LangString DESC_SecPhysX ${LANG_ENGLISH} "Support for NVIDIA PhysX to implement physics."
LangString DESC_SecRocket ${LANG_ENGLISH} "Support for the libRocket GUI library.  This is an optional library that offers an HTML/CSS-like approach to creating user interfaces."
LangString DESC_SecTools ${LANG_ENGLISH} "Useful tools and model converters to help with Panda3D development.  Recommended."
LangString DESC_SecPyBindings ${LANG_ENGLISH} "Contains the Python modules that allow use of Panda3D using Python.  These will only work with a ${REGVIEW}-bit version of Python ${PYVER}."
LangString DESC_SecPython ${LANG_ENGLISH} "Contains a ${REGVIEW}-bit copy of Python ${PYVER} preconfigured to make use of Panda3D."
LangString DESC_SecHeadersLibs ${LANG_ENGLISH} "Headers and libraries needed for C++ development with Panda3D."
LangString DESC_SecSamples ${LANG_ENGLISH} "The sample programs demonstrate how to make Python applications with Panda3D."
LangString DESC_SecMaxPlugins ${LANG_ENGLISH} "Plug-ins for Autodesk 3ds Max (${REGVIEW}-bit) that can be used to export models to Panda3D."
LangString DESC_SecMayaPlugins ${LANG_ENGLISH} "Plug-ins and scripts for Autodesk Maya (${REGVIEW}-bit) that can be used to export models to Panda3D."

var READABLE
var MANPAGE

; See http://nsis.sourceforge.net/Check_if_a_file_exists_at_compile_time for documentation
!macro !defineifexist _VAR_NAME _FILE_NAME
    !tempfile _TEMPFILE
    !ifdef NSIS_WIN32_MAKENSIS
        ; Windows - cmd.exe
        !system 'if exist "${_FILE_NAME}" echo !define ${_VAR_NAME} > "${_TEMPFILE}"'
    !else
        ; Posix - sh
        !system 'if [ -e "${_FILE_NAME}" ]; then echo "!define ${_VAR_NAME}" > "${_TEMPFILE}"; fi'
    !endif
    !include '${_TEMPFILE}'
    !delfile '${_TEMPFILE}'
    !undef _TEMPFILE
!macroend

!insertmacro !defineifexist HAVE_GL "${BUILT}\bin\libpandagl.dll"
!insertmacro !defineifexist HAVE_DX9 "${BUILT}\bin\libpandadx9.dll"
!insertmacro !defineifexist HAVE_OPENAL "${BUILT}\bin\libp3openal_audio.dll"
!insertmacro !defineifexist HAVE_FMOD "${BUILT}\bin\libp3fmod_audio.dll"
!insertmacro !defineifexist HAVE_FFMPEG "${BUILT}\bin\libp3ffmpeg.dll"
!insertmacro !defineifexist HAVE_BULLET "${BUILT}\bin\libpandabullet.dll"
!insertmacro !defineifexist HAVE_ODE "${BUILT}\bin\libpandaode.dll"
!insertmacro !defineifexist HAVE_PHYSX "${BUILT}\bin\libpandaphysx.dll"
!insertmacro !defineifexist HAVE_ROCKET "${BUILT}\bin\libp3rocket.dll"
!insertmacro !defineifexist HAVE_PYTHON "${BUILT}\python"
!insertmacro !defineifexist HAVE_SAMPLES "${SOURCE}\samples"
!insertmacro !defineifexist HAVE_MAX_PLUGINS "${BUILT}\plugins\*.dlo"
!insertmacro !defineifexist HAVE_MAYA_PLUGINS "${BUILT}\plugins\*.mll"

Function runFunction
    ExecShell "open" "$SMPROGRAMS\${TITLE}\Panda3D Manual.lnk"
FunctionEnd

SectionGroup "Panda3D Libraries"
    Section "Core Libraries" SecCore
        SectionIn 1 2 RO

        SetShellVarContext current
        SetOverwrite try

        SetDetailsPrint both
        DetailPrint "Installing Panda3D libraries..."
        SetDetailsPrint listonly

        SetOutPath "$INSTDIR"
        File "${BUILT}\LICENSE"
        File /r /x CVS "${BUILT}\ReleaseNotes"
        SetOutPath $INSTDIR\bin
        File /r /x libpandagl.dll /x libpandadx9.dll /x cgD3D*.dll /x python*.dll /x libpandaode.dll /x libp3fmod_audio.dll /x fmodex*.dll /x libp3ffmpeg.dll /x av*.dll /x postproc*.dll /x swscale*.dll /x swresample*.dll /x NxCharacter*.dll /x cudart*.dll /x PhysX*.dll /x libpandaphysx.dll /x libp3rocket.dll /x boost_python*.dll /x Rocket*.dll /x _rocket*.pyd /x libpandabullet.dll /x OpenAL32.dll /x *_oal.dll /x libp3openal_audio.dll "${BUILT}\bin\*.dll"
        File /nonfatal /r "${BUILT}\bin\Microsoft.*.manifest"
        SetOutPath $INSTDIR\etc
        File /r "${BUILT}\etc\*"

        SetDetailsPrint both
        DetailPrint "Installing models..."
        SetDetailsPrint listonly

        SetOutPath $INSTDIR\models
        File /r /x CVS "${BUILT}\models\*"

        RMDir /r "$SMPROGRAMS\${TITLE}"
        CreateDirectory "$SMPROGRAMS\${TITLE}"
    SectionEnd

    !ifdef HAVE_GL
    Section "OpenGL" SecOpenGL
        SectionIn 1 2 RO

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandagl.dll"
    SectionEnd
    !endif

    !ifdef HAVE_DX9
    Section "Direct3D 9" SecDirect3D9
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandadx9.dll"
        File /nonfatal /r "${BUILT}\bin\cgD3D9.dll"
    SectionEnd
    !endif

    !ifdef HAVE_OPENAL
    Section "OpenAL Audio" SecOpenAL
        SectionIn 1 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3openal_audio.dll"
        File /nonfatal /r "${BUILT}\bin\OpenAL32.dll"
        File /nonfatal /r "${BUILT}\bin\*_oal.dll"
    SectionEnd
    !endif

    !ifdef HAVE_FMOD
    Section "FMOD Audio" SecFMOD
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3fmod_audio.dll"
        File /r "${BUILT}\bin\fmodex*.dll"
    SectionEnd
    !endif

    !ifdef HAVE_FFMPEG
    Section "FFMpeg" SecFFMpeg
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3ffmpeg.dll"
        File /nonfatal /r "${BUILT}\bin\av*.dll"
        File /nonfatal /r "${BUILT}\bin\swscale*.dll"
        File /nonfatal /r "${BUILT}\bin\swresample*.dll"
        File /nonfatal /r "${BUILT}\bin\postproc*.dll"
    SectionEnd
    !endif

    !ifdef HAVE_BULLET
    Section "Bullet Physics" SecBullet
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandabullet.dll"
    SectionEnd
    !endif

    !ifdef HAVE_ODE
    Section "ODE Physics" SecODE
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandaode.dll"
    SectionEnd
    !endif

    !ifdef HAVE_PHYSX
    Section "NVIDIA PhysX" SecPhysX
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandaphysx.dll"
        File /nonfatal /r "${BUILT}\bin\PhysX*.dll"
        File /nonfatal /r "${BUILT}\bin\NxCharacter*.dll"
        File /nonfatal /r "${BUILT}\bin\cudart*.dll"
    SectionEnd
    !endif

    !ifdef HAVE_ROCKET
    Section "libRocket GUI" SecRocket
        SectionIn 1

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3rocket.dll"
        File /nonfatal /r "${BUILT}\bin\Rocket*.dll"
        File /nonfatal /r "${BUILT}\bin\_rocket*.pyd"
        File /nonfatal /r "${BUILT}\bin\boost_python*.dll"
    SectionEnd
    !endif
SectionGroupEnd

Section "Tools and utilities" SecTools
    SectionIn 1 2

    SetDetailsPrint both
    DetailPrint "Installing utilities..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\bin"
    File /r "${BUILT}\bin\*.exe"
    File /nonfatal /r "${BUILT}\bin\*.p3d"
    SetOutPath "$INSTDIR\NSIS"
    File /r /x CVS "${NSISDIR}\*"
SectionEnd

SectionGroup "Python support"
    Section "Python bindings" SecPyBindings
        SectionIn 1 2

        SetDetailsPrint both
        DetailPrint "Installing Panda3D Python modules..."
        SetDetailsPrint listonly

        SetOutPath "$INSTDIR\bin"
        File /nonfatal /r "${BUILT}\bin\*.pyd"

        SetOutPath $INSTDIR\direct\directscripts
        File /r /x CVS /x Opt?-Win32 "${BUILT}\direct\directscripts\*"
        SetOutPath $INSTDIR\direct\filter
        File /r /x CVS /x Opt?-Win32 "${BUILT}\direct\filter\*.sha"
        SetOutPath $INSTDIR\direct
        File /r /x CVS /x Opt?-Win32 "${BUILT}\direct\*.py"

        Delete "$INSTDIR\panda3d.py"
        Delete "$INSTDIR\panda3d.pyc"
        Delete "$INSTDIR\panda3d.pyo"
        SetOutPath $INSTDIR\pandac
        File /r "${BUILT}\pandac\*.py"
        SetOutPath $INSTDIR\panda3d
        File /r "${BUILT}\panda3d\*.py"

        File /r /x bullet.pyd /x ode.pyd /x physx.pyd /x rocket.pyd "${BUILT}\panda3d\*.pyd"

        !ifdef HAVE_BULLET
            SectionGetFlags ${SecBullet} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipBulletPyd
            File /nonfatal /r "${BUILT}\panda3d\bullet.pyd"
            SkipBulletPyd:
        !endif

        !ifdef HAVE_ODE
            SectionGetFlags ${SecODE} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipODEPyd
            File /nonfatal /r "${BUILT}\panda3d\ode.pyd"
            SkipODEPyd:
        !endif

        !ifdef HAVE_PHYSX
            SectionGetFlags ${SecPhysX} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipPhysXPyd
            File /nonfatal /r "${BUILT}\panda3d\physx.pyd"
            SkipPhysXPyd:
        !endif

        !ifdef HAVE_ROCKET
            SectionGetFlags ${SecRocket} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipRocketPyd
            File /nonfatal /r "${BUILT}\panda3d\rocket.pyd"
            SkipRocketPyd:
        !endif

        SetOutPath $INSTDIR\pandac\input
        File /r "${BUILT}\pandac\input\*"
        SetOutPath $INSTDIR\Pmw
        File /r /x CVS "${BUILT}\Pmw\*"

        !ifdef REGVIEW
        SetRegView ${REGVIEW}
        !endif

        ; Check for a system-wide Python installation.
        ; We could check for a user installation of Python as well, but there
        ; is no distinction between 64-bit and 32-bit regviews in HKCU, so we
        ; can't guess whether it might be a compatible version.
        ReadRegStr $0 HKLM "Software\Python\PythonCore\${PYVER}\InstallPath" ""
        StrCmp $0 "$INSTDIR\python" SkipExternalPth 0
        StrCmp $0 "" SkipExternalPth 0
        IfFileExists "$0\ppython.exe" SkipExternalPth 0
        IfFileExists "$0\python.exe" 0 SkipExternalPth

        ; We're pretty sure this Python build is of the right architecture.
        MessageBox MB_YESNO|MB_ICONQUESTION \
            "Your system already has a copy of Python ${PYVER} installed in:$\r$\n$0$\r$\nWould you like to configure it to be able to use the Panda3D libraries?$\r$\nIf you choose no, you will only be able to use Panda3D's own copy of Python." \
            IDYES WriteExternalPth IDNO SkipExternalPth

        WriteExternalPth:
        FileOpen $1 "$0\Lib\site-packages\panda.pth" w
        FileWrite $1 "$INSTDIR$\r$\n"
        FileWrite $1 "$INSTDIR\bin$\r$\n"
        FileClose $1
        SkipExternalPth:
    SectionEnd

    !ifdef HAVE_PYTHON
    Section "Python ${PYVER}" SecPython
        SectionIn 1 2

        !ifdef REGVIEW
        SetRegView ${REGVIEW}
        !endif

        SetDetailsPrint both
        DetailPrint "Installing Python ${PYVER} (${REGVIEW}-bit)..."
        SetDetailsPrint listonly

        SetOutPath "$INSTDIR\bin"
        File /nonfatal "${BUILT}\bin\python*.dll"

        SetOutPath "$INSTDIR\python"
        File /r "${BUILT}\python\*"

        !ifdef PYEXTRAS
        SetOutPath "$INSTDIR\python\lib"
        File /nonfatal /r "${PYEXTRAS}\*"
        !endif

        SetDetailsPrint both
        DetailPrint "Adding registry keys for Python..."
        SetDetailsPrint listonly

        ; Check if a copy of Python is installed for this user.
        ReadRegStr $0 HKCU "Software\Python\PythonCore\${PYVER}\InstallPath" ""
        StrCmp "$0" "$INSTDIR\python" RegPath 0
        StrCmp "$0" "" SkipFileCheck 0
        IfFileExists "$0\python.exe" AskRegPath 0
        SkipFileCheck:

        ; Check if a system-wide copy of Python is installed.
        ReadRegStr $0 HKLM "Software\Python\PythonCore\${PYVER}\InstallPath" ""
        StrCmp "$0" "$INSTDIR\python" RegPath 0
        StrCmp "$0" "" RegPath 0
        IfFileExists "$0\python.exe" AskRegPath RegPath

        AskRegPath:
        MessageBox MB_YESNO|MB_ICONQUESTION \
            "You already have a copy of Python ${PYVER} installed in:$\r$\n$0$\r$\n$\r$\nPanda3D installs its own copy of Python ${PYVER}, which will install alongside your existing copy.  Would you like to make Panda's copy the default Python for your user account?" \
            IDNO SkipRegPath

        RegPath:
        WriteRegStr HKCU "Software\Python\PythonCore\${PYVER}\InstallPath" "" "$INSTDIR\python"
        SkipRegPath:

    SectionEnd
    !endif
SectionGroupEnd

Function ConfirmPythonSelection
    ; Check the current state of the "Python" section selection.
    SectionGetFlags ${SecPython} $R0
    IntOp $R1 $R0 & ${SF_SELECTED}

    ; Is the "Python" selection deselected?
    StrCmp $R1 ${SF_SELECTED} SkipCheck 0

    ; Maybe the user just doesn't want Python support at all?
    SectionGetFlags ${SecPyBindings} $R1
    IntOp $R1 $R1 & ${SF_SELECTED}
    StrCmp $R1 ${SF_SELECTED} 0 SkipCheck

    !ifdef REGVIEW
    SetRegView ${REGVIEW}
    !endif

    ; Check for a user installation of Python.
    ReadRegStr $0 HKCU "Software\Python\PythonCore\${PYVER}\InstallPath" ""
    StrCmp $0 "$INSTDIR\python" CheckSystemWidePython 0
    StrCmp $0 "" CheckSystemWidePython 0
    IfFileExists "$0\ppython.exe" CheckSystemWidePython 0
    IfFileExists "$0\python.exe" SkipCheck CheckSystemWidePython

    ; Check for a system-wide Python installation.
    CheckSystemWidePython:
    ReadRegStr $0 HKLM "Software\Python\PythonCore\${PYVER}\InstallPath" ""
    StrCmp $0 "$INSTDIR\python" AskConfirmation 0
    StrCmp $0 "" AskConfirmation 0
    IfFileExists "$0\ppython.exe" AskConfirmation 0
    IfFileExists "$0\python.exe" SkipCheck AskConfirmation

    ; No compatible Python version found (that wasn't shipped as part
    ; of a different Panda3D build.)  Ask the user if he's sure about this.
    AskConfirmation:
    MessageBox MB_YESNO|MB_ICONQUESTION \
        "You do not appear to have a ${REGVIEW}-bit version of Python ${PYVER} installed.  Are you sure you don't want Panda to install a compatible copy of Python?$\r$\n$\r$\nIf you choose Yes, you will not be able to do Python development with Panda3D until you install a ${REGVIEW}-bit version of Python ${PYVER} and manually configure it to be able to use Panda3D." \
        IDYES SkipCheck

    ; User clicked no, so re-enable the select box and abort.
    IntOp $R0 $R0 | ${SF_SELECTED}
    SectionSetFlags ${SecPython} $R0
    Abort

    SkipCheck:
FunctionEnd

Section "C++ support" SecHeadersLibs
    SectionIn 1

    SetDetailsPrint both
    DetailPrint "Installing header files..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\include
    File /r /x *.exp "${BUILT}\include\*"

    SetDetailsPrint both
    DetailPrint "Installing library archives..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\lib
    File /r /x *.exp "${BUILT}\lib\*"
SectionEnd

!ifdef HAVE_SAMPLES
Section "Sample programs" SecSamples
    SectionIn 1

    ; Necessary for proper start menu shortcut installation
    SetShellVarContext current

    SetDetailsPrint both
    DetailPrint "Installing sample programs..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\samples
    File /nonfatal /r /x CVS "${SOURCE}\samples\*"

    SetDetailsPrint both
    DetailPrint "Creating shortcuts..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR
    WriteINIStr $INSTDIR\Website.url "InternetShortcut" "URL" "https://www.panda3d.org/"
    WriteINIStr $INSTDIR\Manual.url "InternetShortcut" "URL" "https://www.panda3d.org/manual/index.php"
    WriteINIStr $INSTDIR\Samples.url "InternetShortcut" "URL" "https://www.panda3d.org/manual/index.php/Sample_Programs_in_the_Distribution"
    SetOutPath $INSTDIR
    CreateShortCut "$SMPROGRAMS\${TITLE}\Panda3D Manual.lnk" "$INSTDIR\Manual.url" "" "$INSTDIR\bin\eggcacher.exe" 0 "" "" "Panda3D Manual"
    CreateShortCut "$SMPROGRAMS\${TITLE}\Panda3D Website.lnk" "$INSTDIR\Website.url" "" "$INSTDIR\bin\eggcacher.exe" 0 "" "" "Panda3D Website"
    CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Program Manual.lnk" "$INSTDIR\Samples.url" "" "$INSTDIR\bin\eggcacher.exe" 0 "" "" "Sample Program Manual"

    FindFirst $0 $1 $INSTDIR\samples\*
    loop:
        StrCmp $1 "" done
        StrCmp $1 "." next
        StrCmp $1 ".." next
        Push $1
        Push "-"
        Push " "
        Call StrRep
        Call Capitalize
        Pop $R0
        StrCpy $READABLE $R0
        Push $1
        Push "-"
        Push "_"
        Call StrRep
        Pop $R0
        StrCpy $MANPAGE $R0
        DetailPrint "Creating shortcuts for sample program $READABLE"
        CreateDirectory "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE"
        SetOutPath $INSTDIR\samples\$1
        WriteINIStr $INSTDIR\samples\$1\ManualPage.url "InternetShortcut" "URL" "http://panda3d.org/wiki/index.php/Sample_Programs:_$MANPAGE"
        CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE\Manual Page.lnk" "$INSTDIR\samples\$1\ManualPage.url" "" "$INSTDIR\bin\eggcacher.exe" 0 "" "" "Manual Entry on this Sample Program"
        CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE\View Source Code.lnk" "$INSTDIR\samples\$1"
        FindFirst $2 $3 $INSTDIR\samples\$1\*.py
        iloop:
            StrCmp $3 "" idone
            CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE\Run $3.lnk" "$INSTDIR\python\python.exe" "-E $3" "$INSTDIR\bin\eggcacher.exe" 0 SW_SHOWMINIMIZED "" "Run $3"
            CreateShortCut "$INSTDIR\samples\$1\Run $3.lnk" "$INSTDIR\python\python.exe" "-E $3" "$INSTDIR\bin\eggcacher.exe" 0 SW_SHOWMINIMIZED "" "Run $3"
            FindNext $2 $3
            goto iloop
        idone:
    next:
        FindNext $0 $1
        Goto loop
    done:
SectionEnd
!endif

!ifdef HAVE_MAX_PLUGINS
Section "3ds Max plug-ins" SecMaxPlugins
    SectionIn 1 3

    SetDetailsPrint both
    DetailPrint "Installing Autodesk 3ds Max plug-ins..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\plugins
    File /nonfatal /r "${BUILT}\plugins\*.dle"
    File /nonfatal /r "${BUILT}\plugins\*.dlo"
    File "${SOURCE}\doc\INSTALLING-PLUGINS.TXT"
SectionEnd
!endif

!ifdef HAVE_MAYA_PLUGINS
Section "Maya plug-ins" SecMayaPlugins
    SectionIn 1 3

    SetDetailsPrint both
    DetailPrint "Installing Autodesk Maya plug-ins..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\plugins
    File /nonfatal /r "${BUILT}\plugins\*.mll"
    File /nonfatal /r "${BUILT}\plugins\*.mel"
    File /nonfatal /r "${BUILT}\plugins\*.ms"
    File "${SOURCE}\doc\INSTALLING-PLUGINS.TXT"
SectionEnd
!endif

Section -post
    !ifdef REGVIEW
    SetRegView ${REGVIEW}
    !endif

    ; Run eggcacher.  We can't do this in SecCore because we haven't
    ; installed eggcacher at that point yet.
    SetDetailsPrint both
    DetailPrint "Preloading .egg files into the model cache..."
    SetDetailsPrint listonly

    ; We need to set the $PATH for eggcacher.
    SetOutPath $INSTDIR
    ReadEnvStr $R0 "PATH"
    StrCpy $R0 "$INSTDIR\python;$INSTDIR\bin;$R0"
    System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("PATH", R0).r2'

    nsExec::ExecToLog '"$INSTDIR\bin\eggcacher.exe" --concise models samples'

    SetDetailsPrint both
    DetailPrint "Writing the uninstaller ..."
    SetDetailsPrint listonly

    Delete "$INSTDIR\uninst.exe"
    WriteUninstaller "$INSTDIR\uninst.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "DisplayName" "${TITLE}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}" "UninstallString" '"$INSTDIR\uninst.exe"'
    SetOutPath $INSTDIR
    CreateShortcut "$SMPROGRAMS\${TITLE}\Uninstall ${TITLE}.lnk" "$INSTDIR\uninst.exe" ""

    SetDetailsPrint both
    DetailPrint "Adding directories to system PATH..."
    SetDetailsPrint listonly

    # Add the "bin" directory to the PATH.
    Push "$INSTDIR\python"
    Call RemoveFromPath
    Push "$INSTDIR\python\Scripts"
    Call RemoveFromPath
    Push "$INSTDIR\bin"
    Call RemoveFromPath
    Push "$INSTDIR\python"
    Call AddToPath
    Push "$INSTDIR\python\Scripts"
    Call AddToPath
    Push "$INSTDIR\bin"
    Call AddToPath

    # This is needed for the environment variable changes to take effect.
    DetailPrint "Broadcasting WM_WININICHANGE message..."
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=500

SectionEnd

Section Uninstall
    SetDetailsPrint listonly

    SetShellVarContext current
    !ifdef REGVIEW
    SetRegView ${REGVIEW}
    !endif

    SetDetailsPrint both
    DetailPrint "Removing registry entries..."
    SetDetailsPrint listonly

    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${TITLE}"

    ReadRegStr $0 HKLM "Software\Python\PythonCore\${PYVER}\InstallPath" ""
    StrCmp $0 "$INSTDIR\python" 0 SkipUnRegHKLM
    DeleteRegKey HKLM "Software\Python\PythonCore\${PYVER}"
    SkipUnRegHKLM:

    ReadRegStr $0 HKCU "Software\Python\PythonCore\${PYVER}\InstallPath" ""
    StrCmp $0 "$INSTDIR\python" 0 SkipUnRegHKCU
    DeleteRegKey HKCU "Software\Python\PythonCore\${PYVER}"
    SkipUnRegHKCU:

    SetDetailsPrint both
    DetailPrint "Deleting files..."
    SetDetailsPrint listonly

    Delete "$INSTDIR\uninst.exe"
    RMDir /r "$INSTDIR"

    SetDetailsPrint both
    DetailPrint "Removing Start Menu entries..."
    SetDetailsPrint listonly

    SetShellVarContext current
    RMDir /r "$SMPROGRAMS\${TITLE}"
    SetShellVarContext all
    RMDir /r "$SMPROGRAMS\${TITLE}"

    SetDetailsPrint both
    DetailPrint "Removing entries from PATH..."
    SetDetailsPrint listonly

    Push "$INSTDIR\python"
    Call un.RemoveFromPath
    Push "$INSTDIR\python\Scripts"
    Call un.RemoveFromPath
    Push "$INSTDIR\bin"
    Call un.RemoveFromPath

    # This is needed for the environment variable changes to take effect.
    DetailPrint "Broadcasting WM_WININICHANGE message..."
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=500

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
  !ifdef HAVE_GL
    !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenGL} $(DESC_SecOpenGL)
  !endif
  !ifdef HAVE_DX9
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDirect3D9} $(DESC_SecDirect3D9)
  !endif
  !ifdef HAVE_OPENAL
    !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenAL} $(DESC_SecOpenAL)
  !endif
  !ifdef HAVE_FMOD
    !insertmacro MUI_DESCRIPTION_TEXT ${SecFMOD} $(DESC_SecFMOD)
  !endif
  !ifdef HAVE_FFMPEG
    !insertmacro MUI_DESCRIPTION_TEXT ${SecFFMpeg} $(DESC_SecFFMpeg)
  !endif
  !ifdef HAVE_BULLET
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBullet} $(DESC_SecBullet)
  !endif
  !ifdef HAVE_ODE
    !insertmacro MUI_DESCRIPTION_TEXT ${SecODE} $(DESC_SecODE)
  !endif
  !ifdef HAVE_PHYSX
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPhysX} $(DESC_SecPhysX)
  !endif
  !ifdef HAVE_ROCKET
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRocket} $(DESC_SecRocket)
  !endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTools} $(DESC_SecTools)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings} $(DESC_SecPyBindings)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPython} $(DESC_SecPython)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecHeadersLibs} $(DESC_SecHeadersLibs)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSamples} $(DESC_SecSamples)
  !ifdef HAVE_MAX_PLUGINS
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMaxPlugins} $(DESC_SecMaxPlugins)
  !endif
  !ifdef HAVE_MAYA_PLUGINS
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMayaPlugins} $(DESC_SecMayaPlugins)
  !endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END

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
Function StrStr
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

; Capitalizes the first letter of every word.
Function Capitalize
        Exch $R0
        Push $0
        Push $1
        Push $2

        StrCpy $0 0

        capNext:
        ; Grab the next character.
        StrCpy $1 $R0 1 $0
        StrCmp $1 '' end

        ; Capitalize it.
        ${StrFilter} '$1' '+eng' '' '' $1
        ${StrFilter} '$1' '+rus' '' '' $1

        ; Splice it into the string.
        StrCpy $2 $R0 $0
        IntOp $0 $0 + 1
        StrCpy $R0 $R0 '' $0
        StrCpy $R0 '$2$1$R0'

        ; Keep looping through the characters until we find a
        ; delimiter or reach the end of the string.
        loop:
        StrCpy $1 $R0 1 $0
        IntOp $0 $0 + 1
        StrCmp $1 '' end
        StrCmp $1 ' ' capNext
        StrCmp $1 '_' capNext
        StrCmp $1 '-' capNext
        StrCmp $1 '(' capNext
        StrCmp $1 '[' capNext
        Goto loop

        end:
        Pop $2
        Pop $1
        Pop $0
        Exch $R0
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

        DetailPrint "Adding to PATH: $0"

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
                Pop $3
                Pop $2
                Pop $1
                Pop $0
FunctionEnd

; From: http://nsis.sourceforge.net/archive/viewpage.php?pageid=91
Function RemoveFromPath
        Exch $0
        Push $1
        Push $2
        Push $3
        Push $4
        Push $5
        Call IsNT
        Pop $1

        DetailPrint "Removing from PATH: $0"

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
                        Call IsUserAdmin
                        Pop $5
                        StrCmp $5 1 0 NotAdmin
                                ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "PATH"
                                Push $1
                                Push $0
                                Call StrStr ; Find $0 in $1
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
                                        Goto unRemoveFromPath_done


                        NotAdmin:
                                ReadRegStr $1 HKCU "Environment" "PATH"
                                Push $1
                                Push $0
                                Call StrStr ; Find $0 in $1
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

                unRemoveFromPath_done:
                        Pop $5
                        Pop $4
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

                NotAdmin:
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

                NotAdmin:
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
