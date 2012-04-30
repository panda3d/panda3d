// Filename: physxShape.cxx
// Created by:  enn0x (16Sep09)
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

#include "physxShape.h"
#include "physxManager.h"
#include "physxActor.h"
#include "physxBoxShape.h"
#include "physxCapsuleShape.h"
#include "physxPlaneShape.h"
#include "physxSphereShape.h"
#include "physxConvexShape.h"
#include "physxHeightFieldShape.h"
#include "physxTriangleMeshShape.h"
#include "physxWheelShape.h"
#include "physxGroupsMask.h"
#include "physxBounds3.h"
#include "physxSphere.h"
#include "physxBox.h"
#include "physxCapsule.h"
#include "physxRay.h"
#include "physxRaycastHit.h"
#include "physxCcdSkeleton.h"

TypeHandle PhysxShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxShape::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  ptr()->getActor().releaseShape(*ptr());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::factory
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxShape *PhysxShape::
factory(NxShapeType shapeType) {

  switch (shapeType) {

  case NX_SHAPE_PLANE:
    return new PhysxPlaneShape();

  case NX_SHAPE_SPHERE:
    return new PhysxSphereShape();

  case NX_SHAPE_BOX:
    return new PhysxBoxShape();

  case NX_SHAPE_CAPSULE:
    return new PhysxCapsuleShape();

  case NX_SHAPE_CONVEX:
    return new PhysxConvexShape();

  case NX_SHAPE_MESH:
    return new PhysxTriangleMeshShape();

  case NX_SHAPE_HEIGHTFIELD:
    return new PhysxHeightFieldShape();

  case NX_SHAPE_WHEEL:
    return new PhysxWheelShape();
  }

  physx_cat.error() << "Unknown shape type.\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_actor
//       Access: Published
//  Description: Retrieves the actor which this shape is associated
//               with.
////////////////////////////////////////////////////////////////////
PhysxActor *PhysxShape::
get_actor() const {

  nassertr(_error_type == ET_ok, NULL);
  return (PhysxActor *)(ptr()->getActor().userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_name
//       Access: Published
//  Description: Sets a name string for this object. The name can
//               be retrieved again with get_name().
//               This is for debugging and is not used by the
//               physics engine.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  ptr()->setName(_name.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_name
//       Access: Published
//  Description: Returns the name string. 
////////////////////////////////////////////////////////////////////
const char *PhysxShape::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return ptr()->getName();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_flag
//       Access: Published
//  Description: Sets the specified shape flag.
//
//                The shape may be turned into a trigger by setting
//                one or more of the TriggerFlags to true. A trigger
//                shape will not collide with other shapes. Instead,
//                if a shape enters the trigger's volume, a trigger
//                event will be sent. Trigger events can be listened
//                to by DirectObjects.
//                The following trigger events can be sent:
//                - physx-trigger-enter
//                - physx-trigger-stay
//                - physx-trigger-leave
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_flag(PhysxShapeFlag flag, bool value) {

  nassertv(_error_type == ET_ok);

  ptr()->setFlag((NxShapeFlag)flag, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_flag
//       Access: Published
//  Description: Returns the specified shape flag.
////////////////////////////////////////////////////////////////////
bool PhysxShape::
get_flag(PhysxShapeFlag flag) const {

  nassertr(_error_type == ET_ok, false);

  return (ptr()->getFlag((NxShapeFlag)flag)) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_skin_width
//       Access: Published
//  Description: Sets the skin width. 
//               The skin width must be non-negative.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_skin_width(float skinWidth) {

  nassertv(_error_type == ET_ok);
  nassertv(skinWidth >= 0.0f);

  ptr()->setSkinWidth(skinWidth);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_skin_width
//       Access: Published
//  Description: Returns the skin width.
////////////////////////////////////////////////////////////////////
float PhysxShape::
get_skin_width() const {

  nassertr(_error_type == ET_ok, 0.0f);

  return ptr()->getSkinWidth();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_group
//       Access: Published
//  Description: Sets which collision group this shape is part of. 
//
//               Default group is 0. Maximum possible group is 31.
//               Collision groups are sets of shapes which may or
//               may not be set to collision detect with each other;
//               this can be set using
//               PhysxScene::set_group_collision_flag().
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_group(unsigned short group) {

  nassertv(_error_type == ET_ok);
  nassertv(group < 32);

  ptr()->setGroup(group);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_group
//       Access: Published
//  Description: Retrieves the collision group set for this shape. 
//               The collision group is an integer between 0 and
//               31.
////////////////////////////////////////////////////////////////////
unsigned short PhysxShape::
get_group() const {

  nassertr(_error_type == ET_ok, 0);

  return ptr()->getGroup();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_local_pos
//       Access: Published
//  Description: Set the position of the shape in actor space, i.e.
//               relative to the actor it is owned by.
//
//               Calling this method does NOT wake the associated
//               actor up automatically.
//
//               Calling this method does not automatically update
//               the inertia properties of the owning actor (if
//               applicable); use
//               PhysxActor::update_mass_from_shapes() to do this.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_local_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);

  ptr()->setLocalPosition(PhysxManager::point3_to_nxVec3(pos));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_local_pos
//       Access: Published
//  Description: Retrieve the position of the shape in actor space,
//               i.e. relative to the actor it is owned by. 
////////////////////////////////////////////////////////////////////
LPoint3f PhysxShape::
get_local_pos() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());

  return PhysxManager::nxVec3_to_point3(ptr()->getLocalPosition());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_local_mat
//       Access: Published
//  Description: Set the transform of the shape in actor space,
//               i.e. relative to the actor it is owned by.
//
//               Calling this method does NOT wake the associated
//               actor up automatically.
//
//               Calling this method does not automatically update
//               the inertia properties of the owning actor (if
//               applicable); use
//               PhysxActor::update_mass_from_shapes() to do this.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_local_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);

  ptr()->setLocalPose(PhysxManager::mat4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_local_mat
//       Access: Published
//  Description: Retrieve the transform of the shape in actor space,
//               i.e. relative to the actor it is owned by. 
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxShape::
get_local_mat() const {

  nassertr(_error_type == ET_ok, LMatrix4f::zeros_mat());

  return PhysxManager::nxMat34_to_mat4(ptr()->getLocalPose());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_material_index
//       Access: Published
//  Description: Returns the material index currently assigned to
//               the shape. 
////////////////////////////////////////////////////////////////////
unsigned short PhysxShape::
get_material_index() const {

  nassertr(_error_type == ET_ok, 0);
  NxMaterialIndex index = ptr()->getMaterial();
  return (unsigned int)index;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_material
//       Access: Published
//  Description: Assigns a material to the shape.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_material(const PhysxMaterial &material) {

  nassertv(_error_type == ET_ok);
  ptr()->setMaterial(material.ptr()->getMaterialIndex());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_material_index
//       Access: Published
//  Description: Assigns a material index to the shape. 
//
//               The material index can be retrieved by calling
//               PhysxMaterial::get_material_index(). If the material
//               index is invalid, it will still be recorded, but
//               the default material (at index 0) will effectively
//               be used for simulation.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_material_index(unsigned short index) {

  nassertv(_error_type == ET_ok);
  ptr()->setMaterial((NxMaterialIndex)index);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_groups_mask
//       Access: Published
//  Description: Sets 128-bit mask used for collision filtering.
//               Does NOT wake the associated actor up
//               automatically.
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_groups_mask(const PhysxGroupsMask &mask) {

  nassertv(_error_type == ET_ok);
  ptr()->setGroupsMask(mask.get_mask());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_groups_mask
//       Access: Published
//  Description: Gets 128-bit mask used for collision filtering.
////////////////////////////////////////////////////////////////////
PhysxGroupsMask PhysxShape::
get_groups_mask() const {

  PhysxGroupsMask mask;
  nassertr(_error_type == ET_ok, mask);
  mask.set_mask(ptr()->getGroupsMask());
  return mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_world_bounds
//       Access: Published
//  Description: Returns a world space AABB enclosing this shape.
////////////////////////////////////////////////////////////////////
PhysxBounds3 PhysxShape::
get_world_bounds() const {

  PhysxBounds3 bounds;
  nassertr(_error_type == ET_ok, bounds);
  ptr()->getWorldBounds(bounds._bounds);
  return bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::check_overlap_aabb
//       Access: Published
//  Description: Checks whether the shape overlaps a world-space
//               AABB or not.
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_aabb(const PhysxBounds3 &world_bounds) const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->checkOverlapAABB(world_bounds._bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::check_overlap_capsule
//       Access: Published
//  Description: Checks whether the shape overlaps a world-space
//               capsule or not.
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_capsule(const PhysxCapsule &world_capsule) const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->checkOverlapCapsule(world_capsule._capsule);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::check_overlap_obb
//       Access: Published
//  Description: Checks whether the shape overlaps a world-space
//               OBB or not.
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_obb(const PhysxBox &world_box) const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->checkOverlapOBB(world_box._box);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::check_overlap_sphere
//       Access: Published
//  Description: Checks whether the shape overlaps a world-space
//               sphere or not.
////////////////////////////////////////////////////////////////////
bool PhysxShape::
check_overlap_sphere(const PhysxSphere &world_sphere) const {

  nassertr(_error_type == ET_ok, false);
  return ptr()->checkOverlapSphere(world_sphere._sphere);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::raycast
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxRaycastHit PhysxShape::
raycast(const PhysxRay &worldRay, bool firstHit, bool smoothNormal) const {

  NxRaycastHit hit;
  nassertr(_error_type == ET_ok, hit);

  NxU32 hints = NX_RAYCAST_SHAPE | NX_RAYCAST_IMPACT | NX_RAYCAST_DISTANCE;
  if (smoothNormal == true) {
    hints |= NX_RAYCAST_NORMAL;
  }
  else {
    hints |= NX_RAYCAST_FACE_NORMAL;
  }

  ptr()->raycast(worldRay._ray, worldRay._length, hints, hit, firstHit);
  return PhysxRaycastHit(hit);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::set_ccd_skeleton
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxShape::
set_ccd_skeleton(PhysxCcdSkeleton *skel) {

  nassertv(_error_type == ET_ok);

  ptr()->setCCDSkeleton(skel->ptr());
  _skel = skel;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxShape::get_ccd_skeleton
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxCcdSkeleton *PhysxShape::
get_ccd_skeleton() const {

  nassertr(_error_type == ET_ok, NULL);

  return _skel;
}

