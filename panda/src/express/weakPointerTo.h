/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakPointerTo.h
 * @author drose
 * @date 2004-09-27
 */

#ifndef WEAKPOINTERTO_H
#define WEAKPOINTERTO_H

#include "pandabase.h"
#include "weakPointerToBase.h"
#include "pointerTo.h"

/**
 * WeakPointerTo is similar to PointerTo, except that it does not actually
 * prevent the referenced pointer from deleting.  Instead, the referenced
 * pointer is allowed to delete, but if this happens then was_deleted() will
 * return true, and it will be an assertion error to dereference the pointer
 * thereafter.
 */
template <class T>
class WeakPointerTo : public WeakPointerToBase<T> {
public:
  typedef typename WeakPointerToBase<T>::To To;
PUBLISHED:
  constexpr WeakPointerTo() noexcept = default;
  INLINE WeakPointerTo(To *ptr);
  INLINE WeakPointerTo(const PointerTo<T> &copy);
  INLINE WeakPointerTo(const WeakPointerTo<T> &copy);

public:
  INLINE WeakPointerTo(WeakPointerTo<T> &&from) noexcept;

  template<class Y>
  ALWAYS_INLINE WeakPointerTo(const WeakPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakPointerTo(const PointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakPointerTo(WeakPointerTo<Y> &&r) noexcept;

  INLINE To &operator *() const;
  INLINE To *operator -> () const;
  // MSVC.NET 2005 insists that we use T *, and not To *, here.
  INLINE explicit operator T *() const;

PUBLISHED:
  INLINE PointerTo<T> lock() const;
  INLINE To *p() const;
  INLINE To *get_orig() const;

  INLINE WeakPointerTo<T> &operator = (To *ptr);
  INLINE WeakPointerTo<T> &operator = (const PointerTo<T> &copy);
  INLINE WeakPointerTo<T> &operator = (const WeakPointerTo<T> &copy);

public:
  INLINE WeakPointerTo<T> &operator = (WeakPointerTo<T> &&from) noexcept;

  template<class Y>
  ALWAYS_INLINE WeakPointerTo<T> &operator = (const WeakPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakPointerTo<T> &operator = (const PointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakPointerTo<T> &operator = (WeakPointerTo<Y> &&r) noexcept;

PUBLISHED:
  // This function normally wouldn't need to be redefined here, but we do so
  // anyway just to help out interrogate (which doesn't seem to want to
  // automatically export the WeakPointerToBase class).  When this works again
  // in interrogate, we can remove this.
  INLINE void clear() { WeakPointerToBase<T>::clear(); }
};


/**
 * A WeakConstPointerTo is similar to a WeakPointerTo, except it keeps a const
 * pointer to the thing, that will be cleared to NULL when the thing deleted.
 */
template <class T>
class WeakConstPointerTo : public WeakPointerToBase<T> {
public:
  typedef typename WeakPointerToBase<T>::To To;
PUBLISHED:
  constexpr WeakConstPointerTo() noexcept = default;
  INLINE WeakConstPointerTo(const To *ptr);
  INLINE WeakConstPointerTo(const PointerTo<T> &copy);
  INLINE WeakConstPointerTo(const ConstPointerTo<T> &copy);
  INLINE WeakConstPointerTo(const WeakPointerTo<T> &copy);
  INLINE WeakConstPointerTo(const WeakConstPointerTo<T> &copy);

public:
  INLINE WeakConstPointerTo(WeakPointerTo<T> &&from) noexcept;
  INLINE WeakConstPointerTo(WeakConstPointerTo<T> &&from) noexcept;

  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo(const WeakPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo(const WeakConstPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo(const PointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo(const ConstPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo(WeakPointerTo<Y> &&r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo(WeakConstPointerTo<Y> &&r) noexcept;

  INLINE const To &operator *() const;
  INLINE const To *operator -> () const;
  INLINE explicit operator const T *() const;

PUBLISHED:
  INLINE ConstPointerTo<T> lock() const;
  INLINE const To *p() const;
  INLINE const To *get_orig() const;

  INLINE WeakConstPointerTo<T> &operator = (const To *ptr);
  INLINE WeakConstPointerTo<T> &operator = (const PointerTo<T> &copy);
  INLINE WeakConstPointerTo<T> &operator = (const ConstPointerTo<T> &copy);
  INLINE WeakConstPointerTo<T> &operator = (const WeakPointerTo<T> &copy);
  INLINE WeakConstPointerTo<T> &operator = (const WeakConstPointerTo<T> &copy);

public:
  INLINE WeakConstPointerTo<T> &operator = (WeakPointerTo<T> &&from) noexcept;
  INLINE WeakConstPointerTo<T> &operator = (WeakConstPointerTo<T> &&from) noexcept;

  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo<T> &operator = (const WeakPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo<T> &operator = (const WeakConstPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo<T> &operator = (const PointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo<T> &operator = (const ConstPointerTo<Y> &r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo<T> &operator = (WeakPointerTo<Y> &&r) noexcept;
  template<class Y>
  ALWAYS_INLINE WeakConstPointerTo<T> &operator = (WeakConstPointerTo<Y> &&r) noexcept;

PUBLISHED:
  // These functions normally wouldn't need to be redefined here, but we do so
  // anyway just to help out interrogate (which doesn't seem to want to
  // automatically export the WeakPointerToBase class).  When this works again
  // in interrogate, we can remove these.
  INLINE bool is_null() const { return WeakPointerToBase<T>::is_null(); }
  INLINE void clear() { WeakPointerToBase<T>::clear(); }
};

// Provide specializations of std::owner_less, for using a WPT as a map key.
namespace std {
  template<class T>
  struct owner_less<WeakPointerTo<T> > {
    bool operator () (const WeakPointerTo<T> &lhs,
                      const WeakPointerTo<T> &rhs) const noexcept {
      return lhs.owner_before(rhs);
    }
  };

  template<class T>
  struct owner_less<WeakConstPointerTo<T> > {
    bool operator () (const WeakConstPointerTo<T> &lhs,
                      const WeakConstPointerTo<T> &rhs) const noexcept {
      return lhs.owner_before(rhs);
    }
  };
}

#define WPT(type) WeakPointerTo< type >
#define WCPT(type) WeakConstPointerTo< type >

#include "weakPointerTo.I"

#endif
