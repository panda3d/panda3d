/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBodyDesc.cxx
 * @author enn0x
 * @date 2010-09-12
 */

#include "physxSoftBodyDesc.h"
#include "physxSoftBodyMesh.h"
#include "physxManager.h"

/**
 *
 */
void PhysxSoftBodyDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_global_pos(const LPoint3f &pos) {

  _desc.globalPose.t = PhysxManager::point3_to_nxVec3(pos);
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_global_mat(const LMatrix4f &mat) {

  _desc.globalPose = PhysxManager::mat4_to_nxMat34(mat);
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_global_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.globalPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_particle_radius(float radius) {

  _desc.particleRadius = radius;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_relative_grid_spacing(float spacing) {

  _desc.relativeGridSpacing = spacing;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_collision_response_coefficient(float coef) {

  _desc.collisionResponseCoefficient = coef;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_attachment_response_coefficient(float coef) {

  _desc.attachmentResponseCoefficient = coef;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_density(float density) {

  _desc.density = density;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_volume_stiffness(float stiffness) {

  _desc.volumeStiffness = stiffness;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_stretching_stiffness(float stiffness) {

  _desc.stretchingStiffness = stiffness;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_damping_coefficient(float damping) {

  _desc.dampingCoefficient = damping;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_friction(float friction) {

  _desc.friction = friction;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_tear_factor(float tearFactor) {

  _desc.tearFactor = tearFactor;
}

/**
 * Raise or lower individual SoftBodyFlag flags.
 */
void PhysxSoftBodyDesc::
set_flag(PhysxSoftBodyFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 * Number of solver iterations.  Small numbers make the simulation faster
 * while the soft body gets less stiff.
 */
void PhysxSoftBodyDesc::
set_solver_iterations(unsigned int iterations) {

  _desc.solverIterations = iterations;
}

/**
 *
 */
void PhysxSoftBodyDesc::
set_soft_body_mesh(PhysxSoftBodyMesh *mesh) {

  _desc.softBodyMesh = mesh->ptr();
}

/**
 *
 */
const char *PhysxSoftBodyDesc::
get_name() const {

  return _desc.name;
}

/**
 *
 */
LPoint3f PhysxSoftBodyDesc::
get_global_pos() const {

  return PhysxManager::nxVec3_to_point3(_desc.globalPose.t);
}

/**
 *
 */
LMatrix4f PhysxSoftBodyDesc::
get_global_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.globalPose);
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_particle_radius() const {

  return _desc.particleRadius;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_relative_grid_spacing() const {

  return _desc.relativeGridSpacing;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_collision_response_coefficient() const {

  return _desc.collisionResponseCoefficient;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_attachment_response_coefficient() const {

  return _desc.attachmentResponseCoefficient;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_density() const {

  return _desc.density;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_volume_stiffness() const {

  return _desc.volumeStiffness;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_stretching_stiffness() const {

  return _desc.stretchingStiffness;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_damping_coefficient() const {

  return _desc.dampingCoefficient;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_friction() const {

  return _desc.friction;
}

/**
 *
 */
float PhysxSoftBodyDesc::
get_tear_factor() const {

  return _desc.tearFactor;
}

/**
 *
 */
bool PhysxSoftBodyDesc::
get_flag(PhysxSoftBodyFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**
 *
 */
unsigned int PhysxSoftBodyDesc::
get_solver_iterations() const {

  return _desc.solverIterations;
}

/**
 * Used by PhysScene to query the sizes of arrays to allocate for the user
 * buffers in PhysxSoftBodyNode.
 */
/*
void PhysxSoftBodyDesc::
get_mesh_numbers(NxU32 &numVertices, NxU32 &numTriangles) {

  NxSoftBodyMeshDesc meshDesc;
  _desc.clothMesh->saveToDesc(meshDesc);

  numVertices = meshDesc.numVertices;
  numTriangles = meshDesc.numTriangles;
}
*/
