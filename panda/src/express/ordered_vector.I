// Filename: ordered_vector.I
// Created by:  drose (20Feb02)
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
//     Function: ordered_vector::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ordered_vector<Key, Compare, Vector>::
ordered_vector(TypeHandle type_handle) :
  _compare(Compare()),
  _vector(type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ordered_vector<Key, Compare, Vector>::
ordered_vector(const Compare &compare, TypeHandle type_handle) :
  _compare(compare),
  _vector(type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ordered_vector<Key, Compare, Vector>::
ordered_vector(const ordered_vector<Key, Compare, Vector> &copy) :
  _compare(copy._compare),
  _vector(copy._vector)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ordered_vector<Key, Compare, Vector> &ordered_vector<Key, Compare, Vector>::
operator = (const ordered_vector<Key, Compare, Vector> &copy) {
  _compare = copy._compare;
  _vector = copy._vector;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ordered_vector<Key, Compare, Vector>::
~ordered_vector() {
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::begin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
begin() {
  return _vector.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::end
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
end() {
  return _vector.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rbegin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::REVERSE_ITERATOR ordered_vector<Key, Compare, Vector>::
rbegin() {
  return _vector.rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rend
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::REVERSE_ITERATOR ordered_vector<Key, Compare, Vector>::
rend() {
  return _vector.rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::begin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
begin() const {
  return _vector.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::end
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
end() const {
  return _vector.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rbegin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_REVERSE_ITERATOR ordered_vector<Key, Compare, Vector>::
rbegin() const {
  return _vector.rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rend
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_REVERSE_ITERATOR ordered_vector<Key, Compare, Vector>::
rend() const {
  return _vector.rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator []
//       Access: Public
//  Description: Returns the nth element.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::REFERENCE ordered_vector<Key, Compare, Vector>::
operator [] (TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE n) {
  return _vector[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator []
//       Access: Public
//  Description: Returns the nth element.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_REFERENCE ordered_vector<Key, Compare, Vector>::
operator [] (TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE n) const {
  return _vector[n];
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::size
//       Access: Public
//  Description: Returns the number of elements in the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE ordered_vector<Key, Compare, Vector>::
size() const {
  return _vector.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::max_size
//       Access: Public
//  Description: Returns the maximum number of elements that can
//               possibly be stored in an ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE ordered_vector<Key, Compare, Vector>::
max_size() const {
  return _vector.max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::empty
//       Access: Public
//  Description: Returns true if the ordered vector is empty, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
empty() const {
  return _vector.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator == 
//       Access: Public
//  Description: Returns true if the two ordered vectors are
//               memberwise equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
operator == (const ordered_vector<Key, Compare, Vector> &other) const {
  return _vector == other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator != 
//       Access: Public
//  Description: Returns true if the two ordered vectors are not
//               memberwise equivalent, false if they are.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
operator != (const ordered_vector<Key, Compare, Vector> &other) const {
  return _vector != other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator <
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically before the other one, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
operator < (const ordered_vector<Key, Compare, Vector> &other) const {
  return _vector < other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator >
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically after the other one, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
operator > (const ordered_vector<Key, Compare, Vector> &other) const {
  return _vector > other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator <=
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically before the other one or is
//               equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
operator <= (const ordered_vector<Key, Compare, Vector> &other) const {
  return _vector <= other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator >=
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically after the other one or is
//               equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ordered_vector<Key, Compare, Vector>::
operator >= (const ordered_vector<Key, Compare, Vector> &other) const {
  return _vector >= other._vector;
}


////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::insert_unique
//       Access: Public
//  Description: Inserts the indicated key into the ordered vector, at
//               the appropriate place.  If there is already an element
//               sorting equivalent to the key in the vector, the new
//               key is not inserted.
//
//               The return value is a pair, where the first component
//               is the iterator referencing the new element (or the
//               original element), and the second componet is true if
//               the insert operation has taken place.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE pair<TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR, bool> ordered_vector<Key, Compare, Vector>::
insert_unique(const TYPENAME ordered_vector<Key, Compare, Vector>::VALUE_TYPE &key) {
  TAU_PROFILE("ordered_vector::insert_unique(const value_type &)", " ", TAU_USER);
  ITERATOR position = find_insert_position(begin(), end(), key);
#ifdef NDEBUG
  pair<ITERATOR, bool> bogus_result(end(), false);
  nassertr(position >= begin() && position <= end(), bogus_result);
#endif

  // If there's already an equivalent key in the vector, it's at
  // *(position - 1).
  if (position != begin() && !_compare(*(position - 1), key)) {
    pair<ITERATOR, bool> result(position - 1, false);
    nassertr(!_compare(key, *(position - 1)), result);
    return result;
  }

  ITERATOR result = _vector.insert(position, key);
  return pair<ITERATOR, bool>(result, true);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::insert_nonunique
//       Access: Public
//  Description: Inserts the indicated key into the ordered vector, at
//               the appropriate place.  If there are already elements
//               sorting equivalent to the key in the vector, the new
//               value is inserted following them.  
//
//               The return value is the iterator referencing the new
//               element.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
insert_nonunique(const TYPENAME ordered_vector<Key, Compare, Vector>::VALUE_TYPE &key) {
  TAU_PROFILE("ordered_vector::insert_nonunique(const value_type &)", " ", TAU_USER);
  ITERATOR position = find_insert_position(begin(), end(), key);
  nassertr(position >= begin() && position <= end(), end());

  ITERATOR result = _vector.insert(position, key);
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::insert_unverified
//       Access: Public
//  Description: Inserts the indicated key into the ordered vector at
//               the indicated place.  The user is trusted to have
//               already verified that this is the correct sorting
//               position; no checks are made.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
insert_unverified(TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR position, 
                  const TYPENAME ordered_vector<Key, Compare, Vector>::VALUE_TYPE &key) {
  TAU_PROFILE("ordered_vector::insert_unverified(iterator, const value_type &)", " ", TAU_USER);
  ITERATOR result = _vector.insert(position, key);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::erase, with iterator
//       Access: Public
//  Description: Removes the element indicated by the given iterator,
//               and returns the next sequential iterator.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
erase(TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR position) {
  TAU_PROFILE("ordered_vector::erase(iterator)", " ", TAU_USER);
  SIZE_TYPE count = position - begin();
  _vector.erase(position);
  return begin() + count;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::erase, with key
//       Access: Public
//  Description: Removes all elements matching the indicated key;
//               returns the number of elements removed.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE ordered_vector<Key, Compare, Vector>::
erase(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  TAU_PROFILE("ordered_vector::erase(const key_type &)", " ", TAU_USER);
  pair<ITERATOR, ITERATOR> result = equal_range(key);
  SIZE_TYPE count = result.second - result.first;
  erase(result.first, result.second);
  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::erase, a range
//       Access: Public
//  Description: Removes all elements indicated by the given iterator
//               range.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
erase(TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR first,
      TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR last) {
  TAU_PROFILE("ordered_vector::erase(iterator, iterator)", " ", TAU_USER);
  _vector.erase(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::clear
//       Access: Public
//  Description: Removes all elements from the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
clear() {
  TAU_PROFILE("ordered_vector::clear()", " ", TAU_USER);
  _vector.erase(_vector.begin(), _vector.end());
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::find
//       Access: Public
//  Description: Searches for an element with the indicated key and
//               returns its iterator if it is found, or end() if it
//               is not.  If there are multiple elements matching the
//               key, the particular iterator returned is not defined.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
find(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  TAU_PROFILE("ordered_vector::find(const key_type &)", " ", TAU_USER);
  return nci(r_find(begin(), end(), end(), key));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::find
//       Access: Public
//  Description: Searches for an element with the indicated key and
//               returns its iterator if it is found, or end() if it
//               is not.  If there are multiple elements matching the
//               key, the particular iterator returned is not defined.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
find(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  TAU_PROFILE("ordered_vector::find(const key_type &)", " ", TAU_USER);
  return r_find(begin(), end(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::find_particular
//       Access: Public
//  Description: Searches for a particular element and returns its
//               iterator if it is found, or end() if it is not.
//
//               First, the Compare function is used to narrow down
//               the range of elements the element might be located
//               within; then the element is compared elementwise, via
//               ==, until the exact matching element is found.  If
//               multiple matches exist within the vector, the
//               particular iterator returned is not defined.
//
//               The assumption is that == implies !Compare(a, b) and
//               !Compare(b, a), but not necessarily the converse.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
find_particular(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  TAU_PROFILE("ordered_vector::find_particular(const key_type &)", " ", TAU_USER);
  return nci(r_find_particular(begin(), end(), end(), key));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::find_particular
//       Access: Public
//  Description: Searches for a particular element and returns its
//               iterator if it is found, or end() if it is not.
//
//               First, the Compare function is used to narrow down
//               the range of elements the element might be located
//               within; then the element is compared elementwise, via
//               ==, until the exact matching element is found.  If
//               multiple matches exist within the vector, the
//               particular iterator returned is not defined./
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
find_particular(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  TAU_PROFILE("ordered_vector::find_particular(const key_type &)", " ", TAU_USER);
  return r_find_particular(begin(), end(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::count
//       Access: Public
//  Description: Returns the number of elements that sort equivalent
//               to the key that are in the vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE ordered_vector<Key, Compare, Vector>::
count(const key_type &key) const {
  TAU_PROFILE("ordered_vector::count(const key_type &)", " ", TAU_USER);
  return r_count(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::lower_bound
//       Access: Public
//  Description: Returns the iterator for the first element not less
//               than key, or end() if all elements are less than key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
lower_bound(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  TAU_PROFILE("ordered_vector::lower_bound(const key_type &)", " ", TAU_USER);
  return nci(r_lower_bound(begin(), end(), key));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::lower_bound
//       Access: Public
//  Description: Returns the iterator for the first element not less
//               than key, or end() if all elements are less than key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
lower_bound(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  TAU_PROFILE("ordered_vector::lower_bound(const key_type &)", " ", TAU_USER);
  return r_lower_bound(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::upper_bound
//       Access: Public
//  Description: Returns the iterator for the first element greater
//               than key, or end() if no element is greater than
//               key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
upper_bound(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  TAU_PROFILE("ordered_vector::upper_bound(const key_type &)", " ", TAU_USER);
  return nci(r_upper_bound(begin(), end(), key));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::upper_bound
//       Access: Public
//  Description: Returns the iterator for the first element greater
//               than key, or end() if no element is greater than
//               key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
upper_bound(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  TAU_PROFILE("ordered_vector::upper_bound(const key_type &)", " ", TAU_USER);
  return r_upper_bound(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::equal_range
//       Access: Public
//  Description: Returns the pair (lower_bound(key), upper_bound(key)).
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE pair<TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR, TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR> ordered_vector<Key, Compare, Vector>::
equal_range(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  TAU_PROFILE("ordered_vector::equal_range(const key_type &)", " ", TAU_USER);
  pair<TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR, TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR> result;
  result = r_equal_range(begin(), end(), key);
  return pair<TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR, TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR>(nci(result.first), nci(result.second));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::equal_range
//       Access: Public
//  Description: Returns the pair (lower_bound(key), upper_bound(key)).
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE pair<TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR, TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR> ordered_vector<Key, Compare, Vector>::
equal_range(const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  TAU_PROFILE("ordered_vector::equal_range(const key_type &)", " ", TAU_USER);
  return r_equal_range(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::swap
//       Access: Public
//  Description: Exchanges the contents of this vector and the other
//               vector, in constant time (e.g., with a pointer swap).
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
swap(ordered_vector<Key, Compare, Vector> &copy) {
  TAU_PROFILE("ordered_vector::swap(ordered_vector &)", " ", TAU_USER);
  _vector.swap(copy._vector);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::reserve
//       Access: Public
//  Description: Informs the vector of a planned change in size;
//               ensures that the capacity of the vector is greater
//               than or equal to n.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
reserve(TYPENAME ordered_vector<Key, Compare, Vector>::SIZE_TYPE n) {
  TAU_PROFILE("ordered_vector::reserve(size_type)", " ", TAU_USER);
  _vector.reserve(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::sort_unique
//       Access: Public
//  Description: Ensures that the vector is properly sorted after a
//               potentially damaging operation.  This should not
//               normally need to be called, unless the user has
//               written to the vector using the non-const iterators
//               or has called push_back().
//
//               This flavor of sort also eliminates repeated
//               elements.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
sort_unique() {
  TAU_PROFILE("ordered_vector::sort_unique()", " ", TAU_USER);
  sort(begin(), end(), _compare);
  iterator new_end = unique(begin(), end(), EquivalentTest(_compare));
  erase(new_end, end());
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::sort_nonunique
//       Access: Public
//  Description: Ensures that the vector is properly sorted after a
//               potentially damaging operation.  This should not
//               normally need to be called, unless the user has
//               written to the vector using the non-const iterators
//               or has called push_back().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
sort_nonunique() {
  TAU_PROFILE("ordered_vector::sort_nonunique()", " ", TAU_USER);
  stable_sort(begin(), end(), _compare);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::push_back
//       Access: Public
//  Description: Adds the new element to the end of the vector without
//               regard for proper sorting.  This is a bad idea to do
//               except to populate the vector the first time; be sure
//               to call sort() after you have added all the elements.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
push_back(const value_type &key) {
  TAU_PROFILE("ordered_vector::push_back()", " ", TAU_USER);
  _vector.push_back(key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::pop_back
//       Access: Public
//  Description: Removes the last element at the end of the vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ordered_vector<Key, Compare, Vector>::
pop_back() {
  TAU_PROFILE("ordered_vector::pop_back()", " ", TAU_USER);
  _vector.pop_back();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::nci
//       Access: Private
//  Description: I.e. "non-const iterator".  This function is used to
//               typecast a const iterator to a non-const iterator for
//               easy definition of const vs. non-const flavors of
//               some of these methods.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
nci(TYPENAME ordered_vector<Key, Compare, Vector>::CONST_ITERATOR i) {
  return begin() + (i - begin());
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::find_insert_position
//       Access: Private
//  Description: Searches for the appropriate place in the ordered
//               vector to insert the indicated key, and returns the
//               corresponding iterator.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
find_insert_position(TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR first,
                     TYPENAME ordered_vector<Key, Compare, Vector>::ITERATOR last,
                     const TYPENAME ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  ITERATOR result = r_find_insert_position(first, last, key);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_set<Key, Compare, Vector>::
ov_set(TypeHandle type_handle) :
  ordered_vector<Key, Compare, Vector>(type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_set<Key, Compare, Vector>::
ov_set(const Compare &compare, TypeHandle type_handle) :
  ordered_vector<Key, Compare, Vector>(compare, type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_set<Key, Compare, Vector>::
ov_set(const ov_set<Key, Compare, Vector> &copy) :
  ordered_vector<Key, Compare, Vector>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_set<Key, Compare, Vector> &ov_set<Key, Compare, Vector>::
operator = (const ov_set<Key, Compare, Vector> &copy) {
  ordered_vector<Key, Compare, Vector>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::insert
//       Access: Public
//  Description: Maps to insert_unique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
TYPENAME ov_set<Key, Compare, Vector>::ITERATOR ov_set<Key, Compare, Vector>::
insert(TYPENAME ov_set<Key, Compare, Vector>::ITERATOR position, 
       const TYPENAME ov_set<Key, Compare, Vector>::VALUE_TYPE &key) {
  return ordered_vector<Key, Compare, Vector>::insert_unique(position, key);
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::insert
//       Access: Public
//  Description: Maps to insert_unique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE pair<TYPENAME ov_set<Key, Compare, Vector>::ITERATOR, bool> ov_set<Key, Compare, Vector>::
insert(const TYPENAME ov_set<Key, Compare, Vector>::VALUE_TYPE &key) {
  return ordered_vector<Key, Compare, Vector>::insert_unique(key);
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::sort
//       Access: Public
//  Description: Maps to sort_unique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ov_set<Key, Compare, Vector>::
sort() {
  ordered_vector<Key, Compare, Vector>::sort_unique();
}

////////////////////////////////////////////////////////////////////
//     Function: ov_set::verify_list
//       Access: Public
//  Description: Maps to verify_list_unique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ov_set<Key, Compare, Vector>::
verify_list() const {
  return ordered_vector<Key, Compare, Vector>::verify_list_unique();
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_multiset<Key, Compare, Vector>::
ov_multiset(TypeHandle type_handle) :
  ordered_vector<Key, Compare, Vector>(type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_multiset<Key, Compare, Vector>::
ov_multiset(const Compare &compare, TypeHandle type_handle) :
  ordered_vector<Key, Compare, Vector>(compare, type_handle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_multiset<Key, Compare, Vector>::
ov_multiset(const ov_multiset<Key, Compare, Vector> &copy) :
  ordered_vector<Key, Compare, Vector>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE ov_multiset<Key, Compare, Vector> &ov_multiset<Key, Compare, Vector>::
operator = (const ov_multiset<Key, Compare, Vector> &copy) {
  ordered_vector<Key, Compare, Vector>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::insert
//       Access: Public
//  Description: Maps to insert_nonunique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
TYPENAME ov_multiset<Key, Compare, Vector>::ITERATOR ov_multiset<Key, Compare, Vector>::
insert(TYPENAME ov_multiset<Key, Compare, Vector>::ITERATOR position, 
       const TYPENAME ov_multiset<Key, Compare, Vector>::VALUE_TYPE &key) {
  return ordered_vector<Key, Compare, Vector>::insert_nonunique(position, key);
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::insert
//       Access: Public
//  Description: Maps to insert_nonunique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE TYPENAME ov_multiset<Key, Compare, Vector>::ITERATOR ov_multiset<Key, Compare, Vector>::
insert(const TYPENAME ov_multiset<Key, Compare, Vector>::VALUE_TYPE &key) {
  return ordered_vector<Key, Compare, Vector>::insert_nonunique(key);
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::sort
//       Access: Public
//  Description: Maps to sort_nonunique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE void ov_multiset<Key, Compare, Vector>::
sort() {
  ordered_vector<Key, Compare, Vector>::sort_nonunique();
}

////////////////////////////////////////////////////////////////////
//     Function: ov_multiset::verify_list
//       Access: Public
//  Description: Maps to verify_list_nonunique().
////////////////////////////////////////////////////////////////////
template<class Key, class Compare, class Vector>
INLINE bool ov_multiset<Key, Compare, Vector>::
verify_list() const {
  return ordered_vector<Key, Compare, Vector>::verify_list_nonunique();
}
