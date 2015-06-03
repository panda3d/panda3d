// Filename: dtoolbase_cc.h
// Created by:  drose (13Sep00)
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

#ifndef DTOOLBASE_CC_H
#define DTOOLBASE_CC_H

// This file should never be included directly; it's intended to be
// included only from dtoolbase.h.  Include that file instead.

#ifdef __cplusplus

#ifdef USE_TAU
// Tau provides this destructive version of stdbool.h that we must mask.
#define __PDT_STDBOOL_H_
#endif

#ifdef CPPPARSER
#include <iostream>
#include <string>

using namespace std;

#define INLINE inline
#define TYPENAME typename
#define CONSTEXPR
#define NOEXCEPT noexcept
#define FINAL
#define OVERRIDE
#define MOVE(x) x

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

#ifdef PHAVE_IOSTREAM
#include <iostream>
#include <fstream>
#include <iomanip>
#else
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#endif

#ifdef PHAVE_SSTREAM
#include <sstream>
#else
#include "fakestringstream.h"
#endif

#ifdef PHAVE_NEW
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

// Determine the availability of C++11 features.
#if defined(__has_extension) // Clang magic.
#  if __has_extension(cxx_constexpr)
#    define CONSTEXPR constexpr
#  else
#    define CONSTEXPR INLINE
#  endif
#  if __has_extension(cxx_noexcept)
#    define NOEXCEPT noexcept
#  else
#    define NOEXCEPT
#  endif
#  if __has_extension(cxx_rvalue_references) && (__cplusplus >= 201103L)
#    define USE_MOVE_SEMANTICS
#    define MOVE(x) move(x)
#  else
#    define MOVE(x) x
#  endif
#  if __has_extension(cxx_override_control) && (__cplusplus >= 201103L)
#    define FINAL final
#    define OVERRIDE override
#  else
#    define FINAL
#    define OVERRIDE
#  endif
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) && (__cplusplus >= 201103L)
// noexcept was introduced in GCC 4.6, constexpr in GCC 4.7, rvalue refs in
// GCC 4.3.  However, GCC only started defining __cplusplus properly in 4.7.
#  define CONSTEXPR constexpr
#  define NOEXCEPT noexcept
#  define USE_MOVE_SEMANTICS
#  define FINAL final
#  define OVERRIDE override
#  define MOVE(x) move(x)
#elif defined(_MSC_VER) && _MSC_VER >= 1600
// MSVC 2010 has move semantics.  Not much else.
#  define CONSTEXPR INLINE
#  define NOEXCEPT throw()
#  define USE_MOVE_SEMANTICS
#  define FINAL
#  define OVERRIDE
#  define MOVE(x) move(x)
#else
#  define CONSTEXPR INLINE
#  define NOEXCEPT
#  define FINAL
#  define OVERRIDE
#  define MOVE(x) x
#endif

#if !defined(LINK_ALL_STATIC) && defined(EXPORT_TEMPLATES)
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

// The ReferenceCount class is defined later, within Panda, but we
// need to pass around forward references to it here at the very low
// level.
class ReferenceCount;

// We need a pointer to a global MemoryHook object, to manage all
// malloc and free requests from Panda.  See the comments in
// MemoryHook itself.
class MemoryHook;
EXPCL_DTOOL extern MemoryHook *memory_hook;
EXPCL_DTOOL void init_memory_hook();

// Now redefine some handy macros to hook into the above MemoryHook
// object.
#ifndef USE_MEMORY_NOWRAPPERS
#define PANDA_MALLOC_SINGLE(size) (memory_hook->heap_alloc_single(size))
#define PANDA_FREE_SINGLE(ptr) memory_hook->heap_free_single(ptr)
#define PANDA_MALLOC_ARRAY(size) (memory_hook->heap_alloc_array(size))
#define PANDA_REALLOC_ARRAY(ptr, size) (memory_hook->heap_realloc_array(ptr, size))
#define PANDA_FREE_ARRAY(ptr) memory_hook->heap_free_array(ptr)
#else
#define PANDA_MALLOC_SINGLE(size) ::malloc(size)
#define PANDA_FREE_SINGLE(ptr) ::free(ptr)
#define PANDA_MALLOC_ARRAY(size) ::malloc(size)
#define PANDA_REALLOC_ARRAY(ptr, size) ::realloc(ptr, size)
#define PANDA_FREE_ARRAY(ptr) ::free(ptr)
#endif  // USE_MEMORY_NOWRAPPERS

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
// We need another forward-reference function to allow low-level code
// to cooperatively yield the timeslice, in SIMPLE_THREADS mode.
extern EXPCL_DTOOL void (*global_thread_yield)();
extern EXPCL_DTOOL void (*global_thread_consider_yield)();

INLINE void thread_yield() {
  (*global_thread_yield)();
}
INLINE void thread_consider_yield() {
  (*global_thread_consider_yield)();
}

#else

INLINE void thread_yield() {
}
INLINE void thread_consider_yield() {
}

#endif  // HAVE_THREADS && SIMPLE_THREADS

#if defined(USE_TAU) && defined(WIN32)
// Hack around tau's lack of DLL export declarations for Profiler class.
extern EXPCL_DTOOL bool __tau_shutdown;
class EXPCL_DTOOL TauProfile {
public:
  TauProfile(void *&tautimer, char *name, char *type, int group, char *group_name) {
    Tau_profile_c_timer(&tautimer, name, type, group, group_name);
    _tautimer = tautimer;
    TAU_PROFILE_START(_tautimer);
  }
  ~TauProfile() {
    if (!__tau_shutdown) {
      TAU_PROFILE_STOP(_tautimer);
    }
  }

private:
  void *_tautimer;
};

#undef TAU_PROFILE
#define TAU_PROFILE(name, type, group) \
  static void *__tautimer; \
  TauProfile __taupr(__tautimer, name, type, group, #group)

#undef TAU_PROFILE_EXIT
#define TAU_PROFILE_EXIT(msg) \
  __tau_shutdown = true; \
  Tau_exit(msg);

#endif  // USE_TAU

#endif  //  __cplusplus
#endif
