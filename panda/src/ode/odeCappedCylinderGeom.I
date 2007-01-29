// Filename: odeCappedCylinderGeom.I
// Created by:  joswilso (27Dec06)
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

INLINE void OdeCappedCylinderGeom::
set_params(dReal radius, dReal length) {
  dGeomCapsuleSetParams(_id, radius, length);
}

INLINE void OdeCappedCylinderGeom::
get_params(dReal *radius, dReal *length) const {
  dGeomCapsuleGetParams(_id, radius, length);
}

INLINE dReal OdeCappedCylinderGeom::
get_radius() const {
  dReal radius, length;
  dGeomCapsuleGetParams(_id, &radius, &length);
  return radius;
}

INLINE dReal OdeCappedCylinderGeom::
get_length() const {
  dReal radius, length;
  dGeomCapsuleGetParams(_id, &radius, &length);
  return length;
}

INLINE dReal OdeCappedCylinderGeom::
get_point_depth(dReal x, dReal y, dReal z) const {
  return dGeomCapsulePointDepth(_id, x, y, z);
}
