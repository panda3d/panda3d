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
    temp._ptr->insert(temp._ptr->begin(), n, Element());
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
  _ptr->reserve(n);
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
  if (_ptr == NULL) {
    return _empty_array.begin();
  }
  return _ptr->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
end() const {
  if (_ptr == NULL) {
    return _empty_array.begin();
  }
  return _ptr->end();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rbegin() const {
  if (_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return _ptr->rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rend() const {
  if (_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return _ptr->rend();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
size() const {
  return (_ptr == NULL) ? 0 : _ptr->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::max_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
max_size() const {
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return _ptr->max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool PointerToArray<Element>::
empty() const {
  return (_ptr == NULL) ? true : _ptr->empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::reserve
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
reserve(TYPENAME PointerToArray<Element>::size_type n) {
  if (_ptr == NULL) {
    reassign(new RefCountObj<pvector<Element> >);
  }
  _ptr->reserve(n);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::capacity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
capacity() const {
  nassertr(_ptr != NULL, 0);
  return _ptr->capacity();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::front
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
front() const {
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!_ptr->empty()) {
    _ptr->push_back(Element());
  }
  return _ptr->front();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
back() const {
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!_ptr->empty()) {
    _ptr->push_back(Element());
  }
  return _ptr->back();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
insert(iterator position, const Element &x) const {
  nassertr(_ptr != NULL, position);
  nassertr(position >= _ptr->begin() &&
           position <= _ptr->end(), position);
  return _ptr->insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
insert(iterator position, size_type n, const Element &x) const {
  nassertv(_ptr != NULL);
  nassertv(position >= _ptr->begin() &&
           position <= _ptr->end());
  _ptr->insert(position, n, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator position) const {
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(position >= _ptr->begin() &&
           position <= _ptr->end());
  _ptr->erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator first, iterator last) const {
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(first >= _ptr->begin() && first <= _ptr->end());
  nassertv(last >= _ptr->begin() && last <= _ptr->end());
  _ptr->erase(first, last);
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
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!_ptr->empty()) {
    _ptr->push_back(Element());
  }
  nassertr(n < _ptr->size(), _ptr->operator[](0));
  return _ptr->operator[](n);
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
//     Function: PointerToArray::push_back
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
push_back(const Element &x) {
  if (_ptr == NULL) {
    reassign(new RefCountObj<pvector<Element> >);
  }
  _ptr->push_back(x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::pop_back
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
pop_back() {
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(!_ptr->empty());
  _ptr->pop_back();
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
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertv(!_ptr->empty());
  _ptr->clear();
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
  return (_ptr == NULL) ? (Element *)NULL : &(_ptr->front());
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
  return (_ptr == NULL) ? (Element *)NULL : &(_ptr->front());
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
  nassertd(_ptr != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return *_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_void_ptr
//       Access: Public
//  Description: Returns the reference to memory where the vector
//               is stored.  To be used only with get_void_ptr
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void* PointerToArray<Element>::
get_void_ptr() const
{
  return PointerToBase<RefCountObj<pvector<Element> > >::_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::set_void_ptr
//       Access: Public
//  Description: Sets this PTA to point to the pointer passed in
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
set_void_ptr(void* p)
{
  reassign((RefCountObj<pvector<Element> > *)p);
}
////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_ref_count
//       Access: Public
//  Description: Returns the reference count of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int PointerToArray<Element>::
get_ref_count() const {
  return (_ptr == NULL) ? 0 : _ptr->get_ref_count();
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
  if (_ptr == NULL) {
    return _empty_array.begin();
  }
  return _ptr->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
end() const {
  if (_ptr == NULL) {
    return _empty_array.begin();
  }
  return _ptr->end();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rbegin() const {
  if (_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return _ptr->rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rend() const {
  if (_ptr == NULL) {
    return _empty_array.rbegin();
  }
  return _ptr->rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
size() const {
  return (_ptr == NULL) ? 0 : _ptr->size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::max_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
max_size() const {
  nassertd(_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return _ptr->max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool ConstPointerToArray<Element>::
empty() const {
  return (_ptr == NULL) ? true : _ptr->empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::capacity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
capacity() const {
  nassertd(_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  return _ptr->capacity();
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
  nassertd(_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!_ptr->empty()) {
    _ptr->push_back(Element());
  }
  nassertr(n < _ptr->size(), _ptr->operator[](0));
  return _ptr->operator[](n);
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
  nassertd(_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!_ptr->empty()) {
    _ptr->push_back(Element());
  }
  return _ptr->front();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
back() const {
  nassertd(_ptr != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new RefCountObj<pvector<Element> >);
  }
  nassertd(!_ptr->empty()) {
    _ptr->push_back(Element());
  }
  return _ptr->back();
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
  return (_ptr == NULL) ? (const Element *)NULL : &(_ptr->front());
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
  return (_ptr == NULL) ? (const Element *)NULL : &(_ptr->front());
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
  nassertd(_ptr != NULL) {
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
  return (_ptr == NULL) ? 0 : _ptr->get_ref_count();
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

