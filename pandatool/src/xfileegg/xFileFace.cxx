// Filename: xFileFace.cxx
// Created by:  drose (19Jun01)
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

#include "xFileFace.h"
#include "xFileMesh.h"
#include "eggPolygon.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileFace::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileFace::
XFileFace() {
  _material_index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileFace::set_from_egg
//       Access: Public
//  Description: Sets the structure up from the indicated egg data.
////////////////////////////////////////////////////////////////////
void XFileFace::
set_from_egg(XFileMesh *mesh, EggPolygon *egg_poly) {
  // Walk through the polygon's vertices in reverse order, to change
  // from Egg's counter-clockwise convention to DX's clockwise.
  EggPolygon::reverse_iterator vi;
  for (vi = egg_poly->rbegin(); vi != egg_poly->rend(); ++vi) {
    EggVertex *egg_vertex = (*vi);
    Vertex v;
    v._vertex_index = mesh->add_vertex(egg_vertex, egg_poly);
    v._normal_index = mesh->add_normal(egg_vertex, egg_poly);
    _vertices.push_back(v);
  }

  _material_index = mesh->add_material(egg_poly);
}
