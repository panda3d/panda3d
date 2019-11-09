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
;   INCLUDE_PYVER - version of Python that Panda was built with (eg. "2.7", "3.5-32")
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
!include "x64.nsh"

!define MUI_WELCOMEFINISHPAGE_BITMAP "panda-install.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "panda-install.bmp"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION runFunction
!define MUI_FINISHPAGE_RUN_TEXT "Visit the Panda3D Manual"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${SOURCE}/doc/LICENSE"
!insertmacro MUI_PAGE_DIRECTORY

!ifdef INCLUDE_PYVER
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE ConfirmPythonSelection
!endif
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

LicenseData "${SOURCE}/doc/LICENSE"

InstType "Auto (Recommended)"
InstType "Full"
InstType "Light"

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
LangString DESC_SecGroupPython ${LANG_ENGLISH} "Contains modules that provide Python support for Panda3D."
LangString DESC_SecPyShared ${LANG_ENGLISH} "Contains the common Python code used by the Panda3D Python bindings."
LangString DESC_SecPython ${LANG_ENGLISH} "Contains a ${REGVIEW}-bit copy of Python ${INCLUDE_PYVER} preconfigured to make use of Panda3D."
LangString DESC_SecEnsurePip ${LANG_ENGLISH} "Installs the pip package manager into the included Python installation."
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
!insertmacro !defineifexist HAVE_SAMPLES "${SOURCE}\samples"
!insertmacro !defineifexist HAVE_MAX_PLUGINS "${BUILT}\plugins\*.dlo"
!insertmacro !defineifexist HAVE_MAYA_PLUGINS "${BUILT}\plugins\*.mll"

!macro RemovePythonPath PYVER
    ReadRegStr $0 HKCU "Software\Python\PythonCore\${PYVER}\PythonPath\Panda3D" ""
    StrCmp $0 "$INSTDIR" 0 +2
    DeleteRegKey HKCU "Software\Python\PythonCore\${PYVER}\PythonPath\Panda3D"
!macroend

!macro PyBindingSection PYVER EXT_SUFFIX
    LangString DESC_SecPyBindings${PYVER} ${LANG_ENGLISH} "Contains the Python modules that allow use of Panda3D using a ${REGVIEW}-bit version of Python ${PYVER}."

    !insertmacro !defineifexist _present "${BUILT}\panda3d\core${EXT_SUFFIX}"
    !ifdef _present
    Section "${PYVER} bindings" SecPyBindings${PYVER}
        !if "${PYVER}" == "${INCLUDE_PYVER}"
            SectionIn 1 2 3
        !else
            !if "${PYVER}" == "2.7"
                SectionIn 1 2
            !else
                ; See .onInit function where this is dynamically enabled.
                SectionIn 2
            !endif
        !endif

        SetDetailsPrint both
        DetailPrint "Installing Panda3D bindings for Python ${PYVER}..."
        SetDetailsPrint listonly

        SetOutPath $INSTDIR\panda3d

        File /nonfatal /r "${BUILT}\panda3d\core${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\ai${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\direct${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\egg${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\fx${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\interrogatedb${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\physics${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\_rplight${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\skel${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\vision${EXT_SUFFIX}"
        File /nonfatal /r "${BUILT}\panda3d\vrpn${EXT_SUFFIX}"

        !ifdef HAVE_BULLET
            SectionGetFlags ${SecBullet} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipBulletPyd
            File /nonfatal /r "${BUILT}\panda3d\bullet${EXT_SUFFIX}"
            SkipBulletPyd:
        !endif

        !ifdef HAVE_ODE
            SectionGetFlags ${SecODE} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipODEPyd
            File /nonfatal /r "${BUILT}\panda3d\ode${EXT_SUFFIX}"
            SkipODEPyd:
        !endif

        !ifdef HAVE_PHYSX
            SectionGetFlags ${SecPhysX} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipPhysXPyd
            File /nonfatal /r "${BUILT}\panda3d\physx${EXT_SUFFIX}"
            SkipPhysXPyd:
        !endif

        !ifdef HAVE_ROCKET
            SectionGetFlags ${SecRocket} $R0
            IntOp $R0 $R0 & ${SF_SELECTED}
            StrCmp $R0 ${SF_SELECTED} 0 SkipRocketPyd
            File /nonfatal /r "${BUILT}\panda3d\rocket${EXT_SUFFIX}"
            SkipRocketPyd:
        !endif

        SetOutPath $INSTDIR\pandac\input
        File /r "${BUILT}\pandac\input\*"
        SetOutPath $INSTDIR\Pmw
        File /nonfatal /r /x CVS "${BUILT}\Pmw\*"
        SetOutPath $INSTDIR\panda3d.dist-info
        File /nonfatal /r "${BUILT}\panda3d.dist-info\*"

        !ifdef REGVIEW
        SetRegView ${REGVIEW}
        !endif

        ; Install a Panda3D path into the global PythonPath for this version
        ; of Python.
        WriteRegStr HKCU "Software\Python\PythonCore\${PYVER}\PythonPath\Panda3D" "" "$INSTDIR"
    SectionEnd
    !undef _present
    !endif
