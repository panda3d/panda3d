// Filename: stl_compares.I
// Created by:  drose (28Sep04)
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
//     Function: indirect_compare_names::operator ()
//       Access: Public
//  Description: Returns true if a sorts before b, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key>
INLINE bool indirect_compare_names<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a).get_name() < (*b).get_name());
}

#ifdef HAVE_STL_HASH

////////////////////////////////////////////////////////////////////
//     Function: integer_hash::add_hash
//       Access: Public, Static
//  Description: Adds the indicated key into a running hash.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
INLINE size_t integer_hash<Key, Compare>::
add_hash(size_t hash, const Key &key) {
  return (hash * 31) + (size_t)key;
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
  return (hash * 31) + (size_t)(cfloor(key / _threshold + 0.5f));
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
  Key::const_iterator ki;
  for (ki = key.begin(); ki != key.end(); ++ki) {
    hash = (hash * 31) + (size_t)(*ki);
  }
  return hash;
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

#endif  // HAVE_STL_HASH
