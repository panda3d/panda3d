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



// Check the version of ppremake in use.  This is temporary until
// everyone gets up to at least 0.50.  After that, the test in dtool
// will suffice.
#if $[not $[>= $[PPREMAKE_VERSION],0.50]]
  #error You need at least ppremake version 0.50 to process this tree.
#endif


// What is the name and version of this source tree?
#if $[eq $[PACKAGE],]
  #define PACKAGE direct
  #define VERSION 0.80
#endif


// Where should we find the PANDA source directory?
#if $[or $[CTPROJS],$[PANDA]]
  // If we are presently attached, use the environment variable.
  #define PANDA_SOURCE $[PANDA]
  #if $[eq $[PANDA],]
    #error You seem to be attached to some trees, but not PANDA!
  #endif
#else
  // Otherwise, if we are not attached, we guess that the source is a
  // sibling directory to this source root.
  #define PANDA_SOURCE $[standardize $[TOPDIR]/../panda]
#endif

// Where should we install DIRECT?
#if $[or $[CTPROJS],$[DIRECT]]
  #define DIRECT_INSTALL $[DIRECT]
  #define DIRECT_INSTALL_OTHER $(DIRECT)
  #if $[eq $[DIRECT],]
    #error You seem to be attached to some trees, but not DIRECT!
  #endif
#else
  #defer DIRECT_INSTALL $[INSTALL_DIR]
  #defer DIRECT_INSTALL_OTHER $[INSTALL_DIR]
#endif


// Define the inter-tree dependencies.
#define NEEDS_TREES $[NEEDS_TREES] panda


// Also get the PANDA Package file and everything that includes.
#if $[not $[isfile $[PANDA_SOURCE]/Package.pp]]
  #error PANDA source directory not found!  Are you attached properly?
#endif

#include $[PANDA_SOURCE]/Package.pp
