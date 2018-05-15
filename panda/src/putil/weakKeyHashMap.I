/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakKeyHashMap.I
 * @author rdb
 * @date 2015-07-13
 */

/**
 *
 */
template<class Key, class Value>
INLINE WeakKeyHashMap<Key, Value>::
WeakKeyHashMap() :
  _table(NULL),
  _deleted_chain(NULL),
  _table_size(0),
  _num_entries(0)
{
}

/**
 *
 */
template<class Key, class Value>
INLINE WeakKeyHashMap<Key, Value>::
~WeakKeyHashMap() {
  clear();
}

/**
 * Quickly exchanges the contents of this map and the other map.
 */
template<class Key, class Value>
INLINE void WeakKeyHashMap<Key, Value>::
swap(WeakKeyHashMap<Key, Value> &other) {
  TableEntry *t0 = _table;
  _table = other._table;
  other._table = t0;

  DeletedBufferChain *t1 = _deleted_chain;
  _deleted_chain = other._deleted_chain;
  other._deleted_chain = t1;

  size_t t2 = _table_size;
  _table_size = other._table_size;
  other._table_size = t2;

  size_t t3 = _num_entries;
  _num_entries = other._num_entries;
  other._num_entries = t3;
}

/**
 * Searches for the indicated key in the table.  Returns its index number if
 * it is found, or -1 if it is not present in the table.
 */
template<class Key, class Value>
int WeakKeyHashMap<Key, Value>::
find(const Key *key) const {
  if (_table_size == 0) {
    // Special case: the table is empty.
    return -1;
  }

  size_t index = get_hash(key);
  if (!has_element(index)) {
    return -1;
  }
  if (is_element(index, key)) {
    return index;
  }

  // There was some other key at the hashed slot.  That's a hash conflict.
  // Maybe our entry was recorded at a later slot position; scan the
  // subsequent positions until we find the entry or an unused slot,
  // indicating the end of the scan.
  size_t i = index;
  i = (i + 1) & (_table_size - 1);
  while (i != index && has_element(i)) {
    if (is_element(i, key)) {
      return i;
    }
    i = (i + 1) & (_table_size - 1);
  }

  // The key is not in the table.
  return -1;
}

/**
 * Records the indicated key/data pair in the map.  If the key was already
 * present, silently replaces it.  Returns the index at which it was stored.
 */
template<class Key, class Value>
int WeakKeyHashMap<Key, Value>::
store(const Key *key, const Value &data) {
  if (_table_size == 0) {
    // Special case: the first key in an empty table.
    nassertr(_num_entries == 0, -1);
    new_table();
    size_t index = get_hash(key);
    store_new_element(index, key, data);
    ++_num_entries;
#ifdef _DEBUG
    nassertr(validate(), index);
#endif
    return index;
  }

  size_t index = get_hash(key);
  if (!has_element(index)) {
    // This element is not already in the map; add it.
    if (consider_expand_table()) {
      return store(key, data);
    }
    store_new_element(index, key, data);
    ++_num_entries;
#ifdef _DEBUG
    nassertr(validate(), index);
#endif
    return index;
  }
  if (is_element(index, key)) {
    // This element is already in the map; replace the data at that key.
    _table[index]._data = data;
#ifdef _DEBUG
    nassertr(validate(), index);
#endif
    return index;
  }

  // There was some other key at the hashed slot.  That's a hash conflict.
  // Record this entry at a later position.
  size_t i = index;
  i = (i + 1) & (_table_size - 1);
  while (i != index) {
    if (!has_element(i)) {
      if (consider_expand_table()) {
        return store(key, data);
      }
      store_new_element(i, key, data);
      ++_num_entries;
#ifdef _DEBUG
      nassertr(validate(), i);
#endif
      return i;
    }
    if (is_element(i, key)) {
      _table[i]._data = data;
#ifdef _DEBUG
      nassertr(validate(), i);
#endif
      return i;
    }
    i = (i + 1) & (_table_size - 1);
  }

  // Shouldn't get here unless _num_entries == _table_size, which shouldn't be
  // possible due to consider_expand_table().
  nassertr(false, -1);
  return -1;  // To satisfy compiler
}

/**
 * Removes the indicated key and its associated data from the table.  Returns
 * true if the key was removed, false if it was not present.
 */
template<class Key, class Value>
INLINE bool WeakKeyHashMap<Key, Value>::
remove(const Key *key) {
  int index = find(key);
  if (index == -1) {
    return false;
  }
  remove_element(index);
  return true;
}

