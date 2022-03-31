/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutexDirect.h
 * @author drose
 * @date 2008-10-08
 */

#ifndef LIGHTMUTEXDIRECT_H
#define LIGHTMUTEXDIRECT_H

#include "pandabase.h"
#include "mutexImpl.h"
#include "mutexTrueImpl.h"
#include "pnotify.h"

class Thread;

#ifndef DEBUG_THREADS

/**
 * This class implements a lightweight Mutex by making direct calls to the
 * underlying implementation layer.  It doesn't perform any debugging
 * operations.
 */
class EXPCL_PANDA_PIPELINE LightMutexDirect {
protected:
  LightMutexDirect() = default;
  LightMutexDirect(const LightMutexDirect &copy) = delete;
  ~LightMutexDirect() = default;

  void operator = (const LightMutexDirect &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  INLINE void set_name(const std::string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE std::string get_name() const;

  void output(std::ostream &out) const;

private:
#ifdef DO_PSTATS
  // When PStats is compiled in, we use the full implementation of LightMutex,
  // even in the SIMPLE_THREADS case.  We have to do this since any PStatTimer
  // call may trigger a context switch, and any low-level context switch
  // requires all containing mutexes to be true mutexes.
  mutable MutexTrueImpl _impl;
#else
  mutable MutexImpl _impl;
#endif  // DO_PSTATS
};

INLINE std::ostream &
operator << (std::ostream &out, const LightMutexDirect &m) {
  m.output(out);
  return out;
}

#include "lightMutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
