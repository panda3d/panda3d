/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustDummyImpl.h
 * @author drose
 * @date 2002-08-09
 */

#ifndef ATOMICADJUSTDUMMYIMPL_H
#define ATOMICADJUSTDUMMYIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#include "numeric_types.h"

/**
 * A trivial implementation for atomic adjustments for systems that don't
 * require multiprogramming, and therefore don't require special atomic
 * operations.
 */
class EXPCL_DTOOL_DTOOLBASE AtomicAdjustDummyImpl {
public:
  typedef long Integer;
  typedef void *Pointer;

  ALWAYS_INLINE static void inc(TVOLATILE Integer &var);
  ALWAYS_INLINE static bool dec(TVOLATILE Integer &var);
  ALWAYS_INLINE static Integer add(TVOLATILE Integer &var, Integer delta);
  ALWAYS_INLINE static Integer set(TVOLATILE Integer &var, Integer new_value);
  ALWAYS_INLINE static Integer get(const TVOLATILE Integer &var);

  ALWAYS_INLINE static Pointer set_ptr(TVOLATILE Pointer &var, Pointer new_value);
  ALWAYS_INLINE static Pointer get_ptr(const TVOLATILE Pointer &var);

  ALWAYS_INLINE static Integer compare_and_exchange(TVOLATILE Integer &mem,
                                                    Integer old_value,
                                                    Integer new_value);

  ALWAYS_INLINE static Pointer compare_and_exchange_ptr(TVOLATILE Pointer &mem,
                                                        Pointer old_value,
                                                        Pointer new_value);
};

#include "atomicAdjustDummyImpl.I"

#endif
