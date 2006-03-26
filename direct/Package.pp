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
  #define PACKAGE direct
  #define VERSION 0.80
#endif


// Where should we find the PANDA source directory?
#if $[PANDA_SOURCE]
  #define PANDA_SOURCE $[unixfilename $[PANDA_SOURCE]]
#elif $[or $[CTPROJS],$[PANDA]]
  // If we are presently attached, use the environment variable.
  #define PANDA_SOURCE $[unixfilename $[PANDA]]
  #if $[eq $[PANDA],]
    #error You seem to be attached to some trees, but not PANDA!
  #endif
#else
  // Otherwise, if we are not attached, we guess that the source is a
  // sibling directory to this source root.
  #define PANDA_SOURCE $[standardize $[TOPDIR]/../panda]
#endif

// Where should we install DIRECT?
#if $[DIRECT_INSTALL]
  #define DIRECT_INSTALL $[unixfilename $[DIRECT_INSTALL]]
#elif $[CTPROJS]
  #set DIRECT $[unixfilename $[DIRECT]]
  #define DIRECT_INSTALL $[DIRECT]/built
  #if $[eq $[DIRECT],]
    #error You seem to be attached to some trees, but not DIRECT!
  #endif
#else
  #defer DIRECT_INSTALL $[unixfilename $[INSTALL_DIR]]
#endif

// Also get the PANDA Package file and everything that includes.
#if $[not $[isfile $[PANDA_SOURCE]/Package.pp]]
  #printvar PANDA_SOURCE
  #error PANDA source directory not found from direct!  Are you attached properly?
#endif

#include $[PANDA_SOURCE]/Package.pp

// Define the inter-tree dependencies.
#define NEEDS_TREES panda $[NEEDS_TREES]
#define DEPENDABLE_HEADER_DIRS $[DEPENDABLE_HEADER_DIRS] $[PANDA_INSTALL]/include
