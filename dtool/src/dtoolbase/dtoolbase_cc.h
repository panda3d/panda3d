// Filename: dtoolbase_cc.h
// Created by:  drose (13Sep00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PANDABASE_CC_H
#define PANDABASE_CC_H

// This file should never be included directly; it's intended to be
// included only from dtoolbase.h.  Include that file instead.


#ifdef CPPPARSER
#include <iostream>
#include <string>

using namespace std;

#define INLINE inline
#define TYPENAME typename

#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)

// We define the macro PUBLISHED to mark C++ methods that are to be
// published via interrogate to scripting languages.  However, if
// we're not running the interrogate pass (CPPPARSER isn't defined),
// this maps to public.
#define PUBLISHED __published

typedef int streamsize;
typedef int ios_openmode;
typedef int ios_fmtflags;
typedef int ios_iostate;
typedef int ios_seekdir;

#else  // CPPPARSER

#ifdef HAVE_IOSTREAM
#include <iostream>
#include <fstream>
#include <iomanip>
#else
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#endif

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include "fakestringstream.h"
#endif

#ifdef HAVE_NEW
#include <new>
#endif

#include <string>

#ifdef HAVE_NAMESPACE
using namespace std;
#endif

#ifdef HAVE_TYPENAME
#define TYPENAME typename
#else
#define TYPENAME
#endif

#ifndef HAVE_WCHAR_T
// Some C++ libraries (os x 3.1) don't define this.
typedef unsigned short wchar_t;
#endif

#ifndef HAVE_WSTRING
// Some C++ libraries (gcc 2.95) don't define this.
typedef basic_string<wchar_t> wstring;
#endif

#ifndef HAVE_STREAMSIZE
// Some C++ libraries (Irix) don't define this.
typedef int streamsize;
#endif

#ifndef HAVE_IOS_TYPEDEFS
typedef int ios_openmode;
typedef int ios_fmtflags;
typedef int ios_iostate;
// Old iostream libraries used ios::seek_dir instead of ios::seekdir.
typedef ios::seek_dir ios_seekdir;
#else
typedef ios::openmode ios_openmode;
typedef ios::fmtflags ios_fmtflags;
typedef ios::iostate ios_iostate;
typedef ios::seekdir ios_seekdir;
#endif

#if defined(WIN32_VC) && defined(FORCE_INLINING)
// If FORCE_INLINING is defined, we use the keyword __forceinline,
// which tells MS VC++ to override its internal benefit heuristic
// and inline the fn if it is technically possible to do so.
#define INLINE __forceinline
#else
#define INLINE inline
#endif

#if defined(WIN32_VC) && !defined(LINK_ALL_STATIC) && defined(EXPORT_TEMPLATES)
// This macro must be used to export an instantiated template class
// from a DLL.  If the template class name itself contains commas, it
// may be necessary to first define a macro for the class name, to
// allow proper macro parameter passing.
#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname) \
  exptp template class expcl classname;
#else
#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)
#endif

// We define the macro PUBLISHED to mark C++ methods that are to be
// published via interrogate to scripting languages.  However, if
// we're not running the interrogate pass (CPPPARSER isn't defined),
// this maps to public.
#define PUBLISHED public

#endif  // CPPPARSER

// Now redefine global operators new and delete so we can optionally
// provide custom handlers for them.  The MemoryUsage class in Panda
// takes advantage of this to track the size of allocated pointers.
#ifndef NDEBUG
EXPCL_DTOOL void *default_operator_new(size_t size);
EXPCL_DTOOL void default_operator_delete(void *ptr);

extern EXPCL_DTOOL void *(*global_operator_new)(size_t size);
extern EXPCL_DTOOL void (*global_operator_delete)(void *ptr);

#ifdef GLOBAL_OPERATOR_NEW_EXCEPTIONS

// Redefinitions of operator new/delete, for O1 - O3 builds (!NDEBUG)
// only.  These flavors are for modern compilers that expect these
// function prototypes to handle exceptions.
INLINE void *operator new(size_t size) throw (std::bad_alloc) {
  return (*global_operator_new)(size);
}
INLINE void *operator new[](size_t size) throw (std::bad_alloc) {
  return (*global_operator_new)(size);
}

INLINE void operator delete(void *ptr) throw() {
  (*global_operator_delete)(ptr);
}
INLINE void operator delete[](void *ptr) throw() {
  (*global_operator_delete)(ptr);
}
#else   // GLOBAL_OPERATOR_NEW_EXCEPTIONS

// The same definitions as above, for compilers that don't expect
// exception handing in global operator new/delete functions.
INLINE void *operator new(size_t size) {
  return (*global_operator_new)(size);
}
INLINE void *operator new[](size_t size) {
  return (*global_operator_new)(size);
}

INLINE void operator delete(void *ptr) {
  (*global_operator_delete)(ptr);
}
INLINE void operator delete[](void *ptr) {
  (*global_operator_delete)(ptr);
}

#endif  // GLOBAL_OPERATOR_NEW_EXCEPTIONS
#endif  // NDEBUG

#endif
