// Filename: mutexDebug.h
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

#ifndef MUTEXDEBUG_H
#define MUTEXDEBUG_H

#include "pandabase.h"
#include "mutexTrueImpl.h"
#include "conditionVarImpl.h"
#include "thread.h"
#include "namable.h"
#include "pmap.h"

#ifdef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : MutexDebug
// Description : This class implements a standard mutex the hard way,
//               by doing everything by hand.  This does allow fancy
//               things like deadlock detection, however.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE MutexDebug : public Namable {
protected:
  MutexDebug(const string &name, bool allow_recursion, bool lightweight);
  virtual ~MutexDebug();
private:
  INLINE MutexDebug(const MutexDebug &copy);
  INLINE void operator = (const MutexDebug &copy);

PUBLISHED:
  BLOCKING INLINE void acquire(Thread *current_thread = Thread::get_current_thread()) const;
  BLOCKING INLINE bool try_acquire(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE void elevate_lock() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  virtual void output(ostream &out) const;
  void output_with_holder(ostream &out) const;

  typedef void VoidFunc();

public:
  static void increment_pstats();
  static void decrement_pstats();

private:
  void do_acquire(Thread *current_thread);
  bool do_try_acquire(Thread *current_thread);
  void do_release();
  bool do_debug_is_locked() const;

  void report_deadlock(Thread *current_thread);

private:
  INLINE static MutexTrueImpl *get_global_lock();

  bool _allow_recursion;
  bool _lightweight;
  Thread *_locking_thread;
  int _lock_count;
  char *_deleted_name; // To help I.D. a destructed mutex.

  // For _lightweight mutexes.
  typedef pmap<Thread *, int> MissedThreads;
  MissedThreads _missed_threads;

  ConditionVarImpl _cvar_impl;

  static int _pstats_count;
  static MutexTrueImpl *_global_lock;

  friend class ConditionVarDebug;
  friend class ConditionVarFullDebug;
};

INLINE ostream &
operator << (ostream &out, const MutexDebug &m) {
  m.output(out);
  return out;
}

#include "mutexDebug.I"

#endif  //  DEBUG_THREADS

#endif