!macroend

Function runFunction
    ExecShell "open" "$SMPROGRAMS\${TITLE}\Panda3D Manual.lnk"
FunctionEnd

SectionGroup "Panda3D Libraries"
    Section "Core Libraries" SecCore
        SectionIn 1 2 3 RO

        SetShellVarContext current
        SetOverwrite try

        SetDetailsPrint both
        DetailPrint "Installing Panda3D libraries..."
        SetDetailsPrint listonly

        SetOutPath "$INSTDIR"
        File /nonfatal "${BUILT}\LICENSE"
        File /nonfatal "${BUILT}\ReleaseNotes"
        File /nonfatal "${BUILT}\pandaIcon.ico"

        SetOutPath $INSTDIR\etc
        File /r "${BUILT}\etc\*"

        SetOutPath $INSTDIR\bin
        File /r /x api-ms-win-*.dll /x ucrtbase.dll /x libpandagl.dll /x libpandadx9.dll /x cgD3D*.dll /x python*.dll /x libpandaode.dll /x libp3fmod_audio.dll /x fmodex*.dll /x libp3ffmpeg.dll /x av*.dll /x postproc*.dll /x swscale*.dll /x swresample*.dll /x NxCharacter*.dll /x cudart*.dll /x PhysX*.dll /x libpandaphysx.dll /x libp3rocket.dll /x boost_python*.dll /x Rocket*.dll /x _rocket*.pyd /x libpandabullet.dll /x OpenAL32.dll /x *_oal.dll /x libp3openal_audio.dll "${BUILT}\bin\*.dll"
        File /nonfatal /r "${BUILT}\bin\Microsoft.*.manifest"

        ; Before Windows 10, we need these stubs for the UCRT as well.
        ReadRegDWORD $0 HKLM "Software\Microsoft\Windows NT\CurrentVersion" "CurrentMajorVersionNumber"
        ${If} $0 < 10
            ClearErrors
            File /nonfatal /r "${BUILT}\bin\api-ms-win-*.dll"
            File /nonfatal "${BUILT}\bin\ucrtbase.dll"
        ${Endif}

        SetDetailsPrint both
        DetailPrint "Installing models..."
        SetDetailsPrint listonly

        SetOutPath $INSTDIR\models
        File /r /x CVS "${BUILT}\models\*"

        SetDetailsPrint both
        DetailPrint "Installing optional components..."
        SetDetailsPrint listonly

        RMDir /r "$SMPROGRAMS\${TITLE}"
        CreateDirectory "$SMPROGRAMS\${TITLE}"
    SectionEnd

    !ifdef HAVE_GL
    Section "OpenGL" SecOpenGL
        SectionIn 1 2 3 RO

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandagl.dll"
    SectionEnd
    !endif

    !ifdef HAVE_DX9
    Section "Direct3D 9" SecDirect3D9
        SectionIn 1 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandadx9.dll"
        File /nonfatal /r "${BUILT}\bin\cgD3D9.dll"
    SectionEnd
    !endif

    !ifdef HAVE_OPENAL
    Section "OpenAL Audio" SecOpenAL
        SectionIn 1 2 3

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3openal_audio.dll"
        File /nonfatal /r "${BUILT}\bin\OpenAL32.dll"
        File /nonfatal /r "${BUILT}\bin\*_oal.dll"
    SectionEnd
    !endif

    !ifdef HAVE_FMOD
    Section "FMOD Audio" SecFMOD
        SectionIn 1 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3fmod_audio.dll"
        File /r "${BUILT}\bin\fmodex*.dll"
    SectionEnd
    !endif

    !ifdef HAVE_FFMPEG
    Section "FFMpeg" SecFFMpeg
        SectionIn 1 2

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
        SectionIn 1 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandabullet.dll"
    SectionEnd
    !endif

    !ifdef HAVE_ODE
    Section "ODE Physics" SecODE
        SectionIn 1 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandaode.dll"
    SectionEnd
    !endif

    !ifdef HAVE_PHYSX
    Section "NVIDIA PhysX" SecPhysX
        ; Only enable in "Full"
        SectionIn 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libpandaphysx.dll"
        File /nonfatal /r "${BUILT}\bin\PhysX*.dll"
        File /nonfatal /r "${BUILT}\bin\NxCharacter*.dll"
        File /nonfatal /r "${BUILT}\bin\cudart*.dll"
    SectionEnd
    !endif

    !ifdef HAVE_ROCKET
    Section "libRocket GUI" SecRocket
        SectionIn 1 2

        SetOutPath "$INSTDIR\bin"
        File "${BUILT}\bin\libp3rocket.dll"
        File /nonfatal /r "${BUILT}\bin\Rocket*.dll"
        File /nonfatal /r "${BUILT}\bin\_rocket*.pyd"
        File /nonfatal /r "${BUILT}\bin\boost_python*.dll"
    SectionEnd
    !endif
