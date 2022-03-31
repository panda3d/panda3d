/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToArrayBase.I
 * @author drose
 * @date 2006-10-30
 */

/**
 *
 */
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(TypeHandle type_handle) : pvector<Element>(type_handle) {
}

/**
 * Creates an array of initial_size elements.
 */
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(typename ReferenceCountedVector<Element>::size_type initial_size, TypeHandle type_handle) :
  pvector<Element>(initial_size, type_handle)
{
}

/**
 * Creates an array with all elements from begin up to but not including end.
 */
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(const Element *begin, const Element *end, TypeHandle type_handle) :
  pvector<Element>(begin, end, type_handle)
{
}

/**
 * Creates an array that takes its elements from the given vector.
 */
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(pvector<Element> &&from) :
  pvector<Element>(std::move(from))
{
}

/**
 *
 */
template<class Element>
INLINE typename ReferenceCountedVector<Element>::size_type ReferenceCountedVector<Element>::
size() const {
  return pvector<Element>::size();
}

/**
 *
 */
template<class Element>
INLINE typename ReferenceCountedVector<Element>::iterator ReferenceCountedVector<Element>::
insert(iterator position, const Element &x) {
  return pvector<Element>::insert(position, x);
}

/**
 *
 */
template<class Element>
INLINE void ReferenceCountedVector<Element>::
insert(iterator position, size_type n, const Element &x) {
  pvector<Element>::insert(position, n, x);
}

/**
 *
 */
template<class Element>
INLINE void ReferenceCountedVector<Element>::
erase(iterator position) {
  pvector<Element>::erase(position);
}

/**
 *
 */
template<class Element>
INLINE void ReferenceCountedVector<Element>::
erase(iterator first, iterator last) {
  pvector<Element>::erase(first, last);
}

/**
 *
 */
template<class Element>
INLINE void ReferenceCountedVector<Element>::
pop_back() {
  pvector<Element>::pop_back();
}

/**
 *
 */
template<class Element>
INLINE void ReferenceCountedVector<Element>::
clear() {
  pvector<Element>::clear();
}

/**
 *
 */
template<class Element>
INLINE PointerToArrayBase<Element>::
PointerToArrayBase(ReferenceCountedVector<Element> *ptr) :
  PointerToBase<ReferenceCountedVector<Element> >(ptr)
{
}

/**
 *
 */
template<class Element>
INLINE PointerToArrayBase<Element>::
PointerToArrayBase(const PointerToArrayBase<Element> &copy) :
  PointerToBase<ReferenceCountedVector<Element> >(copy)
{
}

/**
 *
 */
template<class Element>
INLINE PointerToArrayBase<Element>::
PointerToArrayBase(PointerToArrayBase<Element> &&from) noexcept :
  PointerToBase<ReferenceCountedVector<Element> >(std::move(from))
{
}

/**
 *
 */
template<class Element>
INLINE PointerToArrayBase<Element>::
~PointerToArrayBase() {
}
