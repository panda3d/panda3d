/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBody.I
 * @author joswilso
 * @date 2006-12-27
 */

/**
 * Returns true if the ID is 0, meaning the OdeBody does not point to a valid
 * body.  It is an error to call a method on an empty body.  Note that an
 * empty OdeBody also evaluates to False.
 */
INLINE bool OdeBody::
is_empty() const {
  return (_id == 0);
}

/**
 * Returns the underlying dBodyID.
 */
INLINE dBodyID OdeBody::
get_id() const {
  return _id;
}

INLINE dReal OdeBody::
get_auto_disable_linear_threshold() const {
  return dBodyGetAutoDisableLinearThreshold(_id);
}

INLINE void OdeBody::
set_auto_disable_linear_threshold(dReal linear_threshold) {
  dBodySetAutoDisableLinearThreshold(_id, linear_threshold);
}

INLINE dReal OdeBody::
get_auto_disable_angular_threshold() const {
  return dBodyGetAutoDisableAngularThreshold(_id);
}

INLINE void OdeBody::
set_auto_disable_angular_threshold(dReal angular_threshold) {
  dBodySetAutoDisableAngularThreshold(_id, angular_threshold);
}

INLINE int OdeBody::
get_auto_disable_steps() const {
  return dBodyGetAutoDisableSteps(_id);
}

INLINE void OdeBody::
set_auto_disable_steps(int steps) {
  dBodySetAutoDisableSteps(_id, steps);
}

INLINE dReal OdeBody::
get_auto_disable_time() const {
  return dBodyGetAutoDisableTime(_id);
}

INLINE void OdeBody::
set_auto_disable_time(dReal time) {
  dBodySetAutoDisableTime(_id, time);
}

INLINE int OdeBody::
get_auto_disable_flag() const {
  return dBodyGetAutoDisableFlag(_id);
}

INLINE void OdeBody::
set_auto_disable_flag(int do_auto_disable) {
  dBodySetAutoDisableFlag(_id, do_auto_disable);
}

INLINE void OdeBody::
set_auto_disable_defaults() {
  dBodySetAutoDisableDefaults(_id);
}

INLINE void OdeBody::
set_data(void *data) {
  dBodySetData(_id, data);
}

INLINE void *OdeBody::
get_data() const {
  return dBodyGetData(_id);
}

INLINE void OdeBody::
set_position(dReal x, dReal y, dReal z) {
  dBodySetPosition(_id, x, y, z);
}

INLINE void OdeBody::
set_position(const LVecBase3f &pos) {
  set_position(pos[0], pos[1], pos[2]);
}

INLINE void OdeBody::
set_rotation(const LMatrix3f &r) {
  dMatrix3 mat3 = { r(0, 0), r(0, 1), r(0, 2), 0,
                    r(1, 0), r(1, 1), r(1, 2), 0,
                    r(2, 0), r(2, 1), r(2, 2), 0 };

  dBodySetRotation(_id, mat3);
}

INLINE void OdeBody::
set_quaternion(const LQuaternionf &q) {
  dQuaternion quat = { q[0], q[1], q[2], q[3] };
  dBodySetQuaternion(_id, quat);
}

INLINE void OdeBody::
set_linear_vel(dReal x, dReal y, dReal z) {
  dBodySetLinearVel(_id, x, y, z);
}

INLINE void OdeBody::
set_linear_vel(const LVecBase3f &vel) {
  set_linear_vel(vel[0], vel[1], vel[2]);
}

INLINE void OdeBody::
set_angular_vel(dReal x, dReal y, dReal z) {
  dBodySetAngularVel(_id, x, y, z);
}

INLINE void OdeBody::
set_angular_vel(const LVecBase3f &vel) {
  set_angular_vel(vel[0], vel[1], vel[2]);
}

INLINE LVecBase3f OdeBody::
get_position() const {
  const dReal *res = dBodyGetPosition(_id);
  return LVecBase3f(res[0], res[1], res[2]);
}

INLINE LMatrix3f OdeBody::
get_rotation() const {
  const dReal *rot = dBodyGetRotation(_id);
  return LMatrix3f(rot[0], rot[1], rot[2],
                   rot[4], rot[5], rot[6],
                   rot[8], rot[9], rot[10]);
}

