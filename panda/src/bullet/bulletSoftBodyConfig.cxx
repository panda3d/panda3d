/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyConfig.cxx
 * @author enn0x
 * @date 2010-04-12
 */

#include "bulletSoftBodyConfig.h"

#include "bulletWorld.h"

#include "lightMutexHolder.h"

/**
 *
 */
BulletSoftBodyConfig::
BulletSoftBodyConfig(btSoftBody::Config &cfg) : _cfg(cfg) {

}

/**
 *
 */
void BulletSoftBodyConfig::
clear_all_collision_flags() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.collisions = 0;
}

/**
 *
 */
void BulletSoftBodyConfig::
set_collision_flag(CollisionFlag flag, bool value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  if (value == true) {
    _cfg.collisions |= flag;
  }
  else {
    _cfg.collisions &= ~(flag);
  }
}

/**
 *
 */
bool BulletSoftBodyConfig::
get_collision_flag(CollisionFlag flag) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (_cfg.collisions & flag) ? true : false;
}

/**
 *
 */
void BulletSoftBodyConfig::
set_aero_model(AeroModel value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.aeromodel = (btSoftBody::eAeroModel::_)value;
}

/**
 *
 */
BulletSoftBodyConfig::AeroModel BulletSoftBodyConfig::
get_aero_model() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (AeroModel)_cfg.aeromodel;
}

/**
 * Getter for property kVCF.
 */
PN_stdfloat BulletSoftBodyConfig::
get_velocities_correction_factor() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kVCF;
}

/**
 * Setter for property kVCF.
 */
void BulletSoftBodyConfig::
set_velocities_correction_factor(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kVCF = (btScalar)value;
}

/**
 * Getter for property kDP.
 */
PN_stdfloat BulletSoftBodyConfig::
get_damping_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kDP;
}

/**
 * Setter for property kDP.
 */
void BulletSoftBodyConfig::
set_damping_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kDP = (btScalar)value;
}

/**
 * Getter for property kDG.
 */
PN_stdfloat BulletSoftBodyConfig::
get_drag_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kDG;
}

/**
 * Setter for property kDG.
 */
void BulletSoftBodyConfig::
set_drag_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kDG = (btScalar)value;
}

/**
 * Getter for property kLF.
 */
PN_stdfloat BulletSoftBodyConfig::
get_lift_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kLF;
}

/**
 * Setter for property kLF.
 */
void BulletSoftBodyConfig::
set_lift_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kLF = (btScalar)value;
}

/**
 * Getter for property kPR.
 */
PN_stdfloat BulletSoftBodyConfig::
get_pressure_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kPR;
}

/**
 * Setter for property kPR.
 */
void BulletSoftBodyConfig::
set_pressure_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kPR = (btScalar)value;
}

/**
 * Getter for property kVC.
 */
PN_stdfloat BulletSoftBodyConfig::
get_volume_conservation_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kVC;
}

/**
 * Setter for property kVC.
 */
void BulletSoftBodyConfig::
set_volume_conservation_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kVC = (btScalar)value;
}

/**
 * Getter for property kDF.
 */
PN_stdfloat BulletSoftBodyConfig::
get_dynamic_friction_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kDF;
}

/**
 * Setter for property kDF.
 */
void BulletSoftBodyConfig::
set_dynamic_friction_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kDF = (btScalar)value;
}

/**
 * Getter for property kMT.
 */
PN_stdfloat BulletSoftBodyConfig::
get_pose_matching_coefficient() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kMT;
}

/**
 * Setter for property kMT.
 */
void BulletSoftBodyConfig::
set_pose_matching_coefficient(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kMT = (btScalar)value;
}

/**
 * Getter for property kCHR.
 */
PN_stdfloat BulletSoftBodyConfig::
get_rigid_contacts_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kCHR;
}

/**
 * Setter for property kCHR.
 */
void BulletSoftBodyConfig::
set_rigid_contacts_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kCHR = (btScalar)value;
}

/**
 * Getter for property kKHR.
 */
PN_stdfloat BulletSoftBodyConfig::
get_kinetic_contacts_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kKHR;
}

/**
 * Setter for property kKHR.
 */
void BulletSoftBodyConfig::
set_kinetic_contacts_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kKHR = (btScalar)value;
}

