/* Filename: dtoolbase.h
 * Created by:  drose (12Sep00)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This file is included at the beginning of every header file and/or
   C or C++ file.  It must be compilable for C as well as C++ files,
   so no C++-specific code or syntax can be put here.  See
   dtoolbase_cc.h for C++-specific stuff. */

#ifndef DTOOLBASE_H
#define DTOOLBASE_H

#include "dtool_config.h"

#ifdef WIN32_VC
/* These warning pragmas must appear before anything else for VC++ to
   respect them.  Sheesh. */

/* C4231: extern before template instantiation */
/* For some reason, this particular warning won't disable. */
#pragma warning (disable : 4231)
/* C4786: 255 char debug symbols */
#pragma warning (disable : 4786)
/* C4251: needs dll interface */
#pragma warning (disable : 4251)
/* C4503: decorated name length exceeded */
#pragma warning (disable : 4503)
/* C4305: truncation from 'const double' to 'float' */
#pragma warning (disable : 4305)
/* C4250: 'myclass' : inherits 'baseclass::member' via dominance */
#pragma warning (disable : 4250)
/* C4355: 'this' : used in base member initializer list */
#pragma warning (disable : 4355)
/* C4244: 'initializing' : conversion from 'double' to 'float', possible loss of data */
#pragma warning (disable : 4244)

#if _MSC_VER >= 1300
 #if _MSC_VER >= 1310
   #define USING_MSVC7_1
//#pragma message("VC 7.1")    
 #else
//#pragma message("VC 7.0") 
 #endif
#define USING_MSVC7
#else 
// #pragma message("VC 6.0")
#endif

// Use NODEFAULT to optimize a switch() stmt to tell MSVC to automatically go to the final untested case 
// after it has failed all the other cases (i.e. 'assume at least one of the cases is always true')
#ifdef _DEBUG
# define NODEFAULT  default: assert(0);
#else
# define NODEFAULT  default: __assume(0);   // special VC keyword
#endif

#else /* if !WIN32_VC */
#ifdef _DEBUG
# define NODEFAULT   default: assert(0);
#else
# define NODEFAULT
#endif
#endif  /* WIN32_VC */

#include "dtoolsymbols.h"

#ifdef __GNUC__
// Large file >2GB support
// this needs be be before systypes.h and other C headers
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#ifdef HAVE_MINMAX_H
#include <minmax.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef CPPPARSER
#include <stdtypedefs.h>
#endif

/*
 We define the macros BEGIN_PUBLISH and END_PUBLISH to bracket
 functions and global variable definitions that are to be published
 via interrogate to scripting languages.
 */
#ifdef CPPPARSER
#define BEGIN_PUBLISH __begin_publish
#define END_PUBLISH __end_publish
#else
#define BEGIN_PUBLISH
#define END_PUBLISH
#endif

#ifdef __cplusplus
#include "dtoolbase_cc.h"
#endif

#endif

