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
#if $[< $[PPREMAKE_VERSION],1.00]
  #error You need at least ppremake version 1.00 to process this tree.
#endif

// What is the name and version of this source tree?
#if $[eq $[PACKAGE],]
  #define PACKAGE dtool
  #define VERSION 0.80
#endif

// Where should install DTOOL, specifically?
#if $[or $[CTPROJS],$[DTOOL]]
  // If we are presently attached, use the environment variable.
  // We define two variables: one for ourselves, which burns in the
  // current value of the DTOOL environment variable (so that any
  // attempt to install in this tree will install correctly, no
  // matter whether we are attached to a different DTOOL later by
  // mistake), and one for other trees to use, which expands to a
  // ordinary reference to the DTOOL environment variable, so
  // they will read from the right tree no matter which DTOOL they're
  // attached to.
  #define DTOOL_INSTALL $[DTOOL]
  #define DTOOL_INSTALL_OTHER $(DTOOL)
  #if $[eq $[DTOOL],]
    #error You seem to be attached to some trees, but not DTOOL!
  #endif
#else
  // Otherwise, if we are not attached, install in the standard place
  // (unless the user specifies otherwise).
  #defer DTOOL_INSTALL $[INSTALL_DIR]
  #defer DTOOL_INSTALL_OTHER $[INSTALL_DIR]
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
  #print Reading $[PPREMAKE_CONFIG]
  #include $[PPREMAKE_CONFIG]
#else
  #print Environment variable PPREMAKE_CONFIG not set; using defaults.
#endif

// Now evaluate all of our deferred variable definitions from
// Config.pp.
#set PYTHON_IPATH $[PYTHON_IPATH]
#set PYTHON_LPATH $[PYTHON_LPATH]
#set HAVE_PYTHON $[HAVE_PYTHON]

#set RAD_MSS_IPATH $[RAD_MSS_IPATH]
#set RAD_MSS_LPATH $[RAD_MSS_LPATH]
#set RAD_MSS_LIBS $[RAD_MSS_LIBS]
#set HAVE_RAD_MSS $[HAVE_RAD_MSS]

#set NSPR_IPATH $[NSPR_IPATH]
#set NSPR_LPATH $[NSPR_LPATH]
#set NSPR_LIBS $[NSPR_LIBS]
#set HAVE_NSPR $[HAVE_NSPR]

#set CRYPTO_IPATH $[CRYPTO_IPATH]
#set CRYPTO_LPATH $[CRYPTO_LPATH]
#set CRYPTO_LIBS $[CRYPTO_LIBS]
#set HAVE_CRYPTO $[HAVE_CRYPTO]

#set JPEG_IPATH $[JPEG_IPATH]
#set JPEG_LPATH $[JPEG_LPATH]
#set JPEG_LIBS $[JPEG_LIBS]
#set HAVE_JPEG $[HAVE_JPEG]

#set TIFF_IPATH $[TIFF_IPATH]
#set TIFF_LPATH $[TIFF_LPATH]
#set TIFF_LIBS $[TIFF_LIBS]
#set HAVE_TIFF $[HAVE_TIFF]

#set FFTW_IPATH $[FFTW_IPATH]
#set FFTW_LPATH $[FFTW_LPATH]
#set FFTW_LIBS $[FFTW_LIBS]
#set HAVE_FFTW $[HAVE_FFTW]

#set VRPN_IPATH $[VRPN_IPATH]
#set VRPN_LPATH $[VRPN_LPATH]
#set VRPN_LIBS $[VRPN_LIBS]
#set HAVE_VRPN $[HAVE_VRPN]

#set ZLIB_IPATH $[ZLIB_IPATH]
#set ZLIB_LPATH $[ZLIB_LPATH]
#set ZLIB_LIBS $[ZLIB_LIBS]
#set HAVE_ZLIB $[HAVE_ZLIB]

#set SOXST_IPATH $[SOXST_IPATH]
#set SOXST_LPATH $[SOXST_LPATH]
#set SOXST_LIBS $[SOXST_LIBS]
#set HAVE_SOXST $[HAVE_SOXST]

#set GL_IPATH $[GL_IPATH]
#set GL_LPATH $[GL_LPATH]
#set GL_LIBS $[GL_LIBS]
#set HAVE_GL $[HAVE_GL]

#set GLX_IPATH $[GLX_IPATH]
#set GLX_LPATH $[GLX_LPATH]
#set HAVE_GLX $[HAVE_GLX]

#set GLUT_IPATH $[GLUT_IPATH]
#set GLUT_LPATH $[GLUT_LPATH]
#set GLUT_LIBS $[GLUT_LIBS]
#set HAVE_GLUT $[HAVE_GLUT]

#set HAVE_WGL $[HAVE_WGL]

#set HAVE_SGIGL $[HAVE_SGIGL]

#set DX_IPATH $[DX_IPATH]
#set DX_LPATH $[DX_LPATH]
#set DX_LIBS $[DX_LIBS]
#set HAVE_DX $[HAVE_DX]

#set HAVE_RIB $[HAVE_RIB]

#set MIKMOD_CONFIG $[MIKMOD_CONFIG]
#set HAVE_MIKMOD $[HAVE_MIKMOD]

#set IPC_IPATH $[IPC_IPATH]
#set IPC_LPATH $[IPC_LPATH]
#set IPC_LIBS $[IPC_LIBS]
#set HAVE_IPC $[HAVE_IPC]

#set NET_IPATH $[NET_IPATH]
#set NET_LPATH $[NET_LPATH]
#set NET_LIBS $[NET_LIBS]
#set HAVE_NET $[HAVE_NET]

#set DO_PSTATS $[DO_PSTATS]

#set AUDIO_IPATH $[AUDIO_IPATH]
#set AUDIO_LPATH $[AUDIO_LPATH]
#set AUDIO_LIBS $[AUDIO_LIBS]
#set HAVE_AUDIO $[HAVE_AUDIO]

#set GTKMM_CONFIG $[GTKMM_CONFIG]
#set HAVE_GTKMM $[HAVE_GTKMM]

#set MAYA_LOCATION $[MAYA_LOCATION]
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

#if $[and $[HAVE_GTKMM],$[GTKMM_CONFIG]]
  #define cflags $[shell $[GTKMM_CONFIG] --cflags]
  #define libs $[shell $[GTKMM_CONFIG] --libs]

  #define GTKMM_CFLAGS $[filter-out -I%,$[cflags]]
  #define GTKMM_IPATH $[unique $[patsubst -I%,%,$[filter -I%,$[cflags]]]]
  #define GTKMM_LPATH $[unique $[patsubst -L%,%,$[filter -L%,$[libs]]]]
  #define GTKMM_LIBS $[patsubst -l%,%,$[filter -l%,$[libs]]]
#endif


// Finally, include the system configure file.
#include $[THISDIRPREFIX]pptempl/System.pp
