// Filename: orthoProjection.cxx
// Created by:  mike (18Feb99)
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

#include "geomLine.h"
#include "orthoProjection.h"

#include <boundingHexahedron.h>

TypeHandle OrthoProjection::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: OrthoProjection::make_copy
//       Access: Public, Virtual
//  Description: Allocates a new Projection just like this one.
////////////////////////////////////////////////////////////////////
Projection *OrthoProjection::
make_copy() const {
  return new OrthoProjection(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: OrthoProjection::get_projection_mat
//       Access: Public, Virtual
//  Description: This computes a transform matrix that performs the
//               orthographic transform defined by the frustum.
////////////////////////////////////////////////////////////////////
LMatrix4f OrthoProjection::
get_projection_mat(CoordinateSystem cs) const {
  return _frustum.get_ortho_projection_mat(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: OrthoProjection::make_geometry
//       Access: Public, Virtual
//  Description: Creates a GeomLine that describes the shape of the
//               frustum for this projection
////////////////////////////////////////////////////////////////////
Geom *OrthoProjection::
make_geometry(const Colorf &color,
              CoordinateSystem cs) const {
  Vertexf rtn, ltn, lbn, rbn;
  Vertexf rtf, ltf, lbf, rbf;

  // x, y, and z here refer to the right, forward, and up vectors,
  // which are not necessarily the x, y, and z axes.

  LVector3f x = LVector3f::right(cs);
  LVector3f y = LVector3f::forward(cs);
  LVector3f z = LVector3f::up(cs);
  LPoint3f o = LPoint3f::origin(cs);

  Vertexf xl = x * _frustum._l;
  Vertexf xr = x * _frustum._r;
  Vertexf zt = z * _frustum._t;
  Vertexf zb = z * _frustum._b;
  Vertexf yn = o + (y * _frustum._fnear);

  rtn = yn + zt + xr;
  ltn = yn + zt + xl;
  lbn = yn + zb + xl;
  rbn = yn + zb + xr;

  float fs = _frustum._ffar / _frustum._fnear;
  Vertexf yf = o + (y * _frustum._ffar);

  rtf = yf + ((zt + xr) * fs);
  ltf = yf + ((zt + xl) * fs);
  lbf = yf + ((zb + xl) * fs);
  rbf = yf + ((zb + xr) * fs);

  PTA_Vertexf coords(0);
  PTA_ushort vindex(0);
  PTA_Colorf colors(0);

  // We just specify overall color
  colors.push_back(color);

  coords.push_back(rtn);
  coords.push_back(ltn);
  coords.push_back(lbn);
  coords.push_back(rbn);
  coords.push_back(rtf);
  coords.push_back(ltf);
  coords.push_back(lbf);
  coords.push_back(rbf);
  coords.push_back(o);

  // Draw the near plane
  vindex.push_back(0); vindex.push_back(1);
  vindex.push_back(1); vindex.push_back(2);
  vindex.push_back(2); vindex.push_back(3);
  vindex.push_back(3); vindex.push_back(0);

  // Draw the far plane
  vindex.push_back(4); vindex.push_back(5);
  vindex.push_back(5); vindex.push_back(6);
  vindex.push_back(6); vindex.push_back(7);
  vindex.push_back(7); vindex.push_back(4);

  // Draw lines from eye to the corners
  vindex.push_back(8); vindex.push_back(4);
  vindex.push_back(8); vindex.push_back(5);
  vindex.push_back(8); vindex.push_back(6);
  vindex.push_back(8); vindex.push_back(7);

  GeomLine* gline = new GeomLine;
  gline->set_coords(coords, G_PER_VERTEX, vindex);
  gline->set_colors(colors, G_OVERALL);
  gline->set_num_prims(12);

  return gline;
}

////////////////////////////////////////////////////////////////////
//     Function: OrthoProjection::make_bounds
//       Access: Public, Virtual
//  Description: Allocates and returns a new BoundingVolume that
//               encloses the frustum used for this kind of
//               projection, if possible.  If a suitable bounding
//               volume cannot be created, returns NULL.
////////////////////////////////////////////////////////////////////
BoundingVolume *OrthoProjection::
make_bounds(CoordinateSystem cs) const {
  return new BoundingHexahedron(_frustum, true, cs);
}
