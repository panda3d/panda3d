// This directory contains the Python code necessary to interface with
// the browser plugin system at runtime.  It also contains the Python
// scripts to create and manage p3d files, which are the actual
// runtime applications, and packages, which are additional code and
// assets that can be downloaded at runtime by p3d files.

#if $[BUILD_P3D_SCRIPTS]

  // If the developer has asked to build the shell script to invoke
  // ppackage.py (or some other Python script in this directory), then
  // do so now.  These convenient scripts aren't built by default,
  // because usually ppackage.p3d etc. is a more reliable way to
  // invoke these applications.  However, there might be development
  // environments that don't have a ppackage.p3d available, in which
  // case it is convenient to have one or more of these scripts.

  #define INSTALL_SCRIPTS $[BUILD_P3D_SCRIPTS:%=%.py]

  // On Windows, we generate a batch file; on other platforms
  // (including Cygwin), we generate a sh script.

  #define install_dir $[$[upcase $[PACKAGE]]_INSTALL]
  #define install_bin_dir $[or $[INSTALL_BIN_DIR],$[install_dir]/bin]

  #define python $[PYTHON_COMMAND]
  #if $[USE_DEBUG_PYTHON]
    #define python $[PYTHON_DEBUG_COMMAND]
  #endif
  #if $[>= $[OPTIMIZE], 4]
    #define python $[python] -OO
  #endif

  #foreach scriptname $[BUILD_P3D_SCRIPTS]
    #if $[MAKE_BAT_SCRIPTS]
      #set INSTALL_SCRIPTS $[INSTALL_SCRIPTS] $[scriptname].bat
    #else
      #set INSTALL_SCRIPTS $[INSTALL_SCRIPTS] $[scriptname]
    #endif

    #if $[MAT_BAT_SCRIPTS]
  #output $[scriptname].bat notouch
@echo off
rem #### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]].
rem ################################# DO NOT EDIT ###########################

$[python] -u $[osfilename $[install_bin_dir]/$[scriptname].py] %1 %2 %3 %4 %5 %6 %7 %8 %9
  #end $[scriptname].bat

  #else  // MAKE_BAT_SCRIPTS

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

  #endif  // MAKE_BAT_SCRIPTS

  #end scriptname

#endif  // WANT_PACKAGE_SCRIPT




