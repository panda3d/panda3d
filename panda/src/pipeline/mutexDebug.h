// Filename: mutexDebug.h
// Created by:  drose (13Feb06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef MUTEXDEBUG_H
#define MUTEXDEBUG_H

#include "pandabase.h"
#include "mutexImpl.h"
#include "conditionVarImpl.h"

class Thread;

#ifdef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : MutexDebug
// Description : This class implements a standard mutex the hard way,
//               by doing everything by hand.  This does allow fancy
//               things like deadlock detection, however.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MutexDebug {
protected:
  MutexDebug(const string &name, bool allow_recursion);
  virtual ~MutexDebug();
private:
  INLINE MutexDebug(const MutexDebug &copy);
  INLINE void operator = (const MutexDebug &copy);

public:
  INLINE void lock() const;
  INLINE void lock(Thread *current_thread) const;
  INLINE void elevate_lock() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  virtual void output(ostream &out) const;

  typedef void VoidFunc();

private:
  void do_lock();
  void do_release();
  bool do_debug_is_locked() const;

  void report_deadlock(Thread *this_thread);

private:
  string _name;
  bool _allow_recursion;
  Thread *_locking_thread;
  int _lock_count;
  ConditionVarImpl _cvar;

  static MutexImpl _global_mutex;

  friend class ConditionVarDebug;
};

INLINE ostream &
operator << (ostream &out, const MutexDebug &m) {
  m.output(out);
  return out;
}

#include "mutexDebug.I"

#endif  //  DEBUG_THREADS

#endif
