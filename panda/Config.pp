//
// Config.pp
//
// This file defines certain configuration variables that are written
// into the various make scripts.  It is processed by ppremake (along
// with the Sources.pp files in each of the various directories) to
// generate build scripts appropriate to each environment.
//
// ppremake is capable of generating generic Unix autoconf/automake
// style build scripts, as well as makefiles customized for SGI's
// MipsPRO compiler or for Microsoft's Visual C++.  It can also
// generate Microsoft Developer's Studio project files directly.  In
// principle, it can be extended to generate suitable build script
// files for any number of different build environments.
//
// All of these build scripts can be tuned for a particular
// environment via this file.  This is the place for the user to
// specify which external packages are installed and where, or to
// enable or disable certain optional features.  However, it is
// suggested that rather than modify this file directly, you create a
// custom file in your home directory and there redefine whatever
// variables are appropriate, and set the environment variable
// PPREMAKE_CONFIG to refer to it.  In this way, you can easily get an
// updated source tree (including a new Config.pp) without risking
// accidentally losing your customizations.  This also avoids having
// to redefine the same variables in different packages (for instance,
// in dtool and in panda).
//
// If you *do* decide to make changes directly to this file, you
// should also comment out the line near the bottom that includes the
// file $[TOPDIRPREFIX]Config.$[PLATFORM].pp, to avoid stomping on the
// changes you make.
//
// The syntax in this file resembles some hybrid between C++
// preprocessor declarations and GNU make variables.  This is the same
// syntax used in the various ppremake system configure files; it's
// designed to be easy to use as a macro language to generate
// makefiles and their ilk.
// 

// What kind of build scripts are we generating?  This selects a
// suitable template file from the ppremake system files.  The
// allowable choices, at present, are:
//
//  autoconf  - Generate configure.in and a series of Makefile.am files,
//              suitable for using with autoconf/automake.  Not quite
//              there yet.
//  stopgap   - Generate original Cary-style Makefile/Makefile.install/etc.
//              files, to ease transition to the new system.
//
#define BUILD_TYPE stopgap

// Define the directory in which the system ppremake files are
// installed.
#define PPREMAKE_DIR /usr/local/panda/share


// In which directory should this package be installed when you type
// "make install"?  This has no meaning when BUILD_TYPE is "stopgap".
#define INSTALL_DIR /usr/local/panda


// What level of compiler optimization/debug symbols should we build?
// The various optimize levels are defined as follows:
//
//   1 - No compiler optimizations, full debug symbols
//   2 - Full compiler optimizations, full debug symbols
//         (if the compiler supports this)
//   3 - Full compiler optimizations, no debug symbols
//   4 - Full optimizations, no debug symbols, and asserts removed
//
// Setting this has no effect when BUILD_TYPE is "stopgap".  In this
// case, the compiler optimizations are selected by setting the
// environment variable OPTIMIZE accordingly at compile time.
#define OPTIMIZE 1



////////////////////////////////////////////////////////////////////
// The remaining variables are considered only if BUILD_TYPE is not
// "autoconf".  (Autoconf can determine these directly.)
////////////////////////////////////////////////////////////////////

// NOTE: In the following, to indicate "yes" to a yes/no question,
// define the variable to be a nonempty string.  To indicate "no",
// define the variable to be an empty string.

// Is Python installed, and should Python interfaces be generated?  If
// Python is installed, which directory is it in?  (If the directory
// is someplace standard like /usr/include, you may leave it blank.)
#define HAVE_PYTHON 1
#define PYTHON_INCLUDE /usr/local/include/python1.6
#define PYTHON_LIB

// Is NSPR installed, and where?
#define HAVE_NSPR 1
#define NSPR_INCLUDE /usr/local/mozilla/dist/*/include
#define NSPR_LIB

// Is VRPN installed, and where?
#define HAVE_VRPN
#define VRPN_INCLUDE
#define VRPN_LIB

// Is ZLIB installed, and where?
#define HAVE_ZLIB 1
#define ZLIB_INCLUDE
#define ZLIB_LIB

// Is the sox libst library installed, and where?
#define HAVE_SOXST 1
#define SOXST_INCLUDE
#define SOXST_LIB

// Is OpenGL installed, and where?
#define HAVE_GL 1
#define GL_INCLUDE
#define GL_LIB
#define GLU_INCLUDE
#define GLU_LIB

// How about GLX?
#define HAVE_GLX 1
#define GLX_INCLUDE
#define GLX_LIB

// Glut?
#define HAVE_GLUT
#define GLUT_INCLUDE
#define GLUT_LIB

// Should we try to build the WGL interface?
#define HAVE_WGL

// Should we try to build the DirectX interface?
#define HAVE_DX

// Do you want to build the Renderman interface?
#define HAVE_RIB

// Is Mikmod installed?
#define HAVE_MIKMOD
#define MIKMOD_CFLAGS
#define MIKMOD_INCLUDE
#define MIKMOD_LIB


//////////////////////////////////////////////////////////////////////
// There are also some additional variables that control specific
// compiler/platform features or characteristics, defined in the
// platform specific file Config.platform.pp.  Be sure to inspect
// these variables for correctness too.  As above, these are
// unnecessary when BUILD_TYPE is "autoconf".
//////////////////////////////////////////////////////////////////////
#include $[TOPDIRPREFIX]Config.$[PLATFORM].pp


// Also pull in whatever package-specific variables there may be.
#include $[TOPDIRPREFIX]Package.pp


// If the environment variable PPREMAKE_CONFIG is set, it points to a
// user-customized Config.pp file, for instance in the user's home
// directory.  This file might redefine any of the variables defined
// above.
#if $[ne $[PPREMAKE_CONFIG],]
  #include $[PPREMAKE_CONFIG]
#endif


// Finally, include the system configure file.
#include $[PPREMAKE_DIR]/System.pp
