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

#include <set>

#ifdef NO_STYLE_ALLOCATOR
// If we're not using custom allocators, just use the standard class
// definition.
#define pset set
#define pmultiset multiset
#else

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

#endif  // NO_STYLE_ALLOCATOR
#endif
