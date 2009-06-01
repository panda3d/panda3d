// Filename: physxActorNode.h
// Created by:  pratt (Apr 7, 2006)
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

#ifndef PHYSXACTORNODE_H
#define PHYSXACTORNODE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"
#include "pandaNode.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxBodyDesc;
class PhysxScene;
class PhysxShape;
class PhysxShapeDesc;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxActorNode
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxActorNode : public PandaNode {
PUBLISHED:
  PhysxActorNode(const string &name);

  INLINE void update_transform();
  INLINE void get_global_pose_optimized(LMatrix4f *result);

  unsigned int get_num_shapes();
  PhysxShape *get_shape(unsigned int i);
  INLINE bool is_valid();

  void add_force(const LVecBase3f & force, PhysxForceMode mode, bool wakeup=true);
  void add_force_at_local_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup=true);
  void add_force_at_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup=true);
  void add_local_force(const LVecBase3f & force, PhysxForceMode mode, bool wakeup=true);
  void add_local_force_at_local_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup=true);
  void add_local_force_at_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup=true);
  void add_local_torque(const LVecBase3f & torque, PhysxForceMode mode, bool wakeup=true);
  void add_torque(const LVecBase3f & torque, PhysxForceMode mode, bool wakeup=true);
  void clear_actor_flag(PhysxActorFlag actor_flag);
  void clear_body_flag(PhysxBodyFlag body_flag);
  float compute_kinetic_energy() const;
  PhysxShape * create_shape(const PhysxShapeDesc & desc);
  float get_angular_damping() const;
  LVecBase3f get_angular_momentum() const;
  LVecBase3f get_angular_velocity() const;
  LMatrix3f get_c_mass_global_orientation() const;
  LMatrix4f get_c_mass_global_pose() const;
  LVecBase3f get_c_mass_global_position() const;
  LMatrix3f get_c_mass_local_orientation() const;
  LMatrix4f get_c_mass_local_pose() const;
  LVecBase3f get_c_mass_local_position() const;
  float get_ccd_motion_threshold() const;
  LMatrix3f get_global_inertia_tensor() const;
  LMatrix3f get_global_inertia_tensor_inverse() const;
  LMatrix3f get_global_orientation() const;
  LQuaternionf get_global_orientation_quat() const;
  INLINE LMatrix4f get_global_pose() const;
  LVecBase3f get_global_position() const;
  unsigned short get_group() const;
  float get_linear_damping() const;
  LVecBase3f get_linear_momentum() const;
  LVecBase3f get_linear_velocity() const;
  LVecBase3f get_local_point_velocity(const LVecBase3f & point) const;
  float get_mass() const;
  LVecBase3f get_mass_space_inertia_tensor() const;
  float get_max_angular_velocity() const;
  unsigned int get_nb_shapes() const;
  LVecBase3f get_point_velocity(const LVecBase3f & point) const;
  PhysxScene & get_scene() const;
  float get_sleep_angular_velocity() const;
  float get_sleep_energy_threshold() const;
  float get_sleep_linear_velocity() const;
  unsigned int get_solver_iteration_count() const;
  bool is_dynamic() const;
  bool is_group_sleeping() const;
  bool is_sleeping() const;
  void move_global_orientation(const LMatrix3f & mat);
  void move_global_orientation_quat(const LQuaternionf & quat);
  void move_global_pose(const LMatrix4f & mat);
  void move_global_position(const LVecBase3f & vec);
  void put_to_sleep();
  void raise_actor_flag(PhysxActorFlag actor_flag);
  void raise_body_flag(PhysxBodyFlag body_flag);
  bool read_actor_flag(PhysxActorFlag actor_flag) const;
  bool read_body_flag(PhysxBodyFlag body_flag) const;
  void recompute_adaptive_force_counters();
  void release_shape(PhysxShape & shape);
  void reset_user_actor_pair_filtering();
  bool save_body_to_desc(PhysxBodyDesc & body_desc);
  void set_angular_damping(float ang_damp);
  void set_angular_momentum(const LVecBase3f & ang_moment);
  void set_angular_velocity(const LVecBase3f & ang_vel);
  void set_c_mass_global_orientation(const LMatrix3f & mat);
  void set_c_mass_global_pose(const LMatrix4f & mat);
  void set_c_mass_global_position(const LVecBase3f & vec);
  void set_c_mass_offset_global_orientation(const LMatrix3f & mat);
  void set_c_mass_offset_global_pose(const LMatrix4f & mat);
  void set_c_mass_offset_global_position(const LVecBase3f & vec);
  void set_c_mass_offset_local_orientation(const LMatrix3f & mat);
  void set_c_mass_offset_local_pose(const LMatrix4f & mat);
  void set_c_mass_offset_local_position(const LVecBase3f & vec);
  void set_ccd_motion_threshold(float thresh);
  void set_global_orientation(const LMatrix3f & mat);
  void set_global_orientation_quat(const LQuaternionf & mat);
  void set_global_pose(const LMatrix4f & mat);
  void set_global_position(const LVecBase3f & vec);
  void set_group(unsigned short actor_group);
  void set_linear_damping(float lin_damp);
  void set_linear_momentum(const LVecBase3f & lin_moment);
  void set_linear_velocity(const LVecBase3f & lin_vel);
  void set_mass(float mass);
  void set_mass_space_inertia_tensor(const LVecBase3f & m);
  void set_max_angular_velocity(float max_ang_vel);
  void set_sleep_angular_velocity(float threshold);
  void set_sleep_energy_threshold(float threshold);
  void set_sleep_linear_velocity(float threshold);
  void set_solver_iteration_count(unsigned int iter_count);
  void update_mass_from_shapes(float density, float total_mass);
  void wake_up(float wake_counter_value);


public:
  NxActor *nActor;

  bool safe_to_flatten() const {
    return false;
  }

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PhysxActorNode", PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  virtual void transform_changed();
  bool _disable_transform_changed;

  // These variables are used by update_transform and
  // get_global_pose_optimized for optimization purposes.
  LMatrix4f _tempMat4;
  float _tempCells[16];

  static TypeHandle _type_handle;
};

#include "physxActorNode.I"

#endif // HAVE_PHYSX

#endif // PHYSXACTORNODE_H
