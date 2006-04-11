// Filename: atomicAdjustWin32Impl.h
// Created by:  drose (07Feb06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ATOMICADJUSTWIN32IMPL_H
#define ATOMICADJUSTWIN32IMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_WIN32_IMPL

#include "numeric_types.h"

#include <windows.h>

#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE 1
#define HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR 1

////////////////////////////////////////////////////////////////////
//       Class : AtomicAdjustWin32Impl
// Description : Uses Windows native calls to implement atomic
//               adjustments.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL AtomicAdjustWin32Impl {
public:
  INLINE static void inc(PN_int32 &var);
  INLINE static bool dec(PN_int32 &var);
  INLINE static PN_int32 set(PN_int32 &var, PN_int32 new_value);
  INLINE static PN_int32 get(const PN_int32 &var);

  INLINE static void *set_ptr(void *&var, void *new_value);
  INLINE static void *get_ptr(void * const &var);

  INLINE static PN_int32 compare_and_exchange(volatile PN_int32 &mem, 
                                              PN_int32 old_value,
                                              PN_int32 new_value);

  INLINE static void *compare_and_exchange_ptr(void * volatile &mem, 
                                               void *old_value,
                                               void *new_value);
};

#include "atomicAdjustWin32Impl.I"

#endif  // THREAD_WIN32_IMPL

#endif
