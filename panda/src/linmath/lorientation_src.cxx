/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lorientation_src.cxx
 * @author frang, charles
 * @date 2000-06-23
 */

TypeHandle FLOATNAME(LOrientation)::_type_handle;

/**

 */
void FLOATNAME(LOrientation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LQuaternion)::init_type();
    register_type(_type_handle, FLOATNAME_STR(LOrientation),
                  FLOATNAME(LQuaternion)::get_class_type());
  }
}
