// Filename: mutexDirect.h
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

#ifndef MUTEXDIRECT_H
#define MUTEXDIRECT_H

#include "pandabase.h"
#include "mutexTrueImpl.h"
#include "pnotify.h"

class Thread;

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : MutexDirect
// Description : This class implements a standard mutex by making
//               direct calls to the underlying implementation layer.
//               It doesn't perform any debugging operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE MutexDirect {
protected:
  INLINE MutexDirect();
  INLINE ~MutexDirect();
private:
  INLINE MutexDirect(const MutexDirect &copy);
  INLINE void operator = (const MutexDirect &copy);

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  BLOCKING INLINE bool try_acquire() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  INLINE void set_name(const string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE string get_name() const;

  void output(ostream &out) const;

private:
  MutexTrueImpl _impl;

  friend class ConditionVarDirect;
  friend class ConditionVarFullDirect;
};

INLINE ostream &
operator << (ostream &out, const MutexDirect &m) {
  m.output(out);
  return out;
}

#include "mutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
