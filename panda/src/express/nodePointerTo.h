/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePointerTo.h
 * @author drose
 * @date 2005-05-07
 */

#ifndef NODEPOINTERTO_H
#define NODEPOINTERTO_H

#include "pandabase.h"
#include "nodePointerToBase.h"

/**
 * This implements the special NodePointerTo template class, which works just
 * like PointerTo except it manages the objects node_ref_count instead of the
 * normal ref_count.
 */
template <class T>
class NodePointerTo : public NodePointerToBase<T> {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  typedef typename NodePointerToBase<T>::To To;
  INLINE NodePointerTo(To *ptr = nullptr);
  INLINE NodePointerTo(const NodePointerTo<T> &copy);
  INLINE NodePointerTo(NodePointerTo<T> &&from) noexcept;

  INLINE NodePointerTo<T> &operator = (NodePointerTo<T> &&from) noexcept;

  INLINE To &operator *() const;
  INLINE To *operator -> () const;

  // MSVC.NET 2005 insists that we use T *, and not To *, here.
  INLINE operator T *() const;

  INLINE To *p() const;

  INLINE NodePointerTo<T> &operator = (To *ptr);
  INLINE NodePointerTo<T> &operator = (const NodePointerTo<T> &copy);
#endif  // CPPPARSER
};


/**
 * A NodeConstPointerTo is similar to a NodePointerTo, except it keeps a const
 * pointer to the thing.
 */
template <class T>
class NodeConstPointerTo : public NodePointerToBase<T> {
public:
  // By hiding this template from interrogate, we improve compile-time speed
  // and memory utilization.
#ifndef CPPPARSER
  typedef typename NodePointerToBase<T>::To To;
  INLINE NodeConstPointerTo(const To *ptr = nullptr);
  INLINE NodeConstPointerTo(const NodePointerTo<T> &copy);
  INLINE NodeConstPointerTo(const NodeConstPointerTo<T> &copy);
  INLINE NodeConstPointerTo(NodePointerTo<T> &&from) noexcept;
  INLINE NodeConstPointerTo(NodeConstPointerTo<T> &&from) noexcept;

  INLINE NodeConstPointerTo<T> &operator = (NodePointerTo<T> &&from) noexcept;
  INLINE NodeConstPointerTo<T> &operator = (NodeConstPointerTo<T> &&from) noexcept;

  INLINE const To &operator *() const;
  INLINE const To *operator -> () const;
  INLINE operator const T *() const;

  INLINE const To *p() const;

  INLINE NodeConstPointerTo<T> &operator = (const To *ptr);
  INLINE NodeConstPointerTo<T> &operator = (const NodePointerTo<T> &copy);
  INLINE NodeConstPointerTo<T> &operator = (const NodeConstPointerTo<T> &copy);
#endif  // CPPPARSER
};

template <class T>
void swap(NodePointerTo<T> &one, NodePointerTo<T> &two) noexcept {
  one.swap(two);
}

template <class T>
void swap(NodeConstPointerTo<T> &one, NodeConstPointerTo<T> &two) noexcept {
  one.swap(two);
}

#define NPT(type) NodePointerTo< type >
#define NCPT(type) NodeConstPointerTo< type >

#include "nodePointerTo.I"

#endif
