// Filename: bulletSoftBodyConfig.I
// Created by:  enn0x (12Apr10)
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
//     Function: BulletSoftBodyConfig::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyConfig::
~BulletSoftBodyConfig() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_velocities_correction_factor
//       Access: Published
//  Description: Getter for property kVCF.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_velocities_correction_factor() const {

  return _cfg.kVCF;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_velocities_correction_factor
//       Access: Published
//  Description: Setter for property kVCF.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_velocities_correction_factor(float value) {

  _cfg.kVCF = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_damping_coefficient
//       Access: Published
//  Description: Getter for property kDP.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_damping_coefficient() const {

  return _cfg.kDP;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_damping_coefficient
//       Access: Published
//  Description: Setter for property kDP.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_damping_coefficient(float value) {

  _cfg.kDP = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_drag_coefficient
//       Access: Published
//  Description: Getter for property kDG.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_drag_coefficient() const {

  return _cfg.kDG;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_drag_coefficient
//       Access: Published
//  Description: Setter for property kDG.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_drag_coefficient(float value) {

  _cfg.kDG = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_lift_coefficient
//       Access: Published
//  Description: Getter for property kLF.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_lift_coefficient() const {

  return _cfg.kLF;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_lift_coefficient
//       Access: Published
//  Description: Setter for property kLF.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_lift_coefficient(float value) {

  _cfg.kLF = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_pressure_coefficient
//       Access: Published
//  Description: Getter for property kPR.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_pressure_coefficient() const {

  return _cfg.kPR;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_pressure_coefficient
//       Access: Published
//  Description: Setter for property kPR.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_pressure_coefficient(float value) {

  _cfg.kPR = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_volume_conversation_coefficient
//       Access: Published
//  Description: Getter for property kVC.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_volume_conversation_coefficient() const {

  return _cfg.kVC;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_volume_conversation_coefficient
//       Access: Published
//  Description: Setter for property kVC.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_volume_conversation_coefficient(float value) {

  _cfg.kVC = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_dynamic_friction_coefficient
//       Access: Published
//  Description: Getter for property kDF.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_dynamic_friction_coefficient() const {

  return _cfg.kDF;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_dynamic_friction_coefficient
//       Access: Published
//  Description: Setter for property kDF.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_dynamic_friction_coefficient(float value) {

  _cfg.kDF = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_pose_matching_coefficient
//       Access: Published
//  Description: Getter for property kMT.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_pose_matching_coefficient() const {

  return _cfg.kMT;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_pose_matching_coefficient
//       Access: Published
//  Description: Setter for property kMT.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_pose_matching_coefficient(float value) {

  _cfg.kMT = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_rigid_contacts_hardness
//       Access: Published
//  Description: Getter for property kCHR.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_rigid_contacts_hardness() const {

  return _cfg.kCHR;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_rigid_contacts_hardness
//       Access: Published
//  Description: Setter for property kCHR.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_rigid_contacts_hardness(float value) {

  _cfg.kCHR = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_kinetic_contacts_hardness
//       Access: Published
//  Description: Getter for property kKHR.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_kinetic_contacts_hardness() const {

  return _cfg.kKHR;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_kinetic_contacts_hardness
//       Access: Published
//  Description: Setter for property kKHR.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_kinetic_contacts_hardness(float value) {

  _cfg.kKHR = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_contacts_hardness
//       Access: Published
//  Description: Getter for property kSHR.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_contacts_hardness() const {

  return _cfg.kSHR;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_contacts_hardness
//       Access: Published
//  Description: Setter for property kSHR.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_contacts_hardness(float value) {

  _cfg.kSHR = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_anchors_hardness
//       Access: Published
//  Description: Getter for property kAHR.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_anchors_hardness() const {

  return _cfg.kAHR;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_anchors_hardness
//       Access: Published
//  Description: Setter for property kAHR.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_anchors_hardness(float value) {

  _cfg.kAHR = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_vs_rigid_hardness
//       Access: Published
//  Description: Getter for property kSRHR_CL.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_vs_rigid_hardness() const {

  return _cfg.kSRHR_CL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_vs_rigid_hardness
//       Access: Published
//  Description: Setter for property kSRHR_CL.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_vs_rigid_hardness(float value) {

  _cfg.kSRHR_CL = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_vs_kinetic_hardness
