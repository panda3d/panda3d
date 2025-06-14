/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjust.h
 * @author drose
 * @date 2002-08-09
 */

#ifndef ATOMICADJUST_H
#define ATOMICADJUST_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#if defined(CPPPARSER)

struct AtomicAdjust {
  typedef long Integer;
  typedef void *UnalignedPointer;
  typedef UnalignedPointer Pointer;
};

#elif defined(THREAD_DUMMY_IMPL) || defined(THREAD_SIMPLE_IMPL)

#include "atomicAdjustDummyImpl.h"
typedef AtomicAdjustDummyImpl AtomicAdjust;

#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))) || (defined(__clang__) && (__clang_major__ >= 3))
// GCC 4.7 and above has built-in __atomic functions for atomic operations.
// Clang 3.0 and above also supports them.

#include "atomicAdjustGccImpl.h"
typedef AtomicAdjustGccImpl AtomicAdjust;

#if (__GCC_ATOMIC_INT_LOCK_FREE + __GCC_ATOMIC_LONG_LOCK_FREE) > 0
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE 1
#endif
#if __GCC_ATOMIC_POINTER_LOCK_FREE > 0
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR 1
#endif

#elif (defined(__i386__) || defined(_M_IX86)) && !defined(__APPLE__)
// For an i386 architecture, we'll always use the i386 implementation.  It
// should be safe for any OS, and it might be a bit faster than any OS-
// provided calls.

#include "atomicAdjustI386Impl.h"
typedef AtomicAdjustI386Impl AtomicAdjust;

// These symbols are defined if the compare_and_exchange() methods are
// implemented natively, without recourse to external locks.  If these are not
// defined, users may elect to implement an operation with some other method
// than compare_and_exchange(), which might be faster.
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE 1
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR 1

#elif defined(THREAD_WIN32_IMPL)

#include "atomicAdjustWin32Impl.h"
typedef AtomicAdjustWin32Impl AtomicAdjust;

#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE 1
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR 1

#elif defined(THREAD_LINUX_IMPL)

#error Linux native threads are currently implemented only for i386; use Posix threads instead.

#elif defined(THREAD_POSIX_IMPL)

#include "atomicAdjustPosixImpl.h"
typedef AtomicAdjustPosixImpl AtomicAdjust;

#endif

#endif
