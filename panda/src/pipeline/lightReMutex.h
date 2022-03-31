/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutex.h
 * @author drose
 * @date 2008-10-08
 */

#ifndef LIGHTREMUTEX_H
#define LIGHTREMUTEX_H

#include "pandabase.h"
#include "mutexDebug.h"
#include "lightReMutexDirect.h"

/**
 * A lightweight reentrant mutex.  See LightMutex and ReMutex.
 *
 * This class inherits its implementation either from MutexDebug or
 * LightReMutexDirect, depending on the definition of DEBUG_THREADS.
 */
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE LightReMutex : public MutexDebug
#else
class EXPCL_PANDA_PIPELINE LightReMutex : public LightReMutexDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE LightReMutex();
public:
  INLINE explicit LightReMutex(const char *name);
PUBLISHED:
  INLINE explicit LightReMutex(const std::string &name);
  LightReMutex(const LightReMutex &copy) = delete;
  ~LightReMutex() = default;

  LightReMutex &operator = (const LightReMutex &copy) = delete;
};

#include "lightReMutex.I"

#endif