//       Access: Published
//  Description: Getter for property kSKHR_CL.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_vs_kinetic_hardness() const {

  return _cfg.kSKHR_CL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_vs_kinetic_hardness
//       Access: Published
//  Description: Setter for property kSKHR_CL.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_vs_kinetic_hardness(float value) {

  _cfg.kSKHR_CL = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_vs_soft_hardness
//       Access: Published
//  Description: Getter for property kSSHR_CL.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_vs_soft_hardness() const {

  return _cfg.kSSHR_CL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_vs_soft_hardness
//       Access: Published
//  Description: Setter for property kSSHR_CL.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_vs_soft_hardness(float value) {

  _cfg.kSSHR_CL = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_vs_rigid_impulse_split
//       Access: Published
//  Description: Getter for property kSR_SPLT_CL.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_vs_rigid_impulse_split() const {

  return _cfg.kSR_SPLT_CL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_vs_rigid_impulse_split
//       Access: Published
//  Description: Setter for property kSR_SPLT_CL.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_vs_rigid_impulse_split(float value) {

  _cfg.kSR_SPLT_CL = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_vs_kinetic_impulse_split
//       Access: Published
//  Description: Getter for property kSK_SPLT_CL.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_vs_kinetic_impulse_split() const {

  return _cfg.kSK_SPLT_CL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_vs_kinetic_impulse_split
//       Access: Published
//  Description: Setter for property kSK_SPLT_CL.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_vs_kinetic_impulse_split(float value) {

  _cfg.kSK_SPLT_CL = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_soft_vs_soft_impulse_split
//       Access: Published
//  Description: Getter for property kSS_SPLT_CL.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_soft_vs_soft_impulse_split() const {

  return _cfg.kSS_SPLT_CL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_soft_vs_soft_impulse_split
//       Access: Published
//  Description: Setter for property kSS_SPLT_CL.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_soft_vs_soft_impulse_split(float value) {

  _cfg.kSS_SPLT_CL = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_maxvolume
//       Access: Published
//  Description: Getter for property maxvolume.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_maxvolume() const {

  return _cfg.maxvolume;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_maxvolume
//       Access: Published
//  Description: Setter for property maxvolume.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_maxvolume(float value) {

  _cfg.maxvolume = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_timescale
//       Access: Published
//  Description: Getter for property timescale.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyConfig::
get_timescale() const {

  return _cfg.timescale;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_timescale
//       Access: Published
//  Description: Setter for property timescale.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_timescale(float value) {

  _cfg.timescale = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_positions_solver_iterations
//       Access: Published
//  Description: Getter for property piterations.
////////////////////////////////////////////////////////////////////
INLINE int BulletSoftBodyConfig::
get_positions_solver_iterations() const {

  return _cfg.piterations;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_positions_solver_iterations
//       Access: Published
//  Description: Setter for property piterations.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_positions_solver_iterations(int value) {

  nassertv(value > 0);
  _cfg.piterations = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_velocities_solver_iterations
//       Access: Published
//  Description: Getter for property viterations.
////////////////////////////////////////////////////////////////////
INLINE int BulletSoftBodyConfig::
get_velocities_solver_iterations() const {

  return _cfg.viterations;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_velocities_solver_iterations
//       Access: Published
//  Description: Setter for property viterations.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_velocities_solver_iterations(int value) {

  nassertv(value > 0);
  _cfg.viterations = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_drift_solver_iterations
//       Access: Published
//  Description: Getter for property diterations.
////////////////////////////////////////////////////////////////////
INLINE int BulletSoftBodyConfig::
get_drift_solver_iterations() const {

  return _cfg.diterations;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_drift_solver_iterations
//       Access: Published
//  Description: Setter for property diterations.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_drift_solver_iterations(int value) {

  nassertv(value > 0);
  _cfg.diterations = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_cluster_solver_iterations
//       Access: Published
//  Description: Getter for property citerations.
////////////////////////////////////////////////////////////////////
INLINE int BulletSoftBodyConfig::
get_cluster_solver_iterations() const {

  return _cfg.citerations;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_cluster_solver_iterations
//       Access: Published
//  Description: Setter for property citerations.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyConfig::
set_cluster_solver_iterations(int value) {

  nassertv(value > 0);
  _cfg.citerations = value;
}

