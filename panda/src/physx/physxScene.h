/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxScene.h
 * @author enn0x
 * @date 2009-09-14
 */

#ifndef PHYSXSCENE_H
#define PHYSXSCENE_H

#include "pandabase.h"
#include "luse.h"
#include "callbackObject.h"

#include "physxObject.h"
#include "physxObjectCollection.h"
#include "physxEnums.h"
#include "physxContactReport.h"
#include "physxControllerReport.h"
#include "physxTriggerReport.h"
#include "physxOverlapReport.h"
#include "physxMask.h"
#include "physxGroupsMask.h"
#include "physx_includes.h"

class PhysxActor;
class PhysxActorDesc;
class PhysxController;
class PhysxControllerDesc;
class PhysxConstraintDominance;
class PhysxDebugGeomNode;
class PhysxForceField;
class PhysxForceFieldDesc;
class PhysxForceFieldShapeGroup;
class PhysxForceFieldShapeGroupDesc;
class PhysxJoint;
class PhysxJointDesc;
class PhysxMaterial;
class PhysxMaterialDesc;
class PhysxOverlapReport;
class PhysxRay;
class PhysxRaycastHit;
class PhysxRaycastReport;
class PhysxSceneStats2;
class PhysxVehicle;
class PhysxVehicleDesc;
class PhysxCloth;
class PhysxClothDesc;
class PhysxSoftBody;
class PhysxSoftBodyDesc;

/**
 * A scene is a collection of bodies, constraints, and effectors which can
 * interact.
 *
 * The scene simulates the behavior of these objects over time.  Several
 * scenes may exist at the same time, but each body, constraint, or effector
 * object is specific to a scene -- they may not be shared.
 *
 * For example, attempting to create a joint in one scene and then using it to
 * attach bodies from a different scene results in undefined behavior.
 */
class EXPCL_PANDAPHYSX PhysxScene : public PhysxObject, public PhysxEnums {

PUBLISHED:
  INLINE PhysxScene();
  INLINE ~PhysxScene();

  void simulate(float dt);
  void fetch_results();
  void set_timing_variable();
  void set_timing_fixed(float maxTimestep=1.0f/60.0f, unsigned int maxIter=8);

  PhysxDebugGeomNode *get_debug_geom_node();

  void enable_contact_reporting(bool enabled);
  bool is_contact_reporting_enabled() const;
  void enable_trigger_reporting(bool enabled);
  bool is_trigger_reporting_enabled() const;
  void enable_controller_reporting(bool enabled);
  bool is_controller_reporting_enabled() const;

  INLINE void set_controller_shape_hit_callback(PT(CallbackObject) cbobj);
  INLINE void set_controller_controller_hit_callback(PT(CallbackObject) cbobj);

  void set_gravity(const LVector3f &gravity);

  LVector3f get_gravity() const;
  PhysxSceneStats2 get_stats2() const;
  bool get_flag(PhysxSceneFlag flag) const;
  bool is_hardware_scene() const;

  // Actors
  unsigned int get_num_actors() const;
  PhysxActor *create_actor(PhysxActorDesc &desc);
  PhysxActor *get_actor(unsigned int idx) const;
  MAKE_SEQ(get_actors, get_num_actors, get_actor);

  // Joints
  unsigned int get_num_joints() const;
  PhysxJoint *create_joint(PhysxJointDesc &desc);
  PhysxJoint *get_joint(unsigned int idx) const;
  MAKE_SEQ(get_joints, get_num_joints, get_joint);

  // Materials
  unsigned int get_num_materials() const;
  unsigned int get_hightest_material_index() const;
  PhysxMaterial *create_material(PhysxMaterialDesc &desc);
  PhysxMaterial *create_material();
  PhysxMaterial *get_material(unsigned int idx) const;
  PhysxMaterial *get_material_from_index(unsigned int idx) const;
  MAKE_SEQ(get_materials, get_num_materials, get_material);

  // Controllers
  unsigned int get_num_controllers() const;
  PhysxController *create_controller(PhysxControllerDesc &controllerDesc);
  PhysxController *get_controller(unsigned int idx) const;
  MAKE_SEQ(get_controllers, get_num_controllers, get_controller);

  // Force fields
  unsigned int get_num_force_fields() const;
  PhysxForceField *create_force_field(PhysxForceFieldDesc &desc);
  PhysxForceField *get_force_field(unsigned int idx) const;
  MAKE_SEQ(get_force_fields, get_num_force_fields, get_force_field);

  // Force field shape groups
  unsigned int get_num_force_field_shape_groups() const;
  PhysxForceFieldShapeGroup *create_force_field_shape_group(PhysxForceFieldShapeGroupDesc &desc);
  PhysxForceFieldShapeGroup *get_force_field_shape_group(unsigned int idx) const;
  MAKE_SEQ(get_force_field_shape_groups, get_num_force_field_shape_groups, get_force_field_shape_group);

  // Cloths
  unsigned int get_num_cloths() const;
  PhysxCloth *create_cloth(PhysxClothDesc &desc);
  PhysxCloth *get_cloth(unsigned int idx) const;
  MAKE_SEQ(get_cloths, get_num_cloths, get_cloth);

