/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutex.h
 * @author drose
 * @date 2008-10-08
 */

#ifndef LIGHTMUTEX_H
#define LIGHTMUTEX_H

#include "pandabase.h"
#include "mutexDebug.h"
#include "lightMutexDirect.h"

/**
 * This is a standard, non-reentrant mutex, similar to the Mutex class.  It is
 * different from Mutex in the case of SIMPLE_THREADS: in this case, the
 * LightMutex class compiles to nothing; it performs no locking whatsoever.
 * It is therefore useful only to protect very small sections of code, during
 * which you are confident there will be no thread yields.
 *
 * In the normal, system-threaded implementation, this class is exactly the
 * same as Mutex.
 *
 * ConditionVars cannot be used with LightMutex; they work only with Mutex.
 *
 * This class inherits its implementation either from MutexDebug or
 * LightMutexDirect, depending on the definition of DEBUG_THREADS.
 */
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE LightMutex : public MutexDebug
#else
class EXPCL_PANDA_PIPELINE LightMutex : public LightMutexDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE LightMutex();
public:
  INLINE explicit LightMutex(const char *name);
PUBLISHED:
  INLINE explicit LightMutex(const std::string &name);
  LightMutex(const LightMutex &copy) = delete;
  ~LightMutex() = default;

  LightMutex &operator = (const LightMutex &copy) = delete;
};

#include "lightMutex.I"

#endif
