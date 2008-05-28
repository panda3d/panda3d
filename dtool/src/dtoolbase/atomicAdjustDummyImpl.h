// Filename: atomicAdjustDummyImpl.h
// Created by:  drose (09Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef ATOMICADJUSTDUMMYIMPL_H
#define ATOMICADJUSTDUMMYIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : AtomicAdjustDummyImpl
// Description : A trivial implementation for atomic adjustments for
//               systems that don't require multiprogramming, and
//               therefore don't require special atomic operations.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL AtomicAdjustDummyImpl {
public:
  typedef long Integer;

  INLINE static void inc(TVOLATILE Integer &var);
  INLINE static bool dec(TVOLATILE Integer &var);
  INLINE static void add(TVOLATILE Integer &var, Integer delta);
  INLINE static Integer set(TVOLATILE Integer &var, Integer new_value);
  INLINE static Integer get(const TVOLATILE Integer &var);

  INLINE static void *set_ptr(void * TVOLATILE &var, void *new_value);
  INLINE static void *get_ptr(void * const TVOLATILE &var);

  INLINE static Integer compare_and_exchange(TVOLATILE Integer &mem, 
                                              Integer old_value,
                                              Integer new_value);

  INLINE static void *compare_and_exchange_ptr(void * TVOLATILE &mem, 
                                               void *old_value,
                                               void *new_value);
};

#include "atomicAdjustDummyImpl.I"

#endif
