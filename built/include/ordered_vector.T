/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ordered_vector.T
 * @author drose
 * @date 2002-02-02
 */

/**
 * Inserts the indicated key into the ordered vector.  The iterator is a hint
 * to the expected position; if this is correct, the insert operation is
 * likely to be faster.  The return value is the iterator referencing the new
 * element.
 *
 * This flavor of insert does not allow multiple copies of the same key to be
 * inserted.  If the key is already present, it is not inserted, and the
 * iterator referencing the original value is returned.
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
insert_unique(typename ordered_vector<Key, Compare, Vector>::ITERATOR position, 
              const typename ordered_vector<Key, Compare, Vector>::VALUE_TYPE &key) {
  TAU_PROFILE("ordered_vector::insert_unique(iterator, const value_type &)", " ", TAU_USER);
  if (position != end()) {
    // If we're not inserting at the end, the element we're
    // inserting before should not lexicographically precede this one.
    if (_compare(*position, key)) {
      return insert_unique(key).first;

    } else if (!_compare(key, *position)) {
      // Oops, !(*position < key) and !(key < *position).  That means
      // they're equivalent, and we shouldn't insert a new one.
      return position;
    }
  }

  if (position != begin()) {
    // If we're not inserting at the beginning, this element should
    // not lexicographically precede the one we're inserting after.
    if (_compare(key, *(position - 1))) {
      return insert_unique(key).first;

    } else if (!_compare(*(position - 1), key)) {
      // Once again, they're equivalent.
      return position - 1;
    }
  }

  // Otherwise, we may insert where the caller requested.
  ITERATOR result = _vector.insert(position, key);
  return result;
}


/**
 * Inserts the indicated key into the ordered vector.  The iterator is a hint
 * to the expected position; if this is correct, the insert operation is
 * likely to be faster.  The return value is the iterator referencing the new
 * element.
 *
 * This flavor of insert allows multiple copies of the
 * same key to be inserted.
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
insert_nonunique(typename ordered_vector<Key, Compare, Vector>::ITERATOR position, 
                 const typename ordered_vector<Key, Compare, Vector>::VALUE_TYPE &key) {
  TAU_PROFILE("ordered_vector::insert_nonunique(iterator, const value_type &)", " ", TAU_USER);
  if (position != end()) {
    // If we're not inserting at the end, the element we're
    // inserting before should not lexicographically precede this one.
    if (_compare(*position, key)) {
      return insert_nonunique(key);
    }
  }

  if (position != begin()) {
    // If we're not inserting at the beginning, this element should
    // not lexicographically precede the one we're inserting after.
    if (_compare(key, *(position - 1))) {
      return insert_nonunique(key);
    }
  }

  // Otherwise, we may insert where the caller requested.
  ITERATOR result = _vector.insert(position, key);
  return result;
}

/**
 * Ensures that the indicated range of elements is sorted correctly.  Returns
 * true if this is the case; otherwise, returns false.
 */
template<class Key, class Compare, class Vector>
bool ordered_vector<Key, Compare, Vector>::
verify_list_unique() const {
  TAU_PROFILE("ordered_vector::verify_list_unique()", " ", TAU_USER);
  if (!empty()) {
    CONST_ITERATOR prev = begin();
    CONST_ITERATOR i = begin();
    ++i;
    while (i < end()) {
      bool ordered_correctly = _compare(*prev, *i);
      if (!ordered_correctly) {
        return true;
      }
      prev = i;
      ++i;
    }
  }
  return true;
}

/**
 * Ensures that the indicated range of elements is sorted correctly.  Returns
 * true if this is the case; otherwise, returns false.
 */
template<class Key, class Compare, class Vector>
bool ordered_vector<Key, Compare, Vector>::
verify_list_nonunique() const {
  TAU_PROFILE("ordered_vector::verify_list_nonunique()", " ", TAU_USER);
  if (!empty()) {
    CONST_ITERATOR prev = begin();
    CONST_ITERATOR i = begin();
    ++i;
    while (i < end()) {
      bool ordered_correctly = !_compare(*i, *prev);
      if (!ordered_correctly) {
        return true;
      }
      prev = i;
      ++i;
    }
  }
  return true;
}


