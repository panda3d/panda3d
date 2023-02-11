/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase4_src.cxx
 * @author drose
 * @date 2000-03-08
 */

TypeHandle FLOATNAME(LVecBase4)::_type_handle;

const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_zero =
  FLOATNAME(LVecBase4)(0, 0, 0, 0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_x =
  FLOATNAME(LVecBase4)(1, 0, 0, 0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_y =
  FLOATNAME(LVecBase4)(0, 1, 0, 0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_z =
  FLOATNAME(LVecBase4)(0, 0, 1, 0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_w =
  FLOATNAME(LVecBase4)(0, 0, 0, 1);

/**
 *
 */
void FLOATNAME(LVecBase4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    register_type(_type_handle, FLOATNAME_STR(LVecBase4));
  }
}
