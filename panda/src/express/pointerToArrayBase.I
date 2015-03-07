// Filename: pointerToArrayBase.I
// Created by:  drose (30Oct06)
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


////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(TypeHandle type_handle) : pvector<Element>(type_handle) {
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(const ReferenceCountedVector<Element> &copy) :
  NodeReferenceCount(copy),
  pvector<Element>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::empty_array constructor
//       Access: Public
//  Description: Creates an array of initial_size elements.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(TYPENAME ReferenceCountedVector<Element>::size_type initial_size, TypeHandle type_handle) :
  pvector<Element>(initial_size, type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ReferenceCountedVector<Element>::
~ReferenceCountedVector() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ReferenceCountedVector<Element>::size_type ReferenceCountedVector<Element>::
size() const {
  return pvector<Element>::size();
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ReferenceCountedVector<Element>::iterator ReferenceCountedVector<Element>::
insert(iterator position, const Element &x) {
  return pvector<Element>::insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
insert(iterator position, size_type n, const Element &x) {
  pvector<Element>::insert(position, n, x);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
erase(iterator position) {
  pvector<Element>::erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
erase(iterator first, iterator last) {
  pvector<Element>::erase(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::pop_back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
pop_back() {
  pvector<Element>::pop_back();
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
clear() {
  pvector<Element>::clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayBase<Element>::
PointerToArrayBase(ReferenceCountedVector<Element> *ptr) :
  PointerToBase<ReferenceCountedVector<Element> >(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayBase::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayBase<Element>::
PointerToArrayBase(const PointerToArrayBase<Element> &copy) :
  PointerToBase<ReferenceCountedVector<Element> >(copy)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayBase::Move Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayBase<Element>::
PointerToArrayBase(PointerToArrayBase<Element> &&from) NOEXCEPT :
  PointerToBase<ReferenceCountedVector<Element> >(move(from))
{
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayBase::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayBase<Element>::
~PointerToArrayBase() {
}

