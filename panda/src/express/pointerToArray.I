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
  PointerToArrayBase<Element>((ReferenceCountedVector<Element> *)NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::empty_array
//       Access: Published, Static
//  Description: Return an empty array of size n
////////////////////////////////////////////////////////////////////
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
  PointerToArrayBase<Element>(new ReferenceCountedVector<Element>) {
  ((To *)(this->_void_ptr))->reserve(n);
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
  PointerToArrayBase<Element>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_col
//       Access: Published
//  Description: Returns the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PStatCollectorForwardBase *PointerToArray<Element>::
get_col() const {
#ifdef DO_PSTATS
  if ((this->_void_ptr) != NULL) {
    return ((To *)(this->_void_ptr))->get_col();
  }
#endif  // DO_PSTATS
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::set_col
//       Access: Published
//  Description: Changes the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
set_col(PStatCollectorForwardBase *col) {
#ifdef DO_PSTATS
  if ((this->_void_ptr) == NULL) {
    reassign(new ReferenceCountedVector<Element>);
  }
  ((To *)(this->_void_ptr))->set_col(col);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
begin() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
end() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->end();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rbegin() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rend() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rend();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
size() const {
  return ((this->_void_ptr) == NULL) ? 0 : ((To *)(this->_void_ptr))->size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::max_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
max_size() const {
  nassertd((this->_void_ptr) != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  return ((To *)(this->_void_ptr))->max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool PointerToArray<Element>::
empty() const {
  return ((this->_void_ptr) == NULL) ? true : ((To *)(this->_void_ptr))->empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::reserve
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
reserve(TYPENAME PointerToArray<Element>::size_type n) {
  if ((this->_void_ptr) == NULL) {
    reassign(new ReferenceCountedVector<Element>);
  }
  ((To *)(this->_void_ptr))->reserve(n);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::capacity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::size_type PointerToArray<Element>::
capacity() const {
  nassertr((this->_void_ptr) != NULL, 0);
  return ((To *)(this->_void_ptr))->capacity();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::front
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
front() const {
  nassertd((this->_void_ptr) != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->front();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::reference PointerToArray<Element>::
back() const {
  nassertd((this->_void_ptr) != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->back();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME PointerToArray<Element>::iterator PointerToArray<Element>::
insert(iterator position, const Element &x) {
  if ((this->_void_ptr) == NULL) {
    reassign(new ReferenceCountedVector<Element>);
    position = end();
  }
  nassertr(position >= ((To *)(this->_void_ptr))->begin() &&
           position <= ((To *)(this->_void_ptr))->end(), position);
  return ((To *)(this->_void_ptr))->insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
insert(iterator position, size_type n, const Element &x) {
  if ((this->_void_ptr) == NULL) {
    reassign(new ReferenceCountedVector<Element>);
    position = end();
  }
  nassertv(position >= ((To *)(this->_void_ptr))->begin() &&
           position <= ((To *)(this->_void_ptr))->end());
  ((To *)(this->_void_ptr))->insert(position, n, x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator position) {
  nassertv((this->_void_ptr) != NULL);
  nassertv(position >= ((To *)(this->_void_ptr))->begin() &&
           position <= ((To *)(this->_void_ptr))->end());
  ((To *)(this->_void_ptr))->erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator first, iterator last) {
  nassertv((this->_void_ptr) != NULL);
  nassertv(first >= ((To *)(this->_void_ptr))->begin() && first <= ((To *)(this->_void_ptr))->end());
  nassertv(last >= ((To *)(this->_void_ptr))->begin() && last <= ((To *)(this->_void_ptr))->end());
  ((To *)(this->_void_ptr))->erase(first, last);
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
  nassertd((this->_void_ptr) != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  nassertr(n < ((To *)(this->_void_ptr))->size(), ((To *)(this->_void_ptr))->operator[](0));
  return ((To *)(this->_void_ptr))->operator[](n);
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
INLINE const Element &PointerToArray<Element>::
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
set_element(size_type n, const Element &value) {
  nassertv(n < ((To *)(this->_void_ptr))->size());
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
  if ((this->_void_ptr) == NULL) {
    reassign(new ReferenceCountedVector<Element>);
  }
  ((To *)(this->_void_ptr))->push_back(x);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::pop_back
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
pop_back() {
  nassertd((this->_void_ptr) != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertv(!((To *)(this->_void_ptr))->empty());
  ((To *)(this->_void_ptr))->pop_back();
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
  nassertd((this->_void_ptr) != NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertv(!((To *)(this->_void_ptr))->empty());
  ((To *)(this->_void_ptr))->clear();
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
  To *vec = (To *)(this->_void_ptr);
  return ((vec == NULL)||(vec->empty())) ? (Element *)NULL : &(vec->front());
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
  To *vec = (To *)(this->_void_ptr);
  return ((vec == NULL)||(vec->empty())) ? (Element *)NULL : &(vec->front());
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
  if ((this->_void_ptr) == NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  return *((To *)(this->_void_ptr));
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get(this->_void_ptr)
//       Access: Public
//  Description: Returns the reference to memory where the vector
//               is stored.  To be used only with set_void_ptr
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void *PointerToArray<Element>::
get_void_ptr() const {
  return (this->_void_ptr);
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
  return ((this->_void_ptr) == NULL) ? 0 : ((To *)(this->_void_ptr))->get_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::get_node_ref_count
//       Access: Public
//  Description: Returns the node_ref of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int PointerToArray<Element>::
get_node_ref_count() const {
  return ((this->_void_ptr) == NULL) ? 0 : ((To *)(this->_void_ptr))->get_node_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::node_ref
//       Access: Public
//  Description: Increments the node_ref of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void PointerToArray<Element>::
node_ref() const {
  if ((this->_void_ptr) == NULL) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  ((To *)(this->_void_ptr))->node_ref();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::node_unref
//       Access: Public
//  Description: Decrements the node_ref of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool PointerToArray<Element>::
node_unref() const {
  nassertr((this->_void_ptr) != NULL, true);
  return ((To *)(this->_void_ptr))->node_unref();
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PointerToArray<Element> &PointerToArray<Element>::
operator = (ReferenceCountedVector<Element> *ptr) {
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
#ifdef DO_PSTATS
  if ((this->_void_ptr) != NULL) {
    PT(PStatCollectorForwardBase) col = ((To *)(this->_void_ptr))->get_col();
    if (col != (PStatCollectorForwardBase *)NULL) {
      // If we have a PStat counter, preserve it.
      reassign(new ReferenceCountedVector<Element>);
      ((To *)(this->_void_ptr))->set_col(col);
      return;
    }
  }
#endif // DO_PSTATS

  reassign((ReferenceCountedVector<Element> *)NULL);
}



////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray() :
  PointerToArrayBase<Element>((ReferenceCountedVector<Element> *)NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const PointerToArray<Element> &copy) :
  PointerToArrayBase<Element>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const ConstPointerToArray<Element> &copy) :
  PointerToArrayBase<Element>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::get_col
//       Access: Published
//  Description: Returns the pointer to the PStatCollector object that
//               tracks the total allocated size of this buffer.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE PStatCollectorForwardBase *ConstPointerToArray<Element>::
get_col() const {
#ifdef DO_PSTATS
  if ((this->_void_ptr) != NULL) {
    return ((To *)(this->_void_ptr))->get_col();
  }
#endif  // DO_PSTATS
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
begin() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
end() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->end();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rbegin() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rend() const {
  if ((this->_void_ptr) == NULL) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::size
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
size() const {
  return ((this->_void_ptr) == NULL) ? 0 : ((To *)(this->_void_ptr))->size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::max_size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
max_size() const {
  nassertd((this->_void_ptr) != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  return ((To *)(this->_void_ptr))->max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool ConstPointerToArray<Element>::
empty() const {
  return ((this->_void_ptr) == NULL) ? true : ((To *)(this->_void_ptr))->empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::capacity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
capacity() const {
  nassertd((this->_void_ptr) != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  return ((To *)(this->_void_ptr))->capacity();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::front
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
front() const {
  nassertd((this->_void_ptr) != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->front();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::back
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
back() const {
  nassertd((this->_void_ptr) != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->back();
}

#ifndef WIN32_VC
////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Indexing operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE TYPENAME ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
operator [](size_type n) const {
  nassertd((this->_void_ptr) != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  nassertr(n < ((To *)(this->_void_ptr))->size(), ((To *)(this->_void_ptr))->operator[](0));
  return ((To *)(this->_void_ptr))->operator[](n);
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
//     Function: ConstPointerToArray::get_element
//       Access: Published
//  Description: This method exists mainly to access the elements of
//               the array easily from a high-level language such as
//               Python, especially on Windows, where the above index
//               element accessor methods can't be defined because of
//               a confusion with the pointer typecast operator.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE const Element &ConstPointerToArray<Element>::
get_element(size_type n) const {
  return (*this)[n];
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
  To *vec = (To *)(this->_void_ptr);
  return ((vec == NULL)||(vec->empty())) ? (const Element *)NULL : &(vec->front());
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
  To *vec = (To *)(this->_void_ptr);
  return ((vec == NULL)||(vec->empty())) ? (const Element *)NULL : &(vec->front());
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
  nassertd((this->_void_ptr) != NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  return *(To *)(this->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::get_ref_count
//       Access: Public
//  Description: Returns the reference count of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int ConstPointerToArray<Element>::
get_ref_count() const {
  return ((this->_void_ptr) == NULL) ? 0 : ((To *)(this->_void_ptr))->get_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::get_node_ref_count
//       Access: Public
//  Description: Returns the node_ref of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE int ConstPointerToArray<Element>::
get_node_ref_count() const {
  return ((this->_void_ptr) == NULL) ? 0 : ((To *)(this->_void_ptr))->get_node_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::node_ref
//       Access: Public
//  Description: Increments the node_ref of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE void ConstPointerToArray<Element>::
node_ref() const {
  if ((this->_void_ptr) == NULL) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>);
  }
  ((To *)(this->_void_ptr))->node_ref();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::node_unref
//       Access: Public
//  Description: Decrements the node_ref of the underlying vector.
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE bool ConstPointerToArray<Element>::
node_unref() const {
  nassertr((this->_void_ptr) != NULL, true);
  return ((To *)(this->_void_ptr))->node_unref();
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerToArray::Assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (ReferenceCountedVector<Element> *ptr) {
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
#ifdef DO_PSTATS
  if ((this->_void_ptr) != NULL) {
    PT(PStatCollectorForwardBase) col = ((To *)(this->_void_ptr))->get_col();
    if (col != (PStatCollectorForwardBase *)NULL) {
      // If we have a PStat counter, preserve it.
      reassign(new ReferenceCountedVector<Element>);
      ((To *)(this->_void_ptr))->set_col(col);
      return;
    }
  }

#else  // DO_PSTATS
  reassign((ReferenceCountedVector<Element> *)NULL);
#endif // DO_PSTATS
}

