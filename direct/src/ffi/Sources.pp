#define INSTALL_SCRIPTS genPyCode.py
#define INSTALL_MODULES panda3d.py

// If we're on Win32 without Cygwin, install the genPyCode.bat file;
// for all other platforms, install the genPyCode sh script.
#if $[MAKE_BAT_SCRIPTS]
  #define INSTALL_SCRIPTS $[INSTALL_SCRIPTS] genPyCode.bat
#else
  #define INSTALL_SCRIPTS $[INSTALL_SCRIPTS] genPyCode
#endif

#include $[THISDIRPREFIX]genPyCode.pp
