// Filename: threadSafePointerToBase.h
// Created by:  drose (28Apr06)
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

#ifndef THREADSAFEPOINTERTOBASE_H
#define THREADSAFEPOINTERTOBASE_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "referenceCount.h"
#include "typedef.h"
#include "memoryUsage.h"
#include "config_express.h"
#include "atomicAdjust.h"

////////////////////////////////////////////////////////////////////
//       Class : ThreadSafePointerToBase
// Description : This is the base class for ThreadSafePointerTo and
//               ThreadSafeConstPointerTo.  Don't try to use it
//               directly; use either derived class instead.
////////////////////////////////////////////////////////////////////
template <class T>
class ThreadSafePointerToBase : public PointerToVoid {
public:
  typedef T To;

protected:
  INLINE ThreadSafePointerToBase(To *ptr);
  INLINE ThreadSafePointerToBase(const ThreadSafePointerToBase<T> &copy);
  INLINE ~ThreadSafePointerToBase();

  INLINE void reassign(To *ptr);
  INLINE void reassign(const ThreadSafePointerToBase<To> &copy);

#ifdef DO_MEMORY_USAGE
  void update_type(To *ptr);
#endif  // DO_MEMORY_USAGE

  // No assignment or retrieval functions are declared in
  // ThreadSafePointerToBase, because we will have to specialize on const
  // vs. non-const later.

PUBLISHED:
  INLINE void clear();

  void output(ostream &out) const;
};

template<class T>
INLINE ostream &operator <<(ostream &out, const ThreadSafePointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

#include "threadSafePointerToBase.I"

#endif
