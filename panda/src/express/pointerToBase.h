// Filename: pointerToBase.h
// Created by:  drose (27Sep04)
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

#ifndef POINTERTOBASE_H
#define POINTERTOBASE_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "referenceCount.h"
#include "typedef.h"
#include "memoryUsage.h"
#include "config_express.h"

////////////////////////////////////////////////////////////////////
//       Class : PointerToBase
// Description : This is the base class for PointerTo and
//               ConstPointerTo.  Don't try to use it directly; use
//               either derived class instead.
////////////////////////////////////////////////////////////////////
template <class T>
class PointerToBase : public PointerToVoid {
public:
  typedef T To;

protected:
  INLINE PointerToBase(To *ptr);
  INLINE PointerToBase(const PointerToBase<T> &copy);
  INLINE ~PointerToBase();

#ifdef USE_MOVE_SEMANTICS
  INLINE PointerToBase(PointerToBase<T> &&from) NOEXCEPT;
  INLINE void reassign(PointerToBase<To> &&from) NOEXCEPT;
#endif

  INLINE void reassign(To *ptr);
  INLINE void reassign(const PointerToBase<To> &copy);

#ifdef DO_MEMORY_USAGE
  void update_type(To *ptr);
#endif  // DO_MEMORY_USAGE

  // No assignment or retrieval functions are declared in
  // PointerToBase, because we will have to specialize on const
  // vs. non-const later.

PUBLISHED:
  INLINE void clear();

  void output(ostream &out) const;
};

template<class T>
INLINE ostream &operator <<(ostream &out, const PointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

#include "pointerToBase.I"

#endif
