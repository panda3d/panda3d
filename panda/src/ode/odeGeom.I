/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeGeom.I
 * @author joswilso
 * @date 2006-12-27
 */

/**
 * Returns true if the ID is 0, meaning the OdeGeom does not point to a valid
 * geom.  It is an error to call a method on an empty geom.  Note that an
 * empty OdeGeom also evaluates to False.
 */
INLINE bool OdeGeom::
is_empty() const {
  return (_id == 0);
}

/**
 * Returns the underlying dGeomID.
 */
INLINE dGeomID OdeGeom::
get_id() const {
  return _id;
}

INLINE bool OdeGeom::
has_body() const {
  return (dGeomGetBody(_id) != nullptr);
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
set_position(const LVecBase3f &pos) {
  set_position(pos[0], pos[1], pos[2]);
}

INLINE void OdeGeom::
set_rotation(const LMatrix3f &r) {
  dMatrix3 rot = { r(0, 0), r(0, 1), r(0, 2), 0,
                   r(1, 0), r(1, 1), r(1, 2), 0,
                   r(2, 0), r(2, 1), r(2, 2), 0 };
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

INLINE void OdeGeom::
get_AABB(LVecBase3f &min, LVecBase3f &max) const {
  dReal result[6];
  dGeomGetAABB(_id, result);
  min.set(result[0], result[2], result[4]);
  max.set(result[1], result[3], result[5]);
}

INLINE int OdeGeom::
is_space() {
  return dGeomIsSpace(_id);
}

INLINE int OdeGeom::
get_class() const {
  return dGeomGetClass(_id);
}

INLINE void OdeGeom::
set_category_bits(const BitMask32 &bits) {
  dGeomSetCategoryBits(_id, bits.get_word());
}

INLINE void OdeGeom::
set_collide_bits(const BitMask32 &bits) {
  dGeomSetCollideBits(_id, bits.get_word());
}

INLINE BitMask32 OdeGeom::
get_category_bits() {
  return BitMask32(dGeomGetCategoryBits(_id));
}

INLINE BitMask32 OdeGeom::
get_collide_bits() {
  return BitMask32(dGeomGetCollideBits(_id));
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
set_offset_position(const LVecBase3f &pos) {
  set_offset_position(pos[0], pos[1], pos[2]);
}

INLINE void OdeGeom::
set_offset_rotation(const LMatrix3f &r) {
  dMatrix3 rot = { r(0, 0), r(0, 1), r(0, 2), 0,
                   r(1, 0), r(1, 1), r(1, 2), 0,
                   r(2, 0), r(2, 1), r(2, 2), 0 };
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
set_offset_world_position(const LVecBase3f &pos) {
  set_offset_world_position(pos[0], pos[1], pos[2]);
}

INLINE void OdeGeom::
set_offset_world_rotation(const LMatrix3f &r) {
  dMatrix3 rot = { r(0, 0), r(0, 1), r(0, 2), 0,
                   r(1, 0), r(1, 1), r(1, 2), 0,
                   r(2, 0), r(2, 1), r(2, 2), 0 };
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

INLINE int OdeGeom::
compare_to(const OdeGeom &other) const {
  if (_id != other._id) {
    return _id < other._id ? -1 : 1;
  }
  return 0;
}
