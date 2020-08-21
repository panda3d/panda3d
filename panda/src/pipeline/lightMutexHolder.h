/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutexHolder.h
 * @author drose
 * @date 2008-10-08
 */

#ifndef LIGHTMUTEXHOLDER_H
#define LIGHTMUTEXHOLDER_H

#include "pandabase.h"
#include "lightMutex.h"

class Thread;

/**
 * Similar to MutexHolder, but for a light mutex.
 */
class EXPCL_PANDA_PIPELINE LightMutexHolder {
public:
  INLINE LightMutexHolder(const LightMutex &mutex);
  INLINE LightMutexHolder(LightMutex *&mutex);
  LightMutexHolder(const LightMutexHolder &copy) = delete;
  INLINE ~LightMutexHolder();

  LightMutexHolder &operator = (const LightMutexHolder &copy) = delete;

private:
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const LightMutex *_mutex;
#endif
};

#include "lightMutexHolder.I"

#endif