SectionGroupEnd

Section "Tools and utilities" SecTools
    SectionIn 1 2 3

    SetDetailsPrint both
    DetailPrint "Installing utilities..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\bin"
    File /r /x deploy-stub.exe /x deploy-stubw.exe "${BUILT}\bin\*.exe"
    File /nonfatal /r "${BUILT}\bin\*.p3d"
    SetOutPath "$INSTDIR\NSIS"
    File /r /x CVS "${NSISDIR}\*"

    WriteRegStr HKCU "Software\Classes\Panda3D.Model" "" "Panda3D model/animation"
    WriteRegStr HKCU "Software\Classes\Panda3D.Model\DefaultIcon" "" "$INSTDIR\bin\pview.exe"
    WriteRegStr HKCU "Software\Classes\Panda3D.Model\shell" "" "open"
    WriteRegStr HKCU "Software\Classes\Panda3D.Model\shell\open\command" "" '"$INSTDIR\bin\pview.exe" -l "%1"'
    WriteRegStr HKCU "Software\Classes\Panda3D.Model\shell\compress" "" "Compress to .pz"
    WriteRegStr HKCU "Software\Classes\Panda3D.Model\shell\compress\command" "" '"$INSTDIR\bin\pzip.exe" "%1"'

    WriteRegStr HKCU "Software\Classes\Panda3D.Compressed" "" "Compressed file"
    WriteRegStr HKCU "Software\Classes\Panda3D.Compressed\DefaultIcon" "" "$INSTDIR\bin\pzip.exe"
    WriteRegStr HKCU "Software\Classes\Panda3D.Compressed\shell" "" "open"
    WriteRegStr HKCU "Software\Classes\Panda3D.Compressed\shell\open\command" "" '"$INSTDIR\bin\pview.exe" -l "%1"'
    WriteRegStr HKCU "Software\Classes\Panda3D.Compressed\shell\decompress" "" "Decompress"
    WriteRegStr HKCU "Software\Classes\Panda3D.Compressed\shell\decompress\command" "" '"$INSTDIR\bin\punzip.exe" "%1"'

    WriteRegStr HKCU "Software\Classes\Panda3D.Multifile" "" "Panda3D Multifile"
    WriteRegStr HKCU "Software\Classes\Panda3D.Multifile\DefaultIcon" "" "$INSTDIR\bin\multify.exe"
    WriteRegStr HKCU "Software\Classes\Panda3D.Multifile\shell" "" "open"
    WriteRegStr HKCU "Software\Classes\Panda3D.Multifile\shell\extract" "" "Extract here"
    WriteRegStr HKCU "Software\Classes\Panda3D.Multifile\shell\extract\command" "" '"$INSTDIR\bin\multify.exe" -xf "%1"'
SectionEnd

SectionGroup "Python modules" SecGroupPython
    Section "Shared code" SecPyShared
        SectionIn 1 2 3

        SetDetailsPrint both
        DetailPrint "Installing Panda3D shared Python modules..."
        SetDetailsPrint listonly

        SetOutPath $INSTDIR\direct\directscripts
        File /r /x CVS /x Opt?-Win32 "${BUILT}\direct\directscripts\*"
        SetOutPath $INSTDIR\direct
        File /r /x CVS /x Opt?-Win32 "${BUILT}\direct\*.py"

        Delete "$INSTDIR\panda3d.py"
        Delete "$INSTDIR\panda3d.pyc"
        Delete "$INSTDIR\panda3d.pyo"
        SetOutPath $INSTDIR\pandac
        File /r "${BUILT}\pandac\*.py"
        SetOutPath $INSTDIR\panda3d
        File /r "${BUILT}\panda3d\*.py"
    SectionEnd

    !insertmacro PyBindingSection 2.7 .pyd
    !if "${REGVIEW}" == "32"
        !insertmacro PyBindingSection 3.5-32 .cp35-win32.pyd
        !insertmacro PyBindingSection 3.6-32 .cp36-win32.pyd
        !insertmacro PyBindingSection 3.7-32 .cp37-win32.pyd
        !insertmacro PyBindingSection 3.8-32 .cp38-win32.pyd
        !insertmacro PyBindingSection 3.9-32 .cp39-win32.pyd
    !else
        !insertmacro PyBindingSection 3.5 .cp35-win_amd64.pyd
        !insertmacro PyBindingSection 3.6 .cp36-win_amd64.pyd
        !insertmacro PyBindingSection 3.7 .cp37-win_amd64.pyd
        !insertmacro PyBindingSection 3.8 .cp38-win_amd64.pyd
        !insertmacro PyBindingSection 3.9 .cp39-win_amd64.pyd
    !endif
