/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexDirect.h
 * @author drose
 * @date 2006-02-13
 */

#ifndef MUTEXDIRECT_H
#define MUTEXDIRECT_H

#include "pandabase.h"
#include "mutexTrueImpl.h"
#include "pnotify.h"

class Thread;

#ifndef DEBUG_THREADS

/**
 * This class implements a standard mutex by making direct calls to the
 * underlying implementation layer.  It doesn't perform any debugging
 * operations.
 */
class EXPCL_PANDA_PIPELINE MutexDirect {
protected:
  MutexDirect() = default;
  MutexDirect(const MutexDirect &copy) = delete;
  ~MutexDirect() = default;

  void operator = (const MutexDirect &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  BLOCKING INLINE bool try_acquire() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  INLINE void set_name(const std::string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE std::string get_name() const;

  void output(std::ostream &out) const;

private:
  mutable MutexTrueImpl _impl;

  friend class ConditionVarDirect;
};

INLINE std::ostream &
operator << (std::ostream &out, const MutexDirect &m) {
  m.output(out);
  return out;
}

#include "mutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
