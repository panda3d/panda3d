//
// Package.pp
//
// This file defines certain configuration variables that are to be
// written into the various make scripts.  It is processed by ppremake
// (along with the Sources.pp files in each of the various
// directories) to generate build scripts appropriate to each
// environment.
//
// This is the package-specific file, which should be at the top of
// every source hierarchy.  It generally gets the ball rolling, and is
// responsible for explicitly including all of the relevent Config.pp
// files.


// What is the name and version of this source tree?
#if $[eq $[PACKAGE],]
  #define PACKAGE dtool
  #define VERSION 0.80
#endif


// Pull in the package-level Config file.  This contains a lot of
// configuration variables that the user might want to fine-tune.
#include $[THISDIRPREFIX]Config.pp

// Also get the platform-specific config file.  This defines a few
// more variables that are more likely to be platform-dependent and
// are less likely to be directly modified by the user.
#include $[THISDIRPREFIX]Config.$[PLATFORM].pp

// If the environment variable PPREMAKE_CONFIG is set, it points to a
// user-customized Config.pp file, for instance in the user's home
// directory.  This file might redefine any of the variables defined
// above.
#if $[ne $[PPREMAKE_CONFIG],]
  #include $[PPREMAKE_CONFIG]
#endif

// Now evaluate all of our deferred variable definitions from
// Config.pp.
#set HAVE_PYTHON $[HAVE_PYTHON]
#set HAVE_NSPR $[HAVE_NSPR]
#set HAVE_VRPN $[HAVE_VRPN]
#set HAVE_ZLIB $[HAVE_ZLIB]
#set HAVE_SOXST $[HAVE_SOXST]
#set HAVE_GL $[HAVE_GL]
#set HAVE_GLX $[HAVE_GLX]
#set HAVE_GLUT $[HAVE_GLUT]
#set HAVE_WGL $[HAVE_WGL]
#set HAVE_SGIGL $[HAVE_SGIGL]
#set HAVE_DX $[HAVE_DX]
#set HAVE_RIB $[HAVE_RIB]
#set HAVE_MIKMOD $[HAVE_MIKMOD]
#set HAVE_NET $[HAVE_NET]
#set HAVE_AUDIO $[HAVE_AUDIO]
#set HAVE_GTKMM $[HAVE_GTKMM]
#set HAVE_MAYA $[HAVE_MAYA]


// Now infer a few more variables based on what was defined.
#if $[and $[HAVE_MIKMOD],$[MIKMOD_CONFIG]]
  #define cflags $[shell $[MIKMOD_CONFIG] --cflags]
  #define libs $[shell $[MIKMOD_CONFIG] --libs]

  #define MIKMOD_CFLAGS $[filter-out -I%,$[cflags]]
  #define MIKMOD_IPATH $[unique $[patsubst -I%,%,$[filter -I%,$[cflags]]]]
  #define MIKMOD_LPATH $[unique $[patsubst -L%,%,$[filter -L%,$[libs]]]]
  #define MIKMOD_LIBS $[patsubst -l%,%,$[filter -l%,$[libs]]]
#endif

// Now infer a few more variables based on what was defined.
#if $[and $[HAVE_GTKMM],$[GTKMM_CONFIG]]
  #define cflags $[shell $[GTKMM_CONFIG] --cflags]
  #define libs $[shell $[GTKMM_CONFIG] --libs]

  #define GTKMM_CFLAGS $[filter-out -I%,$[cflags]]
  #define GTKMM_IPATH $[unique $[patsubst -I%,%,$[filter -I%,$[cflags]]]]
  #define GTKMM_LPATH $[unique $[patsubst -L%,%,$[filter -L%,$[libs]]]]
  #define GTKMM_LIBS $[patsubst -l%,%,$[filter -l%,$[libs]]]
#endif


// Finally, include the system configure file.
#include $[PPREMAKE_DIR]/System.pp