SectionGroupEnd

!ifdef INCLUDE_PYVER
Section "Python ${INCLUDE_PYVER}" SecPython
    SectionIn 1 2 3

    !ifdef REGVIEW
    SetRegView ${REGVIEW}
    !endif

    SetDetailsPrint both
    DetailPrint "Installing Python ${INCLUDE_PYVER} interpreter (${REGVIEW}-bit)..."
    SetDetailsPrint listonly

    SetOutPath "$INSTDIR\bin"
    File /nonfatal "${BUILT}\bin\python*.dll"

    SetOutPath "$INSTDIR\python"
    File /r /x *.pdb "${BUILT}\python\*"

    SetDetailsPrint both
    DetailPrint "Adding registry keys for Python..."
    SetDetailsPrint listonly

    ; Check if a copy of Python is installed for this user.
    ReadRegStr $0 HKCU "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" ""
    StrCmp "$0" "$INSTDIR\python" RegPath 0
    StrCmp "$0" "" SkipFileCheck 0
    IfFileExists "$0\python.exe" AskRegPath 0
    SkipFileCheck:

    ; Check if a system-wide copy of Python is installed.
    ReadRegStr $0 HKLM "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" ""
    StrCmp "$0" "$INSTDIR\python" RegPath 0
    StrCmp "$0" "" RegPath 0
    IfFileExists "$0\python.exe" AskRegPath RegPath

    AskRegPath:
    MessageBox MB_YESNO|MB_ICONQUESTION \
        "You already have a copy of Python ${INCLUDE_PYVER} installed in:$\r$\n$0$\r$\n$\r$\nPanda3D installs its own copy of Python ${INCLUDE_PYVER}, which will install alongside your existing copy.  Would you like to make Panda's copy the default Python for your user account?" \
        IDNO SkipRegPath

    RegPath:
    WriteRegStr HKCU "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" "" "$INSTDIR\python"
    WriteRegStr HKCU "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" "ExecutablePath" "$INSTDIR\python\python.exe"
    SkipRegPath:

SectionEnd

Section "Install pip" SecEnsurePip
    SectionIn 1 2 3

    SetDetailsPrint both
    DetailPrint "Installing the pip package manager..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR
    nsExec::ExecToLog '"$INSTDIR\python\python.exe" -m ensurepip --default-pip'
    Pop $0
    DetailPrint "Command returned exit status $0"
SectionEnd
!endif

!macro MaybeEnablePyBindingSection PYVER
    !if "${INCLUDE_PYVER}" != "${PYVER}"
        !ifdef SecPyBindings${PYVER}
            ; Check if a copy of Python is installed for this user.
            Push $0
            ReadRegStr $0 HKCU "Software\Python\PythonCore\${PYVER}\InstallPath" ""
            StrCmp "$0" "" +2 0
            IfFileExists "$0\python.exe" Py${PYVER}Exists 0

            ; Check if a system-wide copy of Python is installed.
            ReadRegStr $0 HKLM "Software\Python\PythonCore\${PYVER}\InstallPath" ""
            StrCmp "$0" "" Py${PYVER}ExistsNot 0
            IfFileExists "$0\python.exe" Py${PYVER}Exists Py${PYVER}ExistsNot

            Py${PYVER}Exists:
            SectionSetFlags ${SecPyBindings${PYVER}} ${SF_SELECTED}
            SectionSetInstTypes ${SecPyBindings${PYVER}} 3

            Py${PYVER}ExistsNot:
            Pop $0
        !endif
    !endif
!macroend

