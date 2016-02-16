/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBoxGeom.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE void OdeBoxGeom::
set_lengths(dReal lx, dReal ly, dReal lz) {
  dGeomBoxSetLengths(_id, lx, ly, lz);
}

INLINE void OdeBoxGeom::
set_lengths(const LVecBase3f &size) {
  set_lengths(size[0], size[1], size[2]);
}

INLINE LVecBase3f OdeBoxGeom::
get_lengths() {
  dVector3 res;
  dGeomBoxGetLengths(_id, res);
  return LVecBase3f(res[0], res[1], res[2]);
}

INLINE dReal OdeBoxGeom::
get_point_depth(dReal x, dReal y, dReal z) {
  return dGeomBoxPointDepth(_id, x, y, z);
}

INLINE dReal OdeBoxGeom::
get_point_depth(const LPoint3f &p) {
  return get_point_depth(p[0], p[1], p[2]);
}