/**
 * Completely empties the table.
 */
template<class Key, class Value>
void WeakKeyHashMap<Key, Value>::
clear() {
  if (_table_size != 0) {
    for (size_t i = 0; i < _table_size; ++i) {
      if (get_exists_array()[i] != 0) {
        clear_element(i);
      }
    }

    _deleted_chain->deallocate(_table, TypeHandle::none());
    _table = NULL;
    _deleted_chain = NULL;
    _table_size = 0;
    _num_entries = 0;
  }
}

/**
 * Returns a modifiable reference to the data associated with the indicated
 * key, or creates a new data entry and returns its reference.
 */
template<class Key, class Value>
INLINE Value &WeakKeyHashMap<Key, Value>::
operator [] (const Key *key) {
  int index = find(key);
  if (index == -1) {
    index = store(key, Value());
  }
  return modify_data(index);
}

/**
 * Returns the total number of slots in the table.
 */
template<class Key, class Value>
INLINE size_t WeakKeyHashMap<Key, Value>::
get_size() const {
  return _table_size;
}

/**
 * Returns true if there is an element stored in the nth slot, false
 * otherwise.
 *
 * n should be in the range 0 <= n < get_size().
 */
template<class Key, class Value>
INLINE bool WeakKeyHashMap<Key, Value>::
has_element(size_t n) const {
  nassertr(n < _table_size, false);
  return (get_exists_array()[n] != 0 && !_table[n]._key.was_deleted());
}

/**
 * Returns the key in the nth slot of the table.
 *
 * It is an error to call this if there is nothing stored in the nth slot (use
 * has_element() to check this first).  n should be in the range 0 <= n <
 * get_size().
 */
template<class Key, class Value>
INLINE const Key *WeakKeyHashMap<Key, Value>::
get_key(size_t n) const {
  nassertr(has_element(n), _table[n]._key);
  return _table[n]._key;
}

/**
 * Returns the data in the nth slot of the table.
 *
 * It is an error to call this if there is nothing stored in the nth slot (use
 * has_element() to check this first).  n should be in the range 0 <= n <
 * get_size().
 */
template<class Key, class Value>
INLINE const Value &WeakKeyHashMap<Key, Value>::
get_data(size_t n) const {
  nassertr(has_element(n), _table[n]._data);
  return _table[n]._data;
}

/**
 * Returns a modifiable reference to the data in the nth slot of the table.
 *
 * It is an error to call this if there is nothing stored in the nth slot (use
 * has_element() to check this first).  n should be in the range 0 <= n <
 * get_size().
 */
template<class Key, class Value>
INLINE Value &WeakKeyHashMap<Key, Value>::
modify_data(size_t n) {
  nassertr(has_element(n), _table[n]._data);
  return _table[n]._data;
}

/**
 * Changes the data for the nth slot of the table.
 *
 * It is an error to call this if there is nothing stored in the nth slot (use
 * has_element() to check this first).  n should be in the range 0 <= n <
 * get_size().
 */
template<class Key, class Value>
INLINE void WeakKeyHashMap<Key, Value>::
set_data(size_t n, const Value &data) {
  nassertv(has_element(n));
  _table[n]._data = data;
}

#ifdef USE_MOVE_SEMANTICS
/**
 * Changes the data for the nth slot of the table.
 *
 * It is an error to call this if there is nothing stored in the nth slot (use
 * has_element() to check this first).  n should be in the range 0 <= n <
 * get_size().
 */
template<class Key, class Value>
INLINE void WeakKeyHashMap<Key, Value>::
set_data(size_t n, Value &&data) {
  nassertv(has_element(n));
  _table[n]._data = move(data);
}
#endif  // USE_MOVE_SEMANTICS

/**
 * Removes the nth slot from the table.
 *
 * It is an error to call this if there is nothing stored in the nth slot (use
 * has_element() to check this first).  n should be in the range 0 <= n <
 * get_size().
 */
