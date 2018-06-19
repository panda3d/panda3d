/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxActor.h
 * @author enn0x
 * @date 2009-09-14
 */

#ifndef PHYSXACTOR_H
#define PHYSXACTOR_H

#include "pandabase.h"
#include "nodePath.h"
#include "luse.h"

#include "physxObject.h"
#include "physxObjectCollection.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxController;
class PhysxScene;
class PhysxShape;
class PhysxShapeDesc;
class PhysxActorDesc;
class PhysxBodyDesc;

/**
 * Actors are the main simulation objects.  Actors are owned by a scene
 * (PhysxScene).
 *
 * An actor may optionally encapsulate a dynamic rigid body by setting the
 * body member of the actor's descriptor when it is created.  Otherwise the
 * actor will be static (fixed in the world).
 *
 * Instances of PhysxActor are created by calling PhysxScene::create_actor()
 * and destroyed by calling PhysxActor::release().
 */
class EXPCL_PANDAPHYSX PhysxActor : public PhysxObject, public PhysxEnums {

PUBLISHED:
  INLINE PhysxActor();
  INLINE ~PhysxActor();

  bool save_body_to_desc(PhysxBodyDesc &bodyDesc) const;
  void save_to_desc(PhysxActorDesc &actorDesc) const;

  void set_name(const char *name);
  void set_global_pos(const LPoint3f &pos);
  void set_global_mat(const LMatrix4f &mat);
  void set_global_hpr(float h, float p, float r);
  void set_body_flag(PhysxBodyFlag flag, bool value);
  void set_actor_flag(PhysxActorFlag flag, bool value);
  void set_contact_report_flag(PhysxContactPairFlag flag, bool value);
  void set_contact_report_threshold(float threshold);
  void set_group(unsigned int group);
  void set_dominance_group(unsigned int group);
  void set_shape_group( unsigned int group );

  const char *get_name() const;
  LPoint3f get_global_pos() const;
  LMatrix4f get_global_mat() const;
  LQuaternionf get_global_quat() const;
  bool get_body_flag(PhysxBodyFlag flag) const;
  bool get_actor_flag(PhysxActorFlag flag) const;
  unsigned int get_group() const;
  unsigned int get_dominance_group() const;

  bool is_dynamic() const;
  float compute_kinetic_energy() const;
  bool update_mass_from_shapes(float density, float totalMass);

  PhysxScene *get_scene() const;

  // NodePath
  void attach_node_path(const NodePath &np);
  void detach_node_path();
  NodePath get_node_path() const;

  // Shapes
  unsigned int get_num_shapes() const;
  PhysxShape *create_shape(PhysxShapeDesc &desc);
  PhysxShape *get_shape(unsigned int idx) const;
  PhysxShape *get_shape_by_name(const char *name) const;
  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);

  // Forces
  void add_force(const LVector3f force,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_force_at_pos(const LVector3f force, const LPoint3f &pos,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_force_at_local_pos(const LVector3f force, const LPoint3f &pos,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_torque(const LVector3f torque,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_local_force(const LVector3f force,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_local_force_at_pos(const LVector3f force, const LPoint3f &pos,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_local_force_at_local_pos(const LVector3f force, const LPoint3f &pos,
     PhysxForceMode mode=FM_force, bool wakeup=true);
  void add_local_torque(const LVector3f torque,
     PhysxForceMode mode=FM_force, bool wakeup=true);

  // Mass manipulation
  void set_mass(float mass);
  void set_c_mass_offset_local_mat(const LMatrix4f &mat);
  void set_c_mass_offset_local_pos(const LPoint3f &pos);
  void set_c_mass_offset_local_orientation(const LMatrix3f &mat);
  void set_c_mass_offset_global_mat(const LMatrix4f &mat);
  void set_c_mass_offset_global_pos(const LPoint3f &pos);
  void set_c_mass_offset_global_orientation(const LMatrix3f &mat);
  void set_c_mass_global_mat(const LMatrix4f &mat);
  void set_c_mass_global_pos(const LPoint3f &pos);
  void set_c_mass_global_orientation(const LMatrix3f &mat);
  void set_mass_space_inertia_tensor(const LVector3f &m);

  float get_mass() const;
  LMatrix4f get_c_mass_global_mat() const;
  LPoint3f get_c_mass_global_pos() const;
  LMatrix3f get_c_mass_global_orientation() const;
  LMatrix4f get_c_mass_local_mat() const;
  LPoint3f get_c_mass_local_pos() const;
  LMatrix3f get_c_mass_local_orientation() const;
  LVector3f get_mass_space_inertia_tensor() const;
  LMatrix3f get_global_inertia_tensor() const;
  LMatrix3f get_global_inertia_tensor_inverse() const;

  // Damping
  void set_linear_damping(float linDamp);
  void set_angular_damping(float angDamp);
  float get_linear_damping() const;
  float get_angular_damping() const;

  // Velocity
  void set_linear_velocity(const LVector3f &linVel);
  void set_angular_velocity(const LVector3f &angVel);
  void set_max_angular_velocity(float maxAngVel);

  LVector3f get_linear_velocity() const;
  LVector3f get_angular_velocity() const;
  float get_max_angular_velocity() const;

  // Point Velocity
  LVector3f get_point_velocity(const LPoint3f &point) const;
  LVector3f get_local_point_velocity(const LPoint3f &point) const;

  // Momentum
  void set_linear_momentum(const LVector3f &momentum);
  void set_angular_momentum(const LVector3f &momentum);
  LVector3f get_linear_momentum() const;
  LVector3f get_angular_momentum() const;

  // Sleeping
  void set_sleep_linear_velocity(float threshold);
  void set_sleep_angular_velocity(float threshold);
  void set_sleep_energy_threshold(float threshold);
  float get_sleep_linear_velocity() const;
  float get_sleep_angular_velocity() const;
  float get_sleep_energy_threshold() const;
  bool is_sleeping() const;
  void wake_up(float wakeCounterValue=NX_SLEEP_INTERVAL);
  void put_to_sleep();

  // Kinematic
  void move_global_pos(const LPoint3f &pos);
  void move_global_mat(const LMatrix4f &mat);
  void move_global_hpr(float h, float p, float r);

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  void update_transform(const LMatrix4f &m);

PUBLISHED:
  void release();

public:
  INLINE NxActor *ptr() const { return _ptr; };

  void link_controller(PhysxController *controller);
  void link(NxActor *ptr);
  void unlink();

  PhysxObjectCollection<PhysxShape> _shapes;

private:
  NxActor *_ptr;
  NodePath _np;
  PT(PhysxController) _controller;
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxActor",
                  PhysxObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxActor.I"

#endif // PHYSXACTOR_H
