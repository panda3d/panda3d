/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dNoneObject.cxx
 * @author drose
 * @date 2009-06-30
 */

#include "p3dNoneObject.h"

/**
 *
 */
P3DNoneObject::
P3DNoneObject() {
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DNoneObject::
get_type() {
  return P3D_OT_none;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DNoneObject::
get_bool() {
  return false;
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
void P3DNoneObject::
make_string(string &value) {
  value = "None";
}
