/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldDesc.cxx
 * @author enn0x
 * @date 2009-11-06
 */

#include "physxForceFieldDesc.h"
#include "physxForceFieldShapeDesc.h"
#include "physxForceFieldShapeGroup.h"
#include "physxManager.h"
#include "physxActor.h"

/**

 */
void PhysxForceFieldDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

/**

 */
void PhysxForceFieldDesc::
set_pos(const LPoint3f &pos) {

  _desc.pose.t = PhysxManager::point3_to_nxVec3(pos);
}

/**

 */
void PhysxForceFieldDesc::
set_mat(const LMatrix4f &mat) {

  _desc.pose = PhysxManager::mat4_to_nxMat34(mat);
}

/**

 */
void PhysxForceFieldDesc::
set_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.pose.M = PhysxManager::mat3_to_nxMat33(rot);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_constant(const LVector3f &constant) {

  _kernel.constant = PhysxManager::vec3_to_nxVec3(constant);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_position_target(const LPoint3f &target) {

  _kernel.positionTarget = PhysxManager::point3_to_nxVec3(target);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_velocity_target(const LVector3f &target) {

  _kernel.velocityTarget = PhysxManager::vec3_to_nxVec3(target);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_torus_radius(float radius) {

  _kernel.torusRadius = radius;
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_falloff_linear(const LVector3f &falloff) {

  _kernel.falloffLinear = PhysxManager::vec3_to_nxVec3(falloff);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_falloff_quadratic(const LVector3f &falloff) {

  _kernel.falloffQuadratic = PhysxManager::vec3_to_nxVec3(falloff);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_noise(const LVector3f &noise) {

  _kernel.noise = PhysxManager::vec3_to_nxVec3(noise);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_position_multiplier(const LMatrix3f &multiplier) {

  _kernel.positionMultiplier = PhysxManager::mat3_to_nxMat33(multiplier);
}

/**

 */
void PhysxForceFieldDesc::
set_kernel_velocity_multiplier(const LMatrix3f &multiplier) {

  _kernel.velocityMultiplier = PhysxManager::mat3_to_nxMat33(multiplier);
}

/**

 */
void PhysxForceFieldDesc::
create_kernel(NxScene *scenePtr) {

  _desc.kernel = scenePtr->createForceFieldLinearKernel(_kernel);
}

/**

 */
void PhysxForceFieldDesc::
set_coordinates(PhysxForceFieldCoordinates coordinates) {

  _desc.coordinates = (NxForceFieldCoordinates) coordinates;
}

/**

 */
void PhysxForceFieldDesc::
add_include_group_shape(PhysxForceFieldShapeDesc &desc) {

  _desc.includeGroupShapes.push_back(desc.ptr());
}

/**

 */
void PhysxForceFieldDesc::
add_shape_group(PhysxForceFieldShapeGroup *group) {

  _desc.shapeGroups.push_back(group->ptr());
}

/**

 */
void PhysxForceFieldDesc::
set_actor(PhysxActor *actor) {

  _desc.actor = actor->ptr();
}