/**
 * The recursive implementation of find_insert_position().
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::ITERATOR ordered_vector<Key, Compare, Vector>::
r_find_insert_position(typename ordered_vector<Key, Compare, Vector>::ITERATOR first,
                       typename ordered_vector<Key, Compare, Vector>::ITERATOR last,
                       const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) {
  if (first == last) {
    // The list is empty; the insert position is the last of the list.
    return last;
  }

  ITERATOR center = first + (last - first) / 2;
  nassertr(center < last, last);

  if (_compare(key, *center)) {
    // Insert before the center.
    return r_find_insert_position(first, center, key);

  } else {
    // Insert after the center.
    return r_find_insert_position(center + 1, last, key);
  }
}

/**
 * The recursive implementation of find().
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
r_find(typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR first,
       typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR last,
       typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR not_found,
       const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  if (first == last) {
    // The list is empty; the key is not on the list.
    return not_found;
  }

  CONST_ITERATOR center = first + (last - first) / 2;
  nassertr(center < last, last);

  if (_compare(key, *center)) {
    // It must be before the center.
    return r_find(first, center, not_found, key);

  } else if (_compare(*center, key)) {
    // It must be after the center.
    return r_find(center + 1, last, not_found, key);

  } else {
    // Here it is!
    return center;
  }
}

/**
 * The recursive implementation of find_particular().
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
r_find_particular(typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR first,
                  typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR last,
                  typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR not_found,
                  const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  if (first == last) {
    // The list is empty; the key is not on the list.
    return not_found;
  }

  CONST_ITERATOR center = first + (last - first) / 2;
  nassertr(center < last, last);

  if (_compare(key, *center)) {
    // It must be before the center.
    return r_find_particular(first, center, not_found, key);

  } else if (_compare(*center, key)) {
    // It must be after the center.
    return r_find_particular(center + 1, last, not_found, key);

  } else {
    // The center's sort matches the key's sort.  It could be either
    // before or after the center.  First try after.
    CONST_ITERATOR i = center;
    while (i < last && !_compare(key, *i)) {
      if ((*i) == key) {
        return i;
      }
      ++i;
    }

    // No, try before.
    i = center;
    --i;
    while (i >= first && !_compare(key, *i)) {
      if ((*i) == key) {
        return i;
      }
      --i;
    }

    // No such key!
    return not_found;
  }
}

/**
 * The recursive implementation of count().
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::SIZE_TYPE ordered_vector<Key, Compare, Vector>::
r_count(typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR first,
        typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR last,
        const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {

  if (first == last) {
    // The list is empty; the key is not on the list.
    return 0;
  }

  CONST_ITERATOR center = first + (last - first) / 2;
  nassertr(center < last, 0);

  if (_compare(key, *center)) {
    // It must be before the center.
    return r_count(first, center, key);

  } else if (_compare(*center, key)) {
    // It must be after the center.
    return r_count(center + 1, last, key);

  } else {
    // The center matches the key; the range is here.
    size_type lower = r_count(first, center, key);
    size_type upper = r_count(center + 1, last, key);
    return lower + 1 + upper;
  }
}

/**
 * The recursive implementation of lower_bound().
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
r_lower_bound(typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR first,
              typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR last,
              const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  if (first == last) {
    // The list is empty; the key is not on the list.
    return last;
  }

  CONST_ITERATOR center = first + (last - first) / 2;
  nassertr(center < last, last);

  if (_compare(key, *center)) {
    // It must be before the center.
    return r_lower_bound(first, center, key);

  } else if (_compare(*center, key)) {
    // It must be after the center.
    return r_lower_bound(center + 1, last, key);

  } else {
    // The center matches the key; thus, the first element not less
    // than key is at or before the center.
    return r_lower_bound(first, center, key);
  }
}

/**
 * The recursive implementation of upper_bound().
 */
template<class Key, class Compare, class Vector>
typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR ordered_vector<Key, Compare, Vector>::
r_upper_bound(typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR first,
              typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR last,
              const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  if (first == last) {
    // The list is empty; the key is not on the list.
    return last;
  }

  const_iterator center = first + (last - first) / 2;
  nassertr(center < last, last);

  if (_compare(key, *center)) {
    // It must be before the center.
    return r_upper_bound(first, center, key);

  } else if (_compare(*center, key)) {
    // It must be after the center.
    return r_upper_bound(center + 1, last, key);

  } else {
    // The center matches the key; thus, the first element greater
    // than key is after the center.
    return r_upper_bound(center + 1, last, key);
  }
}

/**
 * The recursive implementation of equal_range().
 */
template<class Key, class Compare, class Vector>
std::pair<typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR, typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR> ordered_vector<Key, Compare, Vector>::
r_equal_range(typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR first,
              typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR last,
              const typename ordered_vector<Key, Compare, Vector>::KEY_TYPE &key) const {
  typedef std::pair<typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR, typename ordered_vector<Key, Compare, Vector>::CONST_ITERATOR> pair_type;

  if (first == last) {
    // The list is empty; the key is not on the list.
    return pair_type(last, last);
  }

  CONST_ITERATOR center = first + (last - first) / 2;
  nassertr(center < last, pair_type(last, last));

  if (_compare(key, *center)) {
    // It must be before the center.
    return r_equal_range(first, center, key);

  } else if (_compare(*center, key)) {
    // It must be after the center.
    return r_equal_range(center + 1, last, key);

  } else {
    // The center matches the key; the range is here.
    CONST_ITERATOR lower = r_lower_bound(first, center, key);
    CONST_ITERATOR upper = r_upper_bound(center + 1, last, key);
    return pair_type(lower, upper);
  }
}
