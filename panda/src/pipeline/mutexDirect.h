// Filename: mutexDirect.h
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

#ifndef MUTEXDIRECT_H
#define MUTEXDIRECT_H

#include "pandabase.h"
#include "mutexImpl.h"
#include "pnotify.h"

class Thread;

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : MutexDirect
// Description : This class implements a standard mutex by making
//               direct calls to the underlying implementation layer.
//               It doesn't perform any debugging operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MutexDirect {
protected:
  INLINE MutexDirect();
  INLINE ~MutexDirect();
private:
  INLINE MutexDirect(const MutexDirect &copy);
  INLINE void operator = (const MutexDirect &copy);

public:
  INLINE void lock() const;
  INLINE void release() const;
  INLINE bool debug_is_locked() const;

  void output(ostream &out) const;

private:
  MutexImpl _impl;

  friend class ConditionVarDirect;
};

INLINE ostream &
operator << (ostream &out, const MutexDirect &m) {
  m.output(out);
  return out;
}

#include "mutexDirect.I"

#endif  // !DEBUG_THREADS

#endif
