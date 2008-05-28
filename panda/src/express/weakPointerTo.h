// Filename: weakPointerTo.h
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

#ifndef WEAKPOINTERTO_H
#define WEAKPOINTERTO_H

#include "pandabase.h"
#include "weakPointerToBase.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : WeakPointerTo
// Description : WeakPointerTo is similar to PointerTo, except that it
//               does not actually prevent the referenced pointer from
//               deleting.  Instead, the referenced pointer is allowed
//               to delete, but if this happens then was_deleted()
//               will return true, and it will be an assertion error to
//               dereference the pointer thereafter.
////////////////////////////////////////////////////////////////////
template <class T>
class WeakPointerTo : public WeakPointerToBase<T> {
public:
  typedef TYPENAME WeakPointerToBase<T>::To To;
PUBLISHED:
  INLINE WeakPointerTo(To *ptr = (To *)NULL);
  INLINE WeakPointerTo(const PointerTo<T> &copy);
  INLINE WeakPointerTo(const WeakPointerTo<T> &copy);

public:
  INLINE To &operator *() const;
  INLINE To *operator -> () const;
  // MSVC.NET 2005 insists that we use T *, and not To *, here.
  INLINE operator T *() const;

PUBLISHED:
  INLINE To *p() const;
  INLINE To *get_orig() const;

  INLINE WeakPointerTo<T> &operator = (To *ptr);
  INLINE WeakPointerTo<T> &operator = (const PointerTo<T> &copy);
  INLINE WeakPointerTo<T> &operator = (const WeakPointerTo<T> &copy);

  // This function normally wouldn't need to be redefined here, but
  // we do so anyway just to help out interrogate (which doesn't seem
  // to want to automatically export the WeakPointerToBase class).  When
  // this works again in interrogate, we can remove this.
  INLINE void clear() { WeakPointerToBase<T>::clear(); }
};


////////////////////////////////////////////////////////////////////
//       Class : WeakConstPointerTo
// Description : A WeakConstPointerTo is similar to a WeakPointerTo,
//               except it keeps a const pointer to the thing, that
//               will be cleared to NULL when the thing deleted.
////////////////////////////////////////////////////////////////////
template <class T>
class WeakConstPointerTo : public WeakPointerToBase<T> {
public:
  typedef TYPENAME WeakPointerToBase<T>::To To;
PUBLISHED:
  INLINE WeakConstPointerTo(const To *ptr = (const To *)NULL);
  INLINE WeakConstPointerTo(const PointerTo<T> &copy);
  INLINE WeakConstPointerTo(const ConstPointerTo<T> &copy);
  INLINE WeakConstPointerTo(const WeakPointerTo<T> &copy);
  INLINE WeakConstPointerTo(const WeakConstPointerTo<T> &copy);

public:
  INLINE const To &operator *() const;
  INLINE const To *operator -> () const;
  INLINE operator const T *() const;

PUBLISHED:
  INLINE const To *p() const;
  INLINE const To *get_orig() const;

  INLINE WeakConstPointerTo<T> &operator = (const To *ptr);
  INLINE WeakConstPointerTo<T> &operator = (const PointerTo<T> &copy);
  INLINE WeakConstPointerTo<T> &operator = (const ConstPointerTo<T> &copy);
  INLINE WeakConstPointerTo<T> &operator = (const WeakPointerTo<T> &copy);
  INLINE WeakConstPointerTo<T> &operator = (const WeakConstPointerTo<T> &copy);

  // These functions normally wouldn't need to be redefined here, but
  // we do so anyway just to help out interrogate (which doesn't seem
  // to want to automatically export the WeakPointerToBase class).  When
  // this works again in interrogate, we can remove these.
  INLINE bool is_null() const { return WeakPointerToBase<T>::is_null(); }
  INLINE void clear() { WeakPointerToBase<T>::clear(); }
};

#define WPT(type) WeakPointerTo< type >
#define WCPT(type) WeakConstPointerTo< type >

#include "weakPointerTo.I"

#endif
