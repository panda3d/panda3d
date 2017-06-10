/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dtoolbase_cc.h
 * @author drose
 * @date 2000-09-13
 */

#ifndef DTOOLBASE_CC_H
#define DTOOLBASE_CC_H

// This file should never be included directly; it's intended to be included
// only from dtoolbase.h.  Include that file instead.

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
#define ALWAYS_INLINE inline
#define TYPENAME typename
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#define FINAL final
#define OVERRIDE override
#define MOVE(x) x
#define DEFAULT_CTOR = default
#define DEFAULT_DTOR = default
#define DEFAULT_ASSIGN = default
#define DELETED = delete
#define DELETED_ASSIGN = delete

#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)

// We define the macro PUBLISHED to mark C++ methods that are to be published
// via interrogate to scripting languages.  However, if we're not running the
// interrogate pass (CPPPARSER isn't defined), this maps to public.
#define PUBLISHED __published

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
#include <utility>

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
typedef long streamsize;
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

// Apple has an outdated libstdc++.  Not all is lost, though, as we can fill
// in some important missing functions.
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20070719
typedef decltype(nullptr) nullptr_t;

template<class T> struct remove_reference      {typedef T type;};
template<class T> struct remove_reference<T&>  {typedef T type;};
template<class T> struct remove_reference<T&& >{typedef T type;};

template<class T> typename remove_reference<T>::type &&move(T &&t) {
  return static_cast<typename remove_reference<T>::type&&>(t);
}
#endif

#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#elif defined(__GNUC__)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif

#ifdef FORCE_INLINING
// If FORCE_INLINING is defined, we use the keyword __forceinline, which tells
// MS VC++ to override its internal benefit heuristic and inline the fn if it
// is technically possible to do so.
#define INLINE ALWAYS_INLINE
#else
#define INLINE inline
#endif

// Determine the availability of C++11 features.
#if defined(__has_extension) // Clang magic.
#  if __has_extension(cxx_constexpr)
#    if !defined(__apple_build_version__) || __apple_build_version__ >= 5000000
#      define CONSTEXPR constexpr
#    endif
#  endif
#  if __has_extension(cxx_noexcept)
#    define NOEXCEPT noexcept
#  endif
#  if __has_extension(cxx_rvalue_references) && (__cplusplus >= 201103L)
#    define USE_MOVE_SEMANTICS
#    define MOVE(x) move(x)
#  endif
#  if __has_extension(cxx_override_control) && (__cplusplus >= 201103L)
#    define FINAL final
#    define OVERRIDE override
#  endif
#  if __has_extension(cxx_defaulted_functions)
#     define DEFAULT_CTOR = default
#     define DEFAULT_DTOR = default
#     define DEFAULT_ASSIGN = default
#  endif
#  if __has_extension(cxx_deleted_functions)
#     define DELETED = delete
#  endif
#elif defined(__GNUC__) && (__cplusplus >= 201103L) // GCC

// Starting at GCC 4.4
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
#  define DEFAULT_CTOR = default
#  define DEFAULT_DTOR = default
#  define DEFAULT_ASSIGN = default
#  define DELETED = delete
#  endif

// Starting at GCC 4.6
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#    define CONSTEXPR constexpr
#    define NOEXCEPT noexcept
#    define USE_MOVE_SEMANTICS
#    define FINAL final
#    define MOVE(x) move(x)
#  endif

// Starting at GCC 4.7
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#    define OVERRIDE override
#  endif

// GCC defines several macros which we can query.  List of all supported
// builtin macros: https://gcc.gnu.org/projects/cxx-status.html
#  if !defined(CONSTEXPR) && __cpp_constexpr >= 200704
#    define CONSTEXPR constexpr
#  endif

