#define INSTALL_SCRIPTS genPyCode.py

// If we're on Win32 without Cygwin, install the genPyCode.bat file;
// for all other platforms, install the genPyCode sh script.
#if $[eq $[PLATFORM],Win32]
  #define INSTALL_SCRIPTS $[INSTALL_SCRIPTS] genPyCode.bat
#else
  #define INSTALL_SCRIPTS $[INSTALL_SCRIPTS] genPyCode
#endif

#include $[THISDIRPREFIX]genPyCode.pp
