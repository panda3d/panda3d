/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTickCallbackData.I
 * @author enn0x
 * @date 2012-11-26
 */

/**
 *
 */
INLINE BulletTickCallbackData::
BulletTickCallbackData(btScalar timestep) :
  _timestep(timestep) {

}

/**
 *
 */
INLINE PN_stdfloat BulletTickCallbackData::
get_timestep() const {

  return (PN_stdfloat)_timestep;
}
