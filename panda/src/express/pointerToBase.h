// Filename: pointerToBase.h
// Created by:  drose (27Sep04)
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

  void reassign(To *ptr);
  INLINE void reassign(const PointerToBase<To> &copy);

  // No assignment or retrieval functions are declared in
  // PointerToBase, because we will have to specialize on const
  // vs. non-const later.

public:
  // These comparison functions are common to all things PointerTo, so
  // they're defined up here.
#ifndef CPPPARSER
#ifndef WIN32_VC
  INLINE bool operator == (const To *other) const;
  INLINE bool operator != (const To *other) const;
  INLINE bool operator > (const To *other) const;
  INLINE bool operator <= (const To *other) const;
  INLINE bool operator >= (const To *other) const;
  INLINE bool operator == (To *other) const;
  INLINE bool operator != (To *other) const;
  INLINE bool operator > (To *other) const;
  INLINE bool operator <= (To *other) const;
  INLINE bool operator >= (To *other) const;

  INLINE bool operator == (const PointerToBase<To> &other) const;
  INLINE bool operator != (const PointerToBase<To> &other) const;
  INLINE bool operator > (const PointerToBase<To> &other) const;
  INLINE bool operator <= (const PointerToBase<To> &other) const;
  INLINE bool operator >= (const PointerToBase<To> &other) const;
#endif  // WIN32_VC
  INLINE bool operator < (const To *other) const;
  INLINE bool operator < (const PointerToBase<To> &other) const;
#endif  // CPPPARSER
  INLINE size_t get_hash() const;

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
