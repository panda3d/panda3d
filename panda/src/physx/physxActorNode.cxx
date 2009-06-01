// Filename: physxActorNode.cxx
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

#include "physxActorNode.h"

#include "luse.h"
#include "physxBodyDesc.h"
#include "physxScene.h"
#include "physxShape.h"
#include "physxShapeDesc.h"
#include "physxBoxShape.h"
#include "physxCapsuleShape.h"
#include "physxPlaneShape.h"
#include "physxSphereShape.h"

TypeHandle PhysxActorNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : PhysxActorNode
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxActorNode::
PhysxActorNode(const string &name) : PandaNode(name) {
  _disable_transform_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function : get_num_shapes
//       Access : Published
//  Description : Returns the number of PhysxShapes associated with
//                this PhysxActorNode.
////////////////////////////////////////////////////////////////////
unsigned int PhysxActorNode::
get_num_shapes() {
  nassertr(nActor != NULL, -1);

  return nActor->getNbShapes();
}

////////////////////////////////////////////////////////////////////
//     Function : get_shape
//       Access : Published
//  Description : Returns the PhysxShape associated with this
//                PhysxActorNode at index i.
////////////////////////////////////////////////////////////////////
PhysxShape *PhysxActorNode::
get_shape(unsigned int i) {
  nassertr(nActor != NULL, NULL);

  if((i >= 0) && (i < nActor->getNbShapes())) {
    NxShape * const* nShapes = nActor->getShapes();
    PhysxShape *pShape = (PhysxShape *)nShapes[i]->userData;
    return pShape;
  } else {
    physx_warning( "PhysxActorNode::get_shape: " << this->get_name() << ": index out of bounds; i=" << i );
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : add_force
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_force(const LVecBase3f & force, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addForce(PhysxManager::lVecBase3_to_nxVec3(force), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_force_at_local_pos
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_force_at_local_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addForceAtLocalPos(PhysxManager::lVecBase3_to_nxVec3(force), PhysxManager::lVecBase3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_force_at_pos
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_force_at_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addForceAtPos(PhysxManager::lVecBase3_to_nxVec3(force), PhysxManager::lVecBase3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_local_force
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_local_force(const LVecBase3f & force, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addLocalForce(PhysxManager::lVecBase3_to_nxVec3(force), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_local_force_at_local_pos
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_local_force_at_local_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addLocalForceAtLocalPos(PhysxManager::lVecBase3_to_nxVec3(force), PhysxManager::lVecBase3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_local_force_at_pos
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_local_force_at_pos(const LVecBase3f & force, const LVecBase3f & pos, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addLocalForceAtPos(PhysxManager::lVecBase3_to_nxVec3(force), PhysxManager::lVecBase3_to_nxVec3(pos), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_local_torque
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_local_torque(const LVecBase3f & torque, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addLocalTorque(PhysxManager::lVecBase3_to_nxVec3(torque), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : add_torque
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
add_torque(const LVecBase3f & torque, PhysxForceMode mode, bool wakeup) {
  nassertv(nActor != NULL);

  nActor->addTorque(PhysxManager::lVecBase3_to_nxVec3(torque), (NxForceMode)mode, wakeup);
}

////////////////////////////////////////////////////////////////////
//     Function : clear_actor_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
clear_actor_flag(PhysxActorFlag actor_flag) {
  nassertv(nActor != NULL);

  nActor->clearActorFlag((NxActorFlag)actor_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : clear_body_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
clear_body_flag(PhysxBodyFlag body_flag) {
  nassertv(nActor != NULL);

  nActor->clearBodyFlag((NxBodyFlag)body_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : compute_kinetic_energy
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
compute_kinetic_energy() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->computeKineticEnergy();
}

////////////////////////////////////////////////////////////////////
//     Function : create_shape
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxShape * PhysxActorNode::
create_shape(const PhysxShapeDesc & desc) {
  nassertr(nActor != NULL, NULL);

  // Some pointer nastiness is required to cast PT(PhysxShape) to each
  // specific shape type.
  NxShape *nShape = nActor->createShape(*(desc.nShapeDesc));
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
  return pShape;
}

////////////////////////////////////////////////////////////////////
//     Function : get_angular_damping
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_angular_damping() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getAngularDamping();
}

////////////////////////////////////////////////////////////////////
//     Function : get_angular_momentum
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_angular_momentum() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getAngularMomentum());
}

////////////////////////////////////////////////////////////////////
//     Function : get_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_angular_velocity() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getAngularVelocity());
}

////////////////////////////////////////////////////////////////////
//     Function : get_c_mass_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxActorNode::
get_c_mass_global_orientation() const {
  nassertr(nActor != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nActor->getCMassGlobalOrientation());
}

////////////////////////////////////////////////////////////////////
//     Function : get_c_mass_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxActorNode::
get_c_mass_global_pose() const {
  nassertr(nActor != NULL, *((LMatrix4f *)NULL));

  return PhysxManager::nxMat34_to_lMatrix4(nActor->getCMassGlobalPose());
}

////////////////////////////////////////////////////////////////////
//     Function : get_c_mass_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_c_mass_global_position() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getCMassGlobalPosition());
}

////////////////////////////////////////////////////////////////////
//     Function : get_c_mass_local_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxActorNode::
get_c_mass_local_orientation() const {
  nassertr(nActor != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nActor->getCMassLocalOrientation());
}

////////////////////////////////////////////////////////////////////
//     Function : get_c_mass_local_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxActorNode::
get_c_mass_local_pose() const {
  nassertr(nActor != NULL, *((LMatrix4f *)NULL));

  return PhysxManager::nxMat34_to_lMatrix4(nActor->getCMassLocalPose());
}

////////////////////////////////////////////////////////////////////
//     Function : get_c_mass_local_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_c_mass_local_position() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getCMassLocalPosition());
}

////////////////////////////////////////////////////////////////////
//     Function : get_ccd_motion_threshold
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_ccd_motion_threshold() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getCCDMotionThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_inertia_tensor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxActorNode::
get_global_inertia_tensor() const {
  nassertr(nActor != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nActor->getGlobalInertiaTensor());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_inertia_tensor_inverse
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxActorNode::
get_global_inertia_tensor_inverse() const {
  nassertr(nActor != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nActor->getGlobalInertiaTensorInverse());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxActorNode::
get_global_orientation() const {
  nassertr(nActor != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nActor->getGlobalOrientation());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_orientation_quat
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LQuaternionf PhysxActorNode::
get_global_orientation_quat() const {
  nassertr(nActor != NULL, *((LQuaternionf *)NULL));

  return PhysxManager::nxQuat_to_lQuaternion(nActor->getGlobalOrientationQuat());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_global_position() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getGlobalPosition());
}

////////////////////////////////////////////////////////////////////
//     Function : get_group
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned short PhysxActorNode::
get_group() const {
  nassertr(nActor != NULL, -1);

  return nActor->getGroup();
}

////////////////////////////////////////////////////////////////////
//     Function : get_linear_damping
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_linear_damping() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getLinearDamping();
}

////////////////////////////////////////////////////////////////////
//     Function : get_linear_momentum
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_linear_momentum() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getLinearMomentum());
}

////////////////////////////////////////////////////////////////////
//     Function : get_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_linear_velocity() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getLinearVelocity());
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_point_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_local_point_velocity(const LVecBase3f & point) const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getLocalPointVelocity(PhysxManager::lVecBase3_to_nxVec3(point)));
}

////////////////////////////////////////////////////////////////////
//     Function : get_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_mass() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getMass();
}

////////////////////////////////////////////////////////////////////
//     Function : get_mass_space_inertia_tensor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_mass_space_inertia_tensor() const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getMassSpaceInertiaTensor());
}

////////////////////////////////////////////////////////////////////
//     Function : get_max_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_max_angular_velocity() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getMaxAngularVelocity();
}

////////////////////////////////////////////////////////////////////
//     Function : get_nb_shapes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxActorNode::
get_nb_shapes() const {
  nassertr(nActor != NULL, -1);

  return nActor->getNbShapes();
}

////////////////////////////////////////////////////////////////////
//     Function : get_point_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxActorNode::
get_point_velocity(const LVecBase3f & point) const {
  nassertr(nActor != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nActor->getPointVelocity(PhysxManager::lVecBase3_to_nxVec3(point)));
}

////////////////////////////////////////////////////////////////////
//     Function : get_scene
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxScene & PhysxActorNode::
get_scene() const {
  nassertr(nActor != NULL, *((PhysxScene *)NULL));

  return *((PhysxScene *)(nActor->getScene().userData));
}

////////////////////////////////////////////////////////////////////
//     Function : get_sleep_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_sleep_angular_velocity() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getSleepAngularVelocity();
}

////////////////////////////////////////////////////////////////////
//     Function : get_sleep_energy_threshold
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_sleep_energy_threshold() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getSleepEnergyThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function : get_sleep_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxActorNode::
get_sleep_linear_velocity() const {
  nassertr(nActor != NULL, -1.0f);

  return nActor->getSleepLinearVelocity();
}

////////////////////////////////////////////////////////////////////
//     Function : get_solver_iteration_count
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned int PhysxActorNode::
get_solver_iteration_count() const {
  nassertr(nActor != NULL, -1);

  return nActor->getSolverIterationCount();
}

////////////////////////////////////////////////////////////////////
//     Function : is_dynamic
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxActorNode::
is_dynamic() const {
  nassertr(nActor != NULL, false);

  return nActor->isDynamic();
}

////////////////////////////////////////////////////////////////////
//     Function : is_group_sleeping
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxActorNode::
is_group_sleeping() const {
  nassertr(nActor != NULL, false);

  return nActor->isGroupSleeping();
}

////////////////////////////////////////////////////////////////////
//     Function : is_sleeping
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxActorNode::
is_sleeping() const {
  nassertr(nActor != NULL, false);

  return nActor->isSleeping();
}

////////////////////////////////////////////////////////////////////
//     Function : move_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
move_global_orientation(const LMatrix3f & mat) {
  nassertv(nActor != NULL);

  nActor->moveGlobalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : move_global_orientation_quat
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
move_global_orientation_quat(const LQuaternionf & quat) {
  nassertv(nActor != NULL);

  nActor->moveGlobalOrientationQuat(PhysxManager::lQuaternion_to_nxQuat(quat));
}

////////////////////////////////////////////////////////////////////
//     Function : move_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
move_global_pose(const LMatrix4f & mat) {
  nassertv(nActor != NULL);

  nActor->moveGlobalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : move_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
move_global_position(const LVecBase3f & vec) {
  nassertv(nActor != NULL);

  nActor->moveGlobalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : put_to_sleep
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
put_to_sleep() {
  nassertv(nActor != NULL);

  nActor->putToSleep();
}

////////////////////////////////////////////////////////////////////
//     Function : raise_actor_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
raise_actor_flag(PhysxActorFlag actor_flag) {
  nassertv(nActor != NULL);

  nActor->raiseActorFlag((NxActorFlag)actor_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : raise_body_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
raise_body_flag(PhysxBodyFlag body_flag) {
  nassertv(nActor != NULL);

  nActor->raiseBodyFlag((NxBodyFlag)body_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : read_actor_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxActorNode::
read_actor_flag(PhysxActorFlag actor_flag) const {
  nassertr(nActor != NULL, false);

  return nActor->readActorFlag((NxActorFlag)actor_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : read_body_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxActorNode::
read_body_flag(PhysxBodyFlag body_flag) const {
  nassertr(nActor != NULL, false);

  return nActor->readBodyFlag((NxBodyFlag)body_flag);
}

////////////////////////////////////////////////////////////////////
//     Function : recompute_adaptive_force_counters
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
recompute_adaptive_force_counters() {
  nassertv(nActor != NULL);

  nActor->recomputeAdaptiveForceCounters();
}

////////////////////////////////////////////////////////////////////
//     Function : release_shape
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
release_shape(PhysxShape & shape) {
  nassertv(nActor != NULL);

  nActor->releaseShape(*(shape.nShape));
}

////////////////////////////////////////////////////////////////////
//     Function : reset_user_actor_pair_filtering
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
reset_user_actor_pair_filtering() {
  nassertv(nActor != NULL);

  nActor->resetUserActorPairFiltering();
}

////////////////////////////////////////////////////////////////////
//     Function : save_body_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxActorNode::
save_body_to_desc(PhysxBodyDesc & body_desc) {
  nassertr(nActor != NULL, false);

  return nActor->saveBodyToDesc(body_desc.nBodyDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_angular_damping
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_angular_damping(float ang_damp) {
  nassertv(nActor != NULL);

  nActor->setAngularDamping(ang_damp);
}

////////////////////////////////////////////////////////////////////
//     Function : set_angular_momentum
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_angular_momentum(const LVecBase3f & ang_moment) {
  nassertv(nActor != NULL);

  nActor->setAngularMomentum(PhysxManager::lVecBase3_to_nxVec3(ang_moment));
}

////////////////////////////////////////////////////////////////////
//     Function : set_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_angular_velocity(const LVecBase3f & ang_vel) {
  nassertv(nActor != NULL);

  nActor->setAngularVelocity(PhysxManager::lVecBase3_to_nxVec3(ang_vel));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_global_orientation(const LMatrix3f & mat) {
  nassertv(nActor != NULL);

  nActor->setCMassGlobalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_global_pose(const LMatrix4f & mat) {
  nassertv(nActor != NULL);

  nActor->setCMassGlobalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_global_position(const LVecBase3f & vec) {
  nassertv(nActor != NULL);

  nActor->setCMassGlobalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_offset_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_offset_global_orientation(const LMatrix3f & mat) {
  nassertv(nActor != NULL);

  nActor->setCMassOffsetGlobalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_offset_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_offset_global_pose(const LMatrix4f & mat) {
  nassertv(nActor != NULL);

  nActor->setCMassOffsetGlobalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_offset_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_offset_global_position(const LVecBase3f & vec) {
  nassertv(nActor != NULL);

  nActor->setCMassOffsetGlobalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_offset_local_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_offset_local_orientation(const LMatrix3f & mat) {
  nassertv(nActor != NULL);

  nActor->setCMassOffsetLocalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_offset_local_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_offset_local_pose(const LMatrix4f & mat) {
  nassertv(nActor != NULL);

  nActor->setCMassOffsetLocalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_c_mass_offset_local_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_c_mass_offset_local_position(const LVecBase3f & vec) {
  nassertv(nActor != NULL);

  nActor->setCMassOffsetLocalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_ccd_motion_threshold
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_ccd_motion_threshold(float thresh) {
  nassertv(nActor != NULL);

  nActor->setCCDMotionThreshold(thresh);
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_global_orientation(const LMatrix3f & mat) {
  nassertv(nActor != NULL);

  nActor->setGlobalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_orientation_quat
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_global_orientation_quat(const LQuaternionf & mat) {
  nassertv(nActor != NULL);

  nActor->setGlobalOrientationQuat(PhysxManager::lQuaternion_to_nxQuat(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_global_pose(const LMatrix4f & mat) {
  nassertv(nActor != NULL);

  nActor->setGlobalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_global_position(const LVecBase3f & vec) {
  nassertv(nActor != NULL);

  nActor->setGlobalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_group
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_group(unsigned short actor_group) {
  nassertv(nActor != NULL);

  nActor->setGroup(actor_group);
}

////////////////////////////////////////////////////////////////////
//     Function : set_linear_damping
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_linear_damping(float lin_damp) {
  nassertv(nActor != NULL);

  nActor->setLinearDamping(lin_damp);
}

////////////////////////////////////////////////////////////////////
//     Function : set_linear_momentum
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_linear_momentum(const LVecBase3f & lin_moment) {
  nassertv(nActor != NULL);

  nActor->setLinearMomentum(PhysxManager::lVecBase3_to_nxVec3(lin_moment));
}

////////////////////////////////////////////////////////////////////
//     Function : set_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_linear_velocity(const LVecBase3f & lin_vel) {
  nassertv(nActor != NULL);

  nActor->setLinearVelocity(PhysxManager::lVecBase3_to_nxVec3(lin_vel));
}

////////////////////////////////////////////////////////////////////
//     Function : set_mass
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_mass(float mass) {
  nassertv(nActor != NULL);

  nActor->setMass(mass);
}

////////////////////////////////////////////////////////////////////
//     Function : set_mass_space_inertia_tensor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_mass_space_inertia_tensor(const LVecBase3f & m) {
  nassertv(nActor != NULL);

  nActor->setMassSpaceInertiaTensor(PhysxManager::lVecBase3_to_nxVec3(m));
}

////////////////////////////////////////////////////////////////////
//     Function : set_max_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_max_angular_velocity(float max_ang_vel) {
  nassertv(nActor != NULL);

  nActor->setMaxAngularVelocity(max_ang_vel);
}

////////////////////////////////////////////////////////////////////
//     Function : set_sleep_angular_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_sleep_angular_velocity(float threshold) {
  nassertv(nActor != NULL);

  nActor->setSleepAngularVelocity(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function : set_sleep_energy_threshold
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_sleep_energy_threshold(float threshold) {
  nassertv(nActor != NULL);

  nActor->setSleepEnergyThreshold(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function : set_sleep_linear_velocity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_sleep_linear_velocity(float threshold) {
  nassertv(nActor != NULL);

  nActor->setSleepLinearVelocity(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function : set_solver_iteration_count
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
set_solver_iteration_count(unsigned int iter_count) {
  nassertv(nActor != NULL);

  nActor->setSolverIterationCount(iter_count);
}

////////////////////////////////////////////////////////////////////
//     Function : update_mass_from_shapes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
update_mass_from_shapes(float density, float total_mass) {
  nassertv(nActor != NULL);

  nActor->updateMassFromShapes(density, total_mass);
}

////////////////////////////////////////////////////////////////////
//     Function : wake_up
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
wake_up(float wake_counter_value) {
  nassertv(nActor != NULL);

  nActor->wakeUp(wake_counter_value);
}

////////////////////////////////////////////////////////////////////
//     Function : transform_changed
//       Access : private, virtual
//  Description : node hook.  This function handles outside
//                (non-physics) actions on the PhysxActorNode
//                and updates the internal representation of the node.
//                i.e. copy from PandaNode to PhysxActor
////////////////////////////////////////////////////////////////////
void PhysxActorNode::
transform_changed() {
  nassertv(nActor != NULL);

  PandaNode::transform_changed();

  // _disable_transform_changed is used to prevent a recursive loop between
  // transform_changed and update_transform.
  if(_disable_transform_changed) {
    return;
  }

  if(nActor->isDynamic()) {
    CPT(TransformState) transform = get_transform();
    if(nActor->readBodyFlag(NX_BF_KINEMATIC)) {
      // If the Actor is kinematic, use moveGlobalPose
      nActor->moveGlobalPose(PhysxManager::lMatrix4_to_nxMat34(transform->get_mat()));
    } else {
      // If the Actor is just dynamic, use setGlobalPose
      nActor->setGlobalPose(PhysxManager::lMatrix4_to_nxMat34(transform->get_mat()));
    }
  } else {
    // If the Actor is static, don't allow the transform to be updated
    physx_error("Error: cannot update transform of static PhysxActorNode " << get_name());
  }
}

#endif // HAVE_PHYSX

