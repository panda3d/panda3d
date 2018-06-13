/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dFloatObject.cxx
 * @author drose
 * @date 2009-06-30
 */

#include "p3dFloatObject.h"

using std::ostringstream;
using std::string;

/**
 *
 */
P3DFloatObject::
P3DFloatObject(double value) : _value(value) {
}

/**
 *
 */
P3DFloatObject::
P3DFloatObject(const P3DFloatObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DFloatObject::
get_type() {
  return P3D_OT_float;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DFloatObject::
get_bool() {
  return (_value != 0.0);
}

/**
 * Returns the object value coerced to an integer, if possible.
 */
int P3DFloatObject::
get_int() {
  return (int)_value;
}

/**
 * Returns the object value coerced to a floating-point value, if possible.
 */
double P3DFloatObject::
get_float() {
  return _value;
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
void P3DFloatObject::
make_string(string &value) {
  ostringstream strm;
  strm << _value;
  value = strm.str();
}