/**
 * Getter for property kSHR.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_contacts_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSHR;
}

/**
 * Setter for property kSHR.
 */
void BulletSoftBodyConfig::
set_soft_contacts_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSHR = (btScalar)value;
}

/**
 * Getter for property kAHR.
 */
PN_stdfloat BulletSoftBodyConfig::
get_anchors_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kAHR;
}

/**
 * Setter for property kAHR.
 */
void BulletSoftBodyConfig::
set_anchors_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kAHR = (btScalar)value;
}

/**
 * Getter for property kSRHR_CL.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_vs_rigid_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSRHR_CL;
}

/**
 * Setter for property kSRHR_CL.
 */
void BulletSoftBodyConfig::
set_soft_vs_rigid_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSRHR_CL = (btScalar)value;
}

/**
 * Getter for property kSKHR_CL.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_vs_kinetic_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSKHR_CL;
}

/**
 * Setter for property kSKHR_CL.
 */
void BulletSoftBodyConfig::
set_soft_vs_kinetic_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSKHR_CL = (btScalar)value;
}

/**
 * Getter for property kSSHR_CL.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_vs_soft_hardness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSSHR_CL;
}

/**
 * Setter for property kSSHR_CL.
 */
void BulletSoftBodyConfig::
set_soft_vs_soft_hardness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSSHR_CL = (btScalar)value;
}

/**
 * Getter for property kSR_SPLT_CL.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_vs_rigid_impulse_split() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSR_SPLT_CL;
}

/**
 * Setter for property kSR_SPLT_CL.
 */
void BulletSoftBodyConfig::
set_soft_vs_rigid_impulse_split(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSR_SPLT_CL = (btScalar)value;
}

/**
 * Getter for property kSK_SPLT_CL.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_vs_kinetic_impulse_split() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSK_SPLT_CL;
}

/**
 * Setter for property kSK_SPLT_CL.
 */
void BulletSoftBodyConfig::
set_soft_vs_kinetic_impulse_split(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSK_SPLT_CL = (btScalar)value;
}

/**
 * Getter for property kSS_SPLT_CL.
 */
PN_stdfloat BulletSoftBodyConfig::
get_soft_vs_soft_impulse_split() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.kSS_SPLT_CL;
}

/**
 * Setter for property kSS_SPLT_CL.
 */
void BulletSoftBodyConfig::
set_soft_vs_soft_impulse_split(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.kSS_SPLT_CL = (btScalar)value;
}

/**
 * Getter for property maxvolume.
 */
PN_stdfloat BulletSoftBodyConfig::
get_maxvolume() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.maxvolume;
}

/**
 * Setter for property maxvolume.
 */
void BulletSoftBodyConfig::
set_maxvolume(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.maxvolume = (btScalar)value;
}

/**
 * Getter for property timescale.
 */
PN_stdfloat BulletSoftBodyConfig::
get_timescale() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_cfg.timescale;
}

/**
 * Setter for property timescale.
 */
void BulletSoftBodyConfig::
set_timescale(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _cfg.timescale = (btScalar)value;
}

/**
 * Getter for property piterations.
 */
int BulletSoftBodyConfig::
get_positions_solver_iterations() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _cfg.piterations;
}

/**
 * Setter for property piterations.
 */
void BulletSoftBodyConfig::
set_positions_solver_iterations(int value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(value > 0);
  _cfg.piterations = value;
}

/**
 * Getter for property viterations.
 */
int BulletSoftBodyConfig::
get_velocities_solver_iterations() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _cfg.viterations;
}

/**
 * Setter for property viterations.
 */
void BulletSoftBodyConfig::
set_velocities_solver_iterations(int value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(value > 0);
  _cfg.viterations = value;
}

/**
 * Getter for property diterations.
 */
int BulletSoftBodyConfig::
get_drift_solver_iterations() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _cfg.diterations;
}

/**
 * Setter for property diterations.
 */
void BulletSoftBodyConfig::
set_drift_solver_iterations(int value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(value > 0);
  _cfg.diterations = value;
}

/**
 * Getter for property citerations.
 */
int BulletSoftBodyConfig::
get_cluster_solver_iterations() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _cfg.citerations;
}

/**
 * Setter for property citerations.
 */
void BulletSoftBodyConfig::
set_cluster_solver_iterations(int value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(value > 0);
  _cfg.citerations = value;
}
