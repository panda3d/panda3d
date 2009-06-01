// Filename: physxJoint.cxx
// Created by:  pratt (Jun 16, 2006)
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

#include "physxJoint.h"

#include "luse.h"
#include "physxD6Joint.h"
#include "physxScene.h"

TypeHandle PhysxJoint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : add_limit_plane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxJoint::
add_limit_plane(const LVecBase3f & normal, const LVecBase3f & point_in_plane, float restitution) {
  nassertr(nJoint != NULL, false);

  return nJoint->addLimitPlane(PhysxManager::lVecBase3_to_nxVec3(normal), PhysxManager::lVecBase3_to_nxVec3(point_in_plane), restitution);
}

////////////////////////////////////////////////////////////////////
//     Function : get_breakable
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
get_breakable(float & max_force, float & max_torque) {
  nassertv(nJoint != NULL);

  nJoint->getBreakable(max_force, max_torque);
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_anchor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxJoint::
get_global_anchor() const {
  nassertr(nJoint != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nJoint->getGlobalAnchor());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxJoint::
get_global_axis() const {
  nassertr(nJoint != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nJoint->getGlobalAxis());
}

////////////////////////////////////////////////////////////////////
//     Function : get_limit_point
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxJoint::
get_limit_point(LVecBase3f & world_limit_point) {
  nassertr(nJoint != NULL, false);

  return nJoint->getLimitPoint(PhysxManager::lVecBase3_to_nxVec3(world_limit_point));
}

////////////////////////////////////////////////////////////////////
//     Function : get_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const char * PhysxJoint::
get_name() const {
  nassertr(nJoint != NULL, NULL);

  return nJoint->getName();
}

////////////////////////////////////////////////////////////////////
//     Function : get_next_limit_plane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxJoint::
get_next_limit_plane(LVecBase3f & plane_normal, float & plane_d, float * restitution) {
  nassertr(nJoint != NULL, false);

  return nJoint->getNextLimitPlane(PhysxManager::lVecBase3_to_nxVec3(plane_normal), plane_d, restitution);
}

////////////////////////////////////////////////////////////////////
//     Function : get_scene
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxScene & PhysxJoint::
get_scene() const {
  nassertr(nJoint != NULL, *((PhysxScene *)NULL));

  return *((PhysxScene *)(nJoint->getScene().userData));
}

////////////////////////////////////////////////////////////////////
//     Function : get_state
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJointState PhysxJoint::
get_state() {
  nassertr(nJoint != NULL, physx_js_unbound);

  return (PhysxJointState)nJoint->getState();
}

////////////////////////////////////////////////////////////////////
//     Function : has_more_limit_planes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxJoint::
has_more_limit_planes() {
  nassertr(nJoint != NULL, false);

  return nJoint->hasMoreLimitPlanes();
}

////////////////////////////////////////////////////////////////////
//     Function : is
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void * PhysxJoint::
is(PhysxJointType type) {
  nassertr(nJoint != NULL, NULL);

  return nJoint->is((NxJointType)type);
}

////////////////////////////////////////////////////////////////////
//     Function : is_d6_joint
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxD6Joint * PhysxJoint::
is_d6_joint() {
  nassertr(nJoint != NULL, NULL);

  return (PhysxD6Joint *)(nJoint->isD6Joint()->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : purge_limit_planes
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
purge_limit_planes() {
  nassertv(nJoint != NULL);

  nJoint->purgeLimitPlanes();
}

////////////////////////////////////////////////////////////////////
//     Function : reset_limit_plane_iterator
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
reset_limit_plane_iterator() {
  nassertv(nJoint != NULL);

  nJoint->resetLimitPlaneIterator();
}

////////////////////////////////////////////////////////////////////
//     Function : set_breakable
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
set_breakable(float max_force, float max_torque) {
  nassertv(nJoint != NULL);

  nJoint->setBreakable(max_force, max_torque);
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_anchor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
set_global_anchor(const LVecBase3f & vec) {
  nassertv(nJoint != NULL);

  nJoint->setGlobalAnchor(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
set_global_axis(const LVecBase3f & vec) {
  nassertv(nJoint != NULL);

  nJoint->setGlobalAxis(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_limit_point
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
set_limit_point(const LVecBase3f & point, bool point_is_on_actor2) {
  nassertv(nJoint != NULL);

  nJoint->setLimitPoint(PhysxManager::lVecBase3_to_nxVec3(point), point_is_on_actor2);
}

////////////////////////////////////////////////////////////////////
//     Function : set_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJoint::
set_name(const char * name) {
  nassertv(nJoint != NULL);

  // Because the PhysX engine does not store its own copy of names,
  // we keep a local copy on this instance.  Otherwise, it would be
  // very easy for names to be declared in python and then
  // invalidated when the string is reclaimed from reference
  // counting.
  _name_store = name;
  nJoint->setName(_name_store.c_str());
}

#endif // HAVE_PHYSX

