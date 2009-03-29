//
// Package.pp
//
// This file defines certain configuration variables that are to be
// written into the various make scripts.  It is processed by ppremake
// (along with the Sources.pp files in each of the various
// contribories) to generate build scripts appropriate to each
// environment.
//
// This is the package-specific file, which should be at the top of
// every source hierarchy.  It generally gets the ball rolling, and is
// responsible for explicitly including all of the relevent Config.pp
// files.

// What is the name and version of this source tree?
#if $[eq $[PACKAGE],]
  #define PACKAGE contrib
  #define VERSION 0.80
#endif


// Where should we find the PANDA source contribory?
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
  // sibling contribory to this source root.
  #define PANDA_SOURCE $[standardize $[TOPDIR]/../panda]
#endif

// Where should we install CONTRIB?
#if $[CONTRIB_INSTALL]
  #define CONTRIB_INSTALL $[unixfilename $[CONTRIB_INSTALL]]
#elif $[CTPROJS]
  #set CONTRIB $[unixfilename $[CONTRIB]]
  #define CONTRIB_INSTALL $[CONTRIB]/built
  #if $[eq $[CONTRIB],]
    #error You seem to be attached to some trees, but not CONTRIB!
  #endif
#else
  #defer CONTRIB_INSTALL $[unixfilename $[INSTALL_DIR]]
#endif

// Also get the PANDA Package file and everything that includes.
#if $[not $[isfile $[PANDA_SOURCE]/Package.pp]]
  #printvar PANDA_SOURCE
  #error PANDA source contribory not found from contrib!  Are you attached properly?
#endif

#include $[PANDA_SOURCE]/Package.pp

// Define the inter-tree dependencies.
#define NEEDS_TREES panda $[NEEDS_TREES]
#define DEPENDABLE_HEADER_DIRS $[DEPENDABLE_HEADER_DIRS] $[PANDA_INSTALL]/include
