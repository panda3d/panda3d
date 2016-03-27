/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvector2_src.cxx
 * @author drose
 * @date 2000-03-08
 */

TypeHandle FLOATNAME(LVector2)::_type_handle;

/**
 *
 */
void FLOATNAME(LVector2)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase2)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LVector2),
                  FLOATNAME(LVecBase2)::get_class_type());
  }
}
