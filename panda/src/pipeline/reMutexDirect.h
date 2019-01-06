/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutexDirect.h
 * @author drose
 * @date 2006-02-13
 */

#ifndef REMUTEXDIRECT_H
#define REMUTEXDIRECT_H

#include "pandabase.h"
#include "mutexTrueImpl.h"
#include "conditionVarImpl.h"
#include "thread.h"

#ifndef DEBUG_THREADS

/**
 * This class implements a standard reMutex by making direct calls to the
 * underlying implementation layer.  It doesn't perform any debugging
 * operations.
 */
class EXPCL_PANDA_PIPELINE ReMutexDirect {
protected:
  INLINE ReMutexDirect();
  ReMutexDirect(const ReMutexDirect &copy) = delete;
  ~ReMutexDirect() = default;

  void operator = (const ReMutexDirect &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  BLOCKING INLINE void acquire(Thread *current_thread) const;
  BLOCKING INLINE bool try_acquire() const;
  BLOCKING INLINE bool try_acquire(Thread *current_thread) const;
  INLINE void elevate_lock() const;
  INLINE void release() const;

  INLINE bool debug_is_locked() const;

  INLINE void set_name(const std::string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE std::string get_name() const;

  void output(std::ostream &out) const;

private:
#ifdef HAVE_REMUTEXTRUEIMPL
  mutable ReMutexTrueImpl _impl;

#else
  // If we don't have a reentrant mutex, we have to hand-roll one.
  INLINE void do_lock();
  void do_lock(Thread *current_thread);
  INLINE bool do_try_lock();
  bool do_try_lock(Thread *current_thread);
  void do_elevate_lock();
  void do_unlock(Thread *current_thread = Thread::get_current_thread());

  Thread *_locking_thread;
  int _lock_count;

  MutexTrueImpl _lock_impl;
  ConditionVarImpl _cvar_impl;
#endif  // HAVE_REMUTEXTRUEIMPL

  friend class LightReMutexDirect;
};

INLINE std::ostream &
operator << (std::ostream &out, const ReMutexDirect &m) {
  m.output(out);
  return out;
}

#include "reMutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