INLINE LVecBase4f OdeBody::
get_quaternion() const {
  const dReal *res = dBodyGetQuaternion(_id);
  return LVecBase4f(res[0], res[1], res[2], res[3]);
}

INLINE LVecBase3f OdeBody::
get_linear_vel() const {
  const dReal *res = dBodyGetLinearVel(_id);
  return LVecBase3f(res[0], res[1], res[2]);
}

INLINE LVecBase3f OdeBody::
get_angular_vel() const {
  const dReal *res = dBodyGetAngularVel(_id);
  return LVecBase3f(res[0], res[1], res[2]);
}

INLINE void OdeBody::
set_mass(OdeMass &mass) {
  dBodySetMass(_id, mass.get_mass_ptr());
}

INLINE OdeMass OdeBody::
get_mass() const {
  OdeMass mass;
  dBodyGetMass(_id, mass.get_mass_ptr());
  return mass;
}

INLINE void OdeBody::
add_force(dReal fx, dReal fy, dReal fz) {
  dBodyAddForce(_id, fx, fy, fz);
}

INLINE void OdeBody::
add_force(const LVecBase3f &f) {
  add_force(f[0], f[1], f[2]);
}

INLINE void OdeBody::
add_torque(dReal fx, dReal fy, dReal fz) {
  dBodyAddTorque(_id, fx, fy, fz);
}

INLINE void OdeBody::
add_torque(const LVecBase3f &f) {
  add_torque(f[0], f[1], f[2]);
}

INLINE void OdeBody::
add_rel_force(dReal fx, dReal fy, dReal fz) {
  dBodyAddRelForce(_id, fx, fy, fz);
}

INLINE void OdeBody::
add_rel_force(const LVecBase3f &f) {
  add_rel_force(f[0], f[1], f[2]);
}

INLINE void OdeBody::
add_rel_torque(dReal fx, dReal fy, dReal fz) {
  dBodyAddRelTorque(_id, fx, fy, fz);
}

INLINE void OdeBody::
add_rel_torque(const LVecBase3f &f) {
  add_rel_torque(f[0], f[1], f[2]);
}

INLINE void OdeBody::
add_force_at_pos(dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz) {
  dBodyAddForceAtPos(_id, fx, fy, fz, px, py, pz);
}

INLINE void OdeBody::
add_force_at_pos(const LVecBase3f &f, const LVecBase3f &pos) {
  add_force_at_pos(f[0], f[1], f[2], pos[0], pos[1], pos[2]);
}

INLINE void OdeBody::
add_force_at_rel_pos(dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz) {
  dBodyAddForceAtRelPos(_id, fx, fy, fz, px, py, pz);
}

INLINE void OdeBody::
add_force_at_rel_pos(const LVecBase3f &f, const LVecBase3f &pos) {
  add_force_at_rel_pos(f[0], f[1], f[2], pos[0], pos[1], pos[2]);
}

INLINE void OdeBody::
add_rel_force_at_pos(dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz) {
  dBodyAddRelForceAtPos(_id, fx, fy, fz, px, py, pz);
}

INLINE void OdeBody::
add_rel_force_at_pos(const LVecBase3f &f, const LVecBase3f &pos) {
  add_rel_force_at_pos(f[0], f[1], f[2], pos[0], pos[1], pos[2]);
}

INLINE void OdeBody::
add_rel_force_at_rel_pos(dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz) {
  dBodyAddRelForceAtRelPos(_id, fx, fy, fz, px, py, pz);
}

INLINE void OdeBody::
add_rel_force_at_rel_pos(const LVecBase3f &f, const LVecBase3f &pos) {
  add_rel_force_at_rel_pos(f[0], f[1], f[2], pos[0], pos[1], pos[2]);
}

INLINE void OdeBody::
set_force(dReal x, dReal y, dReal z) {
  dBodySetForce(_id, x, y, z);
}

INLINE void OdeBody::
set_force(const LVecBase3f &f) {
  set_force(f[0], f[1], f[2]);
}

INLINE void OdeBody::
set_torque(dReal x, dReal y, dReal z) {
  dBodySetTorque(_id, x, y, z);
}

