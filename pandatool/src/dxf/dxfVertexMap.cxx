// Filename: dxfVertexMap.cxx
// Created by:  drose (04May04)
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

#include "dxfVertexMap.h"

////////////////////////////////////////////////////////////////////
//     Function: DXFVertexMap::get_vertex_index
//       Access: Public
//  Description: Looks up the vertex in the map, and returns an index
//               unique to that vertex.  If the vertex has been used
//               before, returns the index used previously; otherwise,
//               assigns a new, unique index to the vertex and returns
//               that.
////////////////////////////////////////////////////////////////////
int DXFVertexMap::
get_vertex_index(const DXFVertex &vert) {
  iterator vmi;
  vmi = find(vert);
  if (vmi != end()) {
    // The vertex was already here.
    return (*vmi).second;
  }

  // Nope, need a new vertex.
  int index = size();
  (*this)[vert] = index;

  // That should have added one to the map.
  nassertr((int)size() == index+1, index);

  return index;
}

