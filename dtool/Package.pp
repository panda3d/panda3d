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

// Check the version of ppremake in use.
#if $[< $[PPREMAKE_VERSION],1.11]
  #error You need at least ppremake version 1.11 to process this tree.
#endif

// Get the current version info for Panda.
#include $[THISDIRPREFIX]PandaVersion.pp
#defer PANDA_MAJOR_VERSION $[word 1,$[PANDA_VERSION]]
#defer PANDA_MINOR_VERSION $[word 2,$[PANDA_VERSION]]
#defer PANDA_SEQUENCE_VERSION $[word 3,$[PANDA_VERSION]]
#defer PANDA_VERSION_STR $[PANDA_MAJOR_VERSION].$[PANDA_MINOR_VERSION].$[PANDA_SEQUENCE_VERSION]$[if $[not $[OFFICIAL_VERSION]],c]
#defer PANDA_VERSION_SYMBOL panda_version_$[PANDA_MAJOR_VERSION]_$[PANDA_MINOR_VERSION]_$[PANDA_SEQUENCE_VERSION]$[if $[not $[OFFICIAL_VERSION]],c]

// What is the name of this source tree?
#if $[eq $[PACKAGE],]
  #define PACKAGE dtool
#endif

// Where should we install DTOOL, specifically?
#if $[DTOOL_INSTALL]
  #define DTOOL_INSTALL $[unixfilename $[DTOOL_INSTALL]]
#elif $[or $[CTPROJS],$[DTOOL]]
  // If we are presently attached, use the environment variable.
  // We define two variables: one for ourselves, which burns in the
  // current value of the DTOOL environment variable (so that any
  // attempt to install in this tree will install correctly, no
  // matter whether we are attached to a different DTOOL later by
  // mistake), and one for other trees to use, which expands to a
  // ordinary reference to the DTOOL environment variable, so
  // they will read from the right tree no matter which DTOOL they're
  // attached to.
  #set DTOOL $[unixfilename $[DTOOL]]
  #define DTOOL_INSTALL $[DTOOL]/built
  #if $[eq $[DTOOL],]
    #error You seem to be attached to some trees, but not DTOOL!
  #endif
#else
  // Otherwise, if we are not attached, install in the standard place
  // (unless the user specifies otherwise).
  #defer DTOOL_INSTALL $[unixfilename $[INSTALL_DIR]]
#endif


// These variables tell ppremake how to interpret the contents of the
// PLATFORM variable, and help it to control the effects of functions
// like $[os] and $[isfullpath].

// True if we are building on some flavor of Windows.
#define WINDOWS_PLATFORM $[or $[eq $[PLATFORM],Win32],$[eq $[PLATFORM],Cygwin]]

// True if we are building on some flavor of OS X.
#define OSX_PLATFORM $[or $[eq $[PLATFORM],Darwin],$[eq $[PLATFORM],osx]]

// True if we are building on some flavor of Unix.
#define UNIX_PLATFORM $[and $[not $[WINDOWS_PLATFORM]],$[not $[OSX_PLATFORM]]]



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
  #define PPREMAKE_CONFIG $[unixfilename $[PPREMAKE_CONFIG]]
  #print Reading $[PPREMAKE_CONFIG] (referred to by PPREMAKE_CONFIG)
  #include $[PPREMAKE_CONFIG]

#elif $[wildcard $[unixfilename $[INSTALL_DIR]]/Config.pp]
  // If the PPREMAKE_CONFIG variable is not, but there exists a
  // Config.pp in the compiled-in INSTALL_DIR, use that one by default.
  #define PPREMAKE_CONFIG $[unixfilename $[INSTALL_DIR]]/Config.pp
  #print Reading $[PPREMAKE_CONFIG] (referred to by INSTALL_DIR, because PPREMAKE_CONFIG is empty)
  #include $[PPREMAKE_CONFIG]

#else
  // Otherwise, just carry on without it.
  #print Environment variable PPREMAKE_CONFIG not set; using defaults.
#endif

// Now evaluate all of our deferred variable definitions from
// Config.pp.
#set PYTHON_IPATH $[unixfilename $[PYTHON_IPATH]]
#set PYTHON_LPATH $[unixfilename $[PYTHON_LPATH]]
#set PYTHON_FPATH $[unixfilename $[PYTHON_FPATH]]
#set PYTHON_FRAMEWORK $[unixfilename $[PYTHON_FRAMEWORK]]
#set HAVE_PYTHON $[HAVE_PYTHON]

