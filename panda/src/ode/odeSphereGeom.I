// Filename: odeSphereGeom.I
// Created by:  joswilso (27Dec06)
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

INLINE void OdeSphereGeom::
set_radius(dReal radius) {
  dGeomSphereSetRadius(_id, radius);
}

INLINE dReal OdeSphereGeom::
get_radius() const {
  return dGeomSphereGetRadius(_id);
}

INLINE dReal OdeSphereGeom::
get_point_depth(dReal x, dReal y, dReal z) const {
  return dGeomSpherePointDepth(_id, x, y, z);
}

INLINE dReal OdeSphereGeom::
get_point_depth(const LPoint3f &p) const {
  return get_point_depth(p[0], p[1], p[2]);
}

