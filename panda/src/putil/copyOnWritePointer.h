// Filename: copyOnWritePointer.h
// Created by:  drose (09Apr07)
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

#ifndef COPYONWRITEPOINTER_H
#define COPYONWRITEPOINTER_H

#include "pandabase.h"

#include "copyOnWriteObject.h"
#include "pointerTo.h"
#include "dcast.h"

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

#ifdef USE_MOVE_SEMANTICS
  INLINE CopyOnWritePointer(CopyOnWritePointer &&move) NOEXCEPT;
  INLINE void operator = (CopyOnWritePointer &&move) NOEXCEPT;
#endif

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
  CopyOnWriteObject *_cow_object;
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

#ifdef USE_MOVE_SEMANTICS
  INLINE CopyOnWritePointerTo(CopyOnWritePointerTo &&move) NOEXCEPT;
  INLINE void operator = (CopyOnWritePointerTo &&move) NOEXCEPT;
#endif

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
