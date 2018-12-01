/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stl_compares.I
 * @author drose
 * @date 2004-09-28
 */

/**
 *
 */
template<class Key>
INLINE floating_point_threshold<Key>::
floating_point_threshold(Key threshold) :
  _threshold(threshold)
{
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class Key>
INLINE bool floating_point_threshold<Key>::
operator () (const Key &a, const Key &b) const {
  return cfloor(a / _threshold + 0.5f) < cfloor(b / _threshold + 0.5f);
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class Key>
INLINE bool compare_to<Key>::
operator () (const Key &a, const Key &b) const {
  return (a.compare_to(b) < 0);
}

/**
 * Returns true if a is equivalent to b, false otherwise.
 */
template<class Key>
INLINE bool compare_to<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a.compare_to(b) == 0);
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class Key>
INLINE bool indirect_less<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a) < (*b));
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class Key>
INLINE bool indirect_compare_to<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a).compare_to(*b) < 0);
}

/**
 * Returns true if a is equivalent to b, false otherwise.
 */
template<class Key>
INLINE bool indirect_compare_to<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a == b || (*a).compare_to(*b) == 0);
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class Key>
INLINE bool indirect_compare_names<Key>::
operator () (const Key &a, const Key &b) const {
  return (a != b && (*a).get_name() < (*b).get_name());
}

/**
 * Returns true if a is equivalent to b, false otherwise.
 */
template<class Key>
INLINE bool indirect_compare_names<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a == b || (*a).get_name() == (*b).get_name());
}

/**
 * Adds the indicated key into a running hash.
 */
template<class Key, class Compare>
INLINE size_t integer_hash<Key, Compare>::
add_hash(size_t hash, const Key &key) {
  uint32_t key32 = (uint32_t)(key);
  return AddHash::add_hash(hash, &key32, 1);
}

/**
 * Adds the indicated key into a running hash.
 */
INLINE size_t pointer_hash::
add_hash(size_t hash, const void *key) {
  // We don't mind if this loses precision.
  uint32_t key32 = (uint32_t)reinterpret_cast<uintptr_t>(key);
  return AddHash::add_hash(hash, &key32, 1);
}

/**
 *
 */
template<class Key>
INLINE floating_point_hash<Key>::
floating_point_hash(Key threshold) :
  _threshold(threshold)
{
}

/**
 * Computes a size_t hash from the float.
 */
template<class Key>
INLINE size_t floating_point_hash<Key>::
operator () (const Key &key) const {
  return add_hash(0, key);
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class Key>
INLINE bool floating_point_hash<Key>::
operator () (const Key &a, const Key &b) const {
  return cfloor(a / _threshold + 0.5f) < cfloor(b / _threshold + 0.5f);
}

/**
 * Adds the indicated key into a running hash.
 */
template<class Key>
INLINE size_t floating_point_hash<Key>::
add_hash(size_t hash, const Key &key) const {
  uint32_t key32 = (uint32_t)(key / _threshold + 0.5f);
  return AddHash::add_hash(hash, &key32, 1);
}

/**
 * Trivially computes a size_t hash from the components of the string.
 */
template<class Key, class Compare>
INLINE size_t sequence_hash<Key, Compare>::
operator () (const Key &key) const {
  return add_hash(0, key);
}

/**
 * Adds the elements of the indicated key into a running hash.
 */
template<class Key, class Compare>
INLINE size_t sequence_hash<Key, Compare>::
add_hash(size_t hash, const Key &key) {
#ifdef _DEBUG
  // We assume that the sequence is laid out sequentially in memory.
  if (key.size() > 0) {
    assert(&key[key.size() - 1] - &key[0] == (ptrdiff_t)key.size() - 1);
  }
#endif
  size_t num_bytes = (key.size() * sizeof(key[0]));
  return AddHash::add_hash(hash, (const uint8_t *)&key[0], num_bytes);
}

/**
 * Calls the Key's get_hash() method.
 */
template<class Key, class Compare>
INLINE size_t method_hash<Key, Compare>::
operator () (const Key &key) const {
  return key.get_hash();
}

/**
 * Calls the Key's get_hash() method.
 */
template<class Key, class Compare>
INLINE size_t indirect_method_hash<Key, Compare>::
operator () (const Key &key) const {
  return (*key).get_hash();
}

/**
 * Calls the Key's get_hash() method.
 */
template<class Key>
INLINE size_t indirect_equals_hash<Key>::
operator () (const Key &key) const {
  return (*key).get_hash();
}

/**
 * Returns true if a is equal to b, false otherwise.
 */
template<class Key>
INLINE bool indirect_equals_hash<Key>::
is_equal(const Key &a, const Key &b) const {
  return (a == b || (*a) == (*b));
}
