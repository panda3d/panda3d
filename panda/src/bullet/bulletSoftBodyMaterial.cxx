/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyMaterial.cxx
 * @author enn0x
 * @date 2011-03-19
 */

#include "bulletSoftBodyMaterial.h"

#include "bulletWorld.h"

/**
 *
 */
BulletSoftBodyMaterial::
BulletSoftBodyMaterial(btSoftBody::Material &material) : _material(material) {

}

/**
 * Getter for the property m_kLST.
 */
PN_stdfloat BulletSoftBodyMaterial::
get_linear_stiffness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_material.m_kLST;
}

/**
 * Setter for the property m_kLST.
 */
void BulletSoftBodyMaterial::
set_linear_stiffness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _material.m_kLST = (btScalar)value;
}

/**
 * Getter for the property m_kAST.
 */
PN_stdfloat BulletSoftBodyMaterial::
get_angular_stiffness() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_material.m_kAST;
}

/**
 * Setter for the property m_kAST.
 */
void BulletSoftBodyMaterial::
set_angular_stiffness(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _material.m_kAST = (btScalar)value;
}

/**
 * Getter for the property m_kVST.
 */
PN_stdfloat BulletSoftBodyMaterial::
get_volume_preservation() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_material.m_kVST;
}

/**
 * Setter for the property m_kVST.
 */
void BulletSoftBodyMaterial::
set_volume_preservation(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _material.m_kVST = (btScalar)value;
}
