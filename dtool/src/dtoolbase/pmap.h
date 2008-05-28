// Filename: pmap.h
// Created by:  drose (05Jun01)
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

#ifndef USE_STL_ALLOCATOR
// If we're not using custom allocators, just use the standard class
// definition.
#define pmap map
#define pmultimap multimap

#ifdef HAVE_STL_HASH
#define phash_map stdext::hash_map
#define phash_multimap stdext::hash_multimap
#else  // HAVE_STL_HASH
#define phash_map map
#define phash_multimap multimap
#endif  // HAVE_STL_HASH

#else  // USE_STL_ALLOCATOR

////////////////////////////////////////////////////////////////////
//       Class : pmap
// Description : This is our own Panda specialization on the default
//               STL map.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare = less<Key> >
class pmap : public map<Key, Value, Compare, pallocator_single<pair<const Key, Value> > > {
public:
  typedef pallocator_single<pair<const Key, Value> > allocator;
  typedef map<Key, Value, Compare, allocator> base_class;

  pmap(TypeHandle type_handle = pmap_type_handle) : base_class(Compare(), allocator(type_handle)) { }
  pmap(const pmap<Key, Value, Compare> &copy) : base_class(copy) { }
  pmap(const Compare &comp, TypeHandle type_handle = pmap_type_handle) : base_class(comp, allocator(type_handle)) { }

#ifdef USE_TAU
  TYPENAME base_class::mapped_type &
  operator [] (const TYPENAME base_class::key_type &k) {
    TAU_PROFILE("pmap::operator [] (const key_type &)", " ", TAU_USER);
    return base_class::operator [] (k);
  }

  std::pair<TYPENAME base_class::iterator, bool>
  insert(const TYPENAME base_class::value_type &x) { 
    TAU_PROFILE("pmap::insert(const value_type &)", " ", TAU_USER);
    return base_class::insert(x); 
  }

  TYPENAME base_class::iterator
  insert(TYPENAME base_class::iterator position, 
         const TYPENAME base_class::value_type &x) {
    TAU_PROFILE("pmap::insert(iterator, const value_type &)", " ", TAU_USER);
    return base_class::insert(position, x);
  }

  void
  erase(TYPENAME base_class::iterator position) {
    TAU_PROFILE("pmap::erase(iterator)", " ", TAU_USER);
    base_class::erase(position);
  }

  TYPENAME base_class::size_type
  erase(const TYPENAME base_class::key_type &x) {
    TAU_PROFILE("pmap::erase(const key_type &)", " ", TAU_USER);
    return base_class::erase(x);
  }

  void
  clear() {
    TAU_PROFILE("pmap::clear()", " ", TAU_USER);
    base_class::clear();
  }

  TYPENAME base_class::iterator
  find(const TYPENAME base_class::key_type &x) {
    TAU_PROFILE("pmap::find(const key_type &)", " ", TAU_USER);
    return base_class::find(x);
  }

  TYPENAME base_class::const_iterator
  find(const TYPENAME base_class::key_type &x) const {
    TAU_PROFILE("pmap::find(const key_type &)", " ", TAU_USER);
    return base_class::find(x);
  }

#endif  // USE_TAU
};

////////////////////////////////////////////////////////////////////
//       Class : pmultimap
// Description : This is our own Panda specialization on the default
//               STL multimap.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare = less<Key> >
class pmultimap : public multimap<Key, Value, Compare, pallocator_single<pair<const Key, Value> > > {
public:
  typedef pallocator_single<pair<const Key, Value> > allocator;
  pmultimap(TypeHandle type_handle = pmap_type_handle) : multimap<Key, Value, Compare, allocator>(Compare(), allocator(type_handle)) { }
  pmultimap(const pmultimap<Key, Value, Compare> &copy) : multimap<Key, Value, Compare, allocator>(copy) { }
  pmultimap(const Compare &comp, TypeHandle type_handle = pmap_type_handle) : multimap<Key, Value, Compare, allocator>(comp, allocator(type_handle)) { }
};

#ifdef HAVE_STL_HASH
////////////////////////////////////////////////////////////////////
//       Class : phash_map
// Description : This is our own Panda specialization on the default
//               STL hash_map.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare = method_hash<Key, less<Key> > >
class phash_map : public stdext::hash_map<Key, Value, Compare, pallocator_array<pair<const Key, Value> > > {
public:
  phash_map() : stdext::hash_map<Key, Value, Compare, pallocator_array<pair<const Key, Value> > >() { }
  phash_map(const phash_map<Key, Value, Compare> &copy) : stdext::hash_map<Key, Value, Compare, pallocator_array<pair<const Key, Value> > >(copy) { }
  phash_map(const Compare &comp) : stdext::hash_map<Key, Value, Compare, pallocator_array<pair<const Key, Value> > >(comp) { }
};

////////////////////////////////////////////////////////////////////
//       Class : phash_multimap
// Description : This is our own Panda specialization on the default
//               STL hash_multimap.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare = method_hash<Key, less<Key> > >
class phash_multimap : public stdext::hash_multimap<Key, Value, Compare, pallocator_array<pair<const Key, Value> > > {
public:
  phash_multimap() : stdext::hash_multimap<Key, Value, Compare, pallocator_array<pair<const Key, Value> > >() { }
  phash_multimap(const phash_multimap<Key, Value, Compare> &copy) : stdext::hash_multimap<Key, Value, Compare, pallocator_array<pair<const Key, Value> > >(copy) { }
  phash_multimap(const Compare &comp) : stdext::hash_multimap<Key, Value, Compare, pallocator_array<pair<const Key, Value> > >(comp) { }
};

#else // HAVE_STL_HASH
#define phash_map pmap
#define phash_multimap pmultimap
#endif  // HAVE_STL_HASH

#endif  // USE_STL_ALLOCATOR
#endif
