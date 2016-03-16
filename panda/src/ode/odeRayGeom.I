/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeRayGeom.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE void OdeRayGeom::
set_length(dReal length) {
  dGeomRaySetLength(_id, length);
}

INLINE dReal OdeRayGeom::
get_length() {
  return dGeomRayGetLength(_id);
}

INLINE void OdeRayGeom::
set(dReal px, dReal py, dReal pz, dReal dx, dReal dy, dReal dz) {
  dGeomRaySet(_id, px, py, pz, dx, dy, dz);
}

INLINE void OdeRayGeom::
set(const LVecBase3f &start, const LVecBase3f &dir) {
  set(start[0], start[1], start[2], dir[0], dir[1], dir[2]);
}

INLINE void OdeRayGeom::
get(LVecBase3f &start, LVecBase3f &dir) const {
  dVector3 s, d;
  dGeomRayGet(_id, s, d);
  start.set(s[0], s[1], s[2]);
  dir.set(d[0], d[1], d[2]);
}

INLINE LVecBase3f OdeRayGeom::
get_start() const {
  dVector3 start, dir;
  dGeomRayGet(_id, start, dir);
  return LVecBase3f(start[0], start[1], start[2]);
}

INLINE LVecBase3f OdeRayGeom::
get_direction() const {
  dVector3 start, dir;
  dGeomRayGet(_id, start, dir);
  return LVecBase3f(dir[0], dir[1], dir[2]);
}

INLINE void OdeRayGeom::
set_params(int first_contact, int backface_cull) {
  dGeomRaySetParams(_id, first_contact, backface_cull);
}

INLINE void OdeRayGeom::
get_params(int &first_contact, int &backface_cull) const {
  dGeomRayGetParams(_id, &first_contact, &backface_cull);
}

INLINE int OdeRayGeom::
get_first_contact() const {
  int fc, bc;
  dGeomRayGetParams(_id, &fc, &bc);
  return fc;
}

INLINE int OdeRayGeom::
get_backface_cull() const {
  int fc, bc;
  dGeomRayGetParams(_id, &fc, &bc);
  return bc;
}

INLINE void OdeRayGeom::
set_closest_hit(int closest_hit) {
  dGeomRaySetClosestHit(_id, closest_hit);
}

INLINE int OdeRayGeom::
get_closest_hit() {
  return dGeomRayGetClosestHit(_id);
}
