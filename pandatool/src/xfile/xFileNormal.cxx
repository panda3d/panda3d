// Filename: xFileNormal.cxx
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

#include "xFileNormal.h"
#include "eggVertex.h"
#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileNormal::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileNormal::
XFileNormal(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  _has_normal = true;
  if (egg_vertex->has_normal()) {
    _normal = LCAST(float, egg_vertex->get_normal());
  } else if (egg_prim->has_normal()) {
    _normal = LCAST(float, egg_prim->get_normal());
  } else {
    _normal.set(0.0, 0.0, 0.0);
    _has_normal = false;
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
