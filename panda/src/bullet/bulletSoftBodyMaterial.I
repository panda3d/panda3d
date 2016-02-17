/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyMaterial.I
 * @author enn0x
 * @date 2011-03-19
 */

/**
 *
 */
INLINE BulletSoftBodyMaterial::
~BulletSoftBodyMaterial() {

}

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletSoftBodyMaterial BulletSoftBodyMaterial::
empty() {

  static btSoftBody::Material material;

  return BulletSoftBodyMaterial(material);
}

/**
 *
 */
INLINE btSoftBody::Material &BulletSoftBodyMaterial::
get_material() const {

  return _material;
}

/**
 * Getter for the property m_kLST.
 */
INLINE PN_stdfloat BulletSoftBodyMaterial::
getLinearStiffness() const {

  return (PN_stdfloat)_material.m_kLST;
}

/**
 * Setter for the property m_kLST.
 */
INLINE void BulletSoftBodyMaterial::
setLinearStiffness(PN_stdfloat value) {

  _material.m_kLST = (btScalar)value;
}

/**
 * Getter for the property m_kAST.
 */
INLINE PN_stdfloat BulletSoftBodyMaterial::
getAngularStiffness() const {

  return (PN_stdfloat)_material.m_kAST;
}

/**
 * Setter for the property m_kAST.
 */
INLINE void BulletSoftBodyMaterial::
setAngularStiffness(PN_stdfloat value) {

  _material.m_kAST = (btScalar)value;
}

/**
 * Getter for the property m_kVST.
 */
INLINE PN_stdfloat BulletSoftBodyMaterial::
getVolumePreservation() const {

  return (PN_stdfloat)_material.m_kVST;
}

/**
 * Setter for the property m_kVST.
 */
INLINE void BulletSoftBodyMaterial::
setVolumePreservation(PN_stdfloat value) {

  _material.m_kVST = (btScalar)value;
}