  // Soft bodies
  unsigned int get_num_soft_bodies() const;
  PhysxSoftBody *create_soft_body(PhysxSoftBodyDesc &desc);
  PhysxSoftBody *get_soft_body(unsigned int idx) const;
  MAKE_SEQ(get_soft_bodies, get_num_soft_bodies, get_soft_body);

  // Vehicles
  unsigned int get_num_vehicles() const;
  PhysxVehicle *create_vehicle(PhysxVehicleDesc &desc);
  PhysxVehicle *get_vehicle(unsigned int idx) const;
  MAKE_SEQ(get_vehicles, get_num_vehicles, get_vehicle);

  // Raycast queries
  bool raycast_any_shape(const PhysxRay &ray,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(),
    PhysxGroupsMask *groups=nullptr) const;

  PhysxRaycastHit raycast_closest_shape(const PhysxRay &ray,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(),
    PhysxGroupsMask *groups=nullptr, bool smoothNormal=true) const;

  PhysxRaycastReport raycast_all_shapes(const PhysxRay &ray,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(),
    PhysxGroupsMask *groups=nullptr, bool smoothNormal=true) const;

  bool raycast_any_bounds(const PhysxRay &ray,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(),
    PhysxGroupsMask *groups=nullptr) const;

  PhysxRaycastHit raycast_closest_bounds(const PhysxRay &ray,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(),
    PhysxGroupsMask *groups=nullptr, bool smoothNormal=true) const;

  PhysxRaycastReport raycast_all_bounds(const PhysxRay &ray,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(),
    PhysxGroupsMask *groups=nullptr, bool smoothNormal=true) const;

  // Overlap queries
  PhysxOverlapReport overlap_sphere_shapes(const LPoint3f &center, float radius,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(), bool accurateCollision=true) const;

  PhysxOverlapReport overlap_capsule_shapes(const LPoint3f &p0, const LPoint3f &p1, float radius,
    PhysxShapesType shapesType=ST_all,
    PhysxMask mask=PhysxMask::all_on(), bool accurateCollision=true) const;

  // Filters
  void set_actor_pair_flag(PhysxActor &actorA, PhysxActor &actorB, PhysxContactPairFlag flag, bool value);
  void set_shape_pair_flag(PhysxShape &shapeA, PhysxShape &shapeB, bool value);
  void set_actor_group_pair_flag(unsigned int g1, unsigned int g2, PhysxContactPairFlag flag, bool value);
  void set_group_collision_flag(unsigned int g1, unsigned int g2, bool enable);
  void set_filter_ops(PhysxFilterOp op0, PhysxFilterOp op1, PhysxFilterOp op2);
  void set_filter_bool(bool flag);
  void set_filter_constant0(const PhysxGroupsMask &mask);
  void set_filter_constant1(const PhysxGroupsMask &mask);
  void set_dominance_group_pair(unsigned int g1, unsigned int g2, PhysxConstraintDominance dominance);

  bool get_actor_pair_flag(PhysxActor &actorA, PhysxActor &actorB, PhysxContactPairFlag flag);
  bool get_shape_pair_flag(PhysxShape &shapeA, PhysxShape &shapeB);
  bool get_actor_group_pair_flag(unsigned int g1, unsigned int g2, PhysxContactPairFlag flag);
  bool get_group_collision_flag(unsigned int g1, unsigned int g2);
  bool get_filter_bool() const;
  PhysxGroupsMask get_filter_constant0() const;
  PhysxGroupsMask get_filter_constant1() const;
  PhysxFilterOp get_filter_op0() const;
  PhysxFilterOp get_filter_op1() const;
  PhysxFilterOp get_filter_op2() const;
  PhysxConstraintDominance get_dominance_group_pair(unsigned int g1, unsigned int g2);

PUBLISHED:
  void release();

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  INLINE NxScene *ptr() const { return _ptr; };
  INLINE NxControllerManager *cm() const { return _cm; };

  void link(NxScene *ptr);
  void unlink();

  PhysxObjectCollection<PhysxMaterial> _materials;
  PhysxObjectCollection<PhysxActor> _actors;
  PhysxObjectCollection<PhysxJoint> _joints;
  PhysxObjectCollection<PhysxForceField> _forcefields;
  PhysxObjectCollection<PhysxForceFieldShapeGroup> _ffgroups;
  PhysxObjectCollection<PhysxController> _controllers;
  PhysxObjectCollection<PhysxVehicle> _vehicles;
  PhysxObjectCollection<PhysxCloth> _cloths;
  PhysxObjectCollection<PhysxSoftBody> _softbodies;

  PhysxMaterial *get_wheel_shape_material();

private:
  NxScene *_ptr;
  NxControllerManager *_cm;
  PT(PhysxDebugGeomNode) _debugNode;
  PT(PhysxMaterial) _wheelShapeMaterial;

  PhysxContactReport _contact_report;
  PhysxControllerReport _controller_report;
  PhysxTriggerReport _trigger_report;

  static PStatCollector _pcollector_fetch_results;
  static PStatCollector _pcollector_update_transforms;
  static PStatCollector _pcollector_debug_renderer;
  static PStatCollector _pcollector_simulate;
  static PStatCollector _pcollector_cloth;
  static PStatCollector _pcollector_softbody;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxScene",
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

#include "physxScene.I"

#endif // PHYSXSCENE_H
