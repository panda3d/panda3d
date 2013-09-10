// Filename: bulletSoftBodyWorldInfo.cxx
// Created by:  enn0x (04Mar10)
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

#include "bulletSoftBodyWorldInfo.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyWorldInfo::
BulletSoftBodyWorldInfo(btSoftBodyWorldInfo &info) : _info(info) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::garbage_collect
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
garbage_collect(int lifetime) {

  _info.m_sparsesdf.GarbageCollect(lifetime);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_air_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_air_density(PN_stdfloat density) {

  _info.air_density = (btScalar)density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_water_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_water_density(PN_stdfloat density) {

  _info.water_density = (btScalar)density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_water_offset
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_water_offset(PN_stdfloat offset) {

  _info.water_offset = (btScalar)offset;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_water_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_water_normal(const LVector3 &normal) {

  nassertv(!normal.is_nan());
  _info.water_normal.setValue(normal.get_x(), normal.get_y(), normal.get_z());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_gravity(const LVector3 &gravity) {

  nassertv(!gravity.is_nan());
  _info.m_gravity.setValue(gravity.get_x(), gravity.get_y(), gravity.get_z());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_air_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletSoftBodyWorldInfo::
get_air_density() const {

  return (PN_stdfloat)_info.air_density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_water_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletSoftBodyWorldInfo::
get_water_density() const {

  return (PN_stdfloat)_info.water_density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_water_offset
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletSoftBodyWorldInfo::
get_water_offset() const {

  return (PN_stdfloat)_info.water_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_water_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletSoftBodyWorldInfo::
get_water_normal() const {

  return btVector3_to_LVector3(_info.water_normal);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletSoftBodyWorldInfo::
get_gravity() const {

  return btVector3_to_LVector3(_info.m_gravity);
}