#set NSPR_IPATH $[unixfilename $[NSPR_IPATH]]
#set NSPR_LPATH $[unixfilename $[NSPR_LPATH]]
#set NSPR_LIBS $[NSPR_LIBS]
#set HAVE_NSPR $[HAVE_NSPR]

#set SSL_IPATH $[unixfilename $[SSL_IPATH]]
#set SSL_LPATH $[unixfilename $[SSL_LPATH]]
#set SSL_LIBS $[SSL_LIBS]
#set HAVE_SSL $[HAVE_SSL]

#set JPEG_IPATH $[unixfilename $[JPEG_IPATH]]
#set JPEG_LPATH $[unixfilename $[JPEG_LPATH]]
#set JPEG_LIBS $[JPEG_LIBS]
#set HAVE_JPEG $[HAVE_JPEG]

#set PNG_IPATH $[unixfilename $[PNG_IPATH]]
#set PNG_LPATH $[unixfilename $[PNG_LPATH]]
#set PNG_LIBS $[PNG_LIBS]
#set HAVE_PNG $[HAVE_PNG]

#set TIFF_IPATH $[unixfilename $[TIFF_IPATH]]
#set TIFF_LPATH $[unixfilename $[TIFF_LPATH]]
#set TIFF_LIBS $[TIFF_LIBS]
#set HAVE_TIFF $[HAVE_TIFF]

#set FFTW_IPATH $[unixfilename $[FFTW_IPATH]]
#set FFTW_LPATH $[unixfilename $[FFTW_LPATH]]
#set FFTW_LIBS $[FFTW_LIBS]
#set HAVE_FFTW $[HAVE_FFTW]

#set CG_IPATH $[unixfilename $[CG_IPATH]]
#set CG_LPATH $[unixfilename $[CG_LPATH]]
#set CG_LIBS $[CG_LIBS]
#set HAVE_CG $[HAVE_CG]

#set CGGL_IPATH $[unixfilename $[CGGL_IPATH]]
#set CGGL_LPATH $[unixfilename $[CGGL_LPATH]]
#set CGGL_LIBS $[CGGL_LIBS]
#set HAVE_CGGL $[HAVE_CGGL]

#set VRPN_IPATH $[unixfilename $[VRPN_IPATH]]
#set VRPN_LPATH $[unixfilename $[VRPN_LPATH]]
#set VRPN_LIBS $[VRPN_LIBS]
#set HAVE_VRPN $[HAVE_VRPN]

#set HELIX_IPATH $[unixfilename $[HELIX_IPATH]]
#set HELIX_LPATH $[unixfilename $[HELIX_LPATH]]
#set HELIX_LIBS $[HELIX_LIBS]
#set HAVE_HELIX $[HAVE_HELIX]

#set ZLIB_IPATH $[unixfilename $[ZLIB_IPATH]]
#set ZLIB_LPATH $[unixfilename $[ZLIB_LPATH]]
#set ZLIB_LIBS $[ZLIB_LIBS]
#set HAVE_ZLIB $[HAVE_ZLIB]

#set GL_IPATH $[unixfilename $[GL_IPATH]]
#set GL_LPATH $[unixfilename $[GL_LPATH]]
#set GL_LIBS $[GL_LIBS]
#set HAVE_GL $[HAVE_GL]

#set MESA_IPATH $[unixfilename $[MESA_IPATH]]
#set MESA_LPATH $[unixfilename $[MESA_LPATH]]
#set MESA_LIBS $[MESA_LIBS]
#set MESA_MGL $[MESA_MGL]
#set HAVE_MESA $[HAVE_MESA]

#set CHROMIUM_IPATH $[unixfilename $[CHROMIUM_IPATH]]
#set CHROMIUM_LPATH $[unixfilename $[CHROMIUM_LPATH]]
#set CHROMIUM_LIBS $[CHROMIUM_LIBS]
#set HAVE_CHROMIUM $[HAVE_CHROMIUM]

