/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustGccImpl.h
 * @author rdb
 * @date 2014-07-04
 */

#ifndef ATOMICADJUSTGCCIMPL_H
#define ATOMICADJUSTGCCIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))) || (defined(__clang__) && (__clang_major__ >= 3))

/**
 * Uses GCC built-ins to implement atomic adjustments.
 */
class EXPCL_DTOOL_DTOOLBASE AtomicAdjustGccImpl {
public:
#if __GCC_ATOMIC_LONG_LOCK_FREE >= __GCC_ATOMIC_INT_LOCK_FREE
  // If the long can be more lock-free than int, use it instead.
  typedef __attribute__ ((aligned (__SIZEOF_LONG__))) long Integer;
#else
  typedef __attribute__ ((aligned (__SIZEOF_INT__))) int Integer;
#endif
  typedef void *UnalignedPointer;
  typedef __attribute__ ((aligned (__SIZEOF_POINTER__))) UnalignedPointer Pointer;

  INLINE static void inc(TVOLATILE Integer &var);
  INLINE static bool dec(TVOLATILE Integer &var);
  INLINE static Integer add(TVOLATILE Integer &var, Integer delta);
  INLINE static Integer set(TVOLATILE Integer &var, Integer new_value);
  INLINE static Integer get(const TVOLATILE Integer &var);

  INLINE static Pointer set_ptr(TVOLATILE Pointer &var, Pointer new_value);
  INLINE static Pointer get_ptr(const TVOLATILE Pointer &var);

  INLINE static Integer compare_and_exchange(TVOLATILE Integer &mem,
                                             Integer old_value,
                                             Integer new_value);

  INLINE static Pointer compare_and_exchange_ptr(TVOLATILE Pointer &mem,
                                                 Pointer old_value,
                                                 Pointer new_value);
};

#include "atomicAdjustGccImpl.I"

#endif  // HAVE_POSIX_THREADS

#endif
