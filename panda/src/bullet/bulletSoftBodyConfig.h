// Filename: bulletSoftBodyConfig.h
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

#ifndef __BULLET_SOFT_BODY_CONFIG_H__
#define __BULLET_SOFT_BODY_CONFIG_H__

#include "pandabase.h"

#include "bullet_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletSoftBodyConfig
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletSoftBodyConfig {

PUBLISHED:
  INLINE ~BulletSoftBodyConfig();

  enum CollisionFlag {
    CF_rigid_vs_soft_mask    = 0x000f, // RVSmask: Rigid versus soft mask
    CF_sdf_rigid_soft        = 0x0001, // SDF_RS:  SDF based rigid vs soft
    CF_cluster_rigid_soft    = 0x0002, // CL_RS:   Cluster vs convex rigid vs soft
    CF_soft_vs_soft_mask     = 0x0030, // SVSmask: Soft versus soft mask
    CF_vertex_face_soft_soft = 0x0010, // VF_SS:   Vertex vs face soft vs soft handling
    CF_cluster_soft_soft     = 0x0020, // CL_SS:   Cluster vs cluster soft vs soft handling
    CF_cluster_self          = 0x0040, // CL_SELF: Cluster soft body self collision
  };

  enum AeroModel {
    AM_vertex_point,     // V_Point:    Vertex normals are oriented toward velocity
    AM_vertex_two_sided, // V_TwoSided: Vertex normals are fliped to match velocity
    AM_vertex_one_sided, // V_OneSided: Vertex normals are taken as it is
    AM_face_two_sided,   // F_TwoSided: Face normals are fliped to match velocity
    AM_face_one_sided,   // F_OneSided: Face normals are taken as it is
  };

  void clear_all_collision_flags();
  void set_collision_flag(CollisionFlag flag, bool value);
  bool get_collision_flag(CollisionFlag flag) const;

  void set_aero_model(AeroModel value);
  AeroModel get_aero_model() const;

  INLINE void set_velocities_correction_factor(PN_stdfloat value);
  INLINE void set_damping_coefficient(PN_stdfloat value);
  INLINE void set_drag_coefficient(PN_stdfloat value);
  INLINE void set_lift_coefficient(PN_stdfloat value);
  INLINE void set_pressure_coefficient(PN_stdfloat value);
  INLINE void set_volume_conversation_coefficient(PN_stdfloat value);
  INLINE void set_dynamic_friction_coefficient(PN_stdfloat value);
  INLINE void set_pose_matching_coefficient(PN_stdfloat value);
  INLINE void set_rigid_contacts_hardness(PN_stdfloat value);
  INLINE void set_kinetic_contacts_hardness(PN_stdfloat value);
  INLINE void set_soft_contacts_hardness(PN_stdfloat value);
  INLINE void set_anchors_hardness(PN_stdfloat value);
  INLINE void set_soft_vs_rigid_hardness(PN_stdfloat value);
  INLINE void set_soft_vs_kinetic_hardness(PN_stdfloat value);
  INLINE void set_soft_vs_soft_hardness(PN_stdfloat value);
  INLINE void set_soft_vs_rigid_impulse_split(PN_stdfloat value);
  INLINE void set_soft_vs_kinetic_impulse_split(PN_stdfloat value);
  INLINE void set_soft_vs_soft_impulse_split(PN_stdfloat value);
  INLINE void set_maxvolume(PN_stdfloat value);
  INLINE void set_timescale(PN_stdfloat value);
  INLINE void set_positions_solver_iterations(int value);
  INLINE void set_velocities_solver_iterations(int value);
  INLINE void set_drift_solver_iterations( int value);
  INLINE void set_cluster_solver_iterations(int value);

  INLINE PN_stdfloat get_velocities_correction_factor() const;
  INLINE PN_stdfloat get_damping_coefficient() const;
  INLINE PN_stdfloat get_drag_coefficient() const;
  INLINE PN_stdfloat get_lift_coefficient() const;
  INLINE PN_stdfloat get_pressure_coefficient() const;
  INLINE PN_stdfloat get_volume_conversation_coefficient() const;
  INLINE PN_stdfloat get_dynamic_friction_coefficient() const;
  INLINE PN_stdfloat get_pose_matching_coefficient() const;
  INLINE PN_stdfloat get_rigid_contacts_hardness() const;
  INLINE PN_stdfloat get_kinetic_contacts_hardness() const;
  INLINE PN_stdfloat get_soft_contacts_hardness() const;
  INLINE PN_stdfloat get_anchors_hardness() const;
  INLINE PN_stdfloat get_soft_vs_rigid_hardness() const;
  INLINE PN_stdfloat get_soft_vs_kinetic_hardness() const;
  INLINE PN_stdfloat get_soft_vs_soft_hardness() const;
  INLINE PN_stdfloat get_soft_vs_rigid_impulse_split() const;
  INLINE PN_stdfloat get_soft_vs_kinetic_impulse_split() const;
  INLINE PN_stdfloat get_soft_vs_soft_impulse_split() const;
  INLINE PN_stdfloat get_maxvolume() const;
  INLINE PN_stdfloat get_timescale() const;
  INLINE int get_positions_solver_iterations() const;
  INLINE int get_velocities_solver_iterations() const;
  INLINE int get_drift_solver_iterations() const;
  INLINE int get_cluster_solver_iterations() const;

public:
  BulletSoftBodyConfig(btSoftBody::Config &cfg);

private:
  btSoftBody::Config &_cfg;
};

#include "bulletSoftBodyConfig.I"

#endif // __BULLET_SOFT_BODY_CONFIG_H__
