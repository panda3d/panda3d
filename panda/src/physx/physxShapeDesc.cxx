/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxShapeDesc.cxx
 * @author enn0x
 * @date 2009-09-08
 */

#include "physxShapeDesc.h"
#include "physxManager.h"
#include "physxMaterial.h"

/**
 * Sets a possible debug name.
 */
void PhysxShapeDesc::
set_name(const char *name) {

  _name = name ? name : "";
  ptr()->name = _name.c_str();
}

/**
 * This shape will become a trigger shape if this parameter is set to TRUE. It
 * won't take part in collisions, but trigger events if some other shape
 * passes through it.
 */
void PhysxShapeDesc::
set_trigger(bool value) {

  if (value == true) {
    ptr()->shapeFlags |= NX_TRIGGER_ENABLE;
  }
  else {
    ptr()->shapeFlags &= ~(NX_TRIGGER_ENABLE);
  }
}

/**
 *
 */
void PhysxShapeDesc::
set_local_pos(const LPoint3f &pos) {

  nassertv(!pos.is_nan());
  ptr()->localPose.t = PhysxManager::point3_to_nxVec3(pos);
}

/**
 *
 */
void PhysxShapeDesc::
set_local_mat(const LMatrix4f &mat) {

  nassertv(!mat.is_nan());
  ptr()->localPose = PhysxManager::mat4_to_nxMat34(mat);
}

/**
 *
 */
void PhysxShapeDesc::
set_local_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  ptr()->localPose.M = PhysxManager::mat3_to_nxMat33(rot);
}

/**
 * Specifies by how much shapes can interpenetrate.
 *
 * Two shapes will interpenetrate by the sum of their skin widths.  This means
 * that their graphical representations should be adjusted so that they just
 * touch when the shapes are interpenetrating.
 *
 * The default skin width is the 'physx-skin-width' parameter.
 *
 * A skin width sum of zero for two bodies is not permitted because it will
 * lead to an unstable simulation.
 *
 * If your simulation jitters because resting bodies occasionally lose
 * contact, increasing the size of your collision volumes and the skin width
 * may improve things.
 */
void PhysxShapeDesc::
set_skin_width(float skinWidth) {

  nassertv(skinWidth >= 0.0f);
  ptr()->skinWidth = skinWidth;
}

/**
 *
 */
void PhysxShapeDesc::
set_shape_flag(const PhysxShapeFlag flag, bool value) {

  if (value == true) {
    ptr()->shapeFlags |= flag;
  }
  else {
    ptr()->shapeFlags &= ~(flag);
  }
}

/**
 * Sets the mass of this individual shape when computing mass inertial
 * properties for a rigidbody.  When mass<=0.0 then density and volume
 * determine the mass.  Note that this will only be used if the body has a
 * zero inertia tensor, or if you call PhysxActor::update_mass_from_shapes
 * explicitly.
 */
void PhysxShapeDesc::
set_mass(float mass) {

  ptr()->mass = mass;
}

/**
 * Sets the density of this individual shape when computing mass inertial
 * properties for a rigidbody (unless a valid mass >0.0 is provided). Note
 * that this will only be used if the body has a zero inertia tensor, or if
 * you call PhysxActor::update_mass_from_shapes explicitly.
 */
void PhysxShapeDesc::
set_density(float density) {

  nassertv(density > 0.0f);
  ptr()->density = density;
}

/**
 *
 */
void PhysxShapeDesc::
set_group(unsigned short group) {

  ptr()->group = group;
}

/**
 *
 */
const char *PhysxShapeDesc::
get_name() const {

  return ptr()->name;
}

/**
 *
 */
LPoint3f PhysxShapeDesc::
get_local_pos() const {

  return PhysxManager::nxVec3_to_point3(ptr()->localPose.t);
}

/**
 *
 */
LMatrix4f PhysxShapeDesc::
get_local_mat() const {

  return PhysxManager::nxMat34_to_mat4(ptr()->localPose);
}

/**
 *
 */
float PhysxShapeDesc::
get_skin_width() const {

  return ptr()->skinWidth;
}

/**
 *
 */
bool PhysxShapeDesc::
get_shape_flag(const PhysxShapeFlag flag) const {

  return (ptr()->shapeFlags & flag) ? true : false;
}

/**
 *
 */
float PhysxShapeDesc::
get_mass() const {

  return ptr()->mass;
}

/**
 *
 */
float PhysxShapeDesc::
get_density() const {

  return ptr()->density;
}

/**
 *
 */
unsigned short PhysxShapeDesc::
get_group() const {

  return ptr()->group;
}

/**
 *
 */
void PhysxShapeDesc::
set_material(const PhysxMaterial &material) {

  ptr()->materialIndex = material.ptr()->getMaterialIndex();
}

/**
 *
 */
void PhysxShapeDesc::
set_material_index(unsigned short index) {

  ptr()->materialIndex = index;
}

/**
 *
 */
unsigned short PhysxShapeDesc::
get_material_index() const {

  return ptr()->materialIndex;
}
