/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copyOnWritePointer.h
 * @author drose
 * @date 2007-04-09
 */

#ifndef COPYONWRITEPOINTER_H
#define COPYONWRITEPOINTER_H

#include "pandabase.h"

#include "copyOnWriteObject.h"
#include "pointerTo.h"
#include "dcast.h"

/**
 * This safely stores the primary, owned pointer to a CopyOnWriteObject.  At
 * any time, you may call get_read_pointer() or get_write_pointer() to get a
 * read-only or modifiable pointer to the object stored.
 *
 * There may be multiple copies of a CopyOnWritePointer which all refer to the
 * same shared object.  They will negotiate with each other properly.
 */
class EXPCL_PANDA_PUTIL CopyOnWritePointer {
public:
  INLINE CopyOnWritePointer(CopyOnWriteObject *object = nullptr);
  INLINE CopyOnWritePointer(const CopyOnWritePointer &copy);
  INLINE CopyOnWritePointer(CopyOnWritePointer &&from) noexcept;
  INLINE CopyOnWritePointer(PointerTo<CopyOnWriteObject> &&from) noexcept;
  INLINE ~CopyOnWritePointer();

  INLINE void operator = (const CopyOnWritePointer &copy);
  INLINE void operator = (CopyOnWritePointer &&from) noexcept;
  INLINE void operator = (PointerTo<CopyOnWriteObject> &&from) noexcept;
  INLINE void operator = (CopyOnWriteObject *object);

  INLINE bool operator == (const CopyOnWritePointer &other) const;
  INLINE bool operator != (const CopyOnWritePointer &other) const;
  INLINE bool operator < (const CopyOnWritePointer &other) const;

#ifdef COW_THREADED
  CPT(CopyOnWriteObject) get_read_pointer(Thread *current_thread) const;
  PT(CopyOnWriteObject) get_write_pointer();
#else
  INLINE const CopyOnWriteObject *get_read_pointer(Thread *current_thread) const;
  INLINE CopyOnWriteObject *get_write_pointer();
#endif  // COW_THREADED

  INLINE CopyOnWriteObject *get_unsafe_pointer();

  INLINE bool is_null() const;
  INLINE void clear();

  INLINE bool test_ref_count_integrity() const;
  INLINE bool test_ref_count_nonzero() const;

protected:
  CopyOnWriteObject *_cow_object;
};


/**
 * A template wrapper around the above class, mainly to handle the little
 * typecasting niceties.
 */
template <class T>
class CopyOnWritePointerTo : public CopyOnWritePointer {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  typedef T To;

  INLINE CopyOnWritePointerTo(To *object = nullptr);
  INLINE CopyOnWritePointerTo(const CopyOnWritePointerTo<T> &copy);
  INLINE CopyOnWritePointerTo(CopyOnWritePointerTo &&from) noexcept;
  INLINE CopyOnWritePointerTo(PointerTo<T> &&from) noexcept;

  INLINE void operator = (const CopyOnWritePointerTo<T> &copy);
  INLINE void operator = (To *object);
  INLINE void operator = (CopyOnWritePointerTo &&from) noexcept;
  INLINE void operator = (PointerTo<T> &&from) noexcept;

#ifdef COW_THREADED
  INLINE CPT(To) get_read_pointer(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE PT(To) get_write_pointer();
#else
  INLINE const To *get_read_pointer(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE To *get_write_pointer();
#endif  // COW_THREADED

  INLINE To *get_unsafe_pointer();
#endif  // CPPPARSER
};

#define COWPT(type) CopyOnWritePointerTo< type >

#include "copyOnWritePointer.I"

#endif
