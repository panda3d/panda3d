/*
// Filename: ppremake.h
// Created by:  drose (25Sep00)
// 
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////
*/

#ifndef PPREMAKE_H
#define PPREMAKE_H

#ifdef _MSC_VER
  /* For Visual C, include the special config.h file. */
  #include "config_msvc.h"
#else
  /* Otherwise, include the normal automatically-generated file. */
  #include "config.h"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef __cplusplus
#ifdef HAVE_IOSTREAM
#include <iostream>
#include <fstream>
#include <iomanip>
#ifdef HAVE_SSTREAM
#include <sstream>
#else  /* HAVE_SSTREAM */
#include <strstream>
#endif  /* HAVE_SSTREAM */

typedef std::ios::openmode ios_openmode;
typedef std::ios::fmtflags ios_fmtflags;
typedef std::ios::iostate ios_iostate;
typedef std::ios::seekdir ios_seekdir;

#else  /* HAVE_IOSTREAM */
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <strstream.h>

typedef int ios_openmode;
typedef int ios_fmtflags;
typedef int ios_iostate;
/* Old iostream libraries used ios::seek_dir instead of ios::seekdir. */
typedef ios::seek_dir ios_seekdir;

#endif  /* HAVE_IOSTREAM */

#if defined(HAVE_CYGWIN) || defined(WIN32_VC)
/* Either Cygwin or Visual C++ is a Win32 environment. */
#define WIN32
#endif

#include <string>
#include <map>

#ifdef HAVE_NAMESPACE
using namespace std;
#endif
#endif /* __cplusplus */

#ifndef HAVE_ALLOCA_H
  /* If we don't have alloca.h, use malloc() to implement gnu_regex. */
  #define REGEX_MALLOC 1
#endif

#define PACKAGE_FILENAME "Package.pp"
#define SOURCE_FILENAME "Sources.pp"

#define COMMAND_PREFIX '#'
#define VARIABLE_PREFIX '$'
#define VARIABLE_OPEN_BRACE '['
#define VARIABLE_CLOSE_BRACE ']'
#define PATTERN_WILDCARD '%'
#define BEGIN_COMMENT "//"

#define FUNCTION_PARAMETER_SEPARATOR ','

#define VARIABLE_OPEN_NESTED '('
#define VARIABLE_CLOSE_NESTED ')'
#define VARIABLE_PATSUBST ":"
#define VARIABLE_PATSUBST_DELIM "="

#define SCOPE_DIRNAME_SEPARATOR '/'
#define SCOPE_DIRNAME_WILDCARD "*"
#define SCOPE_DIRNAME_CURRENT "."

#ifdef __cplusplus
/* These are set from the similarly-named variables defined in
   System.pp. */
extern bool unix_platform;
extern bool windows_platform;

/* This is a command-line global parameter. */
extern bool dry_run;
extern bool verbose_dry_run;
extern int verbose; // 0..9 to set verbose level.  0 == off.
extern int debug_expansions;

/* This is set true internally if an error occurred while processing
   any of the scripts. */
extern bool errors_occurred;

/* This structure tracks the number of expansions that are performed
   on a particular string, and the different values it produces, only
   if debug_expansions (above) is set true by command-line parameter
   -x. */
typedef map<string, int> ExpandResultCount;
typedef map<string, ExpandResultCount> DebugExpand;
extern DebugExpand debug_expand;

#endif

/* These are defined so that we may build Filename, DSearchPath, and
   GlobPattern, which are copied from dtool and panda.  We have to
   copy these files in since ppremake must be built first, and stands
   outside of Panda; but we want to minimize the changes we must make
   to these files so that we can easily recopy them at need.

   These symbols just make the build environment a bit more
   Panda-like. */
#define PUBLISHED public
#define INLINE inline
#define EXPCL_DTOOL
#define EXPCL_PANDA

#endif
