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
  #define PACKAGE pandatool
  #define VERSION 0.80
#endif


// Where should we find the PANDA source directory?
#if $[or $[CTPROJS],$[PANDA]]
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

// Where should we install PANDATOOL?
#if $[or $[CTPROJS],$[PANDATOOL]]
  #set PANDATOOL $[unixfilename $[PANDATOOL]]
  #define PANDATOOL_INSTALL $[PANDATOOL]
  #define PANDATOOL_INSTALL_OTHER $(PANDATOOL)
  #if $[eq $[PANDATOOL],]
    #error You seem to be attached to some trees, but not PANDATOOL!
  #endif
#else
  #defer PANDATOOL_INSTALL $[unixfilename $[INSTALL_DIR]]
  #defer PANDATOOL_INSTALL_OTHER $[unixfilename $[INSTALL_DIR]]
#endif


// Define the inter-tree dependencies.
#define NEEDS_TREES $[NEEDS_TREES] panda


// Also get the PANDA Package file and everything that includes.
#if $[not $[isfile $[PANDA_SOURCE]/Package.pp]]
  #error PANDA source directory not found!  Are you attached properly?
#endif

#include $[PANDA_SOURCE]/Package.pp

// Now define the intra-tree dependencies.
#define DEPENDABLE_HEADER_DIRS $[DEPENDABLE_HEADER_DIRS] $[PANDA_INSTALL]/include
