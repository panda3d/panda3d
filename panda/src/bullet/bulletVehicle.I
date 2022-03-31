/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletVehicle.I
 * @author enn0x
 * @date 2010-02-16
 */

/**
 *
 */
INLINE BulletVehicle::
~BulletVehicle() {

  delete _vehicle;
  delete _raycaster;
}

/**
 *
 */
btRaycastVehicle *BulletVehicle::
get_vehicle() const {

  return _vehicle;
}

/**
 * Returns a reference to the BulletVehicleTuning object of this vehicle which
 * offers various vehicle-global tuning options.  Make sure to configure this
 * before adding wheels!
 */
BulletVehicleTuning &BulletVehicle::
get_tuning() {

  return _tuning;
}

