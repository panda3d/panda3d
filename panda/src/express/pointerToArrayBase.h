/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToArrayBase.h
 * @author drose
 * @date 2006-10-30
 */

#ifndef POINTERTOARRAYBASE_H
#define POINTERTOARRAYBASE_H

#include "pandabase.h"
#include "pStatCollectorForwardBase.h"
#include "nodeReferenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "memoryBase.h"

/**
 * This defines the object that is actually stored and reference-counted
 * internally by a PointerToArray.  It is basically a NodeReferenceCount-
 * capable STL vector.
 *
 * We use NodeReferenceCount (instead of just ReferenceCount), which adds
 * node_ref() and node_unref() to the standard ref() and unref().  This is
 * particularly useful for GeomVertexArrayData; other classes may or may not
 * find this additional counter useful, but since it adds relatively little
 * overhead (compared with what is presumably a largish array), we go ahead
 * and add it here, even though it is inherited by many different parts of the
 * system that may not use it.
 */
template <class Element>
class ReferenceCountedVector : public NodeReferenceCount, public pvector<Element> {
public:
  typedef typename pvector<Element>::iterator iterator;
  typedef typename pvector<Element>::size_type size_type;

  INLINE ReferenceCountedVector(TypeHandle type_handle);
  INLINE ReferenceCountedVector(size_type initial_size, TypeHandle type_handle);
  INLINE ReferenceCountedVector(const Element *begin, const Element *end, TypeHandle type_handle);
  INLINE ReferenceCountedVector(pvector<Element> &&from);
  ALLOC_DELETED_CHAIN(ReferenceCountedVector<Element>);

  INLINE size_type size() const;

  INLINE iterator insert(iterator position, const Element &x);
  INLINE void insert(iterator position, size_type n, const Element &x);

  INLINE void erase(iterator position);
  INLINE void erase(iterator first, iterator last);

  INLINE void pop_back();
  INLINE void clear();
};

/**
 * This is the base class for PointerToArray and ConstPointerToArray.  Don't
 * try to use it directly; use either derived class instead.
 *
 * This extends PointerToBase to be a pointer to a ReferenceCountedVector,
 * above, which is essentially a reference-counted STL vector.
 */
template <class Element>
class PointerToArrayBase : public PointerToBase<ReferenceCountedVector<Element> > {
public:
  typedef typename PointerToBase<ReferenceCountedVector<Element> >::To To;

protected:
  INLINE PointerToArrayBase(ReferenceCountedVector<Element> *ptr);
  INLINE PointerToArrayBase(const PointerToArrayBase<Element> &copy);
  INLINE PointerToArrayBase(PointerToArrayBase<Element> &&from) noexcept;

PUBLISHED:
  INLINE ~PointerToArrayBase();
};

#include "pointerToArrayBase.I"

#endif
