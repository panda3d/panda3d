// Filename: physxScene.cxx
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

#ifdef HAVE_PHYSX

#include "physxScene.h"

#include "luse.h"
#include "physxActorNode.h"
#include "physxActorDesc.h"
#include "physxJoint.h"
#include "physxJointDesc.h"
#include "physxMaterial.h"
#include "physxMaterialDesc.h"
#include "physxSceneDesc.h"
#include "physxSceneStats2.h"
#include "physxShape.h"

#include "pStatTimer.h"
#include "pStatClient.h"
#include "pStatCollector.h"

PStatCollector PhysxScene::_update_pcollector("App:PhysX:Update Panda Graph");
PStatCollector PhysxScene::_fetch_results_pcollector("App:PhysX:Update Panda Graph:Fetch Results");
PStatCollector PhysxScene::_update_transforms_pcollector("App:PhysX:Update Panda Graph:Update Transforms");
PStatCollector PhysxScene::_simulate_pcollector("App:PhysX:Simulate");
PStatCollector PhysxScene::_flush_stream_pcollector("App:PhysX:Flush Stream");
PStatCollector PhysxScene::_contact_reporting_pcollector("App:PhysX:Contact Reporting");
PStatCollector PhysxScene::_trigger_reporting_pcollector("App:PhysX:Trigger Reporting");
PStatCollector PhysxScene::_joint_reporting_pcollector("App:PhysX:Joint Reporting");

////////////////////////////////////////////////////////////////////
//     Function : update_panda_graph
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
update_panda_graph() {
  PStatTimer timer(_update_pcollector);

  _fetch_results_pcollector.start();
  nScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
  _fetch_results_pcollector.stop();

  _update_transforms_pcollector.start();
  PhysxActorNode *pActor;
  unsigned int numTransforms;
  NxActiveTransform *activeTransforms = nScene->getActiveTransforms(numTransforms);
  for(unsigned int i=0; i<numTransforms; ++i) {
    pActor = (PhysxActorNode *)activeTransforms[i].userData;
    if(pActor != NULL) {
      pActor->update_transform();
    }
  }

  _update_transforms_pcollector.stop();

  _contact_reporting_pcollector.start();
  if(contact_reporting_enabled) {
    contact_handler.throw_events();
  }
  contact_handler.clear_events();
  _contact_reporting_pcollector.stop();

  _trigger_reporting_pcollector.start();
  if(trigger_reporting_enabled) {
    trigger_handler.throw_events();
  }
  trigger_handler.clear_events();
  _trigger_reporting_pcollector.stop();

  _joint_reporting_pcollector.start();
  if(joint_reporting_enabled) {
    joint_handler.throw_events();
  }
  joint_handler.clear_events();
  _joint_reporting_pcollector.stop();
}

