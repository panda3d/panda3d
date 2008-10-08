// Filename: lightMutexDirect.h
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

#ifndef LIGHTMUTEXDIRECT_H
#define LIGHTMUTEXDIRECT_H

#include "pandabase.h"
#include "mutexImpl.h"

class Thread;

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : LightMutexDirect
// Description : This class implements a lightweight Mutex by making
//               direct calls to the underlying implementation layer.
//               It doesn't perform any debugging operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE LightMutexDirect {
protected:
  INLINE LightMutexDirect();
  INLINE ~LightMutexDirect();
private:
  INLINE LightMutexDirect(const LightMutexDirect &copy);
  INLINE void operator = (const LightMutexDirect &copy);

PUBLISHED:
  BLOCKING INLINE void lock() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  INLINE void set_name(const string &name);
  INLINE void clear_name();
  INLINE bool has_name() const;
  INLINE string get_name() const;

  void output(ostream &out) const;

private:
  MutexImpl _impl;
};

INLINE ostream &
operator << (ostream &out, const LightMutexDirect &m) {
  m.output(out);
  return out;
}

#include "lightMutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
