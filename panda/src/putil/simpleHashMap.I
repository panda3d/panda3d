/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleHashMap.I
 * @author drose
 * @date 2007-07-19
 */

/**
 *
 */
template<class Key, class Value, class Compare>
constexpr SimpleHashMap<Key, Value, Compare>::
SimpleHashMap(const Compare &comp) :
  _table(nullptr),
  _deleted_chain(nullptr),
  _table_size(0),
  _num_entries(0),
  _comp(comp)
{
}

/**
 *
 */
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare>::
SimpleHashMap(const SimpleHashMap &copy) :
  _table_size(copy._table_size),
  _num_entries(copy._num_entries),
  _comp(copy._comp) {

  // We allocate enough bytes for _table_size elements of TableEntry, plus
  // _table_size * 4 more ints at the end (for the index array).
  size_t alloc_size = _table_size * (sizeof(TableEntry) + sizeof(int) * sparsity);

  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());

  for (size_t i = 0; i < _num_entries; ++i) {
    new(&_table[i]) TableEntry(copy._table[i]);
  }

  // Copy the index array.
  memcpy(get_index_array(), copy.get_index_array(), _table_size * sizeof(int) * sparsity);
}

/**
 *
 */
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare>::
SimpleHashMap(SimpleHashMap &&from) noexcept :
  _table(from._table),
  _deleted_chain(from._deleted_chain),
  _table_size(from._table_size),
  _num_entries(from._num_entries),
  _comp(std::move(from._comp))
{
  from._table = nullptr;
  from._deleted_chain = nullptr;
  from._table_size = 0;
  from._num_entries = 0;
}

/**
 *
 */
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare>::
~SimpleHashMap() {
  clear();
}

/**
 *
 */
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare> &SimpleHashMap<Key, Value, Compare>::
operator = (const SimpleHashMap<Key, Value, Compare> &copy) {
  if (this != &copy) {
    _table_size = copy._table_size;
    _num_entries = copy._num_entries;
    _comp = copy._comp;

    // We allocate enough bytes for _table_size elements of TableEntry, plus
    // _table_size * 4 more ints at the end (for the index array).
    size_t alloc_size = _table_size * (sizeof(TableEntry) + sizeof(int) * sparsity);

    _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
    _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
    for (size_t i = 0; i < _num_entries; ++i) {
      new(&_table[i]) TableEntry(copy._table[i]);
    }

    // Copy the index array.
    memcpy(get_index_array(), copy.get_index_array(), _table_size * sizeof(int) * sparsity);
  }
  return *this;
}

/**
 *
 */
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare> &SimpleHashMap<Key, Value, Compare>::
operator = (SimpleHashMap<Key, Value, Compare> &&from) noexcept {
  if (this != &from) {
    _table = from._table;
    _deleted_chain = from._deleted_chain;
    _table_size = from._table_size;
    _num_entries = from._num_entries;
    _comp = std::move(from._comp);

    from._table = nullptr;
    from._deleted_chain = nullptr;
    from._table_size = 0;
    from._num_entries = 0;
  }
}

/**
 * Quickly exchanges the contents of this map and the other map.
 */
