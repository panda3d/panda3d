// Filename: physxClothDesc.cxx
// Created by:  enn0x (30Mar10)
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

#include "physxClothDesc.h"
#include "physxClothMesh.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_global_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_global_pos(const LPoint3f &pos) {

  _desc.globalPose.t = PhysxManager::point3_to_nxVec3(pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_global_mat
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_global_mat(const LMatrix4f &mat) {

  _desc.globalPose = PhysxManager::mat4_to_nxMat34(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_global_hpr
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_global_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.globalPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_thickness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_thickness(float thickness) {

  _desc.thickness = thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_density
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_density(float density) {

  _desc.density = density;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_bending_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_bending_stiffness(float stiffness) {

  _desc.bendingStiffness = stiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_stretching_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_stretching_stiffness(float stiffness) {

  _desc.stretchingStiffness = stiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_damping_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_damping_coefficient(float damping) {

  _desc.dampingCoefficient = damping;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_friction
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_friction(float friction) {

  _desc.friction = friction;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_pressure
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_pressure(float pressure) {

  _desc.pressure = pressure;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_tear_factor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_tear_factor(float tearFactor) {

  _desc.tearFactor = tearFactor;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_flag
//       Access: Published
//  Description: Raise or lower individual ClothFlag flags.
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_flag(PhysxClothFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_solver_iterations
//       Access: Published
//  Description: Number of solver iterations.
//               Small numbers make the simulation faster while 
//               the cloth gets less stiff.
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_solver_iterations(unsigned int iterations) {

  _desc.solverIterations = iterations;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::set_cloth_mesh
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
set_cloth_mesh(PhysxClothMesh *mesh) {

  _desc.clothMesh = mesh->ptr();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
const char *PhysxClothDesc::
get_name() const {

  return _desc.name;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_global_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f PhysxClothDesc::
get_global_pos() const {

  return PhysxManager::nxVec3_to_point3(_desc.globalPose.t);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_global_mat
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxClothDesc::
get_global_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.globalPose);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_thickness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_thickness() const {

  return _desc.thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_density
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_density() const {

  return _desc.density;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_bending_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_bending_stiffness() const {

  return _desc.bendingStiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_stretching_stiffness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_stretching_stiffness() const {

  return _desc.stretchingStiffness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_damping_coefficient
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_damping_coefficient() const {

  return _desc.dampingCoefficient;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_friction
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_friction() const {

  return _desc.friction;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_pressure
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_pressure() const {

  return _desc.pressure;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_tear_factor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxClothDesc::
get_tear_factor() const {

  return _desc.tearFactor;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxClothDesc::
get_flag(PhysxClothFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_solver_iterations
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxClothDesc::
get_solver_iterations() const {

  return _desc.solverIterations;
}

/*
////////////////////////////////////////////////////////////////////
//     Function: PhysxClothDesc::get_mesh_numbers
//       Access: Public
//  Description: Used by PhysScene to query the sizes of arrays
//               to allocate for the user buffers in PhysxClothNode.
////////////////////////////////////////////////////////////////////
void PhysxClothDesc::
get_mesh_numbers(NxU32 &numVertices, NxU32 &numTriangles) {

  NxClothMeshDesc meshDesc;
  _desc.clothMesh->saveToDesc(meshDesc);

  numVertices = meshDesc.numVertices;
  numTriangles = meshDesc.numTriangles;
}
*/

