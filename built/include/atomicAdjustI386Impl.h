/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustI386Impl.h
 * @author drose
 * @date 2006-04-01
 */

#ifndef ATOMICADJUSTI386IMPL_H
#define ATOMICADJUSTI386IMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#if (defined(__i386__) || defined(_M_IX86)) && !defined(__APPLE__) && !defined(__ANDROID__)

#include "numeric_types.h"

/**
 * Uses assembly-language calls to atomically increment and decrement.
 * Although this class is named i386, it actually uses instructions that are
 * specific to 486 and higher.
 */
class EXPCL_DTOOL_DTOOLBASE AtomicAdjustI386Impl {
public:
  typedef ALIGN_4BYTE int32_t Integer;
  typedef void *UnalignedPointer;
  typedef ALIGN_4BYTE UnalignedPointer Pointer;

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

#include "atomicAdjustI386Impl.I"

#endif  // __i386__

#endif