Function .onInit
    ${If} ${REGVIEW} = 64
    ${AndIfNot} ${RunningX64}
        MessageBox MB_OK|MB_ICONEXCLAMATION "You are attempting to install the 64-bit version of Panda3D on a 32-bit version of Windows.  Please download and install the 32-bit version of Panda3D instead."
        Abort
    ${EndIf}

    !ifdef REGVIEW
    SetRegView ${REGVIEW}
    !endif

    ; We never check for 2.7; it is always enabled in Auto mode
    !if "${REGVIEW}" == "32"
        !insertmacro MaybeEnablePyBindingSection 3.5-32
        !insertmacro MaybeEnablePyBindingSection 3.6-32
        !insertmacro MaybeEnablePyBindingSection 3.7-32
        !insertmacro MaybeEnablePyBindingSection 3.8-32
        !insertmacro MaybeEnablePyBindingSection 3.9-32
    !else
        !insertmacro MaybeEnablePyBindingSection 3.5
        !insertmacro MaybeEnablePyBindingSection 3.6
        !insertmacro MaybeEnablePyBindingSection 3.7
        !insertmacro MaybeEnablePyBindingSection 3.8
        !insertmacro MaybeEnablePyBindingSection 3.9
    !endif
FunctionEnd

Function .onSelChange
    ; If someone selects any Python version, the "shared modules" must be on.
    ${If} ${SectionIsPartiallySelected} ${SecGroupPython}
        SectionGetFlags ${SecPyShared} $R0
        IntOp $R0 $R0 | ${SF_SELECTED}
        SectionSetFlags ${SecPyShared} $R0
    ${EndIf}

    !ifdef INCLUDE_PYVER
        ${If} ${SectionIsSelected} ${SecPython}
            !insertmacro SectionFlagIsSet ${SecEnsurePip} ${SF_RO} 0 SkipSelectEnsurePip
            !insertmacro SelectSection ${SecEnsurePip}
            SkipSelectEnsurePip:
            !insertmacro ClearSectionFlag ${SecEnsurePip} ${SF_RO}
        ${Else}
            !insertmacro UnselectSection ${SecEnsurePip}
            !insertmacro SetSectionFlag ${SecEnsurePip} ${SF_RO}
        ${EndIf}
    !endif
FunctionEnd

!ifdef INCLUDE_PYVER
Function ConfirmPythonSelection
    ; Check the current state of the "Python" section selection.
    SectionGetFlags ${SecPython} $R0
    IntOp $R1 $R0 & ${SF_SELECTED}

    ; Is the "Python" selection deselected?
    StrCmp $R1 ${SF_SELECTED} SkipCheck 0

    ; Maybe the user just doesn't want Python support at all?
    !insertmacro SectionFlagIsSet ${SecGroupPython} ${SF_PSELECTED} 0 SkipCheck

    !ifdef REGVIEW
    SetRegView ${REGVIEW}
    !endif

    ; Check for a user installation of Python.
    ReadRegStr $0 HKCU "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" ""
    StrCmp $0 "$INSTDIR\python" CheckSystemWidePython 0
    StrCmp $0 "" CheckSystemWidePython 0
    IfFileExists "$0\ppython.exe" CheckSystemWidePython 0
    IfFileExists "$0\python.exe" SkipCheck CheckSystemWidePython

    ; Check for a system-wide Python installation.
    CheckSystemWidePython:
    ReadRegStr $0 HKLM "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" ""
    StrCmp $0 "$INSTDIR\python" AskConfirmation 0
    StrCmp $0 "" AskConfirmation 0
    IfFileExists "$0\ppython.exe" AskConfirmation 0
    IfFileExists "$0\python.exe" SkipCheck AskConfirmation

    ; No compatible Python version found (that wasn't shipped as part
    ; of a different Panda3D build.)  Ask the user if he's sure about this.
    AskConfirmation:
    MessageBox MB_YESNO|MB_ICONQUESTION \
        "You do not appear to have a ${REGVIEW}-bit version of Python ${INCLUDE_PYVER} installed.  Are you sure you don't want Panda to install a compatible copy of Python?$\r$\n$\r$\nIf you choose Yes, you will not be able to do Python development with Panda3D until you install a ${REGVIEW}-bit version of Python and install the bindings for this version." \
        IDYES SkipCheck

    ; User clicked no, so re-enable the select box and abort.
    IntOp $R0 $R0 | ${SF_SELECTED}
    SectionSetFlags ${SecPython} $R0
    Abort

    SkipCheck:
FunctionEnd
!endif

