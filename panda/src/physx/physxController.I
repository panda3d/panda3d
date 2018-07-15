/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxController.I
 * @author enn0x
 * @date 2009-09-24
 */

/**
 *
 */
INLINE PhysxController::
PhysxController() : PhysxObject() {

  _speed = NxVec3(0.0f, 0.0f, 0.0f);
  _omega = 0.0f;
  _heading = 0.0f;

  _jumping = false;
  _jump_time = 0.0f;
  _jump_v0 = 0.0f;

  _sharpness = 1.0f;
  _min_dist = 0.0001f;

  _up_vector = NxVec3(0.0f, 1.0f, 0.0f);
  _up_axis = (NxHeightFieldAxis)physx_up_axis.get_value();

  switch (_up_axis) {
  case NX_Z:
    _up_quat = NxQuat(90.0f, NxVec3(1.0f, 0.0f, 0.0f));
    break;
  case NX_Y:
    _up_quat = NxQuat(0.0f, NxVec3(1.0f, 0.0f, 0.0f));
    break;
  default:
    physx_cat.error() << "only y-up and z-up are permitted" << std::endl;
  }

  _up_quat_inv = _up_quat;
  _up_quat_inv.invert();
}

/**
 *
 */
INLINE void PhysxController::
ls() const {

  ls(nout);
}

/**
 *
 */
INLINE void PhysxController::
ls(std::ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type().get_name()
                            << " (at 0x" << this << ")";

  out << " A:0x" << get_actor();
  out << "\n";
}
