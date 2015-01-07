// Filename: stl_compares.I
// Created by:  drose (28Sep04)
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
//     Function: floating_point_threshold::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE floating_point_threshold<Key>::
floating_point_threshold(Key threshold) :
  _threshold(threshold)
{
}

////////////////////////////////////////////////////////////////////
//     Function: floating_point_threshold::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool floating_point_threshold<Key>::
operator () (const Key &a, const Key &b) const {
  return cfloor(a / _threshold + 0.5f) < cfloor(b / _threshold + 0.5f);
}

////////////////////////////////////////////////////////////////////
//     Function: compare_to::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool compare_to<Key>::
operator () (const Key &a, const Key &b) const {
  return (a.compare_to(b) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: compare_to::is_equal
//       Access: Public
//  Description: Returns true if a is equivalent to b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool compare_to<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a.compare_to(b) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_less::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_less<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a) < (*b));
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_compare_to::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_compare_to<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a).compare_to(*b) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_compare_to::is_equal
//       Access: Public
//  Description: Returns true if a is equivalent to b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_compare_to<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a == b || (*a).compare_to(*b) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_compare_names::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_compare_names<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a).get_name() < (*b).get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_compare_names::is_equal
//       Access: Public
//  Description: Returns true if a is equivalent to b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_compare_names<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a == b || (*a).get_name() == (*b).get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: integer_hash::add_hash
//       Access: Public, Static
//  Description: Adds the indicated key into a running hash.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE size_t integer_hash<Key, Compare>::
add_hash(size_t hash, const Key &key) {
  PN_uint32 key32 = (PN_uint32)(key);
  return AddHash::add_hash(hash, &key32, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: pointer_hash::add_hash
//       Access: Public, Static
//  Description: Adds the indicated key into a running hash.
////////////////////////////////////////////////////////////////////
INLINE size_t pointer_hash::
add_hash(size_t hash, const void *key) {
  // We don't mind if this loses precision.
  PN_uint32 key32 = (PN_uint32)reinterpret_cast<unsigned long>(key);
  return AddHash::add_hash(hash, &key32, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: floating_point_hash::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE floating_point_hash<Key>::
floating_point_hash(Key threshold) :
  _threshold(threshold)
{
}

////////////////////////////////////////////////////////////////////
//     Function: floating_point_hash::operator ()
//       Access: Public
//  Description: Computes a size_t hash from the float.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE size_t floating_point_hash<Key>::
operator () (const Key &key) const {
  return add_hash(0, key);
}

////////////////////////////////////////////////////////////////////
//     Function: floating_point_hash::operator () (two parameters)
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool floating_point_hash<Key>::
operator () (const Key &a, const Key &b) const {
  return cfloor(a / _threshold + 0.5f) < cfloor(b / _threshold + 0.5f);
}

////////////////////////////////////////////////////////////////////
//     Function: floating_point_hash::add_hash
//       Access: Public
//  Description: Adds the indicated key into a running hash.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE size_t floating_point_hash<Key>::
add_hash(size_t hash, const Key &key) const {
  PN_uint32 key32 = (PN_uint32)(key / _threshold + 0.5f);
  return AddHash::add_hash(hash, &key32, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: sequence_hash::operator ()
//       Access: Public
//  Description: Trivially computes a size_t hash from the components
//               of the string.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE size_t sequence_hash<Key, Compare>::
operator () (const Key &key) const {
  return add_hash(0, key);
}

////////////////////////////////////////////////////////////////////
//     Function: sequence_hash::add_hash
//       Access: Public, Static
//  Description: Adds the elements of the indicated key into a running
//               hash.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE size_t sequence_hash<Key, Compare>::
add_hash(size_t hash, const Key &key) {
#ifdef _DEBUG
  // We assume that the sequence is laid out sequentially in memory.
  if (key.size() > 0) {
    assert(&key[key.size() - 1] - &key[0] == key.size() - 1);
  }
#endif
  size_t num_bytes = (key.size() * sizeof(key[0]));
  return AddHash::add_hash(hash, (const PN_uint8 *)&key[0], num_bytes);
}

////////////////////////////////////////////////////////////////////
//     Function: method_hash::operator ()
//       Access: Public
//  Description: Calls the Key's get_hash() method.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE size_t method_hash<Key, Compare>::
operator () (const Key &key) const {
  return key.get_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_method_hash::operator ()
//       Access: Public
//  Description: Calls the Key's get_hash() method.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE size_t indirect_method_hash<Key, Compare>::
operator () (const Key &key) const {
  return (*key).get_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_equals_hash::operator ()
//       Access: Public
//  Description: Calls the Key's get_hash() method.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE size_t indirect_equals_hash<Key>::
operator () (const Key &key) const {
  return (*key).get_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: indirect_equals_hash::is_equal
//       Access: Public
//  Description: Returns true if a is equal to b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_equals_hash<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a == b || (*a) == (*b));
}
