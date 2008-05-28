// Filename: indexRemapper.cxx
// Created by:  drose (05Aug00)
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

#include "indexRemapper.h"


////////////////////////////////////////////////////////////////////
//     Function: IndexRemapper::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IndexRemapper::
IndexRemapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: IndexRemapper::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IndexRemapper::
~IndexRemapper() {
}

////////////////////////////////////////////////////////////////////
//     Function: IndexRemapper::clear
//       Access: Public
//  Description: Removes all mappings from the object.
////////////////////////////////////////////////////////////////////
void IndexRemapper::
clear() {
  _map_int.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: IndexRemapper::add_mapping
//       Access: Public
//  Description: Adds a mapping from the integer 'from' to 'to'.
////////////////////////////////////////////////////////////////////
void IndexRemapper::
add_mapping(int from, int to) {
  _map_int[from] = to;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexRemapper::in_map
//       Access: Public
//  Description: Returns true if the given 'from' integer has been
//               assigned a mapping, false if it has not.
////////////////////////////////////////////////////////////////////
bool IndexRemapper::
in_map(int from) const {
  return _map_int.count(from) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexRemapper::map_from
//       Access: Public
//  Description: Returns the integer that the given 'from' integer had
//               been set to map to, or the same integer if nothing
//               had been set for it.
////////////////////////////////////////////////////////////////////
int IndexRemapper::
map_from(int from) const {
  map<int, int>::const_iterator mi;
  mi = _map_int.find(from);
  if (mi == _map_int.end()) {
    return from;
  }
  return (*mi).second;
}
