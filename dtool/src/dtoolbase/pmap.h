// Filename: pmap.h
// Created by:  drose (05Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PMAP_H
#define PMAP_H

#include "dtoolbase.h"
#include "pallocator.h"

#include <map>

////////////////////////////////////////////////////////////////////
//       Class : pmap
// Description : This is our own Panda specialization on the default
//               STL map.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare = less<Key> >
class pmap : public map<Key, Value, Compare, pallocator<pair<Key, Value> > > {
public:
  pmap() : map<Key, Value, Compare, pallocator<pair<Key, Value> > >() { }
  pmap(const pmap<Key, Value, Compare> &copy) : map<Key, Value, Compare, pallocator<pair<Key, Value> > >(copy) { }
  pmap(const Compare &comp) : map<Key, Compare, pallocator<Key> >(comp) { }
};

////////////////////////////////////////////////////////////////////
//       Class : pmultimap
// Description : This is our own Panda specialization on the default
//               STL multimap.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Key, class Value, class Compare = less<Key> >
class pmultimap : public multimap<Key, Value, Compare, pallocator<pair<Key, Value> > > {
public:
  pmultimap() : multimap<Key, Value, Compare, pallocator<pair<Key, Value> > >() { }
  pmultimap(const pmultimap<Key, Value, Compare> &copy) : multimap<Key, Value, Compare, pallocator<pair<Key, Value> > >(copy) { }
  pmultimap(const Compare &comp) : multimap<Key, Compare, pallocator<Key> >(comp) { }
};

#endif

