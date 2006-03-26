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
  #define PACKAGE panda
  #define VERSION 0.80
#endif


// Where should we find the DTOOL source directory?
#if $[DTOOL_SOURCE]
  #define DTOOL_SOURCE $[unixfilename $[DTOOL_SOURCE]]
#elif $[or $[CTPROJS],$[DTOOL]]
  // If we are presently attached, use the environment variable.
  #define DTOOL_SOURCE $[unixfilename $[DTOOL]]
  #if $[eq $[DTOOL],]
    #error You seem to be attached to some trees, but not DTOOL!
  #endif
#else
  // Otherwise, if we are not attached, we guess that the source is a
  // sibling directory to this source root.
  #define DTOOL_SOURCE $[standardize $[TOPDIR]/../dtool]
#endif

// Where should we install PANDA?
#if $[PANDA_INSTALL]
  #define PANDA_INSTALL $[unixfilename $[PANDA_INSTALL]]
#elif $[CTPROJS]
  #set PANDA $[unixfilename $[PANDA]]
  #define PANDA_INSTALL $[PANDA]/built
  #if $[eq $[PANDA],]
    #error You seem to be attached to some trees, but not PANDA!
  #endif
#else
  #defer PANDA_INSTALL $[unixfilename $[INSTALL_DIR]]
#endif

// Also get the DTOOL Package file and everything that includes.
#if $[not $[isfile $[DTOOL_SOURCE]/Package.pp]]
  #printvar DTOOL_SOURCE
  #error DTOOL source directory not found from panda!  Are you attached properly?
#endif

#include $[DTOOL_SOURCE]/Package.pp

// Define the inter-tree dependencies.
#define NEEDS_TREES dtool $[NEEDS_TREES]
#define DEPENDABLE_HEADER_DIRS $[DEPENDABLE_HEADER_DIRS] $[DTOOL_INSTALL]/include
