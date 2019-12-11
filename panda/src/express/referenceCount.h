/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file referenceCount.h
 * @author drose
 * @date 1998-10-23
 */

#ifndef REFERENCECOUNT_H
#define REFERENCECOUNT_H

#include "pandabase.h"
#include "weakReferenceList.h"
#include "typedObject.h"
#include "memoryUsage.h"
#include "memoryBase.h"
#include "config_express.h"
#include "atomicAdjust.h"
#include "numeric_types.h"
#include "deletedChain.h"

#include <stdlib.h>

/**
 * A base class for all things that want to be reference-counted.
 * ReferenceCount works in conjunction with PointerTo to automatically delete
 * objects when the last pointer to them goes away.
 */
class EXPCL_PANDA_EXPRESS ReferenceCount : public MemoryBase {
protected:
  INLINE ReferenceCount();
  INLINE ReferenceCount(const ReferenceCount &);
  INLINE void operator = (const ReferenceCount &);

public:
  virtual INLINE ~ReferenceCount();

PUBLISHED:
  INLINE int get_ref_count() const;
  INLINE void ref() const;
  virtual INLINE bool unref() const;

  // The current reference count.
  MAKE_PROPERTY(ref_count, get_ref_count);

  INLINE bool test_ref_count_integrity() const;
  INLINE bool test_ref_count_nonzero() const;

public:
  INLINE void local_object();
  INLINE bool has_weak_list() const;
  INLINE WeakReferenceList *get_weak_list() const;

  INLINE WeakReferenceList *weak_ref();
  INLINE void weak_unref();

  INLINE bool ref_if_nonzero() const;

protected:
  bool do_test_ref_count_integrity() const;
  bool do_test_ref_count_nonzero() const;

private:
  void create_weak_list();

private:
  enum {
    // We use this value as a flag to indicate an object has been indicated as
    // a local object, and should not be deleted except by its own destructor.
    // Really, any nonzero value would do, but having a large specific number
    // makes the sanity checks easier.
    local_ref_count = 10000000,

    // This value is used as a flag to indicate that an object has just been
    // deleted, and you're looking at deallocated memory.  It's not guaranteed
    // to stick around, of course (since the deleted memory might be
    // repurposed for anything else, including a new object), but if you ever
    // do encounter this value in a reference count field, you screwed up.
    deleted_ref_count = -100,
  };

  mutable AtomicAdjust::Integer _ref_count;
  AtomicAdjust::Pointer _weak_list;  // WeakReferenceList *

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "ReferenceCount");
  }

private:
  static TypeHandle _type_handle;
};

template<class RefCountType>
INLINE void unref_delete(RefCountType *ptr);

/**
 * A "proxy" to use to make a reference-countable object whenever the object
 * cannot inherit from ReferenceCount for some reason.  RefCountPr<MyClass>
 * can be treated as an instance of MyClass directly, for the most part,
 * except that it can be reference counted.
 *
 * If you want to declare a RefCountProxy to something that does not have
 * get_class_type(), you will have to define a template specialization on
 * _get_type_handle() and _do_init_type(), as in typedObject.h.
 */
template<class Base>
class RefCountProxy : public ReferenceCount {
public:
  INLINE RefCountProxy();
  INLINE RefCountProxy(const Base &copy);

  INLINE operator Base &();
  INLINE operator const Base &() const;

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  Base _base;
  static TypeHandle _type_handle;
};


/**
 * Another kind of proxy, similar to RefCountProxy.  This one works by
 * inheriting from the indicated base type, giving it an is-a relation instead
 * of a has-a relation.  As such, it's a little more robust, but only works
 * when the base type is, in fact, a class.
 */
template<class Base>
class RefCountObj : public ReferenceCount, public Base {
public:
  INLINE RefCountObj();
  INLINE RefCountObj(const Base &copy);
  ALLOC_DELETED_CHAIN(RefCountObj<Base>);

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "referenceCount.I"

#endif
