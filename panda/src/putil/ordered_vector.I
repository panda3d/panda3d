// Filename: ordered_vector.I
// Created by:  drose (20Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::
ordered_vector(const Compare &compare) :
  _compare(compare)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::
ordered_vector(const ordered_vector<Key, Compare> &copy) :
  _compare(copy._compare),
  _vector(copy._vector)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare> &ordered_vector<Key, Compare>::
operator = (const ordered_vector<Key, Compare> &copy) {
  _compare = copy._compare;
  _vector = copy._vector;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::
~ordered_vector() {
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::begin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
begin() {
  return _vector.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::end
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
end() {
  return _vector.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rbegin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::reverse_iterator ordered_vector<Key, Compare>::
rbegin() {
  return _vector.rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rend
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::reverse_iterator ordered_vector<Key, Compare>::
rend() {
  return _vector.rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::begin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_iterator ordered_vector<Key, Compare>::
begin() const {
  return _vector.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::end
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_iterator ordered_vector<Key, Compare>::
end() const {
  return _vector.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rbegin
//       Access: Public
//  Description: Returns the iterator that marks the first element in
//               the ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_reverse_iterator ordered_vector<Key, Compare>::
rbegin() const {
  return _vector.rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::rend
//       Access: Public
//  Description: Returns the iterator that marks the end of the
//               ordered vector, when viewed in reverse order.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_reverse_iterator ordered_vector<Key, Compare>::
rend() const {
  return _vector.rend();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::size
//       Access: Public
//  Description: Returns the number of elements in the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::size_type ordered_vector<Key, Compare>::
size() const {
  return _vector.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::max_size
//       Access: Public
//  Description: Returns the maximum number of elements that can
//               possibly be stored in an ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::size_type ordered_vector<Key, Compare>::
max_size() const {
  return _vector.max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::empty
//       Access: Public
//  Description: Returns true if the ordered vector is empty, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
empty() const {
  return _vector.max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator == 
//       Access: Public
//  Description: Returns true if the two ordered vectors are
//               memberwise equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
operator == (const ordered_vector<Key, Compare> &other) const {
  return _vector == other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator != 
//       Access: Public
//  Description: Returns true if the two ordered vectors are not
//               memberwise equivalent, false if they are.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
operator != (const ordered_vector<Key, Compare> &other) const {
  return _vector != other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator <
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically before the other one, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
operator < (const ordered_vector<Key, Compare> &other) const {
  return _vector < other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator >
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically after the other one, false
//               otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
operator > (const ordered_vector<Key, Compare> &other) const {
  return _vector > other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator <=
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically before the other one or is
//               equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
operator <= (const ordered_vector<Key, Compare> &other) const {
  return _vector <= other._vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::operator >=
//       Access: Public
//  Description: Returns true if this ordered vector sorts
//               lexicographically after the other one or is
//               equivalent, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
operator >= (const ordered_vector<Key, Compare> &other) const {
  return _vector >= other._vector;
}


////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::insert
//       Access: Public
//  Description: Inserts the indicated key into the ordered vector, at
//               the appropriate place.  If there are already elements
//               sorting equivalent to the key in the vector, the new
//               value is inserted following them.  The return value
//               is the iterator referencing the new element.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
insert(const ordered_vector<Key, Compare>::value_type &key) {
  iterator position = find_insert_position(begin(), end(), key);
  nassertr(position >= begin() && position <= end(), end());

  iterator result = _vector.insert(position, key);
  verify_list();
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::erase, with iterator
//       Access: Public
//  Description: Removes the element indicated by the given iterator,
//               and returns the next sequential iterator.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
erase(ordered_vector<Key, Compare>::iterator position) {
  size_type count = position - begin();
  _vector.erase(position);
  return begin() + count;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::erase, with key
//       Access: Public
//  Description: Removes all elements matching the indicated key;
//               returns the number of elements removed.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::size_type ordered_vector<Key, Compare>::
erase(const ordered_vector<Key, Compare>::key_type &key) {
  pair<iterator, iterator> result = equal_range(key);
  size_type count = result.second - result.first;
  erase(result.first, result.second);
  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::erase, a range
//       Access: Public
//  Description: Removes all elements indicated by the given iterator
//               range.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE void ordered_vector<Key, Compare>::
erase(ordered_vector<Key, Compare>::iterator first,
      ordered_vector<Key, Compare>::iterator last) {
  _vector.erase(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::clear
//       Access: Public
//  Description: Removes all elements from the ordered vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE void ordered_vector<Key, Compare>::
clear() {
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
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
find(const ordered_vector<Key, Compare>::key_type &key) {
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
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_iterator ordered_vector<Key, Compare>::
find(const ordered_vector<Key, Compare>::key_type &key) const {
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
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
find_particular(const ordered_vector<Key, Compare>::key_type &key) {
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
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_iterator ordered_vector<Key, Compare>::
find_particular(const ordered_vector<Key, Compare>::key_type &key) const {
  return r_find_particular(begin(), end(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::count
//       Access: Public
//  Description: Returns the number of elements that sort equivalent
//               to the key that are in the vector.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::size_type ordered_vector<Key, Compare>::
count(const key_type &key) const {
  return r_count(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::lower_bound
//       Access: Public
//  Description: Returns the iterator for the first element not less
//               than key, or end() if all elements are less than key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
lower_bound(const ordered_vector<Key, Compare>::key_type &key) {
  return nci(r_lower_bound(begin(), end(), key));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::lower_bound
//       Access: Public
//  Description: Returns the iterator for the first element not less
//               than key, or end() if all elements are less than key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_iterator ordered_vector<Key, Compare>::
lower_bound(const ordered_vector<Key, Compare>::key_type &key) const {
  return r_lower_bound(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::upper_bound
//       Access: Public
//  Description: Returns the iterator for the first element greater
//               than key, or end() if no element is greater than
//               key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
upper_bound(const ordered_vector<Key, Compare>::key_type &key) {
  return nci(r_upper_bound(begin(), end(), key));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::upper_bound
//       Access: Public
//  Description: Returns the iterator for the first element greater
//               than key, or end() if no element is greater than
//               key.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::const_iterator ordered_vector<Key, Compare>::
upper_bound(const ordered_vector<Key, Compare>::key_type &key) const {
  return r_upper_bound(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::equal_range
//       Access: Public
//  Description: Returns the pair (lower_bound(key), upper_bound(key)).
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE pair<ordered_vector<Key, Compare>::iterator, ordered_vector<Key, Compare>::iterator> ordered_vector<Key, Compare>::
equal_range(const ordered_vector<Key, Compare>::key_type &key) {
  pair<ordered_vector<Key, Compare>::const_iterator, ordered_vector<Key, Compare>::const_iterator> result;
  result = r_equal_range(begin(), end(), key);
  return pair<ordered_vector<Key, Compare>::iterator, ordered_vector<Key, Compare>::iterator>(nci(result.first), nci(result.second));
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::equal_range
//       Access: Public
//  Description: Returns the pair (lower_bound(key), upper_bound(key)).
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE pair<ordered_vector<Key, Compare>::const_iterator, ordered_vector<Key, Compare>::const_iterator> ordered_vector<Key, Compare>::
equal_range(const ordered_vector<Key, Compare>::key_type &key) const {
  return r_equal_range(begin(), end(), key);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::swap
//       Access: Public
//  Description: Exchanges the contents of this vector and the other
//               vector, in constant time (e.g., with a pointer swap).
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE void ordered_vector<Key, Compare>::
swap(ordered_vector<Key, Compare> &copy) {
  _vector.swap(copy._vector);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::reserve
//       Access: Public
//  Description: Informs the vector of a planned change in size;
//               ensures that the capacity of the vector is greater
//               than or equal to n.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE void ordered_vector<Key, Compare>::
reserve(ordered_vector<Key, Compare>::size_type n) {
  _vector.reserve(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::sort
//       Access: Public
//  Description: Ensures that the vector is properly sorted after a
//               potentially damaging operation.  This should not
//               normally need to be called, unless the user has
//               written to the vector using the non-const iterators.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE void ordered_vector<Key, Compare>::
sort() {
  ::sort(begin(), end(), _compare);
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::nci
//       Access: Private
//  Description: I.e. "non-const iterator".  This function is used to
//               typecast a const iterator to a non-const iterator for
//               easy definition of const vs. non-const flavors of
//               some of these methods.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
nci(ordered_vector<Key, Compare>::const_iterator iterator) {
  return begin() + (iterator - begin());
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::find_insert_position
//       Access: Private
//  Description: Searches for the appropriate place in the ordered
//               vector to insert the indicated key, and returns the
//               corresponding iterator.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE ordered_vector<Key, Compare>::iterator ordered_vector<Key, Compare>::
find_insert_position(ordered_vector<Key, Compare>::iterator first,
                     ordered_vector<Key, Compare>::iterator last,
                     const ordered_vector<Key, Compare>::key_type &key) {
  iterator result = r_find_insert_position(first, last, key);

#ifndef NDEBUG
  // Verify the result.
  if (paranoid_ordered_vector) {
    // If there is a node before the indicated position, this node
    // must not precede it lexicograpically.
    if (first < result) {
      nassertr(!_compare(key, *(result - 1)), result);
    }
    // If there is a node after the indicated position, it must not
    // precede this node lexicographically.
    if (result < last) {
      nassertr(!_compare(*(result), key), result);
    }
  }
#endif

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ordered_vector::verify_list
//       Access: Private
//  Description: Ensures that the indicated range of elements is
//               sorted correctly, if paranoid_ordered_vector is set.
//               Generates an assertion failure (and returns false) if
//               this is not the case; otherwise, returns true.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE bool ordered_vector<Key, Compare>::
verify_list() {
#ifndef NDEBUG
  if (paranoid_ordered_vector) {
    return verify_list_impl(begin(), end());
  }
#endif
  return true;
}