template<class Key, class Value>
void WeakKeyHashMap<Key, Value>::
remove_element(size_t n) {
  nassertv(get_exists_array()[n] != 0);

  clear_element(n);
  nassertv(_num_entries > 0);
  --_num_entries;

  // Now we have put a hole in the table.  If there was a hash conflict in the
  // slot following this one, we have to move it down to close the hole.
  size_t i = n;
  i = (i + 1) & (_table_size - 1);
  while (get_exists_array()[i] != 0) {
    if (_table[i]._key.was_deleted()) {
      // It was deleted.  Forget about it.
      clear_element(i);
      --_num_entries;
    } else {
      size_t wants_index = get_hash(_table[i]._key.get_orig());
      if (wants_index != i) {
        // This one was a hash conflict; try to put it where it belongs.  We
        // can't just put it in n, since maybe it belongs somewhere after n.
        while (wants_index != i && has_element(wants_index)) {
          // Hash conflict; move it up.
          wants_index = (wants_index + 1) & (_table_size - 1);
        }
        if (wants_index != i) {
          store_new_element(wants_index, _table[i]._key, _table[i]._data);
          clear_element(i);
        }
      }
    }

    // Continue until we encounter the next unused slot.  Until we do, we
    // can't be sure we've found all of the potential hash conflicts.
    i = (i + 1) & (_table_size - 1);
  }

#ifdef _DEBUG
  nassertv(validate());
#endif
}

/**
 * Returns the number of active entries in the table.  This is not necessarily
 * related to the number of slots in the table as reported by get_size().  Use
 * get_size() to iterate through all of the slots, not get_num_entries().
 *
 * This is merely an upper bound on the number of entries; it may also count
 * false positives for pointers that were recently deleted.
 */
template<class Key, class Value>
INLINE size_t WeakKeyHashMap<Key, Value>::
get_num_entries() const {
  return _num_entries;
}

/**
 * Returns true if the table is empty; i.e.  get_num_entries() == 0.  This may
 * return a false negatives if a pointer was recently deleted; if this returns
 * true, though, you can be sure it's empty.
 */
template<class Key, class Value>
INLINE bool WeakKeyHashMap<Key, Value>::
is_empty() const {
  return (_num_entries == 0);
}

/**
 *
 */
template<class Key, class Value>
void WeakKeyHashMap<Key, Value>::
output(ostream &out) const {
  out << "WeakKeyHashMap (" << _num_entries << " entries): [";
  for (size_t i = 0; i < _table_size; ++i) {
    if (get_exists_array()[i] == 0) {
      out << " *";

    } else {
      out << " " << _table[i]._key;
      size_t index = get_hash(_table[i]._key.get_orig());
      if (index != i) {
        // This was misplaced as the result of a hash conflict.  Report how
        // far off it is.
        out << "(" << ((_table_size + i - index) & (_table_size - 1)) << ")";
      }
    }
  }
  out << " ]";
}

/**
 *
 */
template<class Key, class Value>
void WeakKeyHashMap<Key, Value>::
write(ostream &out) const {
  output(out);
  out << "\n";
}

/**
 * Returns true if the internal table appears to be consistent, false if there
 * are some internal errors.
 */
template<class Key, class Value>
bool WeakKeyHashMap<Key, Value>::
validate() const {
  size_t count = 0;

  const unsigned char *exists_array = get_exists_array();

  for (size_t i = 0; i < _table_size; ++i) {
    if (exists_array[i] != 0) {
      ++count;
      if (_table[i]._key.was_deleted()) {
        continue;
      }
      size_t ideal_index = get_hash(_table[i]._key.get_orig());
      size_t wants_index = ideal_index;
      while (wants_index != i && exists_array[wants_index] != 0) {
        wants_index = (wants_index + 1) & (_table_size - 1);
      }
      if (wants_index != i) {
        util_cat.error()
          << "WeakKeyHashMap is invalid: key " << _table[i]._key
          << " should be in slot " << wants_index << " instead of "
          << i << " (ideal is " << ideal_index << ")\n";
        write(util_cat.error(false));
        return false;
      }
    }
  }

  if (count != _num_entries) {
    util_cat.error()
      << "WeakKeyHashMap is invalid: reports " << _num_entries
      << " entries, actually has " << count << "\n";
    write(util_cat.error(false));
    return false;
  }

  return true;
}

/**
 * Computes an appropriate index number to store the given pointer.
 */
template<class Key, class Value>
INLINE size_t WeakKeyHashMap<Key, Value>::
get_hash(const Key *key) const {
  /*
  // We want a hash constant 0 < k < 1.  This one is suggested by Knuth:
  static const double hash_constant = (sqrt(5.0) - 1.0) / 2.0;
  double f = ((double)(size_t)key * hash_constant);
  f -= floor(f);
  return (size_t)floor(f * _table_size);
  */

  return (((size_t)key * (size_t)9973) >> 8) & (_table_size - 1);
}

/**
 * Returns true if element n matches key.
 */
