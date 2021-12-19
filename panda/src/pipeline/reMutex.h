/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutex.h
 * @author drose
 * @date 2006-01-15
 */

#ifndef REMUTEX_H
#define REMUTEX_H

#include "pandabase.h"
#include "mutexDebug.h"
#include "reMutexDirect.h"

/**
 * A reentrant mutex.  This kind of mutex can be locked more than once by the
 * thread that already holds it, without deadlock.  The thread must eventually
 * release the mutex the same number of times it locked it.
 *
 * This class inherits its implementation either from MutexDebug or
 * ReMutexDirect, depending on the definition of DEBUG_THREADS.
 */
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE ReMutex : public MutexDebug
#else
class EXPCL_PANDA_PIPELINE ReMutex : public ReMutexDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE ReMutex();
public:
  INLINE explicit ReMutex(const char *name);
PUBLISHED:
  INLINE explicit ReMutex(const std::string &name);
  ReMutex(const ReMutex &copy) = delete;
  ~ReMutex() = default;

  void operator = (const ReMutex &copy) = delete;

  EXTENSION(bool acquire(bool blocking=true) const);
  EXTENSION(bool __enter__());
  EXTENSION(void __exit__(PyObject *, PyObject *, PyObject *));
};

#include "reMutex.I"

#endif
