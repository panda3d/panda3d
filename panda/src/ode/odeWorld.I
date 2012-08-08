// Filename: odeWorld.I
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

////////////////////////////////////////////////////////////////////
//     Function: OdeWorld::is_empty
//       Access: Published
//  Description: Returns true if the ID is 0, meaning the OdeWorld
//               does not point to a valid world. It is an error to
//               call a method on an empty world.
//               Note that an empty OdeWorld also evaluates to False.
////////////////////////////////////////////////////////////////////
INLINE bool OdeWorld::
is_empty() const {
  return (_id == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeWorld::get_id
//       Access: Published
//  Description: Returns the underlying dWorldID.
////////////////////////////////////////////////////////////////////
INLINE dWorldID OdeWorld::
get_id() const {
  return _id;
}

INLINE void OdeWorld::
set_gravity(dReal x, dReal y, dReal z) {
  dWorldSetGravity(_id, x, y, z);
}

INLINE void OdeWorld::
set_gravity(const LVecBase3f &vec) {
  dWorldSetGravity(_id, vec[0], vec[1], vec[2]);
}

INLINE LVecBase3f OdeWorld::
get_gravity() const {
  dVector3 gravity;
  dWorldGetGravity(_id, gravity);

  return LVecBase3f(gravity[0],gravity[1],gravity[2]);
}

INLINE void OdeWorld::
set_erp(dReal erp) {
  dWorldSetERP(_id, erp);
}

INLINE dReal OdeWorld::
get_erp() const {
  return dWorldGetERP(_id);
}

INLINE void OdeWorld::
set_cfm(dReal cfm) {
  dWorldSetCFM(_id, cfm);
}

INLINE dReal OdeWorld::
get_cfm() const {
  return dWorldGetCFM(_id);
}

INLINE void OdeWorld::
step(dReal stepsize) {
  dWorldStep(_id, stepsize);
}

INLINE LVecBase3f OdeWorld::
impulse_to_force(dReal stepsize, \
                 dReal ix, dReal iy, dReal iz){
  dVector3 force;
  dWorldImpulseToForce(_id,
                       stepsize,
                       ix, iy, iz,
                       force);
  return LVecBase3f(force[0], force[1], force[2]);
}

INLINE LVecBase3f OdeWorld::
impulse_to_force(dReal stepsize, \
                 const LVecBase3f &impulse){
  dVector3 force;
  dWorldImpulseToForce(_id,
                       stepsize,
                       impulse[0], impulse[1], impulse[2],
                       force);
  return LVecBase3f(force[0], force[1], force[2]);
}

INLINE void OdeWorld::
quick_step(dReal stepsize) {
  dWorldQuickStep(_id, stepsize);
}

INLINE void OdeWorld::
set_quick_step_num_iterations(int num) {
  dWorldSetQuickStepNumIterations(_id, num);
}

INLINE int OdeWorld::
get_quick_step_num_iterations() const {
  return dWorldGetQuickStepNumIterations(_id);
}

INLINE void OdeWorld::
set_quick_step_w(dReal over_relaxation) {
  dWorldSetQuickStepW(_id, over_relaxation);
}

INLINE dReal OdeWorld::
get_quick_step_w() const {
  return dWorldGetQuickStepW(_id);
}

INLINE void OdeWorld::
set_contact_max_correcting_vel(dReal vel) {
  dWorldSetContactMaxCorrectingVel(_id, vel);
}

INLINE dReal OdeWorld::
get_contact_max_correcting_vel() const {
  return dWorldGetContactMaxCorrectingVel(_id);
}

INLINE void OdeWorld::
set_contact_surface_layer(dReal depth) {
  dWorldSetContactSurfaceLayer(_id, depth);
}

INLINE dReal OdeWorld::
get_contact_surface_layer() const {
  return dWorldGetContactSurfaceLayer(_id);
}

INLINE dReal OdeWorld::
get_auto_disable_linear_threshold() const {
  return dWorldGetAutoDisableLinearThreshold(_id);
}

INLINE void OdeWorld::
set_auto_disable_linear_threshold(dReal linear_threshold) {
  dWorldSetAutoDisableLinearThreshold(_id, linear_threshold);
}

INLINE dReal OdeWorld::
get_auto_disable_angular_threshold() const {
  return dWorldGetAutoDisableAngularThreshold(_id);
}

INLINE void OdeWorld::
set_auto_disable_angular_threshold(dReal angular_threshold) {
  dWorldSetAutoDisableAngularThreshold(_id, angular_threshold);
}

INLINE int OdeWorld::
get_auto_disable_steps() const {
  return dWorldGetAutoDisableSteps(_id);
}

INLINE void OdeWorld::
set_auto_disable_steps(int steps) {
  dWorldSetAutoDisableSteps(_id, steps);
}

INLINE dReal OdeWorld::
get_auto_disable_time() const {
  return dWorldGetAutoDisableTime(_id);
}

INLINE void OdeWorld::
set_auto_disable_time(dReal time) {
  dWorldSetAutoDisableTime(_id, time);
}

INLINE int OdeWorld::
get_auto_disable_flag() const {
  return dWorldGetAutoDisableFlag(_id);
}

INLINE void OdeWorld::
set_auto_disable_flag(int do_auto_disable) {
  dWorldSetAutoDisableFlag(_id, do_auto_disable);
}

INLINE int OdeWorld::
compare_to(const OdeWorld &other) const {
  if (_id != other._id) {
    return _id < other._id ? -1 : 1;
  }
  return 0;
}
