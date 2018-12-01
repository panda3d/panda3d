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
#include "config_putil.h"

/**
 * Entry in the SimpleHashMap.
 */
template<class Key, class Value>
class SimpleKeyValuePair {
public:
  INLINE SimpleKeyValuePair(const Key &key, const Value &data) :
    _key(key),
    _data(data) {}

  Key _key;

  ALWAYS_INLINE const Value &get_data() const {
    return _data;
  }
  ALWAYS_INLINE Value &modify_data() {
    return _data;
  }
  ALWAYS_INLINE void set_data(const Value &data) {
    _data = data;
  }
  ALWAYS_INLINE void set_data(Value &&data) {
    _data = std::move(data);
  }

private:
  Value _data;
};

/**
 * Specialisation of SimpleKeyValuePair to not waste memory for nullptr_t
 * values.  This allows effectively using SimpleHashMap as a set.
 */
template<class Key>
class SimpleKeyValuePair<Key, std::nullptr_t> {
public:
  INLINE SimpleKeyValuePair(const Key &key, std::nullptr_t data) :
    _key(key) {}

  Key _key;

  ALWAYS_INLINE constexpr static std::nullptr_t get_data() { return nullptr; }
  ALWAYS_INLINE constexpr static std::nullptr_t modify_data() { return nullptr; }
  ALWAYS_INLINE static void set_data(std::nullptr_t) {}
};

/**
 * This template class implements an unordered map of keys to data,
 * implemented as a hashtable.  It is similar to STL's hash_map, but
 * (a) it has a simpler interface (we don't mess around with iterators),
 * (b) it wants an additional method on the Compare object,
       Compare::is_equal(a, b),
 * (c) it doesn't depend on the system STL providing hash_map,
 * (d) it allows for efficient iteration over the entries,
 * (e) permits removal and resizing during forward iteration, and
 * (f) it has a constexpr constructor.
 *
 * It can also be used as a set, by using nullptr_t as Value typename.
 */
template<class Key, class Value, class Compare = method_hash<Key, std::less<Key> > >
class SimpleHashMap {
  // Per-entry overhead is determined by sizeof(int) * sparsity.  Should be a
  // power of two.
  static const unsigned int sparsity = 2u;

public:
#ifndef CPPPARSER
  constexpr SimpleHashMap(const Compare &comp = Compare());
  INLINE SimpleHashMap(const SimpleHashMap &copy);
  INLINE SimpleHashMap(SimpleHashMap &&from) noexcept;
  INLINE ~SimpleHashMap();

  INLINE SimpleHashMap &operator = (const SimpleHashMap &copy);
  INLINE SimpleHashMap &operator = (SimpleHashMap &&from) noexcept;

  INLINE void swap(SimpleHashMap &other);

  int find(const Key &key) const;
  int store(const Key &key, const Value &data);
  INLINE bool remove(const Key &key);
  void clear();

  INLINE Value &operator [] (const Key &key);
  constexpr size_t size() const;

  INLINE const Key &get_key(size_t n) const;
  INLINE const Value &get_data(size_t n) const;
  INLINE Value &modify_data(size_t n);
  INLINE void set_data(size_t n, const Value &data);
  INLINE void set_data(size_t n, Value &&data);
  void remove_element(size_t n);

  INLINE size_t get_num_entries() const;
  INLINE bool is_empty() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;
  bool validate() const;

  INLINE bool consider_shrink_table();

private:
  INLINE size_t get_hash(const Key &key) const;
  INLINE size_t next_hash(size_t hash) const;

  INLINE int find_slot(const Key &key) const;
  INLINE bool has_slot(size_t slot) const;
  INLINE bool is_element(size_t n, const Key &key) const;
  INLINE size_t store_new_element(size_t n, const Key &key, const Value &data);
  INLINE int *get_index_array() const;

  void new_table();
  INLINE bool consider_expand_table();
  void resize_table(size_t new_size);

public:
  typedef SimpleKeyValuePair<Key, Value> TableEntry;
  TableEntry *_table;
  DeletedBufferChain *_deleted_chain;
  size_t _table_size;
  size_t _num_entries;

  Compare _comp;
#endif  // CPPPARSER
};

template<class Key, class Value, class Compare>
inline std::ostream &operator << (std::ostream &out, const SimpleHashMap<Key, Value, Compare> &shm) {
  shm.output(out);
  return out;
}

#ifndef CPPPARSER
#include "simpleHashMap.I"
#endif  // CPPPARSER

#endif
