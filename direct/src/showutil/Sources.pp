#define INSTALL_SCRIPTS packp3d.py runp3d.py

// Generate an appropriate script to invoke the above Python files.
// On Windows, we generate a batch file; on other platforms (including
// Cygwin), we generate a sh script.

#define install_dir $[$[upcase $[PACKAGE]]_INSTALL]
#define install_bin_dir $[or $[INSTALL_BIN_DIR],$[install_dir]/bin]

#define python $[PYTHON_COMMAND]
#if $[USE_DEBUG_PYTHON]
  #define python $[PYTHON_DEBUG_COMMAND]
#endif

#foreach scriptname packp3d runp3d
  #if $[eq $[PLATFORM],Win32]
    #set INSTALL_SCRIPTS $[INSTALL_SCRIPTS] $[scriptname].bat
  #else
    #set INSTALL_SCRIPTS $[INSTALL_SCRIPTS] $[scriptname]
  #endif

  #if $[eq $[PLATFORM],Win32]
#output $[scriptname].bat notouch
@echo off
rem #### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
rem ################################# DO NOT EDIT ###########################

$[python] -u $[osfilename $[install_bin_dir]/$[scriptname].py] %1 %2 %3 %4 %5 %6 %7 %8 %9
#end $[scriptname].bat

#else  // Win32

#output $[scriptname] binary notouch
#! /bin/sh
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
################################# DO NOT EDIT ###########################

#if $[CTPROJS]
# This script was generated while the user was using the ctattach
# tools.  That had better still be the case.
#if $[WINDOWS_PLATFORM]
$[python] -u `cygpath -w $DIRECT/built/bin/$[scriptname].py` "$@"
#else
$[python] -u $DIRECT/built/bin/$[scriptname].py "$@"
#endif
#else
$[python] -u '$[osfilename $[install_bin_dir]/$[scriptname].py]' "$@"
#endif
#end $[scriptname]

#endif  // Win32

#end scriptname
