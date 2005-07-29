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
  #define PACKAGE dmodels
  #define VERSION 0.80
#endif


// Where should we find the DIRECT source directory?
#if $[or $[CTPROJS],$[DIRECT]]
  // If we are presently attached, use the environment variable.
  #define DIRECT_SOURCE $[DIRECT]
  #if $[eq $[DIRECT],]
    #error You seem to be attached to some trees, but not DIRECT!
  #endif
#else
  // Otherwise, if we are not attached, we guess that the source is a
  // sibling directory to this source root.
  #define DIRECT_SOURCE $[standardize $[TOPDIR]/../direct]
#endif

// Where should we install DMODELS?
#if $[DMODELS_INSTALL]
  #define DMODELS_INSTALL $[unixfilename $[DMODELS_INSTALL]]
#elif $[or $[CTPROJS],$[DMODELS]]
  #define DMODELS_INSTALL $[DMODELS]/built
  #if $[eq $[DMODELS],]
    #error You seem to be attached to some trees, but not DMODELS!
  #endif
#else
  #defer DMODELS_INSTALL $[INSTALL_DIR]
#endif


// Define the inter-tree dependencies.
#define NEEDS_TREES $[NEEDS_TREES] direct


// Also get the DIRECT Package file and everything that includes.
#if $[not $[isfile $[DIRECT_SOURCE]/Package.pp]]
  #printvar DIRECT_SOURCE
  #error DIRECT source directory not found from dmodels!  Are you attached properly?
#endif

#include $[DIRECT_SOURCE]/Package.pp

// Define some global variables for this tree.
#define FLT2EGG_OPTS -no -uo ft
