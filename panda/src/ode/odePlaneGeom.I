// Filename: odePlaneGeom.I
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

INLINE void OdePlaneGeom::
set_params(dReal a, dReal b, dReal c, dReal d) {
  dGeomPlaneSetParams(_id, a, b, c, d);
}

INLINE void OdePlaneGeom::
set_params(const LVecBase4f &params) {
  set_params(params[0], params[1], params[2], params[3]);
}

INLINE LVecBase4f OdePlaneGeom::
get_params() const {
  dVector4 res;
  dGeomPlaneGetParams(_id, res);
  return LVecBase4f(res[0], res[1], res[2], res[3]);
}

INLINE dReal OdePlaneGeom::
get_point_depth(dReal x, dReal y, dReal z) const {
  return dGeomPlanePointDepth(_id, x, y, z);
}

INLINE dReal OdePlaneGeom::
get_point_depth(const LPoint3f &p) const {
  return get_point_depth(p[0], p[1], p[2]);
}
