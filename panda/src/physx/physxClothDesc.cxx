/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothDesc.cxx
 * @author enn0x
 * @date 2010-03-30
 */

#include "physxClothDesc.h"
#include "physxClothMesh.h"
#include "physxManager.h"

/**

 */
void PhysxClothDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

/**

 */
void PhysxClothDesc::
set_global_pos(const LPoint3f &pos) {

  _desc.globalPose.t = PhysxManager::point3_to_nxVec3(pos);
}

/**

 */
void PhysxClothDesc::
set_global_mat(const LMatrix4f &mat) {

  _desc.globalPose = PhysxManager::mat4_to_nxMat34(mat);
}

/**

 */
void PhysxClothDesc::
set_global_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  _desc.globalPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

/**

 */
void PhysxClothDesc::
set_thickness(float thickness) {

  _desc.thickness = thickness;
}

/**

 */
void PhysxClothDesc::
set_density(float density) {

  _desc.density = density;
}

/**

 */
void PhysxClothDesc::
set_bending_stiffness(float stiffness) {

  _desc.bendingStiffness = stiffness;
}

/**

 */
void PhysxClothDesc::
set_stretching_stiffness(float stiffness) {

  _desc.stretchingStiffness = stiffness;
}

/**

 */
void PhysxClothDesc::
set_damping_coefficient(float damping) {

  _desc.dampingCoefficient = damping;
}

/**

 */
void PhysxClothDesc::
set_friction(float friction) {

  _desc.friction = friction;
}

/**

 */
void PhysxClothDesc::
set_pressure(float pressure) {

  _desc.pressure = pressure;
}

/**

 */
void PhysxClothDesc::
set_tear_factor(float tearFactor) {

  _desc.tearFactor = tearFactor;
}

/**
 * Raise or lower individual ClothFlag flags.
 */
void PhysxClothDesc::
set_flag(PhysxClothFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

/**
 * Number of solver iterations.  Small numbers make the simulation faster while
 * the cloth gets less stiff.
 */
void PhysxClothDesc::
set_solver_iterations(unsigned int iterations) {

  _desc.solverIterations = iterations;
}

/**

 */
void PhysxClothDesc::
set_cloth_mesh(PhysxClothMesh *mesh) {

  _desc.clothMesh = mesh->ptr();
}

/**

 */
const char *PhysxClothDesc::
get_name() const {

  return _desc.name;
}

/**

 */
LPoint3f PhysxClothDesc::
get_global_pos() const {

  return PhysxManager::nxVec3_to_point3(_desc.globalPose.t);
}

/**

 */
LMatrix4f PhysxClothDesc::
get_global_mat() const {

  return PhysxManager::nxMat34_to_mat4(_desc.globalPose);
}

/**

 */
float PhysxClothDesc::
get_thickness() const {

  return _desc.thickness;
}

/**

 */
float PhysxClothDesc::
get_density() const {

  return _desc.density;
}

/**

 */
float PhysxClothDesc::
get_bending_stiffness() const {

  return _desc.bendingStiffness;
}

/**

 */
float PhysxClothDesc::
get_stretching_stiffness() const {

  return _desc.stretchingStiffness;
}

/**

 */
float PhysxClothDesc::
get_damping_coefficient() const {

  return _desc.dampingCoefficient;
}

/**

 */
float PhysxClothDesc::
get_friction() const {

  return _desc.friction;
}

/**

 */
float PhysxClothDesc::
get_pressure() const {

  return _desc.pressure;
}

/**

 */
float PhysxClothDesc::
get_tear_factor() const {

  return _desc.tearFactor;
}

/**

 */
bool PhysxClothDesc::
get_flag(PhysxClothFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

/**

 */
unsigned int PhysxClothDesc::
get_solver_iterations() const {

  return _desc.solverIterations;
}

/*
/**
 * Used by PhysScene to query the sizes of arrays to allocate for the user
 * buffers in PhysxClothNode.
 */
void PhysxClothDesc::
get_mesh_numbers(NxU32 &numVertices, NxU32 &numTriangles) {

  NxClothMeshDesc meshDesc;
  _desc.clothMesh->saveToDesc(meshDesc);

  numVertices = meshDesc.numVertices;
  numTriangles = meshDesc.numTriangles;
}
*/
