/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pset.h
 * @author drose
 * @date 2001-06-05
 */

#ifndef PSET_H
#define PSET_H

#include "dtoolbase.h"
#include "pallocator.h"
#include "stl_compares.h"
#include "register_type.h"

#include <set>
#ifdef HAVE_STL_HASH
#include <hash_set>
#endif

#include <initializer_list>

#if !defined(USE_STL_ALLOCATOR) || defined(CPPPARSER)
// If we're not using custom allocators, just use the standard class
// definition.
#define pset std::set
#define pmultiset std::multiset

#ifdef HAVE_STL_HASH
#define phash_set stdext::hash_set
#define phash_multiset stdext::hash_multiset
#else  // HAVE_STL_HASH
#define phash_set std::set
#define phash_multiset std::multiset
#endif  // HAVE_STL_HASH

#else  // USE_STL_ALLOCATOR

/**
 * This is our own Panda specialization on the default STL set.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Compare = std::less<Key> >
class pset : public std::set<Key, Compare, pallocator_single<Key> > {
public:
  typedef pallocator_single<Key> allocator;
  typedef std::set<Key, Compare, allocator> base_class;
  pset(TypeHandle type_handle = pset_type_handle) : base_class(Compare(), allocator(type_handle)) { }
  pset(const Compare &comp, TypeHandle type_handle = pset_type_handle) : base_class(comp, type_handle) { }
  pset(std::initializer_list<Key> init, TypeHandle type_handle = pset_type_handle) : base_class(std::move(init), allocator(type_handle)) { }

#ifdef USE_TAU
  std::pair<typename base_class::iterator, bool>
  insert(const typename base_class::value_type &x) {
    TAU_PROFILE("pset::insert(const value_type &)", " ", TAU_USER);
    return base_class::insert(x);
  }

  typename base_class::iterator
  insert(typename base_class::iterator position,
         const typename base_class::value_type &x) {
    TAU_PROFILE("pset::insert(iterator, const value_type &)", " ", TAU_USER);
    return base_class::insert(position, x);
  }

  void
  erase(typename base_class::iterator position) {
    TAU_PROFILE("pset::erase(iterator)", " ", TAU_USER);
    base_class::erase(position);
  }

  typename base_class::size_type
  erase(const typename base_class::key_type &x) {
    TAU_PROFILE("pset::erase(const key_type &)", " ", TAU_USER);
    return base_class::erase(x);
  }

  void
  clear() {
    TAU_PROFILE("pset::clear()", " ", TAU_USER);
    base_class::clear();
  }

  typename base_class::iterator
  find(const typename base_class::key_type &x) {
    TAU_PROFILE("pset::find(x)", " ", TAU_USER);
    return base_class::find(x);
  }

  typename base_class::const_iterator
  find(const typename base_class::key_type &x) const {
    TAU_PROFILE("pset::find(x)", " ", TAU_USER);
    return base_class::find(x);
  }
#endif  // USE_TAU
};

/**
 * This is our own Panda specialization on the default STL multiset.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Compare = std::less<Key> >
class pmultiset : public std::multiset<Key, Compare, pallocator_single<Key> > {
public:
  typedef pallocator_single<Key> allocator;
  pmultiset(TypeHandle type_handle = pset_type_handle) : std::multiset<Key, Compare, allocator>(Compare(), allocator(type_handle)) { }
  pmultiset(const Compare &comp, TypeHandle type_handle = pset_type_handle) : std::multiset<Key, Compare, allocator>(comp, type_handle) { }
  pmultiset(std::initializer_list<Key> init, TypeHandle type_handle = pset_type_handle) : std::multiset<Key, Compare, allocator>(std::move(init), allocator(type_handle)) { }
};

#ifdef HAVE_STL_HASH
/**
 * This is our own Panda specialization on the default STL hash_set.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Compare = method_hash<Key, std::less<Key> > >
class phash_set : public stdext::hash_set<Key, Compare, pallocator_array<Key> > {
public:
  phash_set() : stdext::hash_set<Key, Compare, pallocator_array<Key> >() { }
  phash_set(const Compare &comp) : stdext::hash_set<Key, Compare, pallocator_array<Key> >(comp) { }
};

/**
 * This is our own Panda specialization on the default STL hash_multiset.  Its
 * main purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Key, class Compare = method_hash<Key, std::less<Key> > >
class phash_multiset : public stdext::hash_multiset<Key, Compare, pallocator_array<Key> > {
public:
  phash_multiset() : stdext::hash_multiset<Key, Compare, pallocator_array<Key> >() { }
  phash_multiset(const Compare &comp) : stdext::hash_multiset<Key, Compare, pallocator_array<Key> >(comp) { }
};

#else // HAVE_STL_HASH
#define phash_set pset
#define phash_multiset pmultiset
#endif  // HAVE_STL_HASH

#endif  // USE_STL_ALLOCATOR
#endif