Section "C++ support" SecHeadersLibs
    SectionIn 1 2

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
    SectionIn 1 2

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
    CreateShortCut "$SMPROGRAMS\${TITLE}\Panda3D Manual.lnk" "$INSTDIR\Manual.url" "" "$INSTDIR\pandaIcon.ico" 0 "" "" "Panda3D Manual"
    CreateShortCut "$SMPROGRAMS\${TITLE}\Panda3D Website.lnk" "$INSTDIR\Website.url" "" "$INSTDIR\pandaIcon.ico" 0 "" "" "Panda3D Website"
    CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Program Manual.lnk" "$INSTDIR\Samples.url" "" "$INSTDIR\pandaIcon.ico" 0 "" "" "Sample Program Manual"

    FindFirst $0 $1 $INSTDIR\samples\*
    loop:
        StrCmp $1 "" done
        StrCmp $1 "." next
        StrCmp $1 ".." next
        FindFirst $2 $3 $INSTDIR\samples\$1\*.py
        StrCmp $3 "" next
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
        WriteINIStr $INSTDIR\samples\$1\ManualPage.url "InternetShortcut" "URL" "https://www.panda3d.org/wiki/index.php/Sample_Programs:_$MANPAGE"
        CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE\Manual Page.lnk" "$INSTDIR\samples\$1\ManualPage.url" "" "$INSTDIR\pandaIcon.ico" 0 "" "" "Manual Entry on this Sample Program"
        CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE\View Source Code.lnk" "$INSTDIR\samples\$1"
        iloop:
            StrCmp $3 "" idone
            CreateShortCut "$SMPROGRAMS\${TITLE}\Sample Programs\$READABLE\Run $3.lnk" "$INSTDIR\python\python.exe" "-E $3" "$INSTDIR\pandaIcon.ico" 0 SW_SHOWMINIMIZED "" "Run $3"
            CreateShortCut "$INSTDIR\samples\$1\Run $3.lnk" "$INSTDIR\python\python.exe" "-E $3" "$INSTDIR\pandaIcon.ico" 0 SW_SHOWMINIMIZED "" "Run $3"
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
    SectionIn 1 2

    SetDetailsPrint both
    DetailPrint "Installing Autodesk 3ds Max plug-ins..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\plugins
    File /nonfatal /r "${BUILT}\plugins\*.dle"
    File /nonfatal /r "${BUILT}\plugins\*.dlo"
    File /nonfatal /r "${BUILT}\plugins\*.ms"
SectionEnd
!endif