template<class Key, class Value>
INLINE bool WeakKeyHashMap<Key, Value>::
is_element(size_t n, const Key *key) const {
  nassertr(has_element(n), false);
  return _table[n]._key == key;
}

/**
 * Constructs a new TableEntry at position n, storing the indicated key and
 * value.
 */
template<class Key, class Value>
INLINE void WeakKeyHashMap<Key, Value>::
store_new_element(size_t n, const Key *key, const Value &data) {
  if (get_exists_array()[n] != 0) {
    // There was already an element in this spot.  This can happen if it was a
    // pointer that had already been deleted.
    nassertv(_table[n]._key.was_deleted());
    _table[n].~TableEntry();
    --_num_entries;
  }
  new(&_table[n]) TableEntry(key, data);
  get_exists_array()[n] = true;
}

/**
 * Destructs the TableEntry at position n.
 */
template<class Key, class Value>
INLINE void WeakKeyHashMap<Key, Value>::
clear_element(size_t n) {
  _table[n].~TableEntry();
  get_exists_array()[n] = false;
}

/**
 * Returns the beginning of the array of _table_size unsigned chars that are
 * the boolean flags for whether each element exists (has been constructed)
 * within the table.
 */
template<class Key, class Value>
INLINE unsigned char *WeakKeyHashMap<Key, Value>::
get_exists_array() const {
  return (unsigned char *)(_table + _table_size);
}

/**
 * Allocates a brand new table.
 */
template<class Key, class Value>
void WeakKeyHashMap<Key, Value>::
new_table() {
  nassertv(_table_size == 0 && _num_entries == 0);

  // Pick a good initial table size.  For now, we make it really small.  Maybe
  // that's the right answer.
  _table_size = 4;

  // We allocate enough bytes for _table_size elements of TableEntry, plus
  // _table_size more bytes at the end (for the exists array).
  size_t alloc_size = _table_size * sizeof(TableEntry) + _table_size;

  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
  memset(get_exists_array(), 0, _table_size);
}

/**
 * Expands the table if it will need it (assuming one more element is about to
 * be added).  Returns true if the table was modified, false otherwise.
 */
template<class Key, class Value>
INLINE bool WeakKeyHashMap<Key, Value>::
consider_expand_table() {
  if (_num_entries >= (_table_size >> 1)) {
    // Actually, first, we should see if there are any deleted pointers.
    // Clean those up and see how much space we save.
    for (size_t i = 0; i < _table_size; ++i) {
      if (get_exists_array()[i] != 0 && _table[i]._key.was_deleted()) {
        remove_element(i);
      }
    }
    if (_num_entries >= (_table_size >> 1)) {
      // Still not enough space.
      expand_table();
    }
    return true;
  }
  return false;
}

/**
 * Doubles the size of the existing table.
 */
template<class Key, class Value>
void WeakKeyHashMap<Key, Value>::
expand_table() {
  nassertv(_table_size != 0);

  WeakKeyHashMap<Key, Value> old_map;
  swap(old_map);

  // Double the table size.
  size_t old_table_size = old_map._table_size;
  _table_size = (old_table_size << 1);
  nassertv(_table == NULL);

  // We allocate enough bytes for _table_size elements of TableEntry, plus
  // _table_size more bytes at the end (for the exists array).
  size_t alloc_size = _table_size * sizeof(TableEntry) + _table_size;
  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
  unsigned char *exists_array = get_exists_array();
  memset(exists_array, 0, _table_size);
  nassertv(_num_entries == 0);

  // Now copy the entries from the old table into the new table.
  for (size_t i = 0; i < old_table_size; ++i) {
    if (old_map.has_element(i)) {
      size_t new_index = get_hash(old_map._table[i]._key.get_orig());

      while (exists_array[new_index] != 0) {
        // Hash conflict;  look for a better spot.  This has to succeed.
        new_index = (new_index + 1) & (_table_size - 1);
      }

#ifdef USE_MOVE_SEMANTICS
      // Use C++11 rvalue references to invoke the move constructor, which may
      // be more efficient.
      new(&_table[new_index]) TableEntry(move(old_map._table[i]));
#else
      new(&_table[new_index]) TableEntry(old_map._table[i]);
#endif
      exists_array[new_index] = true;
      ++_num_entries;
    }
  }

  nassertv(validate());
  nassertv(old_map.validate());

  // Note that since has_element(i) also checks whether the pointer has been
  // deleted, we may end up with fewer entries than we started with.  Good
  // riddance.
  nassertv(_num_entries <= old_map._num_entries);
}
