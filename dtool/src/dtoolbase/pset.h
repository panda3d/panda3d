// Filename: pset.h
// Created by:  drose (05Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PSET_H
#define PSET_H

#include "dtoolbase.h"
#include "pallocator.h"
#include "stl_compares.h"

#include <set>
#ifdef HAVE_STL_HASH
#include <hash_set>
#endif

#ifdef NO_STYLE_ALLOCATOR
// If we're not using custom allocators, just use the standard class
// definition.
#define pset set
#define pmultiset multiset

#ifdef HAVE_STL_HASH
#define phash_set hash_set
#define phash_multiset hash_multiset
#else  // HAVE_STL_HASH
#define phash_set set
#define phash_multiset multiset
#endif  // HAVE_STL_HASH

#else  // NO_STYLE_ALLOCATOR

////////////////////////////////////////////////////////////////////
//       Class : pset
// Description : This is our own Panda specialization on the default
//               STL set.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class pset : public set<Key, Compare, pallocator<Key> > {
public:
  pset() : set<Key, Compare, pallocator<Key> >() { }
  pset(const pset<Key, Compare> &copy) : set<Key, Compare, pallocator<Key> >(copy) { }
  pset(const Compare &comp) : set<Key, Compare, pallocator<Key> >(comp) { }
};

////////////////////////////////////////////////////////////////////
//       Class : pmultiset
// Description : This is our own Panda specialization on the default
//               STL multiset.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class pmultiset : public multiset<Key, Compare, pallocator<Key> > {
public:
  pmultiset() : multiset<Key, Compare, pallocator<Key> >() { }
  pmultiset(const pmultiset<Key, Compare> &copy) : multiset<Key, Compare, pallocator<Key> >(copy) { }
  pmultiset(const Compare &comp) : multiset<Key, Compare, pallocator<Key> >(comp) { }
};

#ifdef HAVE_STL_HASH
////////////////////////////////////////////////////////////////////
//       Class : phash_set
// Description : This is our own Panda specialization on the default
//               STL hash_set.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = method_hash<Key, less<Key> > >
class phash_set : public hash_set<Key, Compare, pallocator<Key> > {
public:
  phash_set() : hash_set<Key, Compare, pallocator<Key> >() { }
  phash_set(const phash_set<Key, Compare> &copy) : hash_set<Key, Compare, pallocator<Key> >(copy) { }
  phash_set(const Compare &comp) : hash_set<Key, Compare, pallocator<Key> >(comp) { }
};

////////////////////////////////////////////////////////////////////
//       Class : phash_multiset
// Description : This is our own Panda specialization on the default
//               STL hash_multiset.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = method_hash<Key, less<Key> > >
class phash_multiset : public hash_multiset<Key, Compare, pallocator<Key> > {
public:
  phash_multiset() : hash_multiset<Key, Compare, pallocator<Key> >() { }
  phash_multiset(const phash_multiset<Key, Compare> &copy) : hash_multiset<Key, Compare, pallocator<Key> >(copy) { }
  phash_multiset(const Compare &comp) : hash_multiset<Key, Compare, pallocator<Key> >(comp) { }
};

#else // HAVE_STL_HASH
#define phash_set pset
#define phash_multiset pmultiset
#endif  // HAVE_STL_HASH

#endif  // NO_STYLE_ALLOCATOR
#endif