////////////////////////////////////////////////////////////////////
//     Function : get_actor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PT(PhysxActorNode) PhysxScene::
get_actor(unsigned int index) {
  if((index >= 0) && (index < get_nb_actors())) {
    NxActor **actors = nScene->getActors();
    return (PhysxActorNode *)(actors[index]->userData);
  } else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : create_actor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PT(PhysxActorNode)  PhysxScene::
create_actor(const PhysxActorDesc & desc, const string &name) {
  NxActor *nActor = nScene->createActor(desc.nActorDesc);
  if(nActor == NULL) {
    return NULL;
  }

  PT(PhysxActorNode) pActor = new PhysxActorNode(name);
  nActor->userData = pActor;
  pActor->nActor = nActor;

  // Must build PhysxShapes for shapes that were created during the creation
  // of the Actor.
  unsigned int numShapes = nActor->getNbShapes();
  NxShape * const* shapes = nActor->getShapes();
  for(unsigned int i = 0; i < numShapes; ++i ) {
    // Some pointer nastiness is required to cast PT(PhysxShape) to each
    // specific shape type.
    NxShape *nShape = shapes[i];
    PT(PhysxShape) pShape;
    PhysxShape *pShapePointer;
    if(nShape->getType() == NX_SHAPE_BOX) {
      pShape = new PhysxBoxShape();
      pShapePointer = (PhysxShape *)pShape;
      ((PhysxBoxShape *)pShapePointer)->nBoxShape = (NxBoxShape *)nShape;
    } else if(nShape->getType() == NX_SHAPE_CAPSULE) {
      pShape = new PhysxCapsuleShape();
      pShapePointer = (PhysxShape *)pShape;
      ((PhysxCapsuleShape *)pShapePointer)->nCapsuleShape = (NxCapsuleShape *)nShape;
    } else if(nShape->getType() == NX_SHAPE_PLANE) {
      pShape = new PhysxPlaneShape();
      pShapePointer = (PhysxShape *)pShape;
      ((PhysxPlaneShape *)pShapePointer)->nPlaneShape = (NxPlaneShape *)nShape;
    } else if(nShape->getType() == NX_SHAPE_SPHERE) {
      pShape = new PhysxSphereShape();
      pShapePointer = (PhysxShape *)pShape;
      ((PhysxSphereShape *)pShapePointer)->nSphereShape = (NxSphereShape *)nShape;
    }
    nShape->userData = pShape;
    pShape->nShape = nShape;

    // Force the shape to re-store its name string, so that the pointer won't
    // be invalidated when the shapeDesc is destroyed.
    if(nShape->getName() != NULL) {
      pShape->set_name(nShape->getName());
    } else {
      pShape->set_name("");
    }

    _shapes.push_back(pShape);
  }

  pActor->update_transform();
  _actors.push_back(pActor);
  return pActor;
}

////////////////////////////////////////////////////////////////////
//     Function : check_results
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
check_results(PhysxSimulationStatus status, bool block) {
  nassertr(nScene != NULL, false);

  return nScene->checkResults((NxSimulationStatus)status, block);
}

////////////////////////////////////////////////////////////////////
//     Function : create_joint
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJoint *PhysxScene::
create_joint(const PhysxJointDesc & desc) {
  NxJoint *nJoint = nScene->createJoint(*(desc.nJointDesc));
  if(nJoint == NULL) {
    return NULL;
  }

  PhysxJoint *pJoint;

  if(nJoint->getType() == NX_JOINT_D6) {
    pJoint = new PhysxD6Joint();
    ((PhysxD6Joint *)pJoint)->nD6Joint = (NxD6Joint *)nJoint;
  }
  nJoint->userData = pJoint;
  pJoint->nJoint = nJoint;

  // Force the joint to re-store its name string, so that the pointer won't
  // be invalidated when the jointDesc is destroyed.
  if(nJoint->getName() != NULL) {
    pJoint->set_name(nJoint->getName());
  }

  _joints.push_back(pJoint);
  return pJoint;
}

////////////////////////////////////////////////////////////////////
//     Function : create_material
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxMaterial *PhysxScene::
create_material(const PhysxMaterialDesc & desc) {
  NxMaterial *nMaterial = nScene->createMaterial(desc.nMaterialDesc);
  if(nMaterial == NULL) {
    return NULL;
  }

  PhysxMaterial *pMaterial = new PhysxMaterial();
  pMaterial->nMaterial = nMaterial;
  nMaterial->userData = pMaterial;

  return pMaterial;
}

////////////////////////////////////////////////////////////////////
//     Function : set_contact_reporting_enabled
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_contact_reporting_enabled(bool enabled) {
  contact_reporting_enabled = enabled;
  if(enabled) {
    nScene->setUserContactReport(&contact_handler);
  } else {
    nScene->setUserContactReport(NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function : is_contact_reporting_enabled
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
is_contact_reporting_enabled() {
  return contact_reporting_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function : set_trigger_reporting_enabled
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_trigger_reporting_enabled(bool enabled) {
  trigger_reporting_enabled = enabled;
  if(enabled) {
    nScene->setUserTriggerReport(&trigger_handler);
  } else {
    nScene->setUserTriggerReport(NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function : is_trigger_reporting_enabled
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
is_trigger_reporting_enabled() {
  return trigger_reporting_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function : set_joint_reporting_enabled
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_joint_reporting_enabled(bool enabled) {
  joint_reporting_enabled = enabled;
  if(enabled) {
    nScene->setUserNotify(&joint_handler);
  } else {
    nScene->setUserNotify(NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function : is_joint_reporting_enabled
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
is_joint_reporting_enabled() {
  return joint_reporting_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function : set_contact_reporting_threshold
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_contact_reporting_threshold(float threshold) {
  contact_handler.set_threshold(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function : get_contact_reporting_threshold
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxScene::
get_contact_reporting_threshold() {
  return contact_handler.get_threshold();
}

////////////////////////////////////////////////////////////////////
//     Function : flush_caches
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
flush_caches() {
  nassertv(nScene != NULL);

  nScene->flushCaches();
}

////////////////////////////////////////////////////////////////////
//     Function : flush_stream
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
flush_stream() {
  nassertv(nScene != NULL);

  PStatTimer timer(_flush_stream_pcollector);
  nScene->flushStream();
}

////////////////////////////////////////////////////////////////////
//     Function : get_actor_group_pair_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_actor_group_pair_flags(unsigned short group1, unsigned short group2) const {
  nassertr(nScene != NULL, -1);

  return nScene->getActorGroupPairFlags(group1, group2);
}

////////////////////////////////////////////////////////////////////
//     Function : get_actor_pair_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_actor_pair_flags(PhysxActorNode & actor_a, PhysxActorNode & actor_b) const {
  nassertr(nScene != NULL, -1);

  return nScene->getActorPairFlags(*(actor_a.nActor), *(actor_b.nActor));
}

////////////////////////////////////////////////////////////////////
//     Function : get_bound_for_island_size
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_bound_for_island_size(PhysxActorNode & actor) {
  nassertr(nScene != NULL, -1);

  return nScene->getBoundForIslandSize(*(actor.nActor));
}

////////////////////////////////////////////////////////////////////
//     Function : get_filter_bool
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
get_filter_bool() const {
  nassertr(nScene != NULL, false);

  return nScene->getFilterBool();
}

////////////////////////////////////////////////////////////////////
//     Function : get_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_flags() const {
  nassertr(nScene != NULL, -1);

  return nScene->getFlags();
}

////////////////////////////////////////////////////////////////////
//     Function : get_gravity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
get_gravity(LVecBase3f & vec) {
  nassertv(nScene != NULL);

  nScene->getGravity(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : get_group_collision_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
get_group_collision_flag(unsigned short group1, unsigned short group2) const {
  nassertr(nScene != NULL, false);

  return nScene->getGroupCollisionFlag(group1, group2);
}

////////////////////////////////////////////////////////////////////
//     Function : get_highest_material_index
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned short PhysxScene::
get_highest_material_index() const {
  nassertr(nScene != NULL, -1);

  return nScene->getHighestMaterialIndex();
}

////////////////////////////////////////////////////////////////////
//     Function : get_material_from_index
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxMaterial * PhysxScene::
get_material_from_index(unsigned short mat_index) {
  nassertr(nScene != NULL, NULL);

  return (PhysxMaterial *)(nScene->getMaterialFromIndex(mat_index)->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : get_max_cpu_for_load_balancing
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxScene::
get_max_cpu_for_load_balancing() {
  nassertr(nScene != NULL, -1.0f);

  return nScene->getMaxCPUForLoadBalancing();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_actor_group_pairs
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_actor_group_pairs() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbActorGroupPairs();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_actors
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_actors() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbActors();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_cloths
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_cloths() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbCloths();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_compartments
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_compartments() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbCompartments();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_dynamic_shapes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_dynamic_shapes() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbDynamicShapes();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_effectors
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_effectors() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbEffectors();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_fluids
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_fluids() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbFluids();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_force_fields
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_force_fields() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbForceFields();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_implicit_screen_meshes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_implicit_screen_meshes() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbImplicitScreenMeshes();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_joints
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_joints() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbJoints();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_materials
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_materials() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbMaterials();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_pairs
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_pairs() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbPairs();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_soft_bodies
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_soft_bodies() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbSoftBodies();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_static_shapes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_nb_static_shapes() const {
  nassertr(nScene != NULL, -1);

  return nScene->getNbStaticShapes();
}

////////////////////////////////////////////////////////////////////
//     Function : get_next_joint
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJoint * PhysxScene::
get_next_joint() {
  nassertr(nScene != NULL, NULL);

  return (PhysxJoint *)(nScene->getNextJoint()->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : get_shape_pair_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_shape_pair_flags(PhysxShape & shape_a, PhysxShape & shape_b) const {
  nassertr(nScene != NULL, -1);

  return nScene->getShapePairFlags(*(shape_a.nShape), *(shape_b.nShape));
}

////////////////////////////////////////////////////////////////////
//     Function : get_sim_type
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSimulationType PhysxScene::
get_sim_type() const {
  return (PhysxSimulationType)nScene->getSimType();
}

////////////////////////////////////////////////////////////////////
//     Function : get_stats2
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSceneStats2 PhysxScene::
get_stats2() const {
  return PhysxSceneStats2(nScene->getStats2());
}

////////////////////////////////////////////////////////////////////
//     Function : get_timing
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
get_timing(float & max_timestep, unsigned int & max_iter, PhysxTimeStepMethod & method, unsigned int * num_sub_steps) const {
  nassertv(nScene != NULL);

  nScene->getTiming(max_timestep, max_iter, (NxTimeStepMethod &)method, num_sub_steps);
}

////////////////////////////////////////////////////////////////////
//     Function : get_total_nb_shapes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxScene::
get_total_nb_shapes() const {
  nassertr(nScene != NULL, -1);

  return nScene->getTotalNbShapes();
}

////////////////////////////////////////////////////////////////////
//     Function : is_writable
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
is_writable() {
  nassertr(nScene != NULL, false);

  return nScene->isWritable();
}

////////////////////////////////////////////////////////////////////
//     Function : lock_queries
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
lock_queries() {
  nassertv(nScene != NULL);

  nScene->lockQueries();
}

////////////////////////////////////////////////////////////////////
//     Function : poll_for_background_work
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxThreadPollResult PhysxScene::
poll_for_background_work(PhysxThreadWait wait_type) {
  return (PhysxThreadPollResult)nScene->pollForBackgroundWork((NxThreadWait)wait_type);
}

////////////////////////////////////////////////////////////////////
//     Function : poll_for_work
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxThreadPollResult PhysxScene::
poll_for_work(PhysxThreadWait wait_type) {
  return (PhysxThreadPollResult)nScene->pollForWork((NxThreadWait)wait_type);
}

////////////////////////////////////////////////////////////////////
//     Function : release_actor
//       Access : Published
//  Description : Releases the PhysxActorNode from the PhysX engine and
//                removes it from the Panda scene graph.  The
//                PhysxActorNode is thereafter dead to the PhysX engine
//                and should be discarded by the application code.
////////////////////////////////////////////////////////////////////
void PhysxScene::
release_actor(PhysxActorNode & pActor) {
  nassertv(nScene != NULL);

  if(pActor.nActor == NULL) {
    physx_warning( "release_actor: " << pActor.get_name() << ": unable to release actor; NxActor is already NULL" );
  } else {
    physx_debug( "release_actor: " << pActor.get_name() << ", " << &pActor );
    // Remove contained shapes from _shapes.
    unsigned int numShapes = pActor.nActor->getNbShapes();
    NxShape * const* shapes = pActor.nActor->getShapes();
    for(unsigned int i = 0; i < numShapes; ++i ) {
      NxShape *nShape = shapes[i];
      PT(PhysxShape) pShape = (PhysxShape *)(nShape->userData);
      pvector<PT(PhysxShape)>::iterator found;
      found = find(_shapes.begin(), _shapes.end(), pShape);
      if (found != _shapes.end()) {
        _shapes.erase(found);
      }
      pShape->nShape->userData = NULL;
      pShape->nShape = NULL;
    }

    // Set back reference pointer to NULL
    pActor.nActor->userData = NULL;

    // Remove pActor from _actors.
    pvector<PT(PhysxActorNode)>::iterator found;
    found = find(_actors.begin(), _actors.end(), &pActor);
    if (found != _actors.end()) {
      _actors.erase(found);
    }

    // Release the NxActor
    nScene->releaseActor(*(pActor.nActor));
    pActor.nActor = NULL;

    // Remove any Panda children
    pActor.remove_all_children();
    while(pActor.get_num_parents() > 0) {
      pActor.get_parent(0)->remove_child(&pActor);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function : release_joint
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
release_joint(PhysxJoint & pJoint) {
  nassertv(nScene != NULL);

  if(pJoint.nJoint == NULL) {
    physx_warning( "release_joint: unable to release joint; NxJoint is already NULL" );
  } else {
    pvector<PT(PhysxJoint)>::iterator found;
    found = find(_joints.begin(), _joints.end(), &pJoint);
    if (found != _joints.end()) {
      _joints.erase(found);
    }
    nScene->releaseJoint(*(pJoint.nJoint));
    pJoint.nJoint = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : release_material
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
release_material(PhysxMaterial & pMaterial) {
  nassertv(nScene != NULL);

  if(pMaterial.nMaterial == NULL) {
    physx_warning( "release_material: unable to release material; NxMaterial is already NULL" );
  } else {
    nScene->releaseMaterial(*(pMaterial.nMaterial));
    pMaterial.nMaterial = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : reset_effector_iterator
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
reset_effector_iterator() {
  nassertv(nScene != NULL);

  nScene->resetEffectorIterator();
}

////////////////////////////////////////////////////////////////////
//     Function : reset_joint_iterator
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
reset_joint_iterator() {
  nassertv(nScene != NULL);

  nScene->resetJointIterator();
}

////////////////////////////////////////////////////////////////////
//     Function : reset_poll_for_work
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
reset_poll_for_work() {
  nassertv(nScene != NULL);

  nScene->resetPollForWork();
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxScene::
save_to_desc(PhysxSceneDesc & desc) const {
  nassertr(nScene != NULL, false);

  return nScene->saveToDesc(desc.nSceneDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_actor_group_pair_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_actor_group_pair_flags(unsigned short group1, unsigned short group2, unsigned int flags) {
  nassertv(nScene != NULL);

  nScene->setActorGroupPairFlags(group1, group2, flags);
}

////////////////////////////////////////////////////////////////////
//     Function : set_actor_pair_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_actor_pair_flags(PhysxActorNode & actor_a, PhysxActorNode & actor_b, unsigned int nx_contact_pair_flag) {
  nassertv(nScene != NULL);

  nScene->setActorPairFlags(*(actor_a.nActor), *(actor_b.nActor), nx_contact_pair_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : set_filter_bool
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_filter_bool(bool flag) {
  nassertv(nScene != NULL);

  nScene->setFilterBool(flag);
}

////////////////////////////////////////////////////////////////////
//     Function : set_filter_ops
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_filter_ops(PhysxFilterOp op0, PhysxFilterOp op1, PhysxFilterOp op2) {
  nassertv(nScene != NULL);

  nScene->setFilterOps((NxFilterOp)op0, (NxFilterOp)op1, (NxFilterOp)op2);
}

////////////////////////////////////////////////////////////////////
//     Function : set_gravity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_gravity(const LVecBase3f & vec) {
  nassertv(nScene != NULL);

  nScene->setGravity(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_group_collision_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_group_collision_flag(unsigned short group1, unsigned short group2, bool enable) {
  nassertv(nScene != NULL);

  nScene->setGroupCollisionFlag(group1, group2, enable);
}

////////////////////////////////////////////////////////////////////
//     Function : set_max_cpu_for_load_balancing
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_max_cpu_for_load_balancing(float cpu_fraction) {
  nassertv(nScene != NULL);

  nScene->setMaxCPUForLoadBalancing(cpu_fraction);
}

////////////////////////////////////////////////////////////////////
//     Function : set_shape_pair_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_shape_pair_flags(PhysxShape & shape_a, PhysxShape & shape_b, unsigned int nx_contact_pair_flag) {
  nassertv(nScene != NULL);

  nScene->setShapePairFlags(*(shape_a.nShape), *(shape_b.nShape), nx_contact_pair_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : set_timing
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
set_timing(float max_timestep, unsigned int max_iter, PhysxTimeStepMethod method) {
  nassertv(nScene != NULL);

  nScene->setTiming(max_timestep, max_iter, (NxTimeStepMethod)method);
}

////////////////////////////////////////////////////////////////////
//     Function : shutdown_worker_threads
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
shutdown_worker_threads() {
  nassertv(nScene != NULL);

  nScene->shutdownWorkerThreads();
}

////////////////////////////////////////////////////////////////////
//     Function : simulate
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
simulate(float elapsed_time) {
  nassertv(nScene != NULL);

  nScene->simulate(elapsed_time);
}

////////////////////////////////////////////////////////////////////
//     Function : unlock_queries
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxScene::
unlock_queries() {
  nassertv(nScene != NULL);

  nScene->unlockQueries();
}

#endif // HAVE_PHYSX