INLINE void OdeBody::
set_torque(const LVecBase3f &f) {
  set_torque(f[0], f[1], f[2]);
}

INLINE LPoint3f OdeBody::
get_rel_point_pos(dReal px, dReal py, dReal pz) const {
  dVector3 result;
  dBodyGetRelPointPos(_id, px, py, pz, result);
  return LPoint3f(result[0], result[1], result[2]);
}

INLINE LPoint3f OdeBody::
get_rel_point_pos(const LVecBase3f &pos) const {
  return get_rel_point_pos(pos[0], pos[1], pos[2]);
}

INLINE LPoint3f OdeBody::
get_rel_point_vel(dReal px, dReal py, dReal pz) const {
  dVector3 result;
  dBodyGetRelPointVel(_id, px, py, pz, result);
  return LPoint3f(result[0], result[1], result[2]);
}

INLINE LPoint3f OdeBody::
get_rel_point_vel(const LVecBase3f &pos) const {
  return get_rel_point_vel(pos[0], pos[1], pos[2]);
}

INLINE LPoint3f OdeBody::
get_point_vel(dReal px, dReal py, dReal pz) const {
  dVector3 result;
  dBodyGetPointVel(_id, px, py, pz, result);
  return LPoint3f(result[0], result[1], result[2]);
}

INLINE LPoint3f OdeBody::
get_point_vel(const LVecBase3f &pos) const {
  return get_point_vel(pos[0], pos[1], pos[2]);
}

INLINE LPoint3f OdeBody::
get_pos_rel_point(dReal px, dReal py, dReal pz) const {
  dVector3 result;
  dBodyGetPosRelPoint(_id, px, py, pz, result);
  return LPoint3f(result[0], result[1], result[2]);
}

INLINE LPoint3f OdeBody::
get_pos_rel_point(const LVecBase3f &pos) const {
  return get_pos_rel_point(pos[0], pos[1], pos[2]);
}

INLINE LVecBase3f OdeBody::
vector_to_world(dReal px, dReal py, dReal pz) const {
  dVector3 result;
  dBodyVectorToWorld(_id, px, py, pz, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeBody::
vector_to_world(const LVecBase3f &pos) const {
  return vector_to_world(pos[0], pos[1], pos[2]);
}

INLINE LVecBase3f OdeBody::
vector_from_world(dReal px, dReal py, dReal pz) const {
  dVector3 result;
  dBodyVectorFromWorld(_id, px, py, pz, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE LVecBase3f OdeBody::
vector_from_world(const LVecBase3f &pos) const {
  return vector_from_world(pos[0], pos[1], pos[2]);
}

INLINE void OdeBody::
set_finite_rotation_mode(int mode) {
  dBodySetFiniteRotationMode(_id, mode);
}

INLINE void OdeBody::
set_finite_rotation_axis(dReal x, dReal y, dReal z) {
  dBodySetFiniteRotationAxis(_id, x, y, z);
}

INLINE void OdeBody::
set_finite_rotation_axis(const LVecBase3f &axis) {
  set_finite_rotation_axis(axis[0], axis[1], axis[2]);
}

INLINE int OdeBody::
get_finite_rotation_mode() const {
  return dBodyGetFiniteRotationMode(_id);
}

INLINE LVecBase3f OdeBody::
get_finite_rotation_axis() const {
  dVector3 result;
  dBodyGetFiniteRotationAxis(_id, result);
  return LVecBase3f(result[0], result[1], result[2]);
}

INLINE int OdeBody::
get_num_joints() const {
  return dBodyGetNumJoints(_id);
}

INLINE void OdeBody::
enable() {
  dBodyEnable(_id);
}

INLINE void OdeBody::
disable() {
  dBodyDisable(_id);
}

INLINE int OdeBody::
is_enabled() const {
  return dBodyIsEnabled(_id);
}

INLINE void OdeBody::
set_gravity_mode(int mode) {
  dBodySetGravityMode(_id, mode);
}

INLINE int OdeBody::
get_gravity_mode() const {
  return dBodyGetGravityMode(_id);
}

INLINE int OdeBody::
compare_to(const OdeBody &other) const {
  if (_id != other._id) {
    return _id < other._id ? -1 : 1;
  }
  return 0;
}
