// Filename: perspectiveProjection.cxx
// Created by:  drose (18Feb99)
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
#include "perspectiveProjection.h"

#include <boundingHexahedron.h>


TypeHandle PerspectiveProjection::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::make_copy
//       Access: Public, Virtual
//  Description: Allocates a new Projection just like this one.
////////////////////////////////////////////////////////////////////
Projection *PerspectiveProjection::
make_copy() const {
  return new PerspectiveProjection(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::get_projection_mat
//       Access: Public, Virtual
//  Description: This computes a transform matrix that performs the
//               perspective transform defined by the frustum.
////////////////////////////////////////////////////////////////////
LMatrix4f PerspectiveProjection::
get_projection_mat(CoordinateSystem cs) const {
  return _frustum.get_perspective_projection_mat(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::make_geometry
//       Access: Public, Virtual
//  Description: Creates a GeomLine that describes the shape of the
//               frustum for this projection
////////////////////////////////////////////////////////////////////
Geom *PerspectiveProjection::
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

  PTA_Vertexf coords=PTA_Vertexf::empty_array(0);
  PTA_ushort vindex=PTA_ushort::empty_array(0);
  PTA_Colorf colors=PTA_Colorf::empty_array(0);

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
  gline->set_coords(coords, vindex);
  gline->set_colors(colors, G_OVERALL);
  gline->set_num_prims(12);

  return gline;
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::make_bounds
//       Access: Public, Virtual
//  Description: Allocates and returns a new BoundingVolume that
//               encloses the frustum used for this kind of
//               projection, if possible.  If a suitable bounding
//               volume cannot be created, returns NULL.
////////////////////////////////////////////////////////////////////
BoundingVolume *PerspectiveProjection::
make_bounds(CoordinateSystem cs) const {
  return new BoundingHexahedron(_frustum, false, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::extrude
//       Access: Public, Virtual
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               projection and (-1,-1) is the lower-left corner,
//               compute the corresponding vector in space that maps
//               to this point, if such a vector can be determined.
//               Returns true if the vector is defined (in which case
//               origin and direction are set to define the vector),
//               or false otherwise.
////////////////////////////////////////////////////////////////////
bool PerspectiveProjection::
extrude(const LPoint2f &point2d, LPoint3f &origin, LVector3f &direction,
        CoordinateSystem cs) const {
  if (point2d[0] < -1 || point2d[0] > 1 ||
      point2d[1] < -1 || point2d[1] > 1) {
    // The point is off the near plane.
    return false;
  }

  // Scale the point from (-1,1) to the range of the frustum.

  LPoint2f scaled(_frustum._l + 0.5f * (point2d[0] + 1.0f) *
                  (_frustum._r - _frustum._l),
                  _frustum._b + 0.5f * (point2d[1] + 1.0f) *
                  (_frustum._t - _frustum._b));

  LVector3f near_vector =
    LVector3f::rfu(scaled[0], _frustum._fnear, scaled[1], cs);
  LVector3f far_vector =
    near_vector * _frustum._ffar / _frustum._fnear;

  origin = LPoint3f::origin(cs) + near_vector;
  direction = far_vector - near_vector;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveProjection::project
//       Access: Public, Virtual
//  Description: Given a 3-d point in space, determine the 2-d point
//               this maps to, in the range (-1,1) in both dimensions,
//               where (0,0) is the center of the projection and
//               (-1,-1) is the lower-left corner.  Returns true if
//               the 3-d point is in front of the projection and
//               within the viewing frustum (in which case point2d is
//               filled in), or false otherwise.
////////////////////////////////////////////////////////////////////
bool PerspectiveProjection::
project(const LPoint3f &point3d, LPoint2f &point2d,
        CoordinateSystem cs) const {
  float f = point3d.dot(LVector3f::forward(cs));
  if (f < _frustum._fnear || f > _frustum._ffar) {
    // The point is outside the near or far clipping planes.
    return false;
  }

  float r = point3d.dot(LVector3f::right(cs));
  float u = point3d.dot(LVector3f::up(cs));

  LPoint2f scaled(r * _frustum._fnear / f, u * _frustum._fnear / f);

  if (scaled[0] < _frustum._l || scaled[0] > _frustum._r ||
      scaled[1] < _frustum._b || scaled[1] > _frustum._t) {
    // The point is outside of the edge planes.
    return false;
  }

  point2d.set((scaled[0] - _frustum._l) * 2.0f /
              (_frustum._r - _frustum._l) - 1.0f,
              (scaled[1] - _frustum._b) * 2.0f /
              (_frustum._t - _frustum._b) - 1.0f);
  return true;
}
