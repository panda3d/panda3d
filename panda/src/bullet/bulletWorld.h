/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWorld.h
 * @author enn0x
 * @date 2010-01-23
 */

#ifndef __BULLET_WORLD_H__
#define __BULLET_WORLD_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "bulletClosestHitRayResult.h"
#include "bulletAllHitsRayResult.h"
#include "bulletClosestHitSweepResult.h"
#include "bulletContactResult.h"
#include "bulletDebugNode.h"
#include "bulletBaseCharacterControllerNode.h"
#include "bulletConstraint.h"
#include "bulletGhostNode.h"
#include "bulletRigidBodyNode.h"
#include "bulletSoftBodyNode.h"
#include "bulletVehicle.h"

#include "typedReferenceCount.h"
#include "transformState.h"
#include "pandaNode.h"
#include "callbackObject.h"
#include "collideMask.h"
#include "luse.h"
#include "lightMutex.h"

class BulletPersistentManifold;
class BulletShape;
class BulletSoftBodyWorldInfo;

extern EXPCL_PANDABULLET PT(CallbackObject) bullet_contact_added_callback;

/**
 *
 */
class EXPCL_PANDABULLET BulletWorld : public TypedReferenceCount {

PUBLISHED:
  BulletWorld();
  INLINE ~BulletWorld();

  void set_gravity(const LVector3 &gravity);
  void set_gravity(PN_stdfloat gx, PN_stdfloat gy, PN_stdfloat gz);
  const LVector3 get_gravity() const;

  BLOCKING int do_physics(PN_stdfloat dt, int max_substeps=1, PN_stdfloat stepsize=1.0f/60.0f);

  BulletSoftBodyWorldInfo get_world_info();

  // Debug
  void set_debug_node(BulletDebugNode *node);
  void clear_debug_node();
  INLINE BulletDebugNode *get_debug_node() const;
  INLINE bool has_debug_node() const;

  // AttachRemove
  void attach(TypedObject *object);
  void remove(TypedObject *object);
  void attach_constraint(BulletConstraint *constraint, bool linked_collision=false);

  // Ghost object
  int get_num_ghosts() const;
  BulletGhostNode *get_ghost(int idx) const;
  MAKE_SEQ(get_ghosts, get_num_ghosts, get_ghost);

  // Rigid body
  int get_num_rigid_bodies() const;
  BulletRigidBodyNode *get_rigid_body(int idx) const;
  MAKE_SEQ(get_rigid_bodies, get_num_rigid_bodies, get_rigid_body);

  // Soft body
  int get_num_soft_bodies() const;
  BulletSoftBodyNode *get_soft_body(int idx) const;
  MAKE_SEQ(get_soft_bodies, get_num_soft_bodies, get_soft_body);

  // Character controller
  int get_num_characters() const;
  BulletBaseCharacterControllerNode *get_character(int idx) const;
  MAKE_SEQ(get_characters, get_num_characters, get_character);

  int get_num_vehicles() const;
  BulletVehicle *get_vehicle(int idx) const;
  MAKE_SEQ(get_vehicles, get_num_vehicles, get_vehicle);

  // Constraint
  int get_num_constraints() const;
  BulletConstraint *get_constraint(int idx) const;
  MAKE_SEQ(get_constraints, get_num_constraints, get_constraint);

  // Raycast and other queries
  BulletClosestHitRayResult ray_test_closest(
    const LPoint3 &from_pos,
    const LPoint3 &to_pos,
    const CollideMask &mask=CollideMask::all_on()) const;

  BulletAllHitsRayResult ray_test_all(
    const LPoint3 &from_pos,
    const LPoint3 &to_pos,
    const CollideMask &mask=CollideMask::all_on()) const;

  BulletClosestHitSweepResult sweep_test_closest(
    BulletShape *shape,
    const TransformState &from_ts,
    const TransformState &to_ts,
    const CollideMask &mask=CollideMask::all_on(),
    PN_stdfloat penetration=0.0f) const;

  BulletContactResult contact_test(PandaNode *node, bool use_filter=false) const;
  BulletContactResult contact_test_pair(PandaNode *node0, PandaNode *node1) const;

  bool filter_test(PandaNode *node0, PandaNode *node1) const;

  // Manifolds
  int get_num_manifolds() const;
  BulletPersistentManifold *get_manifold(int idx) const;
  MAKE_SEQ(get_manifolds, get_num_manifolds, get_manifold);

  // Collision filtering
  void set_group_collision_flag(unsigned int group1, unsigned int group2, bool enable);
  bool get_group_collision_flag(unsigned int group1, unsigned int group2) const;

  void set_force_update_all_aabbs(bool force);
  bool get_force_update_all_aabbs() const;

  // Callbacks
  void set_contact_added_callback(CallbackObject *obj);
  void clear_contact_added_callback();

  void set_tick_callback(CallbackObject *obj, bool is_pretick=false);
  void clear_tick_callback();

  void set_filter_callback(CallbackObject *obj);
  void clear_filter_callback();

  // Configuration
  enum BroadphaseAlgorithm {
    BA_sweep_and_prune,
    BA_dynamic_aabb_tree,
  };

  enum FilterAlgorithm {
    FA_mask,
    FA_groups_mask,
    FA_callback,
  };

