// Filename: mutexHolder.h
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

#ifndef MUTEXHOLDER_H
#define MUTEXHOLDER_H

#include "pandabase.h"
#include "pmutex.h"

////////////////////////////////////////////////////////////////////
//       Class : MutexHolder
// Description : A lightweight C++ object whose constructor calls
//               acquire() and whose destructor calls release() on a
//               mutex.  It is a C++ convenience wrapper to call
//               release() automatically when a block exits (for
//               instance, on return).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE MutexHolder {
public:
  INLINE MutexHolder(const Mutex &mutex);
  INLINE MutexHolder(const Mutex &mutex, Thread *current_thread);
  INLINE MutexHolder(Mutex *&mutex);
  INLINE ~MutexHolder();
private:
  INLINE MutexHolder(const MutexHolder &copy);
  INLINE void operator = (const MutexHolder &copy);

private:
  // If HAVE_THREADS is defined, the Mutex class implements an actual
  // mutex object of some kind.  If HAVE_THREADS is not defined, this
  // will be a MutexDummyImpl, which does nothing much anyway, so we
  // might as well not even store a pointer to one.
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const Mutex *_mutex;
#endif
};

#include "mutexHolder.I"

#endif
