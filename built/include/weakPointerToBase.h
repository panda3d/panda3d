/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakPointerToBase.h
 * @author drose
 * @date 2004-09-27
 */

#ifndef WEAKPOINTERTOBASE_H
#define WEAKPOINTERTOBASE_H

#include "pandabase.h"
#include "pointerToBase.h"
#include "weakPointerToVoid.h"

/**
 * This is the base class for PointerTo and ConstPointerTo.  Don't try to use
 * it directly; use either derived class instead.
 */
template <class T>
class WeakPointerToBase : public WeakPointerToVoid {
public:
  typedef T To;

protected:
  constexpr WeakPointerToBase() noexcept = default;
  INLINE explicit WeakPointerToBase(To *ptr);
  INLINE WeakPointerToBase(const PointerToBase<T> &copy);
  INLINE WeakPointerToBase(const WeakPointerToBase<T> &copy);
  INLINE WeakPointerToBase(WeakPointerToBase<T> &&from) noexcept;
  template<class Y>
  INLINE WeakPointerToBase(const WeakPointerToBase<Y> &r);
  template<class Y>
  INLINE WeakPointerToBase(WeakPointerToBase<Y> &&r) noexcept;

  INLINE ~WeakPointerToBase();

  void reassign(To *ptr);
  INLINE void reassign(const PointerToBase<To> &copy);
  INLINE void reassign(const WeakPointerToBase<To> &copy);
  INLINE void reassign(WeakPointerToBase<To> &&from) noexcept;
  template<class Y>
  INLINE void reassign(const WeakPointerToBase<Y> &copy);
  template<class Y>
  INLINE void reassign(WeakPointerToBase<Y> &&from) noexcept;

  INLINE void update_type(To *ptr);

  INLINE void lock_into(PointerToBase<To> &locked) const;

  // No assignment or retrieval functions are declared in WeakPointerToBase,
  // because we will have to specialize on const vs.  non-const later.

public:
  // These comparison functions are common to all things PointerTo, so they're
  // defined up here.
#ifndef CPPPARSER
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

  INLINE bool operator == (std::nullptr_t) const;
  INLINE bool operator != (std::nullptr_t) const;
  INLINE bool operator > (std::nullptr_t) const;
  INLINE bool operator <= (std::nullptr_t) const;
  INLINE bool operator >= (std::nullptr_t) const;

  INLINE bool operator == (const WeakPointerToBase<To> &other) const;
  INLINE bool operator != (const WeakPointerToBase<To> &other) const;
  INLINE bool operator > (const WeakPointerToBase<To> &other) const;
  INLINE bool operator <= (const WeakPointerToBase<To> &other) const;
  INLINE bool operator >= (const WeakPointerToBase<To> &other) const;

  INLINE bool operator == (const PointerToBase<To> &other) const;
  INLINE bool operator != (const PointerToBase<To> &other) const;
  INLINE bool operator > (const PointerToBase<To> &other) const;
  INLINE bool operator <= (const PointerToBase<To> &other) const;
  INLINE bool operator >= (const PointerToBase<To> &other) const;

  INLINE bool operator < (const To *other) const;
  INLINE bool operator < (std::nullptr_t) const;
  INLINE bool operator < (const WeakPointerToBase<To> &other) const;
  INLINE bool operator < (const PointerToBase<To> &other) const;
#endif  // CPPPARSER

  template<class Y>
  INLINE bool owner_before(const WeakPointerToBase<Y> &other) const noexcept;
  template<class Y>
  INLINE bool owner_before(const PointerToBase<Y> &other) const noexcept;

  // This is needed to be able to access the privates of other instantiations.
  template<typename Y> friend class WeakPointerToBase;

PUBLISHED:
  INLINE void clear();
  INLINE void refresh() const;

  void output(std::ostream &out) const;
};

template<class T>
INLINE std::ostream &operator <<(std::ostream &out, const WeakPointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

#include "weakPointerToBase.I"

#endif
