// Filename: odeCylinderGeom.I
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

INLINE void OdeCylinderGeom::
set_params(dReal radius, dReal length) {
  dGeomCylinderSetParams(_id, radius, length);
}

INLINE void OdeCylinderGeom::
get_params(dReal *radius, dReal *length) const {
  dGeomCapsuleGetParams(_id, radius, length);
}

INLINE dReal OdeCylinderGeom::
get_radius() const {
  dReal radius, length;
  dGeomCylinderGetParams(_id, &radius, &length);
  return radius;
}

INLINE dReal OdeCylinderGeom::
get_length() const {
  dReal radius, length;
  dGeomCylinderGetParams(_id, &radius, &length);
  return length;
}
