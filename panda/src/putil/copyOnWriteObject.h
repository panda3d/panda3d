// Filename: copyOnWriteObject.h
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

#ifndef COPYONWRITEOBJECT_H
#define COPYONWRITEOBJECT_H

#include "pandabase.h"

#include "cachedTypedWritableReferenceCount.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "mutexHolder.h"

// Should we implement full thread protection for CopyOnWritePointer?
// If we can be assured that no other thread will interrupt while a
// write pointer is held, we don't need thread protection.

// Nowadays, this is the same thing as asking if HAVE_THREADS is
// defined.  Maybe we'll just replace COW_THREADED with HAVE_THREADS
// in the future.
#ifdef HAVE_THREADS
  #define COW_THREADED 1
#else
  #undef COW_THREADED
#endif

////////////////////////////////////////////////////////////////////
//       Class : CopyOnWriteObject
// Description : This base class provides basic reference counting,
//               but also can be used with a CopyOnWritePointer to
//               provide get_read_pointer() and get_write_pointer().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL CopyOnWriteObject : public CachedTypedWritableReferenceCount {
public:
  INLINE CopyOnWriteObject();
  INLINE CopyOnWriteObject(const CopyOnWriteObject &copy);
  INLINE void operator = (const CopyOnWriteObject &copy);

PUBLISHED:
#ifdef COW_THREADED
  virtual bool unref() const;
  INLINE void cache_ref() const;
#endif  // COW_THREADED

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy()=0;

private:
#ifdef COW_THREADED
  enum LockStatus {
    LS_unlocked,
    LS_locked_read,
    LS_locked_write,
  };
  Mutex _lock_mutex;
  ConditionVar _lock_cvar;
  LockStatus _lock_status;
  Thread *_locking_thread;
#endif  // COW_THREADED

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    CachedTypedWritableReferenceCount::init_type();
    register_type(_type_handle, "CopyOnWriteObject",
                  CachedTypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class CopyOnWritePointer;
};

////////////////////////////////////////////////////////////////////
//       Class : CopyOnWriteObj
// Description : This is similar to RefCountObj, but it implements a
//               CopyOnWriteObject inheritance instead of a
//               ReferenceCount inheritance.
////////////////////////////////////////////////////////////////////
template<class Base>
class CopyOnWriteObj : public CopyOnWriteObject, public Base {
public:
  INLINE CopyOnWriteObj();
  INLINE CopyOnWriteObj(const Base &copy);
  INLINE CopyOnWriteObj(const CopyOnWriteObj<Base> &copy);
  ALLOC_DELETED_CHAIN(CopyOnWriteObj<Base>);

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type();

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : CopyOnWriteObj1
// Description : For objects (e.g. pvectors) whose constructor
//               takes a single parameter.
////////////////////////////////////////////////////////////////////
template<class Base, class Param1>
class CopyOnWriteObj1 : public CopyOnWriteObject, public Base {
public:
  INLINE CopyOnWriteObj1(Param1 p1);
  INLINE CopyOnWriteObj1(const Base &copy);
  INLINE CopyOnWriteObj1(const CopyOnWriteObj1<Base, Param1> &copy);

  typedef CopyOnWriteObj1<Base, Param1> ThisClass;
  ALLOC_DELETED_CHAIN(ThisClass)

protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "copyOnWriteObject.I"

#endif
