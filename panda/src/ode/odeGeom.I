// Filename: odeGeom.I
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

INLINE dGeomID OdeGeom::
get_id() const {
  return _id;
}

INLINE OdeBody OdeGeom::
get_body() const {
  return OdeBody(dGeomGetBody(_id));
}

INLINE void OdeGeom::
set_body(OdeBody &body) {
  dGeomSetBody(_id, body.get_id());
}

/*
INLINE void OdeGeom::
set_data(void* data) {
  dGeomSetData(_id, data);
}
*/

INLINE void OdeGeom::
set_position(dReal x, dReal y, dReal z) {
  dGeomSetPosition(_id, x, y, z);
}

INLINE void OdeGeom::
set_rotation(const LMatrix3f &r) {
  dMatrix3 rot = { r._m.data[0], r._m.data[1], r._m.data[2], 0,
                   r._m.data[3], r._m.data[4], r._m.data[5], 0,
                   r._m.data[6], r._m.data[7], r._m.data[8], 0 };
  dGeomSetRotation(_id, rot);
}

INLINE void OdeGeom::
set_quaternion(const LQuaternionf &q) {
  dQuaternion quat = { q[0], q[1], q[2], q[3] };
  dGeomSetQuaternion(_id, quat);
}

INLINE LPoint3f OdeGeom::
get_position() const {
  const dReal *pos = dGeomGetPosition(_id);
  return LPoint3f(pos[0], pos[1], pos[2]);
}

INLINE LMatrix3f OdeGeom::
get_rotation() const {
  const dReal *rot = dGeomGetRotation(_id);
  return LMatrix3f(rot[0], rot[1], rot[2],
                   rot[4], rot[5], rot[6],
                   rot[8], rot[9], rot[10]);
}

INLINE LQuaternionf OdeGeom::
get_quaternion() const {
  dQuaternion res;
  dGeomGetQuaternion(_id, res);
  return LQuaternionf(res[0], res[1], res[2], res[3]);
}

/*
INLINE void OdeGeom::
get_aabb(dReal aabb[6]) const {
  return dGeomGetAABB(_id, aabb[6]);
}
*/

INLINE int OdeGeom::
is_space() {
  return dGeomIsSpace(_id);
}

INLINE int OdeGeom::
get_class() const {
  return dGeomGetClass(_id);
}

INLINE void OdeGeom::
set_category_bits(unsigned long bits) {
  dGeomSetCategoryBits(_id, bits);
}

INLINE void OdeGeom::
set_collide_bits(unsigned long bits) {
  dGeomSetCollideBits(_id, bits);
}

INLINE void OdeGeom::
enable() {
  dGeomEnable(_id);
}

INLINE void OdeGeom::
disable() {
  dGeomDisable(_id);
}

INLINE int OdeGeom::
is_enabled() {
  return dGeomIsEnabled(_id);
}

INLINE void OdeGeom::
set_offset_position(dReal x, dReal y, dReal z) {
  dGeomSetOffsetPosition(_id, x, y, z);
}

INLINE void OdeGeom::
set_offset_rotation(const LMatrix3f &r) {
  dMatrix3 rot = { r._m.data[0], r._m.data[1], r._m.data[2], 0,
                   r._m.data[3], r._m.data[4], r._m.data[5], 0,
                   r._m.data[6], r._m.data[7], r._m.data[8], 0 };
  dGeomSetOffsetRotation(_id, rot);
}

INLINE void OdeGeom::
set_offset_quaternion(const LQuaternionf &q) {
  dQuaternion quat = { q[0], q[1], q[2], q[3] };
  dGeomSetOffsetQuaternion(_id, quat);
}

INLINE void OdeGeom::
set_offset_world_position(dReal x, dReal y, dReal z) {
  dGeomSetOffsetWorldPosition(_id, x, y, z);
}

INLINE void OdeGeom::
set_offset_world_rotation(const LMatrix3f &r) {
  dMatrix3 rot = { r._m.data[0], r._m.data[1], r._m.data[2], 0,
                   r._m.data[3], r._m.data[4], r._m.data[5], 0,
                   r._m.data[6], r._m.data[7], r._m.data[8], 0 };
  dGeomSetOffsetWorldRotation(_id, rot);
}

INLINE void OdeGeom::
set_offset_world_quaternion(const LQuaternionf &q) {
  dQuaternion quat = { q[0], q[1], q[2], q[3] };
  dGeomSetOffsetWorldQuaternion(_id, quat);
}

INLINE void OdeGeom::
clear_offset() {
  dGeomClearOffset(_id);
}

INLINE int OdeGeom::
is_offset() {
  return dGeomIsOffset(_id);
}

INLINE LPoint3f OdeGeom::
get_offset_position() const {
  const dReal *pos = dGeomGetOffsetPosition(_id);
  return LPoint3f(pos[0], pos[1], pos[2]);
}

INLINE LMatrix3f OdeGeom::
get_offset_rotation() const {
  const dReal *rot = dGeomGetOffsetRotation(_id);
  return LMatrix3f(rot[0], rot[1], rot[2],
                   rot[4], rot[5], rot[6],
                   rot[8], rot[9], rot[10]);
}

INLINE LQuaternionf OdeGeom::
get_offset_quaternion() const {
  dQuaternion res;
  dGeomGetOffsetQuaternion(_id, res);
  return LQuaternionf(res[0], res[1], res[2], res[3]);
}
