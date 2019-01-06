/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutexDirect.h
 * @author drose
 * @date 2008-10-08
 */

#ifndef LIGHTREMUTEXDIRECT_H
#define LIGHTREMUTEXDIRECT_H

#include "pandabase.h"
#include "mutexImpl.h"
#include "reMutexDirect.h"
#include "thread.h"

#ifndef DEBUG_THREADS

/**
 * This class implements a standard lightReMutex by making direct calls to the
 * underlying implementation layer.  It doesn't perform any debugging
 * operations.
 */
class EXPCL_PANDA_PIPELINE LightReMutexDirect {
protected:
  LightReMutexDirect() = default;
  LightReMutexDirect(const LightReMutexDirect &copy) = delete;
  ~LightReMutexDirect() = default;

  void operator = (const LightReMutexDirect &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  BLOCKING INLINE void acquire(Thread *current_thread) const;
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
  // If we don't have a reentrant mutex, use the one we hand-rolled in
  // ReMutexDirect.
  mutable ReMutexDirect _impl;
#endif  // HAVE_REMUTEXIMPL
};

INLINE std::ostream &
operator << (std::ostream &out, const LightReMutexDirect &m) {
  m.output(out);
  return out;
}

#include "lightReMutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
