// Filename: nodePointerTo.h
// Created by:  drose (07May05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  typedef TYPENAME NodePointerToBase<T>::To To;
  INLINE NodePointerTo(To *ptr = (To *)NULL);
  INLINE NodePointerTo(const NodePointerTo<T> &copy);
  INLINE ~NodePointerTo();

  INLINE To &operator *() const;
  INLINE To *operator -> () const;
  INLINE operator TYPENAME NodePointerToBase<T>::To *() const;

  INLINE To *p() const;

  INLINE NodePointerTo<T> &operator = (To *ptr);
  INLINE NodePointerTo<T> &operator = (const NodePointerTo<T> &copy);
};


////////////////////////////////////////////////////////////////////
//       Class : NodeConstPointerTo
// Description : A NodeConstPointerTo is similar to a NodePointerTo,
//               except it keeps a const pointer to the thing.
////////////////////////////////////////////////////////////////////
template <class T>
class NodeConstPointerTo : public NodePointerToBase<T> {
public:
  typedef TYPENAME NodePointerToBase<T>::To To;
  INLINE NodeConstPointerTo(const To *ptr = (const To *)NULL);
  INLINE NodeConstPointerTo(const NodePointerTo<T> &copy);
  INLINE NodeConstPointerTo(const NodeConstPointerTo<T> &copy);
  INLINE ~NodeConstPointerTo();

  INLINE const To &operator *() const;
  INLINE const To *operator -> () const;
  INLINE operator const TYPENAME NodePointerToBase<T>::To *() const;

  INLINE const To *p() const;

  INLINE NodeConstPointerTo<T> &operator = (const To *ptr);
  INLINE NodeConstPointerTo<T> &operator = (const NodePointerTo<T> &copy);
  INLINE NodeConstPointerTo<T> &operator = (const NodeConstPointerTo<T> &copy);
};

#define NPT(type) NodePointerTo< type >
#define NCPT(type) NodeConstPointerTo< type >

#include "nodePointerTo.I"

#endif
