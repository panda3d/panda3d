/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutexHolder.h
 * @author drose
 * @date 2008-10-08
 */

#ifndef LIGHTREMUTEXHOLDER_H
#define LIGHTREMUTEXHOLDER_H

#include "pandabase.h"
#include "lightReMutex.h"

class Thread;

/**
 * Similar to MutexHolder, but for a light reentrant mutex.
 */
class EXPCL_PANDA_PIPELINE LightReMutexHolder {
public:
  INLINE LightReMutexHolder(const LightReMutex &mutex);
  INLINE LightReMutexHolder(const LightReMutex &mutex, Thread *current_thread);
  INLINE LightReMutexHolder(LightReMutex *&mutex);
  LightReMutexHolder(const LightReMutexHolder &copy) = delete;
  INLINE ~LightReMutexHolder();

  LightReMutexHolder &operator = (const LightReMutexHolder &copy) = delete;

private:
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const LightReMutex *_mutex;
#endif
};

#include "lightReMutexHolder.I"

#endif
