/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyConfig.cxx
 * @author enn0x
 * @date 2010-04-12
 */

#include "bulletSoftBodyConfig.h"

/**
 *
 */
BulletSoftBodyConfig::
BulletSoftBodyConfig(btSoftBody::Config &cfg) : _cfg(cfg) {

}

/**
 *
 */
void BulletSoftBodyConfig::
clear_all_collision_flags() {

  _cfg.collisions = 0;
}

/**
 *
 */
void BulletSoftBodyConfig::
set_collision_flag(CollisionFlag flag, bool value) {

  if (value == true) {
    _cfg.collisions |= flag;
  }
  else {
    _cfg.collisions &= ~(flag);
  }
}

/**
 *
 */
bool BulletSoftBodyConfig::
get_collision_flag(CollisionFlag flag) const {

  return (_cfg.collisions & flag) ? true : false;
}

/**
 *
 */
void BulletSoftBodyConfig::
set_aero_model(AeroModel value) {

  _cfg.aeromodel = (btSoftBody::eAeroModel::_)value;
}

/**
 *
 */
BulletSoftBodyConfig::AeroModel BulletSoftBodyConfig::
get_aero_model() const {

  return (AeroModel)_cfg.aeromodel;
}