#set GLX_IPATH $[unixfilename $[GLX_IPATH]]
#set GLX_LPATH $[unixfilename $[GLX_LPATH]]
#set HAVE_GLX $[HAVE_GLX]

#set HAVE_WGL $[HAVE_WGL]

#set HAVE_SGIGL $[HAVE_SGIGL]

#set DX_IPATH $[unixfilename $[DX_IPATH]]
#set DX_LPATH $[unixfilename $[DX_LPATH]]
#set DX_LIBS $[DX_LIBS]
#set HAVE_DX $[HAVE_DX]

#set HAVE_THREADS $[HAVE_THREADS]

#set NET_IPATH $[unixfilename $[NET_IPATH]]
#set NET_LPATH $[unixfilename $[NET_LPATH]]
#set NET_LIBS $[NET_LIBS]
#set HAVE_NET $[HAVE_NET]

#set DO_PSTATS $[DO_PSTATS]

#set RAD_MSS_IPATH $[unixfilename $[RAD_MSS_IPATH]]
#set RAD_MSS_LPATH $[unixfilename $[RAD_MSS_LPATH]]
#set RAD_MSS_LIBS $[RAD_MSS_LIBS]
#set HAVE_RAD_MSS $[HAVE_RAD_MSS]

#set FMOD_IPATH $[unixfilename $[FMOD_IPATH]]
#set FMOD_LPATH $[unixfilename $[FMOD_LPATH]]
#set FMOD_LIBS $[FMOD_LIBS]
#set HAVE_FMOD $[HAVE_FMOD]

#set CHROMIUM_IPATH $[unixfilename $[CHROMIUM_IPATH]]
#set CHROMIUM_LPATH $[unixfilename $[CHROMIUM_LPATH]]
#set CHROMIUM_LIBS $[CHROMIUM_LIBS]
#set HAVE_CHROMIUM $[HAVE_CHROMIUM]

#set GTKMM_CONFIG $[GTKMM_CONFIG]
#set HAVE_GTKMM $[HAVE_GTKMM]

#set FREETYPE_CONFIG $[FREETYPE_CONFIG]
#set HAVE_FREETYPE $[HAVE_FREETYPE]
#set FREETYPE_CFLAGS $[FREETYPE_CFLAGS]
#set FREETYPE_IPATH $[unixfilename $[FREETYPE_IPATH]]
#set FREETYPE_LPATH $[unixfilename $[FREETYPE_LPATH]]
#set FREETYPE_LIBS $[FREETYPE_LIBS]


#set MAYA_LOCATION $[unixfilename $[MAYA_LOCATION]]
#set HAVE_MAYA $[HAVE_MAYA]

#set SOFTIMAGE_LOCATION $[unixfilename $[SOFTIMAGE_LOCATION]]
#set HAVE_SOFTIMAGE $[HAVE_SOFTIMAGE]


// Now infer a few more variables based on what was defined.
#if $[and $[HAVE_GTKMM],$[GTKMM_CONFIG]]
  #define cflags $[shell $[GTKMM_CONFIG] --cflags]
  #define libs $[shell $[GTKMM_CONFIG] --libs]

  #define GTKMM_CFLAGS $[filter-out -I%,$[cflags]]
  #define GTKMM_IPATH $[unique $[patsubst -I%,%,$[filter -I%,$[cflags]]]]
  #define GTKMM_LPATH $[unique $[patsubst -L%,%,$[filter -L%,$[libs]]]]
  #define GTKMM_LIBS $[patsubst -l%,%,$[filter -l%,$[libs]]]
#endif

#if $[and $[HAVE_FREETYPE],$[FREETYPE_CONFIG]]
  #define cflags $[shell $[FREETYPE_CONFIG] --cflags]
  #define libs $[shell $[FREETYPE_CONFIG] --libs]

  #define FREETYPE_CFLAGS $[filter-out -I%,$[cflags]]
  #define FREETYPE_IPATH $[unique $[patsubst -I%,%,$[filter -I%,$[cflags]]]]
  #define FREETYPE_LPATH $[unique $[patsubst -L%,%,$[filter -L%,$[libs]]]]
  #define FREETYPE_LIBS $[patsubst -l%,%,$[filter -l%,$[libs]]]
#endif


// Finally, include the system configure file.
#include $[THISDIRPREFIX]pptempl/System.pp
