/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxControllerReport.I
 * @author enn0x
 * @date 2009-09-24
 */

/**
 *
 */
INLINE PhysxControllerReport::
PhysxControllerReport() : NxUserControllerHitReport() {

  _enabled = false;
}

/**
 *
 */
INLINE PhysxControllerReport::
~PhysxControllerReport() {

}

/**
 *
 */
INLINE void PhysxControllerReport::
set_shape_hit_callback(PT(CallbackObject) cbobj) {

  _shape_hit_cbobj = cbobj;
}

/**
 *
 */
INLINE void PhysxControllerReport::
set_controller_hit_callback(PT(CallbackObject) cbobj) {

  _controller_hit_cbobj = cbobj;
}
