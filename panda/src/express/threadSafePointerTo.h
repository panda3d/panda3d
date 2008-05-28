// Filename: threadSafePointerTo.h
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

#ifndef THREADSAFEPOINTERTO_H
#define THREADSAFEPOINTERTO_H

#include "pandabase.h"
#include "threadSafePointerToBase.h"

////////////////////////////////////////////////////////////////////
//       Class : ThreadSafePointerTo
// Description : This works exactly like PointerTo, except that the
//               object is designed to be thread-safe: it is generally
//               safe to make unprotected assignments to this pointer,
//               in the sense that the last assignment will win and
//               the reference counts will be properly maintained.
////////////////////////////////////////////////////////////////////
template <class T>
class ThreadSafePointerTo : public ThreadSafePointerToBase<T> {
public:
  typedef TYPENAME ThreadSafePointerToBase<T>::To To;
PUBLISHED:
  INLINE ThreadSafePointerTo(To *ptr = (To *)NULL);
  INLINE ThreadSafePointerTo(const ThreadSafePointerTo<T> &copy);
  INLINE ~ThreadSafePointerTo();

public:
  INLINE To &operator *() const;
  INLINE To *operator -> () const;
  // MSVC.NET 2005 insists that we use T *, and not To *, here.
  INLINE operator T *() const;

PUBLISHED:
  // When downcasting to a derived class from a ThreadSafePointerTo<BaseClass>,
  // C++ would normally require you to cast twice: once to an actual
  // BaseClass pointer, and then again to your desired pointer.  You
  // can use the handy function p() to avoid this first cast and make
  // your code look a bit cleaner.

  // e.g. instead of (MyType *)(BaseClass *)ptr, use (MyType *)ptr.p()

  // If your base class is a derivative of TypedObject, you might want
  // to use the DCAST macro defined in typedObject.h instead,
  // e.g. DCAST(MyType, ptr).  This provides a clean downcast that
  // doesn't require .p() or any double-casting, and it can be
  // run-time checked for correctness.
  INLINE To *p() const;

  INLINE ThreadSafePointerTo<T> &operator = (To *ptr);
  INLINE ThreadSafePointerTo<T> &operator = (const ThreadSafePointerTo<T> &copy);

  // These functions normally wouldn't need to be redefined here, but
  // we do so anyway just to help out interrogate (which doesn't seem
  // to want to automatically export the ThreadSafePointerToBase class).  When
  // this works again in interrogate, we can remove these.
  INLINE bool is_null() const { return ThreadSafePointerToBase<T>::is_null(); }
  INLINE void clear() { ThreadSafePointerToBase<T>::clear(); }
};


////////////////////////////////////////////////////////////////////
//       Class : ThreadSafeConstPointerTo
// Description : 
////////////////////////////////////////////////////////////////////
template <class T>
class ThreadSafeConstPointerTo : public ThreadSafePointerToBase<T> {
public:
  typedef TYPENAME ThreadSafePointerToBase<T>::To To;
PUBLISHED:
  INLINE ThreadSafeConstPointerTo(const To *ptr = (const To *)NULL);
  INLINE ThreadSafeConstPointerTo(const ThreadSafePointerTo<T> &copy);
  INLINE ThreadSafeConstPointerTo(const ThreadSafeConstPointerTo<T> &copy);
  INLINE ~ThreadSafeConstPointerTo();

public:
  INLINE const To &operator *() const;
  INLINE const To *operator -> () const;
  INLINE operator const T *() const;

PUBLISHED:
  INLINE const To *p() const;

  INLINE ThreadSafeConstPointerTo<T> &operator = (const To *ptr);
  INLINE ThreadSafeConstPointerTo<T> &operator = (const ThreadSafePointerTo<T> &copy);
  INLINE ThreadSafeConstPointerTo<T> &operator = (const ThreadSafeConstPointerTo<T> &copy);

  // This functions normally wouldn't need to be redefined here, but
  // we do so anyway just to help out interrogate (which doesn't seem
  // to want to automatically export the ThreadSafePointerToBase class).  When
  // this works again in interrogate, we can remove this.
  INLINE void clear() { ThreadSafePointerToBase<T>::clear(); }
};

#define TSPT(type) ThreadSafePointerTo< type >
#define TSCPT(type) ThreadSafeConstPointerTo< type >

#include "threadSafePointerTo.I"

#endif
