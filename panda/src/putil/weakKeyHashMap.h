// Filename: weakKeyHashMap.h
// Created by:  rdb (13Jul15)
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

#ifndef WEAKKEYHASHMAP_H
#define WEAKKEYHASHMAP_H

#include "pandabase.h"
#include "pvector.h"
#include "config_util.h"
#include "weakPointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : WeakKeyHashMap
// Description : This is a variation on WeakKeyHashMap that stores
//               weak pointers as keys, and automatically frees up
//               entries from the map when the associated key has
//               been deleted.
//
//               This is more efficient than using a naive map of
//               WeakPointerTo keys since that would incur the cost
//               of constructing a weak reference every time a find
//               operation is used.
////////////////////////////////////////////////////////////////////
template<class Key, class Value>
class WeakKeyHashMap {
public:
#ifndef CPPPARSER
  INLINE WeakKeyHashMap();
  INLINE ~WeakKeyHashMap();

  INLINE void swap(WeakKeyHashMap &other);

  int find(const Key *key) const;
  int store(const Key *key, const Value &data);
  INLINE bool remove(const Key *key);
  void clear();

  INLINE Value &operator [] (const Key *key);

  INLINE int get_size() const;
  INLINE bool has_element(int n) const;
  INLINE const Key *get_key(int n) const;
  INLINE const Value &get_data(int n) const;
  INLINE Value &modify_data(int n);
  INLINE void set_data(int n, const Value &data);
#ifdef USE_MOVE_SEMANTICS
  INLINE void set_data(int n, Value &&data);
#endif
  void remove_element(int n);

  INLINE int get_num_entries() const;
  INLINE bool is_empty() const;

  void output(ostream &out) const;
  void write(ostream &out) const;
  bool validate() const;

private:
  INLINE size_t get_hash(const Key *key) const;

  INLINE bool is_element(int n, const Key *key) const;
  INLINE void store_new_element(int n, const Key *key, const Value &data);
  INLINE void clear_element(int n);
  INLINE unsigned char *get_exists_array() const;

  void new_table();
  INLINE bool consider_expand_table();
  void expand_table();

  class TableEntry {
  public:
    INLINE TableEntry(const Key *key, const Value &data) :
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
    WCPT(Key) _key;
    Value _data;
  };

  TableEntry *_table;
  DeletedBufferChain *_deleted_chain;
  size_t _table_size;
  size_t _num_entries;
#endif  // CPPPARSER
};

template<class Key, class Value>
inline ostream &operator << (ostream &out, const WeakKeyHashMap<Key, Value> &shm) {
  shm.output(out);
  return out;
}

#ifndef CPPPARSER
#include "weakKeyHashMap.I"
#endif  // CPPPARSER

#endif
