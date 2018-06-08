/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToArray.I
 * @author drose
 * @date 2000-01-07
 */

#ifndef CPPPARSER

template<class Element>
pvector<Element> PointerToArray<Element>::_empty_array;

template<class Element>
pvector<Element> ConstPointerToArray<Element>::_empty_array;

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(TypeHandle type_handle) :
  PointerToArrayBase<Element>(nullptr),
  _type_handle(type_handle)
{
}

/**
 * Return an empty array of size n
 */
template<class Element>
INLINE PointerToArray<Element>
PointerToArray<Element>::empty_array(size_type n, TypeHandle type_handle) {
  PointerToArray<Element> temp(type_handle);
  temp.reassign(new ReferenceCountedVector<Element>(type_handle));

  To new_array(n, type_handle);
  ((To *)(temp._void_ptr))->swap(new_array);
  return temp;
}

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(size_type n, const Element &value, TypeHandle type_handle) :
  PointerToArrayBase<Element>(new ReferenceCountedVector<Element>(type_handle)),
  _type_handle(type_handle)
{
  ((To *)(this->_void_ptr))->reserve(n);
  insert(begin(), n, value);
}

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(const PointerToArray<Element> &copy) :
  PointerToArrayBase<Element>(copy),
  _type_handle(copy._type_handle)
{
}

