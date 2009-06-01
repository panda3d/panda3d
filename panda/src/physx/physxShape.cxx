// Filename: physxShape.cxx
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

#include "physxShape.h"

#include "luse.h"
#include "physxActorNode.h"
#include "physxBounds3.h"
#include "physxBox.h"
#include "physxBoxShape.h"
#include "physxCapsule.h"
#include "physxCapsuleShape.h"
#include "physxPlaneShape.h"
#include "physxSphere.h"
#include "physxSphereShape.h"

TypeHandle PhysxShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : check_overlap_aabb
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_aabb(const PhysxBounds3 & world_bounds) const {
  nassertr(nShape != NULL, false);

  return nShape->checkOverlapAABB(*(world_bounds.nBounds3));
}

////////////////////////////////////////////////////////////////////
//     Function : check_overlap_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_capsule(const PhysxCapsule & world_capsule) const {
  nassertr(nShape != NULL, false);

  return nShape->checkOverlapCapsule(*(world_capsule.nCapsule));
}

////////////////////////////////////////////////////////////////////
//     Function : check_overlap_obb
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_obb(const PhysxBox & world_box) const {
  nassertr(nShape != NULL, false);

  return nShape->checkOverlapOBB(*(world_box.nBox));
}

////////////////////////////////////////////////////////////////////
//     Function : check_overlap_sphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_sphere(const PhysxSphere & world_sphere) const {
  nassertr(nShape != NULL, false);

  return nShape->checkOverlapSphere(*(world_sphere.nSphere));
}

////////////////////////////////////////////////////////////////////
//     Function : get_actor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxActorNode & PhysxShape::
get_actor() const {
  nassertr(nShape != NULL, *((PhysxActorNode *)NULL));

  return *((PhysxActorNode *)(nShape->getActor().userData));
}

////////////////////////////////////////////////////////////////////
//     Function : get_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
bool PhysxShape::
get_flag(PhysxShapeFlag flag) const {
  nassertr(nShape != NULL, false);

  return (bool)(nShape->getFlag((NxShapeFlag)flag));
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxShape::
get_global_orientation() const {
  nassertr(nShape != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nShape->getGlobalOrientation());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxShape::
get_global_pose() const {
  nassertr(nShape != NULL, *((LMatrix4f *)NULL));

  return PhysxManager::nxMat34_to_lMatrix4(nShape->getGlobalPose());
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxShape::
get_global_position() const {
  nassertr(nShape != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nShape->getGlobalPosition());
}

////////////////////////////////////////////////////////////////////
//     Function : get_group
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned short PhysxShape::
get_group() const {
  nassertr(nShape != NULL, -1);

  return nShape->getGroup();
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxShape::
get_local_orientation() const {
  nassertr(nShape != NULL, *((LMatrix3f *)NULL));

  return PhysxManager::nxMat33_to_lMatrix3(nShape->getLocalOrientation());
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxShape::
get_local_pose() const {
  nassertr(nShape != NULL, *((LMatrix4f *)NULL));

  return PhysxManager::nxMat34_to_lMatrix4(nShape->getLocalPose());
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxShape::
get_local_position() const {
  nassertr(nShape != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nShape->getLocalPosition());
}

////////////////////////////////////////////////////////////////////
//     Function : get_material
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
unsigned short PhysxShape::
get_material() const {
  nassertr(nShape != NULL, -1);

  return nShape->getMaterial();
}

////////////////////////////////////////////////////////////////////
//     Function : get_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const char * PhysxShape::
get_name() const {
  nassertr(nShape != NULL, NULL);

  return nShape->getName();
}

////////////////////////////////////////////////////////////////////
//     Function : get_skin_width
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxShape::
get_skin_width() const {
  nassertr(nShape != NULL, -1.0f);

  return nShape->getSkinWidth();
}

////////////////////////////////////////////////////////////////////
//     Function : get_world_bounds
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
get_world_bounds(PhysxBounds3 & dest) const {
  nassertv(nShape != NULL);

  nShape->getWorldBounds(*(dest.nBounds3));
}

////////////////////////////////////////////////////////////////////
//     Function : is_box
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const PhysxBoxShape * PhysxShape::
is_box() const {
  nassertr(nShape != NULL, NULL);

  return (PhysxBoxShape *)(nShape->isBox()->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : is_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const PhysxCapsuleShape * PhysxShape::
is_capsule() const {
  nassertr(nShape != NULL, NULL);

  return (PhysxCapsuleShape *)(nShape->isCapsule()->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : is_plane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const PhysxPlaneShape * PhysxShape::
is_plane() const {
  nassertr(nShape != NULL, NULL);

  return (PhysxPlaneShape *)(nShape->isPlane()->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : is_sphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const PhysxSphereShape * PhysxShape::
is_sphere() const {
  nassertr(nShape != NULL, NULL);

  return (PhysxSphereShape *)(nShape->isSphere()->userData);
}

////////////////////////////////////////////////////////////////////
//     Function : set_flag
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_flag(PhysxShapeFlag flag, bool value) {
  nassertv(nShape != NULL);

  nShape->setFlag((NxShapeFlag)flag, value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_global_orientation(const LMatrix3f & mat) {
  nassertv(nShape != NULL);

  nShape->setGlobalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_global_pose(const LMatrix4f & mat) {
  nassertv(nShape != NULL);

  nShape->setGlobalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_global_position(const LVecBase3f & vec) {
  nassertv(nShape != NULL);

  nShape->setGlobalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_group
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_group(unsigned short collision_group) {
  nassertv(nShape != NULL);

  nShape->setGroup(collision_group);
}

////////////////////////////////////////////////////////////////////
//     Function : set_local_orientation
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_local_orientation(const LMatrix3f & mat) {
  nassertv(nShape != NULL);

  nShape->setLocalOrientation(PhysxManager::lMatrix3_to_nxMat33(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_local_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_local_pose(const LMatrix4f & mat) {
  nassertv(nShape != NULL);

  nShape->setLocalPose(PhysxManager::lMatrix4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function : set_local_position
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_local_position(const LVecBase3f & vec) {
  nassertv(nShape != NULL);

  nShape->setLocalPosition(PhysxManager::lVecBase3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function : set_material
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_material(unsigned short mat_index) {
  nassertv(nShape != NULL);

  nShape->setMaterial(mat_index);
}

////////////////////////////////////////////////////////////////////
//     Function : set_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_name(const char * name) {
  nassertv(nShape != NULL);

  // Because the PhysX engine does not store its own copy of names,
  // we keep a local copy on this instance.  Otherwise, it would be
  // very easy for names to be declared in python and then
  // invalidated when the string is reclaimed from reference
  // counting.
  _name_store = name;
  nShape->setName(_name_store.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function : set_skin_width
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_skin_width(float skin_width) {
  nassertv(nShape != NULL);

  nShape->setSkinWidth(skin_width);
}

#endif // HAVE_PHYSX



