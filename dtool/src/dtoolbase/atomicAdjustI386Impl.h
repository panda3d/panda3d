// Filename: atomicAdjustI386Impl.h
// Created by:  drose (01Apr06)
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

#ifndef ATOMICADJUSTI386IMPL_H
#define ATOMICADJUSTI386IMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#if defined(__i386__) || defined(_M_IX86)

#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : AtomicAdjustI386Impl
// Description : Uses assembly-language calls to atomically increment
//               and decrement.  Although this class is named i386, it
//               actually uses instructions that are specific to 486
//               and higher.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL AtomicAdjustI386Impl {
public:
  INLINE static void inc(TVOLATILE PN_int32 &var);
  INLINE static bool dec(TVOLATILE PN_int32 &var);
  INLINE static PN_int32 set(TVOLATILE PN_int32 &var, PN_int32 new_value);
  INLINE static PN_int32 get(const TVOLATILE PN_int32 &var);

  INLINE static void *set_ptr(void * TVOLATILE &var, void *new_value);
  INLINE static void *get_ptr(void * const TVOLATILE &var);

  INLINE static PN_int32 compare_and_exchange(TVOLATILE PN_int32 &mem, 
                                              PN_int32 old_value,
                                              PN_int32 new_value);

  INLINE static void *compare_and_exchange_ptr(void * TVOLATILE &mem, 
                                               void *old_value,
                                               void *new_value);
};

#include "atomicAdjustI386Impl.I"

#endif  // __i386__

#endif
