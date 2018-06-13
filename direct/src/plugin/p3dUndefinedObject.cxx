/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dUndefinedObject.cxx
 * @author drose
 * @date 2009-07-07
 */

#include "p3dUndefinedObject.h"

using std::string;

/**
 *
 */
P3DUndefinedObject::
P3DUndefinedObject() {
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DUndefinedObject::
get_type() {
  return P3D_OT_undefined;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DUndefinedObject::
get_bool() {
  return false;
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
void P3DUndefinedObject::
make_string(string &value) {
  value = "Undefined";
}
