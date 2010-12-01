@echo off

REM
REM Verify that we can find the 'makedocs' python script
REM and the python interpreter.  If we can find both, then
REM run 'makedocs'.
REM

set thirdparty=thirdparty
if defined MAKEPANDA_THIRDPARTY set thirdparty=%MAKEPANDA_THIRDPARTY%

if not exist makepanda\makedocs.py goto :missing1
if not exist %thirdparty%\win-python\python.exe goto :missing2
%thirdparty%\win-python\python.exe makepanda\makedocs.py %*
goto done

:missing1
  echo You need to change directory to the root of the panda source tree
  echo before invoking makedocs.
  goto done

:missing2
  echo You seem to be missing the 'thirdparty' directory.  You probably checked
  echo the source code out from sourceforge.  The sourceforge repository is
  echo missing the 'thirdparty' directory.  You will need to supplement the
  echo code by downloading the 'thirdparty' directory from www.panda3d.org
  goto done

:done
