// Filename: physxSoftBodyDesc.cxx
// Created by:  enn0x (12Sep10)
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

#include "physxSoftBodyDesc.h"
#include "physxSoftBodyMesh.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_global_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_global_pos(const LPoint3f &pos) {

  _desc.globalPose.t = PhysxManager::point3_to_nxVec3(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_global_mat
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_global_mat(const LMatrix4f &mat) {

  _desc.globalPose = PhysxManager::mat4_to_nxMat34(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_global_hpr
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_global_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.globalPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_particle_radius
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_particle_radius(float radius) {

  _desc.particleRadius = radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_relative_grid_spacing
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_relative_grid_spacing(float spacing) {

  _desc.relativeGridSpacing = spacing;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_collision_response_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_collision_response_coefficient(float coef) {

  _desc.collisionResponseCoefficient = coef;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_attachment_response_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_attachment_response_coefficient(float coef) {

  _desc.attachmentResponseCoefficient = coef;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_density
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_density(float density) {

  _desc.density = density;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_volume_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_volume_stiffness(float stiffness) {

  _desc.volumeStiffness = stiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_stretching_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_stretching_stiffness(float stiffness) {

  _desc.stretchingStiffness = stiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_damping_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_damping_coefficient(float damping) {

  _desc.dampingCoefficient = damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_friction
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_friction(float friction) {

  _desc.friction = friction;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_tear_factor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_tear_factor(float tearFactor) {

  _desc.tearFactor = tearFactor;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_flag
//       Access: Published
//  Description: Raise or lower individual SoftBodyFlag flags.
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_flag(PhysxSoftBodyFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_solver_iterations
//       Access: Published
//  Description: Number of solver iterations.
//               Small numbers make the simulation faster while 
//               the soft body gets less stiff.
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_solver_iterations(unsigned int iterations) {

  _desc.solverIterations = iterations;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::set_soft_body_mesh
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
set_soft_body_mesh(PhysxSoftBodyMesh *mesh) {

  _desc.softBodyMesh = mesh->ptr();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
const char *PhysxSoftBodyDesc::
get_name() const {

  return _desc.name;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_global_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f PhysxSoftBodyDesc::
get_global_pos() const {

  return PhysxManager::nxVec3_to_point3(_desc.globalPose.t);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_global_mat
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxSoftBodyDesc::
get_global_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.globalPose);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_particle_radius
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_particle_radius() const {

  return _desc.particleRadius;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_relative_grid_spacing
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_relative_grid_spacing() const {

  return _desc.relativeGridSpacing;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_collision_response_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_collision_response_coefficient() const {

  return _desc.collisionResponseCoefficient;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_attachment_response_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_attachment_response_coefficient() const {

  return _desc.attachmentResponseCoefficient;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_density
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_density() const {

  return _desc.density;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_volume_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_volume_stiffness() const {

  return _desc.volumeStiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_stretching_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_stretching_stiffness() const {

  return _desc.stretchingStiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_damping_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_damping_coefficient() const {

  return _desc.dampingCoefficient;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_friction
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_friction() const {

  return _desc.friction;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_tear_factor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxSoftBodyDesc::
get_tear_factor() const {

  return _desc.tearFactor;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxSoftBodyDesc::
get_flag(PhysxSoftBodyFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_solver_iterations
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxSoftBodyDesc::
get_solver_iterations() const {

  return _desc.solverIterations;
}

/*
////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyDesc::get_mesh_numbers
//       Access: Public
//  Description: Used by PhysScene to query the sizes of arrays
//               to allocate for the user buffers in PhysxSoftBodyNode.
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyDesc::
get_mesh_numbers(NxU32 &numVertices, NxU32 &numTriangles) {

  NxSoftBodyMeshDesc meshDesc;
  _desc.clothMesh->saveToDesc(meshDesc);

  numVertices = meshDesc.numVertices;
  numTriangles = meshDesc.numTriangles;
}
*/

