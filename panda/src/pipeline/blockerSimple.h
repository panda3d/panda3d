/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file blockerSimple.h
 * @author drose
 * @date 2007-06-20
 */

#ifndef BLOCKERSIMPLE_H
#define BLOCKERSIMPLE_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "pnotify.h"

/**
 * This is a base class for MutexSimpleImpl and ConditionVarSimpleImpl.  It
 * represents a synchronization primitive that one or more threads might be
 * blocked on.
 */
class EXPCL_PANDA_PIPELINE BlockerSimple {
protected:
  constexpr BlockerSimple() = default;
  INLINE ~BlockerSimple();

protected:
  enum Flags {
    // lock_count is only used for mutexes, not condition variables.
    F_lock_count   = 0x3fffffff,
    F_has_waiters  = 0x40000000,
  };

  unsigned int _flags = 0;

  friend class ThreadSimpleManager;
};

#include "blockerSimple.I"

#endif  // THREAD_SIMPLE_IMPL

#endif
