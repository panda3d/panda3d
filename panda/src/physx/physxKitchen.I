/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxKitchen.I
 * @author enn0x
 * @date 2009-10-12
 */

/**
 *
 */
PhysxKitchen::
PhysxKitchen() {

  _cooking = NxGetCookingLib( NX_PHYSICS_SDK_VERSION );

  bool status = _cooking->NxInitCooking();
  nassertv_always( status );
}

/**
 *
 */
PhysxKitchen::
~PhysxKitchen() {

  _cooking->NxCloseCooking();
}
