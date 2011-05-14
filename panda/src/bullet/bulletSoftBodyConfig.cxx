// Filename: bulletSoftBodyConfig.cxx
// Created by:  enn0x (12Apr10)
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

#include "bulletSoftBodyConfig.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyConfig::
BulletSoftBodyConfig(btSoftBody::Config &cfg) : _cfg(cfg) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::clear_collisions
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyConfig::
clear_all_collision_flags() {

  _cfg.collisions = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_collisions
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyConfig::
set_collision_flag(CollisionFlag flag, bool value) {

  if (value == true) {
    _cfg.collisions |= flag;
  }
  else {
    _cfg.collisions &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_collisions
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletSoftBodyConfig::
get_collision_flag(CollisionFlag flag) const {

  return (_cfg.collisions & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::set_aero_model
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyConfig::
set_aero_model(AeroModel value) {

  _cfg.aeromodel = (btSoftBody::eAeroModel::_)value;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyConfig::get_aero_model
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyConfig::AeroModel BulletSoftBodyConfig::
get_aero_model() const {

  return (AeroModel)_cfg.aeromodel;
}