  MAKE_PROPERTY(gravity, get_gravity, set_gravity);
  MAKE_PROPERTY(world_info, get_world_info);
  MAKE_PROPERTY2(debug_node, has_debug_node, get_debug_node, set_debug_node, clear_debug_node);
  MAKE_SEQ_PROPERTY(ghosts, get_num_ghosts, get_ghost);
  MAKE_SEQ_PROPERTY(rigid_bodies, get_num_rigid_bodies, get_rigid_body);
  MAKE_SEQ_PROPERTY(soft_bodies, get_num_soft_bodies, get_soft_body);
  MAKE_SEQ_PROPERTY(characters, get_num_characters, get_character);
  MAKE_SEQ_PROPERTY(vehicles, get_num_vehicles, get_vehicle);
  MAKE_SEQ_PROPERTY(constraints, get_num_constraints, get_constraint);
  MAKE_SEQ_PROPERTY(manifolds, get_num_manifolds, get_manifold);
  MAKE_PROPERTY(force_update_all_aabbs, get_force_update_all_aabbs,
                                        set_force_update_all_aabbs);

PUBLISHED: // Deprecated methods, will be removed soon
  void attach_ghost(BulletGhostNode *node);
  void remove_ghost(BulletGhostNode *node);

  void attach_rigid_body(BulletRigidBodyNode *node);
  void remove_rigid_body(BulletRigidBodyNode *node);

  void attach_soft_body(BulletSoftBodyNode *node);
  void remove_soft_body(BulletSoftBodyNode *node);

  void attach_character(BulletBaseCharacterControllerNode *node);
  void remove_character(BulletBaseCharacterControllerNode *node);

  void attach_vehicle(BulletVehicle *vehicle);
  void remove_vehicle(BulletVehicle *vehicle);

  void remove_constraint(BulletConstraint *constraint);

public:
  static btCollisionObject *get_collision_object(PandaNode *node);

  INLINE btDynamicsWorld *get_world() const;
  INLINE btBroadphaseInterface *get_broadphase() const;
  INLINE btDispatcher *get_dispatcher() const;

  static LightMutex &get_global_lock();

private:
  void do_sync_p2b(PN_stdfloat dt, int num_substeps);
  void do_sync_b2p();

  void do_attach_ghost(BulletGhostNode *node);
  void do_remove_ghost(BulletGhostNode *node);

  void do_attach_rigid_body(BulletRigidBodyNode *node);
  void do_remove_rigid_body(BulletRigidBodyNode *node);

  void do_attach_soft_body(BulletSoftBodyNode *node);
  void do_remove_soft_body(BulletSoftBodyNode *node);

  void do_attach_character(BulletBaseCharacterControllerNode *node);
  void do_remove_character(BulletBaseCharacterControllerNode *node);

  void do_attach_vehicle(BulletVehicle *vehicle);
  void do_remove_vehicle(BulletVehicle *vehicle);

  void do_attach_constraint(BulletConstraint *constraint, bool linked_collision=false);
  void do_remove_constraint(BulletConstraint *constraint);

  static void tick_callback(btDynamicsWorld *world, btScalar timestep);

  typedef PTA(PT(BulletRigidBodyNode)) BulletRigidBodies;
  typedef PTA(PT(BulletSoftBodyNode)) BulletSoftBodies;
  typedef PTA(PT(BulletGhostNode)) BulletGhosts;
  typedef PTA(PT(BulletBaseCharacterControllerNode)) BulletCharacterControllers;
  typedef PTA(PT(BulletVehicle)) BulletVehicles;
  typedef PTA(PT(BulletConstraint)) BulletConstraints;

  static PStatCollector _pstat_physics;
  static PStatCollector _pstat_simulation;
  static PStatCollector _pstat_p2b;
  static PStatCollector _pstat_b2p;

  struct btFilterCallback1 : public btOverlapFilterCallback {
    virtual bool needBroadphaseCollision(
      btBroadphaseProxy* proxy0,
      btBroadphaseProxy* proxy1) const;
  };

  struct btFilterCallback2 : public btOverlapFilterCallback {
    virtual bool needBroadphaseCollision(
      btBroadphaseProxy* proxy0,
      btBroadphaseProxy* proxy1) const;

    CollideMask _collide[32];
  };

  struct btFilterCallback3 : public btOverlapFilterCallback {
    virtual bool needBroadphaseCollision(
      btBroadphaseProxy* proxy0,
      btBroadphaseProxy* proxy1) const;

    PT(CallbackObject) _filter_callback_obj;
  };

  btBroadphaseInterface *_broadphase;
  btCollisionConfiguration *_configuration;
  btCollisionDispatcher *_dispatcher;
  btConstraintSolver *_solver;
  btSoftRigidDynamicsWorld *_world;

  btGhostPairCallback _ghost_cb;

  FilterAlgorithm _filter_algorithm;
  btFilterCallback1 _filter_cb1;
  btFilterCallback2 _filter_cb2;
  btFilterCallback3 _filter_cb3;
  btOverlapFilterCallback *_filter_cb;

  PT(CallbackObject) _tick_callback_obj;
  PT(CallbackObject) _contact_added_callback_obj;

  PT(BulletDebugNode) _debug;

  btSoftBodyWorldInfo _info;

  BulletRigidBodies _bodies;
  BulletSoftBodies _softbodies;
  BulletGhosts _ghosts;
  BulletCharacterControllers _characters;
  BulletVehicles _vehicles;
  BulletConstraints _constraints;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BulletWorld",
                  TypedReferenceCount::get_class_type());
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

EXPCL_PANDABULLET std::ostream &
operator << (std::ostream &out, BulletWorld::BroadphaseAlgorithm algorithm);
EXPCL_PANDABULLET std::istream &
operator >> (std::istream &in, BulletWorld::BroadphaseAlgorithm &algorithm);

EXPCL_PANDABULLET std::ostream &
operator << (std::ostream &out, BulletWorld::FilterAlgorithm algorithm);
EXPCL_PANDABULLET std::istream &
operator >> (std::istream &in, BulletWorld::FilterAlgorithm &algorithm);

#include "bulletWorld.I"

#endif // __BULLET_WORLD_H__
