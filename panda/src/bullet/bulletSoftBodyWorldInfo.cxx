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
set_air_density(float density) {

  _info.air_density = density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_water_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_water_density(float density) {

  _info.water_density = density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_water_offset
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_water_offset(float offset) {

  _info.water_offset = offset;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_water_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_water_normal(const LVector3f &normal) {

  nassertv(!normal.is_nan());
  _info.water_normal = LVecBase3f_to_btVector3(normal);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::set_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyWorldInfo::
set_gravity(const LVector3f &gravity) {

  nassertv(!gravity.is_nan());
  _info.m_gravity = LVecBase3f_to_btVector3(gravity);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_air_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSoftBodyWorldInfo::
get_air_density() const {

  return _info.air_density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_water_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSoftBodyWorldInfo::
get_water_density() const {

  return _info.water_density;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_water_offset
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletSoftBodyWorldInfo::
get_water_offset() const {

  return _info.water_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_water_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletSoftBodyWorldInfo::
get_water_normal() const {

  return btVector3_to_LVector3f(_info.water_normal);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyWorldInfo::get_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletSoftBodyWorldInfo::
get_gravity() const {

  return btVector3_to_LVector3f(_info.m_gravity);
}

