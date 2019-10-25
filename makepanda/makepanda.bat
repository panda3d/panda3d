@echo off

REM
REM Check the Windows architecture and determine with Python
REM to use; 64-bit or 32-bit. Verify that we can find the 
REM 'makepanda' python script and the python interpreter.
REM If we can find both, then run 'makepanda'.
REM

if %PROCESSOR_ARCHITECTURE% == AMD64 (
  set suffix=-x64
) else (
  set suffix=
)

set thirdparty=thirdparty
if defined MAKEPANDA_THIRDPARTY set thirdparty=%MAKEPANDA_THIRDPARTY%

if exist %thirdparty%\win-python3.7%suffix%\python.exe (
  set pythondir=win-python3.7%suffix%
) else (
  set pythondir=win-python%suffix%
)

if not exist makepanda\makepanda.py goto :missing1
if not exist %thirdparty%\%pythondir%\python.exe goto :missing2
%thirdparty%\%pythondir%\python.exe makepanda\makepanda.py %*
if errorlevel 1 if x%1 == x--slavebuild exit 1
goto done

:missing1
  echo You need to change directory to the root of the panda source tree
  echo before invoking makepanda.  For further install instructions, read 
  echo the installation instructions in the file doc/INSTALL-MK.
  goto done

:missing2
  echo %thirdparty%
  echo You seem to be missing the 'thirdparty' directory.  You probably checked
  echo the source code out from GitHub.  The GitHub repository is
  echo missing the 'thirdparty' directory.  You will need to supplement the
  echo code by downloading the 'thirdparty' directory from www.panda3d.org
  goto done

:done
