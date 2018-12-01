/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToBase.h
 * @author drose
 * @date 2004-09-27
 */

#ifndef POINTERTOBASE_H
#define POINTERTOBASE_H

#include "pandabase.h"
#include "pointerToVoid.h"
#include "referenceCount.h"
#include "typedef.h"
#include "memoryUsage.h"
#include "config_express.h"

/**
 * This is the base class for PointerTo and ConstPointerTo.  Don't try to use
 * it directly; use either derived class instead.
 */
template <class T>
class PointerToBase : public PointerToVoid {
public:
  typedef T To;

protected:
  ALWAYS_INLINE constexpr PointerToBase() noexcept = default;
  INLINE PointerToBase(To *ptr);
  INLINE PointerToBase(const PointerToBase<T> &copy);
  INLINE PointerToBase(PointerToBase<T> &&from) noexcept;
  template<class Y>
  INLINE PointerToBase(PointerToBase<Y> &&r) noexcept;

  INLINE ~PointerToBase();

  INLINE void reassign(To *ptr);
  INLINE void reassign(const PointerToBase<To> &copy);
  INLINE void reassign(PointerToBase<To> &&from) noexcept;
  template<class Y>
  INLINE void reassign(PointerToBase<Y> &&from) noexcept;

  INLINE void update_type(To *ptr);

  // No assignment or retrieval functions are declared in PointerToBase,
  // because we will have to specialize on const vs.  non-const later.

  // This is needed to be able to access the privates of other instantiations.
  template<typename Y> friend class PointerToBase;
  template<typename Y> friend class WeakPointerToBase;

PUBLISHED:
  ALWAYS_INLINE void clear();

  void output(std::ostream &out) const;
};

template<class T>
INLINE std::ostream &operator <<(std::ostream &out, const PointerToBase<T> &pointer) {
  pointer.output(out);
  return out;
}

#include "pointerToBase.I"

#endif
