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


// Pull in the package-level Config file.  This contains a few
// configuration variables that the user might want to fine-tune.
#include $[THISDIRPREFIX]Config.pp


// Also get the DTOOL Package file and everything that includes.
#if $[eq $[wildcard $[DTOOL]],]
  #error Directory defined by $DTOOL not found!  Are you attached properly?
#endif

#include $[DTOOL]/Package.pp
