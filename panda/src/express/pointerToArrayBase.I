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
//     Function: PointerToArrayElement::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayElement<Element>::
PointerToArrayElement() {
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayElement<Element>::
PointerToArrayElement(const PointerToArrayElement<Element> &copy) :
  NodeReferenceCount(copy),
  pvector<Element>(copy)
{
#ifdef DO_PSTATS
  set_col(copy._col);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArrayElement<Element>::
~PointerToArrayElement() {
  adjust_size(size(), 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::get_col
//       Access: Public
//  Description: Returns the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PStatCollectorForwardBase *PointerToArrayElement<Element>::
get_col() const {
#ifdef DO_PSTATS
  return _col;
#else
  return NULL;
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::set_col
//       Access: Public
//  Description: Changes the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
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
//     Function: PointerToArrayElement::size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArrayElement<Element>::size_type PointerToArrayElement<Element>::
size() const {
  return pvector<Element>::size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::insert
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArrayElement<Element>::iterator PointerToArrayElement<Element>::
insert(iterator position, const Element &x) {
  adjust_size(0, 1);
  return pvector<Element>::insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::insert
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
insert(iterator position, size_type n, const Element &x) {
  adjust_size(0, n);
  pvector<Element>::insert(position, n, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::erase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
erase(iterator position) {
  adjust_size(1, 0);
  pvector<Element>::erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::erase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
erase(iterator first, iterator last) {
  adjust_size(last - first, 0);
  pvector<Element>::erase(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::pop_back
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
pop_back() {
  adjust_size(1, 0);
  pvector<Element>::pop_back();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::clear
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
clear() {
  adjust_size(size(), 0);
  pvector<Element>::clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArrayElement::adjust_size
//       Access: Private
//  Description: This internal function is used to update the
//               connected PStatCollector (if any) with the change in
//               size.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArrayElement<Element>::
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
PointerToArrayBase(PointerToArrayElement<Element> *ptr) :
  PointerToBase<PointerToArrayElement<Element> >(ptr)
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
  PointerToBase<PointerToArrayElement<Element> >(copy)
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

