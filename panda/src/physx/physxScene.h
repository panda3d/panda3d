// Filename: physxScene.h
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

#ifndef PHYSXSCENE_H
#define PHYSXSCENE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"
#include "pvector.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"
#include "physxContactHandler.h"
#include "physxTriggerHandler.h"
#include "physxJointHandler.h"

class PhysxActorNode;
class PhysxActorDesc;
class PhysxJoint;
class PhysxJointDesc;
class PhysxMaterial;
class PhysxMaterialDesc;
class PhysxSceneDesc;
class PhysxSceneStats2;
class PhysxShape;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxScene
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxScene {
PUBLISHED:
  void update_panda_graph();
  PT(PhysxActorNode) get_actor(unsigned int index);
  void set_contact_reporting_enabled(bool enabled);
  bool is_contact_reporting_enabled();
  void set_trigger_reporting_enabled(bool enabled);
  bool is_trigger_reporting_enabled();
  void set_joint_reporting_enabled(bool enabled);
  bool is_joint_reporting_enabled();
  void set_contact_reporting_threshold(float threshold);
  float get_contact_reporting_threshold();

  bool check_results(PhysxSimulationStatus status, bool block);
  PT(PhysxActorNode) create_actor(const PhysxActorDesc & desc, const string &name="");
  PhysxJoint *create_joint(const PhysxJointDesc & desc);
  PhysxMaterial *create_material(const PhysxMaterialDesc & desc);
  void flush_caches();
  void flush_stream();
  unsigned int get_actor_group_pair_flags(unsigned short group1, unsigned short group2) const;
  unsigned int get_actor_pair_flags(PhysxActorNode & actor_a, PhysxActorNode & actor_b) const;
  unsigned int get_bound_for_island_size(PhysxActorNode & actor);
  bool get_filter_bool() const;
  unsigned int get_flags() const;
  void get_gravity(LVecBase3f & vec);
  bool get_group_collision_flag(unsigned short group1, unsigned short group2) const;
  unsigned short get_highest_material_index() const;
  PhysxMaterial * get_material_from_index(unsigned short mat_index);
  float get_max_cpu_for_load_balancing();
  unsigned int get_nb_actor_group_pairs() const;
  unsigned int get_nb_actors() const;
  unsigned int get_nb_cloths() const;
  unsigned int get_nb_compartments() const;
  unsigned int get_nb_dynamic_shapes() const;
  unsigned int get_nb_effectors() const;
  unsigned int get_nb_fluids() const;
  unsigned int get_nb_force_fields() const;
  unsigned int get_nb_implicit_screen_meshes() const;
  unsigned int get_nb_joints() const;
  unsigned int get_nb_materials() const;
  unsigned int get_nb_pairs() const;
  unsigned int get_nb_soft_bodies() const;
  unsigned int get_nb_static_shapes() const;
  PhysxJoint * get_next_joint();
  unsigned int get_shape_pair_flags(PhysxShape & shape_a, PhysxShape & shape_b) const;
  PhysxSimulationType get_sim_type() const;
  PhysxSceneStats2 get_stats2() const;
  void get_timing(float & max_timestep, unsigned int & max_iter, PhysxTimeStepMethod & method, unsigned int * num_sub_steps) const;
  unsigned int get_total_nb_shapes() const;
  bool is_writable();
  void lock_queries();
  PhysxThreadPollResult poll_for_background_work(PhysxThreadWait wait_type);
  PhysxThreadPollResult poll_for_work(PhysxThreadWait wait_type);
  void release_actor(PhysxActorNode & pActor);
  void release_joint(PhysxJoint & pJoint);
  void release_material(PhysxMaterial & pMaterial);
  void reset_effector_iterator();
  void reset_joint_iterator();
  void reset_poll_for_work();
  bool save_to_desc(PhysxSceneDesc & desc) const;
  void set_actor_group_pair_flags(unsigned short group1, unsigned short group2, unsigned int flags);
  void set_actor_pair_flags(PhysxActorNode & actor_a, PhysxActorNode & actor_b, unsigned int nx_contact_pair_flag);
  void set_filter_bool(bool flag);
  void set_filter_ops(PhysxFilterOp op0, PhysxFilterOp op1, PhysxFilterOp op2);
  void set_gravity(const LVecBase3f & vec);
  void set_group_collision_flag(unsigned short group1, unsigned short group2, bool enable);
  void set_max_cpu_for_load_balancing(float cpu_fraction);
  void set_shape_pair_flags(PhysxShape & shape_a, PhysxShape & shape_b, unsigned int nx_contact_pair_flag);
  void set_timing(float max_timestep, unsigned int max_iter, PhysxTimeStepMethod method);
  void shutdown_worker_threads();
  void simulate(float elapsed_time);
  void unlock_queries();

public:
  NxScene *nScene;

private:
  // Keep lists of all reference counted Physx objects so that they won't be
  // inadvertantly destroyed before they've been released.
  pvector<PT(PhysxActorNode)> _actors;
  pvector<PT(PhysxJoint)> _joints;
  pvector<PT(PhysxShape)> _shapes;

  bool contact_reporting_enabled;
  PhysxContactHandler contact_handler;
  bool trigger_reporting_enabled;
  PhysxTriggerHandler trigger_handler;
  bool joint_reporting_enabled;
  PhysxJointHandler joint_handler;

  static PStatCollector _update_pcollector;
  static PStatCollector _fetch_results_pcollector;
  static PStatCollector _update_transforms_pcollector;
  static PStatCollector _simulate_pcollector;
  static PStatCollector _flush_stream_pcollector;
  static PStatCollector _contact_reporting_pcollector;
  static PStatCollector _trigger_reporting_pcollector;
  static PStatCollector _joint_reporting_pcollector;
};

#include "physxScene.I"

#endif // HAVE_PHYSX

#endif // PHYSXSCENE_H
