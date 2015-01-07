// Filename: simpleHashMap.I
// Created by:  drose (19Jul07)
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
//     Function: SimpleHashMap::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare>::
SimpleHashMap(const Compare &comp) :
  _table(NULL),
  _deleted_chain(NULL),
  _table_size(0),
  _num_entries(0),
  _comp(comp)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE SimpleHashMap<Key, Value, Compare>::
~SimpleHashMap() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::swap
//       Access: Public
//  Description: Quickly exchanges the contents of this map and the
//               other map.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::find
//       Access: Public
//  Description: Searches for the indicated key in the table.  Returns
//               its index number if it is found, or -1 if it is not
//               present in the table.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
int SimpleHashMap<Key, Value, Compare>::
find(const Key &key) const {
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

  // There was some other key at the hashed slot.  That's a hash
  // conflict.  Maybe our entry was recorded at a later slot position;
  // scan the subsequent positions until we find the entry or an
  // unused slot, indicating the end of the scan.
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

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::store
//       Access: Public
//  Description: Records the indicated key/data pair in the map.  If
//               the key was already present, silently replaces it.
//               Returns the index at which it was stored.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
int SimpleHashMap<Key, Value, Compare>::
store(const Key &key, const Value &data) {
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
    // This element is already in the map; replace the data at that
    // key.
    _table[index]._data = data;
#ifdef _DEBUG
    nassertr(validate(), index);
#endif
    return index;
  }

  // There was some other key at the hashed slot.  That's a hash
  // conflict.  Record this entry at a later position.
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

  // Shouldn't get here unless _num_entries == _table_size, which
  // shouldn't be possible due to consider_expand_table().
  nassertr(false, -1);
  return -1;  // To satisfy compiler
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::remove
//       Access: Public
//  Description: Removes the indicated key and its associated data
//               from the table.  Returns true if the key was removed,
//               false if it was not present.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
remove(const Key &key) {
  int index = find(key);
  if (index == -1) {
    return false;
  }
  remove_element(index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::clear
//       Access: Public
//  Description: Completely empties the table.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
clear() {
  if (_table_size != 0) {
    for (size_t i = 0; i < _table_size; ++i) {
      if (has_element(i)) {
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

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::operator []
//       Access: Public
//  Description: Returns a modifiable reference to the data associated
//               with the indicated key, or creates a new data entry
//               and returns its reference.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE Value &SimpleHashMap<Key, Value, Compare>::
operator [] (const Key &key) {
  int index = find(key);
  if (index == -1) {
    index = store(key, Value());
  }
  return modify_data(index);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::get_size
//       Access: Public
//  Description: Returns the total number of slots in the table.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE int SimpleHashMap<Key, Value, Compare>::
get_size() const {
  return _table_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::has_element
//       Access: Public
//  Description: Returns true if there is an element stored in the nth
//               slot, false otherwise.
//
//               n should be in the range 0 <= n < get_size().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
has_element(int n) const {
  nassertr(n >= 0 && n < (int)_table_size, false);
  return (get_exists_array()[n] != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::get_key
//       Access: Public
//  Description: Returns the key in the nth slot of the table.
//
//               It is an error to call this if there is nothing
//               stored in the nth slot (use has_element() to check
//               this first).  n should be in the range 0 <= n <
//               get_size().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE const Key &SimpleHashMap<Key, Value, Compare>::
get_key(int n) const {
  nassertr(has_element(n), _table[n]._key);
  return _table[n]._key;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::get_data
//       Access: Public
//  Description: Returns the data in the nth slot of the table.
//
//               It is an error to call this if there is nothing
//               stored in the nth slot (use has_element() to check
//               this first).  n should be in the range 0 <= n <
//               get_size().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE const Value &SimpleHashMap<Key, Value, Compare>::
get_data(int n) const {
  nassertr(has_element(n), _table[n]._data);
  return _table[n]._data;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::modify_data
//       Access: Public
//  Description: Returns a modifiable reference to the data in the nth
//               slot of the table.
//
//               It is an error to call this if there is nothing
//               stored in the nth slot (use has_element() to check
//               this first).  n should be in the range 0 <= n <
//               get_size().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE Value &SimpleHashMap<Key, Value, Compare>::
modify_data(int n) {
  nassertr(has_element(n), _table[n]._data);
  return _table[n]._data;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::set_data
//       Access: Public
//  Description: Changes the data for the nth slot of the table.
//
//               It is an error to call this if there is nothing
//               stored in the nth slot (use has_element() to check
//               this first).  n should be in the range 0 <= n <
//               get_size().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE void SimpleHashMap<Key, Value, Compare>::
set_data(int n, const Value &data) {
  nassertv(has_element(n));
  _table[n]._data = data;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::remove_element
//       Access: Public
//  Description: Removes the nth slot from the table.
//
//               It is an error to call this if there is nothing
//               stored in the nth slot (use has_element() to check
//               this first).  n should be in the range 0 <= n <
//               get_size().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
remove_element(int n) {
  nassertv(has_element(n));

  clear_element(n);
  nassertv(_num_entries > 0);
  --_num_entries;

  // Now we have put a hole in the table.  If there was a hash
  // conflict in the slot following this one, we have to move it down
  // to close the hole.
  size_t i = n;
  i = (i + 1) & (_table_size - 1);
  while (has_element(i)) {
    size_t wants_index = get_hash(_table[i]._key);
    if (wants_index != i) {
      // This one was a hash conflict; try to put it where it belongs.
      // We can't just put it in n, since maybe it belongs somewhere
      // after n.
      while (wants_index != i && has_element(wants_index)) {
        wants_index = (wants_index + 1) & (_table_size - 1);
      }
      if (wants_index != i) {
        store_new_element(wants_index, _table[i]._key, _table[i]._data);
        clear_element(i);
      }
    }

    // Continue until we encounter the next unused slot.  Until we do,
    // we can't be sure we've found all of the potential hash
    // conflicts.
    i = (i + 1) & (_table_size - 1);
  }

#ifdef _DEBUG
  nassertv(validate());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::get_num_entries
//       Access: Public
//  Description: Returns the number of active entries in the table.
//               This is not necessarily related to the number of
//               slots in the table as reported by get_size().  Use
//               get_size() to iterate through all of the slots, not
//               get_num_entries().
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE int SimpleHashMap<Key, Value, Compare>::
get_num_entries() const {
  return _num_entries;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::is_empty
//       Access: Public
//  Description: Returns true if the table is empty;
//               i.e. get_num_entries() == 0.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
is_empty() const {
  return (_num_entries == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
output(ostream &out) const {
  out << "SimpleHashMap (" << _num_entries << " entries): [";
  for (size_t i = 0; i < _table_size; ++i) {
    if (!has_element(i)) {
      out << " *";

    } else {
      out << " " << _table[i]._key;
      size_t index = get_hash(_table[i]._key);
      if (index != i) {
        // This was misplaced as the result of a hash conflict.
        // Report how far off it is.
        out << "(" << ((_table_size + i - index) & (_table_size - 1)) << ")";
      }
    }
  }
  out << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
write(ostream &out) const {
  output(out);
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::validate
//       Access: Public
//  Description: Returns true if the internal table appears to be
//               consistent, false if there are some internal errors.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
bool SimpleHashMap<Key, Value, Compare>::
validate() const {
  size_t count = 0;

  for (size_t i = 0; i < _table_size; ++i) {
    if (has_element(i)) {
      ++count;
      size_t ideal_index = get_hash(_table[i]._key);
      size_t wants_index = ideal_index;
      while (wants_index != i && has_element(wants_index)) {
        wants_index = (wants_index + 1) & (_table_size - 1);
      }
      if (wants_index != i) {
        util_cat.error()
          << "SimpleHashMap is invalid: key " << _table[i]._key
          << " should be in slot " << wants_index << " instead of "
          << i << " (ideal is " << ideal_index << ")\n";
        write(util_cat.error(false));
        return false;
      }
    }
  }

  if (count != _num_entries) {
    util_cat.error()
      << "SimpleHashMap is invalid: reports " << _num_entries
      << " entries, actually has " << count << "\n";
    write(util_cat.error(false));
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::get_hash
//       Access: Private
//  Description: Computes an appropriate index number to store the
//               given pointer.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE size_t SimpleHashMap<Key, Value, Compare>::
get_hash(const Key &key) const {
  /*
  // We want a hash constant 0 < k < 1.  This one is suggested by
  // Knuth:
  static const double hash_constant = (sqrt(5.0) - 1.0) / 2.0;
  double f = ((double)_comp(key) * hash_constant);
  f -= floor(f);
  return (size_t)floor(f * _table_size);
  */

  return ((_comp(key) * (size_t)9973) >> 8) & (_table_size - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::is_element
//       Access: Private
//  Description: Returns true if element n matches key.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
is_element(int n, const Key &key) const {
  nassertr(has_element(n), false);
  return _comp.is_equal(_table[n]._key, key);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::store_new_element
//       Access: Private
//  Description: Constructs a new TableEntry at position n, storing
//               the indicated key and value.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE void SimpleHashMap<Key, Value, Compare>::
store_new_element(int n, const Key &key, const Value &data) {
  new(&_table[n]) TableEntry(key, data);
  get_exists_array()[n] = true;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::clear_element
//       Access: Private
//  Description: Destructs the TableEntry at position n.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE void SimpleHashMap<Key, Value, Compare>::
clear_element(int n) {
  _table[n].~TableEntry();
  get_exists_array()[n] = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::get_exists_array
//       Access: Private
//  Description: Returns the beginning of the array of _table_size
//               unsigned chars that are the boolean flags for whether
//               each element exists (has been constructed) within the
//               table.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE unsigned char *SimpleHashMap<Key, Value, Compare>::
get_exists_array() const {
  return (unsigned char *)(_table + _table_size);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::new_table
//       Access: Private
//  Description: Allocates a brand new table.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
new_table() {
  nassertv(_table_size == 0 && _num_entries == 0);

  // Pick a good initial table size.  For now, we make it really
  // small.  Maybe that's the right answer.
  _table_size = 4;

  // We allocate enough bytes for _table_size elements of TableEntry,
  // plus _table_size more bytes at the end (for the exists array).
  size_t alloc_size = _table_size * sizeof(TableEntry) + _table_size;

  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
  memset(get_exists_array(), 0, _table_size);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::consider_expand_table
//       Access: Private
//  Description: Expands the table if it will need it (assuming one
//               more element is about to be added).  Returns true if
//               expanded, false otherwise.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
INLINE bool SimpleHashMap<Key, Value, Compare>::
consider_expand_table() {
  if (_num_entries >= (_table_size >> 1)) {
    expand_table();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleHashMap::expand_table
//       Access: Private
//  Description: Doubles the size of the existing table.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare>
void SimpleHashMap<Key, Value, Compare>::
expand_table() {
  nassertv(_table_size != 0);

  SimpleHashMap<Key, Value, Compare> old_map(_comp);
  swap(old_map);

  // Double the table size.
  size_t old_table_size = old_map._table_size;
  _table_size = (old_table_size << 1);
  nassertv(_table == NULL);

  // We allocate enough bytes for _table_size elements of TableEntry,
  // plus _table_size more bytes at the end (for the exists array).
  size_t alloc_size = _table_size * sizeof(TableEntry) + _table_size;
  _deleted_chain = memory_hook->get_deleted_chain(alloc_size);
  _table = (TableEntry *)_deleted_chain->allocate(alloc_size, TypeHandle::none());
  unsigned char *exists_array = get_exists_array();
  memset(exists_array, 0, _table_size);
  nassertv(_num_entries == 0);

  // Now copy the entries from the old table into the new table.
  int num_added = 0;
  for (size_t i = 0; i < old_table_size; ++i) {
    if (old_map.has_element(i)) {
      size_t new_index = get_hash(old_map._table[i]._key);

      while (exists_array[new_index] != 0) {
        // Hash conflict;  look for a better spot.  This has to succeed.
        new_index = (new_index + 1) & (_table_size - 1);
      }

#ifdef USE_MOVE_SEMANTICS
      // Use C++11 rvalue references to invoke the move constructor,
      // which may be more efficient.
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

  nassertv(_num_entries == old_map._num_entries);
}
