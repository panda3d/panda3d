/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexHolder.h
 * @author drose
 * @date 2002-08-09
 */

#ifndef MUTEXHOLDER_H
#define MUTEXHOLDER_H

#include "pandabase.h"
#include "pmutex.h"

/**
 * A lightweight C++ object whose constructor calls acquire() and whose
 * destructor calls release() on a mutex.  It is a C++ convenience wrapper to
 * call release() automatically when a block exits (for instance, on return).
 */
class EXPCL_PANDA_PIPELINE MutexHolder {
public:
  INLINE MutexHolder(const Mutex &mutex);
  INLINE MutexHolder(const Mutex &mutex, Thread *current_thread);
  INLINE MutexHolder(Mutex *&mutex);
  MutexHolder(const MutexHolder &copy) = delete;
  INLINE ~MutexHolder();

  MutexHolder &operator = (const MutexHolder &copy) = delete;

private:
  // If HAVE_THREADS is defined, the Mutex class implements an actual mutex
  // object of some kind.  If HAVE_THREADS is not defined, this will be a
  // MutexDummyImpl, which does nothing much anyway, so we might as well not
  // even store a pointer to one.
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const Mutex *_mutex;
#endif
};

#include "mutexHolder.I"

#endif
