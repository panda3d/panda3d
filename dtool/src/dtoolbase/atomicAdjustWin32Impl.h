/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file atomicAdjustWin32Impl.h
 * @author drose
 * @date 2006-02-07
 */

#ifndef ATOMICADJUSTWIN32IMPL_H
#define ATOMICADJUSTWIN32IMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef WIN32_VC

#include "numeric_types.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

/**
 * Uses Windows native calls to implement atomic adjustments.
 */
class EXPCL_DTOOL_DTOOLBASE AtomicAdjustWin32Impl {
public:
#ifdef _WIN64
  // For 64-bit builds, we'd prefer to use a 64-bit integer.
  typedef ALIGN_8BYTE LONGLONG Integer;
  typedef void *UnalignedPointer;
  typedef ALIGN_8BYTE UnalignedPointer Pointer;
#else
  typedef ALIGN_4BYTE LONG Integer;
  typedef void *UnalignedPointer;
  typedef ALIGN_4BYTE UnalignedPointer Pointer;
#endif  // _WIN64

  ALWAYS_INLINE static void inc(TVOLATILE Integer &var);
  ALWAYS_INLINE static bool dec(TVOLATILE Integer &var);
  INLINE static Integer add(TVOLATILE Integer &var, Integer delta);
  ALWAYS_INLINE static Integer set(TVOLATILE Integer &var, Integer new_value);
  ALWAYS_INLINE static Integer get(const TVOLATILE Integer &var);

  ALWAYS_INLINE static Pointer set_ptr(TVOLATILE Pointer &var, Pointer new_value);
  ALWAYS_INLINE static Pointer get_ptr(const TVOLATILE Pointer &var);

  INLINE static Integer compare_and_exchange(TVOLATILE Integer &mem,
                                             Integer old_value,
                                             Integer new_value);

  INLINE static Pointer compare_and_exchange_ptr(TVOLATILE Pointer &mem,
                                                 Pointer old_value,
                                                 Pointer new_value);
};

#include "atomicAdjustWin32Impl.I"

#endif  // WIN32_VC

#endif
