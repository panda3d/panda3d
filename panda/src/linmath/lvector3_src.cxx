/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvector3_src.cxx
 * @author drose
 * @date 2000-03-08
 */

TypeHandle FLOATNAME(LVector3)::_type_handle;

/**
 *
 */
void FLOATNAME(LVector3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LVector3),
                  FLOATNAME(LVecBase3)::get_class_type());
  }
}
