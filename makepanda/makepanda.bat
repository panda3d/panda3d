@echo off

REM
REM Verify that we can find the 'makepanda' python script
REM and the python interpreter.  If we can find both, then
REM run 'makepanda'.
REM

if not exist makepanda\makepanda.py goto :missing1
if not exist thirdparty\win-python\python.exe goto :missing2
thirdparty\win-python\python.exe makepanda\makepanda.py %*
if errorlevel 1 if x%1 == x--slavebuild exit 1
goto done

:missing1
  echo You need to change directory to the root of the panda source tree
  echo before invoking makepanda.  For further install instructions, read 
  echo the installation instructions in the file doc/INSTALL-MK.
  goto done

:missing2
  echo You seem to be missing the 'thirdparty' directory.  You probably checked
  echo the source code out from sourceforge.  The sourceforge repository is
  echo missing the 'thirdparty' directory.  You will need to supplement the
  echo code by downloading the 'thirdparty' directory from panda3d.etc.cmu.edu
  goto done

:done