!ifdef HAVE_MAYA_PLUGINS
Section "Maya plug-ins" SecMayaPlugins
    SectionIn 1 2

    SetDetailsPrint both
    DetailPrint "Installing Autodesk Maya plug-ins..."
    SetDetailsPrint listonly

    SetOutPath $INSTDIR\plugins
    File /nonfatal /r "${BUILT}\plugins\*.mll"
    File /nonfatal /r "${BUILT}\plugins\*.mel"
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

    SetOutPath $INSTDIR
    nsExec::ExecToLog '"$INSTDIR\python\python.exe" -m direct.directscripts.eggcacher --concise models samples'
    Pop $0
    DetailPrint "Command returned exit status $0"

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
    DetailPrint "Registering file type associations..."
    SetDetailsPrint listonly

    ; Even though we need the runtime to run these, we might as well tell
    ; Windows what this kind of file is.
    WriteRegStr HKCU "Software\Classes\.p3d" "" "Panda3D applet"
    WriteRegStr HKCU "Software\Classes\.p3d" "Content Type" "application/x-panda3d"
    WriteRegStr HKCU "Software\Classes\.p3d" "PerceivedType" "application"

    ; Register various model files
    WriteRegStr HKCU "Software\Classes\.egg" "" "Panda3D.Model"
    WriteRegStr HKCU "Software\Classes\.egg" "Content Type" "application/x-egg"
    WriteRegStr HKCU "Software\Classes\.egg" "PerceivedType" "gamemedia"
    WriteRegStr HKCU "Software\Classes\.bam" "" "Panda3D.Model"
    WriteRegStr HKCU "Software\Classes\.bam" "Content Type" "application/x-bam"
    WriteRegStr HKCU "Software\Classes\.bam" "PerceivedType" "gamemedia"
    WriteRegStr HKCU "Software\Classes\.pz" "" "Panda3D.Compressed"
    WriteRegStr HKCU "Software\Classes\.pz" "PerceivedType" "compressed"
    WriteRegStr HKCU "Software\Classes\.mf" "" "Panda3D.Multifile"
    WriteRegStr HKCU "Software\Classes\.mf" "PerceivedType" "compressed"
    WriteRegStr HKCU "Software\Classes\.prc" "" "inifile"
    WriteRegStr HKCU "Software\Classes\.prc" "Content Type" "text/plain"
    WriteRegStr HKCU "Software\Classes\.prc" "PerceivedType" "text"

    ; For convenience, if nobody registered .pyd, we will.
    ReadRegStr $0 HKCR "Software\Classes\.pyd" ""
    StrCmp $0 "" 0 +2
    WriteRegStr HKCU "Software\Classes\.pyd" "" "dllfile"

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
    Push "$INSTDIR\python;$INSTDIR\python\Scripts;$INSTDIR\bin"
    Call AddToPath

    # This is needed for the environment variable changes to take effect.
    DetailPrint "Broadcasting WM_WININICHANGE message..."
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=500

    # Now dump the log to disk.
    StrCpy $0 "$INSTDIR\install.log"
    Push $0
    Call DumpLog
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

    ReadRegStr $0 HKCU "Software\Classes\Panda3D.Model\DefaultIcon" ""
    StrCmp $0 "$INSTDIR\bin\pview.exe" 0 +3
    DeleteRegKey HKCU "Software\Classes\Panda3D.Model\DefaultIcon"
    DeleteRegKey HKCU "Software\Classes\Panda3D.Model\shell"

    ReadRegStr $0 HKCU "Software\Classes\Panda3D.Compressed\DefaultIcon" ""
    StrCmp $0 "$INSTDIR\bin\pzip.exe" 0 +3
    DeleteRegKey HKCU "Software\Classes\Panda3D.Compressed\DefaultIcon"
    DeleteRegKey HKCU "Software\Classes\Panda3D.Compressed\shell"

    ReadRegStr $0 HKCU "Software\Classes\Panda3D.Multifile\DefaultIcon" ""
    StrCmp $0 "$INSTDIR\bin\multify.exe" 0 +3
    DeleteRegKey HKCU "Software\Classes\Panda3D.Multifile\DefaultIcon"
    DeleteRegKey HKCU "Software\Classes\Panda3D.Multifile\shell"

    !ifdef INCLUDE_PYVER
        ReadRegStr $0 HKLM "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" ""
        StrCmp $0 "$INSTDIR\python" 0 +2
        DeleteRegKey HKLM "Software\Python\PythonCore\${INCLUDE_PYVER}"

        ReadRegStr $0 HKCU "Software\Python\PythonCore\${INCLUDE_PYVER}\InstallPath" ""
        StrCmp $0 "$INSTDIR\python" 0 +2
        DeleteRegKey HKCU "Software\Python\PythonCore\${INCLUDE_PYVER}"
    !endif

    !insertmacro RemovePythonPath 2.7
    !if "${REGVIEW}" == "32"
        !insertmacro RemovePythonPath 3.5-32
        !insertmacro RemovePythonPath 3.6-32
        !insertmacro RemovePythonPath 3.7-32
        !insertmacro RemovePythonPath 3.8-32
        !insertmacro RemovePythonPath 3.9-32
    !else
        !insertmacro RemovePythonPath 3.5
        !insertmacro RemovePythonPath 3.6
        !insertmacro RemovePythonPath 3.7
        !insertmacro RemovePythonPath 3.8
        !insertmacro RemovePythonPath 3.9
    !endif

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
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGroupPython} $(DESC_SecGroupPython)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPyShared} $(DESC_SecPyShared)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings2.7} $(DESC_SecPyBindings2.7)
  !if "${REGVIEW}" == "32"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.5-32} $(DESC_SecPyBindings3.5-32)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.6-32} $(DESC_SecPyBindings3.6-32)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.7-32} $(DESC_SecPyBindings3.7-32)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.8-32} $(DESC_SecPyBindings3.8-32)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.9-32} $(DESC_SecPyBindings3.9-32)
  !else
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.5} $(DESC_SecPyBindings3.5)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.6} $(DESC_SecPyBindings3.6)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.7} $(DESC_SecPyBindings3.7)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.8} $(DESC_SecPyBindings3.8)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyBindings3.9} $(DESC_SecPyBindings3.9)
  !endif
  !ifdef INCLUDE_PYVER
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPython} $(DESC_SecPython)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecEnsurePip} $(DESC_SecEnsurePip)
  !endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SecHeadersLibs} $(DESC_SecHeadersLibs)
  !ifdef HAVE_SAMPLES
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSamples} $(DESC_SecSamples)
  !endif
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
                ClearErrors
                ReadRegStr $1 HKCU "Environment" "PATH"

                ; If we reached an error, WATCH OUT.  Either this means that
                ; the registry key did not exist, or that it didn't fit in
                ; NSIS' string limit.  If the latter, we have to be very
                ; careful not to overwrite the user's PATH.
                IfErrors AddToPath_Error
                DetailPrint "Current PATH value is set to $1"
                StrCmp $1 "" AddToPath_NTAddPath

                ; Pull off the last character of the PATH string.  If it's a semicolon,
                ; we don't need to add another one, so jump to the section where we
                ; append the new PATH component(s).
                StrCpy $2 $1 1 -1
                StrCmp $2 ";" AddToPath_NTAddPath AddToPath_NTAddSemi

                AddToPath_Error:
                        DetailPrint "Encountered error reading PATH variable."
                        ; Does the variable exist?  If it doesn't, then the
                        ; error happened because we need to create the
                        ; variable.  If it does, then we failed to read it
                        ; because we reached NSIS' string limit.
                        StrCpy $3 0
                        AddToPath_loop:
                                EnumRegValue $4 HKCU "Environment" $3
                                StrCmp $4 "PATH" AddToPath_ExceedLimit
                                StrCmp $4 "" AddToPath_NTAddPath
                                IntOp $3 $3 + 1
                        Goto AddToPath_loop
                AddToPath_ExceedLimit:
                        MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Your PATH environment variable is too long! Please remove extraneous entries before proceeding. Panda3D needs to add the following the PATH so that the Panda3D utilities and libraries can be located correctly.$\n$\n$0$\n$\nIf you wish to add Panda3D to the path yourself, choose Ignore." IDIGNORE AddToPath_done IDRETRY AddToPath_NT
                        SetDetailsPrint both
                        DetailPrint "Cannot append to PATH - variable is likely too long."
                        SetDetailsPrint listonly
                        Abort
                AddToPath_NTAddSemi:
                        StrCpy $1 "$1;"
                        Goto AddToPath_NTAddPath
                AddToPath_NTAddPath:
                        StrCpy $0 "$1$0"
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
                        Push $0
                        StrLen $2 $0
                        ReadRegStr $1 HKCU "Environment" "PATH"
                        Push $1
                        Push $0
                        Call StrStr ; Find $0 in $1
                        Pop $0 ; pos of our dir
                        IntCmp $0 -1 unRemoveFromPath_NT_System
                                ; else, it is in path
                                StrCpy $3 $1 $0 ; $3 now has the part of the path before our dir
                                IntOp $2 $2 + $0 ; $2 now contains the pos after our dir in the path (';')
                                IntOp $2 $2 + 1 ; $2 now containts the pos after our dir and the semicolon.
                                StrLen $0 $1
                                StrCpy $1 $1 $0 $2
                                StrCpy $3 "$3$1"
                                WriteRegExpandStr HKCU "Environment" "PATH" $3

                unRemoveFromPath_NT_System:
                        Pop $0
                        StrLen $2 $0
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
                        Push $0
                        StrLen $2 $0
                        ReadRegStr $1 HKCU "Environment" "PATH"
                        Push $1
                        Push $0
                        Call un.StrStr ; Find $0 in $1
                        Pop $0 ; pos of our dir
                        IntCmp $0 -1 unRemoveFromPath_NT_System
                                ; else, it is in path
                                StrCpy $3 $1 $0 ; $3 now has the part of the path before our dir
                                IntOp $2 $2 + $0 ; $2 now contains the pos after our dir in the path (';')
                                IntOp $2 $2 + 1 ; $2 now containts the pos after our dir and the semicolon.
                                StrLen $0 $1
                                StrCpy $1 $1 $0 $2
                                StrCpy $3 "$3$1"
                                WriteRegExpandStr HKCU "Environment" "PATH" $3

                unRemoveFromPath_NT_System:
                        Pop $0
                        StrLen $2 $0
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

                unRemoveFromPath_done:
                        Pop $5
                        Pop $4
                        Pop $3
                        Pop $2
                        Pop $1
                        Pop $0
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

