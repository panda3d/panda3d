//
// Config.Cygwin.pp
//
// This file defines some custom config variables for the Windows
// platform, when ppremake has been compiled using Cygwin.  It
// inherits most of its parameters from Config.Win32.pp.
//
//  *** UNCOMMENT ONE OF THE TWO OPTIONS BELOW FOR 32 OR 64 BIT BUILDS

// 32-bit
#include $[THISDIRPREFIX]Config.Win32.pp

// 64-bit
// #include $[THISDIRPREFIX]Config.Win64.pp
