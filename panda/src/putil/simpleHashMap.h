/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleHashMap.h
 * @author drose
 * @date 2007-07-19
 */

#ifndef SIMPLEHASHMAP_H
#define SIMPLEHASHMAP_H

#include "pandabase.h"
#include "pvector.h"
#include "config_util.h"

/**
 * This template class implements an unordered map of keys to data,
 * implemented as a hashtable.  It is similar to STL's hash_map, but (a) it
 * has a simpler interface (we don't mess around with iterators), (b) it wants
 * an additional method on the Compare object, Compare::is_equal(a, b), and
 * (c) it doesn't depend on the system STL providing hash_map.
 */
template<class Key, class Value, class Compare = method_hash<Key, less<Key> > >
class SimpleHashMap {
public:
#ifndef CPPPARSER
  INLINE SimpleHashMap(const Compare &comp = Compare());
  INLINE ~SimpleHashMap();

  INLINE void swap(SimpleHashMap &other);

  int find(const Key &key) const;
  int store(const Key &key, const Value &data);
  INLINE bool remove(const Key &key);
  void clear();

  INLINE Value &operator [] (const Key &key);

  INLINE size_t get_size() const;
  INLINE bool has_element(size_t n) const;
  INLINE const Key &get_key(size_t n) const;
  INLINE const Value &get_data(size_t n) const;
  INLINE Value &modify_data(size_t n);
  INLINE void set_data(size_t n, const Value &data);
  void remove_element(size_t n);

  INLINE size_t get_num_entries() const;
  INLINE bool is_empty() const;

  void output(ostream &out) const;
  void write(ostream &out) const;
  bool validate() const;

  INLINE bool consider_shrink_table();

private:
  class TableEntry;

  INLINE size_t get_hash(const Key &key) const;

  INLINE bool is_element(size_t n, const Key &key) const;
  INLINE void store_new_element(size_t n, const Key &key, const Value &data);
  INLINE void clear_element(size_t n);
  INLINE unsigned char *get_exists_array() const;

  void new_table();
  INLINE bool consider_expand_table();
  void resize_table(size_t new_size);

  class TableEntry {
  public:
    INLINE TableEntry(const Key &key, const Value &data) :
      _key(key),
      _data(data) {}
    INLINE TableEntry(const TableEntry &copy) :
      _key(copy._key),
      _data(copy._data) {}
#ifdef USE_MOVE_SEMANTICS
    INLINE TableEntry(TableEntry &&from) NOEXCEPT :
      _key(move(from._key)),
      _data(move(from._data)) {}
#endif
    Key _key;
    Value _data;
  };

  TableEntry *_table;
  DeletedBufferChain *_deleted_chain;
  size_t _table_size;
  size_t _num_entries;

  Compare _comp;
#endif  // CPPPARSER
};

template<class Key, class Value, class Compare>
inline ostream &operator << (ostream &out, const SimpleHashMap<Key, Value, Compare> &shm) {
  shm.output(out);
  return out;
}

#ifndef CPPPARSER
#include "simpleHashMap.I"
#endif  // CPPPARSER

#endif