!ifndef LVM_GETITEMCOUNT
!define LVM_GETITEMCOUNT 0x1004
!endif

!ifndef LVM_GETITEMTEXT
!define LVM_GETITEMTEXT 0x102D
!endif

Function DumpLog
  Exch $5
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $6

  FindWindow $0 "#32770" "" $HWNDPARENT
  GetDlgItem $0 $0 1016
  StrCmp $0 0 exit
  FileOpen $5 $5 "w"
  StrCmp $5 "" exit
    SendMessage $0 ${LVM_GETITEMCOUNT} 0 0 $6
    System::Alloc ${NSIS_MAX_STRLEN}
    Pop $3
    StrCpy $2 0
    System::Call "*(i, i, i, i, i, i, i, i, i) i \
      (0, 0, 0, 0, 0, r3, ${NSIS_MAX_STRLEN}) .r1"
    loop: StrCmp $2 $6 done
      System::Call "User32::SendMessageA(i, i, i, i) i \
        ($0, ${LVM_GETITEMTEXT}, $2, r1)"
      System::Call "*$3(&t${NSIS_MAX_STRLEN} .r4)"
      FileWrite $5 "$4$\r$\n"
      IntOp $2 $2 + 1
      Goto loop
    done:
      FileClose $5
      System::Free $1
      System::Free $3
  exit:
    Pop $6
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
    Exch $5
FunctionEnd
