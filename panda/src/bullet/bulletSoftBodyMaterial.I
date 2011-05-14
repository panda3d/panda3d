// Filename: bulletSoftBodyMaterial.I
// Created by:  enn0x (19Mar11)
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

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyMaterial::
~BulletSoftBodyMaterial() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::empty
//       Access: Published
//  Description: Named constructor intended to be used for asserts
//               with have to return a concrete value.
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyMaterial BulletSoftBodyMaterial::
empty() {

  static btSoftBody::Material material;

  return BulletSoftBodyMaterial(material);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_material
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btSoftBody::Material &BulletSoftBodyMaterial::
get_material() const {

  return _material;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::getLinearStiffness
//       Access: Published
//  Description: Getter for the property m_kLST.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyMaterial::
getLinearStiffness() const {

  return _material.m_kLST;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::setLinearStiffness
//       Access: Published
//  Description: Setter for the property m_kLST.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyMaterial::
setLinearStiffness(float value) {

  _material.m_kLST = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::getAngularStiffness
//       Access: Published
//  Description: Getter for the property m_kAST.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyMaterial::
getAngularStiffness() const {

  return _material.m_kAST;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::setAngularStiffness
//       Access: Published
//  Description: Setter for the property m_kAST.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyMaterial::
setAngularStiffness(float value) {

  _material.m_kAST = value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::getVolumePreservation
//       Access: Published
//  Description: Getter for the property m_kVST.
////////////////////////////////////////////////////////////////////
INLINE float BulletSoftBodyMaterial::
getVolumePreservation() const {

  return _material.m_kVST;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyMaterial::setVolumePreservation
//       Access: Published
//  Description: Setter for the property m_kVST.
////////////////////////////////////////////////////////////////////
INLINE void BulletSoftBodyMaterial::
setVolumePreservation(float value) {

  _material.m_kVST = value;
}

