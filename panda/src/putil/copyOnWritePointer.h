// Filename: copyOnWritePointer.h
// Created by:  drose (09Apr07)
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

#ifndef COPYONWRITEPOINTER_H
#define COPYONWRITEPOINTER_H

#include "pandabase.h"

#include "copyOnWriteObject.h"
#include "pointerTo.h"
#include "dcast.h"

// Should we implement full thread protection for CopyOnWritePointer?
// If we can be assured that no other thread will interrupt while a
// write pointer is held, we don't need thread protection.
#if defined(HAVE_THREADS) && !(defined(SIMPLE_THREADS) && defined(SIMPLE_THREADS_NO_IMPLICIT_YIELD))
  #define COW_THREADED 1
#else
  #undef COW_THREADED
#endif

////////////////////////////////////////////////////////////////////
//       Class : CopyOnWritePointer
// Description : This safely stores the primary, owned pointer to a
//               CopyOnWriteObject.  At any time, you may call
//               get_read_pointer() or get_write_pointer() to get a
//               read-only or modifiable pointer to the object stored.
//
//               There may be multiple copies of a CopyOnWritePointer
//               which all refer to the same shared object.  They will
//               negotiate with each other properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL CopyOnWritePointer {
public:
  INLINE CopyOnWritePointer(CopyOnWriteObject *object = NULL);
  INLINE CopyOnWritePointer(const CopyOnWritePointer &copy);
  INLINE void operator = (const CopyOnWritePointer &copy);
  INLINE void operator = (CopyOnWriteObject *object);
  INLINE ~CopyOnWritePointer();

  INLINE bool operator == (const CopyOnWritePointer &other) const;
  INLINE bool operator != (const CopyOnWritePointer &other) const;
  INLINE bool operator < (const CopyOnWritePointer &other) const;

#ifdef COW_THREADED
  CPT(CopyOnWriteObject) get_read_pointer() const;
  PT(CopyOnWriteObject) get_write_pointer();
#else
  INLINE const CopyOnWriteObject *get_read_pointer() const;
  INLINE CopyOnWriteObject *get_write_pointer();
#endif  // COW_THREADED

  INLINE CopyOnWriteObject *get_unsafe_pointer();

  INLINE bool is_null() const;
  INLINE void clear();

  INLINE bool test_ref_count_integrity() const;
  INLINE bool test_ref_count_nonzero() const;

private:
  CopyOnWriteObject *_object;
};


////////////////////////////////////////////////////////////////////
//       Class : CopyOnWritePointerTo
// Description : A template wrapper around the above class, mainly to
//               handle the little typecasting niceties.
////////////////////////////////////////////////////////////////////
template <class T>
class CopyOnWritePointerTo : public CopyOnWritePointer {
public:
  // By hiding this template from interrogate, we improve compile-time
  // speed and memory utilization.
#ifndef CPPPARSER
  typedef T To;

  INLINE CopyOnWritePointerTo(To *object = NULL);
  INLINE CopyOnWritePointerTo(const CopyOnWritePointerTo<T> &copy);
  INLINE void operator = (const CopyOnWritePointerTo<T> &copy);
  INLINE void operator = (To *object);

#ifdef COW_THREADED
  INLINE CPT(To) get_read_pointer() const;
  INLINE PT(To) get_write_pointer();
#else
  INLINE const To *get_read_pointer() const;
  INLINE To *get_write_pointer();
#endif  // COW_THREADED

  INLINE To *get_unsafe_pointer();
#endif  // CPPPARSER
};

#define COWPT(type) CopyOnWritePointerTo< type >

#include "copyOnWritePointer.I"

#endif