#elif defined(_MSC_VER) && _MSC_VER >= 1900 // Visual Studio 2015
#  define CONSTEXPR constexpr
#  define NOEXCEPT noexcept
#  define USE_MOVE_SEMANTICS
#  define FINAL final
#  define OVERRIDE override
#  define MOVE(x) move(x)
#elif defined(_MSC_VER) && _MSC_VER >= 1600 // Visual Studio 2010
#  define NOEXCEPT throw()
#  define OVERRIDE override
#  define USE_MOVE_SEMANTICS
#  define FINAL sealed
#  define MOVE(x) move(x)
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1800 // Visual Studio 2013
#  define DEFAULT_CTOR = default
#  define DEFAULT_DTOR = default
#  define DEFAULT_ASSIGN = default
#  define DELETED = delete
#endif

// Fallbacks if features are not supported
#ifndef CONSTEXPR
#  define CONSTEXPR INLINE
#endif
#ifndef NOEXCEPT
#  define NOEXCEPT
#endif
#ifndef MOVE
#  define MOVE(x) x
#endif
#ifndef FINAL
#  define FINAL
#endif
#ifndef OVERRIDE
#  define OVERRIDE
#endif
#ifndef DEFAULT_CTOR
#  define DEFAULT_CTOR {}
#endif
#ifndef DEFAULT_DTOR
#  define DEFAULT_DTOR {}
#endif
#ifndef DEFAULT_ASSIGN
#  define DEFAULT_ASSIGN {return *this;}
#endif
#ifndef DELETED
#  define DELETED {assert(false);}
#  define DELETED_ASSIGN {assert(false);return *this;}
#else
#  define DELETED_ASSIGN DELETED
#endif


#if !defined(LINK_ALL_STATIC) && defined(EXPORT_TEMPLATES)
// This macro must be used to export an instantiated template class from a
// DLL.  If the template class name itself contains commas, it may be
// necessary to first define a macro for the class name, to allow proper macro
// parameter passing.
#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname) \
  exptp template class expcl classname;
#else
#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)
#endif

// We define the macro PUBLISHED to mark C++ methods that are to be published
// via interrogate to scripting languages.  However, if we're not running the
// interrogate pass (CPPPARSER isn't defined), this maps to public.
#define PUBLISHED public

#endif  // CPPPARSER

// The ReferenceCount class is defined later, within Panda, but we need to
// pass around forward references to it here at the very low level.
class ReferenceCount;

// We need a pointer to a global MemoryHook object, to manage all malloc and
// free requests from Panda.  See the comments in MemoryHook itself.
class MemoryHook;
EXPCL_DTOOL extern MemoryHook *memory_hook;
EXPCL_DTOOL void init_memory_hook();

// Now redefine some handy macros to hook into the above MemoryHook object.
#ifndef USE_MEMORY_NOWRAPPERS
#define PANDA_MALLOC_SINGLE(size) (ASSUME_ALIGNED(memory_hook->heap_alloc_single(size), MEMORY_HOOK_ALIGNMENT))
#define PANDA_FREE_SINGLE(ptr) memory_hook->heap_free_single(ptr)
#define PANDA_MALLOC_ARRAY(size) (ASSUME_ALIGNED(memory_hook->heap_alloc_array(size), MEMORY_HOOK_ALIGNMENT))
#define PANDA_REALLOC_ARRAY(ptr, size) (ASSUME_ALIGNED(memory_hook->heap_realloc_array(ptr, size), MEMORY_HOOK_ALIGNMENT))
#define PANDA_FREE_ARRAY(ptr) memory_hook->heap_free_array(ptr)
#else
#define PANDA_MALLOC_SINGLE(size) ::malloc(size)
#define PANDA_FREE_SINGLE(ptr) ::free(ptr)
#define PANDA_MALLOC_ARRAY(size) ::malloc(size)
#define PANDA_REALLOC_ARRAY(ptr, size) ::realloc(ptr, size)
#define PANDA_FREE_ARRAY(ptr) ::free(ptr)
#endif  // USE_MEMORY_NOWRAPPERS

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
// We need another forward-reference function to allow low-level code to
// cooperatively yield the timeslice, in SIMPLE_THREADS mode.
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
