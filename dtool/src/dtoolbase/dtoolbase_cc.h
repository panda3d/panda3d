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

// By including checkPandaVersion.h, we guarantee that runtime attempts to
// load any DLL will fail if they inadvertently link with the wrong version of
// dtool, which, transitively, means all DLLs must be from the same
// (ABI-compatible) version of Panda.

#include "checkPandaVersion.h"

#ifdef USE_TAU
// Tau provides this destructive version of stdbool.h that we must mask.
#define __PDT_STDBOOL_H_
#endif

#ifdef CPPPARSER
#include <iostream>
#include <iomanip>
#include <string>
#include <utility>
#include <algorithm>

#define INLINE inline
#define ALWAYS_INLINE inline
#define MOVE(x) x

#define EXPORT_TEMPLATE_CLASS(expcl, exptp, classname)

// We define the macro PUBLISHED to mark C++ methods that are to be published
// via interrogate to scripting languages.  However, if we're not running the
// interrogate pass (CPPPARSER isn't defined), this maps to public.
#define PUBLISHED __published

#define PHAVE_ATOMIC 1

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
#include <algorithm>

#ifndef HAVE_IOS_TYPEDEFS
typedef int ios_openmode;
typedef int ios_fmtflags;
typedef int ios_iostate;
// Old iostream libraries used ios::seek_dir instead of ios::seekdir.
typedef ios::seek_dir ios_seekdir;
#else
typedef std::ios::openmode ios_openmode;
typedef std::ios::fmtflags ios_fmtflags;
typedef std::ios::iostate ios_iostate;
typedef std::ios::seekdir ios_seekdir;
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

// Apple has an outdated libstdc++.  Not all is lost, though, as we can fill
// in some important missing functions.
#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20070719
#include <tr1/tuple>
#include <tr1/cmath>

namespace std {
  using std::tr1::tuple;
  using std::tr1::tie;
  using std::tr1::copysign;

  typedef decltype(nullptr) nullptr_t;

  template<class T> struct remove_reference      {typedef T type;};
  template<class T> struct remove_reference<T&>  {typedef T type;};
  template<class T> struct remove_reference<T&& >{typedef T type;};

  template<class T> typename remove_reference<T>::type &&move(T &&t) {
    return static_cast<typename remove_reference<T>::type&&>(t);
  }

  template<class T> struct owner_less;

  typedef enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst,
  } memory_order;

  #define ATOMIC_FLAG_INIT { 0 }
  class atomic_flag {
    bool _flag;

  public:
    atomic_flag() noexcept = default;
    ALWAYS_INLINE constexpr atomic_flag(bool flag) noexcept : _flag(flag) {}
    atomic_flag(const atomic_flag &) = delete;
    ~atomic_flag() noexcept = default;
    atomic_flag &operator = (const atomic_flag&) = delete;

    ALWAYS_INLINE bool test_and_set(memory_order order = memory_order_seq_cst) noexcept {
      return __atomic_test_and_set(&_flag, order);
    }
    ALWAYS_INLINE void clear(memory_order order = memory_order_seq_cst) noexcept {
      __atomic_clear(&_flag, order);
    }
  };
};
#else
// Expect that we have access to the <atomic> header.
#define PHAVE_ATOMIC 1
#endif

// Determine the availability of C++11 features.
#if defined(_MSC_VER) && _MSC_VER < 1900 // Visual Studio 2015
#error Microsoft Visual C++ 2015 or later is required to compile Panda3D.
#endif

// This is just to support code generated with older versions of interrogate.
#define MOVE(x) (std::move(x))


#ifndef LINK_ALL_STATIC
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
EXPCL_DTOOL_DTOOLBASE extern MemoryHook *memory_hook;
EXPCL_DTOOL_DTOOLBASE void init_memory_hook();

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
extern EXPCL_DTOOL_DTOOLBASE void (*global_thread_yield)();
extern EXPCL_DTOOL_DTOOLBASE void (*global_thread_consider_yield)();

INLINE void thread_yield() {
  (*global_thread_yield)();
}
INLINE void thread_consider_yield() {
  (*global_thread_consider_yield)();
}

#ifdef HAVE_PYTHON
typedef struct _ts PyThreadState;
extern EXPCL_DTOOL_DTOOLBASE PyThreadState *(*global_thread_state_swap)(PyThreadState *tstate);

INLINE PyThreadState *thread_state_swap(PyThreadState *tstate) {
  return (*global_thread_state_swap)(tstate);
}
#endif  // HAVE_PYTHON

#else

INLINE void thread_yield() {
}
INLINE void thread_consider_yield() {
}

#endif  // HAVE_THREADS && SIMPLE_THREADS

#if defined(USE_TAU) && defined(WIN32)
// Hack around tau's lack of DLL export declarations for Profiler class.
extern EXPCL_DTOOL_DTOOLBASE bool __tau_shutdown;
class EXPCL_DTOOL_DTOOLBASE TauProfile {
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
