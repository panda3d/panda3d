/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxActorDesc.cxx
 * @author enn0x
 * @date 2009-09-05
 */

#include "physxActorDesc.h"
#include "physxBodyDesc.h"
#include "physxManager.h"

/**
 * Adds a shape to the list of collision shapes composing this actor.
 */
void PhysxActorDesc::
add_shape(PhysxShapeDesc &desc) {

  _desc.shapes.push_back(desc.ptr());
}

/**
 * Sets the optional debug name for the actor.
 */
void PhysxActorDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

/**
 * Set the density used during mass/intertia computation.  This value is used
 * if the actor's shapes do not have a mass asigned.
 */
void PhysxActorDesc::
set_density(float density) {

  _desc.density = density;
}

/**
 * Set the position of the actor in global space.
 */
void PhysxActorDesc::
set_global_pos(const LPoint3f &pos) {

  _desc.globalPose.t = PhysxManager::point3_to_nxVec3(pos);
}

/**
 * Set the position and orientation of the actor in global space.  Scaling and
 * shear arenot supported, even if the matrix contains a scale or shear.
 */
void PhysxActorDesc::
set_global_mat(const LMatrix4f &mat) {

  _desc.globalPose = PhysxManager::mat4_to_nxMat34(mat);
}

/**
 * Sets the orientation of the actor in global space by providing angles for
 * heading, pitch and roll.
 */
void PhysxActorDesc::
set_global_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.globalPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

/**
 * Sets the body descriptor for this actor.  The actor will be dynmaic if a
 * body descriptor is set, and static if no body descriptor is set.
 */
void PhysxActorDesc::
set_body(PhysxBodyDesc &desc) {

  _desc.body = &(desc._desc);
}

/**
 * Gets the body descriptor for this actor.
 */
PhysxBodyDesc PhysxActorDesc::
get_body() const {
  assert(false /* Not implemented */);

  // PhysxBodyDesc value; value._desc = *(_desc.body); return value;
  return PhysxBodyDesc();
}

/**
 * Returns the optional debug name for this actor.
 */
const char *PhysxActorDesc::
get_name() const {

  return _desc.name;
}

/**
 * Returns the actor's density.
 */
float PhysxActorDesc::
get_density() const {

  return _desc.density;
}

/**
 * Returns the actor's position in global space.
 */
LPoint3f PhysxActorDesc::
get_global_pos() const {

  return PhysxManager::nxVec3_to_point3(_desc.globalPose.t);
}

/**
 * Returns the actor's transform in global space.
 */
LMatrix4f PhysxActorDesc::
get_global_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.globalPose);
}
