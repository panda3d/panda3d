/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustPosixImpl.h
 * @author drose
 * @date 2006-02-10
 */

#ifndef ATOMICADJUSTPOSIXIMPL_H
#define ATOMICADJUSTPOSIXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef HAVE_POSIX_THREADS

#include "numeric_types.h"

#include <pthread.h>

/**
 * Uses POSIX to implement atomic adjustments.
 */
class EXPCL_DTOOL_DTOOLBASE AtomicAdjustPosixImpl {
public:
  // In Posix, "long" is generally the native word size (32- or 64-bit), which
  // is what we'd prefer.
  typedef long Integer;
  typedef void *Pointer;

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

private:
  static pthread_mutex_t _mutex;
};

#include "atomicAdjustPosixImpl.I"

#endif  // HAVE_POSIX_THREADS

#endif
