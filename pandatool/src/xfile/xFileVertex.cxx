// Filename: xFileVertex.cxx
// Created by:  drose (19Jun01)
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

#include "xFileVertex.h"
#include "eggVertex.h"
#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileVertex::
XFileVertex(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  _has_color = true;
  _has_uv = true;
  _point = LCAST(float, egg_vertex->get_pos3());

  if (egg_vertex->has_uv()) {
    _uv = LCAST(float, egg_vertex->get_uv());
  } else {
    _uv.set(0.0, 0.0);
    _has_uv = false;
  }

  if (egg_vertex->has_color()) {
    _color = egg_vertex->get_color();
  } else if (egg_prim->has_color()) {
    _color = egg_prim->get_color();
  } else {
    _color.set(1.0, 1.0, 1.0, 1.0);
    _has_color = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int XFileVertex::
compare_to(const XFileVertex &other) const {
  int ct;
  ct = _point.compare_to(other._point);
  if (ct == 0) {
    ct = _uv.compare_to(other._uv);
  }
  if (ct == 0) {
    ct = _color.compare_to(other._color);
  }
  return ct;
}
