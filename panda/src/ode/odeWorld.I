// Filename: odeWorld.I
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

INLINE dWorldID OdeWorld::
get_id() const {
  return _id;
}

INLINE void OdeWorld::
set_gravity(dReal x, dReal y, dReal z) {
  dWorldSetGravity(_id, x, y, z);
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

INLINE void OdeWorld::
step_fast1(dReal stepsize, int maxiterations) {
  dWorldStepFast1(_id, stepsize, maxiterations);
}

INLINE void OdeWorld::
set_auto_enable_depth_sf1(int auto_enable_depth) {
  dWorldSetAutoEnableDepthSF1(_id, auto_enable_depth);
}

INLINE int OdeWorld::
get_auto_enable_depth_sf1() const {
  return dWorldGetAutoEnableDepthSF1(_id);
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
