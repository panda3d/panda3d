// Filename: reMutexDirect.h
// Created by:  drose (13Feb06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef REMUTEXDIRECT_H
#define REMUTEXDIRECT_H

#include "pandabase.h"
#include "mutexTrueImpl.h"
#include "conditionVarImpl.h"

class Thread;

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : ReMutexDirect
// Description : This class implements a standard reMutex by making
//               direct calls to the underlying implementation layer.
//               It doesn't perform any debugging operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ReMutexDirect {
protected:
  INLINE ReMutexDirect();
  INLINE ~ReMutexDirect();
private:
  INLINE ReMutexDirect(const ReMutexDirect &copy);
  INLINE void operator = (const ReMutexDirect &copy);

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  BLOCKING INLINE void acquire(Thread *current_thread) const;
  BLOCKING INLINE bool try_acquire() const;
  BLOCKING INLINE bool try_acquire(Thread *current_thread) const;
  INLINE void elevate_lock() const;
  INLINE void release() const;

  INLINE bool debug_is_locked() const;

  INLINE void set_name(const string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE string get_name() const;

  void output(ostream &out) const;

private:
#ifdef HAVE_REMUTEXTRUEIMPL
  ReMutexImpl _impl;

#else
  // If we don't have a reentrant mutex, we have to hand-roll one.
  INLINE void do_acquire();
  void do_acquire(Thread *current_thread);
  INLINE bool do_try_acquire();
  bool do_try_acquire(Thread *current_thread);
  void do_elevate_lock();
  void do_release();

  Thread *_locking_thread;
  int _lock_count;

  MutexTrueImpl _lock_impl;
  ConditionVarImpl _cvar_impl;
#endif  // HAVE_REMUTEXTRUEIMPL

  friend class LightReMutexDirect;
};

INLINE ostream &
operator << (ostream &out, const ReMutexDirect &m) {
  m.output(out);
  return out;
}

#include "reMutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
