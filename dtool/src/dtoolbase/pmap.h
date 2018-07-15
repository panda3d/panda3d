/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmap.h
 * @author drose
 * @date 2001-06-05
 */

#ifndef PMAP_H
#define PMAP_H

#include "dtoolbase.h"
#include "pallocator.h"
#include "stl_compares.h"
#include "register_type.h"

#include <map>
#ifdef HAVE_STL_HASH
#include <hash_map>
#endif

#if !defined(USE_STL_ALLOCATOR) || defined(CPPPARSER)
// If we're not using custom allocators, just use the standard class
// definition.
#define pmap std::map
#define pmultimap std::multimap

#ifdef HAVE_STL_HASH
#define phash_map stdext::hash_map
#define phash_multimap stdext::hash_multimap
#else  // HAVE_STL_HASH
#define phash_map map
#define phash_multimap multimap
#endif  // HAVE_STL_HASH

#else  // USE_STL_ALLOCATOR

/**
 * This is our own Panda specialization on the default STL map.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Value, class Compare = std::less<Key> >
class pmap : public std::map<Key, Value, Compare, pallocator_single<std::pair<const Key, Value> > > {
public:
  typedef pallocator_single<std::pair<const Key, Value> > allocator;
  typedef std::map<Key, Value, Compare, allocator> base_class;

  pmap(TypeHandle type_handle = pmap_type_handle) : base_class(Compare(), allocator(type_handle)) { }
  pmap(const Compare &comp, TypeHandle type_handle = pmap_type_handle) : base_class(comp, allocator(type_handle)) { }

#ifdef USE_TAU
  typename base_class::mapped_type &
  operator [] (const typename base_class::key_type &k) {
    TAU_PROFILE("pmap::operator [] (const key_type &)", " ", TAU_USER);
    return base_class::operator [] (k);
  }

  std::pair<typename base_class::iterator, bool>
  insert(const typename base_class::value_type &x) {
    TAU_PROFILE("pmap::insert(const value_type &)", " ", TAU_USER);
    return base_class::insert(x);
  }

  typename base_class::iterator
  insert(typename base_class::iterator position,
         const typename base_class::value_type &x) {
    TAU_PROFILE("pmap::insert(iterator, const value_type &)", " ", TAU_USER);
    return base_class::insert(position, x);
  }

  void
  erase(typename base_class::iterator position) {
    TAU_PROFILE("pmap::erase(iterator)", " ", TAU_USER);
    base_class::erase(position);
  }

  typename base_class::size_type
  erase(const typename base_class::key_type &x) {
    TAU_PROFILE("pmap::erase(const key_type &)", " ", TAU_USER);
    return base_class::erase(x);
  }

  void
  clear() {
    TAU_PROFILE("pmap::clear()", " ", TAU_USER);
    base_class::clear();
  }

  typename base_class::iterator
  find(const typename base_class::key_type &x) {
    TAU_PROFILE("pmap::find(const key_type &)", " ", TAU_USER);
    return base_class::find(x);
  }

  typename base_class::const_iterator
  find(const typename base_class::key_type &x) const {
    TAU_PROFILE("pmap::find(const key_type &)", " ", TAU_USER);
    return base_class::find(x);
  }

#endif  // USE_TAU
};

/**
 * This is our own Panda specialization on the default STL multimap.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Value, class Compare = std::less<Key> >
class pmultimap : public std::multimap<Key, Value, Compare, pallocator_single<std::pair<const Key, Value> > > {
public:
  typedef pallocator_single<std::pair<const Key, Value> > allocator;
  pmultimap(TypeHandle type_handle = pmap_type_handle) : std::multimap<Key, Value, Compare, allocator>(Compare(), allocator(type_handle)) { }
  pmultimap(const Compare &comp, TypeHandle type_handle = pmap_type_handle) : std::multimap<Key, Value, Compare, allocator>(comp, allocator(type_handle)) { }
};

#ifdef HAVE_STL_HASH
/**
 * This is our own Panda specialization on the default STL hash_map.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Value, class Compare = method_hash<Key, std::less<Key> > >
class phash_map : public stdext::hash_map<Key, Value, Compare, pallocator_array<std::pair<const Key, Value> > > {
public:
  phash_map() : stdext::hash_map<Key, Value, Compare, pallocator_array<std::pair<const Key, Value> > >() { }
  phash_map(const Compare &comp) : stdext::hash_map<Key, Value, Compare, pallocator_array<std::pair<const Key, Value> > >(comp) { }
};

/**
 * This is our own Panda specialization on the default STL hash_multimap.  Its
 * main purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Value, class Compare = method_hash<Key, std::less<Key> > >
class phash_multimap : public stdext::hash_multimap<Key, Value, Compare, pallocator_array<std::pair<const Key, Value> > > {
public:
  phash_multimap() : stdext::hash_multimap<Key, Value, Compare, pallocator_array<std::pair<const Key, Value> > >() { }
  phash_multimap(const Compare &comp) : stdext::hash_multimap<Key, Value, Compare, pallocator_array<std::pair<const Key, Value> > >(comp) { }
};

#else // HAVE_STL_HASH
#define phash_map pmap
#define phash_multimap pmultimap
#endif  // HAVE_STL_HASH

#endif  // USE_STL_ALLOCATOR
#endif
