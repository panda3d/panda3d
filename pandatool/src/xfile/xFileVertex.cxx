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

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileVertex::
XFileVertex(const Vertexf &point) : 
  _point(point),
  _normal(0.0, 0.0, 0.0),
  _uv(0.0, 0.0),
  _color(1.0, 1.0, 1.0, 1.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::set_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void XFileVertex::
set_normal(const Normalf &normal) {
  _normal = normal;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::set_uv
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void XFileVertex::
set_uv(const TexCoordf &uv) {
  _uv = uv;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::set_color
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void XFileVertex::
set_color(const Colorf &color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileVertex::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool XFileVertex::
operator < (const XFileVertex &other) const {
  int ct;
  ct = _point.compare_to(other._point);
  if (ct == 0) {
    ct = _normal.compare_to(other._normal);
  }
  if (ct == 0) {
    ct = _uv.compare_to(other._uv);
  }
  if (ct == 0) {
    ct = _color.compare_to(other._color);
  }
  return (ct < 0);
}
