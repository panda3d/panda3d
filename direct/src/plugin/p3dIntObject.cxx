/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dIntObject.cxx
 * @author drose
 * @date 2009-06-30
 */

#include "p3dIntObject.h"

using std::ostringstream;
using std::string;

/**
 *
 */
P3DIntObject::
P3DIntObject(int value) : _value(value) {
}

/**
 *
 */
P3DIntObject::
P3DIntObject(const P3DIntObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DIntObject::
get_type() {
  return P3D_OT_int;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DIntObject::
get_bool() {
  return (_value != 0);
}

/**
 * Returns the object value coerced to an integer, if possible.
 */
int P3DIntObject::
get_int() {
  return _value;
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
void P3DIntObject::
make_string(string &value) {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}
