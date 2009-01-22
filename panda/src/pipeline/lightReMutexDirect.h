// Filename: lightReMutexDirect.h
// Created by:  drose (08Oct08)
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

#ifndef LIGHTREMUTEXDIRECT_H
#define LIGHTREMUTEXDIRECT_H

#include "pandabase.h"
#include "mutexImpl.h"
#include "reMutexDirect.h"

class Thread;

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : LightReMutexDirect
// Description : This class implements a standard lightReMutex by making
//               direct calls to the underlying implementation layer.
//               It doesn't perform any debugging operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE LightReMutexDirect {
protected:
  INLINE LightReMutexDirect();
  INLINE ~LightReMutexDirect();
private:
  INLINE LightReMutexDirect(const LightReMutexDirect &copy);
  INLINE void operator = (const LightReMutexDirect &copy);

PUBLISHED:
  BLOCKING INLINE void acquire() const;
  BLOCKING INLINE void acquire(Thread *current_thread) const;
  INLINE void elevate_lock() const;
  INLINE void release() const;

  INLINE bool debug_is_locked() const;

  INLINE void set_name(const string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE string get_name() const;

  void output(ostream &out) const;

private:
#if defined(HAVE_REMUTEXIMPL) && !defined(DO_PSTATS)
  ReMutexImpl _impl;

#else
  // If we don't have a reentrant mutex, use the one we hand-rolled in
  // ReMutexDirect.
  ReMutexDirect _impl;
#endif  // HAVE_REMUTEXIMPL
};

INLINE ostream &
operator << (ostream &out, const LightReMutexDirect &m) {
  m.output(out);
  return out;
}

#include "lightReMutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
