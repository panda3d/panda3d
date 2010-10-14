//
// Config.Cygwin.pp
//
// This file defines some custom config variables for the Windows
// platform, when ppremake has been compiled using Cygwin.  It
// inherits most of its parameters from Config.Win32.pp.
//

// Note: if you are building for 64-bit Windows, you should configure
// ppremake with the "Cygwin64" platform name instead of "Cygwin".

#include $[THISDIRPREFIX]Config.Win32.pp
