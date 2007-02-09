// Filename: odePlaneGeom.I
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
