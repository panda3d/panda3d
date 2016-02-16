/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyWorldInfo.cxx
 * @author enn0x
 * @date 2010-03-04
 */

#include "bulletSoftBodyWorldInfo.h"

/**

 */
BulletSoftBodyWorldInfo::
BulletSoftBodyWorldInfo(btSoftBodyWorldInfo &info) : _info(info) {

}

/**

 */
void BulletSoftBodyWorldInfo::
garbage_collect(int lifetime) {

  _info.m_sparsesdf.GarbageCollect(lifetime);
}

/**

 */
void BulletSoftBodyWorldInfo::
set_air_density(PN_stdfloat density) {

  _info.air_density = (btScalar)density;
}

/**

 */
void BulletSoftBodyWorldInfo::
set_water_density(PN_stdfloat density) {

  _info.water_density = (btScalar)density;
}

/**

 */
void BulletSoftBodyWorldInfo::
set_water_offset(PN_stdfloat offset) {

  _info.water_offset = (btScalar)offset;
}

/**

 */
void BulletSoftBodyWorldInfo::
set_water_normal(const LVector3 &normal) {

  nassertv(!normal.is_nan());
  _info.water_normal.setValue(normal.get_x(), normal.get_y(), normal.get_z());
}

/**

 */
void BulletSoftBodyWorldInfo::
set_gravity(const LVector3 &gravity) {

  nassertv(!gravity.is_nan());
  _info.m_gravity.setValue(gravity.get_x(), gravity.get_y(), gravity.get_z());
}

/**

 */
PN_stdfloat BulletSoftBodyWorldInfo::
get_air_density() const {

  return (PN_stdfloat)_info.air_density;
}

/**

 */
PN_stdfloat BulletSoftBodyWorldInfo::
get_water_density() const {

  return (PN_stdfloat)_info.water_density;
}

/**

 */
PN_stdfloat BulletSoftBodyWorldInfo::
get_water_offset() const {

  return (PN_stdfloat)_info.water_offset;
}

/**

 */
LVector3 BulletSoftBodyWorldInfo::
get_water_normal() const {

  return btVector3_to_LVector3(_info.water_normal);
}

/**

 */
LVector3 BulletSoftBodyWorldInfo::
get_gravity() const {

  return btVector3_to_LVector3(_info.m_gravity);
}