template<class Key, class Value, class Compare>
INLINE void SimpleHashMap<Key, Value, Compare>::
swap(SimpleHashMap<Key, Value, Compare> &other) {
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
template<class Key, class Value, class Compare>
int SimpleHashMap<Key, Value, Compare>::
find(const Key &key) const {
  if (_table_size == 0) {
    // Special case: the table is empty.
    return -1;
  }

  int slot = find_slot(key);
  if (slot >= 0) {
    return get_index_array()[slot];
  } else {
    // The key is not in the table.
    return -1;
  }
}

/**
 * Records the indicated key/data pair in the map.  If the key was already
 * present, silently replaces it.  Returns the index at which it was stored.
 */
template<class Key, class Value, class Compare>
int SimpleHashMap<Key, Value, Compare>::
store(const Key &key, const Value &data) {
  if (_table_size == 0) {
    // Special case: the first key in an empty table.
    nassertr(_num_entries == 0, -1);
    new_table();
    int pos = store_new_element(get_hash(key), key, data);
#ifdef _DEBUG
    nassertr(validate(), pos);
#endif
    return pos;
  }
  consider_expand_table();

  const int *index_array = get_index_array();
  size_t hash = get_hash(key);
  int index = index_array[hash];
  if (index < 0) {
    // This element is not already in the map; add it.
    if (consider_expand_table()) {
      return store(key, data);
    }
    index = store_new_element(hash, key, data);
#ifdef _DEBUG
    nassertr(validate(), index);
#endif
    return index;
  }
  if (is_element(index, key)) {
    // This element is already in the map; replace the data at that key.
    set_data(index, data);
#ifdef _DEBUG
    nassertr(validate(), index);
#endif
    return index;
  }

  // There was some other key at the hashed slot.  That's a hash conflict.
  // Record this entry at a later position.
  size_t slot = next_hash(hash);
  while (slot != hash) {
    index = index_array[slot];
    if (index < 0) {
      if (consider_expand_table()) {
        return store(key, data);
      }
      index = store_new_element(slot, key, data);
#ifdef _DEBUG
      nassertr(validate(), index);
#endif
      return index;
    }
    if (is_element(index, key)) {
      set_data(index, data);
#ifdef _DEBUG
      nassertr(validate(), index);
#endif
      return index;
    }
    slot = next_hash(slot);
  }

  // Shouldn't get here unless _num_entries == _table_size, which shouldn't be
  // possible due to consider_expand_table().
  nassertr(false, -1);
  return -1;  // To satisfy compiler
}

/**
 * Removes the indicated key and its associated data from the table.  Returns
 * true if the key was removed, false if it was not present.
 *
 * Iterator safety:  To perform removal during iteration, revisit the element
 * at the current index if removal succeeds,  keeping in mind that the number
 * of elements has now shrunk by one.
 */
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
remove(const Key &key) {
  if (_num_entries == 0) {
    // Special case: the table is empty.
    return false;
  }

  int *index_array = get_index_array();
  size_t slot = (size_t)find_slot(key);
  if (slot == (size_t)-1) {
    // It wasn't in the hash map.
    return false;
  }

  // Now remove this element.
  size_t last = _num_entries - 1;
  size_t index = (size_t)index_array[slot];
  if (index < _num_entries) {
    // Find the last element in the index array.
    int other_slot = find_slot(_table[last]._key);
    nassertr(other_slot != -1, false);
    nassertr(index_array[(size_t)other_slot] == (int)last, false);

    // Swap it with the last one, so that we don't get any gaps in the table
    // of entries.
    _table[index] = std::move(_table[last]);
    index_array[(size_t)other_slot] = index;
  }

  _table[last].~TableEntry();
  _num_entries = last;

  // It's important that we do this after the second find_slot, above, since
  // it might otherwise fail due to the unexpected gap, since some indices may
  // not be at their ideal positions right now.
  index_array[slot] = -1;

  //if (consider_shrink_table()) {
  //  // No need to worry about that gap; resize_table() will rebuild the index.
  //  return true;
  //}

  // Now we have put a hole in the index array.  If there was a hash conflict
  // in the slot after this one, we have to move it down to close the hole.
  slot = next_hash(slot);
  while (has_slot(slot)) {
    size_t index = (size_t)index_array[slot];
    size_t wants_slot = get_hash(_table[index]._key);
    if (wants_slot != slot) {
      // This one was a hash conflict; try to put it where it belongs.  We
      // can't just put it in n, since maybe it belongs somewhere after n.
      while (wants_slot != slot && has_slot(wants_slot)) {
        wants_slot = next_hash(wants_slot);
      }
      if (wants_slot != slot) {
        // We just have to flip the slots in the index array; we can keep the
        // elements in the table where they are.
        index_array[wants_slot] = index;
        index_array[slot] = -1;
      }
    }

    // Continue until we encounter the next unused slot.  Until we do, we
    // can't be sure we've found all of the potential hash conflicts.
    slot = next_hash(slot);
  }

#ifdef _DEBUG
  nassertr(validate(), true);
#endif
  return true;
}

/**
 * Completely empties the table.
 */
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
clear() {
  if (_table_size != 0) {
    for (size_t i = 0; i < _num_entries; ++i) {
      _table[i].~TableEntry();
    }

    _deleted_chain->deallocate(_table, TypeHandle::none());
    _table = nullptr;
    _deleted_chain = nullptr;
    _table_size = 0;
    _num_entries = 0;
  }
}

/**
 * Returns a modifiable reference to the data associated with the indicated
 * key, or creates a new data entry and returns its reference.
 */
template<class Key, class Value, class Compare>
INLINE Value &SimpleHashMap<Key, Value, Compare>::
operator [] (const Key &key) {
  int index = find(key);
  if (index == -1) {
    index = store(key, Value());
  }
  return modify_data(index);
}

/**
 * Returns the total number of entries in the table.  Same as get_num_entries.
 */
template<class Key, class Value, class Compare>
constexpr size_t SimpleHashMap<Key, Value, Compare>::
size() const {
  return _num_entries;
}

/**
 * Returns the key in the nth entry of the table.
 *
 * @param n should be in the range 0 <= n < size().
 */
template<class Key, class Value, class Compare>
INLINE const Key &SimpleHashMap<Key, Value, Compare>::
get_key(size_t n) const {
  nassertr(n < _num_entries, _table[n]._key);
  return _table[n]._key;
}

/**
 * Returns the data in the nth entry of the table.
 *
 * @param n should be in the range 0 <= n < size().
 */
template<class Key, class Value, class Compare>
INLINE const Value &SimpleHashMap<Key, Value, Compare>::
get_data(size_t n) const {
  nassertr(n < _num_entries, _table[n].get_data());
  return _table[n].get_data();
}

/**
 * Returns a modifiable reference to the data in the nth entry of the table.
 *
 * @param n should be in the range 0 <= n < size().
 */
template<class Key, class Value, class Compare>
INLINE Value &SimpleHashMap<Key, Value, Compare>::
modify_data(size_t n) {
  nassertr(n < _num_entries, _table[n].modify_data());
  return _table[n].modify_data();
}

/**
 * Changes the data for the nth entry of the table.
 *
 * @param n should be in the range 0 <= n < size().
 */
template<class Key, class Value, class Compare>
INLINE void SimpleHashMap<Key, Value, Compare>::
set_data(size_t n, const Value &data) {
  nassertv(n < _num_entries);
  _table[n].set_data(data);
}

/**
 * Changes the data for the nth entry of the table.
 *
 * @param n should be in the range 0 <= n < size().
 */
template<class Key, class Value, class Compare>
INLINE void SimpleHashMap<Key, Value, Compare>::
set_data(size_t n, Value &&data) {
  nassertv(n < _num_entries);
  _table[n].set_data(std::move(data));
}

/**
 * Removes the nth entry from the table.
 *
 * @param n should be in the range 0 <= n < size().
 */
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
remove_element(size_t n) {
  nassertv(n < _num_entries);
  remove(_table[n]._key);
}

/**
 * Returns the number of active entries in the table.  Same as size().
 */
template<class Key, class Value, class Compare>
INLINE size_t SimpleHashMap<Key, Value, Compare>::
get_num_entries() const {
  return _num_entries;
}

/**
 * Returns true if the table is empty; i.e. get_num_entries() == 0.
 */
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
is_empty() const {
  return (_num_entries == 0);
}

/**
 *
 */
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
output(std::ostream &out) const {
  out << "SimpleHashMap (" << _num_entries << " entries): [";
  const int *index_array = get_index_array();
  size_t num_slots = _table_size * sparsity;
  for (size_t slot = 0; slot < num_slots; ++slot) {
    if (!has_slot(slot)) {
      out << " *";

    } else {
      size_t index = (size_t)index_array[slot];
      out << " " << index;
      size_t ideal_slot = get_hash(_table[index]._key);
      if (ideal_slot != slot) {
        // This was misplaced as the result of a hash conflict.  Report how
        // far off it is.
        out << "(" << ((_table_size + slot - ideal_slot) & (num_slots - 1)) << ")";
      }
    }
  }
  out << " ]";
}

/**
 *
 */
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
write(std::ostream &out) const {
  output(out);
  out << "\n";
  for (size_t i = 0; i < _num_entries; ++i) {
    out << "  " << _table[i]._key << " (hash " << get_hash(_table[i]._key) << ")\n";
  }
}

/**
 * Returns true if the internal table appears to be consistent, false if there
 * are some internal errors.
 */
template<class Key, class Value, class Compare>
bool SimpleHashMap<Key, Value, Compare>::
validate() const {
  size_t count = 0;

  const int *index_array = get_index_array();
  size_t num_slots = _table_size * sparsity;
  for (size_t slot = 0; slot < num_slots; ++slot) {
    if (has_slot(slot)) {
      size_t index = (size_t)index_array[slot];
      ++count;
      if (index >= _num_entries) {
        util_cat.error()
          << "SimpleHashMap " << this << " is invalid: slot " << slot
          << " contains index " << index << " which is past the end of the"
             " table\n";
        write(util_cat.error(false));
        return false;
      }
      nassertd(index < _num_entries) continue;
      size_t ideal_slot = get_hash(_table[index]._key);
      size_t wants_slot = ideal_slot;
      while (wants_slot != slot && has_slot(wants_slot)) {
        wants_slot = next_hash(wants_slot);
      }
      if (wants_slot != slot) {
        util_cat.error()
          << "SimpleHashMap " << this << " is invalid: key "
          << _table[index]._key << " should be in slot " << wants_slot
          << " instead of " << slot << " (ideal is " << ideal_slot << ")\n";
        write(util_cat.error(false));
        return false;
      }
    }
  }

  if (count != _num_entries) {
    util_cat.error()
      << "SimpleHashMap " << this << " is invalid: reports " << _num_entries
      << " entries, actually has " << count << "\n";
    write(util_cat.error(false));
    return false;
  }

  return true;
}

/**
 * Computes an appropriate index number to store the given pointer.
 */
template<class Key, class Value, class Compare>
INLINE size_t SimpleHashMap<Key, Value, Compare>::
get_hash(const Key &key) const {
  /*
  // We want a hash constant 0 < k < 1.  This one is suggested by Knuth:
  static const double hash_constant = (sqrt(5.0) - 1.0) / 2.0;
  double f = ((double)_comp(key) * hash_constant);
  f -= floor(f);
  return (size_t)floor(f * _table_size);
  */

  return ((_comp(key) * (size_t)9973) >> 8) & ((_table_size * sparsity) - 1);
}

/**
 * Given a hash value, increments it, looping around the hash space.
 */
template<class Key, class Value, class Compare>
INLINE size_t SimpleHashMap<Key, Value, Compare>::
next_hash(size_t hash) const {
  return (hash + 1) & ((_table_size * sparsity) - 1);
}

/**
 * Finds the slot in which the given key should fit.
 */
template<class Key, class Value, class Compare>
INLINE int SimpleHashMap<Key, Value, Compare>::
find_slot(const Key &key) const {
  const int *index_array = get_index_array();
  size_t hash = get_hash(key);
  int index = index_array[hash];
  if (index < 0) {
    return -1;
  }

  if (is_element((size_t)index, key)) {
    return hash;
  }

  // There was some other key at the hashed slot.  That's a hash conflict.
  // Maybe our entry was recorded at a later slot position; scan the
  // subsequent positions until we find the entry or an unused slot,
  // indicating the end of the scan.
  size_t slot = next_hash(hash);
  while (slot != hash && has_slot(slot)) {
    if (is_element((size_t)index_array[slot], key)) {
      return (int)slot;
    }
    slot = next_hash(slot);
  }

  return -1;
}

/**
 * Returns true if the given slot refers to an element.
 */
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
has_slot(size_t slot) const {
  return get_index_array()[slot] >= 0;
}

/**
 * Returns true if element n matches key.
 */
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
is_element(size_t n, const Key &key) const {
  nassertr(n < _num_entries, false);
  return _comp.is_equal(_table[n]._key, key);
}

/**
 * Constructs a new TableEntry with the given slot, storing the indicated key
 * and value.
 */
template<class Key, class Value, class Compare>
INLINE size_t SimpleHashMap<Key, Value, Compare>::
store_new_element(size_t slot, const Key &key, const Value &data) {
  size_t index = _num_entries++;
  new(&_table[index]) TableEntry(key, data);
  nassertr(get_index_array()[slot] == -1, index)
  get_index_array()[slot] = index;
  return index;
}

/**
 * Returns the beginning of the array of _table_size ints that are the indices
 * pointing to the location within the table where the elements are stored.
 * within the table.
 */
template<class Key, class Value, class Compare>
INLINE int *SimpleHashMap<Key, Value, Compare>::
get_index_array() const {
  return (int *)(_table + _table_size);
}

/**
 * Allocates a brand new table.
 */
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
new_table() {
  nassertv(_table_size == 0 && _num_entries == 0);

  // Pick a good initial table size.  For now, we make it really small.  Maybe
  // that's the right answer.
  _table_size = 2;

  // We allocate enough bytes for _table_size elements of TableEntry, plus
  // _table_size * 4 more ints at the end (for the index array).
  size_t alloc_size = _table_size * (sizeof(TableEntry) + sizeof(int) * sparsity);

  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
  memset(get_index_array(), -1, _table_size * sizeof(int) * sparsity);
}

/**
 * Expands the table if it will need it (assuming one more element is about to
 * be added).  Returns true if expanded, false otherwise.
 */
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
consider_expand_table() {
  if (_num_entries < _table_size) {
    return false;
  } else {
    resize_table(_table_size << 1);
    return true;
  }
}

/**
 * Shrinks the table if the allocated storage is significantly larger than the
 * number of elements in it.  Returns true if shrunk, false otherwise.
 */
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
consider_shrink_table() {
  // If the number of elements gets less than an eighth of the table size, we
  // know it's probably time to shrink it down.
  if (_table_size <= 16 || _num_entries >= (_table_size >> 3)) {
    return false;
  } else {
    size_t new_size = _table_size;
    do {
      new_size >>= 1;
    } while (new_size >= 16 && _num_entries < (new_size >> 2));
    resize_table(new_size);
    return true;
  }
}

/**
 * Resizes the existing table.
 */
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
resize_table(size_t new_size) {
  nassertv(_table_size != 0);
  nassertv(new_size >= _num_entries);

  DeletedBufferChain *old_chain = _deleted_chain;
  TableEntry *old_table = _table;

  _table_size = new_size;

  // We allocate enough bytes for _table_size elements of TableEntry, plus
  // _table_size * sparsity more ints at the end (for the sparse index array).
  size_t alloc_size = _table_size * sizeof(TableEntry) + _table_size * sparsity * sizeof(int);
  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
  int *index_array = get_index_array();
  memset(index_array, -1, _table_size * sizeof(int) * sparsity);

  // Now copy the entries from the old table into the new table.  We don't
  // have to reorder these, fortunately.  Hopefully, a smart compiler will
  // optimize this to a memcpy.
  for (size_t i = 0; i < _num_entries; ++i) {
    new(&_table[i]) TableEntry(std::move(old_table[i]));
    old_table[i].~TableEntry();
  }

  // We don't need this old thing anymore.
  old_chain->deallocate(old_table, TypeHandle::none());

  // Reindex the table.
  for (size_t i = 0; i < _num_entries; ++i) {
    size_t slot = get_hash(_table[i]._key);

    while (has_slot(slot)) {
      // Hash conflict;  look for a better spot.  This has to succeed.
      slot = next_hash(slot);
    }
    index_array[slot] = (int)i;
  }

  nassertv(validate());
}
