// Filename: pointerToArrayBase.I
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


////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector() {
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
#ifdef DO_PSTATS
  set_col(copy._col);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::empty_array constructor
//       Access: Public
//  Description: Creates an array of initial_size elements.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ReferenceCountedVector<Element>::
ReferenceCountedVector(TYPENAME ReferenceCountedVector<Element>::size_type initial_size) :
  pvector<Element>(initial_size)
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
  adjust_size(size(), 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::get_col
//       Access: Public
//  Description: Returns the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PStatCollectorForwardBase *ReferenceCountedVector<Element>::
get_col() const {
#ifdef DO_PSTATS
  return _col;
#else
  return NULL;
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::set_col
//       Access: Public
//  Description: Changes the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
set_col(PStatCollectorForwardBase *col) {
#ifdef DO_PSTATS
  if (_col != col) {
    adjust_size(size(), 0);
    _col = col;
    adjust_size(0, size());
  }
#endif  // DO_PSTATS
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
  adjust_size(0, 1);
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
  adjust_size(0, n);
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
  adjust_size(1, 0);
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
  adjust_size(last - first, 0);
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
  adjust_size(1, 0);
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
  adjust_size(size(), 0);
  pvector<Element>::clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCountedVector::adjust_size
//       Access: Private
//  Description: This internal function is used to update the
//               connected PStatCollector (if any) with the change in
//               size.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ReferenceCountedVector<Element>::
adjust_size(size_t orig_size, size_t new_size) {
#ifdef DO_PSTATS
  if (_col != (PStatCollectorForwardBase *)NULL) {
    _col->add_level((float)new_size - (float)orig_size);
  }
#endif  // DO_PSTATS
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

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayBase::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayBase<Element>::
~PointerToArrayBase() {
}

