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

// Many of the HAVE_* variables are defined in terms of expressions
// based on the paths and library names, etc., defined above.  These
// are defined using the "defer" command, so that they are not
// evaluated right away, giving the user an opportunity to redefine
// the variables they depend on, or to redefine the HAVE_* variables
// themselves (you can explicitly define a HAVE_* variable to some
// nonempty string to force the package to be marked as installed).

// Is Python installed, and should Python interfaces be generated?  If
// Python is installed, which directory is it in?  (If the directory
// is someplace standard like /usr/include, you may leave it blank.)
#define PYTHON_IPATH /usr/local/include/python1.6
#define PYTHON_LPATH
#defer HAVE_PYTHON $[isdir $[PYTHON_IPATH]]

// Is NSPR installed, and where?
#define NSPR_IPATH /usr/local/mozilla/dist/*/include
#define NSPR_LPATH
#define NSPR_LIBS nspr3
#defer HAVE_NSPR $[libtest $[NSPR_LPATH],$[NSPR_LIBS]]

// Is VRPN installed, and where?
#define VRPN_IPATH
#define VRPN_LPATH
#defer HAVE_VRPN $[isdir $[VRPN_IPATH]]

// Is ZLIB installed, and where?
#define ZLIB_IPATH
#define ZLIB_LPATH
#define ZLIB_LIBS z
#defer HAVE_ZLIB $[libtest $[ZLIB_LPATH],$[ZLIB_LIBS]]

// Is the sox libst library installed, and where?
#define SOXST_IPATH
#define SOXST_LPATH
#define SOXST_LIBS st
#defer HAVE_SOXST $[libtest $[SOXST_LPATH],$[SOXST_LIBS]]

// Is OpenGL installed, and where?  This should include libGL as well
// as libGLU, if they are in different places.
#define GL_IPATH
#define GL_LPATH
#if $[eq $[PLATFORM],Win32]
  #define GL_LIBS \
     opengl32.lib glu32.lib winmm.lib kernel32.lib \
     oldnames.lib mswsock.lib ws2_32.lib \
     advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
#else
  #define GL_LIBS GL GLU
#endif
#defer HAVE_GL $[libtest $[GL_LPATH],$[GL_LIBS]]

// How about GLX?
#define GLX_IPATH
#define GLX_LPATH
#if $[eq $[PLATFORM],Win32]
  #defer HAVE_GLX
#else
  #defer HAVE_GLX $[HAVE_GL]
#endif

// Glut?
#define GLUT_IPATH
#define GLUT_LPATH
#define GLUT_LIBS glut
#defer HAVE_GLUT $[libtest $[GLUT_LPATH],$[GLUT_LIBS]]

// Should we try to build the WGL interface?
#define HAVE_WGL

// Should we try to build the SGI-specific glxdisplay?
#define HAVE_SGIGL

// Should we try to build the DirectX interface?
#define HAVE_DX

// Do you want to build the Renderman interface?
#define HAVE_RIB

// Is Mikmod installed?  How should we run the libmikmod-config program?
#define MIKMOD_CONFIG libmikmod-config
#defer HAVE_MIKMOD $[bintest $[MIKMOD_CONFIG]]

// Is Gtk-- installed?  How should we run the gtkmm-config program?
// This matters only to programs in PANDATOOL.
#define GTKMM_CONFIG gtkmm-config
#defer HAVE_GTKMM $[bintest $[GTKMM_CONFIG]]

// Is Maya installed?  This matters only to programs in PANDATOOL.
#define MAYA_LOCATION /usr/aw/maya2.5
#defer HAVE_MAYA $[isdir $[MAYA_LOCATION]]

// What additional libraries must we link with for network-dependent
// code?
#if $[eq $[PLATFORM],Win32]
  #define NET_LIBS ws2_32.lib
#endif

// What additional libraries must we link with for audio-dependent
// code?
#if $[eq $[PLATFORM],Win32]
  #define AUDIO_LIBS winmm.lib dsound.lib user32.lib ole32.lib dxguid.lib
#endif


//////////////////////////////////////////////////////////////////////
// There are also some additional variables that control specific
// compiler/platform features or characteristics, defined in the
// platform specific file Config.platform.pp.  Be sure to inspect
// these variables for correctness too.  As above, these are
// unnecessary when BUILD_TYPE is "autoconf".
//////////////////////////////////////////////////////////////////////
