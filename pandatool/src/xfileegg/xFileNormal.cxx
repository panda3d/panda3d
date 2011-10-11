// Filename: xFileNormal.cxx
// Created by:  drose (19Jun01)
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

#include "xFileNormal.h"
#include "eggVertex.h"
#include "eggPrimitive.h"
#include "config_xfile.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileNormal::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileNormal::
XFileNormal() {
  _normal.set(0.0, 0.0, 0.0);
  _has_normal = false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNormal::set_from_egg
//       Access: Public
//  Description: Sets the structure up from the indicated egg data.
////////////////////////////////////////////////////////////////////
void XFileNormal::
set_from_egg(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  if (egg_vertex->has_normal() || egg_prim->has_normal()) {
    LNormald norm;
    if (egg_vertex->has_normal()) {
      norm = egg_vertex->get_normal();
    } else {
      norm = egg_prim->get_normal();
    }

    if (xfile_one_mesh) {
      // If this is going into one big mesh, we must ensure every
      // vertex is in world coordinates.
      norm = norm * egg_prim->get_vertex_frame();
    } else {
      // Otherwise, we ensure the vertex is in local coordinates.
      norm = norm * egg_prim->get_vertex_to_node();
    }

    _normal = norm;
    _has_normal = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileNormal::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int XFileNormal::
compare_to(const XFileNormal &other) const {
  int ct;
  ct = _normal.compare_to(other._normal);
  return ct;
}
