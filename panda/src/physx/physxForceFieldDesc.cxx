// Filename: physxForceFieldDesc.cxx
// Created by:  enn0x (06Nov09)
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

#include "physxForceFieldDesc.h"
#include "physxForceFieldShapeDesc.h"
#include "physxForceFieldShapeGroup.h"
#include "physxManager.h"
#include "physxActor.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_pos(const LPoint3f &pos) {

  _desc.pose.t = PhysxManager::point3_to_nxVec3(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_mat
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_mat(const LMatrix4f &mat) {

  _desc.pose = PhysxManager::mat4_to_nxMat34(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_hpr
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.pose.M = PhysxManager::mat3_to_nxMat33(rot);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_constant
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_constant(const LVector3f &constant) {

  _kernel.constant = PhysxManager::vec3_to_nxVec3(constant);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_position_target
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_position_target(const LPoint3f &target) {

  _kernel.positionTarget = PhysxManager::point3_to_nxVec3(target);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_velocity_target
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_velocity_target(const LVector3f &target) {

  _kernel.velocityTarget = PhysxManager::vec3_to_nxVec3(target);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_torus_radius
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_torus_radius(float radius) {

  _kernel.torusRadius = radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_falloff_linear
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_falloff_linear(const LVector3f &falloff) {

  _kernel.falloffLinear = PhysxManager::vec3_to_nxVec3(falloff);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_falloff_quadratic
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_falloff_quadratic(const LVector3f &falloff) {

  _kernel.falloffQuadratic = PhysxManager::vec3_to_nxVec3(falloff);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_noise
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_noise(const LVector3f &noise) {

  _kernel.noise = PhysxManager::vec3_to_nxVec3(noise);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_position_multiplier
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_position_multiplier(const LMatrix3f &multiplier) {

  _kernel.positionMultiplier = PhysxManager::mat3_to_nxMat33(multiplier);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_kernel_velocity_multiplier
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_kernel_velocity_multiplier(const LMatrix3f &multiplier) {

  _kernel.velocityMultiplier = PhysxManager::mat3_to_nxMat33(multiplier);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::create_kernel
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
create_kernel(NxScene *scenePtr) {

  _desc.kernel = scenePtr->createForceFieldLinearKernel(_kernel);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_coordinates
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_coordinates(PhysxForceFieldCoordinates coordinates) {

  _desc.coordinates = (NxForceFieldCoordinates) coordinates;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::add_include_group_shape
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
add_include_group_shape(PhysxForceFieldShapeDesc &desc) {

  _desc.includeGroupShapes.push_back(desc.ptr());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::add_shape_group
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
add_shape_group(PhysxForceFieldShapeGroup *group) {

  _desc.shapeGroups.push_back(group->ptr());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldDesc::set_actor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldDesc::
set_actor(PhysxActor *actor) {

  _desc.actor = actor->ptr();
}

