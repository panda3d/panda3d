/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexDummyImpl.h
 * @author drose
 * @date 2002-08-08
 */

#ifndef MUTEXDUMMYIMPL_H
#define MUTEXDUMMYIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

/**
 * A fake mutex implementation for single-threaded applications that don't
 * need any synchronization control.  This does nothing at all.
 */
class EXPCL_DTOOL_DTOOLBASE MutexDummyImpl {
public:
  constexpr MutexDummyImpl() = default;
  MutexDummyImpl(const MutexDummyImpl &copy) = delete;

  MutexDummyImpl &operator = (const MutexDummyImpl &copy) = delete;

public:
  ALWAYS_INLINE void lock();
  ALWAYS_INLINE bool try_lock();
  ALWAYS_INLINE void unlock();
};

#include "mutexDummyImpl.I"

#endif
