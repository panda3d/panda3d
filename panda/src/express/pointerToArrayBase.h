// Filename: pointerToArrayBase.h
// Created by:  drose (30Oct06)
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

#ifndef POINTERTOARRAYBASE_H
#define POINTERTOARRAYBASE_H

#include "pandabase.h"
#include "pStatCollectorForwardBase.h"
#include "nodeReferenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "memoryBase.h"

////////////////////////////////////////////////////////////////////
//       Class : PointerToArrayElement
// Description : This defines the object that is actually stored and
//               reference-counted internally by a PointerToArray.  It
//               is basically a NodeReferenceCount-capable STL vector.
//
//               We use NodeReferenceCount (instead of just
//               ReferenceCount), which adds node_ref() and
//               node_unref() to the standard ref() and unref().  This
//               is particularly useful for GeomVertexArrayData; other
//               classes may or may not find this additional counter
//               useful, but since it adds relatively little overhead
//               (compared with what is presumably a largish array),
//               we go ahead and add it here, even though it is
//               inherited by many different parts of the system that
//               may not use it.
////////////////////////////////////////////////////////////////////
template <class Element>
class PointerToArrayElement : public NodeReferenceCount, public pvector<Element> {
public:
  typedef TYPENAME pvector<Element>::iterator iterator;
  typedef TYPENAME pvector<Element>::size_type size_type;

  INLINE PointerToArrayElement();
  INLINE PointerToArrayElement(const PointerToArrayElement<Element> &copy);
  INLINE ~PointerToArrayElement();
  ALLOC_DELETED_CHAIN(PointerToArrayElement<Element>);

  INLINE PStatCollectorForwardBase *get_col() const;
  INLINE void set_col(PStatCollectorForwardBase *col);

  INLINE size_type size() const;

  INLINE iterator insert(iterator position, const Element &x);
  INLINE void insert(iterator position, size_type n, const Element &x);

  INLINE void erase(iterator position);
  INLINE void erase(iterator first, iterator last);

  INLINE void pop_back();
  INLINE void clear();

private:
  INLINE void adjust_size(size_t orig_size, size_t new_size);

#ifdef DO_PSTATS
  PT(PStatCollectorForwardBase) _col;
#endif
};

////////////////////////////////////////////////////////////////////
//       Class : PointerToArrayBase
// Description : This is the base class for PointerToArray and
//               ConstPointerToArray.  Don't try to use it directly;
//               use either derived class instead.
//
//               This extends PointerToBase to be a pointer to a
//               PointerToArrayElement, above, which is essentially a
//               reference-counted STL vector.
////////////////////////////////////////////////////////////////////
template <class Element>
class PointerToArrayBase : public PointerToBase<PointerToArrayElement<Element> > {
public:
  typedef TYPENAME PointerToBase<PointerToArrayElement<Element> >::To To;

protected:
  INLINE PointerToArrayBase(PointerToArrayElement<Element> *ptr);
  INLINE PointerToArrayBase(const PointerToArrayBase<Element> &copy);

PUBLISHED:
  INLINE ~PointerToArrayBase();
};

#include "pointerToArrayBase.I"

#endif

