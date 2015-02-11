// Filename: nodePointerTo.h
// Created by:  drose (07May05)
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

#ifndef NODEPOINTERTO_H
#define NODEPOINTERTO_H

#include "pandabase.h"
#include "nodePointerToBase.h"

////////////////////////////////////////////////////////////////////
//       Class : NodePointerTo
// Description : This implements the special NodePointerTo template
//               class, which works just like PointerTo except it
//               manages the objects node_ref_count instead of the
//               normal ref_count.
////////////////////////////////////////////////////////////////////
template <class T>
class NodePointerTo : public NodePointerToBase<T> {
public:
  // By hiding this template from interrogate, we improve compile-time
  // speed and memory utilization.
#ifndef CPPPARSER
  typedef TYPENAME NodePointerToBase<T>::To To;
  INLINE NodePointerTo(To *ptr = (To *)NULL);
  INLINE NodePointerTo(const NodePointerTo<T> &copy);
  INLINE ~NodePointerTo();

#ifdef USE_MOVE_SEMANTICS
  INLINE NodePointerTo(NodePointerTo<T> &&from) NOEXCEPT;
  INLINE NodePointerTo<T> &operator = (NodePointerTo<T> &&from) NOEXCEPT;
#endif

  INLINE To &operator *() const;
  INLINE To *operator -> () const;

  // MSVC.NET 2005 insists that we use T *, and not To *, here.
  INLINE operator T *() const;

  INLINE To *p() const;

  INLINE NodePointerTo<T> &operator = (To *ptr);
  INLINE NodePointerTo<T> &operator = (const NodePointerTo<T> &copy);
#endif  // CPPPARSER
};


////////////////////////////////////////////////////////////////////
//       Class : NodeConstPointerTo
// Description : A NodeConstPointerTo is similar to a NodePointerTo,
//               except it keeps a const pointer to the thing.
////////////////////////////////////////////////////////////////////
template <class T>
class NodeConstPointerTo : public NodePointerToBase<T> {
public:
  // By hiding this template from interrogate, we improve compile-time
  // speed and memory utilization.
#ifndef CPPPARSER
  typedef TYPENAME NodePointerToBase<T>::To To;
  INLINE NodeConstPointerTo(const To *ptr = (const To *)NULL);
  INLINE NodeConstPointerTo(const NodePointerTo<T> &copy);
  INLINE NodeConstPointerTo(const NodeConstPointerTo<T> &copy);
  INLINE ~NodeConstPointerTo();

#ifdef USE_MOVE_SEMANTICS
  INLINE NodeConstPointerTo(NodePointerTo<T> &&from) NOEXCEPT;
  INLINE NodeConstPointerTo(NodeConstPointerTo<T> &&from) NOEXCEPT;
  INLINE NodeConstPointerTo<T> &operator = (NodePointerTo<T> &&from) NOEXCEPT;
  INLINE NodeConstPointerTo<T> &operator = (NodeConstPointerTo<T> &&from) NOEXCEPT;
#endif

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
void swap(NodePointerTo<T> &one, NodePointerTo<T> &two) NOEXCEPT {
  one.swap(two);
}

template <class T>
void swap(NodeConstPointerTo<T> &one, NodeConstPointerTo<T> &two) NOEXCEPT {
  one.swap(two);
}

#define NPT(type) NodePointerTo< type >
#define NCPT(type) NodeConstPointerTo< type >

#include "nodePointerTo.I"

#endif
