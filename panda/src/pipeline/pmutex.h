/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmutex.h
 * @author cary
 * @date 1998-09-16
 */

#ifndef PMUTEX_H
#define PMUTEX_H

#include "pandabase.h"
#include "mutexDebug.h"
#include "mutexDirect.h"

/**
 * A standard mutex, or mutual exclusion lock.  Only one thread can hold
 * ("lock") a mutex at any given time; other threads trying to grab the mutex
 * will block until the holding thread releases it.
 *
 * The standard mutex is not reentrant: a thread may not attempt to lock it
 * twice.  Although this may happen to work on some platforms (e.g.  Win32),
 * it will not work on all platforms; on some platforms, a thread can deadlock
 * itself by attempting to lock the same mutex twice.  If your code requires a
 * reentrant mutex, use the ReMutex class instead.
 *
 * This class inherits its implementation either from MutexDebug or
 * MutexDirect, depending on the definition of DEBUG_THREADS.
 */
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE Mutex : public MutexDebug
#else
class EXPCL_PANDA_PIPELINE Mutex : public MutexDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE Mutex();
public:
  INLINE Mutex(const char *name);
PUBLISHED:
  INLINE explicit Mutex(const std::string &name);
  Mutex(const Mutex &copy) = delete;
  ~Mutex() = default;

  void operator = (const Mutex &copy) = delete;

  EXTENSION(bool acquire(bool blocking=true) const);
  EXTENSION(bool __enter__());
  EXTENSION(void __exit__(PyObject *, PyObject *, PyObject *));

public:
  // This is a global mutex set aside for the purpose of protecting Notify
  // messages from being interleaved between threads.
  static Mutex _notify_mutex;
};

#include "pmutex.I"

#endif
