/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeCappedCylinderGeom.I
 * @author joswilso
 * @date 2006-12-27
 */

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

INLINE dReal OdeCappedCylinderGeom::
get_point_depth(const LPoint3f &p) const {
  return get_point_depth(p[0], p[1], p[2]);
}
