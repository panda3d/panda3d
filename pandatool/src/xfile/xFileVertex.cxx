// Filename: xFileVertex.cxx
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

#include "xFileVertex.h"
#include "eggVertex.h"
#include "eggPrimitive.h"
#include "config_xfile.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileVertex::
XFileVertex() {
  _has_color = false;
  _has_uv = false;
  _point.set(0.0, 0.0, 0.0);
  _uv.set(0.0, 0.0);
  _color.set(1.0, 1.0, 1.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::set_from_egg
//       Access: Public
//  Description: Sets the structure up from the indicated egg data.
////////////////////////////////////////////////////////////////////
void XFileVertex::
set_from_egg(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  Vertexd pos = egg_vertex->get_pos3();

  if (xfile_one_mesh) {
    // If this is going into one big mesh, we must ensure every
    // vertex is in world coordinates.
    pos = pos * egg_prim->get_vertex_frame();
  } else {
    // Otherwise, we ensure the vertex is in local coordinates.
    pos = pos * egg_prim->get_vertex_to_node();
  }

  _point = LCAST(float, pos);

  if (egg_vertex->has_uv()) {
    TexCoordd uv = egg_vertex->get_uv();
    if (egg_prim->has_texture()) {
      // Check if there's a texture matrix on the texture.
      EggTexture *egg_tex = egg_prim->get_texture();
      if (egg_tex->has_transform()) {
        uv = uv * egg_tex->get_transform();
      }
    }

    _uv[0] = uv[0];
    // Windows draws the UV's upside-down.
    _uv[1] = 1.0 - uv[1];
    _has_uv = true;
  }

  if (egg_vertex->has_color()) {
    _color = egg_vertex->get_color();
    _has_color = true;
  } else if (egg_prim->has_color()) {
    _color = egg_prim->get_color();
    _has_color = true;
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
