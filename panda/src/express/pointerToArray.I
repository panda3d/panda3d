// Filename: pointerToArray.I
// Created by:  drose (07Jan00)
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

template<class Element>
pvector<Element> PointerToArray<Element>::_empty_array;

template<class Element>
pvector<Element> ConstPointerToArray<Element>::_empty_array;

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray() :
  PointerToBase<RefCountObj<pvector<Element> > >((RefCountObj<pvector<Element> > *)NULL)
{
}

// return an empty array of size n
template<class Element>
INLINE PointerToArray<Element> 
PointerToArray<Element>::empty_array(size_type n) {
  PointerToArray<Element> temp;
  temp.reserve(n);
  ((To *)temp._void_ptr)->insert(((To *)temp._void_ptr)->begin(), n, Element());
  return temp;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(size_type n, const Element &value) :
  PointerToBase<RefCountObj<pvector<Element> > >(new RefCountObj<pvector<Element> >) {
  ((To *)_void_ptr)->reserve(n);
  insert(begin(), n, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(const PointerToArray<Element> &copy) :
  PointerToBase<RefCountObj<pvector<Element> > >(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
begin() const {
  if (_void_ptr == NULL) {
    return _empty_array.begin();
  }
  return ((To *)_void_ptr)->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
end() const {
  if (_void_ptr == NULL) {
    return _empty_array.begin();
  }
  return ((To *)_void_ptr)->end();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rbegin() const {
  if (_void_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)_void_ptr)->rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rend() const {
  if (_void_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)_void_ptr)->rend();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
size() const {
  return (_void_ptr == NULL) ? 0 : ((To *)_void_ptr)->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::max_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
max_size() const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return ((To *)_void_ptr)->max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool PointerToArray<Element>::
empty() const {
  return (_void_ptr == NULL) ? true : ((To *)_void_ptr)->empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::reserve
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
reserve(TYPENAME PointerToArray<Element>::size_type n) {
  if (_void_ptr == NULL) {
    reassign(new RefCountObj<pvector<Element> >);
  }
  ((To *)_void_ptr)->reserve(n);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::capacity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
capacity() const {
  nassertr(_void_ptr != NULL, 0);
  return ((To *)_void_ptr)->capacity();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::front
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
front() const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!((To *)_void_ptr)->empty()) {
    ((To *)_void_ptr)->push_back(Element());
  }
  return ((To *)_void_ptr)->front();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
back() const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!((To *)_void_ptr)->empty()) {
    ((To *)_void_ptr)->push_back(Element());
  }
  return ((To *)_void_ptr)->back();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
insert(iterator position, const Element &x) const {
  nassertr(_void_ptr != NULL, position);
  nassertr(position >= ((To *)_void_ptr)->begin() &&
           position <= ((To *)_void_ptr)->end(), position);
  return ((To *)_void_ptr)->insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
insert(iterator position, size_type n, const Element &x) const {
  nassertv(_void_ptr != NULL);
  nassertv(position >= ((To *)_void_ptr)->begin() &&
           position <= ((To *)_void_ptr)->end());
  ((To *)_void_ptr)->insert(position, n, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator position) const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(position >= ((To *)_void_ptr)->begin() &&
           position <= ((To *)_void_ptr)->end());
  ((To *)_void_ptr)->erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator first, iterator last) const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(first >= ((To *)_void_ptr)->begin() && first <= ((To *)_void_ptr)->end());
  nassertv(last >= ((To *)_void_ptr)->begin() && last <= ((To *)_void_ptr)->end());
  ((To *)_void_ptr)->erase(first, last);
}

#if !defined(WIN32_VC)
////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Indexing operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
operator [](size_type n) const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!((To *)_void_ptr)->empty()) {
    ((To *)_void_ptr)->push_back(Element());
  }
  nassertr(n < ((To *)_void_ptr)->size(), ((To *)_void_ptr)->operator[](0));
  return ((To *)_void_ptr)->operator[](n);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Indexing operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
operator [](int n) const {
  return operator[]((size_type)n);
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_element
//       Access: Published
//  Description: This method exists mainly to access the elements of
//               the array easily from a high-level language such as
//               Python, especially on Windows, where the above index
//               element accessor methods can't be defined because of
//               a confusion with the pointer typecast operator.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::const_reference PointerToArray<Element>::
get_element(size_type n) const {
  return (*this)[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::set_element
//       Access: Published
//  Description: This method exists mainly to access the elements of
//               the array easily from a high-level language such as
//               Python, especially on Windows, where the above index
//               element accessor methods can't be defined because of
//               a confusion with the pointer typecast operator.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
set_element(size_type n, const_reference value) {
  nassertv(n < ((To *)_void_ptr)->size());
  (*this)[n] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::push_back
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
push_back(const Element &x) {
  if (_void_ptr == NULL) {
    reassign(new RefCountObj<pvector<Element> >);
  }
  ((To *)_void_ptr)->push_back(x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::pop_back
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
pop_back() {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(!((To *)_void_ptr)->empty());
  ((To *)_void_ptr)->pop_back();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::make_empty
//       Access: Published
//  Description: Empties the array pointed to.  This is different from
//               clear(), which reassigns the pointer to a NULL
//               pointer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
make_empty() {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(!((To *)_void_ptr)->empty());
  ((To *)_void_ptr)->clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Typecast operator
//       Access: Public
//  Description: The pointer typecast operator is convenient for
//               maintaining the fiction that we actually have a
//               C-style array.  It returns the address of the first
//               element in the array, unless the pointer is
//               unassigned, in which case it returns NULL.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element>::
operator Element *() const {
  return (_void_ptr == NULL) ? (Element *)NULL : &(((To *)_void_ptr)->front());
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::p
//       Access: Public
//  Description: Function p() is similar to the function from
//               PointerTo.  It does the same thing: it returns the
//               same thing as the typecast operator, above.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE Element *PointerToArray<Element>::
p() const {
  return (_void_ptr == NULL) ? (Element *)NULL : &(((To *)_void_ptr)->front());
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::v
//       Access: Public
//  Description: To access the vector itself, for more direct fiddling
//               with some of the vector's esoteric functionality.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE pvector<Element> &PointerToArray<Element>::
v() const {
  nassertd(_void_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return *((To *)_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_void_ptr
//       Access: Public
//  Description: Returns the reference to memory where the vector
//               is stored.  To be used only with set_void_ptr
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void *PointerToArray<Element>::
get_void_ptr() const {
  return _void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::set_void_ptr
//       Access: Public
//  Description: Sets this PTA to point to the pointer passed in
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
set_void_ptr(void *p) {
  reassign((To *)p);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_ref_count
//       Access: Public
//  Description: Returns the reference count of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int PointerToArray<Element>::
get_ref_count() const {
  return (_void_ptr == NULL) ? 0 : ((To *)_void_ptr)->get_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element> &PointerToArray<Element>::
operator = (RefCountObj<pvector<Element> > *ptr) {
  reassign(ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element> &PointerToArray<Element>::
operator = (const PointerToArray<Element> &copy) {
  reassign(copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::clear
//       Access: Public
//  Description: To empty the PTA, use the clear() method, since
//               assignment to NULL is problematic (given the
//               ambiguity of the pointer type of NULL).
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
clear() {
  reassign((RefCountObj<pvector<Element> > *)NULL);
}



////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray() :
  PointerToBase<RefCountObj<pvector<Element> > >((RefCountObj<pvector<Element> > *)NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const PointerToArray<Element> &copy) :
  PointerToBase<RefCountObj<pvector<Element> > >(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const ConstPointerToArray<Element> &copy) :
  PointerToBase<RefCountObj<pvector<Element> > >(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
begin() const {
  if (_void_ptr == NULL) {
    return _empty_array.begin();
  }
  return ((To *)_void_ptr)->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
end() const {
  if (_void_ptr == NULL) {
    return _empty_array.begin();
  }
  return ((To *)_void_ptr)->end();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rbegin() const {
  if (_void_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)_void_ptr)->rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rend() const {
  if (_void_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)_void_ptr)->rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
size() const {
  return (_void_ptr == NULL) ? 0 : ((To *)_void_ptr)->size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::max_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
max_size() const {
  nassertd(_void_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return ((To *)_void_ptr)->max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool ConstPointerToArray<Element>::
empty() const {
  return (_void_ptr == NULL) ? true : ((To *)_void_ptr)->empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::capacity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
capacity() const {
  nassertd(_void_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return ((To *)_void_ptr)->capacity();
}

#ifndef WIN32_VC
////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Indexing operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
operator [](size_type n) const {
  nassertd(_void_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!((To *)_void_ptr)->empty()) {
    ((To *)_void_ptr)->push_back(Element());
  }
  nassertr(n < ((To *)_void_ptr)->size(), ((To *)_void_ptr)->operator[](0));
  return ((To *)_void_ptr)->operator[](n);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Indexing operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
operator [](int n) const {
  return operator[]((size_type)n);
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::front
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
front() const {
  nassertd(_void_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!((To *)_void_ptr)->empty()) {
    ((To *)_void_ptr)->push_back(Element());
  }
  return ((To *)_void_ptr)->front();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
back() const {
  nassertd(_void_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!((To *)_void_ptr)->empty()) {
    ((To *)_void_ptr)->push_back(Element());
  }
  return ((To *)_void_ptr)->back();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Typecast operator
//       Access: Public
//  Description: The pointer typecast operator is convenient for
//               maintaining the fiction that we actually have a
//               C-style array.  It returns the address of the first
//               element in the array, unless the pointer is
//               unassigned, in which case it returns NULL.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
operator const Element *() const {
  return (_void_ptr == NULL) ? (const Element *)NULL : &(((To *)_void_ptr)->front());
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::p
//       Access: Public
//  Description: Function p() is similar to the function from
//               ConstPointerTo.  It does the same thing: it returns the
//               same thing as the typecast operator, above.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE const Element *ConstPointerToArray<Element>::
p() const {
  return (_void_ptr == NULL) ? (const Element *)NULL : &(((To *)_void_ptr)->front());
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::v
//       Access: Public
//  Description: To access the vector itself, for more direct fiddling
//               with some of the vector's esoteric functionality.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE const pvector<Element> &ConstPointerToArray<Element>::
v() const {
  nassertd(_void_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return *_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::get_ref_count
//       Access: Public
//  Description: Returns the reference count of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int ConstPointerToArray<Element>::
get_ref_count() const {
  return (_void_ptr == NULL) ? 0 : ((To *)_void_ptr)->get_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (RefCountObj<pvector<Element> > *ptr) {
  reassign(ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (const PointerToArray<Element> &copy) {
  reassign(copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (const ConstPointerToArray<Element> &copy) {
  reassign(copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::clear
//       Access: Public
//  Description: To empty the PTA, use the clear() method, since
//               assignment to NULL is problematic (given the
//               ambiguity of the pointer type of NULL).
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ConstPointerToArray<Element>::
clear() {
  reassign((RefCountObj<pvector<Element> > *)NULL);
}