/**
 * Initializes a PointerToArray by copying existing elements.
 */
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(const Element *begin, const Element *end, TypeHandle type_handle) :
  PointerToArrayBase<Element>(new ReferenceCountedVector<Element>(begin, end, type_handle)),
  _type_handle(type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(PointerToArray<Element> &&from) noexcept :
  PointerToArrayBase<Element>(std::move(from)),
  _type_handle(from._type_handle)
{
}

/**
 * Initializes the PTA from a vector.
 */
template<class Element>
INLINE PointerToArray<Element>::
PointerToArray(pvector<Element> &&from, TypeHandle type_handle) :
  PointerToArrayBase<Element>(new ReferenceCountedVector<Element>(std::move(from))),
  _type_handle(type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::iterator PointerToArray<Element>::
begin() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->begin();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::iterator PointerToArray<Element>::
end() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->end();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rbegin() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rbegin();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::reverse_iterator PointerToArray<Element>::
rend() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rend();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::size_type PointerToArray<Element>::
size() const {
  return ((this->_void_ptr) == nullptr) ? 0 : ((To *)(this->_void_ptr))->size();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::size_type PointerToArray<Element>::
max_size() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  return ((To *)(this->_void_ptr))->max_size();
}

/**
 *
 */
template<class Element>
INLINE bool PointerToArray<Element>::
empty() const {
  return ((this->_void_ptr) == nullptr) ? true : ((To *)(this->_void_ptr))->empty();
}

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
reserve(typename PointerToArray<Element>::size_type n) {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->reserve(n);
}

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
resize(typename PointerToArray<Element>::size_type n) {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->resize(n);
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::size_type PointerToArray<Element>::
capacity() const {
  nassertr((this->_void_ptr) != nullptr, 0);
  return ((To *)(this->_void_ptr))->capacity();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::reference PointerToArray<Element>::
front() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->front();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::reference PointerToArray<Element>::
back() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->back();
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::iterator PointerToArray<Element>::
insert(iterator position, const Element &x) {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
    position = end();
  }
  nassertr(position >= ((To *)(this->_void_ptr))->begin() &&
           position <= ((To *)(this->_void_ptr))->end(), position);
  return ((To *)(this->_void_ptr))->insert(position, x);
}

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
insert(iterator position, size_type n, const Element &x) {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
    position = end();
  }
  nassertv(position >= ((To *)(this->_void_ptr))->begin() &&
           position <= ((To *)(this->_void_ptr))->end());
  ((To *)(this->_void_ptr))->insert(position, n, x);
}

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator position) {
  nassertv((this->_void_ptr) != nullptr);
  nassertv(position >= ((To *)(this->_void_ptr))->begin() &&
           position <= ((To *)(this->_void_ptr))->end());
  ((To *)(this->_void_ptr))->erase(position);
}

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
erase(iterator first, iterator last) {
  nassertv((this->_void_ptr) != nullptr);
  nassertv(first >= ((To *)(this->_void_ptr))->begin() && first <= ((To *)(this->_void_ptr))->end());
  nassertv(last >= ((To *)(this->_void_ptr))->begin() && last <= ((To *)(this->_void_ptr))->end());
  ((To *)(this->_void_ptr))->erase(first, last);
}

#if !defined(WIN32_VC) && !defined(WIN64_VC)
/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::reference PointerToArray<Element>::
operator [](size_type n) const {
  nassertd((this->_void_ptr) != nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  nassertr(n < ((To *)(this->_void_ptr))->size(), ((To *)(this->_void_ptr))->operator[](0));
  return ((To *)(this->_void_ptr))->operator[](n);
}

/**
 *
 */
template<class Element>
INLINE typename PointerToArray<Element>::reference PointerToArray<Element>::
operator [](int n) const {
  return operator[]((size_type)n);
}
#endif

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
push_back(const Element &x) {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->push_back(x);
}

/**
 *
 */
template<class Element>
INLINE void PointerToArray<Element>::
pop_back() {
  nassertd((this->_void_ptr) != nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertv(!((To *)(this->_void_ptr))->empty());
  ((To *)(this->_void_ptr))->pop_back();
}

/**
 * Empties the array pointed to.  This is different from clear(), which
 * reassigns the pointer to a NULL pointer.
 */
template<class Element>
INLINE void PointerToArray<Element>::
make_empty() {
  nassertd((this->_void_ptr) != nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertv(!((To *)(this->_void_ptr))->empty());
  ((To *)(this->_void_ptr))->clear();
}

/**
 * The pointer typecast operator is convenient for maintaining the fiction
 * that we actually have a C-style array.  It returns the address of the first
 * element in the array, unless the pointer is unassigned, in which case it
 * returns NULL.
 */
template<class Element>
INLINE PointerToArray<Element>::
operator Element *() const {
  To *vec = (To *)(this->_void_ptr);
  return ((vec == nullptr)||(vec->empty())) ? nullptr : &(vec->front());
}

/**
 * Function p() is similar to the function from PointerTo.  It does the same
 * thing: it returns the same thing as the typecast operator, above.
 */
template<class Element>
INLINE Element *PointerToArray<Element>::
p() const {
  To *vec = (To *)(this->_void_ptr);
  return ((vec == nullptr)||(vec->empty())) ? nullptr : &(vec->front());
}

/**
 * To access the vector itself, for more direct fiddling with some of the
 * vector's esoteric functionality.
 */
template<class Element>
INLINE pvector<Element> &PointerToArray<Element>::
v() const {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  return *((To *)(this->_void_ptr));
}

/**
 * To access the internal ReferenceCountedVector object, for very low-level
 * fiddling.  Know what you are doing!
 */
template<class Element>
INLINE ReferenceCountedVector<Element> *PointerToArray<Element>::
v0() const {
  return (To *)(this->_void_ptr);
}

/**
 * This method exists mainly to access the elements of the array easily from a
 * high-level language such as Python, especially on Windows, where the above
 * index element accessor methods can't be defined because of a confusion with
 * the pointer typecast operator.
 */
template<class Element>
INLINE const Element &PointerToArray<Element>::
get_element(size_type n) const {
  return (*this)[n];
}

/**
 * This method exists mainly to access the elements of the array easily from a
 * high-level language such as Python, especially on Windows, where the above
 * index element accessor methods can't be defined because of a confusion with
 * the pointer typecast operator.
 */
template<class Element>
INLINE void PointerToArray<Element>::
set_element(size_type n, const Element &value) {
  nassertv(n < ((To *)(this->_void_ptr))->size());
  (*this)[n] = value;
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * It returns the entire contents of the vector as a block of raw data in a
 * string.
 */
template<class Element>
INLINE std::string PointerToArray<Element>::
get_data() const {
  return get_subdata(0, size());
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * It replaces the entire contents of the vector from a block of raw data in a
 * string.
 */
template<class Element>
INLINE void PointerToArray<Element>::
set_data(const std::string &data) {
  set_subdata(0, size(), data);
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * It returns the contents of a portion of the vector--from element (n)
 * through element (n + count - 1)--as a block of raw data in a string.
 */
template<class Element>
INLINE std::string PointerToArray<Element>::
get_subdata(size_type n, size_type count) const {
  n = std::min(n, size());
  count = std::max(count, n);
  count = std::min(count, size() - n);
  return std::string((const char *)(p() + n), sizeof(Element) * count);
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * It replaces the contents of a portion of the vector--from element (n)
 * through element (n + count - 1)--as a block of raw data in a string.  The
 * length of the string must be an even multiple of Element size bytes.  The
 * array may be expanded or truncated if the length of the string does not
 * correspond to exactly count elements.
 */
template<class Element>
INLINE void PointerToArray<Element>::
set_subdata(size_type n, size_type count, const std::string &data) {
  nassertv((data.length() % sizeof(Element)) == 0);
  nassertv(n <= size() && n + count <= size());
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  size_type ncount = data.length() / sizeof(Element);
  if (ncount < count) {
    // Reduce the array.
    erase(begin() + n + ncount, begin() + n + count);
  } else if (count < ncount) {
    // Expand the array.
    insert(begin() + n + count, ncount - count, Element());
  }

  // Now boldly replace the data.  Hope there aren't any constructors or
  // destructors involved here.  The user better know what she is doing.
  memcpy(p() + n, data.data(), sizeof(Element) * ncount);
}

/**
 * Returns the reference to memory where the vector is stored.  To be used
 * only with set_void_ptr
 */
template<class Element>
INLINE void *PointerToArray<Element>::
get_void_ptr() const {
  return (this->_void_ptr);
}

/**
 * Sets this PTA to point to the pointer passed in
 */
template<class Element>
INLINE void PointerToArray<Element>::
set_void_ptr(void *p) {
  ((PointerToArray<Element> *)this)->reassign((To *)p);
}

/**
 * Returns the reference count of the underlying vector.
 */
template<class Element>
INLINE int PointerToArray<Element>::
get_ref_count() const {
  return ((this->_void_ptr) == nullptr) ? 0 : ((To *)(this->_void_ptr))->get_ref_count();
}

/**
 * Increments the reference count of the underlying vector.
 */
template<class Element>
INLINE void PointerToArray<Element>::
ref() const {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->ref();
}

/**
 * Decrements the reference count of the underlying vector.
 */
template<class Element>
INLINE bool PointerToArray<Element>::
unref() const {
  nassertr((this->_void_ptr) != nullptr, true);
  return ((To *)(this->_void_ptr))->unref();
}

/**
 * Returns the node_ref of the underlying vector.
 */
template<class Element>
INLINE int PointerToArray<Element>::
get_node_ref_count() const {
  return ((this->_void_ptr) == nullptr) ? 0 : ((To *)(this->_void_ptr))->get_node_ref_count();
}

/**
 * Increments the node_ref of the underlying vector.
 */
template<class Element>
INLINE void PointerToArray<Element>::
node_ref() const {
  if ((this->_void_ptr) == nullptr) {
    ((PointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->node_ref();
}

/**
 * Decrements the node_ref of the underlying vector.
 */
template<class Element>
INLINE bool PointerToArray<Element>::
node_unref() const {
  nassertr((this->_void_ptr) != nullptr, true);
  return ((To *)(this->_void_ptr))->node_unref();
}

/**
 * Counts the frequency at which the given element occurs in the vector.
 */
template<class Element>
INLINE size_t PointerToArray<Element>::
count(const Element &value) const {
  if ((this->_void_ptr) != nullptr) {
    return std::count(begin(), end(), value);
  } else {
    return 0;
  }
}

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element> &PointerToArray<Element>::
operator = (ReferenceCountedVector<Element> *ptr) {
  ((PointerToArray<Element> *)this)->reassign(ptr);
  return *this;
}

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element> &PointerToArray<Element>::
operator = (const PointerToArray<Element> &copy) {
  _type_handle = copy._type_handle;
  ((PointerToArray<Element> *)this)->reassign(copy);
  return *this;
}

/**
 *
 */
template<class Element>
INLINE PointerToArray<Element> &PointerToArray<Element>::
operator = (PointerToArray<Element> &&from) noexcept {
  _type_handle = from._type_handle;
  ((PointerToArray<Element> *)this)->reassign(std::move(from));
  return *this;
}

/**
 * To empty the PTA, use the clear() method, since assignment to NULL is
 * problematic (given the ambiguity of the pointer type of NULL).
 */
template<class Element>
INLINE void PointerToArray<Element>::
clear() {
  ((PointerToArray<Element> *)this)->reassign(nullptr);
}



/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(TypeHandle type_handle) :
  PointerToArrayBase<Element>(nullptr),
  _type_handle(type_handle)
{
}

/**
 * Initializes a ConstPointerToArray by copying existing elements.
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const Element *begin, const Element *end, TypeHandle type_handle) :
  PointerToArrayBase<Element>(new ReferenceCountedVector<Element>(begin, end, type_handle)),
  _type_handle(type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const PointerToArray<Element> &copy) :
  PointerToArrayBase<Element>(copy),
  _type_handle(copy._type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(const ConstPointerToArray<Element> &copy) :
  PointerToArrayBase<Element>(copy),
  _type_handle(copy._type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(PointerToArray<Element> &&from) noexcept :
  PointerToArrayBase<Element>(std::move(from)),
  _type_handle(from._type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(ConstPointerToArray<Element> &&from) noexcept :
  PointerToArrayBase<Element>(std::move(from)),
  _type_handle(from._type_handle)
{
}

/**
 * Initializes the PTA from a vector.
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
ConstPointerToArray(pvector<Element> &&from, TypeHandle type_handle) :
  PointerToArrayBase<Element>(new ReferenceCountedVector<Element>(std::move(from))),
  _type_handle(type_handle)
{
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
begin() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->begin();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::iterator ConstPointerToArray<Element>::
end() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.begin();
  }
  return ((To *)(this->_void_ptr))->end();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rbegin() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rbegin();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::reverse_iterator ConstPointerToArray<Element>::
rend() const {
  if ((this->_void_ptr) == nullptr) {
    return _empty_array.rbegin();
  }
  return ((To *)(this->_void_ptr))->rend();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
size() const {
  return ((this->_void_ptr) == nullptr) ? 0 : ((To *)(this->_void_ptr))->size();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
max_size() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  return ((To *)(this->_void_ptr))->max_size();
}

/**
 *
 */
template<class Element>
INLINE bool ConstPointerToArray<Element>::
empty() const {
  return ((this->_void_ptr) == nullptr) ? true : ((To *)(this->_void_ptr))->empty();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::size_type ConstPointerToArray<Element>::
capacity() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  return ((To *)(this->_void_ptr))->capacity();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
front() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->front();
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
back() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  return ((To *)(this->_void_ptr))->back();
}

#if !defined(WIN32_VC) && !defined(WIN64_VC)
/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
operator [](size_type n) const {
  nassertd((this->_void_ptr) != nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  nassertd(!((To *)(this->_void_ptr))->empty()) {
    ((To *)(this->_void_ptr))->push_back(Element());
  }
  nassertr(n < ((To *)(this->_void_ptr))->size(), ((To *)(this->_void_ptr))->operator[](0));
  return ((To *)(this->_void_ptr))->operator[](n);
}

/**
 *
 */
template<class Element>
INLINE typename ConstPointerToArray<Element>::reference ConstPointerToArray<Element>::
operator [](int n) const {
  return operator[]((size_type)n);
}
#endif

/**
 * The pointer typecast operator is convenient for maintaining the fiction
 * that we actually have a C-style array.  It returns the address of the first
 * element in the array, unless the pointer is unassigned, in which case it
 * returns NULL.
 */
template<class Element>
INLINE ConstPointerToArray<Element>::
operator const Element *() const {
  const To *vec = (const To *)(this->_void_ptr);
  return ((vec == nullptr)||(vec->empty())) ? nullptr : &(vec->front());
}

/**
 * Function p() is similar to the function from ConstPointerTo.  It does the
 * same thing: it returns the same thing as the typecast operator, above.
 */
template<class Element>
INLINE const Element *ConstPointerToArray<Element>::
p() const {
  const To *vec = (const To *)(this->_void_ptr);
  return ((vec == nullptr)||(vec->empty())) ? nullptr : &(vec->front());
}

/**
 * To access the vector itself, for more direct fiddling with some of the
 * vector's esoteric functionality.
 */
template<class Element>
INLINE const pvector<Element> &ConstPointerToArray<Element>::
v() const {
  nassertd((this->_void_ptr) != nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  return *(const To *)(this->_void_ptr);
}

/**
 * To access the internal ReferenceCountedVector object, for very low-level
 * fiddling.  Know what you are doing!
 */
template<class Element>
INLINE const ReferenceCountedVector<Element> *ConstPointerToArray<Element>::
v0() const {
  return (const To *)(this->_void_ptr);
}

/**
 * Casts away the constness of the CPTA(Element), and returns an equivalent
 * PTA(Element).
 */
template<class Element>
INLINE PointerToArray<Element> ConstPointerToArray<Element>::
cast_non_const() const {
  PointerToArray<Element> non_const;
  non_const = (To *)(this->_void_ptr);
  return non_const;
}

/**
 * This method exists mainly to access the elements of the array easily from a
 * high-level language such as Python, especially on Windows, where the above
 * index element accessor methods can't be defined because of a confusion with
 * the pointer typecast operator.
 */
template<class Element>
INLINE const Element &ConstPointerToArray<Element>::
get_element(size_type n) const {
  return (*this)[n];
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * It returns the entire contents of the vector as a block of raw data in a
 * string.
 */
template<class Element>
INLINE std::string ConstPointerToArray<Element>::
get_data() const {
  return get_subdata(0, size());
}

/**
 * This method exists mainly to access the data of the array easily from a
 * high-level language such as Python.
 *
 * It returns the contents of a portion of the vector--from element (n)
 * through element (n + count - 1)--as a block of raw data in a string.
 */
template<class Element>
INLINE std::string ConstPointerToArray<Element>::
get_subdata(size_type n, size_type count) const {
  n = std::min(n, size());
  count = std::max(count, n);
  count = std::min(count, size() - n);
  return std::string((const char *)(p() + n), sizeof(Element) * count);
}

/**
 * Returns the reference count of the underlying vector.
 */
template<class Element>
INLINE int ConstPointerToArray<Element>::
get_ref_count() const {
  return ((this->_void_ptr) == nullptr) ? 0 : ((To *)(this->_void_ptr))->get_ref_count();
}

/**
 * Increments the reference count of the underlying vector.
 */
template<class Element>
INLINE void ConstPointerToArray<Element>::
ref() const {
  if ((this->_void_ptr) == nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->ref();
}

/**
 * Decrements the reference count of the underlying vector.
 */
template<class Element>
INLINE bool ConstPointerToArray<Element>::
unref() const {
  nassertr((this->_void_ptr) != nullptr, true);
  return ((To *)(this->_void_ptr))->unref();
}

/**
 * Returns the node_ref of the underlying vector.
 */
template<class Element>
INLINE int ConstPointerToArray<Element>::
get_node_ref_count() const {
  return ((this->_void_ptr) == nullptr) ? 0 : ((To *)(this->_void_ptr))->get_node_ref_count();
}

/**
 * Increments the node_ref of the underlying vector.
 */
template<class Element>
INLINE void ConstPointerToArray<Element>::
node_ref() const {
  if ((this->_void_ptr) == nullptr) {
    ((ConstPointerToArray<Element> *)this)->reassign(new ReferenceCountedVector<Element>(_type_handle));
  }
  ((To *)(this->_void_ptr))->node_ref();
}

/**
 * Decrements the node_ref of the underlying vector.
 */
template<class Element>
INLINE bool ConstPointerToArray<Element>::
node_unref() const {
  nassertr((this->_void_ptr) != nullptr, true);
  return ((To *)(this->_void_ptr))->node_unref();
}

/**
 * Counts the frequency at which the given element occurs in the vector.
 */
template<class Element>
INLINE size_t ConstPointerToArray<Element>::
count(const Element &value) const {
  if ((this->_void_ptr) != nullptr) {
    return std::count(begin(), end(), value);
  } else {
    return 0;
  }
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (ReferenceCountedVector<Element> *ptr) {
  ((ConstPointerToArray<Element> *)this)->reassign(ptr);
  return *this;
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (const PointerToArray<Element> &copy) {
  _type_handle = copy._type_handle;
  ((ConstPointerToArray<Element> *)this)->reassign(copy);
  return *this;
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (const ConstPointerToArray<Element> &copy) {
  _type_handle = copy._type_handle;
  ((ConstPointerToArray<Element> *)this)->reassign(copy);
  return *this;
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (PointerToArray<Element> &&from) noexcept {
  _type_handle = from._type_handle;
  ((ConstPointerToArray<Element> *)this)->reassign(std::move(from));
  return *this;
}

/**
 *
 */
template<class Element>
INLINE ConstPointerToArray<Element> &ConstPointerToArray<Element>::
operator = (ConstPointerToArray<Element> &&from) noexcept {
  _type_handle = from._type_handle;
  ((ConstPointerToArray<Element> *)this)->reassign(std::move(from));
  return *this;
}

/**
 * To empty the PTA, use the clear() method, since assignment to NULL is
 * problematic (given the ambiguity of the pointer type of NULL).
 */
template<class Element>
INLINE void ConstPointerToArray<Element>::
clear() {
  ((ConstPointerToArray<Element> *)this)->reassign(nullptr);
}

#endif  // CPPPARSER
